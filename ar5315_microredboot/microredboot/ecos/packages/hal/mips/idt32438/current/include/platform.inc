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

#include "idt32438.h"
#include "idt32438Sio.h"
#define K1BASE 0xa0000000


# Macro for copying vectors to RAM if necessary.

	.macro	hal_vectors_init
	.endm



#----------------------------------------------------------------------
# Serial port initialization


	.macro hal_serial_init

	.endm

#
#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM)
	# If we are using a ROM version, include code to
	# initialize PLLs and SDRAM.

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro	hal_reset_vector_first_code
	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

# MEMDELAY is slow as it's running uncached - not perfect but good enough.
#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop

	# Initialize SDRAM configuration.
	.macro	hal_memc_init
	
	# If we're executing from flash, try to set up SDRAM
	/*  Disable WatchDog Timer  */
	 
	li	t0, ERR_CNTL_STATUS
	lw	t1, 0(t0)
	li	t2, ERR_CNTL_VALUE
	and	t1, t2
	sw	t1, 0(t0)
    	li	t0, TIMER_BASE_ADDR
	li	t1, DISABLE_TIMER
	li	t0, WTC_CTL_REG
	sw	t1, 0(t0)
	
	/* Reduce Bus Timeout Count */
    
	li	t1, TIMEOUT_COUNT
	li	t0, BT_TIMER_COMPARE
	sw	t1, 0(t0)
	
	/* Program The GPIO Registers */
	
	li	t1, GPIO_DATA_REG
	sw	zero, 0(t1)
	li	t1, GPIO_INT_LVL
	sw 	zero, 0(t1)

	/*Initialize CS0 - either EPROM or flash */

	li	t0, DEV_CTL_BASE     
	li	t1, MCR_CS0_BS       
	sw	t1, DEV0BASE_OFF(t0)
	li	t1, MCR_CS0_MASK
	sw	t1, DEV0MASK_OFF(t0)
	li	t1, MCR_CS0_CTRL
	sw	t1, DEV0C_OFF(t0)
	li	t1, MCR_CS0_TC
	sw	t1, DEV0TC_OFF(t0)
	
	/*Initialize CS1 - either flash or EPROM  */	
 	
 	li	t0, DEV_CTL_BASE     
	li	t1, MCR_CS1_BS       
	sw	t1, DEV1BASE_OFF(t0)
	li	t1, MCR_CS1_MASK
	sw	t1, DEV1MASK_OFF(t0)
	li	t1, MCR_CS1_CTRL
	sw	t1, DEV1C_OFF(t0)
	li	t1, MCR_CS1_TC
	sw	t1, DEV1TC_OFF(t0)
	
	/* Initialize CS2 - 8 bit port */

	li	t0, DEV_CTL_BASE     
	li	t1, MCR_CS2_BS       
	sw	t1, DEV2BASE_OFF(t0)
	li	t1, MCR_CS2_MASK
	sw	t1, DEV2MASK_OFF(t0)
	li	t1, MCR_CS2_CTRL
	sw	t1, DEV2C_OFF(t0)
	li	t1, MCR_CS2_TC
	sw	t1, DEV2TC_OFF(t0)   

	/* Initialize CS3 - 16 bit port */
	
	li	t0, DEV_CTL_BASE     
	li	t1, MCR_CS3_BS       
	sw	t1, DEV3BASE_OFF(t0)
	li	t1, MCR_CS3_MASK
	sw	t1, DEV3MASK_OFF(t0)
	li	t1, MCR_CS3_CTRL
	sw	t1, DEV3C_OFF(t0)
	li	t1, MCR_CS3_TC
	sw	t1, DEV3TC_OFF(t0)
	
	/*	GPIO Controller setting */ 
	 
	li	t0, PIO_FUNCSEL_REG
	li	t1, PIO_FUNCSEL_VAL
	sw	t1, 0x0(t0)
	
	/* DDR Initialization starts here */
	
	li	t0, DDRD_LLC_REG             /* Hidden register in RC32438    */
	li	t1, DDRD_LLC_VAL
	sw	t1, 0x0(t0)
	li    	t1, 0x0		             /* Add 200 microseconds of delay */
	li    	t2, DELAY_200USEC
1:
	add   	t1, 1
	bne   	t1, t2, 1b
	nop
	
	/* Load the DDR Controller Base Address */
	
	li	t0, DDR_BASE
	li 	t1, DDRC_VAL_AT_INIT         /*load DDRC reset refresh enable */
	sw	t1, 0x10(t0)
	sw 	zero, 4(t0)
	sw	zero, 0xc(t0)
	sw	zero, 0x18(t0)

	/* Store DDR0BASE */
	
	li	t1, DDR0_BASE_VAL
	sw	t1, 0x0(t0)

	/* Store DDR0MASK */
	
	li	t1, DDR0_MASK_VAL
	sw	t1, 0x4(t0)

	/* Load DDR1MASK to disable CS1 */
	
	li	t1, DDR1_BASE_VAL
	sw	t1, 0x08(t0)
	
	li	t1, DDR1_MASK_VAL
	sw	t1, 0x0C(t0)

	/* Load DDR0AMASK to disable alternate Mapping */
	
	li	t1, DDR0_BASE_VAL
	sw	t1, 0x14(t0)
	
	li	t1, DDR0_AMASK_VAL
	sw	t1, 0x18(t0)
	li	t1, DDR_CUST_NOP
	sw	t1, 0x20(t0)
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0000000 | DDR0_BASE_VAL
	sw 	t2, 0x0(t1)
	
	li	t1, 0x0
	li	t2, DELAY_200USEC
1:
	add	t1, 1
	bne	t1, t2, 1b
	nop


	/* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
	
	li	t1, DDR_CUST_PRECHARGE
	sw	t1, 0x20(t0)      /* Write to DDR Custom transaction register */

	 
	/* Generate A10 high to pre-charge both the banks */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0001000 | DDR0_BASE_VAL
	sw	t2, 0x0(t1)


	/* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
	
	li	t1, DDR_LD_EMODE_REG
	sw	t1, 0x20(t0)      /* Write to DDR Custom transaction register */

	 

	/* Generate EMODE register contents on A12-A0 */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0000000 | DDR_EMODE_VAL | DDR0_BASE_VAL
	sw	t2, 0x0(t1)
	
	 

	/* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
	
	li	t1, DDR_LD_MODE_REG
	sw	t1, 0x20 (t0)     /* Write to DDR Custom transaction register */

	/* Generate Mode register contents on the address bus A12-A0  */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0000000 | DDR_DLL_RES_MODE_VAL | DDR0_BASE_VAL
	sw	t2, 0x0(t1)

	/* Delay of  1.6 microseconds ~ 300 delay iteration value */
	
	li	t1, 0x0
	li	t2, 500

1:
	add	t1, 1
	bne	t1, t2, 1b
	nop

	/* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
	
	li	t1, DDR_CUST_PRECHARGE
	sw	t1, 0x20(t0)      /* Write to DDR Custom transaction register */
 
	/* Generate A10 high to pre-charge both the banks */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0001000 | DDR0_BASE_VAL
	sw	t2, 0x0(t1)

	/* Implements 9 cycles of Auto refresh allowing sufficient margin for
	 * stability
	 */
	 
	li	t4, 9
	li	t3, 0
1:
	li	t1, DDR_CUST_REFRESH
	sw	t1, 0x20(t0)      /* Write to DDR Custom transaction register */

	/* Read it back to flush CPU write buffers */
	
	lw    t1, 0x20(t0)	 
	
	/* Perform a write to DDR space to register the command  */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0000000 | DDR0_BASE_VAL
	sw	t2, 0x0(t1)
	
	add	t3, 1
	bne	t3, t4, 1b
	nop

	/* Register t0 carries pointer to the DDR_BASE: 0xB8018000 */
	
	li	t1, DDR_LD_MODE_REG
	sw	t1, 0x20(t0)      /* Write to DDR Custom transaction register */
 

	/* Generate Mode Register contents on the address bus A12-A0 */
	
	li	t2, DATA_PATTERN
	li	t1, 0xA0000000 | DDR_DLL_MODE_VAL | DDR0_BASE_VAL
	sw	t2, 0x0(t1)



	/* Post DDR SDRAM initialization code :
	 * Initialize the refresh timer with fast refresh count 
	 */
	 
	li	t0, RCOUNT  

	li	t1, DDR_REF_CMP_FAST
	
	/* Set the RCOMPARE register */
	
	sw	t1, 0x4(t0)
 

	/* Enable the Refresh timer */
	
	li	t1, 0x1             /* CE set to enabled the  Refresh counter */
	sw	t1, 0x08(t0)


      /* Enable RE-refresh enable in the DDRC register */

	li	t0, DDR_BASE
	li	t1, DDRC_VAL_NORMAL
	sw	t1, 0x10(t0)
 

	
	/* Add 200 microseconds of delay */
	
	li	t1, 0x0
	li	t2, DELAY_200USEC
1:
	add	t1, 1
	bne	t1, t2, 1b
	nop
 
 	/* Disable the refresh counter before changing the compare value */
 
 	li	t0, RCOUNT 
	li	t1, 0x0
	sw	t1, 0x08(t0)

	/* Set the RCOMPARE register */
      
	li	t1, DDR_REF_CMP_VAL
	sw	t1, 0x4(t0)

	/* Enable the Refresh timer */
	
	li	t1, 0x1             /* CE set to enabled the  Refresh counter */
	sw	t1, 0x08(t0)

	/* Add 200 microseconds of delay */

	li	t1, 0x0
	li	t2, DELAY_200USEC

1:
	add	t1, 1
	bne	t1, t2, 1b
	nop
	nop

	/* This completes the DDR controller and the DDR SDRAM initialization 
	 * and the DDR memory is ready for use.
	 */
	.endm
#endif /* CYG_HAL_STARTUP_ROM */

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
	.endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
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
