##=============================================================================
##
##      ts1000.S
##
##      TS1000 board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002 Gary Thomas
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
## Author(s):   hmt
## Contributors:hmt, gthomas
## Date:        1999-06-08
## Purpose:     TS1000 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on an Allied Telesyn TS1000 (PPC855T) board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/arch.inc>		/* register symbols et al */
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */
					/* registers, IMM layout...         */
#include <cyg/hal/quicc/ppc8xx.h>       /* more of the same */
	
#------------------------------------------------------------------------------
# this is controlled with one define for tidiness:
# (and it is undefined by default)

//#define CYGPRI_RAM_START_PROGRAMS_UPMS

#if defined(CYG_HAL_STARTUP_ROM) \
 || defined(CYG_HAL_STARTUP_ROMRAM) \
 || defined(CYGPRI_RAM_START_PROGRAMS_UPMS)
# define CYGPRI_DO_PROGRAM_UPMS
#endif

/* The intention is that we only set up the UPMs in ROM start, be it actual
 * ROM application start or Stub ROMs that we built from the same sources.
 * 
 * The alternative approach - in which we have reliability doubts - is to
 * program the UPMs with *old* timing data in StubROM start, then
 * *reprogram* them with *new* timing data in RAM start - and of course
 * program with *new* timing data in plain ROM application start.
 * (Re-programming from new to new timing data fails - hence the suspicion
 * of reprogramming _at_all_, hence this private configuration)
 * 
 * With CYGPRI_RAM_START_PROGRAMS_UPMS left undefined, the former behaviour
 * - programming the UPMs exactly once - is obtained.  Define it to get the
 * latter, untrusted behaviour.
 */
	
#------------------------------------------------------------------------------

//
// Macros to build BR/OR registers
//
#define _BR(_reg,_BA,_PS,_MS,_V) \
        .long   CYGARC_REG_IMM_BASE+_reg, ((_BA&0xFFFF8000)|(_PS<<10)|(_MS<<6)|_V)
// Port size
#define _PS_32 0x00  // 32 bits
#define _PS_8  0x01  // 8 bits
#define _PS_16 0x02  // 16 bits
// Machine select
#define _MS_GPCM 0x00
#define _MS_UPMA 0x02
#define _MS_UPMB 0x03                                

#define _OR_GPCM(_reg,_AM,_CSNT,_ACS,_BIH,_SCY,_SETA,_TRLX,_EHTR) \
        .long   CYGARC_REG_IMM_BASE+_reg, ((_AM&0xFFFF8000)|(_CSNT<<11)|(_ACS<<9)|(_BIH<<8)|(_SCY<<4)|(_SETA<<3)|(_TRLX<<2)|(_EHTR<<1))

// GPCM - Chip select negation time
#define _CSNT_0 0
#define _CSNT_1 1
                                
// GPCM - Address setup time
#define _ACS_0  0x00  // !CS asserted with address lines
#define _ACS_4  0x02  // !CS asserted 1/4 clock after address lines
#define _ACS_2  0x03  // !CS asserted 1/2 clock after address lines

// Burst Inhibit
#define _BIH_0 0 // Bursting supported
#define _BIH_1 1 // Bursting disabled
        
// GPCM - Address setup times
#define _SCY_0  0x0 // No additional wait states
#define _SCY_1  0x1 // 1 additional wait states
#define _SCY_2  0x2 // 2 additional wait states
#define _SCY_3  0x3 // 3 additional wait states
#define _SCY_4  0x4 // 4 additional wait states
#define _SCY_5  0x5 // 5 additional wait states
#define _SCY_6  0x6 // 6 additional wait states
#define _SCY_7  0x7 // 7 additional wait states
#define _SCY_8  0x8 // 8 additional wait states
#define _SCY_9  0x9 // 9 additional wait states
#define _SCY_10 0xA // 10 additional wait states
#define _SCY_11 0xB // 11 additional wait states
#define _SCY_12 0xC // 12 additional wait states
#define _SCY_13 0xD // 13 additional wait states
#define _SCY_14 0xE // 14 additional wait states
#define _SCY_15 0xF // 15 additional wait states
	
// GPCM - external transfer acknowledge
#define _SETA_0 0   // No external acknowledge
#define _SETA_1 1   // External acknowledge

// GPCM - relaxed timing
#define _TRLX_0 0   // Strict timing
#define _TRLX_1 1   // Relaxed timing (wait states doubled)
        
// GPCM - external hold time
#define _EHTR_0 0   // Strict timing
#define _EHTR_1 1   // One wait state needed when switching banks        
        
#define _OR_UPM(_reg,_AM,_SAM,_G5LA,_G5LS,_BIH)\
        .long   CYGARC_REG_IMM_BASE+_reg,((_AM&0xFFFF8000)|(_SAM<<11)|(_G5LA<<10)|(_G5LS<<9)|(_BIH<<8))

#define _SAM_0 0 // Address lines are not multiplexed
#define _SAM_1 1 // Address lines are multiplexed by controller
        
#define _G5LA_0 0 // Use GPLB5 for GPL5
#define _G5LA_1 1 // Use GPLA5 for GPL5
        
#define _G5LS_0 0 // !GPL5 asserted on low edge
#define _G5LS_1 1 // !GPL5 asserted on high edge
	
#------------------------------------------------------------------------------

//        
// PTA field is (System Clock in MHz * Refresh rate in us) / Prescale
// e.g.  ((14*3.6864)*62.5)/32 => 100.8 => 101        
//
// Since the processor is clocked using the EXTCLK signal, the PLL
// should always run 1-1
//        
#define PLPRCR_PTX 0x000
#define MAMR_PTA 98

//
// Special MPC8xx cache control
//
#define CACHE_UNLOCKALL		0x0a00
#define CACHE_DISABLE		0x0400
#define CACHE_INVALIDATEALL	0x0c00
#define CACHE_ENABLE		0x0200
#define CACHE_ENABLEBIT		0x8000

#define CACHE_FORCEWRITETHROUGH 0x0100
#define CACHE_NOWRITETHROUGH    0x0300
#define CACHE_CLEAR_LE_SWAP     0x0700
				
	
#------------------------------------------------------------------------------

// LED macro uses r23, r25: r4 assumed to point to IMMR
#define LED( x )                            \
        lhz     r25,PADAT(r4)           ;   \
        andi.   r25,r25,(~0x3C&0xFFFF)  ;   \
        ori     r25,r25,(x<<2)          ;   \
        sth     r25,PADAT(r4)           ;   \
	
#------------------------------------------------------------------------------
                
FUNC_START( hal_hardware_init )

        mflr    r30             // Save original return address
        
	# Throughout this routine, r4 is the base address of the control
	# registers.  r3 and r5 are scratch in general.
	
	lwi     r4,CYGARC_REG_IMM_BASE  # base address of control registers
	mtspr	CYGARC_REG_IMMR,r4

        //
        // Set up GPIO port A - used to drive LEDs.
        //
        lhz     r3,PAODR(r4)    // paodr &= ~0x803C
        andi.   r3,r3,(~0x803C&0xFFFF)
        sth     r3,PAODR(r4)
        lhz     r3,PADIR(r4)    // padir |= 0x803C -- all outputs
        ori     r3,r3,0x803C
        sth     r3,PADIR(r4)
        lhz     r3,PAPAR(r4)    // papar &= ~0x803C
        andi.   r3,r3,(~0x803C&0xFFFF)
        sth     r3,PAPAR(r4)
#ifdef CYG_HAL_STARTUP_RAM
        lhz     r3,PADAT(r4)    // Turn off all LEDs, preserve PHY state
        ori     r3,r3,0x003C
#else        
	lwi	r3,0x803C       // Turn off all LEDS, reset PHY
#endif        
	sth	r3,PADAT(r4)
        	
	LED( 0 )			# turn all LEDs off

	# DATA CACHE
	mfspr	r3,CYGARC_REG_DC_CST		/* clear error bits */
        lis     r3,CACHE_UNLOCKALL
	sync
        mtspr   CYGARC_REG_DC_CST,r3		/* unlock all lines */
	
        lis     r3,CACHE_INVALIDATEALL
	sync
        mtspr   CYGARC_REG_DC_CST,r3		/* invalidate all lines */

	lis	r3,CACHE_DISABLE
	sync
	mtspr	CYGARC_REG_DC_CST,r3            /* disable */
	
	lis	r3,CACHE_FORCEWRITETHROUGH
	sync
	mtspr	CYGARC_REG_DC_CST,r3            /* set force-writethrough mode */

	lis	r3,CACHE_CLEAR_LE_SWAP
	sync
	mtspr	CYGARC_REG_DC_CST,r3            /* clear little-endian swap mode */
	# INSTRUCTION CACHE (no writeback modes)
	mfspr	r3,CYGARC_REG_IC_CST		/* clear error bits */
        lis     r3,CACHE_UNLOCKALL
        mtspr   CYGARC_REG_IC_CST,r3		/* unlock all lines */
	isync
        lis     r3,CACHE_INVALIDATEALL
        mtspr   CYGARC_REG_IC_CST,r3		/* invalidate all lines */
        isync
	lis	r3,CACHE_DISABLE
	mtspr	CYGARC_REG_IC_CST,r3            /* disable */
	isync
	
	sync
	
	LED( 0x01 )
	
#ifdef CYG_HAL_STARTUP_ROMRAM
// Need to set the PC into the FLASH (ROM) before the address map changes
	lwi	r3,10f
        lwi     r5,0xFE000000
	or	r3,r3,r5
	mtctr	r3
	bctr
10:	
#endif	
	
	/*
	 * SIU Initialization.
	 */
	lwi	r3,0x00610400
        stw	r3,SIUMCR(r4)

	/*
	 * Enable bus monitor. Disable Watchdog timer.
	 */
	lwi	r3,0xffffff88
	stw	r3,SYPCR(r4)

	/*
	 * Clear REFA & REFB. Enable but freeze timebase.
	 */
	lwi	r3,0x0000            // FIXME:   should this be 0x0000 or 0x00C2
	sth	r3,TBSCR(r4)

	/*
	 * Unlock some RTC registers (see section 5.11.2)
	 */
	lwi	r3,0x55ccaa33
	stw	r3,RTCSCK(r4)
	stw	r3,RTCK(r4)
	stw	r3,RTSECK(r4)
	stw	r3,RTCALK(r4)

	/*
	 * Clear SERC & ALR. RTC runs on freeze. Enable RTC.
	 */
	li	r3,0x0000            // FIXME:   should this be 0x0000 or 0x00C3
	sth	r3,RTCSC(r4)

	/*
	 * Clear periodic timer interrupt status.
	 * Enable periodic timer and stop it on freeze.
	 */
        li	r3,0x0001            // FIXME:   should this be 0x0001 or 0x0083
        sth	r3,PISCR(r4)

	LED( 0x02 )
	
	/*
	 * Perform UPM programming by writing to its 64 RAM locations.
	 * Note that UPM initialization must be done before the Bank Register
	 * initialization. Otherwise, system may hang when writing to Bank
	 * Registers in certain cases.
	 */
#ifdef CYGPRI_DO_PROGRAM_UPMS
	lwi	r5,__upmtbl_start
	lwi	r6,__upmtbl_end
	sub	r7,r6,r5      /* size of table */ 
	srawi	r7,r7,2       /* in words */
        
        lwi	r6,0x00800000     /* Command - OP=Write, UPMB, MAD=0 */
        or      r7,r7,r6
    1:
        lwz	r3,0(r5)      /* get data from table */
        stw	r3,MDR(r4)    /* store the data to MD register */
        stw	r6,MCR(r4)    /* issue command to MCR register */
        addi	r5,r5,4       /* next entry in the table */
        addi	r6,r6,1       /* next MAD address */
        cmpw	r6,r7         /* done yet ? */
        blt	1b
#endif // CYGPRI_DO_PROGRAM_UPMS
	
	LED( 0x03 )

	/*
	 * Set refresh timer prescaler to divide by 8.
	 */
        li	r3,PTP_DIV32
        sth	r3,MPTPR(r4)

        /*
	 * See Table 15-16 MPC860 User's Manual.
	 *
// Set the value of Machine A Mode Register (MAMR) to $5E802114.
// 	Field PTA (bits 0-7) = 94
// 	Field PTAE (bit 8) = 1
// 	Field AMA (bits 9-11) = 0
// 	Field Reserved (bit 12) = 0
// 	Field DSA (bits 13-14) = 0
// 	Field Reserved (bit 15) = 0
// 	Field G0CLA (bits 16-18) = 1
// 	Field GPL_A4DIS (bit 19) = 0
// 	Field RLFA (bits 20-23) = 1
// 	Field WLFA (bits 24-27) = 1
// 	Field TLFA (bits 28-31) = 4
	 */
        lwi     r3,0x00802114|(MAMR_PTA<<24)
        stw	r3,MAMR(r4)
        stw	r3,MBMR(r4)

	/*
	 * Base Register initialization.
	 */

//
// Memory map (device addressing) layout
//
        bl      10f
mc_regs:
// CS0 - FLASH - 0xFE000000..0xFE7FFFFF, 9 wait states, no bursting
        _OR_GPCM(OR0, 0xFF800000, _CSNT_1, _ACS_2, \
	         _BIH_1, _SCY_9, _SETA_0, _TRLX_1, _EHTR_1)
        _BR(BR0, 0xFE000000, _PS_16, _MS_GPCM, 1)

// CS1 - DRAM - 0x00000000..0x00FFFFFF,
        _BR(BR1, 0x00000000, _PS_32, _MS_UPMB, 1)
        _OR_UPM(OR1, 0xFF000000, _SAM_1, _G5LA_0, _G5LS_0, _BIH_0)
        
// CS2 - FPGA Loading - 0x80020000..0x80027FFF, 7 wait states, no bursting
        _BR(BR2, 0x80020000, _PS_32, _MS_GPCM, 1)
        _OR_GPCM(OR2, 0xFFFF8000, _CSNT_1, _ACS_2, \
	         _BIH_1, _SCY_7, _SETA_0, _TRLX_1, _EHTR_1)
        
// CS3 - FPGA - 0x80030000..0x80037FFF, 7 wait states, no bursting
        _OR_GPCM(OR3, 0xFFFF8000, _CSNT_1, _ACS_2, \
	         _BIH_1, _SCY_7, _SETA_0, _TRLX_1, _EHTR_1)	
        _BR(BR3, 0x80030000, _PS_32, _MS_GPCM, 1)
        
// CS4 - DS3 - 0x80040000..0x80047FFF, 7 wait states, no bursting
        _OR_GPCM(OR4, 0xFFFF8000, _CSNT_1, _ACS_2, \
	         _BIH_1, _SCY_7, _SETA_0, _TRLX_1, _EHTR_1)
        _BR(BR4, 0x80040000, _PS_32, _MS_GPCM, 1)
        
// CS5 - DS1 - 0x80050000..0x80057FFF, 7 wait states, no bursting
        _OR_GPCM(OR5, 0xFFFF8000, _CSNT_1, _ACS_2, \
	         _BIH_1, _SCY_7, _SETA_0, _TRLX_1, _EHTR_1)
        _BR(BR5, 0x80050000, _PS_32, _MS_GPCM, 1)
	
        .long   0       // End of table               

//
// Program memory controller registers (using table above)
//
10:     mflr    r3              // Points to table
        subi    r3,r3,4
20:     lwzu    r5,4(r3)        // Next address
        cmpi    0,r5,0
        beq     30f             // done?
        lwzu    r6,4(r3)        // value
	lwzu	r7,4(r3)	// second part of address/value pair
	lwzu	r8,4(r3)
        stw     r6,0(r5)	// store pair in order
	stw	r8,0(r7)
        b       20b
30:
	        
	/*
	 *  SYSTEM CLOCK CONTROL REGISTER
// Set the value of System Clock and Reset Control Register (SCCR) to $00400000.
// 	Field Reserved (bit 0) = 0
// 	Field COM (bits 1-2) = 0
// 	Field Reserved (bits 3-5) = 0
// 	Field TBS (bit 6) = 0
// 	Field RTDIV (bit 7) = 0
// 	Field RTSEL (bit 8) = 0
// 	Field CRQEN (bit 9) = 1
// 	Field PRQEN (bit 10) = 0
// 	Field Reserved (bits 11-12) = 0
// 	Field EBDF (bits 13-14) = 0
// 	Field Reserved (bits 15-16) = 0
// 	Field DFSYNC (bits 17-18) = 0
// 	Field DFBRG (bits 19-20) = 0
// 	Field DFNL (bits 21-23) = 0
// 	Field DFNH (bits 24-26) = 0
// 	Field Reserved (bits 27-31) = 0
	 */
	lwi	r3,0x00400000
	stw	r3,SCCR(r4)
        
	LED( 0x04 )

	/*
	 *  PLL, LOW POWER, AND RESET CONTROL REGISTER
// Set the value of PLL, Low Power and Reset Control Register (PLPRCR) to $00C04000.
// 	Field MF (bits 0-11) = 12
// 	Field Reserved (bits 12-15) = 0
// 	Field SPLSS (bit 16) = 0
// 	Field TEXPS (bit 17) = 1
// 	Field Reserved (bit 18) = 0
// 	Field TMIST (bit 19) = 0
// 	Field Reserved (bit 20) = 0
// 	Field CSRC (bit 21) = 0
// 	Field LPM (bits 22-23) = 0
// 	Field CSR (bit 24) = 0
// 	Field LOLRE (bit 25) = 0
// 	Field FIOPD (bit 26) = 0
// 	Field Reserved (bits 27-31) = 0
	 */
	lwi	r3,0x04000|(PLPRCR_PTX<<20)
	stw	r3,PLPRCR(r4)

	lwi	r3,0x40000
	mtctr	r3
10:	nop
	bdnz	10b

	/* SDRAM Initialization Sequence, UPMB, CS1 */
	li	r3,0
	stw	r3,MAR(r4)

	lwi 	r3,0x80802115;	/* run precharge from loc 21 (0x15) */
  	stw	r3,MCR(r4)

	lwi	r3,0x80802830;	/* run refresh 8 times */
   	stw	r3,MCR(r4)
	
	lwi	r3,0x22<<2;	// Mode register setting
   	stw	r3,MAR(r4)

       	lwi	r3,0x80802116;	/* run MRS pattern from loc 22 (0x16) */
   	stw	r3,MCR(r4)

	# mask interrupt sources in the SIU
	lis	r2,0
	lwi	r3,CYGARC_REG_IMM_SIMASK
	stw	r2,0(r3)

	# set the decrementer to maxint
	lwi	r2,0
	not	r2,r2
	mtdec	r2
	
	# and enable the timebase and decrementer to make sure
	li	r2,1				# TBEnable and not TBFreeze
	lwi	r3,CYGARC_REG_IMM_TBSCR
	sth	r2,0(r3)

	LED( 0x05 ) 

#ifdef CYG_HAL_STARTUP_ROM
	# move return address to where the ROM is
	mflr	r3
        lwi     r4,0x00FFFFFF        // CAUTION!! Assumes only low 16M for ROM
        and     r3,r3,r4
	oris	r3,r3,CYGMEM_REGION_rom>>16
	mtlr	r3
#endif

#ifdef CYG_HAL_STARTUP_ROMRAM
        // Copy image from ROM to RAM
        LED(0x06)
        lwi     r4,0xFE000000
        lwi     r5,0x01FFFFFF   // ROM/FLASH base
        and     r3,r30,r5       // segment relative
        lwi     r30,_hal_hardware_init_done
        sub     r6,r3,r30       // Absolute address
        add     r6,r6,r4        // FLASH address
        lwi     r7,0            // where to copy to
        lwi     r8,__ram_data_end
10:     lwz     r5,0(r6)
        stw     r5,0(r7)
        addi    r6,r6,4
        addi    r7,r7,4
        cmplw   r7,r8
        bne     10b
#endif                

        LED(0x0F)
	
        mtlr    r30             // Restore original link address
	blr
FUNC_END( hal_hardware_init )


#ifdef CYGPRI_DO_PROGRAM_UPMS
# -------------------------------------------------------------------------
# this table initializes the User Programmable Machine (UPM) nastiness
# in the QUICC to control DRAM timing.

__upmtbl_start:

// UPM 0x00: single read
       .long   0x0f0dfc04, 0x0ffffc04, 0x00bf7c04, 0x0ff5fc00
       .long   0x1ffffc05, 0xfffffc04, 0xfffffc04, 0xfffffc04
// UPM 0x08: burst read
       .long   0x0f0dfc04, 0x0ffffc04, 0x00bf7c04, 0x00fffc00
       .long   0x00fffc00, 0x00fffc00, 0x0ff5fc00, 0x1ffffc05
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
       .long   0xfffffc04
// UPM 0x15: initial precharge cycles
       .long   0x1ff5fc35
// UPM 0x16: program mode register
       .long   0xefcabc34, 0x1f357c35 // 0x1fb57c35 or 0xfffffc04
// UPM 0x18: single write
       .long   0x0f0dfc04, 0x0ffffc00, 0x00b77c04, 0x0ffffc04
       .long   0x0ff5fc04, 0x1ffffc05, 0xfffffc04, 0xfffffc04
// UPM 0x20: burst write
       .long   0x0f0dfc04, 0x0ffffc00, 0x00b77c00, 0x00fffc00
       .long   0x00fffc00, 0x00fffc04, 0x0ffffc04, 0x0ff5fc04
       .long   0x1ffffc05, 0xfffffc04, 0xfffffc04, 0xfffffc04
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
// UPM 0x30: refresh
       .long   0x0ff5fc00, 0x0ffffc00, 0x0ffd7c80, 0x0ffffc00
       .long   0x0ffffc00, 0x0ffffc80, 0x3ffffc07, 0xfffffc04
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
// UPM 0x3C: exception
       .long   0xfffffc27, 0xfffffc04, 0xfffffc04, 0xfffffc04

__upmtbl_end:
#endif // CYGPRI_DO_PROGRAM_UPMS
	
FUNC_START(hal_ts1000_set_led)
	lwi     r4,CYGARC_REG_IMM_BASE  # base address of control registers
        lhz     r5,PADAT(r4)         
        andi.   r5,r5,(~0x3C&0xFFFF)
	andi.	r3,r3,0x0F
        slwi    r3,r3,2
        or      r5,r5,r3
        sth     r5,PADAT(r4)         
	lwi	r5,_hold_led
	stw	r3,0(r5)
	blr
FUNC_END(hal_ts1000_set_led)
	.data
_hold_led:	
	.long	0
	.text	
	
FUNC_START(hal_ts1000_get_led)
	lwi	r5,_hold_led
	lwz	r3,0(r5)
	blr
FUNC_END(hal_ts1000_get_led)


#------------------------------------------------------------------------------
# end of ts1000.S
