##=============================================================================
##
##      adder.S
##
##      ADDER board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002, 2003 Gary Thomas
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
## Purpose:     ADDER board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a ADDER PPC860 board.
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
                
FUNC_START( hal_hardware_init )

	# Throughout this routine, r4 is the base address of the control
	# registers.  r3 and r5 are scratch in general.
	
	lwi     r4,CYGARC_REG_IMM_BASE  # base address of control registers
	mtspr	CYGARC_REG_IMMR,r4

#ifndef CYG_HAL_STARTUP_RAM
                	
#define CACHE_UNLOCKALL		0x0a00
#define CACHE_DISABLE		0x0400
#define CACHE_INVALIDATEALL	0x0c00
#define CACHE_ENABLE		0x0200
#define CACHE_ENABLEBIT		0x8000

#define CACHE_FORCEWRITETHROUGH 0x0100
#define CACHE_NOWRITETHROUGH    0x0300
#define CACHE_CLEAR_LE_SWAP     0x0700

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
	/* (dunno what this is, but it sounds like a bad thing) */
	
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

#endif // ! CYG_HAL_STARTUP_RAM        
	
	/*
	 * SIU Initialization.
	 */
	lwi	r3,0x00610400
        stw	r3,SIUMCR(r4)
	
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

#ifdef CYGPRI_DO_PROGRAM_UPMS
	/*
	 * Perform UPM programming by writing to its 64 RAM locations.
	 * Note that UPM initialization must be done before the Bank Register
	 * initialization. Otherwise, system may hang when writing to Bank
	 * Registers in certain cases.
	 */
	lis	r5,__upmtbl_start@h
	ori	r5,r5,__upmtbl_start@l
	lis	r6,__upmtbl_end@h
	ori	r6,r6,__upmtbl_end@l
	sub	r7,r6,r5      /* size of table */ 
	srawi	r7,r7,2       /* in words */
        
        li	r6,0x00000000     /* Command - OP=Write, UPMA, MAD=0 */
    1:
        lwz	r3,0(r5)      /* get data from table */
        stw	r3,MDR(r4)    /* store the data to MD register */
        stw	r6,MCR(r4)    /* issue command to MCR register */
        addi	r5,r5,4       /* next entry in the table */
        addi	r6,r6,1       /* next MAD address */
        cmpw	r6,r7         /* done yet ? */
        blt	1b
#endif // CYGPRI_DO_PROGRAM_UPMS
        
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

//        
// PTA field is (System Clock in MHz * Refresh rate in us) / Prescale
// e.g.  ((14*3.6864)*62.5)/32 => 100.8 => 101        
//                
#if defined(CYGHWR_HAL_POWERPC_MPC8XX_852T)
#if (CYGHWR_HAL_POWERPC_BOARD_SPEED == 48)
#define PLPRCR_VALUE 0x32594000 // MFI=9 MFN=6 MFD=9 PDF=0 S=1 OSCLK=10MEG
#define MAMR_PTA 93 //sysclk * 62.5e-6 /32
#endif
#if (CYGHWR_HAL_POWERPC_BOARD_SPEED == 50)
#define PLPRCR_VALUE 0xD05A4000 // MFI=10 MFN=0 MFD=1 PDF=0 S=1 OSCLK=10MEG
#define MAMR_PTA 97 //sysclk * 62.5e-6 /32
#endif
#else        
#if (CYGHWR_HAL_POWERPC_BOARD_SPEED == 47)
#define PLPRCR_VALUE 0x00C04000 // MFI=9 MFN=6 MFD=9 PDF=0 S=1 OSCLK=10MEG
#define MAMR_PTA 94        
#endif        
#endif

//#define MAMR_PTA (((((((PLPRCR_PTX+1)*3686400)*625)/10000000)+31)/32)&0xFF)
        lwi     r3,0x00802114|(MAMR_PTA<<24)
        stw	r3,MAMR(r4)
        stw	r3,MBMR(r4)

	/*
	 * Base Register initialization.
	 */

       	/* BOOT ROM */
        lwi	r3,0xFE000801	# 16-bit, GPCM
        lwi	r5,0xFF800774   # 7 wait states, up to 8MB
        stw	r3,BR0(r4)
        stw	r5,OR0(r4)

	/* Misc I/O, 16 bit port */
        lwi	r3,0xFA100801
        lwi	r5,0xFFFF8730
        stw	r3,BR2(r4)
        stw	r5,OR2(r4)

	/* ONBOARD DRAM */
        lwi	r3,0x00000081	# 32-bit, UPMA
        lwi	r5,0xFF800E00
        stw	r3,BR1(r4)
        stw	r5,OR1(r4)

	/* DRAM DIMM BANK1 */
        lwi	r3,0x00000080	# 32-bit, UPMA, INVALID
        lwi	r5,0xFFFF87FC
        stw	r3,BR3(r4)
        stw	r5,OR3(r4)

#if 0        
	/* NVRAM */
        lwi	r3,0xfa000401	# 8-bit, GPCM
        lwi	r5,0xffe00930
        stw	r3,BR4(r4)
        stw	r5,OR4(r4)

	/* PCI BRIDGE MEM/IO */
        lwi	r3,0x80000001	# 32-bit, GPCM
        lwi	r5,0xa0000108
        stw	r3,BR5(r4)
        stw	r5,OR5(r4)

	/* PCI BRIDGE REGISTERS */
        lwi	r3,0xfa210001	# 32-bit, GPCM
        lwi	r5,0xffff0108
        stw	r3,BR6(r4)
        stw	r5,OR6(r4)

	/* FLASH */
        lwi	r3,0xfc000001	# 32-bit, GPCM
        lwi	r5,0xff800940
        stw	r3,BR7(r4)
        stw	r5,OR7(r4)
#endif
        
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
//	lwi	r3,0x04000|(PLPRCR_PTX<<20)
	lwi	r3,PLPRCR_VALUE
	stw	r3,PLPRCR(r4)

	lwi	r3,0x40000
	mtctr	r3
10:	nop
	bdnz	10b

	/* SDRAM Initialization Sequence, UPMA, CS1 */
	li	r3,0
	stw	r3,MAR(r4)

	lwi 	r3,0x80002115;	/* run precharge from loc 21 (0x15) */
  	stw	r3,MCR(r4)

	lwi	r3,0x80002830;	/* run refresh 8 times */
   	stw	r3,MCR(r4)
	
	lwi	r3,0x88;	/* MR 88 for high range */
   	stw	r3,MAR(r4)

       	lwi	r3,0x80002116;	/* run MRS pattern from loc 22 (0x16) */
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
        mflr    r3              
        lwi     r4,0xFE000000
        lwi     r5,0x01FFFFFF   // ROM/FLASH base
        and     r3,r3,r5        // segment relative
        lwi     r6,_hal_hardware_init_done
        mtlr    r6
        sub     r6,r3,r6        // Absolute address
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
	blr
FUNC_END( hal_hardware_init )


#ifdef CYGPRI_DO_PROGRAM_UPMS
# -------------------------------------------------------------------------
# this table initializes the User Programmable Machine (UPM) nastiness
# in the QUICC to control DRAM timing.

__upmtbl_start:
// single read   (offset 0x00 in upm ram)
       .long   0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xe0bbbc00
       .long   0x10f77c44, 0xf3fffc07, 0xfffffc04, 0xfffffc04
// burst read    (offset 0x08 in upm ram)
       .long   0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xf0affc00
       .long   0xf0affc00, 0xf0affc00, 0xf0affc00, 0x10a77c44
       .long   0xf7bffc47, 0xfffffc35, 0xfffffc34, 0xfffffc35
       .long   0xfffffc35, 0x1ff77c35, 0xfffffc34, 0x1fb57c35
// single write  (offset 0x18 in upm ram)
       .long   0x1f27fc24, 0xe0aebc04, 0x00b93c00, 0x13f77c47
       .long   0xfffdfc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
// burst write   (offset 0x20 in upm ram)
       .long   0x1f07fc24, 0xeeaebc00, 0x10ad7c00, 0xf0affc00
       .long   0xf0affc00, 0xe0abbc00, 0x1fb77c47, 0xfffffc04
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
// refresh       (offset 0x30 in upm ram)
       .long   0x1ff5fca4, 0xfffffc04, 0xfffffc04, 0xfffffc04
       .long   0xfffffc84, 0xfffffc07, 0xfffffc04, 0xfffffc04
       .long   0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04
// exception     (offset 0x3C in upm ram)
       .long   0xfffffc27, 0xfffffc04, 0xfffffc04, 0xfffffc04
__upmtbl_end:
#endif // CYGPRI_DO_PROGRAM_UPMS

#------------------------------------------------------------------------------
# end of adder.S
