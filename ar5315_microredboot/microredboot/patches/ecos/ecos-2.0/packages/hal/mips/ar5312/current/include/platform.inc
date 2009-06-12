#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	AR5312 assembler header file
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
## Author(s): 	
## Contributors:
## Date:	2003-10-20
## Purpose:	AR5312 SOC definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AR5312 SOC
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
#include <cyg/hal/plf_io.h>

#include "ar531xreg.h"

#define K1BASE 0xa0000000

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
	la	t3,other_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

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

	.endm

#else

	.macro	hal_vectors_init
	.endm

#endif

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM)
	# If we are using a ROM version, include code to
	# initialize PLLs and SDRAM.

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro	hal_reset_vector_first_code

	# If this was an NMI, it probably came from the watchdog timer,
	# and we want to preserve state.  PLL should be initialized
	# alredy.
	mfc0	k0,status
	lui	k1,0x0008 /* NMI */
	and	k1,k1,k0
	bnez	k1,pllskip
	nop

	# Clear any lingering AHB errors by reading both addr registers
	li	t0, K1BASE | AR531X_PROCADDR
	lw	zero, 0(t0)
	li	t0, K1BASE | AR531X_DMAADDR
	lw	zero, 0(t0)

        # Must not access any MAC registers for at least 16MS after reset.
        # This delay is put here rather than after PLL initialization
        # because eventually we'll probably have to determine the input
        # clock frequency (in order to set CLOCKCTL1) by reading MAC
        # information.
        #
        # We're running uncached from flash at this point.  Every
        # flash read takes 64 cycles * 100ns (at 40MHz/4).
        li      t0, 4000 # conservative value -- 25.6 ms.
1:      sub     t0, 1
        bnez    t0, 1b
	nop

	li	a0, K1BASE|AR531X_REV
	lw	a0, 0(a0)
	andi 	a0,0xff
	beq 	a0,0x50,viperpll
	nop
	
	li	t0, K1BASE | AR531X_CLOCKCTL2
	lw	t1,(t0)
	bgez	t1,pllskip              # upper bit guaranteed non-0 at reset
	nop

viperpll:

	li	t0, K1BASE | AR531X_CLOCKCTL2
	lw	t1,(t0)
	beq	t1,0x0,pllskip              # upper bit guaranteed non-0 at reset
	nop
pllenable:

	beq a0,0x50,viper
	nop
	
	li	t0, K1BASE | AR531X_CLOCKCTL1
	li	t1, AR531X_CLOCKCTL1_SELECTION
	sw	t1, (t0)

	li	t0, K1BASE | AR5312_SCRATCH
	li	t1, AR531X_CPU_CLOCK_RATE
	sw	t1, (t0)

	li	t0, K1BASE | AR531X_CLOCKCTL2
        li      t1, AR531X_CLOCKCTL2_WANT_DIV2 | AR531X_CLOCKCTL2_WANT_RESET
	sw	t1, (t0)		       # causes a processor reset

viper:
	# For Viper 0xbc003064 register has to be programmed with 0x91000 to get 180Mhz Processor clock
        # Set /2 clocking and turn OFF AR531X_CLOCKCTL2_STATUS_PLL_BYPASS.
        # Processor RESETs at this point; the CLOCKCTL registers retain
        # their new values across the reset.
	li	t0, K1BASE | AR531X_CLOCKCTL1
	li 	t1, 0x91245
	sw 	t1,(t0)

	li	t0, K1BASE | AR531X_CLOCKCTL2
	li 	t1,0x1
	sw	t1, (t0)		       # causes a processor reset
1:      b       1b /* NOTREACHED */
	nop

pllskip:
	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

# MEMDELAY is slow as it's running uncached - not perfect but good enough.
#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop

	# Initialize SDRAM configuration.
	#   - The 256Mb mode is not supported.
	#   - Only 4 bank devices are supported.
	#
	.macro	hal_memc_init
	# If we're executing from flash, try to set up SDRAM

	li	t5, 0       # 0-->x32, MEM_CFG0_X-->x16
	li	t6, 0x22000 # 0x22000-->4 word burst, 0x23000-->8 word burst
	li	a0, K1BASE|AR531X_REV
	lw	a0, 0(a0)
	andi	a0,0xff
	bne	a0,0x50,99f
	nop
	
98:
	/* Configure for a x16 SDRAM with 8-word burst */
	li	t5, MEM_CFG0_X           # try x16
	li	t6, 0x23000              # use 8-word burst
99:
	li	a0, K1BASE | AR531X_MEM_CFG0
	li	a1, K1BASE | AR531X_MEM_CFG1
	li	a2, K1BASE | AR531X_MEM_REF

	/* 1) wait 100us - handled by latency to boot up to this point. */

	/* 2) Set the I and M bits to issue an SDRAM nop */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_M | MEM_CFG1_I
	sw	t0, 0(a1)		# MEM_CFG1

	/* 3) Wait 200us (roughly) */
	MEMDELAY(50, t2)

	/* 4) Reset the M bit to issue an SDRAM PRE-ALL */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_I
	sw	t0, 0(a1)		# MEM_CFG1
	sync

	/* 5) Generate a refresh every 16 clocks (spec says 10) */
	li	t0, 16			# very fast refresh for now
	sw	t0, 0(a2)		# MEM_REF

	/* 6) Wait for 128 clocks (8 refresh cycles) */
	MEMDELAY(5, t2)

	/*
	 * 7) Set operational value into the reset timer.
	 *    [out of order from ARM SDRAM ctrl spec]
	 */

	/* 8) Set command write mode, and read each SDRAM */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_M
	sw	t0, 0(a1)		# MEM_CFG1
	sync
	li	t0, K1BASE | AR531X_SDRAM0 
	or	t0, t6 # burst
	lw	zero, 0(t0)
	li	t0, K1BASE | AR531X_SDRAM1
	or	t0, t6 # burst
	lw	zero, 0(t0)

	/* 9) Program configuration register 0 */
	li	t0, MEM_CFG0_C | MEM_CFG0_C2 | MEM_CFG0_R1 | \
		    MEM_CFG0_B0 | MEM_CFG0_B1
	or	t0, t5 # x16 or x32
	sw	t0, 0(a0)		# MEM_CFG0
	sync

	li	t0, AR531X_SDRAM_MEMORY_REFRESH_VALUE
	sw	t0, 0(a2)		# MEM_REF
	sync

	/* 10) Clear I and M and set cfg1 to the normal operational value */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S)
	sw	t0, 0(a1)		# MEM_CFG1
	sync

        /* 11) Determine whether the SDRAM is x16 or x32 */
	li	a3, K1BASE|AR531X_SDRAM0 # Pick a memory address
	li	t0, 0x12345678           # Pick a data pattern to write
	lw	t1, 4(a3)                # Read the 2nd word
	sw	t0, 0(a3)                # Write the 1st word
	lw	t0, 4(a3)                # See of the second word changed
	beq	t0, t1, 1f               # If so, must be a x16 part
	nop

	b	98b

1:
	/* 12) The SDRAM is ready for normal operation */

	/* Size bank 0 - assumed to be present */
	li	a3, K1BASE|AR531X_SDRAM0
	sw	zero, 0(a3)		# base value
	li	t1, MEM_CFG1_AC_2	# min 2MB
	li	t3, 0x200000		# 2MB
1:
	or	a2, a3, t3		# construct probe address
	sw	t1, 0(a2)		# write it
	lw	t2, 0(a3)		# read back @ base
	bnez	t2, 2f			# alias detected?
	nop
	sll	t3, 1			# next address
	add	t1, 1			# next alias
	ble	t1, MEM_CFG1_AC_64, 1b	# 128MB not supported!
	nop
2:	sll	t0, t1, MEM_CFG1_AC0_S	# align for bank0
	or	t0, MEM_CFG1_E0 | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S)
	sw	t0, 0(a1)		# MEM_CFG1
	sync

	and	t0, ~(MEM_CFG1_E1 | MEM_CFG1_AC1)	# clear bank 1 enable

	/* Probe bank 1 as it is optional */
	li	a3, K1BASE|AR531X_SDRAM1
	li	t1, 0x5555aaaa		# 2 probe patterns
	li	t2, 0x3333cccc
	sw	t1, 0(a3)
	sw	t2, 4(a3)
	lw	t3, 0(a3)
	lw	t4, 4(a3)
	bne	t1, t3, nobank1
	nop
	bne	t2, t4, nobank1
	nop

	/* Size bank 1 */
	li	a3, K1BASE|AR531X_SDRAM1
	sw	zero, 0(a3)		# base value
	li	t1, MEM_CFG1_AC_2	# min 2MB
	li	t3, 0x200000		# 2MB
1:
	or	a2, a3, t3		# construct probe address
	sw	t1, 0(a2)		# write it
	lw	t2, 0(a3)		# read back @ base
	bnez	t2, 2f			# alias detectd?
	nop
	sll	t3, 1			# next address
	add	t1, 1			# next alias
	ble	t1, MEM_CFG1_AC_64, 1b	# 128MB not supported!
	nop
2:	sll	t1, MEM_CFG1_AC1_S	# align for bank1
	or	t0, t1			# combine with current value
	or	t0, MEM_CFG1_E1		# enable bank1

nobank1:
	sw	t0, 0(a1)		# MEM_CFG1
	sync
	.endm
#endif /* CYG_HAL_STARTUP_ROM */

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
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
	sw	a1,12*4(a3)
	sw	a1,13*4(a3)
	sw	a1,14*4(a3)
	sw	a1,15*4(a3)

	sw	a1,32*4(a3)
	sw	a1,33*4(a3)
	sw	a1,34*4(a3)
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

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
