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

#include "../arch/arm/mach-feroceon-mv78xx0/config/mvSysHwConfig.h"

/****************************************/
/* Interrupts                           */
/****************************************/

#ifdef __ASSEMBLY__
/* for Asm only */
#define MV_ASM_CPU_CAUSE_LOW_REG 	        (INTER_REGS_BASE + 0x20204)
#define MV_ASM_CPU_CAUSE_HIGH_REG 	        (INTER_REGS_BASE + 0x20208)
#define MV_ASM_CPU_MASK_LOW_REG 		(INTER_REGS_BASE + 0x20210)
#define MV_ASM_CPU_MASK_HIGH_REG		(INTER_REGS_BASE + 0x20214)
#define MV_ASM_GPP_IRQ_CAUSE_REG		(INTER_REGS_BASE + 0x10114)
#define MV_ASM_GPP_IRQ_MASK_REG			(INTER_REGS_BASE + 0x1011C)
#endif

/* MV78XX0 Interrupt Numbers */
#define IRQ_START			      			 0
#define IRQ_TIMER0			      			 8
#define IRQ_PEX_INT(dev)		            (32+(dev))

#define CESA_IRQ							19

#define ETH_PORT_IRQ_NUM(dev)     	        (40+((dev)*4))
#define IRQ_USB_CTRL(dev)	            	(16 + (dev))
#define IRQ_UART(dev)		             	(12 + (dev))	
#define XOR0_IRQ_NUM						22
#define XOR1_IRQ_NUM						23
#define IRQ_CPU_GPP_0_7	             		56
#define IRQ_CPU_GPP_8_15                    57
#define IRQ_CPU_GPP_16_23                   58
#define SATA_IRQ_NUM						26

#define DOORBELL_IN_IRQ                     60
#define IRQ_ASM_GPP_START                   64
#define IRQ_GPP_START			     		64

#define NR_IRQS                      		96


#define IRQ_GPP_MASK      				(BIT24 | BIT25 | BIT26)

