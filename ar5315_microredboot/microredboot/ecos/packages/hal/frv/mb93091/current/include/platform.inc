#ifndef _CYGONCE_PLATFORM_INC_H_
#define _CYGONCE_PLATFORM_INC_H_
// #========================================================================
// #
// #    platform.inc
// #
// #    Fujitsu platform specific setups (assembler macros)
// #
// #========================================================================
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
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     gthomas
// # Contributors:  gthomas
// # Date:          2001-09-16
// # Purpose:       Fujitsu (FRV400) platform specific setups
// # Description:   This file defines various macros used by the generic
// #                HAL startup code.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

// This does not work in standalone mode on CB70. 
//#define DEBUG_LEDS

// Display a value in the system LEDs
        .macro  LED n
#ifdef DEBUG_LEDS
	li	_MB93091_FPGA_GPHL,gr15
	lduhi	@(gr15,0),gr14
	srli	gr14,#8,gr14
	andicc	gr14,#0x80,gr14,icc0
	bne	icc0,0,999f
        sethi   #(_MB93091_MB_LEDS>>16),gr15
        setlo   #(_MB93091_MB_LEDS&0xFFFF),gr15
        setlos  #\n,gr14
        not     gr14,gr14
        sti     gr14,@(gr15,0)
      999:
#endif
        membar
        .endm

// Platform initialization - only the necessary bits required to get the
// board started from a cold reset.
        .macro  platform_init
        li      0x7FFF,gr4      // First, a good long spin
05:     nop
        subi    gr4,1,gr4
        cmp     gr4,gr0,icc0
        bne     icc0,0,05b   

// First work out what kind of board we are, and set the bitmask which
// determines which elements from _platform_tab we use. In gr8:
//  bit 0 -- FR401, 64MiB   (CB11)
//  bit 1 -- FR403+, 128MiB (CB30,CB60)
//  bit 2 -- FR405,  64MiB  (CB70)
//  bit 3 -- FR555,  256MiB (CB41)
//  bit 4 -- VDK motherboard 
//  bit 5 -- FR451-P 128MiB

	movsg	psr,gr4
	srli	gr4,#24,gr4

	setlos  #0x18,gr8
	cmpi	gr4,#0x31,icc0 // FR555
	beq	icc0,#0,plf_set
	cmpi	gr4,#0x11,icc0 // FR501
	beq	icc0,#0,plf_set
	cmpi	gr4,#0x12,icc0 // FR501A
	beq	icc0,#0,plf_set

	setlos	#0x11,gr8
	cmpi	gr4,#0x20,icc0 // FR401
	beq	icc0,#0,plf_set
	cmpi	gr4,#0x21,icc0 // FR401A
	beq	icc0,#0,plf_set

	setlos	#0x12,gr8
	cmpi	gr4,#0x22,icc0 // FR403
	beq	icc0,#0,plf_set
	cmpi	gr4,#0x50,icc0 // FR451
	beq     icc0,#0,plf_set_maybe_cb70
	cmpi	gr4,#0x40,icc0 // FR405
	bne	icc0,#0,plf_set // if _not_ FR405, try this one and pray

plf_set_maybe_cb70:
        // OK, it's an FR405 or an FR451. 
        // Need to distinguish between CB60/CB70 or CB451-FPGA/CB451-F
	li	_MB93091_FPGA_VDKID,gr4
	lduhi	@(gr4,0),gr5
	cmpi	gr5,#0x46,icc0 // CB70 has this and it's 0x0046. CB60 doesn't.
	bne	icc0,#0,plf_set
	setlos	#0x04,gr8 // It's a CB70
	li	_MB93091_FPGA_GPHL,gr4
	lduhi	@(gr4,0),gr5
	andicc	gr5,#0x0100,gr0,icc0
	bne	icc0,0,no_mobo
	ori	gr8,0x10,gr8	// Add in the motherboard bit
 no_mobo:
	srli	gr5,#8,gr5
	andicc	gr5,#0x02,gr5,icc0
	bne	icc0,0,plf_set
	ori	gr8,0x20,gr8	// Add in the 451-P (extra SDRAM) bit

	 // gr8 now holds correct bitmask

 plf_set:
        call    10f             // position independent way to get @_platform_tab
_platform_tab_fr4xx:
//
// SDRAM setups for FR4xx
//
//	.long	_MB93091_MB_LEDS,0x3003
//      .long   _FRV400_SDRAM_BR0,0x00000000    // SDRAM 0x0XXXXXXX
//      .long   _FRV400_SDRAM_AM0,0x0FF00000
  
//      
// LOCAL bus setups for FR4xx
//
        .long   7,_FRV400_LBUS_CR0,0x03010D01     // ROM/FLASH 0xFF000000..0xFFFFFFFF
                                // 16 bits wide, 13 wait states, 1 idle

        .long   7,_FRV400_LBUS_BR1,0x10000000     // PCI bridge 0x10000000..0x100FFFFF
        .long   7,_FRV400_LBUS_AM1,0x000FFFFF
        .long   7,_FRV400_LBUS_CR1,0x00000000

        .long   7,_FRV400_LBUS_BR2,0x20000000     // SRAM, FPGA, PCI 0x20000000..0x2FFFFFFF
        .long   7,_FRV400_LBUS_AM2,0x0FFFFFFF
        .long   7,_FRV400_LBUS_CR2,0x00000000

	// organize rest into something sane
        .long   7,_FRV400_LBUS_BR3,0xF0300000
        .long   7,_FRV400_LBUS_AM3,0x000FFFFF
        .long   7,_FRV400_LBUS_CR3,0x00000000

        .long   6,_FRV400_LBUS_BR4,0xF0400000
        .long   6,_FRV400_LBUS_AM4,0x000FFFFF
        .long   6,_FRV400_LBUS_CR4,0x00000000

        .long   6,_FRV400_LBUS_BR5,0xF0500000
        .long   6,_FRV400_LBUS_AM5,0x000FFFFF
        .long   6,_FRV400_LBUS_CR5,0x00000000

        .long   4,_FRV400_LBUS_BR6,0xF0600000     // DM9000 on CB70 CPU card
        .long   4,_FRV400_LBUS_AM6,0x000FFFFF
        .long   4,_FRV400_LBUS_CR6,0x00400707

        .long   6,_FRV400_LBUS_BR7,0xF0700000
        .long   6,_FRV400_LBUS_AM7,0x000FFFFF
        .long   6,_FRV400_LBUS_CR7,0x00000000

	.long	7,_FRV400_GPIO_SIR,0x000c954f     // Routing for Rx0, Rx1, CTS
	.long	7,_FRV400_GPIO_SOR,0x00036ab0     // Routing for Tx0, Tx1, RTS, TOUT0, TOUT1
        
        .long   7,_FRV400_SDRAM_CTL,0x05022000    // SDRAM mode/control
        .long   4,_FRV400_SDRAM_AN0,0x00010201
        .long   2,_FRV400_SDRAM_AN0,0x00010102
        .long   1,_FRV400_SDRAM_AN0,0x00010101
	.long   7,_FRV400_SDRAM_BR0,0x00000000
	.long   7,_FRV400_SDRAM_AM0,0x03ffffff
        .long   2,_FRV400_SDRAM_AN1,0x00010102
        .long 0x20,_FRV400_SDRAM_AN1,0x00010201
	.long 0x22,_FRV400_SDRAM_BR1,0x04000000
	.long 0x22,_FRV400_SDRAM_AM1,0x03ffffff
        .long   7,_FRV400_SDRAM_ART,0x00000820
        .long   7,_FRV400_SDRAM_RCN,0x00000000
        .long   5,_FRV400_SDRAM_MS, 0x00020200
        .long   2,_FRV400_SDRAM_MS, 0x00010000
 //        .long   1,_FRV400_SDRAM_MS, 0x00020201
        .long   6,_FRV400_SDRAM_CFG,0x80000100
        .long   1,_FRV400_SDRAM_CFG,0x80000000
//?     .long   7,_FRV400_CLK_CTRL, 0x00000001     // External clock divisor (/2)
//
// SDRAM setups for FR5xx
//
        .long   8,_FRV550_SDRAM_ARS0,0x00000000    // SDRAM 0x0XXXXXXX
        .long   8,_FRV550_SDRAM_AMK0,0x000000FF

//
// LOCAL bus setups for FR5xx
//
        .long   8,_FRV550_LBUS_CR0,0x03010701     // ROM/FLASH 0xFF000000..0xFFFFFFFF
                                // 16 bits wide, 7 wait states, 1 idle

	.long   8,_FRV550_LBUS_BR1,0x10000000     // PCI bridge 0x10000000..0x100FFFFF
        .long   8,_FRV550_LBUS_AM1,0x000FFFFF
        .long   8,_FRV550_LBUS_CR1,0x00000000

	.long   8,_FRV550_LBUS_BR2,0x20000000     // SRAM, FPGA, PCI 0x20000000..0x2FFFFFFF
        .long   8,_FRV550_LBUS_AM2,0x0FFFFFFF
        .long   8,_FRV550_LBUS_CR2,0x00000000

        .long   8,_FRV550_LBUS_BR3,0x00000000     // SDRAM?
        .long   8,_FRV550_LBUS_AM3,0xFFFFFFFF
        .long   8,_FRV550_LBUS_CR3,0x00000F07

	.long   8,_FRV550_GPIO_SIR,0x000c957f     // Routing for Rx0, Rx1, CTS
        .long   8,_FRV550_GPIO_SOR,0x00336ab0     // Routing for Tx0, Tx1, RTS, TOUT0, TOUT1

	.long   8,_FRV550_SDRAM_CTL,0x000C6320    // SDRAM mode/control
        .long   8,_FRV550_SDRAM_AN, 0x32323232    // assume BA[0:1], RA[12:0], CA[9:0] (32Mbit*64)
        .long   8,_FRV550_SDRAM_ART,0x00000616

        .long   8,_FRV550_SDRAM_RCN,0x00000100
        .long   8,_FRV550_SDRAM_CFG,0x00000000
        .long   8,_FRV550_SDRAM_MS, 0x00003300

//?     .long   8,_FRV550_CLK_CTRL, 0x00000001     // External clock divisor (/2)

//      
// PCI controller/bridge
//
//	.long	0x10,_MB93091_MB_LEDS,0x100b
	.long	0x10,_MB93091_PCI_SLBUS_CONFIG,    0x000800E2		// This matches the docs
//	.long	0x10,_MB93091_PCI_SLBUS_CONFIG,    0x000000E0		// This matches the samples
	.long	0x10,_MB93091_PCI_ECS0_CONFIG,     0x00000000
	.long	0x10,_MB93091_PCI_ECS1_CONFIG,     0x000003C1
	.long	0x10,_MB93091_PCI_ECS2_CONFIG,     0x000001C1
	.long	0x10,_MB93091_PCI_ECS0_RANGE,      0x00000000
	.long	0x10,_MB93091_PCI_ECS0_ADDR,       0x00000000
	.long	0x10,_MB93091_PCI_ECS1_RANGE,      0x00007FFE         
	.long	0x10,_MB93091_PCI_ECS1_ADDR,       0x08108000          
	.long	0x10,_MB93091_PCI_ECS2_RANGE,      0x00007FFE
	.long	0x10,_MB93091_PCI_ECS2_ADDR,       0x08100000
	.long	0x10,_MB93091_PCI_PCIIO_RANGE,     0x0001FFFE
	.long	0x10,_MB93091_PCI_PCIIO_ADDR,      0x00120000         
	.long	0x10,_MB93091_PCI_PCIMEM_RANGE,    0x0003FFFE       
	.long	0x10,_MB93091_PCI_PCIMEM_ADDR,     0x00140000
	.long	0x10,_MB93091_PCI_PCIIO_PCI_ADDR,  0x24000001     
	.long	0x10,_MB93091_PCI_PCIMEM_PCI_ADDR, 0x28000000
	.long	0x10,_MB93091_MB_PCI_ARBITER,      0x00000001
	.long	0x10,_MB93091_MB_PCI_ARBITER,      0x00000001
	.long	0x10,_MB93091_PCI_SLBUS_CONFIG, 0x800800E2
//	.long	0x10,_MB93091_PCI_SLBUS_CONFIG, 0x800000E0
	// Turn the LEDs off.
	.long	0x10,_MB93091_MB_LEDS,0x0000ffff

// End list with SDRAM controller poll address
	
        .long   7,0,_FRV400_SDRAM_STS
        .long   8,0,_FRV550_SDRAM_STS

	
10:     movsg   lr,gr4                  // _platform_tab -> list of initializations
20:     ldi     @(gr4,4),gr5            // Register
        ldi     @(gr4,8),gr6            // Value
	ldi	@(gr4,0),gr7		// Platform Bitmask
	and	gr7,gr8,gr7
	cmpi	gr7,#0,icc0
	beq	icc0,0,25f		// Skip over tuples not for us
        cmp     gr5,gr0,icc0            // End of list (with SDRAM poll addr)?
        beq     icc0,0,30f
        sti     gr6,@(gr5,0)
25:	addi    gr4,3*4,gr4
        bra     20b                     // Next item

30:     ldi     @(gr6,0),gr5		// gr6 == _FRVxxx_SDRAM_STS
        cmp     gr5,gr0,icc0
        bne     icc0,0,30b              // Wait for SDRAM ready                        
	call	40f

	
40:	

// Clear all AMPR registers. They must not overlap.	
	movgs	gr0,DAMPR0
	movgs	gr0,DAMPR1
	movgs	gr0,DAMPR2
	movgs	gr0,DAMPR3
	movgs	gr0,DAMPR4
	movgs	gr0,DAMPR5
	movgs	gr0,DAMPR6
	movgs	gr0,DAMPR7
	movgs	gr0,IAMPR0
	movgs	gr0,IAMPR1
	movgs	gr0,IAMPR2
	movgs	gr0,IAMPR3
	movgs	gr0,IAMPR4
	movgs	gr0,IAMPR5
	movgs	gr0,IAMPR6
	movgs	gr0,IAMPR7
	
	li	0x000000C9,gr4		// Set 0x0XXXXXXX supervisor only, cache - SDRAM
	movgs	gr4,DAMPR1
	
	li	0xF000007D,gr4		// Set 0xF0XXXXXX supervisor only, no cache - chip selects (16MB)
	movgs	gr4,DAMPR2

	li	0x200000BD,gr4		// Set 0x2XXXXXXX supervisor only, no cache - Motherboard resources
	movgs	gr4,DAMPR6

	LED 0x1003

	li	0x100000BD,gr4		// Set 0x1XXXXXXX supervisor only, no cache - PCI bridge
	movgs	gr4,DAMPR7

	andi	gr8,#8,gr7	// Check CPU type again. 0x8 is FR5xx
	cmpi	gr7,#0,icc0
	beq	icc0,#0,70f

// And the rest, for FR555...	
	movgs	gr0,IAMPR8
	movgs	gr0,IAMPR9
	movgs	gr0,IAMPR10
	movgs	gr0,IAMPR11
	movgs	gr0,IAMPR12
	movgs	gr0,IAMPR13
	movgs	gr0,IAMPR14
	movgs	gr0,IAMPR15
	movgs	gr0,DAMPR8
	movgs	gr0,DAMPR9
	movgs	gr0,DAMPR10
	movgs	gr0,DAMPR11
	movgs	gr0,DAMPR12
	movgs	gr0,DAMPR13
	movgs	gr0,DAMPR14
	movgs	gr0,DAMPR15

	movgs	gr0,DAMLR1
	li	0x20000000,gr4
	movgs	gr4,DAMLR6
	li	0x10000000,gr4
	movgs	gr4,DAMLR7

70:
	LED 0x1004
	movsg	hsr0,gr4
	li	(1<<25),gr5		// Enable data MMU
	or	gr4,gr5,gr4
	movgs	gr4,hsr0
	LED 0x1005
        .endm                
                
#endif // _CYGONCE_PLATFORM_INC_H_
