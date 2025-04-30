
krait-cc.o:     file format elf32-littlearm


Disassembly of section .text.krait_notifier_cb:

00000000 <krait_notifier_cb>:
{
	int ret = 0;
	struct krait_mux_clk *mux = container_of(nb, struct krait_mux_clk,
						 clk_nb);
	/* Switch to safe parent */
	if (event == PRE_RATE_CHANGE) {
   0:	e3510001 	cmp	r1, #1
{
   4:	e16d41f0 	strd	r4, [sp, #-16]!
   8:	e1a04000 	mov	r4, r0
   c:	e58d6008 	str	r6, [sp, #8]
  10:	e58de00c 	str	lr, [sp, #12]
	if (event == PRE_RATE_CHANGE) {
  14:	0a000014 	beq	6c <krait_notifier_cb+0x6c>
	/*
	 * By the time POST_RATE_CHANGE notifier is called,
	 * clk framework itself would have changed the parent for the new rate.
	 * Only otherwise, put back to the old parent.
	 */
	} else if (event == POST_RATE_CHANGE) {
  18:	e3510002 	cmp	r1, #2
  1c:	0a000004 	beq	34 <krait_notifier_cb+0x34>
static inline int notifier_from_errno(int err)
{
	if (err)
		return NOTIFY_STOP_MASK | (NOTIFY_OK - err);

	return NOTIFY_OK;
  20:	e3a00001 	mov	r0, #1
			ret = krait_mux_clk_ops.set_parent(&mux->hw,
							   mux->old_index);
	}

	return notifier_from_errno(ret);
}
  24:	e1cd40d0 	ldrd	r4, [sp]
  28:	e59d6008 	ldr	r6, [sp, #8]
  2c:	e28dd00c 	add	sp, sp, #12
  30:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
		if (!mux->reparent)
  34:	e5503011 	ldrb	r3, [r0, #-17]	@ 0xffffffef
  38:	e3530000 	cmp	r3, #0
  3c:	1afffff7 	bne	20 <krait_notifier_cb+0x20>
			ret = krait_mux_clk_ops.set_parent(&mux->hw,
  40:	e3003000 	movw	r3, #0
  44:	e5501012 	ldrb	r1, [r0, #-18]	@ 0xffffffee
  48:	e240000c 	sub	r0, r0, #12
  4c:	e3403000 	movt	r3, #0
  50:	e5933034 	ldr	r3, [r3, #52]	@ 0x34
  54:	e12fff33 	blx	r3
	if (err)
  58:	e3500000 	cmp	r0, #0
  5c:	0affffef 	beq	20 <krait_notifier_cb+0x20>
		return NOTIFY_STOP_MASK | (NOTIFY_OK - err);
  60:	e2600001 	rsb	r0, r0, #1
  64:	e3800902 	orr	r0, r0, #32768	@ 0x8000
	return notifier_from_errno(ret);
  68:	eaffffed 	b	24 <krait_notifier_cb+0x24>
		mux->old_index = krait_mux_clk_ops.get_parent(&mux->hw);
  6c:	e3005000 	movw	r5, #0
  70:	e240600c 	sub	r6, r0, #12
  74:	e3405000 	movt	r5, #0
  78:	e1a00006 	mov	r0, r6
  7c:	e5953038 	ldr	r3, [r5, #56]	@ 0x38
  80:	e12fff33 	blx	r3
  84:	e1a02000 	mov	r2, r0
		ret = krait_mux_clk_ops.set_parent(&mux->hw, mux->safe_sel);
  88:	e5953034 	ldr	r3, [r5, #52]	@ 0x34
  8c:	e1a00006 	mov	r0, r6
  90:	e5541013 	ldrb	r1, [r4, #-19]	@ 0xffffffed
		mux->old_index = krait_mux_clk_ops.get_parent(&mux->hw);
  94:	e5442012 	strb	r2, [r4, #-18]	@ 0xffffffee
		ret = krait_mux_clk_ops.set_parent(&mux->hw, mux->safe_sel);
  98:	e12fff33 	blx	r3
	if (err)
  9c:	e3500000 	cmp	r0, #0
		mux->reparent = false;
  a0:	e3a03000 	mov	r3, #0
  a4:	e5443011 	strb	r3, [r4, #-17]	@ 0xffffffef
  a8:	0affffdc 	beq	20 <krait_notifier_cb+0x20>
		return NOTIFY_STOP_MASK | (NOTIFY_OK - err);
  ac:	e2600001 	rsb	r0, r0, #1
  b0:	e3800902 	orr	r0, r0, #32768	@ 0x8000
  b4:	eaffffda 	b	24 <krait_notifier_cb+0x24>

Disassembly of section .init.text:

00000000 <krait_cc_driver_init>:
	.driver = {
		.name = "krait-cc",
		.of_match_table = krait_cc_match_table,
	},
};
module_platform_driver(krait_cc_driver);
   0:	e3000000 	movw	r0, #0
   4:	e3a01000 	mov	r1, #0
   8:	e3400000 	movt	r0, #0
   c:	eafffffe 	b	0 <__platform_driver_register>

Disassembly of section .text.krait_of_get:

00000000 <krait_of_get>:
	unsigned int idx = clkspec->args[0];
   0:	e5902008 	ldr	r2, [r0, #8]
	if (idx >= clks_max) {
   4:	e3520004 	cmp	r2, #4
   8:	8a000003 	bhi	1c <krait_of_get+0x1c>
	return clks[idx] ? : ERR_PTR(-ENODEV);
   c:	e7910102 	ldr	r0, [r1, r2, lsl #2]
  10:	e3500000 	cmp	r0, #0
  14:	03e00012 	mvneq	r0, #18
}
  18:	e12fff1e 	bx	lr
		pr_err("%s: invalid clock index %d\n", __func__, idx);
  1c:	e3001000 	movw	r1, #0
  20:	e3000000 	movw	r0, #0
{
  24:	e52d4008 	str	r4, [sp, #-8]!
		pr_err("%s: invalid clock index %d\n", __func__, idx);
  28:	e3401000 	movt	r1, #0
  2c:	e3400000 	movt	r0, #0
{
  30:	e58de004 	str	lr, [sp, #4]
		pr_err("%s: invalid clock index %d\n", __func__, idx);
  34:	ebfffffe 	bl	0 <_printk>
}
  38:	e59d4000 	ldr	r4, [sp]
  3c:	e28dd004 	add	sp, sp, #4
		return ERR_PTR(-EINVAL);
  40:	e3e00015 	mvn	r0, #21
}
  44:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

Disassembly of section .exit.text:

00000000 <krait_cc_driver_exit>:
module_platform_driver(krait_cc_driver);
   0:	e3000000 	movw	r0, #0
   4:	e3400000 	movt	r0, #0
   8:	eafffffe 	b	0 <platform_driver_unregister>

Disassembly of section .text.krait_add_clks:

00000000 <krait_add_clks>:
	if (id >= 0) {
   0:	e3510000 	cmp	r1, #0
{
   4:	e16d42f4 	strd	r4, [sp, #-36]!	@ 0xffffffdc
   8:	e1cd60f8 	strd	r6, [sp, #8]
   c:	e1a06000 	mov	r6, r0
  10:	e1cd81f0 	strd	r8, [sp, #16]
  14:	e1cda1f8 	strd	sl, [sp, #24]
  18:	e58de020 	str	lr, [sp, #32]
  1c:	e24dd044 	sub	sp, sp, #68	@ 0x44
	if (id >= 0) {
  20:	e58d1008 	str	r1, [sp, #8]
{
  24:	e58d2010 	str	r2, [sp, #16]
	if (id >= 0) {
  28:	ba00014f 	blt	56c <krait_add_clks+0x56c>
		offset = 0x4501 + (0x1000 * id);
  2c:	e3043501 	movw	r3, #17665	@ 0x4501
		s = p = kasprintf(GFP_KERNEL, "%d", id);
  30:	e1a02001 	mov	r2, r1
		offset = 0x4501 + (0x1000 * id);
  34:	e0833601 	add	r3, r3, r1, lsl #12
		s = p = kasprintf(GFP_KERNEL, "%d", id);
  38:	e3001000 	movw	r1, #0
  3c:	e3401000 	movt	r1, #0
  40:	e3a00d33 	mov	r0, #3264	@ 0xcc0
		offset = 0x4501 + (0x1000 * id);
  44:	e58d3004 	str	r3, [sp, #4]
		s = p = kasprintf(GFP_KERNEL, "%d", id);
  48:	ebfffffe 	bl	0 <kasprintf>
		if (!s)
  4c:	e2503000 	subs	r3, r0, #0
  50:	e58d3014 	str	r3, [sp, #20]
  54:	0a00016e 	beq	614 <krait_add_clks+0x614>
		s = p = kasprintf(GFP_KERNEL, "%d", id);
  58:	e58d3000 	str	r3, [sp]
	struct clk_init_data init = {
  5c:	e3a02004 	mov	r2, #4
  60:	e3003000 	movw	r3, #0
  64:	e3a04000 	mov	r4, #0
  68:	e3a05000 	mov	r5, #0
  6c:	e3403000 	movt	r3, #0
  70:	e3a0c001 	mov	ip, #1
				     const char *fmt, va_list ap) __malloc;
__printf(3, 4) char *devm_kasprintf(struct device *dev, gfp_t gfp,
				    const char *fmt, ...) __malloc;
static inline void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
	return devm_kmalloc(dev, size, gfp | __GFP_ZERO);
  74:	e3a01020 	mov	r1, #32
  78:	e1a00006 	mov	r0, r6
  7c:	e1cd42f4 	strd	r4, [sp, #36]	@ 0x24
  80:	e58d203c 	str	r2, [sp, #60]	@ 0x3c
  84:	e3a02d37 	mov	r2, #3520	@ 0xdc0
  88:	e28d8024 	add	r8, sp, #36	@ 0x24
  8c:	e58d3028 	str	r3, [sp, #40]	@ 0x28
  90:	e1cd42fc 	strd	r4, [sp, #44]	@ 0x2c
  94:	e1cd43f4 	strd	r4, [sp, #52]	@ 0x34
  98:	e5cdc038 	strb	ip, [sp, #56]	@ 0x38
  9c:	ebfffffe 	bl	0 <devm_kmalloc>
	if (!div)
  a0:	e2509000 	subs	r9, r0, #0
  a4:	0a000156 	beq	604 <krait_add_clks+0x604>
	init.name = kasprintf(GFP_KERNEL, "hfpll%s_div", s);
  a8:	e59d5000 	ldr	r5, [sp]
  ac:	e3001000 	movw	r1, #0
  b0:	e3a00d33 	mov	r0, #3264	@ 0xcc0
	div->offset = offset;
  b4:	e59d3004 	ldr	r3, [sp, #4]
	init.name = kasprintf(GFP_KERNEL, "hfpll%s_div", s);
  b8:	e3401000 	movt	r1, #0
  bc:	e1a02005 	mov	r2, r5
	div->offset = offset;
  c0:	e5893000 	str	r3, [r9]
	div->mask = 0x3;
  c4:	e3a03003 	mov	r3, #3
  c8:	e5893004 	str	r3, [r9, #4]
	div->divisor = 2;
  cc:	e3a03002 	mov	r3, #2
	div->hw.init = &init;
  d0:	e589801c 	str	r8, [r9, #28]
	div->divisor = 2;
  d4:	e5c93008 	strb	r3, [r9, #8]
	div->shift = 6;
  d8:	e3a03006 	mov	r3, #6
  dc:	e589300c 	str	r3, [r9, #12]
	div->lpl = id >= 0;
  e0:	e59d3008 	ldr	r3, [sp, #8]
  e4:	e1e03003 	mvn	r3, r3
  e8:	e1a03fa3 	lsr	r3, r3, #31
  ec:	e5c93010 	strb	r3, [r9, #16]
  f0:	e58d300c 	str	r3, [sp, #12]
	init.name = kasprintf(GFP_KERNEL, "hfpll%s_div", s);
  f4:	ebfffffe 	bl	0 <kasprintf>
	if (!init.name)
  f8:	e3500000 	cmp	r0, #0
	init.name = kasprintf(GFP_KERNEL, "hfpll%s_div", s);
  fc:	e58d0024 	str	r0, [sp, #36]	@ 0x24
	if (!init.name)
 100:	0a00013f 	beq	604 <krait_add_clks+0x604>
	init.parent_data = p_data;
 104:	e3004000 	movw	r4, #0
	parent_name = kasprintf(GFP_KERNEL, "hfpll%s", s);
 108:	e3001000 	movw	r1, #0
	init.parent_data = p_data;
 10c:	e3404000 	movt	r4, #0
	parent_name = kasprintf(GFP_KERNEL, "hfpll%s", s);
 110:	e1a02005 	mov	r2, r5
 114:	e3401000 	movt	r1, #0
 118:	e3a00d33 	mov	r0, #3264	@ 0xcc0
	init.parent_data = p_data;
 11c:	e58d4030 	str	r4, [sp, #48]	@ 0x30
	parent_name = kasprintf(GFP_KERNEL, "hfpll%s", s);
 120:	ebfffffe 	bl	0 <kasprintf>
	if (!parent_name) {
 124:	e2505000 	subs	r5, r0, #0
 128:	0a000133 	beq	5fc <krait_add_clks+0x5fc>
	ret = devm_clk_hw_register(dev, &div->hw);
 12c:	e2897014 	add	r7, r9, #20
 130:	e1a00006 	mov	r0, r6
	p_data[0].fw_name = parent_name;
 134:	e5845004 	str	r5, [r4, #4]
	ret = devm_clk_hw_register(dev, &div->hw);
 138:	e1a01007 	mov	r1, r7
	p_data[0].fw_name = parent_name;
 13c:	e5845008 	str	r5, [r4, #8]
	ret = devm_clk_hw_register(dev, &div->hw);
 140:	ebfffffe 	bl	0 <devm_clk_hw_register>
	if (ret) {
 144:	e2502000 	subs	r2, r0, #0

#define IS_ERR_VALUE(x) unlikely((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void * __must_check ERR_PTR(long error)
{
	return (void *) error;
 148:	11a07002 	movne	r7, r2
 14c:	1a00001e 	bne	1cc <krait_add_clks+0x1cc>
	if (id < 0)
 150:	e59d3008 	ldr	r3, [sp, #8]
 154:	e3530000 	cmp	r3, #0
 158:	aa0000f7 	bge	53c <krait_add_clks+0x53c>
 15c:	e300b000 	movw	fp, #0
		for_each_online_cpu(cpu)
 160:	e300a000 	movw	sl, #0
 164:	e58d501c 	str	r5, [sp, #28]
 168:	e340b000 	movt	fp, #0
 16c:	e340a000 	movt	sl, #0
 170:	ea000000 	b	178 <krait_add_clks+0x178>
 174:	e2842001 	add	r2, r4, #1
 178:	e59b1000 	ldr	r1, [fp]
 17c:	e1a0000a 	mov	r0, sl
 180:	ebfffffe 	bl	0 <_find_next_bit_le>
 184:	e59b3000 	ldr	r3, [fp]
 188:	e1a04000 	mov	r4, r0
 18c:	e1500003 	cmp	r0, r3
 190:	2a00000c 	bcs	1c8 <krait_add_clks+0x1c8>
			clk_prepare_enable(div->hw.clk);
 194:	e5995018 	ldr	r5, [r9, #24]
/* clk_prepare_enable helps cases using clk_enable in non-atomic context. */
static inline int clk_prepare_enable(struct clk *clk)
{
	int ret;

	ret = clk_prepare(clk);
 198:	e1a00005 	mov	r0, r5
 19c:	ebfffffe 	bl	0 <clk_prepare>
 1a0:	e1a03000 	mov	r3, r0
	if (ret)
		return ret;
	ret = clk_enable(clk);
 1a4:	e1a00005 	mov	r0, r5
	if (ret)
 1a8:	e3530000 	cmp	r3, #0
 1ac:	1afffff0 	bne	174 <krait_add_clks+0x174>
	ret = clk_enable(clk);
 1b0:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 1b4:	e3500000 	cmp	r0, #0
 1b8:	0affffed 	beq	174 <krait_add_clks+0x174>
		clk_unprepare(clk);
 1bc:	e1a00005 	mov	r0, r5
 1c0:	ebfffffe 	bl	0 <clk_unprepare>
 1c4:	eaffffea 	b	174 <krait_add_clks+0x174>
 1c8:	e59d501c 	ldr	r5, [sp, #28]
	kfree(parent_name);
 1cc:	e1a00005 	mov	r0, r5
 1d0:	ebfffffe 	bl	0 <kfree>
	kfree(init.name);
 1d4:	e59d0024 	ldr	r0, [sp, #36]	@ 0x24
 1d8:	ebfffffe 	bl	0 <kfree>
	if (IS_ERR(hfpll_div)) {
 1dc:	e3770a01 	cmn	r7, #4096	@ 0x1000
 1e0:	8a000081 	bhi	3ec <krait_add_clks+0x3ec>
	struct clk_init_data init = {
 1e4:	e3009000 	movw	r9, #0
 1e8:	e3a0a000 	mov	sl, #0
 1ec:	e3a0b000 	mov	fp, #0
 1f0:	e3a02004 	mov	r2, #4
 1f4:	e3409000 	movt	r9, #0
 1f8:	e3005000 	movw	r5, #0
 1fc:	e3a03002 	mov	r3, #2
 200:	e3405000 	movt	r5, #0
 204:	e1cda2f4 	strd	sl, [sp, #36]	@ 0x24
 208:	e3a01034 	mov	r1, #52	@ 0x34
 20c:	e1a00006 	mov	r0, r6
 210:	e58d9028 	str	r9, [sp, #40]	@ 0x28
 214:	e58d203c 	str	r2, [sp, #60]	@ 0x3c
 218:	e3a02d37 	mov	r2, #3520	@ 0xdc0
 21c:	e1c8a0f8 	strd	sl, [r8, #8]
 220:	e1c8a1f0 	strd	sl, [r8, #16]
 224:	e58d5030 	str	r5, [sp, #48]	@ 0x30
 228:	e5cd3038 	strb	r3, [sp, #56]	@ 0x38
 22c:	ebfffffe 	bl	0 <devm_kmalloc>
	if (!mux)
 230:	e2504000 	subs	r4, r0, #0
 234:	0a0000f2 	beq	604 <krait_add_clks+0x604>
	mux->shift = 2;
 238:	e3a03002 	mov	r3, #2
	if (of_machine_is_compatible("qcom,ipq8064") ||
 23c:	e3000000 	movw	r0, #0
 240:	e3400000 	movt	r0, #0
	mux->shift = 2;
 244:	e584300c 	str	r3, [r4, #12]
	mux->offset = offset;
 248:	e59d3004 	ldr	r3, [sp, #4]
 24c:	e5843004 	str	r3, [r4, #4]
	mux->lpl = id >= 0;
 250:	e59d300c 	ldr	r3, [sp, #12]
	mux->hw.init = &init;
 254:	e5848024 	str	r8, [r4, #36]	@ 0x24
	mux->lpl = id >= 0;
 258:	e5c43014 	strb	r3, [r4, #20]
	mux->parent_map = sec_mux_map;
 25c:	e3003000 	movw	r3, #0
 260:	e3403000 	movt	r3, #0
 264:	e5843000 	str	r3, [r4]
	mux->mask = 0x3;
 268:	e3a03003 	mov	r3, #3
 26c:	e5843008 	str	r3, [r4, #8]
	mux->safe_sel = 0;
 270:	e3a03000 	mov	r3, #0
 274:	e5c43015 	strb	r3, [r4, #21]
	if (of_machine_is_compatible("qcom,ipq8064") ||
 278:	ebfffffe 	bl	0 <of_machine_is_compatible>
 27c:	e3500000 	cmp	r0, #0
 280:	0a0000c1 	beq	58c <krait_add_clks+0x58c>
		mux->disable_sec_src_gating = true;
 284:	e3a03001 	mov	r3, #1
 288:	e5c43018 	strb	r3, [r4, #24]
	init.name = kasprintf(GFP_KERNEL, "krait%s_sec_mux", s);
 28c:	e3001000 	movw	r1, #0
 290:	e59d2000 	ldr	r2, [sp]
 294:	e3a00d33 	mov	r0, #3264	@ 0xcc0
 298:	e3401000 	movt	r1, #0
 29c:	ebfffffe 	bl	0 <kasprintf>
	if (!init.name)
 2a0:	e3500000 	cmp	r0, #0
	init.name = kasprintf(GFP_KERNEL, "krait%s_sec_mux", s);
 2a4:	e58d0024 	str	r0, [sp, #36]	@ 0x24
	if (!init.name)
 2a8:	0a0000d5 	beq	604 <krait_add_clks+0x604>
	if (unique_aux) {
 2ac:	e59d3010 	ldr	r3, [sp, #16]
 2b0:	e3530000 	cmp	r3, #0
 2b4:	1a000066 	bne	454 <krait_add_clks+0x454>
		sec_mux_list[1].name = "apu_aux";
 2b8:	e3003000 	movw	r3, #0
	ret = devm_clk_hw_register(dev, &mux->hw);
 2bc:	e284a01c 	add	sl, r4, #28
		sec_mux_list[1].name = "apu_aux";
 2c0:	e3403000 	movt	r3, #0
	ret = devm_clk_hw_register(dev, &mux->hw);
 2c4:	e1a0100a 	mov	r1, sl
 2c8:	e1a00006 	mov	r0, r6
		sec_mux_list[1].name = "apu_aux";
 2cc:	e5853018 	str	r3, [r5, #24]
	ret = devm_clk_hw_register(dev, &mux->hw);
 2d0:	ebfffffe 	bl	0 <devm_clk_hw_register>
	if (ret) {
 2d4:	e3500000 	cmp	r0, #0
 2d8:	11a0a000 	movne	sl, r0
 2dc:	0a00006d 	beq	498 <krait_add_clks+0x498>
	kfree(init.name);
 2e0:	e59d0024 	ldr	r0, [sp, #36]	@ 0x24
 2e4:	ebfffffe 	bl	0 <kfree>
	if (IS_ERR(sec_mux)) {
 2e8:	e37a0a01 	cmn	sl, #4096	@ 0x1000
 2ec:	8a0000c0 	bhi	5f4 <krait_add_clks+0x5f4>
	struct clk_init_data init = {
 2f0:	e3a04000 	mov	r4, #0
 2f4:	e3a05000 	mov	r5, #0
 2f8:	e3a02004 	mov	r2, #4
 2fc:	e300b000 	movw	fp, #0
 300:	e3a03003 	mov	r3, #3
 304:	e340b000 	movt	fp, #0
 308:	e1cd42f4 	strd	r4, [sp, #36]	@ 0x24
 30c:	e3a01034 	mov	r1, #52	@ 0x34
 310:	e1a00006 	mov	r0, r6
 314:	e58d9028 	str	r9, [sp, #40]	@ 0x28
 318:	e58d203c 	str	r2, [sp, #60]	@ 0x3c
 31c:	e3a02d37 	mov	r2, #3520	@ 0xdc0
 320:	e1c840f8 	strd	r4, [r8, #8]
 324:	e1c841f0 	strd	r4, [r8, #16]
 328:	e58db030 	str	fp, [sp, #48]	@ 0x30
 32c:	e5cd3038 	strb	r3, [sp, #56]	@ 0x38
 330:	ebfffffe 	bl	0 <devm_kmalloc>
	if (!mux)
 334:	e2504000 	subs	r4, r0, #0
 338:	e3a03003 	mov	r3, #3
 33c:	0a0000b0 	beq	604 <krait_add_clks+0x604>
	init.name = kasprintf(GFP_KERNEL, "krait%s_pri_mux", s);
 340:	e59d5000 	ldr	r5, [sp]
 344:	e3001000 	movw	r1, #0
 348:	e3a00d33 	mov	r0, #3264	@ 0xcc0
	mux->offset = offset;
 34c:	e59d2004 	ldr	r2, [sp, #4]
	init.name = kasprintf(GFP_KERNEL, "krait%s_pri_mux", s);
 350:	e3401000 	movt	r1, #0
	mux->mask = 0x3;
 354:	e1c420f4 	strd	r2, [r4, #4]
	mux->lpl = id >= 0;
 358:	e59d300c 	ldr	r3, [sp, #12]
	init.name = kasprintf(GFP_KERNEL, "krait%s_pri_mux", s);
 35c:	e1a02005 	mov	r2, r5
	mux->lpl = id >= 0;
 360:	e5c43014 	strb	r3, [r4, #20]
	mux->parent_map = pri_mux_map;
 364:	e3003000 	movw	r3, #0
 368:	e3403000 	movt	r3, #0
	mux->hw.init = &init;
 36c:	e5848024 	str	r8, [r4, #36]	@ 0x24
	mux->parent_map = pri_mux_map;
 370:	e5843000 	str	r3, [r4]
	mux->shift = 0;
 374:	e3a03000 	mov	r3, #0
 378:	e584300c 	str	r3, [r4, #12]
	mux->safe_sel = 2;
 37c:	e3a03002 	mov	r3, #2
 380:	e5c43015 	strb	r3, [r4, #21]
	init.name = kasprintf(GFP_KERNEL, "krait%s_pri_mux", s);
 384:	ebfffffe 	bl	0 <kasprintf>
	if (!init.name)
 388:	e3500000 	cmp	r0, #0
	init.name = kasprintf(GFP_KERNEL, "krait%s_pri_mux", s);
 38c:	e58d0024 	str	r0, [sp, #36]	@ 0x24
	if (!init.name)
 390:	0a00009b 	beq	604 <krait_add_clks+0x604>
	hfpll_name = kasprintf(GFP_KERNEL, "hfpll%s", s);
 394:	e3001000 	movw	r1, #0
 398:	e1a02005 	mov	r2, r5
 39c:	e3401000 	movt	r1, #0
 3a0:	e3a00d33 	mov	r0, #3264	@ 0xcc0
 3a4:	ebfffffe 	bl	0 <kasprintf>
	if (!hfpll_name) {
 3a8:	e2505000 	subs	r5, r0, #0
 3ac:	0a000096 	beq	60c <krait_add_clks+0x60c>
	ret = devm_clk_hw_register(dev, &mux->hw);
 3b0:	e1a00006 	mov	r0, r6
	p_data[0].fw_name = hfpll_name;
 3b4:	e58b5004 	str	r5, [fp, #4]
	p_data[1].hw = hfpll_div;
 3b8:	e58b7010 	str	r7, [fp, #16]
	ret = devm_clk_hw_register(dev, &mux->hw);
 3bc:	e284701c 	add	r7, r4, #28
 3c0:	e1a01007 	mov	r1, r7
	p_data[0].fw_name = hfpll_name;
 3c4:	e58b5008 	str	r5, [fp, #8]
	p_data[2].hw = sec_mux;
 3c8:	e58ba020 	str	sl, [fp, #32]
	ret = devm_clk_hw_register(dev, &mux->hw);
 3cc:	ebfffffe 	bl	0 <devm_clk_hw_register>
	if (ret) {
 3d0:	e3500000 	cmp	r0, #0
 3d4:	11a07000 	movne	r7, r0
 3d8:	0a00000d 	beq	414 <krait_add_clks+0x414>
	kfree(hfpll_name);
 3dc:	e1a00005 	mov	r0, r5
 3e0:	ebfffffe 	bl	0 <kfree>
	kfree(init.name);
 3e4:	e59d0024 	ldr	r0, [sp, #36]	@ 0x24
 3e8:	ebfffffe 	bl	0 <kfree>
	kfree(p);
 3ec:	e59d0014 	ldr	r0, [sp, #20]
 3f0:	ebfffffe 	bl	0 <kfree>
}
 3f4:	e1a00007 	mov	r0, r7
 3f8:	e28dd044 	add	sp, sp, #68	@ 0x44
 3fc:	e1cd40d0 	ldrd	r4, [sp]
 400:	e1cd60d8 	ldrd	r6, [sp, #8]
 404:	e1cd81d0 	ldrd	r8, [sp, #16]
 408:	e1cda1d8 	ldrd	sl, [sp, #24]
 40c:	e28dd020 	add	sp, sp, #32
 410:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
	mux->clk_nb.notifier_call = krait_notifier_cb;
 414:	e3003000 	movw	r3, #0
 418:	e1a02004 	mov	r2, r4
	ret = krait_notifier_register(dev, mux->hw.clk, mux);
 41c:	e5941020 	ldr	r1, [r4, #32]
	mux->clk_nb.notifier_call = krait_notifier_cb;
 420:	e3403000 	movt	r3, #0
	ret = devm_clk_notifier_register(dev, clk, &mux->clk_nb);
 424:	e1a00006 	mov	r0, r6
	mux->clk_nb.notifier_call = krait_notifier_cb;
 428:	e5a23028 	str	r3, [r2, #40]!	@ 0x28
	ret = devm_clk_notifier_register(dev, clk, &mux->clk_nb);
 42c:	ebfffffe 	bl	0 <devm_clk_notifier_register>
	if (ret)
 430:	e2504000 	subs	r4, r0, #0
 434:	0affffe8 	beq	3dc <krait_add_clks+0x3dc>
		dev_err(dev, "failed to register clock notifier: %d\n", ret);
 438:	e3001000 	movw	r1, #0
 43c:	e1a02004 	mov	r2, r4
 440:	e1a00006 	mov	r0, r6
 444:	e3401000 	movt	r1, #0
 448:	e1a07004 	mov	r7, r4
 44c:	ebfffffe 	bl	0 <_dev_err>
 450:	eaffffe1 	b	3dc <krait_add_clks+0x3dc>
		parent_name = kasprintf(GFP_KERNEL, "acpu%s_aux", s);
 454:	e3001000 	movw	r1, #0
 458:	e59d2000 	ldr	r2, [sp]
 45c:	e3a00d33 	mov	r0, #3264	@ 0xcc0
 460:	e3401000 	movt	r1, #0
 464:	ebfffffe 	bl	0 <kasprintf>
		if (!parent_name) {
 468:	e2503000 	subs	r3, r0, #0
 46c:	e58d3018 	str	r3, [sp, #24]
 470:	0a000061 	beq	5fc <krait_add_clks+0x5fc>
	ret = devm_clk_hw_register(dev, &mux->hw);
 474:	e284a01c 	add	sl, r4, #28
 478:	e1a00006 	mov	r0, r6
		sec_mux_list[1].fw_name = parent_name;
 47c:	e5853014 	str	r3, [r5, #20]
	ret = devm_clk_hw_register(dev, &mux->hw);
 480:	e1a0100a 	mov	r1, sl
		sec_mux_list[1].fw_name = parent_name;
 484:	e5853018 	str	r3, [r5, #24]
	ret = devm_clk_hw_register(dev, &mux->hw);
 488:	ebfffffe 	bl	0 <devm_clk_hw_register>
	if (ret) {
 48c:	e3500000 	cmp	r0, #0
 490:	11a0a000 	movne	sl, r0
 494:	1a000047 	bne	5b8 <krait_add_clks+0x5b8>
	mux->clk_nb.notifier_call = krait_notifier_cb;
 498:	e3003000 	movw	r3, #0
 49c:	e1a02004 	mov	r2, r4
	ret = devm_clk_notifier_register(dev, clk, &mux->clk_nb);
 4a0:	e5941020 	ldr	r1, [r4, #32]
	mux->clk_nb.notifier_call = krait_notifier_cb;
 4a4:	e3403000 	movt	r3, #0
	ret = devm_clk_notifier_register(dev, clk, &mux->clk_nb);
 4a8:	e1a00006 	mov	r0, r6
	mux->clk_nb.notifier_call = krait_notifier_cb;
 4ac:	e5a23028 	str	r3, [r2, #40]!	@ 0x28
	ret = devm_clk_notifier_register(dev, clk, &mux->clk_nb);
 4b0:	ebfffffe 	bl	0 <devm_clk_notifier_register>
	if (ret)
 4b4:	e2502000 	subs	r2, r0, #0
 4b8:	1a000057 	bne	61c <krait_add_clks+0x61c>
	if (id < 0)
 4bc:	e59d3008 	ldr	r3, [sp, #8]
 4c0:	e3530000 	cmp	r3, #0
 4c4:	aa00003e 	bge	5c4 <krait_add_clks+0x5c4>
		for_each_online_cpu(cpu)
 4c8:	e3001000 	movw	r1, #0
 4cc:	e300b000 	movw	fp, #0
 4d0:	e58da008 	str	sl, [sp, #8]
 4d4:	e3401000 	movt	r1, #0
 4d8:	e340b000 	movt	fp, #0
 4dc:	e58d701c 	str	r7, [sp, #28]
 4e0:	e1a0a001 	mov	sl, r1
 4e4:	ea000000 	b	4ec <krait_add_clks+0x4ec>
 4e8:	e2852001 	add	r2, r5, #1
 4ec:	e59b1000 	ldr	r1, [fp]
 4f0:	e1a0000a 	mov	r0, sl
 4f4:	ebfffffe 	bl	0 <_find_next_bit_le>
 4f8:	e59b3000 	ldr	r3, [fp]
 4fc:	e1a05000 	mov	r5, r0
 500:	e1500003 	cmp	r0, r3
 504:	2a000026 	bcs	5a4 <krait_add_clks+0x5a4>
			clk_prepare_enable(mux->hw.clk);
 508:	e5947020 	ldr	r7, [r4, #32]
	ret = clk_prepare(clk);
 50c:	e1a00007 	mov	r0, r7
 510:	ebfffffe 	bl	0 <clk_prepare>
 514:	e1a03000 	mov	r3, r0
	ret = clk_enable(clk);
 518:	e1a00007 	mov	r0, r7
	if (ret)
 51c:	e3530000 	cmp	r3, #0
 520:	1afffff0 	bne	4e8 <krait_add_clks+0x4e8>
	ret = clk_enable(clk);
 524:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 528:	e3500000 	cmp	r0, #0
 52c:	0affffed 	beq	4e8 <krait_add_clks+0x4e8>
		clk_unprepare(clk);
 530:	e1a00007 	mov	r0, r7
 534:	ebfffffe 	bl	0 <clk_unprepare>
 538:	eaffffea 	b	4e8 <krait_add_clks+0x4e8>
		clk_prepare_enable(div->hw.clk);
 53c:	e5994018 	ldr	r4, [r9, #24]
	ret = clk_prepare(clk);
 540:	e1a00004 	mov	r0, r4
 544:	ebfffffe 	bl	0 <clk_prepare>
	if (ret)
 548:	e3500000 	cmp	r0, #0
 54c:	1affff1e 	bne	1cc <krait_add_clks+0x1cc>
	ret = clk_enable(clk);
 550:	e1a00004 	mov	r0, r4
 554:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 558:	e3500000 	cmp	r0, #0
 55c:	0affff1a 	beq	1cc <krait_add_clks+0x1cc>
		clk_unprepare(clk);
 560:	e1a00004 	mov	r0, r4
 564:	ebfffffe 	bl	0 <clk_unprepare>
 568:	eaffff17 	b	1cc <krait_add_clks+0x1cc>
		s = "_l2";
 56c:	e3003000 	movw	r3, #0
	void *p = NULL;
 570:	e3a02000 	mov	r2, #0
		s = "_l2";
 574:	e3403000 	movt	r3, #0
 578:	e58d3000 	str	r3, [sp]
		offset = 0x500;
 57c:	e3a03c05 	mov	r3, #1280	@ 0x500
	void *p = NULL;
 580:	e58d2014 	str	r2, [sp, #20]
		offset = 0x500;
 584:	e58d3004 	str	r3, [sp, #4]
 588:	eafffeb3 	b	5c <krait_add_clks+0x5c>
	    of_machine_is_compatible("qcom,apq8064"))
 58c:	e3000000 	movw	r0, #0
 590:	e3400000 	movt	r0, #0
 594:	ebfffffe 	bl	0 <of_machine_is_compatible>
	if (of_machine_is_compatible("qcom,ipq8064") ||
 598:	e3500000 	cmp	r0, #0
 59c:	1affff38 	bne	284 <krait_add_clks+0x284>
 5a0:	eaffff39 	b	28c <krait_add_clks+0x28c>
 5a4:	e59da008 	ldr	sl, [sp, #8]
 5a8:	e59d701c 	ldr	r7, [sp, #28]
	if (unique_aux)
 5ac:	e59d3010 	ldr	r3, [sp, #16]
 5b0:	e3530000 	cmp	r3, #0
 5b4:	0affff49 	beq	2e0 <krait_add_clks+0x2e0>
		kfree(parent_name);
 5b8:	e59d0018 	ldr	r0, [sp, #24]
 5bc:	ebfffffe 	bl	0 <kfree>
 5c0:	eaffff46 	b	2e0 <krait_add_clks+0x2e0>
		clk_prepare_enable(mux->hw.clk);
 5c4:	e5944020 	ldr	r4, [r4, #32]
	ret = clk_prepare(clk);
 5c8:	e1a00004 	mov	r0, r4
 5cc:	ebfffffe 	bl	0 <clk_prepare>
	if (ret)
 5d0:	e3500000 	cmp	r0, #0
 5d4:	1afffff4 	bne	5ac <krait_add_clks+0x5ac>
	ret = clk_enable(clk);
 5d8:	e1a00004 	mov	r0, r4
 5dc:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 5e0:	e3500000 	cmp	r0, #0
 5e4:	0afffff0 	beq	5ac <krait_add_clks+0x5ac>
		clk_unprepare(clk);
 5e8:	e1a00004 	mov	r0, r4
 5ec:	ebfffffe 	bl	0 <clk_unprepare>
 5f0:	eaffffed 	b	5ac <krait_add_clks+0x5ac>
		pri_mux = sec_mux;
 5f4:	e1a0700a 	mov	r7, sl
 5f8:	eaffff7b 	b	3ec <krait_add_clks+0x3ec>
	kfree(init.name);
 5fc:	e59d0024 	ldr	r0, [sp, #36]	@ 0x24
 600:	ebfffffe 	bl	0 <kfree>
		return ERR_PTR(-ENOMEM);
 604:	e3e0700b 	mvn	r7, #11
 608:	eaffff77 	b	3ec <krait_add_clks+0x3ec>
		clk = ERR_PTR(-ENOMEM);
 60c:	e3e0700b 	mvn	r7, #11
 610:	eaffff73 	b	3e4 <krait_add_clks+0x3e4>
			return ERR_PTR(-ENOMEM);
 614:	e3e0700b 	mvn	r7, #11
 618:	eaffff75 	b	3f4 <krait_add_clks+0x3f4>
		dev_err(dev, "failed to register clock notifier: %d\n", ret);
 61c:	e3001000 	movw	r1, #0
 620:	e1a00006 	mov	r0, r6
 624:	e58d2008 	str	r2, [sp, #8]
 628:	e3401000 	movt	r1, #0
 62c:	ebfffffe 	bl	0 <_dev_err>
 630:	e59d2008 	ldr	r2, [sp, #8]
 634:	e1a0a002 	mov	sl, r2
		goto err_clk;
 638:	eaffffdb 	b	5ac <krait_add_clks+0x5ac>

Disassembly of section .text.krait_cc_probe:

00000000 <krait_cc_probe>:
{
   0:	e16d42f0 	strd	r4, [sp, #-32]!	@ 0xffffffe0
   4:	e1cd60f8 	strd	r6, [sp, #8]
   8:	e58da018 	str	sl, [sp, #24]
   c:	e1a0a000 	mov	sl, r0
	id = of_match_device(krait_cc_match_table, dev);
  10:	e3000000 	movw	r0, #0
{
  14:	e1cd81f0 	strd	r8, [sp, #16]
	struct device *dev = &pdev->dev;
  18:	e28a8010 	add	r8, sl, #16
	id = of_match_device(krait_cc_match_table, dev);
  1c:	e3400000 	movt	r0, #0
  20:	e1a01008 	mov	r1, r8
{
  24:	e58de01c 	str	lr, [sp, #28]
  28:	e24dd010 	sub	sp, sp, #16
	id = of_match_device(krait_cc_match_table, dev);
  2c:	ebfffffe 	bl	0 <of_match_device>
	if (!id)
  30:	e2507000 	subs	r7, r0, #0
  34:	0a000086 	beq	254 <krait_cc_probe+0x254>
	clk = clk_get(dev, "qsb");
  38:	e3001000 	movw	r1, #0
  3c:	e1a00008 	mov	r0, r8
  40:	e3401000 	movt	r1, #0
  44:	ebfffffe 	bl	0 <clk_get>
	if (IS_ERR(clk))
  48:	e3700a01 	cmn	r0, #4096	@ 0x1000
  4c:	8a000074 	bhi	224 <krait_cc_probe+0x224>
	qsb_rate = clk_get_rate(clk);
  50:	ebfffffe 	bl	0 <clk_get_rate>
	if (!id->data) {
  54:	e59730c0 	ldr	r3, [r7, #192]	@ 0xc0
  58:	e3530000 	cmp	r3, #0
  5c:	0a000063 	beq	1f0 <krait_cc_probe+0x1f0>
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;

	return devm_kmalloc(dev, bytes, flags);
  60:	e3a02d37 	mov	r2, #3520	@ 0xdc0
  64:	e3a01014 	mov	r1, #20
  68:	e1a00008 	mov	r0, r8
  6c:	ebfffffe 	bl	0 <devm_kmalloc>
	if (!clks)
  70:	e2506000 	subs	r6, r0, #0
  74:	0a000078 	beq	25c <krait_cc_probe+0x25c>
  78:	e3005000 	movw	r5, #0
	for_each_possible_cpu(cpu) {
  7c:	e3009000 	movw	r9, #0
  80:	e3a02000 	mov	r2, #0
  84:	e3405000 	movt	r5, #0
  88:	e3409000 	movt	r9, #0
  8c:	ea000003 	b	a0 <krait_cc_probe+0xa0>
		mux = krait_add_clks(dev, cpu, id->data);
  90:	ebfffffe 	bl	0 <krait_cc_probe>
		clks[cpu] = mux->clk;
  94:	e5903004 	ldr	r3, [r0, #4]
	for_each_possible_cpu(cpu) {
  98:	e2842001 	add	r2, r4, #1
		clks[cpu] = mux->clk;
  9c:	e7863104 	str	r3, [r6, r4, lsl #2]
	for_each_possible_cpu(cpu) {
  a0:	e5951000 	ldr	r1, [r5]
  a4:	e1a00009 	mov	r0, r9
  a8:	ebfffffe 	bl	0 <_find_next_bit_le>
		mux = krait_add_clks(dev, cpu, id->data);
  ac:	e59720c0 	ldr	r2, [r7, #192]	@ 0xc0
	for_each_possible_cpu(cpu) {
  b0:	e1a04000 	mov	r4, r0
		mux = krait_add_clks(dev, cpu, id->data);
  b4:	e1a00008 	mov	r0, r8
	for_each_possible_cpu(cpu) {
  b8:	e5953000 	ldr	r3, [r5]
		mux = krait_add_clks(dev, cpu, id->data);
  bc:	e1a01004 	mov	r1, r4
  c0:	e2522000 	subs	r2, r2, #0
  c4:	13a02001 	movne	r2, #1
	for_each_possible_cpu(cpu) {
  c8:	e1540003 	cmp	r4, r3
  cc:	3affffef 	bcc	90 <krait_cc_probe+0x90>
	l2_pri_mux = krait_add_clks(dev, -1, id->data);
  d0:	e3e01000 	mvn	r1, #0
  d4:	ebfffffe 	bl	0 <krait_cc_probe>
	if (IS_ERR(l2_pri_mux))
  d8:	e3700a01 	cmn	r0, #4096	@ 0x1000
  dc:	8a000039 	bhi	1c8 <krait_cc_probe+0x1c8>
	clks[l2_mux] = l2_pri_mux->clk;
  e0:	e5903004 	ldr	r3, [r0, #4]
	for_each_online_cpu(cpu) {
  e4:	e3009000 	movw	r9, #0
  e8:	e3a02000 	mov	r2, #0
  ec:	e3409000 	movt	r9, #0
	clks[l2_mux] = l2_pri_mux->clk;
  f0:	e5863010 	str	r3, [r6, #16]
	for_each_online_cpu(cpu) {
  f4:	ea00000a 	b	124 <krait_cc_probe+0x124>
		WARN(clk_prepare_enable(clks[cpu]),
  f8:	e7967104 	ldr	r7, [r6, r4, lsl #2]
	ret = clk_prepare(clk);
  fc:	e1a00007 	mov	r0, r7
 100:	ebfffffe 	bl	0 <clk_prepare>
 104:	e1a03000 	mov	r3, r0
	ret = clk_enable(clk);
 108:	e1a00007 	mov	r0, r7
	if (ret)
 10c:	e3530000 	cmp	r3, #0
 110:	1a000002 	bne	120 <krait_cc_probe+0x120>
	ret = clk_enable(clk);
 114:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 118:	e3500000 	cmp	r0, #0
 11c:	1a000030 	bne	1e4 <krait_cc_probe+0x1e4>
	for_each_online_cpu(cpu) {
 120:	e2842001 	add	r2, r4, #1
 124:	e5951000 	ldr	r1, [r5]
 128:	e1a00009 	mov	r0, r9
 12c:	ebfffffe 	bl	0 <_find_next_bit_le>
 130:	e5953000 	ldr	r3, [r5]
 134:	e1a04000 	mov	r4, r0
 138:	e1500003 	cmp	r0, r3
 13c:	2a00000c 	bcs	174 <krait_cc_probe+0x174>
		clk_prepare_enable(clks[l2_mux]);
 140:	e5967010 	ldr	r7, [r6, #16]
	ret = clk_prepare(clk);
 144:	e1a00007 	mov	r0, r7
 148:	ebfffffe 	bl	0 <clk_prepare>
 14c:	e1a03000 	mov	r3, r0
	ret = clk_enable(clk);
 150:	e1a00007 	mov	r0, r7
	if (ret)
 154:	e3530000 	cmp	r3, #0
 158:	1affffe6 	bne	f8 <krait_cc_probe+0xf8>
	ret = clk_enable(clk);
 15c:	ebfffffe 	bl	0 <clk_enable>
	if (ret)
 160:	e3500000 	cmp	r0, #0
 164:	0affffe3 	beq	f8 <krait_cc_probe+0xf8>
		clk_unprepare(clk);
 168:	e1a00007 	mov	r0, r7
 16c:	ebfffffe 	bl	0 <clk_unprepare>
 170:	eaffffe0 	b	f8 <krait_cc_probe+0xf8>
		clk = clks[cpu];
 174:	e5964000 	ldr	r4, [r6]
		if (!clk)
 178:	e3540000 	cmp	r4, #0
 17c:	1a000056 	bne	2dc <krait_cc_probe+0x2dc>
		clk = clks[cpu];
 180:	e5964004 	ldr	r4, [r6, #4]
		if (!clk)
 184:	e3540000 	cmp	r4, #0
 188:	1a000099 	bne	3f4 <krait_cc_probe+0x3f4>
		clk = clks[cpu];
 18c:	e5964008 	ldr	r4, [r6, #8]
		if (!clk)
 190:	e3540000 	cmp	r4, #0
 194:	1a000073 	bne	368 <krait_cc_probe+0x368>
		clk = clks[cpu];
 198:	e596400c 	ldr	r4, [r6, #12]
		if (!clk)
 19c:	e3540000 	cmp	r4, #0
 1a0:	1a0000b6 	bne	480 <krait_cc_probe+0x480>
		clk = clks[cpu];
 1a4:	e5964010 	ldr	r4, [r6, #16]
		if (!clk)
 1a8:	e3540000 	cmp	r4, #0
 1ac:	1a00002c 	bne	264 <krait_cc_probe+0x264>
	of_clk_add_provider(dev->of_node, krait_of_get, clks);
 1b0:	e3001000 	movw	r1, #0
 1b4:	e59a00e8 	ldr	r0, [sl, #232]	@ 0xe8
 1b8:	e1a02006 	mov	r2, r6
 1bc:	e3401000 	movt	r1, #0
 1c0:	ebfffffe 	bl	0 <of_clk_add_provider>
	return 0;
 1c4:	e3a00000 	mov	r0, #0
}
 1c8:	e28dd010 	add	sp, sp, #16
 1cc:	e1cd40d0 	ldrd	r4, [sp]
 1d0:	e1cd60d8 	ldrd	r6, [sp, #8]
 1d4:	e1cd81d0 	ldrd	r8, [sp, #16]
 1d8:	e59da018 	ldr	sl, [sp, #24]
 1dc:	e28dd01c 	add	sp, sp, #28
 1e0:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
 1e4:	e1a00007 	mov	r0, r7
 1e8:	ebfffffe 	bl	0 <clk_unprepare>
 1ec:	eaffffcb 	b	120 <krait_cc_probe+0x120>
		clk = clk_register_fixed_factor(dev, "acpu_aux",
 1f0:	e3a02002 	mov	r2, #2
 1f4:	e3001000 	movw	r1, #0
 1f8:	e3401000 	movt	r1, #0
 1fc:	e1a00008 	mov	r0, r8
 200:	e58d2004 	str	r2, [sp, #4]
 204:	e3a02001 	mov	r2, #1
 208:	e58d2000 	str	r2, [sp]
 20c:	e3002000 	movw	r2, #0
 210:	e3402000 	movt	r2, #0
 214:	ebfffffe 	bl	0 <clk_register_fixed_factor>
		if (IS_ERR(clk))
 218:	e3700a01 	cmn	r0, #4096	@ 0x1000
 21c:	9affff8f 	bls	60 <krait_cc_probe+0x60>
 220:	eaffffe8 	b	1c8 <krait_cc_probe+0x1c8>
		clk = clk_register_fixed_rate(dev, "qsb", NULL, 0, QSB_RATE);
 224:	e3a02de9 	mov	r2, #14912	@ 0x3a40
 228:	e3a03000 	mov	r3, #0
 22c:	e3402d69 	movt	r2, #3433	@ 0xd69
 230:	e3001000 	movw	r1, #0
 234:	e3401000 	movt	r1, #0
 238:	e1a00008 	mov	r0, r8
 23c:	e58d2000 	str	r2, [sp]
 240:	e1a02003 	mov	r2, r3
 244:	ebfffffe 	bl	0 <clk_register_fixed_rate>
	if (IS_ERR(clk))
 248:	e3700a01 	cmn	r0, #4096	@ 0x1000
 24c:	8affffdd 	bhi	1c8 <krait_cc_probe+0x1c8>
 250:	eaffff7e 	b	50 <krait_cc_probe+0x50>
		return -ENODEV;
 254:	e3e00012 	mvn	r0, #18
 258:	eaffffda 	b	1c8 <krait_cc_probe+0x1c8>
		return -ENOMEM;
 25c:	e3e0000b 	mvn	r0, #11
 260:	eaffffd8 	b	1c8 <krait_cc_probe+0x1c8>
		cur_rate = clk_get_rate(clk);
 264:	e1a00004 	mov	r0, r4
 268:	ebfffffe 	bl	0 <clk_get_rate>
		if (cur_rate < 384000000) {
 26c:	e3053fff 	movw	r3, #24575	@ 0x5fff
		cur_rate = clk_get_rate(clk);
 270:	e1a05000 	mov	r5, r0
		if (cur_rate < 384000000) {
 274:	e34136e3 	movt	r3, #5859	@ 0x16e3
 278:	e1500003 	cmp	r0, r3
 27c:	9a0000a2 	bls	50c <krait_cc_probe+0x50c>
		clk_set_rate(clk, AUX_RATE);
 280:	e3a01b02 	mov	r1, #2048	@ 0x800
 284:	e1a00004 	mov	r0, r4
 288:	e3421faf 	movt	r1, #12207	@ 0x2faf
 28c:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, HFPLL_RATE);
 290:	e3a01c46 	mov	r1, #17920	@ 0x4600
 294:	e1a00004 	mov	r0, r4
 298:	e34213c3 	movt	r1, #9155	@ 0x23c3
 29c:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, cur_rate);
 2a0:	e1a01005 	mov	r1, r5
 2a4:	e1a00004 	mov	r0, r4
 2a8:	ebfffffe 	bl	0 <clk_set_rate>
		dev_info(dev, "%s @ %lu KHz\n", cpu < 4 ? cpu_s : l2_s,
 2ac:	e1a00004 	mov	r0, r4
 2b0:	ebfffffe 	bl	0 <clk_get_rate>
 2b4:	e1a02000 	mov	r2, r0
 2b8:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
 2bc:	e3001000 	movw	r1, #0
 2c0:	e1a00008 	mov	r0, r8
 2c4:	e733f312 	udiv	r3, r2, r3
 2c8:	e3002000 	movw	r2, #0
 2cc:	e3401000 	movt	r1, #0
 2d0:	e3402000 	movt	r2, #0
 2d4:	ebfffffe 	bl	0 <_dev_info>
 2d8:	eaffffb4 	b	1b0 <krait_cc_probe+0x1b0>
			snprintf(cpu_s, 5, "CPU%d", cpu);
 2dc:	e28d500b 	add	r5, sp, #11
 2e0:	e3002000 	movw	r2, #0
 2e4:	e3a03000 	mov	r3, #0
 2e8:	e3402000 	movt	r2, #0
 2ec:	e3a01005 	mov	r1, #5
 2f0:	e1a00005 	mov	r0, r5
 2f4:	ebfffffe 	bl	0 <snprintf>
		cur_rate = clk_get_rate(clk);
 2f8:	e1a00004 	mov	r0, r4
 2fc:	ebfffffe 	bl	0 <clk_get_rate>
		if (cur_rate < 384000000) {
 300:	e3053fff 	movw	r3, #24575	@ 0x5fff
		cur_rate = clk_get_rate(clk);
 304:	e1a07000 	mov	r7, r0
		if (cur_rate < 384000000) {
 308:	e34136e3 	movt	r3, #5859	@ 0x16e3
 30c:	e1500003 	cmp	r0, r3
 310:	9a000086 	bls	530 <krait_cc_probe+0x530>
		clk_set_rate(clk, AUX_RATE);
 314:	e3a01b02 	mov	r1, #2048	@ 0x800
 318:	e1a00004 	mov	r0, r4
 31c:	e3421faf 	movt	r1, #12207	@ 0x2faf
 320:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, HFPLL_RATE);
 324:	e3a01c46 	mov	r1, #17920	@ 0x4600
 328:	e1a00004 	mov	r0, r4
 32c:	e34213c3 	movt	r1, #9155	@ 0x23c3
 330:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, cur_rate);
 334:	e1a01007 	mov	r1, r7
 338:	e1a00004 	mov	r0, r4
 33c:	ebfffffe 	bl	0 <clk_set_rate>
		dev_info(dev, "%s @ %lu KHz\n", cpu < 4 ? cpu_s : l2_s,
 340:	e1a00004 	mov	r0, r4
 344:	ebfffffe 	bl	0 <clk_get_rate>
 348:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
 34c:	e3001000 	movw	r1, #0
 350:	e1a02005 	mov	r2, r5
 354:	e3401000 	movt	r1, #0
 358:	e733f310 	udiv	r3, r0, r3
 35c:	e1a00008 	mov	r0, r8
 360:	ebfffffe 	bl	0 <_dev_info>
 364:	eaffff85 	b	180 <krait_cc_probe+0x180>
			snprintf(cpu_s, 5, "CPU%d", cpu);
 368:	e28d500b 	add	r5, sp, #11
 36c:	e3002000 	movw	r2, #0
 370:	e3a03002 	mov	r3, #2
 374:	e3402000 	movt	r2, #0
 378:	e3a01005 	mov	r1, #5
 37c:	e1a00005 	mov	r0, r5
 380:	ebfffffe 	bl	0 <snprintf>
		cur_rate = clk_get_rate(clk);
 384:	e1a00004 	mov	r0, r4
 388:	ebfffffe 	bl	0 <clk_get_rate>
		if (cur_rate < 384000000) {
 38c:	e3053fff 	movw	r3, #24575	@ 0x5fff
		cur_rate = clk_get_rate(clk);
 390:	e1a07000 	mov	r7, r0
		if (cur_rate < 384000000) {
 394:	e34136e3 	movt	r3, #5859	@ 0x16e3
 398:	e1500003 	cmp	r0, r3
 39c:	9a000073 	bls	570 <krait_cc_probe+0x570>
		clk_set_rate(clk, AUX_RATE);
 3a0:	e3a01b02 	mov	r1, #2048	@ 0x800
 3a4:	e1a00004 	mov	r0, r4
 3a8:	e3421faf 	movt	r1, #12207	@ 0x2faf
 3ac:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, HFPLL_RATE);
 3b0:	e3a01c46 	mov	r1, #17920	@ 0x4600
 3b4:	e1a00004 	mov	r0, r4
 3b8:	e34213c3 	movt	r1, #9155	@ 0x23c3
 3bc:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, cur_rate);
 3c0:	e1a01007 	mov	r1, r7
 3c4:	e1a00004 	mov	r0, r4
 3c8:	ebfffffe 	bl	0 <clk_set_rate>
		dev_info(dev, "%s @ %lu KHz\n", cpu < 4 ? cpu_s : l2_s,
 3cc:	e1a00004 	mov	r0, r4
 3d0:	ebfffffe 	bl	0 <clk_get_rate>
 3d4:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
 3d8:	e3001000 	movw	r1, #0
 3dc:	e1a02005 	mov	r2, r5
 3e0:	e3401000 	movt	r1, #0
 3e4:	e733f310 	udiv	r3, r0, r3
 3e8:	e1a00008 	mov	r0, r8
 3ec:	ebfffffe 	bl	0 <_dev_info>
 3f0:	eaffff68 	b	198 <krait_cc_probe+0x198>
			snprintf(cpu_s, 5, "CPU%d", cpu);
 3f4:	e28d500b 	add	r5, sp, #11
 3f8:	e3002000 	movw	r2, #0
 3fc:	e3a03001 	mov	r3, #1
 400:	e3402000 	movt	r2, #0
 404:	e3a01005 	mov	r1, #5
 408:	e1a00005 	mov	r0, r5
 40c:	ebfffffe 	bl	0 <snprintf>
		cur_rate = clk_get_rate(clk);
 410:	e1a00004 	mov	r0, r4
 414:	ebfffffe 	bl	0 <clk_get_rate>
		if (cur_rate < 384000000) {
 418:	e3053fff 	movw	r3, #24575	@ 0x5fff
		cur_rate = clk_get_rate(clk);
 41c:	e1a07000 	mov	r7, r0
		if (cur_rate < 384000000) {
 420:	e34136e3 	movt	r3, #5859	@ 0x16e3
 424:	e1500003 	cmp	r0, r3
 428:	9a000048 	bls	550 <krait_cc_probe+0x550>
		clk_set_rate(clk, AUX_RATE);
 42c:	e3a01b02 	mov	r1, #2048	@ 0x800
 430:	e1a00004 	mov	r0, r4
 434:	e3421faf 	movt	r1, #12207	@ 0x2faf
 438:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, HFPLL_RATE);
 43c:	e3a01c46 	mov	r1, #17920	@ 0x4600
 440:	e1a00004 	mov	r0, r4
 444:	e34213c3 	movt	r1, #9155	@ 0x23c3
 448:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, cur_rate);
 44c:	e1a01007 	mov	r1, r7
 450:	e1a00004 	mov	r0, r4
 454:	ebfffffe 	bl	0 <clk_set_rate>
		dev_info(dev, "%s @ %lu KHz\n", cpu < 4 ? cpu_s : l2_s,
 458:	e1a00004 	mov	r0, r4
 45c:	ebfffffe 	bl	0 <clk_get_rate>
 460:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
 464:	e3001000 	movw	r1, #0
 468:	e1a02005 	mov	r2, r5
 46c:	e3401000 	movt	r1, #0
 470:	e733f310 	udiv	r3, r0, r3
 474:	e1a00008 	mov	r0, r8
 478:	ebfffffe 	bl	0 <_dev_info>
 47c:	eaffff42 	b	18c <krait_cc_probe+0x18c>
			snprintf(cpu_s, 5, "CPU%d", cpu);
 480:	e28d500b 	add	r5, sp, #11
 484:	e3002000 	movw	r2, #0
 488:	e3a03003 	mov	r3, #3
 48c:	e3402000 	movt	r2, #0
 490:	e3a01005 	mov	r1, #5
 494:	e1a00005 	mov	r0, r5
 498:	ebfffffe 	bl	0 <snprintf>
		cur_rate = clk_get_rate(clk);
 49c:	e1a00004 	mov	r0, r4
 4a0:	ebfffffe 	bl	0 <clk_get_rate>
		if (cur_rate < 384000000) {
 4a4:	e3053fff 	movw	r3, #24575	@ 0x5fff
		cur_rate = clk_get_rate(clk);
 4a8:	e1a07000 	mov	r7, r0
		if (cur_rate < 384000000) {
 4ac:	e34136e3 	movt	r3, #5859	@ 0x16e3
 4b0:	e1500003 	cmp	r0, r3
 4b4:	9a000035 	bls	590 <krait_cc_probe+0x590>
		clk_set_rate(clk, AUX_RATE);
 4b8:	e3a01b02 	mov	r1, #2048	@ 0x800
 4bc:	e1a00004 	mov	r0, r4
 4c0:	e3421faf 	movt	r1, #12207	@ 0x2faf
 4c4:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, HFPLL_RATE);
 4c8:	e3a01c46 	mov	r1, #17920	@ 0x4600
 4cc:	e1a00004 	mov	r0, r4
 4d0:	e34213c3 	movt	r1, #9155	@ 0x23c3
 4d4:	ebfffffe 	bl	0 <clk_set_rate>
		clk_set_rate(clk, cur_rate);
 4d8:	e1a01007 	mov	r1, r7
 4dc:	e1a00004 	mov	r0, r4
 4e0:	ebfffffe 	bl	0 <clk_set_rate>
		dev_info(dev, "%s @ %lu KHz\n", cpu < 4 ? cpu_s : l2_s,
 4e4:	e1a00004 	mov	r0, r4
 4e8:	ebfffffe 	bl	0 <clk_get_rate>
 4ec:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
 4f0:	e3001000 	movw	r1, #0
 4f4:	e1a02005 	mov	r2, r5
 4f8:	e3401000 	movt	r1, #0
 4fc:	e733f310 	udiv	r3, r0, r3
 500:	e1a00008 	mov	r0, r8
 504:	ebfffffe 	bl	0 <_dev_info>
 508:	eaffff25 	b	1a4 <krait_cc_probe+0x1a4>
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 50c:	e3002000 	movw	r2, #0
 510:	e3001000 	movw	r1, #0
 514:	e3402000 	movt	r2, #0
 518:	e3401000 	movt	r1, #0
 51c:	e1a00008 	mov	r0, r8
			cur_rate = AUX_RATE;
 520:	e3a05b02 	mov	r5, #2048	@ 0x800
 524:	e3425faf 	movt	r5, #12207	@ 0x2faf
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 528:	ebfffffe 	bl	0 <_dev_info>
			cur_rate = AUX_RATE;
 52c:	eaffff53 	b	280 <krait_cc_probe+0x280>
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 530:	e3001000 	movw	r1, #0
 534:	e1a02005 	mov	r2, r5
 538:	e3401000 	movt	r1, #0
 53c:	e1a00008 	mov	r0, r8
			cur_rate = AUX_RATE;
 540:	e3a07b02 	mov	r7, #2048	@ 0x800
 544:	e3427faf 	movt	r7, #12207	@ 0x2faf
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 548:	ebfffffe 	bl	0 <_dev_info>
			cur_rate = AUX_RATE;
 54c:	eaffff70 	b	314 <krait_cc_probe+0x314>
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 550:	e3001000 	movw	r1, #0
 554:	e1a02005 	mov	r2, r5
 558:	e3401000 	movt	r1, #0
 55c:	e1a00008 	mov	r0, r8
			cur_rate = AUX_RATE;
 560:	e3a07b02 	mov	r7, #2048	@ 0x800
 564:	e3427faf 	movt	r7, #12207	@ 0x2faf
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 568:	ebfffffe 	bl	0 <_dev_info>
			cur_rate = AUX_RATE;
 56c:	eaffffae 	b	42c <krait_cc_probe+0x42c>
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 570:	e3001000 	movw	r1, #0
 574:	e1a02005 	mov	r2, r5
 578:	e3401000 	movt	r1, #0
 57c:	e1a00008 	mov	r0, r8
			cur_rate = AUX_RATE;
 580:	e3a07b02 	mov	r7, #2048	@ 0x800
 584:	e3427faf 	movt	r7, #12207	@ 0x2faf
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 588:	ebfffffe 	bl	0 <_dev_info>
			cur_rate = AUX_RATE;
 58c:	eaffff83 	b	3a0 <krait_cc_probe+0x3a0>
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 590:	e3001000 	movw	r1, #0
 594:	e1a02005 	mov	r2, r5
 598:	e3401000 	movt	r1, #0
 59c:	e1a00008 	mov	r0, r8
			cur_rate = AUX_RATE;
 5a0:	e3a07b02 	mov	r7, #2048	@ 0x800
 5a4:	e3427faf 	movt	r7, #12207	@ 0x2faf
			dev_info(dev, "%s @ Undefined rate. Forcing new rate.\n",
 5a8:	ebfffffe 	bl	0 <_dev_info>
			cur_rate = AUX_RATE;
 5ac:	eaffffc1 	b	4b8 <krait_cc_probe+0x4b8>
