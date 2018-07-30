/*
 * Nexell VIP interface
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

#ifndef HW_NEXELL_VIP_H
#define HW_NEXELL_VIP_H

#define TYPE_NEXELL_VIP "riscv.nexell.vip"

#define NEXELL_VIP(obj) \
    OBJECT_CHECK(NexellVipState, (obj), TYPE_NEXELL_VIP)

/*
 * Offsets for Vip Register
 */

#define VIP_CONFIG		0x0000
#define VIP_HVINT		0x0004
#define VIP_SYNCCTRL		0x0008
#define VIP_SYNCMON		0x000C
#define VIP_VBEGIN		0x0010
#define VIP_VEND		0x0014
#define VIP_HBEGIN		0x0018
#define VIP_HEND		0x001C
#define VIP_FIFOCTRL		0x0020
#define VIP_HCOUNT		0x0024
#define VIP_VCOUNT		0x0028
#define VIP_PADCLK_SEL		0x0030
#define VIP_INFIFOCLR		0x0034
#define VIP_CDENB		0x0200
#define VIP_ODINT		0x0204
#define VIP_IMGWIDTH		0x0208
#define VIP_IMGHEIGHT		0x020C
#define CLIP_LEFT		0x0210
#define CLIP_RIGHT		0x0214
#define CLIP_TOP		0x0218
#define CLIP_BOTTOM		0x021C
#define DECI_TARGETW		0x0220
#define DECI_TARGETH		0x0224
#define DECI_DELTAW		0x0228
#define DECI_DELTAH		0x022C
#define DECI_CLEARW		0x0230
#define DECI_CLEARH		0x0234
#define DECI_FORMAT		0x0244
#define DECI_LUADDR		0x0248
#define DECI_LUSTRIDE		0x024C
#define DECI_CRADDR		0x0250
#define DECI_CRSTRIDE		0x0254
#define DECI_CBADDR		0x0258
#define DECI_CBSTRIDE		0x025C
#define CLIP_FORMAT		0x0288
#define CLIP_LUADDR		0x028C
#define CLIP_LUSTRIDE		0x0290
#define CLIP_CRADDR		0x0294
#define CLIP_CRSTRIDE		0x0298
#define CLIP_CBADDR		0x029C
#define CLIP_CBSTRIDE		0x02A0
#define VIP_SCANMODE		0x02C0
#define VIP_PORTSEL		0x02D8

/*VIP_CONFIG*/
#define VIP_ENB_BIT		0
/*VIP_CDENB*/
#define VIP_SEP_ENB_BIT		8
#define VIP_CLIP_ENB_BIT	1
#define VIP_DECI_ENB_BIT	0
/*VIP_HVINT*/
#define VIP_HVINT_PEND_BIT	0
#define VIP_HVINT_PEND_VAL	0x3
#define VIP_HVINT_PEND_MASK	0x03
#define VIP_HVINT_ENB_BIT	4
#define VIP_HVINT_ENB_VAL	0x3
#define VIP_HVINT_ENB_MASK	0x30
/*VIP_ODINT*/
#define VIP_ODINT_PEND_BIT	0
#define VIP_ODINT_ENB_BIT	8

typedef struct {
    const char  *name; /* for debug only */
    uint32_t     offset;
} NexellVipRegs;

static const NexellVipRegs nx_vip_regs[] = {
	{"vip_config", VIP_CONFIG},
	{"vip_hvint", VIP_HVINT},
	{"vip_syncctrl", VIP_SYNCCTRL},
	{"vip_syncmon", VIP_SYNCMON},
	{"vip_vbegin", VIP_VBEGIN},
	{"vip_vend", VIP_VEND},
	{"vip_hbegin", VIP_HBEGIN},
	{"vip_hend", VIP_HEND},
	{"vip_fifoctrl", VIP_FIFOCTRL},
	{"vip_hcount", VIP_HCOUNT},
	{"vip_vcount", VIP_VCOUNT},
	{"vip_padclk_sel", VIP_PADCLK_SEL},
	{"vip_infifoclr", VIP_INFIFOCLR},
	{"vip_cdenb", VIP_CDENB},
	{"vip_odint", VIP_ODINT},
	{"vip_imgwidth", VIP_IMGWIDTH},
	{"vip_imgheight", VIP_IMGHEIGHT},
	{"clip_left", CLIP_LEFT},
	{"clip_right", CLIP_RIGHT},
	{"clip_top", CLIP_TOP},
	{"clip_bottom", CLIP_BOTTOM},
	{"deci_targetw", DECI_TARGETW},
	{"deci_targeth", DECI_TARGETH},
	{"deci_deltaw", DECI_DELTAW},
	{"deci_deltah", DECI_DELTAH},
	{"deci_clearw", DECI_CLEARW},
	{"deci_clearh", DECI_CLEARH},
	{"deci_format", DECI_FORMAT},
	{"deci_luaddr", DECI_LUADDR},
	{"deci_lustride", DECI_LUSTRIDE},
	{"deci_craddr", DECI_CRADDR},
	{"deci_crstride", DECI_CRSTRIDE},
	{"deci_cbaddr", DECI_CBADDR},
	{"deci_cbstride", DECI_CBSTRIDE},
	{"clip_format", CLIP_FORMAT},
	{"clip_luaddr", CLIP_LUADDR},
	{"clip_lustride", CLIP_LUSTRIDE},
	{"clip_craddr", CLIP_CRADDR},
	{"clip_crstride",CLIP_CRSTRIDE},
	{"clip_cbaddr", CLIP_CBADDR},
	{"clip_cbstride", CLIP_CBSTRIDE},
	{"vip_scanmode", VIP_SCANMODE},
	{"vip_portsel", VIP_PORTSEL},
};

#define VIP_NUM_OF_REGISTERS ARRAY_SIZE(nx_vip_regs)

typedef struct {
	/*< private >*/
	SysBusDevice parent_obj;
	/*< public >*/
	MemoryRegion mmio;
	hwaddr base_addr;
	qemu_irq irq;
	QEMUTimer *timer;
	uint32_t regs[VIP_NUM_OF_REGISTERS];
} NexellVipState;

NexellVipState *nexell_vip_create(MemoryRegion *address_space, hwaddr base,
		int size, qemu_irq irq);

#endif
