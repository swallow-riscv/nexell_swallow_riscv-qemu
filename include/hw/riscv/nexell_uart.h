/*
 * QEMU Designware UART emulation
 *
 * Copyright (c) 2018 Nexell, Inc.
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

#ifndef NEXELL_UART_H
#define NEXELL_UART_H

#include "hw/hw.h"
#include "sysemu/sysemu.h"
#include "chardev/char-fe.h"
#include "exec/memory.h"
#include "qemu/fifo8.h"
#include "chardev/char.h"

#define UART_FIFO_LENGTH    16      /* Designware Fifo Length */

typedef struct NexellUARTState {
    uint16_t divider;

    /* need to check register size */
    uint8_t rbr; /* receive register */
    /* need to check register size */
    uint8_t thr; /* transmit holding register */
    uint8_t dll; /* */
    uint8_t tsr; /* */
    uint8_t ier; /* */
    uint8_t dlh; /* */
    uint8_t iir; /* */
    uint8_t fcr; /* */
    uint8_t lcr;
    uint8_t mcr;
    /* need to check register size */
    uint8_t lsr; /* read only */
    uint8_t msr; /* read only */
    uint8_t scr;
    uint8_t lpdll;
    uint8_t lpdlh;
    uint32_t srbr[16];
    uint32_t sthr[16];
    uint8_t far;
    uint8_t tfr;
    uint32_t rfw;
    uint8_t usr;
    uint32_t tfl;
    uint32_t rfl;
    uint8_t srr;
    uint8_t srts;
    uint8_t sbcb;
    uint8_t sdmam;
    uint8_t sfe;
    uint8_t srt;
    uint8_t stet;
    uint8_t htx;
    uint8_t dmasa;
    uint8_t tcr;
    uint8_t doer;
    uint8_t roer;
    uint32_t doetr;
    uint32_t tatr;
    uint8_t dlf;
    uint8_t rar;
    uint8_t tar;
    uint8_t lecr;
    uint32_t cpr;
    uint32_t ucv;
    uint32_t ctr;

    uint8_t fcr_vmstate; /* we can't write directly this value
                            it has side effects */
    /* NOTE: this hidden state is necessary for tx irq generation as
       it can be reset while reading iir */
    int thr_ipending;
    qemu_irq irq;
    CharBackend chr;
    int last_break_enable;
    int it_shift;
    int baudbase;
    uint32_t tsr_retry;
    guint watch_tag;
    uint32_t wakeup;

    /* Time when the last byte was successfully sent out of the tsr */
    uint64_t last_xmit_ts;
    Fifo8 recv_fifo;
    Fifo8 xmit_fifo;
    /* Interrupt trigger level for recv_fifo */
    uint8_t recv_fifo_itl;

    QEMUTimer *fifo_timeout_timer;
    int timeout_ipending;           /* timeout interrupt pending state */

    uint64_t char_transmit_time;    /* time to transmit a char in ticks */
    int poll_msl;

    QEMUTimer *modem_status_poll;
    MemoryRegion io;
} NexellUARTState;

extern const VMStateDescription vmstate_nexell_uart;
extern const MemoryRegionOps nexell_uart_io_ops;

void nexell_uart_realize_core(NexellUARTState *s, Error **errp);
void nexell_uart_exit_core(NexellUARTState *s);
void nexell_uart_set_frequency(NexellUARTState *s, uint32_t frequency);

/* legacy pre qom */
NexellUARTState *nexell_uart_init(int base, qemu_irq irq, int baudbase,
                         Chardev *chr, MemoryRegion *system_io);
NexellUARTState *nexell_uart_mm_init(MemoryRegion *address_space,
                            hwaddr base, int it_shift,
                            qemu_irq irq, int baudbase,
                            Chardev *chr, enum device_endian end);

#endif
