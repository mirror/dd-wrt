/*
 * BCM947XX Boot code for standalone apps.
 *
 * Code should be position-independent until it copies itself to SDRAM.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include "mipsinc.h"
#include "sbconfig.h"
#include "sbchipc.h"
#include "bcmdevs.h"

	.text
	LEAF(startup)
	.set	noreorder

	# XXX: the following code snipet sets clk frequency to 200M
	# correct pll clk freq to real speed in the 5350 case.
	# It is Ugly...but
	li      t0,KSEG1ADDR(SB_ENUM_BASE)      # Is there a chipcommon core?
	lw      t1,(SBCONFIGOFF + SBIDHIGH)(t0)
	and     t1,t1,SBIDH_CC_MASK
	srl     t1,t1,SBIDH_CC_SHIFT
	bne     t1,SB_CC,2f

	li	a2,KSEG1ADDR(SB_ENUM_BASE)
	li	a3,BCM5350_DEVICE_ID	# 5350 ChipID
	lw	t1,CC_CHIPID(a2)	# ChipID register
	andi	t1,CID_ID_MASK		# chip id is bit 0-15
	bne	a3,t1,2f		# if not 5350 then skip
	nop
	li	a3,CLKC_5350_N
	lw	t1,CC_CLKC_N(a2)
	beq	a3,t1,2f		# move ahead if clk freq set correctly
	nop
	sw	a3,CC_CLKC_N(a2)	# set control N1 to select 6
	li	t1,1
	sw	t1,CC_WATCHDOG(a2)	# set WatchDog Reset
1:	b	1b
	nop

2:
	/* Check if we booted from SDRAM */
	bal	1f
	nop
1:	li	t0,PHYSADDR_MASK
	and	t0,t0,ra
	li	t1,SB_FLASH1
	bge	t0,t1,inflash
	nop

	/* Call draminit to size memory */
	jal	board_draminit
	nop
	b	setsp
	nop

inflash:
	/* Is this chipc rev 11 or 12 and a serial flash? */
	li	t0,KSEG1ADDR(SB_ENUM_BASE)
	lw	t1,(SBCONFIGOFF + SBIDHIGH)(t0)
	and	t2,t1,SBIDH_CC_MASK
	srl	t2,t2,SBIDH_CC_SHIFT
	bne	t2,SB_CC,checkcon		/* Not chipc */
	nop
	and	t2,t1,SBIDH_RC_MASK
	and	t3,t1,SBIDH_RCE_MASK
	srl	t3,t3,SBIDH_RCE_SHIFT
	or	t2,t3
	ble	t2,10,checkcon			/* ccrev <= 10 */
	nop
	bge	t2,13,checkcon			/* ccrev >= 13 */
	nop
	lw	t0,CC_CAPABILITIES(t0)
	and	t0,t0,CAP_FLASH_MASK
	beq	t0,SFLASH_AT,switchkseg0	/* Atmel sflash */
	nop
	beq	t0,SFLASH_ST,switchkseg0	/* ST sflash */
	nop


checkcon:
	/* Check if the caches are already on */
	mfc0	t0,C0_CONFIG
	and	t0,CONF_CM_CMASK
	beq	t0,CONF_CM_UNCACHED,initcaches
	nop

	b	switchkseg0
	nop

initcaches:
	/* Turn on the caches in the CP0 register */
	mfc0	t0,C0_DIAGNOSTIC
	or	t0,(BRCM_IC_ENABLE | BRCM_DC_ENABLE) /* Enable both I$ and D$ */
	mtc0	t0,C0_DIAGNOSTIC


1:	/* Get cache sizes */
	.set	mips32
	mfc0	s0,C0_CONFIG,1
	.set	mips0

	li	s1,CONF1_DL_MASK
	and	s1,s0
	beq	s1,zero,nodc
	nop

	srl	s1,CONF1_DL_SHIFT
	li	t0,CONF1_DL_BASE
	sll	s1,t0,s1		/* s1 has D$ cache line size */

	li	s2,CONF1_DA_MASK
	and	s2,s0
	srl	s2,CONF1_DA_SHIFT
	addiu	s2,CONF1_DA_BASE	/* s2 now has D$ associativity */

	li	t0,CONF1_DS_MASK
	and	t0,s0
	srl	t0,CONF1_DS_SHIFT
	li	s3,CONF1_DS_BASE
	sll	s3,s3,t0		/* s3 has D$ sets per way */

	multu	s2,s3			/* sets/way * associativity */
	mflo	t0			/* total cache lines */

	multu	s1,t0			/* D$ linesize * lines */
	mflo	s2			/* s2 is now D$ size in bytes */

	/* Initilize the D$: */
	mtc0	zero,C0_TAGLO
	mtc0	zero,C0_TAGHI

	li	t0,KSEG0		/* Just an address for the first $ line */
	addu	t1,t0,s2		/*  + size of cache == end */

	.set	mips3
1:	cache	Index_Store_Tag_D,0(t0)
	.set	mips0
	bne	t0,t1,1b
	addu	t0,s1

nodc:
	/* Now we get to do it all again for the I$ */

	li	t0,CONF1_IL_MASK
	and	t0,s0
	beq	t0,zero,noic
	nop

	srl	t0,CONF1_IL_SHIFT
	li	s3,CONF1_IL_BASE
	sll	s3,t0			/* s3 has I$ cache line size */

	li	t0,CONF1_IA_MASK
	and	t0,s0
	srl	t0,CONF1_IA_SHIFT
	addiu	s4,t0,CONF1_IA_BASE	/* s4 now has I$ associativity */

	li	t0,CONF1_IS_MASK
	and	t0,s0
	srl	t0,CONF1_IS_SHIFT
	li	s5,CONF1_IS_BASE
	sll	s5,t0			/* s5 has I$ sets per way */

	multu	s4,s5			/* sets/way * associativity */
	mflo	t0			/* s4 is not total cache lines */

	multu	s3,t0			/* I$ linesize * lines */
	mflo	s4			/* s4 is cache size in bytes */

	/* Initilize the I$: */
	mtc0	zero,C0_TAGLO
	mtc0	zero,C0_TAGHI

	li	t0,KSEG0		/* Just an address for the first $ line */
	addu	t1,t0,s4		/*  + size of cache == end */

	.set	mips3
1:	cache	Index_Store_Tag_I,0(t0)
	.set	mips0
	bne	t0,t1,1b
	addu	t0,s1

noic:	
	/* Caches initialized, change cacheability */
	mfc0	t0,C0_CONFIG
	and	t0,~CONF_CM_CMASK
	or	t0,CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0,C0_CONFIG
	nop

switchkseg0:
	/* And now jump to KSEG0 */
	li	t0,KSEG1ADDR(SB_FLASH1)
	la	t1,text_start
	la	t2,initdram
	sub	t0,t0,t1
	add	t2,t2,t0
	and	t2,PHYSADDR_MASK
	or	t2,KSEG0
	jr	t2
	nop

initdram:
	/* Initialize SDRAM */
	li	t0,KSEG0ADDR(SB_FLASH1)
	la	t1,text_start
	la	t2,board_draminit
	sub	t0,t0,t1
	add	t2,t2,t0
	jalr	t2
	nop

	/* v0 now contains memory size in bytes */

	/* Copy self to SDRAM */
#ifdef	CONFIG_XIP
	la	a0,text_end
	la	a1,data_start
	la	a2,input_data
#else
	li	a0,KSEG0ADDR(SB_FLASH1)
	la	a1,text_start
	and	a1,PHYSADDR_MASK	/* Uncached writes to avoid a flush */
	or	a1,KSEG1
	la	a2,input_data
	and	a2,PHYSADDR_MASK
	or	a2,KSEG1
#endif
1:	lw	t0,0(a0)
	sw	t0,0(a1)
	add	a0,4
	add	a1,4
	blt	a1,a2,1b
	nop

setsp:
	/* Set up stack pointer */
	or	v0,KSEG0
	sub	sp,v0,4

	/* Clear BSS */	
	la	a0,bss_start
	la	a1,bss_end
1:	sw	zero,0(a0)
	addi	a0,a0,4
	blt	a0,a1,1b
	nop

	/* Setup trap handlers */
	li	t0,KSEG0ADDR(SB_FLASH1)
	la	t1,text_start
	la	t2,trap_init
	sub	t0,t0,t1
	add	t2,t2,t0
	jalr	t2
	nop

	/* Jump to C */
	la	t0,c_main
	jal	t0
	move	a0,ra

	/* In case c_main returns */
theend:	nop
	nop
	.set	mips32
	wait
	.set	mips0
	nop
	nop
	b	theend
	nop

	/* Black hole for traps with BEV on */
	.org	0x380
bevtrap: nop
	nop
	.set	mips32
	wait
	.set	mips0
	nop
	nop
	b	bevtrap
	nop

	/* Record the size of the binary */
	.org	BISZ_OFFSET
	.word	BISZ_MAGIC
	.word	text_start
	.word	text_end
	.word	data_start
	.word	data_end
	.word	bss_start
	.word	bss_end

	/* Embedded NVRAM */
	.balign	0x400	
	.globl	embedded_nvram
embedded_nvram:
	.fill	0x100,4,~(0x48534c46)
	
	.set reorder
	END(startup)
