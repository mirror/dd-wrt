#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	AR7100 assembler header file
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
## Purpose:	AR7100 SOC definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AR7100 SOC
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
#include <cyg/hal/plf_cache.h>
#include <cyg/hal/plf_io.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
/* #include <cyg/hal/mips-regs.h> */
#include <cyg/hal/var_arch.h>

#include "ar7100_soc.h"
#include "ar7100_sio.h"

/*
 * Helper macros.
 * These Clobber t7, t8 and t9
 */
#define clear_mask(_reg, _mask)                     \
    li  t7, _reg|K1BASE;                            \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define set_val(_reg, _mask, _val)                  \
    li  t7, _reg|K1BASE;                            \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    li  t9, _val;                                   \
    or  t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define set_val_reg(_reg, _mask, _valreg)           \
    li  t7, _reg;                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    or  t8, t8, _valreg;                            \
    sw  t8, 0(t7)


#define get_val(_reg, _mask, _shift, _res_reg)      \
    li  t7, _reg|K1BASE;                                   \
    lw  t8, 0(t7);                                  \
    li  t9, _mask;                                  \
    and t8, t8, t9;                                 \
    srl _res_reg, t8, _shift                        \

#define pll_clr(_mask)                              \
    clear_mask(AR7100_CPU_PLL_CONFIG, _mask)

#define pll_set(_mask, _val)                        \
    set_val(AR7100_CPU_PLL_CONFIG,  _mask, _val)

#define pll_set_reg(_mask, _reg)                    \
    set_val_reg(AR7100_CPU_PLL_CONFIG,  _mask, _reg)

#define pll_get(_mask, _shift, _res_reg)            \
    get_val(AR7100_CPU_PLL_CONFIG, _mask, _shift, _res_reg)

#define clk_clr(_mask)                               \
    clear_mask(AR7100_CPU_CLOCK_CONTROL, _mask)

#define clk_set(_mask, _val)                         \
    set_val(AR7100_CPU_CLOCK_CONTROL,  _mask, _val)

#define clk_get(_mask, _shift, _res_reg)              \
    get_val(AR7100_CPU_CLOCK_CONTROL, _mask, _shift, _res_reg)


#------------------------------------------------------------------------------
# Macro for copying vectors to RAM if necessary.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)

	.macro	hal_vectors_init
	# If we don~t play nice with a ROM monitor, copy the required
	# vectors into the proper location.
	la	t0,0x80000000		# dest addr
	la	t1,utlb_vector		# source addr
	la	t3,utlb_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

	la	t0,0x80000180		# dest addr
	la	t1,other_vector		# source addr
	la	t3,other_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

	.set mips3                      # Set ISA to MIPS 3 to allow cache insns
	# Now clear the region in the caches
	la	t0,0x80000000		# dest addr
	ori	t1,t0,0x200		# source addr
1:	cache	0x01,0x0000(t0)	        # Flush word from data cache
	cache	0x01,0x1000(t0)
	cache	0x01,0x2000(t0)
	cache	0x01,0x3000(t0)
	nop
	cache	0x00,0x0000(t0)		# Invalidate icache for word
	cache	0x00,0x1000(t0)
	cache	0x00,0x2000(t0)
	cache	0x00,0x3000(t0)
	nop
	addi	t0,t0,0x20
	bne	t0,t1,1b
	 nop
	.set mips0                      # reset ISA to default

	.endm

#else

	.macro	hal_vectors_init
	.endm

#endif

#----------------------------------------------------------------------
# Serial port initialization

	.macro hal_serial_init
#if 0
	li      a0, CYGARC_KSEG_UNCACHED|AR7100_GPIO_OE
	li      t0, 0xcff
	sw      t0, 0(a0)
	lw      zero, 0(a0)             # flush

	li      a0, CYGARC_KSEG_UNCACHED|AR7100_GPIO_OUT
	li      t0, 0x3b
	sw      t0, 0(a0)
	lw      zero, 0(a0)             # flush

	li      a0, 0xb8040028
	li      t0, 0x100
	sw      t0, 0(a0)
	lw      zero, 0(a0)             # flush


    /* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

	UART16550_WRITE(OFS_INTR_ENABLE, 0);

/* put UART CLOCK RATE into a0 */
	li      a0, 100000000            # TBD: derive sys frequency
	nop
	div     a0, DESIRED_BAUD_RATE
	div     a0, 16
	srl     t2, a0, 8
	andi    t1, a0, 0xff            
	andi    t2, 0xff                

2:
	/* Init UART @ 9600 baud */
	li      a0, CYGARC_KSEG_UNCACHED|AR7100_UART_BASE
	li      t0, LCR_DLAB            # access divisor latch
	sb      t0, LCR<<2(a0)
	sb      t1, DLL<<2(a0)          # divisor low
	sb      t2, DLM<<2(a0)          # divisor high
	li      t0, CHAR_LEN_8          # 8b mode
	sb      t0, LCR<<2(a0)
	sb      zero, IER<<2(a0)        # disable interrupts
	li      t0, FCR_EN|FCR_RXCLR|FCR_TXCLR
	sb      t0, FCR<<2(a0)          # reset and enable fifos
#endif
	.endm

#define PLL_CONFIG_SW_UPDATE_VAL (1 << 31)
#define CLOCK_CTRL_SWITCH_VAL    (1 << 1)

/* We need an easy way to read pll setting from flash. for now    */
/* this is read from a fixed offset in last sector of flash.      */

#if (CYGNUM_FLASH_BLOCK_NUM == 64)
#define CFG_FLASH_PLL_SETTING_ADDR  0x1f3f0000  /* 4MB version */
#else
#define CFG_FLASH_PLL_SETTING_ADDR  0x1f7f0000  /* 8MB version */
#endif

	.macro	hal_get_speed_setting_from_flash_into_t1

	/* Disable SPI remap */
	li      a0, CYGARC_UNCACHED_ADDRESS(AR7100_SPI_CLOCK)
	li      t0, 0x43
	sw      t0, 0(a0)

	 /* Now read pll setting fm flash */
    	li  	t0, CYGARC_UNCACHED_ADDRESS(CFG_FLASH_PLL_SETTING_ADDR)
    	lw  	t1, 0(t0)

	/* Enable SPI remap */
	li      a0, CYGARC_UNCACHED_ADDRESS(AR7100_SPI_CLOCK)
	li      t0, 0x3
	sw      t0, 0(a0)

	/* Vet range */
	/* First, check for the "Secret" values */

	li	t0, AR7100_PLL_720_360_180
	beq t1, t0, 2f
	nop
	li	t0, AR7100_PLL_800_400_200
	beq	t1, t0, 2f
	nop

	/*
	** Else, check the range of values
	*/
	 
	li	t0, AR7100_PLL_200_200_100
	blt	t1, t0,	1f
	nop
	li	t0, AR7100_PLL_680_340_170
	ble	t1, t0, 2f
	nop
1:
	/* Value is bad, use default process based on CDL */	

#if (CYGNUM_PLL_FREQ == AR7100_PLL_800_400_200)
	li	t1, AR7100_PLL_800_400_200
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_720_360_180)
	li	t1, AR7100_PLL_720_360_180
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_680_340_170)
	li	t1, AR7100_PLL_680_340_170
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_600_300_150)
	li	t1, AR7100_PLL_600_300_150
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_400_400_200)
	li	t1, AR7100_PLL_400_400_200
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_333_333_166)
	li	t1, AR7100_PLL_333_333_166
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_300_300_150)
	li	t1, AR7100_PLL_300_300_150
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_266_266_133)
	li	t1, AR7100_PLL_266_266_133
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_266_266_66)
	li	t1, AR7100_PLL_266_266_133
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_200_200_100)
	li	t1, AR7100_PLL_200_200_100
#elif (CYGNUM_PLL_FREQ == AR7100_PLL_USE_REV_ID)

	li	t0, K1BASE | AR7100_REV_ID
	lw	t1, 0(t0)
	andi	t1, AR7100_REV_ID_MASK

	li	t0, AR7100_REV_ID_AR7130
	bne	t0, t1, 1f
	nop
	li	t1, AR7100_PLL_300_300_150
	b	2f

	nop
1:	li	t0, AR7100_REV_ID_AR7141
	bne	t0, t1, 1f
	li	t1, AR7100_PLL_300_300_150
	b	2f
	nop	

1:	li	t0, AR7100_REV_ID_AR7161
	bne	t0, t1, 1f
	li	t1, AR7100_PLL_680_340_170
	b	2f
	nop

1:	li	t1, AR7100_PLL_300_300_150

#else
#error: must specify PLL behavior in CDL
#endif

2:
	.endm

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
	# If we are using a ROM version, include code to
	# initialize PLLs and SDRAM.

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE

	.macro	hal_reset_vector_first_code

	lar     t0, rel_start
	li	t1, 0xff3fffff
	and	t0, t0, t1
	j       t0
	nop

rel_start:

	hal_get_speed_setting_from_flash_into_t1

    	li  	t2, AR7100_PLL_800_400_200
    	beq 	t1, t2, pll_800_400_200
    	nop
    	li  	t2, AR7100_PLL_720_360_180
    	beq 	t1, t2, pll_720_360_180
    	nop
    	li  	t2, AR7100_PLL_680_340_170
    	beq 	t1, t2, pll_680_340_170
    	nop
    	li  	t2, AR7100_PLL_600_300_150
    	beq 	t1, t2, pll_600_300_150
    	nop
    	li  	t2, AR7100_PLL_400_400_200
    	beq 	t1, t2, pll_400_400_200
    	nop
    	li  	t2, AR7100_PLL_333_333_166
    	beq 	t1, t2, pll_333_333_166
    	nop
    	li  	t2, AR7100_PLL_300_300_150
    	beq 	t1, t2, pll_300_300_150
    	nop
    	li  	t2, AR7100_PLL_266_266_133
    	beq 	t1, t2, pll_266_266_133
    	nop
    	li  	t2, AR7100_PLL_266_266_66
    	beq 	t1, t2,	pll_266_266_66
    	nop
    	li  	t2, AR7100_PLL_200_200_100
    	b	pll_200_200_100
    	nop
	
pll_800_400_200:
    	li 	t0, (0x0  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x1  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_720_360_180:
    	li 	t0, (0x0  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x11 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x1  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_680_340_170:
    	li 	t0, (0x0  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x10 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x1  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_600_300_150:
    	li 	t0, (0x1  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x3  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x1d << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_400_400_200:
    	li 	t0, (0x1  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_333_333_166:
    	li 	t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x18 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_300_300_150:
    	li 	t0, (0x3  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x3  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x1d << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_266_266_133:
    	li 	t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_266_266_66:
    	li 	t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

pll_200_200_100:
    	li 	t0, (0x3  << PLL_CONFIG_CPU_DIV_SHIFT)
    	li 	t1, (0x3  << PLL_CONFIG_DDR_DIV_SHIFT)
    	li 	t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    	li 	t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    	li 	t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
		li	t5,	(0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    	b 	got_setting
    	nop

got_setting:
	clk_get(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CONTROL_RST_SWITCH_SHIFT, t6)
	bne zero, t6, initialize_pll
	nop 

	pll_set_reg(PLL_CONFIG_CPU_DIV_MASK, t0)
	pll_set_reg(PLL_CONFIG_AHB_DIV_MASK, t2)
	pll_set_reg(PLL_CONFIG_DDR_DIV_MASK, t1)
	pll_set_reg(PLL_CONFIG_PLL_DIVOUT_MASK, t5)

	pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)

	/*
	 * Will cause a reset
	 */
	clk_set(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CTRL_SWITCH_VAL)
	clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 1) 
	nop
    
initialize_pll:
	pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)
	clk_clr(CLOCK_CONTROL_RST_SWITCH_MASK)
	pll_set_reg(PLL_CONFIG_PLL_FB_MASK, t3)
	pll_set_reg(PLL_CONFIG_PLL_LOOP_BW_MASK, t4)

	pll_clr(PLL_CONFIG_PLL_POWER_DOWN_MASK);
	pll_clr(PLL_CONFIG_PLL_BYPASS_MASK);

wait_for_pll_lock:
	pll_get(PLL_CONFIG_LOCKED_MASK, PLL_CONFIG_LOCKED_SHIFT, t6)
	beq zero, t6, wait_for_pll_lock
	nop 

pll_locked:
	clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 1)

	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

# MEMDELAY is slow as it's running uncached - not perfect but good enough.
#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop

#if defined(CYG_HAL_STARTUP_ROMRAM)
#define CYGPKG_HAL_MIPS_LAR_DEFINED
        .macro  lar     reg,addr
        .set    noat
        move    $at,ra                  # save ra
        la      \reg,\addr              # get address into register
        la      ra,x\@                  # get linked address of label
        subu    \reg,\reg,ra            # subtract it from value
        bal     x\@                     # branch and link to label
        nop                             #  to get current actual address
x\@:
        addu    \reg,\reg,ra            # add actual address
        move    ra,$at                  # restore ra
        .set    at
        .endm
#endif

#define CAS_LATENCY_SHIFT	 	27
#define CAS_LATENCY_MASK 		 7

	# Initialize SDRAM configuration.

	.macro	hal_memc_init

	hal_get_speed_setting_from_flash_into_t1

	/* check if it is needed to set CAS to 3 */

	li      a0, CYGARC_UNCACHED_ADDRESS(AR7100_DDR_CONFIG)

	li	t2, AR7100_PLL_400_400_200
        beq	t1, t2, 1f
	nop

	li	t2, AR7100_PLL_600_300_150
        beq	t1, t2, 1f
	nop

	/* Use whatever is in CDL file, presumably CAS 2.5 */
	li	t0, CYGNUM_DDR_CONFIG_VAL
	sw	t0, 0(a0)
	b	2f
	nop

1:	/* set CAS to 3 - the value is 6 for this register */
	li	t0, CYGNUM_DDR_CONFIG_VAL & ~(CAS_LATENCY_MASK<<CAS_LATENCY_SHIFT)
	li	t2, 6<<CAS_LATENCY_SHIFT
	or	t0, t0, t2
	sw	t0, 0(a0)

2:	li      a1, K1BASE | AR7100_DDR_CONFIG2
	li	t8, CYGNUM_DDR_CONFIG2_VAL
	sw	t8, 0(a1)
        MEMDELAY(30, t2)

        li      a0, K1BASE | AR7100_DDR_CONTROL
        li      a1, K1BASE | AR7100_DDR_EXT_MODE

        li      t5, 8
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

        li      t5, 1
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

        li      t5, 0
        sw      t5, 0(a1)
        MEMDELAY(30, t2)

        li      t5, 2
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

        li      t5, 8
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

	# DDR_MODE
        li      a2, K1BASE | AR7100_DDR_MODE

        /* t1 still has the mode in it */

	li	t2, AR7100_PLL_400_400_200
        beq	t1, t2, 1f
	nop

	li	t2, AR7100_PLL_600_300_150
        beq	t1, t2, 1f
	nop

	li	t5, CYGNUM_DDR_MODE_VAL
	b	2f
	nop

1:	/* 0x30 is CAS 3 in this register */
	li	t2, CYGNUM_DDR_MODE_VAL & 0x0f
	li	t5, 0x30
	or	t5, t5, t2

2:	sw	t5, 0(a2)
	MEMDELAY(30, t2)

        li      t5, 1
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

        li      a0, K1BASE | AR7100_DDR_REFRESH
        li      t5, CYGNUM_DDR_REFRESH_VAL
        sw      t5, 0(a0)
        MEMDELAY(30, t2)

        li      a2, K1BASE | AR7100_DDR_RD_DATA_THIS_CYCLE
	li	t5, CYGNUM_DDR_RD_DATA_THIS_CYCLE_VAL
	sw	t5, 0(a2)
	
        li      a1, 0x07
        li	a0, K1BASE|AR7100_DDR_TAP_CONTROL0
        sw      a1, 0(a0)
        li	a0, K1BASE|AR7100_DDR_TAP_CONTROL1
        sw      a1, 0(a0)
        li	a0, K1BASE|AR7100_DDR_TAP_CONTROL2
        sw      a1, 0(a0)
        li	a0, K1BASE|AR7100_DDR_TAP_CONTROL3
        sw      a1, 0(a0)

#if defined(CYG_HAL_STARTUP_ROMRAM)
        # Having got the RAM working, we must now relocate the Entire
        # ROM into it and then continue execution from RAM.

        la      t0,reset_vector         # dest addr
        lar     t1,reset_vector         # source addr
        la      t3,__ram_data_end       # end dest addr
1:
        lw      v0,0(t1)                # get word
        sw      v0,0(t0)                # write word
        addiu   t1,t1,4
        addiu   t0,t0,4
        bne     t0,t3,1b
        nop

        la      v0,2f                   # RAM address to go to
        jr      v0
        nop
        nop
        nop
        nop
2:
        # We are now executing out of RAM!
#endif

	.endm

#endif /* CYG_HAL_STARTUP_ROM */

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
        defined(CYG_HAL_STARTUP_ROMRAM) ||              \
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
	la	a0,__default_interrupt_vsr
	la	a1,__default_exception_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	sw	a1,1*4(a3)
	sw	a1,2*4(a3)
	sw	a1,3*4(a3)
	sw	a1,4*4(a3)
	sw	a1,5*4(a3)
	sw	a1,6*4(a3)
	sw	a1,7*4(a3)
	sw	a1,8*4(a3)
	sw	a1,9*4(a3)
	sw	a1,10*4(a3)
	sw	a1,11*4(a3)
	nop
	nop
	sw	a1,12*4(a3)
	sw	a1,13*4(a3)
	sw	a1,14*4(a3)
	sw	a1,15*4(a3)

	sw	a1,32*4(a3)
	sw	a1,33*4(a3)
	sw	a1,34*4(a3)
	.endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	la	a0,__default_interrupt_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
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
