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

	.text
	.globl	CPRT_flt
CPRT_flt:
	add	r0,r13,r0,lsr#10
	ldr	r2,[r0]
	mov	r0,r1
	mov	r3,#0
	cmp	r2,#0
	beq	CPRT_flt_zero

	ldr	r6,=round_table
	and	r5,r4,#0x000000e0
	and	r4,r4,#0x00080000
	orr	r5,r5,r4,lsr#11
	ldr	r6,[r6,r5,lsr#3]	// address of rounding function
	
	ands	r1,r2,#0x80000000
	rsbne	r2,r2,#0
	mov	r4,#31
	
	cmp	r2,#0x00010000
	movcc	r2,r2,lsl#16
	subcc	r4,r4,#16
	
	cmp	r2,#0x01000000
	movcc	r2,r2,lsl#8
	subcc	r4,r4,#8
	
	cmp	r2,#0x10000000
	movcc	r2,r2,lsl#4
	subcc	r4,r4,#4
	
	cmp	r2,#0x40000000
	movcc	r2,r2,lsl#2
	subcc	r4,r4,#2
	
	cmp	r2,#0x80000000
	movcc	r2,r2,lsl#1
	subcc	r4,r4,#1

	mov	r5,#0
	ldr	r14,=fastfpe_next
	mov	pc,r6

CPRT_flt_zero:
	mov	r1,#0
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
	
/*---------------------------------------------------------------------------*/

	.globl	CPRT_fix
CPRT_fix:
	ldmia	r2,{r1,r2,r3,r5}
	bl	CPDO_rnd_core
	
	add	r0,r13,r0,lsr#10
	cmp	r5,#0
	blt	CPRT_fix_zero
	cmp	r5,#30
	bgt	CPRT_fix_overflow

CPRT_fix_no_overflow:	
	rsb	r5,r5,#31
	mov	r2,r2,lsr r5
	tst	r1,#0x80000000
	rsbne	r2,r2,#0
CPRT_fix_zero_back:
	str	r2,[r0]
	ldr	r1,[r10,#128]
	orr	r1,r1,r4	// set flags possibly caused by rounding
	str	r1,[r10,#128]
	b	fastfpe_next

CPRT_fix_zero:
	mov	r2,#0
	b	CPRT_fix_zero_back

CPRT_fix_overflow:
	cmp	r1,#0x80000000	// -2^31 is not exactly an overflow ...
	cmpeq	r2,#0x80000000
	cmpeq	r5,#31
	beq	CPRT_fix_no_overflow

	mov	r2,#0x80000000
	tst	r1,#0x80000000
	subeq	r2,r2,#1
	str	r2,[r0]

	ldr	r1,[r10,#128]
	orr	r1,r1,#1	// set invalid operation flag
	str	r1,[r10,#128]
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPRT_wfs
CPRT_wfs:
        ldr     r0,[r13,r0,lsr#10]
        str     r0,[r10,#128]
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPRT_rfs
CPRT_rfs:
        ldr     r1,[r10,#128]
        bic     r1,r1,#0xff000000
        orr     r1,r1,#0x02000000       @ Software Emulation, not Acorn FPE
        str     r1,[r13,r0,lsr#10]
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPRT_cmf
CPRT_cmf:
	ldmia	r1,{r1,r3,r5,r7}
	ldmia	r2,{r2,r4,r6,r8}

CPRT_cmf_e:
	ldr	r0,[r13,#16*4]
	bic	r0,r0,#0xf0000000
	
	cmp	r7,#0x7fffffff
	beq	CPRT_cmf_nan1
CPRT_cmf_nixnan1:
	cmp	r8,#0x7fffffff
	beq	CPRT_cmf_nan2
CPRT_cmf_nixnan2:

	cmp	r1,r2
	beq	CPRT_cmf_equalsign
	b	CPRT_cmf_signx

CPRT_cmf_nan1:
	orrs	r11,r5,r3,lsl#1		// ignore MSB
	beq	CPRT_cmf_nixnan1
	b	CPRT_cmf_unordered

CPRT_cmf_nan2:
	orrs	r11,r6,r4,lsl#1		// ignore MSB
	beq	CPRT_cmf_nixnan2
	b	CPRT_cmf_unordered

CPRT_cmf_equalsign:
	cmp	r7,r8
	beq	CPRT_cmf_equalexponent
	bgt	CPRT_cmf_sign
	b	CPRT_cmf_signb	

CPRT_cmf_equalexponent:
	cmp	r3,r4
	cmpeq	r5,r6
	beq	CPRT_cmf_equal
	bhi	CPRT_cmf_sign
	b	CPRT_cmf_signb

CPRT_cmf_signx:
	teq	r7,#0x80000000
	teqeq	r8,#0x80000000
	beq	CPRT_cmf_equal
CPRT_cmf_sign:
	tst	r1,#0x80000000
	orreq	r0,r0,#0x20000000	// PSR carry
	orrne	r0,r0,#0x80000000	// PSR negative
	str	r0,[r13,#16*4]
	b	fastfpe_next

CPRT_cmf_signb:
	tst	r1,#0x80000000
	orrne	r0,r0,#0x20000000	// PSR carry
	orreq	r0,r0,#0x80000000	// PSR negative
	str	r0,[r13,#16*4] 
	b	fastfpe_next

CPRT_cmf_equal:
	orr	r0,r0,#0x60000000	// PSR carry, zero
	str	r0,[r13,#16*4]
	b	fastfpe_next

CPRT_cmf_unordered:
	ldr	r1,[r10,#128]
	orr	r1,r1,#1		// set invalid operation flag
	str	r1,[r10,#128]

	tst	r0,#1<<12		// FPSR AC bit set ?
	orrne	r0,r0,#0x20000000	// PSR carry
	orr	r0,r0,#0x10000000	// PSR overflow
        str     r0,[r13,#16*4]

        b       fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPRT_cnf
CPRT_cnf:
	ldmia	r1,{r1,r3,r5,r7}
	ldmia	r2,{r2,r4,r6,r8}
	eor	r2,r2,#0x80000000
	b	CPRT_cmf_e

/*---------------------------------------------------------------------------*/
