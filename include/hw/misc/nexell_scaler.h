/*
 * Nexell Scaler interface
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

#ifndef HW_NEXELL_SCALER_H
#define HW_NEXELL_SCALER_H

#define NEXELL_SCALER_MAX 0x1000
#define TYPE_NEXELL_SCALER "misc.nexell.scaler"

#define NEXELL_SCALER(obj) \
    OBJECT_CHECK(NexellScalerState, (obj), TYPE_NEXELL_SCALER)

/*
 * Offsets for Scaler Register
 */

#define NEXELL_SCALER_RUNREG		0x0000
#define NEXELL_SCALER_CFGREG		0x0004
#define NEXELL_SCALER_INTREG		0x0008
#define NEXELL_SCALER_SRCADDR		0x000c
#define NEXELL_SCALER_SRCSTRIDE		0x0010
#define NEXELL_SCALER_SRCSIZE		0x0014
#define NEXELL_SCALER_DESTADDR0		0x0018
#define NEXELL_SCALER_DESTSTRIDE0	0x001c
#define NEXELL_SCALER_DESTADDR1		0x0020
#define NEXELL_SCALER_DESTSTRIDE1	0x0024
#define NEXELL_SCALER_DESTSIZE		0x0028
#define NEXELL_SCALER_DELTAX		0x002c
#define NEXELL_SCALER_DELTAY		0x0030
#define NEXELL_SCALER_HYSOFTREG		0x0034
#define NEXELL_SCALER_CMDBUFADDR	0x0038
#define NEXELL_SCALER_CMDBUFCON		0x003c
#define NEXELL_SCALER_YVFILTER		0x0040
#define NEXELL_SCALER_YVFILTER_MAX	0x009c
#define NEXELL_SCALER_YHFILTER		0x0100
#define NEXELL_SCALER_YHFILTER_MAX	0x037c

struct nexell_scaler_register {
	uint32_t runreg;
	uint32_t cfgreg;
	uint32_t intreg;
	uint32_t srcaddrreg;
	uint32_t srcstride;
	uint32_t srcsizereg;
	uint32_t destaddrreg0;
	uint32_t deststride0;
	uint32_t destaddrreg1;
	uint32_t deststride1;
	uint32_t destsizereg;
	uint32_t deltaxreg;
	uint32_t deltayreg;
	uint32_t hvsoftreg;
	uint32_t *cmdbufaddr;
	uint32_t cmdbufcon;
	uint32_t yvfilter[3][8];
	int32_t yhfilter[5][32];
};

typedef struct {
	/*< private >*/
	SysBusDevice parent_obj;
	/*< public >*/
	MemoryRegion mmio;
	hwaddr base_addr;
	qemu_irq irq;
	struct nexell_scaler_register regs;
} NexellScalerState;

NexellScalerState *nexell_scaler_create(MemoryRegion *address_space, hwaddr base,
		qemu_irq irq);

#endif
