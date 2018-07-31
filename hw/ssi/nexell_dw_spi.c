/*
 * Copyright (C) 2018 Choonghyun Jeon <suker@nexell.co.kr>
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

#include "qemu/osdep.h"
#include "hw/ssi/nexell_dw_spi.h"
#include "qemu/log.h"

#ifndef SWALLOW_SPI_ERR_DEBUG
#define SWALLOW_SPI_ERR_DEBUG   1
#endif

#define DB_PRINT(fmt, ...) \
    do { printf("[QEMU]: " fmt , ## __VA_ARGS__); \
        printf("\n");                             \
    } while (0)

static void spi_update_irq(SWALLOWSpiState *s)
{
    int irq;

    irq = !!(s->regs[e_DW_SPI_ISR]);

    qemu_set_irq(s->irq, irq);
}

static void swallow_spi_reset(DeviceState *d)
{
    SWALLOWSpiState *s = SWALLOW_SPI(d);

    memset(s->regs, 0, sizeof s->regs);

    s->fifo_depth = 4;
    s->frame_count = 1;
    s->enabled = false;
}

static uint64_t
spi_read(void *opaque, hwaddr addr, unsigned int size)
{
    SWALLOWSpiState *s = opaque;
    uint32_t ret = 0;

    switch (addr) {
    case DW_SPI_IMR:
        ret = 0x3f & s->regs[e_DW_SPI_IMR];
        break;
    case DW_SPI_TXFLTR:
        ret = s->regs[e_DW_SPI_TXFLTR];
    default:
        break;
    }

    /* DB_PRINT("%s, addr=0x%x, ret = 0x%x", __func__, (unsigned int)addr, ret); */
    spi_update_irq(s);
    return ret;
}

/* static uint32_t spi_check_busy(SWALLOWSpiState *s) */
/* { */
/*     uint32_t temp = (s->regs[DW_SPI_SR] >> 0) & 0x1; */
/*     if (temp == 0) */
/*         return 0; //not busy */
/*     else */
/*         return 1; //busy */
/* } */

/* static void spi_set_interrupt_Enable(SWALLOWSpiState *s, nx_spi_intr_mask intNum, bool Enable) */
/* { */
/*     uint32_t irq_mask; */
/*     uint32_t old_irq_mask; */

/*     old_irq_mask = s->regs[DW_SPI_IMR]; */
/*     if (Enable == true) */
/*         irq_mask = old_irq_mask | ( 1 << intNum ); */
/*     else */
/*         irq_mask = old_irq_mask & ~( 1 << intNum ); */

/*     s->regs[DW_SPI_IMR] = irq_mask; */
/* } */

static void spi_set_interrupt_Enable_All(SWALLOWSpiState *s, bool Enable)
{
    uint32_t irq_mask;

    if (Enable == true) {
        irq_mask = 0x3F;
        DB_PRINT("%s: irq mask = 0x3F, int enable all",__func__);
    }
    else {
        irq_mask = 0x00;
        DB_PRINT("%s: irq mask = 0x00, int disable all",__func__);
    }

    s->regs[e_DW_SPI_IMR] = irq_mask;
}

/* static bool spi_get_interrupt_Enable(SWALLOWSpiState *s, nx_spi_intr_mask intNum) */
/* { */
/*     uint32_t irq_mask; */

/*     irq_mask = s->regs[e_DW_SPI_IMR]; */

/*     if ( 0 != (( 1 << intNum ) & irq_mask ) ) */
/*         return false; */
/*     else */
/*         return true; */
/* } */

/* static bool spi_get_interrupt_Pending(SWALLOWSpiState *s, nx_spi_intr_mask intNum) */
/* { */
/*     uint32_t masked_irq_status; */

/*     masked_irq_status = s->regs[e_DW_SPI_RISR]; */

/*     if ( 0 != (( 1 << intNum ) & masked_irq_status ) ) */
/*         return true; */
/*     else */
/*         return false; */
/* } */

/* static void spi_clear_interrupt_Pending(SWALLOWSpiState *s, nx_spi_intr_mask intNum) */
/* { */
/*     uint32_t irq_pending; */

/*     masked_irq_status = s->regs[e_DW_SPI_RISR]; */

/*     if (IntNum == TXFIFO_EMPTY) */
/*         irq_pending = s->regs[e_DW_SPI_ICR] */
/*     if (IntNum == TXFIFO_OVER ) */
/*         irq_pending = s->regs[e_DW_SPI_TXOICR] */
/*     if (IntNum == RXFIFO_UNDER) */
/*         irq_pending = s->regs[e_DW_SPI_RXUICR] */
/*     if (IntNum == RXFIFO_OVER ) */
/*         irq_pending = s->regs[e_DW_SPI_RXOICR] */
/*     if (IntNum == RXFIFO_FULL ) */
/*         irq_pending = s->regs[e_DW_SPI_ICR] */
/*     if (IntNum == MULTI_MAST_CONTEN) */
/*         irq_pending = s->regs[e_DW_SPI_MSTICR] */
/* } */

/* static nx_spi_intr_mask spi_get_interrupt_PendingNumber(SWALLOWSpiState *s) */
/* { */
/*     uint32_t irq_status; */

/*     irq_status = s->regs[e_DW_SPI_ISR]; */

/*     if ((irq_status >> 0) & 0x01) */
/*         return TXFIFO_EMPTY; */
/*     if ((irq_status >> 1) & 0x01) */
/*         return TXFIFO_OVER; */
/*     if ((irq_status >> 2) & 0x01) */
/*         return RXFIFO_UNDER; */
/*     if ((irq_status >> 3) & 0x01) */
/*         return RXFIFO_OVER; */
/*     if ((irq_status >> 4) & 0x01) */
/*         return RXFIFO_FULL; */
/*     if ((irq_status >> 5) & 0x01) */
/*         return MULTI_MAST_CONTEN; */
/*     return -1; */
/* } */

/* static void spi_flush_txfifo(SWALLOWSpiState *s) */
/* { */
/*     uint32_t tx; */
/*     uint32_t rx; */
/* //    bool sps = !!(s->regs[e_DW_SPI_CTRL0] & C_SPS); */

/*     /\* */
/*      * Chip Select(CS) is automatically controlled by this controller. */
/*      * If SPS bit is set in Control register then CS is asserted */
/*      * until all the frames set in frame count of Control register are */
/*      * transferred. If SPS is not set then CS pulses between frames. */
/*      * Note that Slave Select register specifies which of the CS line */
/*      * has to be controlled automatically by controller. Bits SS[7:1] are for */
/*      * masters in FPGA fabric since we model only Microcontroller subsystem */
/*      * of Smartfusion2 we control only one CS(SS[0]) line. */
/*      *\/ */
/*     DB_PRINT("fifo empty = 0x%x",fifo32_is_empty(&s->tx_fifo)); */
/*     DB_PRINT("frame_count = 0x%x",s->frame_count); */
/*     while (!fifo32_is_empty(&s->tx_fifo) && s->frame_count) { */
/*         assert_cs(s); */

/* //        s->regs[R_SPI_STATUS] &= ~(S_TXDONE | S_RXRDY); */

/*         tx = fifo32_pop(&s->tx_fifo); */
/*         DB_PRINT("data tx:0x%x",tx); */
/*         rx = ssi_transfer(s->spi, tx); */
/*         DB_PRINT("data rx:0x%x",rx); */

/*         /\* if (fifo32_num_used(&s->rx_fifo) == s->fifo_depth) { *\/ */
/*         /\*     s->regs[R_SPI_STATUS] |= S_RXCHOVRF; *\/ */
/*         /\*     s->regs[R_SPI_RIS] |= S_RXCHOVRF; *\/ */
/*         /\* } else { *\/ */
/*         /\*     fifo32_push(&s->rx_fifo, rx); *\/ */
/*         /\*     s->regs[R_SPI_STATUS] &= ~S_RXFIFOEMP; *\/ */
/*         /\*     if (fifo32_num_used(&s->rx_fifo) == (s->fifo_depth - 1)) { *\/ */
/*         /\*         s->regs[R_SPI_STATUS] |= S_RXFIFOFULNXT; *\/ */
/*         /\*     } else if (fifo32_num_used(&s->rx_fifo) == s->fifo_depth) { *\/ */
/*         /\*         s->regs[R_SPI_STATUS] |= S_RXFIFOFUL; *\/ */
/*         /\*     } *\/ */
/*         /\* } *\/ */
/*         s->frame_count--; */
/*         /\* if (!sps) { *\/ */
/*         /\*     deassert_cs(s); *\/ */
/*         /\* } *\/ */
/*     } */

/*    DB_PRINT("\n"); */
/* } */

static void spi_write(void *opaque, hwaddr addr,
            uint64_t val64, unsigned int size)
{
    SWALLOWSpiState *s = opaque;
    uint32_t value = val64;
    uint32_t TMOD = 0;
    uint32_t tempVal = 0;

    /* DB_PRINT("%s: addr=0x%x, value=0x%x", __func__, (unsigned int)addr, value); */

    switch (addr) {
    case DW_SPI_CTRL0:
        TMOD = value;
        if (TMOD == SPI_TMOD_TR)
            DB_PRINT("TMOD : Transmit and Receive");
        else if (TMOD == SPI_TMOD_TO)
            DB_PRINT("TMOD : Transmit Only");
        else if (TMOD == SPI_TMOD_RO)
            DB_PRINT("TMOD : Receive Only");
        else if (TMOD == SPI_TMOD_EPROMREAD)
            DB_PRINT("TMOD : EEPROM Read");
        else
            DB_PRINT("TMOD : Error Invalid value!!");
        break;
    case DW_SPI_TXFLTR:
        s->regs[e_DW_SPI_TXFLTR]=value;
        break;

    case DW_SPI_SSIENR:
        s->regs[e_DW_SPI_SSIENR] = value;
        s->fifo_depth = 32;
        s->enabled = value;
        s->frame_count = 1;
        DB_PRINT("%s: DW_apb_ssi %s, val = %d",__func__,value ? "Enable" : "Disable", value);
        break;

    case DW_SPI_IMR:
        tempVal = 0x3f & value;
        if (tempVal == 0x3f)
            spi_set_interrupt_Enable_All(s, 1);
        else if (tempVal == 0x00)
            spi_set_interrupt_Enable_All(s, 0);
        else {
            s->regs[e_DW_SPI_IMR] = tempVal;
            if ((SPI_INT_MSTI & tempVal) == SPI_INT_MSTI)
                DB_PRINT("DW_SPI_IMR -> SPI_INT_MSTI");
            if ((SPI_INT_RXFI & tempVal) == SPI_INT_RXFI)
                DB_PRINT("DW_SPI_IMR -> SPI_INT_RXFI");
            if ((SPI_INT_RXOI & tempVal) == SPI_INT_RXOI)
                DB_PRINT("DW_SPI_IMR -> SPI_INT_RXOI");
            if ((SPI_INT_RXUI & tempVal) == SPI_INT_RXUI)
                DB_PRINT("DW_SPI_IMR -> SPI_INT_RXUI");
        }
        break;

    default:
        break;
    }

    spi_update_irq(s);
}

static const MemoryRegionOps spi_ops = {
    .read = spi_read,
    .write = spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4
    }
};

static void swallow_spi_realize(DeviceState *dev, Error **errp)
{
    SWALLOWSpiState *s = SWALLOW_SPI(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    s->spi = ssi_create_bus(dev, "spi");

    sysbus_init_irq(sbd, &s->irq);
    ssi_auto_connect_slaves(dev, &s->cs_line, s->spi);
    sysbus_init_irq(sbd, &s->cs_line);

    memory_region_init_io(&s->mmio, OBJECT(s), &spi_ops, s,
                          TYPE_SWALLOW_SPI, 64);
    sysbus_init_mmio(sbd, &s->mmio);

    fifo32_create(&s->tx_fifo, DW_SPI_MAX*2);
    fifo32_create(&s->rx_fifo, DW_SPI_MAX*2);
}

static const VMStateDescription vmstate_swallow_spi = {
    .name = TYPE_SWALLOW_SPI,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_FIFO32(tx_fifo, SWALLOWSpiState),
        VMSTATE_FIFO32(rx_fifo, SWALLOWSpiState),
        VMSTATE_UINT32_ARRAY(regs, SWALLOWSpiState, 16),
        VMSTATE_END_OF_LIST()
    }
};

static void swallow_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = swallow_spi_realize;
    dc->reset = swallow_spi_reset;
    dc->vmsd = &vmstate_swallow_spi;
}

static const TypeInfo swallow_spi_info = {
    .name           = TYPE_SWALLOW_SPI,
    .parent         = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(SWALLOWSpiState),
    .class_init     = swallow_spi_class_init,
};

static void swallow_spi_register_types(void)
{
    type_register_static(&swallow_spi_info);
}

type_init(swallow_spi_register_types)
