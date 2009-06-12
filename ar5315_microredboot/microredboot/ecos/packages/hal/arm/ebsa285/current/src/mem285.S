// #========================================================================
// #
// #    mem285.S
// #
// #    StrongARM EBSA-285 memory setup
// #
// #========================================================================
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
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     Red Hat, hmt
// # Contributors:  Red Hat, hmt
// # Date:          1999-07-05
// # Purpose:       StrongARM EBSA-285 SDRAM initialization
// # Description:   SDRAM Initialization for Intel(R) SA-110 21285 Companion
// #                Chip
// #                Intel is a Registered Trademark of Intel Corporation.
// #                Other Brands and Trademarks are the property of their
// #                respective owners.        
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

//	.file "mem285.S"
        .title "SDRAM Init for Intel(R) SA-110 21285 Companion Chip"

#include <cyg/hal/hal_ebsa285.h>

#include <pkgconf/system.h>

	.text
        .align     4

#define ARRAY_0_MODE_REGISTER     (SA110_SDRAM_ARRAY_0_MODE_REGISTER_BASE)
#define ARRAY_1_MODE_REGISTER     (SA110_SDRAM_ARRAY_1_MODE_REGISTER_BASE)

#define MODE_REGISTER_STEP (ARRAY_1_MODE_REGISTER - ARRAY_0_MODE_REGISTER)

        // [6:4] /CAS Latency is 2 (2)
        // [ 3 ] Burst Type is 0, Sequential
        // [2:0] Burst Length is 2, meaning 4
#define SDRAM_MODE_REGISTER_SETUP 0x22
        // Shifted left 2 because this is a word-address-offset!
#define SDRAM_MODE_REGISTER_SETUP_OFFSET ((SDRAM_MODE_REGISTER_SETUP) << 2)

	
#define SDRAM_TIMING_VALUE        (SA110_SDRAM_ROW_PRECHARGE_2_CYCLES    | \
                                   SA110_SDRAM_LAST_DATA_IN_3_CYCLES     | \
                                   SA110_SDRAM_RAS_TO_CAS_DELAY_2_CYCLES | \
                                   SA110_SDRAM_CAS_LATENCY_2_CYCLES      | \
                                   SA110_SDRAM_ROW_CYCLE_TIME_4_CYCLES   | \
                                   SA110_SDRAM_COMMAND_DRIVE_SAME_CYCLE)

#define SDRAM_TIMING_VALUE_MIN    (SDRAM_TIMING_VALUE                    | \
                                   SA110_SDRAM_REFRESH_INTERVAL_MIN)

#define SDRAM_TIMING_VALUE_NORMAL (SDRAM_TIMING_VALUE                    | \
                                   SA110_SDRAM_REFRESH_INTERVAL_NORMAL)



	/*
	 * This subroutine sizes and configures up to four banks of SDRAM DIMMs.
	 * It runs early without a stack.
	 *
	 * R0 - R9 are destroyed. All others preserved.
	 * Except r11 which is also destroyed.
	 *
	 */
	.global __mem285_init
__mem285_init:

	/*
	 * First we find out whether the SDRAMs are already initialized,
	 * and if so, leave them alone.  RAM start implies just do the
	 * sizing sums to return top of memory.
	 */
	ldr	r0, =SA110_CONTROL_STATUS_BASE

#ifndef CYG_HAL_STARTUP_RAM
	// This is conditional even in ROM start for
	// a) testing ROM images eg. stubs in RAM really
	// b) cooperation with eg. POST code, so we are not really at reset
	ldr	r0, =SA110_CONTROL_STATUS_BASE
        ldr     r1, [r0, #SA110_SDRAM_TIMING_o]
	ldr	r2, =SDRAM_TIMING_VALUE_NORMAL
	cmps	r1, r2
	movne	r11, #0
	bne	12f
#endif	// ! defined CYG_HAL_STARTUP_RAM

	// Add up the sizes and return in r0:
	mov	r1, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o
	mov	r2, #0
	mov	r3, #1
    1:
	ldr	r4, [r0, r1]
	ands	r4, r4, #7		// if zero, no mem here
	addne	r2, r2, r3, asl r4	// tot up array sizes (in 1/2 Megs)
	add	r1, r1, #4
	cmps	r1, #(SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o + 4)
	blt	1b

	mov	r0, r2, asl #19		// get size into Mb
	mov	pc, lr

#ifndef CYG_HAL_STARTUP_RAM

    12:
	/*
         * Write to the SDRAM Timing Register in the 21285.  Disable
         * refresh totally.
         */
        mov     r1, #0
        str     r1, [r0, #SA110_SDRAM_TIMING_o]

	// Disable each array
	mov	r1, #0
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o]
	
        // Wait for 8 refresh cycles to complete
        mov     r1, #(9 * 32)
    1:  subs    r1, r1, #1
        bpl     1b

        /*
         * Force an all-banks recharge on all four SDRAM arrays
         *
         * This code came from the SA-IOP ver 1.0 (3-16-98) spec pg 22
         *
         * You must access all four arrays regardless of whether there is
         * memory there because the 21285 counts the precharge accesses and
         * inhibits access to the SDRAM until all four have been done.
         *
         * An all banks rechargs is initiated by a read from any address
         * in the mode register space.
         */

        mov     r1, #3 
        mov     r0, #ARRAY_0_MODE_REGISTER
   1:
        ldr     r2, [r0]
        add     r0, r0, #MODE_REGISTER_STEP
        subs    r1, r1, #1
        bpl     1b

        /*
         * Now we need to write to the SDRAM Mode Register.
         * The address is important, not the data.  The mode register
         * should be configured for a burst size of 4 with linear addressing
         */
        mov     r1, #3
        mov     r0, #ARRAY_0_MODE_REGISTER
   1:
        str     r0, [r0, #SDRAM_MODE_REGISTER_SETUP_OFFSET]
        add     r0, r0, #MODE_REGISTER_STEP
        subs    r1, r1, #1
        bpl     1b
        
        /*
         * Write to the SDRAM Timing Register in the 21285.  Set the
         * refresh interval to the minimum because we have to wait for
         * 8 refresh cycles to complete before we can rely on the SDRAMs
         * to be operating normally
         */
	ldr	r0, =SA110_CONTROL_STATUS_BASE
        ldr     r1, =SDRAM_TIMING_VALUE_MIN
        str     r1, [r0, #SA110_SDRAM_TIMING_o]

	// Disable each array
	mov	r1, #0
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o]
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o]
	
        // Wait for 8 refresh cycles to complete
        mov     r1, #(9 * 32)
    1:  subs    r1, r1, #1
        bpl     1b

        // Now reset the Refresh interval to a sensible value
        ldr     r1, =SDRAM_TIMING_VALUE_NORMAL
        str     r1, [r0, #SA110_SDRAM_TIMING_o]
	
	/* start out assuming 64M part with MUX mode 2 */
	mov	r1, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE2)
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o]
	add	r1, r1, #(64 << 20) // Add 64Mb
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o]
	add	r1, r1, #(64 << 20) // Add 64Mb again
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o]
	add	r1, r1, #(64 << 20) // Add 64Mb and again
        str     r1, [r0, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o]
	
	/*
	 * First, try to figure out which banks are populated and
	 * the real mux mode for those banks.
	 *
	 * At this point:
	 *   r0 - Base of control/status registers
	 *
	 * Register usage:
	 *   r8 - offset to SDRAM addr/size register
	 *   r5 - pattern
	 *   r4 - inverse pattern
	 *   r3 - scratch/mux mode output
         *   r2 - scratch offset
         *   r1 - base address of 64M block in consideration
         *   r0 - base address of control register sets
	 */	
	mov	r8, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o
	mov     r1, #0

	ldr	r5, =0x12345678
	mvn	r4, r5

    20:
	str	r5, [r1]      // Offset 0 should work regardless
	str	r4, [r1, #4]  // put something else on the data bus
	ldr	r3, [r1]      // read back original
	cmps    r3, r5

	// If we didn't read pattern, then no memory present
	movne	r3, #0
	strne	r3, [r0, r8]            // write to addr/size register
	bne	49f                     // straight to next loop

	/*
	 * This bank is populated, so try to determine mux mode.
	 * All banks are currently set for mux mode 2.
	 */

	// A21 having no effect distinguishes the need for mux mode 0.
	str	r5, [r1]
	mov	r2, #(1 << 21)
	str	r4, [r1, r2]   // Store bad value at A21 mirror address
				 // expect to trash value at r1 if mode 0
	ldr	r3, [r1]
	cmps	r3, r5
	// If we don't read back pattern, then its mux mode 0
	
	// Force to 32M size to include A18 when sizing:
	movne	r3, #(SA110_SDRAM_SIZE_32MB | SA110_SDRAM_MUX_MODE0)
	bne	2f

	// A23 having effect distinguishes the need for mux mode 2.
	str	r5, [r1]
	mov	r2, #(1 << 23)
        str	r4, [r1, r2]   // Store bad value at A23 mirror address
                               // expect to preserve value at r1 if mode 2
	ldr	r3, [r1]
	cmps	r3, r5
	// if pattern still there, then mode 2
	moveq	r3, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE2)
	beq	2f

	// A22 having effect distinguishes the need for mux mode 4.
	str	r5, [r1]
	mov	r2, #(1 << 22)
	str	r4, [r1, r2]   // Store bad value at A22 mirror address
                               // expect to preserve value at r1 if mode 4
	ldr	r3, [r1]
	cmps	r3, r5
	// if pattern A still there, then mode 4
	moveq	r3, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE4)
	beq	2f

	/*
	 * At this point it is either mode 1 or 3. There is no clear cut
	 * test to differentiate the two, so make a best guess now, then
	 * correct later (if necessary) while sizing the bank.
	 */

        // NB the bank is still in mux mode 2, so A24 is fed to the wire for
        // A22 (mode 1) or no-connection (mode 3); so:
        // A24 having effect distinguishes the need for mux mode 1
        // A24 having no effect distinguishes the need for mux mode 3
	str	r5, [r1]
	mov	r2, #(1 << 24)
	str	r4, [r1, r2]
	ldr	r3, [r1]
	cmps	r3, r5
	// If pattern, try mode 1
	moveq	r3, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE1)
        // otherwise, try mode 3
	movne	r3, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE3)

	bne 2f

    2:
	orr	r3, r3, r1			// add in base address
	str	r3, [r0, r8]			// write to addr/size register
        
	/*
	 * Now that mux mode for this array is (hopefully) setup, we can try
	 * to size this SDRAM array.
         * 
         * Register usage:
         *    r8 - offset to current size/mode register
         *    r1 - offset to current base (in 64M blocks)
         *    r0 - base address of control register sets
	 */

        mov     r4, #(63 << 20)         // 63Mb to start with
    1:  str     r4, [r1, r4]
        subs    r4, r4, #(1 << 20)      // go down in increments of 1Mb
        bpl     1b

	str	r4, [r1, #4]            // change pattern on data bus

	// search for first unexpected data in ascending order
        mov     r4, #0
    1:
	ldr	r5, [r1, r4]
        cmps    r5, r4
	bne	23f                     // different so end of array
        add     r4, r4, #(1 << 20)      // go up in increments of 1Mb
        cmps    r4, #(64 << 20)
	blt	1b
        // fall-through assumes it is a 64Mb device
   23:	
	movs	r4, r4, lsr #20		// get a plain number of Mb
	// if this gave a zero, maybe we were mistaken about the RAM
        // working earlier: disable this bank.
	streq	r4, [r0, r8]            // write to addr/size register
	beq	49f                     // straight to next loop

        // apparently, mode 3 devices *must* be 8Mb; if we got a different
        // answer, set it to mode 1 and go back to try again:
	cmps    r4, #8
        beq     4f

	// skip if 8Mb; we are happy
	ldr     r3, [r0, r8]  	        // read in the mode we set
	and	r3, r3, #SA110_SDRAM_MUX_MODE_MASK
	cmp	r3, #SA110_SDRAM_MUX_MODE3
	// Must be misconfigured mux mode. Set to mode 1 and retry
	moveq	r3, #(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE1)
	beq     2b
	// not mux mode 3; drop though OK

    4:
	// convert MB size to register size val
	mov	r5, #0
	mov     r3, r4
    5:  movs    r3, r3, lsr #1
        add	r5, r5, #1
        bcc     5b

	// Double check that the size was a power of 2
	mov     r6, #1
        mov     r6, r6, lsl r5		// should get Mb count back doubled
	cmps	r6, r4, lsl #1          // compare with doubled
	movne	r5, #0			// disable this bank
        ldr	r3, [r0, r8]            // Load current setting
	bic     r3, r3, #7
	orr	r3, r3, r5		// insert the correct size code
	str	r3, [r0, r8]		// into the control register

   49:
	add	r8, r8, #4			// next addr/size register
	add	r1, r1, #(64<<20)		// next array
        cmps    r1, #(256<<20)                  // top address + 1 bank
	blt	20b
        // END of main loop to size all 4 DRAM banks

	/*
	 * At this point, the size values are all in the control registers.
	 *
	 * We want to set memory up to be contiguous. Since the
	 * banks' base address needs to be naturally aligned, we
	 * need to sort the bank sizes from large to small.
	 *
	 * Register usage:
         *   r0 - base address of control register sets
	 *   r1	- bitmap of which slots we have covered in toto
	 *   r2	- cumulative base address of mapped SDRAM
	 *   r3	- biggest size code this pass
	 *   r4	- bit index of current slot
	 *   r5	- bit index of biggest slot found this pass
	 *   r6	- scratch control reg contents
	 *   r7 - scratch size code
	 *   r8 - address of current slot's control register
	 *   r9	- address of biggest slot found's control register
	 */
	mov	r1, #0			// bitmap of which we have covered
	mov	r2, #0			// cumulative base address
	// do... until there are no more slots to deal with
    70:
	mov	r3, #0			// biggest this pass
	mov     r4, #1			// bit index of current slot
	mov	r5, #0			// bit index of biggest slot found
	mov	r8, #SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o
	mov	r9, #0			// address of biggest slot found
	// Foreach slot we have not yet dealt with
    75:
	tst	r4, r1
	bne	88f
	ldr	r6, [r0, r8]
	and	r7, r6, #7
	cmps	r7, r3
	movgt	r3, r7			// save biggest's size
	movgt	r5, r4			// save biggest's index
	movgt	r9, r8			// save biggest's reg address
    88:
	mov	r4, r4, asl #1
	add	r8, r8, #4
	cmps	r4, #0x10
	blt	75b			// next slot
	// Did we find a largest slot?
	cmps	r5, #0
	beq	95f	// No!  Finished

	orr	r1, r1, r5		// can forget r4 and r5 now
	ldr	r6, [r0, r9]		// get the control register
	bic	r6, r6, #0x0ff00000	// clear base address bits
	orr	r6, r6, r2		// insert base address to use
	str	r6, [r0, r9]		// store the new control register
	mov	r6, #1
	mov	r6, r6, asl r3
	mov	r6, r6, asl #19		// 1 << (size-code + 19) is size
	add	r2, r2, r6		// increment the cumulating address

	b	70b			// go look for the next one

    95:	// all done!
	// at this point, r2 contains the top of memory.
	// (r11 is the value from last time or zero if first time)

	cmps	r11, r2			// Same answer as last time?
	movne	r11, r2			// if not, save memsize
	bne	12b			// ...and try again.
	
	mov	r0, r2
	mov	pc, lr	
#endif // ! defined CYG_HAL_STARTUP_RAM
//FUNC_END __mem285_init

/* EOF mem285.S */
