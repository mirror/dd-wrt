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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      Intel XScale IQ80321 platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_verde.h>          // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/iq80321.h>            // Platform specific hardware definitions
#include <cyg/hal/hal_spd.h>

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

.macro  NOPs count
        .rept \count
        nop
        nop
        .endr
.endm

// ------------------------------------------------------------------------
// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO                                     \
	b	667f			                    ;\
   666:					                    ;\
	.byte	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3  ;\
	.byte	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7  ;\
	.byte	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B  ;\
	.byte	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F  ;\
   667:					                    ;\
	ldr	r0, =666b		                    ;\
	add	r0, r0, #\x		                    ;\
	ldrb	r1, [r0]		                    ;\
	ldr	r0, =DISPLAY_RIGHT                          ;\
	str	r1, [r0]

#define PAUSE                            \
        ldr     r1,=0x8000;              \
  555:  sub     r1,r1,#1;                \
        cmp     r1,#0;                   \
        bne     555b;

#define DCACHE_SIZE (32 * 1024)

// ------------------------------------------------------------------------
// MCU Register Values

// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform        
	.macro _platform_setup1
	// This is where we wind up immediately after reset. At this point, we
	// are executing from the boot address (0x00000000), not the eventual
	// flash address. Do some basic setup using position independent code
	// then switch to real flash address

// FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 
// This is a quick and dirty workaround to an apparent gas/ld
// bug. The computed UNMAPPED_PTR(reset_vector) is off by 0x20.
        .rept 0x20/4
	nop
        .endr
// FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 

        ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

        // enable coprocessor access
	ldr	r0, =0x20c1              // CP13,CP7,CP6,CP0
        mcr     p15, 0, r0, c15, c1, 0

	// Drain write and fill buffer
	mcr	p15, 0, r0, c7, c10, 4
	CPWAIT	r0

	// Setup PBIU chip selects
	ldr	r8, =PBIU_PBCR

	ldr	r2, =PBLR_SZ_4K
	ldr	r1, =IQ80321_UART_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_8
	str     r1, [r8, #0x10]  // PBIU_PBAR1
	str     r2, [r8, #0x14]  // PBIU_PBLR1

	ldr	r1, =IQ80321_DISPLAY_RIGHT_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_32
	str     r1, [r8, #0x18]  // PBIU_PBAR2
	str     r2, [r8, #0x1C]  // PBIU_PBLR2

	ldr	r1, =IQ80321_DISPLAY_LEFT_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_32
	str     r1, [r8, #0x20]  // PBIU_PBAR3
	str     r2, [r8, #0x24]  // PBIU_PBLR3

	ldr	r1, =IQ80321_ROTARY_SWITCH_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_32
	str     r1, [r8, #0x28]  // PBIU_PBAR4
	str     r2, [r8, #0x2C]  // PBIU_PBLR4

	ldr	r1, =IQ80321_BATTERY_STATUS_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_32
	str     r1, [r8, #0x30]  // PBIU_PBAR5
	str     r2, [r8, #0x34]  // PBIU_PBLR5
	
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_1
	// ====================================================================

	// Enable the Icache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_I
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_2
	// ====================================================================

	// value to write into PBIU_PBAR0 to establish runtime flash address
	ldr     r1, =IQ80321_FLASH_ADDR | PBAR_FLASH | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_16

	// value to write into PBIU_PBLR0 to establish runtime flash address
	ldr	r2, =PBLR_SZ_8M

	// value to load into pc to jump to real runtime address
	ldr     r7, =1f

	ldr	r9, =IQ80321_DISPLAY_RIGHT_ADDR
	ldr	r10,=IQ80321_DISPLAY_LEFT_ADDR
	ldr	r11,=DISPLAY_F

	b       icache_boundary
	.p2align 5
icache_boundary:
	// Here is where we switch from boot address (0x000000000) to the
	// actual flash runtime address. We align to cache boundary so we
        // execute from cache during the switchover. Cachelines are 8 words.
	str     r1, [r8, #0x08]  // PBIU_PBAR0
	str     r2, [r8, #0x0c]  // PBIU_PBLR0
        nop
        nop
        mov     pc, r7
        str     r11, [r9]    // We should never reach this point. If we do,
        str     r11, [r10]   // display FF and loop forever.
    0:  b       0b           
    1:
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_3
	// ====================================================================
		
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
	
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_4
	// ====================================================================
		
	//
	// ***  I2C interface initialization ***
	//

	// Pointers to I2C Registers
	ldr	r11, =I2C_BASE0		// base address of the I2C unit
        
	//  Write 0 to avoid interfering with I2C bus.
	//  (See GPIO section in 80321 manual)
	ldr	r2, =GPIO_GPOD
	mov	r3, #0
	strb	r3, [r2]

	// Reset I2C Unit
	mov	r1, #ICR_RESET
	str	r1, [r11, #I2C_ICR0]
	ldr	r1, =0x7ff
	str	r1, [r11, #I2C_ISR0]
	mov	r1, #0
	str	r1, [r11, #I2C_ICR0]

	//  Setup I2C Slave Address Register
	mov	r1, #I2C_DEVID		// Load slave address r1.
	str	r1, [r11, #I2C_ISAR0]	// Save the value 0x02 (I2C_DEVID) in the register.

	//  Enable I2C Interface Unit - status will be polled
	ldr	r1, =ICR_GCALL | ICR_ENB | ICR_SCLENB
	str	r1, [r11, #I2C_ICR0]

	//
	//  *** Now read the SPD Data ***
	//

	// Initialize regs for loop
	mov	r4, #0		// SDRAM size
	mov	r5, #0		// R5 has running checksum calculation
	mov	r6, #0		// Counter incremented before byte is read
	mov	r7, #64		// Number of bytes to read in the Presence Detect EEPROM of SDRAM
	mov	r8, #0		// Flags: b0-b6 == bankcnt, b7 = x16 flag
	mov	r9, #RFR_15_6us	// Refresh rate (assume normal 15.6us)
	mov	r10, #0		// Bank size
	mov	r14, #0         // ECC flag

	ldr	r0, [r11, #I2C_ISR0]	// Load I2C Status Reg into R0
	str	r0, [r11, #I2C_ISR0]	// Clear status

	/*  FREE REGISTERS ARE R0 - R3 */

	// *** Put out address, with WRITE mode ***

	// Set SDRAM module address and write mode
	mov	r1, #SDRAM_DEVID	// Load slave address for SDRAM module. 0xA2 (Presence Detect Data)
	bic	r1, r1, #IDBR_MODE	// Clear read bit (bit #0)
	str	r1, [r11, #I2C_IDBR0]	// Store to data register

	// Initiate dummy write to set EEPROM pointer to 0
	ldr	r1, [r11, #I2C_ICR0]	// read the current Control Register value
	bic	r1, r1,	#ICR_STOP	// No stop bit
	orr	r1, r1,	#ICR_START | ICR_TRANSFER
	str	r1, [r11, #I2C_ICR0]	// Store to control register

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_0
	// ====================================================================
		
	// Wait for transmit empty status
	mov	r1, #I2C_TIMOUT		// Initialize I2C timeout counter
    0:	subs	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r11, #I2C_ISR0]	// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked, IDBR Transmit Empty
	beq	0b			// If bit = 0 then branch to 0 and check again
	str	r0, [r11, #I2C_ISR0]	// Write back status to clear

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_1
	// ====================================================================
		
	// Write pointer register on EEPROM to 0x00000000
	mov	r1, #0			// Load base address of SDRAM module EEPROM
	str	r1, [r11, #I2C_IDBR0]   // Store to data register

	//  Send address to EEPROM
	ldr	r1, [r11, #I2C_ICR0]	// read the current Control Register value
	bic	r1, r1,	#ICR_START | ICR_STOP
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing
	str	r1, [r11, #I2C_ICR0]	// Store to control register

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_2
	// ====================================================================
		
	// Wait for transmit empty status
	mov	r1, #I2C_TIMOUT		// Initialize I2C timeout counter
    0:	subs	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r11, #I2C_ISR0]	// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked, IDBR Transmit Empty
	beq	0b			// If bit = 0 then branch to 0 and check again
	str	r0, [r11, #I2C_ISR0]	// Write back status to clear
1:
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_3
	// ====================================================================
		
	// *** Read SDRAM PD data ***

	// *** Put out address, with READ mode ***

	//  Set SDRAM module address and read mode
	mov	r0, #SDRAM_DEVID	// Load slave address for SDRAM module (0xA2)
	orr	r1, r0, #IDBR_MODE	// Set read bit (bit #0)
	str	r1, [r11, #I2C_IDBR0]	// Store to data register

	//  Send next read request
	ldr	r1, [r11, #I2C_ICR0]	// read the current Control Register value
	bic	r1, r1,	#ICR_STOP	// No stop bit
	orr	r1, r1,	#ICR_START | ICR_TRANSFER
	str	r1, [r11, #I2C_ICR0]	// Store to control register

	// Wait for transmit empty status
	mov	r1, #I2C_TIMOUT		// Initialize I2C timeout counter
    0:	subs	r1, r1, #1		// Increment I2C timeout counter (r1 = r1 + 1)
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r11, #I2C_ISR0]	// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_EMPTY	// Bit #6 is checked, IDBR Transmit Empty
	beq	0b			// If bit = 0 then branch to 0 and check again
	str	r0, [r11, #I2C_ISR0]	// Write back status to clear

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_9, DISPLAY_4
	// ====================================================================

  spd_loop:
	// read the next Byte of Serial Presence Detect data

	ldr	r1, [r11, #I2C_ICR0]	// read the current Control Register value
	bic	r1, r1,	#ICR_START	// No start bit (already started)
	orr	r1, r1, #ICR_TRANSFER	// Set transfer bit - bit is self_clearing

	// we have to set NACK before reading the last byte
	add     r2, r6, #1
	cmp	r2, r7			// r7 = 64 (decimal) so if r6 = 64, this is the last byte to be read
	orreq	r1, r1, #ICR_ACK | ICR_STOP
	str	r1, [r11, #I2C_ICR0]	// Store to control register

	// Wait for read full status
	mov	r1, #I2C_TIMOUT		// Initialize I2C timeout counter
    0:	subs	r1, r1, #1		// decrement timeout
	beq	i2c_error		// Kick out of SDRAM initialization if timeout occurs
	ldr	r0, [r11, #I2C_ISR0]	// Load I2C Status Reg into R0
	ands	r3, r0, #ISR_FULL	// Bit #7 is checked
	beq	0b			// If bit = 0 then branch to 0 and check again
	str	r0, [r11, #I2C_ISR0]	// Write back status to clear

	ldr	r1, [r11, #I2C_IDBR0]	// Read the byte

	// check for checksum byte
	subs	r2, r6, #SPD_CHECKSUM
	addne	r5, r5, r1		// Add it to the checksum if not the checksum byte
	bne	1f			// skip checksum comparison
	and	r5, r5, #0xff		//	against the calculated checksum
	cmp	r1, r5
	beq	spd_continue

	// bad checksum
	HEX_DISPLAY r2, r3, DISPLAY_7, DISPLAY_7
	0: b 0b

    1:
	// Check for bank count byte
	subs	r2, r6, #SPD_BANKCNT
	moveq	r8, r1			// Store bank count
	beq	spd_continue
	
	// Check for ECC
	subs	r2, r6, #SPD_CONFIG
	bne	1f
	subs	r2, r1, #2
	addeq	r14, r14, #1
	b	spd_continue
    1:

	// Check for refresh rate
	subs	r2, r6, #SPD_REFRESH
	bne	1f

	ands	r2, r1, #0x7f
	moveq	r9, #RFR_15_6us
	subs	r3, r2, #1
	moveq	r9, #RFR_3_9us
	subs	r3, r2, #2
	moveq	r9, #RFR_7_8us

	b	spd_continue

    1:
 	// Check for SDRAM width byte
	subs	r2, r6, #SPD_SDRAM_WIDTH
	bne	1f

	ands	r2, r1, #0x10		// Check for data width of 16
	orr	r8, r8, r2, lsl #3      // set b7 in r8 if x16

#if 0 // drive strength doesn't depend on width
	ldreq   r2, =x8_table		// x8 if bit not set
	ldrne   r2, =x16_table		// x16 if bit not set
	b	init_drive_strength

    x16_table:
	.word   0x18	// Data Bus Pull Up
	.word   0x18	// Data Bus Pull Down
	.word   0x22	// Clock Pull Up
	.word   0x20	// Clock Pull Down
	.word   0x30	// Clock Enable Pull Up
	.word   0x30	// Clock Enable Pull Down
	.word   0x30	// Chip Select Pull Up
	.word   0x30	// Chip Select Pull Down
	.word   0x18	// Receive Enable Pull Up
	.word   0x18	// Receive Enable Pull Down
	.word   0x3c	// Address Bus Pull Up
	.word   0x3c	// Address Bus Pull Down

    x8_table:
	.word   0x18	// Data Bus Pull Up
	.word   0x18	// Data Bus Pull Down
	.word   0x22	// Clock Pull Up
	.word   0x20	// Clock Pull Down
	.word   0x30	// Clock Enable Pull Up
	.word   0x30	// Clock Enable Pull Down
	.word   0x30	// Chip Select Pull Up
	.word   0x30	// Chip Select Pull Down
	.word   0x18	// Receive Enable Pull Up
	.word   0x18	// Receive Enable Pull Down
	.word   0x3c	// Address Bus Pull Up
	.word   0x3c	// Address Bus Pull Down
#else
	b	spd_continue

    registered_table:
	.word   13	// Data Bus Pull Up
	.word   13	// Data Bus Pull Down
	.word   34	// Clock Pull Up
	.word   32	// Clock Pull Down
	.word   48	// Clock Enable Pull Up
	.word   48	// Clock Enable Pull Down
	.word   13	// Chip Select Pull Up
	.word   13	// Chip Select Pull Down
	.word   13	// Receive Enable Pull Up
	.word   13	// Receive Enable Pull Down
	.word   13	// Address Bus Pull Up
	.word   13	// Address Bus Pull Down

    unbuffered_table:
	.word   13	// Data Bus Pull Up
	.word   13	// Data Bus Pull Down
	.word   34	// Clock Pull Up
	.word   32	// Clock Pull Down
	.word   48	// Clock Enable Pull Up
	.word   48	// Clock Enable Pull Down
	.word   24	// Chip Select Pull Up
	.word   24	// Chip Select Pull Down
	.word   13	// Receive Enable Pull Up
	.word   13	// Receive Enable Pull Down
	.word   24	// Address Bus Pull Up
	.word   24	// Address Bus Pull Down
#endif

    init_drive_strength:

        ldr	r1, =MCU_DBUDSR
	mov	r3, #12         // 12 contiguous registers to set
    0:
	ldr	r0, [r2], #4	// load value
        str	r0, [r1], #4	// store to register
	subs	r3, r3, #1
	bne	0b
	b	spd_continue
    1:

	// Check for module attribute byte
	subs	r2, r6, #SPD_MOD_ATTRIB
	bne	1f
        ldr     r0, =MCU_SDCR
	mov     r2, #SDCR_INIT_VAL
	ands	r3, r1, #SPD_ATTRIB_REG_CTL  // check for registered modules
        beq     2f      
	orr	r2, r2, #2
	str	r2, [r0]
	ldr     r2, =registered_table
	b	init_drive_strength
    2:
	str	r2, [r0]
	ldr     r2, =unbuffered_table
	b	init_drive_strength
    1:

	// Check for bank size byte
	subs	r2, r6, #SPD_BANKSZ
	bne	1f
	mov	r10, r1, lsl #2		// Store bank size in Mbytes (shift left 2 bits)
	and     r3, r8, #0x7f           // isolate bank count     
	mul	r2, r3, r10		// Multiply by bank count to get DRAM size in MB
	mov	r4, r2, lsl #20		// Convert size to bytes  - r4 contains DRAM size in bytes
	b	spd_continue
    1:

  spd_continue:
	// Continue reading bytes if not done
	add	r6, r6, #1	// Increment byte counter
	cmp	r6, r7
	bne	spd_loop
	
	b	i2c_disable

	.ltorg
    i2c_error:
	// hit the leds if an error occurred
	HEX_DISPLAY r2, r3, DISPLAY_5, DISPLAY_5
	b i2c_error
    i2c_disable:
	//  Disable I2C Interface Unit
	ldr	r1, [r11, #I2C_ICR0] 
	bic	r1, r1, #ICR_ENB | ICR_SCLENB	// Disable I2C unit
	str	r1, [r11, #I2C_ICR0]
	
	// At this point, r4 = SDRAM size in bytes, r8 = Bank count, r10 = bank size in MB


	// *** SDRAM setup ***

	// Set the DDR SDRAM Base Register - SDBR (the lowest address for memory)
        ldr     r0, =MCU_SDBR
        mov     r1, #SDRAM_PHYS_BASE
        str     r1, [r0]

	// Set up bank 0 register
	subs	r1, r10, #32
	moveq	r0, #SBR_32MEG		// Program SDRAM Bank0 Boundary register to 32 MB
	beq	1f
	subs	r1, r10, #64		// do we have 64 MB banks?
	moveq	r0, #SBR_64MEG		// Program SDRAM Bank0 Boundary register to 64 MB
	beq	1f
	subs	r1, r10, #128		// do we have 128 MB banks?
	moveq	r0, #SBR_128MEG		// Program SDRAM Bank0 Boundary register to 128 MB
	beq	1f
	subs	r1, r10, #256		// do we have 256 MB banks?
	moveq	r0, #SBR_256MEG		// Program SDRAM Bank0 Boundary register to 64 MB
	beq	1f
	subs	r1, r10, #512		// do we have 512 MB banks?
	moveq	r0, #SBR_512MEG		// Program SDRAM Bank0 Boundary register to 64 MB
	beq	1f
	
     bank_err:
  	HEX_DISPLAY r2, r3, DISPLAY_F, DISPLAY_F
	b	bank_err
    1:
        mov     r1, #SDRAM_PHYS_BASE
        mov     r2, #0x1f
        and     r2, r2, r1, lsr #25
	add	r2, r2, r0

	ands	r1, r8, #0x80
	and	r8, r8, #0x7f
	beq	1f
	// x16
	subs	r1, r10, #128
	addeq   r2, r2, #0x80000000
    1:
	ldr	r1, =MCU_SBR0
	str	r2, [r1]		// store SBR0

	subs	r1, r8, #2		// do we have 2 banks???
	addeq	r2, r2, r0              // SBR1 == SBR0+r0 if two banks
	ldr	r1, =MCU_SBR1
	str	r2, [r1]

 	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_5
	// ====================================================================
		
	DELAY_FOR 0x1800000, r0

	//  Disable the refresh counter by setting the RFR to zero.
        // (from section 7.2.2.6 of the Verde technical specification)
        ldr     r0, =MCU_RFR
        mov     r1, #0
        str     r1, [r0]

        // Issue one NOP cycle after the 200 us device deselect. A NOP is 
        // accomplished by setting the SDIR to 0101.
        ldr     r0, =MCU_SDIR
        mov     r1, #SDIR_CMD_NOP
        str     r1, [r0]
	
        // Issue a precharge-all command to the DDR SDRAM interface by setting 
        // the SDIR to 0100.
        mov     r1, #SDIR_CMD_PRECHARGE_ALL
        str     r1, [r0]

        // Issue an extended-mode-register-set command to enable the DLL by 
        // writing 0110 to the SDIR.
        NOPs    8
		
        mov     r1, #SDIR_CMD_ENABLE_DLL
        str     r1, [r0]

        // After waiting T mrd cycles (4 clocks at 200 MHz), issue a 
        // mode-register-set command by writing to the SDIR to program the DDR 
        // SDRAM parameters and to Reset the DLL. Setting the SDIR to 0010 
        // programs the MCU for CAS Latency of two while setting the SDIR to 0011
        // programs the MCU for CAS Latency of two and one-half. The MCU supports 
        // the following DDR SDRAM mode parameters:
        //   a. CAS Latency (CL) = two or two and one-half
        //   b. Wrap Type (WT) = Sequential
        //   c. Burst Length (BL) = four
        NOPs    8

        mov     r1, #SDIR_CMD_CAS_LAT_2_A   // Set CAS Latency to 2
        str     r1, [r0]

        // After waiting T mrd cycles (4 clocks at 200 MHz), issue a precharge-all
        // command to the DDR SDRAM interface by setting the SDIR to 0100.
        NOPs    8

        mov     r1, #SDIR_CMD_PRECHARGE_ALL
        str     r1, [r0]
	
        // After waiting T rp cycles (4 clocks at 200 MHz), provide two 
        // auto-refresh cycles. An auto-refresh cycle is accomplished by 
        // setting the SDIR to 0111. Software must ensure at least T rfc 
        // cycles (16 clocks at 200 MHz) between each auto-refresh command.
        NOPs    8

        mov     r1, #SDIR_CMD_AUTO_REFRESH      // 1st of two auto-refresh cycle commands
        str     r1, [r0]

        NOPs    8

        str     r1, [r0]                        // 2nd of two auto-refresh cycle commands

        NOPs    8   

	// Issues a mode-register-set command by writing to the SDIR to program the
	// DDR SDRAM parameters without resetting the DLL. Setting the SDIR to 0000 
        // programs the MCU for CAS Latency of two while setting the SDIR to 0001
        // programs the MCU for CAS Latency of two and one-half. The MCU supports 
        // the following DDR SDRAM mode parameters:
        //   a. CAS Latency (CL) = two or two and one-half
        //   b. Wrap Type (WT) = Sequential
        //   c. Burst Length (BL) = four
        mov     r1, #SDIR_CMD_CAS_LAT_2_B       // Set CAS Latency to 2
        str     r1, [r0]

        NOPs    8
	
        mov     r1, #0xF                        // DDR Normal Operation
        str     r1, [r0]
	
        // Re-enable the refresh counter by setting the RFR to the required value.
        //
        ldr     r0, =MCU_RFR
        str     r9, [r0]

        // DSDR   - Data Strobe Delay Register                       (Section 7.6.25)
        ldr     r0, =MCU_DSDR
        ldr     r1, =DSDR_REC_VAL
        str     r1, [r0]

        // REDR   - Receive Enable Delay Register                    (Section 7.6.26)
        ldr     r0, =MCU_REDR
        ldr     r1, =REDR_REC_VAL
        str     r1, [r0]

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_6
	// ====================================================================
		
        // delay before using SDRAM 
        DELAY_FOR 0x1800000, r0
	
	// Enable the Dcache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_C
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        // Enable branch target buffer
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_BTB
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c7, 0   // flush Icache, Dcache and BTB
        CPWAIT  r0

        mcr     p15, 0, r0, c8, c7, 0   // flush instuction and data TLBs
        CPWAIT  r0

	mcr	p15, 0, r0, c7, c10, 4	// drain the write & fill buffers
	CPWAIT r0	
	
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_S, DISPLAY_L
	// ====================================================================
	
        ldr     r0, =MCU_ECTST           // clear test register
        mov     r1, #0
        str     r1, [r0]

        ldr     r0, =MCU_ECCR
        mov     r1, #0x0                 // disable ECC, disable reporting
        str     r1, [r0]

#ifdef CYGSEM_HAL_ARM_IQ80321_BATTERY_TEST
	//  Battery Backup SDRAM Memory Test
        //  Move test pattern into register prior to memory scrub
	ldr	r9, =SDRAM_BATTERY_TEST_ADDR
	ldr	r10, [r9]
#endif

	orrs	r14, r14, r14
	beq	no_ecc1

        ldr     r0, =MCU_ECCR
        mov     r1, #0x8                 // enable ECC, disable reporting
        str     r1, [r0]

  no_ecc1:

#ifdef CYGSEM_HAL_ARM_IQ80321_CLEAR_PCI_RETRY
	// Minimally setup ATU and release "retry" bit.
	ldr     r1, =ATU_IATVR2
	mov     r0, #SDRAM_PHYS_BASE
	str     r0, [r1]
	ldr	r0, =0xffffffff
	sub	r1, r4, #1
	sub	r0, r0, r1
	bic	r0, r0, #0x3f
	ldr	r1, =ATU_IALR2
	str     r0, [r1]
	ldr     r0, =((0xFFFFFFFF - ((64 * 1024 * 1024) - 1)) & 0xFFFFFFC0)
        ldr     r1, =ATU_IALR1
	str     r0, [r1]
	mov	r0, #0xc
        ldr     r1, =ATU_IABAR1
	str     r0, [r1]
        ldr     r1, =ATU_IABAR2
        str     r0, [r1]
	mov	r0, #0
        ldr     r1, =ATU_IAUBAR1
	str     r0, [r1]
        ldr     r1, =ATU_PCSR
	ldr	r0, [r1]
	and	r13, r0, #4     // save retry bit for later
	bic	r0, r0, #4
	str	r0, [r1]
#endif
	
        // scrub init
	mov	r12, r4		// size of memory to scrub
	mov	r8, r4		// save DRAM size
        mov     r0, #0
        mov     r1, #0
        mov     r2, #0
        mov     r3, #0
        mov     r4, #0
        mov     r5, #0
        mov     r6, #0
        mov     r7, #0

        ldr     r11, =SDRAM_UNCACHED_BASE

        // scrub Loop
    0:
        stmia   r11!, {r0-r7}
        subs    r12, r12, #32
        bne     0b

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_S, DISPLAY_E
	// ====================================================================

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_7
	// ====================================================================

        // clean, drain, flush the main Dcache
        ldr     r1, =DCACHE_FLUSH_AREA  // use a CACHEABLE area of
                                        // memory that's mapped above SDRAM
        mov     r0, #1024               // number of lines in the Dcache
    0:
        mcr     p15, 0, r1, c7, c2, 5   // allocate a Dcache line
        add     r1, r1, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_8
	// ====================================================================

	// clean, drain, flush the mini Dcache
        ldr     r2, =DCACHE_FLUSH_AREA + DCACHE_SIZE
        mov     r0, #64                 // number of lines in the Dcache
    0:
        mcr     p15, 0, r2, c7, c2, 5   // allocate a Dcache line
        add     r2, r2, #32          // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

        mcr     p15, 0, r0, c7, c6, 0   // flush Dcache
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_9
	// ====================================================================
	orrs	r14, r14, r14
	beq	no_ecc2
	
        ldr     r0, =MCU_MCISR
        mov     r1, #7
        str     r1, [r0]

        ldr     r0, =MCU_ECCR
        ldr     r1, =0x0f                        // enable ECC
        str     r1, [r0]
	
  no_ecc2:
	
	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_A
	// ====================================================================

#ifdef CYGSEM_HAL_ARM_IQ80321_BATTERY_TEST
	// Battery Backup SDRAM Memory Test
	// Store test pattern back into memory
	str	r10, [r9]
#endif

	// Save SDRAM size
        ldr     r1, =hal_dram_size  /* [see hal_intr.h] */
	str	r8, [r1]

#ifdef CYGSEM_HAL_ARM_IQ80321_CLEAR_PCI_RETRY
	// Save boot time retry flag.
        ldr     r1, =hal_pcsr_cfg_retry
	str	r13, [r1]
#endif

	// Move mmu tables into RAM so page table walks by the cpu
	// don't interfere with FLASH programming.
	ldr	r0, =mmu_table
	add     r2, r0, #0x4000     	// End of tables
	mov	r1, #SDRAM_BASE
	orr	r1, r1, #0x4000		// RAM tables

	// everything can go as-is
    1:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	cmp	r0, r2
	bne	1b

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_B
	// ====================================================================

        // clean, drain, flush the main Dcache
        ldr     r1, =DCACHE_FLUSH_AREA  // use a CACHEABLE area of memory
        mov     r0, #1024               // number of lines in the Dcache
    0:
        mcr     p15, 0, r1, c7, c2, 5   // allocate a Dcache line
        add     r1, r1, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

	// clean, drain, flush the mini Dcache
        ldr     r2, =DCACHE_FLUSH_AREA + DCACHE_SIZE
        mov     r0, #64                 // number of lines in the Dcache
    0:
        mcr     p15, 0, r2, c7, c2, 5   // allocate a Dcache line
        add     r2, r2, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

        mcr     p15, 0, r0, c7, c6, 0   // flush Dcache
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_C
	// ====================================================================

	// Set the TTB register to DRAM mmu_table
	ldr	r0, =(SDRAM_PHYS_BASE | 0x4000) // RAM tables
	mov	r1, #0
	mcr	p15, 0, r1, c7, c5, 0		// flush I cache
	mcr	p15, 0, r1, c7, c10, 4		// drain WB
	mcr	p15, 0, r0, c2, c0, 0		// load page table pointer
	mcr	p15, 0, r1, c8, c7, 0		// flush TLBs
	CPWAIT  r0

	// ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_D
	// ====================================================================
	.endm    // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  hal_pcsr_cfg_retry
hal_pcsr_cfg_retry:   .long   0  // Boot-time value of PCSR Retry bit.
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
