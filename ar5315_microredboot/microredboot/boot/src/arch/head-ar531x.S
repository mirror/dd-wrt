/*
 *  head-ar531x.S
 *
 *  Copyright (C) 2009 NewMedia-NET GmbH (derived from linux boot code)
 *  Copyright (C) 2003 Instant802 Networks, Inc.
 *  Copyright (C) 2001 Atheros Communications, Inc.,  All Rights Reserved.
 *
 *  originally written for xscale based linux kernel decompression
 *  now adapted for AR531X based redboot stub and kernel loader
 *  copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 *  licensed under GPL conditions
 *  based on Vxworks sources from Atheros Communications, Inc.
 *
 *
 *  License: GPL
 */
#include "ramconfig.h"

#ifdef AR5312
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
#include "ar531xreg.h"
#include "ar531xSio.h"

/*
 * WARNING:
 * This code must be Position Independent Code (PIC) as it runs in
 * the prom address space (0xbfc00000) even though it is linked
 * in memory.  Only call routines in romLib.s and romInit.s with
 * bal.
 */
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



/*
 * romTlbInit - set all TLB entries to invalid on the 4Kc.  This
 * is not strictly needed on cold reset as the tlb shutdown condition
 * is avoided, but it does not discuss warm reset.  Better to be safe.
 */
.macro romTlbInit
	MFC0_T0_CONFIG1
	and	t0, CFG1_MMUSIZE
	srl	t0, CFG1_MMUSIZE_S
	beqz	t0, 2f			# no TLB entries is the 4Kp
	mtc0	zero, CP0_ENTRYLO0	# set-up invalid TLB entry
	mtc0	zero, CP0_ENTRYLO1
	li	t1, 0x2000		# first page
	mtc0	zero, CP0_PAGEMASK	# 4KB pages
	.set	push
	.set	noreorder
1:
	mtc0	t1, CP0_ENTRYHI		# each entry needs a unique vaddr
	nop
	mtc0	t0, CP0_INDEX		# set TLB index for tlbwr
	nop
	tlbwi
	addi	t0, -1			# decr loop counter
	addi	t1, 0x2000		# next page * 2
	bgez	t0, 1b			# loop
	nop				# BDSLOT
	.set	pop
2:
.endm

/*
 * romCacheInit - Low level init of the (primary) caches via index
 *                invalidate cache ops.  We do not have to worry
 *                about parity like the R4000.
 */

.macro romCacheInit
	.set 	mips3
	/* Inval data cache tags: t1=ways, t2=linesize, t3=depth */
	MFC0_T0_CONFIG1
	and	t1, t0, CFG1_DA
	srl	t1, CFG1_DA_S
	add	t1, 1
	and	t2, t0, CFG1_DL
	srl	t2, CFG1_DL_S
	addi	t2, 1
	and	t3, t0, CFG1_DS
	srl	t3, CFG1_DS_S
	li	t0, 64
	sll	t3, t0, t3
	li	t0, 1
	sll	t2, t0, t2
	beqz	t3, 2f			# any d$?
	mul	a1, t1, t3		# number of lines (ways * depth)
	li	a0, KSEG0
1:
	cache	1, 0(a0)		# invalidate data line
	add	a0, t2			# p += linesize
	add	a1, -1			# number of lines - 1
	bgtz	a1, 1b			# any left?
2:

	/* Inval instruction cache tags: t1=ways, t2=linesize, t3=depth */
	MFC0_T0_CONFIG1
	and	t1, t0, CFG1_IA
	srl	t1, CFG1_IA_S
	add	t1, 1
	and	t2, t0, CFG1_IL
	srl	t2, CFG1_IL_S
	and	t3, t0, CFG1_IS
	srl	t3, CFG1_IS_S
	addi	t2, 1
	li	t0, 64
	sll	t3, t0, t3
	li	t0, 1
	sll	t2, t0, t2
	beqz	t3, 2f			# any i$?
	mul	a1, t1, t3		# number of lines
	li	a0, KSEG0
1:
	cache	0, 0(a0)		# invalidate instr line
	add	a0, t2			# p += linesize
	add	a1, -1			# number of lines - 1
	bgtz	a1, 1b			# any left?
2:
.endm


/*
 * romInitFlash - init flash address space in ar531x flash controller
 */
.macro romInitFlash

	/* Configure base flash part initially to the max size */
	li	a0, KSEG1 | AR531X_FLASHCTL0
	lw	t0, 0(a0)
	and	t0, FLASHCTL_MW			/* keep bus width from HW */
	/* 0x7 == 12.5ns * 8 == 100ns which is typical for flash roms */
	or	t0, FLASHCTL_E | FLASHCTL_AC_8M | FLASHCTL_RBLE | \
		    (0x01 << FLASHCTL_IDCY_S) | \
		    (0x07 << FLASHCTL_WST1_S) | \
		    (0x07 << FLASHCTL_WST2_S)
	sw	t0, 0(a0)

	/* Disable other banks */
	li	a0, KSEG1 | AR531X_FLASHCTL1
	lw	t0, 0(a0)
	and	t0, ~(FLASHCTL_E | FLASHCTL_AC)
	sw	t0, 0(a0)
	li	a0, KSEG1 | AR531X_FLASHCTL2
	lw	t0, 0(a0)
	and	t0, ~(FLASHCTL_E | FLASHCTL_AC)
	sw	t0, 0(a0)
.endm

/*
 * romSizeMemory - initialize SDRAM configuration.
 *   - The 256Mb mode is not supported.
 *   - Only 4 bank devices are supported.
 *
 * MEMDELAY is slow as it's running uncached - not perfect but good enough.
 */

#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b

.macro romSizeMemory
	/* If we're executing from flash, try to set up SDRAM */
	li	t5, 0       # 0-->x32, MEM_CFG0_X-->x16
	li	t6, 0x22000 # 0x22000-->4 word burst, 0x23000-->8 word burst
	li	a0, KSEG1|AR531X_REV
	lw	a0, 0(a0)
	andi	a0,0xff
	bne	a0,0x50,99f
	nop
	
98:
	/* Configure for a x16 SDRAM with 8-word burst */
	li	t5, MEM_CFG0_X           # try x16
	li	t6, 0x23000              # use 8-word burst
99:
	li	a0, KSEG1 | AR531X_MEM_CFG0
	li	a1, KSEG1 | AR531X_MEM_CFG1
	li	a2, KSEG1 | AR531X_MEM_REF

	/* 1) wait 100us - handled by latency to boot up to this point. */

	/* 2) Set the I and M bits to issue an SDRAM nop */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_M | MEM_CFG1_I
	sw	t0, 0(a1)		# MEM_CFG1

	/* 3) Wait 200us (roughly) */
	MEMDELAY(50, t2)

	/* 4) Reset the M bit to issue an SDRAM PRE-ALL */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_I
	sw	t0, 0(a1)		# MEM_CFG1
	sync

	/* 5) Generate a refresh every 16 clocks (spec says 10) */
	li	t0, 16			# very fast refresh for now
	sw	t0, 0(a2)		# MEM_REF

	/* 6) Wait for 128 clocks (8 refresh cycles) */
	MEMDELAY(5, t2)

	/*
	 * 7) Set operational value into the reset timer.
	 *    [out of order from ARM SDRAM ctrl spec]
	 */

	/* 8) Set command write mode, and read each SDRAM */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S) | \
		    MEM_CFG1_M
	sw	t0, 0(a1)		# MEM_CFG1
	sync
	li	t0, KSEG1 | AR531X_SDRAM0 
	or	t0, t6 # burst
	lw	zero, 0(t0)
	li	t0, KSEG1 | AR531X_SDRAM1
	or	t0, t6 # burst
	lw	zero, 0(t0)

	/* 9) Program configuration register 0 */
	li	t0, MEM_CFG0_C | MEM_CFG0_C2 | MEM_CFG0_R1 | \
		    MEM_CFG0_B0 | MEM_CFG0_B1
	or	t0, t5 # x16 or x32
	sw	t0, 0(a0)		# MEM_CFG0
	sync

	li	t0, AR531X_SDRAM_MEMORY_REFRESH_VALUE
	sw	t0, 0(a2)		# MEM_REF
	sync

	/* 10) Clear I and M and set cfg1 to the normal operational value */
	li	t0, MEM_CFG1_E0 | (MEM_CFG1_AC_128 << MEM_CFG1_AC0_S) | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S)
	sw	t0, 0(a1)		# MEM_CFG1
	sync

        /* 11) Determine whether the SDRAM is x16 or x32 */
	li	a3, KSEG1|AR531X_SDRAM0 # Pick a memory address
	li	t0, 0x12345678           # Pick a data pattern to write
	lw	t1, 4(a3)                # Read the 2nd word
	sw	t0, 0(a3)                # Write the 1st word
	lw	t0, 4(a3)                # See of the second word changed
	beq	t0, t1, 1f               # If so, must be a x16 part
	nop

	b	98b

1:
	/* 12) The SDRAM is ready for normal operation */

	/* Size bank 0 - assumed to be present */
	li	a3, KSEG1|AR531X_SDRAM0
	sw	zero, 0(a3)		# base value
	li	t1, MEM_CFG1_AC_2	# min 2MB
	li	t3, 0x200000		# 2MB
1:
	or	a2, a3, t3		# construct probe address
	sw	t1, 0(a2)		# write it
	lw	t2, 0(a3)		# read back @ base
	bnez	t2, 2f			# alias detected?
	nop
	sll	t3, 1			# next address
	add	t1, 1			# next alias
	ble	t1, MEM_CFG1_AC_64, 1b	# 128MB not supported!
	nop

/* reverse engineered part from Alfa AP48 bootloader binary */
2:
	lw	t5, 0(a0)
	lui 	t2, 0x80
	bge 	t2, t3, 3f
	ori     t0, t5, MEM_CFG0_F0
	sw	t0, 0(a0)
	sync
	lw	t2, 0(a2)
	beq	t1, t2, 1f
	sll	t3, 1
	add	t1, 1
	ori	t5, MEM_CFG0_F0

3:	
	sw	t5, 0(a0)
	
	sll	t0, t1, MEM_CFG1_AC0_S	# align for bank0
	or	t0, MEM_CFG1_E0 | \
		    MEM_CFG1_E1 | (MEM_CFG1_AC_128 << MEM_CFG1_AC1_S)
	sw	t0, 0(a1)		# MEM_CFG1
	sync

	and	t0, ~(MEM_CFG1_E1 | MEM_CFG1_AC1)	# clear bank 1 enable

	/* Probe bank 1 as it is optional */
	li	a3, KSEG1|AR531X_SDRAM1
	li	t1, 0x5555aaaa		# 2 probe patterns
	li	t2, 0x3333cccc
	sw	t1, 0(a3)
	sw	t2, 4(a3)
	lw	t3, 0(a3)
	lw	t4, 4(a3)
	bne	t1, t3, nobank1
	nop
	bne	t2, t4, nobank1
	nop

	/* Size bank 1 */
	li	a3, KSEG1|AR531X_SDRAM1
	sw	zero, 0(a3)		# base value
	li	t1, MEM_CFG1_AC_2	# min 2MB
	li	t3, 0x200000		# 2MB
1:
	or	a2, a3, t3		# construct probe address
	sw	t1, 0(a2)		# write it
	lw	t2, 0(a3)		# read back @ base
	bnez	t2, 2f			# alias detectd?
	nop
	sll	t3, 1			# next address
	add	t1, 1			# next alias
	ble	t1, MEM_CFG1_AC_64, 1b	# 128MB not supported!
	nop
2:	sll	t1, MEM_CFG1_AC1_S	# align for bank1
	or	t0, t1			# combine with current value
	or	t0, MEM_CFG1_E1		# enable bank1

nobank1:
	sw	t0, 0(a1)		# MEM_CFG1
	sync

.endm

__AR531X_init:

	mfc0	k0,$12
	lui	k1,0x0008 /* NMI */
	and	k1,k1,k0
	bnez	k1,pllskip
	nop

	/* Clear any prior AHB errors by reading both addr registers */
	li	t0, KSEG1 | AR531X_PROCADDR
	lw	zero, 0(t0)
	li	t0, KSEG1 | AR531X_DMAADDR
	lw	zero, 0(t0)

        /*
         * Must not access any MAC registers for at least 16MS after reset.
         * This delay is put here rather than after PLL initialization
         * because eventually we'll probably have to determine the input
         * clock frequency (in order to set CLOCKCTL1) by reading MAC
         * information.
         *
         * We're running uncached from flash at this point.  Every
         * A flash read takes 64 cycles * 100ns (at 40MHz/4).
         */
        li      t0, 4000 # conservative value -- 25.6 ms.
1:      sub     t0, 1
        bnez    t0, 1b
	nop

	li	t0, KSEG1 | AR5312_CLOCKCTL2
	lw	t1, (t0)
	bgez	t1, pllskip              # upper bit guaranteed non-0 at reset
	nop

	li	a0, KSEG1 | AR531X_REV
	lw	a0, 0(a0)
	andi	a0, 0xff
	beq	a0, 0x50, viperpll
	nop

pllenable:
	li	t0, KSEG1 | AR5312_CLOCKCTL1
	li	t1, AR5312_CLOCKCTL1_SELECTION
	sw	t1, (t0)
	nop

	li	t0, KSEG1 | AR5312_SCRATCH
	li	t1, AR5312_CPU_CLOCK_RATE
	sw	t1, (t0)

        /*
         * Set /2 clocking and turn OFF AR5312_CLOCKCTL2_STATUS_PLL_BYPASS.
         * Processor RESETs at this point; the CLOCKCTL registers retain
         * their new values across the reset.
         */
	li	t0, KSEG1 | AR5312_CLOCKCTL2
        li      t1, AR5312_CLOCKCTL2_WANT_DIV2 | AR5312_CLOCKCTL2_WANT_RESET
	sw	t1, (t0)		       # causes a processor reset
	nop
1:      b       1b /* NOTREACHED */

viperpll:
	# For Viper 0xbc003064 register has to be programmed with 0x91000 to
	# get 180Mhz Processor clock
	# Set /2 clocking and turn OFF AR531X_CLOCKCTL2_STATUS_PLL_BYPASS.
	# Processor RESETs at this point; the CLOCKCTL registers retain
	# their new values across the reset.

	li      t0, KSEG1 | AR5312_CLOCKCTL1
	li      t1, AR2313_CLOCKCTL1_SELECTION
	sw      t1,(t0)

	li      t0, KSEG1 | AR5312_CLOCKCTL2
	li      t1, AR5312_CLOCKCTL2_WANT_RESET
	sw      t1, (t0)                       # causes a processor reset
1:	b       1b /* NOTREACHED */

pllskip:
__AR531X_reboot:

	.set	nomove
	/* Disable all interrupts, select prom exception handlers */
	li	t1, CAUSE_IV
	mtc0	t1, CP0_CAUSE
	nop
	li	t0, ST0_CU0 | ST0_BEV
	mtc0	t0, CP0_STATUS

	/* Clear software interrupts or lurking watchpoints */
	mtc0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI

	/* Clear any prior AHB errors by reading both addr registers */
	li	t0, KSEG1|AR531X_PROCADDR
	li	t1, KSEG1|AR531X_DMAADDR
	lw	zero, 0(t0)
	lw	zero, 0(t1)
	.set	move

	/* Give us as long as possible before a clock interrupt */
	li	t0, 1
	mtc0	t0, CP0_COUNT
	mtc0	zero, CP0_COMPARE




	/* Set watchpoint for low memory to debug null pointers */
	li	t0, 0x40000ff8		# G + Ignore 11:3 in check
	mtc0	t0, CP0_WATCHHI
	li	t0, 7			# watch rwx
	mtc0	t0, CP0_WATCHLO

	romInitFlash		# init flash configuration
	nop
	romSizeMemory		# init memory configuration
	nop
	romTlbInit		# for 4Kc
	nop
        /* Disable NMI callback after memory is ready */
        li      a0, 0xa0000410
        sw      zero, 0(a0)

	/*
	 * Must have cache enabled before cache init.  At this point
	 * we are running in bfc00000 so the text is uncached.
	 */
	mfc0	t0, CP0_CONFIG
	and	t1, t0, CFG_MT		# check for TLB or FM
	sub	t1, CFG_MT_TLB
	beqz	t1, 1f
	/* 4Kp init all fixed mapping modes -- leave U and 23 uncached */
	and	t0, ~(CFG_KU | CFG_K23)
	or	t0, (CONF_CM_UNCACHED << CFG_KU_S) | \
		    (CONF_CM_UNCACHED << CFG_K23_S)
1:
	and	t0, ~CFG_K0MASK
	li	t0, CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG
	nop
	nop
	romCacheInit		# init/invalidate cache tags

	nop
	li	a0, KSEG1|AR531X_RESET
	lw	t0, 0(a0)
	and	t0, ~RESET_APB
	or	t0, RESET_UART0
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

	and	t0, ~RESET_UART0
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

1:	/* Use internal clocking */
	li	a0, KSEG1|AR531X_CLOCKCTL
	lw	t0, 0(a0)
	and	t0, ~CLOCKCTL_UART0
	sw	t0, 0(a0)

	/* put UART CLOCK RATE into a0 */
	li	a0, KSEG1|AR5312_SCRATCH
	lw	a0, 0(a0)
	bnez	a0, 3f			# CPU Clock freq specified?
	li	a0, 180000000		# No: assume 180MHz
3:	srl	a0, 2			# derive UART freq
	div	a0, 115200
	div	a0, 16
	srl	t2, a0, 8
	andi	t1, a0, 0xff		# AR531X_NS16550_DLL_VALUE
	andi	t2, 0xff		# AR531X_NS16550_DLM_VALUE
	
2:

	/* Init UART @ 115200 baud */
	li	a0, KSEG1|AR531X_UART0
	li	t0, LCR_DLAB		# access divisor latch
	sw	t0, LCR<<2(a0)
	sw	t1, DLL<<2(a0)		# divisor low
	sw	t2, DLM<<2(a0)		# divisor high
	li	t0, CHAR_LEN_8		# 8b mode
	sw	t0, LCR<<2(a0)
	sw	zero, IER<<2(a0)	# disable interrupts
	li	t0, FCR_EN|FCR_RXCLR|FCR_TXCLR

	sw	t0, FCR<<2(a0)		# reset and enable fifos
	
	
	
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

	/* Want to switch to cached segment now... */

	j	main

#endif