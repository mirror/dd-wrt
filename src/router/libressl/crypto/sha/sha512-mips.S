.text
.set	noat
#if !defined(__vxworks) || defined(__pic__)
.option	pic2
#endif

.align	5
.globl	sha512_block_data_order
.ent	sha512_block_data_order
sha512_block_data_order:
	.frame	$29,192,$31
	.mask	3237937152,-4
	.set	noreorder
	.cpload	$25
	sub $29,192
	sw	$31,192-1*4($29)
	sw	$30,192-2*4($29)
	sw	$23,192-3*4($29)
	sw	$22,192-4*4($29)
	sw	$21,192-5*4($29)
	sw	$20,192-6*4($29)
	sw	$19,192-7*4($29)
	sw	$18,192-8*4($29)
	sw	$17,192-9*4($29)
	sw	$16,192-10*4($29)
	sll $23,$6,7
	.set	reorder
	la	$6,K512		# PIC-ified 'load address'

	ld	$1,0*8($4)		# load context
	ld	$2,1*8($4)
	ld	$3,2*8($4)
	ld	$7,3*8($4)
	ld	$24,4*8($4)
	ld	$25,5*8($4)
	ld	$30,6*8($4)
	ld	$31,7*8($4)

	add $23,$5		# pointer to the end of input
	sw	$23,16*8($29)
	b	.Loop

.align	5
.Loop:
	ldl	$8,7($5)
	ldr	$8,0($5)
	ldl	$9,15($5)
	ldr	$9,8($5)
	ori	$13,$0,0xFF
	dsll	$15,$13,32
	or	$13,$15		# 0x000000FF000000FF
	and	$14,$8,$13	# byte swap(0)
	dsrl	$15,$8,24
	dsll	$14,24
	and	$15,$13
	dsll	$13,8			# 0x0000FF000000FF00
	or	$14,$15
	and	$15,$8,$13
	dsrl	$8,8
	dsll	$15,8
	and	$8,$13
	or	$14,$15
	or	$8,$14
	dsrl	$14,$8,32
	dsll	$8,32
	or	$8,$14
	daddu	$12,$8,$31			# 0
	dsrl	$31,$24,14
	xor	$15,$25,$30
	dsll	$14,$24,23
	and	$15,$24
	dsrl	$13,$24,18
	xor	$31,$14
	dsll	$14,$24,46
	xor	$31,$13
	dsrl	$13,$24,41
	xor	$31,$14
	dsll	$14,$24,50
	xor	$31,$13
	xor	$15,$30			# Ch(e,f,g)
	xor	$13,$14,$31			# Sigma1(e)

	dsrl	$31,$1,28
	daddu	$12,$15
	ld	$15,0($6)		# K[0]
	dsll	$14,$1,25
	daddu	$12,$13
	dsrl	$13,$1,34
	xor	$31,$14
	dsll	$14,$1,30
	xor	$31,$13
	dsrl	$13,$1,39
	xor	$31,$14
	dsll	$14,$1,36
	xor	$31,$13
	sd	$8,0($29)	# offload to ring buffer
	xor	$31,$14			# Sigma0(a)

	or	$13,$1,$2
	and	$14,$1,$2
	and	$13,$3
	or	$14,$13			# Maj(a,b,c)
	daddu	$12,$15			# +=K[0]
	daddu	$31,$14

	daddu	$7,$12
	daddu	$31,$12
	ldl	$10,23($5)
	ldr	$10,16($5)
	ori	$14,$0,0xFF
	dsll	$16,$14,32
	or	$14,$16		# 0x000000FF000000FF
	and	$15,$9,$14	# byte swap(1)
	dsrl	$16,$9,24
	dsll	$15,24
	and	$16,$14
	dsll	$14,8			# 0x0000FF000000FF00
	or	$15,$16
	and	$16,$9,$14
	dsrl	$9,8
	dsll	$16,8
	and	$9,$14
	or	$15,$16
	or	$9,$15
	dsrl	$15,$9,32
	dsll	$9,32
	or	$9,$15
	daddu	$13,$9,$30			# 1
	dsrl	$30,$7,14
	xor	$16,$24,$25
	dsll	$15,$7,23
	and	$16,$7
	dsrl	$14,$7,18
	xor	$30,$15
	dsll	$15,$7,46
	xor	$30,$14
	dsrl	$14,$7,41
	xor	$30,$15
	dsll	$15,$7,50
	xor	$30,$14
	xor	$16,$25			# Ch(e,f,g)
	xor	$14,$15,$30			# Sigma1(e)

	dsrl	$30,$31,28
	daddu	$13,$16
	ld	$16,8($6)		# K[1]
	dsll	$15,$31,25
	daddu	$13,$14
	dsrl	$14,$31,34
	xor	$30,$15
	dsll	$15,$31,30
	xor	$30,$14
	dsrl	$14,$31,39
	xor	$30,$15
	dsll	$15,$31,36
	xor	$30,$14
	sd	$9,8($29)	# offload to ring buffer
	xor	$30,$15			# Sigma0(a)

	or	$14,$31,$1
	and	$15,$31,$1
	and	$14,$2
	or	$15,$14			# Maj(a,b,c)
	daddu	$13,$16			# +=K[1]
	daddu	$30,$15

	daddu	$3,$13
	daddu	$30,$13
	ldl	$11,31($5)
	ldr	$11,24($5)
	ori	$15,$0,0xFF
	dsll	$17,$15,32
	or	$15,$17		# 0x000000FF000000FF
	and	$16,$10,$15	# byte swap(2)
	dsrl	$17,$10,24
	dsll	$16,24
	and	$17,$15
	dsll	$15,8			# 0x0000FF000000FF00
	or	$16,$17
	and	$17,$10,$15
	dsrl	$10,8
	dsll	$17,8
	and	$10,$15
	or	$16,$17
	or	$10,$16
	dsrl	$16,$10,32
	dsll	$10,32
	or	$10,$16
	daddu	$14,$10,$25			# 2
	dsrl	$25,$3,14
	xor	$17,$7,$24
	dsll	$16,$3,23
	and	$17,$3
	dsrl	$15,$3,18
	xor	$25,$16
	dsll	$16,$3,46
	xor	$25,$15
	dsrl	$15,$3,41
	xor	$25,$16
	dsll	$16,$3,50
	xor	$25,$15
	xor	$17,$24			# Ch(e,f,g)
	xor	$15,$16,$25			# Sigma1(e)

	dsrl	$25,$30,28
	daddu	$14,$17
	ld	$17,16($6)		# K[2]
	dsll	$16,$30,25
	daddu	$14,$15
	dsrl	$15,$30,34
	xor	$25,$16
	dsll	$16,$30,30
	xor	$25,$15
	dsrl	$15,$30,39
	xor	$25,$16
	dsll	$16,$30,36
	xor	$25,$15
	sd	$10,16($29)	# offload to ring buffer
	xor	$25,$16			# Sigma0(a)

	or	$15,$30,$31
	and	$16,$30,$31
	and	$15,$1
	or	$16,$15			# Maj(a,b,c)
	daddu	$14,$17			# +=K[2]
	daddu	$25,$16

	daddu	$2,$14
	daddu	$25,$14
	ldl	$12,39($5)
	ldr	$12,32($5)
	ori	$16,$0,0xFF
	dsll	$18,$16,32
	or	$16,$18		# 0x000000FF000000FF
	and	$17,$11,$16	# byte swap(3)
	dsrl	$18,$11,24
	dsll	$17,24
	and	$18,$16
	dsll	$16,8			# 0x0000FF000000FF00
	or	$17,$18
	and	$18,$11,$16
	dsrl	$11,8
	dsll	$18,8
	and	$11,$16
	or	$17,$18
	or	$11,$17
	dsrl	$17,$11,32
	dsll	$11,32
	or	$11,$17
	daddu	$15,$11,$24			# 3
	dsrl	$24,$2,14
	xor	$18,$3,$7
	dsll	$17,$2,23
	and	$18,$2
	dsrl	$16,$2,18
	xor	$24,$17
	dsll	$17,$2,46
	xor	$24,$16
	dsrl	$16,$2,41
	xor	$24,$17
	dsll	$17,$2,50
	xor	$24,$16
	xor	$18,$7			# Ch(e,f,g)
	xor	$16,$17,$24			# Sigma1(e)

	dsrl	$24,$25,28
	daddu	$15,$18
	ld	$18,24($6)		# K[3]
	dsll	$17,$25,25
	daddu	$15,$16
	dsrl	$16,$25,34
	xor	$24,$17
	dsll	$17,$25,30
	xor	$24,$16
	dsrl	$16,$25,39
	xor	$24,$17
	dsll	$17,$25,36
	xor	$24,$16
	sd	$11,24($29)	# offload to ring buffer
	xor	$24,$17			# Sigma0(a)

	or	$16,$25,$30
	and	$17,$25,$30
	and	$16,$31
	or	$17,$16			# Maj(a,b,c)
	daddu	$15,$18			# +=K[3]
	daddu	$24,$17

	daddu	$1,$15
	daddu	$24,$15
	ldl	$13,47($5)
	ldr	$13,40($5)
	ori	$17,$0,0xFF
	dsll	$19,$17,32
	or	$17,$19		# 0x000000FF000000FF
	and	$18,$12,$17	# byte swap(4)
	dsrl	$19,$12,24
	dsll	$18,24
	and	$19,$17
	dsll	$17,8			# 0x0000FF000000FF00
	or	$18,$19
	and	$19,$12,$17
	dsrl	$12,8
	dsll	$19,8
	and	$12,$17
	or	$18,$19
	or	$12,$18
	dsrl	$18,$12,32
	dsll	$12,32
	or	$12,$18
	daddu	$16,$12,$7			# 4
	dsrl	$7,$1,14
	xor	$19,$2,$3
	dsll	$18,$1,23
	and	$19,$1
	dsrl	$17,$1,18
	xor	$7,$18
	dsll	$18,$1,46
	xor	$7,$17
	dsrl	$17,$1,41
	xor	$7,$18
	dsll	$18,$1,50
	xor	$7,$17
	xor	$19,$3			# Ch(e,f,g)
	xor	$17,$18,$7			# Sigma1(e)

	dsrl	$7,$24,28
	daddu	$16,$19
	ld	$19,32($6)		# K[4]
	dsll	$18,$24,25
	daddu	$16,$17
	dsrl	$17,$24,34
	xor	$7,$18
	dsll	$18,$24,30
	xor	$7,$17
	dsrl	$17,$24,39
	xor	$7,$18
	dsll	$18,$24,36
	xor	$7,$17
	sd	$12,32($29)	# offload to ring buffer
	xor	$7,$18			# Sigma0(a)

	or	$17,$24,$25
	and	$18,$24,$25
	and	$17,$30
	or	$18,$17			# Maj(a,b,c)
	daddu	$16,$19			# +=K[4]
	daddu	$7,$18

	daddu	$31,$16
	daddu	$7,$16
	ldl	$14,55($5)
	ldr	$14,48($5)
	ori	$18,$0,0xFF
	dsll	$20,$18,32
	or	$18,$20		# 0x000000FF000000FF
	and	$19,$13,$18	# byte swap(5)
	dsrl	$20,$13,24
	dsll	$19,24
	and	$20,$18
	dsll	$18,8			# 0x0000FF000000FF00
	or	$19,$20
	and	$20,$13,$18
	dsrl	$13,8
	dsll	$20,8
	and	$13,$18
	or	$19,$20
	or	$13,$19
	dsrl	$19,$13,32
	dsll	$13,32
	or	$13,$19
	daddu	$17,$13,$3			# 5
	dsrl	$3,$31,14
	xor	$20,$1,$2
	dsll	$19,$31,23
	and	$20,$31
	dsrl	$18,$31,18
	xor	$3,$19
	dsll	$19,$31,46
	xor	$3,$18
	dsrl	$18,$31,41
	xor	$3,$19
	dsll	$19,$31,50
	xor	$3,$18
	xor	$20,$2			# Ch(e,f,g)
	xor	$18,$19,$3			# Sigma1(e)

	dsrl	$3,$7,28
	daddu	$17,$20
	ld	$20,40($6)		# K[5]
	dsll	$19,$7,25
	daddu	$17,$18
	dsrl	$18,$7,34
	xor	$3,$19
	dsll	$19,$7,30
	xor	$3,$18
	dsrl	$18,$7,39
	xor	$3,$19
	dsll	$19,$7,36
	xor	$3,$18
	sd	$13,40($29)	# offload to ring buffer
	xor	$3,$19			# Sigma0(a)

	or	$18,$7,$24
	and	$19,$7,$24
	and	$18,$25
	or	$19,$18			# Maj(a,b,c)
	daddu	$17,$20			# +=K[5]
	daddu	$3,$19

	daddu	$30,$17
	daddu	$3,$17
	ldl	$15,63($5)
	ldr	$15,56($5)
	ori	$19,$0,0xFF
	dsll	$21,$19,32
	or	$19,$21		# 0x000000FF000000FF
	and	$20,$14,$19	# byte swap(6)
	dsrl	$21,$14,24
	dsll	$20,24
	and	$21,$19
	dsll	$19,8			# 0x0000FF000000FF00
	or	$20,$21
	and	$21,$14,$19
	dsrl	$14,8
	dsll	$21,8
	and	$14,$19
	or	$20,$21
	or	$14,$20
	dsrl	$20,$14,32
	dsll	$14,32
	or	$14,$20
	daddu	$18,$14,$2			# 6
	dsrl	$2,$30,14
	xor	$21,$31,$1
	dsll	$20,$30,23
	and	$21,$30
	dsrl	$19,$30,18
	xor	$2,$20
	dsll	$20,$30,46
	xor	$2,$19
	dsrl	$19,$30,41
	xor	$2,$20
	dsll	$20,$30,50
	xor	$2,$19
	xor	$21,$1			# Ch(e,f,g)
	xor	$19,$20,$2			# Sigma1(e)

	dsrl	$2,$3,28
	daddu	$18,$21
	ld	$21,48($6)		# K[6]
	dsll	$20,$3,25
	daddu	$18,$19
	dsrl	$19,$3,34
	xor	$2,$20
	dsll	$20,$3,30
	xor	$2,$19
	dsrl	$19,$3,39
	xor	$2,$20
	dsll	$20,$3,36
	xor	$2,$19
	sd	$14,48($29)	# offload to ring buffer
	xor	$2,$20			# Sigma0(a)

	or	$19,$3,$7
	and	$20,$3,$7
	and	$19,$24
	or	$20,$19			# Maj(a,b,c)
	daddu	$18,$21			# +=K[6]
	daddu	$2,$20

	daddu	$25,$18
	daddu	$2,$18
	ldl	$16,71($5)
	ldr	$16,64($5)
	ori	$20,$0,0xFF
	dsll	$22,$20,32
	or	$20,$22		# 0x000000FF000000FF
	and	$21,$15,$20	# byte swap(7)
	dsrl	$22,$15,24
	dsll	$21,24
	and	$22,$20
	dsll	$20,8			# 0x0000FF000000FF00
	or	$21,$22
	and	$22,$15,$20
	dsrl	$15,8
	dsll	$22,8
	and	$15,$20
	or	$21,$22
	or	$15,$21
	dsrl	$21,$15,32
	dsll	$15,32
	or	$15,$21
	daddu	$19,$15,$1			# 7
	dsrl	$1,$25,14
	xor	$22,$30,$31
	dsll	$21,$25,23
	and	$22,$25
	dsrl	$20,$25,18
	xor	$1,$21
	dsll	$21,$25,46
	xor	$1,$20
	dsrl	$20,$25,41
	xor	$1,$21
	dsll	$21,$25,50
	xor	$1,$20
	xor	$22,$31			# Ch(e,f,g)
	xor	$20,$21,$1			# Sigma1(e)

	dsrl	$1,$2,28
	daddu	$19,$22
	ld	$22,56($6)		# K[7]
	dsll	$21,$2,25
	daddu	$19,$20
	dsrl	$20,$2,34
	xor	$1,$21
	dsll	$21,$2,30
	xor	$1,$20
	dsrl	$20,$2,39
	xor	$1,$21
	dsll	$21,$2,36
	xor	$1,$20
	sd	$15,56($29)	# offload to ring buffer
	xor	$1,$21			# Sigma0(a)

	or	$20,$2,$3
	and	$21,$2,$3
	and	$20,$7
	or	$21,$20			# Maj(a,b,c)
	daddu	$19,$22			# +=K[7]
	daddu	$1,$21

	daddu	$24,$19
	daddu	$1,$19
	ldl	$17,79($5)
	ldr	$17,72($5)
	ori	$21,$0,0xFF
	dsll	$23,$21,32
	or	$21,$23		# 0x000000FF000000FF
	and	$22,$16,$21	# byte swap(8)
	dsrl	$23,$16,24
	dsll	$22,24
	and	$23,$21
	dsll	$21,8			# 0x0000FF000000FF00
	or	$22,$23
	and	$23,$16,$21
	dsrl	$16,8
	dsll	$23,8
	and	$16,$21
	or	$22,$23
	or	$16,$22
	dsrl	$22,$16,32
	dsll	$16,32
	or	$16,$22
	daddu	$20,$16,$31			# 8
	dsrl	$31,$24,14
	xor	$23,$25,$30
	dsll	$22,$24,23
	and	$23,$24
	dsrl	$21,$24,18
	xor	$31,$22
	dsll	$22,$24,46
	xor	$31,$21
	dsrl	$21,$24,41
	xor	$31,$22
	dsll	$22,$24,50
	xor	$31,$21
	xor	$23,$30			# Ch(e,f,g)
	xor	$21,$22,$31			# Sigma1(e)

	dsrl	$31,$1,28
	daddu	$20,$23
	ld	$23,64($6)		# K[8]
	dsll	$22,$1,25
	daddu	$20,$21
	dsrl	$21,$1,34
	xor	$31,$22
	dsll	$22,$1,30
	xor	$31,$21
	dsrl	$21,$1,39
	xor	$31,$22
	dsll	$22,$1,36
	xor	$31,$21
	sd	$16,64($29)	# offload to ring buffer
	xor	$31,$22			# Sigma0(a)

	or	$21,$1,$2
	and	$22,$1,$2
	and	$21,$3
	or	$22,$21			# Maj(a,b,c)
	daddu	$20,$23			# +=K[8]
	daddu	$31,$22

	daddu	$7,$20
	daddu	$31,$20
	ldl	$18,87($5)
	ldr	$18,80($5)
	ori	$22,$0,0xFF
	dsll	$8,$22,32
	or	$22,$8		# 0x000000FF000000FF
	and	$23,$17,$22	# byte swap(9)
	dsrl	$8,$17,24
	dsll	$23,24
	and	$8,$22
	dsll	$22,8			# 0x0000FF000000FF00
	or	$23,$8
	and	$8,$17,$22
	dsrl	$17,8
	dsll	$8,8
	and	$17,$22
	or	$23,$8
	or	$17,$23
	dsrl	$23,$17,32
	dsll	$17,32
	or	$17,$23
	daddu	$21,$17,$30			# 9
	dsrl	$30,$7,14
	xor	$8,$24,$25
	dsll	$23,$7,23
	and	$8,$7
	dsrl	$22,$7,18
	xor	$30,$23
	dsll	$23,$7,46
	xor	$30,$22
	dsrl	$22,$7,41
	xor	$30,$23
	dsll	$23,$7,50
	xor	$30,$22
	xor	$8,$25			# Ch(e,f,g)
	xor	$22,$23,$30			# Sigma1(e)

	dsrl	$30,$31,28
	daddu	$21,$8
	ld	$8,72($6)		# K[9]
	dsll	$23,$31,25
	daddu	$21,$22
	dsrl	$22,$31,34
	xor	$30,$23
	dsll	$23,$31,30
	xor	$30,$22
	dsrl	$22,$31,39
	xor	$30,$23
	dsll	$23,$31,36
	xor	$30,$22
	sd	$17,72($29)	# offload to ring buffer
	xor	$30,$23			# Sigma0(a)

	or	$22,$31,$1
	and	$23,$31,$1
	and	$22,$2
	or	$23,$22			# Maj(a,b,c)
	daddu	$21,$8			# +=K[9]
	daddu	$30,$23

	daddu	$3,$21
	daddu	$30,$21
	ldl	$19,95($5)
	ldr	$19,88($5)
	ori	$23,$0,0xFF
	dsll	$9,$23,32
	or	$23,$9		# 0x000000FF000000FF
	and	$8,$18,$23	# byte swap(10)
	dsrl	$9,$18,24
	dsll	$8,24
	and	$9,$23
	dsll	$23,8			# 0x0000FF000000FF00
	or	$8,$9
	and	$9,$18,$23
	dsrl	$18,8
	dsll	$9,8
	and	$18,$23
	or	$8,$9
	or	$18,$8
	dsrl	$8,$18,32
	dsll	$18,32
	or	$18,$8
	daddu	$22,$18,$25			# 10
	dsrl	$25,$3,14
	xor	$9,$7,$24
	dsll	$8,$3,23
	and	$9,$3
	dsrl	$23,$3,18
	xor	$25,$8
	dsll	$8,$3,46
	xor	$25,$23
	dsrl	$23,$3,41
	xor	$25,$8
	dsll	$8,$3,50
	xor	$25,$23
	xor	$9,$24			# Ch(e,f,g)
	xor	$23,$8,$25			# Sigma1(e)

	dsrl	$25,$30,28
	daddu	$22,$9
	ld	$9,80($6)		# K[10]
	dsll	$8,$30,25
	daddu	$22,$23
	dsrl	$23,$30,34
	xor	$25,$8
	dsll	$8,$30,30
	xor	$25,$23
	dsrl	$23,$30,39
	xor	$25,$8
	dsll	$8,$30,36
	xor	$25,$23
	sd	$18,80($29)	# offload to ring buffer
	xor	$25,$8			# Sigma0(a)

	or	$23,$30,$31
	and	$8,$30,$31
	and	$23,$1
	or	$8,$23			# Maj(a,b,c)
	daddu	$22,$9			# +=K[10]
	daddu	$25,$8

	daddu	$2,$22
	daddu	$25,$22
	ldl	$20,103($5)
	ldr	$20,96($5)
	ori	$8,$0,0xFF
	dsll	$10,$8,32
	or	$8,$10		# 0x000000FF000000FF
	and	$9,$19,$8	# byte swap(11)
	dsrl	$10,$19,24
	dsll	$9,24
	and	$10,$8
	dsll	$8,8			# 0x0000FF000000FF00
	or	$9,$10
	and	$10,$19,$8
	dsrl	$19,8
	dsll	$10,8
	and	$19,$8
	or	$9,$10
	or	$19,$9
	dsrl	$9,$19,32
	dsll	$19,32
	or	$19,$9
	daddu	$23,$19,$24			# 11
	dsrl	$24,$2,14
	xor	$10,$3,$7
	dsll	$9,$2,23
	and	$10,$2
	dsrl	$8,$2,18
	xor	$24,$9
	dsll	$9,$2,46
	xor	$24,$8
	dsrl	$8,$2,41
	xor	$24,$9
	dsll	$9,$2,50
	xor	$24,$8
	xor	$10,$7			# Ch(e,f,g)
	xor	$8,$9,$24			# Sigma1(e)

	dsrl	$24,$25,28
	daddu	$23,$10
	ld	$10,88($6)		# K[11]
	dsll	$9,$25,25
	daddu	$23,$8
	dsrl	$8,$25,34
	xor	$24,$9
	dsll	$9,$25,30
	xor	$24,$8
	dsrl	$8,$25,39
	xor	$24,$9
	dsll	$9,$25,36
	xor	$24,$8
	sd	$19,88($29)	# offload to ring buffer
	xor	$24,$9			# Sigma0(a)

	or	$8,$25,$30
	and	$9,$25,$30
	and	$8,$31
	or	$9,$8			# Maj(a,b,c)
	daddu	$23,$10			# +=K[11]
	daddu	$24,$9

	daddu	$1,$23
	daddu	$24,$23
	ldl	$21,111($5)
	ldr	$21,104($5)
	ori	$9,$0,0xFF
	dsll	$11,$9,32
	or	$9,$11		# 0x000000FF000000FF
	and	$10,$20,$9	# byte swap(12)
	dsrl	$11,$20,24
	dsll	$10,24
	and	$11,$9
	dsll	$9,8			# 0x0000FF000000FF00
	or	$10,$11
	and	$11,$20,$9
	dsrl	$20,8
	dsll	$11,8
	and	$20,$9
	or	$10,$11
	or	$20,$10
	dsrl	$10,$20,32
	dsll	$20,32
	or	$20,$10
	daddu	$8,$20,$7			# 12
	dsrl	$7,$1,14
	xor	$11,$2,$3
	dsll	$10,$1,23
	and	$11,$1
	dsrl	$9,$1,18
	xor	$7,$10
	dsll	$10,$1,46
	xor	$7,$9
	dsrl	$9,$1,41
	xor	$7,$10
	dsll	$10,$1,50
	xor	$7,$9
	xor	$11,$3			# Ch(e,f,g)
	xor	$9,$10,$7			# Sigma1(e)

	dsrl	$7,$24,28
	daddu	$8,$11
	ld	$11,96($6)		# K[12]
	dsll	$10,$24,25
	daddu	$8,$9
	dsrl	$9,$24,34
	xor	$7,$10
	dsll	$10,$24,30
	xor	$7,$9
	dsrl	$9,$24,39
	xor	$7,$10
	dsll	$10,$24,36
	xor	$7,$9
	sd	$20,96($29)	# offload to ring buffer
	xor	$7,$10			# Sigma0(a)

	or	$9,$24,$25
	and	$10,$24,$25
	and	$9,$30
	or	$10,$9			# Maj(a,b,c)
	daddu	$8,$11			# +=K[12]
	daddu	$7,$10

	daddu	$31,$8
	daddu	$7,$8
	ldl	$22,119($5)
	ldr	$22,112($5)
	ori	$10,$0,0xFF
	dsll	$12,$10,32
	or	$10,$12		# 0x000000FF000000FF
	and	$11,$21,$10	# byte swap(13)
	dsrl	$12,$21,24
	dsll	$11,24
	and	$12,$10
	dsll	$10,8			# 0x0000FF000000FF00
	or	$11,$12
	and	$12,$21,$10
	dsrl	$21,8
	dsll	$12,8
	and	$21,$10
	or	$11,$12
	or	$21,$11
	dsrl	$11,$21,32
	dsll	$21,32
	or	$21,$11
	daddu	$9,$21,$3			# 13
	dsrl	$3,$31,14
	xor	$12,$1,$2
	dsll	$11,$31,23
	and	$12,$31
	dsrl	$10,$31,18
	xor	$3,$11
	dsll	$11,$31,46
	xor	$3,$10
	dsrl	$10,$31,41
	xor	$3,$11
	dsll	$11,$31,50
	xor	$3,$10
	xor	$12,$2			# Ch(e,f,g)
	xor	$10,$11,$3			# Sigma1(e)

	dsrl	$3,$7,28
	daddu	$9,$12
	ld	$12,104($6)		# K[13]
	dsll	$11,$7,25
	daddu	$9,$10
	dsrl	$10,$7,34
	xor	$3,$11
	dsll	$11,$7,30
	xor	$3,$10
	dsrl	$10,$7,39
	xor	$3,$11
	dsll	$11,$7,36
	xor	$3,$10
	sd	$21,104($29)	# offload to ring buffer
	xor	$3,$11			# Sigma0(a)

	or	$10,$7,$24
	and	$11,$7,$24
	and	$10,$25
	or	$11,$10			# Maj(a,b,c)
	daddu	$9,$12			# +=K[13]
	daddu	$3,$11

	daddu	$30,$9
	daddu	$3,$9
	ld	$8,0($29)	# prefetch from ring buffer
	ldl	$23,127($5)
	ldr	$23,120($5)
	ori	$11,$0,0xFF
	dsll	$13,$11,32
	or	$11,$13		# 0x000000FF000000FF
	and	$12,$22,$11	# byte swap(14)
	dsrl	$13,$22,24
	dsll	$12,24
	and	$13,$11
	dsll	$11,8			# 0x0000FF000000FF00
	or	$12,$13
	and	$13,$22,$11
	dsrl	$22,8
	dsll	$13,8
	and	$22,$11
	or	$12,$13
	or	$22,$12
	dsrl	$12,$22,32
	dsll	$22,32
	or	$22,$12
	daddu	$10,$22,$2			# 14
	dsrl	$2,$30,14
	xor	$13,$31,$1
	dsll	$12,$30,23
	and	$13,$30
	dsrl	$11,$30,18
	xor	$2,$12
	dsll	$12,$30,46
	xor	$2,$11
	dsrl	$11,$30,41
	xor	$2,$12
	dsll	$12,$30,50
	xor	$2,$11
	xor	$13,$1			# Ch(e,f,g)
	xor	$11,$12,$2			# Sigma1(e)

	dsrl	$2,$3,28
	daddu	$10,$13
	ld	$13,112($6)		# K[14]
	dsll	$12,$3,25
	daddu	$10,$11
	dsrl	$11,$3,34
	xor	$2,$12
	dsll	$12,$3,30
	xor	$2,$11
	dsrl	$11,$3,39
	xor	$2,$12
	dsll	$12,$3,36
	xor	$2,$11
	sd	$22,112($29)	# offload to ring buffer
	xor	$2,$12			# Sigma0(a)

	or	$11,$3,$7
	and	$12,$3,$7
	and	$11,$24
	or	$12,$11			# Maj(a,b,c)
	daddu	$10,$13			# +=K[14]
	daddu	$2,$12

	daddu	$25,$10
	daddu	$2,$10
	ld	$9,8($29)	# prefetch from ring buffer
	ori	$12,$0,0xFF
	dsll	$14,$12,32
	or	$12,$14		# 0x000000FF000000FF
	and	$13,$23,$12	# byte swap(15)
	dsrl	$14,$23,24
	dsll	$13,24
	and	$14,$12
	dsll	$12,8			# 0x0000FF000000FF00
	or	$13,$14
	and	$14,$23,$12
	dsrl	$23,8
	dsll	$14,8
	and	$23,$12
	or	$13,$14
	or	$23,$13
	dsrl	$13,$23,32
	dsll	$23,32
	or	$23,$13
	daddu	$11,$23,$1			# 15
	dsrl	$1,$25,14
	xor	$14,$30,$31
	dsll	$13,$25,23
	and	$14,$25
	dsrl	$12,$25,18
	xor	$1,$13
	dsll	$13,$25,46
	xor	$1,$12
	dsrl	$12,$25,41
	xor	$1,$13
	dsll	$13,$25,50
	xor	$1,$12
	xor	$14,$31			# Ch(e,f,g)
	xor	$12,$13,$1			# Sigma1(e)

	dsrl	$1,$2,28
	daddu	$11,$14
	ld	$14,120($6)		# K[15]
	dsll	$13,$2,25
	daddu	$11,$12
	dsrl	$12,$2,34
	xor	$1,$13
	dsll	$13,$2,30
	xor	$1,$12
	dsrl	$12,$2,39
	xor	$1,$13
	dsll	$13,$2,36
	xor	$1,$12
	sd	$23,120($29)	# offload to ring buffer
	xor	$1,$13			# Sigma0(a)

	or	$12,$2,$3
	and	$13,$2,$3
	and	$12,$7
	or	$13,$12			# Maj(a,b,c)
	daddu	$11,$14			# +=K[15]
	daddu	$1,$13

	daddu	$24,$11
	daddu	$1,$11
	ld	$10,16($29)	# prefetch from ring buffer
	b	.L16_xx
.align	4
.L16_xx:
	dsrl	$14,$9,7		# Xupdate(16)
	daddu	$8,$17			# +=X[i+9]
	dsll	$13,$9,56
	dsrl	$12,$9,1
	xor	$14,$13
	dsll	$13,7
	xor	$14,$12
	dsrl	$12,$9,8
	xor	$14,$13

	dsrl	$15,$22,6
	xor	$14,$12			# sigma0(X[i+1])
	dsll	$13,$22,3
	daddu	$8,$14
	dsrl	$12,$22,19
	xor	$15,$13
	dsll	$13,42
	xor	$15,$12
	dsrl	$12,$22,61
	xor	$15,$13

	xor	$15,$12			# sigma1(X[i+14])
	daddu	$8,$15
	daddu	$12,$8,$31			# 16
	dsrl	$31,$24,14
	xor	$15,$25,$30
	dsll	$14,$24,23
	and	$15,$24
	dsrl	$13,$24,18
	xor	$31,$14
	dsll	$14,$24,46
	xor	$31,$13
	dsrl	$13,$24,41
	xor	$31,$14
	dsll	$14,$24,50
	xor	$31,$13
	xor	$15,$30			# Ch(e,f,g)
	xor	$13,$14,$31			# Sigma1(e)

	dsrl	$31,$1,28
	daddu	$12,$15
	ld	$15,128($6)		# K[16]
	dsll	$14,$1,25
	daddu	$12,$13
	dsrl	$13,$1,34
	xor	$31,$14
	dsll	$14,$1,30
	xor	$31,$13
	dsrl	$13,$1,39
	xor	$31,$14
	dsll	$14,$1,36
	xor	$31,$13
	sd	$8,0($29)	# offload to ring buffer
	xor	$31,$14			# Sigma0(a)

	or	$13,$1,$2
	and	$14,$1,$2
	and	$13,$3
	or	$14,$13			# Maj(a,b,c)
	daddu	$12,$15			# +=K[16]
	daddu	$31,$14

	daddu	$7,$12
	daddu	$31,$12
	ld	$11,24($29)	# prefetch from ring buffer
	dsrl	$15,$10,7		# Xupdate(17)
	daddu	$9,$18			# +=X[i+9]
	dsll	$14,$10,56
	dsrl	$13,$10,1
	xor	$15,$14
	dsll	$14,7
	xor	$15,$13
	dsrl	$13,$10,8
	xor	$15,$14

	dsrl	$16,$23,6
	xor	$15,$13			# sigma0(X[i+1])
	dsll	$14,$23,3
	daddu	$9,$15
	dsrl	$13,$23,19
	xor	$16,$14
	dsll	$14,42
	xor	$16,$13
	dsrl	$13,$23,61
	xor	$16,$14

	xor	$16,$13			# sigma1(X[i+14])
	daddu	$9,$16
	daddu	$13,$9,$30			# 17
	dsrl	$30,$7,14
	xor	$16,$24,$25
	dsll	$15,$7,23
	and	$16,$7
	dsrl	$14,$7,18
	xor	$30,$15
	dsll	$15,$7,46
	xor	$30,$14
	dsrl	$14,$7,41
	xor	$30,$15
	dsll	$15,$7,50
	xor	$30,$14
	xor	$16,$25			# Ch(e,f,g)
	xor	$14,$15,$30			# Sigma1(e)

	dsrl	$30,$31,28
	daddu	$13,$16
	ld	$16,136($6)		# K[17]
	dsll	$15,$31,25
	daddu	$13,$14
	dsrl	$14,$31,34
	xor	$30,$15
	dsll	$15,$31,30
	xor	$30,$14
	dsrl	$14,$31,39
	xor	$30,$15
	dsll	$15,$31,36
	xor	$30,$14
	sd	$9,8($29)	# offload to ring buffer
	xor	$30,$15			# Sigma0(a)

	or	$14,$31,$1
	and	$15,$31,$1
	and	$14,$2
	or	$15,$14			# Maj(a,b,c)
	daddu	$13,$16			# +=K[17]
	daddu	$30,$15

	daddu	$3,$13
	daddu	$30,$13
	ld	$12,32($29)	# prefetch from ring buffer
	dsrl	$16,$11,7		# Xupdate(18)
	daddu	$10,$19			# +=X[i+9]
	dsll	$15,$11,56
	dsrl	$14,$11,1
	xor	$16,$15
	dsll	$15,7
	xor	$16,$14
	dsrl	$14,$11,8
	xor	$16,$15

	dsrl	$17,$8,6
	xor	$16,$14			# sigma0(X[i+1])
	dsll	$15,$8,3
	daddu	$10,$16
	dsrl	$14,$8,19
	xor	$17,$15
	dsll	$15,42
	xor	$17,$14
	dsrl	$14,$8,61
	xor	$17,$15

	xor	$17,$14			# sigma1(X[i+14])
	daddu	$10,$17
	daddu	$14,$10,$25			# 18
	dsrl	$25,$3,14
	xor	$17,$7,$24
	dsll	$16,$3,23
	and	$17,$3
	dsrl	$15,$3,18
	xor	$25,$16
	dsll	$16,$3,46
	xor	$25,$15
	dsrl	$15,$3,41
	xor	$25,$16
	dsll	$16,$3,50
	xor	$25,$15
	xor	$17,$24			# Ch(e,f,g)
	xor	$15,$16,$25			# Sigma1(e)

	dsrl	$25,$30,28
	daddu	$14,$17
	ld	$17,144($6)		# K[18]
	dsll	$16,$30,25
	daddu	$14,$15
	dsrl	$15,$30,34
	xor	$25,$16
	dsll	$16,$30,30
	xor	$25,$15
	dsrl	$15,$30,39
	xor	$25,$16
	dsll	$16,$30,36
	xor	$25,$15
	sd	$10,16($29)	# offload to ring buffer
	xor	$25,$16			# Sigma0(a)

	or	$15,$30,$31
	and	$16,$30,$31
	and	$15,$1
	or	$16,$15			# Maj(a,b,c)
	daddu	$14,$17			# +=K[18]
	daddu	$25,$16

	daddu	$2,$14
	daddu	$25,$14
	ld	$13,40($29)	# prefetch from ring buffer
	dsrl	$17,$12,7		# Xupdate(19)
	daddu	$11,$20			# +=X[i+9]
	dsll	$16,$12,56
	dsrl	$15,$12,1
	xor	$17,$16
	dsll	$16,7
	xor	$17,$15
	dsrl	$15,$12,8
	xor	$17,$16

	dsrl	$18,$9,6
	xor	$17,$15			# sigma0(X[i+1])
	dsll	$16,$9,3
	daddu	$11,$17
	dsrl	$15,$9,19
	xor	$18,$16
	dsll	$16,42
	xor	$18,$15
	dsrl	$15,$9,61
	xor	$18,$16

	xor	$18,$15			# sigma1(X[i+14])
	daddu	$11,$18
	daddu	$15,$11,$24			# 19
	dsrl	$24,$2,14
	xor	$18,$3,$7
	dsll	$17,$2,23
	and	$18,$2
	dsrl	$16,$2,18
	xor	$24,$17
	dsll	$17,$2,46
	xor	$24,$16
	dsrl	$16,$2,41
	xor	$24,$17
	dsll	$17,$2,50
	xor	$24,$16
	xor	$18,$7			# Ch(e,f,g)
	xor	$16,$17,$24			# Sigma1(e)

	dsrl	$24,$25,28
	daddu	$15,$18
	ld	$18,152($6)		# K[19]
	dsll	$17,$25,25
	daddu	$15,$16
	dsrl	$16,$25,34
	xor	$24,$17
	dsll	$17,$25,30
	xor	$24,$16
	dsrl	$16,$25,39
	xor	$24,$17
	dsll	$17,$25,36
	xor	$24,$16
	sd	$11,24($29)	# offload to ring buffer
	xor	$24,$17			# Sigma0(a)

	or	$16,$25,$30
	and	$17,$25,$30
	and	$16,$31
	or	$17,$16			# Maj(a,b,c)
	daddu	$15,$18			# +=K[19]
	daddu	$24,$17

	daddu	$1,$15
	daddu	$24,$15
	ld	$14,48($29)	# prefetch from ring buffer
	dsrl	$18,$13,7		# Xupdate(20)
	daddu	$12,$21			# +=X[i+9]
	dsll	$17,$13,56
	dsrl	$16,$13,1
	xor	$18,$17
	dsll	$17,7
	xor	$18,$16
	dsrl	$16,$13,8
	xor	$18,$17

	dsrl	$19,$10,6
	xor	$18,$16			# sigma0(X[i+1])
	dsll	$17,$10,3
	daddu	$12,$18
	dsrl	$16,$10,19
	xor	$19,$17
	dsll	$17,42
	xor	$19,$16
	dsrl	$16,$10,61
	xor	$19,$17

	xor	$19,$16			# sigma1(X[i+14])
	daddu	$12,$19
	daddu	$16,$12,$7			# 20
	dsrl	$7,$1,14
	xor	$19,$2,$3
	dsll	$18,$1,23
	and	$19,$1
	dsrl	$17,$1,18
	xor	$7,$18
	dsll	$18,$1,46
	xor	$7,$17
	dsrl	$17,$1,41
	xor	$7,$18
	dsll	$18,$1,50
	xor	$7,$17
	xor	$19,$3			# Ch(e,f,g)
	xor	$17,$18,$7			# Sigma1(e)

	dsrl	$7,$24,28
	daddu	$16,$19
	ld	$19,160($6)		# K[20]
	dsll	$18,$24,25
	daddu	$16,$17
	dsrl	$17,$24,34
	xor	$7,$18
	dsll	$18,$24,30
	xor	$7,$17
	dsrl	$17,$24,39
	xor	$7,$18
	dsll	$18,$24,36
	xor	$7,$17
	sd	$12,32($29)	# offload to ring buffer
	xor	$7,$18			# Sigma0(a)

	or	$17,$24,$25
	and	$18,$24,$25
	and	$17,$30
	or	$18,$17			# Maj(a,b,c)
	daddu	$16,$19			# +=K[20]
	daddu	$7,$18

	daddu	$31,$16
	daddu	$7,$16
	ld	$15,56($29)	# prefetch from ring buffer
	dsrl	$19,$14,7		# Xupdate(21)
	daddu	$13,$22			# +=X[i+9]
	dsll	$18,$14,56
	dsrl	$17,$14,1
	xor	$19,$18
	dsll	$18,7
	xor	$19,$17
	dsrl	$17,$14,8
	xor	$19,$18

	dsrl	$20,$11,6
	xor	$19,$17			# sigma0(X[i+1])
	dsll	$18,$11,3
	daddu	$13,$19
	dsrl	$17,$11,19
	xor	$20,$18
	dsll	$18,42
	xor	$20,$17
	dsrl	$17,$11,61
	xor	$20,$18

	xor	$20,$17			# sigma1(X[i+14])
	daddu	$13,$20
	daddu	$17,$13,$3			# 21
	dsrl	$3,$31,14
	xor	$20,$1,$2
	dsll	$19,$31,23
	and	$20,$31
	dsrl	$18,$31,18
	xor	$3,$19
	dsll	$19,$31,46
	xor	$3,$18
	dsrl	$18,$31,41
	xor	$3,$19
	dsll	$19,$31,50
	xor	$3,$18
	xor	$20,$2			# Ch(e,f,g)
	xor	$18,$19,$3			# Sigma1(e)

	dsrl	$3,$7,28
	daddu	$17,$20
	ld	$20,168($6)		# K[21]
	dsll	$19,$7,25
	daddu	$17,$18
	dsrl	$18,$7,34
	xor	$3,$19
	dsll	$19,$7,30
	xor	$3,$18
	dsrl	$18,$7,39
	xor	$3,$19
	dsll	$19,$7,36
	xor	$3,$18
	sd	$13,40($29)	# offload to ring buffer
	xor	$3,$19			# Sigma0(a)

	or	$18,$7,$24
	and	$19,$7,$24
	and	$18,$25
	or	$19,$18			# Maj(a,b,c)
	daddu	$17,$20			# +=K[21]
	daddu	$3,$19

	daddu	$30,$17
	daddu	$3,$17
	ld	$16,64($29)	# prefetch from ring buffer
	dsrl	$20,$15,7		# Xupdate(22)
	daddu	$14,$23			# +=X[i+9]
	dsll	$19,$15,56
	dsrl	$18,$15,1
	xor	$20,$19
	dsll	$19,7
	xor	$20,$18
	dsrl	$18,$15,8
	xor	$20,$19

	dsrl	$21,$12,6
	xor	$20,$18			# sigma0(X[i+1])
	dsll	$19,$12,3
	daddu	$14,$20
	dsrl	$18,$12,19
	xor	$21,$19
	dsll	$19,42
	xor	$21,$18
	dsrl	$18,$12,61
	xor	$21,$19

	xor	$21,$18			# sigma1(X[i+14])
	daddu	$14,$21
	daddu	$18,$14,$2			# 22
	dsrl	$2,$30,14
	xor	$21,$31,$1
	dsll	$20,$30,23
	and	$21,$30
	dsrl	$19,$30,18
	xor	$2,$20
	dsll	$20,$30,46
	xor	$2,$19
	dsrl	$19,$30,41
	xor	$2,$20
	dsll	$20,$30,50
	xor	$2,$19
	xor	$21,$1			# Ch(e,f,g)
	xor	$19,$20,$2			# Sigma1(e)

	dsrl	$2,$3,28
	daddu	$18,$21
	ld	$21,176($6)		# K[22]
	dsll	$20,$3,25
	daddu	$18,$19
	dsrl	$19,$3,34
	xor	$2,$20
	dsll	$20,$3,30
	xor	$2,$19
	dsrl	$19,$3,39
	xor	$2,$20
	dsll	$20,$3,36
	xor	$2,$19
	sd	$14,48($29)	# offload to ring buffer
	xor	$2,$20			# Sigma0(a)

	or	$19,$3,$7
	and	$20,$3,$7
	and	$19,$24
	or	$20,$19			# Maj(a,b,c)
	daddu	$18,$21			# +=K[22]
	daddu	$2,$20

	daddu	$25,$18
	daddu	$2,$18
	ld	$17,72($29)	# prefetch from ring buffer
	dsrl	$21,$16,7		# Xupdate(23)
	daddu	$15,$8			# +=X[i+9]
	dsll	$20,$16,56
	dsrl	$19,$16,1
	xor	$21,$20
	dsll	$20,7
	xor	$21,$19
	dsrl	$19,$16,8
	xor	$21,$20

	dsrl	$22,$13,6
	xor	$21,$19			# sigma0(X[i+1])
	dsll	$20,$13,3
	daddu	$15,$21
	dsrl	$19,$13,19
	xor	$22,$20
	dsll	$20,42
	xor	$22,$19
	dsrl	$19,$13,61
	xor	$22,$20

	xor	$22,$19			# sigma1(X[i+14])
	daddu	$15,$22
	daddu	$19,$15,$1			# 23
	dsrl	$1,$25,14
	xor	$22,$30,$31
	dsll	$21,$25,23
	and	$22,$25
	dsrl	$20,$25,18
	xor	$1,$21
	dsll	$21,$25,46
	xor	$1,$20
	dsrl	$20,$25,41
	xor	$1,$21
	dsll	$21,$25,50
	xor	$1,$20
	xor	$22,$31			# Ch(e,f,g)
	xor	$20,$21,$1			# Sigma1(e)

	dsrl	$1,$2,28
	daddu	$19,$22
	ld	$22,184($6)		# K[23]
	dsll	$21,$2,25
	daddu	$19,$20
	dsrl	$20,$2,34
	xor	$1,$21
	dsll	$21,$2,30
	xor	$1,$20
	dsrl	$20,$2,39
	xor	$1,$21
	dsll	$21,$2,36
	xor	$1,$20
	sd	$15,56($29)	# offload to ring buffer
	xor	$1,$21			# Sigma0(a)

	or	$20,$2,$3
	and	$21,$2,$3
	and	$20,$7
	or	$21,$20			# Maj(a,b,c)
	daddu	$19,$22			# +=K[23]
	daddu	$1,$21

	daddu	$24,$19
	daddu	$1,$19
	ld	$18,80($29)	# prefetch from ring buffer
	dsrl	$22,$17,7		# Xupdate(24)
	daddu	$16,$9			# +=X[i+9]
	dsll	$21,$17,56
	dsrl	$20,$17,1
	xor	$22,$21
	dsll	$21,7
	xor	$22,$20
	dsrl	$20,$17,8
	xor	$22,$21

	dsrl	$23,$14,6
	xor	$22,$20			# sigma0(X[i+1])
	dsll	$21,$14,3
	daddu	$16,$22
	dsrl	$20,$14,19
	xor	$23,$21
	dsll	$21,42
	xor	$23,$20
	dsrl	$20,$14,61
	xor	$23,$21

	xor	$23,$20			# sigma1(X[i+14])
	daddu	$16,$23
	daddu	$20,$16,$31			# 24
	dsrl	$31,$24,14
	xor	$23,$25,$30
	dsll	$22,$24,23
	and	$23,$24
	dsrl	$21,$24,18
	xor	$31,$22
	dsll	$22,$24,46
	xor	$31,$21
	dsrl	$21,$24,41
	xor	$31,$22
	dsll	$22,$24,50
	xor	$31,$21
	xor	$23,$30			# Ch(e,f,g)
	xor	$21,$22,$31			# Sigma1(e)

	dsrl	$31,$1,28
	daddu	$20,$23
	ld	$23,192($6)		# K[24]
	dsll	$22,$1,25
	daddu	$20,$21
	dsrl	$21,$1,34
	xor	$31,$22
	dsll	$22,$1,30
	xor	$31,$21
	dsrl	$21,$1,39
	xor	$31,$22
	dsll	$22,$1,36
	xor	$31,$21
	sd	$16,64($29)	# offload to ring buffer
	xor	$31,$22			# Sigma0(a)

	or	$21,$1,$2
	and	$22,$1,$2
	and	$21,$3
	or	$22,$21			# Maj(a,b,c)
	daddu	$20,$23			# +=K[24]
	daddu	$31,$22

	daddu	$7,$20
	daddu	$31,$20
	ld	$19,88($29)	# prefetch from ring buffer
	dsrl	$23,$18,7		# Xupdate(25)
	daddu	$17,$10			# +=X[i+9]
	dsll	$22,$18,56
	dsrl	$21,$18,1
	xor	$23,$22
	dsll	$22,7
	xor	$23,$21
	dsrl	$21,$18,8
	xor	$23,$22

	dsrl	$8,$15,6
	xor	$23,$21			# sigma0(X[i+1])
	dsll	$22,$15,3
	daddu	$17,$23
	dsrl	$21,$15,19
	xor	$8,$22
	dsll	$22,42
	xor	$8,$21
	dsrl	$21,$15,61
	xor	$8,$22

	xor	$8,$21			# sigma1(X[i+14])
	daddu	$17,$8
	daddu	$21,$17,$30			# 25
	dsrl	$30,$7,14
	xor	$8,$24,$25
	dsll	$23,$7,23
	and	$8,$7
	dsrl	$22,$7,18
	xor	$30,$23
	dsll	$23,$7,46
	xor	$30,$22
	dsrl	$22,$7,41
	xor	$30,$23
	dsll	$23,$7,50
	xor	$30,$22
	xor	$8,$25			# Ch(e,f,g)
	xor	$22,$23,$30			# Sigma1(e)

	dsrl	$30,$31,28
	daddu	$21,$8
	ld	$8,200($6)		# K[25]
	dsll	$23,$31,25
	daddu	$21,$22
	dsrl	$22,$31,34
	xor	$30,$23
	dsll	$23,$31,30
	xor	$30,$22
	dsrl	$22,$31,39
	xor	$30,$23
	dsll	$23,$31,36
	xor	$30,$22
	sd	$17,72($29)	# offload to ring buffer
	xor	$30,$23			# Sigma0(a)

	or	$22,$31,$1
	and	$23,$31,$1
	and	$22,$2
	or	$23,$22			# Maj(a,b,c)
	daddu	$21,$8			# +=K[25]
	daddu	$30,$23

	daddu	$3,$21
	daddu	$30,$21
	ld	$20,96($29)	# prefetch from ring buffer
	dsrl	$8,$19,7		# Xupdate(26)
	daddu	$18,$11			# +=X[i+9]
	dsll	$23,$19,56
	dsrl	$22,$19,1
	xor	$8,$23
	dsll	$23,7
	xor	$8,$22
	dsrl	$22,$19,8
	xor	$8,$23

	dsrl	$9,$16,6
	xor	$8,$22			# sigma0(X[i+1])
	dsll	$23,$16,3
	daddu	$18,$8
	dsrl	$22,$16,19
	xor	$9,$23
	dsll	$23,42
	xor	$9,$22
	dsrl	$22,$16,61
	xor	$9,$23

	xor	$9,$22			# sigma1(X[i+14])
	daddu	$18,$9
	daddu	$22,$18,$25			# 26
	dsrl	$25,$3,14
	xor	$9,$7,$24
	dsll	$8,$3,23
	and	$9,$3
	dsrl	$23,$3,18
	xor	$25,$8
	dsll	$8,$3,46
	xor	$25,$23
	dsrl	$23,$3,41
	xor	$25,$8
	dsll	$8,$3,50
	xor	$25,$23
	xor	$9,$24			# Ch(e,f,g)
	xor	$23,$8,$25			# Sigma1(e)

	dsrl	$25,$30,28
	daddu	$22,$9
	ld	$9,208($6)		# K[26]
	dsll	$8,$30,25
	daddu	$22,$23
	dsrl	$23,$30,34
	xor	$25,$8
	dsll	$8,$30,30
	xor	$25,$23
	dsrl	$23,$30,39
	xor	$25,$8
	dsll	$8,$30,36
	xor	$25,$23
	sd	$18,80($29)	# offload to ring buffer
	xor	$25,$8			# Sigma0(a)

	or	$23,$30,$31
	and	$8,$30,$31
	and	$23,$1
	or	$8,$23			# Maj(a,b,c)
	daddu	$22,$9			# +=K[26]
	daddu	$25,$8

	daddu	$2,$22
	daddu	$25,$22
	ld	$21,104($29)	# prefetch from ring buffer
	dsrl	$9,$20,7		# Xupdate(27)
	daddu	$19,$12			# +=X[i+9]
	dsll	$8,$20,56
	dsrl	$23,$20,1
	xor	$9,$8
	dsll	$8,7
	xor	$9,$23
	dsrl	$23,$20,8
	xor	$9,$8

	dsrl	$10,$17,6
	xor	$9,$23			# sigma0(X[i+1])
	dsll	$8,$17,3
	daddu	$19,$9
	dsrl	$23,$17,19
	xor	$10,$8
	dsll	$8,42
	xor	$10,$23
	dsrl	$23,$17,61
	xor	$10,$8

	xor	$10,$23			# sigma1(X[i+14])
	daddu	$19,$10
	daddu	$23,$19,$24			# 27
	dsrl	$24,$2,14
	xor	$10,$3,$7
	dsll	$9,$2,23
	and	$10,$2
	dsrl	$8,$2,18
	xor	$24,$9
	dsll	$9,$2,46
	xor	$24,$8
	dsrl	$8,$2,41
	xor	$24,$9
	dsll	$9,$2,50
	xor	$24,$8
	xor	$10,$7			# Ch(e,f,g)
	xor	$8,$9,$24			# Sigma1(e)

	dsrl	$24,$25,28
	daddu	$23,$10
	ld	$10,216($6)		# K[27]
	dsll	$9,$25,25
	daddu	$23,$8
	dsrl	$8,$25,34
	xor	$24,$9
	dsll	$9,$25,30
	xor	$24,$8
	dsrl	$8,$25,39
	xor	$24,$9
	dsll	$9,$25,36
	xor	$24,$8
	sd	$19,88($29)	# offload to ring buffer
	xor	$24,$9			# Sigma0(a)

	or	$8,$25,$30
	and	$9,$25,$30
	and	$8,$31
	or	$9,$8			# Maj(a,b,c)
	daddu	$23,$10			# +=K[27]
	daddu	$24,$9

	daddu	$1,$23
	daddu	$24,$23
	ld	$22,112($29)	# prefetch from ring buffer
	dsrl	$10,$21,7		# Xupdate(28)
	daddu	$20,$13			# +=X[i+9]
	dsll	$9,$21,56
	dsrl	$8,$21,1
	xor	$10,$9
	dsll	$9,7
	xor	$10,$8
	dsrl	$8,$21,8
	xor	$10,$9

	dsrl	$11,$18,6
	xor	$10,$8			# sigma0(X[i+1])
	dsll	$9,$18,3
	daddu	$20,$10
	dsrl	$8,$18,19
	xor	$11,$9
	dsll	$9,42
	xor	$11,$8
	dsrl	$8,$18,61
	xor	$11,$9

	xor	$11,$8			# sigma1(X[i+14])
	daddu	$20,$11
	daddu	$8,$20,$7			# 28
	dsrl	$7,$1,14
	xor	$11,$2,$3
	dsll	$10,$1,23
	and	$11,$1
	dsrl	$9,$1,18
	xor	$7,$10
	dsll	$10,$1,46
	xor	$7,$9
	dsrl	$9,$1,41
	xor	$7,$10
	dsll	$10,$1,50
	xor	$7,$9
	xor	$11,$3			# Ch(e,f,g)
	xor	$9,$10,$7			# Sigma1(e)

	dsrl	$7,$24,28
	daddu	$8,$11
	ld	$11,224($6)		# K[28]
	dsll	$10,$24,25
	daddu	$8,$9
	dsrl	$9,$24,34
	xor	$7,$10
	dsll	$10,$24,30
	xor	$7,$9
	dsrl	$9,$24,39
	xor	$7,$10
	dsll	$10,$24,36
	xor	$7,$9
	sd	$20,96($29)	# offload to ring buffer
	xor	$7,$10			# Sigma0(a)

	or	$9,$24,$25
	and	$10,$24,$25
	and	$9,$30
	or	$10,$9			# Maj(a,b,c)
	daddu	$8,$11			# +=K[28]
	daddu	$7,$10

	daddu	$31,$8
	daddu	$7,$8
	ld	$23,120($29)	# prefetch from ring buffer
	dsrl	$11,$22,7		# Xupdate(29)
	daddu	$21,$14			# +=X[i+9]
	dsll	$10,$22,56
	dsrl	$9,$22,1
	xor	$11,$10
	dsll	$10,7
	xor	$11,$9
	dsrl	$9,$22,8
	xor	$11,$10

	dsrl	$12,$19,6
	xor	$11,$9			# sigma0(X[i+1])
	dsll	$10,$19,3
	daddu	$21,$11
	dsrl	$9,$19,19
	xor	$12,$10
	dsll	$10,42
	xor	$12,$9
	dsrl	$9,$19,61
	xor	$12,$10

	xor	$12,$9			# sigma1(X[i+14])
	daddu	$21,$12
	daddu	$9,$21,$3			# 29
	dsrl	$3,$31,14
	xor	$12,$1,$2
	dsll	$11,$31,23
	and	$12,$31
	dsrl	$10,$31,18
	xor	$3,$11
	dsll	$11,$31,46
	xor	$3,$10
	dsrl	$10,$31,41
	xor	$3,$11
	dsll	$11,$31,50
	xor	$3,$10
	xor	$12,$2			# Ch(e,f,g)
	xor	$10,$11,$3			# Sigma1(e)

	dsrl	$3,$7,28
	daddu	$9,$12
	ld	$12,232($6)		# K[29]
	dsll	$11,$7,25
	daddu	$9,$10
	dsrl	$10,$7,34
	xor	$3,$11
	dsll	$11,$7,30
	xor	$3,$10
	dsrl	$10,$7,39
	xor	$3,$11
	dsll	$11,$7,36
	xor	$3,$10
	sd	$21,104($29)	# offload to ring buffer
	xor	$3,$11			# Sigma0(a)

	or	$10,$7,$24
	and	$11,$7,$24
	and	$10,$25
	or	$11,$10			# Maj(a,b,c)
	daddu	$9,$12			# +=K[29]
	daddu	$3,$11

	daddu	$30,$9
	daddu	$3,$9
	ld	$8,0($29)	# prefetch from ring buffer
	dsrl	$12,$23,7		# Xupdate(30)
	daddu	$22,$15			# +=X[i+9]
	dsll	$11,$23,56
	dsrl	$10,$23,1
	xor	$12,$11
	dsll	$11,7
	xor	$12,$10
	dsrl	$10,$23,8
	xor	$12,$11

	dsrl	$13,$20,6
	xor	$12,$10			# sigma0(X[i+1])
	dsll	$11,$20,3
	daddu	$22,$12
	dsrl	$10,$20,19
	xor	$13,$11
	dsll	$11,42
	xor	$13,$10
	dsrl	$10,$20,61
	xor	$13,$11

	xor	$13,$10			# sigma1(X[i+14])
	daddu	$22,$13
	daddu	$10,$22,$2			# 30
	dsrl	$2,$30,14
	xor	$13,$31,$1
	dsll	$12,$30,23
	and	$13,$30
	dsrl	$11,$30,18
	xor	$2,$12
	dsll	$12,$30,46
	xor	$2,$11
	dsrl	$11,$30,41
	xor	$2,$12
	dsll	$12,$30,50
	xor	$2,$11
	xor	$13,$1			# Ch(e,f,g)
	xor	$11,$12,$2			# Sigma1(e)

	dsrl	$2,$3,28
	daddu	$10,$13
	ld	$13,240($6)		# K[30]
	dsll	$12,$3,25
	daddu	$10,$11
	dsrl	$11,$3,34
	xor	$2,$12
	dsll	$12,$3,30
	xor	$2,$11
	dsrl	$11,$3,39
	xor	$2,$12
	dsll	$12,$3,36
	xor	$2,$11
	sd	$22,112($29)	# offload to ring buffer
	xor	$2,$12			# Sigma0(a)

	or	$11,$3,$7
	and	$12,$3,$7
	and	$11,$24
	or	$12,$11			# Maj(a,b,c)
	daddu	$10,$13			# +=K[30]
	daddu	$2,$12

	daddu	$25,$10
	daddu	$2,$10
	ld	$9,8($29)	# prefetch from ring buffer
	dsrl	$13,$8,7		# Xupdate(31)
	daddu	$23,$16			# +=X[i+9]
	dsll	$12,$8,56
	dsrl	$11,$8,1
	xor	$13,$12
	dsll	$12,7
	xor	$13,$11
	dsrl	$11,$8,8
	xor	$13,$12

	dsrl	$14,$21,6
	xor	$13,$11			# sigma0(X[i+1])
	dsll	$12,$21,3
	daddu	$23,$13
	dsrl	$11,$21,19
	xor	$14,$12
	dsll	$12,42
	xor	$14,$11
	dsrl	$11,$21,61
	xor	$14,$12

	xor	$14,$11			# sigma1(X[i+14])
	daddu	$23,$14
	daddu	$11,$23,$1			# 31
	dsrl	$1,$25,14
	xor	$14,$30,$31
	dsll	$13,$25,23
	and	$14,$25
	dsrl	$12,$25,18
	xor	$1,$13
	dsll	$13,$25,46
	xor	$1,$12
	dsrl	$12,$25,41
	xor	$1,$13
	dsll	$13,$25,50
	xor	$1,$12
	xor	$14,$31			# Ch(e,f,g)
	xor	$12,$13,$1			# Sigma1(e)

	dsrl	$1,$2,28
	daddu	$11,$14
	ld	$14,248($6)		# K[31]
	dsll	$13,$2,25
	daddu	$11,$12
	dsrl	$12,$2,34
	xor	$1,$13
	dsll	$13,$2,30
	xor	$1,$12
	dsrl	$12,$2,39
	xor	$1,$13
	dsll	$13,$2,36
	xor	$1,$12
	sd	$23,120($29)	# offload to ring buffer
	xor	$1,$13			# Sigma0(a)

	or	$12,$2,$3
	and	$13,$2,$3
	and	$12,$7
	or	$13,$12			# Maj(a,b,c)
	daddu	$11,$14			# +=K[31]
	daddu	$1,$13

	daddu	$24,$11
	daddu	$1,$11
	ld	$10,16($29)	# prefetch from ring buffer
	and	$14,0xfff
	li	$15,2071
	.set	noreorder
	bne	$14,$15,.L16_xx
	add $6,16*8		# Ktbl+=16

	lw	$23,16*8($29)	# restore pointer to the end of input
	ld	$8,0*8($4)
	ld	$9,1*8($4)
	ld	$10,2*8($4)
	add $5,16*8
	ld	$11,3*8($4)
	daddu	$1,$8
	ld	$12,4*8($4)
	daddu	$2,$9
	ld	$13,5*8($4)
	daddu	$3,$10
	ld	$14,6*8($4)
	daddu	$7,$11
	ld	$15,7*8($4)
	daddu	$24,$12
	sd	$1,0*8($4)
	daddu	$25,$13
	sd	$2,1*8($4)
	daddu	$30,$14
	sd	$3,2*8($4)
	daddu	$31,$15
	sd	$7,3*8($4)
	sd	$24,4*8($4)
	sd	$25,5*8($4)
	sd	$30,6*8($4)
	sd	$31,7*8($4)

	bne	$5,$23,.Loop
	sub $6,512	# rewind $6

	lw	$31,192-1*4($29)
	lw	$30,192-2*4($29)
	lw	$23,192-3*4($29)
	lw	$22,192-4*4($29)
	lw	$21,192-5*4($29)
	lw	$20,192-6*4($29)
	lw	$19,192-7*4($29)
	lw	$18,192-8*4($29)
	lw	$17,192-9*4($29)
	lw	$16,192-10*4($29)
	jr	$31
	add $29,192
.end	sha512_block_data_order

.rdata
.align	5
K512:
	.dword	0x428a2f98d728ae22, 0x7137449123ef65cd
	.dword	0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc
	.dword	0x3956c25bf348b538, 0x59f111f1b605d019
	.dword	0x923f82a4af194f9b, 0xab1c5ed5da6d8118
	.dword	0xd807aa98a3030242, 0x12835b0145706fbe
	.dword	0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2
	.dword	0x72be5d74f27b896f, 0x80deb1fe3b1696b1
	.dword	0x9bdc06a725c71235, 0xc19bf174cf692694
	.dword	0xe49b69c19ef14ad2, 0xefbe4786384f25e3
	.dword	0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65
	.dword	0x2de92c6f592b0275, 0x4a7484aa6ea6e483
	.dword	0x5cb0a9dcbd41fbd4, 0x76f988da831153b5
	.dword	0x983e5152ee66dfab, 0xa831c66d2db43210
	.dword	0xb00327c898fb213f, 0xbf597fc7beef0ee4
	.dword	0xc6e00bf33da88fc2, 0xd5a79147930aa725
	.dword	0x06ca6351e003826f, 0x142929670a0e6e70
	.dword	0x27b70a8546d22ffc, 0x2e1b21385c26c926
	.dword	0x4d2c6dfc5ac42aed, 0x53380d139d95b3df
	.dword	0x650a73548baf63de, 0x766a0abb3c77b2a8
	.dword	0x81c2c92e47edaee6, 0x92722c851482353b
	.dword	0xa2bfe8a14cf10364, 0xa81a664bbc423001
	.dword	0xc24b8b70d0f89791, 0xc76c51a30654be30
	.dword	0xd192e819d6ef5218, 0xd69906245565a910
	.dword	0xf40e35855771202a, 0x106aa07032bbd1b8
	.dword	0x19a4c116b8d2d0c8, 0x1e376c085141ab53
	.dword	0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8
	.dword	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb
	.dword	0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3
	.dword	0x748f82ee5defb2fc, 0x78a5636f43172f60
	.dword	0x84c87814a1f0ab72, 0x8cc702081a6439ec
	.dword	0x90befffa23631e28, 0xa4506cebde82bde9
	.dword	0xbef9a3f7b2c67915, 0xc67178f2e372532b
	.dword	0xca273eceea26619c, 0xd186b8c721c0c207
	.dword	0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178
	.dword	0x06f067aa72176fba, 0x0a637dc5a2c898a6
	.dword	0x113f9804bef90dae, 0x1b710b35131c471b
	.dword	0x28db77f523047d84, 0x32caab7b40c72493
	.dword	0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c
	.dword	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a
	.dword	0x5fcb6fab3ad6faec, 0x6c44198c4a475817
.asciiz	"SHA512 for MIPS, CRYPTOGAMS by <appro@openssl.org>"
.align	5

#if defined(HAVE_GNU_STACK)
.section .note.GNU-stack,"",%progbits
#endif
