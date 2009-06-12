#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	DDB-VRC4373 board assembler header file
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
## Author(s): 	nickg
## Contributors:	nickg
## Date:	1999-04-06
## Purpose:	VRC4373 board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the VRC4373 board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/mips.inc>

##-----------------------------------------------------------------------------
## VRC4372 registers
			
#define CYGHWR_HAL_MIPS_VRC4373_BASE            0xbc000000
#define CYGHWR_HAL_MIPS_VRC4373_INTC_POL        (CYGHWR_HAL_MIPS_VRC4373_BASE+0x200)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_TRIG       (CYGHWR_HAL_MIPS_VRC4373_BASE+0x204)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_PINS       (CYGHWR_HAL_MIPS_VRC4373_BASE+0x208)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_MASK0      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x20c)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_STAT0      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x210)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_MASK1      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x214)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_STAT1      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x218)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_MASK2      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x21c)
#define CYGHWR_HAL_MIPS_VRC4373_INTC_STAT2      (CYGHWR_HAL_MIPS_VRC4373_BASE+0x220)
	
##-----------------------------------------------------------------------------
## configure the architecture HAL to define the right things.

## ISR tables are defined in platform.S
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

## VSR table is at a fixed RAM address defined by the linker script
#define CYG_HAL_MIPS_VSR_TABLE_DEFINED

##-----------------------------------------------------------------------------

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_PMON)

## Initial SR value for use with PMON:
## CP0 usable
## Vectors to RAM
## All hw ints disabled
#define INITIAL_SR_PLF	0x10000000	

#elif defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

## Initial SR value for use with GDB stubs:
## CP0 and CP1 usable
## FP registers are 64 bit	
## Vectors to RAM
## All hw ints disabled
#define INITIAL_SR_PLF	0x34000000	
	
#else	

## Initial SR value for use standalone:
## CP0 usable
## Vectors to RAM
## All hw ints disabled
#define INITIAL_SR_PLF	0x10000000	

#endif

#------------------------------------------------------------------------------
## Load Address and Relocate. This macro is used in code that may be
## linked to execute out of RAM but is actually executed from ROM. The
## code that initializes the memory controller and copies the ROM
## contents to RAM must work in this way, for example. This macro is used
## in place of an "la" macro instruction when loading code and data
## addresses.  There are two versions of the macro here. The first
## assumes that we are executing in the ROM space at 0xbfc00000 and are
## linked to run in the RAM space at 0x80000000.  It simply adds the
## difference between the two to the loaded address.  The second is more
## code, but will execute correctly at either location since it
## calculates the difference at runtime.  The second variant is enabled
## by default.


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
	subu	\reg,\reg,ra		# subtract it from value
	bal	x\@			# branch and link to label
	nop				#  to get current actual address
x\@:
	addu	\reg,\reg,ra		# add actual address
	move	ra,$at			# restore ra
	.set	at
	.endm

#endif

#define	CYGPKG_HAL_MIPS_LAR_DEFINED

#endif			
	
#------------------------------------------------------------------------------
# MMU macros.
# The MMU must be set up on this board before we can access any external devices,
# including the memory controller, so we have no RAM to work with yet.
# Since the setup code must work only in registers, we do not do a subroutine
# linkage here, instead the setup code knows to jump back here when finished.
	
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.macro	hal_mmu_init
	.extern	hal_mmu_setup
	lar	k0,hal_mmu_setup
	jr	k0
        nop
	.global hal_mmu_setup_return
hal_mmu_setup_return:
	.endm

#define CYGPKG_HAL_MIPS_MMU_DEFINED

#endif	

#------------------------------------------------------------------------------
# MEMC macros.
# 
	
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.macro	hal_memc_init
	.extern	hal_memc_setup
	lar	k0,hal_memc_setup
	jalr	k0
	nop

#if defined(CYG_HAL_STARTUP_ROMRAM)
	# Having got the RAM working, we must now relocate the Entire
	# ROM into it and then continue execution from RAM.

	la	t0,reset_vector		# dest addr
	lar	t1,reset_vector		# source addr
	la	t3,__ram_data_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	sw	v0,0(t0)		# write word
	addiu	t1,t1,4
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

#endif	

#------------------------------------------------------------------------------
# Interrupt controller initialization.

	# initialize all interrupts to disabled
	.macro	hal_intc_init
	mfc0	v0,status
	nop
	la	v1,0xFFFF00FF
	and	v0,v0,v1		# clear the IntMask bits
	ori	v0,v0,0x3800		# set 3 IPL bits
	mtc0	v0,status
	nop
	nop
	nop
	# mask them all in the VRC4372 interrupt controller too,
	# and write zeros to the status registers to clear any
	# pending interrupts.
	la	v0,CYGHWR_HAL_MIPS_VRC4373_INTC_MASK0
	sw	zero,0(v0)
	sw	zero,4(v0)
	sw	zero,8(v0)
	sw	zero,12(v0)
	sw	zero,16(v0)
	sw	zero,20(v0)
	.endm

#define CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

#------------------------------------------------------------------------------
# Interrupt Translator.
# This translates an interrupt number into an ISR table offset. Vector 0
# contains a special ISR for dealing with spurious interrupts from the
# Vrc437x, and vectors 1-3 contain springboards, so we chain via vector 4.
# This macro translates interrupt 0 to vector 0 and all others to vector 4.

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	beqz	\inum,1f	# jump if interrupt is zero
	 move	v0,zero		# set v0=0 in delay slot
	addi	v0,v0,4		# non zero vector, inc v0
1:	move	\vnum,v0	# store 0 or 4 in vnum
	.endm
#define CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#endif
#endif

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED


	.macro	hal_mon_init

	hal_mon_copy_trampoline

	hal_mon_init_vsr_table

	.endm


#if defined(CYGSEM_HAL_USE_ROM_MONITOR_PMON)

	# Copy the other_vector trampoline code into the RAM
	# area so we intercept all interrupts.

	.macro	hal_mon_copy_trampoline
	la	a0,other_vector
	la	a1,other_vector_end
	la	t0,0xa0000180
1:
	lw	v0,0(a0)
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
	.endm


	# plant a pointer to the breakpoint springboard into the
	# correct vsr table slot.

	.macro	hal_mon_init_vsr_table
	.extern hal_breakpoint_springboard
	la	v1,hal_vsr_table

	# Plant the interrupt VSR
	la	v0,__default_interrupt_vsr
	sw	v0,(0*4)(v1)

	# And the breakpoint VSR
	la	v0,hal_breakpoint_springboard
	sw	v0,(9*4)(v1)

	# Temporarily also plant all the others, so all exceptions
	# go to PMON.
	sw	v0,(4*4)(v1)
	sw	v0,(5*4)(v1)
	sw	v0,(6*4)(v1)
	sw	v0,(7*4)(v1)
	sw	v0,(8*4)(v1)
	sw	v0,(10*4)(v1)
	sw	v0,(11*4)(v1)
	sw	v0,(12*4)(v1)
	sw	v0,(13*4)(v1)
	sw	v0,(14*4)(v1)
	sw	v0,(15*4)(v1)

 
	.endm

#elif defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

	# The stubs have a trampoline of their own installed which
	# already goes through the VSR table.

	.macro	hal_mon_copy_trampoline
	.endm

	# plant a pointer to the interrupt VSR handler in the
	# correct vsr table slot. Leave the rest for the monitor.

	.macro	hal_mon_init_vsr_table
	la	v0,__default_interrupt_vsr
	la	v1,hal_vsr_table
	sw	v0,(0*4)(v1)

	# plant a pointer to our own bus error handler. See the
	# comments in platform.S.
	.extern	hal_bus_error_vsr
	la	v0,hal_bus_error_vsr
	sw	v0,(7*4)(v1)

	.endm

#else
	# The other_vector trampoline is already installed as part of
	# the executable image. However, the TLB exception is, in RAM
	# in the analogous place to the reset vector in ROM. In a
	# ROM or ROMRAM startup we need to copy it into place.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
	.macro	hal_mon_copy_trampoline
	la	a0,utlb_vector
	la	a1,utlb_vector_end
	la	t0,0xa0000000
1:
	lw	v0,0(a0)
	sw	v0,0(t0)
	sw	v0,0x80(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
#if defined(CYG_HAL_STARTUP_ROM)
	la	a0,other_vector
	la	a1,other_vector_end
	la	t0,0xa0000180
1:
	lw	v0,0(a0)
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
#endif
	.endm
#else
	.macro	hal_mon_copy_trampoline
	.endm
#endif

	# Fill the VSR table with the default VSRs.
	# If we contain the stubs, the default VSR will pass
	# exceptions on to the stubs.

	.macro	hal_mon_init_vsr_table
	la	v0,__default_exception_vsr
	la	v1,hal_vsr_table

	sw	v0,(1*4)(v1)
	sw	v0,(2*4)(v1)
	sw	v0,(3*4)(v1)
	sw	v0,(4*4)(v1)
	sw	v0,(5*4)(v1)
	sw	v0,(6*4)(v1)
#	sw	v0,(7*4)(v1)	# Bus error
	sw	v0,(8*4)(v1)
	sw	v0,(9*4)(v1)
	sw	v0,(10*4)(v1)
	sw	v0,(11*4)(v1)
	sw	v0,(12*4)(v1)
	sw	v0,(13*4)(v1)
	sw	v0,(14*4)(v1)
	sw	v0,(15*4)(v1)
	sw	v0,(23*4)(v1)
	sw	v0,(24*4)(v1)
#	sw	v0,(32*4)(v1)	# debug
	sw	v0,(33*4)(v1)	# utlb
	sw	v0,(34*4)(v1)	# nmi

	la	v0,__default_interrupt_vsr
	sw	v0,(0*4)(v1)

	# plant a pointer to our own bus error handler. See the
	# comments in platform.S.
	.extern	hal_bus_error_vsr
	la	v0,hal_bus_error_vsr
	sw	v0,(7*4)(v1)

	.endm

#endif


#define CYGPKG_HAL_MIPS_MON_DEFINED

#endif	

#------------------------------------------------------------------------------
# Diagnostic macros


#ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED

#if 0

	# This code generates characters and hex values to a
	# Grammar Engine PromICE AI interface.

#define AILOC	0xbfc70000

	.macro	hal_diag_init
	la	v0,AILOC
1:	
	lbu	v1,3(v0)
	nop ; nop ; nop
	subu	v1,0xCC
	beqz	v1,1b
	nop
	lbu	v1,2(v0)

	b	9f
	nop

	.global	hal_diag_ai_write_char
hal_diag_ai_write_char:	

	.set	noat
	la	v0,AILOC	# v0 = AI location
		
1:
	lbu	v1,3(v0)	# v1 = status register
	nop ; nop ; nop
	andi	v1,v1,1		# v1 = TDA bit
	bnez	v1,1b		# loop while non-zero
	nop
	
	sll	a0,a0,1		# a0 = a0<<1
	ori	a0,a0,0x0201	# or in start and stop bits
	li	$at,10		# we have 10 bits to send
2:
	andi	v1,a0,1		# v1 = ls bit of char
	add	v1,v0,v1	# v1 = address of ZERO or ONE register
	lbu	zero,0(v1)	# read it to send bit
	la	v1,100		# delay a bit to let PROMICE deal with it
3:	bnez	v1,3b		# loop while non-zero
	add	v1,v1,-1	# decrement in delay slot
	srl	a0,a0,1		# a0 = a0>>1
	subu	$at,1		# decrement count
	bnez	$at,2b		# loop while non-zero
	nop

	jr	ra		# all done, return
	nop

	.set	at

	.global hal_diag_ai_write_hex1
hal_diag_ai_write_hex1:
	la	v0,9
	andi	a0,a0,0xf
	ble	a0,v0,1f
	nop
	addi	a0,a0,('A'-'9'-1)
1:	addi	a0,a0,'0'
	b	hal_diag_ai_write_char
	nop

	.global hal_diag_ai_write_hex2
hal_diag_ai_write_hex2:
	move	t0,ra		# save ra
	move	t1,a0		# save arg
	srl	a0,a0,4		# ms nibble
	
	bal	hal_diag_ai_write_hex1
	nop
	
	move	a0,t1		# retrieve a0
	move	ra,t0		# retrieve ra
	b	hal_diag_ai_write_hex1
	nop

	.global hal_diag_ai_write_hex4
hal_diag_ai_write_hex4:
	move	t2,ra		# save ra
	move	t3,a0		# save arg
	srl	a0,a0,8		# ms byte
	
	bal	hal_diag_ai_write_hex2
	nop
	
	move	a0,t3		# retrieve a0
	move	ra,t2		# retrieve ra
	b	hal_diag_ai_write_hex2
	nop


	.global hal_diag_ai_write_hex8
hal_diag_ai_write_hex8:
	move	t4,ra		# save ra
	move	t5,a0		# save arg
	srl	a0,a0,16	# ms short
	
	bal	hal_diag_ai_write_hex4
	nop
	
	move	a0,t5		# retrieve a0
	move	ra,t4		# retrieve ra
	b	hal_diag_ai_write_hex4
	nop

	
9:
	# Output a '!' to check that the interface is working

	li	a0,'!'
	bal	hal_diag_ai_write_char
	nop

	.endm

	# Utility macro to emit a character
	.macro	hal_diag_writec char
	.extern hal_diag_ai_write_char
	la	a0,\char
	lar	v0,hal_diag_ai_write_char
	jalr	v0
#	bal	hal_diag_ai_write_char
	nop
	.endm

#if 0
	# This macro outputs a '+', the exception number as a
	# character offset from 'A' and the exception address
	# in hex.
	.macro	hal_diag_excpt_start
	hal_diag_writec '+'
	srl	k0,k0,2
	addi	a0,k0,'A'
	jal	hal_diag_ai_write_char
	nop
	move	a0,t6			# we know t6 contains the epc value
	jal	hal_diag_ai_write_hex8
	nop
	.endm
#else
	.macro	hal_diag_excpt_start
	.endm
#endif

#if 0
	# This macro outputs a '=' and the vector number as a
	# character offset from 'A'.
	.macro	hal_diag_intr_start
	.extern hal_diag_ai_write_char
	hal_diag_writec '='
	addi	a0,s2,'A'
	jal	hal_diag_ai_write_char
	nop
	.endm
#else
	.macro	hal_diag_intr_start
	.endm
#endif

#if 0
	.macro	hal_diag_restore
	hal_diag_writec '^'
	lw	a0,mipsreg_pc(sp)
	lar	k0,hal_diag_ai_write_hex8
	jalr	k0
	.endm
#else
	.macro	hal_diag_restore
	.endm
#endif

#define CYGPKG_HAL_MIPS_DIAG_DEFINED
		
#elif 0

#define DELAY(n) \
        li      $at,n;		\
9:	bnez    $at,9b;	 	\
        subu    $at,1; 		\

 /* Zilog Access Delay */
#define DELZ 		DELAY( (200) )


	.macro	hal_diag_init
	la	v0,0xc2000000
	DELZ
	lbu	v1,8(v0)
	andi	v1,v1,0xfc
	DELZ
	sb	v1,8(v0)
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	la	v0,0xc2000000
	DELZ
	lbu	v1,8(v0)
	xori	v1,v1,0x01
	DELZ
	sb	v1,8(v0)
	.endm

	.macro	hal_diag_restore
	la	v0,0xc2000000
	DELZ
	lbu	v1,8(v0)
	xori	v1,v1,0x01
	DELZ
	sb	v1,8(v0)

#	li	a0,0x0310			# a0 = type = INTR,RAISE
#	lw	a1,mipsreg_sr(sp)		# a1 = sr
#	mfc0	a2,status
#	jal	cyg_instrument			# call instrument function
#	nop

	.endm

#define CYGPKG_HAL_MIPS_DIAG_DEFINED


#endif

#endif	



#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
