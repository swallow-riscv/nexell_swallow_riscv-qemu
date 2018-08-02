/*
 * NEXELL I2C Bus Serial Interface Emulation
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
#include "hw/i2c/nexell_i2c.h"
#include "hw/i2c/i2c.h"
#include "qemu/log.h"
#include "exec/address-spaces.h"

#ifndef DEBUG_NEXELL_I2C
#define DEBUG_NEXELL_I2C 0
#endif

#define DPRINTF(fmt, args...) \
    do { \
        if (DEBUG_NEXELL_I2C) { \
            fprintf(stderr, "[%s]%s: " fmt , TYPE_NEXELL_I2C, \
                                             __func__, ##args); \
        } \
    } while (0)

static const char *nexell_i2c_get_regname(unsigned offset)
{
	switch (offset) {
	case NEXELL_I2C_CON:
		return "NEXELL_I2C_CON";
	case NEXELL_I2C_TAR:
		return "NEXELL_I2C_TAR";
	case NEXELL_I2C_SAR:
		return "NEXELL_I2C_SAR";
	case NEXELL_I2C_HS_MADDR:
		return "NEXELL_I2C_HS_MADDR";
	case NEXELL_I2C_DATA_CMD:
		return "NEXELL_I2C_DATA_CMD";
	case NEXELL_I2C_SS_SCL_HCNT:
		return "NEXELL_I2C_SS_SCL_HCNT";
	case NEXELL_I2C_SS_SCL_LCNT:
		return "NEXELL_I2C_SS_SCL_LCNT:";
	case NEXELL_I2C_FS_SCL_LCNT:
		return "NEXELL_I2C_FS_SCL_LCNT";
	case NEXELL_I2C_HS_SCL_HCNT:
		return "NEXELL_I2C_HS_SCL_HCNT";
	case NEXELL_I2C_INTR_STAT:
		return "NEXELL_I2C_INTR_STAT";
	case NEXELL_I2C_INTR_MASK:
		return "NEXELL_I2C_INTR_MASK";
	case NEXELL_I2C_RAW_INTR_STAT:
		return "NEXELL_I2C_RAW_INTR_STAT";
	case NEXELL_I2C_RX_TL:
		return "NEXELL_I2C_RX_TL";
	case NEXELL_I2C_CLR_INTR:
		return "NEXELL_I2C_CLR_INTR";
	case NEXELL_I2C_CLR_RX_UNDER:
		return "NEXELL_I2C_CLR_RX_UNDER";
	case NEXELL_I2C_CLR_RX_OVER:
		return "NEXELL_I2C_CLR_RX_UNDER";
	case NEXELL_I2C_CLR_TX_OVER:
		return "NEXELL_I2C_CLR_TX_OVER";
	case NEXELL_I2C_CLR_RD_REQ:
		return "NEXELL_I2C_CLR_RD_REQ";
	case NEXELL_I2C_CLR_TX_ABRT:
		return "NEXELL_I2C_CLR_TX_ABRT";
	case NEXELL_I2C_CLR_RX_DONE:
		return "NEXELL_I2C_CLR_RX_DONE";
	case NEXELL_I2C_CLR_STOP_DET:
		return "NEXELL_I2C_CLR_STOP_DET";
	case NEXELL_I2C_CLR_START_DET:
		return "NEXELL_I2C_CLR_START_DET";
	case NEXELL_I2C_CLR_GEN_CALL:
		return "NEXELL_I2C_CLR_GEN_CALL";
	case NEXELL_I2C_STATUS:
		return "NEXELL_I2C_STATUS";
	case NEXELL_I2C_TXFLR:
		return "NEXELL_I2C_TXFLR";
	case NEXELL_I2C_RXFLR:
		return "NEXELL_I2C_RXFLR";
	case NEXELL_I2C_SDA_HOLD:
		return "NEXELL_I2C_SDA_HOLD";
	case NEXELL_I2C_SLV_DATA_NACK_ONLY:
		return "NEXELL_I2C_SLV_DATA_NACK_ONLY";
	case NEXELL_I2C_DMA_CR:
		return "NEXELL_I2C_DMA_RDLR";
	case NEXELL_I2C_DMA_TDLR:
		return "NEXELL_I2C_DMA_RDLR";
	case NEXELL_I2C_DMA_RDLR:
		return "NEXELL_I2C_DMA_RDLR";
	case NEXELL_I2C_SDA_SETUP:
		return "NEXELL_I2C_SDA_SETUP";
	case NEXELL_I2C_ACK_GENERAL_CALL:
		return "NEXELL_I2C_ACK_GENERAL_CALL";
	case NEXELL_I2C_COMP_PARAM_1:
		return "NEXELL_I2C_COMP_PARAM_1";

	default:
		return "[?]";
	}
}

static inline bool nexell_i2c_is_enabled(NEXELLI2CState *s)
{
	return s->enable & I2C_ENABLE;
}

static inline bool nexell_i2c_status_active(NEXELLI2CState *s)
{
	return (s->status & I2C_STATUS_ACTIVE);
}

static inline bool nexell_i2c_is_master(NEXELLI2CState *s)
{
	return s->con & I2C_MASTER_ENABLE;
}

static void nexell_i2c_reset(DeviceState *dev)
{
	NEXELLI2CState *s = NEXELL_I2C(dev);

	s->con = CON_RESET;
	s->tar = TARGET_ADDR_RESET;
	s->sar = SLAVE_ADDR_RESET;
	s->enable = ENABLE_RESET;
	s->status = STATUS_RESET;
	s->txflr = TXFLR_RESET;
	s->rxflr = RXFLR_RESET;
	s->tx_abrt_source = TX_ABRT_SOURCE_RESET;
	s->data_cmd = DATA_RESET;
	s->ss_scl_hcnt = SS_SCL_HCNT_RESET;
	s->ss_scl_lcnt = SS_SCL_LCNT_RESET;
	s->fs_scl_hcnt = FS_SCL_HCNT_RESET;
	s->fs_scl_lcnt = FS_SCL_LCNT_RESET;
	s->intr_stat = INTR_STAT_RESET;
	s->intr_mask = INT_MASK_RESET;
	s->raw_intr_stat = RAW_INTR_STATUS_RESET;
	s->rx_tl = RX_TL_RESET;
	s->tx_tl = TX_TL_RESET;
	s->clr_intr = CLR_INTR_RESET;
	s->clr_rx_under = CLR_RX_UNDER_RESET;
	s->clr_rx_over = CLR_RX_OVER_RESET;
	s->clr_tx_over = CLR_TX_OVER_RESET;
	s->clr_rd_req = CLR_RD_REQ_RESET;
	s->clr_tx_abrt = CLR_TX_ABRT_RESET;
	s->clr_rx_done = CLR_RX_DONE_RESET;
	s->clr_activity = CLR_ACTIVITY_RESET;
	s->clr_stop_det = CLR_STOP_DET_RESET;
	s->clr_gen_call = CLR_GEN_CALL_RESET;
	s->sda_hold = SDA_HOLD_RESET;
	s->enable_status = ENABLE_STATUS_RESET;
	s->com_param1 = COM_PARAM1_RESET|I2C_TX_BUFFER_DEPTH_MASK|I2C_RX_BUFFER_DEPTH_MASK|I2C_COMP_PARAM1_SPEED_MASK;
	s->com_version = COM_VERSION_RESET;
	s->com_type = COM_TYPE_RESET;

	s->i2dr_read  = DATA_RESET;
	s->i2dr_write = DATA_RESET;
}

static inline void nexell_i2c_raise_interrupt(NEXELLI2CState *s)
{
	if (nexell_i2c_is_enabled(s) && nexell_i2c_status_active(s)) {
		qemu_irq_raise(s->irq);
	}
}

static uint64_t nexell_i2c_read(void *opaque, hwaddr offset,
                             unsigned size)
{
	uint32_t value;
	NEXELLI2CState *s = NEXELL_I2C(opaque);


	switch (offset) {
	case NEXELL_I2C_COMP_PARAM_1:
		value = s->com_param1;
		break;

	case NEXELL_I2C_COMP_TYPE:
		value = s ->com_type;
		break;

	case NEXELL_I2C_DATA_CMD:
		value = s->i2dr_read;

		if (nexell_i2c_is_master(s)) {
			int ret = BIT_MASK_8;
			if (s->tar == TARGET_ADDR_RESET) {
				qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Trying to read "
						"without specifying the slave address\n",
						TYPE_NEXELL_I2C, __func__);
			} else {
				ret = i2c_recv(s->bus);

				if (ret >= 0) {
					nexell_i2c_raise_interrupt(s);
				} else {
					qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: read failed "
							"for device 0x%02x\n", TYPE_NEXELL_I2C,
							__func__, s->tar);
					ret = BIT_MASK_8;
				}
			}
			s->i2dr_read = ret;
		} else {
			qemu_log_mask(LOG_UNIMP, "[%s]%s: slave mode not implemented\n",
					TYPE_NEXELL_I2C, __func__);
		}
		break;
	case NEXELL_I2C_CON:
		value = s->con;
		break;
	case NEXELL_I2C_SAR:
		value = s->sar;
		break;
	case NEXELL_I2C_TAR:
		value = s->tar;
		break;
	case NEXELL_I2C_INTR_STAT:
		value = s->intr_stat;
		break;
	case NEXELL_I2C_RAW_INTR_STAT:
		value = s->raw_intr_stat;
		break;
	case NEXELL_I2C_CLR_INTR:
		value = s->intr_mask;
		s->intr_stat = INTR_STAT_RESET;
		s->raw_intr_stat = RAW_INTR_STATUS_RESET;
		break;
	case NEXELL_I2C_CLR_RX_UNDER:
		value = s->clr_rx_under;
		s->intr_stat &= ~INTR_RX_UNDER;
		break;
	case NEXELL_I2C_CLR_RX_OVER:
		value = s->clr_rx_over;
		s->intr_stat &= ~INTR_RX_OVER;
		break;
	case NEXELL_I2C_CLR_TX_OVER:
		value = s->clr_tx_over;
		s->intr_stat &= ~INTR_TX_OVER;
		break;
	case NEXELL_I2C_CLR_RD_REQ:
		value = s->clr_rd_req;
		s->intr_stat &= ~INTR_RD_REQ;
		break;
	case NEXELL_I2C_CLR_TX_ABRT:
		value = s->clr_tx_abrt;
		s->intr_stat &= ~INTR_TX_ABRT;
		break;
	case NEXELL_I2C_CLR_RX_DONE:
		value = s->clr_rx_done;
		s->intr_stat &= ~INTR_RX_DONE;
		break;
	case NEXELL_I2C_CLR_ACTIVITY:
		value = s->clr_activity;
		s->intr_stat &= ~INTR_ACTIVITY;
		break;
	case NEXELL_I2C_CLR_STOP_DET:
		value = s->clr_stop_det;
		s->intr_stat &= ~INTR_STOP_DET;
		break;
	case NEXELL_I2C_CLR_START_DET:
		value = s->clr_start_det;
		s->intr_stat &= ~INTR_START_DET;
		break;
	case NEXELL_I2C_CLR_GEN_CALL:
		value = s->clr_gen_call;
		s->intr_stat &= ~INTR_GEN_CALL;
		break;
	case NEXELL_I2C_ENABLE:
		value = s->enable;
		break;
	case NEXELL_I2C_STATUS:
		s->status &= ~I2C_STATUS_ACTIVE;
		value = s->status;
		break;
	case NEXELL_I2C_TXFLR:
		value = s->txflr;
		break;
	case NEXELL_I2C_RXFLR:
		value = s->rxflr;
		break;
	case NEXELL_I2C_SDA_HOLD:
		value = s->sda_hold;
		break;
	case NEXELL_I2C_TX_ABRT_SOURCE:
		value = s->tx_abrt_source;
		break;
	case NEXELL_I2C_ENABLE_STATUS:
		value = s->enable_status;
		break;
	case NEXELL_I2C_COMP_VERSION:
		value = s->com_version;
		break;
	default:
		qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad address at offset 0x%"
				HWADDR_PRIx "\n", TYPE_NEXELL_I2C, __func__, offset);
		value = 0;
		break;
	}

	DPRINTF("read %s [0x%" HWADDR_PRIx "] -> 0x%02x\n",
			nexell_i2c_get_regname(offset), offset, value);

	return (uint64_t)value;
}

static void nexell_i2c_write(void *opaque, hwaddr offset,
                          uint64_t value, unsigned size)
{
	NEXELLI2CState *s = NEXELL_I2C(opaque);
	static bool first_int = true;
	static bool first_transfer = true;

	DPRINTF("write %s [0x%" HWADDR_PRIx "] <- 0x%02x\n",
			nexell_i2c_get_regname(offset), offset, (int)value);

	switch (offset) {
	case NEXELL_I2C_TAR: /* master mode */
		value &= BIT_MASK_32;
		s->tar = value & I2C_TAR_MASK;
		break;
	case NEXELL_I2C_SAR: /* slave mode */
		s->sar = value & I2C_SAR_MASK;
		break;
	case NEXELL_I2C_CON:
		value &= BIT_MASK_32;
		if (nexell_i2c_is_enabled(s)) {
			uint16_t tar = s->tar;
			uint16_t sar = s->sar;

			if (s->tar != TARGET_ADDR_RESET) {
				i2c_end_transfer(s->bus);
				nexell_i2c_reset(DEVICE(s));
			}
			s->tar = tar;
			s->sar = sar;
		} else {
			s->con = value;

			if (nexell_i2c_is_master(s)) {
				s->status |= I2C_STATUS_ACTIVE;

			} else {
				s->status &= ~I2C_STATUS_ACTIVE;

				if (s->tar != TARGET_ADDR_RESET) {
					i2c_end_transfer(s->bus);
					s->tar = TARGET_ADDR_RESET;
				}
			}

			if (s->con & I2C_RESTART_ENABLE) {
				if (s->tar != TARGET_ADDR_RESET) {
					s->con &= ~I2C_RESTART_ENABLE;
				}
			}
		}
		break;
	case NEXELL_I2C_DATA_CMD:
		s->i2dr_write = (uint16_t)value;

		if (!nexell_i2c_is_enabled(s)) {
			break;
		}
		if (nexell_i2c_is_master(s)) {
			if(s->i2dr_write & DATA_CMD_R && first_transfer) {
				first_transfer = false;
				s->i2dr_write = s->tar << 1;
				if (i2c_start_transfer(s->bus, extract32(s->i2dr_write, 1, 7),
							extract32(s->i2dr_write, 0, 1))) {
					s->status &= ~I2C_SLAVE_ADDR_INVALID;
					i2c_nack(s->bus);
					qemu_irq_lower(s->irq);
					i2c_end_transfer(s->bus);
					nexell_i2c_reset(DEVICE(s));
				} else {
					s->status |= I2C_SLAVE_ADDR_INVALID;
					qemu_log("i2c_start_transfer success - ack is generated\n");
				}
			} else if(!first_transfer && s->i2dr_write & DATA_CMD_R) {
				s->i2dr_write &= ~DATA_CMD_R;
				if (i2c_start_transfer(s->bus, extract32(s->i2dr_write, 1, 7),
							extract32(s->i2dr_write, 0, 1))) {
				} else {
					qemu_log("[I2C] ack\n");
				}
			} else {
				if (i2c_send(s->bus, s->i2dr_write)) {
					s->status &= ~I2C_SLAVE_ADDR_INVALID;
					s->tar = TARGET_ADDR_RESET;
					i2c_end_transfer(s->bus);
				} else {
					s->intr_stat |= INTR_STOP_DET;
					s->status |= I2C_SLAVE_ADDR_INVALID;

					if(s->intr_stat & INTR_STOP_DET) {
						nexell_i2c_raise_interrupt(s);
						i2c_end_transfer(s->bus);
					}
				}
			}
		} else {
			qemu_log_mask(LOG_UNIMP, "[%s]%s: slave mode not implemented\n",
					TYPE_NEXELL_I2C, __func__);
		}
		break;
	case NEXELL_I2C_SS_SCL_HCNT:
		value &= BIT_MASK_16;
		s->ss_scl_hcnt = value;
		break;
	case NEXELL_I2C_SS_SCL_LCNT:
		value  &= BIT_MASK_16;
		s->ss_scl_lcnt = value;
		break;
	case NEXELL_I2C_FS_SCL_HCNT:
		value &= BIT_MASK_16;
		s->fs_scl_hcnt = value;
		break;
	case NEXELL_I2C_FS_SCL_LCNT:
		value &= BIT_MASK_16;
		s->fs_scl_hcnt = value;
		break;
	case NEXELL_I2C_INTR_MASK:
		value &= BIT_MASK_32;
		s->intr_mask = value;

		if (value == 0)
			break;
		if(first_int) {
			first_int = false;
			s->raw_intr_stat |= INTR_START_DET;
			s->intr_stat |= INTR_START_DET;
			s->intr_stat |= INTR_TX_EMPTY;
			s->raw_intr_stat |= INTR_TX_EMPTY;
			/*
			s->intr_stat |= INTR_RX_FULL;
			s->raw_intr_stat |= INTR_RX_FULL;
			*/
			if ( s->intr_stat & INTR_TX_EMPTY) {
				nexell_i2c_raise_interrupt(s);
				s->intr_stat |= INTR_STOP_DET;
			} else if (s->intr_stat & INTR_RX_FULL){
				nexell_i2c_raise_interrupt(s);
			} else {
				s->intr_stat |= INTR_STOP_DET;
			}
		}
		else {
			if(s->tar == TARGET_ADDR_RESET)
				break;

			if(s->i2dr_write & DATA_CMD_RESTART_ENABLE) {
				s->intr_stat |= INTR_STOP_DET;

				s->i2dr_write = DATA_RESET;
				s->intr_stat &= ~INTR_TX_EMPTY;
			} else if(s->tar != TARGET_ADDR_RESET) {
				s->raw_intr_stat |= INTR_START_DET;
				s->intr_stat |= INTR_START_DET;
				s->intr_stat |= INTR_TX_EMPTY;
				s->raw_intr_stat |= INTR_TX_EMPTY;
				nexell_i2c_raise_interrupt(s);
			}
		}
		break;
	case NEXELL_I2C_RX_TL:
		value &= BIT_MASK_32;
		s->rx_tl = value;
		break;
	case NEXELL_I2C_TX_TL:
		value &= BIT_MASK_32;
		s->tx_tl = value;
		break;
	case NEXELL_I2C_SDA_HOLD:
		value &= BIT_MASK_32;
		s->sda_hold = value;
		break;
	case NEXELL_I2C_ENABLE:
		if (value & I2C_ENABLE) {
			s->enable |= value;
		}
		else
			s->enable = value;
		break;
	case NEXELL_I2C_STATUS:
		s->status = ((s->status & I2C_STATUS_ACTIVE) | (value & ~I2C_STATUS_ACTIVE));

		if (!nexell_i2c_is_master(s)) {
			break;
		}

		break;
	default:
		qemu_log_mask(LOG_GUEST_ERROR, "[%s]%s: Bad address at offset 0x%"
                      HWADDR_PRIx "\n", TYPE_NEXELL_I2C, __func__, offset);
		break;
	}
}

static const MemoryRegionOps nexell_i2c_ops = {
	.read = nexell_i2c_read,
	.write = nexell_i2c_write,
	.valid.min_access_size = 1,
	.valid.max_access_size = 2,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription nexell_i2c_vmstate = {
	.name = TYPE_NEXELL_I2C,
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_UINT32(con, NEXELLI2CState),
		VMSTATE_UINT32(tar, NEXELLI2CState),
		VMSTATE_UINT32(sar, NEXELLI2CState),
		VMSTATE_UINT32(hs_maddr, NEXELLI2CState),
		VMSTATE_UINT32(data_cmd, NEXELLI2CState),
		VMSTATE_UINT32(ss_scl_hcnt, NEXELLI2CState),
		VMSTATE_UINT32(ss_scl_lcnt, NEXELLI2CState),
		VMSTATE_UINT32(fs_scl_hcnt, NEXELLI2CState),
		VMSTATE_UINT32(fs_scl_lcnt, NEXELLI2CState),
		VMSTATE_UINT32(hs_scl_hcnt, NEXELLI2CState),
		VMSTATE_UINT32(hs_scl_lcnt, NEXELLI2CState),
		VMSTATE_UINT32(intr_stat, NEXELLI2CState),
		VMSTATE_UINT32(intr_mask, NEXELLI2CState),
		VMSTATE_UINT32(raw_intr_stat, NEXELLI2CState),
		VMSTATE_UINT32(rx_tl, NEXELLI2CState),
		VMSTATE_UINT32(tx_tl, NEXELLI2CState),
		VMSTATE_UINT32(clr_intr, NEXELLI2CState),
		VMSTATE_UINT32(clr_rx_under, NEXELLI2CState),
		VMSTATE_UINT32(clr_rx_over, NEXELLI2CState),
		VMSTATE_UINT32(clr_tx_over, NEXELLI2CState),
		VMSTATE_UINT32(clr_tx_empty, NEXELLI2CState),
		VMSTATE_UINT32(clr_rd_req, NEXELLI2CState),
		VMSTATE_UINT32(clr_tx_abrt, NEXELLI2CState),
		VMSTATE_UINT32(clr_rx_done, NEXELLI2CState),
		VMSTATE_UINT32(clr_activity, NEXELLI2CState),
		VMSTATE_UINT32(clr_start_det, NEXELLI2CState),
		VMSTATE_UINT32(clr_stop_det, NEXELLI2CState),
		VMSTATE_UINT32(clr_gen_call, NEXELLI2CState),
		VMSTATE_UINT32(enable, NEXELLI2CState),
		VMSTATE_UINT32(status, NEXELLI2CState),
		VMSTATE_UINT32(txflr, NEXELLI2CState),
		VMSTATE_UINT32(rxflr, NEXELLI2CState),
		VMSTATE_UINT32(sda_hold, NEXELLI2CState),
		VMSTATE_UINT32(slv_data_nack_only, NEXELLI2CState),
		VMSTATE_UINT32(slv_dma_cr, NEXELLI2CState),
		VMSTATE_UINT32(dma_tdlr, NEXELLI2CState),
		VMSTATE_UINT32(dma_rdlr, NEXELLI2CState),
		VMSTATE_UINT32(sda_setup, NEXELLI2CState),
		VMSTATE_UINT32(ack_general_call, NEXELLI2CState),
		VMSTATE_UINT32(com_param1, NEXELLI2CState),
		VMSTATE_UINT32(com_type, NEXELLI2CState),
		VMSTATE_END_OF_LIST()
	}
};

static void nexell_i2c_realize(DeviceState *dev, Error **errp)
{
	NEXELLI2CState *s = NEXELL_I2C(dev);

	memory_region_init_io(&s->iomem, OBJECT(s), &nexell_i2c_ops, s,
			TYPE_NEXELL_I2C, NEXELL_I2C_MEM_SIZE);
	sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
	s->bus = i2c_init_bus(DEVICE(dev), NULL);
	nexell_i2c_reset(dev);
}

static void nexell_i2c_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->vmsd = &nexell_i2c_vmstate;
	dc->reset = nexell_i2c_reset;
	dc->realize = nexell_i2c_realize;
	dc->desc = "NEXELL I2C Controller";
}

static const TypeInfo nexell_i2c_type_info = {
	.name = TYPE_NEXELL_I2C,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(NEXELLI2CState),
	.class_init = nexell_i2c_class_init,
};

static void nexell_i2c_register_types(void)
{
	type_register_static(&nexell_i2c_type_info);
}

type_init(nexell_i2c_register_types)
