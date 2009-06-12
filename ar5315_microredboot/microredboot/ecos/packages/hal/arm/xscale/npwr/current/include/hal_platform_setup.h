#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: gthomas
// Date:         2000-10-10
// Purpose:      Intel IOP310 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // More MMU definitions
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_iop310.h>         // Platform specific hardware definitions

// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO                 \
	b	667f			;\
   666:					;\
	.byte	0xc0, 0xf9, 0xa4, 0xb0  ;\
	.byte	0x99, 0x92, 0x82, 0xf8  ;\
	.byte	0x80, 0x90, 0x88, 0x83  ;\
	.byte	0xa7, 0xa1, 0x86, 0x8e  ;\
   667:					;\
	ldr	r0, =666b		;\
	add	r0, r0, #\x		;\
	ldrb	r1, [r0]		;\
	ldr	r0, =DISPLAY_RIGHT	;\
	str	r1, [r0]

#undef CYGHWR_LED_MACRO


// The main useful output of this file is PLATFORM_SETUP1: it invokes lots
// of other stuff (may depend on RAM or ROM start).  The other stuff is
// divided into further macros to make it easier to manage what's enabled
// when.

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
//#define CYGHWR_HAL_ARM_HAS_MMU
#else
#define PLATFORM_SETUP1
#endif

#define	RAM_BASE	0xa0000000
#define	DRAM_SIZE	(512*1024*1024)		// max size of available SDRAM
#define	DCACHE_SIZE	(32*1024)		// size of the Dcache

// Reserved area for battery backup SDRAM memory test
// This area is not zeroed out by initialization code
#define	SDRAM_BATTERY_TEST_BASE		0xA1FFFFF0	// base address of last 16 memory locations in a 32MB SDRAM



        // Display 'lvalue:rvalue' on the hex display
        // lvalue and rvalue must be of the form 'DISPLAY_x'
        // where 'x' is a hex digit from 0-F.
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue	
	ldr	\reg0, =DISPLAY_LEFT		// display left digit
	ldr	\reg1, =\lvalue
	str	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	ldr	\reg1, =\rvalue			// display right digit
	str	\reg1, [\reg0]
	.endm

	// Trigger the logic analyzer by writing a particular
	// address, and triggering on that address.
	.macro TRIGGER_LA_ON_ADDRESS address, reg0, reg1
	mrc	p15, 0, \reg0, c1, c0, 0     // read ARM control register
	//	CPWAIT  \reg0
	ldr	\reg1, =\address
	str	\reg0, [\reg1]
	.endm

	// Delay a bit
	.macro DELAY_FOR cycles, reg0
	ldr	\reg0, =\cycles
	subs	\reg0, \reg0, #1
	subne	pc,  pc, #0xc
	.endm
	
	// wait for coprocessor write complete
	.macro CPWAIT reg
        mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

	// form a first-level section entry
	.macro FL_SECTION_ENTRY base,x,ap,p,d,c,b
	.word (\base << 20) | (\x << 12) | (\ap << 10) | (\p << 9) |\
	      (\d << 5) | (\c << 3) | (\b << 2) | 2
	.endm

	// form a first-level page table entry
	.macro FL_PT_ENTRY base,p,d
	// I wanted to use logical operations here, but since I am using symbols later 
	// to fill in the parameters, I had to use addition to force the assembler to
	// do it right
	.word \base + (\p << 9) + (\d << 5) + 1
	.endm

	// form a second level small page entry
	.macro SL_SMPAGE_ENTRY base,ap3,ap2,ap1,ap0,c,b
	.word (\base << 12) | (\ap3 << 10) | (\ap2 << 8) | (\ap1 << 6) |\
	      (\ap0 << 4) | (\c << 3) | (\b << 2) | 2
	.endm

	// form a second level extended small page entry
	.macro SL_XSMPAGE_ENTRY base,x,ap,c,b
	.word (\base << 12) | (\x << 6) | (\ap << 4) | (\c << 3) | (\b << 2) | 3
	.endm


	// start of platform setup
	.macro _platform_setup1

	// This is where we wind up immediately after reset. On the IOP310, we have
	// to jump around a hole in flash which runs from 0x00001000 - 0x0001fff.
	// The start of _platform_setup1 will be below 0x1000 and since we need to
	// align the mmu table on a 16k boundary, we just branch around the page
	// table which we will locate at FLASH_BASE+0x4000.
	b _real_platform_setup

	.p2align 13
	// the following alignment creates the mmu table at address 0x4000.
    mmu_table:

	// 1MB of FLASH with i80312 MMRs mapped in using 4K small pages so we can
	// set the access permission on flash and memory-mapped registers properly.
	FL_PT_ENTRY mmu_table_flashbase,0,0

	// Remaining 7MB of FLASH
	//   rw, cacheable, non-bufferable
	.set	__base,1
	.rept	7
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr	

	// nothing interesting here (Address Translation)
	.rept	0xA00 - 0x8
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// up to 512MB ECC SDRAM mapped 1-to-1
	// first 1MB mapped in 4K chunks
	//   x=c=b=1
	FL_PT_ENTRY mmu_table_rambase,1,0
	.set	__base,__base+1
	.rept	0xC00 - 0xA01
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr

	// Cache flush region.
	// Don't need physical memory, just a cached area.
	.rept	0xD00 - 0xC00
	FL_SECTION_ENTRY __base,0,3,0,0,1,1
	.set	__base,__base+1
	.endr
	
	// Alias for first 1MB of FLASH
	//  rw, cacheable, non-bufferable
	FL_SECTION_ENTRY 0x000,0,3,0,0,1,0
	.set	__base,__base+1

	// Invalid
	.rept	0xE00 - 0xD01
	.word 0
	.set	__base,__base+1
	.endr

	// Uncached and unbuffered alias for first 256MB of SDRAM. This
	// area can be used by device drivers for DMA operations. Buffers
	// should be cache aligned.
	.set	__base,0xA00
	.rept	0xF00 - 0xE00
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// only I/O at 0xFE8xxxxx
	.set	__base,0xF00
	.rept	0x1000 - 0xF00
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Immediately after the above table (at 0x8000) is the
	// second level page table which breaks up the lowest 1MB
	// of physical memory into 4KB sized virtual pages. These
	// pages work around a hole in flash (0x1000-0x1fff) used
	// by the Yavapai companion chip internal registers.
    mmu_table_flashbase:
	// Virtual address 0 (Flash boot code).
	// Map 4k page at 0x00000000 virt --> 0xA0000000 physical
	// This allows us to have a writable vector table.
	//   Read-Write, cacheable, bufferable
	SL_XSMPAGE_ENTRY 0xa0000,1,3,1,1

	// Virtual address 0x1000 (Memory mapped registers)
	// Map 1-to-1, but don't cache or buffer
	//   Read-Write, non-cacheable, non-bufferable         
	.set	__base,1		   
	SL_SMPAGE_ENTRY __base,3,3,3,3,0,0
	.set	__base,__base+1

	// Virtual address 0x2000-0x100000 (remainder of flash1)
	//   Read-Write, cacheable, non-bufferable
	.rept	0x100 - 0x2
	SL_SMPAGE_ENTRY __base,3,3,3,3,1,0
	.set	__base,__base+1
	.endr

	// Now is the second level table for the first megabyte
	// of DRAM.
    mmu_table_rambase:
	// Map first meg of SDRAM
	//   Read-Write, cacheable, bufferable
	.set    __base,0xA0000
	.rept	0x100
	SL_XSMPAGE_ENTRY __base,1,3,1,1
	.set	__base,__base+1
	.endr

_real_platform_setup:
	// Drain write and fill buffer
	mcr	p15,0,r0,c7,c10,4
	CPWAIT	r0

	// Disable write buffer coalescing
	mrc	p15,0,r0,c1,c0,1
	orr	r0,r0,#1		// set the disable bit
	mcr	p15,0,r0,c1,c0,1
	CPWAIT	r0

	// Delay appx 60 ms to let battery-backup reset complete
	DELAY_FOR 0x400000, r0
        // Eventually we will be able to check a register bit
        // to determine when this is complete 

	HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_1
		
	//
	// ***  I2C interface initialization ***
	//

	//  Setup I2C Slave Address Register
	ldr	r1, =I2C_DEVID		// Load slave address r1.
	ldr	r2, =ISAR_ADDR		// Load address of the I2C Slave Address Register in r2.
	ldr	r3, =0x0000007f		// Load mask in r3.
	and	r1, r3, r3		// The mask zeroes the 25 MSBs of r1 just to make sure.
	str	r3, [r2]		// Save the value 0x02 (I2C_DEVID) in the register.

	//  Setup I2C Clock Count Register
	ldr	r2, =ICCR_ADDR		// Load the address of the I2C Clock Control Register in r2.
	ldr     r3, =0x0000014d		// Set for 5.05 us transition time at 66MHz (0x14D = 333).
	str	r3, [r2]	        // Save the value in the register.

	//  Enable I2C Interface Unit - status will be polled
	ldr	r2, =ICR_ADDR		// Load the address of the Control Register in r2.
	ldr	r1, =ICR_GCALL		// Disable General Call (will be master)
	ldr	r3, =ICR_ENB		// Enable I2C unit ).
	orr	r1, r3, r1		// OR the two and store in R1
	ldr	r3, =ICR_SCLENB		// Enable I2C Clock Generator disabled
	orr	r1, r3, r1		// OR the two and store in R1
	str	r1, [r2]		// Save the value to the Control Register.

	//
	//  *** Now read the SPD Data ***
	//

	// Pointers to I2C Registers
	ldr	r11, =ICR_ADDR		// Load the address of the I2C Control Register in r11.
	ldr	r12, =ISR_ADDR		// Load the address of the I2C Status Register in r12.
	ldr	r13, =IDBR_ADDR		// Load the address of the I2C Data Buffer Register in r13.

	// Initialize byte counters
	ldr	r6, =0x00000000  // Counter incremented before byte is read
	ldr	r7, =0x00000040  // Number of bytes to read in the Presence Detect EEPROM of SDRAM: 64 bytes
	ldr	r5, =0x00000000  // R5 has running checksum calculation
	ldr	r9, =I2C_TIMOUT  // Timeout limit in case EEPROM does not respond

	// At the end of all this, R4 has DRAM size, R8 has bank count, and R10 has Bank size
	ldr	r10,=0x00000000  // Bank size
	ldr	r8, =0x00000000  // Bank count
	ldr	r4, =0x00000000  // SDRAM size

	/*  FREE REGISTERS ARE R0 - R3 */

	// *** Put out address, with WRITE mode ***

	// Set SDRAM module address and write mode
	ldr	r1, =SDRAM_DEVID	// Load slave address for SDRAM module: 0xA2 (Presence Detect Data)
	bic	r1, r1, #IDBR_MODE	// Clear read bit (bit #0)
	str	r1, [r13]		// Store to data register

	// Initiate dummy write to set EEPROM pointer to 0
	ldr	r1, [r11]		// read the current Control Register value
	orr	r1, r1,	#ICR_START	// Set start bit
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing
	str	r1, [r11]		// Store to control register

	// Wait for transmit empty status
	ldr	r1, =0x00000000		// Initialize I2C timeout counter
    0:
	add	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	cmp	r1, r9
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r12]		// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked: IDBR Transmit Empty
	beq	0b		
	str	r0, [r12]		// Write back status to clear

	// *** Write pointer register on EEPROM to 0x00000000 ***

	//  Set SDRAM module EEPROM address to 0
	ldr	r1, =0x00000000		// Load base address of SDRAM module EEPROM
	str	r1, [r13]	        // Store to data register

	//  Send address to EEPROM
	ldr	r1, [r11]		// read the current Control Register value
	bic	r1, r1,	#ICR_START	// No start bit (already started)
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing
	str	r1, [r11]		// Store to control register

	// Wait for transmit empty status
	ldr	r1, =0x00000000		// Initialize I2C timeout counter
    0:
	add	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	cmp	r1, r9
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r12]		// Load I2C Status Reg into R0 -  ld	(r12), r10
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked: IDBR Transmit Empty
	beq	0b		                    
	str	r0, [r12]		// Write back status to clear

	// *** Read SDRAM PD data ***

	// *** Put out address, with READ mode ***

	//  Set SDRAM module address and read mode
	ldr	r0, =SDRAM_DEVID	// Load slave address for SDRAM module (0xA2)
	orr	r1, r0, #IDBR_MODE	// Set read bit (bit #0)
	str	r1, [r13]		// Store to data register

	//  Send next read request
	ldr	r1, [r11]		// read the current Control Register value
	orr	r1, r1,	#ICR_START	// Set start bit
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing
	str	r1, [r11]		// Store to control register

	// Wait for transmit empty status
	ldr	r1, =0x00000000		// Initialize I2C timeout counter
    0:
	add	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	cmp	r1, r9
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r12]		// Load I2C Status Reg into R0 -  ld	(r12), r10
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked: IDBR Transmit Empty
	beq	0b		                    
	str	r0, [r12]		// Write back status to clear

    sdram_loop:
	add	r6, r6, #1		// Increment byte counter
	
	// *** READ the next Byte!!! ***

	ldr	r1, [r11]		// read the current Control Register value
	bic	r1, r1,	#ICR_START	// No start bit (already started)
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing

	// we have to set NACK before reading the last bit
	cmp	r6, r7			// r7 = 64 (decimal) so if r6 = 64, this is the last byte to be read
	bne	1f			// If bytes left, skip ahead
	orr	r1, r1, #ICR_ACK	// Set NACK if this is the last byte
	orr	r1, r1, #ICR_STOP	// Set STOP if this is the last byte
    1:
	str	r1, [r11]		// Store to control register

	// Wait for read full status
	ldr	r1, =0x00000000		// Initialize I2C timeout counter
    0:
	add	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	cmp	r1, r9
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r12]		// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_FULL	// Bit #6 is checked: IDBR Transmit Empty
	beq	0b		                    
	str	r0, [r12]		// Write back status to clear

	// Read the data byte
	ldr	r1, [r13] 		// Read the byte

	ldr	r2, =CHECKSUM_BYTE 
	cmp	r6, r2			// is it the CHECKSUM byte???
	beq	1f
	add	r5, r5, r1		// Add it to the checksum if not the checksum byte
	bal	2f			// skip checksum comparison
    1:
	ldr	r0, =0xff		// If this is the checksum byte, compare it
	and	r5, r5, r0		//	against the calculated checksum
	cmp	r1, r5
	bne	bad_checksum		// If no match, skip SDRAM controller initialization
    2:
	ldr	r2, =BANKCNT_BYTE	// Check for bank count byte
	cmp	r6, r2
	bne	1f
	mov	r8, r1			// Store bank count
    1:
	ldr	r2, =BANKSZ_BYTE	// Check for bank size byte
	cmp	r6, r2
	bne	1f

	ldr	r2, =0x04		// Store bank size in Mbytes (shift left 2 bits)
	mul	r10, r1, r2
	mul	r2, r8, r10		// Multiply by bank count to get DRAM size in MB
	ldr	r0, =0x100000
	mul	r4, r2, r0		// Convert size to bytes  - r4 contains DRAM size in bytes

1:
	// Handle the SDRAM drive strength setup here since we are out of
	// temporary registers to hold the SDRAM width value until after
	// all of the SPD data has been read.  Using the value of r8 for
	// the Bank Count is allright here since the SPD specification states that
	// the Bank Count SPD byte is #5 and the SDRAM Width SPD byte is #13.

	ldr	r2, =SDRAM_WIDTH_BYTE 	// Check for SDRAM width byte
	cmp	r6, r2
	bne	1f
	mov	r2, #0x10	  // Check for data width of 16
	cmp	r1, r2
	bne	SDRAM_DRIVE_X8

	// Module is composed of x16 devices
	mov	r2, #0x02			
	cmp	r2, r8		// do we have 2 banks???
	beq	SDRAM_DRIVE_2_BANK_X16

	// Module is composed of 1 Bank of x16 devices
	ldr	r1, =SDCR_ADDR		// point at SDRAM Control Register
	ldr	r2, =SDCR_1BANK_X16	// drive strength value
	str	r2, [r1]		// set value in SDCR
	b	1f

SDRAM_DRIVE_2_BANK_X16:
	// Module is composed of 2 Banks of x16 devices
	ldr	r1, =SDCR_ADDR		// point at SDRAM Control Register
	ldr	r2, =SDCR_2BANK_X16	// drive strength value
	str	r2, [r1]		// set value in SDCR
	b	1f

SDRAM_DRIVE_X8:
	// Module is composed of x8 devices
	mov	r2, #0x02			
	cmp	r2, r8			// do we have 2 banks???
	beq	SDRAM_DRIVE_2_BANK_X8

	// Module is composed of 1 Bank of x8 devices
	ldr	r1, =SDCR_ADDR		// point at SDRAM Control Register
	ldr	r2, =SDCR_1BANK_X8	// drive strength value
	str	r2, [r1]		// set value in SDCR
	b	1f

SDRAM_DRIVE_2_BANK_X8:
	// Module is composed of 2 Banks of x16 devices
	ldr	r1, =SDCR_ADDR		// point at SDRAM Control Register
	ldr	r2, =SDCR_2BANK_X8	// drive strength value
	str	r2, [r1]		// set value in SDCR
    1:


	// Continue reading bytes if not done
	cmp	r6, r7
	bne	sdram_loop
	
	b	i2c_disable

    bad_checksum:
	HEX_DISPLAY r2, r3, DISPLAY_7, DISPLAY_7	

    i2c_error:
	// hit the leds if an error occurred
	HEX_DISPLAY r2, r3, DISPLAY_5, DISPLAY_5


    i2c_disable:
	//  Disable I2C Interface Unit
	ldr	r1, [r11] 
	bic	r1, r1, #ICR_ENB	// Disable I2C unit
	bic	r1, r1, #ICR_SCLENB	// Disable I2C clock generator
	str	r1, [r11]		// Store to control register
	
	// ADD THIS???:
	//  cmpobne	1, g9, test_init
	// Skip SDRAM controller initialization if checksum test failed

	// *** SDRAM setup ***

	ldr	r9, =MMR_BASE		// get base of MMRs
	ldr	r0, =RAM_BASE		// Program SDRAM Base Address register
	str	r0, [r9, #SDBR_OFF]    

	// Set up bank 0 register
    CHECK_32MB:
	ldr	r1, =RAM_32MEG		// do we have 32 MB banks?
	cmp	r10, r1
	bne	CHECK_64MB

	ldr	r0, =SBR_32MEG		// Program SDRAM Bank0 Boundary register to 32 MB
	b	SET_BANK1

    CHECK_64MB:
	ldr	r1, =RAM_64MEG		// do we have 64 MB banks?
	cmp	r10, r1
	bne	CHECK_128MB

	ldr	r0, =SBR_64MEG		// Program SDRAM Bank0 Boundary register to 64 MB
	b	SET_BANK1

    CHECK_128MB:
	ldr	r1, =RAM_128MEG		// do we have 128 MB banks?
	cmp	r10, r1
	bne	CHECK_256MB

	ldr	r0, =SBR_128MEG		// Program SDRAM Bank0 Boundary register to 128 MB
	b	SET_BANK1

    CHECK_256MB:
	ldr	r1, =RAM_256MEG		// do we have 256 MB banks?
	cmp	r10, r1
	bne	dram_error

	ldr	r0, =SBR_256MEG		// Program SDRAM Bank0 Boundary register to 64 MB
	b	SET_BANK1

    SET_BANK1:
	str	r0, [r9, #SBR0_OFF]	// store SBR0

	ldr	r2, =0x02			
	cmp	r2, r8			// do we have 2 banks???
	bne	SDRAM_1_BANK

	add	r0, r0, r0		// SDRAM Bank1 Boundary register is double SBR0
	str	r0, [r9, #SBR1_OFF]
	b	END_DRAM_SIZE

    SDRAM_1_BANK:
	// SDRAM Bank1 Boundary register is same as SBR0 for 1 bank configuration
	str	r0, [r9, #SBR1_OFF]
	b	END_DRAM_SIZE

    END_DRAM_SIZE:
	b	init_dram

    dram_error:

  	HEX_DISPLAY r2, r3, DISPLAY_F, DISPLAY_F

   init_dram:
	ldr	r0, =0			// turn off refresh
	str	r0, [r9, #RFR_OFF]
	
	ldr	r0,   =MRS_NO_OP        // Issue NOP cmd to SDRAM
	str	r0, [r9, #SDIR_OFF]
	DELAY_FOR 0x4000, r0
	
	ldr	r0, =MRS_PRECHRG	// Issue 1 Precharge all
	str	r0, [r9, #SDIR_OFF]    
	DELAY_FOR 0x4000, r0


	ldr	r0, =MRS_AUTO_RFRSH	// Issue 1 Auto Refresh command
	str	r0, [r9, #SDIR_OFF]    
	DELAY_FOR 0x4000, r0


	ldr	r0, =MRS_AUTO_RFRSH
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #1
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #2
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #3
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #4
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #5
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #6
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #7
	str	r0, [r9, #SDIR_OFF]    // Auto Refresh #8

	ldr	r0, =MRS_CAS_LAT_2	// set the CAS latency
	str	r0, [r9, #SDIR_OFF]
	DELAY_FOR 0x4000, r0
	
	ldr	r0, =MRS_NORM_OP	// Issue a Normal Operation command
	str	r0, [r9, #SDIR_OFF]     

	ldr	r0, =RFR_INIT_VAL	// Program Refresh Rate register
	str	r0, [r9, #RFR_OFF]     

	// ldr   r0, =(FLASH_BASE :AND: &FFFF0000)
	// str   r0, [r10, #FEBR1_OFF]   ; Program Flash Bank1 Base Address register

	// ldr   r0, =(FLASH_SIZE :AND: &FFFF0000)
	// str   r0, [r10, #FBSR1_OFF]   ; Program Flash Bank1 Size register

	// ldr   r0, =FWSR0_INIT_VAL
	// str   r0, [r10, #FWSR0_OFF]   ; Program Flash Bank0 Wait State register

	// ldr   r0, =FWSR1_INIT_VAL
	// str   r0, [r10, #FWSR1_OFF]   ; Program Flash Bank1 Wait State register

	HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_2

	// begin initializing the i80310

	// Enable access to all coprocessor registers
	ldr	r0, =0x2001			// enable access to all coprocessors
	mcr	p15, 0, r0, c15, c1, 0
	
	mcr	p15, 0, r0, c7, c10, 4		// drain the write & fill buffers
	CPWAIT r0	
	
	mcr	p15, 0, r0, c7, c7, 0		// flush Icache, Dcache and BTB
	CPWAIT r0	
	
	mcr	p15, 0, r0, c8, c7, 0		// flush instuction and data TLBs
	CPWAIT r0	
	
	// Enable the Icache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_I
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	// Set the TTB register
	ldr	r0, =mmu_table
	mcr	p15, 0, r0, c2, c0, 0

	// Enable permission checks in all domains
	ldr	r0, =0x55555555
	mcr	p15, 0, r0, c3, c0, 0
	
	// Enable the MMU
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_M
	orr	r0, r0, #MMU_Control_R
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0
	
	mcr	p15, 0, r0, c7, c10, 4		// drain the write & fill buffers
	CPWAIT r0	
	
	// Enable the Dcache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_C
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        // Initialize branch target buffer
        BTB_INIT r0

	//  Battery Backup SDRAM Memory Test
        //  Move 4 byte Test Pattern into register prior to zeroing out
	//  contents of SDRAM locations
	ldr	r9, =SDRAM_BATTERY_TEST_BASE
	ldr	r10, [r9]

	IOP310_EARLY_PCI_SETUP  r0, r1, r4, 0x113C, 0x0700
	
	// scrub/init SDRAM if enabled/present
	ldr	r11, =RAM_BASE	// base address of SDRAM
	mov	r12, r4		// size of memory to scrub
	mov	r8,r4		// save DRAM size
	mov	r0, #0		// scrub with 0x0000:0000
	mov	r1, #0
	mov	r2, #0				
	mov	r3, #0
	mov	r4, #0					
	mov	r5, #0
	mov	r6, #0					
	mov	r7, #0
    10: // fastScrubLoop
	subs	r12, r12, #32	// 32 bytes/line
	stmia	r11!, {r0-r7}
	beq	15f
	b	10b
    15:
	
	// now copy 1st 4K page of flash into first 4K of RAM.
	ldr	r1, =RAM_BASE	// base address of SDRAM
	mov     r2, #0xd0000000 // alias for first 1M of flash
	mov     r3, #0x1000
    16:
	ldr     r4, [r2]
	add	r2, r2, #4
	str	r4, [r1]
	add	r1, r1, #4
	subs    r3, r3, #4
	bne     16b

	// Battery Backup SDRAM Memory Test
	// Store 4 byte Test Pattern back into memory
	str r10, [r9, #0x0]

	HEX_DISPLAY r0, r1, DISPLAY_1, DISPLAY_0

	// clean/drain/flush the main Dcache
	mov	r1, #DCACHE_FLUSH_AREA           // use a CACHEABLE area of
	                                         // the memory map above SDRAM
	mov	r0, #1024			 // number of lines in the Dcache
    20:
	mcr	p15, 0, r1, c7, c2, 5		 // allocate a Dcache line
	add	r1, r1, #32			 // increment the address to
	                                         // the next cache line
	subs	r0, r0, #1			 // decrement the loop count
	bne	20b

	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_9

	// clean/drain/flush the mini Dcache
	ldr	r2, =(DCACHE_FLUSH_AREA+DCACHE_SIZE) // use a CACHEABLE area of
	                                        // the memory map above SDRAM
	mov	r0, #64			        // number of lines in the mini Dcache
    21:
	mcr	p15, 0, r2, c7, c2, 5		// allocate a Dcache line
	add	r2, r2, #32			// increment the address to
  	                                        // the next cache line
	subs	r0, r0, #1			// decrement the loop count
	bne	21b

	mcr	p15, 0, r0, c7, c6, 0		// flush Dcache
	CPWAIT r0
	
	HEX_DISPLAY r0, r1, DISPLAY_7, DISPLAY_7

	mcr	p15, 0, r0, c7, c10, 4		// drain the write & fill buffers
	CPWAIT r0	
	
	// enable ECC stuff here
	mcr p15, 0, r0, c7, c10, 4		// 
	CPWAIT r0

	mrc	p13, 0, r0, c0, c1, 0		// BCU_WAIT --> wait until the BCU isn't busy
	submi	pc, pc, #0xc
	
    checkme:	// add in multi-bit error reporting */
	mrc	p13, 0, r0, c0, c1, 0		// disable ECC
	and	r0, r0, #(-1-8)
	mcr	p13, 0, r0, c0, c1, 0		
	orr	r0, r0, #6			// enable single-bit correction,
	mcr	p13, 0, r0, c0, c1, 0		// multi-bit detection
	orr	r0, r0, #8			// enable ECC
	mcr	p13, 0, r0, c0, c1, 0		

	mrc	p13, 0, r0, c0, c1, 0		// BCU_WAIT --> wait until the BCU isn't busy
	submi	pc, pc, #0xc

	// Enable ECC circuitry in Yavapai
	ldr	r1, =ECCR_ADDR
	mov	r0, #0x6  // Enable single bit ECC Correction (Reporting Enabled)
	str	r0, [r1, #0]

	HEX_DISPLAY r0, r1, DISPLAY_6, DISPLAY_6

#if 1
	mov	r0, #0x1000000
    1:  subs	r0,r0,#1
	bne	1b
#endif
	// Save SDRAM size
        ldr     r1, =hal_dram_size  /* [see hal_intr.h] */
	str	r8, [r1]

	// Move mmu tables into RAM so page table walks by the cpu
	// don't interfere with FLASH programming.
	ldr	r0, =mmu_table
	mov     r4, r0
	add     r2, r0, #0x4800     	// End of tables
	mov	r1, #RAM_BASE
	orr	r1, r1, #0x4000		// RAM tables
	mov     r5, r1

	// first, fixup physical address to second level
	// table used to map first 1MB of flash.
	ldr	r3, [r0], #4
	sub     r3, r3, r4
	add	r3, r3, r5
	str	r3, [r1], #4
	// everything else can go as-is
    1:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	cmp	r0, r2
	bne	1b

	// go back and fixup physical address to second level
	// table used to map first 1MB of SDRAM.
	add     r1, r5, #(0xA00 * 4)
	ldr	r0, [r1]    		// entry for first 1MB of DRAM
	sub     r0, r0, r4
	add	r0, r0, r5
	str	r0, [r1]    		// store it back
	
	// Flush the cache
        mov    r0, #DCACHE_FLUSH_AREA	/* cache flush region */
        add    r1, r0, #0x8000		/* 32KB cache         */
  667:
        mcr    p15,0,r0,c7,c2,5		/* allocate a line    */
        add    r0, r0, #32       	/* 32 bytes/line      */
        teq    r1, r0
        bne    667b
        mcr    p15,0,r0,c7,c6,0		/* invalidate data cache */
        /* cpuwait */
        mrc    p15,0,r1,c2,c0,0		/* arbitrary read   */
        mov    r1,r1
        sub    pc,pc,#4
        mcr    p15,0,r0,c7,c10,4
        /* cpuwait */
        mrc    p15,0,r1,c2,c0,0		/* arbitrary read   */
        mov    r1,r1
        sub    pc,pc,#4
        nop

	HEX_DISPLAY r0, r1, DISPLAY_5, DISPLAY_2

	// Set the TTB register to DRAM mmu_table
	mov	r0, r5
	mov	r1, #0
	mcr	p15, 0, r1, c7, c5, 0		// flush I cache
	mcr	p15, 0, r1, c7, c10, 4		// drain WB
	mcr	p15, 0, r0, c2, c0, 0		// load page table pointer
	mcr	p15, 0, r1, c8, c7, 0		// flush TLBs
	CPWAIT  r0

	// Interrupt init
        mov	r0, #0 // enable no sources
        mcr	p13,0,r0,c0,c0,0 // write to INTCTL
        // Steer both BCU and PMU to IRQ
        mcr	p13,0,r0,c8,c0,0 // write to INTSTR

	HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_0

	.endm    // _platform_setup1


#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  _80312_EMISR
_80312_EMISR:   .long   0       // Companion chip "clear-on-read" interrupt status
				// register for the performance monitor unit.
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */

