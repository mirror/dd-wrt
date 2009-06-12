##=============================================================================
##
##      mbx.S
##
##      MBX board hardware setup
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
## Author(s):   hmt
## Contributors:hmt
## Date:        1999-06-08
## Purpose:     MBX board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a MBX860 or 821 PowerPC board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_powerpc.h>
#include <pkgconf/hal_powerpc_mbx.h>
        
#include <cyg/hal/arch.inc>		/* register symbols et al */
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */
					/* registers, IMM layout...         */
#include <cyg/hal/quicc/ppc8xx.h>       /* more of the same */
	
#------------------------------------------------------------------------------
# this is kept thus for commonality with CygMon code

#if 40 == CYGHWR_HAL_POWERPC_BOARD_SPEED
# define __40MHZ 1
#elif 50 == CYGHWR_HAL_POWERPC_BOARD_SPEED
# define __50MHZ 1
#else
# error Bad Board speed defined: see CYGBLD_HAL_PLATFORM_H
#endif

#------------------------------------------------------------------------------
# this is controlled with one define for tidiness:
# (and it is undefined by default)

//#define CYGPRI_RAM_START_PROGRAMS_UPMS

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYGPRI_RAM_START_PROGRAMS_UPMS)
# define CYGPRI_DO_PROGRAM_UPMS
#endif

#if defined(CYGPRI_RAM_START_PROGRAMS_UPMS) && defined(CYGSEM_HAL_ROM_MONITOR)
/* use old tables in a StubROM build iff RAM start will reprogram them */
# define CYGPRI_USE_OLD_UPM_TABLES
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

// LED macro uses r3, r5: r4 left alone
#define LED( x ) \
	lwi	r5,0xfa100001;         \
	lwi	r3,((x) & 0xe);        \
	stb	r3,0(r5)

	
#------------------------------------------------------------------------------
                
FUNC_START( hal_hardware_init )

	# Throughout this routine, r4 is the base address of the control
	# registers.  r3 and r5 are scratch in general.
	
	lwi     r4,CYGARC_REG_IMM_BASE  # base address of control registers
	mtspr	CYGARC_REG_IMMR,r4

	LED( 0 )			# turn all LEDs on
	
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
	
	/*
	 * SIU Initialization.
	 */
/*        lwi	r3,0x0062A900*/
/*        lwi	r3,0x006A2900*/
	lwi	r3,0x00602400
        stw	r3,SIUMCR(r4)

	/*
	 * Enable bus monitor. Disable Watchdog timer.
	 */
	lwi	r3,0xffffff88
	stw	r3,SYPCR(r4)

	/*
	 * Clear REFA & REFB. Enable but freeze timebase.
	 */
	lwi	r3,0x00c2
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
	li	r3,0x00c3
	sth	r3,RTCSC(r4)

	/*
	 * Clear periodic timer interrupt status.
	 * Enable periodic timer and stop it on freeze.
	 */
        li	r3,0x0083
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
        
        li	r6,0x0000     /* Command - OP=Write, UPMA, MAD=0 */
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
	 * PTA = 0x13 (clock dependent)
	 * PTAE = enabled
	 * AMA = 0
	 * DSA = 2 cycle disable
	 * G0CLA = A12
	 * GPL_A4DIS = 0
	 * RLFA = 0
	 * WLFA = 0
	 * TLFA = 0
	 */
#ifdef __50MHZ
	lwi	r3,0x18801000
#else
	lwi	r3,0x13801000
#endif
        stw	r3,MAMR(r4)

	/*
	 * Base Register initialization.
	 */
	/* BOOT ROM */
        lwi	r3,0xfe000401	# 8-bit, GPCM
#ifdef __50MHZ
        lwi	r5,0xff800960 /* for 120nS devices, else 0xff800940 */
#else
        lwi	r5,0xff800930
#endif
#ifdef CYGHWR_HAL_POWERPC_MBX_BOOT_32BIT_FLASH
	// Then the 32-bit FLASH device is on CS0, the "ROM" is on CS7
        stw	r3,BR7(r4)
        stw	r5,OR7(r4)
#else	// Boot from 8-bit ROM - the default, on CS0
        stw	r3,BR0(r4)
        stw	r5,OR0(r4)
#endif
	/* ONBOARD DRAM */
        lwi	r3,0x00000081	# 32-bit, UPMA
        lwi	r5,0xffc00400
        stw	r3,BR1(r4)
        stw	r5,OR1(r4)

	/* DRAM DIMM BANK0 */
        lwi	r3,0x00000080	# 32-bit, UPMA, INVALID
        lwi	r5,0x00000400
        stw	r3,BR2(r4)
        stw	r5,OR2(r4)

	/* DRAM DIMM BANK1 */
        lwi	r3,0x00000080	# 32-bit, UPMA, INVALID
        lwi	r5,0x00000400
        stw	r3,BR3(r4)
        stw	r5,OR3(r4)

	/* NVRAM */
        lwi	r3,0xfa000401	# 8-bit, GPCM
#ifdef __50MHZ
        lwi	r5,0xffe00930
#else
        lwi	r5,0xffe00920
#endif
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
#ifdef __50MHZ
        lwi	r5,0xff800940
#else
        lwi	r5,0xff800930
#endif
#ifdef CYGHWR_HAL_POWERPC_MBX_BOOT_32BIT_FLASH
	// Then the 32-bit FLASH device is on CS0, the "ROM" is on CS7
        stw	r3,BR0(r4)
        stw	r5,OR0(r4)
#else	// Boot from 8-bit ROM - so the 32-bit FLASH is on CS7
        stw	r3,BR7(r4)
        stw	r5,OR7(r4)
#endif
	/*
	 *  SYSTEM CLOCK CONTROL REGISTER
	 *
	 *  COM   (1:2)   = 0
	 *  TBS   (6)     = 1
	 *  RTDIV (7)     = 0
	 *  RTSEL (8)     = 0
	 *  CRQEN (9)     = 0
	 *  PRQEN (10)    = 0
	 *  EBDF  (13:14) = 0
	 *  DFSYNC(17:18) = 0
	 *  DFBRG (19:20) = 0
	 *  DFNL  (21:23) = 0
	 *  DFNH  (24:26) = 0
	 */
	lwi	r3,0x02000000
	stw	r3,SCCR(r4)
	
	/*
	 *  The following sets up a 40MHz CPU clock.
	 *  I've seen 2 variations of MBX boards. One
	 *  uses a direct feed (1:1) 40MHz clock on
	 *  EXTCLK inputs. The other uses a 32KHz
	 *  oscillator on the OSCM inputs.
	 */
	lwz	r3,PLPRCR(r4)
	rlwinm	r3,r3,12,20,31
	cmpwi	r3,0
	beq     1f
	
	/*
	 *  PLL, LOW POWER, AND RESET CONTROL REGISTER
	 *
	 *  MF    (0:11)  = depends on source clock
	 *  SPLSS (16)    = 1
	 *  TEXPS (17)    = 1
	 *  TMIST (19)    = 1
	 *  CSRC  (21)    = 0
	 *  LPM   (22:23) = 0
	 *  CSR   (24)    = 0
	 *  LOLRE (25)    = 0
	 *  FIOPD (26)    = 0
	 */
	/*  MF    (0:11)  = 0x4c4 = 1220 = (40MHz/32.768KHz) */
#ifdef __50MHZ
	lwi	r3,0x5f50d000
#else
	lwi	r3,0x4c40d000
#endif
	b	2f
    1:
	/*  MF    (0:11)  = 0x000 = 1 = (1:1) */
	lwi	r3,0x0000d000
    2:
	stw	r3,PLPRCR(r4)

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

	LED( 8 ) # turn red led off

#ifdef CYG_HAL_STARTUP_ROM
	# move return address to where the ROM is
	mflr	r3
	andi.	r3,r3,0xffff
	oris	r3,r3,CYGMEM_REGION_rom>>16
	mtlr	r3
#endif

	blr
FUNC_END( hal_hardware_init )


#ifdef CYGPRI_DO_PROGRAM_UPMS
# -------------------------------------------------------------------------
# this table initializes the User Programmable Machine (UPM) nastiness
# in the QUICC to control DRAM timing.

__upmtbl_start:

#ifdef __25MHZ
	/* UPM contents for 25MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0xffffec07, 0xffffec07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xcfffe004, 0x0fffa404, 0x08ff2c00, 0x33ff6c0f
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xcfffe004, 0x0fffa404, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x03ff2c0c, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x33ff6c0f, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xc0ffec04, 0x07ffec04, 0x3fffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#ifdef __33MHZ
	/* UPM contents for 33MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0xffffec07, 0xffffec07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xcfffe004, 0x0fff2404, 0x08ff2c00, 0x33ff6c07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xcfffe004, 0x0fff2404, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x03ff2c0c, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x33ff6c07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xc0ffec04, 0x03ffec04, 0x1fffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#ifdef CYGPRI_USE_OLD_UPM_TABLES

	// BUT new tables received from motorola are further down

	// And I just discovered a good reason for using the NEW TABLES:
	// with the old tables, the cache zero-a-line command does not
	// work.  It only zeros the first 12 bytes of the line, not all 16.
	// This may be related to having the cache set up write-through, as
	// seems necessary to have it work on this platform.

#ifdef __40MHZ
	/* UPM contents for 40MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xefffe004, 0x0fffe004, 0x0eefac04, 0x00af2c04
        .long	0x03af2c08, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xefffe004, 0x0fffe004, 0x0eefac04, 0x00af2c04
        .long	0x03af2c08, 0x0caf2c04, 0x00af2c04, 0x03af2c08
        .long	0x0caf2c04, 0x00af2c04, 0x03af2c08, 0x0caf2c04
        .long	0x00af2c04, 0x03af2c08, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xefffe004, 0x0fffa004, 0x0eff2c04, 0x00ff2c00
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xefffe004, 0x0fffa004, 0x0eff2c04, 0x00ff2c00
        .long	0x0fff2c0c, 0x0cff2c00, 0x03ff2c0c, 0x0cff2c00
        .long	0x03ff2c0c, 0x0cff2c00, 0x33ff6c07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xf0ffec04, 0x00ffec04, 0x0fffec04, 0x0fffec04
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#ifdef __50MHZ
	/* UPM contents for 50MHZ clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
	.long	0xffffe004, 0x0fffe004, 0x0fffe404, 0x0cafac04
	.long	0x00af2c04, 0x0faf2c08, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
	.long	0xffffe004, 0x0fffe004, 0x0fffe404, 0x0cafac04
	.long	0x00af2c04, 0x0faf2c08, 0x0caf2c04, 0x00af2c04
	.long	0x0faf2c08, 0x0caf2c04, 0x00af2c04, 0x0faf2c08
	.long	0x0caf2c04, 0x00af2c04, 0x0faf2c08, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
	.long	0xffffe004, 0x0fffe004, 0x0fffa404, 0x0cff2c04
	.long	0x00ff2c00, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
	.long	0xffffe004, 0x0fffe004, 0x0fffa404, 0x0cff2c04
	.long	0x00ff2c00, 0x0fff2c08, 0x0cff2c04, 0x00ff2c00
	.long	0x0fff2c00, 0x0cff2c04, 0x00ff2c00, 0x0fff2c08
	.long	0x0cff2c04, 0x00ff2c00, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
	.long	0xf0ffec04, 0xc0ffec04, 0x00ffec04, 0x0fffec04
	.long	0x1fffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
		
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#else // !CYGPRI_USE_OLD_UPM_TABLES: use the NEW TABLES

	// for RAM startup or ROM application when NOT making a stub rom,
	// ie. CYGSEM_HAL_ROM_MONITOR not defined.

#ifdef __40MHZ
	/* UPM contents for 40MHz clk. DRAM: EDO,4K,60ns */
	.long	0xcfafc004, 0x0fafc404, 0x0caf0c04, 0x30af0c00
	.long	0xf1bf4805, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xcfafc004, 0x0fafc404, 0x0caf0c04, 0x03af0c08
	.long	0x0caf0c04, 0x03af0c08, 0x0caf0c04, 0x03af0c08
	.long	0x0caf0c04, 0x30af0c00, 0xf3bf4805, 0xffffc005
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xcfff0004, 0x0fff0404, 0x0cff0c00, 0x33ff4804
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xcfff0004, 0x0fff0404, 0x0cff0c00, 0x03ff0c0c
	.long	0x0cff0c00, 0x03ff0c0c, 0x0cff0c00, 0x03ff0c0c
	.long	0x0cff0c00, 0x33ff4804, 0xffffc005, 0xffffc005
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xfcffc004, 0xc0ffc004, 0x01ffc004, 0x0fffc004
	.long	0x3fffc004, 0xffffc005, 0xffffc005, 0xffffc005
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xffffc007, 0xffffc007, 0xffffc007, 0xffffc007
#endif
	
#ifdef __50MHZ
	/* UPM contents for 50MHZ clk. DRAM: EDO,4K,60ns */
	
	.long	0xcfafc004, 0x0fafc404, 0x0caf8c04, 0x10af0c04
	.long	0xf0af0c00, 0xf3bf4805, 0xffffc005, 0xffffc005

	.long	0xcfafc004, 0x0fafc404, 0x0caf8c04, 0x00af0c04
	.long	0x07af0c08, 0x0caf0c04, 0x01af0c04, 0x0faf0c08
	.long	0x0caf0c04, 0x01af0c04, 0x0faf0c08, 0x0caf0c04
	.long	0x10af0c04, 0xf0afc000, 0xf3bf4805, 0xffffc005

	.long	0xcfff0004, 0x0fff0404, 0x0cff0c00, 0x13ff4804
	.long	0xffffc004, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xcfff0004, 0x0fff0404, 0x0cff0c00, 0x03ff0c0c
	.long	0x0cff0c00, 0x03ff0c0c, 0x0cff0c00, 0x03ff0c0c
	.long	0x0cff0c00, 0x13ff4804, 0xffffc004, 0xffffc005
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xfcffc004, 0xc0ffc004, 0x01ffc004, 0x0fffc004
	.long	0x1fffc004, 0xffffc004, 0xffffc005, 0xffffc005
	.long	0xffffc005, 0xffffc005, 0xffffc005, 0xffffc005

	.long	0xffffc007, 0xffffc007, 0xffffc007, 0xffffc007
#endif
#endif	// OLD/NEW TABLES == CYGPRI_USE_OLD_UPM_TABLES or not.
	// depending on CYGSEM_HAL_ROM_MONITOR and whether RAM
	// start re-initializes.
	
__upmtbl_end:
#endif // CYGPRI_DO_PROGRAM_UPMS
	
FUNC_START(hal_mbx_set_led)
	andi.	r3,r3,0x0e
	lwi	r4,0xfa100001
	stb	r3,0(r4)
	blr
FUNC_END(hal_mbx_set_led)
	
FUNC_START(hal_mbx_flash_led)
	lwi	r4,0xfa100001
    1:	
	li	r5,10
	stb	r5,0(r4)
	
	lis	r5,10
	mtctr   r5
    2:	
	bdnz	2b

	li	r5,12
	stb	r5,0(r4)
	
	lis	r5,10
	mtctr   r5
    3:	
	bdnz	3b
	
	subi	r3,r3,1
	cmpwi	r3,0
	bge	1b

	li	r5,6
	stb	r5,0(r4)

	lis	r5,20
	mtctr   r5
    4:	
	bdnz	4b

	blr
FUNC_END(hal_mbx_flash_led)


#------------------------------------------------------------------------------
# end of mbx.S
