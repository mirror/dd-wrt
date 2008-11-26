
/*
Rounds fp register r1-r4, additional mantissa bits in r5 and stores result
at address r0. Returns to fastfpe_next.
*/

/*------------------------------------------------------------------------*/

	.data
	.globl	round_table
round_table:
	.word	round_single_ne
	.word	round_single_p
	.word	round_single_m
	.word	round_single_z
	.word	round_double_ne
	.word	round_double_p
	.word	round_double_m
	.word	round_double_z
	.word	round_extended_ne
	.word	round_extended_p
	.word	round_extended_m
	.word	round_extended_z
	.word	round_undef
	.word	round_undef
	.word	round_undef
	.word	round_undef

/*------------------------------------------------------------------------*/

	.text
round_single_ne:
	cmp	r4,#127
	bgt	round_single_nz_ne_overflow
	cmp	r4,#-126-23-1
	blt	round_single_z_ne_underflow
	cmp	r4,#-126
	blt	round_single_ne_denormalized

	adds	r6,r2,#0x80		// add 0x80.00000000.00000000 to
	bcs	round_single_add_ov	// mantissa and additional bits
	
	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,#0xff	// test for inexact

	ldrne	r7,[r10,#128]
	orrne	r7,r7,#16	// set inexact flag
	strne	r7,[r10,#128]

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r6,#0xff
	biceq	r6,r6,#0x100	// the even thingy

	mov	r3,#0		// remove bits not existing in single
	bic	r2,r6,#0xff	// remove bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_ne_denormalized:
	add	r7,r4,#150
	mov	r6,#0xffffffff
	mov	r6,r6,lsr r7

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,r6
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8	// set inexact, underflow flag
	strne	r8,[r10,#128]

	mov	r8,#0x80000000
	mov	r8,r8,lsr r7
	adds	r2,r2,r8
	bcs	round_single_ne_denormalized_ov

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,r6
	biceq	r2,r2,r8,lsl #1	// the even thingy

	mov	r3,#0
	bic	r2,r2,r6	// removing bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_ne_denormalized_ov:
	cmp	r4,#-150
	cmpeq	r3,#0
	cmpeq	r2,#0
	beq	round_single_z_ne_underflow	// 1.0*2^-150 to zero!
	add	r4,r4,#1
	cmp	r4,#-126	// left denormalized range ?
	cmpge	r2,#0x80	// yes -> overflow also without denormalisation ?
	ldrge	r5,[r10,#128]
	bicge	r5,r5,#8	// yes -> clear underflow flag
	strge	r5,[r10,#128]
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/

round_single_p:
	teq	r1,#0
	beq	round_single_nz
	b	round_single_z

/*------------------------------------------------------------------------*/

round_single_m:
	teq	r1,#0
	beq	round_single_z
	b	round_single_nz

/*------------------------------------------------------------------------*/

round_single_z:
	cmp	r4,#127
	bgt	round_single_z_overflow
	cmp	r4,#-126-23
	blt	round_single_z_ne_underflow
	cmp	r4,#-126
	blt	round_single_z_denormalized

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,#0xff	// testing for inexact
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16	// set inexact flag
	strne	r5,[r10,#128]

	mov	r3,#0
	bic	r2,r2,#0xff	// removing bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_z_overflow:
	cmp	r4,#0x7fffffff
	beq	round_single_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0xffffff00
	mov	r3,#0
	mov	r4,#127		// biggest non-infinity single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_infnan:
	orrs	r5,r3,r2,lsl#1		// is it Inf? ignore MSB
	beq	round_single_infnan_store
	tst	r2,#0x40000000		// is it a SNaN?
	beq	round_single_infnan_create_qnan
	mov	r3,#0			// these bits can not be stored
	bic	r2,r2,#0xff		// in single precision
round_single_infnan_store:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_infnan_create_qnan:
	mov	r1,#0x80000000
	mov	r2,#0xffffff00
	bic	r2,r2,#0x80000000	// r2 = 0x7fffff00
	mov	r3,#0
	ldr	r5,[r10,#128]
	orr	r5,r5,#1		// set invalid operation flag
	str	r5,[r10,#128]
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_z_ne_underflow:
	cmp	r4,#0x80000000
	beq	round_single_z_zero
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, underflow flag
	strne	r5,[r10,#128]
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000	// was by ERROR -127
round_single_z_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_z_denormalized:
	mov	r6,#0xffffffff
	add	r7,r4,#150

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,r6,lsr r7	// testing for tinyness
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, undeflow flag
	strne	r5,[r10,#128]

	mov	r3,#0
	bic	r2,r2,r6,lsr r7	// removing bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next
	
/*------------------------------------------------------------------------*/

round_single_nz:
	cmp	r4,#127
	bgt	round_single_nz_ne_overflow
	cmp	r4,#-126-23
	blt	round_single_nz_underflow
	cmp	r4,#-126
	blt	round_single_nz_denormalized

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,#0xffffffff	// add 0xff.ffffffff.ffffffff to
	adcs	r2,r2,#0xff		// mantissa and additional bits
	bcs	round_single_add_ov
	
	cmp	r5,#0xffffffff
	cmpeq	r3,#0xffffffff
	andeq	r5,r2,#0xff
	cmpeq	r5,#0xff	// test for inexact

	bic	r2,r2,#0xff	// remove bits not existing in single

round_single_add_ov_back:
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16	// set inexact flag
	strne	r5,[r10,#128]

	mov	r3,#0		// remove bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_add_ov:
	add	r4,r4,#1
	cmp	r4,#127
	bgt	round_single_nz_ne_overflow
	movs	r2,#0x80000000	// so that inexact flag gets set !!!
	b	round_single_add_ov_back

round_single_nz_ne_overflow:
	cmp	r4,#0x7fffffff
	beq	round_single_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000	// set MSB
	mov	r3,#0
	mov	r4,#0x7fffffff
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_nz_underflow:
	cmp	r4,#0x80000000
	beq	round_single_nz_zero

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, underflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000
	mov	r3,#0
	mov	r4,#-149	// smallest non-zero single
round_single_nz_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_nz_denormalized:
	mov	r6,#0xffffffff
	add	r7,r4,#150
	mov	r6,r6,lsr r7

	teq	r5,#0
	teqeq	r3,#0
	tsteq	r2,r6
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8	// set inexact, underflow flag
	strne	r8,[r10,#128]

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,#0xffffffff
	adcs	r2,r2,r6
	bcs	round_single_nz_denormalized_ov

	mov	r3,#0
	bic	r2,r2,r6	// removing bits not existing in single
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_single_nz_denormalized_ov:
	add	r4,r4,#1
	cmp	r4,#-126	// left denormalized range ?
	cmpge	r2,#0x100	// yes -> overflow also without denormalisation ?
	ldrge	r5,[r10,#128]
	bicge	r5,r5,#8	// yes -> clear underflow flag
	strge	r5,[r10,#128]
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next
	
/*------------------------------------------------------------------------*/

round_double_ne:
	mov	r7,#0xffffffff		// to generate e.g. 0x7ff

	cmp	r4,#1024
	bge	round_double_nz_ne_overflow
	add	r6,r4,#1024
	cmp	r6,#-1022+1024
	blt	round_double_ne_denormalized

	teq	r5,#0
	tsteq	r3,r7,lsr#32-11		// testing for inexact
	ldrne	r6,[r10,#128]
	orrne	r6,r6,#16		// set inexact flag
	strne	r6,[r10,#128]

	adds	r3,r3,#0x400		// add 0x0.00000400.00000000 to
	adcs	r2,r2,#0		// mantissa and additional bits
	bcs	round_double_add_ov

	teq	r5,#0
	tsteq	r3,r7,lsr#32-11
	biceq	r3,r3,#0x800		// the even thingy
	
	bic	r3,r3,r7,lsr#32-11	// remove bits not existing in double

	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_ne_denormalized:
	cmp	r6,#-1022-52-1+1024
	blt	round_double_z_ne_underflow

	adds	r6,r6,#1022+53-32-1024

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6			// testing for tinyness
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8		// set inexact, undeflow flag
	strne	r8,[r10,#128]

	bics	r8,r6,r6,lsr#1		// generate ...0001000...
	movne	r11,#0			// from     ...0001111...
	biceq	r11,r7,r7,lsr#1		// 64bit
	
	adds	r3,r3,r11
	adcs	r2,r2,r8
	bcs	round_double_ne_denormalized_ov

	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6
	bne	round_double_ne_denormalized_noeventhingy
	adds	r11,r11,r11
	adc	r8,r8,r8
	bic	r3,r3,r11
	bic	r2,r2,r8		// the even thingy

round_double_ne_denormalized_noeventhingy:
	bic	r3,r3,r7		// removing bits not existing in
	bic	r2,r2,r6		// denormalized double
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_ne_denormalized_ov:
	add	r6,r4,#1024
	cmp	r6,#-1023-52+1024
	cmpeq	r3,#0
	cmpeq	r2,#0
	beq	round_single_z_ne_underflow	// 1.0*2^(-1023-52) to zero!
	add	r4,r4,#1
	cmp	r6,#-1022-1+1024	// left denormalized range ?
	cmpge	r3,#0x400		// yes -> overflow also without denormalisation ?
	ldrge	r5,[r10,#128]
	bicge	r5,r5,#8		// yes -> clear underflow flag
	strge	r5,[r10,#128]
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/

round_double_p:
	teq	r1,#0
	beq	round_double_nz
	b	round_double_z

/*------------------------------------------------------------------------*/

round_double_m:
	teq	r1,#0
	beq	round_double_z
	b	round_double_nz

/*------------------------------------------------------------------------*/

round_double_z:
	mov	r7,#0xffffffff

	cmp	r4,#1024
	bge	round_double_z_overflow
	add	r6,r4,#1024
	cmp	r6,#-1022+1024
	blt	round_double_z_denormalized

	teq	r5,#0
	tsteq	r3,r7,lsr#32-11		// testing for inexact
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16		// set inexact flag
	strne	r5,[r10,#128]

	bic	r3,r3,r7,lsr#32-11	// removing bits not existing in double
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_z_overflow:
	cmp	r4,#0x7fffffff
	beq	round_double_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0xffffffff
	mov	r3,r2,lsl#11	// 0xfffff800
	mov	r4,#1024
	sub	r4,r4,#1	// 1023; biggest non-infinity double
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_infnan:
	orrs	r5,r3,r2,lsl#1		// is it Inf? ignore MSB
	beq	round_double_infnan_store
	tst	r2,#0x40000000		// is it a SNaN?
	beq	round_double_infnan_create_qnan
	bic	r3,r3,r7,lsr#32-11	// clear bits not in double
round_double_infnan_store:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_infnan_create_qnan:
	mov	r1,#0x80000000
	mov	r2,#0x7fffffff
	mov	r3,r2,lsl#11		// 0xfffff800
	ldr	r5,[r10,#128]
	orr	r5,r5,#1		// set invalid operation flag
	str	r5,[r10,#128]
	b	round_double_infnan_store

round_double_z_ne_underflow:
	cmp	r4,#0x80000000
	beq	round_double_z_zero
	ldr	r5,[r10,#128]
	orr	r5,r5,#16+8		// set inexact, underflow flag
	str	r5,[r10,#128]
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
round_double_z_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_z_denormalized:
	cmp	r6,#-1022-52+1024
	blt	round_double_z_ne_underflow

	adds	r6,r6,#1022+53-32-1024

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6		// testing for tinyness
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, undeflow flag
	strne	r5,[r10,#128]

	bic	r3,r3,r7	// rmoving bits not existing in
	bic	r2,r2,r6	// denormalized double
	stmia	r0,{r1-r4}
	b	fastfpe_next
	
/*------------------------------------------------------------------------*/

round_double_nz:
	mov	r7,#0xffffffff		// to generate e.g. 0x7ff

	cmp	r4,#1024
	bge	round_double_nz_ne_overflow
	add	r6,r4,#1024
	cmp	r6,#-1022+1024
	blt	round_double_nz_denormalized

	teq	r5,#0
	tsteq	r3,r7,lsr#32-11	// testing for inexact
	ldrne	r6,[r10,#128]
	orrne	r6,r6,#16	// set inexact flag
	strne	r6,[r10,#128]

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,r7,lsr#32-11	// add 0x0.000007ff.ffffffff to
	adcs	r2,r2,#0		// mantissa and additional bits
	bcs	round_double_add_ov
	
	bic	r3,r3,r7,lsr#32-11	// remove bits not existing in double

	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_add_ov:
	add	r4,r4,#1
	cmp	r4,#1024
	bge	round_double_nz_ne_overflow

//	ldrne	r6,[r10,#128]
//	orrne	r6,r6,#16	// set inexact flag
//	strne	r6,[r10,#128]
	mov	r2,#0x80000000
	mov	r3,#0
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_nz_ne_overflow:
	cmp	r4,#0x7fffffff
	beq	round_double_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000	// set MSB
	mov	r3,#0
	mov	r4,#0x7fffffff
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_nz_underflow:
	cmp	r4,#0x80000000
	beq	round_double_nz_zero

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, underflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000
	mov	r3,#0
	mov	r4,#-1074+1024
	sub	r4,r4,#1024	// smallest non-zero double
round_double_nz_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_nz_denormalized:
	cmp	r6,#-1022-52+1024
	blt	round_double_nz_underflow

	adds	r6,r6,#1022+53-32-1024

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6		// testing for tinyness
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8	// set inexact, undeflow flag
	strne	r8,[r10,#128]

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,r7
	adcs	r2,r2,r6
	bcs	round_double_nz_denormalized_ov

	bic	r3,r3,r7	// rmoving bits not existing in
	bic	r2,r2,r6	// denormalized double
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_double_nz_denormalized_ov:
	add	r4,r4,#1
	add	r6,r4,#1024
	cmp	r6,#-1022+1024	// left denormalized range ?
	cmpge	r3,#0x800	// yes -> overflow also without denormalisation ?
	ldrge	r5,[r10,#128]
	bicge	r5,r5,#8	// yes -> clear underflow flag
	strge	r5,[r10,#128]
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/

round_extended_ne:
	mov	r7,#0xffffffff		// to generate e.g. 0x7ff

	cmp	r4,#16384
	bge	round_extended_nz_ne_overflow
	add	r6,r4,#16384
	cmp	r6,#-16382+16384
	blt	round_extended_ne_denormalized

	teq	r5,#0			// testing for inexact
	ldrne	r6,[r10,#128]
	orrne	r6,r6,#16		// set inexact flag
	strne	r6,[r10,#128]

	adds	r5,r5,#0x80000000	// add 0x0.00000400.00000000 to
	adcs	r3,r3,#0		// mantissa and additional bits
	adcs	r2,r2,#0
	bcs	round_extended_add_ov

	teq	r5,#0
	biceq	r3,r3,#1		// the even thingy
	
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_ne_denormalized:
	cmp	r6,#-16382-63-1+16384
	blt	round_extended_z_ne_underflow

	adds	r6,r6,#16382+64-32-16384

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6			// testing for tinyness
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8		// set inexact, undeflow flag
	strne	r8,[r10,#128]

	bics	r8,r6,r6,lsr#1		// generate ...0001000...
	movne	r11,#0			// from     ...0001111...
	biceq	r11,r7,r7,lsr#1		// 64bit
	
	adds	r3,r3,r11
	adcs	r2,r2,r8
	bcs	round_extended_ne_denormalized_ov

	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6
	bne	round_extended_ne_denormalized_noeventhingy
	adds	r11,r11,r11
	adc	r8,r8,r8
	bic	r3,r3,r11
	bic	r2,r2,r8		// the even thingy

round_extended_ne_denormalized_noeventhingy:
	bic	r3,r3,r7		// removing bits not existing in
	bic	r2,r2,r6		// denormalized extended
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_ne_denormalized_ov:
	add	r6,r4,#16384
	cmp	r6,#-16383-63+16384
	cmpeq	r5,#0
	cmpeq	r3,#0
	cmpeq	r2,#0
	beq	round_single_z_ne_underflow	// 1.0*2^(-16383-63) to zero!
	add	r4,r4,#1
	cmp	r6,#-16382-1+16384	// left denormalized range ?
	blt	round_extended_ne_still_denormalized
	cmp	r5,#0x80000000		// FIXME yes -> overflow also without denormalisation ?
	ldrcs	r5,[r10,#128]
	biccs	r5,r5,#8		// yes -> clear underflow flag
	strcs	r5,[r10,#128]
round_extended_ne_still_denormalized:
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/

round_extended_p:
	teq	r1,#0
	beq	round_extended_nz
	b	round_extended_z

/*------------------------------------------------------------------------*/

round_extended_m:
	teq	r1,#0
	beq	round_extended_z
	b	round_extended_nz

/*------------------------------------------------------------------------*/

round_extended_z:
	mov	r7,#0xffffffff

	cmp	r4,#16384
	bge	round_extended_z_overflow
	add	r6,r4,#16384
	cmp	r6,#-16382+16384
	blt	round_extended_z_denormalized

	teq	r5,#0			// testing for inexact
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16		// set inexact flag
	strne	r5,[r10,#128]

	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_z_overflow:
	cmp	r4,#0x7fffffff
	beq	round_extended_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0xffffffff
	mov	r3,#0xffffffff
	mov	r4,#16384
	sub	r4,r4,#1	// 16383; biggest non-infinity extended
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_infnan:
	orrs	r5,r3,r2,lsl#1		// is it Inf? ignore MSB
	beq	round_extended_infnan_store
	tst	r2,#0x40000000		// is it a SNaN?
	beq	round_extended_infnan_create_qnan
	bic	r3,r3,r7,lsr#32-11	// clear bits not in extended
round_extended_infnan_store:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_infnan_create_qnan:
	mov	r1,#0x80000000
	mov	r2,#0x7fffffff
	mov	r3,#0xffffffff
	ldr	r5,[r10,#128]
	orr	r5,r5,#1		// set invalid operation flag
	str	r5,[r10,#128]
	b	round_extended_infnan_store

round_extended_z_ne_underflow:
	cmp	r4,#0x80000000
	beq	round_extended_z_zero
	ldr	r5,[r10,#128]
	orr	r5,r5,#16+8		// set inexact, underflow flag
	str	r5,[r10,#128]
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
round_extended_z_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_z_denormalized:
	cmp	r6,#-16382-63+16384
	blt	round_extended_z_ne_underflow

	adds	r6,r6,#16382+64-32-16384

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6		// testing for tinyness
	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, undeflow flag
	strne	r5,[r10,#128]

	bic	r3,r3,r7	// removing bits not existing in
	bic	r2,r2,r6	// denormalized extended
	stmia	r0,{r1-r4}
	b	fastfpe_next
	
/*------------------------------------------------------------------------*/

round_extended_nz:
	mov	r7,#0xffffffff		// to generate e.g. 0x7ff

	cmp	r4,#16384
	bge	round_extended_nz_ne_overflow
	add	r6,r4,#16384
	cmp	r6,#-16382+16384
	blt	round_extended_nz_denormalized

	teq	r5,#0		// testing for inexact
	ldrne	r6,[r10,#128]
	orrne	r6,r6,#16	// set inexact flag
	strne	r6,[r10,#128]

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,#0		// add 0x0.0.ffffffff to
	adcs	r2,r2,#0		// mantissa and additional bits
	bcs	round_extended_add_ov
	
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_add_ov:
	add	r4,r4,#1
	cmp	r4,#16384
	bge	round_extended_nz_ne_overflow

//	ldrne	r6,[r10,#128]
//	orrne	r6,r6,#16	// set inexact flag
//	strne	r6,[r10,#128]
	mov	r2,#0x80000000
	mov	r3,#0
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_nz_ne_overflow:
	cmp	r4,#0x7fffffff
	beq	round_extended_infnan

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+4	// set inexact,overflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000	// set MSB
	mov	r3,#0
	mov	r4,#0x7fffffff
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_nz_underflow:
	cmp	r4,#0x80000000
	beq	round_extended_nz_zero

	ldrne	r5,[r10,#128]
	orrne	r5,r5,#16+8	// set inexact, underflow flag
	strne	r5,[r10,#128]
	mov	r2,#0x80000000
	mov	r3,#0
	mov	r4,#-16445+16384
	sub	r4,r4,#16384	// smallest non-zero extended
round_extended_nz_zero:
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_nz_denormalized:
	cmp	r6,#-16382-63+16384
	blt	round_extended_nz_underflow

	adds	r6,r6,#16382+64-32-16384

	addmi	r6,r6,#32
	movmi	r6,r7,lsr r6

	movpl	r7,r7,lsr r6
	movpl	r6,#0
	
	teq	r5,#0
	tsteq	r3,r7
	tsteq	r2,r6		// testing for tinyness
	ldrne	r8,[r10,#128]
	orrne	r8,r8,#16+8	// set inexact, undeflow flag
	strne	r8,[r10,#128]

	adds	r5,r5,#0xffffffff
	adcs	r3,r3,r7
	adcs	r2,r2,r6
	bcs	round_extended_nz_denormalized_ov

	bic	r3,r3,r7	// removing bits not existing in
	bic	r2,r2,r6	// denormalized extended
	stmia	r0,{r1-r4}
	b	fastfpe_next

round_extended_nz_denormalized_ov:
	add	r4,r4,#1
	add	r6,r4,#16384
	cmp	r6,#-16382+16384	// left denormalized range ?
	cmpge	r3,#1			// yes -> overflow also without denormalisation ?
	ldrge	r5,[r10,#128]
	bicge	r5,r5,#8		// yes -> clear underflow flag
	strge	r5,[r10,#128]
	mov	r3,#0
	mov	r2,#0x80000000
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/

round_undef:
	stmia	r0,{r1-r4}
	b	fastfpe_next

/*------------------------------------------------------------------------*/
