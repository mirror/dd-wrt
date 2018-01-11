/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2018 René van Dorst <opensource@vdorst.com>. All Rights Reserved.
 * Copyright (C) 2015-2017 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <asm/asm.h>
#include <asm/prefetch.h>

.data
.align 4
jmp_table: .word .Lchacha20_mips_xor_64b,.Lchacha20_mips_xor_1b,.Lchacha20_mips_xor_2b,.Lchacha20_mips_xor_3b,.Lchacha20_mips_xor_4b,.Lchacha20_mips_xor_5b,.Lchacha20_mips_xor_6b,.Lchacha20_mips_xor_7b,.Lchacha20_mips_xor_8b,.Lchacha20_mips_xor_9b,.Lchacha20_mips_xor_10b,.Lchacha20_mips_xor_11b,.Lchacha20_mips_xor_12b,.Lchacha20_mips_xor_13b,.Lchacha20_mips_xor_14b,.Lchacha20_mips_xor_15b,.Lchacha20_mips_xor_16b,.Lchacha20_mips_xor_17b,.Lchacha20_mips_xor_18b,.Lchacha20_mips_xor_19b,.Lchacha20_mips_xor_20b,.Lchacha20_mips_xor_21b,.Lchacha20_mips_xor_22b,.Lchacha20_mips_xor_23b,.Lchacha20_mips_xor_24b,.Lchacha20_mips_xor_25b,.Lchacha20_mips_xor_26b,.Lchacha20_mips_xor_27b,.Lchacha20_mips_xor_28b,.Lchacha20_mips_xor_29b,.Lchacha20_mips_xor_30b,.Lchacha20_mips_xor_31b,.Lchacha20_mips_xor_32b,.Lchacha20_mips_xor_33b,.Lchacha20_mips_xor_34b,.Lchacha20_mips_xor_35b,.Lchacha20_mips_xor_36b,.Lchacha20_mips_xor_37b,.Lchacha20_mips_xor_38b,.Lchacha20_mips_xor_39b,.Lchacha20_mips_xor_40b,.Lchacha20_mips_xor_41b,.Lchacha20_mips_xor_42b,.Lchacha20_mips_xor_43b,.Lchacha20_mips_xor_44b,.Lchacha20_mips_xor_45b,.Lchacha20_mips_xor_46b,.Lchacha20_mips_xor_47b,.Lchacha20_mips_xor_48b,.Lchacha20_mips_xor_49b,.Lchacha20_mips_xor_50b,.Lchacha20_mips_xor_51b,.Lchacha20_mips_xor_52b,.Lchacha20_mips_xor_53b,.Lchacha20_mips_xor_54b,.Lchacha20_mips_xor_55b,.Lchacha20_mips_xor_56b,.Lchacha20_mips_xor_57b,.Lchacha20_mips_xor_58b,.Lchacha20_mips_xor_59b,.Lchacha20_mips_xor_60b,.Lchacha20_mips_xor_61b,.Lchacha20_mips_xor_62b,.Lchacha20_mips_xor_63b

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SH  swl
#define SR  8
#define ROTR(n) rotr n, 24
#else
#define SH  sh
#define SR  16
#define ROTR(n)
#endif

#define X0  $v0
#define X1  $v1
#define X2  $ra
#define X3  $fp
#define X4  $s0
#define X5  $s1
#define X6  $s2
#define X7  $s3
#define X8  $s4
#define X9  $s5
#define X10 $s6
#define X11 $s7
#define X12 $t0
#define X13 $t1
#define X14 $t2
#define X15 $t3
#define T0  $t4
#define T1  $t5
#define T2  $t6
#define T3  $t7
#define T4  $t8
#define T5  $t9
#define T6  $at
#define T(n) T ## n
#define X(n) X ## n

#define AXR(A, B, C, D,  K, L, M, N,  V, W, Y, Z,  S) \
	addu X(A), X(K); \
	addu X(B), X(L); \
	addu X(C), X(M); \
	addu X(D), X(N); \
	xor  X(V), X(A); \
	xor  X(W), X(B); \
	xor  X(Y), X(C); \
	xor  X(Z), X(D); \
	rotl X(V), S;    \
	rotl X(W), S;    \
	rotl X(Y), S;    \
	rotl X(Z), S;

#define CHACHA20_BLOCK_SIZE 64
#define STACK_SIZE	96
#define OUT		$a0
#define IN		$a1
#define BYTES		$a2
#define KEY		$a3
#define ROUND_INDEX	T6
#define COUNTER		T4
#define COUNTER_0	T5
#define LAST_BLOCK	T6

#define CONSTANT_1	0x61707865
#define CONSTANT_2	0x3320646e
#define CONSTANT_3	0x79622d32
#define CONSTANT_4	0x6b206574

#define TEMP_XOR_OFFSET 40

.text
// chacha20_mips(u8 *out, const u8 *in, size_t len, const u32 key[8], const u32 counter[4]);
.align 4
.set reorder
.set noat
.globl chacha20_mips
.ent   chacha20_mips
chacha20_mips:
	// this is in the fifth argument
	lw COUNTER, 16($sp)

	slti	T6, BYTES, 256
	addiu $sp, -(STACK_SIZE)
	bnez	T6, .Lchacha20_mips_noprefetch
	PREF( Pref_Load, 0(KEY) )
	PREF( Pref_Load, 32(KEY) )
	// Prefetch first 256 bytes.
	PREF( Pref_Load,  0(IN) )
	PREF( Pref_Load, 32(IN) )
	PREF( Pref_Load, 64(IN) )
	PREF( Pref_Load, 96(IN) )
	PREF( Pref_Load,128(IN) )
	PREF( Pref_Load,160(IN) )
	PREF( Pref_Load,192(IN) )
	PREF( Pref_Load,224(IN) )

.Lchacha20_mips_noprefetch:

	// Return if no bytes
	.set noreorder
	beqz BYTES, .Lchacha20_mips_end
	nop
	.set reorder

	sw $ra,  0($sp)
	sw $fp,  4($sp)
	sw $s0,  8($sp)
	sw $s1, 12($sp)
	sw $s2, 16($sp)
	sw $s3, 20($sp)
	sw $s4, 24($sp)
	sw $s5, 28($sp)
	sw $s6, 32($sp)
	sw $s7, 36($sp)

	lw COUNTER_0, 0(COUNTER)

.Loop_chacha:
	lw X4,   0(KEY)
	lw X5,   4(KEY)
	lw X6,   8(KEY)
	lw X7,  12(KEY)
	lui X0, %hi(CONSTANT_1)
	lui X1, %hi(CONSTANT_2)
	lui X2, %hi(CONSTANT_3)
	lui X3, %hi(CONSTANT_4)
	lw X8,  16(KEY)
	lw X9,  20(KEY)
	lw X10, 24(KEY)
	lw X11, 28(KEY)
	ori X0, %lo(CONSTANT_1)
	ori X1, %lo(CONSTANT_2)
	ori X2, %lo(CONSTANT_3)
	ori X3, %lo(CONSTANT_4)
	move X12, COUNTER_0
	lw X13,  4(COUNTER)
	lw X14,  8(COUNTER)
	lw X15, 12(COUNTER)

	li	ROUND_INDEX, 9
.Loop_chacha_xor_rounds:
	AXR( 0, 1, 2, 3,  4, 5, 6, 7, 12,13,14,15, 16);
	AXR( 8, 9,10,11, 12,13,14,15,  4, 5, 6, 7, 12);
	AXR( 0, 1, 2, 3,  4, 5, 6, 7, 12,13,14,15,  8);
	AXR( 8, 9,10,11, 12,13,14,15,  4, 5, 6, 7,  7);
	AXR( 0, 1, 2, 3,  5, 6, 7, 4, 15,12,13,14, 16);
	AXR(10,11, 8, 9, 15,12,13,14,  5, 6, 7, 4, 12);
	AXR( 0, 1, 2, 3,  5, 6, 7, 4, 15,12,13,14,  8);
	AXR(10,11, 8, 9, 15,12,13,14,  5, 6, 7, 4,  7);

	.set noreorder
	bnez ROUND_INDEX, .Loop_chacha_xor_rounds
	subu ROUND_INDEX, 1
	.set reorder

	lui T0, %hi(CONSTANT_1)
	lui T1, %hi(CONSTANT_2)
	lui T2, %hi(CONSTANT_3)
	lui T3, %hi(CONSTANT_4)
	ori T0, %lo(CONSTANT_1)
	ori T1, %lo(CONSTANT_2)
	ori T2, %lo(CONSTANT_3)
	ori T3, %lo(CONSTANT_4)
	addu X0, T0
	addu X1, T1
	addu X2, T2
	addu X3, T3
	lw T0, 0(KEY)
	lw T1, 4(KEY)
	lw T2, 8(KEY)
	lw T3, 12(KEY)
		lui     T6, %hi(jmp_table)
	addu X4, T0
	addu X5, T1
	addu X6, T2
	addu X7, T3
	lw T0, 16(KEY)
	lw T1, 20(KEY)
	lw T2, 24(KEY)
	lw T3, 28(KEY)
		ins     T6, BYTES, 2, 6
	addu X8, T0
	sltiu	T0, BYTES, CHACHA20_BLOCK_SIZE
	addu X9, T1
	addu X10, T2
	addu X11, T3
	lw T1, 4(COUNTER)
	lw T2, 8(COUNTER)
	lw T3, 12(COUNTER)
		addiu   T6, %lo(jmp_table)
	addu X12, COUNTER_0
	addu X13, T1
		lw      T6, 0(T6)
	addu X14, T2
	addu X15, T3

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	wsbh 	X0
	wsbh 	X1
	wsbh 	X2
	wsbh 	X3
	wsbh 	X4
	wsbh 	X5
	wsbh 	X6
	wsbh 	X7
	wsbh 	X8
	wsbh 	X9
	wsbh 	X10
	wsbh 	X11
	wsbh 	X12
	wsbh 	X13
	wsbh 	X14
	wsbh 	X15
	rotr 	X0, 16
	rotr 	X1, 16
	rotr 	X2, 16
	rotr 	X3, 16
	rotr 	X4, 16
	rotr 	X5, 16
	rotr 	X6, 16
	rotr 	X7, 16
	rotr 	X8, 16
	rotr 	X9, 16
	rotr 	X10, 16
	rotr 	X11, 16
	rotr 	X12, 16
	rotr 	X13, 16
	rotr 	X14, 16
	rotr 	X15, 16
#endif

	.set noreorder
	beqz	T0, .Lchacha20_mips_xor_64b
	addiu BYTES, -(CHACHA20_BLOCK_SIZE)
	.set reorder

	jr      T6
	nop

.Lchacha20_mips_xor_64b:

	slti	T6, BYTES, 256
	bnez	T6, .Lchacha20_mips_noprefetch2

	PREF( Pref_Load,192(IN) )
	PREF( Pref_Load,224(IN) )

.Lchacha20_mips_noprefetch2:

	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	lw T3, 12(IN)
	xor X0, T0
	xor X1, T1
	xor X2, T2
	xor X3, T3
	sw X0,   0(OUT)
	sw X1,   4(OUT)
	sw X2,   8(OUT)
	sw X3,  12(OUT)

	lw T0, 16(IN)
	lw T1, 20(IN)
	lw T2, 24(IN)
	lw T3, 28(IN)
	xor X4, T0
	xor X5, T1
	xor X6, T2
	xor X7, T3
	sw X4,  16(OUT)
	sw X5,  20(OUT)
	sw X6,  24(OUT)
	sw X7,  28(OUT)

	lw T0, 32(IN)
	lw T1, 36(IN)
	lw T2, 40(IN)
	lw T3, 44(IN)
	xor X8, T0
	xor X9, T1
	xor X10, T2
	xor X11, T3
	sw X8,  32(OUT)
	sw X9,  36(OUT)
	sw X10, 40(OUT)
	sw X11, 44(OUT)

	lw T0, 48(IN)
	lw T1, 52(IN)
	lw T2, 56(IN)
	lw T3, 60(IN)
	xor X12, T0
	xor X13, T1
	xor X14, T2
	xor X15, T3
	sw X12, 48(OUT)
	sw X13, 52(OUT)
	sw X14, 56(OUT)
	sw X15, 60(OUT)

	// update pointers
	addiu IN,  CHACHA20_BLOCK_SIZE
	addiu OUT, CHACHA20_BLOCK_SIZE

.set noreorder
	// bytes left?
	bnez BYTES, .Loop_chacha
	addiu COUNTER_0, 1
.set reorder

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_63b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	lw T3,60(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	xor X15,T3
	sw X12,48(OUT)
	sw X13,52(OUT)
	sw X14,56(OUT)
	srl T5, X15, SR
	SH X15,60(OUT)
	sb T5,62(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_62b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	lw T3,60(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	xor X15,T3
	sw X12,48(OUT)
	sw X13,52(OUT)
	sw X14,56(OUT)
	SH X15,60(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_61b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	lw T3,60(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	xor X15,T3
	sw X12,48(OUT)
	sw X13,52(OUT)
	sw X14,56(OUT)
	ROTR(X15)
	sb X15,60(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_60b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	sw X12,48(OUT)
	sw X13,52(OUT)
	sw X14,56(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_59b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	sw X12,48(OUT)
	sw X13,52(OUT)
	srl T5, X14, SR
	SH X14,56(OUT)
	sb T5,58(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_58b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	sw X12,48(OUT)
	sw X13,52(OUT)
	SH X14,56(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_57b:
	lw T0,48(IN)
	lw T1,52(IN)
	lw T2,56(IN)
	xor X12,T0
	xor X13,T1
	xor X14,T2
	sw X12,48(OUT)
	sw X13,52(OUT)
	ROTR(X14)
	sb X14,56(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_56b:
	lw T0,48(IN)
	lw T1,52(IN)
	xor X12,T0
	xor X13,T1
	sw X12,48(OUT)
	sw X13,52(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_55b:
	lw T0,48(IN)
	lw T1,52(IN)
	xor X12,T0
	xor X13,T1
	sw X12,48(OUT)
	srl T5, X13, SR
	SH X13,52(OUT)
	sb T5,54(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_54b:
	lw T0,48(IN)
	lw T1,52(IN)
	xor X12,T0
	xor X13,T1
	sw X12,48(OUT)
	SH X13,52(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_53b:
	lw T0,48(IN)
	lw T1,52(IN)
	xor X12,T0
	xor X13,T1
	sw X12,48(OUT)
	ROTR(X13)
	sb X13,52(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_52b:
	lw T0,48(IN)
	xor X12,T0
	sw X12,48(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_51b:
	lw T0,48(IN)
	xor X12,T0
	srl T5, X12, SR
	SH X12,48(OUT)
	sb T5,50(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_50b:
	lw T0,48(IN)
	xor X12,T0
	SH X12,48(OUT)

	b .Lchacha20_mips_xor_48b
	nop

.Lchacha20_mips_xor_49b:
	lw T0,48(IN)
	xor X12,T0
	ROTR(X12)
	sb X12,48(OUT)

.Lchacha20_mips_xor_48b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	lw T3,44(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	xor X11,T3
	sw X8,32(OUT)
	sw X9,36(OUT)
	sw X10,40(OUT)
	sw X11,44(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_47b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	lw T3,44(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	xor X11,T3
	sw X8,32(OUT)
	sw X9,36(OUT)
	sw X10,40(OUT)
	srl T5, X11, SR
	SH X11,44(OUT)
	sb T5,46(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_46b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	lw T3,44(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	xor X11,T3
	sw X8,32(OUT)
	sw X9,36(OUT)
	sw X10,40(OUT)
	SH X11,44(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_45b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	lw T3,44(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	xor X11,T3
	sw X8,32(OUT)
	sw X9,36(OUT)
	sw X10,40(OUT)
	ROTR(X11)
	sb X11,44(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_44b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	sw X8,32(OUT)
	sw X9,36(OUT)
	sw X10,40(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_43b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	sw X8,32(OUT)
	sw X9,36(OUT)
	srl T5, X10, SR
	SH X10,40(OUT)
	sb T5,42(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_42b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	sw X8,32(OUT)
	sw X9,36(OUT)
	SH X10,40(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_41b:
	lw T0,32(IN)
	lw T1,36(IN)
	lw T2,40(IN)
	xor X8,T0
	xor X9,T1
	xor X10,T2
	sw X8,32(OUT)
	sw X9,36(OUT)
	ROTR(X10)
	sb X10,40(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_40b:
	lw T0,32(IN)
	lw T1,36(IN)
	xor X8,T0
	xor X9,T1
	sw X8,32(OUT)
	sw X9,36(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_39b:
	lw T0,32(IN)
	lw T1,36(IN)
	xor X8,T0
	xor X9,T1
	sw X8,32(OUT)
	srl T5, X9, SR
	SH X9,36(OUT)
	sb T5,38(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_38b:
	lw T0,32(IN)
	lw T1,36(IN)
	xor X8,T0
	xor X9,T1
	sw X8,32(OUT)
	SH X9,36(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_37b:
	lw T0,32(IN)
	lw T1,36(IN)
	xor X8,T0
	xor X9,T1
	sw X8,32(OUT)
	ROTR(X9)
	sb X9,36(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_36b:
	lw T0,32(IN)
	xor X8,T0
	sw X8,32(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_35b:
	lw T0,32(IN)
	xor X8,T0
	srl T5, X8, SR
	SH X8,32(OUT)
	sb T5,34(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_34b:
	lw T0,32(IN)
	xor X8,T0
	SH X8,32(OUT)

	b .Lchacha20_mips_xor_32b
	nop

.Lchacha20_mips_xor_33b:
	lw T0,32(IN)
	xor X8,T0
	ROTR(X8)
	sb X8,32(OUT)

.Lchacha20_mips_xor_32b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	lw T3,28(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	xor X7,T3
	sw X4,16(OUT)
	sw X5,20(OUT)
	sw X6,24(OUT)
	sw X7,28(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_31b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	lw T3,28(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	xor X7,T3
	sw X4,16(OUT)
	sw X5,20(OUT)
	sw X6,24(OUT)
	srl T5, X7, SR
	SH X7,28(OUT)
	sb T5,30(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_30b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	lw T3,28(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	xor X7,T3
	sw X4,16(OUT)
	sw X5,20(OUT)
	sw X6,24(OUT)
	SH X7,28(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_29b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	lw T3,28(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	xor X7,T3
	sw X4,16(OUT)
	sw X5,20(OUT)
	sw X6,24(OUT)
	ROTR(X7)
	sb X7,28(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_28b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	sw X4,16(OUT)
	sw X5,20(OUT)
	sw X6,24(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_27b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	sw X4,16(OUT)
	sw X5,20(OUT)
	srl T5, X6, SR
	SH X6,24(OUT)
	sb T5,26(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_26b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	sw X4,16(OUT)
	sw X5,20(OUT)
	SH X6,24(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_25b:
	lw T0,16(IN)
	lw T1,20(IN)
	lw T2,24(IN)
	xor X4,T0
	xor X5,T1
	xor X6,T2
	sw X4,16(OUT)
	sw X5,20(OUT)
	ROTR(X6)
	sb X6,24(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_24b:
	lw T0,16(IN)
	lw T1,20(IN)
	xor X4,T0
	xor X5,T1
	sw X4,16(OUT)
	sw X5,20(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_23b:
	lw T0,16(IN)
	lw T1,20(IN)
	xor X4,T0
	xor X5,T1
	sw X4,16(OUT)
	srl T5, X5, SR
	SH X5,20(OUT)
	sb T5,22(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_22b:
	lw T0,16(IN)
	lw T1,20(IN)
	xor X4,T0
	xor X5,T1
	sw X4,16(OUT)
	SH X5,20(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_21b:
	lw T0,16(IN)
	lw T1,20(IN)
	xor X4,T0
	xor X5,T1
	sw X4,16(OUT)
	ROTR(X5)
	sb X5,20(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_20b:
	lw T0,16(IN)
	xor X4,T0
	sw X4,16(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_19b:
	lw T0,16(IN)
	xor X4,T0
	srl T5, X4, SR
	SH X4,16(OUT)
	sb T5,18(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_18b:
	lw T0,16(IN)
	xor X4,T0
	SH X4,16(OUT)

	b .Lchacha20_mips_xor_16b
	nop

.Lchacha20_mips_xor_17b:
	lw T0,16(IN)
	xor X4,T0
	ROTR(X4)
	sb X4,16(OUT)

.Lchacha20_mips_xor_16b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	lw T3,12(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	xor X3,T3
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	sw X2, 8(OUT)
	sw X3,12(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_15b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	lw T3,12(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	xor X3,T3
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	sw X2, 8(OUT)
	srl T5, X3, SR
	SH X3,12(OUT)
	sb T5,14(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_14b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	lw T3,12(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	xor X3,T3
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	sw X2, 8(OUT)
	SH X3,12(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_13b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	lw T3,12(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	xor X3,T3
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	sw X2, 8(OUT)
	ROTR(X3)
	sb X3,12(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_12b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	sw X2, 8(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_11b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	srl T5, X2, SR
	SH X2, 8(OUT)
	sb T5,10(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_10b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	SH X2, 8(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_9b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	lw T2, 8(IN)
	xor X0,T0
	xor X1,T1
	xor X2,T2
	sw X0, 0(OUT)
	sw X1, 4(OUT)
	ROTR(X2)
	sb X2, 8(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_8b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	xor X0,T0
	xor X1,T1
	sw X0, 0(OUT)
	sw X1, 4(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_7b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	xor X0,T0
	xor X1,T1
	sw X0, 0(OUT)
	srl T5, X1, SR
	SH X1, 4(OUT)
	sb T5, 6(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_6b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	xor X0,T0
	xor X1,T1
	sw X0, 0(OUT)
	SH X1, 4(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_5b:
	lw T0, 0(IN)
	lw T1, 4(IN)
	xor X0,T0
	xor X1,T1
	sw X0, 0(OUT)
	ROTR(X1)
	sb X1, 4(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_4b:
	lw T0, 0(IN)
	xor X0,T0
	sw X0, 0(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_3b:
	lw T0, 0(IN)
	xor X0,T0
	rotr T5, X0, SR
	SH X0, 0(OUT)
	sb T5, 2(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_2b:
	lw T0, 0(IN)
	xor X0,T0
	SH X0, 0(OUT)

	b .Lchacha20_mips_done
	nop

.Lchacha20_mips_xor_1b:
	lw T0, 0(IN)
	xor X0,T0
	ROTR(X0)
	sb X0, 0(OUT)

.Lchacha20_mips_done:
	lw $ra,  0($sp)
	lw $fp,  4($sp)
	lw $s0,  8($sp)
	lw $s1, 12($sp)
	lw $s2, 16($sp)
	lw $s3, 20($sp)
	lw $s4, 24($sp)
	lw $s5, 28($sp)
	lw $s6, 32($sp)
	lw $s7, 36($sp)

.Lchacha20_mips_end:
	.set noreorder
	jr	$ra
	addiu	$sp, STACK_SIZE
	.set reorder
	.set at
.end chacha20_mips


