/*
 *
 *  This file contains the register definitions for the Excalibur
 *  Timer TIMER00.
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

#ifndef __INT_CTRL_H
#define __INT_CTRL_H

#define PCI_IRQ_OFFSET			  64    /* PCI start IRQ number */
#define FIQ_OFFSET                32

#define IRQ_SOURCE(base_addr)   (INT_CTRL_TYPE(base_addr  + 0x00))
#define IRQ_MASK(base_addr)     (INT_CTRL_TYPE (base_addr  + 0x04 ))
#define IRQ_CLEAR(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x08 ))
#define IRQ_TMODE(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x0C ))
#define IRQ_TLEVEL(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x10 ))
#define IRQ_STATUS(base_addr)   (INT_CTRL_TYPE (base_addr  + 0x14 ))
#define FIQ_SOURCE(base_addr)   (INT_CTRL_TYPE (base_addr  + 0x20 ))
#define FIQ_MASK(base_addr)     (INT_CTRL_TYPE (base_addr  + 0x24 ))
#define FIQ_CLEAR(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x28 ))
#define FIQ_TMODE(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x2C ))
#define FIQ_LEVEL(base_addr)    (INT_CTRL_TYPE (base_addr  + 0x30 ))
#define FIQ_STATUS(base_addr)   (INT_CTRL_TYPE (base_addr  + 0x34 ))

#ifdef CONFIG_SL3516_ASIC
#define	IRQ_SERIRQ0_OFFSET                 	30
#define	IRQ_PCID_OFFSET	                 	29
#define	IRQ_PCIC_OFFSET	                 	28
#define	IRQ_PCIB_OFFSET	                 	27
#define IRQ_PWR_OFFSET				    	26
#define IRQ_CIR_OFFSET						25
#define	IRQ_GPIO2_OFFSET                   	24
#define	IRQ_GPIO1_OFFSET                   	23
#define	IRQ_GPIO_OFFSET	                	22
#define	IRQ_SSP_OFFSET                     	21
#define IRQ_LPC_OFFSET                      20
#define IRQ_LCD_OFFSET                      19
#define	IRQ_UART_OFFSET                		18
#define	IRQ_RTC_OFFSET             			17
#define	IRQ_TIMER3_OFFSET                  	16
#define	IRQ_TIMER2_OFFSET                  	15
#define	IRQ_TIMER1_OFFSET                  	14
#define IRQ_FLASH_OFFSET					12
#define	IRQ_USB1_OFFSET                    	11
#define IRQ_USB0_OFFSET						10
#define	IRQ_DMA_OFFSET                     	9
#define	IRQ_PCI_OFFSET                 		8
#define	IRQ_IPSEC_OFFSET            		7
#define	IRQ_RAID_OFFSET                     6
#define	IRQ_IDE1_OFFSET                    	5
#define	IRQ_IDE0_OFFSET                		4
#define	IRQ_WATCHDOG_OFFSET                 3
#define	IRQ_GMAC1_OFFSET                    2
#define IRQ_GMAC0_OFFSET					1
#define	IRQ_CPU0_IP_IRQ_OFFSET              0

#define	IRQ_SERIRQ0_MASK                 	(1<<30)
#define IRQ_PCID_MASK				    	(1<<29)
#define IRQ_PCIC_MASK				    	(1<<28)
#define IRQ_PCIB_MASK				    	(1<<27)
#define IRQ_PWR_MASK				    	(1<<26)
#define IRQ_CIR_MASK						(1<<25)
#define	IRQ_GPIO2_MASK                   	(1<<24)
#define	IRQ_GPIO1_MASK                    	(1<<23)
#define	IRQ_GPIO_MASK	                    (1<<22)
#define	IRQ_SSP_MASK                        (1<<21)
#define IRQ_LPC_MASK                        (1<<20)
#define IRQ_LCD_MASK                        (1<<19)
#define	IRQ_UART_MASK                	    (1<<18)
#define	IRQ_RTC_MASK             		    (1<<17)
#define	IRQ_TIMER3_MASK                     (1<<16)
#define	IRQ_TIMER2_MASK                     (1<<15)
#define	IRQ_TIMER1_MASK                     (1<<14)
#define IRQ_FLASH_MASK					    (1<<12)
#define	IRQ_USB1_MASK                       (1<<11)
#define IRQ_USB0_MASK					    (1<<10)
#define	IRQ_DMA_MASK                        (1<< 9)
#define	IRQ_PCI_MASK                 	    (1<< 8)
#define	IRQ_IPSEC_MASK            		    (1<< 7)
#define	IRQ_RAID_MASK                       (1<< 6)
#define	IRQ_IDE1_MASK                       (1<< 5)
#define	IRQ_IDE0_MASK                 	    (1<< 4)
#define	IRQ_WATCHDOG_MASK                   (1<< 3)
#define	IRQ_GMAC1_MASK                      (1<< 2)
#define IRQ_GMAC0_MASK					    (1<< 1)
#define	IRQ_CPU0_IP_IRQ_MASK                (1<< 0)
#else
#define	IRQ_SERIRQ0_OFFSET                 	30
#define	IRQ_PCID_OFFSET	                 	29
#define	IRQ_PCIC_OFFSET	                 	28
#define	IRQ_PCIB_OFFSET	                 	27
#define IRQ_PWR_OFFSET				    	26
#define IRQ_CIR_OFFSET						25
#define	IRQ_GPIO2_OFFSET                   	24
#define	IRQ_GPIO1_OFFSET                   	23
#define	IRQ_GPIO_OFFSET	                	22
#define	IRQ_SSP_OFFSET                     	21
#define IRQ_LPC_OFFSET                      20
#define IRQ_LCD_OFFSET                      19
#define	IRQ_UART_OFFSET                		18
#define	IRQ_RTC_OFFSET             			17
#define	IRQ_TIMER3_OFFSET                  	16
#define	IRQ_TIMER2_OFFSET                  	15
#define	IRQ_TIMER1_OFFSET                  	14
#define IRQ_FLASH_OFFSET					12
#define	IRQ_USB1_OFFSET                    	11
#define IRQ_USB0_OFFSET						10
#define	IRQ_DMA_OFFSET                     	9
#define	IRQ_PCI_OFFSET                 		8
#define	IRQ_IPSEC_OFFSET            		7
#define	IRQ_RAID_OFFSET                     6
#define	IRQ_IDE1_OFFSET                    	5
#define	IRQ_IDE0_OFFSET                		4
#define	IRQ_WATCHDOG_OFFSET                 3
#define	IRQ_GMAC1_OFFSET                    2
#define IRQ_GMAC0_OFFSET					1
#define	IRQ_CPU0_IP_IRQ_OFFSET              0

#define	IRQ_SERIRQ0_MASK                 	(1<<30)
#define IRQ_PCID_MASK				    	(1<<29)
#define IRQ_PCIC_MASK				    	(1<<28)
#define IRQ_PCIB_MASK				    	(1<<27)
#define IRQ_PWR_MASK				    	(1<<26)
#define IRQ_CIR_MASK						(1<<25)
#define	IRQ_GPIO2_MASK                   	(1<<24)
#define	IRQ_GPIO1_MASK                    	(1<<23)
#define	IRQ_GPIO_MASK	                    (1<<22)
#define	IRQ_SSP_MASK                        (1<<21)
#define IRQ_LPC_MASK                        (1<<20)
#define IRQ_LCD_MASK                        (1<<19)
#define	IRQ_UART_MASK                	    (1<<18)
#define	IRQ_RTC_MASK             		    (1<<17)
#define	IRQ_TIMER3_MASK                     (1<<16)
#define	IRQ_TIMER2_MASK                     (1<<15)
#define	IRQ_TIMER1_MASK                     (1<<14)
#define IRQ_FLASH_MASK					    (1<<12)
#define	IRQ_USB1_MASK                       (1<<11)
#define IRQ_USB0_MASK					    (1<<10)
#define	IRQ_DMA_MASK                        (1<< 9)
#define	IRQ_PCI_MASK                 	    (1<< 8)
#define	IRQ_IPSEC_MASK            		    (1<< 7)
#define	IRQ_RAID_MASK                       (1<< 6)
#define	IRQ_IDE1_MASK                       (1<< 5)
#define	IRQ_IDE0_MASK                 	    (1<< 4)
#define	IRQ_WATCHDOG_MASK                   (1<< 3)
#define	IRQ_GMAC1_MASK                      (1<< 2)
#define IRQ_GMAC0_MASK					    (1<< 1)
#define	IRQ_CPU0_IP_IRQ_MASK                (1<< 0)
#endif


#endif /* __INT_CTRL_H */


