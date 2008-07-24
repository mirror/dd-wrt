/*
 *  linux/include/asm-arm/arch-camelot/irqs.h
 *
 *  Copyright (C) 2001 Altera Corporation
 *
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

/* Use the Excalibur chip definitions */
#define INT_CTRL_TYPE
#include "asm/arch/int_ctrl.h"

#ifdef CONFIG_SL3516_ASIC
#define	IRQ_SERIRQ_MAX					31
#define	IRQ_SERIRQ1						31
#define	IRQ_SERIRQ0                  	30
#define	IRQ_PCID 	                  	29
#define	IRQ_PCIC    	             	28
#define	IRQ_PCIB        	         	27
#define IRQ_PWR							26
#define IRQ_CIR							25
#define	IRQ_GPIO2                   	24
#define	IRQ_GPIO1                   	23
#define	IRQ_GPIO	                	22
#define	IRQ_SSP                     	21
#define IRQ_LPC                         20
#define IRQ_LCD                         19
#define	IRQ_UART                  		18
#define	IRQ_RTC                			17
#define	IRQ_TIMER3                  	16
#define	IRQ_TIMER2                    	15
#define	IRQ_TIMER1                    	14
#define IRQ_FLASH						12
#define	IRQ_USB1                    	11
#define IRQ_USB0						10
#define	IRQ_DMA                     	9
#define	IRQ_PCI                    		8
#define	IRQ_IPSEC              			7
#define	IRQ_RAID                        6
#define	IRQ_IDE1                    	5
#define	IRQ_IDE0                   		4
#define	IRQ_WATCHDOG                    3
#define	IRQ_GMAC1                       2
#define IRQ_GMAC0						1
#define	IRQ_CPU0_IP_IRQ                 0
#else
#define	IRQ_SERIRQ_MAX					31
#define	IRQ_SERIRQ1						31
#define	IRQ_SERIRQ0                  	30
#define	IRQ_PCID 	                  	29
#define	IRQ_PCIC    	             	28
#define	IRQ_PCIB        	         	27
#define IRQ_PWR							26
#define IRQ_CIR							25
#define	IRQ_GPIO2                   	24
#define	IRQ_GPIO1                   	23
#define	IRQ_GPIO	                	22
#define	IRQ_SSP                     	21
#define IRQ_LPC                         20
#define IRQ_LCD                         19
#define	IRQ_UART                  		18
#define	IRQ_RTC                			17
#define	IRQ_TIMER3                  	16
#define	IRQ_TIMER2                    	15
#define	IRQ_TIMER1                    	14
#define IRQ_FLASH						12
#define	IRQ_USB1                    	11
#define IRQ_USB0						10
#define	IRQ_DMA                     	9
#define	IRQ_PCI                    		8
#define	IRQ_IPSEC              			7
#define	IRQ_RAID                        6
#define	IRQ_IDE1                    	5
#define	IRQ_IDE0                   		4
#define	IRQ_WATCHDOG                    3
#define	IRQ_GMAC1                       2
#define IRQ_GMAC0						1
#endif

#define ARCH_TIMER_IRQ		               IRQ_TIMER2   /* for MV 4.0 */

#define IRQ_PCI_INTA				       PCI_IRQ_OFFSET + 0
#define IRQ_PCI_INTB				       PCI_IRQ_OFFSET + 1
#define IRQ_PCI_INTC				       PCI_IRQ_OFFSET + 2
#define IRQ_PCI_INTD				       PCI_IRQ_OFFSET + 3

#define NR_IRQS                           (IRQ_PCI_INTD + 4)



