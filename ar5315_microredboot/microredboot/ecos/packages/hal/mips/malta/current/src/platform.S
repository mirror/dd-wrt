##
#=============================================================================
##	platform.S
##
##	MIPS Malta platform code
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
## Contributors:dmoseley, jskov
## Date:	2001-03-20
## Purpose:	MIPS Malta platform code
## Description:	Platform specific code for Malta board.
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
#include <cyg/hal/plf_defs.inc>
#include CYGBLD_HAL_PLATFORM_H

##-----------------------------------------------------------------------------

##-----------------------------------------------------------------------------
# Platform Initialization.
# This code performs platform specific initialization.

##-----------------------------------------------------------------------------
## I2C accessors - these need to delay after accessing the controller. 
// Need to delay between clocking the serial bits since the CPU is way too
// fast for the I2C interface. CPU runs at CYGHWR_HAL_MIPS_MALTA_CPU_CLOCK
// and the I2C no fasten than 1.7MHz. We want to delay the CPU for half
// an I2C cycle and two instructions are executed per loop, hence:

#define PAUSE                                                      \
    li     t0, (20*(CYGHWR_HAL_MIPS_MALTA_CPU_CLOCK/1700000/2/2)); \
99: bne    t0, zero, 99b;                                          \
     addiu t0, -1


#define I2C_OE(v)              \
    li  t0,v;                  \
    sw	t0,HAL_I2CFPGA_OE(a1); \
    PAUSE

#define I2C_OUT(v)              \
    li  t0,v;                   \
    sw	t0,HAL_I2CFPGA_OUT(a1); \
    PAUSE

#define I2C_IN(v)                      \
    lw	 v,HAL_I2CFPGA_INP(a1);        \
    andi v,v,HAL_I2CFPGA_IN_SDA_MASK;  \
    PAUSE

#define I2C_ENABLE                \
    I2C_OE(HAL_I2C_CIN_DIN);      \
    I2C_OUT(HAL_I2C_CHIGH_DHIGH); \
    li  t0,HAL_I2CFPGA_SEL_FPGA;  \
    sw	t0,HAL_I2CFPGA_SEL(a1);   \
    PAUSE

##-----------------------------------------------------------------------------
## MEMC initialization.
##

#if defined(CYG_HAL_STARTUP_ROM)

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
    jal	  read_spd_value
     li	  a0, \func			 # delay slot
.if \err_check
    beq	  v0, zero, error
     nop
.endif
    move  \ret_reg, v0
.if \mask
    and	  \ret_reg, \mask
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
FUNC_START(hal_malta_init_sdram)

	.set noreorder

	# Save the return address
	move	s8, ra

	# Setup the base address registers
	li	s7, CYGARC_UNCACHED_ADDRESS(HAL_GALILEO_REGISTER_BASE)
	#
	# Change the Galileo Base address to HAL_MALTA_CONTROLLER_BASE
	#
	li	t0, HAL_MALTA_CONTROLLER_BASE_ISD_CONFIG
	sw	t0, HAL_GALILEO_INT_SPACE_DECODE_OFFSET(s7)
	li	s7, CYGARC_UNCACHED_ADDRESS(HAL_MALTA_CONTROLLER_BASE)

	# Setup the Galileo controller Endian configuration
	li	t0, (HAL_GALILEO_BYTE_SWAP)
	sw	t0, HAL_GALILEO_PCI_INTERNAL_COMMAND_OFFSET(s7)

	# Setup the PCI_0 Timeout and retry configuration
	li	t0, HAL_GALILEO_PCI0_TIMEOUT_RETRY_VALUE
	sw	t0, HAL_GALILEO_PCI0_TIMEOUT_RETRY_OFFSET(s7)

	# Setup Galileo as PCI Master
	MASK_WRITE_PCI_REG HAL_GALILEO_PCI0_STATUS_COMMAND_REGNUM, HAL_MALTA_NULL_DEVNUM, \
			   (HAL_GALILEO_PCI0_CONFIG_MEMEn | HAL_GALILEO_PCI0_CONFIG_MasEn | HAL_GALILEO_PCI0_CONFIG_SErrEn)

	# Setup Galileo PCI latency timer
	MASK_WRITE_PCI_REG HAL_GALILEO_PCI0_BIST_REGNUM, HAL_MALTA_NULL_DEVNUM, \
			   HAL_GALILEO_PCI0_LAT_TIMER_VAL

	# Enable FPGA I2C
	li	a1, CYGARC_UNCACHED_ADDRESS(HAL_I2CFPGA_BASE)
	I2C_ENABLE

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

	READ_SPD_VALUE HAL_SPD_GET_BURST_LENGTH, NO_MASK, s6, ERROR_CHECK

	#
	# Determine Size in Mbit
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
	# SPD Density of Each Row on Module value is used. Bit 7 represents
	# 512MB, bit 6 256MB, etc. Highest set bit is size of bank 0.
	# If there are two banks and these have different sizes, an extra
	# bit will be set. If the sizes are the same, only the one bit is set.
	# Note, at the exit of this loop, the size-bit of bank 0 will have been
	# shifted out, allowing an easy check for multiple sizes below
	#
	li	s0, SZ_512M
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
	# Determine if Bank 1 is different than Bank 0. If no additional bits
	# set, size is the same.
	#
	and	t1, s5, 0xFF
	beq	t1, zero, 8f
	 move	s1, s0
	#
	# Determine size of Bank 1. It will be at least one factor smaller
	# than that of bank 0.
	#
	sll	s1,1
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

#if 0	// FIXME: Dunno what this is supposed to do, but it changes the RMW flag,
	// not anything related to RAM width.
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
#endif

	#
	# Store the SDRAM configuration register
	#
	sw	s5, HAL_GALILEO_SDRAM_CONFIG_OFFSET(s7)
	
	#
	# Setup SDRAM Bank 0 Address Decoding
	#
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_RAM_BASE)		 # Physical bottom of Bank 0
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
	add	a0, s0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_RAM_BASE)	 # Physical bottom of Bank 1
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
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_PCI_MEM0_BASE)
	add	a1, a0, HAL_MALTA_PCI_MEM0_SIZE
	subu	a1, 1							 # Physical top of Mem Bank 0
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIMEM0_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIMEM0_HD_OFFSET(s7)

	
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_PCI_MEM1_BASE)
	add	a1, a0, HAL_MALTA_PCI_MEM1_SIZE
	subu	a1, 1							 # Physical top of Mem Bank 1
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIMEM1_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIMEM1_HD_OFFSET(s7)

	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_PCI_IO_BASE)
	add	a1, a0, HAL_MALTA_PCI_IO_SIZE
	subu	a1, 1							 # Physical top of IO Bank
	srl	t0, a0, HAL_GALILEO_CPU_DECODE_SHIFT
	srl	t1, a1, HAL_GALILEO_CPU_DECODE_SHIFT
	sw	t0, HAL_GALILEO_PCIIO_LD_OFFSET(s7)
	sw	t1, HAL_GALILEO_PCIIO_HD_OFFSET(s7)

	# Here's a nice gotcha. The Intel southbridge *must* see IO
	# starting from 0.
	sw	zero,HAL_GALILEO_PCI_IO_REMAP(s7)

	#
	# Setup FLASH Address Decoding
	#
	li	a0, CYGARC_PHYSICAL_ADDRESS(HAL_MALTA_FLASH_BASE)	 # Physical bottom of Flash Bank
	add	a1, a0, HAL_MALTA_FLASH_SIZE
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
	li	v0, HAL_MALTA_MEMERROR
	move	ra, s8
	jr	ra
	nop

FUNC_END(hal_malta_init_sdram)

    .macro i2c_start
    # Start: SDA low -> high with SLC high
    I2C_OE(HAL_I2C_COUT_DOUT)
    I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_OUT(HAL_I2C_CHIGH_DLOW)
    I2C_OUT(HAL_I2C_CLOW_DLOW)
    .endm
    
    .macro i2c_stop
    # Stop: SDA high -> low with SLC high
    I2C_OE(HAL_I2C_COUT_DOUT)
    I2C_OUT(HAL_I2C_CLOW_DLOW)
    I2C_OUT(HAL_I2C_CHIGH_DLOW)
    I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    .endm

    .macro i2c_write
    # Value to write in t1.
    # Writes happen by clocking SCL low->high->low while SDA defines
    # the bit to be sent (MSB first).
    I2C_OE(HAL_I2C_COUT_DOUT)
    li      t2, 7
1:  srlv    t3, t1, t2
    andi    t3, 1
    beq     t3, zero, 2f
     nop
    # Send 1
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    b       3f
     nop
    
2:  # Send 0
    I2C_OUT(HAL_I2C_CLOW_DLOW)
    I2C_OUT(HAL_I2C_CHIGH_DLOW)
    I2C_OUT(HAL_I2C_CLOW_DLOW)
    
3:  bne     t2, zero, 1b
     addiu   t2, -1
    
    # Now tristate the SDA and pulse the clock. Receiver will
    # ack the transfer by pulling SDA low.
    # Read by pulsing clock. Leave result in t1
    I2C_OE(HAL_I2C_COUT_DIN)
    I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_IN(t1)
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    .endm

    .macro i2c_read
    # Value read is returned in t1
    # Reads happen by clocking SCL high->low while reading SDA
    I2C_OE(HAL_I2C_COUT_DIN)
    move    t1,zero
    li      t2, 7
1:  I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_IN(t3)
    sll     t1,1
    or      t1,t1,t3
    I2C_OUT(HAL_I2C_CLOW_DHIGH)

    bne     t2, zero, 1b
     addiu   t2, -1

    // Send ack by clocking with SDA low.
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    I2C_OE(HAL_I2C_COUT_DOUT)
    I2C_OUT(HAL_I2C_CHIGH_DHIGH)
    I2C_OUT(HAL_I2C_CLOW_DHIGH)
    .endm
		
##
## Read a value from the SDRAM SPD device.
##
## Parameters:	 a0 = subaddress
## Returns:	 v0 = SPD value read
##
FUNC_START(read_spd_value)
	.set noreorder
	# Setup a base address register
	li	a1, CYGARC_UNCACHED_ADDRESS(HAL_I2CFPGA_BASE)

	i2c_start

	# Write address of SDRAM sense controller
	li	t1,( HAL_I2C_SPD_ADDRESS | HAL_I2C_WRITE )
	i2c_write
	li	t0,HAL_I2CFPGA_OUT_SDA_NACK
	beq     t0,t1,i2c_error
         move   v0,zero

	# Write address of data wanted
	move	t1,a0
	i2c_write
	li	t0,HAL_I2CFPGA_OUT_SDA_NACK
	beq     t0,t1,i2c_error
         move   v0,zero
	
	i2c_start

	# Write address of SDRAM sense controller
	li	t1,( HAL_I2C_SPD_ADDRESS | HAL_I2C_READ )
	i2c_write
	li	t0,HAL_I2CFPGA_OUT_SDA_NACK
	beq     t0,t1,i2c_error
         move   v0,zero

	# Read data
	i2c_read
	move	v0,t1

	i2c_stop
	
i2c_error:
	jr	ra
	 nop

FUNC_END(read_spd_value)
#endif // defined(CYG_HAL_STARTUP_ROM)

##-----------------------------------------------------------------------------
## ISR springboard.
## This routine decodes the interrupt from the southbridge and vectors to it.

        # On entry:
        # a0 = MIPS status register interrupt number (1,2 or 3)
        # a1 = ISR data value (= interrupt controller reg address)
        # a2 = saved reg dump ptr
        # s0 = saved reg dump ptr
        # s1 = vector table offset
        # s2 = interrupt number
        # a3,v0,v1 etc available for use

        .text

FUNC_START(hal_isr_springboard_southbridge)
	.set noreorder
	# Get req bits of controller 1
	lb      v0,0(a1)
	lb      v1,1(a1)             # apply mask
	xori    v1,v1,0xffff
	and     v0,v0,v1
	andi    v1,v0,0xffff&~(1<<(CYGNUM_HAL_INTERRUPT_CASCADE-CYGNUM_HAL_INTERRUPT_CTRL1_BASE))
	bne	v1,zero,1f
	 ori	a2,zero,CYGNUM_HAL_INTERRUPT_CTRL1_BASE
        # If cascade is set, check controller 2
	andi    v0,v0,(1<<(CYGNUM_HAL_INTERRUPT_CASCADE-CYGNUM_HAL_INTERRUPT_CTRL1_BASE))
	beq	v0,zero,2f
	 lb	v0,HAL_PIIX4_MASTER_SLAVE_OFFSET(a1)
	lb      v1,HAL_PIIX4_MASTER_SLAVE_OFFSET+1(a1)   # apply mask
	xori    v1,v1,0xffff
	and     v0,v0,v1
	bne	v0,zero,1f
	 ori	a2,zero,CYGNUM_HAL_INTERRUPT_CTRL2_BASE
	# Spurious interrupt, return to VSR
2:	jr      ra
	 move	v0,zero
1:	

	# FIXME: May want to rewrite this to do ls bit on byte
	#        to save a few cycles.
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
         nop                                    # default vsr already in ra

FUNC_END(hal_isr_springboard_southbridge)

       
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
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.

	.extern hal_default_isr

	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.long	hal_isr_springboard_southbridge
	.rept	CYGNUM_HAL_ISR_COUNT-1
	.long	hal_default_isr
	.endr

	.globl	hal_interrupt_data
hal_interrupt_data:
	.long	HAL_PIIX4_MASTER_OCW3
	.rept	CYGNUM_HAL_ISR_COUNT-1
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
	.endr

	
##-----------------------------------------------------------------------------
## end of platform.S
