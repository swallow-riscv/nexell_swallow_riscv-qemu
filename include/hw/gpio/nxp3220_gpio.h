/*
 * Nexell NXP3220, Drone(Swallow) processors GPIO registers definition.
 *
 * Copyright (C) 2018 Sung-Woo Park <swpark@nexell.co.kr>
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
#ifndef NEXELL_GPIO_H
#define NEXELL_GPIO_H

#include "hw/sysbus.h"

#define TYPE_NXP3220_GPIO "nxp3220.gpio"
#define NXP3220_GPIO(obj) OBJECT_CHECK(NXP3220GpioState, (obj), TYPE_NXP3220_GPIO)

#define NXP3220_GPIO_MEM_SIZE	0x80

/* NXP3220, Swallow GPIO Controller memory mamp */
#define OUT_ADDR	0x00 /* out value register */
#define OUTENB_ADDR	0x04 /* output enable register */
#define DETMODE0_ADDR	0x08 /* detect mode register 0 */
#define DETMODE1_ADDR	0x0c /* detect mode register 1 */
#define INTENB_ADDR	0x10 /* interrupt enable register */
#define DET_ADDR	0x14 /* event detect register */
#define PAD_ADDR	0x18 /* pad status(input value) register */
#define ALTFN0_ADDR	0x20 /* alternate function select register 0 */
#define ALTFN1_ADDR	0x24 /* alternate function select register 1 */
#define DETMODEEX_ADDR	0x28 /* event detect mode extended register */
#define DETENB_ADDR	0x3c /* interrupt pending detect enable register */
#define SLEW_ADDR	0x40 /* slew register */
#define SLEWSET_ADDR	0x44 /* slew set of/off register */
#define DRV1_ADDR	0x48 /* drive strength LSB register */
#define DRV1SET_ADDR	0x4c /* drive strength LSB set on/off register */
#define DRV0_ADDR	0x50 /* drive strength MSB register */
#define DRV0SET_ADDR	0x54 /* drive strength MSB set on/off register */
#define PULLSEL_ADDR	0x58 /* pull up/down selection register */
#define PULLSET_ADDR	0x5c /* pull up/down selection on/off register */
#define PULLENB_ADDR	0x60 /* pull enable/disable register */
#define PULLENBSET_ADDR	0x64 /* pull enable/disable selection on/off register */
#define DIR_ADDR	0x74 /* direction selection register */
#define DIRSET_ADDR	0x78 /* direction selection on/off register */
#define ALTFNEX_ADDR	0x7c /* alternate function select extension */

#define NXP3220_GPIO_PIN_COUNT	32

typedef struct NXP3220GpioState {
	/* private */
	SysBusDevice parent_obj;

	/* public */
	MemoryRegion iomem;

	uint32_t out;
	uint32_t outenb;
	uint32_t detmode0;
	uint32_t detmode1;
	uint32_t intenb;
	uint32_t det;
	uint32_t pad;
	uint32_t altfn0;
	uint32_t altfn1;
	uint32_t detmodeex;
	uint32_t detenb;
	uint32_t slew;
	uint32_t slew_disable_default;
	uint32_t drv1;
	uint32_t drv1_disable_default;
	uint32_t drv0;
	uint32_t drv0_disable_default;
	uint32_t pullsel;
	uint32_t pullsel_disable_default;
	uint32_t pullenb;
	uint32_t pullenb_disable_default;
	uint32_t inenb;
	uint32_t inenb_disable_default;
	uint32_t altfnex;

	qemu_irq irq;
	qemu_irq output[NXP3220_GPIO_PIN_COUNT];
} NXP3220GpioState;


#endif
