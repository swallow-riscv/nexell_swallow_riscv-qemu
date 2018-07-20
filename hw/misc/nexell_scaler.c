/*
 * QEMU model of the Scaler of the Nexell
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
#include "qapi/error.h"
#include "hw/sysbus.h"
#include "chardev/char.h"
#include "chardev/char-fe.h"
#include "sysemu/dma.h"
#include "target/riscv/cpu.h"
#include "hw/misc/nexell_scaler.h"
#include "exec/log.h"
#include "hw/ppc/spapr.h"

#define NEXELL_SCALER_INT_ENABLE	16
#define NEXELL_SCALER_INT_CLEAR		8

enum {
	NEXELL_SCALER_INT_DONE = 0,
	NEXELL_SCALER_INT_CMD_PROC = 1,
};

enum {
	CMD_HEADER = 0,
	CMD_SRC_ADDR = 1,
	CMD_SRC_STRIDE = 2,
	CMD_SRC_SIZE = 3,
	CMD_DST_ADDR = 4,
	CMD_DST_STRIDE = 5,
	CMD_DST_SIZE = 8,
};

static uint64_t
nexell_scaler_read(void *opaque, hwaddr addr, unsigned int size)
{
	NexellScalerState *s = opaque;
	uint64_t val = 0;
	int i, j;

	qemu_log("[%s] addr:0x%4x, size:0x%x\n", __func__,
			(int)addr, size);

	switch (addr) {
	case NEXELL_SCALER_RUNREG:
		val = s->regs.runreg;
		break;
	case NEXELL_SCALER_CFGREG:
		val = s->regs.cfgreg;
		break;
	case NEXELL_SCALER_INTREG:
		val = s->regs.intreg;
		break;
	case NEXELL_SCALER_SRCADDR:
		val = s->regs.srcaddrreg;
		break;
	case NEXELL_SCALER_SRCSTRIDE:
		val = s->regs.srcstride;
		break;
	case NEXELL_SCALER_SRCSIZE:
		val = s->regs.srcsizereg;
		break;
	case NEXELL_SCALER_DESTADDR0:
		val = s->regs.destaddrreg0;
		break;
	case NEXELL_SCALER_DESTSTRIDE0:
		val = s->regs.deststride0;
		break;
	case NEXELL_SCALER_DESTADDR1:
		val = s->regs.destaddrreg1;
		break;
	case NEXELL_SCALER_DESTSTRIDE1:
		val = s->regs.deststride1;
		break;
	case NEXELL_SCALER_DESTSIZE:
		val = s->regs.destsizereg;
		break;
	case NEXELL_SCALER_DELTAX:
		val = s->regs.deltaxreg;
		break;
	case NEXELL_SCALER_DELTAY:
		val = s->regs.deltayreg;
		break;
	case NEXELL_SCALER_HYSOFTREG:
		val = s->regs.hvsoftreg;
		break;
	case NEXELL_SCALER_CMDBUFADDR:
		break;
	case NEXELL_SCALER_CMDBUFCON:
		val = s->regs.cmdbufcon;
		break;
	default:
		if ((addr >= NEXELL_SCALER_YVFILTER) &&
				(addr <= NEXELL_SCALER_YVFILTER_MAX)) {
			addr -= NEXELL_SCALER_YVFILTER;
			j = addr / 4;
			i = j / 8;
			j = j % 8;
			val = s->regs.yvfilter[i][j];
			break;
		} else if ((addr >= NEXELL_SCALER_YHFILTER) &&
				(addr <= NEXELL_SCALER_YHFILTER_MAX)) {
			addr -= NEXELL_SCALER_YHFILTER;
			j = addr / 4;
			i = j / 32;
			j = j % 32;
			val = s->regs.yhfilter[i][j];
			break;
		} else
			error_report("[%s] invalid addr:0x%4x\n", __func__, (int)addr);
		break;
	}
	qemu_log("[%s] addr:0x%4x, val:0x%4x, size:0x%x\n",
			__func__, (int)addr, (int)val, size);
	return val;
}

static void
nexell_scaler_write(void *opaque, hwaddr addr,
	uint64_t val, unsigned int size)
{
	NexellScalerState *s = opaque;
	int i, j;

	qemu_log("[%s] addr:0x%4x, val:0x%4x, size:0x%x\n",
			__func__, (int)addr, (int)val, size);

	switch (addr) {
	case NEXELL_SCALER_RUNREG:
		s->regs.runreg = val;
		break;
	case NEXELL_SCALER_CFGREG:
		s->regs.cfgreg = val;
		break;
	case NEXELL_SCALER_INTREG:
	{
		uint32_t status = s->regs.intreg;
		if (!val)
			qemu_log("[%s] disble interrupt all\n", __func__);
		else{
			if (val & (1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_ENABLE))) {
				if (!(status & (1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_ENABLE))))
					qemu_log("enable CMD_PROC interrupt\n");
			}
			if (!(val & (1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_ENABLE)))) {
				if (status & (1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_ENABLE)))
					qemu_log("disable CMD_PROC interrupt\n");
			}

			if (val & (1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_CLEAR))) {
				qemu_log("clear CMD_PROC interrupt\n");
				val &= ~(1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_CLEAR));
				val &= ~(1 << NEXELL_SCALER_INT_CMD_PROC);
			}
			if (val & (1 << (NEXELL_SCALER_INT_DONE + NEXELL_SCALER_INT_CLEAR))) {
				qemu_log("clear  Done interrupt\n");
				val &= ~(1 << (NEXELL_SCALER_INT_DONE + NEXELL_SCALER_INT_CLEAR));
				val &= ~(1 << NEXELL_SCALER_INT_DONE);
			}
		}
		s->regs.intreg = val;
	}
		break;
	case NEXELL_SCALER_SRCADDR:
		s->regs.srcaddrreg = val;
		break;
	case NEXELL_SCALER_SRCSTRIDE:
		s->regs.srcstride = val;
		break;
	case NEXELL_SCALER_SRCSIZE:
		s->regs.srcsizereg = val;
		break;
	case NEXELL_SCALER_DESTADDR0:
		s->regs.destaddrreg0 = val;
		break;
	case NEXELL_SCALER_DESTSTRIDE0:
		s->regs.deststride0 = val;
		break;
	case NEXELL_SCALER_DESTADDR1:
		s->regs.destaddrreg1 = val;
		break;
	case NEXELL_SCALER_DESTSTRIDE1:
		s->regs.deststride1 = val;
		break;
	case NEXELL_SCALER_DESTSIZE:
		s->regs.destsizereg = val;
		break;
	case NEXELL_SCALER_DELTAX:
		s->regs.deltaxreg = val;
		break;
	case NEXELL_SCALER_DELTAY:
		s->regs.deltayreg = val;
		break;
	case NEXELL_SCALER_HYSOFTREG:
		s->regs.hvsoftreg = val;
		break;
	case NEXELL_SCALER_CMDBUFADDR:
		s->regs.cmdbufaddr = (uint32_t*)val;
		break;
	case NEXELL_SCALER_CMDBUFCON:
	{
		qemu_log("[%s] scaler %s\n", __func__, (val) ? "run":"stop");
		if (val) {
			uint32_t *cmd_buf = s->regs.cmdbufaddr;
			dma_addr_t dst_addr = (dma_addr_t)NULL, src_addr = (dma_addr_t)NULL;
			int src_size, dst_size, src_stride, dst_stride;
			uint32_t buf[CMD_DST_SIZE+1];
			/*uint8_t *src_buf = NULL;*/
			/*uint32_t i = 0;*/

			cpu_physical_memory_read((int64_t)cmd_buf, buf, sizeof(buf));
			src_addr = (dma_addr_t)buf[CMD_SRC_ADDR];
			src_stride = buf[CMD_SRC_STRIDE];
			src_size = buf[CMD_SRC_SIZE];
			dst_addr = (dma_addr_t)buf[CMD_DST_ADDR];
			dst_stride = buf[CMD_DST_STRIDE];
			dst_size = buf[CMD_DST_SIZE];
			qemu_log("[Scaling] src - addr:%p, stride:%d, size:%d\n",
					(void*)src_addr, src_stride, src_size);
			qemu_log("[Scaling] dst - addr:%p, stride:%d, size:%d\n",
					(void*)dst_addr, dst_stride, dst_size);
			/*src_buf = g_malloc(1280);
			if (src_buf == NULL) {
				qemu_log("[Scaling] Failed to alloc buf for src\n");
				break;
			}
			for (i = 0; i < src_size; i += 1280) {
				cpu_physical_memory_read((int64_t)src_addr + i, src_buf, 1280);
				qemu_log("[%d]write\n", i);
				cpu_physical_memory_write((int64_t)dst_addr + i, src_buf, 1280);
			}
			qemu_log("[Scaling] copy done\n");
			if (s->regs.intreg &
					(1 << (NEXELL_SCALER_INT_CMD_PROC + NEXELL_SCALER_INT_ENABLE))) {
				s->regs.intreg |=
					((1 << NEXELL_SCALER_INT_CMD_PROC) | (1 << NEXELL_SCALER_INT_DONE));
				qemu_irq_raise(s->irq);
				qemu_log("[Scaling] generate CMD_PROC and DONE interrupts\n");
			}
			if (src_buf)
				g_free(src_buf);
			*/
			qemu_log("[Scaling] free buf\n");
		}
		s->regs.cmdbufcon = val;
	}
		break;
	default:
		if ((addr >= NEXELL_SCALER_YVFILTER) &&
				(addr <= NEXELL_SCALER_YVFILTER_MAX)) {
			addr -= NEXELL_SCALER_YVFILTER;
			j = addr / 4;
			i = j / 8;
			j = j % 8;
			s->regs.yvfilter[i][j] = val;
			break;
		} else if ((addr >= NEXELL_SCALER_YHFILTER) &&
				(addr <= NEXELL_SCALER_YHFILTER_MAX)) {
			addr -= NEXELL_SCALER_YHFILTER;
			j = addr / 4;
			i = j / 32;
			j = j % 32;
			s->regs.yhfilter[i][j] = val;
			break;
		} else
			error_report("[%s] invalid addr:0x%4x\n", __func__, (int)addr);
		break;
	}
}

static const MemoryRegionOps scaler_ops = {
	.read = nexell_scaler_read,
	.write = nexell_scaler_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.valid = {
		.min_access_size = 4,
		.max_access_size = 4,
		.unaligned = false
	}
};

/*
* Create Scaler device.
*/
NexellScalerState *nexell_scaler_create(MemoryRegion *address_space, hwaddr base,
		qemu_irq irq)
{
	NexellScalerState *s = g_malloc(sizeof(NexellScalerState));

	s->base_addr = base;
	s->irq = irq;
	memset((void*)&s->regs, 0x0, sizeof(struct nexell_scaler_register));
	memory_region_init_io(&s->mmio, NULL, &scaler_ops, s,
				TYPE_NEXELL_SCALER, NEXELL_SCALER_MAX);
	memory_region_add_subregion(address_space, s->base_addr, &s->mmio);
	qemu_log("[%s] hwaddr base:0x%4x\n", __func__, (int)s->base_addr);
	return s;
}
