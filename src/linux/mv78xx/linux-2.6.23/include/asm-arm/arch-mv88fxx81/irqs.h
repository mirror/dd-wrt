/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/autoconf.h>
#if defined (CONFIG_ARCH_MV88f1181)
#	include "../arch/arm/mach-mv88fxx81/mv88f1181/mvSysHwConfig.h"
#elif defined (CONFIG_ARCH_MV88f5181)
#       include "../arch/arm/mach-mv88fxx81/mv88f5181/mvSysHwConfig.h"
#endif

/* for Asm only */
#define MV_ASM_IRQ_BASE_REG 		(INTER_REGS_BASE + 0x20000)
#define MV_ASM_IRQ_CAUSE_REG		0x200
#define MV_ASM_IRQ_MASK_REG		0x204
#define MV_ASM_GPP_IRQ_BASE_REG		(INTER_REGS_BASE + 0x10000)
#define MV_ASM_GPP_IRQ_CAUSE_REG	0x110   /* use data in for cause in case of level interrupts */ 
#define MV_ASM_GPP_IRQ_MASK_REG        	0x11c	/* level mask */

/* for c */
#define MV_IRQ_CAUSE_REG		0x20200
#define MV_IRQ_MASK_REG			0x20204
#define MV_GPP_IRQ_CAUSE_REG		0x10114
#define MV_GPP_IRQ_MASK_REG        	0x1011c
#define MV_AHBTOMBUS_IRQ_CAUSE_REG 	0x20114


#define MV_PCI_MASK_REG(pciIf)		(0x41910 - ((pciIf) * 0x10000))
#define MV_PCI_MASK_ABCD		(BIT24 | BIT25 | BIT26 | BIT27 )

/* 
 *  Interrupt numbers
 */
#define IRQ_START			0
#define IRQ_BRIDGE			0
#define IRQ_UART0			3
#define IRQ_UART1                       4
#define IRQ_TWSI			5
#define IRQ_GPP_0_7			6
#define IRQ_GPP_8_15                    7
#if defined (CONFIG_ARCH_MV88f5181)
#define IRQ_GPP_16_23                   8
#define IRQ_GPP_24_31                   9
#endif
#define IRQ_PEX0_ERR			10
#define IRQ_PEX0_INT			11
#if defined (CONFIG_ARCH_MV88f1181)
#define IRQ_PEX1_ERR			12
#define IRQ_PEX1_INT			13
#elif defined (CONFIG_ARCH_MV88f5181)
#define IRQ_PCI_ERR			15
#define IRQ_USB_BR_ERR			16
#define IRQ_USB_CTRL(x)			((x==0)? 17:12)
#define IRQ_GB_RX			18
#define IRQ_GB_TX                       19
#define IRQ_GB_MISC                     20
#define IRQ_GB_SUM                      21
#define IRQ_GB_ERR                      22
#define IRQ_IDMA_ERR                    23
#define IRQ_IDMA_0                      24
#define IRQ_IDMA_1                      25
#define IRQ_IDMA_2                      26
#define IRQ_IDMA_3                      27
#define CESA_IRQ			28
#endif

#define IRQ_GPP_START			32
#define IRQ_ASM_GPP_START               32
#define IRQ_GPP_0			32
#define IRQ_GPP_1                       33
#define IRQ_GPP_2                       34
#define IRQ_GPP_3                       35
#define IRQ_GPP_4                       36
#define IRQ_GPP_5                       37
#define IRQ_GPP_6                       38
#define IRQ_GPP_7                       39
#define IRQ_GPP_8                       40
#define IRQ_GPP_9                       41
#define IRQ_GPP_10                      42
#define IRQ_GPP_11                      43
#define IRQ_GPP_12                      44
#define IRQ_GPP_13                      45
#define IRQ_GPP_14                      46
#define IRQ_GPP_15                      47
#if defined (CONFIG_ARCH_MV88f5181)
#define IRQ_GPP_16                      48
#define IRQ_GPP_17                      49
#define IRQ_GPP_18                      50
#define IRQ_GPP_19                      51
#define IRQ_GPP_20                      52
#define IRQ_GPP_21                      53
#define IRQ_GPP_22                      54
#define IRQ_GPP_23                      55
#define IRQ_GPP_24                      56
#define IRQ_GPP_25                      57
#define IRQ_GPP_26                      58
#define IRQ_GPP_27                      59
#define IRQ_GPP_28                      60
#define IRQ_GPP_29                      61
#define IRQ_GPP_30                      62
#define IRQ_GPP_31                      63
#endif
#define NR_IRQS                         64

#define MV_VALID_INT_LOW		0x2cd9
#define MV_VALID_INT_HIGH		0xffff

/*********** timer **************/

#define TIME_IRQ        	IRQ_BRIDGE
#define BRIDGE_INT_CAUSE_REG	0x20110
#define BRIDGE_INT_MASK_REG    	0x20114
#define TIMER_BIT_MASK(x)	(1<<(x+1))

