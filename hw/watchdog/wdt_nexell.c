/*
 * NEXELL Watchdog Controller
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

#include "qapi/error.h"
#include "qemu/log.h"
#include "qemu/timer.h"
#include "sysemu/watchdog.h"
#include "hw/sysbus.h"
#include "hw/watchdog/wdt_nexell.h"
#include <stdint.h>

#define NEXELL_DEBUG_WDT 0

#define WDT_CON				0x00
#define WDT_DAT		                0x04
#define WDT_CNT				0x08
#define WDT_CLRINT                      0x0C

#define WDT_ENABLE			(1 << 5)
#define WDT_RESET_ENABLE		(1 << 0)
#define WDT_INT_ENABLE			(1 << 2)

#define CHECK_PRESCALER_VAL(x)		((x >> 8) & 0xff)
#define CHECK_DIVIDER_VAL(x)		((x >> 3) & (BIT(1) | BIT(0)))

#define PCLK_HZ 100000000 /* 100 Mhz */

static bool nexell_wdt_is_enabled(const NexellWDTState *s)
{
	return s->wdt_con & WDT_ENABLE;
}

static bool nexell_wdt_int_enabled(const NexellWDTState *s)
{
	return s->wdt_con & WDT_INT_ENABLE;
}

static void nexell_wdt_timer_expired(void *dev)
{
	NexellWDTState *s = NEXELL_WDT(dev);

    	qemu_log_mask(CPU_LOG_RESET, "Watchdog timer expired!\n");
	if(NEXELL_DEBUG_WDT)
		qemu_log("Watchdog timer expired!\n");

	timer_del(s->timer);

	qemu_irq_raise(s->irq);

	watchdog_perform_action();
}

static uint64_t nexell_wdt_read(void *opaque, hwaddr offset, unsigned size)
{
	NexellWDTState *s = NEXELL_WDT(opaque);
	uint32_t value;

	if(NEXELL_DEBUG_WDT)
		qemu_log("[WDT QEMU] read addr: %#x", (uint32_t)offset);

	switch (offset) {
	case WDT_CON:
		value = s->wdt_con;
		break;
	case WDT_DAT:
		value = s->wdt_dat;
		break;
	case WDT_CNT:
		value = s->wdt_cnt;
		break;
	case WDT_CLRINT:
		value = s->wdt_clrint;
		break;

	default:
		qemu_log_mask(LOG_GUEST_ERROR,
				"%s: Out-of-bounds read at offset 0x%" HWADDR_PRIx "\n",
				__func__, offset);
		value = 0;
		break;
	}

	if(NEXELL_DEBUG_WDT)
		qemu_log(", val: %#x\n", value);
	return value;

}

static void nexell_wdt_write(void *opaque, hwaddr offset, uint64_t data,
                             unsigned size)
{
	NexellWDTState *s = NEXELL_WDT(opaque);
	uint32_t value = (uint32_t) data;
	int prescaler = 1, divider = 1;
	uint64_t timeout;

	if(NEXELL_DEBUG_WDT)
		qemu_log("[WDT QEMU] write addr: %#x, val: %#x\n", (uint32_t)offset, (uint32_t)value);

	switch (offset) {
	case WDT_CON:

	if(NEXELL_DEBUG_WDT)
		qemu_log("[WDT QEMU] prescaler val: %d, divider val: %ld\n", CHECK_PRESCALER_VAL(value), CHECK_DIVIDER_VAL(value));

	/* prescaler value */
	if(CHECK_PRESCALER_VAL(value) < 1)
		prescaler = 1;
	else
		prescaler = CHECK_PRESCALER_VAL(value);

	/* divider value */
	switch(CHECK_DIVIDER_VAL(value)) {
	case 0:
		divider = 16;
		break;
	case 1:
		divider = 32;
		break;
	case 2:
		divider = 64;
		break;
	case 3:
		divider = 128;
		break;
	}
	s->pclk_freq = PCLK_HZ / prescaler / divider;
        s->wdt_con = value;
	timeout = muldiv64(s->wdt_cnt, NANOSECONDS_PER_SECOND, s->pclk_freq);

	if(NEXELL_DEBUG_WDT) {
		qemu_log("[WDT QEMU] wdt enable: %d\n", nexell_wdt_is_enabled(s));
		qemu_log("[WDT QEMU] s->wdt_cnt: %#x\n", s->wdt_cnt);
		qemu_log("[WDT QEMU] s->pclk_freq: %"PRIu64"\n", s->pclk_freq);
		qemu_log("[WDT QEMU] timeout: %ld\n", timeout);
	}

	/* stop */
        if(!(value & WDT_ENABLE) && !(value & WDT_RESET_ENABLE)) {
		if(NEXELL_DEBUG_WDT)
			qemu_log("[WDT QEMU] stop WDT\n");

        	if(s->timer) {
			if(NEXELL_DEBUG_WDT)
				qemu_log("[WDT QEMU] timer_del\n");
			timer_del(s->timer);
		}
        	return;
	}

	/* start */
	if(nexell_wdt_is_enabled(s)) {

		if(NEXELL_DEBUG_WDT)
			qemu_log("[WDT QEMU] start WDT\n");

		/* reset */
		s->wdt_con = 0x8001;
		s->wdt_dat = 0x8000;
		s->wdt_cnt = 0x8000;
		s->wdt_clrint = 0;

		s->pclk_freq = PCLK_HZ;

		/* start reboot */
		timer_mod(s->timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + timeout);
	}
        break;
    case WDT_DAT:
        s->wdt_dat = value;
        break;
    case WDT_CNT:
	s->wdt_cnt = value;
	break;
    case WDT_CLRINT:
	s->wdt_clrint = value;
	break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Out-of-bounds write at offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
        break;
    }
    return;
}

static WatchdogTimerModel model = {
    .wdt_name = TYPE_NEXELL_WDT,
    .wdt_description = "Nexell watchdog device",
};

static const VMStateDescription vmstate_nexell_wdt = {
    .name = "vmstate_nexell_wdt",
    .version_id = 0,
    .minimum_version_id = 0,
    .fields = (VMStateField[]) {
        VMSTATE_TIMER_PTR(timer, NexellWDTState),
        VMSTATE_END_OF_LIST()
    }
};

static const MemoryRegionOps nexell_wdt_ops = {
    .read = nexell_wdt_read,
    .write = nexell_wdt_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid.min_access_size = 4,
    .valid.max_access_size = 4,
    .valid.unaligned = false,
};

static void nexell_wdt_reset(DeviceState *dev)
{
	NexellWDTState *s = NEXELL_WDT(dev);
	if(NEXELL_DEBUG_WDT)
		qemu_log("[WDT QEMU] nexell_wdt_reset\n");

	s->wdt_con = 0x8001;
	s->wdt_dat = 0x8000;
	s->wdt_cnt = 0x8000;
	s->wdt_clrint = 0;

	s->pclk_freq = PCLK_HZ;

	if(s->timer)
		timer_del(s->timer);
}


static void nexell_wdt_realize(DeviceState *dev, Error **errp)
{
	SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
	NexellWDTState *s = NEXELL_WDT(dev);

	if(NEXELL_DEBUG_WDT)
		qemu_log("[WDT QEMU] nexell_wdt_realize\n");

	/* register reset */
	s->wdt_con = 0x8001;
	s->wdt_dat = 0x8000;
	s->wdt_cnt = 0x8000;
	s->wdt_clrint = 0;

	/* create wdt timer */
	s->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, nexell_wdt_timer_expired, dev);

	s->pclk_freq = PCLK_HZ;

	memory_region_init_io(&s->iomem, OBJECT(s), &nexell_wdt_ops, s,
			TYPE_NEXELL_WDT, NEXELL_WDT_MEM_SIZE);
	sysbus_init_mmio(sbd, &s->iomem);
}

static void nexell_wdt_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->realize = nexell_wdt_realize;
	dc->reset = nexell_wdt_reset;
	set_bit(DEVICE_CATEGORY_MISC, dc->categories);
	dc->vmsd = &vmstate_nexell_wdt;
}

static const TypeInfo nexell_wdt_info = {
	.parent = TYPE_SYS_BUS_DEVICE,
	.name  = TYPE_NEXELL_WDT,
	.instance_size  = sizeof(NexellWDTState),
	.class_init = nexell_wdt_class_init,
};

static void wdt_nexell_register_types(void)
{
	watchdog_add_model(&model);
	type_register_static(&nexell_wdt_info);
}

type_init(wdt_nexell_register_types)
