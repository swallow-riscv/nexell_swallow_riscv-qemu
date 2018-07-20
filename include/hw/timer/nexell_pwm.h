/*
 * Nexell Swallow processors PWM registers definition.
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
#include "qemu/log.h"
#include "hw/sysbus.h"
#include "qemu/timer.h"
#include "qemu-common.h"
#include "qemu/main-loop.h"
#include "hw/ptimer.h"

#ifdef DEBUG_PWM
#define     DPRINTF(fmt, ...) \
            do { fprintf(stdout, "PWM: [%24s:%5d] " fmt, __func__, __LINE__, \
                ## __VA_ARGS__); } while (0)
#else
#define     DPRINTF(fmt, ...) do {} while (0)
#endif

#define     NEXELL_PWM_TIMERS_NUM      4
#define     NEXELL_PWM_REG_MEM_SIZE    0x10000

#define     T0CFG0       0x0000
#define     T0CFG1       0x0004
#define     TCON0        0x0008
#define     TCNTB0       0x000C
#define     TCMPB0       0x0010
#define     TCNTO0       0x0014
#define     TINT_CSTAT0  0x0018

#define     T1CFG0       0x0100
#define     T1CFG1       0x0104
#define     TCON1        0x0108
#define     TCNTB1       0x010C
#define     TCMPB1       0x0110
#define     TCNTO1       0x0114
#define     TINT_CSTAT1  0x0118

#define     T2CFG0       0x0200
#define     T2CFG1       0x0204
#define     TCON2        0x0208
#define     TCNTB2       0x020C
#define     TCMPB2       0x0210
#define     TCNTO2       0x0214
#define     TINT_CSTAT2  0x0218

#define     T3CFG0       0x0300
#define     T3CFG1       0x0304
#define     TCON3        0x0308
#define     TCNTB3       0x030C
#define     TCMPB3       0x0310
#define     TCNTO3       0x0314
#define     TINT_CSTAT3  0x0318

#define     GET_PRESCALER(reg) ((reg & 0xff) + 1)
#define     GET_DIVIDER(reg) ((reg & 0x7) + 1)

#define     TCON_TIMER_START            (1 << 0)
#define     TCON_TIMER_MANUAL_UPD       (1 << 1)
#define     TCON_TIMER_OUTPUT_INV       (1 << 2)
#define     TCON_TIMER_AUTO_RELOAD      (1 << 3)

#define     TINT_CSTAT_STATUS           (1 << 5)
#define     TINT_CSTAT_ENABLE           (1 << 0)

/* timer struct */
typedef struct {
	uint32_t    id;             /* timer id */
	qemu_irq    irq;            /* local timer irq */
	uint32_t    freq;           /* timer frequency */

	/* use ptimer.c to represent count down timer */
	ptimer_state *ptimer;       /* timer  */

	/* registers */
	uint32_t    reg_tcfg[2];
	uint32_t    reg_tcon;
	uint32_t    reg_tcntb;      /* counter register buffer */
	uint32_t    reg_tcmpb;      /* compare register buffer */
	uint32_t    reg_tint_cstat;

	struct NexellPWMState *parent;

} NexellPWM;

#define TYPE_NEXELL_PWM "nexell.pwm"
#define NEXELL_PWM(obj) \
    OBJECT_CHECK(NexellPWMState, (obj), TYPE_NEXELL_PWM)

typedef struct NexellPWMState {
	SysBusDevice parent_obj;

	MemoryRegion iomem;

	NexellPWM timer[NEXELL_PWM_TIMERS_NUM];

} NexellPWMState;
