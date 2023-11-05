#if (defined(_MIPS_ARCH_MIPS32R3) || defined(_MIPS_ARCH_MIPS32R5) || \
     defined(_MIPS_ARCH_MIPS32R6)) \
     && !defined(_MIPS_ARCH_MIPS32R2)
# define _MIPS_ARCH_MIPS32R2
#endif

#if defined(_MIPS_ARCH_MIPS32R6)
# define multu(rs,rt)
# define mflo(rd,rs,rt)	mulu	rd,rs,rt
# define mfhi(rd,rs,rt)	muhu	rd,rs,rt
#else
# define multu(rs,rt)	multu	rs,rt
# define mflo(rd,rs,rt)	mflo	rd
# define mfhi(rd,rs,rt)	mfhi	rd
#endif

#ifdef	__KERNEL__
# define poly1305_init   poly1305_init_mips
# define poly1305_blocks poly1305_blocks_mips
# define poly1305_emit   poly1305_emit_mips
#endif

#if defined(__MIPSEB__) && !defined(MIPSEB)
# define MIPSEB
#endif

#ifdef MIPSEB
# define MSB 0
# define LSB 3
#else
# define MSB 3
# define LSB 0
#endif

.text
.set	noat
.set	noreorder

.align	5
.globl	poly1305_init
.ent	poly1305_init
poly1305_init:
	.frame	$29,0,$31
	.set	reorder

	sw	$0,0($4)
	sw	$0,4($4)
	sw	$0,8($4)
	sw	$0,12($4)
	sw	$0,16($4)

	beqz	$5,.Lno_key

#if defined(_MIPS_ARCH_MIPS32R6)
	andi	$1,$5,3		# $5 % 4
	subu	$5,$5,$1		# align $5
	sll	$1,$1,3		# byte to bit offset
	lw	$8,0($5)
	lw	$9,4($5)
	lw	$10,8($5)
	lw	$11,12($5)
	beqz	$1,.Laligned_key

	lw	$24,16($5)
	subu	$2,$0,$1
# ifdef	MIPSEB
	sllv	$8,$8,$1
	srlv	$25,$9,$2
	sllv	$9,$9,$1
	or	$8,$8,$25
	srlv	$25,$10,$2
	sllv	$10,$10,$1
	or	$9,$9,$25
	srlv	$25,$11,$2
	sllv	$11,$11,$1
	or	$10,$10,$25
	srlv	$24,$24,$2
	or	$11,$11,$24
# else
	srlv	$8,$8,$1
	sllv	$25,$9,$2
	srlv	$9,$9,$1
	or	$8,$8,$25
	sllv	$25,$10,$2
	srlv	$10,$10,$1
	or	$9,$9,$25
	sllv	$25,$11,$2
	srlv	$11,$11,$1
	or	$10,$10,$25
	sllv	$24,$24,$2
	or	$11,$11,$24
# endif
.Laligned_key:
#else
	lwl	$8,0+MSB($5)
	lwl	$9,4+MSB($5)
	lwl	$10,8+MSB($5)
	lwl	$11,12+MSB($5)
	lwr	$8,0+LSB($5)
	lwr	$9,4+LSB($5)
	lwr	$10,8+LSB($5)
	lwr	$11,12+LSB($5)
#endif
#ifdef	MIPSEB
# if defined(_MIPS_ARCH_MIPS32R2)
	wsbh	$8,$8		# byte swap
	wsbh	$9,$9
	wsbh	$10,$10
	wsbh	$11,$11
	rotr	$8,$8,16
	rotr	$9,$9,16
	rotr	$10,$10,16
	rotr	$11,$11,16
# else
	srl	$1,$8,24		# byte swap
	srl	$2,$8,8
	andi	$24,$8,0xFF00
	sll	$8,$8,24
	andi	$2,0xFF00
	sll	$24,$24,8
	or	$8,$1
	 srl	$1,$9,24
	or	$2,$24
	 srl	$24,$9,8
	or	$8,$2
	 andi	$2,$9,0xFF00
	 sll	$9,$9,24
	 andi	$24,0xFF00
	 sll	$2,$2,8
	 or	$9,$1
	srl	$1,$10,24
	 or	$24,$2
	srl	$2,$10,8
	 or	$9,$24
	andi	$24,$10,0xFF00
	sll	$10,$10,24
	andi	$2,0xFF00
	sll	$24,$24,8
	or	$10,$1
	 srl	$1,$11,24
	or	$2,$24
	 srl	$24,$11,8
	or	$10,$2
	 andi	$2,$11,0xFF00
	 sll	$11,$11,24
	 andi	$24,0xFF00
	 sll	$2,$2,8
	 or	$11,$1
	 or	$24,$2
	 or	$11,$24
# endif
#endif
	lui	$1,0x0fff
	ori	$1,0xffff		# 0x0fffffff
	and	$8,$8,$1
	subu	$1,3			# 0x0ffffffc
	and	$9,$9,$1
	and	$10,$10,$1
	and	$11,$11,$1

	sw	$8,20($4)
	sw	$9,24($4)
	sw	$10,28($4)
	sw	$11,32($4)

	srl	$2,$9,2
	srl	$24,$10,2
	srl	$25,$11,2
	addu	$9,$9,$2		# s1 = r1 + (r1 >> 2)
	addu	$10,$10,$24
	addu	$11,$11,$25
	sw	$9,36($4)
	sw	$10,40($4)
	sw	$11,44($4)
.Lno_key:
	li	$2,0
	jr	$31
.end	poly1305_init
.globl	poly1305_blocks
.align	5
.ent	poly1305_blocks
poly1305_blocks:
	.frame	$29,16*4,$31
	.mask	0x00ff0000,-4
	.set	noreorder
	subu	$29, $29,4*12
	sw	$23,4*11($29)
	sw	$22,4*10($29)
	sw	$21, 4*9($29)
	sw	$20, 4*8($29)
	sw	$19, 4*7($29)
	sw	$18, 4*6($29)
	sw	$17, 4*5($29)
	sw	$16, 4*4($29)
	.set	reorder

	srl	$6,4			# number of complete blocks
	li	$25,1
	beqz	$6,.Labort

#if defined(_MIPS_ARCH_MIPS32R6)
	andi	$25,$5,3
	subu	$5,$5,$25		# align $5
	sll	$25,$25,3		# byte to bit offset
#endif

	lw	$12,0($4)		# load hash value
	lw	$13,4($4)
	lw	$14,8($4)
	lw	$15,12($4)
	lw	$16,16($4)

	lw	$17,20($4)		# load key
	lw	$18,24($4)
	lw	$19,28($4)
	lw	$20,32($4)
	lw	$21,36($4)
	lw	$22,40($4)
	lw	$23,44($4)

	sll	$6,4
	addu	$6,$6,$5		# end of buffer
	b	.Loop

.align	4
.Loop:
#if defined(_MIPS_ARCH_MIPS32R6)
	lw	$8,0($5)		# load input
	lw	$9,4($5)
	lw	$10,8($5)
	lw	$11,12($5)
	beqz	$25,.Laligned_inp

	lw	$2,16($5)
	subu	$24,$0,$25
# ifdef	MIPSEB
	sllv	$8,$8,$25
	srlv	$1,$9,$24
	sllv	$9,$9,$25
	or	$8,$8,$1
	srlv	$1,$10,$24
	sllv	$10,$10,$25
	or	$9,$9,$1
	srlv	$1,$11,$24
	sllv	$11,$11,$25
	or	$10,$10,$1
	srlv	$2,$2,$24
	or	$11,$11,$2
# else
	srlv	$8,$8,$25
	sllv	$1,$9,$24
	srlv	$9,$9,$25
	or	$8,$8,$1
	sllv	$1,$10,$24
	srlv	$10,$10,$25
	or	$9,$9,$1
	sllv	$1,$11,$24
	srlv	$11,$11,$25
	or	$10,$10,$1
	sllv	$2,$2,$24
	or	$11,$11,$2
# endif
.Laligned_inp:
#else
	lwl	$8,0+MSB($5)		# load input
	lwl	$9,4+MSB($5)
	lwl	$10,8+MSB($5)
	lwl	$11,12+MSB($5)
	lwr	$8,0+LSB($5)
	lwr	$9,4+LSB($5)
	lwr	$10,8+LSB($5)
	lwr	$11,12+LSB($5)
#endif
#ifdef	MIPSEB
# if defined(_MIPS_ARCH_MIPS32R2)
	wsbh	$8,$8			# byte swap
	wsbh	$9,$9
	wsbh	$10,$10
	wsbh	$11,$11
	rotr	$8,$8,16
	rotr	$9,$9,16
	rotr	$10,$10,16
	rotr	$11,$11,16
# else
	srl	$1,$8,24		# byte swap
	srl	$2,$8,8
	andi	$24,$8,0xFF00
	sll	$8,$8,24
	andi	$2,0xFF00
	sll	$24,$24,8
	or	$8,$1
	 srl	$1,$9,24
	or	$2,$24
	 srl	$24,$9,8
	or	$8,$2
	 andi	$2,$9,0xFF00
	 sll	$9,$9,24
	 andi	$24,0xFF00
	 sll	$2,$2,8
	 or	$9,$1
	srl	$1,$10,24
	 or	$24,$2
	srl	$2,$10,8
	 or	$9,$24
	andi	$24,$10,0xFF00
	sll	$10,$10,24
	andi	$2,0xFF00
	sll	$24,$24,8
	or	$10,$1
	 srl	$1,$11,24
	or	$2,$24
	 srl	$24,$11,8
	or	$10,$2
	 andi	$2,$11,0xFF00
	 sll	$11,$11,24
	 andi	$24,0xFF00
	 sll	$2,$2,8
	 or	$11,$1
	 or	$24,$2
	 or	$11,$24
# endif
#endif
	srl	$2,$16,2		# modulo-scheduled reduction
	andi	$16,$16,3
	sll	$1,$2,2

	addu	$8,$8,$12		# accumulate input
	 addu	$2,$2,$1
	sltu	$12,$8,$12
	addu	$8,$8,$2		# ... and residue
	sltu	$1,$8,$2

	addu	$9,$9,$13
	 addu	$12,$12,$1		# carry
	sltu	$13,$9,$13
	addu	$9,$9,$12
	sltu	$12,$9,$12

	addu	$10,$10,$14
	 addu	$13,$13,$12		# carry
	sltu	$14,$10,$14
	addu	$10,$10,$13
	sltu	$13,$10,$13

	addu	$11,$11,$15
	 addu	$14,$14,$13		# carry
	sltu	$15,$11,$15
	addu	$11,$11,$14

#if defined(_MIPS_ARCH_MIPS32R2) && !defined(_MIPS_ARCH_MIPS32R6)
	multu	$17,$8			# d0*r0
	 sltu	$14,$11,$14
	maddu	$23,$9		# d1*s3
	 addu	$15,$15,$14		# carry
	maddu	$22,$10		# d2*s2
	 addu	$16,$16,$7
	maddu	$21,$11		# d3*s1
	 addu	$16,$16,$15
	mfhi	$1
	mflo	$12

	multu	$18,$8			# d0*r1
	maddu	$17,$9			# d1*r0
	maddu	$23,$10		# d2*s3
	maddu	$22,$11		# d3*s2
	maddu	$21,$16		# h4*s1
	maddu	$1,$25		# hi*1
	mfhi	$1
	mflo	$13

	multu	$19,$8			# d0*r2
	maddu	$18,$9			# d1*r1
	maddu	$17,$10			# d2*r0
	maddu	$23,$11		# d3*s3
	maddu	$22,$16		# h4*s2
	maddu	$1,$25		# hi*1
	mfhi	$1
	mflo	$14

	mul	$2,$17,$16		# h4*r0

	multu	$20,$8			# d0*r3
	maddu	$19,$9			# d1*r2
	maddu	$18,$10			# d2*r1
	maddu	$17,$11			# d3*r0
	maddu	$23,$16		# h4*s3
	maddu	$1,$25		# hi*1
	mfhi	$1
	mflo	$15

	 addiu	$5,$5,16

	addu	$16,$2,$1
#else
	multu	($17,$8)		# d0*r0
	mflo	($12,$17,$8)
	mfhi	($13,$17,$8)

	 sltu	$14,$11,$14
	 addu	$15,$15,$14		# carry

	multu	($23,$9)		# d1*s3
	mflo	($1,$23,$9)
	mfhi	($2,$23,$9)

	 addu	$16,$16,$7
	 addiu	$5,$5,16
	 addu	$16,$16,$15

	multu	($22,$10)		# d2*s2
	mflo	($7,$22,$10)
	mfhi	($24,$22,$10)
	 addu	$12,$12,$1
	 addu	$13,$13,$2
	multu	($21,$11)		# d3*s1
	 sltu	$1,$12,$1
	 addu	$13,$13,$1

	mflo	($1,$21,$11)
	mfhi	($2,$21,$11)
	 addu	$12,$12,$7
	 addu	$13,$13,$24
	multu	($18,$8)		# d0*r1
	 sltu	$7,$12,$7
	 addu	$13,$13,$7


	mflo	($7,$18,$8)
	mfhi	($14,$18,$8)
	 addu	$12,$12,$1
	 addu	$13,$13,$2
	multu	($17,$9)		# d1*r0
	 sltu	$1,$12,$1
	 addu	$13,$13,$1

	mflo	($1,$17,$9)
	mfhi	($2,$17,$9)
	 addu	$13,$13,$7
	 sltu	$7,$13,$7
	multu	($23,$10)		# d2*s3
	 addu	$14,$14,$7

	mflo	($7,$23,$10)
	mfhi	($24,$23,$10)
	 addu	$13,$13,$1
	 addu	$14,$14,$2
	multu	($22,$11)		# d3*s2
	 sltu	$1,$13,$1
	 addu	$14,$14,$1

	mflo	($1,$22,$11)
	mfhi	($2,$22,$11)
	 addu	$13,$13,$7
	 addu	$14,$14,$24
	multu	($21,$16)		# h4*s1
	 sltu	$7,$13,$7
	 addu	$14,$14,$7

	mflo	($7,$21,$16)
	 addu	$13,$13,$1
	 addu	$14,$14,$2
	multu	($19,$8)		# d0*r2
	 sltu	$1,$13,$1
	 addu	$14,$14,$1


	mflo	($1,$19,$8)
	mfhi	($15,$19,$8)
	 addu	$13,$13,$7
	 sltu	$7,$13,$7
	multu	($18,$9)		# d1*r1
	 addu	$14,$14,$7

	mflo	($7,$18,$9)
	mfhi	($24,$18,$9)
	 addu	$14,$14,$1
	 sltu	$1,$14,$1
	multu	($17,$10)		# d2*r0
	 addu	$15,$15,$1

	mflo	($1,$17,$10)
	mfhi	($2,$17,$10)
	 addu	$14,$14,$7
	 addu	$15,$15,$24
	multu	($23,$11)		# d3*s3
	 sltu	$7,$14,$7
	 addu	$15,$15,$7

	mflo	($7,$23,$11)
	mfhi	($24,$23,$11)
	 addu	$14,$14,$1
	 addu	$15,$15,$2
	multu	($22,$16)		# h4*s2
	 sltu	$1,$14,$1
	 addu	$15,$15,$1

	mflo	($1,$22,$16)
	 addu	$14,$14,$7
	 addu	$15,$15,$24
	multu	($20,$8)		# d0*r3
	 sltu	$7,$14,$7
	 addu	$15,$15,$7


	mflo	($7,$20,$8)
	mfhi	($24,$20,$8)
	 addu	$14,$14,$1
	 sltu	$1,$14,$1
	multu	($19,$9)		# d1*r2
	 addu	$15,$15,$1

	mflo	($1,$19,$9)
	mfhi	($2,$19,$9)
	 addu	$15,$15,$7
	 sltu	$7,$15,$7
	multu	($17,$11)		# d3*r0
	 addu	$24,$24,$7

	mflo	($7,$17,$11)
	mfhi	($11,$17,$11)
	 addu	$15,$15,$1
	 addu	$24,$24,$2
	multu	($18,$10)		# d2*r1
	 sltu	$1,$15,$1
	 addu	$24,$24,$1

	mflo	($1,$18,$10)
	mfhi	($2,$18,$10)
	 addu	$15,$15,$7
	 addu	$24,$24,$11
	multu	($23,$16)		# h4*s3
	 sltu	$7,$15,$7
	 addu	$24,$24,$7

	mflo	($7,$23,$16)
	 addu	$15,$15,$1
	 addu	$24,$24,$2
	multu	($17,$16)		# h4*r0
	 sltu	$1,$15,$1
	 addu	$24,$24,$1


	mflo	($16,$17,$16)
	 addu	$15,$15,$7
	 sltu	$7,$15,$7
	 addu	$24,$24,$7
	addu	$16,$16,$24

	li	$7,1		# if we loop, padbit is 1
#endif
	bne	$5,$6,.Loop

	sw	$12,0($4)		# store hash value
	sw	$13,4($4)
	sw	$14,8($4)
	sw	$15,12($4)
	sw	$16,16($4)

	.set	noreorder
.Labort:
	lw	$23,4*11($29)
	lw	$22,4*10($29)
	lw	$21, 4*9($29)
	lw	$20, 4*8($29)
	lw	$19, 4*7($29)
	lw	$18, 4*6($29)
	lw	$17, 4*5($29)
	lw	$16, 4*4($29)
	jr	$31
	addu	$29,$29,4*12
.end	poly1305_blocks
.align	5
.globl	poly1305_emit
.ent	poly1305_emit
poly1305_emit:
	.frame	$29,0,$31
	.set	reorder

	lw	$7,16($4)
	lw	$1,0($4)
	lw	$2,4($4)
	lw	$24,8($4)
	lw	$25,12($4)

	li	$8,-4			# final reduction
	srl	$4,$7,2
	and	$8,$8,$7
	andi	$7,$7,3
	addu	$4,$4,$8

	addu	$1,$1,$4
	sltu	$4,$1,$4
	 addiu	$8,$1,5		# compare to modulus
	addu	$2,$2,$4
	 sltiu	$9,$8,5
	sltu	$4,$2,$4
	 addu	$9,$9,$2
	addu	$24,$24,$4
	 sltu	$10,$9,$2
	sltu	$4,$24,$4
	 addu	$10,$10,$24
	addu	$25,$25,$4
	 sltu	$11,$10,$24
	sltu	$4,$25,$4
	 addu	$11,$11,$25
	addu	$7,$7,$4
	 sltu	$4,$11,$25
	 addu	$4,$7

	srl	$4,2			# see if it carried/borrowed
	subu	$4,$0,$4

	xor	$8,$1
	xor	$9,$2
	xor	$10,$24
	xor	$11,$25
	and	$8,$4
	and	$9,$4
	and	$10,$4
	and	$11,$4
	xor	$8,$1
	xor	$9,$2
	xor	$10,$24
	xor	$11,$25

	lw	$1,0($6)		# load nonce
	lw	$2,4($6)
	lw	$24,8($6)
	lw	$25,12($6)

	addu	$8,$1		# accumulate nonce
	sltu	$4,$8,$1

	addu	$9,$2
	sltu	$2,$9,$2
	addu	$9,$4
	sltu	$4,$9,$4
	addu	$4,$2

	addu	$10,$24
	sltu	$24,$10,$24
	addu	$10,$4
	sltu	$4,$10,$4
	addu	$4,$24

	addu	$11,$25
	addu	$11,$4

	srl	$1,$8,8		# write mac value
	srl	$2,$8,16
	srl	$24,$8,24
	sb	$8, 0($5)
	sb	$1,1($5)
	srl	$1,$9,8
	sb	$2,2($5)
	srl	$2,$9,16
	sb	$24,3($5)
	srl	$24,$9,24
	sb	$9, 4($5)
	sb	$1,5($5)
	srl	$1,$10,8
	sb	$2,6($5)
	srl	$2,$10,16
	sb	$24,7($5)
	srl	$24,$10,24
	sb	$10, 8($5)
	sb	$1,9($5)
	srl	$1,$11,8
	sb	$2,10($5)
	srl	$2,$11,16
	sb	$24,11($5)
	srl	$24,$11,24
	sb	$11, 12($5)
	sb	$1,13($5)
	sb	$2,14($5)
	sb	$24,15($5)

	jr	$31
.end	poly1305_emit
.rdata
.asciiz	"Poly1305 for MIPS32, CRYPTOGAMS by @dot-asm"
.align	2
