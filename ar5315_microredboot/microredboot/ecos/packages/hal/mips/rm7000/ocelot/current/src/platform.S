##=============================================================================
##
##      platform.S
##
##      MIPS Ocelot platform code
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
## Purpose:     MIPS Ocelot platform code
## Description: Platform specific code for Ocelot board.
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
#include <cyg/hal/hal_misc.h>

##-----------------------------------------------------------------------------
## MEMC initialization.

## This code is table driven, which is somewhat more compact that coding it 
## all.
## Table entries consist of an address and a value to store in that address.
## A zero address terminates the table. Two special address values modify the
## behaviour:   
## DELAY_LOOP   loops for the number of iterations in the value field.
## WRITE16      treats the next 2 words as an address and value to be written 
##              with a 16 bit write cycle.

#define HAL_GALILEO_CONTROLLER_BASE 0xb4000000
#define GL_CPU_CFG           0xb4000000
#define GL_CPU_CFG_WRITERATE 0x00010000
        
#define GL_DEV_PAR_BANK0     0xb400045c
#define GL_DEV_PAR_BANK1     0xb4000460
#define GL_DEV_PAR_BANK2     0xb4000464
#define GL_DEV_PAR_BANK3     0xb4000468
#define GL_DEV_PAR_BOOT      0xb400046c

#define GL_SDRAM_CFG         0xb4000448
#define GL_SDRAM_CFG_REF_INT_CNT_default 0x00000200
#define GL_SDRAM_CFG_CPU_TO_DRAM_ERR     0x00020000
#define GL_SDRAM_CFG_ECC_INT             0x00040000
#define GL_SDRAM_CFG_DUP_BA              0x00100000
#define GL_SDRAM_CFG_REG_SDRAM           0x00800000
#define GL_SDRAM_CFG_DADR_12_SEL         0x01000000
#define GL_SDRAM_CFG_init (GL_SDRAM_CFG_REF_INT_CNT_default| \
	                   GL_SDRAM_CFG_CPU_TO_DRAM_ERR|     \
	                   GL_SDRAM_CFG_ECC_INT|             \
	                   GL_SDRAM_CFG_DUP_BA|              \
	                   GL_SDRAM_CFG_REG_SDRAM|           \
	                   GL_SDRAM_CFG_DADR_12_SEL)

#define GL_SDRAM_PAR_BANK0   0xb400044c
#define GL_SDRAM_PAR_BANK2   0xb4000454
#define GL_CPU_SCS10_LO      0xb4000008
#define GL_CPU_SCS10_HI      0xb4000010
#define GL_CPU_SCS32_LO      0xb4000018
#define GL_CPU_SCS32_HI      0xb4000020

#define GL_SDRAM_SCS0_LO     0xb4000400
#define GL_SDRAM_SCS0_HI     0xb4000404
#define GL_SDRAM_SCS1_LO     0xb4000408
#define GL_SDRAM_SCS1_HI     0xb400040c
#define GL_SDRAM_SCS2_LO     0xb4000410
#define GL_SDRAM_SCS2_HI     0xb4000414
#define GL_SDRAM_SCS3_LO     0xb4000418
#define GL_SDRAM_SCS3_HI     0xb400041c

#define GL_SDRAM_MODE        0xb4000474
#define GL_SDRAM_MODE_NORMAL 0x00000000
#define GL_SDRAM_MODE_ENABLE 0x00000003

#define PLD_BOARD_STAT       0xbc000004
#define PLD_INT_SET          0xbc00000c
#define PLD_INT_CLEAR        0xbc00000d
#define PLD_INT_SET_USER_LED 0x40

#define CPU_TO_LE(x) (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x >> 24) & 0xff))

#define HAL_OCELOT_RAM_BASE                      0x00000000

#define HAL_GALILEO_CPU_DECODE_SHIFT            21
#define HAL_GALILEO_DEV_DECODE_SHIFT            20
		     
#define HAL_GALILEO_SCS10_LD_OFFSET             0x008
#define HAL_GALILEO_SCS10_HD_OFFSET             0x010
#define HAL_GALILEO_SCS32_LD_OFFSET             0x018
#define HAL_GALILEO_SCS32_HD_OFFSET             0x020

#define HAL_GALILEO_SCS0_LD_OFFSET              0x400
#define HAL_GALILEO_SCS0_HD_OFFSET              0x404
#define HAL_GALILEO_SCS1_LD_OFFSET              0x408
#define HAL_GALILEO_SCS1_HD_OFFSET              0x40c
#define HAL_GALILEO_SCS2_LD_OFFSET              0x410
#define HAL_GALILEO_SCS2_HD_OFFSET              0x414
#define HAL_GALILEO_SCS3_LD_OFFSET              0x418
#define HAL_GALILEO_SCS3_HD_OFFSET              0x41c

#if !defined(CYG_HAL_STARTUP_RAM)
        
#define DELAY_LOOP      1
#define WRITE16         2       


#define swap(v, t1, t2, t3)			\
	lui  t1,0xff;				\
	andi t2,v,0xff00;			\
	and  t3,v,t1;				\
	sll  t1,v,0x18;				\
	srl  v,v,0x18;				\
	or   v,v,t1;				\
	srl  t3,t3,0x08;			\
	sll  t2,t2,0x08;			\
	or   v,v,t3;				\
	or   v,v,t2;
	

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
         addiu   a1,-1          # decrement in delay slot
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

	// Complete the SDRAM setup.
	la	t0,GL_SDRAM_MODE
	la	t1,CPU_TO_LE(GL_SDRAM_MODE_ENABLE)
	sw	t1,0(t0)
	//  Wait 100us after writing mode
	la	a1,(CYGHWR_HAL_MIPS_RM7000_CPU_CLOCK * 100 / 1000000)

	lui	a0,0xa000	// uncached bank0 device
	sw	zero,0(a0)

	mtc0	zero,$9
1:	mfc0	a0,$9
	nop
	blt	a0, a1, 1b
	 nop

	lui	a0,0xa080	// uncached bank2 device
	sw	zero,0(a0)

	mtc0	zero,$9
1:	mfc0	a0,$9
	nop
	blt	a0, a1, 1b
	 nop

	la	t0,GL_SDRAM_MODE
	la	t1,CPU_TO_LE(GL_SDRAM_MODE_NORMAL)
	sw	t1,0(t0)

	// Set CPU cfg word
        la      a0,GL_CPU_CFG
	lw	a1,0(a0)
	la	t0,CPU_TO_LE(0x20000)
	or	a1,a1,t0	// disable retries

	// CPU WriteRate - set GT to match CPU
	mfc0	t0,$16
	la	t1, 0x0f000000
	and	t0, t0, t1
	beqz	t0,1f		// defaults to zero
	 nop
	la	t0, CPU_TO_LE(GL_CPU_CFG_WRITERATE)
	or	a1, a1, t0
1:	
	sw	a1,0(a0)

	# FIXME: Hardwired for 256MB - need kuseg magic to map full 512MB.
	#
	# Setup SDRAM Bank 0 Address Decoding
	#
	la	s7, HAL_GALILEO_CONTROLLER_BASE
	la	a0, HAL_OCELOT_RAM_BASE		        # Phys bottom of Bank 0
	la      a1, (HAL_OCELOT_RAM_BASE+128*1024*1024)
	subu	a1, 1					# Phys top of Bank0

	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT	# Setup SCS[1:0]
	swap(t0,t1,t2,t3)                               #   1st level decoding
	sw	t0, HAL_GALILEO_SCS10_LD_OFFSET(s7)	#   (CPU Decode Region)
	srl	t0, a1, HAL_GALILEO_CPU_DECODE_SHIFT	#
	swap(t0,t1,t2,t3)
	sw	t0, HAL_GALILEO_SCS10_HD_OFFSET(s7)	#

	srl	t0, a0, HAL_GALILEO_DEV_DECODE_SHIFT	# Setup SCS0
	swap(t0,t1,t2,t3)                               #   2nd level decoding
	sw	t0, HAL_GALILEO_SCS0_LD_OFFSET(s7)	#   (Device Sub-decode Region)
	srl	t0, a1, HAL_GALILEO_DEV_DECODE_SHIFT	#
	swap(t0,t1,t2,t3)
	sw	t0, HAL_GALILEO_SCS0_HD_OFFSET(s7)	#

	#
	# Setup SDRAM Bank 1 Address Decoding
	#
	la      a0, (HAL_OCELOT_RAM_BASE+128*1024*1024) # Phys bottom of Bank 1
	la      a1, (HAL_OCELOT_RAM_BASE+256*1024*1024)
	subu	a1, 1					# Phys top of Bank1

	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT	# Setup SCS[3:2]
	swap(t0,t1,t2,t3)                               #   1st level decoding
	sw	t0, HAL_GALILEO_SCS32_LD_OFFSET(s7)	#   (CPU Decode Region)
	srl	t0, a1, HAL_GALILEO_CPU_DECODE_SHIFT	#
	swap(t0,t1,t2,t3)
	sw	t0, HAL_GALILEO_SCS32_HD_OFFSET(s7)	#

	srl	t0, a0, HAL_GALILEO_DEV_DECODE_SHIFT	# Setup SCS2
	swap(t0,t1,t2,t3)                               #   2nd level decoding
	sw	t0, HAL_GALILEO_SCS2_LD_OFFSET(s7)	#   (Device Sub-decode Region)
	srl	t0, a1, HAL_GALILEO_DEV_DECODE_SHIFT	#
	swap(t0,t1,t2,t3)
	sw	t0, HAL_GALILEO_SCS2_HD_OFFSET(s7)	#

        jr      ra
         nop

FUNC_END(hal_memc_setup)
        
##-----------------------------------------------------------------------------
## The initialization table

hal_memc_setup_table:

        .long   DELAY_LOOP,       10000 # Wait for HW to settle

	.long	GL_DEV_PAR_BANK0, CPU_TO_LE(0x000b6b33) // PLD
	.long   GL_DEV_PAR_BANK1, CPU_TO_LE(0x000bad53) // NVRAM
	.long   GL_DEV_PAR_BANK2, CPU_TO_LE(0x000bad54) // UART
	.long	GL_DEV_PAR_BANK3, CPU_TO_LE(0x000fef73) // flash disk
	.long	GL_DEV_PAR_BOOT,  CPU_TO_LE(0x144bee63) // boot flash

	.long   GL_SDRAM_CFG, CPU_TO_LE(GL_SDRAM_CFG_init)
	.long   GL_SDRAM_PAR_BANK0, CPU_TO_LE(0x00000c69 | 0x4000)
	.long   GL_SDRAM_PAR_BANK2, CPU_TO_LE(0x00000c69 | 0x4000)

	.long   GL_CPU_SCS10_LO, CPU_TO_LE(0x00000000)
	.long   GL_CPU_SCS10_HI, CPU_TO_LE(0x00000003)
	.long   GL_CPU_SCS32_LO, CPU_TO_LE(0x00000004)
	.long   GL_CPU_SCS32_HI, CPU_TO_LE(0x00000007)

	.long	GL_SDRAM_SCS0_LO, CPU_TO_LE(0x00000000)
	.long	GL_SDRAM_SCS0_HI, CPU_TO_LE(0x00000007)
	.long	GL_SDRAM_SCS1_LO, CPU_TO_LE(0x000000ff)
	.long	GL_SDRAM_SCS1_HI, CPU_TO_LE(0x00000000)
	.long	GL_SDRAM_SCS2_LO, CPU_TO_LE(0x00000008)
	.long	GL_SDRAM_SCS2_HI, CPU_TO_LE(0x0000000f)
	.long	GL_SDRAM_SCS3_LO, CPU_TO_LE(0x000000ff)
	.long	GL_SDRAM_SCS3_HI, CPU_TO_LE(0x00000000)

        .long   0, 0
                
#endif // !CYG_HAL_STARTUP_RAM
        
##-----------------------------------------------------------------------------
## ISR tables.
                        
        .extern hal_default_isr
        
        .data

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        .long   hal_default_isr         # chaining vector
        
        .globl  hal_interrupt_data
hal_interrupt_data:
        .long   0                       # chaining vector data

        .globl  hal_interrupt_objects
hal_interrupt_objects:
        .long   0                       # chaining vector object

        .globl  cyg_hal_interrupt_level
cyg_hal_interrupt_level:
        .rept   CYGNUM_HAL_ISR_COUNT
        .byte   0
        .endr

#else // CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        
        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr
        .long   hal_default_isr

        .rept   CYGNUM_HAL_ISR_COUNT-10
        .long   hal_default_isr
        .endr
        
        .globl  hal_interrupt_data
hal_interrupt_data:
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0

        .rept   CYGNUM_HAL_ISR_COUNT-10
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
