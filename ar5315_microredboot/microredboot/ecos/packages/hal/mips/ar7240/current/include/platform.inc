#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	AR7240 assembler header file
##
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	
## Contributors:
## Date:	2003-10-20
## Purpose:	AR7240 SOC definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AR7240 SOC
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/mips.inc>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_cache.h>
#include <cyg/hal/plf_io.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
/* #include <cyg/hal/mips-regs.h> */
#include <cyg/hal/var_arch.h>

#include "ar7240_soc.h"

#------------------------------------------------------------------------------
# Macro for copying vectors to RAM if necessary.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)

	.macro	hal_vectors_init
	# If we don~t play nice with a ROM monitor, copy the required
	# vectors into the proper location.
	la	t0,0x80000000		# dest addr
	la	t1,utlb_vector		# source addr
	la	t3,utlb_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	nop


	la	t0,0x80000180		# dest addr
	la	t1,other_vector		# source addr
	la	t3,other_vector_end	# for vectored interrupts
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	nop

#if 0
	.set mips3                      # Set ISA to MIPS 3 to allow cache insns
	# Now clear the region in the caches
	la	t0,0x80000000		# dest addr
	ori	t1,t0,0x200		# source addr
1:	cache	0x01,0x0000(t0)	        # Flush word from data cache
	cache	0x01,0x1000(t0)
	cache	0x01,0x2000(t0)
	cache	0x01,0x3000(t0)
	nop
	cache	0x00,0x0000(t0)		# Invalidate icache for word
	cache	0x00,0x1000(t0)
	cache	0x00,0x2000(t0)
	cache	0x00,0x3000(t0)
	nop
	addi	t0,t0,0x20
	bne	t0,t1,1b
	 nop
	.set mips0                      # reset ISA to default
#endif

	.endm

#else

	.macro	hal_vectors_init
	.endm

#endif


#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
        defined(CYG_HAL_STARTUP_ROMRAM) ||              \
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
	hal_vectors_init
#if 1
	la	a0,__default_interrupt_vsr
	la	a1,__default_exception_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	sw	a1,1*4(a3)
	sw	a1,2*4(a3)
	sw	a1,3*4(a3)
	sw	a1,4*4(a3)
	sw	a1,5*4(a3)
	sw	a1,6*4(a3)
	sw	a1,7*4(a3)
	sw	a1,8*4(a3)
	sw	a1,9*4(a3)
	sw	a1,10*4(a3)
	sw	a1,11*4(a3)
	nop
	nop
	sw	a1,12*4(a3)
	sw	a1,13*4(a3)
	sw	a1,14*4(a3)
	sw	a1,15*4(a3)

	sw	a1,32*4(a3)
	sw	a1,33*4(a3)
	sw	a1,34*4(a3)
#endif
	.endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	la	a0,__default_interrupt_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	.endm

#else

	.macro	hal_mon_init
	.endm

#endif
			

#define CYGPKG_HAL_MIPS_MON_DEFINED

#endif	


#ifndef CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

        .macro  hal_intc_init
        mfc0    v0,status
        nop
        lui     v1,0xFFFF
        ori     v1,v1,0x04FF
        and     v0,v0,v1        # Clear the IntMask bits except IM[0]
        mtc0    v0,status
        nop
        nop
        nop
        .endm

#define CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

#endif

#ifndef CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
        .macro  hal_intc_decode vnum
        mfc0    v1,status                     # get status register (interrupt mask)
        mfc0    v0,cause                      # get cause register
        and     v0,v0,v1                      # apply interrupt mask
        andi    v1,v0,0x4000                  # test misc interrupt
        bnez    v1,decode0
        srl     v0,v0,10                      # shift interrupt bits down
        andi    v0,v0,0x3f                    # isolate 6 interrupt bits
        beqz    v0,ignore
        la      v1,hal_intc_translation_table
        add     v0,v0,v1                      # index into table
        lb      \vnum,0(v0)                   # pick up vector number
        b       decode2
        nop
    decode0:
        la      t0,CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_STATUS)                 # Get MISC interrupt status reg
        lw      v0,0(t0) 
        add     t0,t0,4
        lw      v1,0(t0)
        and     v0,v0,v1                      # apply interrupt mask
        andi    v0,v0,0xff
        clz     v1,v0                         # count leading zeros into v1
        la      v0,37                         # v0 = 31 + vector offset 6
        sub     \vnum,v0,v1                   # vnum = (31 + vector offset) -v1 == vector number
        nop
    decode2:
        .endm

        .macro  hal_intc_translate inum,vnum
        move    \vnum,\inum                     # Vector == interrupt number
        .endm

        .macro  hal_intc_decode_data
hal_intc_translation_table:
        .byte    0,  0,  1,  1
        .byte    2,  2,  2,  2
        .byte    3,  3,  3,  3
        .byte    3,  3,  3,  3
        .byte    4,  4,  4,  4
        .byte    4,  4,  4,  4
        .byte    4,  4,  4,  4
        .byte    4,  4,  4,  4
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .byte    5,  5,  5,  5
        .endm

#define CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
#define CYGPKG_HAL_MIPS_INTC_DEFINED

// We also define our own interrupt tables in platform.S...
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

#endif



#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
