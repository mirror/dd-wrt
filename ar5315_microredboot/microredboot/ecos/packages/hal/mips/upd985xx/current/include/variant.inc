#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
//#=============================================================================
//#
//#	variant.inc
//#
//#	UPD985XX assembler header file
//#
//#=============================================================================
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
//#=============================================================================
//######DESCRIPTIONBEGIN####
//#
//# Author(s): 	hmt, nickg
//# Contributors:	nickg
//# Date:	2001-05-24
//# Purpose:	UPD985XX definitions.
//# Description:	This file contains various definitions and macros that are
//#              useful for writing assembly code for the UPD985XX CPU.
//# Usage:
//#		#include <cyg/hal/variant.inc>
//#		...
//#		
//#
//#####DESCRIPTIONEND####
//#
//#=============================================================================

        .set    mips3

#include <cyg/hal/mips.inc>

#include <cyg/hal/platform.inc>

#include <pkgconf/hal.h>

#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/arch.inc>	

#include <cyg/hal/var_arch.h>
#include <cyg/hal/var_intr.h>

// ------------------------------------------------------------------------
// Basic LED and poke-uart level debugging stuff.

#if 0
	.macro DELAY dd
	la	k0,\dd
98:	addiu	k0,k0,-1
	bnez	k0,98b
	 nop
	.endm

	.macro LED nn
	la	k1,0xb0000000
	la	k0,0xfdff
	sw	k0,0x20(k1)
	nop
	nop
	nop
	la	t0,\nn
99:	
	la	k0,0x100
	sw	k0,0x24(k1)

	DELAY 0x40000

	la	k0,0x000
	sw	k0,0x24(k1)
	  
	DELAY 0x30000

	addi	t0,t0,-1
	bnez	t0,99b
	 nop

	DELAY 0xd0000

	.endm

	.macro PRINT nn
	la	k1,0xb0000000
	DELAY 0x10000
	la	k0, '.'
	sw	k0,0x80(k1)
	DELAY 0x10000
	la	k0, 0xf & ((\nn) >> 4)
	addi	k0,k0,'0'
	sw	k0,0x80(k1)
	DELAY 0x10000
	la	k0, 0xf & ((\nn))
	addi	k0,k0,'0'
	sw	k0,0x80(k1)
	DELAY 0x10000
	la	k0, '.'
	sw	k0,0x80(k1)
	.endm	

	.macro DEBUG nn
	#LED \nn
	PRINT \nn
	.endm

#else

	.macro DEBUG nn
	.endm

#endif

##-----------------------------------------------------------------------------
## configure the architecture HAL to define the right things.

## ISR tables are larger than the defaults defined in vectors.S
## We define our own in var_misc.c
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

## VSR table is at a fixed RAM address defined by the linker script
#define CYG_HAL_MIPS_VSR_TABLE_DEFINED

//------------------------------------------------------------------------------
// Set up initial value for config register. Sets endian mode and
// disables the cache on kseg0.

#if defined(CYGPKG_HAL_MIPS_MSBFIRST)
# define	INITIAL_CONFIG0	***!!!Nope, it should be little-endian!!!***
#elif defined(CYGPKG_HAL_MIPS_LSBFIRST)
# define	INITIAL_CONFIG0	0x00000002
#else
# error MIPS endianness not set by configuration
#endif

//------------------------------------------------------------------------------
// Initial SR value for use standalone and with GDB_stubs:
// CP0 usable
// Vectors to RAM
// All hw ints disabled

#define INITIAL_SR	0x10000000	

#------------------------------------------------------------------------------
# Cache macros.
	

#ifndef CYGPKG_HAL_MIPS_CACHE_DEFINED

	.macro	hal_cache_init

	mfc0	v0,config0		# disable Kseg0 caching in config0 register
	nop
	nop
	la	v1,0xfffffff8
	and	v0,v0,v1
	ori	v0,v0,2
	mtc0	v0,config0
	nop
	nop
	nop

	// If we invalidate the caches in RAM startup, this destroys
	// network debugging == the network device.
	// Don~t fully understand why, but this exclusion fixes it.
	// Invalidating caches could destroy other RedBoot state so we
	// shouldn~t do it anyway.
#ifndef CYG_HAL_STARTUP_RAM

        .set mips3                      # Set ISA to MIPS 3 to allow cache insns

        // Now ensure the caches are invalidated. The caches are NOT cleared or
	// invalidated on non-power-up resets and may come up in a random state
	// on power-up. Hence they may contain stale or randomly bogus data.
	// Here we use the index-store-tag cache operation to clear all the cache
	// tags and states to zero. This will render them all invalid on the
	// VR4300.

        # D-cache:
        la      t0,0x80000000
        addi    t1,t0,0x2000
1:
	mtc0	zero,$28
	mtc0	zero,$29
        cache   0x09,0(t0)
        addi    t0,t0,0x10
        sub     v0,t1,t0
        bgez    v0,1b
         nop

	# I-cache:
        la      a0,0x80000000
        addi    a1,a0,0x4000
1:
	mtc0	zero,$28
	mtc0	zero,$29
        cache   0x08,0(a0)
        addi    a0,a0,0x20
        sub     v0,a1,a0
        bgez    v0,1b
         nop

        .set mips0                      # reset ISA to default

#endif

	.endm

#define CYGPKG_HAL_MIPS_CACHE_DEFINED

#endif	

//-----------------------------------------------------------------------------
// Load Address and Relocate. This macro is used in code that may be
// linked to execute out of RAM but is actually executed from ROM. The
// code that initializes the memory controller and copies the ROM
// contents to RAM must work in this way, for example. This macro is used
// in place of an "la" macro instruction when loading code and data
// addresses.  There are two versions of the macro here. The first
// assumes that we are executing in the ROM space at 0xbfc00000 and are
// linked to run in the RAM space at 0x80000000.  It simply adds the
// difference between the two to the loaded address.  The second is more
// code, but will execute correctly at either location since it
// calculates the difference at runtime.  The second variant is enabled
// by default.


#ifdef CYG_HAL_STARTUP_ROMRAM

#if 0
	.macro	lar	reg,addr
	.set	noat
	la	\reg,\addr
	la	$at,0x3fc00000
	addu	\reg,\reg,$at
	.set	at
	.endm
#else
	.macro	lar	reg,addr
	.set	noat
	move	$at,ra			# save ra
	la	\reg,\addr		# get address into register
	la	ra,x\@			# get linked address of label
	subiu	\reg,\reg,ra		# subtract it from value
	bal	x\@			# branch and link to label
	nop				#  to get current actual address
x\@:
	addiu	\reg,\reg,ra		# add actual address
	move	ra,$at			# restore ra
	.set	at
	.endm

#endif

#define	CYGPKG_HAL_MIPS_LAR_DEFINED

#endif			
	
//----------------------------------------------------------------------------
// MMU macros.
// The MMU must be set up on this board before we can access any external devices,
// including the memory controller, so we have no RAM to work with yet.
// Since the setup code must work only in registers, we do not do a subroutine
// linkage here, instead the setup code knows to jump back here when finished.
	
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

// There is none.  We can access all areas via kseg[01] so we are happy
// with no MMU setup.

// NO #define CYGPKG_HAL_MIPS_MMU_DEFINED

#endif	

//----------------------------------------------------------------------------
// MEMC macros.
// 
	.macro	hal_memc_init

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.extern	hal_memc_setup
	lar	k0,hal_memc_setup
	jalr	k0
	nop

#endif

#if defined(CYG_HAL_STARTUP_ROMRAM)

	# Having got the RAM working, we must now relocate the Entire
	# ROM into it and then continue execution from RAM.

	la	t0,0x80000000		# dest addr
	la	t1,0xbfc00000		# source addr
	la	t3,__ram_data_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addiu	t1,t1,4
	sw	v0,0(t0)		# write word
	addiu	t0,t0,4
	bne	t0,t3,1b
	nop

	la	v0,2f			# RAM address to go to
	jr	v0
	nop
2:	
	# We are now executing out of RAM!

#endif

	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

#------------------------------------------------------------------------------
# Interrupt controller initialization.

#ifndef CYGPKG_HAL_MIPS_INTC_DEFINED

#ifndef CYGPKG_HAL_MIPS_INTC_INIT_DEFINED
	# initialize all interrupts to disabled
	.macro	hal_intc_init
	mfc0	v0,status
	nop
	lui	v1,0xFFFF
	ori	v1,v1,0x00FF
	and	v0,v0,v1		# clear the IntMask bits
	mtc0	v0,status
	nop
	nop
	nop
	.endm
#endif

#ifndef CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
	.macro	hal_intc_decode vnum
	mfc0	v1,status		# get status register (interrupt mask)
	nop				# delay slot
	mfc0	v0,cause		# get cause register
	nop				# delay slot
	and	v0,v0,v1		# apply interrupt mask
	andi    v1,v0,0x0300		# test for soft interrupt bits
	beqz    v1, 43f			# neither of them
	 srl	v1,v1,8			# shift interrupt bits down
	addi	v1,v1,-1		# now have 1,2,3 in v1 -> 0,1,2
	andi	\vnum,v1,1		# -> 0,1,0 is the right answer
	b	44f
	 nop
43:
	srl	v0,v0,10		# shift interrupt bits down
	andi	v0,v0,0x3f		# isolate 6 interrupt bits
	la	v1,hal_intc_translation_table
	add	v0,v0,v1		# index into table
	lb	\vnum,0(v0)		# pick up vector number
	addi	\vnum,\vnum,2		# offset soft intrs
44:	nop
	.endm
#endif

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	move	\vnum,zero			# Just vector zero is supported
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	move	\vnum,\inum			# Vector == interrupt number
	.endm
#endif
#endif

	.macro	hal_intc_decode_data
hal_intc_translation_table:	
	.byte	0, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	3, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	4, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	3, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	5, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	3, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	4, 0, 1, 0
	.byte	2, 0, 1, 0
	.byte	3, 0, 1, 0
	.byte	2, 0, 1, 0
	.endm

#define CYGPKG_HAL_MIPS_INTC_DEFINED

#endif // CYGPKG_HAL_MIPS_INTC_DEFINED


#------------------------------------------------------------------------------
# Monitor initialization.


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
	sw	v0,0x0000(t0)		# write word to utlb vec
	sw	v0,0x0080(t0)		# and also to xtlb vector (64-bit)
	addi	t0,t0,4                 #         (no harm done)
	bne	t1,t3,1b
	 nop

	la	t0,0x80000000		# dest addr page base
	la	t1,other_vector		# source addr
	la	t3,other_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0x0180(t0)		# write word to other vector
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

	// We are running uncached here anyhow, so no need to flush caches

	.endm

#else

	.macro	hal_vectors_init
	.endm

#endif


#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init

	hal_vectors_init

        # Set default exception VSR for all vectors
        ori     a0,zero,64 // not CYGNUM_HAL_VSR_COUNT at all
	la	a1,__default_exception_vsr
	la	a2,hal_vsr_table
1:      sw      a1,0(a2)
        addi    a2,a2,4
        addi    a0,a0,-1
        bne     a0,zero,1b
         nop

        # Now set special VSRs

        # FIXME: Should use proper definitions

        la	a0,hal_vsr_table
        # Set interrupt VSR
        la	a1,__default_interrupt_vsr
        sw      a1,0*4(a0) // CYGNUM_HAL_VECTOR_INTERRUPT
        # Add special handler on breakpoint vector to allow GDB and
        # GCC to both use 'break' without conflicts.
        la	a1,__break_vsr_springboard
        sw	a1,9*4(a0) // CYGNUM_HAL_VECTOR_BREAKPOINT

        # Set exception handler on special vectors
// but these are already set up above:
//	  la	a1,__default_exception_vsr
//	  sw	a1,32*4(a0)             # debug
//	  sw	a1,33*4(a0)             # utlb
//	  sw	a1,34*4(a0)             # nmi
        .endm
	
#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	hal_vectors_init
        # Set interrupt VSR
       	la	a0,hal_vsr_table
        la	a1,__default_interrupt_vsr
        sw      a1,0*4(a0) // CYGNUM_HAL_VECTOR_INTERRUPT
	.endm

#else

	.macro	hal_mon_init
	hal_vectors_init
	.endm

#endif

#define CYGPKG_HAL_MIPS_MON_DEFINED
#endif	

#------------------------------------------------------------------------------
# Diagnostic macros


#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
