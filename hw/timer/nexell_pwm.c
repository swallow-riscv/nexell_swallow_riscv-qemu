/*
 * Nexell Swallow processors PWM emulation.
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
#include "hw/timer/nexell_pwm.h"

#ifdef DEBUG_PWM
#define DPRINTF(fmt, ...) \
        do { fprintf(stdout, "PWM: [%24s:%5d] " fmt, __func__, __LINE__, \
                ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while (0)
#endif

/*** VMState ***/
static const VMStateDescription vmstate_nexell_pwm = {
	.name = "nexell.pwm.pwm",
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_UINT32(id, NexellPWM),
		VMSTATE_UINT32(freq, NexellPWM),
		VMSTATE_PTIMER(ptimer, NexellPWM),
		VMSTATE_UINT32_ARRAY(reg_tcfg, NexellPWM, 2),
		VMSTATE_UINT32(reg_tcon, NexellPWM),
		VMSTATE_UINT32(reg_tint_cstat, NexellPWM),
		VMSTATE_UINT32(reg_tcntb, NexellPWM),
		VMSTATE_UINT32(reg_tcmpb, NexellPWM),
		VMSTATE_END_OF_LIST()
	}
};

static const VMStateDescription vmstate_nexell_pwm_state = {
	.name = "nexell.pwm",
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_STRUCT_ARRAY(timer, NexellPWMState,
				NEXELL_PWM_TIMERS_NUM, 0,
				vmstate_nexell_pwm, NexellPWM),
		VMSTATE_END_OF_LIST()
	}
};

/*
 * PWM update frequency
 */
static void nexell_pwm_update_freq(NexellPWMState *s, uint32_t id)
{
	uint32_t freq;
	freq = s->timer[id].freq;
	s->timer[id].freq = 24000000 / (GET_PRESCALER(s->timer[id].reg_tcfg[0]) * GET_DIVIDER(s->timer[id].reg_tcfg[1]));

	if (freq != s->timer[id].freq) {
		ptimer_set_freq(s->timer[id].ptimer, s->timer[id].freq);
		DPRINTF("freq=%dHz\n", s->timer[id].freq);
	}
}

/*
 * Counter tick handler
 */
static void nexell_pwm_tick(void *opaque)
{
	NexellPWM *s = (NexellPWM *)opaque;
	NexellPWMState *p = (NexellPWMState *)s->parent;
	uint32_t id = s->id;
	bool cmp = false;

	DPRINTF("timer %d tick\n", id);

	/* set irq status */
	p->timer[id].reg_tint_cstat |= TINT_CSTAT_STATUS;

	/* raise IRQ */
	if (p->timer[id].reg_tint_cstat & TINT_CSTAT_ENABLE) {
		DPRINTF("timer %d IRQ\n", id);
		p->timer[id].reg_tint_cstat |= (~TINT_CSTAT_ENABLE | TINT_CSTAT_STATUS);
	}

	/* reload timer */
	cmp = p->timer[id].reg_tcon & TCON_TIMER_AUTO_RELOAD;

	if (cmp) {
		DPRINTF("auto reload timer %d count to %x\n", id,
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
 * PWM Read
 */
static uint64_t nexell_pwm_read(void *opaque, hwaddr offset,
        unsigned size)
{
	NexellPWMState *s = (NexellPWMState *)opaque;
	uint32_t value = 0;
	int index;

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
				"nexell.pwm: bad read offset " TARGET_FMT_plx,
				offset);
		break;
	}
	return value;
}

/*
 * PWM Write
 */
static void nexell_pwm_write(void *opaque, hwaddr offset,
        uint64_t value, unsigned size)
{
	NexellPWMState *s = (NexellPWMState *)opaque;
	int index;
	uint32_t new_val;

	switch (offset) {
	case T0CFG0: case T1CFG0:
	case T2CFG0: case T3CFG0:
		index = (offset - T0CFG0) / 0x100;
		s->timer[index].reg_tcfg[0] = value;
		nexell_pwm_update_freq(s, s->timer[index].id);
		break;

	case T0CFG1: case T1CFG1:
	case T2CFG1: case T3CFG1:
		index = (offset - T0CFG0) / 0x100;
		s->timer[index].reg_tcfg[1] = value;
		nexell_pwm_update_freq(s, s->timer[index].id);
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
			ptimer_set_count(s->timer[index].ptimer, s->timer[index].reg_tcntb);
			DPRINTF("set timer %d count to %x\n", i,
					s->timer[index].reg_tcntb);
		}

		if ((value & TCON_TIMER_START) >
				(s->timer[index].reg_tcon & TCON_TIMER_START)) {
			/* changed to start */
			ptimer_run(s->timer[index].ptimer, 1);
			DPRINTF("run timer %d\n", index);
		}

		if ((value & TCON_TIMER_START) <
				(s->timer[index].reg_tcon & TCON_TIMER_START)) {
			/* changed to stop */
			ptimer_stop(s->timer[index].ptimer);
			DPRINTF("stop timer %d\n", index);
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
				"nexell.pwm: bad write offset " TARGET_FMT_plx,
				offset);
		break;

	}
}

/*
 * Set default values to timer fields and registers
 */
static void nexell_pwm_reset(DeviceState *d)
{
	NexellPWMState *s = NEXELL_PWM(d);
	int i;

	for (i = 0; i < NEXELL_PWM_TIMERS_NUM; i++) {
		s->timer[i].reg_tcfg[0] = 0x1;
		s->timer[i].reg_tcfg[1] = 0x0;
		s->timer[i].reg_tcon = 0;
		s->timer[i].reg_tint_cstat = 0;
		s->timer[i].reg_tcmpb = 0;
		s->timer[i].reg_tcntb = 0;

		nexell_pwm_update_freq(s, s->timer[i].id);
		ptimer_stop(s->timer[i].ptimer);
	}
}

static const MemoryRegionOps nexell_pwm_ops = {
	.read = nexell_pwm_read,
	.write = nexell_pwm_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

/*
 * PWM timer initialization
 */
static void nexell_pwm_init(Object *obj)
{
	NexellPWMState *s = NEXELL_PWM(obj);
	SysBusDevice *dev = SYS_BUS_DEVICE(obj);
	int i;
	QEMUBH *bh;

	for (i = 0; i < NEXELL_PWM_TIMERS_NUM; i++) {
		bh = qemu_bh_new(nexell_pwm_tick, &s->timer[i]);
		s->timer[i].ptimer = ptimer_init(bh, PTIMER_POLICY_DEFAULT);
		s->timer[i].id = i;
		s->timer[i].parent = s;
	}

	memory_region_init_io(&s->iomem, obj, &nexell_pwm_ops, s,
			"nexell-pwm", NEXELL_PWM_REG_MEM_SIZE);
	sysbus_init_mmio(dev, &s->iomem);
}

static void nexell_pwm_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->reset = nexell_pwm_reset;
	dc->vmsd = &vmstate_nexell_pwm_state;
}

static const TypeInfo nexell_pwm_info = {
	.name          = TYPE_NEXELL_PWM,
	.parent        = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(NexellPWMState),
	.instance_init = nexell_pwm_init,
	.class_init    = nexell_pwm_class_init,
};

static void nexell_pwm_register_types(void)
{
	type_register_static(&nexell_pwm_info);
}

type_init(nexell_pwm_register_types)
