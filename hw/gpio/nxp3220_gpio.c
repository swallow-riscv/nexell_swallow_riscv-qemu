/*
 * Nexell NXP3220, Swallow processors GPIO emulation.
 *
 * Copyright (C) 2018 Sung-woo Park <swpark@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/gpio/nxp3220_gpio.h"
#include "qemu/log.h"

#ifndef DEBUG_NEXELL_GPIO
#define DEBUG_NEXELL_GPIO 0
#endif

typedef enum NexellGPIOLevel {
	NEXELL_GPIO_LEVEL_LOW = 0,
	NEXELL_GPIO_LEVEL_HIGH = 1,
} NexellGPIOLevel;

#define DPRINTF(fmt, args...) \
	do { \
		if (DEBUG_NEXELL_GPIO) { \
			fprintf(stderr, "[%s]%s: " fmt, TYPE_NXP3220_GPIO, \
				__func__, ##args); \
		} \
	} while (0)

static uint64_t nxp3220_gpio_read(void *opaque, hwaddr offset, unsigned size)
{
	NXP3220GpioState *s = NXP3220_GPIO(opaque);
	uint32_t reg_value = 0;

	switch (offset) {
	case OUT_ADDR:
		reg_value = s->out;
		break;
	case OUTENB_ADDR:
		reg_value = s->outenb;
		break;
	case DETMODE0_ADDR:
		reg_value = s->detmode0;
		break;
	case DETMODE1_ADDR:
		reg_value = s->detmode1;
		break;
	case INTENB_ADDR:
		reg_value = s->intenb;
		break;
	case DET_ADDR:
		reg_value = s->det;
		break;
	case PAD_ADDR:
		reg_value = s->pad;
		break;
	case ALTFN0_ADDR:
		reg_value = s->altfn0;
		break;
	case ALTFN1_ADDR:
		reg_value = s->altfn1;
		break;
	case DETMODEEX_ADDR:
		reg_value = s->detmodeex;
		break;
	case DETENB_ADDR:
		reg_value = s->detenb;
		break;
	case SLEW_ADDR:
		reg_value = s->slew;
		break;
	case SLEWSET_ADDR:
		reg_value = s->slew_disable_default;
		break;
	case DRV1_ADDR:
		reg_value = s->drv1;
		break;
	case DRV1SET_ADDR:
		reg_value = s->drv1_disable_default;
		break;
	case DRV0_ADDR:
		reg_value = s->drv0;
		break;
	case DRV0SET_ADDR:
		reg_value = s->drv0_disable_default;
		break;
	case PULLSEL_ADDR:
		reg_value = s->pullsel;
		break;
	case PULLSET_ADDR:
		reg_value = s->pullsel_disable_default;
		break;
	case PULLENB_ADDR:
		reg_value = s->pullenb;
		break;
	case PULLENBSET_ADDR:
		reg_value = s->pullenb_disable_default;
		break;
	case DIR_ADDR:
		reg_value = s->inenb;
		break;
	case DIRSET_ADDR:
		reg_value = s->inenb_disable_default;
		break;
	case ALTFNEX_ADDR:
		reg_value = s->altfnex;
		break;
	default:
		qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
			      HWADDR_PRIx "\n", TYPE_NXP3220_GPIO, __func__, offset);
	}

	return reg_value;
}

static void nxp3220_gpio_write(void *opaque, hwaddr offset, uint64_t value,
			       unsigned size)
{
	NXP3220GpioState *s = NXP3220_GPIO(opaque);

	switch (offset) {
	case OUT_ADDR:
		s->out = value;
		s->pad = s->out & s->outenb;
		break;
	case OUTENB_ADDR:
		s->outenb = value;
		s->pad = s->out & s->outenb;
		break;
	case DETMODE0_ADDR:
		s->detmode0 = value;
		break;
	case DETMODE1_ADDR:
		s->detmode1 = value;
		break;
	case INTENB_ADDR:
		s->intenb = value;
		break;
	case DET_ADDR:
		s->det = value;
		break;
	case PAD_ADDR:
		break;
	case ALTFN0_ADDR:
		s->altfn0 = value;
		break;
	case ALTFN1_ADDR:
		s->altfn1 = value;
		break;
	case DETMODEEX_ADDR:
		s->detmodeex = value;
		break;
	case DETENB_ADDR:
		s->detenb = value;
		break;
	case SLEW_ADDR:
		s->slew = value;
		break;
	case SLEWSET_ADDR:
		s->slew_disable_default = value;
		break;
	case DRV1_ADDR:
		s->drv1 = value;
		break;
	case DRV1SET_ADDR:
		s->drv1_disable_default = value;
		break;
	case DRV0_ADDR:
		s->drv0 = value;
		break;
	case DRV0SET_ADDR:
		s->drv0_disable_default = value;
		break;
	case PULLSEL_ADDR:
		s->pullsel = value;
		break;
	case PULLSET_ADDR:
		s->pullsel_disable_default = value;
		break;
	case PULLENB_ADDR:
		s->pullenb = value;
		break;
	case PULLENBSET_ADDR:
		s->pullenb_disable_default = value;
		break;
	case DIR_ADDR:
		s->inenb = value;
		break;
	case DIRSET_ADDR:
		s->inenb_disable_default = value;
		break;
	case ALTFNEX_ADDR:
		s->altfnex = value;
		break;
	default:
		qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad register at offset 0x%"
			      HWADDR_PRIx "\n", TYPE_NXP3220_GPIO, __func__, offset);
	}
}

static const MemoryRegionOps nxp3220_gpio_ops = {
	.read = nxp3220_gpio_read,
	.write = nxp3220_gpio_write,
	.valid.min_access_size = 4,
	.valid.max_access_size = 4,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static void nxp3220_gpio_set(void *opaque, int line, int level)
{
}

static void nxp3220_gpio_realize(DeviceState *dev, Error **errp)
{
	NXP3220GpioState *s = NXP3220_GPIO(dev);

	memory_region_init_io(&s->iomem, OBJECT(s), &nxp3220_gpio_ops, s,
			      TYPE_NXP3220_GPIO, NXP3220_GPIO_MEM_SIZE);

	qdev_init_gpio_in(DEVICE(s), nxp3220_gpio_set, NXP3220_GPIO_PIN_COUNT);
	qdev_init_gpio_out(DEVICE(s), s->output, NXP3220_GPIO_PIN_COUNT);

	sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq);
	sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nxp3220_gpio_reset(DeviceState *dev)
{
	NXP3220GpioState *s = NXP3220_GPIO(dev);

	s->out = 0;
	s->outenb = 0;
	s->detmode0 = 0;
	s->detmode1 = 0;
	s->intenb = 0;
	s->det = 0;
	s->pad = 0;
	s->altfn0 = 0;
	s->altfn1 = 0;
	s->detmodeex = 0;
	s->detenb = 0;
	s->slew = 0xffffffff;
	s->slew_disable_default = 0xffffffff;
	s->drv1 = 0;
	s->drv1_disable_default = 0xffffffff;
	s->drv0 = 0;
	s->drv0_disable_default = 0xffffffff;
	s->pullsel = 0;
	s->pullsel_disable_default = 0;
	s->pullenb = 0;
	s->pullenb_disable_default = 0;
	s->inenb = 0;
	s->inenb_disable_default = 0;
	s->altfnex = 0;
}

static const VMStateDescription vmstate_nxp3220_gpio = {
	.name = TYPE_NXP3220_GPIO,
	.version_id = 1,
	.minimum_version_id = 1,
	.minimum_version_id_old = 1,
	.fields = (VMStateField[]) {
		VMSTATE_UINT32(out, NXP3220GpioState),
		VMSTATE_UINT32(outenb, NXP3220GpioState),
		VMSTATE_UINT32(detmode0, NXP3220GpioState),
		VMSTATE_UINT32(detmode1, NXP3220GpioState),
		VMSTATE_UINT32(intenb, NXP3220GpioState),
		VMSTATE_UINT32(det, NXP3220GpioState),
		VMSTATE_UINT32(pad, NXP3220GpioState),
		VMSTATE_UINT32(altfn0, NXP3220GpioState),
		VMSTATE_UINT32(altfn1, NXP3220GpioState),
		VMSTATE_UINT32(detmodeex, NXP3220GpioState),
		VMSTATE_UINT32(detenb, NXP3220GpioState),
		VMSTATE_UINT32(slew, NXP3220GpioState),
		VMSTATE_UINT32(slew_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(drv1, NXP3220GpioState),
		VMSTATE_UINT32(drv1_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(drv0, NXP3220GpioState),
		VMSTATE_UINT32(drv0_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(pullsel, NXP3220GpioState),
		VMSTATE_UINT32(pullsel_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(pullenb, NXP3220GpioState),
		VMSTATE_UINT32(pullenb_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(inenb, NXP3220GpioState),
		VMSTATE_UINT32(inenb_disable_default, NXP3220GpioState),
		VMSTATE_UINT32(altfnex, NXP3220GpioState),
		VMSTATE_END_OF_LIST()
	}
};

static void nxp3220_gpio_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->realize = nxp3220_gpio_realize;
	dc->reset = nxp3220_gpio_reset;
	dc->vmsd = &vmstate_nxp3220_gpio;
	dc->desc = "Nexell NXP3220 GPIO controller";
}

static const TypeInfo nxp3220_gpio_info = {
	.name = TYPE_NXP3220_GPIO,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(NXP3220GpioState),
	.class_init = nxp3220_gpio_class_init,
};

static void nxp3220_gpio_register_types(void)
{
	type_register_static(&nxp3220_gpio_info);
}

type_init(nxp3220_gpio_register_types)
