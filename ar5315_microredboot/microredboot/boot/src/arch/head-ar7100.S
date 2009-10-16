/*
 * head-ar7100.S
 * originally written for xscale based linux kernel decompression
 * now adapted for AR531X based redboot stub and kernel loader
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 * based on Vxworks sources from Atheros Communications, Inc.
 *
 */
#include "ramconfig.h"

#if defined(AR7100)

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/irqflags.h>
#include <asm/regdef.h>
#include <asm/page.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

#include "mips4Kx.h"
#include "ar7100_soc.h"
#include "ar7100_sio.h"

#define CYGARC_KSEG_MASK                               (0xE0000000)
#define CYGARC_KSEG_CACHED                             (0x80000000)
#define CYGARC_KSEG_UNCACHED                           (0xA0000000)
#define CYGARC_CACHED_ADDRESS(x)                       ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_CACHED)
#define CYGARC_UNCACHED_ADDRESS(x)                     ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_UNCACHED)


#define CYGNUM_DDR_CONFIG_VAL 0xefbc8cd0
#define CYGNUM_DDR_CONFIG2_VAL 0x8e7156a2
#define CYGNUM_DDR_MODE_VAL 0x61
#define CYGNUM_DDR_REFRESH_VAL 0x461b
#define CYGNUM_DDR_RD_DATA_THIS_CYCLE_VAL 0xffff

#define DESIRED_BAUD_RATE           115200

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


#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop


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



/*
 * WARNING:
 * This code must be Position Independent Code (PIC) as it runs in
 * the prom address space (0xbfc00000) even though it is linked
 * in memory.  Only call routines in romLib.s and romInit.s with
 * bal.
 */
#ifdef CONFIG_BOOTLOADER

	.section	".start"

#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f; li k0,bev


	/*
	 * Only for Flash based booting - bootloader mode goes straight
	 * to the decompressor, relying on the bootloader to have
	 * initialised the memory controller etc
	 * 
	 * Also use this for JTAG mode.
	 */

	.set	noreorder

__AR531X_start:
	RVECENT(__AR531X_init,0)	/* PROM entry point */
	RVECENT(__AR531X_reboot,1)	/* software reboot */
	RVECENT(romReserved,2)
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17)
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)
	XVECENT(romExcHandle,0x200)	/* tlbmiss: for 4Kc */
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)
	XVECENT(romExcHandle,0x280)	/* xtlbmiss: for 4Kc */
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)
	XVECENT(romExcHandle,0x300)	/* cache error: should not happen */
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	/* bfc00380: R4000 general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)
	XVECENT(romExcHandle,0x400)	/* 0xbfc00400: 4Kx fast interrupt */
	RVECENT(romReserved,129)
	RVECENT(romReserved,130)
	RVECENT(romReserved,131)
	RVECENT(romReserved,132)
	RVECENT(romReserved,133)
	RVECENT(romReserved,134)
	RVECENT(romReserved,135)
	RVECENT(romReserved,136)
	RVECENT(romReserved,137)
	RVECENT(romReserved,138)
	RVECENT(romReserved,139)
	RVECENT(romReserved,140)
	RVECENT(romReserved,141)
	RVECENT(romReserved,142)
	RVECENT(romReserved,143)
	XVECENT(romExcHandle,0x480)	/* 0xbfc00480: 4Kx debug interrupt */
	RVECENT(romReserved,145)
	RVECENT(romReserved,146)
	RVECENT(romReserved,147)
	RVECENT(romReserved,148)
	RVECENT(romReserved,149)
	RVECENT(romReserved,150)
	RVECENT(romReserved,151)
	RVECENT(romReserved,152)
	RVECENT(romReserved,153)
	RVECENT(romReserved,154)
	RVECENT(romReserved,155)
	RVECENT(romReserved,156)
	RVECENT(romReserved,157)
	RVECENT(romReserved,158)
	RVECENT(romReserved,159)

	.set reorder

	/* Want this fairly early in output file */
	.asciz "MicroRedBoot Copyright 2009 NewMedia-NET GmbH / DD-WRT.COM"
	.align 4


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


2:
	.endm


/*
 * romExcHandle - rom hook to forward exceptions to sysConsoleDump()
 * to dump the system state to the console and panic.  Not for general
 * use.
 */
romExcHandle:
	b	__AR531X_init

/*
 * romReserved - Handle a jump to an unknown vector.
 * void romReserved(void);
 */
romReserved:
	b	__AR531X_init		# just start over


__AR531X_init:
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


#define CAS_LATENCY_SHIFT	 	27
#define CAS_LATENCY_MASK 		 7
		
__AR531X_reboot:
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

# serial init
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



	nop
	
        la      t0,_text               # dest addr
#       la      t1,_text          # source addr
#	lar	t1,__AR531X_start
        la      t1,_text-0x80400000+BOOTBASE          # source addr
        la      t3,_text+0x10000       # end dest addr
3:      
        lw      v0,0(t1)                # get word
        sw      v0,0(t0)                # write word
        addiu   t1,t1,4
        addiu   t0,t0,4
        bne     t0,t3,3b
        nop
        la      v0,4f                   # RAM address to go to
        jr      v0
        nop
4:       
	nop
	j	main
	nop




#endif	/* ! CONFIG_BOOTLOADER */
#endif
