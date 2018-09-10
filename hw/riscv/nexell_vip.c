/*
 * qemu model of the VIP of the nexell
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
#include "hw/riscv/nexell_vip.h"
#include "exec/log.h"
#include "hw/ppc/spapr.h"

#define RISCV_DEBUG_VIP 0

#define VIP_FRAMERATE_MS	30

static void set_reg_value(NexellVipState *s, hwaddr offset,
		uint64_t val)
{
	const NexellVipRegs *reg_p = nx_vip_regs;
	int i;

	for (i = 0; i < VIP_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			s->regs[i] = val;
		}
		reg_p++;
	}
}

static uint64_t get_reg_value(NexellVipState *s, hwaddr offset)
{
	const NexellVipRegs *reg_p = nx_vip_regs;
	int i;

	for (i = 0; i < VIP_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			return s->regs[i];
		}
		reg_p++;
	}

	return 0;
}

static void update_frame(NexellVipState *s)
{
	uint64_t val = 0;
	uint32_t flag = 0;

	val = get_reg_value(s, VIP_CDENB);
	if (val & (1 << VIP_CLIP_ENB_BIT)) {
		dma_addr_t luaddr = 0, craddr = 0, cbaddr = 0;
		uint32_t size = 0;
		uint8_t *buf = NULL;

		/* get destination address */
		luaddr = get_reg_value(s, CLIP_LUADDR);
		craddr = get_reg_value(s, CLIP_CRADDR);
		cbaddr = get_reg_value(s, CLIP_CBADDR);
		size = (craddr + (craddr - cbaddr)) - luaddr;
		/*qemu_log("[clip_update] addr:%p, size=%d\n", (void*)luaddr, size);*/
		/* copy some frame data */
		if (buf == NULL) {
			buf = g_malloc(size);
			if (buf == NULL)
				qemu_log("Failed to alloc buf for copy\n");
		}
		if (buf) {
			memset((void*)buf, 0x66, size);
			cpu_physical_memory_write((int64_t)luaddr, buf, size);
			flag++;
			if (buf)
				free(buf);
		}
	}
	if (val & (1 << VIP_DECI_ENB_BIT)) {
		dma_addr_t luaddr = 0, craddr = 0, cbaddr = 0;
		uint32_t size = 0;
		uint8_t *buf = NULL;

		/* get destination address */
		luaddr = get_reg_value(s, DECI_LUADDR);
		craddr = get_reg_value(s, DECI_CRADDR);
		cbaddr = get_reg_value(s, DECI_CBADDR);
		size = (craddr + (craddr - cbaddr)) - luaddr;
		/*qemu_log("[deci_update] addr:%p, size=%d\n", (void*)luaddr, size);*/
		if (buf == NULL) {
			buf = g_malloc(size);
			if (buf == NULL)
				qemu_log("Failed to alloc buf for copy\n");
		}
		if (buf) {
			memset((void*)buf, 0x33, size);
			/* copy some frame data */
			cpu_physical_memory_write((int64_t)luaddr, buf, size);
			flag++;
			if (buf)
				free(buf);
		}
	}
	if (val & (1 << VIP_SEP_ENB_BIT))
		flag++;

	if (flag) {
		val = get_reg_value(s, VIP_HVINT);
		val |= (VIP_HVINT_PEND_VAL << VIP_HVINT_PEND_BIT);
		set_reg_value(s, VIP_HVINT, val);
		if (val & (1 << VIP_HVINT_ENB_BIT))
			qemu_irq_raise(s->irq);
		val = get_reg_value(s, VIP_ODINT);
		val |= (1 << VIP_ODINT_PEND_BIT);
		set_reg_value(s, VIP_ODINT, val);
		if (val & (1 << VIP_ODINT_ENB_BIT))
			qemu_irq_raise(s->irq);
		timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + VIP_FRAMERATE_MS);
	}
}

static void set_stream(NexellVipState *s, bool enable)
{
	if (RISCV_DEBUG_VIP)
		qemu_log("[%s] %s\n", __func__, (enable) ? "stream start" : "stop stream");

	if (enable) {
		s->timer = timer_new_ms(QEMU_CLOCK_VIRTUAL, (QEMUTimerCB*)update_frame, s);
		timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + VIP_FRAMERATE_MS);
	} else {
		timer_del(s->timer);
		timer_free(s->timer);
	}
}

static uint64_t
nexell_vip_read(void *opaque, hwaddr offset, unsigned int size)
{
	NexellVipState	*s = opaque;
	const NexellVipRegs *reg_p = nx_vip_regs;
	unsigned int i;

	for (i = 0; i < VIP_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			if (RISCV_DEBUG_VIP)
				qemu_log("[read] %s [0x%04x] -> 0x%04x\n", reg_p->name,
						(uint32_t)offset, s->regs[i]);
			return s->regs[i];
		}
		reg_p++;
	}
	qemu_log("QEMU VIP ERROR: bad read offset 0x%04x\n", (uint32_t)offset);

	return 0;
}

static void
nexell_vip_write(void *opaque, hwaddr offset,
	uint64_t val, unsigned int size)
{
	NexellVipState *s = opaque;
	const NexellVipRegs *reg_p = nx_vip_regs;
	unsigned int i;

	for (i = 0; i < VIP_NUM_OF_REGISTERS; i++) {
		if (reg_p->offset == offset) {
			if (RISCV_DEBUG_VIP)
				qemu_log("[write] %s [0x%04x] val:0x%04x\n", reg_p->name,
						(uint32_t)offset, (uint32_t)val);
			switch (offset) {
				case VIP_HVINT:
					if (!val) {
						if (RISCV_DEBUG_VIP)
							qemu_log("disable h/vsync interrrupt all\n");
					} else {
						if (val &
							(VIP_HVINT_PEND_VAL << VIP_HVINT_PEND_BIT)) {
								if (RISCV_DEBUG_VIP)
									qemu_log("clear h/vsync interrupt\n");
								val &= ~VIP_HVINT_PEND_MASK;
							}
					}
					break;
				case VIP_ODINT:
					if (!val) {
						if (RISCV_DEBUG_VIP)
							qemu_log("disable done interrrup\nt");
					} else {
						if (val & (1 << VIP_ODINT_PEND_BIT)) {
							if (RISCV_DEBUG_VIP)
								qemu_log("clear done interrupt\n");
							val &= ~(1 << VIP_ODINT_PEND_BIT);
						}
					}
					break;
				case VIP_CDENB:
					if (!val) {
						if (RISCV_DEBUG_VIP)
							qemu_log("disable seperator, clip and deci\n");
					} else {
						uint64_t v = get_reg_value(s, offset);

						if (val & (1 << VIP_SEP_ENB_BIT)) {
							if (!(v & (1 << VIP_SEP_ENB_BIT))) {
								if (RISCV_DEBUG_VIP)
									qemu_log("enable seperator\n");
							}
						} else {
							if (v & (1 << VIP_SEP_ENB_BIT)) {
								if (RISCV_DEBUG_VIP)
									qemu_log("disable seperator\n");
							}
						}
						if (val & (1 << VIP_CLIP_ENB_BIT)) {
							if (!(v & (1 << VIP_CLIP_ENB_BIT))) {
								if (RISCV_DEBUG_VIP)
									qemu_log("enable clipper\n");
							}
						} else {
							if (v & (1 << VIP_CLIP_ENB_BIT)) {
								if (RISCV_DEBUG_VIP)
									qemu_log("disable clipper\n");
							}
						}
						if (val & (1 << VIP_DECI_ENB_BIT)) {
							if (!(v & (1 << VIP_DECI_ENB_BIT))) {
								if (RISCV_DEBUG_VIP)
									qemu_log("enable decimator\n");
							}
						} else {
							if (v & (1 << VIP_DECI_ENB_BIT)) {
								if (RISCV_DEBUG_VIP)
									qemu_log("disable decimator\n");
							}
						}
					}
					break;
				case VIP_CONFIG:
					if (!val) {
						if (RISCV_DEBUG_VIP)
							qemu_log("disable vip");
					} else {
						uint64_t v = get_reg_value(s, offset);
						if (val & (1 << VIP_ENB_BIT)) {
							if (!(v & (1 << VIP_ENB_BIT))) {
								if (RISCV_DEBUG_VIP)
									qemu_log("enable vip\n");
								set_stream(s, true);
							}
						} else {
							if (v & (1 << VIP_ENB_BIT)) {
								if (RISCV_DEBUG_VIP)
									qemu_log("disable vip\n");
								set_stream(s, false);
							}
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
	qemu_log("QEMU VIP ERROR: bad read offset 0x%04x\n", (uint32_t)offset);
}

static const MemoryRegionOps vip_ops = {
	.read = nexell_vip_read,
	.write = nexell_vip_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.valid = {
		.min_access_size = 4,
		.max_access_size = 4,
		.unaligned = false
	}
};

/*
* Create vip device.
*/
NexellVipState *nexell_vip_create(MemoryRegion *address_space, hwaddr base,
		int size, qemu_irq irq)
{
	NexellVipState *s = g_malloc(sizeof(NexellVipState));

	s->base_addr = base;
	s->irq = irq;
	memory_region_init_io(&s->mmio, NULL, &vip_ops, s,
				TYPE_NEXELL_VIP, size);
	memory_region_add_subregion(address_space, s->base_addr, &s->mmio);
	if (RISCV_DEBUG_VIP)
		qemu_log("[%s] hwaddr base:0x%4x\n", __func__, (int)s->base_addr);
	return s;
}
