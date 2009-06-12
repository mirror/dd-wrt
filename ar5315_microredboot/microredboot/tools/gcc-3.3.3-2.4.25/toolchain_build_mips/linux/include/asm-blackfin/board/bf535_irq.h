/*
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/irq.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Changed by HuTao Apr18, 2003
 *
 * Copyright was missing when I got the code so took from MIPS arch ...MaTed---
 * Copyright (C) 1994 by Waldorf GMBH, written by Ralf Baechle
 * Copyright (C) 1995, 96, 97, 98, 99, 2000, 2001 by Ralf Baechle
 *
 * Adapted for BlackFin (ADI) by Ted Ma <mated@sympatico.ca>
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2002 Lineo, Inc. <mattw@lineo.com>
 */

#ifndef _BF535_IRQ_H_
#define _BF535_IRQ_H_

/*
 * Interrupt source definitions
             Event Source    Core Event Name
Core        Emulation               **
 Events         (highest priority)  EMU         0
            Reset                   RST         1
            NMI                     NMI         2
            Exception               EVX         3
            Reserved                --          4
            Hardware Error          IVHW        5
            Core Timer              IVTMR       6 *
            System RTC              IVG7        7
            Interrupts USB Rx/Tx    IVG7        8
            ** SPARE **             --          9
            PCI                     IVG7        10
            SPORT0 RX/TX            IVG8        11
            ** SPARE **             --          12
            SPORT1 RX/TX            IVG8        13
            ** SPARE **             --          14
            SPI0 RX/TX              IVG9        15
            ** SPARE **             --          16
            SPI1 RX/TX              IVG9        17
            ** SPARE **             --          18
            UART0 RX/TX             IVG10       19
            **SPARE **              --          20
            UART1 RX/TX             IVG10       21
            ** SPARE **             --          22
            Timer0                  IVG11       23
            Timer1                  IVG11       24
            Timer2                  IVG11       25
            ** SPARE **                         26
            Programmable Flags      **
                    Interrupt A     IVG12       27 (all 8)
                    Interrupt B     IVG12       28 (all 8)
            Memory DMA              --          29
            Watchdog Timer          IVG13       30
            Software Interrupt 1    IVG14       31
            Software Interrupt 2    --
                 (lowest priority)  IVG15       32 *
 */
/* The ABSTRACT IRQ definitions */
/** the first seven of the following are fixed, the rest you change if you need to **/
#define	IRQ_EMU			0	// Emulation
#define	IRQ_RST			1	// reset
#define	IRQ_NMI			2	// Non Maskable
#define	IRQ_EVX			3	// Exception
#define	IRQ_UNUSED		4	//  - unused interrupt
#define	IRQ_HWERR		5	// Hardware Error
#define	IRQ_CORETMR		6	// Core timer
#define	IRQ_RTC			7	// Real Time Clock		
#define	IRQ_USB			8	// Universal Serial Bus
#define	IRQ_SPARE1		9	//  Spare ?? USB
#define IRQ_PCI_INTA		9	// PCI INTA

// Exchanged with IRQ_UART1. HuTao Apr18 2003
#define	IRQ_PCI			10 	// Personal Computer Interface ??
#define	IRQ_SPORT0		11	// Synchronous Serial Port 0
#define	IRQ_SPARE2		12	//  Spare ?? split SPORT0
#define IRQ_PCI_INTB		12	// PCI INTB

#define	IRQ_SPORT1		13	// Synchronous Serial Port 1
#define	IRQ_SPARE3		14	//  Spare ?? split SPORT1
#define IRQ_PCI_INTC		14

#define	IRQ_SPI0		15	// Serial Peripheral Interface 0
#define	IRQ_SPARE4		16	//  Spare 
#define IRQ_PCI_INTD		16

#define	IRQ_SPI1		17	// Serial Peripheral Interface 1
#define	IRQ_SPARE5		18	//  Spare
#define	IRQ_UART0		19	// UART 0
#define	IRQ_SPARE6		20	//  Spare

// Changed to make UART driver work. HuTao Apr18 2003
#define	IRQ_UART1		21 	// UART 1

#define	IRQ_SPARE7		22	//  Spare
#define	IRQ_TMR0		23	// Timer 0
#define	IRQ_TMR1		24	// Timer 1
#define	IRQ_TMR2		25	// Timer 2
#define	IRQ_SPARE8		26	//  Spare
#define	IRQ_PROG_INTA		27	// Programmable Flags A (8)
#define	IRQ_PROG_INTB		28	// Programmable Flags B (8)
#define	IRQ_MEM_DMA		29	// Memory DMA Xfer Comp
#define	IRQ_WATCH	   	30	// Watch Dog Timer
#define	IRQ_SW_INT1		31	// Software Int 1
#define	IRQ_SW_INT2		32	// Software Int 2 (reserved for SYSCALL)

#define SYS_IRQS		33	// Number of interrupt levels the kernel sees.

#endif /* _BF535_IRQ_H_ */
