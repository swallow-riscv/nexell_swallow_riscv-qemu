/*
 * Nexell Swallow processors Timer emulation.
 *
 * Copyright (C) 2018  Nexell Co., Ltd.
 * Author: Seoji, Kim <seoji@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/sysbus.h"
#include "qemu/timer.h"
#include "qemu-common.h"
#include "qemu/main-loop.h"
#include "hw/ptimer.h"
#include "hw/timer/nexell_timer.h"


#ifdef DEBUG_TIMER
#define DPRINTF(fmt, ...) \
        do { fprintf(stdout, "TIMER: [%24s:%5d] " fmt, __func__, __LINE__, \
                ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while (0)
#endif

#define TIMER_LOG 0

/*** VMState ***/
static const VMStateDescription vmstate_nexell_timer = {
	.name = "nexell.timer.timer",
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_UINT32(id, NexellTIMER),
		VMSTATE_UINT32(freq, NexellTIMER),
		VMSTATE_PTIMER(ptimer, NexellTIMER),
		VMSTATE_UINT32_ARRAY(reg_tcfg, NexellTIMER, 2),
		VMSTATE_UINT32(reg_tcon, NexellTIMER),
		VMSTATE_UINT32(reg_tint_cstat, NexellTIMER),
		VMSTATE_UINT32(reg_tcntb, NexellTIMER),
		VMSTATE_UINT32(reg_tcmpb, NexellTIMER),
		VMSTATE_END_OF_LIST()
	}
};

static const VMStateDescription vmstate_nexell_timer_state = {
	.name = "nexell.timer",
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_STRUCT_ARRAY(timer, NexellTIMERState,
				NEXELL_TIMER_TIMERS_NUM, 0,
				vmstate_nexell_timer, NexellTIMER),
		VMSTATE_END_OF_LIST()
	}
};

static const char *nexell_timer_get_regname(unsigned offset)
{
	switch (offset) {
	case T0CFG0:
		return "T0CFG0";
	case T0CFG1:
		return "T0CFG1";
	case TCON0:
		return "TCON0";
	case TCNTB0:
		return "TCNTB0";
	case TCMPB0:
		return "TCMPB0";
	case TCNTO0:
		return "TCNTO0";
	case TINT_CSTAT0:
		return "TINT_CSTAT0";
	case T1CFG0:
		return "T1CFG0";
	case T1CFG1:
		return "T1CFG0";
	case TCON1:
		return "T1CFG0";
	case TCNTB1:
		return "TCNTB1";
	case TCMPB1:
		return "TCMPB1";
	case TCNTO1:
		return "TCNTO1";
	case TINT_CSTAT1:
		return "TINT_CSTAT1";
	case T2CFG0:
		return "T2CFG0";
	case T2CFG1:
		return "T2CFG1";
	case TCON2:
		return "TCON2";
	case TCNTB2:
		return "TCNTB2";
	case TCMPB2:
		return "TCMTB2";
	case TCNTO2:
		return "TCNTO2";
	case TINT_CSTAT2:
		return "TINT_CSTAT2";
	case T3CFG0:
		return "T3CFG0";
	case T3CFG1:
		return "T3CFG1";
	case TCON3:
		return "TCON3";
	case TCNTB3:
		return "TCNTB3";
	case TCMPB3:
		return "TCMPB3";
	case TCNTO3:
		return "TCNTO3";
	case TINT_CSTAT3:
		return "TINT_CSTAT3";

	default:
		return "?";
	}

}

/*
 * TIMER update frequency
 */
static void nexell_timer_update_freq(NexellTIMERState *s, uint32_t id)
{
	if (TIMER_LOG)
		qemu_log("[QEMU] nexell_timer_update_freq\n");
	uint32_t freq;
	freq = s->timer[id].freq;
	/* based on OSC */
	s->timer[id].freq = 24000000 / (GET_PRESCALER(s->timer[id].reg_tcfg[0]) * GET_DIVIDER(s->timer[id].reg_tcfg[1]));
	/* based on TCLK */
	s->timer[id].freq = 10000000;

	if (freq != s->timer[id].freq) {
		ptimer_set_freq(s->timer[id].ptimer, s->timer[id].freq);
		DPRINTF("freq=%dHz\n", s->timer[id].freq);
		
		if (TIMER_LOG)
			qemu_log("[QEMU] freq=%dHz\n", s->timer[id].freq);
	}
}

/*
 * Counter tick handler
 */
static void nexell_timer_tick(void *opaque)
{
	NexellTIMER *s = (NexellTIMER *)opaque;
	NexellTIMERState *p = (NexellTIMERState *)s->parent;
	uint32_t id = s->id;
	bool cmp = false;

	if (TIMER_LOG) {
		qemu_log("[QEMU] nexell_timer_tick\n");
		qemu_log("[QEMU] timer %d tick\n", id);
	}

	DPRINTF("timer %d tick\n", id);

	/* set irq status */
	p->timer[id].reg_tint_cstat |= TINT_CSTAT_STATUS;

	/* raise IRQ */
	if (p->timer[id].reg_tint_cstat & TINT_CSTAT_ENABLE) {
		DPRINTF("timer %d IRQ\n", id);
		if (TIMER_LOG)
			qemu_log("[QEMU] timer %d IRQ\n", id);
		p->timer[id].reg_tint_cstat |= (~TINT_CSTAT_ENABLE | TINT_CSTAT_STATUS);
		if (TIMER_LOG)
			qemu_log("qemu_irq_raise\n");
		qemu_irq_raise(p->timer[id].irq);
	}

	/* reload timer */
	cmp = p->timer[id].reg_tcon & TCON_TIMER_AUTO_RELOAD;

	if (cmp) {
		DPRINTF("auto reload timer %d count to %x\n", id,
				p->timer[id].reg_tcntb);
		if (TIMER_LOG)
			qemu_log("[QEMU] auto reload timer %d count to %x\n", id,
				p->timer[id].reg_tcntb);
		ptimer_set_count(p->timer[id].ptimer, p->timer[id].reg_tcntb);
		ptimer_run(p->timer[id].ptimer, 1);
	} else {
		/* stop timer, set status to STOP, see Basic Timer Operation */
		p->timer[id].reg_tcon &= ~TCON_TIMER_START;
		ptimer_stop(p->timer[id].ptimer);
	}
}

/*
 * TIMER Read
 */
static uint64_t nexell_timer_read(void *opaque, hwaddr offset,
        unsigned size)
{
	NexellTIMERState *s = (NexellTIMERState *)opaque;
	uint32_t value = 0;
	int index;

	if (TIMER_LOG)
		qemu_log("[QEMU] nexell_timer_read offset: %s,", nexell_timer_get_regname((unsigned)offset));

	switch (offset) {
	case T0CFG0: case T1CFG0:
	case T2CFG0: case T3CFG0:
		index = (offset - T0CFG0) / 0x100;
		value = s->timer[index].reg_tcfg[0];
		break;

	case T0CFG1: case T1CFG1:
	case T2CFG1: case T3CFG1:
		index = (offset - T0CFG1) / 0x100;
		value = s->timer[index].reg_tcfg[1];
		break;

	case TCON0: case TCON1:
	case TCON2: case TCON3:
		index = (offset - TCON0) / 0x100;
		value = s->timer[index].reg_tcon;
		break;

	case TCNTB0: case TCNTB1:
	case TCNTB2: case TCNTB3:
		index = (offset - TCNTB0) / 0x100;
		value = s->timer[index].reg_tcntb;
		break;

	case TCMPB0: case TCMPB1:
	case TCMPB2: case TCMPB3:
		index = (offset - TCMPB0) / 0x100;
		value = s->timer[index].reg_tcmpb;
		break;

	case TCNTO0: case TCNTO1:
	case TCNTO2: case TCNTO3:
		index = (offset - TCNTO0) / 0x100;
		value = ptimer_get_count(s->timer[index].ptimer);
		break;

	case TINT_CSTAT0: case TINT_CSTAT1:
	case TINT_CSTAT2: case TINT_CSTAT3:
		index = (offset - TINT_CSTAT0) / 0x100;
		value = s->timer[index].reg_tint_cstat;
		break;

	default:
		qemu_log_mask(LOG_GUEST_ERROR,
				"nexell.timer: bad read offset " TARGET_FMT_plx,
				offset);
		break;
	}
	if (TIMER_LOG)
		qemu_log(" value: %#x\n", value);	
	return value;
}

/*
 * TIMER Write
 */
static void nexell_timer_write(void *opaque, hwaddr offset,
        uint64_t value, unsigned size)
{
	NexellTIMERState *s = (NexellTIMERState *)opaque;
	int index;
	uint32_t new_val;
	if (TIMER_LOG)
		qemu_log("[QEMU] nexell_timer_write offset: %s, value: %lu\n", nexell_timer_get_regname((unsigned)offset), value);

	switch (offset) {
	case T0CFG0: case T1CFG0:
	case T2CFG0: case T3CFG0:
		index = (offset - T0CFG0) / 0x100;
		s->timer[index].reg_tcfg[0] = value;
		nexell_timer_update_freq(s, s->timer[index].id);
		break;

	case T0CFG1: case T1CFG1:
	case T2CFG1: case T3CFG1:
		index = (offset - T0CFG0) / 0x100;
		s->timer[index].reg_tcfg[1] = value;
		nexell_timer_update_freq(s, s->timer[index].id);
		break;

	case TCON0: case TCON1:
	case TCON2: case TCON3:
		index = (offset - TCON0) / 0x100;
		if ((value & TCON_TIMER_MANUAL_UPD) >
				(s->timer[index].reg_tcon & TCON_TIMER_MANUAL_UPD)) {
			/*
			 * TCNTB and TCMPB are loaded into TCNT and TCMP.
			 * Update timers.
			 */

			/* this will start timer to run, this ok, because
			 * during processing start bit timer will be stopped
			 * if needed */

			if (TIMER_LOG)
				qemu_log("ptimer_set_count\n");
			ptimer_set_count(s->timer[index].ptimer, s->timer[index].reg_tcntb);
			DPRINTF("set timer %d count to %x\n", index,
					s->timer[index].reg_tcntb);
			if (TIMER_LOG)
				qemu_log("[QEMU] set timer %d count to %x\n", index,
						s->timer[index].reg_tcntb);
		}

		if ((value & TCON_TIMER_START) >
				(s->timer[index].reg_tcon & TCON_TIMER_START)) {
			/* changed to start */
			DPRINTF("run timer %d\n", index);
			if (TIMER_LOG)
				qemu_log("[QEMU] run timer %d\n", index);
			ptimer_run(s->timer[index].ptimer, 1);
		}

		if ((value & TCON_TIMER_START) <
				(s->timer[index].reg_tcon & TCON_TIMER_START)) {
			/* changed to stop */
			DPRINTF("stop timer %d\n", index);
			if (TIMER_LOG)
				qemu_log("[QEMU] stop timer %d\n", index);
			ptimer_stop(s->timer[index].ptimer);
		}
		s->timer[index].reg_tcon = value;
		break;

	case TCNTB0: case TCNTB1:
	case TCNTB2: case TCNTB3:
		index = (offset - TCNTB0) / 0x100;
		s->timer[index].reg_tcntb = value;
		break;

	case TCMPB0: case TCMPB1:
	case TCMPB2: case TCMPB3:
		index = (offset - TCMPB0) / 0x100;
		s->timer[index].reg_tcmpb = value;
		break;

	case TINT_CSTAT0: case TINT_CSTAT1:
	case TINT_CSTAT2: case TINT_CSTAT3:
		index = (offset - TINT_CSTAT0) / 0x100;
		new_val = (s->timer[index].reg_tint_cstat & 0x3E0) + (0x1F & value);
		new_val &= ~(0x3E0 & value);

		if ((new_val & TINT_CSTAT_STATUS) <
				(s->timer[index].reg_tint_cstat & TINT_CSTAT_STATUS)) {
			qemu_irq_lower(s->timer[index].irq);
		}

		s->timer[index].reg_tint_cstat = new_val;
		break;

	default:
		qemu_log_mask(LOG_GUEST_ERROR,
				"nexell.timer: bad write offset " TARGET_FMT_plx,
				offset);
		break;

	}
}

/*
 * Set default values to timer fields and registers
 */
static void nexell_timer_reset(DeviceState *d)
{
	NexellTIMERState *s = NEXELL_TIMER(d);
	int i;

	for (i = 0; i < NEXELL_TIMER_TIMERS_NUM; i++) {
		s->timer[i].reg_tcfg[0] = 0x1;
		s->timer[i].reg_tcfg[1] = 0x0;
		s->timer[i].reg_tcon = 0;
		s->timer[i].reg_tint_cstat = 0;
		s->timer[i].reg_tcmpb = 0;
		s->timer[i].reg_tcntb = 0;

		nexell_timer_update_freq(s, s->timer[i].id);
		ptimer_stop(s->timer[i].ptimer);
	}
}

static const MemoryRegionOps nexell_timer_ops = {
	.read = nexell_timer_read,
	.write = nexell_timer_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

/*
 * TIMER timer initialization
 */
static void nexell_timer_init(Object *obj)
{
	NexellTIMERState *s = NEXELL_TIMER(obj);
	SysBusDevice *dev = SYS_BUS_DEVICE(obj);
	int i;
	QEMUBH *bh;
	
	if (TIMER_LOG)
		qemu_log("[QEMU] nexell_timer_init\n");

	for (i = 0; i < NEXELL_TIMER_TIMERS_NUM; i++) {
		bh = qemu_bh_new(nexell_timer_tick, &s->timer[i]);
		s->timer[i].ptimer = ptimer_init(bh, PTIMER_POLICY_DEFAULT);
		s->timer[i].id = i;
		s->timer[i].parent = s;
	}

	memory_region_init_io(&s->iomem, obj, &nexell_timer_ops, s,
			"nexell-timer", NEXELL_TIMER_REG_MEM_SIZE);
	sysbus_init_mmio(dev, &s->iomem);

	
}

static void nexell_timer_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->reset = nexell_timer_reset;
	dc->vmsd = &vmstate_nexell_timer_state;
}

static const TypeInfo nexell_timer_info = {
	.name          = TYPE_NEXELL_TIMER,
	.parent        = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(NexellTIMERState),
	.instance_init = nexell_timer_init,
	.class_init    = nexell_timer_class_init,
};

static void nexell_timer_register_types(void)
{
	type_register_static(&nexell_timer_info);
}

type_init(nexell_timer_register_types)
