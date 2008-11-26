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

The parameters to these functions are r0=destination pointer, r1 and r2
source pointers. r4 is the instruction. They may use r0-r8, r11. They return
to r14, which contains the address of a rounding function. The rounding
function expects r0=address, r1-r4=sign, mantissa high, mantissa low,
exponent, r5=additional lower mantissa bits.

CPDO_rnf_core expects the return address in r14.
*/

/*---------------------------------------------------------------------------*/

	.globl	CPDO_adf
CPDO_adf:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

	cmp	r11,#0x7fffffff
	cmpne	r11,#0x80000000
	cmpne	r4,#0x7fffffff
	cmpne	r4,#0x80000000
	beq	CPDO_adf_extra
	
	cmp	r1,r6
	bne	CPDO_suf_s

CPDO_adf_s:
	subs	r6,r4,r11
	bmi	CPDO_adf_normalize1st

CPDO_adf_normalize2nd:
	cmp	r6,#32
	ble	CPDO_adf_normalize2nd_1
	cmp	r6,#64
	bgt	CPDO_adf_normalize2nd_3

CPDO_adf_normalize2nd_2:
	sub	r6,r6,#32
	rsb	r11,r6,#32
	mov	r5,r8,lsr r6
	add	r5,r5,r7,lsl r11
	movs	r11,r8,lsl r11
	orrne	r5,r5,#1
	mov	r8,r7,lsr r6
	mov	r7,#0
	b	CPDO_adf_add

CPDO_adf_normalize2nd_1:
	rsb	r11,r6,#32
	mov	r5,r8,lsl r11
	mov	r8,r8,lsr r6
	add	r8,r8,r7,lsl r11
	mov	r7,r7,lsr r6
	b	CPDO_adf_add

CPDO_adf_normalize2nd_3:
	mov	r5,#0x40000000
	mov	pc,r14

CPDO_adf_normalize1st:
	mov	r4,r11
	rsb	r6,r6,#0
	cmp	r6,#32
	ble	CPDO_adf_normalize1st_1
	cmp	r6,#64
	bgt	CPDO_adf_normalize1st_3

CPDO_adf_normalize1st_2:
	sub	r6,r6,#32
	rsb	r11,r6,#32
	mov	r5,r3,lsr r6
	add	r5,r5,r2,lsl r11
	movs	r11,r3,lsl r11
	orrne	r5,r5,#1
	mov	r3,r2,lsr r6
	mov	r2,#0
	b	CPDO_adf_add

CPDO_adf_normalize1st_1:
	rsb	r11,r6,#32
	mov	r5,r3,lsl r11
	mov	r3,r3,lsr r6
	add	r3,r3,r2,lsl r11
	mov	r2,r2,lsr r6
	b	CPDO_adf_add

CPDO_adf_normalize1st_3:
	mov	r5,#0x40000000
	mov	r2,r7
	mov	r3,r8
	mov	pc,r14

CPDO_adf_add:
	adds	r3,r3,r8
	adcs	r2,r2,r7
	bcc	CPDO_adf_add_no_overflow

	movs	r2,r2,rrx
	movs	r3,r3,rrx
	movs	r5,r5,rrx
	orrcs	r5,r5,#1
	add	r4,r4,#1

CPDO_adf_add_no_overflow:
	mov	pc,r14

CPDO_adf_extra:
	cmp	r4,#0x7fffffff
	beq	CPDO_adf_1st_infnan
	cmp	r11,#0x7fffffff
	beq	CPDO_adf_2nd_infnan
	cmp	r11,#0x80000000
	beq	CPDO_adf_2nd_0

CPDO_adf_1st_0:
	mov	r1,r6
	mov	r2,r7
	mov	r3,r8
	mov	r4,r11
	mov	r5,#0
	mov	pc,r14

CPDO_adf_2nd_0:
	cmp	r4,#0x80000000
	beq	CPDO_adf_both_0
	mov	r5,#0
	mov	pc,r14

CPDO_adf_both_0:
	cmp	r1,r6
	beq	CPDO_adf_both_0_equal_sign
	and	r5,r5,#0x00000060
	cmp	r5,#0x00000040	// rounding mode M?
	moveq	r1,#0x80000000
	movne	r1,#0
CPDO_adf_both_0_equal_sign:
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14	

CPDO_adf_1st_infnan:
	cmp	r11,#0x7fffffff
	beq	CPDO_adf_both_infnan
CPDO_adf_1st_infnan_entry:
	orrs	r5,r3,r2,lsl#1	// ignore MSB
	moveq	pc,r14		// Inf
	tst	r2,#0x40000000
	movne	pc,r14		// QNaN
CPDO_adf_generate_qnan:
	mov	r1,#0x80000000
	mov	r2,#0x7fffffff
	mov	r3,#0xffffffff
	mov	r4,#0x7fffffff
	ldr	r5,[r10,#128]
	orr	r5,r5,#1	// set invalid operation flag
	str	r5,[r10,#128]
	mov	pc,r14

CPDO_adf_2nd_infnan:
	mov	r1,r6
	mov	r2,r7
	mov	r3,r8
	mov	r4,r11
	b CPDO_adf_1st_infnan_entry

CPDO_adf_both_infnan:
	orrs	r5,r3,r2,lsl#1		// ignore MSB
	beq	CPDO_adf_1st_inf
	orrs	r5,r8,r7,lsl#1		// ignore MSB
	beq	CPDO_adf_2nd_inf
	tst	r2,#0x40000000
	tstne	r7,#0x40000000
	beq	CPDO_adf_generate_qnan	// at least one is SNaN
	orrs	r5,r3,r2,lsl#1		// ignore MSB, FIXME! what is going on here?
	moveq	r1,r6			// if first is not NaN
	moveq	r2,r7			// give second as result
	moveq	r3,r8
	mov	pc,r14

CPDO_adf_1st_inf:
	orrs	r5,r8,r7,lsl#1		// ignore MSB
	beq	CPDO_adf_both_inf
	tst	r7,#0x40000000
	beq	CPDO_adf_generate_qnan
	mov	r1,r6			//if 2nd no SNaN return 2nd
	mov	r2,r7
	mov	r3,r8
	mov	pc,r14

CPDO_adf_2nd_inf:
	tst	r2,#0x40000000
	beq	CPDO_adf_generate_qnan
	mov	pc,r14			// if 1st no SNaN just return it

CPDO_adf_both_inf:
	cmp	r1,r6
	bne	CPDO_adf_generate_qnan	// signs of both inf are different
	mov	pc,r14

/*--------------------------------------------------------------------------*/

	.globl	CPDO_suf
CPDO_suf:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

CPDO_suf_l:
	cmp	r11,#0x7fffffff
	cmpne	r11,#0x80000000
	cmpne	r4,#0x7fffffff
	cmpne	r4,#0x80000000
	beq	CPDO_suf_extra
	
	cmp	r1,r6
	bne	CPDO_adf_s

CPDO_suf_s:
	subs	r6,r4,r11
	blt	CPDO_suf_normalize1st
	bgt	CPDO_suf_normalize2nd
	cmp	r2,r7
	cmpeq	r3,r8
	beq	CPDO_suf_zero
	mov	r5,#0
	bcs	CPDO_suf_sub_1stbigger
	eor	r1,r1,#0x80000000
	b	CPDO_suf_sub_2ndbigger

CPDO_suf_normalize2nd:
	cmp	r6,#32
	ble	CPDO_suf_normalize2nd_1
	cmp	r6,#64
	bgt	CPDO_suf_normalize2nd_3

CPDO_suf_normalize2nd_2:
	sub	r6,r6,#32
	rsb	r11,r6,#32
	mov	r5,r8,lsr r6
	add	r5,r5,r7,lsl r11
	movs	r11,r8,lsl r11
	orrne	r5,r5,#1
	mov	r8,r7,lsr r6
	mov	r7,#0
	b	CPDO_suf_sub_1stbigger

CPDO_suf_normalize2nd_1:
	rsb	r11,r6,#32
	mov	r5,r8,lsl r11
	mov	r8,r8,lsr r6
	add	r8,r8,r7,lsl r11
	mov	r7,r7,lsr r6
	b	CPDO_suf_sub_1stbigger

CPDO_suf_normalize2nd_3:
	sub	r6,r6,#64
	cmp	r6,#32
	bge	CPDO_suf_normalize2nd_4
	rsb	r11,r6,#32
	mov	r5,r7,lsr r6
	orrs	r11,r8,r7,lsl r11
	orrne	r5,r5,#1
	mov	r7,#0
	mov	r8,#0
	b	CPDO_suf_sub_1stbigger

CPDO_suf_normalize2nd_4:
	mov	r5,#1
	mov	r7,#0
	mov	r8,#0
	b	CPDO_suf_sub_1stbigger

CPDO_suf_normalize1st:
	eor	r1,r1,#0x80000000
	mov	r4,r11
	rsb	r6,r6,#0
	cmp	r6,#32
	ble	CPDO_suf_normalize1st_1
	cmp	r6,#64
	bgt	CPDO_suf_normalize1st_3

CPDO_suf_normalize1st_2:
	sub	r6,r6,#32
	rsb	r11,r6,#32
	mov	r5,r3,lsr r6
	add	r5,r5,r2,lsl r11
	movs	r11,r3,lsl r11
	orrne	r5,r5,#1
	mov	r3,r2,lsr r6
	mov	r2,#0
	b	CPDO_suf_sub_2ndbigger

CPDO_suf_normalize1st_1:
	rsb	r11,r6,#32
	mov	r5,r3,lsl r11
	mov	r3,r3,lsr r6
	add	r3,r3,r2,lsl r11
	mov	r2,r2,lsr r6
	b	CPDO_suf_sub_2ndbigger

CPDO_suf_normalize1st_3:
	sub	r6,r6,#64
	cmp	r6,#32
	bge	CPDO_suf_normalize1st_4
	rsb	r11,r6,#32
	mov	r5,r2,lsr r6
	orrs	r11,r3,r2,lsl r11
	orrne	r5,r5,#1
	mov	r2,#0
	mov	r3,#0
	b	CPDO_suf_sub_2ndbigger

CPDO_suf_normalize1st_4:
	mov	r5,#1
	mov	r2,#0
	mov	r3,#0
	b	CPDO_suf_sub_2ndbigger

CPDO_suf_sub_1stbigger:
	rsbs	r5,r5,#0
	sbcs	r3,r3,r8
	sbcs	r2,r2,r7
	movmi	pc,r14
	b	CPDO_suf_norm

CPDO_suf_sub_2ndbigger:
	rsbs	r5,r5,#0
	sbcs	r3,r8,r3
	sbcs	r2,r7,r2
	movmi	pc,r14

CPDO_suf_norm:
	teq	r2,#0		// normalize 32 bit
	bne	CPDO_suf_norm16
	teq	r3,#0		// normalize 64 bit
	bne	CPDO_suf_norm32
	mov	r2,r5
	mov	r3,#0
	mov	r5,#0
	sub	r4,r4,#64
	mov	pc,r14
CPDO_suf_norm32:
	mov	r2,r3
	mov	r3,r5
	mov	r5,#0
	sub	r4,r4,#32
CPDO_suf_norm16:
	cmp	r2,#0x00010000	// normalize 16 bit
	bcs	CPDO_suf_norm8
	mov	r2,r2,lsl#16
	orr	r2,r2,r3,lsr#16
	mov	r3,r3,lsl#16
	orr	r3,r3,r5,lsr#16
	mov	r5,r5,lsl#16
	sub	r4,r4,#16
CPDO_suf_norm8:
	cmp	r2,#0x01000000	// normalize 8 bit
	bcs	CPDO_suf_norm4
	mov	r2,r2,lsl#8
	orr	r2,r2,r3,lsr#24
	mov	r3,r3,lsl#8
	orr	r3,r3,r5,lsr#24
	mov	r5,r5,lsl#8
	sub	r4,r4,#8
CPDO_suf_norm4:
	cmp	r2,#0x10000000	// normalize 4 bit
	bcs	CPDO_suf_norm2
	mov	r2,r2,lsl#4
	orr	r2,r2,r3,lsr#28
	mov	r3,r3,lsl#4
	orr	r3,r3,r5,lsr#28
	mov	r5,r5,lsl#4
	sub	r4,r4,#4
CPDO_suf_norm2:
	cmp	r2,#0x40000000	// normalize 2 bit
	bcs	CPDO_suf_norm1
	mov	r2,r2,lsl#2
	orr	r2,r2,r3,lsr#30
	mov	r3,r3,lsl#2
	orr	r3,r3,r5,lsr#30
	mov	r5,r5,lsl#2
	sub	r4,r4,#2
CPDO_suf_norm1:
	cmp	r2,#0x80000000	// normalize 1 bit
	bcs	CPDO_suf_norme
	mov	r2,r2,lsl#1
	orr	r2,r2,r3,lsr#31
	mov	r3,r3,lsl#1
	orr	r3,r3,r5,lsr#31
	mov	r5,r5,lsl#1
	sub	r4,r4,#1
CPDO_suf_norme:
	mov	pc,r14

CPDO_suf_zero:
	and	r5,r5,#0x00000060
	cmp	r5,#0x00000040	// rounding mode M?
	moveq	r1,#0x80000000
	movne	r1,#0
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14	

CPDO_suf_extra:				// nearly the same as with adf
	cmp	r11,#0x7fffffff		// the only thing we need to do is
	bne	CPDO_suf_extra_sign	// to invert the second sign if
	orrnes	r5,r8,r7,lsl#1		// it is not a NaN, ignore MSB
	bne	CPDO_adf_extra
CPDO_suf_extra_sign:
	eor	r6,r6,#0x80000000
	b	CPDO_adf_extra

/*---------------------------------------------------------------------------*/

	.globl CPDO_rsf
CPDO_rsf:
	ldmia	r1,{r6,r7,r8,r11}
	ldmia	r2,{r1,r2,r3,r4}
	b	CPDO_suf_l

/*---------------------------------------------------------------------------*/

	.globl	CPDO_muf
CPDO_muf:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

	cmp	r11,#0x7fffffff
	cmpne	r4,#0x7fffffff
	beq	CPDO_muf_extra
	eor	r1,r1,r6		// sign
	cmp	r11,#0x80000000
	cmpne	r4,#0x80000000
	beq	CPDO_muf_zero
	
	add	r4,r4,r11		// exponent
#if 0

#define	x4	r0
#define	x3	r1
#define	x2	r4
#define x1	r6
#define	y4	r9
#define	y3	r7
#define	y2	r10
#define	y1	r8
#define z4	r2
#define z3	r3
#define	z2	r5
#define	z1	r11
#define tmp	r12

	stmdb	r13!, {r0, r1, r4, r9, r10, r12}
	mov	x4,r2,lsr#16
	bic	x3,r2,x4,lsl#16
	mov	x2,r3,lsr#16
	bic	x1,r3,x2,lsl#16
	mov	y4,r7,lsr#16
	bic	y3,r7,y4,lsl#16
	mov	y2,r8,lsr#16
	bic	y1,r8,y2,lsl#16
	mul	z1,x1,y1
	mul	tmp,x1,y2
	mov	z2,#0
	adds	z1,z1,tmp,lsl#16
	adc	z2,z2,tmp,lsr#16
	mul	tmp,x2,y1
	adds	z1,z1,tmp,lsl#16
	adcs	z2,z2,tmp,lsr#16
	mul	tmp,x1,y4
	mov	z3,#0
	adds	z2,z2,tmp,lsl#16
	adc	z3,z3,tmp,lsr#16
	mul	tmp,x2,y3
	adds	z2,z2,tmp,lsl#16
	adc	z3,z3,tmp,lsr#16
	mul	tmp,x3,y2
	adds	z2,z2,tmp,lsl#16
	adc	z3,z3,tmp,lsr#16
	mul	tmp,x4,y1
	adds	z2,z2,tmp,lsl#16
	adc	z3,z3,tmp,lsr#16
	mul	tmp,x3,y4
	mul	z4,x4,y4
	adds	z3,z3,tmp,lsl#16
	adc	z4,z4,tmp,lsr#16
	mul	tmp,x4,y3
	adds	z3,z3,tmp,lsl#16
	adc	z4,z4,tmp,lsr#16
	mul	tmp,x1,y3
	adds	z2,z2,tmp
	mul	tmp,x2,y4
	adcs	z3,z3,tmp
	mul	tmp,x2,y2
	adc	z4,z4,#0
	adds	z2,z2,tmp
	mul	tmp,x3,y3
	adcs	z3,z3,tmp
	mul	tmp,x3,y1
	adc	z4,z4,#0
	adds	z2,z2,tmp
	teq	z1,#0
	orrne	z2,z2,#1	// z1 must not be lost for rounding
	mul	tmp,x4,y2
	adcs	z3,z3,tmp
	adcs	z4,z4,#0
	ldmia	r13!, {r0, r1, r4, r9, r10, r12}
#else

#define x32     r2
#define x10     r3
#define y32     r7
#define y10     r8
#define z3      r0
#define z2      r1
#define z1      r4
#define z0      r6
#define v1      r9
#define v0      r11
#define tmp     r5

	stmdb	r13!,{r0,r1,r4,r9}

        mov     z3,x32,lsr#16
        bic     z2,x32,z3,lsl#16
        movs    v1,y32,lsr#16
        bic     v0,y32,v1,lsl#16

        mul     tmp,z3,v0
        mul     z3,v1,z3
        mulne   v1,z2,v1
        mul     z2,v0,z2
        adds    z2,z2,tmp,lsl#16
        adc     z3,z3,tmp,lsr#16
        adds    z2,z2,v1,lsl#16
        adc     z3,z3,v1,lsr#16

        mov     z1,x10,lsr#16
        bic     z0,x10,z1,lsl#16
        movs    v1,y10,lsr#16
        bic     v0,y10,v1,lsl#16

        mul     tmp,z1,v0
        mul     z1,v1,z1
        mulne   v1,z0,v1
        mul     z0,v0,z0
        adds    z0,z0,tmp,lsl#16
        adc     z1,z1,tmp,lsr#16
        adds    z0,z0,v1,lsl#16
        adc     z1,z1,v1,lsr#16

        adds    z2,z2,z1        // z3 is max. 0xfffffffe
        adc     z3,z3,#0        // so this trick is possible
        adds    z1,z2,z0        // to save one addition
        adcs    z2,z2,z3
        adc     z3,z3,#0

        subs    x10,x32,x10
        mov     v0,#0
        mov     v1,v0,rrx

        sublo   v0,y32,y10
        subnes  y10,y10,y32

        orreq   v1,v1,#1<<31
        eorcs   v1,v1,#1<<31
        subcc   v0,v0,x10

        movs    x32,x10,lsr#16
        bic     x10,x10,x32,lsl#16
        mov     y32,y10,lsr#16
        bic     y10,y10,y32,lsl#16

        mul     tmp,x10,y10
        mla     v0,x32,y32,v0
        mulne   x32,y10,x32
        adds    tmp,tmp,x32,lsl#16
        adc     v0,v0,x32,lsr#16
        mul     y32,x10,y32
        adds    tmp,tmp,y32,lsl#16
        adc     v0,v0,y32,lsr#16
        adds    r5,z1,tmp
        adcs    r3,z2,v0
        adc     r2,z3,v1,asr#31

	teq	z0,#0
	orrne	r5,r5,#1	// z0 must not be lost for rounding
	cmp	r2,#0
	
	ldmia	r13!,{r0,r1,r4,r9}
#endif

	bpl	CPDO_muf_norm
	add	r4,r4,#1
	mov	pc,r14

CPDO_muf_norm:
	adds	r5,r5,r5
	adcs	r3,r3,r3
	adc	r2,r2,r2
	mov	pc,r14

CPDO_muf_extra:
	cmp	r4,#0x7fffffff
	beq	CPDO_muf_1st_infnan
CPDO_muf_2nd_infnan:
	orrs	r5,r8,r7,lsl#1		// ignore MSB
	bne	CPDO_muf_2nd_nan
	cmp	r4,#0x80000000
	beq	CPDO_muf_generate_qnan
	mov	r2,r7			// copy MSB
	mov	r3,#0
	mov	r4,#0x7fffffff
	eor	r1,r1,r6
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14	

CPDO_muf_1st_infnan:
	cmp	r11,#0x7fffffff
	beq	CPDO_muf_both_infnan
	orrs	r5,r3,r2,lsl#1		// ignore MSB
	bne	CPDO_muf_1st_nan
	cmp	r11,#0x80000000
	beq	CPDO_muf_generate_qnan
//	mov	r4,#0x7fffffff
	eor	r1,r1,r6
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_muf_both_infnan:
	orrs	r5,r3,r2,lsl#1			// ignore MSB
	beq	CPDO_muf_both_infnan_1st_inf
	orrs	r5,r8,r7,lsl#1			// ignore MSB
	beq	CPDO_muf_both_infnan_2nd_inf
	tst	r2,#0x40000000
	tstne	r7,#0x40000000
	beq	CPDO_muf_generate_qnan
	mov	pc,r14

CPDO_muf_both_infnan_1st_inf:
	orrs	r5,r8,r7,lsl#1		// ignore MSB
	beq	CPDO_muf_both_inf
	b	CPDO_muf_2nd_nan	

CPDO_muf_both_infnan_2nd_inf:
	b	CPDO_muf_1st_nan

CPDO_muf_both_inf:
	eor	r1,r1,r6
	orr	r2,r2,r7		// copy both MSB
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_muf_zero:
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_muf_1st_nan:
	tst	r2,#0x40000000
	beq	CPDO_muf_generate_qnan
	mov	pc,r14

CPDO_muf_2nd_nan:
	tst	r7,#0x40000000
	beq	CPDO_muf_generate_qnan
	mov	r1,r6
	mov	r2,r7
	mov	r3,r8
	mov	r4,r11
	mov	pc,r14

CPDO_muf_generate_qnan:
	mov	r1,#0x80000000
	mov	r2,#0x7fffffff
	mov	r3,#0xffffffff
	mov	r4,#0x7fffffff
	ldr	r5,[r10,#128]
	orr	r5,r5,#1
	str	r5,[r10,#128]
	mov	pc,r14

/*---------------------------------------------------------------------------*/

	.globl	CPDO_muf_M
CPDO_muf_M:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

	cmp	r11,#0x7fffffff
	cmpne	r4,#0x7fffffff
	beq	CPDO_muf_extra
	eor	r1,r1,r6		// sign
	cmp	r11,#0x80000000
	cmpne	r4,#0x80000000
	beq	CPDO_muf_zero
	
	add	r4,r4,r11		// exponent
#if 0
	umull	r11,r5,r3,r8		// r5|r11 = lo1*lo2
	teq	r11,#0
	orrne	r5,r5,#0x00000001	// r11 must not be lost for rounding
	umull	r11,r6,r8,r2		// r6|r11 = lo2*hi1
	umull	r8,r2,r7,r2		// r2|r8 = hi1*hi2
	umull	r12,r3,r7,r3		// r3|r12 = lo1*hi2
	adds	r5,r5,r12
	adcs	r3,r3,r6
	adc	r2,r2,#0
	adds	r5,r5,r11
	adcs	r3,r3,r8
	adcs	r2,r2,#0
#else
	umull	r12,r11,r2,r7
	umull	r2,r6,r8,r2
	umull	r8,r5,r3,r8
	adds	r5,r5,r2
	adcs	r12,r12,r6
	adc	r11,r11,#0
	umull	r7,r6,r3,r7
	adds	r5,r5,r7
	adcs	r3,r12,r6
	adc	r2,r11,#0
	teq	r8,#0
	orrne	r5,r5,#1	// r8 must not be lost for rounding
	cmp	r2,#0	
#endif
	bpl	CPDO_muf_norm
	add	r4,r4,#1
	mov	pc,r14

/*---------------------------------------------------------------------------*/

CPDO_infnan_1:
	stmia	r0,{r1,r3,r5,r7}
	b	fastfpe_next

CPDO_infnan_2:
	stmia	r0,{r2,r4,r6,r8}
	b	fastfpe_next
	
CPDO_nan_12:
	orr	r2,r3,r4
	b	CPDO_inf_1

CPDO_nan:
	mov	r2,#0x40000000		@ create non signalling NaN
	b	CPDO_inf_1
	
CPDO_inf:
	mov	r2,#0
CPDO_inf_1:
	mov	r3,#0
	mov	r4,#0x7fffffff
CPDO_store_1234:
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
	
CPDO_zero:
	mov	r1,#0
CPDO_zero_1:
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next

CPDO_muf_end:
	cmp	r8,#0x20000000
	bge	CPDO_inf
	cmp	r8,#0xe0000000
	ble	CPDO_zero_1
	stmia	r0,{r1,r2,r7,r8}
	b	fastfpe_next

/*---------------------------------------------------------------------------*/

	.globl	CPDO_dvf
CPDO_dvf:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

CPDO_dvf_l:
	cmp	r11,#0x7fffffff
	cmpne	r4,#0x7fffffff
	beq	CPDO_dvf_infnan
	eor	r1,r1,r6
	cmp	r11,#0x80000000
	cmpne	r4,#0x80000000
	beq	CPDO_dvf_zero

	sub	r4,r4,r11

#define x4      r11
#define x3      r7
#define x2      r12
#define x1      r8
#define y2      r14
#define y1      r9
#undef z3
#define z3      r4
#undef z2
#define z2      r5
#undef z1
#define z1      r6
#undef tmp
#define tmp     r10

	cmp	r2,r7
	cmpeq	r3,r8
	bcs	CPDO_dvf_no_normalize

	sub	r4,r4,#1
	stmdb	r13!,{r1,r4,r9,r10,r11,r14}
	mov	r4,r2,lsr#31
	mov	r5,r2,lsl#1
	orr	r5,r5,r3,lsr#31
	mov	r6,r3,lsl#1	// dividend
	b	CPDO_dvf_normalize_back

CPDO_dvf_no_normalize:
	stmdb	r13!,{r1,r4,r9,r10,r11,r14}
	mov	r4,#0
	mov	r5,r2
	mov	r6,r3		// dividend

CPDO_dvf_normalize_back:
	mov	r1,#0
	sub	r10,r1,r7,lsr#1
	mov	r11,#0x40000000

	.macro	inv_step
	adds	r11,r10,r11,lsl#1
	subcc	r11,r11,r10
	adc	r1,r1,r1
	.endm

	.rept	17
	inv_step
	.endr

	mov	r1,r1,lsl#15
	adds	r1,r1,#1<<15
	movcs	r1,#0xffffffff	// inverse
	mov	r1,r1,lsr#16

	mov	r2,#0
	mov	r3,#0		// clear result space

        mov     x4,r7,lsr#16
        bic     x3,r7,x4,lsl#16
        mov     x2,r8,lsr#16
        bic     x1,r8,x2,lsl#16	// split divisor for 16x16=32bit mul

CPDO_dvf_loop_entry:
	mov	r4,r4,lsl#16
	orrs	r4,r4,r5,lsr#16
	mov	r5,r5,lsl#16
	orr	r5,r5,r6,lsr#16
	mov	r6,r6,lsl#16  	// shift dividend left by 16

	bmi	CPDO_dvf_loop_negative
        mov     r10,r4,lsr#16
        mul     r9,r10,r1
        bic     r10,r4,r10,lsl#16
        mul     r10,r1,r10
        add     r9,r9,r10,lsr#16	//estimate 16 bits of result in r9

	mov	r2,r2,lsl#16
	orr	r2,r2,r3,lsr#16
	adds	r3,r9,r3,lsl#16		// shift result left by 16 and
	adc	r2,r2,#0		// add in new result bits

        mov     r9,r9,lsl#1
        mov     y2,r9,lsr#16
        bic     y1,r9,y2,lsl#16
        mul     tmp,x1,y1
        subs    z1,z1,tmp
        mul     tmp,x3,y1
        sbcs    z2,z2,tmp
        mul     tmp,x4,y2
        sbc     z3,z3,tmp
        mul     tmp,x2,y2
        subs    z2,z2,tmp
        sbc     z3,z3,#0
        mul     tmp,x2,y1
        subs    z1,z1,tmp,lsl#16
        sbcs    z2,z2,tmp,lsr#16
        sbc     z3,z3,#0
        mul     tmp,x1,y2
        subs    z1,z1,tmp,lsl#16
        sbcs    z2,z2,tmp,lsr#16
        sbc     z3,z3,#0
        mul     tmp,x4,y1
        subs    z2,z2,tmp,lsl#16
        sbc     z3,z3,tmp,lsr#16
        mul     tmp,x3,y2
        subs    z2,z2,tmp,lsl#16
        sbc     z3,z3,tmp,lsr#16	// subtract divisor * estimated result

	tst	r2,#0xff000000
	beq	CPDO_dvf_loop_entry

	b	CPDO_dvf_end_entry

CPDO_dvf_loop_negative:
        rsb     r14,r4,#0
        mov     r10,r14,lsr#16
        mul     r9,r10,r1
        bic     r10,r14,r10,lsl#16
        mul     r10,r1,r10
        add     r9,r9,r10,lsr#16	// estimate 16 bits of result in r9

	mov	r2,r2,lsl#16
	orr	r2,r2,r3,lsr#16
	rsbs	r3,r9,r3,lsl#16		// shift result left by 16 and
	sbc	r2,r2,#0		// add in new result bits

        mov     r9,r9,lsl#1
        mov     y2,r9,lsr#16
        bic     y1,r9,y2,lsl#16
        mul     tmp,x1,y1
        adds    z1,z1,tmp
        mul     tmp,x3,y1
        adcs    z2,z2,tmp
        mul     tmp,x4,y2
        adc     z3,z3,tmp
        mul     tmp,x2,y2
        adds    z2,z2,tmp
        adc     z3,z3,#0
        mul     tmp,x2,y1
        adds    z1,z1,tmp,lsl#16
        adcs    z2,z2,tmp,lsr#16
        adc     z3,z3,#0
        mul     tmp,x1,y2
        adds    z1,z1,tmp,lsl#16
        adcs    z2,z2,tmp,lsr#16
        adc     z3,z3,#0
        mul     tmp,x4,y1
        adds    z2,z2,tmp,lsl#16
        adc     z3,z3,tmp,lsr#16
        mul     tmp,x3,y2
        adds    z2,z2,tmp,lsl#16
        adc     z3,z3,tmp,lsr#16	// subtract divisor * estimated result

	tst	r2,#0xff000000
	beq	CPDO_dvf_loop_entry

CPDO_dvf_end_entry:
	movs	r4,r4,asr#1
	movs	r5,r5,rrx	// remainder was shifted left by 1
	movs	r6,r6,rrx	// relative to divisor

        orr     r7,x3,x4,lsl#16
        orr     r8,x1,x2,lsl#16	// put the split divisor together again

	cmp	r4,#0
	blt	CPDO_dvf_end_negative
	cmpeq	r5,r7
	cmpeq	r6,r8
	bcc	CPDO_dvf_end

CPDO_dvf_end_positive:
	adds	r3,r3,#1
	adc	r2,r2,#0

	subs	r6,r6,r8
	sbcs	r5,r5,r7
	sbcs	r4,r4,#0
	bne	CPDO_dvf_end_positive

	cmp	r5,r7
	cmpeq	r6,r8
	bcs	CPDO_dvf_end_positive
	b	CPDO_dvf_end

CPDO_dvf_end_negative:
	subs	r3,r3,#1
	sbc	r2,r2,#0

	adds	r6,r6,r8
	adcs	r5,r5,r7
	adcs	r4,r4,#0
	bmi	CPDO_dvf_end_negative

CPDO_dvf_end:
	orrs	r9,r5,r6
	ldmia	r13!,{r1,r4,r9,r10,r11,r14}
	moveq	pc,r14

	adds	r6,r6,r6
	adcs	r5,r5,r5
	movcs	r5,#0xc0000000
	movcs	pc,r14

	cmp	r5,r7
	cmpeq	r6,r8
	movcc	r5,#0x40000000
	moveq	r5,#0x80000000
	movhi	r5,#0xc0000000
	mov	pc,r14

CPDO_dvf_zero:
	cmp	r11,#0x80000000
	beq	CPDO_dvf_by_zero

	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next		// 0 already there
@	mov	pc,r14

CPDO_dvf_by_zero:
	cmp	r4,#0x80000000
	beq	CPDO_dvf_generate_qnan	// first 0 too

	mov	r2,#0x80000000	// set MSB
	mov	r3,#0
	mov	r4,#0x7fffffff
	ldr	r5,[r10,#128]
	orr	r5,r5,#2	// division by zero
	str	r5,[r10,#128]
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_dvf_infnan:
	cmp	r4,#0x7fffffff
	beq	CPDO_dvf_1st_infnan

	orrs	r5,r8,r7,lsl#1		// ignore MSB
	beq	CPDO_dvf_2nd_inf
	mov	r1,r6
	mov	r2,r7
	mov	r3,r8
	mov	r4,r11
	b	CPDO_dvf_1st_or_2nd_nan

CPDO_dvf_2nd_inf:
	eor	r1,r1,r6
	mov	r2,#0
	mov	r3,#0
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next	// zero created
@	mov	pc,r14

CPDO_dvf_1st_infnan:
	cmp	r11,#0x7fffffff
	beq	CPDO_dvf_both_infnan

	orrs	r5,r3,r2,lsl#1	// 1st inf? ignore MSB
	bne	CPDO_dvf_1st_or_2nd_nan

	eor	r1,r1,r6	// sign for inf
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next	// inf already there
@	mov	pc,r14

CPDO_dvf_1st_or_2nd_nan:
	tst	r2,#0x40000000
	beq	CPDO_dvf_generate_qnan
	mov	pc,r14		// qnan1/2 already/copied there	

CPDO_dvf_both_infnan:
	orrs	r5,r3,r2,lsl#1	// ignore MSB
	beq	CPDO_dvf_both_infnan_1st_inf
	orrs	r5,r8,r7,lsl#1	// ignore MSB
	beq	CPDO_dvf_both_infnan_2nd_inf
	tst	r2,#0x40000000
	tstne	r7,#0x40000000
	beq	CPDO_dvf_generate_qnan
	mov	pc,r14

CPDO_dvf_both_infnan_1st_inf:
	tst	r7,#0x40000000		// 2nd inf or SNaN ?
	beq	CPDO_dvf_generate_qnan
	mov	r1,r6
	mov	r2,r7
	mov	r3,r8
	mov	r4,r11			// copy 2nd QNaN
	mov	pc,r14

CPDO_dvf_both_infnan_2nd_inf:
	tst	r2,#0x40000000		// 1st SNaN ?
	beq	CPDO_dvf_generate_qnan
	mov	pc,r14

CPDO_dvf_generate_qnan:
	mov	r1,#0x80000000
	mov	r2,#0x7fffffff
	mov	r3,#0xffffffff
	mov	r4,#0x7fffffff
	ldr	r5,[r10,#128]
	orr	r5,r5,#1
	str	r5,[r10,#128]
	mov	pc,r14

/*---------------------------------------------------------------------------*/

	.globl	CPDO_dvf_M
CPDO_dvf_M:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

CPDO_dvf_M_l:
	cmp	r11,#0x7fffffff
	cmpne	r4,#0x7fffffff
	beq	CPDO_dvf_infnan
	eor	r1,r1,r6
	cmp	r11,#0x80000000
	cmpne	r4,#0x80000000
	beq	CPDO_dvf_zero

	sub	r4,r4,r11

	cmp	r2,r7
	cmpeq	r3,r8
	bcs	CPDO_dvf_M_no_normalize

	sub	r4,r4,#1
	stmdb	r13!,{r1,r4,r9,r10}
	mov	r4,r2,lsr#31
	mov	r5,r2,lsl#1
	orr	r5,r5,r3,lsr#31
	mov	r6,r3,lsl#1	// dividend
	b	CPDO_dvf_M_normalize_back

CPDO_dvf_M_no_normalize:
	stmdb	r13!,{r1,r4,r9,r10}
	mov	r4,#0
	mov	r5,r2
	mov	r6,r3		// dividend

CPDO_dvf_M_normalize_back:
	mov	r1,#0
	sub	r10,r1,r7,lsr#1
	mov	r11,#0x40000000

	.macro	inv_step2
	adds	r11,r10,r11,lsl#1
	subcc	r11,r11,r10
	adc	r1,r1,r1
	.endm

	.rept	18
	inv_step2
	.endr

	mov	r1,r1,lsl#14
	adds	r1,r1,#1<<15
	movcs	r1,#0xffffffff	// inverse

	mov	r2,#0
	mov	r3,#0		// clear result space

CPDO_dvf_M_loop_entry:
	mov	r4,r4,lsl#16
	orrs	r4,r4,r5,lsr#16
	mov	r5,r5,lsl#16
	orr	r5,r5,r6,lsr#16
	mov	r6,r6,lsl#16  	// shift dividend left by 16

	bmi	CPDO_dvf_M_loop_negative
	umull	r10,r9,r4,r1	// estimate 16 bits of result in r9

	mov	r2,r2,lsl#16
	orr	r2,r2,r3,lsr#16
	adds	r3,r9,r3,lsl#16	// shift result left by 16 and
	adc	r2,r2,#0	// add in new result bits

	mov	r9,r9,lsl#1
	umull	r11,r10,r8,r9	// divisor lo * estimated result
	subs	r6,r6,r11
	sbcs	r5,r5,r10
	sbc	r4,r4,#0

	umull	r11,r10,r7,r9	// divisor hi * estimated result
	subs	r5,r5,r11
	sbc	r4,r4,r10

	tst	r2,#0xff000000
	beq	CPDO_dvf_M_loop_entry

	b	CPDO_dvf_M_end_entry

CPDO_dvf_M_loop_negative:
	rsb	r11,r4,#0
	umull	r10,r9,r11,r1	// estimate 16 bits of result in r9

	mov	r2,r2,lsl#16
	orr	r2,r2,r3,lsr#16
	rsbs	r3,r9,r3,lsl#16	// shift result left by 16 and
	sbc	r2,r2,#0	// add in new result bits

	mov	r9,r9,lsl#1
	umull	r11,r10,r8,r9	// divisor lo * estimated result
	adds	r6,r6,r11
	adcs	r5,r5,r10
	adc	r4,r4,#0

	umlal	r5,r4,r7,r9	// divisor hi * estimated result

	tst	r2,#0xff000000
	beq	CPDO_dvf_M_loop_entry

CPDO_dvf_M_end_entry:
	movs	r4,r4,asr#1
	movs	r5,r5,rrx	// remainder was shifted left by 1
	movs	r6,r6,rrx	// relative to divisor

	cmp	r4,#0
	blt	CPDO_dvf_M_end_negative
	cmpeq	r5,r7
	cmpeq	r6,r8
	bcc	CPDO_dvf_M_end

CPDO_dvf_M_end_positive:
	adds	r3,r3,#1
	adc	r2,r2,#0

	subs	r6,r6,r8
	sbcs	r5,r5,r7
	sbcs	r4,r4,#0

	cmp	r5,r7
	cmpeq	r6,r8
	bcs	CPDO_dvf_M_end_positive
	b	CPDO_dvf_M_end

CPDO_dvf_M_end_negative:
	subs	r3,r3,#1
	sbc	r2,r2,#0

	adds	r6,r6,r8
	adcs	r5,r5,r7
	adcs	r4,r4,#0
	bmi	CPDO_dvf_M_end_negative

CPDO_dvf_M_end:
	orrs	r9,r5,r6
	ldmia	r13!,{r1,r4,r9,r10}
	moveq	pc,r14

	adds	r6,r6,r6
	adcs	r5,r5,r5
	movcs	r5,#0xc0000000
	movcs	pc,r14

	cmp	r5,r7
	cmpeq	r6,r8
	movcc	r5,#0x40000000
	moveq	r5,#0x80000000
	movhi	r5,#0xc0000000
	mov	pc,r14

/*---------------------------------------------------------------------------*/

	.globl	CPDO_rdf
CPDO_rdf:
	ldmia	r1,{r6,r7,r8,r11}
	ldmia	r2,{r1,r2,r3,r4}
	b	CPDO_dvf_l

/*---------------------------------------------------------------------------*/

	.globl	CPDO_rdf_M
CPDO_rdf_M:
	ldmia	r1,{r6,r7,r8,r11}
	ldmia	r2,{r1,r2,r3,r4}
	b	CPDO_dvf_M_l

/*---------------------------------------------------------------------------*/

	.globl	CPDO_rmf
CPDO_rmf:
	ldmia	r2,{r6,r7,r8,r11}
	ldmia	r1,{r1,r2,r3,r4}

        cmp     r11,#0x7fffffff
        cmpne	r4,#0x7fffffff
        beq     CPDO_rmf_infnan
        cmp     r11,#0x80000000
	cmpne	r4,#0x80000000
        beq     CPDO_rmf_zero

	cmp	r4,r11
        bge	CPDO_rmf_loop_entry
	b	CPDO_rmf_smaller

CPDO_rmf_loop_0:
	mov	r5,#0
CPDO_rmf_loop:
	cmp	r4,r11
	ble	CPDO_rmf_loop_end

	sub	r4,r4,#1

	adds	r3,r3,r3
	adcs	r2,r2,r2
	bcs	CPDO_rmf_loop_anyway

CPDO_rmf_loop_entry:
	cmp	r2,r7
	cmpeq	r3,r8
	bcc	CPDO_rmf_loop_0

CPDO_rmf_loop_anyway:
	subs	r3,r3,r8
	sbc	r2,r2,r7
	mov	r5,#1
	b	CPDO_rmf_loop

CPDO_rmf_loop_end:
	teq	r2,#0
	teqeq	r3,#0
	beq	CPDO_rmf_created_zero

	//eor	r1,r1,r6	// only if result not zero

	mov	r6,r2,lsr#31
	mov	r11,r2,lsl#1
	orr	r11,r11,r3,lsr#31

	cmp	r6,#0
	cmpeq	r11,r7
	rsbeqs	r6,r8,r3,lsl#1
	cmpeq	r5,#1		// for nearest-even
	bcc	CPDO_rmf_norm

	eor	r1,r1,#0x80000000
	subs	r3,r8,r3
	sbc	r2,r7,r2
	
CPDO_rmf_norm:
        teq     r2,#0           // normalize 32 bit
        moveq   r2,r3
        moveq   r3,#0
        subeq   r4,r4,#32

        cmp     r2,#0x00010000  // normalize 16 bit
        movcc   r2,r2,lsl#16
        orrcc   r2,r2,r3,lsr#16
        movcc   r3,r3,lsl#16
        subcc   r4,r4,#16

        cmp     r2,#0x01000000  // normalize 8 bit
        movcc   r2,r2,lsl#8
        orrcc   r2,r2,r3,lsr#24
        movcc   r3,r3,lsl#8
        subcc   r4,r4,#8

        cmp     r2,#0x10000000  // normalize 4 bit
        movcc   r2,r2,lsl#4
        orrcc   r2,r2,r3,lsr#28
        movcc   r3,r3,lsl#4
        subcc   r4,r4,#4

        cmp     r2,#0x40000000  // normalize 2 bit
        movcc   r2,r2,lsl#2
        orrcc   r2,r2,r3,lsr#30
        movcc   r3,r3,lsl#2
        subcc   r4,r4,#2

        cmp     r2,#0x80000000  // normalize 1 bit
        movcc   r2,r2,lsl#1
        orrcc   r2,r2,r3,lsr#31
        movcc   r3,r3,lsl#1
        subcc   r4,r4,#1

	mov	r5,#0
	mov	pc,r14

CPDO_rmf_created_zero:
	mov	r4,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_rmf_smaller:
	add	r5,r4,#1
	cmp	r5,r11
	blt	CPDO_rmf_norm
	cmp	r2,r7
	cmpeq	r3,r8
	bls	CPDO_rmf_norm

	eor	r1,r1,#0x80000000
	adds	r8,r8,r8
	adc	r7,r7,r7
	subs	r3,r8,r3
	sbc	r2,r7,r2
	b	CPDO_rmf_norm

CPDO_rmf_zero:
	cmp	r11,#0x80000000
	beq	CPDO_rmf_generate_qnan
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next
@	mov	pc,r14

CPDO_rmf_infnan:
        cmp     r4,#0x7fffffff
        beq     CPDO_rmf_1st_infnan

        orrs    r5,r8,r7,lsl#1	// ignore MSB
        beq     CPDO_rmf_2nd_inf
        mov     r1,r6
        mov     r2,r7
        mov     r3,r8
        mov     r4,r11
        b       CPDO_rmf_1st_or_2nd_nan

CPDO_rmf_2nd_inf:
	mov     pc,r14		// result = 1st operand

CPDO_rmf_1st_infnan:
        cmp     r11,#0x7fffffff
        beq     CPDO_rmf_both_infnan

        orrs    r5,r3,r2,lsl#1	// 1st inf?
        bne     CPDO_rmf_1st_or_2nd_nan

	b	CPDO_rmf_generate_qnan

CPDO_rmf_1st_or_2nd_nan:
        tst     r2,#0x40000000
        beq     CPDO_rmf_generate_qnan
        mov     pc,r14          // qnan1/2 already/copied there

CPDO_rmf_both_infnan:
        orrs    r5,r3,r2,lsl#1	// ignore MSB
        beq     CPDO_rmf_both_infnan_1st_inf
        orrs    r5,r8,r7,lsl#1	// ignore MSB
        beq     CPDO_rmf_both_infnan_2nd_inf
        tst     r2,#0x40000000
        tstne   r7,#0x40000000
        beq     CPDO_rmf_generate_qnan
        mov     pc,r14

CPDO_rmf_both_infnan_1st_inf:
        tst     r7,#0x40000000          // 2nd inf or SNaN ?
        beq     CPDO_rmf_generate_qnan
        mov     r1,r6
        mov     r2,r7
        mov     r3,r8
        mov     r4,r11                  // copy 2nd QNaN
        mov     pc,r14

CPDO_rmf_both_infnan_2nd_inf:
        tst     r2,#0x40000000          // 1st SNaN ?
        beq     CPDO_rmf_generate_qnan
        mov     pc,r14

CPDO_rmf_generate_qnan:
	mov     r1,#0x80000000
	mov     r2,#0x7fffffff
	mov     r3,#0xffffffff
	mov     r4,#0x7fffffff
	ldr     r5,[r10,#128]
	orr     r5,r5,#1
	str     r5,[r10,#128]
	mov     pc,r14
	
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/

	.globl	CPDO_mvf
CPDO_mvf:
	ldmia	r2,{r1,r2,r3,r4}
	mov	r5,#0
	mov	pc,r14

/*---------------------------------------------------------------------------*/

	.globl	CPDO_mnf
CPDO_mnf:
	ldmia	r2,{r1,r2,r3,r4}
	eor	r1,r1,#0x80000000
	mov	r5,#0
	mov	pc,r14

/*---------------------------------------------------------------------------*/

	.globl	CPDO_abs
CPDO_abs:
	ldmia	r2,{r1,r2,r3,r4}
	bic	r1,r1,#0x80000000
	stmia	r0,{r1,r2,r3,r4}
	b	fastfpe_next

/*---------------------------------------------------------------------------*/
	
	.globl	CPDO_sqt
CPDO_sqt:
	ldmia	r2,{r1,r2,r3,r4}
	cmp	r1,#0
	bne	CPDO_nan
	cmp	r4,#0x7fffffff
	beq	CPDO_store_1234

	tst	r4,r4,lsr#1		@carry=exponent bit 0
	bcc	CPDO_sqt_exponenteven
	adds	r3,r3,r3
	adc	r2,r2,r2
	cmp	r2,#0x20000000		@set carry for loop
CPDO_sqt_exponenteven:
	mov	r4,r4,asr #1
	str	r4,[r0,#12]

	mov	r4,#0x80000000
	mov	r5,#0
	sub	r2,r2,#0x80000000

	mov	r8,#0x40000000
	mov	r14,#0x80000000

	mov	r1,#1
	b	CPDO_sqt_loop1_first
CPDO_sqt_loop1:
	adds	r3,r3,r3
	adcs	r2,r2,r2
CPDO_sqt_loop1_first:
	add	r6,r4,r8,lsr r1		@r7 const = r5
	bcs	CPDO_sqt_loop1_1
	cmp	r2,r6
	cmpeq	r3,r5			@r5 for r7
	bcc	CPDO_sqt_loop1_0
CPDO_sqt_loop1_1:
	orr	r4,r4,r14,lsr r1
	subs	r3,r3,r5		@r5 for r7
	sbc	r2,r2,r6
CPDO_sqt_loop1_0:
	add	r1,r1,#1
	cmp	r1,#30
	ble	CPDO_sqt_loop1

	adds	r3,r3,r3
	adcs	r2,r2,r2
	bcs	CPDO_sqt_between_1
	adds	r7,r5,#0x80000000
	adc	r6,r4,#0
	cmp	r2,r6
	cmpeq	r3,r7
	bcc	CPDO_sqt_between_0
CPDO_sqt_between_1:
	orr	r4,r4,#0x00000001
	subs	r3,r3,r5
	sbc	r2,r2,r4
	subs	r3,r3,#0x80000000
	sbc	r2,r2,#0
CPDO_sqt_between_0:
	mov	r1,#0

CPDO_sqt_loop2:
	adds	r3,r3,r3
	adcs	r2,r2,r2
	bcs	CPDO_sqt_loop2_1
	adds	r7,r5,r8,lsr r1
	adc	r6,r4,#0
	cmp	r2,r6
	cmpeq	r3,r7
	bcc	CPDO_sqt_loop2_0
CPDO_sqt_loop2_1:
	orr	r5,r5,r14,lsr r1
	subs	r3,r3,r5
	sbc	r2,r2,r4
	subs	r3,r3,r8,lsr r1
	sbc	r2,r2,#0
CPDO_sqt_loop2_0:
	add	r1,r1,#1
	cmp	r1,#30
	ble	CPDO_sqt_loop2

	adds	r3,r3,r3
	adcs	r2,r2,r2
	bcs	CPDO_sqt_after_1
	cmp	r2,r6
	cmpeq	r3,r7
	bcc	CPDO_sqt_after_0
CPDO_sqt_after_1:
	orr	r5,r5,#0x00000001
CPDO_sqt_after_0:

	mov	r1,#0
	stmia	r0,{r1,r4,r5}
	b	fastfpe_next

/*---------------------------------------------------------------------------*/
	
	.globl	CPDO_rnd
CPDO_rnd:
	ldmia	r2,{r1,r2,r3,r5}
        bl      CPDO_rnd_core
	ldr	r6,[r10,#128]
	stmia	r0,{r1,r2,r3,r5}
	orr	r6,r6,r4
	str	r6,[r10,#128]
    	b	fastfpe_next
	
/*---------------------------------------------------------------------------*/

	.globl	CPDO_rnd_core
CPDO_rnd_core:
	and	r6,r4,#0x00000060
	mov	r4,#0		// for return of exception flags
	cmp	r5,#63
	bge	CPDO_rnd_big
	add	pc,pc,r6,lsr#3
	mov	r0,r0
	b	CPDO_rnd_NE
	b	CPDO_rnd_P
	b	CPDO_rnd_M
	b	CPDO_rnd_Z

CPDO_rnd_NE:
	cmp	r5,#0
	blt	CPDO_rnd_NE_01

	subs	r6,r5,#31
	bpl	CPDO_rnd_NE_2
	mov	r7,#0x40000000
	mov	r8,#0x7fffffff
	mov	r7,r7,lsr r5
	mov	r8,r8,lsr r5
	teq	r3,#0
	tsteq	r2,r8
	orrne	r4,r4,#16	// set inexact flag
	adds	r2,r2,r7
	bcs	CPDO_rnd_overflow
	teq	r3,#0
	tsteq	r2,r8
	beq	CPDO_rnd_NE_equal
	mov	r3,#0
	bic	r2,r2,r8
	mov	pc,r14

CPDO_rnd_NE_2:
	mov	r7,#0x80000000
	mov	r8,#0xffffffff
	mov	r7,r7,lsr r6
	mov	r8,r8,lsr r6
	tst	r3,r8
	orrne	r4,r4,#16	// set inexact flag
	adds	r3,r3,r7
	adcs	r2,r2,#0
	bcs	CPDO_rnd_overflow
	tst	r3,r8
	beq	CPDO_rnd_NE_equal
	bic	r3,r3,r8
	mov	pc,r14

CPDO_rnd_NE_equal:
	mov	r7,#0x80000000
	subs	r6,r5,#32
	bicpl	r3,r3,r7,lsr r6
	bicmi	r2,r2,r7,lsr r5
	mov	pc,r14

CPDO_rnd_NE_01:
	cmp	r5,#-1
	bne	CPDO_rnd_0
	cmp	r2,#0x80000000
	cmpeq	r3,#0
	beq	CPDO_rnd_0

	mov	r2,#0x80000000
	mov	r3,#0
	mov	r5,#0
	orr	r4,r4,#16	// set inexact flag
	mov	pc,r14

CPDO_rnd_P:
	teq	r1,#0
	beq	CPDO_rnd_NZ
	b	CPDO_rnd_Z

CPDO_rnd_M:
	teq	r1,#0
	beq	CPDO_rnd_Z
	b	CPDO_rnd_NZ
	
CPDO_rnd_Z:
	cmp	r5,#0		// smaller than 1 will be 0
	blt	CPDO_rnd_0

	rsbs	r6,r5,#31
	bmi	CPDO_rnd_Z_2
	cmp	r3,#0
	mov	r3,#0
	mov	r7,r2,lsr r6
	teqeq	r2,r7,lsl r6
	mov	r2,r7,lsl r6
	orrne	r4,r4,#16	// set inexact flag
	mov	pc,r14

CPDO_rnd_Z_2:
	rsb	r6,r5,#63
	mov	r7,r3,lsr r6
	teq	r3,r7,lsl r6
	mov	r3,r7,lsl r6
	orrne	r4,r4,#16	// set inexact flag
	mov	pc,r14

CPDO_rnd_0:
	cmp	r5,#0x80000000
	moveq	pc,r14		// already 0 -> ok

	mov	r2,#0
	mov	r3,#0
	mov	r5,#0x80000000
	orr	r4,r4,#16	// set inexact flag
	mov	pc,r14

CPDO_rnd_NZ:
	cmp	r5,#0		// smaller than 1 will be stay 0 or become 1
	blt	CPDO_rnd_NZ_01

	mov	r7,#0x7fffffff
	subs	r6,r5,#32
	bpl	CPDO_rnd_NZ_2
	mov	r7,r7,lsr r5
	teq	r3,#0
	tsteq	r2,r7
	orrne	r4,r4,#16	// set inexact flag
	adds	r3,r3,#0xffffffff
	adcs	r2,r2,r7
	bcs	CPDO_rnd_overflow
	mov	r3,#0
	bic	r2,r2,r7
	mov	pc,r14

CPDO_rnd_NZ_2:
	mov	r7,r7,lsr r6
	tst	r3,r7
	orrne	r4,r4,#16	// set inexact flag
	adds	r3,r3,r7
	adcs	r2,r2,#0
	bcs	CPDO_rnd_overflow
	bic	r3,r3,r7
	mov	pc,r14

CPDO_rnd_NZ_01:
	cmp	r5,#0x80000000
	moveq	pc,r14		// already 0 -> ok

	mov	r2,#0x80000000
	mov	r3,#0
	mov	r5,#0
	orr	r4,r4,#16	// set inexact flag
	mov	pc,r14

CPDO_rnd_overflow:
	mov	r2,#0x80000000
	mov	r3,#0
	add	r5,r5,#1
	mov	pc,r14

CPDO_rnd_big:
	cmp	r5,#0x7fffffff
	movne	pc,r14			// just big
	orrs	r6,r3,r2,lsl#1		// ignore MSB
	moveq	pc,r14			// infinity
	tst	r2,#0x40000000		// signalling NaN ?
	orreq	r4,r4,#1		// set invalid operation flag
	orreq	r2,r2,#0x40000000	// make quiet NaN
	mov	pc,r14

/*---------------------------------------------------------------------------*/
