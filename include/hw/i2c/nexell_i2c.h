/*
 * NEXELL I2C Bus Serial Interface registers definition
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

#ifndef NEXELL_I2C_H
#define NEXELL_I2C_H

#include "hw/sysbus.h"

#define TYPE_NEXELL_I2C "nexell.i2c"
#define NEXELL_I2C(obj) OBJECT_CHECK(NEXELLI2CState, (obj), TYPE_NEXELL_I2C)

#define NEXELL_I2C_MEM_SIZE		0x10000
#define BIT_MASK_8			0xFF
#define BIT_MASK_16			0xFFFF
#define BIT_MASK_32			0xFFFFFFFF

#define I2C_MASTER_ENABLE		0x1
#define I2C_ENABLE			0x1
#define I2C_STATUS_ACTIVE		0x1
#define I2C_INT_ENABLE			0
#define I2C_RESTART_ENABLE		(1 << 5)
#define I2C_TAR_MASK			0x3F
#define I2C_SAR_MASK			0x3F
#define I2C_CON_MASK			0x63
#define I2C_COMP_PARAM1_SPEED_MASK	(1 <<2) | (1 << 3)
#define I2C_DATA_MASK			0xFF
#define I2C_SLAVE_ADDR_INVALID		(1 << 17)
#define I2C_TX_BUFFER_DEPTH_MASK	0x70000		/* 8 */
#define I2C_RX_BUFFER_DEPTH_MASK	0x700		/* 8 */
#define INTR_RX_UNDER			(1 << 0)
#define INTR_RX_OVER			(1 << 1)
#define INTR_RX_FULL			(1 << 2)
#define INTR_TX_OVER			(1 << 3)
#define INTR_TX_EMPTY			(1 << 4)
#define INTR_RD_REQ			(1 << 5)
#define INTR_TX_ABRT			(1 << 6)
#define INTR_RX_DONE			(1 << 7)
#define INTR_ACTIVITY			(1 << 8)
#define INTR_STOP_DET			(1 << 9)
#define INTR_START_DET			(1 << 10)
#define INTR_GEN_CALL			(1 << 11)
#define INTR_MASK_DISABLE		0
#define DATA_CMD_RESTART_ENABLE		(1 << 9)
#define DATA_CMD_R			0x1

#define TARGET_ADDR_RESET		0x1055
#define CON_RESET			0x7D
#define SLAVE_ADDR_RESET		0x55
#define ENABLE_RESET			0
#define TXFLR_RESET			0
#define RXFLR_RESET			0
#define ENABLE_STATUS_RESET		0
#define SS_SCL_HCNT_RESET		0x28
#define SS_SCL_LCNT_RESET		0x28
#define FS_SCL_HCNT_RESET		0x6
#define FS_SCL_LCNT_RESET		0xD
#define INTR_STAT_RESET			0
#define INT_MASK_RESET			0x8FF
#define RAW_INTR_STATUS_RESET		0
#define RX_TL_RESET			0
#define TX_TL_RESET			0
#define CLR_INTR_RESET			0
#define CLR_RX_UNDER_RESET		0
#define CLR_RX_OVER_RESET		0
#define CLR_TX_OVER_RESET		0
#define CLR_RD_REQ_RESET		0
#define CLR_TX_ABRT_RESET		0
#define CLR_RX_DONE_RESET		0
#define CLR_ACTIVITY_RESET		0
#define CLR_STOP_DET_RESET		0
#define CLR_START_DER_RESET		0
#define CLR_GEN_CALL_RESET		0
#define SDA_HOLD_RESET			0x1
#define COM_PARAM1_RESET		0
#define COM_VERSION_RESET		0x3131312A
#define COM_TYPE_RESET			0x44570140
#define STATUS_RESET			0x06
#define TX_ABRT_SOURCE_RESET		0
#define	DATA_RESET			0

enum {
	NEXELL_I2C0,
	NEXELL_I2C1,
	NEXELL_I2C2,
	NEXELL_I2C3,
	NEXELL_I2C4,
	NEXELL_I2C5,
	NEXELL_I2C6,
	NEXELL_I2C7,
	NEXELL_I2C8,
	NEXELL_I2C9,
	NEXELL_I2C10,
	NEXELL_I2C11,
};


/* NEXELL I2C memory map */

/* offset */
enum {
	NEXELL_I2C_CON				= 0,
	NEXELL_I2C_TAR				= 4,
	NEXELL_I2C_SAR				= 8,
	NEXELL_I2C_HS_MADDR			= 12,
	NEXELL_I2C_DATA_CMD			= 16,
	NEXELL_I2C_SS_SCL_HCNT			= 20,
	NEXELL_I2C_SS_SCL_LCNT			= 24,
	NEXELL_I2C_FS_SCL_HCNT			= 28,
	NEXELL_I2C_FS_SCL_LCNT			= 32,
	NEXELL_I2C_HS_SCL_HCNT			= 36,
	NEXELL_I2C_HS_SCL_LCT			= 40,
	NEXELL_I2C_INTR_STAT			= 44,
	NEXELL_I2C_INTR_MASK			= 48,
	NEXELL_I2C_RAW_INTR_STAT		= 52,
	NEXELL_I2C_RX_TL			= 56,
	NEXELL_I2C_TX_TL			= 60,
	NEXELL_I2C_CLR_INTR			= 64,
	NEXELL_I2C_CLR_RX_UNDER			= 68,
	NEXELL_I2C_CLR_RX_OVER			= 72,
	NEXELL_I2C_CLR_TX_OVER			= 76,
	NEXELL_I2C_CLR_RD_REQ			= 80,
	NEXELL_I2C_CLR_TX_ABRT			= 84,
	NEXELL_I2C_CLR_RX_DONE			= 88,
	NEXELL_I2C_CLR_ACTIVITY			= 92,
	NEXELL_I2C_CLR_STOP_DET			= 96,
	NEXELL_I2C_CLR_START_DET		= 100,
	NEXELL_I2C_CLR_GEN_CALL			= 104,
	NEXELL_I2C_ENABLE			= 108,
	NEXELL_I2C_STATUS			= 112,
	NEXELL_I2C_TXFLR			= 116,
	NEXELL_I2C_RXFLR			= 120,
	NEXELL_I2C_SDA_HOLD			= 124,
	NEXELL_I2C_TX_ABRT_SOURCE		= 128,
	NEXELL_I2C_SLV_DATA_NACK_ONLY		= 132,
	NEXELL_I2C_DMA_CR			= 136,
	NEXELL_I2C_DMA_TDLR			= 140,
	NEXELL_I2C_DMA_RDLR			= 144,
	NEXELL_I2C_SDA_SETUP			= 148,
	NEXELL_I2C_ACK_GENERAL_CALL		= 152,
	NEXELL_I2C_ENABLE_STATUS		= 156,
	NEXELL_I2C_COMP_PARAM_1			= 244,
	NEXELL_I2C_COMP_VERSION			= 248,
	NEXELL_I2C_COMP_TYPE			= 252
};

typedef struct NEXELLI2CState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    I2CBus *bus;
    qemu_irq irq;

    /* register status */
    uint32_t con;
    uint32_t tar;
    uint32_t sar;
    uint32_t hs_maddr;
    uint32_t data_cmd;
    uint32_t ss_scl_hcnt;
    uint32_t ss_scl_lcnt;
    uint32_t fs_scl_hcnt;
    uint32_t fs_scl_lcnt;
    uint32_t hs_scl_hcnt;
    uint32_t hs_scl_lcnt;
    uint32_t intr_stat;
    uint32_t intr_mask;
    uint32_t raw_intr_stat;
    uint32_t rx_tl;
    uint32_t tx_tl;
    uint32_t clr_intr;
    uint32_t clr_rx_under;
    uint32_t clr_rx_over;
    uint32_t clr_tx_over;
    uint32_t clr_tx_empty;
    uint32_t clr_rd_req;
    uint32_t clr_tx_abrt;
    uint32_t clr_rx_done;
    uint32_t clr_activity;
    uint32_t clr_start_det;
    uint32_t clr_stop_det;
    uint32_t clr_gen_call;
    uint32_t enable;
    uint32_t status;
    uint32_t txflr;
    uint32_t rxflr;
    uint32_t tx_abrt_source;
    uint32_t sda_hold;
    uint32_t slv_data_nack_only;
    uint32_t slv_dma_cr;
    uint32_t dma_tdlr;
    uint32_t dma_rdlr;
    uint32_t sda_setup;
    uint32_t ack_general_call;
    uint32_t enable_status;
    uint32_t com_param1;
    uint32_t com_version;
    uint32_t com_type;

    uint16_t i2dr_read;
    uint16_t i2dr_write;
} NEXELLI2CState;

#endif /* NEXELL_I2C_H */
