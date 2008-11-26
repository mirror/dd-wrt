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

#include "../arch/arm/mach-feroceon-kw/config/mvSysHwConfig.h"

/* for Asm only */
/* #define MV_ASM_IRQ_BASE_REG 		(INTER_REGS_BASE + 0x20000) */
#define MV_ASM_IRQ_CAUSE_LOW_REG	(INTER_REGS_BASE + 0x20200)
#define MV_ASM_IRQ_CAUSE_HIGH_REG	(INTER_REGS_BASE + 0x20210)

#define MV_ASM_IRQ_MASK_LOW_REG		(INTER_REGS_BASE + 0x20204)
#define MV_ASM_IRQ_MASK_HIGH_REG	(INTER_REGS_BASE + 0x20214)

/* #define MV_ASM_GPP_IRQ_BASE_REG		(INTER_REGS_BASE + 0x10000) */
#define MV_ASM_GPP_IRQ_CAUSE_REG	(INTER_REGS_BASE + 0x10110) /* use data in for cause in case of level interrupts */
#define MV_ASM_GPP_IRQ_HIGH_CAUSE_REG	(INTER_REGS_BASE + 0x10150) /* use high data in for cause in case of level interrupts */ 
 
#define MV_ASM_GPP_IRQ_MASK_REG        	(INTER_REGS_BASE + 0x1011c)	/* level low mask */
#define MV_ASM_GPP_IRQ_HIGH_MASK_REG    (INTER_REGS_BASE + 0x1015c)	/* level high mask */

/* for c */
#define MV_IRQ_CAUSE_LOW_REG		0x20200
#define MV_IRQ_CAUSE_HIGH_REG		0x20210

#define MV_IRQ_MASK_LOW_REG		0x20204
#define MV_IRQ_MASK_HIGH_REG		0x20214

#define MV_GPP_IRQ_CAUSE_REG		0x10110
#define MV_GPP_IRQ_HIGH_CAUSE_REG	0x10150

#define MV_GPP_IRQ_EDGE_REG		0x10118
#define MV_GPP_IRQ_HIGH_EDGE_REG	0x10158

#define MV_GPP_IRQ_MASK_REG        	0x1011c
#define MV_GPP_IRQ_HIGH_MASK_REG        0x1015c

#define MV_GPP_IRQ_POLARITY        	0x1010c
#define MV_GPP_IRQ_HIGH_POLARITY        0x1014c

#if 0
#define MV_AHBTOMBUS_IRQ_CAUSE_REG 	0x20114
#endif

#define MV_PCI_MASK_REG			0x41910
#define MV_PCI_MASK_ABCD		(BIT24 | BIT25 | BIT26 | BIT27 )

#define GPP_IRQ_TYPE_LEVEL		0
#define GPP_IRQ_TYPE_CHANGE_LEVEL	1


/* 
 *  Interrupt numbers
 */
#define IRQ_START			0
#define IRQ_MAIN_HIGH_SUM		0
#define IRQ_BRIDGE			1


#define XOR0_IRQ_NUM			5
#define XOR1_IRQ_NUM			6
#define XOR2_IRQ_NUM			7
#define XOR3_IRQ_NUM			8
#define IRQ_XOR0_ERR			42
#define IRQ_XOR1_ERR			43

#define IRQ_PEX0_INT			9
#define IRQ_PEX0_ERR			44

#define ETH_PORT_IRQ_NUM(x)		((x == 0) ? 11 : 15)
#define IRQ_ETH_ERR(x)			((x == 0) ? 46 : 47)	    

#define IRQ_USB_CTRL(x)			((x == 0) ? 19 : 20)
#define IRQ_USB_BR_ERR			48

#define SATA_IRQ_NUM   			21

#define CESA_IRQ			22
#define CESA_ERR			49

#define SPI_IRQ				23

#define IRQ_AUDIO_INT			24
#define IRQ_AUDIO_ERR			50

#define IRQ_TS_INT(x)			((x == 0) ? 26 : 27)

# define SDIO_IRQ_NUM			28

#define IRQ_TWSI			29
#define IRQ_AVB				30

#define TDM_IRQ_INT			31

#define IRQ_UART0			33
#define IRQ_UART1                       34

#define IRQ_GPP_LOW_0_7			35
#define IRQ_GPP_LOW_8_15                36
#define IRQ_GPP_LOW_16_23               37
#define IRQ_GPP_LOW_24_31               38
#define IRQ_GPP_HIGH_0_7		39
#define IRQ_GPP_HIGH_8_15               40
#define IRQ_GPP_HIGH_16_23              41

#define IRQ_GPP_START			64
#define IRQ_ASM_GPP_START               64

#define IRQ_GPP_0			64
#define IRQ_GPP_1                       65
#define IRQ_GPP_2                       66
#define IRQ_GPP_3                       67
#define IRQ_GPP_4                       68
#define IRQ_GPP_5                       69
#define IRQ_GPP_6                       70
#define IRQ_GPP_7                       71
#define IRQ_GPP_8                       72
#define IRQ_GPP_9                       73
#define IRQ_GPP_10                      74
#define IRQ_GPP_11                      75
#define IRQ_GPP_12                      76
#define IRQ_GPP_13                      77
#define IRQ_GPP_14                      78
#define IRQ_GPP_15                      79
#define IRQ_GPP_16                      80
#define IRQ_GPP_17                      81
#define IRQ_GPP_18                      82
#define IRQ_GPP_19                      83
#define IRQ_GPP_20                      84
#define IRQ_GPP_21                      85
#define IRQ_GPP_22                      86
#define IRQ_GPP_23                      87
#define IRQ_GPP_24                      88
#define IRQ_GPP_25                      89
#define IRQ_GPP_26                      90
#define IRQ_GPP_27                      91
#define IRQ_GPP_28                      92
#define IRQ_GPP_29                      93
#define IRQ_GPP_30                      94
#define IRQ_GPP_31                      95
#define IRQ_GPP_32                      96
#define IRQ_GPP_33                      97
#define IRQ_GPP_34                      98
#define IRQ_GPP_35                      99
#define IRQ_GPP_36                      100
#define IRQ_GPP_37                      101
#define IRQ_GPP_38                      102
#define IRQ_GPP_39                      103
#define IRQ_GPP_40                      104
#define IRQ_GPP_41                      105
#define IRQ_GPP_42                      106
#define IRQ_GPP_43                      107
#define IRQ_GPP_44                      108
#define IRQ_GPP_45                      109
#define IRQ_GPP_46                      110
#define IRQ_GPP_47                      111
#define IRQ_GPP_48                      112
#define IRQ_GPP_49                      113


#define NR_IRQS                         128
#if 0
#define MV_VALID_INT_LOW		0x2cd9
#define MV_VALID_INT_HIGH		0xffff
#endif
/*********** timer **************/

#define TIME_IRQ        	IRQ_BRIDGE
#define BRIDGE_INT_CAUSE_REG	0x20110
#define BRIDGE_INT_MASK_REG    	0x20114
#define TIMER_BIT_MASK(x)	(1<<(x+1))


