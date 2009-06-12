##
#=============================================================================
##	platform.S
##
##	MIPS Atlas platform code
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
## Author(s):	dmoseley
## Contributors:	dmoseley
## Date:	2000-06-06
## Purpose:	MIPS Atlas platform code
## Description:	Platform specific code for Atlas board.
##
##
##
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

#include <cyg/hal/arch.inc>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_arch.h>

##-----------------------------------------------------------------------------

##-----------------------------------------------------------------------------
# Platform Initialization.
# This code performs platform specific initialization.

##-----------------------------------------------------------------------------
## MEMC initialization.
##

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.text
	.set	noreorder


.macro MASK_WRITE_PCI_REG regnum, devnum, mask
	.set noreorder
	# First, read the appropriate register
	li	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_ConfigEn | \regnum | \devnum
	sw	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_OFFSET(s7)
	lw	t1, HAL_GALILEO_PCI0_CONFIG_DATA_OFFSET(s7)

	# Now, mask in the appropriate bits
	li	t2, \mask
	or	t1, t2

	# Write the updated value
	li	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_ConfigEn | \regnum | \devnum
	sw	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCI0_CONFIG_DATA_OFFSET(s7)
.endm

.macro WRITE_PCI_REG regnum, devnum, base
	.set noreorder
	li	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_ConfigEn | \regnum | \devnum
	li	t1, \base
	sw	t0, HAL_GALILEO_PCI0_CONFIG_ADDR_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCI0_CONFIG_DATA_OFFSET(s7)
.endm

#define NO_MASK        0
#define NO_ERROR_CHECK 0
#define ERROR_CHECK    1
.macro READ_SPD_VALUE func, mask, ret_reg, err_check
	.set noreorder
	jal	read_spd_value
	li	a0, \func			 # delay slot
.if \err_check
	beq	v0, zero, error
	nop
.endif
	move	\ret_reg, v0
.if \mask
	and	\ret_reg, \mask
.endif
.endm

##-----------------------------------------------------------------------------
##
## Initialize the RAM.
##
## To do that, we need to first initialize the Galileo PCI stuff to gain access
## to the SAA9730.
## From there, use the I2C bus of the SAA9730 to read the SPD SDRAM
## config data. We then setup the Galileo SDRAM configuration
##
##  Returns
##  v0 = Error Code
##  v1 = SDRAM size
##
FUNC_START(hal_atlas_init_sdram)

	.set noreorder

	# Save the return address
	move	s8, ra

	# Setup the base address registers
	li	s7, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_REGISTER_BASE)

	# Setup the Galileo controller Endian configuration
	li	t0, (HAL_GALILEO_BYTE_SWAP)
	sw	t0, HAL_GALILEO_PCI_INTERNAL_COMMAND_OFFSET(s7)

	# Setup the PCI_0 Timeout and retry configuration
	li	t0, HAL_GALILEO_PCI0_TIMEOUT_RETRY_VALUE
	sw	t0, HAL_GALILEO_PCI0_TIMEOUT_RETRY_OFFSET(s7)

	# Setup Galileo as PCI Master
	MASK_WRITE_PCI_REG HAL_GALILEO_PCI0_STATUS_COMMAND_REGNUM, HAL_ATLAS_NULL_DEVNUM, \
			   (HAL_GALILEO_PCI0_CONFIG_MEMEn | HAL_GALILEO_PCI0_CONFIG_MasEn | HAL_GALILEO_PCI0_CONFIG_SErrEn)

	# Setup Galileo PCI latency timer
	MASK_WRITE_PCI_REG HAL_GALILEO_PCI0_BIST_REGNUM, HAL_ATLAS_NULL_DEVNUM, \
			   HAL_GALILEO_PCI0_LAT_TIMER_VAL

	# Setup base address for SAA9730
	WRITE_PCI_REG HAL_GALILEO_PCI0_SCS32_BASE_REGNUM, HAL_ATLAS_SAA9730_DEVNUM, \
		      CYGARC_PHYSICAL_ADDRESS(HAL_GALILEO_PCI0_MEM0_BASE)

	# Setup SAA9730 command and status register
	MASK_WRITE_PCI_REG HAL_GALILEO_PCI0_STATUS_COMMAND_REGNUM, HAL_ATLAS_SAA9730_DEVNUM, \
			   (HAL_GALILEO_PCI0_CONFIG_MEMEn | HAL_GALILEO_PCI0_CONFIG_SErrEn)

	# Init the I2C controller
	li	t0, HAL_SAA9730_I2CSC_I2CCC_6400
	li	t1, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_PCI0_MEM0_BASE)
	sw	t0, HAL_SAA9730_I2CSC_OFFSET(t1)

	##=====================================================================================
	##
	## Read the SPD device parameters and determine memory size
	##
	READ_SPD_VALUE HAL_SPD_GET_NUM_ROW_BITS, 0xf, s0, ERROR_CHECK
	READ_SPD_VALUE HAL_SPD_GET_NUM_COL_BITS, 0xf, s1, ERROR_CHECK
	READ_SPD_VALUE HAL_SPD_GET_NUM_DEVICE_BANKS, NO_MASK, s2, ERROR_CHECK

	READ_SPD_VALUE HAL_SPD_GET_SDRAM_WIDTH, 0x7f, s3, ERROR_CHECK
	READ_SPD_VALUE HAL_SPD_GET_NUM_MODULE_BANKS, NO_MASK, s4, ERROR_CHECK
	READ_SPD_VALUE HAL_SPD_GET_ROW_DENSITY, NO_MASK, s5, ERROR_CHECK

	#
	# Determine Size
	#     SIZE = SDRAM_WIDTH * NUM_DEVICE_BANKS * 2 ^ (NUM_ROW_BITS + NUM_COL_BITS)
	#
	addu	t0, s0, s1		# t0 = (NUM_ROW_BITS + NUM_COL_BITS)
	li	t1, 1			# t1 = 2 ^ 0
	sll	t1, t0			# t1 = 2 ^ (NUM_ROW_BITS + NUM_COL_BITS)
	multu	s2, t1
	mflo	s6			# s6 = NUM_DEVICE_BANKS * 2 ^ (NUM_ROW_BITS + NUM_COL_BITS)
	nop
	nop
	nop
	multu	s6, s3
	mflo	s6			# s6 = SDRAM_WIDTH * NUM_DEVICE_BANKS * 2 ^ (NUM_ROW_BITS + NUM_COL_BITS)
	nop
	nop
	nop

	#
	# Determine size of Bank 0
	#
	li	s0, HAL_ATLAS_MAX_BANKSIZE
0:
	and	t1, s5, BIT7
	bnez	t1, 8f
	sll	s5, 1
	b	0b
	srl	s0, 1
8:

	#
	# Determine if Bank 1 exists
	#
	li	t0, 1
	beq	s4, t0, 8f
	move	s1, zero
	#
	# Determine if Bank 1 is different than Bank 0
	#
	and	t1, s5, 0xFF
	beq	t1, zero, 8f
	move	s1, s0
	#
	# Determine size of Bank 1
	#
	li	s1, HAL_ATLAS_MAX_BANKSIZE
0:
	and	t1, s5, BIT7
	bnez	t1, 8f
	sll	s5, 1
	b	0b
	srl	s1, 1
8:

	#
	# FIXME: We should probably do some validation on the various
	#	 memory parameters here at some point.
	#

	#
	# Set the base SDRAM bank configuration value.
	# All other fields are zero, and the proper value is masked
	# in when they are known
	#
	li	s5, HAL_GALILEO_SDRAM_SRAS_TO_SCAS_DELAY_3C | \
		    HAL_GALILEO_SDRAM_WIDTH_64BIT | \
		    HAL_GALILEO_SDRAM_SRAS_PRECHARGE_3C

	#
	# Setup the CASLAT value.
	# Support only CASLAT = 2
	#
	READ_SPD_VALUE HAL_SPD_GET_CAS_LAT, NO_MASK, v0, NO_ERROR_CHECK
	and	t0, v0, 2
	beqz	t0, error
	nop
	ori	s5, HAL_GALILEO_SDRAM_BANK0_CASLAT_2

	#
	# Setup SDRAM device size
	#
	li	t0, SZ_16M
	beq	s6, t0, 8f
	nop
	ori	s5, HAL_GALILEO_SDRAM_BANK0_SZ_64M
8:

	#
	# Setup burst length: Support only 8
	#
	READ_SPD_VALUE HAL_SPD_GET_BURST_LENGTH, NO_MASK, v0, NO_ERROR_CHECK
	and	t0, v0, 8
	beqz	t0, error
	nop

	#
	# Setup Parity.
	# Only support Parity/Noparity.	 Don't support ECC.
	#
	READ_SPD_VALUE HAL_SPD_GET_CONFIG_TYPE, NO_MASK, v0, NO_ERROR_CHECK
	li	t0, HAL_SPD_CONFIG_TYPE_PARITY
	beq	t0, v0, 0f
	nop
	li	t0, HAL_SPD_CONFIG_TYPE_ECC
	beq	t0, v0, error
	nop
	b	8f
	li	v1, 0
0:
	ori	s5, HAL_GALILEO_SDRAM_BANK0_PARITY
	li	v1, 1
8:

	#
	# Setup number of device banks
	# Only support 2 or 4 banks
	#
	li	t0, 2
	beq	s2, t0, 8f
	nop
	li	t0, 4
	beq	s2, t0, 0f
	nop
	b	error
	nop
0:
	ori	s5, HAL_GALILEO_SDRAM_NUM_BANKS_4
8:

	#
	# Now actually store the bank config register
	#
	sw	s5, HAL_GALILEO_SDRAM_BANK0_OFFSET(s7)
	sw	s5, HAL_GALILEO_SDRAM_BANK2_OFFSET(s7)

	#
	# Setup the SDRAM configuration register
	# All other fields are zero, and the proper value is masked
	# in when they are known
	#
	li	s5, HAL_GALILEO_SDRAM_DUPLICATE_BANK_ADDR | HAL_GALILEO_SDRAM_BANK_INTERLEAVE_DIS

	#
	# Setup the Refresh Rate
	#
	READ_SPD_VALUE HAL_SPD_GET_REFRESH_RATE, 0x7f, v0, NO_ERROR_CHECK

	li	t0, HAL_SPD_REFRESH_RATE_125
	beq	t0, v0, 8f
	li	t0, HAL_SPD_REFRESH_COUNTER_125

	li	t0, HAL_SPD_REFRESH_RATE_62_5
	beq	t0, v0, 8f
	li	t0, HAL_SPD_REFRESH_COUNTER_62_5

	li	t0, HAL_SPD_REFRESH_RATE_31_3
	beq	t0, v0, 8f
	li	t0, HAL_SPD_REFRESH_COUNTER_31_3

	li	t0, HAL_SPD_REFRESH_RATE_15_625
	beq	t0, v0, 8f
	li	t0, HAL_SPD_REFRESH_COUNTER_15_625

	li	t0, HAL_SPD_REFRESH_RATE_7_8
	beq	t0, v0, 8f
	li	t0, HAL_SPD_REFRESH_COUNTER_7_8

	# Unknown: assume 3.9 microseconds
	li	t0, HAL_SPD_REFRESH_COUNTER_3_9
8:

	or	s5, t0

	#
	# Setup RAM_WIDTH
	#
	beqz	v1, 8f
	nop
	READ_SPD_VALUE HAL_SPD_GET_ERROR_CHECK_WIDTH, 0x7f, v0, NO_ERROR_CHECK
	beq	v0, zero, 8f
	nop
	ori	s5, HAL_GALILEO_SDRAM_CFG_RAM_WIDTH
8:

	#
	# Store the SDRAM configuration register
	#
	sw	s5, HAL_GALILEO_SDRAM_CONFIG_OFFSET(s7)

	#
	# Reset SAA9730 now that we are done with the I2C unit.
	# This allows the generic PCI library to start with a clean
	# slate of devices on the PCI bus.
	#
	li	a0, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_PCI0_MEM0_BASE)
	li	t0, HAL_SAA9730_SYSRESET_ALL
	sw	t0, HAL_SAA9730_SYSRESET_OFFSET(a0)
	
	#
	# Change the Galileo Base address to HAL_ATLAS_CONTROLLER_BASE
	#
	li	t0, HAL_ATLAS_CONTROLLER_BASE_ISD_CONFIG
	sw	t0, HAL_GALILEO_INT_SPACE_DECODE_OFFSET(s7)
	li	s7, CYGARC_UNCACHED_ADDRESS(HAL_ATLAS_CONTROLLER_BASE)

	#
	# Setup SDRAM Bank 0 Address Decoding
	#
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_RAM_BASE)		 # Physical bottom of Bank 0
	add	a1, s0, a0
	subu	a1, 1							 # Physical top of Bank 0

	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT			 # Setup SCS[1:0]
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT			 #   First level decoding
	sw	t0, HAL_GALILEO_SCS10_LD_OFFSET(s7)			 #   (ie Processor Decode Region)
	sw	t1, HAL_GALILEO_SCS10_HD_OFFSET(s7)			 #

	srl	t0, a0, HAL_GALILEO_DEV_DECODE_SHIFT			 # Setup SCS0
	srl	t1, a1, HAL_GALILEO_DEV_DECODE_SHIFT			 #   Second level decoding
	sw	t0, HAL_GALILEO_SCS0_LD_OFFSET(s7)			 #   (ie Device Sub-decode Region)
	sw	t1, HAL_GALILEO_SCS0_HD_OFFSET(s7)			 #

	#
	# Setup SDRAM Bank 1 Address Decoding
	#
	add	a0, s0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_RAM_BASE)	 # Physical bottom of Bank 1
	add	a1, a0, s1
	subu	a1, 1							 # Physical top of Bank 1

	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT			 # Setup SCS[3:2]
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT			 #   First level decoding
	sw	t0, HAL_GALILEO_SCS32_LD_OFFSET(s7)			 #   (ie Processor Decode Region)
	sw	t1, HAL_GALILEO_SCS32_HD_OFFSET(s7)			 #

	srl	t0, a0, HAL_GALILEO_DEV_DECODE_SHIFT			 # Setup SCS2
	srl	t1, a1, HAL_GALILEO_DEV_DECODE_SHIFT			 #   Second level decoding
	sw	t0, HAL_GALILEO_SCS2_LD_OFFSET(s7)			 #   (ie Device Sub-decode Region)
	sw	t1, HAL_GALILEO_SCS2_HD_OFFSET(s7)			 #

	#
	# Setup PCI windows
	#
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_PCI_MEM0_BASE)
	add	a1, a0, HAL_ATLAS_PCI_MEM0_SIZE
	subu	a1, 1							 # Physical top of Bank 1
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIMEM0_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIMEM0_HD_OFFSET(s7)
	
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_PCI_MEM1_BASE)
	add	a1, a0, HAL_ATLAS_PCI_MEM1_SIZE
	subu	a1, 1							 # Physical top of Bank 1
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIMEM1_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIMEM1_HD_OFFSET(s7)

	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_PCI_IO_BASE)
	add	a1, a0, HAL_ATLAS_PCI_IO_SIZE
	subu	a1, 1							 # Physical top of Bank 1
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIIO_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIIO_HD_OFFSET(s7)

	#
	# Setup FLASH Address Decoding
	#
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_ATLAS_FLASH_BASE)	 # Physical bottom of Flash Bank
	add	a1, a0, HAL_ATLAS_FLASH_SIZE
	subu	a1, 1							 # Physical top of Flash Bank

	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT			 # Setup CS[2:0]
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT			 #   First level decoding
	sw	t0, HAL_GALILEO_CS20_LD_OFFSET(s7)			 #   (ie Processor Decode Region)
	sw	t1, HAL_GALILEO_CS20_HD_OFFSET(s7)			 #

	srl	t0, a0, HAL_GALILEO_DEV_DECODE_SHIFT			 # Setup CS0
	srl	t1, a1, HAL_GALILEO_DEV_DECODE_SHIFT			 #   Second level decoding
	sw	t0, HAL_GALILEO_CS0_LD_OFFSET(s7)			 #   (ie Device Sub-decode Region)
	sw	t1, HAL_GALILEO_CS0_HD_OFFSET(s7)			 #

	#
	#  Now disable all unused decodes
	#  (SCS1, SCS3, PCI1xx, CS1, CS2)
	#
	li	t0, 0xffff
	move	t1, zero
	sw	t0, HAL_GALILEO_SCS1_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_SCS1_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_SCS3_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_SCS3_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_PCI1IO_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCI1IO_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_PCI1MEM0_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCI1MEM0_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_PCI1MEM1_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCI1MEM1_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_CS1_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_CS1_HD_OFFSET(s7)
	sw	t0, HAL_GALILEO_CS2_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_CS2_HD_OFFSET(s7)

noerror:
	move	v0, zero
	add	v1, s0, s1
	move	ra, s8
	jr	ra
	nop

error:
	li	v0, HAL_ATLAS_MEMERROR
	move	ra, s8
	jr	ra
	nop

FUNC_END(hal_atlas_init_sdram)

##
## Read a value from the SDRAM SPD device.
##
## Parameters:	 a0 = subaddress
## Returns:	 v0 = SPD value read
##
FUNC_START(read_spd_value)
	#
	# Setup a base address register
	#
	li	a1, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_PCI0_MEM0_BASE)

	#
	# Write the I2C command
	#
	sll	a0, 16
	li	t0, (HAL_SAA9730_I2CTFR_ATTR1_CONT | \
		     ((0xa0 << 24) | HAL_SAA9730_I2CTFR_ATTR2_START) | \
		     ((0xa1 << 8) | HAL_SAA9730_I2CTFR_ATTR0_START))
	or	a0, t0
	sw	a0, HAL_SAA9730_I2CTFR_OFFSET(a1)
1:	lw	t0, HAL_SAA9730_I2CTFR_OFFSET(a1)
	and	t0, 0x1
	bnez	t0, 1b
	nop

	#
	# Read the SPD value
	#
	li	a0, HAL_SAA9730_I2CTFR_ATTR2_STOP
	sw	a0, HAL_SAA9730_I2CTFR_OFFSET(a1)
1:	lw	t0, HAL_SAA9730_I2CTFR_OFFSET(a1)
	and	t0, 0x1
	bnez	t0, 1b
	nop

	#
	# Setup the return value.
	#
	lw	v0, HAL_SAA9730_I2CTFR_OFFSET(a1)
	srl	v0, 24

	jr	ra
	nop
FUNC_END(read_spd_value)
#endif /* defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) */

##-----------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.

	.extern hal_default_isr
	
	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.rept	25
	.long	hal_default_isr
	.endr

	.globl	hal_interrupt_data
hal_interrupt_data:
	.rept	25
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	25
	.long	0
	.endr

	
##-----------------------------------------------------------------------------
## end of platform.S
