/*
 * Copyright (c) 2018 ChoonHyun Jeon <suker@nexell.co.kr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef HW_SWALLOW_SPI_H
#define HW_SWALLOW_SPI_H

#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qemu/fifo32.h"

/* Register offsets */
#define DW_SPI_CTRL0			0x00
#define DW_SPI_CTRL1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFLTR			0x18
#define DW_SPI_RXFLTR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60

enum {
    e_DW_SPI_CTRL0 = 0,
    e_DW_SPI_CTRL1,
    e_DW_SPI_SSIENR,
    e_DW_SPI_MWCR,
    e_DW_SPI_SER,
    e_DW_SPI_BAUDR,
    e_DW_SPI_TXFLTR,
    e_DW_SPI_RXFLTR,
    e_DW_SPI_TXFLR,
    e_DW_SPI_RXFLR,
    e_DW_SPI_SR,
    e_DW_SPI_IMR,
    e_DW_SPI_ISR,
    e_DW_SPI_RISR,
    e_DW_SPI_TXOICR,
    e_DW_SPI_RXOICR,
    e_DW_SPI_RXUICR,
    e_DW_SPI_MSTICR,
    e_DW_SPI_ICR,
    e_DW_SPI_DMACR,
    e_DW_SPI_DMATDLR,
    e_DW_SPI_DMARDLR,
    e_DW_SPI_IDR,
    e_DW_SPI_VERSION,
    e_DW_SPI_DR,
};

#define DW_SPI_MAX                      32

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET			0

#define SPI_FF_OFFSET			21
#define SPI_FF_MASK			(0x3 << SPI_FF_OFFSET)

#define SPI_FRF_OFFSET			4
#define SPI_FRF_SPI			0x0
#define SPI_FRF_SSP			0x1
#define SPI_FRF_MICROWIRE		0x2
#define SPI_FRF_RESV			0x3

#define SPI_MODE_OFFSET			6
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7

#define SPI_TMOD_OFFSET			8
#define SPI_TMOD_MASK			(0x3 << SPI_TMOD_OFFSET)
#define	SPI_TMOD_TR			0x0		/* xmit & recv */
#define SPI_TMOD_TO			0x1		/* xmit only */
#define SPI_TMOD_RO			0x2		/* recv only */
#define SPI_TMOD_EPROMREAD		0x3		/* eeprom read mode */

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12

/* Bit fields in SR, 7 bits */
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT			(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)

/* Bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI			(1 << 0)
#define SPI_INT_TXOI			(1 << 1)
#define SPI_INT_RXUI			(1 << 2)
#define SPI_INT_RXOI			(1 << 3)
#define SPI_INT_RXFI			(1 << 4)
#define SPI_INT_MSTI			(1 << 5)

/* Bit fields in DMACR */
#define SPI_DMA_RDMAE			(1 << 0)
#define SPI_DMA_TDMAE			(1 << 1)

/* TX RX interrupt level threshold, max can be 256 */
#define SPI_INT_THRESHOLD		32

typedef enum {
    TXFIFO_EMPTY            = 0,
    TXFIFO_OVER             = 1,
    RXFIFO_UNDER            = 2,
    RXFIFO_OVER             = 3,
    RXFIFO_FULL             = 4,
    MULTI_MAST_CONTEN       = 5
} nx_spi_intr_mask;

#define TYPE_SWALLOW_SPI   "swallow-spi"
#define SWALLOW_SPI(obj)   OBJECT_CHECK(SWALLOWSpiState, (obj), TYPE_SWALLOW_SPI)

#define R_SPI_MAX             16

typedef struct SWALLOWSpiState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;

    qemu_irq irq;

    qemu_irq cs_line;

    SSIBus *spi;

    Fifo32 rx_fifo;
    Fifo32 tx_fifo;

    int fifo_depth;
    uint32_t frame_count;
    bool enabled;

    uint32_t regs[R_SPI_MAX];
} SWALLOWSpiState;

#endif /* HW_SWALLOW_SPI_H */
