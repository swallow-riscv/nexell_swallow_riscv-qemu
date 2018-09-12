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
#include "hw/i2c/nexell_i2c.h"
#include "hw/watchdog/wdt_nexell.h"

#define TYPE_NEXELL_SWALLOW_BOARD "riscv.nexell_swallow"
#define NEXELL_SWALLOW(obj) \
    OBJECT_CHECK(NexellSwallowState, (obj), TYPE_NEXELL_SWALLOW_BOARD)

enum SwallowConfiguration {
	SWALLOW_NUM_GPIOS = 8,
	SWALLOW_NUM_I2C = 12,
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
    NEXELLADCState adc0;
    NexellPWMState pwm0;
    NexellPWMState pwm1;
    NexellPWMState pwm2;
    NEXELLI2CState i2c[SWALLOW_NUM_I2C];
    NexellWDTState wdt;

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
    NEXELL_SWALLOW_I2C0,
    NEXELL_SWALLOW_I2C1,
    NEXELL_SWALLOW_I2C2,
    NEXELL_SWALLOW_I2C3,
    NEXELL_SWALLOW_I2C4,
    NEXELL_SWALLOW_I2C5,
    NEXELL_SWALLOW_I2C6,
    NEXELL_SWALLOW_I2C7,
    NEXELL_SWALLOW_I2C8,
    NEXELL_SWALLOW_I2C9,
    NEXELL_SWALLOW_I2C10,
    NEXELL_SWALLOW_I2C11,
    NEXELL_SWALLOW_ADC0,
    NEXELL_SWALLOW_WDT,
    NEXELL_SWALLOW_VIP,
    NEXELL_SWALLOW_SCALER,
    NEXELL_SWALLOW_SPI0,
    NEXELL_SWALLOW_SPI1,
    NEXELL_SWALLOW_SPI2,
    NEXELL_SWALLOW_UART0,
    NEXELL_SWALLOW_PWM0,
    NEXELL_SWALLOW_PWM1,
    NEXELL_SWALLOW_PWM2,
    NEXELL_SWALLOW_UART1,
    NEXELL_SWALLOW_UART2,
    NEXELL_SWALLOW_UART3,
    NEXELL_SWALLOW_UART4,
    NEXELL_SWALLOW_UART5,
    NEXELL_SWALLOW_USB,
    NEXELL_SWALLOW_SRAM,
    NEXELL_SWALLOW_DRAM
};

enum {
    USB_IRQ = 13,
    VIP_IRQ = 14,
    SCALER_IRQ = 18,
    I2C0_IRQ = 34,
    I2C1_IRQ = 35,
    I2C2_IRQ = 36,
    I2C3_IRQ = 37,
    I2C4_IRQ = 38,
    I2C5_IRQ = 39,
    I2C6_IRQ = 40,
    I2C7_IRQ = 41,
    I2C8_IRQ = 42,
    I2C9_IRQ = 43,
    I2C10_IRQ = 44,
    I2C11_IRQ = 45,
    UART0_IRQ = 56,
    UART1_IRQ = 57,
    UART2_IRQ = 58,
    UART3_IRQ = 59,
    UART4_IRQ = 60,
    UART5_IRQ = 61,
    ADC0_IRQ = 46,
    WDT_IRQ = 47,
    SPI0_IRQ = 48,
    SPI1_IRQ = 49,
    SPI2_IRQ = 50,
    PWM0_IRQ_INT0 = 62,
    PWM1_IRQ_INT0 = 66,
    PWM2_IRQ_INT0 = 70,
    IRQ_NDEV = 128,
};

enum {
    NEXELL_SWALLOW_CLOCK_FREQ = 200000000
};

#define NEXELL_SWALLOW_PLIC_HART_CONFIG "MS"
#define NEXELL_SWALLOW_PLIC_NUM_SOURCES 128
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
