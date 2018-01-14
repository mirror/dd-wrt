/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2017 René van Dorst <opensource@vdorst.com>. All Rights Reserved.
 */

#include <linux/linkage.h>

#define POLY1305_BLOCK_SIZE 16
.text
#define H0 $t0
#define H1 $t1
#define H2 $t2
#define H3 $t3
#define H4 $t4

#define R0 $t5
#define R1 $t6
#define R2 $t7
#define R3 $t8

#define O0 $s0
#define O1 $s4
#define O2 $v1
#define O3 $t9

#define S1 $s1
#define S2 $s2
#define S3 $s3

#define SC $at
#define CA $v0

#define poly       $a0
#define src        $a1
#define srclen     $a2
#define hibit      $a3

#define PTR_POLY1305_R(n) ( 0 + (n*4)) ## ($a0)
#define PTR_POLY1305_CA   (16        ) ## ($a0)
#define PTR_POLY1305_H(n) (20 + (n*4)) ## ($a0)

#define POLY1305_BLOCK_SIZE 16
#define POLY1305_STACK_SIZE 8 * 4

// static unsigned int poly1305_blocks_mips   (struct poly1305_ctx *ctx, const u8 *src, unsigned int srclen, u32 hibit)
// static void         poly1305_blocks_generic(void *ctx, const u8 *inp, size_t len, u32 padbit)
.set reorder
.set noat
.globl poly1305_blocks_mips
.ent poly1305_blocks_mips
poly1305_blocks_mips:
	.frame  $sp,POLY1305_STACK_SIZE,$31
	// srclen &= 0xFFFFFFFF - (POLY1305_BLOCK_SIZE - 1) = 0xFFFFFFF0
	ins	srclen, $zero, 0, 3

	.set noreorder
	// check size > 16 bytes
	bgtz	srclen, .Lpoly1305_blocks_mips_start
	// calculate end address. endaddress = src + (srclen & 0xFFFFFFF0)
	addu	srclen, src
	.set reorder

	jr $ra
	nop

.align 4
.Lpoly1305_blocks_mips_start:
	addiu	$sp, -POLY1305_STACK_SIZE
	lw R0, PTR_POLY1305_R(0)
	lw R1, PTR_POLY1305_R(1)
	lw R2, PTR_POLY1305_R(2)
	lw R3, PTR_POLY1305_R(3)

	// store the used save registers.
	sw  $s0, 0($sp)
	sw  $s1, 4($sp)
	sw  $s2, 8($sp)
	sw  $s3, 12($sp)
	sw  $s4, 16($sp)

	// load Hx and Carry
	lw CA, PTR_POLY1305_CA
	lw H0, PTR_POLY1305_H(0)
	lw H1, PTR_POLY1305_H(1)
	lw H2, PTR_POLY1305_H(2)
	lw H3, PTR_POLY1305_H(3)
	lw H4, PTR_POLY1305_H(4)

	srl S2, R2, 2
	addu S2, R2
	srl S3, R3, 2
	addu S3, R3

	addiu SC, $zero, 1

.Lpoly1305_loop:
	lw O0, 0(src)
	lw O1, 4(src)
	lw O2, 8(src)
	lw O3,12(src)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	wsbh O0
	wsbh O1
	wsbh O2
	wsbh O3
	rotr O0, 16
	rotr O1, 16
	rotr O2, 16
	rotr O3, 16
#endif

	// h0 = (u32)(d0 = (u64)h0 + le32_to_cpuvp(inp + 0) + c 'Carry_previous cycle');
	multu H0, SC
	maddu CA, SC
	maddu O0, SC
	mfhi  CA
	mflo  O0

	// h4 += (u32)(d3 >> 32) + padbit;
	addu H4, hibit

	// h1 = (u32)(d1 = (u64)h1 + (d0 >> 32) + le32_to_cpuvp(inp + 4));

	multu H1, SC
	maddu O1, SC
	maddu CA, SC
	mfhi  CA
	mflo  O1

	srl S1, R1, 2

	// h2 = (u32)(d2 = (u64)h2 + (d1 >> 32) + le32_to_cpuvp(inp + 8));

	multu H2, SC
	maddu O2, SC
	maddu CA, SC
	mfhi  CA
	mflo  O2

	addu S1, R1

	// h3 = (u32)(d3 = (u64)h3 + (d2 >> 32) + le32_to_cpuvp(inp + 12));

	multu H3, SC
	maddu O3, SC
	maddu CA, SC
	mfhi  CA
	mflo  O3

	// D0

	multu O0, R0
	maddu O1, S3
	maddu O2, S2
	addu H4, CA
	maddu O3, S1
	mfhi	CA
	mflo	H0

	// D1

	multu O0, R1
	maddu O1, R0
	maddu O2, S3
	maddu O3, S2
	maddu H4, S1
	maddu CA, SC
	mfhi	CA
	mflo	H1

	// D2

	multu O0, R2
	maddu O1, R1
	maddu O2, R0
	maddu O3, S3
	maddu H4, S2
	maddu CA, SC
	mfhi	CA
	mflo	H2

	// D4

	mul	S1, H4, R0

	// D3

	multu O0, R3
	maddu O1, R2
	maddu O2, R1
	maddu O3, R0
	maddu H4, S3
	maddu CA, SC
	mfhi	CA
	mflo	H3

	addiu src,     POLY1305_BLOCK_SIZE

	// h4 += (u32)(d3 >> 32);
	addu	S1, CA
	// h4 &= 3
	andi	H4, S1, 3
	// c = (h4 >> 2) + (h4 & ~3U);
	srl	CA, S1, 2
	ins	S1, $zero, 0, 2

	// able to do a 16 byte block.
	.set noreorder
	bne	src, srclen, .Lpoly1305_loop
	// Delay slot is always executed.
	addu	CA, S1
	.set reorder

	// restore the used save registers.
	lw  $s0, 0($sp)
	lw  $s1, 4($sp)
	lw  $s2, 8($sp)
	lw  $s3, 12($sp)
	lw  $s4, 16($sp)

	// store Hx and Carry
	sw CA, PTR_POLY1305_CA
	sw H0, PTR_POLY1305_H(0)
	sw H1, PTR_POLY1305_H(1)
	sw H2, PTR_POLY1305_H(2)
	sw H3, PTR_POLY1305_H(3)
	sw H4, PTR_POLY1305_H(4)

.Lpoly1305_blocks_mips_end:
	// Jump Back
	.set noreorder
	jr	$ra
	addiu	$sp, POLY1305_STACK_SIZE
	.set reorder
.end poly1305_blocks_mips
.set at
.set reorder

#define MAC $a1
#define NONCE $a2

#define G0 $t5
#define G1 $t6
#define G2 $t7
#define G3 $t8
#define G4 $t9

#define O4 $v1

.set reorder
.set noat
.align 4
.globl poly1305_emit_mips
.ent poly1305_emit_mips
poly1305_emit_mips:
	// load Hx and Carry
	lw CA, PTR_POLY1305_CA
	lw H0, PTR_POLY1305_H(0)
	lw H1, PTR_POLY1305_H(1)
	lw H2, PTR_POLY1305_H(2)
	lw H3, PTR_POLY1305_H(3)
	lw H4, PTR_POLY1305_H(4)

	/* Add left over carry */
	addu	H0, CA
	sltu	CA, H0, CA
	addu	H1, CA
	sltu	CA, H1, CA
	addu	H2, CA
	sltu	CA, H2, CA
	addu	H3, CA
	sltu	CA, H3, CA
	addu	H4, CA

	/* compare to modulus by computing h + -p */
	addiu	G0, H0, 5
	sltu	CA, H0, CA
	addu	G1, H1, CA
	sltu	CA, H1, CA
	addu	G2, H2, CA
	sltu	CA, H2, CA
	addu	G3, H3, CA
	sltu	CA, H3, CA
	addu	G4, H4, CA

	srl	SC, G4, 2

	/* if there was carry into 131st bit, h3:h0 = g3:g0 */
	movn	H0, G0, SC
	movn	H1, G1, SC
	movn	H2, G2, SC
	movn	H3, G3, SC

	lw	CA, 0(NONCE)
	lw	G4, 4(NONCE)
	lw	G2, 8(NONCE)
	lw	G3,12(NONCE)

	li	SC,1

	/* mac = (h + nonce) % (2^128) */
	addu	H0, CA
	sltu	CA, H0, CA

	//H1
	multu	H1, SC
	maddu	G4, SC
	maddu	CA, SC
	mfhi	CA
	mflo	H1

	//H2
	multu	H2, SC
	maddu	G2, SC
	maddu	CA, SC
	mfhi	CA
	mflo	H2

	//H3
	multu	H3, SC
	maddu	G3, SC
	maddu	CA, SC
	mflo	H3

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	wsbh H0
	wsbh H1
	wsbh H2
	wsbh H3
	rotr H0, 16
	rotr H1, 16
	rotr H2, 16
	rotr H3, 16
#endif

	// store MAC
	sw H0, 0(MAC)
	sw H1, 4(MAC)
	sw H2, 8(MAC)

	.set noreorder
	jr	$ra
	sw H3,12(MAC)
	.set reorder
.end poly1305_emit_mips

#define PR0 $t0
#define PR1 $t1
#define PR2 $t2
#define PR3 $t3
#define PT0 $t4

// asmlinkage void poly1305_init_mips(void *ctx, const u8 key[16]);
.align 4
.globl poly1305_init_mips
.ent poly1305_init_mips
poly1305_init_mips:
	lw PR0, 0($a1)
	lw PR1, 4($a1)
	lw PR2, 8($a1)
	lw PR3,12($a1)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	wsbh PR0
	wsbh PR1
	wsbh PR2
	wsbh PR3
	rotr PR0, 16
	rotr PR1, 16
	rotr PR2, 16
	rotr PR3, 16
#endif

	// store Hx and Carry
	sw $zero, PTR_POLY1305_CA
	sw $zero, PTR_POLY1305_H(0)
	sw $zero, PTR_POLY1305_H(1)
	sw $zero, PTR_POLY1305_H(2)
	sw $zero, PTR_POLY1305_H(3)
	sw $zero, PTR_POLY1305_H(4)

	lui PT0, 0x0FFF
	ori PT0, 0xFFFC

	// AND 0x0fffffff;
	ext PR0, PR0, 0, (32-4)

	// AND 0x0ffffffc;
	and PR1, PT0
	and PR2, PT0
	and PR3, PT0

	// store Rx
	sw PR0, PTR_POLY1305_R(0)
	sw PR1, PTR_POLY1305_R(1)
	sw PR2, PTR_POLY1305_R(2)

	.set noreorder
	// Jump Back
	jr $ra
	sw PR3, PTR_POLY1305_R(3)
	.set reorder
.end poly1305_init_mips
