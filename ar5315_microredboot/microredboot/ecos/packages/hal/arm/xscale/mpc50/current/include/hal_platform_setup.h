/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    <knud.woehler@microplex.de>
// Date:         2003-01-09
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

#include <pkgconf/system.h>		// System-wide configuration info
#include CYGBLD_HAL_VARIANT_H		// Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H	// Platform specific configuration
#include <cyg/hal/hal_pxa2x0.h>	// Platform specific hardware definitions
#include <cyg/hal/hal_mmu.h>		// MMU definitions
#include <cyg/hal/hal_mm.h>		// more MMU definitions
#include <cyg/hal/mpc50.h>

/**********************************************************************************************************************
* MMU/Cache 
**********************************************************************************************************************/
.macro init_mmu_cache_on
		ldr		r0, =0x2001					
		mcr		p15, 0, r0, c15, c1, 0
		mcr		p15, 0, r0, c7, c10, 4		// drain the write & fill buffers
		CPWAIT	r0	
		mcr		p15, 0, r0, c7, c7, 0		// flush Icache, Dcache and BTB
		CPWAIT	r0	
		mcr		p15, 0, r0, c8, c7, 0		// flush instuction and data TLBs
		CPWAIT	r0	
	
		// Icache on
		mrc		p15, 0, r0, c1, c0, 0
		orr		r0, r0, #MMU_Control_I
		orr		r0, r0, #MMU_Control_BTB		// Enable the BTB
		mcr		p15, 0, r0, c1, c0, 0
		CPWAIT	r0

		// create stack for "C"
		ldr     r1,=__startup_stack
		ldr     r2,=PXA2X0_RAM_BANK0_BASE
		orr     sp,r1,r2
		bl		hal_mmu_init	// create MMU Tables 

		// Enable permission checks in all domains
		ldr		r0, =0x55555555
		mcr		p15, 0, r0, c3, c0, 0
	
		// MMU on
		ldr     r2,=1f
		mrc		p15, 0, r0, c1, c0, 0
		orr		r0, r0, #MMU_Control_M
		orr		r0, r0, #MMU_Control_R
		mcr		p15, 0, r0, c1, c0, 0
		mov		pc,r2
		nop
		nop
		nop
1:
	
		mcr		p15, 0, r0, c7, c10, 4		// drain the write & fill buffers
		CPWAIT	r0	
	
		// Dcache on
		mrc		p15, 0, r0, c1, c0, 0
		orr		r0, r0, #MMU_Control_C
		mcr		p15, 0, r0, c1, c0, 0
		CPWAIT	r0

		// clean/drain/flush the main Dcache
		mov		r1, #0xc0000000
		mov		r0, #1024
2:
		mcr		p15, 0, r1, c7, c2, 5
		add		r1, r1, #32
		subs	r0, r0, #1
		bne		2b

		// clean/drain/flush the mini Dcache
		//ldr	r1, =(DCACHE_FLUSH_AREA+DCACHE_SIZE) // use a CACHEABLE area of
		mov		r0, #64					// number of lines in the mini Dcache
3:
		mcr		p15, 0, r1, c7, c2, 5	// allocate a Dcache line
		add		r1, r1, #32				// increment the address to
		subs	r0, r0, #1				// decrement the loop count
		bne		3b

		// flush Dcache
		mcr		p15, 0, r0, c7, c6, 0
		CPWAIT	r0

		// drain the write & fill buffers
		mcr		p15, 0, r0, c7, c10, 4
		CPWAIT	r0	

.endm



.macro	init_mmu_off
		mov		r0, #0x78			 
		mcr		p15, 0, r0, c1, c0, 0	// caches off -- MMU off or ID map
		mcr		p15, 0, r0, c7, c7, 0	// Invalidate the I & D cache, mini- d cache, and BTB
		mcr		p15, 0, r0, c7, c10, 4	// Drain write buffer -- r0 ignored
		CPWAIT	r0
		nop
		nop
		nop
		nop
		mvn		r0, #0                      
		mcr		p15, 0, r0, c3, c0, 0  
.endm

/**********************************************************************************************************************
* Clock 
**********************************************************************************************************************/
#define CCCR_OFFS (PXA2X0_CCCR-PXA2X0_CLK_BASE)
.macro init_clks
		ldr		r1, =PXA2X0_CLK_BASE
		// TurboMode=400MHz/RunMode=200MHz/Memory=100MHz/SDRam=100MHz
//		ldr		r0, =(PXA2X0_CCCR_L27 | PXA2X0_CCCR_M2 | PXA2X0_CCCR_N20)
		// TurboMode=300MHz/RunMode=200MHz/Memory=100MHz/SDRam=100MHz	
//		ldr		r0, =(PXA2X0_CCCR_L27 | PXA2X0_CCCR_M2 | PXA2X0_CCCR_N15)	
		adr		r0, mpc50_static_info
		ldr		r0, [r0, #MPC50_VAL_OFFS_CCCR]

		str		r0, [r1, #CCCR_OFFS]					// set Core Clock
		mov		r0,	#3
		mcr		p14, 0, r0, c6, c0, 0					// Turbo Mode on
.endm

/**********************************************************************************************************************
* Interrupt controller
**********************************************************************************************************************/
#define ICLR_OFFS (PXA2X0_ICLR-PXA2X0_IC_BASE)
#define ICMR_OFFS (PXA2X0_ICMR-PXA2X0_IC_BASE)
.macro init_intc_cnt
		ldr		r1, =PXA2X0_IC_BASE
		mov		r0, #0
		str		r0, [r1, #ICLR_OFFS]									// clear Interrupt level Register
		str		r0,	[r1, #ICMR_OFFS]									// clear Interrupt mask Register
.endm

/**********************************************************************************************************************
* SDRAM 
**********************************************************************************************************************/

//#define MDCNFG_VAL	0x094B094B	// SDRAM Config Reg (32Bit, 9 Col, 13 Row, 2 Bank, CL2)
//#define MDREFR_VAL	0x0005b018  // SDRAM Refresh Reg SDCLK=memory clock
//#define MDREFR_VAL	0x000ff018  // SDRAM Refresh Reg SDCLK=1/2 memory clock
#define MDMRS_VAL	0x00000000  // SDRAM Mode Reg Set Config Reg
#define MSC0_VAL	0x199123da	// CS1(FPGA)/CS0(Flash)
#define MSC1_VAL	0x7ff07ff1	// CS3(not used)/CS2(ETH)

#define OSCR_OFFS	(PXA2X0_OSCR-PXA2X0_OSTIMER_BASE)
#define MDREFR_OFFS	(PXA2X0_MDREFR-PXA2X0_MEMORY_CTL_BASE)
#define MDCNFG_OFFS	(PXA2X0_MDCNFG-PXA2X0_MEMORY_CTL_BASE)
#define MDMRS_OFFS	(PXA2X0_MDMRS-PXA2X0_MEMORY_CTL_BASE)
#define MSC0_OFFS	(PXA2X0_MSC0-PXA2X0_MEMORY_CTL_BASE)
#define MSC1_OFFS	(PXA2X0_MSC1-PXA2X0_MEMORY_CTL_BASE)
#define MSC2_OFFS	(PXA2X0_MSC2-PXA2X0_MEMORY_CTL_BASE)
.macro init_sdram_cnt
// Hardware Reset Operation (S. 5-83)
// Step 1

// wait 200 usec
		ldr		r1,	=PXA2X0_OSTIMER_BASE								// set OS Timer Count
		mov		r0,	#0
		str		r0,	[r1, #OSCR_OFFS] 
		ldr		r2,	=0x300												// wait 200 usec 
61:		
		ldr		r0,	[r1, #OSCR_OFFS] 
		cmp		r2,	r0
		bgt		61b

		ldr		r1,  =PXA2X0_MEMORY_CTL_BASE

        ldr     r0,  =MSC0_VAL
		str		r0,	[r1, #MSC0_OFFS]									// FPGA/Flash
        
		ldr     r0,  =MSC1_VAL
		str		r0,	[r1, #MSC1_OFFS]									// ETH

		// Refresh Register 
        //ldr     r3,  =MDREFR_VAL										// load SDRAM refresh info
		adr		r3,  mpc50_static_info
		ldr		r3,  [r3, #MPC50_VAL_OFFS_MDREFR]


		ldr     r2,  =0xFFF												// DRI field
        and     r3,  r3,  r2                 
        ldr     r4,  [r1, #MDREFR_OFFS]									// read Reset Status
        bic     r4,  r4,  r2
 		bic     r4,  r4,  #(0x01000000 | 0x02000000)					// clear K1Free, K2Free, 
		bic		r4,  r4,  #0x00004000									// K0DB2
        orr     r4,  r4,  r3											// add DRI field
        str     r4,  [r1, #MDREFR_OFFS]									// 
        ldr     r4,  [r1, #MDREFR_OFFS]

// Step 2
// 

// Step 3
        //ldr     r3,  =MDREFR_VAL										// load SDRAM Refresh Info 
		adr		r3,  mpc50_static_info
		ldr		r3,  [r3, #MPC50_VAL_OFFS_MDREFR]


		ldr     r2,  =0x000f0000										
        and     r3,  r3,  r2                 
        orr     r4,  r4,  r3
        str     r4,  [r1, #MDREFR_OFFS]
        ldr     r4,  [r1, #MDREFR_OFFS]        
        
        bic     r4,  r4,  #0x00400000									// Self Refresh off
        str     r4,  [r1, #MDREFR_OFFS]
        ldr     r4,  [r1, #MDREFR_OFFS]
       
		orr     r4,  r4,  #0x00008000									// SDCKE1 on
        str     r4,  [r1, #MDREFR_OFFS]
        ldr     r4,  [r1, #MDREFR_OFFS]

		orr     r4,  r4,  #0x00800000									// K0Free on
        str     r4,  [r1, #MDREFR_OFFS]
        ldr     r4,  [r1, #MDREFR_OFFS]
		nop
        nop

 // Step 4       
		//ldr     r2,  =MDCNFG_VAL
		adr		r2,  mpc50_static_info
		ldr		r2,  [r2, #MPC50_VAL_OFFS_MDCNFG]


		bic     r2,  r2,  #0x0003		// DE1-0
        bic     r2,  r2,  #0x00030000	// DE3-2
        str     r2,  [r1, #MDCNFG_OFFS]

// Step 5
// wait 200 usec 
		ldr r1,	=PXA2X0_OSTIMER_BASE
		mov r0,	#0
		str r0,	[r1, #OSCR_OFFS] 
		ldr r2,	=0x300
71:	
		ldr r0,	[r1, #OSCR_OFFS] 
		cmp r2,	r0
		bgt 71b	
		
// Step 6
		mov    r0, #0x78
		mcr    p15, 0, r0, c1, c0, 0	// (caches off, MMU off, etc.)

// Step 7
	    ldr     r2, =PXA2X0_RAM_BANK0_BASE
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]
	    str     r2, [r2]

// Step 8
//
// Step 9
// SDRAM enable
  		//ldr     r3,  =MDCNFG_VAL
		adr		r3,  mpc50_static_info
		ldr		r3,  [r3, #MPC50_VAL_OFFS_MDCNFG]


		ldr     r2,  =0x00030003
        and     r2,  r3,  r2                 
		ldr		r1,  =PXA2X0_MEMORY_CTL_BASE
        ldr     r3,  [r1, #MDCNFG_OFFS]
		orr     r3,  r3,  r2
		str     r3,  [r1, #MDCNFG_OFFS]
  		
// Step 10
        ldr     r2,  =MDMRS_VAL
        str     r2,  [r1, #MDMRS_OFFS]
.endm


/**********************************************************************************************************************
* GPIOs
**********************************************************************************************************************/

// GPIO		Name			Pin		GPDR	GAFR	GPSR

//	0		INT0			L10		0		00		0
//	1		INT1			L12		0		00		0
//	2		FLX-CLK			L13		1		00		0
//	3		FLX-DAT			K14		1		00		0

//	4		FLX-CONFIG		J12		1		00		1
//	5		FLX-STATUS		J11		0		00		0
//	6		FLX-CONFDONE	H14		0		00		0
//	7		LAN_INT			G15		0		00		0

//	8		USB-H-ON		F14		1		00		1
//	9		USB-CL-ON		F12		0		00		0
//	10		MPSB-INT0		F7		0		00		0
//	11		-				A7		0		00		0

//	12		MPSB-INT1		B6		0		00		0
//	13		MBGNT			B5		1		10		0
//	14		MBREQ			B4		0		01		0
//	15		nCS_1			T8		1		10		1

#define GAFR0_L_VAL		0x80000000 //0x98000000

//	16		PWM0			E12		0		00		0
//	17		PWM1			D12		0		00		0
//	18		RDY				C1		0		01		0
//	19		DREQ[1]			N14		0		00		0

//	20		DREQ[0]			N12		0		00		0
//	21		DVAL0			N15		0		00		0
//	22		DVAL1			M12		0		00		0
//	23		SSPSCLK			F9		0		00		0

//	24		SSPSFRM			E9		0		00		0
//	25		SSPTXD			D9		0		00		0
//	26		SSPRXD			A9		0		00		0
//	27		SSPEXTCLK		B9		0		00		0

//	28		BITCLK			C9		0		00		0
//	29		SDATIN0			E10		0		00		0
//	30		SDATOUT			A10		0		00		0
//	31		SYNC			E11		0		00		0

#define GPDR0_VAL		0x0000811c //0x0000a11c
#define GAFR0_U_VAL		0x00000010
#define GPSR0_VAL		0x00008110

//	32		SDATIN1			A16		0		00		0
//	33		Reset-Button	T13		0		00		0
//	34		FFRXD			A13		0		01		0
//	35		FFCTS			A14		0		01		0

//	36		FFDCD			A12		0		01		0
//	37		FFDSR			B11		0		01		0
//	38		FFRI			B10		0		01		0
//	39		FFTXD			E13		1		10		1

//	40		FFDTR			F10		1		10		1
//	41		FFRTS			F8		1		10		1
//	42		BTRXD			B13		0		00		0
//	43		BTTXD			D13		0		00		0

//	44		BTCTS			A15		0		00		0
//	45		BTRTS			B14		0		00		0
//	46		IRRXD			B15		0		00		0
//	47		IRTXD			C15		0		00		0

#define GAFR1_L_VAL		0x000a9550


//	48		LED_DP			P13		1		00		0
//	49		LED_G			T14		1		00		0
//	50		LED_F			T15		1		00		0
//	51		LED_E			R15		1		00		0

//	52		LED_D			P14		1		00		0
//	53		LED_C			R16		1		00		0
//	54		LED_B			P16		1		00		0
//	55		LED_A			M13		1		00		0

//	56		GPIO56			N16		0		00		0
//	57		GPIO57			M16		0		00		0
//	58		LCDD0			E7		0		00		0
//	59		LCDD1			D7		0		00		0

//	60		LCDD2			C7		0		00		0
//	61		LCDD3			B7		0		00		0
//	62		LCDD4			E6		0		00		0
//	63		LCDD5			D6		0		00		0

#define GPDR1_VAL		0x00ff0380
#define GAFR1_U_VAL		0x00000000
#define GPSR1_VAL		0x00000380
#define	GPSR_LED_VAL	0x00ff0000

//	64		LCDD6			E5		0		00		0
//	65		LCDD7			A6		0		00		0
//	66		LCDD8			C5		0		00		0
//	67		LCDD9			A5		0		00		0

//	68		LCDD10			D5		0		00		0
//	69		LCDD11			A4		0		00		0
//	70		LCDD12			A3		0		00		0
//	71		LCDD13			A2		0		00		0

//	72		LCDD14			C3		0		00		0
//	73		LCDD15			B3		0		00		0
//	74		LCDFCLK			E8		0		00		0
//	75		LCDLCLK			D8		0		00		0

//	76		LCDPCLK			B8		0		00		0
//	77		LCDBIAS			A8		0		00		0
//	78		n_CS2			P9		1		10		1
//	79		LAN_RES			T9		1		00		1

#define GAFR2_L_VAL		0x20000000

//	80		n_CS4			R13		0		00		0

#define GPDR2_VAL		0x0000c000
#define GAFR2_U_VAL		0x00000000
#define GPSR2_VAL		0x0000c000


#define GPSR0_OFFS		(PXA2X0_GPSR0-PXA2X0_GPIO_BASE)
#define GPCR0_OFFS		(PXA2X0_GPCR0-PXA2X0_GPIO_BASE)
#define GPSR1_OFFS		(PXA2X0_GPSR1-PXA2X0_GPIO_BASE)
#define GPCR1_OFFS		(PXA2X0_GPCR1-PXA2X0_GPIO_BASE)
#define GPSR2_OFFS		(PXA2X0_GPSR2-PXA2X0_GPIO_BASE)
#define GPCR2_OFFS		(PXA2X0_GPCR2-PXA2X0_GPIO_BASE)
#define GPDR0_OFFS		(PXA2X0_GPDR0-PXA2X0_GPIO_BASE)
#define GPDR1_OFFS		(PXA2X0_GPDR1-PXA2X0_GPIO_BASE)
#define GPDR2_OFFS		(PXA2X0_GPDR2-PXA2X0_GPIO_BASE)
#define GAFR0_L_OFFS	(PXA2X0_GAFR0_L-PXA2X0_GPIO_BASE)
#define GAFR0_U_OFFS	(PXA2X0_GAFR0_U-PXA2X0_GPIO_BASE)
#define GAFR1_L_OFFS	(PXA2X0_GAFR1_L-PXA2X0_GPIO_BASE)
#define GAFR1_U_OFFS	(PXA2X0_GAFR1_U-PXA2X0_GPIO_BASE)
#define GAFR2_L_OFFS	(PXA2X0_GAFR2_L-PXA2X0_GPIO_BASE)
#define GAFR2_U_OFFS	(PXA2X0_GAFR2_U-PXA2X0_GPIO_BASE)
#define PSSR_OFFS		(PXA2X0_PSSR-PXA2X0_PM_BASE)

.macro init_mpc_gpio
        ldr		r1, =PXA2X0_GPIO_BASE

        ldr		r0, =GPSR0_VAL			// set GPIO outputs to default
        str		r0, [r1, #GPSR0_OFFS]
		mvn		r0, r0
		str		r0, [r1, #GPCR0_OFFS]
		ldr		r0, =GPSR1_VAL			// set GPIO outputs to default
        str		r0, [r1, #GPSR1_OFFS]
		mvn		r0, r0
        str		r0, [r1, #GPCR1_OFFS]
        ldr		r0, =GPSR2_VAL			// set GPIO outputs to default
        str		r0, [r1, #GPSR2_OFFS]
		mvn		r0, r0
		str		r0, [r1, #GPCR2_OFFS]

		ldr		r0, =GPDR0_VAL			// GPIO direction
		str		r0, [r1, #GPDR0_OFFS]
		ldr		r0, =GPDR1_VAL			// GPIO direction
		str		r0, [r1, #GPDR1_OFFS]
		ldr		r0, =GPDR2_VAL			// GPIO direction
		str		r0, [r1, #GPDR2_OFFS]

        ldr		r0, =GAFR0_L_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR0_L_OFFS]
        ldr		r0, =GAFR0_U_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR0_U_OFFS]
        ldr		r0, =GAFR1_L_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR1_L_OFFS]
        ldr		r0, =GAFR1_U_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR1_U_OFFS]
        ldr		r0, =GAFR2_L_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR2_L_OFFS]
        ldr		r0, =GAFR2_U_VAL		// GPIO alternate function
        str		r0, [r1, #GAFR2_U_OFFS]

        ldr		r1, =PXA2X0_PM_BASE
		ldr		r0,	=0x20
        str		r0, [r1, #PSSR_OFFS]	// enable GPIO inputs 

.endm

/**********************************************************************************************************************
* LED 
**********************************************************************************************************************/
//   -7-
// |     |
// 1     6
// |     |
//   -2-
// |     |
// 3     5
// |     |
//   -4-     0

#define CYGHWR_LED_MACRO				\
	b		2f							;\
1:										;\
	.byte 0xfb, 0x61, 0xdd, 0xf5		;\
	.byte 0x67, 0xb7, 0xbf, 0xe1		;\
	.byte 0xff, 0xf7, 0xef, 0x3f		;\
	.byte 0x1d, 0x7d, 0x9f, 0x8f		;\
2:										;\
	ldr		r1, =PXA2X0_GPIO_BASE		;\
	mov		r0, #0x00ff0000				;\
	str		r0, [r1, #GPSR1_OFFS]		;\
	sub		r0, pc, #((3f+4)-1b)		;\
3:										;\
	ldrb	r0,	[r0, #\x]				;\
	mov		r0, r0, lsl #16				;\
	str		r0, [r1, #GPCR1_OFFS]		;

/**********************************************************************************************************************
* initialize controller 
**********************************************************************************************************************/

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#else
#define PLATFORM_SETUP1
#endif

.macro _platform_setup1
	.rept 0x20/4
	nop
	.endr
	b		1f

.globl mpc50_static_info	// Space for some static information
mpc50_static_info:
	.byte 'M','P','C','5'	// Magic
	.rept 16				
	.long 0
	.endr
1:
	init_mmu_off			// MMU on (and Cache)
	init_mpc_gpio			// GPIOs 
	LED(12)
	init_sdram_cnt			// SDRAM 
	LED(11)
	init_intc_cnt			// Interrupt Controller 
	LED(10)
	init_clks				// Clocks 
	LED(9)
	init_mmu_cache_on		// MMU and Cache 
	LED(8)
.endm

#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */

