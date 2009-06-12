#ifndef CYGONCE_FLEXANET_H
#define CYGONCE_FLEXANET_H

/*=============================================================================
//
//      flexanet.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Jordi Colomer <jco@ict.es>
// Contributors: Jordi Colomer <jxo@ict.es>
// Date:         2001-06-15
// Purpose:      SA1110/Flexanet platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/flexanet.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


#ifndef __ASSEMBLER__
//
// Board Control Register
// Note: This register is write-only.  Thus a shadow copy is provided so that
// it may be safely updated/shared by multiple threads.
//
extern unsigned long _flexanet_BCR;  // Shadow copy

extern void flexanet_BCR(unsigned long mask, unsigned long value);

#endif

// 
// Signal assertion levels
//
#define SA1110_LOGIC_ONE(m)  (m & 0xFFFFFFFF)
#define SA1110_LOGIC_ZERO(m) (m & 0x00000000)

//
// GPIO settings
//
#define SA1110_GPIO_DIR       0x080037FE
#define SA1110_GPIO_ALT       0x080037FC
#define SA1110_GPIO_CLR       0x080037FE
#define SA1110_GPIO_SET       0x00000000


//
// Flexanet Board Control Register
//
#define SA1110_BOARD_CONTROL        REG32_PTR(0x10000000)

/* Power-up value */
#define SA1110_BCR_MIN              0x00000000

/* Mandatory bits */
#define SA1110_BCR_LED_GREEN  (1<<0)  /* General-purpose green LED (1 = on) */
#define SA1110_BCR_SPARE_1    (1<<1)  /* Not defined */
#define SA1110_BCR_CF1_RST    (1<<2)  /* Compact Flash Slot #1 Reset (1 = reset) */
#define SA1110_BCR_CF2_RST    (1<<3)  /* Compact Flash Slot #2 Reset (1 = reset) */
#define SA1110_BCR_GUI_NRST   (1<<4)  /* GUI board reset (0 = reset) */
#define SA1110_BCR_RTS1       (1<<5)  /* RS232 RTS for UART-1 */
#define SA1110_BCR_RTS3       (1<<6)  /* RS232 RTS for UART-3 */
#define SA1110_BCR_XCDBG0     (1<<7)  /* Not defined. Wired to XPLA3 for debug */

/* BCR extension, only required by L3-bus in some audio codecs */
#define SA1110_BCR_L3MOD      (1<<8)  /* L3-bus MODE signal */
#define SA1110_BCR_L3DAT      (1<<9)  /* L3-bus DATA signal */
#define SA1110_BCR_L3CLK      (1<<10) /* L3-bus CLK signal */
#define SA1110_BCR_SPARE_11   (1<<11) /* Not defined */
#define SA1110_BCR_SPARE_12   (1<<12) /* Not defined */
#define SA1110_BCR_SPARE_13   (1<<13) /* Not defined */
#define SA1110_BCR_SPARE_14   (1<<14) /* Not defined */
#define SA1110_BCR_SPARE_15   (1<<15) /* Not defined */

/* Board Status Register */
#define SA1110_BSR_CTS1       (1<<0)  /* RS232 CTS for UART-1 */
#define SA1110_BSR_CTS3       (1<<1)  /* RS232 CTS for UART-3 */
#define SA1110_BSR_DSR1       (1<<2)  /* RS232 DSR for UART-1 */
#define SA1110_BSR_DSR3       (1<<3)  /* RS232 DSR for UART-3 */
#define SA1110_BSR_ID0        (1<<4)  /* Board identification */
#define SA1110_BSR_ID1        (1<<5)
#define SA1110_BSR_CFG0       (1<<6)  /* Board configuration options */
#define SA1110_BSR_CFG1       (1<<7)

/* GPIOs for which the generic definition doesn't say much */
#define SA1110_GPIO_CF1_NCD         (1<<14)  /* Card Detect from CF slot #1 */
#define SA1110_GPIO_CF2_NCD         (1<<15)  /* Card Detect from CF slot #2 */
#define SA1110_GPIO_CF1_IRQ         (1<<16)  /* IRQ from CF slot #1 */
#define SA1110_SA1110_GPIO_CF2_IRQ  (1<<17)  /* IRQ from CF slot #2 */
#define SA1110_GPIO_APP_IRQ         (1<<18)  /* Extra IRQ from application bus */
#define SA1110_GPIO_RADIO_REF       (1<<20)  /* Ref. clock for UART3 (Radio) */
#define SA1110_GPIO_CF1_BVD1    	(1<<21)  /* BVD1 from CF slot #1 */
#define SA1110_GPIO_CF2_BVD1        (1<<22)  /* BVD1 from CF slot #2 */
#define SA1110_GPIO_GUI_IRQ	        (1<<23)  /* IRQ from GUI board (i.e., UCB1300) */
#define SA1110_GPIO_ETH_IRQ	        (1<<24)  /* IRQ from Ethernet controller */
#define SA1110_GPIO_INTIP_IRQ    	(1<<25)  /* Measurement IRQ (INTIP) */
#define SA1110_GPIO_XMI_IRQ	        (1<<26)  /* External Module Insertion interrupt */

/* IRQ mappings from GPIOs */
#define SA1110_IRQ_GPIO_CF1_CD		CYGNUM_HAL_INTERRUPT_GPIO14
#define SA1110_IRQ_GPIO_CF2_CD      CYGNUM_HAL_INTERRUPT_GPIO15
#define SA1110_IRQ_GPIO_CF1_IRQ     CYGNUM_HAL_INTERRUPT_GPIO16
#define SA1110_IRQ_GPIO_CF2_IRQ     CYGNUM_HAL_INTERRUPT_GPIO17
#define SA1110_IRQ_GPIO_APP         CYGNUM_HAL_INTERRUPT_GPIO18
#define SA1110_IRQ_GPIO_CF1_BVD1    CYGNUM_HAL_INTERRUPT_GPIO21
#define SA1110_IRQ_GPIO_CF2_BVD1    CYGNUM_HAL_INTERRUPT_GPIO22
#define SA1110_IRQ_GPIO_GUI         CYGNUM_HAL_INTERRUPT_GPIO23
#define SA1110_IRQ_GPIO_ETH         CYGNUM_HAL_INTERRUPT_GPIO24
#define SA1110_IRQ_GPIO_INTIP       CYGNUM_HAL_INTERRUPT_GPIO25

/* On-Board Ethernet (physical addrs) */
#define SA1110_FHH_ETH_IOBASE       0x18000000  /* I/O base */
#define SA1110_FHH_ETH_MMBASE       0x18800000  /* Attribute-memory base */

/*---------------------------------------------------------------------------*/
/* end of flexanet.h                                                          */
#endif /* CYGONCE_FLEXANET_H */
