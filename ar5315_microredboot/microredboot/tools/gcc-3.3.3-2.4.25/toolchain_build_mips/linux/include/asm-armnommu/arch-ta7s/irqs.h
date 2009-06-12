/*
 * include/asm-armnommu/arch-ta7s/irqs.h
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) :
 * 
 */
#ifndef	_ASM_ARCH_IRQS_H
#define	_ASM_ARCH_IRQS_H

#if defined(A7VL) || defined(A7VE) || defined(A7VC) || defined(A7VT)
#define	NR_IRQS	31
#else
#define	NR_IRQS	16
#endif

#define IRQ_FIQ          0
#define IRQ_SOFTWARE     1
#define IRQ_SERIAL_0     2
#define IRQ_TIMER_0      3
#define IRQ_TIMER_1      4
#define IRQ_SERIAL_1     5
#define IRQ_WATCHDOG     6
#define IRQ_DMA_0        7
#define IRQ_DMA_1        8
#define IRQ_DMA_2        9
#define IRQ_DMA_3       10
#define IRQ_CSL_USER_0  11
#define IRQ_CSL_USER_1  12
#define IRQ_CSL_USER_2  13
#define IRQ_JTAG        14
#define IRQ_BREAKPOINT  15

#if defined(A7VL) || defined(A7VE) || defined(A7VC) || defined(A7VT)
#define IRQ_MFTA	16
#define IRQ_TWSI0	17
#define IRQ_TWSI1	18
#define IRQ_DMA_4	19
#define IRQ_DMA_5	20
#define IRQ_USB		21
#define IRQ_MAC0_TX	22
#define IRQ_MAC0_RX	23
#define IRQ_DMA_6	24
#define IRQ_DMA_7	25
#define IRQ_PLL		26
#define IRQ_MAC1_TX	27
#define IRQ_MAC1_RX	28
#define IRQ_CAN		29
#define	IRQ_ADC		30
#endif

#define IRQ_TC0	IRQ_TIMER_0

#endif
