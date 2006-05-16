	.file	1 "ipsec_dev.c"
	.abicalls
 #APP
	.macro	__sti
	.set	push
	.set	reorder
	.set	noat
	mfc0	$1,$12
	ori	$1,0x1f
	xori	$1,0x1e
	mtc0	$1,$12
	.set	pop
	.endm
	.macro	__cli
	.set	push
	.set	noat
	mfc0	$1,$12
	ori	$1,1
	xori	$1,1
	.set	noreorder
	mtc0	$1,$12
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	.set	pop
	.endm
	.macro	__save_flags flags
	.set	push
	.set	reorder
	mfc0	\flags, $12
	.set	pop
	.endm
	.macro	__save_and_cli result
	.set	push
	.set	reorder
	.set	noat
	mfc0	\result, $12
	ori	$1, \result, 1
	xori	$1, 1
	.set	noreorder
	mtc0	$1, $12
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	.set	pop
	.endm
	.macro	__restore_flags flags
	.set	noreorder
	.set	noat
	mfc0	$1, $12
	andi	\flags, 1
	ori	$1, 1
	xori	$1, 1
	or	\flags, $1
	mtc0	\flags, $12
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	sll	$0, $0, 1			# nop
	.set	at
	.set	reorder
	.endm
 #NO_APP
	.section .modinfo,"a",@progbits
	.align	2
	.type	__module_kernel_version,@object
	.size	__module_kernel_version,22
__module_kernel_version:
	.ascii	"kernel_version=2.4.20\000"
	.globl	memcpy
	.data
	.align	2
	.type	s_stats,@object
	.size	s_stats,64
s_stats:
	.word	1
	.space	60
	.align	2
	.type	ipsec_fb_tunnel_dev,@object
	.size	ipsec_fb_tunnel_dev,340
ipsec_fb_tunnel_dev:
	.ascii	"ipsec0\000"
	.space	9
	.space	36
	.word	ipsec_fb_tunnel_init
	.space	284
	.align	2
	.type	ipsec_fb_tunnel,@object
	.size	ipsec_fb_tunnel,156
ipsec_fb_tunnel:
	.space	4
	.word	ipsec_fb_tunnel_dev
	.space	104
	.ascii	"ipsec0\000"
	.space	9
	.space	28
	.align	2
	.type	tunnels,@object
	.size	tunnels,16
tunnels:
	.word	tunnels_wc
	.word	tunnels_l
	.word	tunnels_r
	.word	tunnels_r_l
	.align	2
	.type	ipsec_lock,@object
	.size	ipsec_lock,0
ipsec_lock:
	.text
	.align	2
	.ent	ipsec_tunnel_lookup
ipsec_tunnel_lookup:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	srl	$2,$4,4
	srl	$3,$5,4
	xor	$2,$4,$2
	xor	$3,$5,$3
	andi	$7,$2,0xf
	andi	$8,$3,0xf
	xor	$2,$7,$8
	sll	$2,$2,2
	lw	$6,tunnels_r_l($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1397
	sll	$2,$7,2
	.set	macro
	.set	reorder

$L1372:
	lw	$2,144($6)
	#nop
	beq	$5,$2,$L1394
$L1371:
	lw	$6,0($6)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1372
	sll	$2,$7,2
	.set	macro
	.set	reorder

$L1397:
	lw	$6,tunnels_r($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1398
	sll	$2,$8,2
	.set	macro
	.set	reorder

$L1378:
	lw	$3,148($6)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$4,$3,$L1395
	move	$2,$6
	.set	macro
	.set	reorder

$L1377:
	lw	$6,0($6)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1378
	sll	$2,$8,2
	.set	macro
	.set	reorder

$L1398:
	lw	$6,tunnels_l($2)
	#nop
	beq	$6,$0,$L1390
$L1384:
	lw	$3,144($6)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$5,$3,$L1396
	move	$2,$6
	.set	macro
	.set	reorder

$L1383:
	lw	$6,0($6)
	#nop
	bne	$6,$0,$L1384
$L1390:
	lw	$6,tunnels_wc
	#nop
	beq	$6,$0,$L1387
	lw	$3,4($6)
	#nop
	lhu	$3,88($3)
	#nop
	andi	$3,$3,0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1387
	move	$2,$6
	.set	macro
	.set	reorder

$L1399:
	j	$31
$L1387:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

$L1396:
	lw	$3,4($6)
	#nop
	lhu	$3,88($3)
	#nop
	andi	$3,$3,0x1
	bne	$3,$0,$L1399
	j	$L1383
$L1395:
	lw	$3,4($6)
	#nop
	lhu	$3,88($3)
	#nop
	andi	$3,$3,0x1
	bne	$3,$0,$L1399
	j	$L1377
$L1394:
	lw	$3,148($6)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L1371
	move	$2,$6
	.set	macro
	.set	reorder

	lw	$3,4($6)
	#nop
	lhu	$3,88($3)
	#nop
	andi	$3,$3,0x1
	bne	$3,$0,$L1399
	j	$L1371
	.end	ipsec_tunnel_lookup
$Lfe1:
	.size	ipsec_tunnel_lookup,$Lfe1-ipsec_tunnel_lookup
	.align	2
	.ent	ipsec_bucket
ipsec_bucket:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	lw	$5,144($4)
	lw	$3,148($4)
	srl	$2,$5,4
	srl	$4,$3,4
	xor	$2,$5,$2
	xor	$6,$3,$4
	andi	$7,$2,0xf
	move	$4,$0
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1401
	move	$2,$0
	.set	macro
	.set	reorder

	andi	$4,$6,0xf
	li	$2,2			# 0x2
$L1401:
	beq	$5,$0,$L1402
	xor	$4,$4,$7
	ori	$2,$2,0x1
$L1402:
	sll	$2,$2,2
	lw	$2,tunnels($2)
	sll	$3,$4,2
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	.end	ipsec_bucket
$Lfe2:
	.size	ipsec_bucket,$Lfe2-ipsec_bucket
	.align	2
	.ent	ipsec_tunnel_unlink
ipsec_tunnel_unlink:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 16, extra= 8
	.mask	0x90010000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	lw	$2,irq_stat+8
	sw	$16,24($sp)
	addu	$2,$2,1
	sw	$31,32($sp)
	sw	$28,28($sp)
	move	$16,$4
	sw	$2,irq_stat+8
 #APP
 #NO_APP
	la	$25,ipsec_bucket
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1418
	move	$3,$2
	.set	macro
	.set	reorder

$L1407:
	lw	$3,0($3)
$L1418:
	lw	$2,0($3)
	#nop
	beq	$2,$0,$L1412
	bne	$16,$2,$L1407
	lw	$2,0($16)
	#nop
	sw	$2,0($3)
$L1412:
 #APP
 #NO_APP
	la	$3,irq_stat+8
	lw	$2,0($3)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1416
	sw	$2,0($3)
	.set	macro
	.set	reorder

$L1411:
	lw	$31,32($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1416:
	lw	$2,-8($3)
	#nop
	beq	$2,$0,$L1411
	la	$25,do_softirq
	jal	$31,$25
	j	$L1411
	.end	ipsec_tunnel_unlink
$Lfe3:
	.size	ipsec_tunnel_unlink,$Lfe3-ipsec_tunnel_unlink
	.align	2
	.ent	ipsec_tunnel_link
ipsec_tunnel_link:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 16, extra= 8
	.mask	0x90010000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	sw	$16,24($sp)
	sw	$31,32($sp)
	sw	$28,28($sp)
	move	$16,$4
	la	$25,ipsec_bucket
	jal	$31,$25
	lw	$3,irq_stat+8
	#nop
	addu	$3,$3,1
	sw	$3,irq_stat+8
 #APP
 #NO_APP
	lw	$3,0($2)
	#nop
	sw	$3,0($16)
	sw	$16,0($2)
 #APP
 #NO_APP
	la	$3,irq_stat+8
	lw	$2,0($3)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1425
	sw	$2,0($3)
	.set	macro
	.set	reorder

$L1421:
	lw	$31,32($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1425:
	lw	$2,-8($3)
	#nop
	beq	$2,$0,$L1421
	la	$25,do_softirq
	jal	$31,$25
	j	$L1421
	.end	ipsec_tunnel_link
$Lfe4:
	.size	ipsec_tunnel_link,$Lfe4-ipsec_tunnel_link
	.align	2
	.globl	ipsec_tunnel_locate
	.ent	ipsec_tunnel_locate
ipsec_tunnel_locate:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	lw	$6,36($4)
	lw	$5,32($4)
	move	$3,$0
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1427
	move	$4,$0
	.set	macro
	.set	reorder

	srl	$2,$6,4
	xor	$2,$6,$2
	andi	$3,$2,0xf
	li	$4,2			# 0x2
$L1427:
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L1428
	srl	$2,$5,4
	.set	macro
	.set	reorder

	xor	$2,$5,$2
	andi	$2,$2,0xf
	xor	$3,$3,$2
	ori	$4,$4,0x1
$L1428:
	sll	$2,$4,2
	lw	$2,tunnels($2)
	sll	$3,$3,2
	addu	$2,$2,$3
	lw	$4,0($2)
	#nop
	beq	$4,$0,$L1435
$L1432:
	lw	$3,144($4)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$5,$3,$L1436
	move	$2,$4
	.set	macro
	.set	reorder

$L1431:
	lw	$4,0($4)
	#nop
	bne	$4,$0,$L1432
$L1435:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

$L1436:
	lw	$3,148($4)
	#nop
	bne	$6,$3,$L1431
	j	$31
	.end	ipsec_tunnel_locate
$Lfe5:
	.size	ipsec_tunnel_locate,$Lfe5-ipsec_tunnel_locate
	.rdata
	.align	2
$LC2:
	.ascii	"ipsec%d\000"
	.text
	.align	2
	.globl	ipsec_tunnel_create
	.ent	ipsec_tunnel_create
ipsec_tunnel_create:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 16, extra= 8
	.mask	0x900f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,48
	.cprestore 16
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$31,44($sp)
	sw	$28,40($sp)
	sw	$16,24($sp)
	move	$19,$4
	li	$17,1			# 0x1
	la	$18,__this_module+16
 #APP
	1:   ll      $2, 0($18)      # atomic_add
     addu    $2, $17                  
     sc      $2, 0($18)                  
     beqz    $2, 1b                  

 #NO_APP
	lw	$2,4($18)
	li	$4,496			# 0x1f0
	ori	$2,$2,0x18
	li	$5,496			# 0x1f0
	sw	$2,4($18)
	la	$25,kmalloc
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1451
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	li	$6,496			# 0x1f0
	la	$25,memset
	jal	$31,$25
	lw	$3,240($16)
	addu	$17,$16,340
	ori	$3,$3,0x10
	la	$2,ipsec_tunnel_init
	move	$4,$19
	sw	$2,52($16)
	sw	$3,240($16)
	sw	$17,104($16)
	sw	$16,4($17)
	addu	$2,$16,452
	addu	$3,$4,32
$L1439:
	lw	$5,0($4)
	lw	$6,4($4)
	lw	$7,8($4)
	lw	$8,12($4)
	sw	$5,0($2)
	sw	$6,4($2)
	sw	$7,8($2)
	sw	$8,12($2)
	addu	$4,$4,16
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L1439
	addu	$2,$2,16
	.set	macro
	.set	reorder

	lw	$3,0($4)
	lw	$5,4($4)
	lw	$6,8($4)
	sw	$3,0($2)
	sw	$5,4($2)
	sw	$6,8($2)
	sb	$0,127($17)
	addu	$2,$17,112
	move	$3,$16
 #APP
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	addiu	$2,1
	sb	$1,($3)
	bnez	$1,1b
	addiu	$3,1
	.set	at
	.set	reorder
 #NO_APP
	lb	$2,0($16)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1452
	li	$19,1			# 0x1
	.set	macro
	.set	reorder

	move	$4,$16
$L1456:
	la	$25,register_netdevice
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	bltz	$2,$L1449
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $8, 232($16)      # atomic_add
     addu    $8, $2                  
     sc      $8, 232($16)                  
     beqz    $8, 1b                  

 #NO_APP
	move	$4,$17
	la	$25,ipsec_tunnel_link
	jal	$31,$25
	move	$2,$17
$L1437:
	lw	$31,44($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

$L1449:
	move	$4,$16
$L1455:
	la	$25,kfree
	jal	$31,$25
	li	$2,1			# 0x1
 #APP
	1:   ll      $3, 0($18)      # atomic_sub
     subu    $3, $2                  
     sc      $3, 0($18)                  
     beqz    $3, 1b                  

 #NO_APP
	lw	$3,4($18)
	.set	noreorder
	.set	nomacro
	j	$L1453
	move	$2,$0
	.set	macro
	.set	reorder

$L1452:
$L1442:
	slt	$2,$19,100
	move	$6,$19
	la	$5,$LC2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1443
	move	$4,$16
	.set	macro
	.set	reorder

	la	$25,sprintf
	jal	$31,$25
	move	$4,$16
	la	$25,__dev_get_by_name
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1454
	li	$2,100			# 0x64
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L1442
	addu	$19,$19,1
	.set	macro
	.set	reorder

$L1443:
	li	$2,100			# 0x64
$L1454:
	.set	noreorder
	.set	nomacro
	beq	$19,$2,$L1455
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$2,0($16)
	#nop
	sw	$2,112($17)
	lw	$2,4($16)
	#nop
	sw	$2,116($17)
	lw	$2,8($16)
	#nop
	sw	$2,120($17)
	lw	$2,12($16)
	.set	noreorder
	.set	nomacro
	j	$L1456
	sw	$2,124($17)
	.set	macro
	.set	reorder

$L1451:
 #APP
	1:   ll      $3, 0($18)      # atomic_sub
     subu    $3, $17                  
     sc      $3, 0($18)                  
     beqz    $3, 1b                  

 #NO_APP
	lw	$3,4($18)
$L1453:
	ori	$3,$3,0x8
	.set	noreorder
	.set	nomacro
	j	$L1437
	sw	$3,4($18)
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_create
$Lfe6:
	.size	ipsec_tunnel_create,$Lfe6-ipsec_tunnel_create
	.align	2
	.ent	ipsec_tunnel_destructor
ipsec_tunnel_destructor:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	la	$2,ipsec_fb_tunnel_dev
	la	$3,__this_module+16
	.set	noreorder
	.set	nomacro
	beq	$4,$2,$L1459
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $2, 0($3)      # atomic_sub
     subu    $2, $5                  
     sc      $2, 0($3)                  
     beqz    $2, 1b                  

 #NO_APP
	lw	$2,4($3)
	#nop
	ori	$2,$2,0x8
	sw	$2,4($3)
$L1459:
	j	$31
	.end	ipsec_tunnel_destructor
$Lfe7:
	.size	ipsec_tunnel_destructor,$Lfe7-ipsec_tunnel_destructor
	.align	2
	.ent	ipsec_tunnel_uninit
ipsec_tunnel_uninit:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 16, extra= 8
	.mask	0x90010000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	sw	$16,24($sp)
	sw	$31,32($sp)
	move	$16,$4
	la	$2,ipsec_fb_tunnel_dev
	.set	noreorder
	.set	nomacro
	beq	$16,$2,$L1470
	sw	$28,28($sp)
	.set	macro
	.set	reorder

	lw	$4,104($16)
	la	$25,ipsec_tunnel_unlink
	jal	$31,$25
$L1467:
	li	$2,1			# 0x1
$L1472:
 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $3, 232($16)                           
     subu  $4, $3, $2                       
     sc    $4, 232($16)                           
     beqz  $4, 1b                           
     subu  $4, $3, $2                       
     sync                                   
.set pop                                    

 #NO_APP
	move	$2,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1471
	move	$4,$16
	.set	macro
	.set	reorder

$L1469:
	lw	$31,32($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1471:
	la	$25,netdev_finish_unregister
	jal	$31,$25
	j	$L1469
$L1470:
	lw	$2,irq_stat+8
	#nop
	addu	$2,$2,1
	sw	$2,irq_stat+8
 #APP
 #NO_APP
	sw	$0,tunnels_wc
 #APP
 #NO_APP
	la	$3,irq_stat+8
	lw	$2,0($3)
	#nop
	addu	$2,$2,-1
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1467
	sw	$2,0($3)
	.set	macro
	.set	reorder

	lw	$2,-8($3)
	#nop
	beq	$2,$0,$L1467
	la	$25,do_softirq
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1472
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_uninit
$Lfe8:
	.size	ipsec_tunnel_uninit,$Lfe8-ipsec_tunnel_uninit
	.align	2
	.globl	ipsec_esp_err
	.ent	ipsec_esp_err
ipsec_esp_err:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 16, extra= 8
	.mask	0x90010000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	sw	$31,32($sp)
	sw	$28,28($sp)
	sw	$16,24($sp)
	lw	$2,28($4)
	lw	$4,128($4)
	lbu	$16,0($2)
	lbu	$3,1($2)
	li	$2,11			# 0xb
	.set	noreorder
	.set	nomacro
	beq	$16,$2,$L1484
	slt	$2,$16,12
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1473
	li	$2,3			# 0x3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$16,$2,$L1495
	slt	$2,$3,6
	.set	macro
	.set	reorder

$L1473:
	lw	$31,32($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1495:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1474
	slt	$2,$3,3
	.set	macro
	.set	reorder

	beq	$2,$0,$L1473
$L1474:
	lw	$5,12($4)
	lw	$4,16($4)
	la	$25,ipsec_tunnel_lookup
	jal	$31,$25
	move	$4,$2
	beq	$4,$0,$L1473
	lw	$2,148($4)
	#nop
	beq	$2,$0,$L1473
	lbu	$2,140($4)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1496
	li	$2,11			# 0xb
	.set	macro
	.set	reorder

$L1491:
	#.set	volatile
	lw	$2,jiffies
	#.set	novolatile
	lw	$3,108($4)
	#nop
	subu	$2,$2,$3
	sltu	$2,$2,3000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1497
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,104($4)
	#nop
	addu	$2,$2,1
$L1497:
	sw	$2,104($4)
	#.set	volatile
	lw	$2,jiffies
	#.set	novolatile
	.set	noreorder
	.set	nomacro
	j	$L1473
	sw	$2,108($4)
	.set	macro
	.set	reorder

$L1490:
$L1496:
	bne	$16,$2,$L1491
	j	$L1473
$L1484:
	bne	$3,$0,$L1473
	j	$L1474
	.end	ipsec_esp_err
$Lfe9:
	.size	ipsec_esp_err,$Lfe9-ipsec_esp_err
	.globl	memcpy
	.align	2
	.globl	ipsec_esp_encapsulate
	.ent	ipsec_esp_encapsulate
ipsec_esp_encapsulate:
	.frame	$sp,112,$31		# vars= 32, regs= 11/0, args= 24, extra= 8
	.mask	0xd0ff0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,112
	.cprestore 24
	sw	$23,92($sp)
	sw	$22,88($sp)
	sw	$19,76($sp)
	sw	$17,68($sp)
	sw	$16,64($sp)
	sw	$31,104($sp)
	sw	$fp,100($sp)
	sw	$28,96($sp)
	sw	$21,84($sp)
	sw	$20,80($sp)
	sw	$18,72($sp)
	move	$22,$4
	lw	$21,32($22)
	move	$17,$5
	lw	$fp,36($22)
	move	$16,$17
	li	$5,1			# 0x1
	li	$23,8			# 0x8
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L1504
	li	$19,2			# 0x2
	.set	macro
	.set	reorder

	lw	$2,0($21)
	#nop
	lw	$3,32($2)
	lw	$5,28($2)
	addu	$23,$3,8
$L1504:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L1505
	sltu	$2,$5,4
	.set	macro
	.set	reorder

	beq	$2,$0,$L1506
	li	$5,4			# 0x4
$L1506:
	lw	$2,64($22)
	#nop
	addu	$19,$2,2
$L1505:
	lw	$4,92($17)
	subu	$6,$0,$5
	addu	$2,$4,$5
	addu	$2,$2,1
	lw	$5,124($17)
	and	$2,$2,$6
	lw	$3,128($17)
	sw	$2,56($sp)
	subu	$3,$3,$5
	subu	$4,$2,$4
	lw	$2,96($17)
	addu	$18,$4,-2
	addu	$3,$3,$23
	addu	$5,$3,20
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1536
	addu	$19,$19,$18
	.set	macro
	.set	reorder

	move	$3,$0
$L1510:
	move	$4,$17
	addu	$6,$3,$19
	li	$7,32			# 0x20
	la	$25,skb_copy_expand
	jal	$31,$25
	move	$17,$2
	beq	$17,$0,$L1512
	lw	$3,12($16)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1512
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $4, 40($3)      # atomic_add
     addu    $4, $2                  
     sc      $4, 40($3)                  
     beqz    $4, 1b                  

 #NO_APP
	la	$2,sock_wfree
	sw	$2,140($17)
	sw	$3,12($17)
	lw	$2,120($17)
 #APP
	1:   ll      $4, 84($3)      # atomic_add
     addu    $4, $2                  
     sc      $4, 84($3)                  
     beqz    $4, 1b                  

 #NO_APP
$L1512:
	#nop
	#.set	volatile
	lw	$2,112($16)
	#.set	novolatile
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L1539
	move	$4,$16
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 112($16)                           
     subu  $2, $4, $3                       
     sc    $2, 112($16)                           
     beqz  $2, 1b                           
     subu  $2, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1513
	move	$4,$16
	.set	macro
	.set	reorder

$L1515:
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L1537
	move	$2,$0
	.set	macro
	.set	reorder

	lw	$4,132($17)
	lw	$3,96($17)
	addu	$2,$4,$18
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L1538
	addu	$20,$2,2
	.set	macro
	.set	reorder

	lw	$2,92($17)
	lw	$3,136($17)
	addu	$4,$4,$19
	addu	$2,$2,$19
	sw	$2,92($17)
	sltu	$3,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1521
	sw	$4,132($17)
	.set	macro
	.set	reorder

$L1520:
	move	$5,$19
	move	$4,$17
	la	$6,$L1520
	la	$25,skb_over_panic
	jal	$31,$25
$L1521:
	li	$2,4			# 0x4
	sb	$2,-1($20)
	sb	$18,-2($20)
	.set	noreorder
	.set	nomacro
	blez	$18,$L1535
	addu	$2,$20,-2
	.set	macro
	.set	reorder

	addu	$2,$2,-1
$L1540:
	sb	$18,0($2)
	addu	$18,$18,-1
	.set	noreorder
	.set	nomacro
	bgtz	$18,$L1540
	addu	$2,$2,-1
	.set	macro
	.set	reorder

$L1535:
	lw	$2,128($17)
	lw	$3,92($17)
	lw	$4,124($17)
	subu	$2,$2,$23
	addu	$3,$3,$23
	sw	$3,92($17)
	sltu	$4,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1526
	sw	$2,128($17)
	.set	macro
	.set	reorder

$L1527:
	move	$5,$23
	move	$4,$17
	la	$6,$L1527
	la	$25,skb_under_panic
	jal	$31,$25
$L1526:
	lw	$5,20($22)
	li	$6,16711680			# 0xff0000
	andi	$4,$5,0xff00
	and	$3,$5,$6
	sll	$4,$4,8
	sll	$2,$5,24
	or	$2,$2,$4
	srl	$3,$3,8
	or	$2,$2,$3
	lw	$18,128($17)
	srl	$5,$5,24
	or	$2,$2,$5
	sw	$2,0($18)
	lw	$3,28($22)
	#nop
	addu	$3,$3,1
	andi	$4,$3,0xff00
	sll	$4,$4,8
	and	$6,$3,$6
	sll	$2,$3,24
	or	$2,$2,$4
	srl	$6,$6,8
	or	$2,$2,$6
	srl	$4,$3,24
	or	$2,$2,$4
	sw	$3,28($22)
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L1533
	sw	$2,4($18)
	.set	macro
	.set	reorder

	lw	$2,0($21)
	addu	$16,$18,8
	lw	$5,32($2)
	move	$4,$16
	la	$25,get_random_bytes
	jal	$31,$25
	lw	$2,0($21)
	lw	$7,56($sp)
	lw	$5,32($2)
	sw	$16,16($sp)
	addu	$5,$18,$5
	addu	$5,$5,8
	lw	$2,56($2)
	move	$4,$21
	move	$6,$5
	move	$25,$2
	jal	$31,$25
$L1533:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L1534
	addu	$16,$sp,32
	.set	macro
	.set	reorder

	lw	$3,0($fp)
	subu	$2,$20,$18
	sw	$2,16($sp)
	sw	$16,20($sp)
	lw	$6,60($22)
	lw	$2,72($3)
	addu	$5,$22,40
	move	$4,$fp
	move	$7,$18
	move	$25,$2
	jal	$31,$25
	lw	$6,64($22)
	move	$4,$20
	move	$5,$16
	la	$25,memcpy
	jal	$31,$25
$L1534:
	move	$2,$17
$L1503:
	lw	$31,104($sp)
	lw	$fp,100($sp)
	lw	$23,92($sp)
	lw	$22,88($sp)
	lw	$21,84($sp)
	lw	$20,80($sp)
	lw	$19,76($sp)
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,112
	.set	macro
	.set	reorder

$L1537:
	la	$4,s_stats+52
	lw	$3,0($4)
	#nop
	addu	$3,$3,1
	.set	noreorder
	.set	nomacro
	j	$L1503
	sw	$3,0($4)
	.set	macro
	.set	reorder

$L1513:
$L1539:
	la	$25,__kfree_skb
	jal	$31,$25
	j	$L1515
$L1536:
	lw	$3,136($17)
	lw	$2,132($17)
	.set	noreorder
	.set	nomacro
	j	$L1510
	subu	$3,$3,$2
	.set	macro
	.set	reorder

$L1538:
	li	$4,788			# 0x314
	la	$25,__out_of_line_bug
	jal	$31,$25
	.end	ipsec_esp_encapsulate
$Lfe10:
	.size	ipsec_esp_encapsulate,$Lfe10-ipsec_esp_encapsulate
	.align	2
	.globl	ipsec_esp_decapsulate
	.ent	ipsec_esp_decapsulate
ipsec_esp_decapsulate:
	.frame	$sp,88,$31		# vars= 24, regs= 8/0, args= 24, extra= 8
	.mask	0x903f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,88
	.cprestore 24
	sw	$21,76($sp)
	sw	$18,64($sp)
	sw	$31,84($sp)
	move	$18,$4
	sw	$28,80($sp)
	sw	$20,72($sp)
	sw	$19,68($sp)
	sw	$17,60($sp)
	sw	$16,56($sp)
	lbu	$6,105($18)
	move	$21,$5
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1544
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$2,136($18)
	#nop
	#.set	volatile
	lw	$3,0($2)
	#.set	novolatile
	li	$2,1			# 0x1
	beq	$3,$2,$L1544
	li	$4,1			# 0x1
$L1544:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1595
	move	$16,$18
	.set	macro
	.set	reorder

$L1605:
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1547
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$2,136($18)
	#nop
	#.set	volatile
	lw	$3,0($2)
	#.set	novolatile
	li	$2,1			# 0x1
	beq	$3,$2,$L1547
	li	$4,1			# 0x1
$L1547:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1550
	move	$4,$18
	.set	macro
	.set	reorder

	li	$5,32			# 0x20
	la	$25,skb_copy
	jal	$31,$25
	move	$17,$2
	#.set	volatile
	lw	$2,112($18)
	#.set	novolatile
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L1606
	move	$4,$18
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 112($16)                           
     subu  $2, $4, $3                       
     sc    $2, 112($16)                           
     beqz  $2, 1b                           
     subu  $2, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1551
	move	$4,$18
	.set	macro
	.set	reorder

	move	$18,$17
$L1550:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L1596
	move	$2,$0
	.set	macro
	.set	reorder

$L1542:
	lw	$2,96($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1555
	move	$4,$18
	.set	macro
	.set	reorder

	li	$5,32			# 0x20
	la	$25,skb_linearize
	jal	$31,$25
	beq	$2,$0,$L1555
	la	$3,s_stats+24
$L1604:
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
$L1557:
	#.set	volatile
	lw	$2,112($18)
	#.set	novolatile
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L1607
	move	$4,$18
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 112($18)                           
     subu  $2, $4, $3                       
     sc    $2, 112($18)                           
     beqz  $2, 1b                           
     subu  $2, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1590
	move	$2,$0
	.set	macro
	.set	reorder

$L1541:
	lw	$31,84($sp)
	lw	$21,76($sp)
	lw	$20,72($sp)
	lw	$19,68($sp)
	lw	$18,64($sp)
	lw	$17,60($sp)
	lw	$16,56($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,88
	.set	macro
	.set	reorder

$L1590:
	move	$4,$18
$L1607:
	la	$25,__kfree_skb
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1541
	move	$2,$0
	.set	macro
	.set	reorder

$L1555:
	lw	$20,128($18)
	#nop
	lw	$5,0($20)
	#nop
	andi	$3,$5,0xff00
	sll	$4,$5,24
	srl	$2,$5,8
	sll	$3,$3,8
	or	$4,$4,$3
	andi	$2,$2,0xff00
	or	$4,$4,$2
	srl	$5,$5,24
	or	$6,$4,$5
	beq	$6,$0,$L1559
	lw	$2,32($18)
	#nop
	lw	$5,12($2)
	lw	$4,16($2)
	la	$25,ipsec_sa_get
	jal	$31,$25
	move	$19,$2
	beq	$19,$0,$L1559
	lw	$3,36($19)
	#nop
	beq	$3,$0,$L1566
	lw	$2,92($18)
	lw	$4,64($19)
	lw	$3,0($3)
	sltu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1567
	addu	$17,$sp,32
	.set	macro
	.set	reorder

	la	$3,s_stats+28
	j	$L1604
$L1567:
	lw	$16,132($18)
	#nop
	subu	$16,$16,$4
	subu	$2,$16,$20
	sw	$2,16($sp)
	sw	$17,20($sp)
	lw	$6,60($19)
	lw	$4,36($19)
	lw	$2,72($3)
	addu	$5,$19,40
	move	$7,$20
	move	$25,$2
	jal	$31,$25
	lw	$6,64($19)
	move	$4,$16
	move	$5,$17
	la	$25,memcmp
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1568
	move	$4,$19
	.set	macro
	.set	reorder

	la	$25,ipsec_sa_put
	jal	$31,$25
	la	$3,s_stats+8
	j	$L1604
$L1568:
	lw	$2,92($18)
	lw	$3,64($19)
	#nop
	subu	$5,$2,$3
	sltu	$2,$5,$2
	beq	$2,$0,$L1566
	lw	$2,96($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1597
	move	$4,$18
	.set	macro
	.set	reorder

	move	$6,$0
	la	$25,___pskb_trim
	jal	$31,$25
$L1566:
	lw	$3,92($18)
	#nop
	sltu	$2,$3,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1573
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$2,96($18)
	addu	$3,$3,-8
	sltu	$2,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1598
	sw	$3,92($18)
	.set	macro
	.set	reorder

	lw	$2,128($18)
	#nop
	addu	$2,$2,8
	sw	$2,128($18)
	move	$4,$2
$L1573:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1599
	move	$4,$19
	.set	macro
	.set	reorder

	lw	$16,32($19)
	#nop
	beq	$16,$0,$L1608
	lw	$16,0($16)
	addu	$3,$20,8
	lw	$2,32($16)
	lw	$7,92($18)
	addu	$5,$20,$2
	sw	$3,16($sp)
	addu	$5,$5,8
	subu	$7,$7,$2
	lw	$4,32($19)
	lw	$2,72($16)
	move	$6,$5
	move	$25,$2
	jal	$31,$25
	lw	$16,32($16)
	lw	$3,92($18)
	#nop
	sltu	$2,$3,$16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1579
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$2,96($18)
	subu	$3,$3,$16
	sltu	$2,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1600
	sw	$3,92($18)
	.set	macro
	.set	reorder

	lw	$2,128($18)
	#nop
	addu	$2,$2,$16
	sw	$2,128($18)
	move	$4,$2
$L1579:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1601
	move	$4,$19
	.set	macro
	.set	reorder

$L1608:
	la	$25,ipsec_sa_put
	jal	$31,$25
	lw	$4,132($18)
	#nop
	addu	$4,$4,-1
	lbu	$2,0($4)
	addu	$4,$4,-1
	sb	$2,0($21)
	lw	$6,92($18)
	lbu	$5,0($4)
	addu	$2,$6,-2
	sltu	$2,$2,$5
	beq	$2,$0,$L1582
	la	$3,s_stats+12
	j	$L1604
$L1582:
	blez	$5,$L1593
$L1585:
	addu	$4,$4,-1
	lbu	$3,0($4)
	move	$2,$5
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1602
	addu	$5,$5,-1
	.set	macro
	.set	reorder

	la	$3,s_stats+12
	j	$L1604
$L1602:
	bgtz	$5,$L1585
$L1593:
	lw	$2,132($18)
	#nop
	lbu	$2,-2($2)
	#nop
	subu	$2,$6,$2
	addu	$5,$2,-2
	sltu	$2,$5,$6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1541
	move	$2,$18
	.set	macro
	.set	reorder

	lw	$2,96($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1603
	move	$4,$18
	.set	macro
	.set	reorder

	move	$6,$0
	la	$25,___pskb_trim
	jal	$31,$25
$L1588:
	.set	noreorder
	.set	nomacro
	j	$L1541
	move	$2,$18
	.set	macro
	.set	reorder

$L1603:
	lw	$2,128($18)
	sw	$5,92($18)
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L1588
	sw	$2,132($18)
	.set	macro
	.set	reorder

$L1601:
	la	$25,ipsec_sa_put
	jal	$31,$25
	la	$3,s_stats+28
	j	$L1604
$L1599:
	la	$25,ipsec_sa_put
	jal	$31,$25
	la	$3,s_stats+28
	j	$L1604
$L1597:
	lw	$2,128($18)
	sw	$5,92($18)
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L1566
	sw	$2,132($18)
	.set	macro
	.set	reorder

$L1559:
	la	$3,s_stats+4
	j	$L1604
$L1596:
	la	$4,s_stats+24
	lw	$3,0($4)
	#nop
	addu	$3,$3,1
	.set	noreorder
	.set	nomacro
	j	$L1541
	sw	$3,0($4)
	.set	macro
	.set	reorder

$L1551:
$L1606:
	la	$25,__kfree_skb
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1550
	move	$18,$17
	.set	macro
	.set	reorder

$L1595:
	#.set	volatile
	lw	$2,112($18)
	#.set	novolatile
	#nop
	xori	$2,$2,0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1542
	move	$16,$18
	.set	macro
	.set	reorder

	j	$L1605
$L1598:
	li	$4,828			# 0x33c
	la	$25,__out_of_line_bug
	jal	$31,$25
$L1600:
	li	$4,828			# 0x33c
	la	$25,__out_of_line_bug
	jal	$31,$25
	.end	ipsec_esp_decapsulate
$Lfe11:
	.size	ipsec_esp_decapsulate,$Lfe11-ipsec_esp_decapsulate
	.align	2
	.globl	ipsec_esp_rcv_tunnel
	.ent	ipsec_esp_rcv_tunnel
ipsec_esp_rcv_tunnel:
	.frame	$sp,40,$31		# vars= 0, regs= 4/0, args= 16, extra= 8
	.mask	0x90030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	sw	$17,28($sp)
	sw	$31,36($sp)
	sw	$28,32($sp)
	sw	$16,24($sp)
	move	$17,$4
	lw	$2,96($17)
	lw	$5,92($17)
	#nop
	subu	$6,$5,$2
	sltu	$2,$6,20
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1613
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	sltu	$2,$5,20
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1613
	move	$3,$0
	.set	macro
	.set	reorder

	li	$5,20			# 0x14
	subu	$5,$5,$6
	la	$25,__pskb_pull_tail
	jal	$31,$25
	sltu	$3,$0,$2
$L1613:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1616
	li	$6,12			# 0xc
	.set	macro
	.set	reorder

	lw	$16,32($17)
	lw	$2,128($17)
	addu	$4,$17,44
	move	$5,$0
	sw	$2,32($17)
	sw	$16,36($17)
	la	$25,memset
	jal	$31,$25
	sb	$0,106($17)
	li	$2,8			# 0x8
	lw	$5,16($16)
	lw	$4,12($16)
	sh	$2,116($17)
	la	$25,ipsec_tunnel_lookup
	jal	$31,$25
	move	$6,$2
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1622
	move	$4,$17
	.set	macro
	.set	reorder

	la	$3,s_stats+32
	lw	$2,0($3)
	lw	$4,92($17)
	addu	$2,$2,1
	sw	$2,0($3)
	lw	$2,8($6)
	lw	$3,16($6)
	lw	$5,4($6)
	addu	$3,$3,$4
	addu	$2,$2,1
	lw	$4,40($17)
	sw	$2,8($6)
	sw	$3,16($6)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1618
	sw	$5,24($17)
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
 #APP
	1:   ll      $3, 4($4)      # atomic_sub
     subu    $3, $2                  
     sc      $3, 4($4)                  
     beqz    $3, 1b                  

 #NO_APP
$L1618:
	lw	$5,152($17)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L1619
	sw	$0,40($17)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	lw	$2,0($5)
 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 0($2)                           
     subu  $6, $4, $3                       
     sc    $6, 0($2)                           
     beqz  $6, 1b                           
     subu  $6, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	move	$3,$6
	beq	$3,$0,$L1626
$L1619:
	sw	$0,152($17)
$L1628:
	move	$4,$17
	la	$25,netif_rx
	jal	$31,$25
	move	$2,$0
$L1629:
	lw	$31,36($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1626:
	lw	$4,0($5)
	#nop
	lw	$2,4($4)
	#nop
	move	$25,$2
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1628
	sw	$0,152($17)
	.set	macro
	.set	reorder

$L1622:
	la	$3,s_stats+20
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	li	$5,3			# 0x3
	li	$6,2			# 0x2
	move	$7,$0
	sw	$2,0($3)
	la	$25,icmp_send
	jal	$31,$25
$L1616:
	#.set	volatile
	lw	$2,112($17)
	#.set	novolatile
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L1630
	move	$4,$17
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 112($17)                           
     subu  $2, $4, $3                       
     sc    $2, 112($17)                           
     beqz  $2, 1b                           
     subu  $2, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1629
	move	$2,$0
	.set	macro
	.set	reorder

	move	$4,$17
$L1630:
	la	$25,__kfree_skb
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1629
	move	$2,$0
	.set	macro
	.set	reorder

	.end	ipsec_esp_rcv_tunnel
$Lfe12:
	.size	ipsec_esp_rcv_tunnel,$Lfe12-ipsec_esp_rcv_tunnel
	.align	2
	.globl	ipsec_esp_rcv
	.ent	ipsec_esp_rcv
ipsec_esp_rcv:
	.frame	$sp,40,$31		# vars= 8, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	addu	$5,$sp,24
	sw	$31,36($sp)
	sw	$28,32($sp)
	la	$25,ipsec_esp_decapsulate
	jal	$31,$25
	move	$5,$2
	beq	$5,$0,$L1631
	lbu	$3,24($sp)
	li	$2,4			# 0x4
	la	$7,s_stats+16
	move	$4,$5
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1637
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,0($7)
	#.set	volatile
	lw	$3,112($5)
	#.set	novolatile
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	beq	$3,$6,$L1638
	sw	$2,0($7)
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $3, 112($5)                           
     subu  $2, $3, $6                       
     sc    $2, 112($5)                           
     beqz  $2, 1b                           
     subu  $2, $3, $6                       
     sync                                   
.set pop                                    

 #NO_APP
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1634
	move	$2,$0
	.set	macro
	.set	reorder

$L1631:
	lw	$31,36($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

$L1634:
	move	$4,$5
$L1638:
	la	$25,__kfree_skb
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1631
	move	$2,$0
	.set	macro
	.set	reorder

$L1637:
	la	$25,ipsec_esp_rcv_tunnel
	jal	$31,$25
	j	$L1631
	.end	ipsec_esp_rcv
$Lfe13:
	.size	ipsec_esp_rcv,$Lfe13-ipsec_esp_rcv
	.align	2
	.ent	ipsec_tunnel_xmit
ipsec_tunnel_xmit:
	.frame	$sp,112,$31		# vars= 32, regs= 11/0, args= 24, extra= 8
	.mask	0xd0ff0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,112
	.cprestore 24
	sw	$fp,100($sp)
	sw	$22,88($sp)
	sw	$18,72($sp)
	sw	$5,116($sp)
	sw	$31,104($sp)
	sw	$28,96($sp)
	sw	$23,92($sp)
	sw	$21,84($sp)
	sw	$20,80($sp)
	sw	$19,76($sp)
	sw	$17,68($sp)
	sw	$16,64($sp)
	lw	$20,104($5)
	move	$18,$4
	lw	$3,100($20)
	addu	$fp,$20,132
	lhu	$2,6($fp)
	addu	$3,$3,1
	lw	$21,32($18)
	lw	$19,16($fp)
	lbu	$23,133($20)
	sw	$3,100($20)
	sw	$2,60($sp)
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1643
	addu	$22,$20,8
	.set	macro
	.set	reorder

$L1716:
	lw	$2,44($20)
	la	$3,s_stats+40
	addu	$2,$2,1
	sw	$2,44($20)
$L1715:
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
$L1644:
	lw	$2,20($22)
	#.set	volatile
	lw	$3,112($18)
	#.set	novolatile
	addu	$2,$2,1
	sw	$2,20($22)
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1720
	move	$4,$18
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $3, 112($18)                           
     subu  $4, $3, $2                       
     sc    $4, 112($18)                           
     beqz  $4, 1b                           
     subu  $4, $3, $2                       
     sync                                   
.set pop                                    

 #NO_APP
	move	$2,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1705
	move	$4,$18
	.set	macro
	.set	reorder

$L1707:
	lw	$3,100($20)
	move	$2,$0
	addu	$3,$3,-1
	sw	$3,100($20)
$L1642:
	lw	$31,104($sp)
	lw	$fp,100($sp)
	lw	$23,92($sp)
	lw	$22,88($sp)
	lw	$21,84($sp)
	lw	$20,80($sp)
	lw	$19,76($sp)
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,112
	.set	macro
	.set	reorder

$L1705:
$L1720:
	la	$25,__kfree_skb
	jal	$31,$25
	j	$L1707
$L1643:
	lhu	$3,116($18)
	li	$2,8			# 0x8
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1645
	andi	$2,$23,0x1
	.set	macro
	.set	reorder

	la	$3,s_stats+56
	j	$L1715
$L1645:
	beq	$2,$0,$L1646
	lbu	$23,1($21)
$L1646:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L1708
	addu	$4,$sp,32
	.set	macro
	.set	reorder

$L1722:
	move	$5,$0
	li	$6,20			# 0x14
	lw	$16,12($fp)
	lw	$17,128($20)
	la	$25,memset
	jal	$31,$25
	addu	$4,$sp,56
	andi	$2,$23,0x1e
	addu	$5,$sp,32
	sw	$19,32($sp)
	sw	$16,36($sp)
	sw	$17,44($sp)
	sb	$2,48($sp)
	la	$25,ip_route_output_key
	jal	$31,$25
	beq	$2,$0,$L1651
	lw	$2,76($20)
	la	$3,s_stats+44
	addu	$2,$2,1
	sw	$2,76($20)
$L1717:
	lw	$2,0($3)
	#nop
	addu	$2,$2,1
	sw	$2,0($3)
$L1650:
	lw	$2,40($18)
	#nop
	beq	$2,$0,$L1644
	lw	$2,96($2)
	#nop
	beq	$2,$0,$L1644
	lw	$2,28($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1644
	move	$4,$18
	.set	macro
	.set	reorder

	move	$25,$2
	jal	$31,$25
	j	$L1644
$L1651:
	lw	$4,56($sp)
	lw	$3,116($sp)
	lw	$2,12($4)
	#nop
	beq	$2,$3,$L1709
	lw	$2,36($4)
	#nop
	addu	$5,$2,-82
	slt	$2,$5,68
	beq	$2,$0,$L1655
	lw	$2,44($20)
	#nop
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1656
	sw	$2,44($20)
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
 #APP
	1:   ll      $3, 4($4)      # atomic_sub
     subu    $3, $2                  
     sc      $3, 4($4)                  
     beqz    $3, 1b                  

 #NO_APP
$L1656:
	la	$3,s_stats+48
	j	$L1715
$L1655:
	lw	$3,40($18)
	#nop
	beq	$3,$0,$L1657
	lw	$2,36($3)
	#nop
	sltu	$2,$5,$2
	beq	$2,$0,$L1657
	sw	$5,36($3)
$L1657:
	lhu	$2,6($21)
	lw	$4,60($sp)
	andi	$2,$2,0x40
	or	$4,$4,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1658
	sw	$4,60($sp)
	.set	macro
	.set	reorder

	lhu	$2,2($21)
	#nop
	srl	$3,$2,8
	sll	$2,$2,8
	or	$2,$2,$3
	andi	$2,$2,0xffff
	slt	$2,$5,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1658
	andi	$3,$5,0xff00
	.set	macro
	.set	reorder

	sll	$3,$3,8
	sll	$7,$5,24
	srl	$2,$5,8
	or	$7,$7,$3
	andi	$2,$2,0xff00
	or	$7,$7,$2
	srl	$2,$5,24
	or	$7,$7,$2
	move	$4,$18
	li	$5,3			# 0x3
	li	$6,4			# 0x4
	la	$25,icmp_send
	jal	$31,$25
	lw	$3,56($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1665
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $6, 4($3)      # atomic_sub
     subu    $6, $2                  
     sc      $6, 4($3)                  
     beqz    $6, 1b                  

 #NO_APP
$L1665:
	la	$3,s_stats+48
	j	$L1715
$L1658:
	lw	$4,104($20)
	#nop
	blez	$4,$L1666
	#.set	volatile
	lw	$2,jiffies
	#.set	novolatile
	lw	$3,108($20)
	#nop
	subu	$2,$2,$3
	sltu	$2,$2,3000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1667
	addu	$2,$4,-1
	.set	macro
	.set	reorder

	lw	$3,40($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1666
	sw	$2,104($20)
	.set	macro
	.set	reorder

	lw	$2,96($3)
	#nop
	beq	$2,$0,$L1666
	lw	$2,28($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1666
	move	$4,$18
	.set	macro
	.set	reorder

	move	$25,$2
	jal	$31,$25
$L1666:
	lw	$2,32($18)
	lw	$3,56($sp)
	sw	$2,28($18)
	lw	$5,112($3)
	lw	$4,108($3)
	lw	$6,152($20)
	lbu	$17,1($21)
	lbu	$19,8($21)
	la	$25,ipsec_sa_get
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1710
	move	$5,$18
	.set	macro
	.set	reorder

	move	$4,$16
	la	$25,ipsec_esp_encapsulate
	jal	$31,$25
	move	$18,$2
	move	$4,$16
	la	$25,ipsec_sa_put
	jal	$31,$25
	beq	$18,$0,$L1711
	lw	$2,128($18)
	lw	$3,92($18)
	lw	$4,124($18)
	addu	$2,$2,-20
	addu	$3,$3,20
	sw	$3,92($18)
	sltu	$4,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1677
	sw	$2,128($18)
	.set	macro
	.set	reorder

$L1678:
	move	$4,$18
	li	$5,20			# 0x14
	la	$6,$L1678
	la	$25,skb_under_panic
	jal	$31,$25
$L1677:
	lw	$2,128($18)
	addu	$4,$18,44
	sw	$2,32($18)
	move	$5,$0
	li	$6,12			# 0xc
	la	$25,memset
	jal	$31,$25
	lw	$3,40($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1680
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $6, 4($3)      # atomic_sub
     subu    $6, $2                  
     sc      $6, 4($3)                  
     beqz    $6, 1b                  

 #NO_APP
$L1680:
	lw	$16,32($18)
	li	$3,-241			# 0xffffff0f
	lw	$2,0($16)
	#nop
	and	$2,$2,$3
	ori	$2,$2,0x40
	li	$3,-16			# 0xfffffff0
	and	$2,$2,$3
	lw	$3,56($sp)
	ori	$2,$2,0x5
	sw	$3,40($18)
	sw	$2,0($16)
	lw	$2,60($sp)
	#nop
	sh	$2,6($16)
	li	$2,50			# 0x32
	sb	$2,9($16)
	andi	$2,$17,0x2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1682
	andi	$3,$23,0xfc
	.set	macro
	.set	reorder

	andi	$2,$17,0x3
	or	$3,$3,$2
$L1682:
	sb	$3,1($16)
	lw	$3,56($sp)
	#nop
	lw	$2,108($3)
	#nop
	sw	$2,16($16)
	lw	$2,112($3)
	#nop
	sw	$2,12($16)
	lbu	$2,8($fp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1712
	sb	$2,8($16)
	.set	macro
	.set	reorder

$L1684:
	lw	$5,152($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L1685
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,0($5)
 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $4, 0($2)                           
     subu  $6, $4, $3                       
     sc    $6, 0($2)                           
     beqz  $6, 1b                           
     subu  $6, $4, $3                       
     sync                                   
.set pop                                    

 #NO_APP
	move	$3,$6
	beq	$3,$0,$L1713
$L1685:
	la	$4,s_stats+60
	lw	$2,0($4)
	lhu	$3,92($18)
	addu	$2,$2,1
	sw	$2,0($4)
	sb	$0,107($18)
	andi	$2,$3,0xff
	sll	$2,$2,8
	srl	$3,$3,8
	lhu	$4,6($16)
	or	$2,$2,$3
	sh	$2,2($16)
	sw	$0,152($18)
	andi	$4,$4,0x40
	lw	$17,92($18)
	lw	$5,56($sp)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1691
	move	$4,$16
	.set	macro
	.set	reorder

	sh	$0,4($16)
$L1719:
	la	$25,ip_send_check
	jal	$31,$25
	lw	$3,nf_hooks+152
	la	$2,nf_hooks+152
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1695
	move	$6,$18
	.set	macro
	.set	reorder

	lw	$2,40($18)
	lw	$3,92($18)
	lw	$2,36($2)
	#nop
	sltu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1698
	move	$4,$18
	.set	macro
	.set	reorder

	la	$5,ip_finish_output
	la	$25,ip_fragment
	jal	$31,$25
	move	$3,$2
$L1718:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1702
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	beq	$3,$2,$L1702
	lw	$3,20($22)
	lw	$2,64($22)
	addu	$3,$3,1
	addu	$2,$2,1
	sw	$2,64($22)
	.set	noreorder
	.set	nomacro
	j	$L1707
	sw	$3,20($22)
	.set	macro
	.set	reorder

$L1702:
	lw	$3,12($22)
	lw	$2,4($22)
	addu	$3,$3,$17
	addu	$2,$2,1
	sw	$2,4($22)
	.set	noreorder
	.set	nomacro
	j	$L1707
	sw	$3,12($22)
	.set	macro
	.set	reorder

$L1698:
	la	$25,ip_finish_output
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1718
	move	$3,$2
	.set	macro
	.set	reorder

$L1695:
	lw	$2,56($sp)
	#nop
	lw	$2,12($2)
	li	$4,2			# 0x2
	sw	$2,16($sp)
	li	$5,3			# 0x3
	la	$2,do_ip_send
	move	$7,$0
	sw	$2,20($sp)
	la	$25,nf_hook_slow
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1718
	move	$3,$2
	.set	macro
	.set	reorder

$L1691:
	la	$25,__ip_select_ident
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1719
	move	$4,$16
	.set	macro
	.set	reorder

$L1713:
	lw	$4,0($5)
	#nop
	lw	$2,4($4)
	#nop
	move	$25,$2
	jal	$31,$25
	j	$L1685
$L1712:
	.set	noreorder
	.set	nomacro
	j	$L1684
	sb	$19,8($16)
	.set	macro
	.set	reorder

$L1711:
	lw	$3,56($sp)
	#nop
	beq	$3,$0,$L1676
	li	$2,1			# 0x1
 #APP
	1:   ll      $4, 4($3)      # atomic_sub
     subu    $4, $2                  
     sc      $4, 4($3)                  
     beqz    $4, 1b                  

 #NO_APP
$L1676:
	lw	$2,28($22)
	la	$4,s_stats+52
	addu	$2,$2,1
	sw	$2,28($22)
	lw	$3,0($4)
	lw	$2,100($20)
	addu	$3,$3,1
	addu	$2,$2,-1
	sw	$2,100($20)
	sw	$3,0($4)
	.set	noreorder
	.set	nomacro
	j	$L1642
	move	$2,$0
	.set	macro
	.set	reorder

$L1710:
	lw	$3,56($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1671
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $4, 4($3)      # atomic_sub
     subu    $4, $2                  
     sc      $4, 4($3)                  
     beqz    $4, 1b                  

 #NO_APP
$L1671:
	lw	$2,28($22)
	#.set	volatile
	lw	$3,112($18)
	#.set	novolatile
	addu	$2,$2,1
	sw	$2,28($22)
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L1721
	move	$4,$18
	.set	macro
	.set	reorder

 #APP
	.set push                                   
.set noreorder           # atomic_sub_return
1:   ll    $3, 112($18)                           
     subu  $6, $3, $2                       
     sc    $6, 112($18)                           
     beqz  $6, 1b                           
     subu  $6, $3, $2                       
     sync                                   
.set pop                                    

 #NO_APP
	move	$2,$6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1672
	move	$4,$18
	.set	macro
	.set	reorder

$L1674:
	la	$2,s_stats+36
	lw	$4,0($2)
	lw	$3,100($20)
	addu	$4,$4,1
	addu	$3,$3,-1
	sw	$4,0($2)
	sw	$3,100($20)
	.set	noreorder
	.set	nomacro
	j	$L1642
	move	$2,$0
	.set	macro
	.set	reorder

$L1672:
$L1721:
	la	$25,__kfree_skb
	jal	$31,$25
	j	$L1674
$L1667:
	.set	noreorder
	.set	nomacro
	j	$L1666
	sw	$0,104($20)
	.set	macro
	.set	reorder

$L1709:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L1716
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $6, 4($4)      # atomic_sub
     subu    $6, $2                  
     sc      $6, 4($4)                  
     beqz    $6, 1b                  

 #NO_APP
	j	$L1716
$L1708:
	lw	$2,40($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1714
	sw	$2,56($sp)
	.set	macro
	.set	reorder

	lw	$19,120($2)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$19,$0,$L1722
	addu	$4,$sp,32
	.set	macro
	.set	reorder

	la	$3,s_stats+56
	j	$L1717
$L1714:
	lw	$2,80($20)
	la	$3,s_stats+56
	addu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L1715
	sw	$2,80($20)
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_xmit
$Lfe14:
	.size	ipsec_tunnel_xmit,$Lfe14-ipsec_tunnel_xmit
	.align	2
	.ent	ipsec_sa_check_ioctl_version
ipsec_sa_check_ioctl_version:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 16, extra= 8
	.mask	0x10000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	sw	$28,24($sp)
	lw	$4,16($4)
	lw	$3,948($28)
	addu	$2,$4,1
	or	$2,$4,$2
	and	$3,$3,$2
 #APP
 #NO_APP
	sll	$2,$2,24
 #APP
 #NO_APP
	sra	$5,$2,24
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1725
	move	$6,$7
	.set	macro
	.set	reorder

 #APP
	1:	lb	$2,0($4)
	move	$6,$0
2:
	.section	.fixup,"ax"
3:	li	$6,-14
	move	$2,$0
	j	2b
	.previous
	.section	__ex_table,"a"
	.word	1b,3b
	.previous
 #NO_APP
	sll	$2,$2,24
	sra	$5,$2,24
$L1725:
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1743
	li	$2,-14			# 0xfffffff2
	.set	macro
	.set	reorder

$L1723:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

$L1743:
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L1733
	move	$6,$7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bltz	$3,$L1735
	move	$2,$5
	.set	macro
	.set	reorder

 #APP
	1:	sb	$2, 0($4)			# __put_user_asm
	move	$6, $0
2:
	.section	.fixup,"ax"
3:	li	$6,-14
	j	2b
	.previous
	.section	__ex_table,"a"
	.word	1b,3b
	.previous
 #NO_APP
$L1735:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1723
	li	$2,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L1723
	li	$2,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L1733:
	.set	noreorder
	.set	nomacro
	j	$L1723
	move	$2,$0
	.set	macro
	.set	reorder

	.end	ipsec_sa_check_ioctl_version
$Lfe15:
	.size	ipsec_sa_check_ioctl_version,$Lfe15-ipsec_sa_check_ioctl_version
	.align	2
	.ent	ipsec_tunnel_ioctl
ipsec_tunnel_ioctl:
	.frame	$sp,304,$31		# vars= 216, regs= 7/0, args= 48, extra= 8
	.mask	0x901f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,304
	.cprestore 48
	sw	$20,288($sp)
	sw	$19,284($sp)
	sw	$18,280($sp)
	sw	$17,276($sp)
	sw	$31,296($sp)
	sw	$28,292($sp)
	sw	$16,272($sp)
	move	$18,$4
	move	$19,$5
	move	$17,$0
	li	$2,1			# 0x1
	la	$20,__this_module+16
 #APP
	1:   ll      $3, 0($20)      # atomic_add
     addu    $3, $2                  
     sc      $3, 0($20)                  
     beqz    $3, 1b                  

 #NO_APP
	lw	$2,4($20)
	la $6,-35312($6)
	ori	$2,$2,0x18
	sw	$2,4($20)
	sltu	$2,$6,9
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1878
	sll	$2,$6,2
	.set	macro
	.set	reorder

	lw	$2,$L1879($2)
	#nop
	.cpadd	$2
	j	$2
	.rdata
	.align	3
$L1879:
	.gpword	$L1746
	.gpword	$L1760
	.gpword	$L1802
	.gpword	$L1780
	.gpword	$L1814
	.gpword	$L1834
	.gpword	$L1861
	.gpword	$L1878
	.gpword	$L1872
	.text
$L1746:
	la	$2,ipsec_fb_tunnel_dev
	.set	noreorder
	.set	nomacro
	beq	$18,$2,$L1881
	move	$16,$0
	.set	macro
	.set	reorder

$L1747:
	beq	$16,$0,$L1882
	addu	$5,$sp,160
$L1887:
	addu	$4,$16,144
	move	$3,$5
	addu	$2,$16,112
$L1754:
	lw	$6,0($2)
	lw	$7,4($2)
	lw	$8,8($2)
	lw	$9,12($2)
	sw	$6,0($3)
	sw	$7,4($3)
	sw	$8,8($3)
	sw	$9,12($3)
	addu	$2,$2,16
	.set	noreorder
	.set	nomacro
	bne	$2,$4,$L1754
	addu	$3,$3,16
	.set	macro
	.set	reorder

	lw	$4,16($19)
	lw	$6,0($2)
	lw	$7,4($2)
	lw	$8,8($2)
	sw	$6,0($3)
	sw	$7,4($3)
	sw	$8,8($3)
	lw	$3,948($28)
	addu	$2,$4,44
	or	$2,$4,$2
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1756
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

 #APP
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	
 #NO_APP
$L1756:
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L1885:
	li	$17,-14			# 0xfffffff2
$L1764:
	li	$2,1			# 0x1
$L1890:
 #APP
	1:   ll      $3, 0($20)      # atomic_sub
     subu    $3, $2                  
     sc      $3, 0($20)                  
     beqz    $3, 1b                  

 #NO_APP
	lw	$3,4($20)
	move	$2,$17
	ori	$3,$3,0x8
	sw	$3,4($20)
	lw	$31,296($sp)
	lw	$20,288($sp)
	lw	$19,284($sp)
	lw	$18,280($sp)
	lw	$17,276($sp)
	lw	$16,272($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,304
	.set	macro
	.set	reorder

$L1882:
	lw	$16,104($18)
	.set	noreorder
	.set	nomacro
	j	$L1887
	addu	$5,$sp,160
	.set	macro
	.set	reorder

$L1881:
	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,44
	or	$2,$5,$2
	and	$3,$3,$2
	li	$6,44			# 0x2c
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1749
	addu	$2,$sp,160
	.set	macro
	.set	reorder

	move	$4,$2
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1749:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1885
	move	$4,$2
	.set	macro
	.set	reorder

	la	$25,ipsec_tunnel_locate
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1747
	move	$16,$2
	.set	macro
	.set	reorder

$L1760:
	lw	$2,464($28)
	#nop
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1762
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$2,4($28)
	li	$3,1			# 0x1
	ori	$2,$2,0x100
	sw	$2,4($28)
$L1763:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1764
	addu	$16,$sp,160
	.set	macro
	.set	reorder

	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,44
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1766
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

	move	$4,$16
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1766:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$4,180($sp)
	li	$2,64			# 0x40
	andi	$3,$4,0xf0
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lbu	$3,189($sp)
	li	$2,50			# 0x32
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	andi	$3,$4,0xf
	li	$2,5			# 0x5
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lhu	$3,186($sp)
	#nop
	andi	$2,$3,0xffbf
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lbu	$2,188($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1772
	ori	$2,$3,0x40
	.set	macro
	.set	reorder

	sh	$2,186($sp)
$L1772:
	move	$4,$16
	la	$25,ipsec_tunnel_locate
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1764
	li	$17,-17			# 0xffffffef
	.set	macro
	.set	reorder

	move	$4,$16
	la	$25,ipsec_tunnel_create
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1764
	li	$17,-132			# 0xffffff7c
	.set	macro
	.set	reorder

	lw	$4,16($19)
	lw	$3,948($28)
	addu	$2,$4,44
	or	$2,$4,$2
	and	$3,$3,$2
	addu	$5,$16,112
	move	$17,$0
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1756
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

 #APP
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	
 #NO_APP
	j	$L1756
$L1762:
	.set	noreorder
	.set	nomacro
	j	$L1763
	move	$3,$0
	.set	macro
	.set	reorder

$L1802:
	lw	$2,464($28)
	#nop
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1804
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$2,4($28)
	li	$3,1			# 0x1
	ori	$2,$2,0x100
	sw	$2,4($28)
$L1805:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	la	$2,ipsec_fb_tunnel_dev
	.set	noreorder
	.set	nomacro
	beq	$18,$2,$L1883
	move	$4,$18
	.set	macro
	.set	reorder

$L1888:
	la	$25,unregister_netdevice
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1764
	move	$17,$2
	.set	macro
	.set	reorder

$L1883:
	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,44
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	addu	$2,$sp,160
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1808
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

	move	$4,$2
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1808:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1764
	move	$4,$2
	.set	macro
	.set	reorder

	la	$25,ipsec_tunnel_locate
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1764
	li	$17,-2			# 0xfffffffe
	.set	macro
	.set	reorder

	la	$2,ipsec_fb_tunnel
	.set	noreorder
	.set	nomacro
	beq	$16,$2,$L1764
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$18,4($16)
	.set	noreorder
	.set	nomacro
	j	$L1888
	move	$4,$18
	.set	macro
	.set	reorder

$L1804:
	.set	noreorder
	.set	nomacro
	j	$L1805
	move	$3,$0
	.set	macro
	.set	reorder

$L1780:
	lw	$2,464($28)
	#nop
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1782
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$2,4($28)
	li	$3,1			# 0x1
	ori	$2,$2,0x100
	sw	$2,4($28)
$L1783:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	la	$2,ipsec_fb_tunnel_dev
	.set	noreorder
	.set	nomacro
	beq	$18,$2,$L1764
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,44
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	addu	$7,$sp,160
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1786
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

	move	$4,$7
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1786:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$4,180($sp)
	li	$2,64			# 0x40
	andi	$3,$4,0xf0
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lbu	$3,189($sp)
	li	$2,50			# 0x32
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	andi	$3,$4,0xf
	li	$2,5			# 0x5
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lhu	$3,186($sp)
	#nop
	andi	$2,$3,0xffbf
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lbu	$2,188($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1792
	ori	$2,$3,0x40
	.set	macro
	.set	reorder

	sh	$2,186($sp)
$L1792:
	move	$4,$7
	la	$25,ipsec_tunnel_locate
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1793
	li	$17,-17			# 0xffffffef
	.set	macro
	.set	reorder

	lw	$2,4($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$18,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L1793:
	lhu	$2,88($18)
	#nop
	andi	$2,$2,0x10
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1880
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lw	$2,196($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L1794:
	lw	$16,104($18)
	move	$17,$0
	move	$4,$16
	la	$25,ipsec_tunnel_unlink
	jal	$31,$25
	lw	$2,176($sp)
	move	$4,$16
	sw	$2,128($16)
	lw	$2,192($sp)
	#nop
	sw	$2,144($16)
	lw	$2,196($sp)
	#nop
	sw	$2,148($16)
	lw	$2,200($sp)
	#nop
	sw	$2,152($16)
	lw	$2,192($sp)
	#nop
	sw	$2,120($18)
	lw	$2,196($sp)
	#nop
	sw	$2,112($18)
	la	$25,ipsec_tunnel_link
	jal	$31,$25
	move	$4,$18
	la	$25,netdev_state_change
	jal	$31,$25
	lbu	$2,188($sp)
	addu	$5,$16,112
	sb	$2,140($16)
	lbu	$2,181($sp)
	#nop
	sb	$2,133($16)
	lhu	$2,186($sp)
	lw	$3,948($28)
	sh	$2,138($16)
	lw	$4,16($19)
	#nop
	addu	$2,$4,44
	or	$2,$4,$2
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1756
	li	$6,44			# 0x2c
	.set	macro
	.set	reorder

 #APP
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	
 #NO_APP
	j	$L1756
$L1880:
	lw	$2,196($sp)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	j	$L1794
$L1782:
	.set	noreorder
	.set	nomacro
	j	$L1783
	move	$3,$0
	.set	macro
	.set	reorder

$L1814:
	move	$4,$19
	la	$25,ipsec_sa_check_ioctl_version
	jal	$31,$25
	move	$17,$2
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,104
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	addu	$18,$sp,56
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1817
	li	$6,104			# 0x68
	.set	macro
	.set	reorder

	move	$4,$18
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1817:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,56($sp)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lw	$2,60($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1884
	li	$17,-2			# 0xfffffffe
	.set	macro
	.set	reorder

$L1822:
	lw	$4,60($sp)
	lw	$5,64($sp)
	lw	$6,68($sp)
	la	$25,ipsec_sa_get
	jal	$31,$25
	move	$16,$2
$L1889:
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1764
	move	$4,$18
	.set	macro
	.set	reorder

	move	$5,$0
	li	$6,104			# 0x68
	la	$25,memset
	jal	$31,$25
	lw	$2,12($16)
	lw	$3,32($16)
	sw	$2,60($sp)
	lw	$2,16($16)
	#nop
	sw	$2,64($sp)
	lw	$2,20($16)
	#nop
	sw	$2,68($sp)
	lw	$2,24($16)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1825
	sw	$2,72($sp)
	.set	macro
	.set	reorder

	lw	$2,0($3)
	addu	$3,$sp,76
	lw	$2,12($2)
 #APP
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	addiu	$2,1
	sb	$1,($3)
	bnez	$1,1b
	addiu	$3,1
	.set	at
	.set	reorder
 #NO_APP
	#nop
	lw	$2,32($16)
	#nop
	lw	$2,16($2)
	#nop
	sw	$2,108($sp)
$L1825:
	lw	$2,36($16)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1827
	addu	$3,$sp,116
	.set	macro
	.set	reorder

	lw	$2,0($2)
	#nop
	lw	$2,12($2)
 #APP
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	addiu	$2,1
	sb	$1,($3)
	bnez	$1,1b
	addiu	$3,1
	.set	at
	.set	reorder
 #NO_APP
	#nop
	lw	$2,60($16)
	#nop
	sw	$2,148($sp)
	lw	$2,64($16)
	#nop
	sw	$2,156($sp)
$L1827:
	move	$4,$16
	la	$25,ipsec_sa_put
	jal	$31,$25
	lw	$4,16($19)
	lw	$3,948($28)
	addu	$2,$4,104
	or	$2,$4,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1874
	li	$6,104			# 0x68
	.set	macro
	.set	reorder

	move	$5,$18
 #APP
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	
 #NO_APP
	j	$L1874
$L1884:
	lw	$2,64($sp)
	#nop
	bne	$2,$0,$L1822
	lw	$4,68($sp)
	la	$25,ipsec_sa_get_num
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1889
	move	$16,$2
	.set	macro
	.set	reorder

$L1834:
	lw	$2,464($28)
	#nop
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1836
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$2,4($28)
	li	$3,1			# 0x1
	ori	$2,$2,0x100
	sw	$2,4($28)
$L1837:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1764
	move	$4,$19
	.set	macro
	.set	reorder

	la	$25,ipsec_sa_check_ioctl_version
	jal	$31,$25
	move	$17,$2
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,104
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1840
	li	$6,104			# 0x68
	.set	macro
	.set	reorder

	addu	$4,$sp,56
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1840:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,56($sp)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lw	$4,60($sp)
	#nop
	beq	$4,$0,$L1764
	lw	$5,64($sp)
	#nop
	beq	$5,$0,$L1764
	lw	$6,68($sp)
	#nop
	beq	$6,$0,$L1764
	la	$25,ipsec_sa_get
	jal	$31,$25
	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L1846
	li	$17,-17			# 0xffffffef
	.set	macro
	.set	reorder

	move	$4,$16
	la	$25,ipsec_sa_put
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L1846:
	lb	$2,76($sp)
	#nop
	beq	$2,$0,$L1847
	lw	$6,108($sp)
	#nop
	sltu	$2,$6,33
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lw	$5,112($sp)
	lw	$3,948($28)
	addu	$2,$5,$6
	or	$2,$5,$2
	or	$2,$2,$6
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1850
	li	$17,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	addu	$4,$sp,208
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1850:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L1847:
	lb	$2,116($sp)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1891
	addu	$2,$sp,76
	.set	macro
	.set	reorder

	lw	$6,148($sp)
	#nop
	sltu	$2,$6,33
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

	lw	$5,152($sp)
	lw	$3,948($28)
	addu	$2,$5,$6
	or	$2,$5,$2
	or	$2,$2,$6
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1857
	li	$17,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	addu	$4,$sp,240
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1857:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	addu	$2,$sp,76
$L1891:
	sw	$2,16($sp)
	addu	$2,$sp,208
	sw	$2,20($sp)
	lw	$2,108($sp)
	lw	$4,60($sp)
	sw	$2,24($sp)
	addu	$2,$sp,116
	sw	$2,28($sp)
	addu	$2,$sp,240
	sw	$2,32($sp)
	lw	$2,148($sp)
	lw	$5,64($sp)
	sw	$2,36($sp)
	lw	$6,68($sp)
	lw	$2,156($sp)
	lw	$7,72($sp)
	sw	$2,40($sp)
	sb	$0,107($sp)
	sb	$0,147($sp)
	la	$25,ipsec_sa_add
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1764
	move	$17,$2
	.set	macro
	.set	reorder

$L1836:
	.set	noreorder
	.set	nomacro
	j	$L1837
	move	$3,$0
	.set	macro
	.set	reorder

$L1861:
	lw	$2,464($28)
	#nop
	andi	$2,$2,0x1000
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1863
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	lw	$2,4($28)
	li	$3,1			# 0x1
	ori	$2,$2,0x100
	sw	$2,4($28)
$L1864:
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1764
	move	$4,$19
	.set	macro
	.set	reorder

	la	$25,ipsec_sa_check_ioctl_version
	jal	$31,$25
	move	$17,$2
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$5,16($19)
	lw	$3,948($28)
	addu	$2,$5,104
	or	$2,$5,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1867
	li	$6,104			# 0x68
	.set	macro
	.set	reorder

	addu	$4,$sp,56
 #APP
	.set	noreorder
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
	
 #NO_APP
$L1867:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	lw	$4,60($sp)
	lw	$5,64($sp)
	lw	$6,68($sp)
	la	$25,ipsec_sa_del
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L1764
	li	$17,-2			# 0xfffffffe
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L1764
	move	$17,$0
	.set	macro
	.set	reorder

$L1863:
	.set	noreorder
	.set	nomacro
	j	$L1864
	move	$3,$0
	.set	macro
	.set	reorder

$L1878:
	.set	noreorder
	.set	nomacro
	j	$L1764
	li	$17,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L1872:
	lw	$4,16($19)
	lw	$3,948($28)
	addu	$2,$4,64
	or	$2,$4,$2
	and	$3,$3,$2
	li	$17,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	bltz	$3,$L1874
	li	$6,64			# 0x40
	.set	macro
	.set	reorder

	la	$5,s_stats
 #APP
	.set	noat
	la	$1, __copy_user
	jalr	$1
	.set	at
	
 #NO_APP
$L1874:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L1890
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L1890
	move	$17,$0
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_ioctl
$Lfe16:
	.size	ipsec_tunnel_ioctl,$Lfe16-ipsec_tunnel_ioctl
	.align	2
	.ent	ipsec_tunnel_get_stats
ipsec_tunnel_get_stats:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	lw	$2,104($4)
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,8
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_get_stats
$Lfe17:
	.size	ipsec_tunnel_get_stats,$Lfe17-ipsec_tunnel_get_stats
	.align	2
	.ent	ipsec_tunnel_change_mtu
ipsec_tunnel_change_mtu:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	addu	$2,$5,-68
	li	$3,65440			# 0xffa0
	sltu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L1895
	li	$2,-22			# 0xffffffea
	.set	macro
	.set	reorder

	sw	$5,96($4)
	move	$2,$0
$L1895:
	j	$31
	.end	ipsec_tunnel_change_mtu
$Lfe18:
	.size	ipsec_tunnel_change_mtu,$Lfe18-ipsec_tunnel_change_mtu
	.align	2
	.ent	ipsec_tunnel_init_gen
ipsec_tunnel_init_gen:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	la	$2,ipsec_tunnel_uninit
	sw	$2,244($4)
	la	$2,ipsec_tunnel_destructor
	sw	$2,248($4)
	la	$2,ipsec_tunnel_xmit
	sw	$2,260($4)
	la	$2,ipsec_tunnel_get_stats
	sw	$2,68($4)
	la	$2,ipsec_tunnel_ioctl
	sw	$2,284($4)
	la	$2,ipsec_tunnel_change_mtu
	sw	$2,300($4)
	li	$2,31			# 0x1f
	sh	$2,100($4)
	li	$2,52			# 0x34
	sh	$2,102($4)
	li	$2,1480			# 0x5c8
	sw	$2,96($4)
	li	$2,128			# 0x80
	sh	$2,88($4)
	lw	$3,104($4)
	li	$2,4			# 0x4
	sb	$2,128($4)
	sw	$0,64($4)
	lw	$2,144($3)
	#nop
	sw	$2,120($4)
	lw	$2,148($3)
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$2,112($4)
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_init_gen
$Lfe19:
	.size	ipsec_tunnel_init_gen,$Lfe19-ipsec_tunnel_init_gen
	.align	2
	.ent	ipsec_tunnel_init
ipsec_tunnel_init:
	.frame	$sp,96,$31		# vars= 32, regs= 9/0, args= 16, extra= 8
	.mask	0x907f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,96
	.cprestore 16
	sw	$21,76($sp)
	sw	$20,72($sp)
	sw	$19,68($sp)
	sw	$31,88($sp)
	sw	$28,84($sp)
	sw	$22,80($sp)
	sw	$18,64($sp)
	sw	$17,60($sp)
	sw	$16,56($sp)
	move	$20,$4
	lw	$22,104($20)
	la	$25,ipsec_tunnel_init_gen
	jal	$31,$25
	addu	$2,$22,132
	lw	$21,16($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L1898
	move	$19,$0
	.set	macro
	.set	reorder

	lbu	$16,1($2)
	addu	$4,$sp,24
	move	$5,$0
	li	$6,20			# 0x14
	lw	$17,12($2)
	lw	$18,128($22)
	la	$25,memset
	jal	$31,$25
	andi	$16,$16,0x1e
	addu	$4,$sp,48
	addu	$5,$sp,24
	sw	$21,24($sp)
	sw	$17,28($sp)
	sw	$18,36($sp)
	sb	$16,40($sp)
	la	$25,ip_route_output_key
	jal	$31,$25
	beq	$2,$0,$L1908
$L1899:
	lhu	$2,88($20)
	#nop
	ori	$2,$2,0x10
	sh	$2,88($20)
$L1898:
	beq	$19,$0,$L1909
$L1907:
	lhu	$2,102($19)
	#nop
	addu	$2,$2,60
$L1911:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L1905
	sh	$2,102($20)
	.set	macro
	.set	reorder

	lw	$2,96($19)
	#nop
	addu	$2,$2,-82
$L1910:
	sw	$2,96($20)
	lw	$3,128($22)
	lw	$31,88($sp)
	sw	$3,64($20)
	lw	$22,80($sp)
	lw	$21,76($sp)
	lw	$20,72($sp)
	lw	$19,68($sp)
	lw	$18,64($sp)
	lw	$17,60($sp)
	lw	$16,56($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,96
	.set	macro
	.set	reorder

$L1905:
	.set	noreorder
	.set	nomacro
	j	$L1910
	li	$2,1418			# 0x58a
	.set	macro
	.set	reorder

$L1909:
	lw	$4,128($22)
	#nop
	beq	$4,$0,$L1902
	la	$25,__dev_get_by_index
	jal	$31,$25
	move	$19,$2
$L1902:
	.set	noreorder
	.set	nomacro
	bne	$19,$0,$L1907
	li	$2,92			# 0x5c
	.set	macro
	.set	reorder

	j	$L1911
$L1908:
	lw	$3,48($sp)
	#nop
	lw	$19,12($3)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1899
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

 #APP
	1:   ll      $4, 4($3)      # atomic_sub
     subu    $4, $2                  
     sc      $4, 4($3)                  
     beqz    $4, 1b                  

 #NO_APP
	j	$L1899
	.end	ipsec_tunnel_init
$Lfe20:
	.size	ipsec_tunnel_init,$Lfe20-ipsec_tunnel_init
	.align	2
	.ent	ipsec_fb_tunnel_open
ipsec_fb_tunnel_open:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	li	$2,1			# 0x1
	la	$3,__this_module+16
 #APP
	1:   ll      $4, 0($3)      # atomic_add
     addu    $4, $2                  
     sc      $4, 0($3)                  
     beqz    $4, 1b                  

 #NO_APP
	lw	$4,4($3)
	move	$2,$0
	ori	$4,$4,0x18
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$4,4($3)
	.set	macro
	.set	reorder

	.end	ipsec_fb_tunnel_open
$Lfe21:
	.size	ipsec_fb_tunnel_open,$Lfe21-ipsec_fb_tunnel_open
	.align	2
	.ent	ipsec_fb_tunnel_close
ipsec_fb_tunnel_close:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	li	$2,1			# 0x1
	la	$3,__this_module+16
 #APP
	1:   ll      $4, 0($3)      # atomic_sub
     subu    $4, $2                  
     sc      $4, 0($3)                  
     beqz    $4, 1b                  

 #NO_APP
	lw	$4,4($3)
	move	$2,$0
	ori	$4,$4,0x8
	.set	noreorder
	.set	nomacro
	j	$31
	sw	$4,4($3)
	.set	macro
	.set	reorder

	.end	ipsec_fb_tunnel_close
$Lfe22:
	.size	ipsec_fb_tunnel_close,$Lfe22-ipsec_fb_tunnel_close
	.align	2
	.ent	ipsec_fb_tunnel_init
ipsec_fb_tunnel_init:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 16, extra= 8
	.mask	0x90010000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,40
	.cprestore 16
	sw	$16,24($sp)
	sw	$31,32($sp)
	move	$16,$4
	sw	$28,28($sp)
	la	$25,ipsec_tunnel_init_gen
	jal	$31,$25
	la	$4,ipsec_fb_tunnel+132
	lw	$2,0($4)
	li	$3,-241			# 0xffffff0f
	and	$2,$2,$3
	ori	$2,$2,0x40
	li	$3,-16			# 0xfffffff0
	and	$2,$2,$3
	ori	$2,$2,0x5
	la	$3,ipsec_fb_tunnel_open
	sw	$3,252($16)
	sw	$2,0($4)
	la	$3,ipsec_fb_tunnel_close
	li	$2,50			# 0x32
	sw	$3,256($16)
	sb	$2,9($4)
	li	$2,1			# 0x1
 #APP
	1:   ll      $3, 232($16)      # atomic_add
     addu    $3, $2                  
     sc      $3, 232($16)                  
     beqz    $3, 1b                  

 #NO_APP
	lw	$31,32($sp)
	lw	$16,24($sp)
	addu	$4,$4,-132
	move	$2,$0
	sw	$4,tunnels_wc
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

	.end	ipsec_fb_tunnel_init
$Lfe23:
	.size	ipsec_fb_tunnel_init,$Lfe23-ipsec_fb_tunnel_init
	.rdata
	.align	2
$LC3:
	.ascii	"IPSEC_ESP\000"
	.data
	.align	2
	.type	ipsec_esp_protocol,@object
	.size	ipsec_esp_protocol,24
ipsec_esp_protocol:
	.word	ipsec_esp_rcv
	.word	ipsec_esp_err
	.space	4
	.byte	50
	.space	7
	.word	$LC3
	.align	2
	.type	banner,@object
	.size	banner,37
banner:
	.ascii	"<6>IPsec over IPv4 tunneling driver\n\000"
	.text
	.align	2
	.globl	ipsec_tunnel_module_init
	.ent	ipsec_tunnel_module_init
ipsec_tunnel_module_init:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	la	$4,banner
	sw	$31,28($sp)
	sw	$28,24($sp)
	la	$25,printk
	jal	$31,$25
	la	$25,ipsec_sa_init
	jal	$31,$25
	la	$3,ipsec_fb_tunnel_dev+104
	la	$2,ipsec_fb_tunnel
	addu	$4,$3,-104
	sw	$2,0($3)
	la	$25,register_netdev
	jal	$31,$25
	move	$3,$2
	la	$4,ipsec_esp_protocol
	beq	$3,$0,$L1917
$L1915:
	lw	$31,28($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

$L1917:
	la	$25,inet_add_protocol
	jal	$31,$25
	.set	noreorder
	.set	nomacro
	j	$L1915
	move	$2,$0
	.set	macro
	.set	reorder

	.end	ipsec_tunnel_module_init
$Lfe24:
	.size	ipsec_tunnel_module_init,$Lfe24-ipsec_tunnel_module_init
	.rdata
	.align	2
$LC4:
	.ascii	"<6>ipsec close: can't remove protocol\n\000"
	.text
	.align	2
	.ent	ipsec_tunnel_module_fini
ipsec_tunnel_module_fini:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	la	$4,ipsec_esp_protocol
	sw	$31,28($sp)
	sw	$28,24($sp)
	la	$25,inet_del_protocol
	jal	$31,$25
	la	$4,$LC4
	bltz	$2,$L1920
$L1919:
	la	$4,ipsec_fb_tunnel_dev
	la	$25,unregister_netdev
	jal	$31,$25
	la	$25,ipsec_sa_destroy
	jal	$31,$25
	lw	$31,28($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

$L1920:
	la	$25,printk
	jal	$31,$25
	j	$L1919
	.end	ipsec_tunnel_module_fini
$Lfe25:
	.size	ipsec_tunnel_module_fini,$Lfe25-ipsec_tunnel_module_fini
	.globl	init_module
	init_module=ipsec_tunnel_module_init
	.globl	cleanup_module
	cleanup_module=ipsec_tunnel_module_fini
	.globl	__module_author
	.section .modinfo,"a",@progbits
	.align	2
	.type	__module_author,@object
	.size	__module_author,51
__module_author:
	.ascii	"author=Tobias Ringstrom <tobias@ringstrom.mine.nu>\000"
	.globl	__module_description
	.align	2
	.type	__module_description,@object
	.size	__module_description,45
__module_description:
	.ascii	"description=IPsec over IPv4 tunneling driver\000"
	.align	2
	.type	__module_license,@object
	.size	__module_license,12
__module_license:
	.ascii	"license=GPL\000"
	.local	tunnels_r_l
	.comm	tunnels_r_l,64,4
	.local	tunnels_r
	.comm	tunnels_r,64,4
	.local	tunnels_l
	.comm	tunnels_l,64,4
	.local	tunnels_wc
	.comm	tunnels_wc,4,4
	.text
	.align	2
	.ent	do_ip_send
do_ip_send:
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 16, extra= 8
	.mask	0x90000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	reorder
	subu	$sp,$sp,32
	.cprestore 16
	sw	$31,28($sp)
	sw	$28,24($sp)
	move	$2,$4
	lw	$3,40($2)
	lw	$6,92($2)
	lw	$2,36($3)
	la	$5,ip_finish_output
	sltu	$2,$2,$6
	beq	$2,$0,$L1640
	la	$25,ip_fragment
	jal	$31,$25
$L1641:
	lw	$31,28($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

$L1640:
	la	$25,ip_finish_output
	jal	$31,$25
	j	$L1641
	.end	do_ip_send
$Lfe26:
	.size	do_ip_send,$Lfe26-do_ip_send
	.ident	"GCC: (GNU) 3.0 20010422 (prerelease) with bcm4710a0 modifications"
