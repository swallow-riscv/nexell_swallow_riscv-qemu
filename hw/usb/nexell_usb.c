/*
 * qemu model of the USB of the nexell
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

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qemu/timer.h"
#include "qapi/error.h"
#include "hw/sysbus.h"
#include "chardev/char.h"
#include "chardev/char-fe.h"
#include "sysemu/dma.h"
#include "target/riscv/cpu.h"
#include "exec/log.h"
#include "hw/ppc/spapr.h"
#include "hw/usb/nexell_usb.h"

#define NEXELL_DEBUG_USB 0

static void set_reg_value(NexellUsbState *s, hwaddr offset,
		uint64_t val)
{
	const NexellUsbRegs *reg_p = nx_usb_regs;
	int i;

	for (i = 0; i < USB_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			s->regs[i] = val;
		}
		reg_p++;
	}
}

#if 0
static uint64_t get_reg_value(NexellUsbState *s, hwaddr offset)
{
	const NexellUsbRegs *reg_p = nx_usb_regs;
	int i;

	for (i = 0; i < USB_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			return s->regs[i];
		}
		reg_p++;
	}

	return 0;
}
#endif

static void init_reg_values(NexellUsbState *s)
{
	if (NEXELL_DEBUG_USB)
		qemu_log("[%s]\n", __func__);

	set_reg_value(s, GOTGCTL, 0x000d0000);
	set_reg_value(s, GOTGINT, 0x00000000);
	set_reg_value(s, GAHBCFG, 0x00000000);
	set_reg_value(s, GUSBCFG, 0x00001400);
	set_reg_value(s, GRSTCTL, 0x80000000);
	set_reg_value(s, GINTSTS, 0x54000020);
	set_reg_value(s, GINTMSK, 0x00000000);
	set_reg_value(s, GRXSTSR, 0x00000000);
	set_reg_value(s, GRXSTSP, 0x00000000);
	set_reg_value(s, GRXFSIZ, 0x00000400);
	set_reg_value(s, GNPTXFSIZ, 0x00400400);
	set_reg_value(s, GNPTXSTS, 0x00080040);
	set_reg_value(s, GI2CCTL, 0x00000000);
	set_reg_value(s, GPVNDCTL, 0x00000000);
	set_reg_value(s, GGPIO, 0x00000000);
	set_reg_value(s, GUID, 0x00000000);
	set_reg_value(s, GSNPSID, 0x4f54330a);
	set_reg_value(s, GHWCFG1, 0x00000000);
	set_reg_value(s, GHWCFG2, 0x2288c854);
	set_reg_value(s, GHWCFG3, 0x09160468);
	set_reg_value(s, GHWCFG4, 0x0a008030);
	set_reg_value(s, GLPMCFG, 0x00000000);
	set_reg_value(s, HPTXFSIZ, 0x00000000);
	set_reg_value(s, DPTXFSIZN(1), 0x02000440);
	set_reg_value(s, DPTXFSIZN(2), 0x02000640);
	set_reg_value(s, DPTXFSIZN(3), 0x00000000);
	set_reg_value(s, DPTXFSIZN(4), 0x00000000);
	set_reg_value(s, DPTXFSIZN(5), 0x00000000);
	set_reg_value(s, DPTXFSIZN(6), 0x00000000);
	set_reg_value(s, DPTXFSIZN(7), 0x00000000);
	set_reg_value(s, DPTXFSIZN(8), 0x00000000);
	set_reg_value(s, DPTXFSIZN(9), 0x00000000);
	set_reg_value(s, DPTXFSIZN(10), 0x00000000);
	set_reg_value(s, DPTXFSIZN(11), 0x00000000);
	set_reg_value(s, DPTXFSIZN(12), 0x00000000);
	set_reg_value(s, DPTXFSIZN(13), 0x00000000);
	set_reg_value(s, DPTXFSIZN(14), 0x00000000);
	set_reg_value(s, DPTXFSIZN(15), 0x00000000);
	set_reg_value(s, HCFG, 0x00000000);
	set_reg_value(s, HFIR, 0x00000000);
	set_reg_value(s, HFNUM, 0x00000000);
	set_reg_value(s, HPTXSTS, 0x00000000);
	set_reg_value(s, HAINT, 0x00000000);
	set_reg_value(s, HAINTMSK, 0x00000000);
	set_reg_value(s, HPRT0, 0x00000000);
	set_reg_value(s, HCCHAR(0), 0x00000000);
	set_reg_value(s, DCFG, 0x00000000);
	set_reg_value(s, DCTL, 0x00000000);
	set_reg_value(s, DIEPMSK, 0x00000000);
	set_reg_value(s, DOEPMSK, 0x00000040);
	set_reg_value(s, DAINT, 0x00000000);
	set_reg_value(s, DAINTMSK, 0x00000000);
}

static uint64_t
nexell_usb_read(void *opaque, hwaddr offset, unsigned int size)
{
	NexellUsbState	*s = opaque;
	const NexellUsbRegs *reg_p = nx_usb_regs;
	uint32_t i;

	for (i = 0; i < USB_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			if (NEXELL_DEBUG_USB)
				qemu_log("[read] %s [0x%04x] -> 0x%04x\n", reg_p->name,
						(uint32_t)offset, s->regs[i]);
			return s->regs[i];
		}
		reg_p++;
	}
	qemu_log("QEMU USB ERROR: bad read offset 0x%04x\n", (uint32_t)offset);

	return 0;
}

static void
nexell_usb_write(void *opaque, hwaddr offset,
	uint64_t val, unsigned int size)
{
	NexellUsbState *s = opaque;
	const NexellUsbRegs *reg_p = nx_usb_regs;
	uint32_t i;

	for (i = 0; i < USB_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			if (NEXELL_DEBUG_USB)
				qemu_log("[write] %s [0x%04x] val:0x%04x\n", reg_p->name,
						(uint32_t)offset, (uint32_t)val);
			switch (offset) {
				case GRSTCTL:
					if (val) {
						if (val & GRSTCTL_CSFTRST) {
							if (NEXELL_DEBUG_USB)
								qemu_log("reset all register\n");
							/* give some delay */
							val &= ~GRSTCTL_CSFTRST;
							val |= GRSTCTL_AHBIDLE;
						}
						if (val & GRSTCTL_TXFFLSH) {
							if (NEXELL_DEBUG_USB)
								qemu_log("TX FIFO Flusing\n");
							/* give some delay */
							val &= ~GRSTCTL_TXFFLSH;
							val &= ~GRSTCTL_TXFNUM_MASK;
						}
						if (val & GRSTCTL_RXFFLSH) {
							if (NEXELL_DEBUG_USB)
								qemu_log("RX FIFO Flusing\n");
							/* give some delay */
							val &= ~GRSTCTL_RXFFLSH;
						}
					}
					break;
				default:
					break;
			}
			s->regs[i] = val;
			return;
		}
		reg_p++;
	}
	qemu_log("QEMU USB ERROR: bad write offset 0x%04x\n", (uint32_t)offset);
}

static const MemoryRegionOps usb_ops = {
	.read = nexell_usb_read,
	.write = nexell_usb_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.valid = {
		.min_access_size = 4,
		.max_access_size = 4,
		.unaligned = false
	}
};

/*
* Create usb device.
*/
NexellUsbState *nexell_usb_create(MemoryRegion *address_space, hwaddr base,
		int size, qemu_irq irq)
{
	NexellUsbState *s = g_malloc(sizeof(NexellUsbState));

	s->base_addr = base;
	s->irq = irq;
	memory_region_init_io(&s->mmio, NULL, &usb_ops, s,
				TYPE_NEXELL_USB, size);
	memory_region_add_subregion(address_space, s->base_addr, &s->mmio);
	if (NEXELL_DEBUG_USB)
		qemu_log("[%s] hwaddr base:0x%4x\n", __func__, (int)s->base_addr);
	init_reg_values(s);

	return s;
}
