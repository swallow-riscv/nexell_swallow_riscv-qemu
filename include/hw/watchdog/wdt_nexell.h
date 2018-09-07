/*
 * NEXELL Watchdog Controller
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

#ifndef NEXELL_WDT_H
#define NEXELL_WDT_H

#include "hw/sysbus.h"

#define TYPE_NEXELL_WDT "nexell.wdt"
#define NEXELL_WDT(obj) \
    OBJECT_CHECK(NexellWDTState, (obj), TYPE_NEXELL_WDT)

#define NEXELL_WDT_MEM_SIZE		0x10000

typedef struct NexellWDTState {
    /*< private >*/
    SysBusDevice parent_obj;
    QEMUTimer *timer;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t wdt_con;
    uint32_t wdt_dat;
    uint32_t wdt_cnt;
    uint32_t wdt_clrint;

    qemu_irq irq;

    uint64_t pclk_freq;
} NexellWDTState;

#endif  /* NEXELL_WDT_H */
