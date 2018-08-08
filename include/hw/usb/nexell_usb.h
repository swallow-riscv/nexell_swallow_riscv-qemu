/*
 * Nexell USB interface
 *
 * Author: Hyejung, Kwon <cjscld15@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_NEXELL_USB_H
#define HW_NEXELL_USB_H

#include "hw/usb/nx_usb_hw.h"

#define TYPE_NEXELL_USB "usb.nexell.usb"

#define NEXELL_USB(obj) \
    OBJECT_CHECK(NexellUsbState, (obj), TYPE_NEXELL_USB)

#define USB_NUM_OF_REGISTERS ARRAY_SIZE(nx_usb_regs)

typedef struct {
	/*< private >*/
	SysBusDevice parent_obj;
	/*< public >*/
	MemoryRegion mmio;
	hwaddr base_addr;
	qemu_irq irq;
	uint32_t regs[USB_NUM_OF_REGISTERS];
} NexellUsbState;

NexellUsbState *nexell_usb_create(MemoryRegion *address_space, hwaddr base,
		int size, qemu_irq irq);

#endif
