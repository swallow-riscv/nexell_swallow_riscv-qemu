/*
 * Nexell Swallow processors ADC registers definition.
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

#ifndef HW_NEXELL_ADC_H
#define HW_NEXELL_ADC_H

#define ADC_CON		0x00
#define ADC_DAT		0x04
#define ADC_INTENB	0x08
#define ADC_INTCLR	0x0c
#define ADC_PRESCON	0x10
#define ADC_EN		0x18

#define ADC_CON_ADON (1 << 2)
#define ADC_CON_SWSTART	(1 << 0)
#define ADC_INT_CLR	 (1 << 0)
#define ADC_INT_ENB	 (1 << 0)


#define ADC_COMMON_ADDRESS 0x100

#define TYPE_NEXELL_ADC "nexell-adc"
#define NEXELL_ADC(obj) \
    OBJECT_CHECK(NEXELLADCState, (obj), TYPE_NEXELL_ADC)

typedef struct {
	/* <private> */
	SysBusDevice parent_obj;

	/* <public> */
	MemoryRegion mmio;

	uint32_t adc_con;
	uint32_t adc_dat;
	uint32_t adc_intenb;
	uint32_t adc_intclr;
	uint32_t adc_prescon;
	uint32_t adc_en;

	qemu_irq irq;
} NEXELLADCState;

#endif /* HW_NEXELL_ADC_H */
