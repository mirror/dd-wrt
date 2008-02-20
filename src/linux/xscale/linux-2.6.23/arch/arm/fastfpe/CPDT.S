/*
The FP structure has 4 words reserved for each register, the first is used just
for the sign in bit 31, the second and third are for the mantissa (unsigned
integer, high 32 bit first) and the fourth is the exponent (signed integer).
The mantissa is always normalized.

If the exponent is 0x80000000, that is the most negative value, the number
represented is 0 and both mantissa words are also 0.

If the exponent is 0x7fffffff, that is the biggest positive value, the number
represented is infinity if the mantissa is 0, otherwise it is a NaN.

Decimal and packed decimal numbers are not supported yet.
*/

/*---------------------------------------------------------------------------*/

	.globl	CPDT_load_single
CPDT_load_single:
	ldr	r1,[r6]
	
	and	r2,r1,#0x80000000	@ r2 = sign
	
	mov	r5,r1,lsr#23
	bics	r5,r5,#0x100
	beq	CPDT_ls_e0		@ exponent = 0; zero/denormalized
	teq	r5,#255
	beq	CPDT_ls_e255		@ exponent = 255; infinity/NaN
	
	sub     r5,r5,#127		@ r5 = exponent, remove normalized bias
	
	mov	r3,r1,lsl#8
	orr	r3,r3,#0x80000000
	mov	r4,#0			@ r3,r4 = mantissa

	stmia	r0,{r2-r5}
	b	fastfpe_next
	
CPDT_ls_e0:
	movs	r3,r1,lsl#9
	beq	CPDT_load_zero
	
	mov	r5,#-127

CPDT_ls_e0_norm:
	tst	r3,#0x80000000
	subeq	r5,r5,#1
	moveq	r3,r3,lsl#1
	beq	CPDT_ls_e0_norm
	
	mov	r4,#0
	stmia	r0,{r2-r5}
	b	fastfpe_next
	
CPDT_ls_e255:
	mov	r3,r1,lsl#8
	bics	r3,r3,#0x80000000
	orreq	r3,r3,#0x80000000	// set MSB for inf
	mov	r4,#0
	mov	r5,#0x7fffffff
	stmia	r0,{r2-r5}
	b	fastfpe_next	

CPDT_load_zero:
	mov	r3,#0
	mov	r4,#0
	mov	r5,#0x80000000
	stmia	r0,{r2-r5}
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPDT_load_double
CPDT_load_double:
	ldr	r1,[r6]
	ldr	r6,[r6,#4]
	
	and	r2,r1,#0x80000000	@ r2 = sign
	
	mov	r5,r1,lsr#20
	bics	r5,r5,#0x800
	beq	CPDT_ld_e0		@ exponent = 0; zero/denormalized
	add	r4,r5,#1
	teq	r4,#2048
	beq	CPDT_ld_e2047		@ exponent = 2047; infinity/NaN
	
	add     r5,r5,#1
	sub	r5,r5,#1024		@ r5 = exponent, remove normalized bias
	
	mov	r3,r1,lsl#11
	orr	r3,r3,#0x80000000
	orr	r3,r3,r6,lsr #21
	mov	r4,r6,lsl#11		@ r3,r4 = mantissa

	stmia	r0,{r2-r5}
	b	fastfpe_next
	
CPDT_ld_e0:
	mov	r3,r1,lsl#12
	orr	r3,r3,r6,lsr#20
	movs	r4,r6,lsl#12
	teqeq	r3,#0
	beq	CPDT_load_zero
	
	mov	r5,#1
	sub	r5,r5,#1024
	
CPDT_ld_e0_norm:
	tst	r3,#0x80000000
	bne	CPDT_ld_e0_norm_end
	sub	r5,r5,#1
	movs	r4,r4,lsl#1
	adc	r3,r3,r3
	b	CPDT_ld_e0_norm
CPDT_ld_e0_norm_end:	
	stmia	r0,{r2-r5}
	b	fastfpe_next

CPDT_ld_e2047:
	mov	r3,r1,lsl#11
	orr	r3,r3,r6,lsr #21
	bic	r3,r3,#0x80000000
	mov	r4,r6,lsl#11		@ r3,r4 = mantissa
	orrs	r5,r3,r4
	orreq	r3,r3,#0x80000000	// set MSB fo inf
	mov	r5,#0x7fffffff
	stmia	r0,{r2-r5}
	b	fastfpe_next
	
/*---------------------------------------------------------------------------*/

	.globl	CPDT_load_extended
CPDT_load_extended:
	ldr	r1,[r6]
	ldr	r3,[r6,#4]
	ldr	r4,[r6,#8]
	
	and	r2,r1,#0x8000
	mov	r2,r2,lsl#16
	mov	r5,r1,lsl#17
	movs	r5,r5,lsr#17
	beq	CPDT_le_e0
	add	r1,r5,#1
	teq	r1,#32768
	beq	CPDT_le_e32767
	
	add	r5,r5,#1
	sub	r5,r5,#16384
	
	stmia	r0,{r2-r5}
	b	fastfpe_next
	
CPDT_le_e0:
	teq	r3,#0
	teqeq	r4,#0
	beq	CPDT_load_zero
	
	mov	r5,#2
	sub	r5,r5,#16384
	b	CPDT_ld_e0_norm
	
CPDT_le_e32767:
	mov	r5,#0x7fffffff
	stmia	r0,{r2-r5}
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPDT_load_decimal
CPDT_load_decimal:
	
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPDT_store_single
CPDT_store_single:
	ldmia	r0,{r1-r4}
	
	cmp	r4,#-127
	ble	CPDT_ss_e0
	cmp	r4,#128
	bge	CPDT_ss_e255

	add	r4,r4,#127
	orr	r1,r1,r4,lsl#23
	
	bic	r2,r2,#0x80000000
	orr	r1,r1,r2,lsr#8

	str	r1,[r6]
	b	fastfpe_next

CPDT_ss_e0:
	cmp	r4,#-150
	ble	CPDT_ss_zero
	
	add	r4,r4,#126
	rsb	r4,r4,#0
	mov	r2,r2,lsr r4
	
	orr	r1,r1,r2,lsr#8
	
CPDT_ss_zero:
	str	r1,[r6]
	b	fastfpe_next

CPDT_ss_e255:
	orr	r1,r1,#0x7f000000
	orr	r1,r1,#0x00800000
	cmp	r4,#0x7fffffff
	movne	r2,#0
	movne	r3,#0
	bic	r2,r2,#0x80000000
	orrs	r4,r3,r2,lsl#24		// only bits not stored in single
	bne	CPDT_ss_nan_special	// NaN must not become Inf
CPDT_ss_nan_back:
	orr	r1,r1,r2,lsr#8
	str	r1,[r6]
	b	fastfpe_next

CPDT_ss_nan_special:
	cmp	r2,#1<<8
	movlt	r2,#1<<8
	b	CPDT_ss_nan_back

/*---------------------------------------------------------------------------*/

	.globl	CPDT_store_double
CPDT_store_double:
	ldmia	r0,{r1-r4}
	
	cmp	r4,#1024		@ this check has to be first, or
	bge	CPDT_sd_e2047		@ overflow can occur on second !
	add	r0,r4,#3
	cmp	r0,#-1023+3		@ cmp with -1023
	ble	CPDT_sd_e0

	sub	r4,r4,#1
	add	r4,r4,#1024
	orr	r1,r1,r4,lsl#20
	
	bic	r2,r2,#0x80000000
	orr	r1,r1,r2,lsr#11
	
	mov	r2,r2,lsl#21
	orr	r2,r2,r3,lsr#11
	
	stmia	r6,{r1,r2}
	b	fastfpe_next

CPDT_sd_e0:
	add	r0,r4,#1075-1024
	cmp	r0,#-1024
	ble	CPDT_sd_zero
	
	add	r4,r4,#1024
	sub	r4,r4,#2
CPDT_sd_unnormalize:
	movs	r2,r2,lsr#1
	mov	r3,r3,rrx
	adds	r4,r4,#1
	bne	CPDT_sd_unnormalize
	
	orr	r1,r1,r2,lsr#11
	mov	r2,r2,lsl#21
	orr	r2,r2,r3,lsr#11
	
	stmia	r6,{r1,r2}
	b	fastfpe_next

CPDT_sd_zero:
	mov	r2,#0
	stmia	r6,{r1,r2}
	b	fastfpe_next

CPDT_sd_e2047:
	orr	r1,r1,#0x7f000000
	orr	r1,r1,#0x00f00000
	cmp	r4,#0x7fffffff
	movne	r2,#0
	movne	r3,#0
	movs	r5,r3,lsl#21		// only bits not stored in double !
	bne	CPDT_sd_nan_special
CPDT_sd_nan_back:
	orr	r1,r1,r2,lsr#11
	mov	r2,r2,lsl#21
	orr	r2,r2,r3,lsr#11
	stmia	r6,{r1,r2}
	b	fastfpe_next

CPDT_sd_nan_special:
	bics	r2,r2,#0x80000000
	bne	CPDT_sd_nan_back
	cmp	r3,#1<<11
	movlt	r3,#1<<11
	b	CPDT_sd_nan_back

/*---------------------------------------------------------------------------*/

	.globl	CPDT_store_extended
CPDT_store_extended:
	ldmia	r0,{r1-r4}
	
	cmp	r4,#16384		@ this check has to be first, or
	bge	CPDT_se_e32767		@ overflow can occur with second !
	add	r0,r4,#63
	cmp	r0,#-16383+63
	ble	CPDT_se_e0
	
	sub	r4,r4,#1
	add	r4,r4,#16384
	orr	r1,r4,r1,lsr#16
	
	stmia	r6,{r1-r3}
	b	fastfpe_next

CPDT_se_e0:
	add	r0,r4,#16446-16384
	cmp	r0,#-16384
	ble	CPDT_se_zero
	
	add	r4,r4,#16384
	sub	r4,r4,#2
CPDT_se_unnormalize:
	movs	r2,r2,lsr#1
	mov	r3,r3,rrx
	adds	r4,r4,#1
	bne	CPDT_se_unnormalize

	mov	r1,r1,lsr#16
	stmia	r6,{r1-r3}
	b	fastfpe_next
	
CPDT_se_zero:
	mov	r1,r1,lsr#16
	mov	r2,#0
	mov	r3,#0
	stmia	r6,{r1-r3}
	b	fastfpe_next

CPDT_se_e32767:
	cmp	r4,#0x7fffffff
	movne	r2,#0
	movne	r3,#0
	mov	r1,r1,lsr#16
	orr	r1,r1,#0x00007f00
	orr	r1,r1,#0x000000ff
	stmia	r6,{r1-r3}
	b	fastfpe_next
		
/*---------------------------------------------------------------------------*/

	.globl	CPDT_store_decimal
CPDT_store_decimal:

	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPDT_sfm
CPDT_sfm_loop:
	add	r0,r0,#1<<12
	and	r0,r0,#7<<12
CPDT_sfm:
	add	r7,r10,r0,lsr#8
	ldmia	r7,{r2-r5}
	bic	r3,r3,#0x80000000
	orr	r3,r3,r2
	stmia	r6!,{r3-r5}

	subs	r1,r1,#1
	bne	CPDT_sfm_loop
	b	fastfpe_next
	
/*---------------------------------------------------------------------------*/

	.globl	CPDT_lfm
CPDT_lfm_loop:
	add	r0,r0,#1<<12
	and	r0,r0,#7<<12
CPDT_lfm:
	add	r7,r10,r0,lsr#8
	ldmia	r6!,{r3-r5}
	and	r2,r3,#0x80000000
	cmp	r5,#0x80000000		// check if the number was 0
	cmpne	r5,#0x7fffffff		// or inf/NaN
	biceq	r3,r3,#0x80000000	// yes -> clear mantissa MSB
	orrne	r3,r3,#0x80000000	// no -> set mantissa MSB
	stmia	r7,{r2-r5}

	subs	r1,r1,#1
	bne	CPDT_lfm_loop
	b	fastfpe_next
	
/*---------------------------------------------------------------------------*/
