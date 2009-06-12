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

// Display a value in the system LEDs
        .macro  LED n
        sethi   #(_FRV400_MB_LEDS>>16),gr15
        setlo   #(_FRV400_MB_LEDS&0xFFFF),gr15
        setlos  #\n,gr14
        not     gr14,gr14
        sti     gr14,@(gr15,0)
        .endm

// Platform initialization - only the necessary bits required to get the
// board started from a cold reset.
        .macro  platform_init
        li      0x7FFF,gr4      // First, a good long spin
05:     nop
        subi    gr4,1,gr4
        cmp     gr4,gr0,icc0
        bne     icc0,0,05b        
        call    10f             // position independent way to get @_platform_tab
_platform_tab:
//
// SDRAM setups
//
        .long   _FRV400_SDRAM_BR0,0x00000000    // SDRAM 0x0XXXXXXX
        .long   _FRV400_SDRAM_AM0,0x0FF00000
  
//      
// LOCAL bus setups
//
        .long   _FRV400_LBUS_CR0,0x03010D01     // ROM/FLASH 0xFF000000..0xFFFFFFFF
                                // 16 bits wide, 13 wait states, 1 idle

        .long   _FRV400_LBUS_BR1,0x10000000     // PCI bridge 0x10000000..0x100FFFFF
        .long   _FRV400_LBUS_AM1,0x000FFFFF
        .long   _FRV400_LBUS_CR1,0x00000000

        .long   _FRV400_LBUS_BR2,0x20000000     // SRAM, FPGA, PCI 0x20000000..0x2FFFFFFF
        .long   _FRV400_LBUS_AM2,0x0FFFFFFF
        .long   _FRV400_LBUS_CR2,0x00000000

        .long   _FRV400_LBUS_BR3,0x00000000     // SDRAM?
        .long   _FRV400_LBUS_AM3,0xFFFFFFFF
        .long   _FRV400_LBUS_CR3,0x00000F07

	.long	_FRV400_GPIO_SIR,0x000c954f     // Routing for Rx0, Rx1, CTS
	.long	_FRV400_GPIO_SOR,0x00036ab0     // Routing for Tx0, Tx1, RTS, TOUT0, TOUT1
        
        .long   _FRV400_SDRAM_CTL,0x05022000    // SDRAM mode/control
        .long   _FRV400_SDRAM_AN0,0x00010101
        .long   _FRV400_SDRAM_ART,0x00000820
        .long   _FRV400_SDRAM_RCN,0x00000000
        .long   _FRV400_SDRAM_MS, 0x00020200
        .long   _FRV400_SDRAM_CFG,0x80000000

//?        .long   _FRV400_CLK_CTRL,0x00000001     // External clock divisor (/2)
  
//      
// PCI controller/bridge
//
	.long	_FRV400_PCI_SLBUS_CONFIG,    0x000800E2		// This matches the docs
//	.long	_FRV400_PCI_SLBUS_CONFIG,    0x000000E0		// This matches the samples
	.long	_FRV400_PCI_ECS0_CONFIG,     0x00000000
	.long	_FRV400_PCI_ECS1_CONFIG,     0x000003C1
	.long	_FRV400_PCI_ECS2_CONFIG,     0x000001C1
	.long	_FRV400_PCI_ECS0_RANGE,      0x00000000
	.long	_FRV400_PCI_ECS0_ADDR,       0x00000000
	.long	_FRV400_PCI_ECS1_RANGE,      0x00007FFE         
	.long	_FRV400_PCI_ECS1_ADDR,       0x08108000          
	.long	_FRV400_PCI_ECS2_RANGE,      0x00007FFE
	.long	_FRV400_PCI_ECS2_ADDR,       0x08100000
	.long	_FRV400_PCI_PCIIO_RANGE,     0x0001FFFE
	.long	_FRV400_PCI_PCIIO_ADDR,      0x00120000         
	.long	_FRV400_PCI_PCIMEM_RANGE,    0x0003FFFE       
	.long	_FRV400_PCI_PCIMEM_ADDR,     0x00140000
	.long	_FRV400_PCI_PCIIO_PCI_ADDR,  0x24000001     
	.long	_FRV400_PCI_PCIMEM_PCI_ADDR, 0x28000000    
	.long	_FRV400_MB_PCI_ARBITER,      0x00000001
	.long	_FRV400_MB_PCI_ARBITER,      0x00000001
        
	.long	_FRV400_PCI_SLBUS_CONFIG, 0x800800E2
//	.long	_FRV400_PCI_SLBUS_CONFIG, 0x800000E0
        .long   0
	.long	_FRV400_SDRAM_STS
        
10:     movsg   lr,gr4                  // _platform_tab -> list of initializations
20:     ldi     @(gr4,0),gr5            // Register
        ldi     @(gr4,4),gr6            // Value
        cmp     gr5,gr0,icc0            // End of list?
        beq     icc0,0,30f
        sti     gr6,@(gr5,0)
        addi    gr4,2*4,gr4
        bra     20b                     // Next item
30:     ldi     @(gr6,0),gr5		// gr6 == _FRV400_SDRAM_STS
        cmp     gr5,gr0,icc0
        bne     icc0,0,30b              // Wait for SDRAM ready                        

//
// Note: it is unclear from the documentation if this works at all.  There is no
// description of how these registers are searched and what would happen if they
// overlap.  If it turns out that they are not allowed to overlap, then this setup
// will have to be restructured.
//
	li	0x03F0003D,gr4		// Set 0x03FXXXXX supervisor only, no cache - PCI window (1MB)
	movgs	gr4,DAMPR0
	li	0x000000C9,gr4		// Set 0x0XXXXXXX supervisor only, cache - SDRAM
	movgs	gr4,DAMPR1
	li	0x200000BD,gr4		// Set 0x2XXXXXXX supervisor only, no cache - Motherboard resources
	movgs	gr4,DAMPR6
	li	0x100000BD,gr4		// Set 0x1XXXXXXX supervisor only, no cache - PCI bridge
	movgs	gr4,DAMPR7
	movsg	hsr0,gr4
	li	(1<<25),gr5		// Enable data MMU
	or	gr4,gr5,gr4
	movgs	gr4,hsr0

        .endm                
                
#endif // _CYGONCE_PLATFORM_INC_H_
