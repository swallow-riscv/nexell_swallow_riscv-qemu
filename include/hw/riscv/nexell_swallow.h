/*
 * QEMU RISC-V Nexell Swallow machine interface
 *
 * Copyright (c) 2018 Nexell, Inc.
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

#ifndef HW_NEXELL_SWALLOW_H
#define HW_NEXELL_SWALLOW_H

#include "hw/gpio/nxp3220_gpio.h"

#define TYPE_NEXELL_SWALLOW_BOARD "riscv.nexell_swallow"
#define NEXELL_SWALLOW(obj) \
    OBJECT_CHECK(NexellSwallowState, (obj), TYPE_NEXELL_SWALLOW_BOARD)

enum SwallowConfiguration {
	SWALLOW_NUM_GPIOS = 8,
};

enum SwallowMemoryMap {
	SWALLOW_GPIO0_ADDR = 0x20700000,
	SWALLOW_GPIO1_ADDR = 0x20710000,
	SWALLOW_GPIO2_ADDR = 0x20720000,
	SWALLOW_GPIO3_ADDR = 0x20730000,
	SWALLOW_GPIO4_ADDR = 0x20740000,
	SWALLOW_GPIO5_ADDR = 0x20750000,
	SWALLOW_GPIO6_ADDR = 0x20760000,
	SWALLOW_GPIO7_ADDR = 0x20770000,
};

typedef struct {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    RISCVHartArrayState soc;
    DeviceState *plic;
    void *fdt;
    int fdt_size;

    NXP3220GpioState gpio[SWALLOW_NUM_GPIOS];
} NexellSwallowState;

enum {
    NEXELL_SWALLOW_DEBUG,
    NEXELL_SWALLOW_MROM,
    NEXELL_SWALLOW_TEST,
    NEXELL_SWALLOW_CLINT,
    NEXELL_SWALLOW_PLIC,
    NEXELL_SWALLOW_UART0,
    NEXELL_SWALLOW_UART1,
    NEXELL_SWALLOW_UART2,
    NEXELL_SWALLOW_UART3,
    NEXELL_SWALLOW_UART4,
    NEXELL_SWALLOW_UART5,
    NEXELL_SWALLOW_SRAM,
    NEXELL_SWALLOW_DRAM
};

enum {
    UART0_IRQ = 56,
    UART1_IRQ = 57,
    UART2_IRQ = 58,
    UART3_IRQ = 59,
    UART4_IRQ = 60,
    UART5_IRQ = 61,
    IRQ_NDEV = 89,
};

enum {
    NEXELL_SWALLOW_CLOCK_FREQ = 1000000000
};

#define NEXELL_SWALLOW_PLIC_HART_CONFIG "MS"
#define NEXELL_SWALLOW_PLIC_NUM_SOURCES 127
#define NEXELL_SWALLOW_PLIC_NUM_PRIORITIES 7
#define NEXELL_SWALLOW_PLIC_PRIORITY_BASE 0x0
#define NEXELL_SWALLOW_PLIC_PENDING_BASE 0x1000
#define NEXELL_SWALLOW_PLIC_ENABLE_BASE 0x2000
#define NEXELL_SWALLOW_PLIC_ENABLE_STRIDE 0x80
#define NEXELL_SWALLOW_PLIC_CONTEXT_BASE 0x200000
#define NEXELL_SWALLOW_PLIC_CONTEXT_STRIDE 0x1000

#if defined(TARGET_RISCV32)
#define NEXELL_SWALLOW_CPU TYPE_RISCV_CPU_NEXELL_SWALLOW32
#elif defined(TARGET_RISCV64)
#define NEXELL_SWALLOW_CPU TYPE_RISCV_CPU_NEXELL_SWALLOW64
#endif

#endif
