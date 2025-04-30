
clk-krait.o:     file format elf32-littlearm


Disassembly of section .text.krait_div_recalc_rate:

00000000 <krait_div_recalc_rate>:
	return 0;
}

static unsigned long
krait_div_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
   0:	e16d41f0 	strd	r4, [sp, #-16]!
   4:	e1a05000 	mov	r5, r0

	div = krait_get_l2_indirect_reg(d->offset);
	div >>= d->shift;
	div &= d->mask;

	return DIV_ROUND_UP(parent_rate, krait_val_to_div(div));
   8:	e2414001 	sub	r4, r1, #1
{
   c:	e58d6008 	str	r6, [sp, #8]
  10:	e58de00c 	str	lr, [sp, #12]
	div = krait_get_l2_indirect_reg(d->offset);
  14:	e5100014 	ldr	r0, [r0, #-20]	@ 0xffffffec
  18:	ebfffffe 	bl	0 <krait_get_l2_indirect_reg>
	div &= d->mask;
  1c:	e5153010 	ldr	r3, [r5, #-16]
	div >>= d->shift;
  20:	e5152008 	ldr	r2, [r5, #-8]
}
  24:	e59d6008 	ldr	r6, [sp, #8]
	div &= d->mask;
  28:	e0033230 	and	r3, r3, r0, lsr r2
	return DIV_ROUND_UP(parent_rate, krait_val_to_div(div));
  2c:	e2833001 	add	r3, r3, #1
  30:	e1a03083 	lsl	r3, r3, #1
  34:	e0844003 	add	r4, r4, r3
}
  38:	e730f314 	udiv	r0, r4, r3
  3c:	e1cd40d0 	ldrd	r4, [sp]
  40:	e28dd00c 	add	sp, sp, #12
  44:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

Disassembly of section .text.krait_mux_get_parent:

00000000 <krait_mux_get_parent>:
{
   0:	e52d4008 	str	r4, [sp, #-8]!
   4:	e1a04000 	mov	r4, r0
   8:	e58de004 	str	lr, [sp, #4]
	sel = krait_get_l2_indirect_reg(mux->offset);
   c:	e5100018 	ldr	r0, [r0, #-24]	@ 0xffffffe8
  10:	ebfffffe 	bl	0 <krait_get_l2_indirect_reg>
	sel &= mux->mask;
  14:	e5143014 	ldr	r3, [r4, #-20]	@ 0xffffffec
	return clk_mux_val_to_index(hw, mux->parent_map, 0, sel);
  18:	e3a02000 	mov	r2, #0
	sel >>= mux->shift;
  1c:	e5141010 	ldr	r1, [r4, #-16]
	sel &= mux->mask;
  20:	e0033130 	and	r3, r3, r0, lsr r1
	return clk_mux_val_to_index(hw, mux->parent_map, 0, sel);
  24:	e514101c 	ldr	r1, [r4, #-28]	@ 0xffffffe4
  28:	e1a00004 	mov	r0, r4
	mux->en_mask = sel;
  2c:	e504300c 	str	r3, [r4, #-12]
	return clk_mux_val_to_index(hw, mux->parent_map, 0, sel);
  30:	ebfffffe 	bl	0 <clk_mux_val_to_index>
}
  34:	e59d4000 	ldr	r4, [sp]
  38:	e28dd004 	add	sp, sp, #4
  3c:	e6ef0070 	uxtb	r0, r0
  40:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

Disassembly of section .text.krait_mux_set_parent:

00000000 <krait_mux_set_parent>:
{
   0:	e1a02001 	mov	r2, r1
	sel = clk_mux_index_to_val(mux->parent_map, 0, index);
   4:	e3a01000 	mov	r1, #0
{
   8:	e16d41f8 	strd	r4, [sp, #-24]!	@ 0xffffffe8
   c:	e1a04000 	mov	r4, r0
  10:	e1cd60f8 	strd	r6, [sp, #8]
  14:	e58d8010 	str	r8, [sp, #16]
  18:	e58de014 	str	lr, [sp, #20]
	sel = clk_mux_index_to_val(mux->parent_map, 0, index);
  1c:	e510001c 	ldr	r0, [r0, #-28]	@ 0xffffffe4
  20:	ebfffffe 	bl	0 <clk_mux_index_to_val>
  24:	e1a05000 	mov	r5, r0
	if (__clk_is_enabled(hw->clk))
  28:	e5940004 	ldr	r0, [r4, #4]
	mux->en_mask = sel;
  2c:	e504500c 	str	r5, [r4, #-12]
	if (__clk_is_enabled(hw->clk))
  30:	ebfffffe 	bl	0 <__clk_is_enabled>
  34:	e3500000 	cmp	r0, #0
  38:	1a000007 	bne	5c <krait_mux_set_parent+0x5c>
	mux->reparent = true;
  3c:	e3a03001 	mov	r3, #1
}
  40:	e3a00000 	mov	r0, #0
	mux->reparent = true;
  44:	e5443005 	strb	r3, [r4, #-5]
}
  48:	e1cd40d0 	ldrd	r4, [sp]
  4c:	e1cd60d8 	ldrd	r6, [sp, #8]
  50:	e59d8010 	ldr	r8, [sp, #16]
  54:	e28dd014 	add	sp, sp, #20
  58:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
	spin_lock_irqsave(&krait_clock_reg_lock, flags);
  5c:	e3000000 	movw	r0, #0
  60:	e3400000 	movt	r0, #0
  64:	ebfffffe 	bl	0 <_raw_spin_lock_irqsave>
  68:	e1a07000 	mov	r7, r0
	regval = krait_get_l2_indirect_reg(mux->offset);
  6c:	e5140018 	ldr	r0, [r4, #-24]	@ 0xffffffe8
  70:	ebfffffe 	bl	0 <krait_get_l2_indirect_reg>
	if (mux->disable_sec_src_gating) {
  74:	e5543004 	ldrb	r3, [r4, #-4]
	regval = krait_get_l2_indirect_reg(mux->offset);
  78:	e1a06000 	mov	r6, r0
	if (mux->disable_sec_src_gating) {
  7c:	e3530000 	cmp	r3, #0
  80:	1a00001c 	bne	f8 <krait_mux_set_parent+0xf8>
	regval &= ~(mux->mask << mux->shift);
  84:	e14421d4 	ldrd	r2, [r4, #-20]	@ 0xffffffec
	if (mux->lpl) {
  88:	e5541008 	ldrb	r1, [r4, #-8]
	regval |= (sel & mux->mask) << mux->shift;
  8c:	e0055002 	and	r5, r5, r2
	regval &= ~(mux->mask << mux->shift);
  90:	e1c66312 	bic	r6, r6, r2, lsl r3
	if (mux->lpl) {
  94:	e3510000 	cmp	r1, #0
	regval |= (sel & mux->mask) << mux->shift;
  98:	e1866315 	orr	r6, r6, r5, lsl r3
	if (mux->lpl) {
  9c:	1a000011 	bne	e8 <krait_mux_set_parent+0xe8>
	krait_set_l2_indirect_reg(mux->offset, regval);
  a0:	e5140018 	ldr	r0, [r4, #-24]	@ 0xffffffe8
  a4:	e1a01006 	mov	r1, r6
  a8:	ebfffffe 	bl	0 <krait_set_l2_indirect_reg>
	if (mux->disable_sec_src_gating) {
  ac:	e5543004 	ldrb	r3, [r4, #-4]
  b0:	e3530000 	cmp	r3, #0
  b4:	1a000014 	bne	10c <krait_mux_set_parent+0x10c>
	mb();
  b8:	f57ff04f 	dsb	sy
	udelay(1);
  bc:	e3003000 	movw	r3, #0
  c0:	e30406dc 	movw	r0, #18140	@ 0x46dc
  c4:	e3403000 	movt	r3, #0
  c8:	e3400003 	movt	r0, #3
  cc:	e5933004 	ldr	r3, [r3, #4]
  d0:	e12fff33 	blx	r3
	raw_spin_unlock_irq(&lock->rlock);
}

static __always_inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
  d4:	e3000000 	movw	r0, #0
  d8:	e1a01007 	mov	r1, r7
  dc:	e3400000 	movt	r0, #0
  e0:	ebfffffe 	bl	0 <_raw_spin_unlock_irqrestore>
}
  e4:	eaffffd4 	b	3c <krait_mux_set_parent+0x3c>
		regval &= ~(mux->mask << (mux->shift + LPL_SHIFT));
  e8:	e2833008 	add	r3, r3, #8
  ec:	e1c66312 	bic	r6, r6, r2, lsl r3
		regval |= (sel & mux->mask) << (mux->shift + LPL_SHIFT);
  f0:	e1866315 	orr	r6, r6, r5, lsl r3
  f4:	eaffffe9 	b	a0 <krait_mux_set_parent+0xa0>
		regval |= SECCLKAGD;
  f8:	e3806010 	orr	r6, r0, #16
		krait_set_l2_indirect_reg(mux->offset, regval);
  fc:	e5140018 	ldr	r0, [r4, #-24]	@ 0xffffffe8
 100:	e1a01006 	mov	r1, r6
 104:	ebfffffe 	bl	0 <krait_set_l2_indirect_reg>
 108:	eaffffdd 	b	84 <krait_mux_set_parent+0x84>
		krait_set_l2_indirect_reg(mux->offset, regval);
 10c:	e5140018 	ldr	r0, [r4, #-24]	@ 0xffffffe8
 110:	e3c61010 	bic	r1, r6, #16
 114:	ebfffffe 	bl	0 <krait_set_l2_indirect_reg>
 118:	eaffffe6 	b	b8 <krait_mux_set_parent+0xb8>

Disassembly of section .text.krait_div_set_rate:

00000000 <krait_div_set_rate>:
{
   0:	e16d41f0 	strd	r4, [sp, #-16]!
   4:	e1a05000 	mov	r5, r0
	spin_lock_irqsave(&krait_clock_reg_lock, flags);
   8:	e3000000 	movw	r0, #0
{
   c:	e58d6008 	str	r6, [sp, #8]
	spin_lock_irqsave(&krait_clock_reg_lock, flags);
  10:	e3400000 	movt	r0, #0
{
  14:	e58de00c 	str	lr, [sp, #12]
	u8 div_val = krait_div_to_val(d->divisor);
  18:	e555300c 	ldrb	r3, [r5, #-12]
  1c:	e1a030a3 	lsr	r3, r3, #1
  20:	e2433001 	sub	r3, r3, #1
  24:	e6ef4073 	uxtb	r4, r3
	spin_lock_irqsave(&krait_clock_reg_lock, flags);
  28:	ebfffffe 	bl	0 <_raw_spin_lock_irqsave>
  2c:	e1a06000 	mov	r6, r0
	regval = krait_get_l2_indirect_reg(d->offset);
  30:	e5150014 	ldr	r0, [r5, #-20]	@ 0xffffffec
  34:	ebfffffe 	bl	0 <krait_get_l2_indirect_reg>
	regval &= ~(d->mask << d->shift);
  38:	e515c010 	ldr	ip, [r5, #-16]
  3c:	e5152008 	ldr	r2, [r5, #-8]
	regval |= (div_val & d->mask) << d->shift;
  40:	e004300c 	and	r3, r4, ip
	regval &= ~(d->mask << d->shift);
  44:	e1c0121c 	bic	r1, r0, ip, lsl r2
	if (d->lpl) {
  48:	e5550004 	ldrb	r0, [r5, #-4]
	regval |= (div_val & d->mask) << d->shift;
  4c:	e1811213 	orr	r1, r1, r3, lsl r2
	if (d->lpl) {
  50:	e3500000 	cmp	r0, #0
  54:	0a000002 	beq	64 <krait_div_set_rate+0x64>
		regval &= ~(d->mask << (d->shift + LPL_SHIFT));
  58:	e2822008 	add	r2, r2, #8
  5c:	e1c1121c 	bic	r1, r1, ip, lsl r2
		regval |= (div_val & d->mask) << (d->shift + LPL_SHIFT);
  60:	e1811213 	orr	r1, r1, r3, lsl r2
	krait_set_l2_indirect_reg(d->offset, regval);
  64:	e5150014 	ldr	r0, [r5, #-20]	@ 0xffffffec
  68:	ebfffffe 	bl	0 <krait_set_l2_indirect_reg>
  6c:	e3000000 	movw	r0, #0
  70:	e1a01006 	mov	r1, r6
  74:	e3400000 	movt	r0, #0
  78:	ebfffffe 	bl	0 <_raw_spin_unlock_irqrestore>
}
  7c:	e1cd40d0 	ldrd	r4, [sp]
  80:	e3a00000 	mov	r0, #0
  84:	e59d6008 	ldr	r6, [sp, #8]
  88:	e28dd00c 	add	sp, sp, #12
  8c:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

Disassembly of section .text.krait_div_determine_rate:

00000000 <krait_div_determine_rate>:
{
   0:	e16d41f0 	strd	r4, [sp, #-16]!
   4:	e1a04001 	mov	r4, r1
   8:	e1a05000 	mov	r5, r0
   c:	e58d6008 	str	r6, [sp, #8]
  10:	e58de00c 	str	lr, [sp, #12]
	req->best_parent_rate = clk_hw_round_rate(clk_hw_get_parent(hw),
  14:	ebfffffe 	bl	0 <clk_hw_get_parent>
  18:	e5941000 	ldr	r1, [r4]
						  req->rate * d->divisor);
  1c:	e555300c 	ldrb	r3, [r5, #-12]
	req->best_parent_rate = clk_hw_round_rate(clk_hw_get_parent(hw),
  20:	e0010391 	mul	r1, r1, r3
  24:	ebfffffe 	bl	0 <clk_hw_round_rate>
  28:	e1a02000 	mov	r2, r0
}
  2c:	e3a00000 	mov	r0, #0
	req->best_parent_rate = clk_hw_round_rate(clk_hw_get_parent(hw),
  30:	e584200c 	str	r2, [r4, #12]
	req->rate = DIV_ROUND_UP(req->best_parent_rate, d->divisor);
  34:	e555100c 	ldrb	r1, [r5, #-12]
  38:	e2413001 	sub	r3, r1, #1
  3c:	e0833002 	add	r3, r3, r2
  40:	e733f113 	udiv	r3, r3, r1
  44:	e5843000 	str	r3, [r4]
}
  48:	e1cd40d0 	ldrd	r4, [sp]
  4c:	e59d6008 	ldr	r6, [sp, #8]
  50:	e28dd00c 	add	sp, sp, #12
  54:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
