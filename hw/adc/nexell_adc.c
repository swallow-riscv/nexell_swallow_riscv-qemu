/*
 * Nexell Swallow processors ADC emulation.
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
#include "hw/sysbus.h"
#include "hw/hw.h"
#include "qemu/log.h"
#include "hw/adc/nexell_adc.h"
#include "stdlib.h"
#include "time.h"

#ifndef STM_ADC_ERR_DEBUG
#define STM_ADC_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (STM_ADC_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0)

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void update_irq(NEXELLADCState *s)
{
	qemu_irq_raise(s->irq);
}

static void nexell_adc_reset(DeviceState *dev)
{
	NEXELLADCState *s = NEXELL_ADC(dev);

	s->adc_con = 0x00000004;
	s->adc_dat = 0x00000000;
	s->adc_intenb = 0x00000000;
	s->adc_intclr = 0x00000000;
	s->adc_prescon = 0x00000000;
	s->adc_en = 0x00000001;
}
#if 1
static uint32_t nexell_adc_generate_value(NEXELLADCState *s)
{
	/* 10 bit data */
	return s->adc_dat &= 0x3FF;
}
#endif

static uint64_t nexell_adc_read(void *opaque, hwaddr addr,
                                     unsigned int size)
{
	NEXELLADCState *s = opaque;
	uint32_t value;

	DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);

	if (addr >= ADC_COMMON_ADDRESS) {
		qemu_log_mask(LOG_UNIMP,
				"%s: ADC Common Register Unsupported\n", __func__);
    }
    switch (addr) {
    case ADC_CON:
		value = s->adc_con;
		break;
    case ADC_DAT:
		if(!(s->adc_con & ADC_CON_ADON) && (s->adc_con & ADC_CON_SWSTART))
		{
			s->adc_con ^= ADC_CON_SWSTART;

			return nexell_adc_generate_value(s);
		} else {
			return 0;
		}
		break;
	case ADC_INTENB:
		value = s->adc_intenb;
		break;
	case ADC_INTCLR:
		value = s->adc_intclr;
		break;
    case ADC_PRESCON:
		value = s->adc_prescon;
		break;
	case ADC_EN:
		value = s->adc_en;
		break;
	default:
		qemu_log_mask(LOG_GUEST_ERROR,
				"%s: Bad offset 0x%" HWADDR_PRIx "\n", __func__, addr);
		value = 0;
		break;
    }
    return (uint64_t)value;
}

static void nexell_adc_write(void *opaque, hwaddr addr,
                       uint64_t val64, unsigned int size)
{
    NEXELLADCState *s = opaque;
    uint32_t value = (uint32_t) val64;

    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n",
             addr, value);

    if (addr >= 0x100) {
        qemu_log_mask(LOG_UNIMP,
                      "%s: ADC Common Register Unsupported\n", __func__);
    }

    switch (addr) {
    case ADC_CON:
		s->adc_con = value;
		if (value & ADC_CON_SWSTART) {
			srand(time(NULL));

			/* AD conversion emulation */
			uint32_t adc_in = rand()%5;
			s->adc_dat = adc_in * 1024 / 1.8;
			s->adc_dat &= 0x3ff;

			/* interrupt pended */
			s->adc_intclr |= ADC_INT_CLR;

			if(s->adc_intenb & ADC_INT_ENB)
				update_irq(s);
		}
		break;
	case ADC_DAT:
		s->adc_dat = value;
		break;
	case ADC_INTENB:
		s->adc_intenb = value;
		break;
	case ADC_INTCLR:
		if (value & ADC_INT_CLR)
			/* pending clear */
			s->adc_intclr = value | ~ADC_INT_CLR;
		break;
	case ADC_PRESCON:
		s->adc_prescon = value;
		break;
	case ADC_EN:
		s->adc_en = value;
		break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%" HWADDR_PRIx "\n", __func__, addr);
		break;
    }
}

static const MemoryRegionOps nexell_adc_ops = {
    .read = nexell_adc_read,
    .write = nexell_adc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_nexell_adc = {
    .name = TYPE_NEXELL_ADC,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(adc_con, NEXELLADCState),
        VMSTATE_UINT32(adc_dat, NEXELLADCState),
        VMSTATE_UINT32(adc_intenb, NEXELLADCState),
        VMSTATE_UINT32(adc_intclr, NEXELLADCState),
        VMSTATE_UINT32(adc_prescon, NEXELLADCState),
        VMSTATE_UINT32(adc_en, NEXELLADCState),
        VMSTATE_END_OF_LIST()
    }
};

static void nexell_adc_init(Object *obj)
{
    NEXELLADCState *s = NEXELL_ADC(obj);

    memory_region_init_io(&s->mmio, obj, &nexell_adc_ops, s,
                          TYPE_NEXELL_ADC, 0xFF);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static void nexell_adc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = nexell_adc_reset;
    dc->vmsd = &vmstate_nexell_adc;
}

static const TypeInfo nexell_adc_info = {
    .name          = TYPE_NEXELL_ADC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NEXELLADCState),
    .instance_init = nexell_adc_init,
    .class_init    = nexell_adc_class_init,
};

static void nexell_adc_register_types(void)
{
    type_register_static(&nexell_adc_info);
}

type_init(nexell_adc_register_types)
