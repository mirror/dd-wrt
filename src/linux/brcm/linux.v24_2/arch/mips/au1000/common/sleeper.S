/*
 * Copyright 2002 Embedded Edge, LLC
 * Author: dan@embeddededge.com
 *
 * Sleep helper for Au1xxx sleep mode.
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/config.h>
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/au1000.h>

/*
 * Note: This file is *not* conditional on CONFIG_PM since Alchemy sleep 
 * need not be tied to any particular power management scheme.
 */

	.extern ___flush_cache_all

	.text
	.align	5

/*
 * Save the processor general registers and go to sleep. A wakeup
 * condition will get us back here to restore the registers.
 */

/* still need to fix alignment issues here */
save_and_sleep_frmsz = 48
NESTED(save_and_sleep, save_and_sleep_frmsz, ra)
	.set noreorder
	.set nomacro
	.set noat
	subu sp, save_and_sleep_frmsz
	sw ra, save_and_sleep_frmsz-4(sp)
	sw s0, save_and_sleep_frmsz-8(sp)
	sw s1, save_and_sleep_frmsz-12(sp)
	sw s2, save_and_sleep_frmsz-16(sp)
	sw s3, save_and_sleep_frmsz-20(sp)
	sw s4, save_and_sleep_frmsz-24(sp)
	sw s5, save_and_sleep_frmsz-28(sp)
	sw s6, save_and_sleep_frmsz-32(sp)
	sw s7, save_and_sleep_frmsz-36(sp)
	sw s8, save_and_sleep_frmsz-40(sp)
	sw gp, save_and_sleep_frmsz-44(sp)

	/* We only need to save the registers that the calling function 
	 * hasn't saved for us.  0 is always zero.  8 - 15, 24 and 25 are 
	 * temporaries and can be used without saving. 26 and 27 are reserved 
	 * for interrupt/trap handling and expected to change.  29 is the 
	 * stack pointer which is handled as a special case here.
	 */
	subu	sp, PT_SIZE
	sw	$1, PT_R1(sp)
	sw	$2, PT_R2(sp)
	sw	$3, PT_R3(sp)
	sw	$4, PT_R4(sp)
	sw	$5, PT_R5(sp)
	sw	$6, PT_R6(sp)
	sw	$7, PT_R7(sp)
	sw	$16, PT_R16(sp)
	sw	$17, PT_R17(sp)
	sw	$18, PT_R18(sp)
	sw	$19, PT_R19(sp)
	sw	$20, PT_R20(sp)
	sw	$21, PT_R21(sp)
	sw	$22, PT_R22(sp)
	sw	$23, PT_R23(sp)
	sw	$28, PT_R28(sp)
	sw	$30, PT_R30(sp)
	sw	$31, PT_R31(sp)
#define PT_C0STATUS PT_LO
#define PT_CONTEXT PT_HI
#define PT_PAGEMASK PT_EPC
#define PT_CONFIG PT_BVADDR
	mfc0	k0, CP0_STATUS
	sw	k0, PT_C0STATUS(sp) // 0x20
	mfc0	k0, CP0_CONTEXT
	sw	k0, PT_CONTEXT(sp) // 0x1c
	mfc0	k0, CP0_PAGEMASK
	sw	k0, PT_PAGEMASK(sp) // 0x18
	mfc0	k0, CP0_CONFIG
	sw	k0, PT_CONFIG(sp) // 0x14

	.set macro
	.set at

	li t0, SYS_SLPPWR
	sw	zero, 0(t0)	/* Get the processor ready to sleep */
	sync

	/* Now set up the scratch registers so the boot rom will
	 * return to this point upon wakeup.
	 * sys_scratch0 : SP
	 * sys_scratch1 : RA
	 */
	li	t0, SYS_SCRATCH0
	li	t1, SYS_SCRATCH1
	sw	sp, 0(t0)
	la	k0, resume_from_sleep
	sw	k0, 0(t1)

/*
 * Flush DCACHE to make sure context is in memory
	 */
	la 	t1,___flush_cache_all   /* _flush_cache_all is a function pointer */
	lw 	t0,0(t1)
	jal t0
	nop

/* Put SDRAM into self refresh.  Preload instructions into cache,
 * issue a precharge, then auto refresh, then sleep commands to it.
 */
 	la	t0, sdsleep
	.set	mips3
 	cache	0x14, 0(t0)
 	cache	0x14, 32(t0)
 	cache	0x14, 64(t0)
 	cache	0x14, 96(t0)
	.set	mips0

	/* Put SDRAM to sleep */
sdsleep:
	li 	a0, MEM_PHYS_ADDR
	or 	a0, a0, 0xA0000000
#if defined(CONFIG_SOC_AU1000) || defined(CONFIG_SOC_AU1100) || defined(CONFIG_SOC_AU1500)
	lw 	k0, MEM_SDMODE0(a0)
	sw	zero, MEM_SDPRECMD(a0) 	/* Precharge */
	sw	zero, MEM_SDAUTOREF(a0)	/* Auto Refresh */
	sw	zero, MEM_SDSLEEP(a0)  	/* Sleep */
	sync
#endif
#if defined(CONFIG_SOC_AU1550) || defined(CONFIG_SOC_AU1200)
	sw	zero, MEM_SDPRECMD(a0) 	/* Precharge */
	sw 	zero, MEM_SDSREF(a0)
	
	#lw 	t0, MEM_SDSTAT(a0)
	#and t0, t0, 0x01000000
	li 	t0, 0x01000000
refresh_not_set:
	lw 	t1, MEM_SDSTAT(a0)
	and	t2, t1, t0
	beq	zero, t2, refresh_not_set
	nop

	li	t0, ~0x30000000
	lw 	t1, MEM_SDCONFIGA(a0)
	and t1, t0, t1
	sw 	t1, MEM_SDCONFIGA(a0)
	sync
#endif

	li	t0, SYS_SLEEP
	sw	zero, 0(t0)	/* Put processor to sleep */
	sync
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop


	/* This is where we return upon wakeup.
	 * Reload all of the registers and return.
	 */
resume_from_sleep:
	nop
	.set nomacro
	.set noat

	lw	k0, PT_C0STATUS(sp) // 0x20
	mtc0	k0, CP0_STATUS
	lw	k0, PT_CONTEXT(sp) // 0x1c
	mtc0	k0, CP0_CONTEXT
	lw	k0, PT_PAGEMASK(sp) // 0x18
	mtc0	k0, CP0_PAGEMASK
	lw	k0, PT_CONFIG(sp) // 0x14
	mtc0	k0, CP0_CONFIG
	lw	$1, PT_R1(sp)
	lw	$2, PT_R2(sp)
	lw	$3, PT_R3(sp)
	lw	$4, PT_R4(sp)
	lw	$5, PT_R5(sp)
	lw	$6, PT_R6(sp)
	lw	$7, PT_R7(sp)
	lw	$16, PT_R16(sp)
	lw	$17, PT_R17(sp)
	lw	$18, PT_R18(sp)
	lw	$19, PT_R19(sp)
	lw	$20, PT_R20(sp)
	lw	$21, PT_R21(sp)
	lw	$22, PT_R22(sp)
	lw	$23, PT_R23(sp)
	lw	$28, PT_R28(sp)
	lw	$30, PT_R30(sp)
	lw	$31, PT_R31(sp)

	.set macro
	.set at

	/* clear the wake source, but save it as the return value of the function */
	li	t0, SYS_WAKESRC
	lw v0, 0(t0)
	sw v0, PT_R2(sp)
	sw zero, 0(t0)

	addiu	sp, PT_SIZE

	lw gp, save_and_sleep_frmsz-44(sp)
	lw s8, save_and_sleep_frmsz-40(sp)
	lw s7, save_and_sleep_frmsz-36(sp)
	lw s6, save_and_sleep_frmsz-32(sp)
	lw s5, save_and_sleep_frmsz-28(sp)
	lw s4, save_and_sleep_frmsz-24(sp)
	lw s3, save_and_sleep_frmsz-20(sp)
	lw s2, save_and_sleep_frmsz-16(sp)
	lw s1, save_and_sleep_frmsz-12(sp)
	lw s0, save_and_sleep_frmsz-8(sp)
	lw ra, save_and_sleep_frmsz-4(sp)

	addu sp, save_and_sleep_frmsz
	jr	ra
	nop
	.set reorder
END(save_and_sleep)

