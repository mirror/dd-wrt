
ar8327.o:     file format elf32-littlearm


Disassembly of section .text.ar8327_get_pad_cfg:

00000000 <ar8327_get_pad_cfg>:
extern const struct ar8xxx_mib_desc ar8236_mibs[39];
extern const struct switch_attr ar8xxx_sw_attr_vlan[1];

static u32
ar8327_get_pad_cfg(struct ar8327_pad_cfg *cfg)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	u32 t;

	if (!cfg)
   c:	e3500000 	cmp	r0, #0
  10:	0a00000e 	beq	50 <ar8327_get_pad_cfg+0x50>
		return 0;

	t = 0;
	switch (cfg->mode) {
  14:	e5903000 	ldr	r3, [r0]
  18:	e2433001 	sub	r3, r3, #1
  1c:	e3530008 	cmp	r3, #8
  20:	979ff103 	ldrls	pc, [pc, r3, lsl #2]
  24:	ea000055 	b	180 <ar8327_get_pad_cfg+0x180>
  28:	000000f8 	.word	0x000000f8
  2c:	00000114 	.word	0x00000114
  30:	00000138 	.word	0x00000138
  34:	00000098 	.word	0x00000098
  38:	000000bc 	.word	0x000000bc
  3c:	00000060 	.word	0x00000060
  40:	00000058 	.word	0x00000058
  44:	0000004c 	.word	0x0000004c
  48:	000000f0 	.word	0x000000f0
  4c:	e3a00802 	mov	r0, #131072	@ 0x20000
		t = AR8327_PAD_PHYX_MII_EN;
		break;
	}

	return t;
}
  50:	e24bd00c 	sub	sp, fp, #12
  54:	e89da800 	ldm	sp, {fp, sp, pc}
		t = AR8327_PAD_PHYX_GMII_EN;
  58:	e3a00801 	mov	r0, #65536	@ 0x10000
  5c:	eafffffb 	b	50 <ar8327_get_pad_cfg+0x50>
		t |= cfg->txclk_delay_sel << AR8327_PAD_RGMII_TXCLK_DELAY_SEL_S;
  60:	e590c00c 	ldr	ip, [r0, #12]
		t |= cfg->rxclk_delay_sel << AR8327_PAD_RGMII_RXCLK_DELAY_SEL_S;
  64:	e5903010 	ldr	r3, [r0, #16]
		if (cfg->txclk_delay_en)
  68:	e5d01007 	ldrb	r1, [r0, #7]
		if (cfg->rxclk_delay_en)
  6c:	e5d02008 	ldrb	r2, [r0, #8]
		t |= cfg->rxclk_delay_sel << AR8327_PAD_RGMII_RXCLK_DELAY_SEL_S;
  70:	e1a03a03 	lsl	r3, r3, #20
		if (cfg->txclk_delay_en)
  74:	e2611000 	rsb	r1, r1, #0
  78:	e1833b0c 	orr	r3, r3, ip, lsl #22
  7c:	e2011402 	and	r1, r1, #33554432	@ 0x2000000
		if (cfg->rxclk_delay_en)
  80:	e2622000 	rsb	r2, r2, #0
  84:	e2022401 	and	r2, r2, #16777216	@ 0x1000000
		if (cfg->txclk_delay_en)
  88:	e1833001 	orr	r3, r3, r1
  8c:	e1833002 	orr	r3, r3, r2
  90:	e3830301 	orr	r0, r3, #67108864	@ 0x4000000
  94:	eaffffed 	b	50 <ar8327_get_pad_cfg+0x50>
		if (cfg->rxclk_sel)
  98:	e5d03004 	ldrb	r3, [r0, #4]
		if (cfg->txclk_sel)
  9c:	e5d00005 	ldrb	r0, [r0, #5]
		t = AR8327_PAD_PHY_MII_EN;
  a0:	e3530000 	cmp	r3, #0
		if (cfg->txclk_sel)
  a4:	e2600000 	rsb	r0, r0, #0
		t = AR8327_PAD_PHY_MII_EN;
  a8:	13a03c05 	movne	r3, #1280	@ 0x500
  ac:	03a03b01 	moveq	r3, #1024	@ 0x400
		if (cfg->txclk_sel)
  b0:	e2000c02 	and	r0, r0, #512	@ 0x200
  b4:	e1800003 	orr	r0, r0, r3
  b8:	eaffffe4 	b	50 <ar8327_get_pad_cfg+0x50>
		if (cfg->txclk_sel)
  bc:	e5d03005 	ldrb	r3, [r0, #5]
		if (cfg->rxclk_sel)
  c0:	e5d02004 	ldrb	r2, [r0, #4]
		if (cfg->pipe_rxclk_sel)
  c4:	e5d01006 	ldrb	r1, [r0, #6]
		if (cfg->txclk_sel)
  c8:	e2633000 	rsb	r3, r3, #0
		if (cfg->rxclk_sel)
  cc:	e2622000 	rsb	r2, r2, #0
		if (cfg->txclk_sel)
  d0:	e2033a02 	and	r3, r3, #8192	@ 0x2000
		t = AR8327_PAD_PHY_GMII_EN;
  d4:	e3510000 	cmp	r1, #0
		if (cfg->rxclk_sel)
  d8:	e2022a01 	and	r2, r2, #4096	@ 0x1000
		t = AR8327_PAD_PHY_GMII_EN;
  dc:	13a00b12 	movne	r0, #18432	@ 0x4800
  e0:	03a00901 	moveq	r0, #16384	@ 0x4000
		if (cfg->txclk_sel)
  e4:	e1833002 	orr	r3, r3, r2
  e8:	e1830000 	orr	r0, r3, r0
  ec:	eaffffd7 	b	50 <ar8327_get_pad_cfg+0x50>
		t = AR8327_PAD_PHYX_MII_EN;
  f0:	e3a00701 	mov	r0, #262144	@ 0x40000
		break;
  f4:	eaffffd5 	b	50 <ar8327_get_pad_cfg+0x50>
		if (cfg->txclk_sel)
  f8:	e5d03005 	ldrb	r3, [r0, #5]
  fc:	e5d00004 	ldrb	r0, [r0, #4]
 100:	e2633000 	rsb	r3, r3, #0
 104:	e2800004 	add	r0, r0, #4
 108:	e2033002 	and	r3, r3, #2
 10c:	e1830000 	orr	r0, r3, r0
 110:	eaffffce 	b	50 <ar8327_get_pad_cfg+0x50>
		if (cfg->rxclk_sel)
 114:	e5d03004 	ldrb	r3, [r0, #4]
		if (cfg->txclk_sel)
 118:	e5d00005 	ldrb	r0, [r0, #5]
		t = AR8327_PAD_MAC_GMII_EN;
 11c:	e3530000 	cmp	r3, #0
		if (cfg->txclk_sel)
 120:	e2600000 	rsb	r0, r0, #0
		t = AR8327_PAD_MAC_GMII_EN;
 124:	13a03050 	movne	r3, #80	@ 0x50
 128:	03a03040 	moveq	r3, #64	@ 0x40
		if (cfg->txclk_sel)
 12c:	e2000020 	and	r0, r0, #32
 130:	e1800003 	orr	r0, r0, r3
 134:	eaffffc5 	b	50 <ar8327_get_pad_cfg+0x50>
		t |= cfg->txclk_delay_sel << AR8327_PAD_RGMII_TXCLK_DELAY_SEL_S;
 138:	e590e00c 	ldr	lr, [r0, #12]
		t |= cfg->rxclk_delay_sel << AR8327_PAD_RGMII_RXCLK_DELAY_SEL_S;
 13c:	e5903010 	ldr	r3, [r0, #16]
		if (cfg->sgmii_delay_en)
 140:	e5d0c009 	ldrb	ip, [r0, #9]
		if (cfg->rxclk_delay_en)
 144:	e5d01008 	ldrb	r1, [r0, #8]
		if (cfg->txclk_delay_en)
 148:	e5d02007 	ldrb	r2, [r0, #7]
		t |= cfg->rxclk_delay_sel << AR8327_PAD_RGMII_RXCLK_DELAY_SEL_S;
 14c:	e1a03a03 	lsl	r3, r3, #20
		if (cfg->sgmii_delay_en)
 150:	e26c0000 	rsb	r0, ip, #0
 154:	e1833b0e 	orr	r3, r3, lr, lsl #22
 158:	e2000702 	and	r0, r0, #524288	@ 0x80000
		if (cfg->rxclk_delay_en)
 15c:	e2611000 	rsb	r1, r1, #0
		if (cfg->sgmii_delay_en)
 160:	e1833000 	orr	r3, r3, r0
		if (cfg->txclk_delay_en)
 164:	e2622000 	rsb	r2, r2, #0
		if (cfg->rxclk_delay_en)
 168:	e2011401 	and	r1, r1, #16777216	@ 0x1000000
		if (cfg->txclk_delay_en)
 16c:	e2022402 	and	r2, r2, #33554432	@ 0x2000000
		if (cfg->sgmii_delay_en)
 170:	e1833001 	orr	r3, r3, r1
 174:	e1833002 	orr	r3, r3, r2
 178:	e3830080 	orr	r0, r3, #128	@ 0x80
 17c:	eaffffb3 	b	50 <ar8327_get_pad_cfg+0x50>
		return 0;
 180:	e3a00000 	mov	r0, #0
 184:	eaffffb1 	b	50 <ar8327_get_pad_cfg+0x50>

Disassembly of section .text.ar8327_cleanup:

00000000 <ar8327_cleanup>:
	return 0;
}

static void
ar8327_cleanup(struct ar8xxx_priv *priv)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	ar8327_leds_cleanup(priv);
}
   c:	e24bd00c 	sub	sp, fp, #12
  10:	e89da800 	ldm	sp, {fp, sp, pc}

Disassembly of section .text.ar8327_sw_get_ports:

00000000 <ar8327_sw_get_ports>:
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
}

static int
ar8327_sw_get_ports(struct switch_dev *dev, struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	u8 ports = priv->vlan_table[val->port_vlan];
   c:	e5912004 	ldr	r2, [r1, #4]
	int i;

	val->len = 0;
  10:	e3a03000 	mov	r3, #0
	u8 ports = priv->vlan_table[val->port_vlan];
  14:	e303cbb6 	movw	ip, #15286	@ 0x3bb6
  18:	e0802002 	add	r2, r0, r2
  1c:	e7d2400c 	ldrb	r4, [r2, ip]
	val->len = 0;
  20:	e5813008 	str	r3, [r1, #8]
	for (i = 0; i < dev->ports; i++) {
  24:	e590e024 	ldr	lr, [r0, #36]	@ 0x24
  28:	e15e0003 	cmp	lr, r3
  2c:	0a000021 	beq	b8 <ar8327_sw_get_ports+0xb8>
  30:	e280cc4b 	add	ip, r0, #19200	@ 0x4b00
		if (!(ports & (1 << i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if ((priv->vlan_tagged & (1 << i)) || (priv->pvid[i] != val->port_vlan))
  34:	e2805901 	add	r5, r0, #16384	@ 0x4000
  38:	e28cc0b8 	add	ip, ip, #184	@ 0xb8
			p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
  3c:	e3a06004 	mov	r6, #4
  40:	ea000005 	b	5c <ar8327_sw_get_ports+0x5c>
  44:	e5876004 	str	r6, [r7, #4]
	for (i = 0; i < dev->ports; i++) {
  48:	e590e024 	ldr	lr, [r0, #36]	@ 0x24
  4c:	e2833001 	add	r3, r3, #1
  50:	e28cc002 	add	ip, ip, #2
  54:	e15e0003 	cmp	lr, r3
  58:	9a000016 	bls	b8 <ar8327_sw_get_ports+0xb8>
		if (!(ports & (1 << i)))
  5c:	e1a02354 	asr	r2, r4, r3
  60:	e3120001 	tst	r2, #1
  64:	0afffff8 	beq	4c <ar8327_sw_get_ports+0x4c>
		p = &val->value.ports[val->len++];
  68:	e5912008 	ldr	r2, [r1, #8]
  6c:	e591e00c 	ldr	lr, [r1, #12]
  70:	e2828001 	add	r8, r2, #1
  74:	e08e7182 	add	r7, lr, r2, lsl #3
  78:	e5818008 	str	r8, [r1, #8]
		p->id = i;
  7c:	e78e3182 	str	r3, [lr, r2, lsl #3]
		if ((priv->vlan_tagged & (1 << i)) || (priv->pvid[i] != val->port_vlan))
  80:	e5d52bb6 	ldrb	r2, [r5, #2998]	@ 0xbb6
  84:	e1a02352 	asr	r2, r2, r3
  88:	e2122001 	ands	r2, r2, #1
  8c:	1affffec 	bne	44 <ar8327_sw_get_ports+0x44>
  90:	e591e004 	ldr	lr, [r1, #4]
  94:	e1dc80b0 	ldrh	r8, [ip]
  98:	e158000e 	cmp	r8, lr
  9c:	1affffe8 	bne	44 <ar8327_sw_get_ports+0x44>
		else
			p->flags = 0;
  a0:	e5872004 	str	r2, [r7, #4]
	for (i = 0; i < dev->ports; i++) {
  a4:	e590e024 	ldr	lr, [r0, #36]	@ 0x24
  a8:	e2833001 	add	r3, r3, #1
  ac:	e28cc002 	add	ip, ip, #2
  b0:	e15e0003 	cmp	lr, r3
  b4:	8affffe8 	bhi	5c <ar8327_sw_get_ports+0x5c>
	}
	return 0;
}
  b8:	e3a00000 	mov	r0, #0
  bc:	e24bd020 	sub	sp, fp, #32
  c0:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_ports:

00000000 <ar8327_sw_set_ports>:

static int
ar8327_sw_set_ports(struct switch_dev *dev, struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd00c 	sub	sp, sp, #12
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	u8 *vt = &priv->vlan_table[val->port_vlan];
  10:	e591c004 	ldr	ip, [r1, #4]
	int i;

	*vt = 0;
  14:	e3a02000 	mov	r2, #0
  18:	e3033bb6 	movw	r3, #15286	@ 0x3bb6
  1c:	e080c00c 	add	ip, r0, ip
  20:	e7cc2003 	strb	r2, [ip, r3]
	for (i = 0; i < val->len; i++) {
  24:	e591e008 	ldr	lr, [r1, #8]
  28:	e15e0002 	cmp	lr, r2
  2c:	0a00002a 	beq	dc <ar8327_sw_set_ports+0xdc>
		} else {
			priv->vlan_tagged &= ~(1 << p->id);
			priv->pvid[p->id] = val->port_vlan;
		}

		*vt |= 1 << p->id;
  30:	e08c3003 	add	r3, ip, r3
				priv->vlan_tagged |= (1 << p->id);
  34:	e3a06001 	mov	r6, #1
	for (i = 0; i < val->len; i++) {
  38:	e1a0c002 	mov	ip, r2
			priv->vlan_tagged &= ~(1 << p->id);
  3c:	e280a901 	add	sl, r0, #16384	@ 0x4000
			priv->pvid[p->id] = val->port_vlan;
  40:	e30295d8 	movw	r9, #9688	@ 0x25d8
		*vt |= 1 << p->id;
  44:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
  48:	ea00000f 	b	8c <ar8327_sw_set_ports+0x8c>
			if (val->port_vlan == priv->pvid[p->id]) {
  4c:	e5917004 	ldr	r7, [r1, #4]
  50:	e1d440b8 	ldrh	r4, [r4, #8]
  54:	e1570004 	cmp	r7, r4
  58:	1a000003 	bne	6c <ar8327_sw_set_ports+0x6c>
				priv->vlan_tagged |= (1 << p->id);
  5c:	e5da3bb6 	ldrb	r3, [sl, #2998]	@ 0xbb6
  60:	e1833008 	orr	r3, r3, r8
  64:	e5ca3bb6 	strb	r3, [sl, #2998]	@ 0xbb6
		*vt |= 1 << p->id;
  68:	e79e3005 	ldr	r3, [lr, r5]
	for (i = 0; i < val->len; i++) {
  6c:	e28cc001 	add	ip, ip, #1
		*vt |= 1 << p->id;
  70:	e1823316 	orr	r3, r2, r6, lsl r3
  74:	e6ef2073 	uxtb	r2, r3
  78:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
  7c:	e5c32000 	strb	r2, [r3]
	for (i = 0; i < val->len; i++) {
  80:	e5913008 	ldr	r3, [r1, #8]
  84:	e153000c 	cmp	r3, ip
  88:	9a000013 	bls	dc <ar8327_sw_set_ports+0xdc>
		struct switch_port *p = &val->value.ports[i];
  8c:	e591e00c 	ldr	lr, [r1, #12]
  90:	e1a0518c 	lsl	r5, ip, #3
  94:	e08e4005 	add	r4, lr, r5
			if (val->port_vlan == priv->pvid[p->id]) {
  98:	e79e318c 	ldr	r3, [lr, ip, lsl #3]
		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
  9c:	e5947004 	ldr	r7, [r4, #4]
			if (val->port_vlan == priv->pvid[p->id]) {
  a0:	e0834009 	add	r4, r3, r9
				priv->vlan_tagged |= (1 << p->id);
  a4:	e1a08316 	lsl	r8, r6, r3
		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
  a8:	e3170004 	tst	r7, #4
			if (val->port_vlan == priv->pvid[p->id]) {
  ac:	e0804084 	add	r4, r0, r4, lsl #1
		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
  b0:	1affffe5 	bne	4c <ar8327_sw_set_ports+0x4c>
			priv->vlan_tagged &= ~(1 << p->id);
  b4:	e5da3bb6 	ldrb	r3, [sl, #2998]	@ 0xbb6
  b8:	e1c33008 	bic	r3, r3, r8
  bc:	e5ca3bb6 	strb	r3, [sl, #2998]	@ 0xbb6
			priv->pvid[p->id] = val->port_vlan;
  c0:	e79e318c 	ldr	r3, [lr, ip, lsl #3]
  c4:	e5914004 	ldr	r4, [r1, #4]
  c8:	e0833009 	add	r3, r3, r9
  cc:	e0803083 	add	r3, r0, r3, lsl #1
  d0:	e1c340b8 	strh	r4, [r3, #8]
		*vt |= 1 << p->id;
  d4:	e79e318c 	ldr	r3, [lr, ip, lsl #3]
  d8:	eaffffe3 	b	6c <ar8327_sw_set_ports+0x6c>
	}
	return 0;
}
  dc:	e3a00000 	mov	r0, #0
  e0:	e24bd028 	sub	sp, fp, #40	@ 0x28
  e4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

Disassembly of section .text.ar8327_sw_get_leds:

00000000 <ar8327_sw_get_leds>:
}

int
ar8327_sw_get_leds(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e1a03000 	mov	r3, r0
	if (!priv->ledstate || priv->ledstate == 1)
		val->value.i = 1;
	else
		val->value.i = 0;
	return 0;
}
  10:	e3a00000 	mov	r0, #0
	if (!priv->ledstate || priv->ledstate == 1)
  14:	e2833a01 	add	r3, r3, #4096	@ 0x1000
  18:	e5933ba0 	ldr	r3, [r3, #2976]	@ 0xba0
  1c:	e3530001 	cmp	r3, #1
  20:	83a03000 	movhi	r3, #0
  24:	93a03001 	movls	r3, #1
  28:	e582300c 	str	r3, [r2, #12]
}
  2c:	e24bd00c 	sub	sp, fp, #12
  30:	e89da800 	ldm	sp, {fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_eee:

00000000 <ar8327_sw_set_eee>:

static int
ar8327_sw_set_eee(struct switch_dev *dev,
		  const struct switch_attr *attr,
		  struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	struct ar8327_data *data = priv->chip_data;
	int port = val->port_vlan;
   c:	e5923004 	ldr	r3, [r2, #4]
	int phy;

	if (port >= dev->ports)
  10:	e590c024 	ldr	ip, [r0, #36]	@ 0x24
	struct ar8327_data *data = priv->chip_data;
  14:	e5901258 	ldr	r1, [r0, #600]	@ 0x258
	if (port >= dev->ports)
  18:	e153000c 	cmp	r3, ip
  1c:	2a00000b 	bcs	50 <ar8327_sw_set_eee+0x50>
		return -EINVAL;
	if (port == 0 || port == 6)
  20:	e3530006 	cmp	r3, #6
  24:	13530000 	cmpne	r3, #0
  28:	03a00001 	moveq	r0, #1
  2c:	13a00000 	movne	r0, #0
  30:	0a000008 	beq	58 <ar8327_sw_set_eee+0x58>
		return -EOPNOTSUPP;

	phy = port - 1;

	data->eee[phy] = !!(val->value.i);
  34:	e592200c 	ldr	r2, [r2, #12]
  38:	e0811003 	add	r1, r1, r3
  3c:	e2523000 	subs	r3, r2, #0
  40:	13a03001 	movne	r3, #1
  44:	e5c1300f 	strb	r3, [r1, #15]

	return 0;
}
  48:	e24bd00c 	sub	sp, fp, #12
  4c:	e89da800 	ldm	sp, {fp, sp, pc}
		return -EINVAL;
  50:	e3e00015 	mvn	r0, #21
  54:	eafffffb 	b	48 <ar8327_sw_set_eee+0x48>
		return -EOPNOTSUPP;
  58:	e3e0005e 	mvn	r0, #94	@ 0x5e
  5c:	eafffff9 	b	48 <ar8327_sw_set_eee+0x48>

Disassembly of section .text.ar8327_sw_get_eee:

00000000 <ar8327_sw_get_eee>:

static int
ar8327_sw_get_eee(struct switch_dev *dev,
		  const struct switch_attr *attr,
		  struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	const struct ar8327_data *data = priv->chip_data;
	int port = val->port_vlan;
   c:	e5923004 	ldr	r3, [r2, #4]
	int phy;

	if (port >= dev->ports)
  10:	e590c024 	ldr	ip, [r0, #36]	@ 0x24
	const struct ar8327_data *data = priv->chip_data;
  14:	e5901258 	ldr	r1, [r0, #600]	@ 0x258
	if (port >= dev->ports)
  18:	e153000c 	cmp	r3, ip
  1c:	2a000009 	bcs	48 <ar8327_sw_get_eee+0x48>
		return -EINVAL;
	if (port == 0 || port == 6)
  20:	e3530006 	cmp	r3, #6
  24:	13530000 	cmpne	r3, #0
  28:	03a00001 	moveq	r0, #1
  2c:	13a00000 	movne	r0, #0
  30:	0a000006 	beq	50 <ar8327_sw_get_eee+0x50>
		return -EOPNOTSUPP;

	phy = port - 1;

	val->value.i = data->eee[phy];
  34:	e0811003 	add	r1, r1, r3
  38:	e5d1300f 	ldrb	r3, [r1, #15]
  3c:	e582300c 	str	r3, [r2, #12]

	return 0;
}
  40:	e24bd00c 	sub	sp, fp, #12
  44:	e89da800 	ldm	sp, {fp, sp, pc}
		return -EINVAL;
  48:	e3e00015 	mvn	r0, #21
  4c:	eafffffb 	b	40 <ar8327_sw_get_eee+0x40>
		return -EOPNOTSUPP;
  50:	e3e0005e 	mvn	r0, #94	@ 0x5e
  54:	eafffff9 	b	40 <ar8327_sw_get_eee+0x40>

Disassembly of section .text.ar8327_sw_set_port_vlan_prio:

00000000 <ar8327_sw_set_port_vlan_prio>:
}

static int
ar8327_sw_set_port_vlan_prio(struct switch_dev *dev, const struct switch_attr *attr,
			     struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int port = val->port_vlan;
   c:	e5923004 	ldr	r3, [r2, #4]

	if (port >= dev->ports)
  10:	e5901024 	ldr	r1, [r0, #36]	@ 0x24
  14:	e1530001 	cmp	r3, r1
  18:	2a00000d 	bcs	54 <ar8327_sw_set_port_vlan_prio+0x54>
		return -EINVAL;
	if (port == 0 || port == 6)
  1c:	e3530006 	cmp	r3, #6
  20:	13530000 	cmpne	r3, #0
  24:	03a01001 	moveq	r1, #1
  28:	13a01000 	movne	r1, #0
  2c:	0a00000a 	beq	5c <ar8327_sw_set_port_vlan_prio+0x5c>
		return -EOPNOTSUPP;
	if (val->value.i < 0 || val->value.i > 7)
  30:	e592200c 	ldr	r2, [r2, #12]
  34:	e3520007 	cmp	r2, #7
  38:	8a000005 	bhi	54 <ar8327_sw_set_port_vlan_prio+0x54>
		return -EINVAL;

	priv->port_vlan_prio[port] = val->value.i;
  3c:	e0803003 	add	r3, r0, r3
  40:	e304cbd8 	movw	ip, #19416	@ 0x4bd8

	return 0;
  44:	e1a00001 	mov	r0, r1
	priv->port_vlan_prio[port] = val->value.i;
  48:	e7c3200c 	strb	r2, [r3, ip]
}
  4c:	e24bd00c 	sub	sp, fp, #12
  50:	e89da800 	ldm	sp, {fp, sp, pc}
		return -EINVAL;
  54:	e3e00015 	mvn	r0, #21
  58:	eafffffb 	b	4c <ar8327_sw_set_port_vlan_prio+0x4c>
		return -EOPNOTSUPP;
  5c:	e3e0005e 	mvn	r0, #94	@ 0x5e
  60:	eafffff9 	b	4c <ar8327_sw_set_port_vlan_prio+0x4c>

Disassembly of section .text.ar8327_sw_get_port_vlan_prio:

00000000 <ar8327_sw_get_port_vlan_prio>:

static int
ar8327_sw_get_port_vlan_prio(struct switch_dev *dev, const struct switch_attr *attr,
                  struct switch_val *val)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int port = val->port_vlan;

	val->value.i = priv->port_vlan_prio[port];
   c:	e592c004 	ldr	ip, [r2, #4]
{
  10:	e1a03000 	mov	r3, r0
	val->value.i = priv->port_vlan_prio[port];
  14:	e3041bd8 	movw	r1, #19416	@ 0x4bd8

	return 0;
}
  18:	e3a00000 	mov	r0, #0
	val->value.i = priv->port_vlan_prio[port];
  1c:	e083300c 	add	r3, r3, ip
  20:	e7d33001 	ldrb	r3, [r3, r1]
  24:	e582300c 	str	r3, [r2, #12]
}
  28:	e24bd00c 	sub	sp, fp, #12
  2c:	e89da800 	ldm	sp, {fp, sp, pc}

Disassembly of section .text.ar8327_read_port_status:

00000000 <ar8327_read_port_status>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	t = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
   c:	e281101f 	add	r1, r1, #31
  10:	e1a01101 	lsl	r1, r1, #2
  14:	ebfffffe 	bl	0 <ar8xxx_read>
	if (t & AR8216_PORT_STATUS_LINK_UP &&
  18:	e2003c03 	and	r3, r0, #768	@ 0x300
  1c:	e3530c03 	cmp	r3, #768	@ 0x300
  20:	1a000004 	bne	38 <ar8327_read_port_status+0x38>
		if (t & AR8327_PORT_STATUS_TXFLOW_AUTO)
  24:	e3100b01 	tst	r0, #1024	@ 0x400
		t &= ~(AR8216_PORT_STATUS_TXFLOW | AR8216_PORT_STATUS_RXFLOW);
  28:	e3c00030 	bic	r0, r0, #48	@ 0x30
			t |= AR8216_PORT_STATUS_TXFLOW;
  2c:	13800010 	orrne	r0, r0, #16
		if (t & AR8327_PORT_STATUS_RXFLOW_AUTO)
  30:	e3100b02 	tst	r0, #2048	@ 0x800
			t |= AR8216_PORT_STATUS_RXFLOW;
  34:	13800020 	orrne	r0, r0, #32
}
  38:	e24bd00c 	sub	sp, fp, #12
  3c:	e89da800 	ldm	sp, {fp, sp, pc}

Disassembly of section .text.ar8327_sw_get_disable:

00000000 <ar8327_sw_get_disable>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	int port = val->port_vlan;
   c:	e5921004 	ldr	r1, [r2, #4]
{
  10:	e1a04002 	mov	r4, r2
	if (port >= dev->ports)
  14:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
  18:	e1510003 	cmp	r1, r3
  1c:	2a000011 	bcs	68 <ar8327_sw_get_disable+0x68>
	if (port == 0 || port == 6)
  20:	e3510006 	cmp	r1, #6
  24:	13510000 	cmpne	r1, #0
  28:	03a05001 	moveq	r5, #1
  2c:	13a05000 	movne	r5, #0
  30:	0a00000a 	beq	60 <ar8327_sw_get_disable+0x60>
	t = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  34:	e281101f 	add	r1, r1, #31
  38:	e1a01101 	lsl	r1, r1, #2
  3c:	ebfffffe 	bl	0 <ar8xxx_read>
	if (!(t & AR8216_PORT_STATUS_LINK_AUTO) && !(t & (AR8216_PORT_SPEED_10M << AR8216_PORT_STATUS_SPEED_S)) && !(t & (AR8216_PORT_SPEED_100M << AR8216_PORT_STATUS_SPEED_S)) && !(t & (AR8216_PORT_SPEED_1000M << AR8216_PORT_STATUS_SPEED_S)))
  40:	e3003203 	movw	r3, #515	@ 0x203
  44:	e1100003 	tst	r0, r3
	return 0;
  48:	e3a00000 	mov	r0, #0
		val->value.i = 1;
  4c:	03a03001 	moveq	r3, #1
		val->value.i = 0;
  50:	1584500c 	strne	r5, [r4, #12]
		val->value.i = 1;
  54:	0584300c 	streq	r3, [r4, #12]
}
  58:	e24bd014 	sub	sp, fp, #20
  5c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
		return -EOPNOTSUPP;
  60:	e3e0005e 	mvn	r0, #94	@ 0x5e
  64:	eafffffb 	b	58 <ar8327_sw_get_disable+0x58>
		return -EINVAL;
  68:	e3e00015 	mvn	r0, #21
  6c:	eafffff9 	b	58 <ar8327_sw_get_disable+0x58>

Disassembly of section .text.ar8327_sw_set_leds:

00000000 <ar8327_sw_set_leds>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (priv->ledstate == 0) {
   c:	e2805a01 	add	r5, r0, #4096	@ 0x1000
{
  10:	e1a04000 	mov	r4, r0
	if (priv->ledstate == 0) {
  14:	e5953ba0 	ldr	r3, [r5, #2976]	@ 0xba0
{
  18:	e1a06002 	mov	r6, r2
	if (priv->ledstate == 0) {
  1c:	e3530000 	cmp	r3, #0
  20:	0a00002c 	beq	d8 <ar8327_sw_set_leds+0xd8>
	if (!!val->value.i) {
  24:	e596600c 	ldr	r6, [r6, #12]
  28:	e3560000 	cmp	r6, #0
  2c:	0a000014 	beq	84 <ar8327_sw_set_leds+0x84>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL0, priv->ledregs[0]);
  30:	e5952ba4 	ldr	r2, [r5, #2980]	@ 0xba4
  34:	e1a00004 	mov	r0, r4
  38:	e3a01050 	mov	r1, #80	@ 0x50
  3c:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL1, priv->ledregs[1]);
  40:	e5952ba8 	ldr	r2, [r5, #2984]	@ 0xba8
  44:	e1a00004 	mov	r0, r4
  48:	e3a01054 	mov	r1, #84	@ 0x54
  4c:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL2, priv->ledregs[2]);
  50:	e5952bac 	ldr	r2, [r5, #2988]	@ 0xbac
  54:	e1a00004 	mov	r0, r4
  58:	e3a01058 	mov	r1, #88	@ 0x58
  5c:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL3, priv->ledregs[3]);
  60:	e5952bb0 	ldr	r2, [r5, #2992]	@ 0xbb0
  64:	e1a00004 	mov	r0, r4
  68:	e3a0105c 	mov	r1, #92	@ 0x5c
  6c:	ebfffffe 	bl	0 <ar8xxx_write>
		priv->ledstate = 1;
  70:	e3a03001 	mov	r3, #1
}
  74:	e3a00000 	mov	r0, #0
		priv->ledstate = 1;
  78:	e5853ba0 	str	r3, [r5, #2976]	@ 0xba0
}
  7c:	e24bd01c 	sub	sp, fp, #28
  80:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
		ar8xxx_write(priv, AR8327_REG_LED_CTRL0, 0);
  84:	e1a02006 	mov	r2, r6
  88:	e1a00004 	mov	r0, r4
  8c:	e3a01050 	mov	r1, #80	@ 0x50
  90:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL1, 0);
  94:	e1a02006 	mov	r2, r6
  98:	e1a00004 	mov	r0, r4
  9c:	e3a01054 	mov	r1, #84	@ 0x54
  a0:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL2, 0);
  a4:	e1a02006 	mov	r2, r6
  a8:	e1a00004 	mov	r0, r4
  ac:	e3a01058 	mov	r1, #88	@ 0x58
  b0:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL3, 0);
  b4:	e1a00004 	mov	r0, r4
  b8:	e1a02006 	mov	r2, r6
  bc:	e3a0105c 	mov	r1, #92	@ 0x5c
  c0:	ebfffffe 	bl	0 <ar8xxx_write>
  c4:	e3a03002 	mov	r3, #2
}
  c8:	e3a00000 	mov	r0, #0
		priv->ledstate = 1;
  cc:	e5853ba0 	str	r3, [r5, #2976]	@ 0xba0
}
  d0:	e24bd01c 	sub	sp, fp, #28
  d4:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
	    priv->ledregs[0] = ar8xxx_read(priv, AR8327_REG_LED_CTRL0);
  d8:	e3a01050 	mov	r1, #80	@ 0x50
  dc:	ebfffffe 	bl	0 <ar8xxx_read>
  e0:	e1a03000 	mov	r3, r0
	    priv->ledregs[1] = ar8xxx_read(priv, AR8327_REG_LED_CTRL1);
  e4:	e3a01054 	mov	r1, #84	@ 0x54
  e8:	e1a00004 	mov	r0, r4
	    priv->ledregs[0] = ar8xxx_read(priv, AR8327_REG_LED_CTRL0);
  ec:	e5853ba4 	str	r3, [r5, #2980]	@ 0xba4
	    priv->ledregs[1] = ar8xxx_read(priv, AR8327_REG_LED_CTRL1);
  f0:	ebfffffe 	bl	0 <ar8xxx_read>
  f4:	e1a03000 	mov	r3, r0
	    priv->ledregs[2] = ar8xxx_read(priv, AR8327_REG_LED_CTRL2);
  f8:	e3a01058 	mov	r1, #88	@ 0x58
  fc:	e1a00004 	mov	r0, r4
	    priv->ledregs[1] = ar8xxx_read(priv, AR8327_REG_LED_CTRL1);
 100:	e5853ba8 	str	r3, [r5, #2984]	@ 0xba8
	    priv->ledregs[2] = ar8xxx_read(priv, AR8327_REG_LED_CTRL2);
 104:	ebfffffe 	bl	0 <ar8xxx_read>
 108:	e1a03000 	mov	r3, r0
	    priv->ledregs[3] = ar8xxx_read(priv, AR8327_REG_LED_CTRL3);
 10c:	e3a0105c 	mov	r1, #92	@ 0x5c
 110:	e1a00004 	mov	r0, r4
	    priv->ledregs[2] = ar8xxx_read(priv, AR8327_REG_LED_CTRL2);
 114:	e5853bac 	str	r3, [r5, #2988]	@ 0xbac
	    priv->ledregs[3] = ar8xxx_read(priv, AR8327_REG_LED_CTRL3);
 118:	ebfffffe 	bl	0 <ar8xxx_read>
	    priv->ledstate = 1;
 11c:	e3a03001 	mov	r3, #1
 120:	e5853ba0 	str	r3, [r5, #2976]	@ 0xba0
	    priv->ledregs[3] = ar8xxx_read(priv, AR8327_REG_LED_CTRL3);
 124:	e5850bb0 	str	r0, [r5, #2992]	@ 0xbb0
	    priv->ledstate = 1;
 128:	eaffffbd 	b	24 <ar8327_sw_set_leds+0x24>

Disassembly of section .text.ar8327_setup_port:

00000000 <ar8327_setup_port>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd004 	sub	sp, sp, #4
	u32 pvid = priv->vlan_id[priv->pvid[port]];
  10:	e2813d97 	add	r3, r1, #9664	@ 0x25c0
	if (priv->vlan) {
  14:	e2807a01 	add	r7, r0, #4096	@ 0x1000
	u32 pvid = priv->vlan_id[priv->pvid[port]];
  18:	e2833018 	add	r3, r3, #24
{
  1c:	e1a06002 	mov	r6, r2
	if (priv->vlan) {
  20:	e5d72bb4 	ldrb	r2, [r7, #2996]	@ 0xbb4
	u32 pvid = priv->vlan_id[priv->pvid[port]];
  24:	e0803083 	add	r3, r0, r3, lsl #1
{
  28:	e1a04001 	mov	r4, r1
  2c:	e1a05000 	mov	r5, r0
	u32 pvid = priv->vlan_id[priv->pvid[port]];
  30:	e1d330b8 	ldrh	r3, [r3, #8]
	if (priv->vlan) {
  34:	e3520000 	cmp	r2, #0
  38:	03039050 	movweq	r9, #12368	@ 0x3050
  3c:	03a0adc1 	moveq	sl, #12352	@ 0x3040
	u32 pvid = priv->vlan_id[priv->pvid[port]];
  40:	e2833edd 	add	r3, r3, #3536	@ 0xdd0
  44:	e2833008 	add	r3, r3, #8
  48:	e0803083 	add	r3, r0, r3, lsl #1
  4c:	e1d320b6 	ldrh	r2, [r3, #6]
  50:	e0822802 	add	r2, r2, r2, lsl #16
	if (priv->vlan) {
  54:	0a000008 	beq	7c <ar8327_setup_port+0x7c>
	if (priv->vlan && priv->port_vlan_prio[port]) {
  58:	e0801001 	add	r1, r0, r1
  5c:	e3043bd8 	movw	r3, #19416	@ 0x4bd8
  60:	e3866c03 	orr	r6, r6, #768	@ 0x300
  64:	e7d13003 	ldrb	r3, [r1, r3]
		t |= prio << AR8327_PORT_VLAN0_DEF_CPRI_S;
  68:	e3a09050 	mov	r9, #80	@ 0x50
  6c:	e3a0a040 	mov	sl, #64	@ 0x40
	if (priv->vlan && priv->port_vlan_prio[port]) {
  70:	e3530000 	cmp	r3, #0
		t |= prio << AR8327_PORT_VLAN0_DEF_CPRI_S;
  74:	10833803 	addne	r3, r3, r3, lsl #16
  78:	11822683 	orrne	r2, r2, r3, lsl #13
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN0(port), t);
  7c:	e2848084 	add	r8, r4, #132	@ 0x84
  80:	e1a08188 	lsl	r8, r8, #3
  84:	e1a00005 	mov	r0, r5
  88:	e1a01008 	mov	r1, r8
  8c:	ebfffffe 	bl	0 <ar8xxx_write>
	if (priv->vlan && priv->port_vlan_prio[port])
  90:	e5d73bb4 	ldrb	r3, [r7, #2996]	@ 0xbb4
  94:	e3530000 	cmp	r3, #0
  98:	0a000004 	beq	b0 <ar8327_setup_port+0xb0>
  9c:	e0852004 	add	r2, r5, r4
  a0:	e3043bd8 	movw	r3, #19416	@ 0x4bd8
  a4:	e7d23003 	ldrb	r3, [r2, r3]
		t |= AR8327_PORT_VLAN1_VLAN_PRI_PROP;
  a8:	e3530000 	cmp	r3, #0
  ac:	11a0a009 	movne	sl, r9
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
  b0:	e0844084 	add	r4, r4, r4, lsl #1
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN1(port), t);
  b4:	e1a0200a 	mov	r2, sl
  b8:	e2881004 	add	r1, r8, #4
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
  bc:	e1a04104 	lsl	r4, r4, #2
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN1(port), t);
  c0:	e1a00005 	mov	r0, r5
  c4:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
  c8:	e3862705 	orr	r2, r6, #1310720	@ 0x140000
  cc:	e1a00005 	mov	r0, r5
  d0:	e2841e66 	add	r1, r4, #1632	@ 0x660
  d4:	ebfffffe 	bl	0 <ar8xxx_write>
}
  d8:	e24bd028 	sub	sp, fp, #40	@ 0x28
  dc:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_disable:

00000000 <ar8327_sw_set_disable>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	int port = val->port_vlan;
   c:	e5921004 	ldr	r1, [r2, #4]
{
  10:	e1a03002 	mov	r3, r2
  14:	e1a05000 	mov	r5, r0
	if (port >= dev->ports)
  18:	e5902024 	ldr	r2, [r0, #36]	@ 0x24
  1c:	e1510002 	cmp	r1, r2
  20:	2a000023 	bcs	b4 <ar8327_sw_set_disable+0xb4>
	if (port == 0 || port == 6)
  24:	e3510006 	cmp	r1, #6
  28:	13510000 	cmpne	r1, #0
  2c:	03a07001 	moveq	r7, #1
  30:	13a07000 	movne	r7, #0
  34:	0a000020 	beq	bc <ar8327_sw_set_disable+0xbc>
	if (!!(val->value.i))  {
  38:	e593300c 	ldr	r3, [r3, #12]
  3c:	e3530000 	cmp	r3, #0
  40:	1a00000c 	bne	78 <ar8327_sw_set_disable+0x78>
		priv->disabled[port] = 0;
  44:	e0805101 	add	r5, r0, r1, lsl #2
  48:	e2855a01 	add	r5, r5, #4096	@ 0x1000
		if (priv->state[port])
  4c:	e5952b60 	ldr	r2, [r5, #2912]	@ 0xb60
		priv->disabled[port] = 0;
  50:	e5853b80 	str	r3, [r5, #2944]	@ 0xb80
		if (priv->state[port])
  54:	e3520000 	cmp	r2, #0
  58:	1a000002 	bne	68 <ar8327_sw_set_disable+0x68>
	return 0;
  5c:	e3a00000 	mov	r0, #0
}
  60:	e24bd01c 	sub	sp, fp, #28
  64:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
			ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), priv->state[port]);
  68:	e281101f 	add	r1, r1, #31
  6c:	e1a01101 	lsl	r1, r1, #2
  70:	ebfffffe 	bl	0 <ar8xxx_write>
  74:	eafffff8 	b	5c <ar8327_sw_set_disable+0x5c>
		priv->disabled[port] = 1;
  78:	e0806101 	add	r6, r0, r1, lsl #2
		priv->state[port] = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  7c:	e281101f 	add	r1, r1, #31
  80:	e1a04101 	lsl	r4, r1, #2
		priv->disabled[port] = 1;
  84:	e3a03001 	mov	r3, #1
  88:	e2866a01 	add	r6, r6, #4096	@ 0x1000
		priv->state[port] = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  8c:	e1a01004 	mov	r1, r4
		priv->disabled[port] = 1;
  90:	e5863b80 	str	r3, [r6, #2944]	@ 0xb80
		priv->state[port] = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  94:	ebfffffe 	bl	0 <ar8xxx_read>
  98:	e1a03000 	mov	r3, r0
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  9c:	e1a02007 	mov	r2, r7
  a0:	e1a01004 	mov	r1, r4
  a4:	e1a00005 	mov	r0, r5
		priv->state[port] = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  a8:	e5863b60 	str	r3, [r6, #2912]	@ 0xb60
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  ac:	ebfffffe 	bl	0 <ar8xxx_write>
  b0:	eaffffe9 	b	5c <ar8327_sw_set_disable+0x5c>
		return -EINVAL;
  b4:	e3e00015 	mvn	r0, #21
  b8:	eaffffe8 	b	60 <ar8327_sw_set_disable+0x60>
		return -EOPNOTSUPP;
  bc:	e3e0005e 	mvn	r0, #94	@ 0x5e
  c0:	eaffffe6 	b	60 <ar8327_sw_set_disable+0x60>

Disassembly of section .text.ar8327_sw_get_port_igmp_snooping:

00000000 <ar8327_sw_get_port_igmp_snooping>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	int port = val->port_vlan;
   c:	e5927004 	ldr	r7, [r2, #4]
{
  10:	e1a04000 	mov	r4, r0
  14:	e1a05002 	mov	r5, r2
	if (port >= dev->ports)
  18:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
  1c:	e1570003 	cmp	r7, r3
  20:	2a000022 	bcs	b0 <ar8327_sw_get_port_igmp_snooping+0xb0>
	mutex_lock(&priv->reg_mutex);
  24:	e2808f8f 	add	r8, r0, #572	@ 0x23c
	fwd_ctrl = (BIT(port) << AR8327_FWD_CTRL1_IGMP_S);
  28:	e3a06401 	mov	r6, #16777216	@ 0x1000000
	mutex_lock(&priv->reg_mutex);
  2c:	e1a00008 	mov	r0, r8
	fwd_ctrl = (BIT(port) << AR8327_FWD_CTRL1_IGMP_S);
  30:	e1a06716 	lsl	r6, r6, r7
	mutex_lock(&priv->reg_mutex);
  34:	ebfffffe 	bl	0 <mutex_lock>
	return (ar8xxx_read(priv, AR8327_REG_FWD_CTRL1) &
  38:	e3001624 	movw	r1, #1572	@ 0x624
  3c:	e1a00004 	mov	r0, r4
  40:	ebfffffe 	bl	0 <ar8xxx_read>
			fwd_ctrl) == fwd_ctrl &&
  44:	e1d66000 	bics	r6, r6, r0
  48:	13a03000 	movne	r3, #0
  4c:	0a000005 	beq	68 <ar8327_sw_get_port_igmp_snooping+0x68>
	mutex_unlock(&priv->reg_mutex);
  50:	e1a00008 	mov	r0, r8
	val->value.i = ar8327_get_port_igmp(priv, port);
  54:	e585300c 	str	r3, [r5, #12]
	mutex_unlock(&priv->reg_mutex);
  58:	ebfffffe 	bl	0 <mutex_unlock>
	return 0;
  5c:	e3a00000 	mov	r0, #0
}
  60:	e24bd024 	sub	sp, fp, #36	@ 0x24
  64:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		     AR8327_FRAME_ACK_CTRL_S(port));
  68:	e2772000 	rsbs	r2, r7, #0
  6c:	e2073003 	and	r3, r7, #3
  70:	e2022003 	and	r2, r2, #3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  74:	e2871003 	add	r1, r7, #3
		     AR8327_FRAME_ACK_CTRL_S(port));
  78:	52623000 	rsbpl	r3, r2, #0
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  7c:	e3570000 	cmp	r7, #0
		     AR8327_FRAME_ACK_CTRL_S(port));
  80:	e1a03183 	lsl	r3, r3, #3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  84:	a1a01007 	movge	r1, r7
  88:	e1a00004 	mov	r0, r4
  8c:	e3c11003 	bic	r1, r1, #3
	frame_ack = ((AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  90:	e3a04007 	mov	r4, #7
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  94:	e2811e21 	add	r1, r1, #528	@ 0x210
	frame_ack = ((AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  98:	e1a04314 	lsl	r4, r4, r3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  9c:	ebfffffe 	bl	0 <ar8xxx_read>
			fwd_ctrl) == fwd_ctrl &&
  a0:	e1d44000 	bics	r4, r4, r0
  a4:	03a03001 	moveq	r3, #1
  a8:	13a03000 	movne	r3, #0
  ac:	eaffffe7 	b	50 <ar8327_sw_get_port_igmp_snooping+0x50>
		return -EINVAL;
  b0:	e3e00015 	mvn	r0, #21
  b4:	eaffffe9 	b	60 <ar8327_sw_get_port_igmp_snooping+0x60>

Disassembly of section .text.ar8327_sw_get_igmp_v3:

00000000 <ar8327_sw_get_igmp_v3>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	mutex_lock(&priv->reg_mutex);
   c:	e2806f8f 	add	r6, r0, #572	@ 0x23c
{
  10:	e1a04000 	mov	r4, r0
	mutex_lock(&priv->reg_mutex);
  14:	e1a00006 	mov	r0, r6
{
  18:	e1a05002 	mov	r5, r2
	mutex_lock(&priv->reg_mutex);
  1c:	ebfffffe 	bl	0 <mutex_lock>
	val_reg = ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL1);
  20:	e3a01f85 	mov	r1, #532	@ 0x214
  24:	e1a00004 	mov	r0, r4
  28:	ebfffffe 	bl	0 <ar8xxx_read>
	val->value.i = ((val_reg & AR8327_FRAME_ACK_CTRL_IGMP_V3_EN) != 0);
  2c:	e7e03c50 	ubfx	r3, r0, #24, #1
	mutex_unlock(&priv->reg_mutex);
  30:	e1a00006 	mov	r0, r6
	val->value.i = ((val_reg & AR8327_FRAME_ACK_CTRL_IGMP_V3_EN) != 0);
  34:	e585300c 	str	r3, [r5, #12]
	mutex_unlock(&priv->reg_mutex);
  38:	ebfffffe 	bl	0 <mutex_unlock>
}
  3c:	e3a00000 	mov	r0, #0
  40:	e24bd01c 	sub	sp, fp, #28
  44:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_igmp_v3:

00000000 <ar8327_sw_set_igmp_v3>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	mutex_lock(&priv->reg_mutex);
   c:	e2806f8f 	add	r6, r0, #572	@ 0x23c
{
  10:	e1a05002 	mov	r5, r2
  14:	e1a04000 	mov	r4, r0
	mutex_lock(&priv->reg_mutex);
  18:	e1a00006 	mov	r0, r6
  1c:	ebfffffe 	bl	0 <mutex_lock>
	if (val->value.i)
  20:	e595300c 	ldr	r3, [r5, #12]
}

static inline void
ar8xxx_reg_clear(struct ar8xxx_priv *priv, int reg, u32 val)
{
	ar8xxx_rmw(priv, reg, val, 0);
  24:	e3a01f85 	mov	r1, #532	@ 0x214
  28:	e1a00004 	mov	r0, r4
  2c:	e3530000 	cmp	r3, #0
	ar8xxx_rmw(priv, reg, 0, val);
  30:	13a03401 	movne	r3, #16777216	@ 0x1000000
  34:	13a02000 	movne	r2, #0
	ar8xxx_rmw(priv, reg, val, 0);
  38:	03a02401 	moveq	r2, #16777216	@ 0x1000000
  3c:	ebfffffe 	bl	0 <ar8xxx_rmw>
	mutex_unlock(&priv->reg_mutex);
  40:	e1a00006 	mov	r0, r6
  44:	ebfffffe 	bl	0 <mutex_unlock>
}
  48:	e3a00000 	mov	r0, #0
  4c:	e24bd01c 	sub	sp, fp, #28
  50:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_port_igmp_snooping:

00000000 <ar8327_sw_set_port_igmp_snooping>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	int port = val->port_vlan;
   c:	e5924004 	ldr	r4, [r2, #4]
{
  10:	e1a06000 	mov	r6, r0
  14:	e1a08002 	mov	r8, r2
	if (port >= dev->ports)
  18:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
  1c:	e1540003 	cmp	r4, r3
  20:	2a00002e 	bcs	e0 <ar8327_sw_set_port_igmp_snooping+0xe0>
	mutex_lock(&priv->reg_mutex);
  24:	e2807f8f 	add	r7, r0, #572	@ 0x23c
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  28:	e2845003 	add	r5, r4, #3
	mutex_lock(&priv->reg_mutex);
  2c:	e1a00007 	mov	r0, r7
  30:	ebfffffe 	bl	0 <mutex_lock>
	if (enable) {
  34:	e598800c 	ldr	r8, [r8, #12]
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  38:	e3540000 	cmp	r4, #0
			 AR8327_FRAME_ACK_CTRL_S(port);
  3c:	e2043003 	and	r3, r4, #3
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  40:	a1a05004 	movge	r5, r4
			 AR8327_FRAME_ACK_CTRL_S(port);
  44:	e2742000 	rsbs	r2, r4, #0
  48:	e2022003 	and	r2, r2, #3
			   BIT(port) << AR8327_FWD_CTRL1_MC_FLOOD_S,
  4c:	e3a01001 	mov	r1, #1
			 AR8327_FRAME_ACK_CTRL_S(port);
  50:	52623000 	rsbpl	r3, r2, #0
			   BIT(port) << AR8327_FWD_CTRL1_MC_FLOOD_S,
  54:	e1a01411 	lsl	r1, r1, r4
			 AR8327_FRAME_ACK_CTRL_S(port);
  58:	e1a03183 	lsl	r3, r3, #3
	if (enable) {
  5c:	e3580000 	cmp	r8, #0
	u32 val_frame_ack = (AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  60:	e3a02007 	mov	r2, #7
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  64:	e3c55003 	bic	r5, r5, #3
  68:	e2855e21 	add	r5, r5, #528	@ 0x210
	u32 val_frame_ack = (AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  6c:	e1a04312 	lsl	r4, r2, r3
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  70:	e1a02c01 	lsl	r2, r1, #24
  74:	e1a01401 	lsl	r1, r1, #8
	if (enable) {
  78:	1a00000d 	bne	b4 <ar8327_sw_set_port_igmp_snooping+0xb4>
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  7c:	e1a03001 	mov	r3, r1
  80:	e1a00006 	mov	r0, r6
  84:	e3001624 	movw	r1, #1572	@ 0x624
  88:	ebfffffe 	bl	0 <ar8xxx_rmw>
  8c:	e1a03008 	mov	r3, r8
  90:	e1a02004 	mov	r2, r4
  94:	e1a01005 	mov	r1, r5
  98:	e1a00006 	mov	r0, r6
  9c:	ebfffffe 	bl	0 <ar8xxx_rmw>
	mutex_unlock(&priv->reg_mutex);
  a0:	e1a00007 	mov	r0, r7
  a4:	ebfffffe 	bl	0 <mutex_unlock>
	return 0;
  a8:	e3a00000 	mov	r0, #0
}
  ac:	e24bd024 	sub	sp, fp, #36	@ 0x24
  b0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  b4:	e1a03002 	mov	r3, r2
  b8:	e1a00006 	mov	r0, r6
  bc:	e1a02001 	mov	r2, r1
  c0:	e3001624 	movw	r1, #1572	@ 0x624
  c4:	ebfffffe 	bl	0 <ar8xxx_rmw>
	ar8xxx_rmw(priv, reg, 0, val);
  c8:	e1a03004 	mov	r3, r4
  cc:	e3a02000 	mov	r2, #0
  d0:	e1a01005 	mov	r1, r5
  d4:	e1a00006 	mov	r0, r6
  d8:	ebfffffe 	bl	0 <ar8xxx_rmw>
}
  dc:	eaffffef 	b	a0 <ar8327_sw_set_port_igmp_snooping+0xa0>
		return -EINVAL;
  e0:	e3e00015 	mvn	r0, #21
  e4:	eafffff0 	b	ac <ar8327_sw_set_port_igmp_snooping+0xac>

Disassembly of section .text.ar8327_set_mirror_regs:

00000000 <ar8327_set_mirror_regs>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL0,
   c:	e3a030f0 	mov	r3, #240	@ 0xf0
  10:	e3a01e62 	mov	r1, #1568	@ 0x620
  14:	e1a02003 	mov	r2, r3
  18:	e3005974 	movw	r5, #2420	@ 0x974
  1c:	e3a04e66 	mov	r4, #1632	@ 0x660
{
  20:	e1a06000 	mov	r6, r0
	ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL0,
  24:	ebfffffe 	bl	0 <ar8xxx_rmw>
	ar8xxx_rmw(priv, reg, val, 0);
  28:	e1a01004 	mov	r1, r4
  2c:	e3a03000 	mov	r3, #0
  30:	e3a02402 	mov	r2, #33554432	@ 0x2000000
  34:	e1a00006 	mov	r0, r6
  38:	ebfffffe 	bl	0 <ar8xxx_rmw>
  3c:	e3a03000 	mov	r3, #0
  40:	e1a01005 	mov	r1, r5
  44:	e3a02801 	mov	r2, #65536	@ 0x10000
  48:	e1a00006 	mov	r0, r6
  4c:	ebfffffe 	bl	0 <ar8xxx_rmw>
	for (port = 0; port < AR8327_NUM_PORTS; port++) {
  50:	e284400c 	add	r4, r4, #12
  54:	e30036b4 	movw	r3, #1716	@ 0x6b4
  58:	e1540003 	cmp	r4, r3
  5c:	e2855008 	add	r5, r5, #8
  60:	1afffff0 	bne	28 <ar8327_set_mirror_regs+0x28>
	if (priv->source_port >= AR8327_NUM_PORTS ||
  64:	e2864901 	add	r4, r6, #16384	@ 0x4000
  68:	e5942bd0 	ldr	r2, [r4, #3024]	@ 0xbd0
  6c:	e3520006 	cmp	r2, #6
  70:	ca000007 	bgt	94 <ar8327_set_mirror_regs+0x94>
	    priv->monitor_port >= AR8327_NUM_PORTS ||
  74:	e5943bd4 	ldr	r3, [r4, #3028]	@ 0xbd4
	if (priv->source_port >= AR8327_NUM_PORTS ||
  78:	e3530006 	cmp	r3, #6
  7c:	d3a05000 	movle	r5, #0
  80:	c3a05001 	movgt	r5, #1
	    priv->monitor_port >= AR8327_NUM_PORTS ||
  84:	e1520003 	cmp	r2, r3
  88:	03855001 	orreq	r5, r5, #1
  8c:	e3550000 	cmp	r5, #0
  90:	0a000001 	beq	9c <ar8327_set_mirror_regs+0x9c>
}
  94:	e24bd01c 	sub	sp, fp, #28
  98:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
	ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL0,
  9c:	e1a03203 	lsl	r3, r3, #4
  a0:	e3a020f0 	mov	r2, #240	@ 0xf0
  a4:	e3a01e62 	mov	r1, #1568	@ 0x620
  a8:	e1a00006 	mov	r0, r6
  ac:	ebfffffe 	bl	0 <ar8xxx_rmw>
	if (priv->mirror_rx)
  b0:	e5d43bcc 	ldrb	r3, [r4, #3020]	@ 0xbcc
  b4:	e3530000 	cmp	r3, #0
  b8:	1a00000a 	bne	e8 <ar8327_set_mirror_regs+0xe8>
	if (priv->mirror_tx)
  bc:	e5d43bcd 	ldrb	r3, [r4, #3021]	@ 0xbcd
  c0:	e3530000 	cmp	r3, #0
  c4:	0afffff2 	beq	94 <ar8327_set_mirror_regs+0x94>
		ar8xxx_reg_set(priv, AR8327_REG_PORT_HOL_CTRL1(priv->source_port),
  c8:	e594cbd0 	ldr	ip, [r4, #3024]	@ 0xbd0
  cc:	e3001974 	movw	r1, #2420	@ 0x974
	ar8xxx_rmw(priv, reg, 0, val);
  d0:	e3a03801 	mov	r3, #65536	@ 0x10000
  d4:	e3a02000 	mov	r2, #0
  d8:	e1a00006 	mov	r0, r6
  dc:	e081118c 	add	r1, r1, ip, lsl #3
  e0:	ebfffffe 	bl	0 <ar8xxx_rmw>
}
  e4:	eaffffea 	b	94 <ar8327_set_mirror_regs+0x94>
		ar8xxx_reg_set(priv, AR8327_REG_PORT_LOOKUP(priv->source_port),
  e8:	e5941bd0 	ldr	r1, [r4, #3024]	@ 0xbd0
	ar8xxx_rmw(priv, reg, 0, val);
  ec:	e3a03402 	mov	r3, #33554432	@ 0x2000000
  f0:	e1a02005 	mov	r2, r5
  f4:	e1a00006 	mov	r0, r6
  f8:	e0811081 	add	r1, r1, r1, lsl #1
  fc:	e1a01101 	lsl	r1, r1, #2
 100:	e2811e66 	add	r1, r1, #1632	@ 0x660
 104:	ebfffffe 	bl	0 <ar8xxx_rmw>
}
 108:	eaffffeb 	b	bc <ar8327_set_mirror_regs+0xbc>

Disassembly of section .text.ar8327_init_globals:

00000000 <ar8327_init_globals>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	ar8xxx_write(priv, AR8327_REG_FWD_CTRL0, t);
   c:	e3a02e4f 	mov	r2, #1264	@ 0x4f0
  10:	e3a01e62 	mov	r1, #1568	@ 0x620
	struct ar8327_data *data = priv->chip_data;
  14:	e5905258 	ldr	r5, [r0, #600]	@ 0x258
{
  18:	e1a04000 	mov	r4, r0
	ar8xxx_write(priv, AR8327_REG_FWD_CTRL0, t);
  1c:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_FWD_CTRL1, t);
  20:	e3072f7f 	movw	r2, #32639	@ 0x7f7f
  24:	e1a00004 	mov	r0, r4
  28:	e340207f 	movt	r2, #127	@ 0x7f
  2c:	e3001624 	movw	r1, #1572	@ 0x624
  30:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_rmw(priv, AR8327_REG_MAX_FRAME_SIZE,
  34:	e3023344 	movw	r3, #9028	@ 0x2344
  38:	e3032fff 	movw	r2, #16383	@ 0x3fff
  3c:	e3a01078 	mov	r1, #120	@ 0x78
  40:	e1a00004 	mov	r0, r4
  44:	ebfffffe 	bl	0 <ar8xxx_rmw>
	ar8xxx_rmw(priv, reg, 0, val);
  48:	e3a03001 	mov	r3, #1
  4c:	e3a02000 	mov	r2, #0
  50:	e3a01030 	mov	r1, #48	@ 0x30
  54:	e1a00004 	mov	r0, r4
  58:	ebfffffe 	bl	0 <ar8xxx_rmw>
		data->eee[i] = false;
  5c:	e3a03000 	mov	r3, #0
  60:	e5853010 	str	r3, [r5, #16]
  64:	e5c53014 	strb	r3, [r5, #20]
}
  68:	e24bd014 	sub	sp, fp, #20
  6c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

Disassembly of section .text.ar8327_sw_hw_apply:

00000000 <ar8327_sw_hw_apply>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	const struct ar8327_data *data = priv->chip_data;
   c:	e5906258 	ldr	r6, [r0, #600]	@ 0x258
{
  10:	e1a05000 	mov	r5, r0
	ret = ar8xxx_sw_hw_apply(dev);
  14:	ebfffffe 	bl	0 <ar8xxx_sw_hw_apply>
	if (ret)
  18:	e2507000 	subs	r7, r0, #0
  1c:	1a00000e 	bne	5c <ar8327_sw_hw_apply+0x5c>
  20:	e286600f 	add	r6, r6, #15
  24:	e3a04004 	mov	r4, #4
			       AR8327_EEE_CTRL_DISABLE_PHY(i));
  28:	e3a08001 	mov	r8, #1
		if (data->eee[i])
  2c:	e5f6c001 	ldrb	ip, [r6, #1]!
	ar8xxx_rmw(priv, reg, val, 0);
  30:	e3a03000 	mov	r3, #0
			       AR8327_EEE_CTRL_DISABLE_PHY(i));
  34:	e1a02418 	lsl	r2, r8, r4
	ar8xxx_rmw(priv, reg, 0, val);
  38:	e3a01c01 	mov	r1, #256	@ 0x100
  3c:	e1a00005 	mov	r0, r5
	for (i=0; i < AR8XXX_NUM_PHYS; i++) {
  40:	e2844002 	add	r4, r4, #2
		if (data->eee[i])
  44:	e15c0003 	cmp	ip, r3
  48:	01a03002 	moveq	r3, r2
  4c:	01a0200c 	moveq	r2, ip
  50:	ebfffffe 	bl	0 <ar8xxx_rmw>
	for (i=0; i < AR8XXX_NUM_PHYS; i++) {
  54:	e354000e 	cmp	r4, #14
  58:	1afffff3 	bne	2c <ar8327_sw_hw_apply+0x2c>
}
  5c:	e1a00007 	mov	r0, r7
  60:	e24bd024 	sub	sp, fp, #36	@ 0x24
  64:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

Disassembly of section .text.ar8327_init_port:

00000000 <ar8327_init_port>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (port == AR8216_PORT_CPU)
   c:	e3510000 	cmp	r1, #0
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  10:	e281501f 	add	r5, r1, #31
	struct ar8327_data *data = priv->chip_data;
  14:	e5903258 	ldr	r3, [r0, #600]	@ 0x258
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  18:	e1a05105 	lsl	r5, r5, #2
{
  1c:	e1a04001 	mov	r4, r1
  20:	e1a06000 	mov	r6, r0
	if (port == AR8216_PORT_CPU)
  24:	1a000019 	bne	90 <ar8327_init_port+0x90>
		t = data->port0_status;
  28:	e5932000 	ldr	r2, [r3]
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), t);
  2c:	e1a01005 	mov	r1, r5
  30:	e1a00006 	mov	r0, r6
  34:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_PORT_HEADER(port), 0);
  38:	e2851020 	add	r1, r5, #32
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN0(port), 0);
  3c:	e2845084 	add	r5, r4, #132	@ 0x84
  40:	e1a05185 	lsl	r5, r5, #3
	ar8xxx_write(priv, AR8327_REG_PORT_HEADER(port), 0);
  44:	e1a00006 	mov	r0, r6
  48:	e3a02000 	mov	r2, #0
  4c:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
  50:	e0844084 	add	r4, r4, r4, lsl #1
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN0(port), 0);
  54:	e1a00006 	mov	r0, r6
  58:	e1a01005 	mov	r1, r5
  5c:	e3a02000 	mov	r2, #0
  60:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_PORT_VLAN1(port), t);
  64:	e2851004 	add	r1, r5, #4
  68:	e1a00006 	mov	r0, r6
  6c:	e3a02a03 	mov	r2, #12288	@ 0x3000
  70:	ebfffffe 	bl	0 <ar8xxx_write>
	ar8xxx_write(priv, AR8327_REG_PORT_LOOKUP(port), t);
  74:	e1a01104 	lsl	r1, r4, #2
  78:	e3a02705 	mov	r2, #1310720	@ 0x140000
  7c:	e1a00006 	mov	r0, r6
  80:	e2811e66 	add	r1, r1, #1632	@ 0x660
  84:	ebfffffe 	bl	0 <ar8xxx_write>
}
  88:	e24bd01c 	sub	sp, fp, #28
  8c:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
	else if (port == 6)
  90:	e3510006 	cmp	r1, #6
  94:	0a000009 	beq	c0 <ar8327_init_port+0xc0>
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  98:	e1a01005 	mov	r1, r5
  9c:	e3a02000 	mov	r2, #0
  a0:	ebfffffe 	bl	0 <ar8xxx_write>
		msleep(100);
  a4:	e3a00064 	mov	r0, #100	@ 0x64
  a8:	ebfffffe 	bl	0 <msleep>
		ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), t);
  ac:	e3a02c12 	mov	r2, #4608	@ 0x1200
  b0:	e1a01005 	mov	r1, r5
  b4:	e1a00006 	mov	r0, r6
  b8:	ebfffffe 	bl	0 <ar8xxx_write>
  bc:	eaffffdd 	b	38 <ar8327_init_port+0x38>
		t = data->port6_status;
  c0:	e5932004 	ldr	r2, [r3, #4]
	if (port != AR8216_PORT_CPU && port != 6) {
  c4:	eaffffd8 	b	2c <ar8327_init_port+0x2c>

Disassembly of section .text.ar8327_sw_set_port_link:

00000000 <ar8327_sw_set_port_link>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (port == AR8216_PORT_CPU || port == 6) {
   c:	e3510006 	cmp	r1, #6
  10:	13510000 	cmpne	r1, #0
{
  14:	e1a04000 	mov	r4, r0
  18:	e1a07002 	mov	r7, r2
	if (port == AR8216_PORT_CPU || port == 6) {
  1c:	0a00002e 	beq	dc <ar8327_sw_set_port_link+0xdc>
	t = ar8xxx_read(priv, AR8327_REG_PORT_STATUS(port));
  20:	e281101f 	add	r1, r1, #31
  24:	e1a05101 	lsl	r5, r1, #2
  28:	e1a01005 	mov	r1, r5
  2c:	ebfffffe 	bl	0 <ar8xxx_read>
	if (link->duplex)
  30:	e5d73001 	ldrb	r3, [r7, #1]
	t &= ~AR8216_PORT_STATUS_FLOW_CONTROL;
  34:	e3c00d49 	bic	r0, r0, #4672	@ 0x1240
	if (link->aneg) {
  38:	e5d72002 	ldrb	r2, [r7, #2]
	t &= ~AR8216_PORT_STATUS_FLOW_CONTROL;
  3c:	e3c00003 	bic	r0, r0, #3
	if (link->duplex)
  40:	e2633000 	rsb	r3, r3, #0
	if (link->aneg) {
  44:	e3520000 	cmp	r2, #0
	if (link->duplex)
  48:	e2033040 	and	r3, r3, #64	@ 0x40
  4c:	e1833000 	orr	r3, r3, r0
		t |= AR8216_PORT_STATUS_LINK_AUTO;
  50:	13836c12 	orrne	r6, r3, #4608	@ 0x1200
	if (link->aneg) {
  54:	1a000011 	bne	a0 <ar8327_sw_set_port_link+0xa0>
		if (link->tx_flow)
  58:	e5d76003 	ldrb	r6, [r7, #3]
		t &= ~AR8216_PORT_STATUS_RXFLOW;
  5c:	e3c33030 	bic	r3, r3, #48	@ 0x30
		if (link->rx_flow)
  60:	e5d72004 	ldrb	r2, [r7, #4]
		switch (link->speed) {
  64:	e5971008 	ldr	r1, [r7, #8]
		if (link->tx_flow)
  68:	e2666000 	rsb	r6, r6, #0
		if (link->rx_flow)
  6c:	e2622000 	rsb	r2, r2, #0
		if (link->tx_flow)
  70:	e2066010 	and	r6, r6, #16
		if (link->rx_flow)
  74:	e2022020 	and	r2, r2, #32
		switch (link->speed) {
  78:	e3510064 	cmp	r1, #100	@ 0x64
		if (link->tx_flow)
  7c:	e1866002 	orr	r6, r6, r2
  80:	e1866003 	orr	r6, r6, r3
			t |= AR8216_PORT_SPEED_100M <<
  84:	03866001 	orreq	r6, r6, #1
		switch (link->speed) {
  88:	0a000004 	beq	a0 <ar8327_sw_set_port_link+0xa0>
  8c:	e3510ffa 	cmp	r1, #1000	@ 0x3e8
			t |= AR8216_PORT_SPEED_1000M <<
  90:	03866002 	orreq	r6, r6, #2
		switch (link->speed) {
  94:	0a000001 	beq	a0 <ar8327_sw_set_port_link+0xa0>
  98:	e351000a 	cmp	r1, #10
  9c:	1a00000c 	bne	d4 <ar8327_sw_set_port_link+0xd4>
	ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), 0);
  a0:	e3a02000 	mov	r2, #0
  a4:	e1a01005 	mov	r1, r5
  a8:	e1a00004 	mov	r0, r4
  ac:	ebfffffe 	bl	0 <ar8xxx_write>
	msleep(100);
  b0:	e3a00064 	mov	r0, #100	@ 0x64
  b4:	ebfffffe 	bl	0 <msleep>
	ar8xxx_write(priv, AR8327_REG_PORT_STATUS(port), t);
  b8:	e1a00004 	mov	r0, r4
  bc:	e1a02006 	mov	r2, r6
  c0:	e1a01005 	mov	r1, r5
  c4:	ebfffffe 	bl	0 <ar8xxx_write>
	return 0;
  c8:	e3a00000 	mov	r0, #0
}
  cc:	e24bd01c 	sub	sp, fp, #28
  d0:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
			t |= AR8216_PORT_STATUS_LINK_AUTO;
  d4:	e3866c02 	orr	r6, r6, #512	@ 0x200
			break;
  d8:	eafffff0 	b	a0 <ar8327_sw_set_port_link+0xa0>
		return -EINVAL;
  dc:	e3e00015 	mvn	r0, #21
  e0:	eafffff9 	b	cc <ar8327_sw_set_port_link+0xcc>

Disassembly of section .text.ar8327_phy_rgmii_set:

00000000 <ar8327_phy_rgmii_set>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd008 	sub	sp, sp, #8
	struct device_node *np = phydev->mdio.dev.of_node;
  10:	e59170d8 	ldr	r7, [r1, #216]	@ 0xd8
	u16 phy_val = 0;
  14:	e3a05000 	mov	r5, #0
{
  18:	e1a04000 	mov	r4, r0
	int phyaddr = phydev->mdio.addr;
  1c:	e5916148 	ldr	r6, [r1, #328]	@ 0x148
	u16 phy_val = 0;
  20:	e14b51be 	strh	r5, [fp, #-30]	@ 0xffffffe2
	if (!np)
  24:	e1570005 	cmp	r7, r5
  28:	0a000038 	beq	110 <ar8327_phy_rgmii_set+0x110>
 * Return: true if the property exists false otherwise.
 */
static inline bool of_property_read_bool(const struct device_node *np,
					 const char *propname)
{
	struct property *prop = of_find_property(np, propname, NULL);
  2c:	e3001000 	movw	r1, #0
  30:	e1a02005 	mov	r2, r5
  34:	e3401000 	movt	r1, #0
  38:	e1a00007 	mov	r0, r7
  3c:	ebfffffe 	bl	0 <of_find_property>
	if (!of_property_read_bool(np, "qca,phy-rgmii-en")) {
  40:	e1500005 	cmp	r0, r5
  44:	0a000033 	beq	118 <ar8327_phy_rgmii_set+0x118>
	ar8xxx_phy_dbg_read(priv, phyaddr,
  48:	e24b301e 	sub	r3, fp, #30
  4c:	e3a02012 	mov	r2, #18
  50:	e1a01006 	mov	r1, r6
  54:	e1a00004 	mov	r0, r4
  58:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_read>
	phy_val |= AR8327_PHY_MODE_SEL_RGMII;
  5c:	e15b31be 	ldrh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
  60:	e3a02012 	mov	r2, #18
  64:	e1a01006 	mov	r1, r6
  68:	e1a00004 	mov	r0, r4
	phy_val |= AR8327_PHY_MODE_SEL_RGMII;
  6c:	e3833008 	orr	r3, r3, #8
  70:	e14b31be 	strh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
  74:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
  78:	e3001000 	movw	r1, #0
  7c:	e1a02005 	mov	r2, r5
  80:	e3401000 	movt	r1, #0
  84:	e1a00007 	mov	r0, r7
  88:	ebfffffe 	bl	0 <of_find_property>
	if (!of_property_read_bool(np, "qca,txclk-delay-en")) {
  8c:	e3500000 	cmp	r0, #0
  90:	0a000028 	beq	138 <ar8327_phy_rgmii_set+0x138>
	ar8xxx_phy_dbg_read(priv, phyaddr,
  94:	e24b301e 	sub	r3, fp, #30
  98:	e3a02005 	mov	r2, #5
  9c:	e1a01006 	mov	r1, r6
  a0:	e1a00004 	mov	r0, r4
  a4:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_read>
	phy_val |= AR8327_PHY_SYS_CTRL_RGMII_TX_DELAY;
  a8:	e15b31be 	ldrh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
  ac:	e3a02005 	mov	r2, #5
  b0:	e1a01006 	mov	r1, r6
  b4:	e1a00004 	mov	r0, r4
	phy_val |= AR8327_PHY_SYS_CTRL_RGMII_TX_DELAY;
  b8:	e3833c01 	orr	r3, r3, #256	@ 0x100
  bc:	e14b31be 	strh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
  c0:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
  c4:	e3001000 	movw	r1, #0
  c8:	e1a02005 	mov	r2, r5
  cc:	e1a00007 	mov	r0, r7
  d0:	e3401000 	movt	r1, #0
  d4:	ebfffffe 	bl	0 <of_find_property>
	if (!of_property_read_bool(np, "qca,rxclk-delay-en")) {
  d8:	e3500000 	cmp	r0, #0
  dc:	0a000011 	beq	128 <ar8327_phy_rgmii_set+0x128>
	ar8xxx_phy_dbg_read(priv, phyaddr,
  e0:	e24b301e 	sub	r3, fp, #30
  e4:	e1a02005 	mov	r2, r5
  e8:	e1a01006 	mov	r1, r6
  ec:	e1a00004 	mov	r0, r4
  f0:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_read>
	phy_val |= AR8327_PHY_TEST_CTRL_RGMII_RX_DELAY;
  f4:	e15b31be 	ldrh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
  f8:	e1a02005 	mov	r2, r5
  fc:	e1a01006 	mov	r1, r6
 100:	e1a00004 	mov	r0, r4
	phy_val |= AR8327_PHY_TEST_CTRL_RGMII_RX_DELAY;
 104:	e3833902 	orr	r3, r3, #32768	@ 0x8000
 108:	e14b31be 	strh	r3, [fp, #-30]	@ 0xffffffe2
	ar8xxx_phy_dbg_write(priv, phyaddr,
 10c:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
}
 110:	e24bd01c 	sub	sp, fp, #28
 114:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
		pr_err("ar8327: qca,phy-rgmii-en is not specified\n");
 118:	e3000000 	movw	r0, #0
 11c:	e3400000 	movt	r0, #0
 120:	ebfffffe 	bl	0 <_printk>
		return;
 124:	eafffff9 	b	110 <ar8327_phy_rgmii_set+0x110>
		pr_err("ar8327: qca,rxclk-delay-en is not specified\n");
 128:	e3000000 	movw	r0, #0
 12c:	e3400000 	movt	r0, #0
 130:	ebfffffe 	bl	0 <_printk>
		return;
 134:	eafffff5 	b	110 <ar8327_phy_rgmii_set+0x110>
		pr_err("ar8327: qca,txclk-delay-en is not specified\n");
 138:	e3000000 	movw	r0, #0
 13c:	e3400000 	movt	r0, #0
 140:	ebfffffe 	bl	0 <_printk>
		return;
 144:	eafffff1 	b	110 <ar8327_phy_rgmii_set+0x110>

Disassembly of section .text.ar8327_phy_fixup:

00000000 <ar8327_phy_fixup>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd008 	sub	sp, sp, #8
	switch (priv->chip_rev) {
  10:	e5d03251 	ldrb	r3, [r0, #593]	@ 0x251
{
  14:	e1a04000 	mov	r4, r0
  18:	e1a05001 	mov	r5, r1
	switch (priv->chip_rev) {
  1c:	e3530002 	cmp	r3, #2
  20:	0a00000d 	beq	5c <ar8327_phy_fixup+0x5c>
  24:	e3530004 	cmp	r3, #4
  28:	0a000010 	beq	70 <ar8327_phy_fixup+0x70>
  2c:	e3530001 	cmp	r3, #1
  30:	1a000024 	bne	c8 <ar8327_phy_fixup+0xc8>
		ar8xxx_phy_dbg_write(priv, phy, 0, 0x02ea);
  34:	e30032ea 	movw	r3, #746	@ 0x2ea
  38:	e3a02000 	mov	r2, #0
  3c:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
		ar8xxx_phy_dbg_write(priv, phy, 0x3d, 0x68a0);
  40:	e30638a0 	movw	r3, #26784	@ 0x68a0
  44:	e3a0203d 	mov	r2, #61	@ 0x3d
  48:	e1a01005 	mov	r1, r5
  4c:	e1a00004 	mov	r0, r4
  50:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
}
  54:	e24bd014 	sub	sp, fp, #20
  58:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
		ar8xxx_phy_mmd_write(priv, phy, 0x7, 0x3c, 0x0);
  5c:	e3a02000 	mov	r2, #0
  60:	e3a0303c 	mov	r3, #60	@ 0x3c
  64:	e58d2000 	str	r2, [sp]
  68:	e3a02007 	mov	r2, #7
  6c:	ebfffffe 	bl	0 <ar8xxx_phy_mmd_write>
		ar8xxx_phy_mmd_write(priv, phy, 0x3, 0x800d, 0x803f);
  70:	e308003f 	movw	r0, #32831	@ 0x803f
  74:	e308300d 	movw	r3, #32781	@ 0x800d
  78:	e3a02003 	mov	r2, #3
  7c:	e1a01005 	mov	r1, r5
  80:	e58d0000 	str	r0, [sp]
  84:	e1a00004 	mov	r0, r4
  88:	ebfffffe 	bl	0 <ar8xxx_phy_mmd_write>
		ar8xxx_phy_dbg_write(priv, phy, 0x3d, 0x6860);
  8c:	e3063860 	movw	r3, #26720	@ 0x6860
  90:	e3a0203d 	mov	r2, #61	@ 0x3d
  94:	e1a01005 	mov	r1, r5
  98:	e1a00004 	mov	r0, r4
  9c:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
		ar8xxx_phy_dbg_write(priv, phy, 0x5, 0x2c46);
  a0:	e3023c46 	movw	r3, #11334	@ 0x2c46
  a4:	e3a02005 	mov	r2, #5
  a8:	e1a01005 	mov	r1, r5
  ac:	e1a00004 	mov	r0, r4
  b0:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
		ar8xxx_phy_dbg_write(priv, phy, 0x3c, 0x6000);
  b4:	e3a03a06 	mov	r3, #24576	@ 0x6000
  b8:	e3a0203c 	mov	r2, #60	@ 0x3c
  bc:	e1a01005 	mov	r1, r5
  c0:	e1a00004 	mov	r0, r4
  c4:	ebfffffe 	bl	0 <ar8xxx_phy_dbg_write>
}
  c8:	e24bd014 	sub	sp, fp, #20
  cc:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

Disassembly of section .text.ar8327_vtu_load_vlan:

00000000 <ar8327_vtu_load_vlan>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
		if ((port_mask & BIT(i)) == 0)
   c:	e3120001 	tst	r2, #1
{
  10:	e1a05000 	mov	r5, r0
  14:	e1a06001 	mov	r6, r1
		if ((port_mask & BIT(i)) == 0)
  18:	0a00002f 	beq	dc <ar8327_vtu_load_vlan+0xdc>
		else if (priv->vlan == 0)
  1c:	e2803a01 	add	r3, r0, #4096	@ 0x1000
  20:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
  24:	e3530000 	cmp	r3, #0
  28:	1a000064 	bne	1c0 <ar8327_vtu_load_vlan+0x1c0>
		if ((port_mask & BIT(i)) == 0)
  2c:	e3120002 	tst	r2, #2
	val = AR8327_VTU_FUNC0_VALID | AR8327_VTU_FUNC0_IVL;
  30:	e3a04706 	mov	r4, #1572864	@ 0x180000
		if ((port_mask & BIT(i)) == 0)
  34:	1a00002c 	bne	ec <ar8327_vtu_load_vlan+0xec>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  38:	e38440c0 	orr	r4, r4, #192	@ 0xc0
		if ((port_mask & BIT(i)) == 0)
  3c:	e3120004 	tst	r2, #4
  40:	0a000034 	beq	118 <ar8327_vtu_load_vlan+0x118>
		else if (priv->vlan == 0)
  44:	e2853a01 	add	r3, r5, #4096	@ 0x1000
  48:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
  4c:	e3530000 	cmp	r3, #0
  50:	0a000031 	beq	11c <ar8327_vtu_load_vlan+0x11c>
  54:	e2853901 	add	r3, r5, #16384	@ 0x4000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
  58:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
  5c:	e3110004 	tst	r1, #4
  60:	0a000094 	beq	2b8 <ar8327_vtu_load_vlan+0x2b8>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  64:	e3844c02 	orr	r4, r4, #512	@ 0x200
		if ((port_mask & BIT(i)) == 0)
  68:	e3120008 	tst	r2, #8
  6c:	1a000031 	bne	138 <ar8327_vtu_load_vlan+0x138>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  70:	e3844b03 	orr	r4, r4, #3072	@ 0xc00
		if ((port_mask & BIT(i)) == 0)
  74:	e3120010 	tst	r2, #16
  78:	0a000034 	beq	150 <ar8327_vtu_load_vlan+0x150>
		else if (priv->vlan == 0)
  7c:	e2853a01 	add	r3, r5, #4096	@ 0x1000
  80:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
  84:	e3530000 	cmp	r3, #0
  88:	0a000031 	beq	154 <ar8327_vtu_load_vlan+0x154>
  8c:	e2853901 	add	r3, r5, #16384	@ 0x4000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
  90:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
  94:	e3110010 	tst	r1, #16
  98:	0a000072 	beq	268 <ar8327_vtu_load_vlan+0x268>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  9c:	e3844a02 	orr	r4, r4, #8192	@ 0x2000
		if ((port_mask & BIT(i)) == 0)
  a0:	e3120020 	tst	r2, #32
  a4:	1a000052 	bne	1f4 <ar8327_vtu_load_vlan+0x1f4>
  a8:	e3120040 	tst	r2, #64	@ 0x40
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  ac:	e3844903 	orr	r4, r4, #49152	@ 0xc000
		if ((port_mask & BIT(i)) == 0)
  b0:	0a00002f 	beq	174 <ar8327_vtu_load_vlan+0x174>
		else if (priv->vlan == 0)
  b4:	e2853a01 	add	r3, r5, #4096	@ 0x1000
  b8:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
  bc:	e3530000 	cmp	r3, #0
  c0:	0a00002c 	beq	178 <ar8327_vtu_load_vlan+0x178>
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
  c4:	e2853901 	add	r3, r5, #16384	@ 0x4000
  c8:	e5d32bb6 	ldrb	r2, [r3, #2998]	@ 0xbb6
  cc:	e3120040 	tst	r2, #64	@ 0x40
  d0:	0a000050 	beq	218 <ar8327_vtu_load_vlan+0x218>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  d4:	e3844802 	orr	r4, r4, #131072	@ 0x20000
  d8:	ea000026 	b	178 <ar8327_vtu_load_vlan+0x178>
		if ((port_mask & BIT(i)) == 0)
  dc:	e3120002 	tst	r2, #2
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
  e0:	e3a04030 	mov	r4, #48	@ 0x30
  e4:	e3404018 	movt	r4, #24
		if ((port_mask & BIT(i)) == 0)
  e8:	0affffd2 	beq	38 <ar8327_vtu_load_vlan+0x38>
		else if (priv->vlan == 0)
  ec:	e2853a01 	add	r3, r5, #4096	@ 0x1000
  f0:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
  f4:	e3530000 	cmp	r3, #0
  f8:	0affffcf 	beq	3c <ar8327_vtu_load_vlan+0x3c>
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
  fc:	e2853901 	add	r3, r5, #16384	@ 0x4000
 100:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
 104:	e3110002 	tst	r1, #2
 108:	0a000074 	beq	2e0 <ar8327_vtu_load_vlan+0x2e0>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 10c:	e3844080 	orr	r4, r4, #128	@ 0x80
		if ((port_mask & BIT(i)) == 0)
 110:	e3120004 	tst	r2, #4
 114:	1affffcf 	bne	58 <ar8327_vtu_load_vlan+0x58>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 118:	e3844c03 	orr	r4, r4, #768	@ 0x300
		if ((port_mask & BIT(i)) == 0)
 11c:	e3120008 	tst	r2, #8
 120:	0affffd2 	beq	70 <ar8327_vtu_load_vlan+0x70>
		else if (priv->vlan == 0)
 124:	e2853a01 	add	r3, r5, #4096	@ 0x1000
 128:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
 12c:	e3530000 	cmp	r3, #0
 130:	0affffcf 	beq	74 <ar8327_vtu_load_vlan+0x74>
 134:	e2853901 	add	r3, r5, #16384	@ 0x4000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 138:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
 13c:	e3110008 	tst	r1, #8
 140:	0a000052 	beq	290 <ar8327_vtu_load_vlan+0x290>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 144:	e3844b02 	orr	r4, r4, #2048	@ 0x800
		if ((port_mask & BIT(i)) == 0)
 148:	e3120010 	tst	r2, #16
 14c:	1affffcf 	bne	90 <ar8327_vtu_load_vlan+0x90>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 150:	e3844a03 	orr	r4, r4, #12288	@ 0x3000
		if ((port_mask & BIT(i)) == 0)
 154:	e3120020 	tst	r2, #32
 158:	0affffd2 	beq	a8 <ar8327_vtu_load_vlan+0xa8>
		else if (priv->vlan == 0)
 15c:	e2853a01 	add	r3, r5, #4096	@ 0x1000
 160:	e5d33bb4 	ldrb	r3, [r3, #2996]	@ 0xbb4
 164:	e3530000 	cmp	r3, #0
 168:	1a000020 	bne	1f0 <ar8327_vtu_load_vlan+0x1f0>
		if ((port_mask & BIT(i)) == 0)
 16c:	e3120040 	tst	r2, #64	@ 0x40
 170:	1a000000 	bne	178 <ar8327_vtu_load_vlan+0x178>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 174:	e3844803 	orr	r4, r4, #196608	@ 0x30000
	if (ar8216_wait_bit(priv, AR8327_REG_VTU_FUNC1,
 178:	e3a03000 	mov	r3, #0
 17c:	e3a02102 	mov	r2, #-2147483648	@ 0x80000000
 180:	e3001614 	movw	r1, #1556	@ 0x614
 184:	e1a00005 	mov	r0, r5
 188:	ebfffffe 	bl	0 <ar8216_wait_bit>
 18c:	e3500000 	cmp	r0, #0
 190:	1a000008 	bne	1b8 <ar8327_vtu_load_vlan+0x1b8>
		ar8xxx_write(priv, AR8327_REG_VTU_FUNC0, val);
 194:	e1a02004 	mov	r2, r4
 198:	e3a01e61 	mov	r1, #1552	@ 0x610
 19c:	e1a00005 	mov	r0, r5
 1a0:	ebfffffe 	bl	0 <ar8xxx_write>
	op = AR8327_VTU_FUNC1_OP_LOAD | (vid << AR8327_VTU_FUNC1_VID_S);
 1a4:	e1a02806 	lsl	r2, r6, #16
	ar8xxx_write(priv, AR8327_REG_VTU_FUNC1, op);
 1a8:	e3001614 	movw	r1, #1556	@ 0x614
 1ac:	e1a00005 	mov	r0, r5
 1b0:	e382210a 	orr	r2, r2, #-2147483646	@ 0x80000002
 1b4:	ebfffffe 	bl	0 <ar8xxx_write>
}
 1b8:	e24bd01c 	sub	sp, fp, #28
 1bc:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 1c0:	e2803901 	add	r3, r0, #16384	@ 0x4000
 1c4:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
 1c8:	e3110001 	tst	r1, #1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 1cc:	13a04020 	movne	r4, #32
 1d0:	13404018 	movtne	r4, #24
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 1d4:	0a00004b 	beq	308 <ar8327_vtu_load_vlan+0x308>
		if ((port_mask & BIT(i)) == 0)
 1d8:	e3120002 	tst	r2, #2
 1dc:	0affff95 	beq	38 <ar8327_vtu_load_vlan+0x38>
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 1e0:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
 1e4:	e3110002 	tst	r1, #2
 1e8:	0a00003c 	beq	2e0 <ar8327_vtu_load_vlan+0x2e0>
 1ec:	eaffffc6 	b	10c <ar8327_vtu_load_vlan+0x10c>
 1f0:	e2853901 	add	r3, r5, #16384	@ 0x4000
 1f4:	e5d31bb6 	ldrb	r1, [r3, #2998]	@ 0xbb6
 1f8:	e3110020 	tst	r1, #32
 1fc:	0a00000f 	beq	240 <ar8327_vtu_load_vlan+0x240>
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 200:	e3844902 	orr	r4, r4, #32768	@ 0x8000
		if ((port_mask & BIT(i)) == 0)
 204:	e3120040 	tst	r2, #64	@ 0x40
 208:	0affffd9 	beq	174 <ar8327_vtu_load_vlan+0x174>
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 20c:	e5d32bb6 	ldrb	r2, [r3, #2998]	@ 0xbb6
 210:	e3120040 	tst	r2, #64	@ 0x40
 214:	1affffae 	bne	d4 <ar8327_vtu_load_vlan+0xd4>
 218:	e3002bc4 	movw	r2, #3012	@ 0xbc4
 21c:	e19330b2 	ldrh	r3, [r3, r2]
 220:	e2833edd 	add	r3, r3, #3536	@ 0xdd0
 224:	e2833008 	add	r3, r3, #8
 228:	e0853083 	add	r3, r5, r3, lsl #1
 22c:	e1d330b6 	ldrh	r3, [r3, #6]
 230:	e1530006 	cmp	r3, r6
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 234:	03844801 	orreq	r4, r4, #65536	@ 0x10000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 238:	1affffa5 	bne	d4 <ar8327_vtu_load_vlan+0xd4>
 23c:	eaffffcd 	b	178 <ar8327_vtu_load_vlan+0x178>
 240:	e3001bc2 	movw	r1, #3010	@ 0xbc2
 244:	e19310b1 	ldrh	r1, [r3, r1]
 248:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 24c:	e2811008 	add	r1, r1, #8
 250:	e0851081 	add	r1, r5, r1, lsl #1
 254:	e1d110b6 	ldrh	r1, [r1, #6]
 258:	e1560001 	cmp	r6, r1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 25c:	03844901 	orreq	r4, r4, #16384	@ 0x4000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 260:	1affffe6 	bne	200 <ar8327_vtu_load_vlan+0x200>
 264:	eaffffe6 	b	204 <ar8327_vtu_load_vlan+0x204>
 268:	e2831d2f 	add	r1, r3, #3008	@ 0xbc0
 26c:	e1d110b0 	ldrh	r1, [r1]
 270:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 274:	e2811008 	add	r1, r1, #8
 278:	e0851081 	add	r1, r5, r1, lsl #1
 27c:	e1d110b6 	ldrh	r1, [r1, #6]
 280:	e1560001 	cmp	r6, r1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 284:	03844a01 	orreq	r4, r4, #4096	@ 0x1000
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 288:	1affff83 	bne	9c <ar8327_vtu_load_vlan+0x9c>
 28c:	eaffff83 	b	a0 <ar8327_vtu_load_vlan+0xa0>
 290:	e3001bbe 	movw	r1, #3006	@ 0xbbe
 294:	e19310b1 	ldrh	r1, [r3, r1]
 298:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 29c:	e2811008 	add	r1, r1, #8
 2a0:	e0851081 	add	r1, r5, r1, lsl #1
 2a4:	e1d110b6 	ldrh	r1, [r1, #6]
 2a8:	e1560001 	cmp	r6, r1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 2ac:	03844b01 	orreq	r4, r4, #1024	@ 0x400
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 2b0:	1affffa3 	bne	144 <ar8327_vtu_load_vlan+0x144>
 2b4:	eaffffa3 	b	148 <ar8327_vtu_load_vlan+0x148>
 2b8:	e3001bbc 	movw	r1, #3004	@ 0xbbc
 2bc:	e19310b1 	ldrh	r1, [r3, r1]
 2c0:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 2c4:	e2811008 	add	r1, r1, #8
 2c8:	e0851081 	add	r1, r5, r1, lsl #1
 2cc:	e1d110b6 	ldrh	r1, [r1, #6]
 2d0:	e1560001 	cmp	r6, r1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 2d4:	03844c01 	orreq	r4, r4, #256	@ 0x100
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 2d8:	1affff61 	bne	64 <ar8327_vtu_load_vlan+0x64>
 2dc:	eaffff61 	b	68 <ar8327_vtu_load_vlan+0x68>
 2e0:	e3001bba 	movw	r1, #3002	@ 0xbba
 2e4:	e19310b1 	ldrh	r1, [r3, r1]
 2e8:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 2ec:	e2811008 	add	r1, r1, #8
 2f0:	e0851081 	add	r1, r5, r1, lsl #1
 2f4:	e1d110b6 	ldrh	r1, [r1, #6]
 2f8:	e1560001 	cmp	r6, r1
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 2fc:	03844040 	orreq	r4, r4, #64	@ 0x40
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 300:	1affff81 	bne	10c <ar8327_vtu_load_vlan+0x10c>
 304:	eaffff81 	b	110 <ar8327_vtu_load_vlan+0x110>
 308:	e3001bb8 	movw	r1, #3000	@ 0xbb8
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 30c:	e3a04020 	mov	r4, #32
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 310:	e19310b1 	ldrh	r1, [r3, r1]
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 314:	e3404018 	movt	r4, #24
		else if ((priv->vlan_tagged & BIT(i)) || (priv->vlan_id[priv->pvid[i]] != vid))
 318:	e2811edd 	add	r1, r1, #3536	@ 0xdd0
 31c:	e2811008 	add	r1, r1, #8
 320:	e0801081 	add	r1, r0, r1, lsl #1
 324:	e1d100b6 	ldrh	r0, [r1, #6]
		val |= mode << AR8327_VTU_FUNC0_EG_MODE_S(i);
 328:	e3a01010 	mov	r1, #16
 32c:	e3401018 	movt	r1, #24
 330:	e1560000 	cmp	r6, r0
 334:	01a04001 	moveq	r4, r1
		if ((port_mask & BIT(i)) == 0)
 338:	e3120002 	tst	r2, #2
 33c:	1affffa7 	bne	1e0 <ar8327_vtu_load_vlan+0x1e0>
 340:	eaffff3c 	b	38 <ar8327_vtu_load_vlan+0x38>

Disassembly of section .text.ar8327_atu_flush_port:

00000000 <ar8327_atu_flush_port>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e1a05001 	mov	r5, r1
	ret = ar8216_wait_bit(priv, AR8327_REG_ATU_FUNC,
  10:	e3a03000 	mov	r3, #0
  14:	e3a02102 	mov	r2, #-2147483648	@ 0x80000000
  18:	e300160c 	movw	r1, #1548	@ 0x60c
{
  1c:	e1a06000 	mov	r6, r0
	ret = ar8216_wait_bit(priv, AR8327_REG_ATU_FUNC,
  20:	ebfffffe 	bl	0 <ar8216_wait_bit>
	if (!ret) {
  24:	e2504000 	subs	r4, r0, #0
  28:	0a000002 	beq	38 <ar8327_atu_flush_port+0x38>
}
  2c:	e1a00004 	mov	r0, r4
  30:	e24bd01c 	sub	sp, fp, #28
  34:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
		t = (port << AR8327_ATU_PORT_NUM_S);
  38:	e1a02405 	lsl	r2, r5, #8
		ar8xxx_write(priv, AR8327_REG_ATU_FUNC, t);
  3c:	e1a00006 	mov	r0, r6
  40:	e300160c 	movw	r1, #1548	@ 0x60c
  44:	e3822116 	orr	r2, r2, #-2147483643	@ 0x80000005
  48:	ebfffffe 	bl	0 <ar8xxx_write>
}
  4c:	e1a00004 	mov	r0, r4
  50:	e24bd01c 	sub	sp, fp, #28
  54:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

Disassembly of section .text.ar8327_atu_flush:

00000000 <ar8327_atu_flush>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	ret = ar8216_wait_bit(priv, AR8327_REG_ATU_FUNC,
   c:	e3a03000 	mov	r3, #0
  10:	e3a02102 	mov	r2, #-2147483648	@ 0x80000000
  14:	e300160c 	movw	r1, #1548	@ 0x60c
{
  18:	e1a05000 	mov	r5, r0
	ret = ar8216_wait_bit(priv, AR8327_REG_ATU_FUNC,
  1c:	ebfffffe 	bl	0 <ar8216_wait_bit>
	if (!ret)
  20:	e2504000 	subs	r4, r0, #0
  24:	0a000002 	beq	34 <ar8327_atu_flush+0x34>
}
  28:	e1a00004 	mov	r0, r4
  2c:	e24bd014 	sub	sp, fp, #20
  30:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
		ar8xxx_write(priv, AR8327_REG_ATU_FUNC,
  34:	e1a00005 	mov	r0, r5
  38:	e3a02106 	mov	r2, #-2147483647	@ 0x80000001
  3c:	e300160c 	movw	r1, #1548	@ 0x60c
  40:	ebfffffe 	bl	0 <ar8xxx_write>
}
  44:	e1a00004 	mov	r0, r4
  48:	e24bd014 	sub	sp, fp, #20
  4c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

Disassembly of section .text.ar8327_read_port_eee_status:

00000000 <ar8327_read_port_eee_status>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (port >= priv->dev.ports)
   c:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
  10:	e1530001 	cmp	r3, r1
  14:	9a000002 	bls	24 <ar8327_read_port_eee_status+0x24>
	if (port == 0 || port == 6)
  18:	e3510006 	cmp	r1, #6
  1c:	13510000 	cmpne	r1, #0
  20:	1a000002 	bne	30 <ar8327_read_port_eee_status+0x30>
		return 0;
  24:	e3a00000 	mov	r0, #0
}
  28:	e24bd00c 	sub	sp, fp, #12
  2c:	e89da800 	ldm	sp, {fp, sp, pc}
	t = ar8xxx_phy_mmd_read(priv, phy, 0x7, 0x8000);
  30:	e3a03902 	mov	r3, #32768	@ 0x8000
  34:	e3a02007 	mov	r2, #7
  38:	e2411001 	sub	r1, r1, #1
  3c:	ebfffffe 	bl	0 <ar8xxx_phy_mmd_read>
 */
static inline u32 mmd_eee_adv_to_ethtool_adv_t(u16 eee_adv)
{
	u32 adv = 0;

	if (eee_adv & MDIO_EEE_100TX)
  40:	e7e030d0 	ubfx	r3, r0, #1, #1
  44:	e1a02000 	mov	r2, r0
		adv |= ADVERTISED_100baseT_Full;
	if (eee_adv & MDIO_EEE_1000T)
  48:	e3100004 	tst	r0, #4
	if (eee_adv & MDIO_EEE_100TX)
  4c:	e1a00183 	lsl	r0, r3, #3
		adv |= ADVERTISED_1000baseT_Full;
  50:	13800020 	orrne	r0, r0, #32
	if (eee_adv & MDIO_EEE_10GT)
  54:	e3120008 	tst	r2, #8
		adv |= ADVERTISED_10000baseT_Full;
  58:	13800a01 	orrne	r0, r0, #4096	@ 0x1000
	if (eee_adv & MDIO_EEE_1000KX)
  5c:	e3120010 	tst	r2, #16
		adv |= ADVERTISED_1000baseKX_Full;
  60:	13800802 	orrne	r0, r0, #131072	@ 0x20000
	if (eee_adv & MDIO_EEE_10GKX4)
  64:	e3120020 	tst	r2, #32
		adv |= ADVERTISED_10000baseKX4_Full;
  68:	13800701 	orrne	r0, r0, #262144	@ 0x40000
	if (eee_adv & MDIO_EEE_10GKR)
  6c:	e3120040 	tst	r2, #64	@ 0x40
		adv |= ADVERTISED_10000baseKR_Full;
  70:	13800702 	orrne	r0, r0, #524288	@ 0x80000
}
  74:	e24bd00c 	sub	sp, fp, #12
  78:	e89da800 	ldm	sp, {fp, sp, pc}

Disassembly of section .text.ar8327_vtu_flush:

00000000 <ar8327_vtu_flush>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (ar8216_wait_bit(priv, AR8327_REG_VTU_FUNC1,
   c:	e3a03000 	mov	r3, #0
  10:	e3a02102 	mov	r2, #-2147483648	@ 0x80000000
  14:	e3001614 	movw	r1, #1556	@ 0x614
{
  18:	e1a04000 	mov	r4, r0
	if (ar8216_wait_bit(priv, AR8327_REG_VTU_FUNC1,
  1c:	ebfffffe 	bl	0 <ar8216_wait_bit>
  20:	e3500000 	cmp	r0, #0
  24:	1a000003 	bne	38 <ar8327_vtu_flush+0x38>
	ar8xxx_write(priv, AR8327_REG_VTU_FUNC1, op);
  28:	e3a02106 	mov	r2, #-2147483647	@ 0x80000001
  2c:	e3001614 	movw	r1, #1556	@ 0x614
  30:	e1a00004 	mov	r0, r4
  34:	ebfffffe 	bl	0 <ar8xxx_write>
}
  38:	e24bd014 	sub	sp, fp, #20
  3c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

Disassembly of section .text.ar8327_sw_get_igmp_snooping:

00000000 <ar8327_sw_get_igmp_snooping>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	for (port = 0; port < dev->ports; port++) {
   c:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
{
  10:	e1a06000 	mov	r6, r0
  14:	e1a07002 	mov	r7, r2
	for (port = 0; port < dev->ports; port++) {
  18:	e3530000 	cmp	r3, #0
  1c:	0a00002d 	beq	d8 <ar8327_sw_get_igmp_snooping+0xd8>
  20:	e3a04000 	mov	r4, #0
	mutex_lock(&priv->reg_mutex);
  24:	e2808f8f 	add	r8, r0, #572	@ 0x23c
	fwd_ctrl = (BIT(port) << AR8327_FWD_CTRL1_IGMP_S);
  28:	e3a09001 	mov	r9, #1
  2c:	ea000009 	b	58 <ar8327_sw_get_igmp_snooping+0x58>
	mutex_unlock(&priv->reg_mutex);
  30:	e1a00008 	mov	r0, r8
	val->value.i = ar8327_get_port_igmp(priv, port);
  34:	e587300c 	str	r3, [r7, #12]
	for (port = 0; port < dev->ports; port++) {
  38:	e2844001 	add	r4, r4, #1
	mutex_unlock(&priv->reg_mutex);
  3c:	ebfffffe 	bl	0 <mutex_unlock>
		if (ar8327_sw_get_port_igmp_snooping(dev, attr, val) ||
  40:	e597300c 	ldr	r3, [r7, #12]
  44:	e3530000 	cmp	r3, #0
  48:	0a000022 	beq	d8 <ar8327_sw_get_igmp_snooping+0xd8>
	for (port = 0; port < dev->ports; port++) {
  4c:	e5963024 	ldr	r3, [r6, #36]	@ 0x24
  50:	e1530004 	cmp	r3, r4
  54:	9a00001f 	bls	d8 <ar8327_sw_get_igmp_snooping+0xd8>
		val->port_vlan = port;
  58:	e5874004 	str	r4, [r7, #4]
	if (port >= dev->ports)
  5c:	e5963024 	ldr	r3, [r6, #36]	@ 0x24
	mutex_lock(&priv->reg_mutex);
  60:	e1a00008 	mov	r0, r8
	if (port >= dev->ports)
  64:	e1530004 	cmp	r3, r4
  68:	9a00001a 	bls	d8 <ar8327_sw_get_igmp_snooping+0xd8>
	fwd_ctrl = (BIT(port) << AR8327_FWD_CTRL1_IGMP_S);
  6c:	e1a05419 	lsl	r5, r9, r4
	mutex_lock(&priv->reg_mutex);
  70:	ebfffffe 	bl	0 <mutex_lock>
	fwd_ctrl = (BIT(port) << AR8327_FWD_CTRL1_IGMP_S);
  74:	e1a05c05 	lsl	r5, r5, #24
	return (ar8xxx_read(priv, AR8327_REG_FWD_CTRL1) &
  78:	e3001624 	movw	r1, #1572	@ 0x624
  7c:	e1a00006 	mov	r0, r6
  80:	ebfffffe 	bl	0 <ar8xxx_read>
  84:	e3a03000 	mov	r3, #0
			fwd_ctrl) == fwd_ctrl &&
  88:	e1d55000 	bics	r5, r5, r0
  8c:	1affffe7 	bne	30 <ar8327_sw_get_igmp_snooping+0x30>
		     AR8327_FRAME_ACK_CTRL_S(port));
  90:	e0530004 	subs	r0, r3, r4
  94:	e2043003 	and	r3, r4, #3
  98:	e2000003 	and	r0, r0, #3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  9c:	e2841003 	add	r1, r4, #3
		     AR8327_FRAME_ACK_CTRL_S(port));
  a0:	52603000 	rsbpl	r3, r0, #0
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  a4:	e3540000 	cmp	r4, #0
		     AR8327_FRAME_ACK_CTRL_S(port));
  a8:	e1a03183 	lsl	r3, r3, #3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  ac:	a1a01004 	movge	r1, r4
	frame_ack = ((AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  b0:	e3a02007 	mov	r2, #7
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  b4:	e3c11003 	bic	r1, r1, #3
	frame_ack = ((AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  b8:	e1a05312 	lsl	r5, r2, r3
		(ar8xxx_read(priv, AR8327_REG_FRAME_ACK_CTRL(port)) &
  bc:	e1a00006 	mov	r0, r6
  c0:	e2811e21 	add	r1, r1, #528	@ 0x210
  c4:	ebfffffe 	bl	0 <ar8xxx_read>
			fwd_ctrl) == fwd_ctrl &&
  c8:	e1d52000 	bics	r2, r5, r0
  cc:	03a03001 	moveq	r3, #1
  d0:	13a03000 	movne	r3, #0
  d4:	eaffffd5 	b	30 <ar8327_sw_get_igmp_snooping+0x30>
}
  d8:	e3a00000 	mov	r0, #0
  dc:	e24bd024 	sub	sp, fp, #36	@ 0x24
  e0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

Disassembly of section .text.ar8327_sw_set_igmp_snooping:

00000000 <ar8327_sw_set_igmp_snooping>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd004 	sub	sp, sp, #4
	for (port = 0; port < dev->ports; port++) {
  10:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
{
  14:	e1a07000 	mov	r7, r0
  18:	e1a09002 	mov	r9, r2
	for (port = 0; port < dev->ports; port++) {
  1c:	e3530000 	cmp	r3, #0
  20:	0a00002a 	beq	d0 <ar8327_sw_set_igmp_snooping+0xd0>
  24:	e3a04000 	mov	r4, #0
	mutex_lock(&priv->reg_mutex);
  28:	e280af8f 	add	sl, r0, #572	@ 0x23c
		val->port_vlan = port;
  2c:	e5894004 	str	r4, [r9, #4]
	if (port >= dev->ports)
  30:	e5973024 	ldr	r3, [r7, #36]	@ 0x24
	mutex_lock(&priv->reg_mutex);
  34:	e1a0000a 	mov	r0, sl
	if (port >= dev->ports)
  38:	e1530004 	cmp	r3, r4
  3c:	9a000023 	bls	d0 <ar8327_sw_set_igmp_snooping+0xd0>
	mutex_lock(&priv->reg_mutex);
  40:	ebfffffe 	bl	0 <mutex_lock>
	if (enable) {
  44:	e599800c 	ldr	r8, [r9, #12]
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  48:	e3540000 	cmp	r4, #0
  4c:	e284c003 	add	ip, r4, #3
  50:	a1a0c004 	movge	ip, r4
			 AR8327_FRAME_ACK_CTRL_S(port);
  54:	e2742000 	rsbs	r2, r4, #0
  58:	e2022003 	and	r2, r2, #3
  5c:	e2046003 	and	r6, r4, #3
			   BIT(port) << AR8327_FWD_CTRL1_MC_FLOOD_S,
  60:	e3a03001 	mov	r3, #1
			 AR8327_FRAME_ACK_CTRL_S(port);
  64:	52626000 	rsbpl	r6, r2, #0
	if (enable) {
  68:	e3580000 	cmp	r8, #0
			   BIT(port) << AR8327_FWD_CTRL1_MC_FLOOD_S,
  6c:	e1a03413 	lsl	r3, r3, r4
			 AR8327_FRAME_ACK_CTRL_S(port);
  70:	e1a06186 	lsl	r6, r6, #3
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  74:	e3ccc003 	bic	ip, ip, #3
	u32 val_frame_ack = (AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  78:	e3a02007 	mov	r2, #7
	int reg_frame_ack = AR8327_REG_FRAME_ACK_CTRL(port);
  7c:	e28c5e21 	add	r5, ip, #528	@ 0x210
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  80:	e3001624 	movw	r1, #1572	@ 0x624
	u32 val_frame_ack = (AR8327_FRAME_ACK_CTRL_IGMP_MLD |
  84:	e1a06612 	lsl	r6, r2, r6
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  88:	e1a0c403 	lsl	ip, r3, #8
  8c:	e1a02c03 	lsl	r2, r3, #24
  90:	e1a00007 	mov	r0, r7
	if (enable) {
  94:	0a000010 	beq	dc <ar8327_sw_set_igmp_snooping+0xdc>
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  98:	e1a03002 	mov	r3, r2
  9c:	e1a0200c 	mov	r2, ip
  a0:	ebfffffe 	bl	0 <ar8xxx_rmw>
  a4:	e1a03006 	mov	r3, r6
  a8:	e3a02000 	mov	r2, #0
  ac:	e1a01005 	mov	r1, r5
  b0:	e1a00007 	mov	r0, r7
  b4:	ebfffffe 	bl	0 <ar8xxx_rmw>
	mutex_unlock(&priv->reg_mutex);
  b8:	e1a0000a 	mov	r0, sl
	for (port = 0; port < dev->ports; port++) {
  bc:	e2844001 	add	r4, r4, #1
	mutex_unlock(&priv->reg_mutex);
  c0:	ebfffffe 	bl	0 <mutex_unlock>
	for (port = 0; port < dev->ports; port++) {
  c4:	e5973024 	ldr	r3, [r7, #36]	@ 0x24
  c8:	e1540003 	cmp	r4, r3
  cc:	3affffd6 	bcc	2c <ar8327_sw_set_igmp_snooping+0x2c>
}
  d0:	e3a00000 	mov	r0, #0
  d4:	e24bd028 	sub	sp, fp, #40	@ 0x28
  d8:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		ar8xxx_rmw(priv, AR8327_REG_FWD_CTRL1,
  dc:	e1a0300c 	mov	r3, ip
  e0:	e3001624 	movw	r1, #1572	@ 0x624
  e4:	ebfffffe 	bl	0 <ar8xxx_rmw>
	ar8xxx_rmw(priv, reg, val, 0);
  e8:	e1a03008 	mov	r3, r8
  ec:	e1a02006 	mov	r2, r6
  f0:	e1a01005 	mov	r1, r5
  f4:	e1a00007 	mov	r0, r7
  f8:	ebfffffe 	bl	0 <ar8xxx_rmw>
	mutex_unlock(&priv->reg_mutex);
  fc:	e1a0000a 	mov	r0, sl
	for (port = 0; port < dev->ports; port++) {
 100:	e2844001 	add	r4, r4, #1
	mutex_unlock(&priv->reg_mutex);
 104:	ebfffffe 	bl	0 <mutex_unlock>
	for (port = 0; port < dev->ports; port++) {
 108:	e5973024 	ldr	r3, [r7, #36]	@ 0x24
 10c:	e1530004 	cmp	r3, r4
 110:	8affffc5 	bhi	2c <ar8327_sw_set_igmp_snooping+0x2c>
}
 114:	e3a00000 	mov	r0, #0
 118:	e24bd028 	sub	sp, fp, #40	@ 0x28
 11c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

Disassembly of section .text.ar8327_get_arl_entry:

00000000 <ar8327_get_arl_entry>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	switch (op) {
   c:	e253c000 	subs	ip, r3, #0
	struct mii_bus *bus = priv->mii_bus;
  10:	e590e0f4 	ldr	lr, [r0, #244]	@ 0xf4
{
  14:	e1a04000 	mov	r4, r0
  18:	e1a08001 	mov	r8, r1
  1c:	e1a09002 	mov	r9, r2
	switch (op) {
  20:	0a00002c 	beq	d8 <ar8327_get_arl_entry+0xd8>
  24:	e35c0001 	cmp	ip, #1
  28:	1a000028 	bne	d0 <ar8327_get_arl_entry+0xd0>
		ar8xxx_mii_write32(priv, r2, r1_func,
  2c:	e3a0311a 	mov	r3, #-2147483642	@ 0x80000006
  30:	e3a02006 	mov	r2, #6
  34:	e3a01010 	mov	r1, #16
		udelay(10);
  38:	e3007000 	movw	r7, #0
  3c:	e30c6498 	movw	r6, #50328	@ 0xc498
	int timeout = 20;
  40:	e3a05014 	mov	r5, #20
		udelay(10);
  44:	e3407000 	movt	r7, #0
  48:	e3406020 	movt	r6, #32
		ar8xxx_mii_write32(priv, r2, r1_func,
  4c:	ebfffffe 	bl	0 <ar8xxx_mii_write32>
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
  50:	ea000004 	b	68 <ar8327_get_arl_entry+0x68>
  54:	e2555001 	subs	r5, r5, #1
  58:	0a000051 	beq	1a4 <ar8327_get_arl_entry+0x1a4>
		udelay(10);
  5c:	e5973004 	ldr	r3, [r7, #4]
  60:	e12fff33 	blx	r3

#else

static inline int _cond_resched(void)
{
	return __cond_resched();
  64:	ebfffffe 	bl	0 <__cond_resched>
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
  68:	e3a02006 	mov	r2, #6
  6c:	e3a01010 	mov	r1, #16
  70:	e1a00004 	mov	r0, r4
  74:	ebfffffe 	bl	0 <ar8xxx_mii_read32>
  78:	e3500000 	cmp	r0, #0
		udelay(10);
  7c:	e1a00006 	mov	r0, r6
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
  80:	bafffff3 	blt	54 <ar8327_get_arl_entry+0x54>
		val0 = ar8xxx_mii_read32(priv, r2, r1_data0);
  84:	e3a02000 	mov	r2, #0
  88:	e3a01010 	mov	r1, #16
  8c:	e1a00004 	mov	r0, r4
  90:	ebfffffe 	bl	0 <ar8xxx_mii_read32>
  94:	e1a05000 	mov	r5, r0
		val1 = ar8xxx_mii_read32(priv, r2, r1_data1);
  98:	e3a02002 	mov	r2, #2
  9c:	e3a01010 	mov	r1, #16
  a0:	e1a00004 	mov	r0, r4
  a4:	ebfffffe 	bl	0 <ar8xxx_mii_read32>
  a8:	e1a03000 	mov	r3, r0
		val2 = ar8xxx_mii_read32(priv, r2, r1_data2);
  ac:	e3a02004 	mov	r2, #4
  b0:	e1a00004 	mov	r0, r4
  b4:	e3a01010 	mov	r1, #16
		val1 = ar8xxx_mii_read32(priv, r2, r1_data1);
  b8:	e1a04003 	mov	r4, r3
		val2 = ar8xxx_mii_read32(priv, r2, r1_data2);
  bc:	ebfffffe 	bl	0 <ar8xxx_mii_read32>
		*status = val2 & AR8327_ATU_STATUS;
  c0:	e200300f 	and	r3, r0, #15
		if (!*status)
  c4:	e3530000 	cmp	r3, #0
		*status = val2 & AR8327_ATU_STATUS;
  c8:	e5893000 	str	r3, [r9]
		if (!*status)
  cc:	1a00002e 	bne	18c <ar8327_get_arl_entry+0x18c>
}
  d0:	e24bd024 	sub	sp, fp, #36	@ 0x24
  d4:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		bus->write(bus, 0x18, 0, page);
  d8:	e59e5050 	ldr	r5, [lr, #80]	@ 0x50
  dc:	e3a03003 	mov	r3, #3
  e0:	e1a0200c 	mov	r2, ip
  e4:	e3a01018 	mov	r1, #24
  e8:	e1a0000e 	mov	r0, lr
}

static inline void
wait_for_page_switch(void)
{
	udelay(5);
  ec:	e3006000 	movw	r6, #0
		udelay(10);
  f0:	e30c7498 	movw	r7, #50328	@ 0xc498
		bus->write(bus, 0x18, 0, page);
  f4:	e12fff35 	blx	r5
  f8:	e3406000 	movt	r6, #0
  fc:	e306024c 	movw	r0, #25164	@ 0x624c
 100:	e5963004 	ldr	r3, [r6, #4]
 104:	e3400010 	movt	r0, #16
		udelay(10);
 108:	e3407020 	movt	r7, #32
	int timeout = 20;
 10c:	e3a05014 	mov	r5, #20
 110:	e12fff33 	blx	r3
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
 114:	ea000004 	b	12c <ar8327_get_arl_entry+0x12c>
 118:	e2555001 	subs	r5, r5, #1
 11c:	0a000024 	beq	1b4 <ar8327_get_arl_entry+0x1b4>
		udelay(10);
 120:	e5963004 	ldr	r3, [r6, #4]
 124:	e12fff33 	blx	r3
 128:	ebfffffe 	bl	0 <__cond_resched>
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
 12c:	e3a02006 	mov	r2, #6
 130:	e3a01010 	mov	r1, #16
 134:	e1a00004 	mov	r0, r4
 138:	ebfffffe 	bl	0 <ar8xxx_mii_read32>
 13c:	e3500000 	cmp	r0, #0
		udelay(10);
 140:	e1a00007 	mov	r0, r7
	while (ar8xxx_mii_read32(priv, r2, r1) & AR8327_ATU_FUNC_BUSY && --timeout) {
 144:	bafffff3 	blt	118 <ar8327_get_arl_entry+0x118>
		ar8xxx_mii_write32(priv, r2, r1_data0, 0);
 148:	e3a03000 	mov	r3, #0
 14c:	e1a00004 	mov	r0, r4
 150:	e1a02003 	mov	r2, r3
 154:	e3a01010 	mov	r1, #16
 158:	ebfffffe 	bl	0 <ar8xxx_mii_write32>
		ar8xxx_mii_write32(priv, r2, r1_data1, 0);
 15c:	e1a00004 	mov	r0, r4
 160:	e3a03000 	mov	r3, #0
 164:	e3a02002 	mov	r2, #2
 168:	e3a01010 	mov	r1, #16
 16c:	ebfffffe 	bl	0 <ar8xxx_mii_write32>
		ar8xxx_mii_write32(priv, r2, r1_data2, 0);
 170:	e3a03000 	mov	r3, #0
 174:	e3a02004 	mov	r2, #4
 178:	e3a01010 	mov	r1, #16
 17c:	e1a00004 	mov	r0, r4
 180:	ebfffffe 	bl	0 <ar8xxx_mii_write32>
}
 184:	e24bd024 	sub	sp, fp, #36	@ 0x24
 188:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		a->portmap = (val1 & AR8327_ATU_PORTS) >> AR8327_ATU_PORTS_S;
 18c:	e7e63854 	ubfx	r3, r4, #16, #7
 190:	e1c830b0 	strh	r3, [r8]
		a->mac[0] = (val0 & AR8327_ATU_ADDR0) >> AR8327_ATU_ADDR0_S;
 194:	e5885002 	str	r5, [r8, #2]
		a->mac[4] = (val1 & AR8327_ATU_ADDR4) >> AR8327_ATU_ADDR4_S;
 198:	e1c840b6 	strh	r4, [r8, #6]
}
 19c:	e24bd024 	sub	sp, fp, #36	@ 0x24
 1a0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		pr_err("ar8327: timeout waiting for atu to become ready\n");
 1a4:	e3000000 	movw	r0, #0
 1a8:	e3400000 	movt	r0, #0
 1ac:	ebfffffe 	bl	0 <_printk>
}
 1b0:	eaffffb3 	b	84 <ar8327_get_arl_entry+0x84>
		pr_err("ar8327: timeout waiting for atu to become ready\n");
 1b4:	e3000000 	movw	r0, #0
 1b8:	e3400000 	movt	r0, #0
 1bc:	ebfffffe 	bl	0 <_printk>
}
 1c0:	eaffffe0 	b	148 <ar8327_get_arl_entry+0x148>

Disassembly of section .text.ar8327_hw_init:

00000000 <ar8327_hw_init>:
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd010 	sub	sp, sp, #16
		index = kmalloc_index(size);

		if (!index)
			return ZERO_SIZE_PTR;

		return kmalloc_trace(
  10:	e3003000 	movw	r3, #0
  14:	e1a06000 	mov	r6, r0
  18:	e3403000 	movt	r3, #0
  1c:	e3a02038 	mov	r2, #56	@ 0x38
  20:	e5930018 	ldr	r0, [r3, #24]
  24:	e3a01d37 	mov	r1, #3520	@ 0xdc0
  28:	ebfffffe 	bl	0 <kmalloc_trace>
	if (!priv->chip_data)
  2c:	e3500000 	cmp	r0, #0
  30:	e1a07000 	mov	r7, r0
	priv->chip_data = kzalloc(sizeof(struct ar8327_data), GFP_KERNEL);
  34:	e5860258 	str	r0, [r6, #600]	@ 0x258
	if (!priv->chip_data)
  38:	0a0000bd 	beq	334 <ar8327_hw_init+0x334>
	if (priv->pdev->of_node)
  3c:	e5963100 	ldr	r3, [r6, #256]	@ 0x100
  40:	e59340d8 	ldr	r4, [r3, #216]	@ 0xd8
  44:	e3540000 	cmp	r4, #0
  48:	0a000042 	beq	158 <ar8327_hw_init+0x158>
  4c:	e3001000 	movw	r1, #0
  50:	e3a02000 	mov	r2, #0
  54:	e3401000 	movt	r1, #0
  58:	e1a00004 	mov	r0, r4
  5c:	ebfffffe 	bl	0 <of_find_property>
	reset_init = active_high ? GPIOD_OUT_HIGH : GPIOD_OUT_LOW;
  60:	e2508000 	subs	r8, r0, #0
	reset = devm_gpiod_get_optional(priv->pdev, "reset", reset_init);
  64:	e3001000 	movw	r1, #0
  68:	e5960100 	ldr	r0, [r6, #256]	@ 0x100
  6c:	03a02003 	moveq	r2, #3
  70:	13a02007 	movne	r2, #7
  74:	e3401000 	movt	r1, #0
  78:	e24b9028 	sub	r9, fp, #40	@ 0x28
  7c:	ebfffffe 	bl	0 <devm_gpiod_get_optional>
	if (!reset)
  80:	e2505000 	subs	r5, r0, #0
  84:	0a00000f 	beq	c8 <ar8327_hw_init+0xc8>
 */
static inline int of_property_read_u32_array(const struct device_node *np,
					     const char *propname,
					     u32 *out_values, size_t sz)
{
	int ret = of_property_read_variable_u32_array(np, propname, out_values,
  88:	e3a03000 	mov	r3, #0
  8c:	e3001000 	movw	r1, #0
  90:	e3401000 	movt	r1, #0
  94:	e1a02009 	mov	r2, r9
  98:	e1a00004 	mov	r0, r4
  9c:	e58d3000 	str	r3, [sp]
  a0:	e3a03001 	mov	r3, #1
  a4:	ebfffffe 	bl	0 <of_property_read_variable_u32_array>
	if (msec > 20)
  a8:	e51b0028 	ldr	r0, [fp, #-40]	@ 0xffffffd8
  ac:	e3500014 	cmp	r0, #20
  b0:	da0000a1 	ble	33c <ar8327_hw_init+0x33c>
		msleep(msec);
  b4:	ebfffffe 	bl	0 <msleep>
	gpiod_set_value_cansleep(reset, !active_high);
  b8:	e16f1f18 	clz	r1, r8
  bc:	e1a00005 	mov	r0, r5
  c0:	e1a012a1 	lsr	r1, r1, #5
  c4:	ebfffffe 	bl	0 <gpiod_set_value_cansleep>
	paddr = of_get_property(np, "qca,ar8327-initvals", &len);
  c8:	e3001000 	movw	r1, #0
  cc:	e1a02009 	mov	r2, r9
  d0:	e1a00004 	mov	r0, r4
  d4:	e3401000 	movt	r1, #0
  d8:	ebfffffe 	bl	0 <of_get_property>
	if (!paddr || len < (2 * sizeof(*paddr)))
  dc:	e3500000 	cmp	r0, #0
  e0:	0a00009f 	beq	364 <ar8327_hw_init+0x364>
  e4:	e51bc028 	ldr	ip, [fp, #-40]	@ 0xffffffd8
  e8:	e35c0007 	cmp	ip, #7
  ec:	9a00009c 	bls	364 <ar8327_hw_init+0x364>
	len /= sizeof(*paddr);
  f0:	e1a0c12c 	lsr	ip, ip, #2
  f4:	e2804004 	add	r4, r0, #4
	for (i = 0; i < len - 1; i += 2) {
  f8:	e3a05000 	mov	r5, #0
	len /= sizeof(*paddr);
  fc:	e50bc028 	str	ip, [fp, #-40]	@ 0xffffffd8
static __always_inline __u32 __swab32p(const __u32 *p)
{
#ifdef __arch_swab32p
	return __arch_swab32p(p);
#else
	return __swab32(*p);
 100:	e8140006 	ldmda	r4, {r1, r2}
		switch (reg) {
 104:	e351031f 	cmp	r1, #2080374784	@ 0x7c000000
 108:	e6bf2f32 	rev	r2, r2
			data->port0_status = val;
 10c:	05872000 	streq	r2, [r7]
		switch (reg) {
 110:	0a000006 	beq	130 <ar8327_hw_init+0x130>
 114:	e3510325 	cmp	r1, #-1811939328	@ 0x94000000
			data->port6_status = val;
 118:	05872004 	streq	r2, [r7, #4]
		switch (reg) {
 11c:	0a000003 	beq	130 <ar8327_hw_init+0x130>
			ar8xxx_write(priv, reg, val);
 120:	e6bf1f31 	rev	r1, r1
 124:	e1a00006 	mov	r0, r6
 128:	ebfffffe 	bl	0 <ar8xxx_write>
	for (i = 0; i < len - 1; i += 2) {
 12c:	e51bc028 	ldr	ip, [fp, #-40]	@ 0xffffffd8
 130:	e2855002 	add	r5, r5, #2
 134:	e24c3001 	sub	r3, ip, #1
 138:	e1550003 	cmp	r5, r3
 13c:	e2844008 	add	r4, r4, #8
 140:	baffffee 	blt	100 <ar8327_hw_init+0x100>
	ar8xxx_phy_init(priv);
 144:	e1a00006 	mov	r0, r6
 148:	ebfffffe 	bl	0 <ar8xxx_phy_init>
	return 0;
 14c:	e3a00000 	mov	r0, #0
}
 150:	e24bd024 	sub	sp, fp, #36	@ 0x24
 154:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
					     priv->phy->mdio.dev.platform_data);
 158:	e59630fc 	ldr	r3, [r6, #252]	@ 0xfc
 15c:	e593403c 	ldr	r4, [r3, #60]	@ 0x3c
	if (!pdata)
 160:	e3540000 	cmp	r4, #0
 164:	0a00007e 	beq	364 <ar8327_hw_init+0x364>
	priv->get_port_link = pdata->get_port_link;
 168:	e594302c 	ldr	r3, [r4, #44]	@ 0x2c
 16c:	e5863104 	str	r3, [r6, #260]	@ 0x104
	if (!cfg->force_link)
 170:	e5d43010 	ldrb	r3, [r4, #16]
 174:	e3130001 	tst	r3, #1
		return AR8216_PORT_STATUS_LINK_AUTO;
 178:	03a03c02 	moveq	r3, #512	@ 0x200
	if (!cfg->force_link)
 17c:	0a000009 	beq	1a8 <ar8327_hw_init+0x1a8>
	t |= cfg->duplex ? AR8216_PORT_STATUS_DUPLEX : 0;
 180:	e5d43018 	ldrb	r3, [r4, #24]
	switch (cfg->speed) {
 184:	e5942014 	ldr	r2, [r4, #20]
	t |= cfg->duplex ? AR8216_PORT_STATUS_DUPLEX : 0;
 188:	e1a03203 	lsl	r3, r3, #4
	switch (cfg->speed) {
 18c:	e3520001 	cmp	r2, #1
 190:	e2033070 	and	r3, r3, #112	@ 0x70
		t |= AR8216_PORT_SPEED_100M;
 194:	0383300d 	orreq	r3, r3, #13
	switch (cfg->speed) {
 198:	0a000002 	beq	1a8 <ar8327_hw_init+0x1a8>
 19c:	e3520002 	cmp	r2, #2
	t |= cfg->txpause ? AR8216_PORT_STATUS_TXFLOW : 0;
 1a0:	1383300c 	orrne	r3, r3, #12
		t |= AR8216_PORT_SPEED_1000M;
 1a4:	0383300e 	orreq	r3, r3, #14
	data->port0_status = ar8327_get_port_init_status(&pdata->port0_cfg);
 1a8:	e5873000 	str	r3, [r7]
	if (!cfg->force_link)
 1ac:	e5d4301c 	ldrb	r3, [r4, #28]
 1b0:	e3130001 	tst	r3, #1
		return AR8216_PORT_STATUS_LINK_AUTO;
 1b4:	03a03c02 	moveq	r3, #512	@ 0x200
	if (!cfg->force_link)
 1b8:	0a000009 	beq	1e4 <ar8327_hw_init+0x1e4>
	t |= cfg->duplex ? AR8216_PORT_STATUS_DUPLEX : 0;
 1bc:	e5d43024 	ldrb	r3, [r4, #36]	@ 0x24
	switch (cfg->speed) {
 1c0:	e5942020 	ldr	r2, [r4, #32]
	t |= cfg->duplex ? AR8216_PORT_STATUS_DUPLEX : 0;
 1c4:	e1a03203 	lsl	r3, r3, #4
	switch (cfg->speed) {
 1c8:	e3520001 	cmp	r2, #1
 1cc:	e2033070 	and	r3, r3, #112	@ 0x70
		t |= AR8216_PORT_SPEED_100M;
 1d0:	0383300d 	orreq	r3, r3, #13
	switch (cfg->speed) {
 1d4:	0a000002 	beq	1e4 <ar8327_hw_init+0x1e4>
 1d8:	e3520002 	cmp	r2, #2
	t |= cfg->txpause ? AR8216_PORT_STATUS_TXFLOW : 0;
 1dc:	1383300c 	orrne	r3, r3, #12
		t |= AR8216_PORT_SPEED_1000M;
 1e0:	0383300e 	orreq	r3, r3, #14
	data->port6_status = ar8327_get_port_init_status(&pdata->port6_cfg);
 1e4:	e5873004 	str	r3, [r7, #4]
	t = ar8327_get_pad_cfg(pdata->pad0_cfg);
 1e8:	e5945000 	ldr	r5, [r4]
 1ec:	e1a00005 	mov	r0, r5
 1f0:	ebfffffe 	bl	0 <ar8327_hw_init>
	if (chip_is_ar8337(priv) && !pdata->pad0_cfg->mac06_exchange_dis)
 1f4:	e5d63250 	ldrb	r3, [r6, #592]	@ 0x250
	t = ar8327_get_pad_cfg(pdata->pad0_cfg);
 1f8:	e1a02000 	mov	r2, r0
	if (chip_is_ar8337(priv) && !pdata->pad0_cfg->mac06_exchange_dis)
 1fc:	e3530013 	cmp	r3, #19
 200:	0a000053 	beq	354 <ar8327_hw_init+0x354>
	ar8xxx_write(priv, AR8327_REG_PAD0_MODE, t);
 204:	e3a01004 	mov	r1, #4
 208:	e1a00006 	mov	r0, r6
 20c:	ebfffffe 	bl	0 <ar8xxx_write>
	t = ar8327_get_pad_cfg(pdata->pad5_cfg);
 210:	e5940004 	ldr	r0, [r4, #4]
 214:	ebfffffe 	bl	0 <ar8327_hw_init>
 218:	e1a02000 	mov	r2, r0
	ar8xxx_write(priv, AR8327_REG_PAD5_MODE, t);
 21c:	e3a01008 	mov	r1, #8
 220:	e1a00006 	mov	r0, r6
 224:	ebfffffe 	bl	0 <ar8xxx_write>
	t = ar8327_get_pad_cfg(pdata->pad6_cfg);
 228:	e5940008 	ldr	r0, [r4, #8]
 22c:	ebfffffe 	bl	0 <ar8327_hw_init>
 230:	e1a02000 	mov	r2, r0
	ar8xxx_write(priv, AR8327_REG_PAD6_MODE, t);
 234:	e3a0100c 	mov	r1, #12
 238:	e1a00006 	mov	r0, r6
 23c:	ebfffffe 	bl	0 <ar8xxx_write>
	pos = ar8xxx_read(priv, AR8327_REG_POWER_ON_STRAP);
 240:	e3a01010 	mov	r1, #16
 244:	e1a00006 	mov	r0, r6
 248:	ebfffffe 	bl	0 <ar8xxx_read>
	led_cfg = pdata->led_cfg;
 24c:	e5948028 	ldr	r8, [r4, #40]	@ 0x28
	pos = ar8xxx_read(priv, AR8327_REG_POWER_ON_STRAP);
 250:	e1a05000 	mov	r5, r0
	if (led_cfg) {
 254:	e3580000 	cmp	r8, #0
 258:	0a000015 	beq	2b4 <ar8327_hw_init+0x2b4>
		if (led_cfg->open_drain)
 25c:	e5d83010 	ldrb	r3, [r8, #16]
		ar8xxx_write(priv, AR8327_REG_LED_CTRL0, led_cfg->led_ctrl0);
 260:	e3a01050 	mov	r1, #80	@ 0x50
 264:	e5982000 	ldr	r2, [r8]
		if (led_cfg->open_drain)
 268:	e3530000 	cmp	r3, #0
			new_pos |= AR8327_POWER_ON_STRAP_LED_OPEN_EN;
 26c:	13809401 	orrne	r9, r0, #16777216	@ 0x1000000
			new_pos &= ~AR8327_POWER_ON_STRAP_LED_OPEN_EN;
 270:	03c09401 	biceq	r9, r0, #16777216	@ 0x1000000
		ar8xxx_write(priv, AR8327_REG_LED_CTRL0, led_cfg->led_ctrl0);
 274:	e1a00006 	mov	r0, r6
 278:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL1, led_cfg->led_ctrl1);
 27c:	e5982004 	ldr	r2, [r8, #4]
 280:	e3a01054 	mov	r1, #84	@ 0x54
 284:	e1a00006 	mov	r0, r6
 288:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL2, led_cfg->led_ctrl2);
 28c:	e5982008 	ldr	r2, [r8, #8]
 290:	e3a01058 	mov	r1, #88	@ 0x58
 294:	e1a00006 	mov	r0, r6
 298:	ebfffffe 	bl	0 <ar8xxx_write>
		ar8xxx_write(priv, AR8327_REG_LED_CTRL3, led_cfg->led_ctrl3);
 29c:	e598200c 	ldr	r2, [r8, #12]
 2a0:	e3a0105c 	mov	r1, #92	@ 0x5c
 2a4:	e1a00006 	mov	r0, r6
 2a8:	ebfffffe 	bl	0 <ar8xxx_write>
		if (new_pos != pos)
 2ac:	e1550009 	cmp	r5, r9
			new_pos |= AR8327_POWER_ON_STRAP_POWER_ON_SEL;
 2b0:	13895102 	orrne	r5, r9, #-2147483648	@ 0x80000000
	if (pdata->sgmii_cfg) {
 2b4:	e594300c 	ldr	r3, [r4, #12]
 2b8:	e3530000 	cmp	r3, #0
 2bc:	0a00000c 	beq	2f4 <ar8327_hw_init+0x2f4>
		if (priv->chip_rev == 1)
 2c0:	e5d61251 	ldrb	r1, [r6, #593]	@ 0x251
		ar8xxx_write(priv, AR8327_REG_SGMII_CTRL, t);
 2c4:	e1a00006 	mov	r0, r6
		t = pdata->sgmii_cfg->sgmii_ctrl;
 2c8:	e5932000 	ldr	r2, [r3]
		if (priv->chip_rev == 1)
 2cc:	e3510001 	cmp	r1, #1
		ar8xxx_write(priv, AR8327_REG_SGMII_CTRL, t);
 2d0:	e3a010e0 	mov	r1, #224	@ 0xe0
			t |= AR8327_SGMII_CTRL_EN_PLL |
 2d4:	0382200e 	orreq	r2, r2, #14
			t &= ~(AR8327_SGMII_CTRL_EN_PLL |
 2d8:	13c2200e 	bicne	r2, r2, #14
		ar8xxx_write(priv, AR8327_REG_SGMII_CTRL, t);
 2dc:	ebfffffe 	bl	0 <ar8xxx_write>
		if (pdata->sgmii_cfg->serdes_aen)
 2e0:	e594300c 	ldr	r3, [r4, #12]
 2e4:	e5d33004 	ldrb	r3, [r3, #4]
 2e8:	e3530000 	cmp	r3, #0
			new_pos &= ~AR8327_POWER_ON_STRAP_SERDES_AEN;
 2ec:	13c55080 	bicne	r5, r5, #128	@ 0x80
			new_pos |= AR8327_POWER_ON_STRAP_SERDES_AEN;
 2f0:	03855080 	orreq	r5, r5, #128	@ 0x80
	ar8xxx_write(priv, AR8327_REG_POWER_ON_STRAP, new_pos);
 2f4:	e1a02005 	mov	r2, r5
 2f8:	e3a01010 	mov	r1, #16
 2fc:	e1a00006 	mov	r0, r6
 300:	ebfffffe 	bl	0 <ar8xxx_write>
	if (pdata->leds && pdata->num_leds) {
 304:	e5943034 	ldr	r3, [r4, #52]	@ 0x34
 308:	e3530000 	cmp	r3, #0
 30c:	0affff8c 	beq	144 <ar8327_hw_init+0x144>
 310:	e5940030 	ldr	r0, [r4, #48]	@ 0x30
 314:	e3500000 	cmp	r0, #0
 318:	0affff89 	beq	144 <ar8327_hw_init+0x144>
				kmalloc_caches[kmalloc_type(flags)][index],
				flags, size);
#endif
	}
	return __kmalloc(size, flags);
 31c:	e1a00100 	lsl	r0, r0, #2
 320:	e3a01d37 	mov	r1, #3520	@ 0xdc0
 324:	ebfffffe 	bl	0 <__kmalloc>
		if (!data->leds)
 328:	e3500000 	cmp	r0, #0
		data->leds = kzalloc(pdata->num_leds * sizeof(void *),
 32c:	e5870008 	str	r0, [r7, #8]
		if (!data->leds)
 330:	1affff83 	bne	144 <ar8327_hw_init+0x144>
		return -ENOMEM;
 334:	e3e0000b 	mvn	r0, #11
 338:	eaffff84 	b	150 <ar8327_hw_init+0x150>
		usleep_range(msec * 1000, msec * 1000 + 1000);
 33c:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
void usleep_range_state(unsigned long min, unsigned long max,
			unsigned int state);

static inline void usleep_range(unsigned long min, unsigned long max)
{
	usleep_range_state(min, max, TASK_UNINTERRUPTIBLE);
 340:	e3a02002 	mov	r2, #2
 344:	e0000093 	mul	r0, r3, r0
 348:	e0801003 	add	r1, r0, r3
 34c:	ebfffffe 	bl	0 <usleep_range_state>
}
 350:	eaffff58 	b	b8 <ar8327_hw_init+0xb8>
	if (chip_is_ar8337(priv) && !pdata->pad0_cfg->mac06_exchange_dis)
 354:	e5d53014 	ldrb	r3, [r5, #20]
 358:	e3530000 	cmp	r3, #0
	    t |= AR8337_PAD_MAC06_EXCHANGE_EN;
 35c:	03802102 	orreq	r2, r0, #-2147483648	@ 0x80000000
 360:	eaffffa7 	b	204 <ar8327_hw_init+0x204>
		return ret;
 364:	e3e00015 	mvn	r0, #21
 368:	eaffff78 	b	150 <ar8327_hw_init+0x150>
