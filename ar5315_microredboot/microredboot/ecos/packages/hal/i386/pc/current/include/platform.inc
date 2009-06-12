#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##      platform.inc
##
##      PC platform support
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
## Author(s):   jskov
## Contributors:jskov, pjo, nickg
## Date:        1999-01-07
## Purpose:     PC platform support
## Description: This file contains any PC specific assembler macros needed to
##		run eCos on a standard i386 PC.
##
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/i386.inc>

#include <cyg/hal/pcmb.inc>	

##=============================================================================
## CPU initialization

#ifndef CYGPKG_HAL_I386_CPU_INIT_DEFINED
	
#define CYGPKG_HAL_I386_CPU_INIT_DEFINED

##=============================================================================
## ROM and GRUB startup
##
## Although these two startup types are, on the face of it, very different,
## the actual work that needs to be done here for them both is much the same.
## In both cases the system has been initialized in real mode and the transition 
## to 32 bit protected mode has been done. Here all we need to do is set up
## our own GDT and IDT, reload the segment registers and proceed.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_GRUB)

	.macro	hal_cpu_init

hal_cpu_init_start:

	# Disable interrupt handling.
	cli

	# Make a GDT pointer in location 0 and load new GDTR
	movw	$(gdtEnd - gdtStart),%ax
	movw	%ax,0
	leal	gdtStart,%eax
	movl	%eax,2
	lgdt	0

	# Make an IDT pointer in location 0 and load new LDTR
	movw	$0x77F,%ax
	movw	%ax,0
	leal	idtStart,%eax
	movl	%eax,2	
	lidt	0

	# Jump long to reload CS
	jmpl	$8,$3f

	.align	4, 0xFF	
gdtStart:
	/* Selector 0x00 == invalid. */
	.word	0x0000
	.word	0x0000
	.byte	0x00
	.byte	0x00
	.byte	0x00
	.byte	0x00

	/* Selector 0x08 == code. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xCF
	.byte	0x00

	/* Selector 0x10 == data. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xCF
	.byte	0x00

	/* Selector 0x18 == shorter code: faults any code 
         * access 0xF0000000-0xFFFFFFFF.
         */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xC7
	.byte	0x00

	/* Selector 0x20 == data; faults any access 0xF0000000-0xFFFFFFFF. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xC7
	.byte	0x00

	.align	4, 0xFF
gdtEnd:

#if defined(CYG_HAL_STARTUP_GRUB)
        # Multiboot header -- this must be in the first 8k 
        # of the image file.

#define GRUB_MAGIC      0x1BADB002      /* Magic number */
#define GRUB_FLAGS      0x00000000      /* Flags        */
        
        .align	4, 0xFF        
        .long   GRUB_MAGIC
        .long   GRUB_FLAGS
        .long   0-GRUB_MAGIC-GRUB_FLAGS /* Checksum */
#endif

3:
	# Set up data selectors
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	movw	%ax, %ss
        
        # Set up SP
	movl	$__interrupt_stack,%esp
	
	/* Reset the flags register. */
	push	$0
	popf

#if defined(CYG_HAL_STARTUP_GRUB)
        # Save the multiboot info passed in %EBX away into a
	# global variable for later use.
        .data
        .global hal_multiboot_info
hal_multiboot_info:     
        .long   0
        .text
        movl    %ebx,hal_multiboot_info
#endif       
        
hal_cpu_init_end:
        nop

	.endm	
#endif	
	
	
##=============================================================================
## RAM startup	
	
#ifdef CYG_HAL_STARTUP_RAM

	.macro	hal_cpu_init
	.endm
	
#endif /* CYG_HAL_STARTUP_RAM */

#endif // CYGPKG_HAL_I386_CPU_INIT_DEFINED

##=============================================================================
## IDT support
		
	.macro set_IDT_entry idx,addr
	pushl	$(idtStart+8*\idx) 
	pushl	$\addr
	.extern cyg_hal_pc_set_idt_entry
	call	cyg_hal_pc_set_idt_entry
	addl	$8,%esp
	.endm

  

	.macro hal_idt_init
	.text	
hal_idt_init:
	set_IDT_entry  0, hal_pc_exception_0
	set_IDT_entry  1, hal_pc_exception_1
	set_IDT_entry  2, hal_pc_exception_2
	set_IDT_entry  3, hal_pc_exception_3
	set_IDT_entry  4, hal_pc_exception_4
	set_IDT_entry  5, hal_pc_exception_5
	set_IDT_entry  6, hal_pc_exception_6
	set_IDT_entry  7, hal_pc_exception_7
	set_IDT_entry  8, hal_pc_exception_8
	set_IDT_entry  9, hal_pc_exception_9
	set_IDT_entry  10, hal_pc_exception_10
	set_IDT_entry  11, hal_pc_exception_11
	set_IDT_entry  12, hal_pc_exception_12
	set_IDT_entry  13, hal_pc_exception_13
	set_IDT_entry  14, hal_pc_exception_14
	set_IDT_entry  15, hal_pc_exception_15
	set_IDT_entry  16, hal_pc_exception_16
	set_IDT_entry  17, hal_pc_exception_17
	set_IDT_entry  18, hal_pc_exception_18
	set_IDT_entry  19, hal_pc_exception_19
	set_IDT_entry  20, hal_pc_exception_20
	set_IDT_entry  21, hal_pc_exception_21
	set_IDT_entry  22, hal_pc_exception_22
	set_IDT_entry  23, hal_pc_exception_23
	set_IDT_entry  24, hal_pc_exception_24
	set_IDT_entry  25, hal_pc_exception_25
	set_IDT_entry  26, hal_pc_exception_26
	set_IDT_entry  27, hal_pc_exception_27
	set_IDT_entry  28, hal_pc_exception_28
	set_IDT_entry  29, hal_pc_exception_29
	set_IDT_entry  30, hal_pc_exception_30
	set_IDT_entry  31, hal_pc_exception_31
	set_IDT_entry  32, hal_pc_irq_32
	set_IDT_entry  33, hal_pc_irq_33
	set_IDT_entry  34, hal_pc_irq_34
	set_IDT_entry  35, hal_pc_irq_35
	set_IDT_entry  36, hal_pc_irq_36
	set_IDT_entry  37, hal_pc_irq_37
	set_IDT_entry  38, hal_pc_irq_38
	set_IDT_entry  39, hal_pc_irq_39
	set_IDT_entry  40, hal_pc_irq_40
	set_IDT_entry  41, hal_pc_irq_41
	set_IDT_entry  42, hal_pc_irq_42
	set_IDT_entry  43, hal_pc_irq_43
	set_IDT_entry  44, hal_pc_irq_44
	set_IDT_entry  45, hal_pc_irq_45
	set_IDT_entry  46, hal_pc_irq_46
	set_IDT_entry  47, hal_pc_irq_47
#ifdef CYGPKG_HAL_SMP_SUPPORT
	set_IDT_entry  48, hal_pc_irq_48
	set_IDT_entry  49, hal_pc_irq_49
	set_IDT_entry  50, hal_pc_irq_50
	set_IDT_entry  51, hal_pc_irq_51
	set_IDT_entry  52, hal_pc_irq_52
	set_IDT_entry  53, hal_pc_irq_53
	set_IDT_entry  54, hal_pc_irq_54
	set_IDT_entry  55, hal_pc_irq_55
	set_IDT_entry  56, hal_pc_irq_56
	set_IDT_entry  57, hal_pc_irq_57
	set_IDT_entry  58, hal_pc_irq_58
	set_IDT_entry  59, hal_pc_irq_59
	set_IDT_entry  60, hal_pc_irq_60
	set_IDT_entry  61, hal_pc_irq_61
	set_IDT_entry  62, hal_pc_irq_62
	set_IDT_entry  63, hal_pc_irq_63

	set_IDT_entry  64, hal_pc_irq_64
	set_IDT_entry  65, hal_pc_irq_65
	set_IDT_entry  66, hal_pc_irq_66
	set_IDT_entry  67, hal_pc_irq_67
#endif
	.endm
	
##=============================================================================
#endif // ifndef CYGONCE_HAL_PLATFORM_INC	
## end of platform.inc
