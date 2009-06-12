##=============================================================================
##
##      platform.S
##
##      MIPS REF4955-TX4955 platform code
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   nickg
## Contributors:nickg, jskov
## Date:        2000-05-09
## Purpose:     MIPS REF4955-TX4955 platform code
## Description: Platform specific code for REF4955-TX4955 board.
##
######DESCRIPTIONEND####
##
##=============================================================================

         .set    noreorder

#include <pkgconf/system.h>     
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>    
#endif
                        
#include <cyg/hal/arch.inc>     
#include <cyg/hal/plf_defs.inc>     

##-----------------------------------------------------------------------------
## ISR springboard.
## This routine decodes the interrupt from the various interrupt controllers
## and vectors to it.

        # On entry:
        # a0 = MIPS status register interrupt number (1,2 or 3)
        # a1 = ISR data value (= interrupt controller reg address)
        # a2 = saved reg dump ptr
        # s0 = saved reg dump ptr
        # s1 = vector table offset
        # s2 = interrupt number
        # a3,v0,v1 etc available for use

        .text
        
FUNC_START(hal_isr_springboard_pci)
        lbu     v0,0(a1)
        nor     v0,v0,v0
        la      a1,CYGARC_REG_PCI_MASK
        lbu     a2,0(a1)
        and     v0,v0,a2
        addi    a2,$0,CYGNUM_HAL_INTERRUPT_INTC_PCI_base
        b       hal_isr_springboard
         nop
FUNC_END(hal_isr_springboard_pci)
        
FUNC_START(hal_isr_springboard_io)
        lbu     v0,0(a1)
        nor     v0,v0,v0
        la      a1,CYGARC_REG_IO_MASK
        lbu     a2,0(a1)
        and     v0,v0,a2
        addi    a2,$0,CYGNUM_HAL_INTERRUPT_INTC_IO_base
        b       hal_isr_springboard
         nop
FUNC_END(hal_isr_springboard_io)
        
hal_isr_springboard_v320usc:
        lw      v0,0(a1)
        la      a1,CYGARC_REG_INT_CFG0
        lw      v1,0(a1)
        lw      a2,4(a1)                # CFG1
        or      v1,v1,a2
        lw      a2,8(a1)                # CFG2
        or      v1,v1,a2
        la      a1,CYGARC_REG_INT_CFG3
        lw      a2,0(a1)
        or      v1,v1,a2
        and     v0,v0,v1
        addi    a2,$0,CYGNUM_HAL_INTERRUPT_INTC_V320USC_base
        
FUNC_START(hal_isr_springboard)
                
        # The following code implements an ls bit index algorithm similar
        # to that in hal_lsbit_index() in hal_misc.c. 
        negu    v1,v0                           # v1 = -v0
        and     v1,v1,v0                        # v1 &= v0 [isolate ls bit]
        sll     v0,v1,16                        # v0 = v1<<16
        subu    v1,v0,v1                        # v1 = v0 - v1
        sll     a0,v1,6                         # a0 = v1<<6
        addu    v1,v1,a0                        # v1 += a0
        sll     a1,v1,4                         # a1 = v1<<4
        addu    v1,v1,a1                        # v1 += a1
        la      v0,hal_isr_springboard_table    # v0 = table address
        srl     v1,v1,26                        # v1 = v1>>26
        addu    v1,v1,v0                        # v1 = table entry address
        lb      a0,0(v1)                        # a0 = intc isr number
        
        add     s2,a0,a2                        # s2 = eCos isr number
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
hal_isr_springboard_chaining:
        # This serves as the __default_interrupt_isr entry-point in
        # chaning mode, thus ensuring that all interrupts from
        # vectors 0-5 eventually end up on the special CHAINING vector.
        # (See the hal_interrupt_handlers table)
        ori     s1,zero,CYGNUM_HAL_INTERRUPT_CHAINING*4 # s1 = chaining isr ix
#else
        sll     s1,s2,2                         # s1 = isr table index
#endif

        la      v1,hal_interrupt_handlers
        add     v1,v1,s1                        # v1 = isr handler address
        lw      v1,0(v1)                        # v1 = isr handler

        la      a1,hal_interrupt_data
        add     a1,a1,s1                        # a1 = address of data ptr
        lw      a1,0(a1)                        # a1 = data pointer

        move    a0,s2                           # pass interrupt number 

        jr      v1                              # jump to handler, return is to
                                                # default vsr already in ra
        nop

FUNC_END(hal_isr_springboard)

       
hal_isr_springboard_table:      
        .byte  -1,  0,  1, 12,  2,  6,  0, 13
        .byte   3,  0,  7,  0,  0,  0,  0, 14
        .byte  10,  4,  0,  0,  8,  0,  0, 25
        .byte   0,  0,  0,  0,  0, 21, 27, 15
        .byte  31, 11,  5,  0,  0,  0,  0,  0
        .byte   9,  0,  0, 24,  0,  0, 20, 26
        .byte  30,  0,  0,  0,  0, 23,  0, 19
        .byte  29,  0, 22, 18, 28, 17, 16,  0

        
##-----------------------------------------------------------------------------
## MEMC initialization.
## This also initializes the PCI bus and ISA bus bridge, so at the end of this
## we should have full access to all the memory and devices we need.
## This code is table driven, which is somewhat more compact that coding it all.
## Table entries consist of an address and a value to store in that address.
## A zero address terminates the table. Two special address values modify the
## behaviour:   
## DELAY_LOOP   loops for the number of iterations in the value field.
## WRITE16      treats the next 2 words as an address and value to be written 
##              with a 16 bit write cycle.
        
#if !defined(CYG_HAL_STARTUP_RAM)
        
#define DELAY_LOOP      1
#define WRITE16         2       

FUNC_START(hal_memc_setup)

        lar     t0,hal_memc_setup_table
        la      t1,0xbfc00000
        la      t2,DELAY_LOOP
        la      t3,WRITE16
1:      
        lw      a0,0(t0)        # next table entry
        lw      a1,4(t0)        # value to write
        addiu   t0,8            # go to next entry
        beq     a0,t2,2f        # Check for delay
        nop
        beq     a0,t3,3f        # Check for 16 bit write
        nop
        beqz    a0,9f           # zero terminates loop
        nop
        sw      a1,0(a0)        # write it
        lw      zero,0(t1)      # uncached read to flush write buffer
        b       1b
        nop
2:
        lw      zero,0(t1)      # uncached read to flush write buffer   
        bnez    a1,2b           # count down by value in a1
        addiu   a1,-1           # decrement in delay slot
        b       1b              # go back to loop
        nop
3:      
        lw      a3,0(t0)        # get next word
        addiu   t0,4            # skip it
        sh      a3,0(a1)        # store halfword        
        lw      zero,0(t1)      # uncached read to flush write buffer
        b       1b
        nop
9:
        jr      ra
        nop

FUNC_END(hal_memc_setup)        

        
##-----------------------------------------------------------------------------
## The initialization table

#define USC_LB_BUS_CFG    0xb800007c
#define USC_SDRAM_MA_CMD  0xb8000088
        
hal_memc_setup_table:

        .long   DELAY_LOOP,       10000 # Wait for HW to settle

#ifdef CYGPKG_HAL_MIPS_MSBFIRST
        # V320SC : big-endian, max bus watch timer
        .long   USC_LB_BUS_CFG, 0x04010ff0
#else
        .long   USC_LB_BUS_CFG, 0x04000ff0
#endif        
        .long   USC_SDRAM_MA_CMD, 0x00a70000
        .long   USC_SDRAM_MA_CMD, 0x00e70220

        .long   0, 0
                
#endif // !CYG_HAL_STARTUP_RAM
        
##-----------------------------------------------------------------------------
## ISR tables.
                        
        .extern hal_default_isr
        
        .data

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        # These first 6 vectors all go through a springboard for further
        # interrupt controller vector decoding before ending up on the
        # special chaining vector below.
        .long   hal_isr_springboard_v320usc
        .long   hal_isr_springboard_pci
        .long   hal_isr_springboard_chaining
        .long   hal_isr_springboard_chaining
        .long   hal_isr_springboard_io
        .long   hal_isr_springboard_chaining

        .long   hal_default_isr         # chaining vector
        
        .globl  hal_interrupt_data
hal_interrupt_data:
        .long   CYGARC_REG_INT_STAT
        .long   CYGARC_REG_PCI_STAT
        .long   0
        .long   0
        .long   CYGARC_REG_IO_STAT
        .long   0

        .long   0                       # chaining vector data

        .globl  hal_interrupt_objects
hal_interrupt_objects:
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0

        .long   0                       # chaining vector object

        .globl  cyg_hal_interrupt_level
cyg_hal_interrupt_level:
        .rept   CYGNUM_HAL_ISR_COUNT
        .byte   0
        .endr

#else // CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        
        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        .long   hal_isr_springboard_v320usc
        .long   hal_isr_springboard_pci
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_isr_springboard_io
        .long   hal_default_isr

        .rept   CYGNUM_HAL_ISR_COUNT-6
        .long   hal_default_isr
        .endr
        
        .globl  hal_interrupt_data
hal_interrupt_data:
        .long   CYGARC_REG_INT_STAT
        .long   CYGARC_REG_PCI_STAT
        .long   0
        .long   0
        .long   CYGARC_REG_IO_STAT
        .long   0

        .rept   CYGNUM_HAL_ISR_COUNT-6
        .long   0
        .endr

        .globl  hal_interrupt_objects
hal_interrupt_objects:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

        .globl  cyg_hal_interrupt_level
cyg_hal_interrupt_level:
        .rept   CYGNUM_HAL_ISR_COUNT
        .byte   0
        .endr

#endif // CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

##-----------------------------------------------------------------------------
## end of platform.S
