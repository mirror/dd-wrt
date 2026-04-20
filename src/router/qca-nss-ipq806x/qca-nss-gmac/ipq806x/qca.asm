
qca-nss-gmac.o:     file format elf32-littlearm


Disassembly of section .text:

00000000 <nss_gmac_check_pcs_status>:

	ctx = gmacdev->ctx;
	qsgmii_base = ctx->qsgmii_base;
	id = gmacdev->sgmii_pcs_chanid;

	gmacdev->link_state = LINKDOWN;
       0:	e3a01000 	mov	r1, #0
	id = gmacdev->sgmii_pcs_chanid;
       4:	e5902010 	ldr	r2, [r0, #16]
	qsgmii_base = ctx->qsgmii_base;
       8:	e5903284 	ldr	r3, [r0, #644]	@ 0x284
       c:	e593300c 	ldr	r3, [r3, #12]
	gmacdev->link_state = LINKDOWN;
      10:	e5801064 	str	r1, [r0, #100]	@ 0x64

#define __raw_readl __raw_readl
static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 val;
	asm volatile("ldr %0, %1"
      14:	e5933074 	ldr	r3, [r3, #116]	@ 0x74

	/* confirm link is up in PCS_QSGMII_MAC_STATUS register */
	reg = nss_gmac_read_reg(qsgmii_base, PCS_QSGMII_MAC_STAT);
	if (!(reg & PCS_MAC_STAT_CHn_LINK(id)))
      18:	e1a02182 	lsl	r2, r2, #3
      1c:	e3a01001 	mov	r1, #1
      20:	e013c211 	ands	ip, r3, r1, lsl r2
      24:	012fff1e 	bxeq	lr
		return;

	gmacdev->link_state = LINKUP;
      28:	e5801064 	str	r1, [r0, #100]	@ 0x64

	/* save duplexity */
	if (reg & PCS_MAC_STAT_CHn_DUPLEX(id))
      2c:	e3a01002 	mov	r1, #2
      30:	e0131211 	ands	r1, r3, r1, lsl r2
      34:	13a01001 	movne	r1, #1
      38:	03a01000 	moveq	r1, #0
      3c:	e5801068 	str	r1, [r0, #104]	@ 0x68
		gmacdev->duplex_mode = DUPLEX_FULL;
	else
		gmacdev->duplex_mode = DUPLEX_HALF;

	/* save speed */
	switch (PCS_MAC_STAT_CHn_SPEED(id, reg)) {
      40:	e3a0100c 	mov	r1, #12
      44:	e0033211 	and	r3, r3, r1, lsl r2
      48:	e2822002 	add	r2, r2, #2
      4c:	e1a03233 	lsr	r3, r3, r2
      50:	e3530001 	cmp	r3, #1
      54:	0a000005 	beq	70 <nss_gmac_check_pcs_status+0x70>
      58:	e3530002 	cmp	r3, #2
      5c:	0a000005 	beq	78 <nss_gmac_check_pcs_status+0x78>
      60:	e3530000 	cmp	r3, #0
	case 0:
		gmacdev->speed = SPEED_10;
      64:	03a0300a 	moveq	r3, #10
	switch (PCS_MAC_STAT_CHn_SPEED(id, reg)) {
      68:	0a000003 	beq	7c <nss_gmac_check_pcs_status+0x7c>
      6c:	e12fff1e 	bx	lr
		break;

	case 1:
		gmacdev->speed = SPEED_100;
      70:	e3a03064 	mov	r3, #100	@ 0x64
      74:	ea000000 	b	7c <nss_gmac_check_pcs_status+0x7c>
		break;

	case 2:
		gmacdev->speed = SPEED_1000;
      78:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
      7c:	e580306c 	str	r3, [r0, #108]	@ 0x6c
		break;
	}
}
      80:	e12fff1e 	bx	lr

00000084 <phy_write.isra.0>:
 *
 * NOTE: MUST NOT be called from interrupt context,
 * because the bus read/write functions may wait for an interrupt
 * to conclude the operation.
 */
static inline int phy_write(struct phy_device *phydev, u32 regnum, u16 val)
      84:	e1a03002 	mov	r3, r2
{
	return mdiobus_write(phydev->mdio.bus, phydev->mdio.addr, regnum, val);
      88:	e1a02001 	mov	r2, r1
      8c:	e5901148 	ldr	r1, [r0, #328]	@ 0x148
      90:	e5900118 	ldr	r0, [r0, #280]	@ 0x118
      94:	eafffffe 	b	0 <mdiobus_write>

00000098 <nss_gmac_check_link>:
	/*
	 * Unlike the bitops with the '__' prefix above, this one *is* atomic,
	 * so `volatile` must always stay here with no cast-aways. See
	 * `Documentation/atomic_bitops.txt` for the details.
	 */
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
      98:	e5903018 	ldr	r3, [r0, #24]
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags))
      9c:	e3130040 	tst	r3, #64	@ 0x40
	if (phydev->link)
      a0:	15903290 	ldrne	r3, [r0, #656]	@ 0x290
      a4:	15d301f1 	ldrbne	r0, [r3, #497]	@ 0x1f1
      a8:	17e002d0 	ubfxne	r0, r0, #5, #1
		return LINKUP;
      ac:	03a00001 	moveq	r0, #1
}
      b0:	e12fff1e 	bx	lr

000000b4 <nss_gmac_read_phy_reg>:
{
      b4:	e92d4070 	push	{r4, r5, r6, lr}
	addr = ((phy_base << gmii_dev_shift) & gmii_dev_mask)
      b8:	e1a01581 	lsl	r1, r1, #11
{
      bc:	e1a05003 	mov	r5, r3
	addr = ((phy_base << gmii_dev_shift) & gmii_dev_mask)
      c0:	e59d3010 	ldr	r3, [sp, #16]
      c4:	e6ff1071 	uxth	r1, r1
	    | (((uint32_t)reg_offset << gmii_reg_shift) & gmii_reg_mask)
      c8:	e1a02302 	lsl	r2, r2, #6
      cc:	e2022d1f 	and	r2, r2, #1984	@ 0x7c0
{
      d0:	e1a04000 	mov	r4, r0
	addr = ((phy_base << gmii_dev_shift) & gmii_dev_mask)
      d4:	e1811003 	orr	r1, r1, r3
      d8:	e1811002 	orr	r1, r1, r2
	addr = addr | gmii_busy;
      dc:	e3811001 	orr	r1, r1, #1
	asm volatile("str %1, %0"
      e0:	e5801010 	str	r1, [r0, #16]
	for (loop_variable = 0; loop_variable
      e4:	e3a0600b 	mov	r6, #11
      e8:	ea000007 	b	10c <nss_gmac_read_phy_reg+0x58>
	asm volatile("ldr %0, %1"
      ec:	e5940010 	ldr	r0, [r4, #16]
		if (!(temp & gmii_busy)) {
      f0:	e2100001 	ands	r0, r0, #1
      f4:	1a000002 	bne	104 <nss_gmac_read_phy_reg+0x50>
      f8:	e5943014 	ldr	r3, [r4, #20]
				(uint16_t)(nss_gmac_read_reg(reg_base,
      fc:	e1c530b0 	strh	r3, [r5]
			return 0;
     100:	e8bd8070 	pop	{r4, r5, r6, pc}
		msleep(100);
     104:	e3a00064 	mov	r0, #100	@ 0x64
     108:	ebfffffe 	bl	0 <msleep>
	     < DEFAULT_LOOP_VARIABLE; loop_variable++) {
     10c:	e2566001 	subs	r6, r6, #1
     110:	1afffff5 	bne	ec <nss_gmac_read_phy_reg+0x38>
	return -EIO;
     114:	e3e00004 	mvn	r0, #4
}
     118:	e8bd8070 	pop	{r4, r5, r6, pc}

0000011c <nss_gmac_write_phy_reg>:
{
     11c:	e92d4070 	push	{r4, r5, r6, lr}
     120:	e1a04000 	mov	r4, r0
	asm volatile("str %1, %0"
     124:	e5803014 	str	r3, [r0, #20]
	    | gmii_write | mdc_clk_div;
     128:	e59d3010 	ldr	r3, [sp, #16]
	addr = ((phy_base << gmii_dev_shift) & gmii_dev_mask)
     12c:	e1a01581 	lsl	r1, r1, #11
	    | ((reg_offset << gmii_reg_shift) & gmii_reg_mask)
     130:	e1a02302 	lsl	r2, r2, #6
	addr = ((phy_base << gmii_dev_shift) & gmii_dev_mask)
     134:	e6ff1071 	uxth	r1, r1
	    | ((reg_offset << gmii_reg_shift) & gmii_reg_mask)
     138:	e2022d1f 	and	r2, r2, #1984	@ 0x7c0
	    | gmii_write | mdc_clk_div;
     13c:	e1811003 	orr	r1, r1, r3
     140:	e1811002 	orr	r1, r1, r2
	addr = addr | gmii_busy;
     144:	e3811003 	orr	r1, r1, #3
     148:	e5801010 	str	r1, [r0, #16]
	for (loop_variable = 0; loop_variable
     14c:	e3a0500b 	mov	r5, #11
     150:	ea000004 	b	168 <nss_gmac_write_phy_reg+0x4c>
	asm volatile("ldr %0, %1"
     154:	e5940010 	ldr	r0, [r4, #16]
		if (!(temp & gmii_busy))
     158:	e2100001 	ands	r0, r0, #1
     15c:	08bd8070 	popeq	{r4, r5, r6, pc}
		msleep(100);
     160:	e3a00064 	mov	r0, #100	@ 0x64
     164:	ebfffffe 	bl	0 <msleep>
	     < DEFAULT_LOOP_VARIABLE; loop_variable++) {
     168:	e2555001 	subs	r5, r5, #1
     16c:	1afffff8 	bne	154 <nss_gmac_write_phy_reg+0x38>
	return -EIO;
     170:	e3e00004 	mvn	r0, #4
}
     174:	e8bd8070 	pop	{r4, r5, r6, pc}

00000178 <nss_gmac_mii_rd_reg>:
{
     178:	e92d4010 	push	{r4, lr}
	if (IS_ERR(gmacdev->phydev)) {
     17c:	e5903290 	ldr	r3, [r0, #656]	@ 0x290
     180:	e3730a01 	cmn	r3, #4096	@ 0x1000
     184:	9a000004 	bls	19c <nss_gmac_mii_rd_reg+0x24>
		netdev_info(gmacdev->netdev, "Error: Reading uninitialized PHY...\n");
     188:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
     18c:	e59f101c 	ldr	r1, [pc, #28]	@ 1b0 <nss_gmac_mii_rd_reg+0x38>
     190:	ebfffffe 	bl	0 <netdev_info>
     194:	e3a00000 	mov	r0, #0
     198:	e8bd8010 	pop	{r4, pc}
	return mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, regnum);
     19c:	e5930118 	ldr	r0, [r3, #280]	@ 0x118
     1a0:	e5931148 	ldr	r1, [r3, #328]	@ 0x148
     1a4:	ebfffffe 	bl	0 <mdiobus_read>
	data = (uint16_t)phy_read(gmacdev->phydev, reg);
     1a8:	e6ff0070 	uxth	r0, r0
}
     1ac:	e8bd8010 	pop	{r4, pc}
     1b0:	00000000 	.word	0x00000000

000001b4 <nss_gmac_mii_wr_reg>:
{
     1b4:	e1a0c000 	mov	ip, r0
	if (IS_ERR(gmacdev->phydev))
     1b8:	e5900290 	ldr	r0, [r0, #656]	@ 0x290
{
     1bc:	e1a01002 	mov	r1, r2
	if (IS_ERR(gmacdev->phydev))
     1c0:	e3700a01 	cmn	r0, #4096	@ 0x1000
     1c4:	9a000002 	bls	1d4 <nss_gmac_mii_wr_reg+0x20>
		netdev_info(gmacdev->netdev, "Error: Writing uninitialized PHY...\n");
     1c8:	e59c0090 	ldr	r0, [ip, #144]	@ 0x90
     1cc:	e59f1008 	ldr	r1, [pc, #8]	@ 1dc <nss_gmac_mii_wr_reg+0x28>
     1d0:	eafffffe 	b	0 <netdev_info>
		phy_write(gmacdev->phydev, reg, data);
     1d4:	e1a02003 	mov	r2, r3
     1d8:	eaffffa9 	b	84 <phy_write.isra.0>
     1dc:	00000025 	.word	0x00000025

000001e0 <nss_gmac_reset_phy>:
{
     1e0:	e92d4070 	push	{r4, r5, r6, lr}
	nss_gmac_mii_wr_reg(gmacdev, phyid, MII_BMCR, BMCR_RESET);
     1e4:	e3a03902 	mov	r3, #32768	@ 0x8000
{
     1e8:	e1a04000 	mov	r4, r0
     1ec:	e1a05001 	mov	r5, r1
	nss_gmac_mii_wr_reg(gmacdev, phyid, MII_BMCR, BMCR_RESET);
     1f0:	e3a02000 	mov	r2, #0
     1f4:	ebfffffe 	bl	1b4 <nss_gmac_mii_wr_reg>
			    nss_gmac_mii_rd_reg(gmacdev, phyid, MII_BMCR)
     1f8:	e1a01005 	mov	r1, r5
     1fc:	e3a02000 	mov	r2, #0
     200:	e1a00004 	mov	r0, r4
     204:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
	nss_gmac_mii_wr_reg(gmacdev, phyid, MII_BMCR,
     208:	e3803a01 	orr	r3, r0, #4096	@ 0x1000
     20c:	e3a02000 	mov	r2, #0
     210:	e1a01005 	mov	r1, r5
     214:	e1a00004 	mov	r0, r4
     218:	e6ff3073 	uxth	r3, r3
     21c:	ebfffffe 	bl	1b4 <nss_gmac_mii_wr_reg>
	set_bit(__NSS_GMAC_AUTONEG, &gmacdev->flags);
     220:	e2841018 	add	r1, r4, #24
     224:	e3a00003 	mov	r0, #3
     228:	ebfffffe 	bl	0 <_set_bit>
	netdev_info(gmacdev->netdev, "Phy %u reset OK\n", phyid);
     22c:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     230:	e1a02005 	mov	r2, r5
     234:	e59f1004 	ldr	r1, [pc, #4]	@ 240 <nss_gmac_reset_phy+0x60>
}
     238:	e8bd4070 	pop	{r4, r5, r6, lr}
	netdev_info(gmacdev->netdev, "Phy %u reset OK\n", phyid);
     23c:	eafffffe 	b	0 <netdev_info>
     240:	0000004a 	.word	0x0000004a

00000244 <nss_gmac_read_version>:
					     uint32_t regoffset)
{
	uint32_t addr = 0;
	uint32_t data;

	addr = (uint32_t)regbase + regoffset;
     244:	e5903000 	ldr	r3, [r0]
     248:	e5933020 	ldr	r3, [r3, #32]
	gmacdev->version = data;
     24c:	e5803014 	str	r3, [r0, #20]
}
     250:	e3a00000 	mov	r0, #0
     254:	e12fff1e 	bx	lr

00000258 <nss_gmac_dma_bus_mode_init>:
	nss_gmac_write_reg(gmacdev->dma_base, dma_bus_mode, init_value);
     258:	e5903004 	ldr	r3, [r0, #4]
	asm volatile("str %1, %0"
     25c:	e5831000 	str	r1, [r3]
}
     260:	e3a00000 	mov	r0, #0
     264:	e12fff1e 	bx	lr

00000268 <nss_gmac_dma_axi_bus_mode_init>:
					  uint32_t regoffset,
					  uint32_t regdata)
{
	uint32_t addr = 0;

	addr = (uint32_t)regbase + regoffset;
     268:	e5903004 	ldr	r3, [r0, #4]
     26c:	e5831028 	str	r1, [r3, #40]	@ 0x28
}
     270:	e3a00000 	mov	r0, #0
     274:	e12fff1e 	bx	lr

00000278 <nss_gmac_dma_control_init>:
     278:	e5903004 	ldr	r3, [r0, #4]
     27c:	e5831018 	str	r1, [r3, #24]
}
     280:	e3a00000 	mov	r0, #0
     284:	e12fff1e 	bx	lr

00000288 <nss_gmac_wd_enable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     288:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     28c:	e5923000 	ldr	r3, [r2]
					       uint32_t regoffset,
					       uint32_t bitpos)
{
	uint32_t data = 0;

	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     290:	e3c33502 	bic	r3, r3, #8388608	@ 0x800000
	asm volatile("str %1, %0"
     294:	e5823000 	str	r3, [r2]
}
     298:	e12fff1e 	bx	lr

0000029c <nss_gmac_jab_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     29c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     2a0:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     2a4:	e3833501 	orr	r3, r3, #4194304	@ 0x400000
	asm volatile("str %1, %0"
     2a8:	e5823000 	str	r3, [r2]
}
     2ac:	e12fff1e 	bx	lr

000002b0 <nss_gmac_frame_burst_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     2b0:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     2b4:	e5923000 	ldr	r3, [r2]
     2b8:	e3833602 	orr	r3, r3, #2097152	@ 0x200000
	asm volatile("str %1, %0"
     2bc:	e5823000 	str	r3, [r2]
}
     2c0:	e12fff1e 	bx	lr

000002c4 <nss_gmac_jumbo_frame_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     2c4:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     2c8:	e5923000 	ldr	r3, [r2]
     2cc:	e3833601 	orr	r3, r3, #1048576	@ 0x100000
	asm volatile("str %1, %0"
     2d0:	e5823000 	str	r3, [r2]
}
     2d4:	e12fff1e 	bx	lr

000002d8 <nss_gmac_jumbo_frame_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     2d8:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     2dc:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     2e0:	e3c33601 	bic	r3, r3, #1048576	@ 0x100000
	asm volatile("str %1, %0"
     2e4:	e5823000 	str	r3, [r2]
}
     2e8:	e12fff1e 	bx	lr

000002ec <nss_gmac_twokpe_frame_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     2ec:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     2f0:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     2f4:	e3833302 	orr	r3, r3, #134217728	@ 0x8000000
	asm volatile("str %1, %0"
     2f8:	e5823000 	str	r3, [r2]
}
     2fc:	e12fff1e 	bx	lr

00000300 <nss_gmac_twokpe_frame_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     300:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     304:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     308:	e3c33302 	bic	r3, r3, #134217728	@ 0x8000000
	asm volatile("str %1, %0"
     30c:	e5823000 	str	r3, [r2]
}
     310:	e12fff1e 	bx	lr

00000314 <nss_gmac_disable_crs>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     314:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     318:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     31c:	e3833801 	orr	r3, r3, #65536	@ 0x10000
	asm volatile("str %1, %0"
     320:	e5823000 	str	r3, [r2]
}
     324:	e12fff1e 	bx	lr

00000328 <nss_gmac_enable_crs>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     328:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     32c:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     330:	e3c33801 	bic	r3, r3, #65536	@ 0x10000
	asm volatile("str %1, %0"
     334:	e5823000 	str	r3, [r2]
}
     338:	e12fff1e 	bx	lr

0000033c <nss_gmac_select_gmii>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     33c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     340:	e5923000 	ldr	r3, [r2]
     344:	e3c33902 	bic	r3, r3, #32768	@ 0x8000
	asm volatile("str %1, %0"
     348:	e5823000 	str	r3, [r2]
}
     34c:	e12fff1e 	bx	lr

00000350 <nss_gmac_select_mii>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     350:	e5903000 	ldr	r3, [r0]
	asm volatile("ldr %0, %1"
     354:	e5932000 	ldr	r2, [r3]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     358:	e3822902 	orr	r2, r2, #32768	@ 0x8000
	asm volatile("str %1, %0"
     35c:	e5832000 	str	r2, [r3]
	if (gmacdev->speed == SPEED_100) {
     360:	e590206c 	ldr	r2, [r0, #108]	@ 0x6c
     364:	e3520064 	cmp	r2, #100	@ 0x64
	asm volatile("ldr %0, %1"
     368:	e5932000 	ldr	r2, [r3]
     36c:	03822901 	orreq	r2, r2, #16384	@ 0x4000
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     370:	13c22901 	bicne	r2, r2, #16384	@ 0x4000
	asm volatile("str %1, %0"
     374:	e5832000 	str	r2, [r3]
}
     378:	e12fff1e 	bx	lr

0000037c <nss_gmac_rx_own_enable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     37c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     380:	e5923000 	ldr	r3, [r2]
     384:	e3c33a02 	bic	r3, r3, #8192	@ 0x2000
	asm volatile("str %1, %0"
     388:	e5823000 	str	r3, [r2]
}
     38c:	e12fff1e 	bx	lr

00000390 <nss_gmac_rx_own_disable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     390:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     394:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     398:	e3833a02 	orr	r3, r3, #8192	@ 0x2000
	asm volatile("str %1, %0"
     39c:	e5823000 	str	r3, [r2]
}
     3a0:	e12fff1e 	bx	lr

000003a4 <nss_gmac_loopback_off>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     3a4:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     3a8:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     3ac:	e3c33a01 	bic	r3, r3, #4096	@ 0x1000
	asm volatile("str %1, %0"
     3b0:	e5823000 	str	r3, [r2]
}
     3b4:	e12fff1e 	bx	lr

000003b8 <nss_gmac_set_full_duplex>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     3b8:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     3bc:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     3c0:	e3833b02 	orr	r3, r3, #2048	@ 0x800
	asm volatile("str %1, %0"
     3c4:	e5823000 	str	r3, [r2]
}
     3c8:	e12fff1e 	bx	lr

000003cc <nss_gmac_set_half_duplex>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     3cc:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     3d0:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     3d4:	e3c33b02 	bic	r3, r3, #2048	@ 0x800
	asm volatile("str %1, %0"
     3d8:	e5823000 	str	r3, [r2]
}
     3dc:	e12fff1e 	bx	lr

000003e0 <nss_gmac_retry_enable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     3e0:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     3e4:	e5923000 	ldr	r3, [r2]
     3e8:	e3c33c02 	bic	r3, r3, #512	@ 0x200
	asm volatile("str %1, %0"
     3ec:	e5823000 	str	r3, [r2]
}
     3f0:	e12fff1e 	bx	lr

000003f4 <nss_gmac_retry_disable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     3f4:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     3f8:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     3fc:	e3833c02 	orr	r3, r3, #512	@ 0x200
	asm volatile("str %1, %0"
     400:	e5823000 	str	r3, [r2]
}
     404:	e12fff1e 	bx	lr

00000408 <nss_gmac_pad_crc_strip_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     408:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     40c:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     410:	e3c33080 	bic	r3, r3, #128	@ 0x80
	asm volatile("str %1, %0"
     414:	e5823000 	str	r3, [r2]
}
     418:	e12fff1e 	bx	lr

0000041c <nss_gmac_back_off_limit>:
	data = nss_gmac_read_reg(gmacdev->mac_base, gmac_config);
     41c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     420:	e5923000 	ldr	r3, [r2]
	data &= (~gmac_backoff_limit);
     424:	e3c33060 	bic	r3, r3, #96	@ 0x60
	data |= value;
     428:	e1833001 	orr	r3, r3, r1
	asm volatile("str %1, %0"
     42c:	e5823000 	str	r3, [r2]
}
     430:	e12fff1e 	bx	lr

00000434 <nss_gmac_deferral_check_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     434:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     438:	e5923000 	ldr	r3, [r2]
     43c:	e3c33010 	bic	r3, r3, #16
	asm volatile("str %1, %0"
     440:	e5823000 	str	r3, [r2]
}
     444:	e12fff1e 	bx	lr

00000448 <nss_gmac_rx_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base, gmac_config,
     448:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     44c:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     450:	e3833004 	orr	r3, r3, #4
	asm volatile("str %1, %0"
     454:	e5823000 	str	r3, [r2]
}
     458:	e12fff1e 	bx	lr

0000045c <nss_gmac_rx_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     45c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     460:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     464:	e3c33004 	bic	r3, r3, #4
	asm volatile("str %1, %0"
     468:	e5823000 	str	r3, [r2]
}
     46c:	e12fff1e 	bx	lr

00000470 <nss_gmac_tx_enable>:
	nss_gmac_set_reg_bits(gmacdev->mac_base, gmac_config,
     470:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     474:	e5923000 	ldr	r3, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     478:	e3833008 	orr	r3, r3, #8
	asm volatile("str %1, %0"
     47c:	e5823000 	str	r3, [r2]
}
     480:	e12fff1e 	bx	lr

00000484 <nss_gmac_tx_disable>:
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     484:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     488:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     48c:	e3c33008 	bic	r3, r3, #8
	asm volatile("str %1, %0"
     490:	e5823000 	str	r3, [r2]
}
     494:	e12fff1e 	bx	lr

00000498 <nss_gmac_frame_filter_enable>:
	addr = (uint32_t)regbase + regoffset;
     498:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     49c:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     4a0:	e3c33102 	bic	r3, r3, #-2147483648	@ 0x80000000
	asm volatile("str %1, %0"
     4a4:	e5823004 	str	r3, [r2, #4]
}
     4a8:	e12fff1e 	bx	lr

000004ac <nss_gmac_src_addr_filter_disable>:
	addr = (uint32_t)regbase + regoffset;
     4ac:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     4b0:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     4b4:	e3c33c02 	bic	r3, r3, #512	@ 0x200
	asm volatile("str %1, %0"
     4b8:	e5823004 	str	r3, [r2, #4]
}
     4bc:	e12fff1e 	bx	lr

000004c0 <nss_gmac_dst_addr_filter_normal>:
	addr = (uint32_t)regbase + regoffset;
     4c0:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     4c4:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     4c8:	e3c33008 	bic	r3, r3, #8
	asm volatile("str %1, %0"
     4cc:	e5823004 	str	r3, [r2, #4]
}
     4d0:	e12fff1e 	bx	lr

000004d4 <nss_gmac_set_pass_control>:
	addr = (uint32_t)regbase + regoffset;
     4d4:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     4d8:	e5923004 	ldr	r3, [r2, #4]
	data &= (~gmac_pass_control);
     4dc:	e3c330c0 	bic	r3, r3, #192	@ 0xc0
	data |= passcontrol;
     4e0:	e1833001 	orr	r3, r3, r1
	asm volatile("str %1, %0"
     4e4:	e5823004 	str	r3, [r2, #4]
}
     4e8:	e12fff1e 	bx	lr

000004ec <nss_gmac_broadcast_enable>:
     4ec:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     4f0:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     4f4:	e3c33020 	bic	r3, r3, #32
	asm volatile("str %1, %0"
     4f8:	e5823004 	str	r3, [r2, #4]
}
     4fc:	e12fff1e 	bx	lr

00000500 <nss_gmac_multicast_enable>:
	addr = (uint32_t)regbase + regoffset;
     500:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     504:	e5923004 	ldr	r3, [r2, #4]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     508:	e3833010 	orr	r3, r3, #16
	asm volatile("str %1, %0"
     50c:	e5823004 	str	r3, [r2, #4]
}
     510:	e12fff1e 	bx	lr

00000514 <nss_gmac_multicast_disable>:
	addr = (uint32_t)regbase + regoffset;
     514:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     518:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     51c:	e3c33010 	bic	r3, r3, #16
	asm volatile("str %1, %0"
     520:	e5823004 	str	r3, [r2, #4]
}
     524:	e12fff1e 	bx	lr

00000528 <nss_gmac_multicast_hash_filter_disable>:
	addr = (uint32_t)regbase + regoffset;
     528:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     52c:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     530:	e3c33004 	bic	r3, r3, #4
	asm volatile("str %1, %0"
     534:	e5823004 	str	r3, [r2, #4]
}
     538:	e12fff1e 	bx	lr

0000053c <nss_gmac_promisc_enable>:
	addr = (uint32_t)regbase + regoffset;
     53c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     540:	e5923004 	ldr	r3, [r2, #4]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     544:	e3833001 	orr	r3, r3, #1
	asm volatile("str %1, %0"
     548:	e5823004 	str	r3, [r2, #4]
}
     54c:	e12fff1e 	bx	lr

00000550 <nss_gmac_promisc_disable>:
	addr = (uint32_t)regbase + regoffset;
     550:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     554:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     558:	e3c33001 	bic	r3, r3, #1
	asm volatile("str %1, %0"
     55c:	e5823004 	str	r3, [r2, #4]
}
     560:	e12fff1e 	bx	lr

00000564 <nss_gmac_unicast_hash_filter_disable>:
	addr = (uint32_t)regbase + regoffset;
     564:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     568:	e5923004 	ldr	r3, [r2, #4]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     56c:	e3c33002 	bic	r3, r3, #2
	asm volatile("str %1, %0"
     570:	e5823004 	str	r3, [r2, #4]
}
     574:	e12fff1e 	bx	lr

00000578 <nss_gmac_unicast_pause_frame_detect_disable>:
	addr = (uint32_t)regbase + regoffset;
     578:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     57c:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     580:	e3c33008 	bic	r3, r3, #8
	asm volatile("str %1, %0"
     584:	e5823018 	str	r3, [r2, #24]
}
     588:	e12fff1e 	bx	lr

0000058c <nss_gmac_rx_flow_control_disable>:
	addr = (uint32_t)regbase + regoffset;
     58c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     590:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     594:	e3c33004 	bic	r3, r3, #4
	asm volatile("str %1, %0"
     598:	e5823018 	str	r3, [r2, #24]
}
     59c:	e12fff1e 	bx	lr

000005a0 <nss_gmac_tx_flow_control_disable>:
	addr = (uint32_t)regbase + regoffset;
     5a0:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     5a4:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     5a8:	e3c33002 	bic	r3, r3, #2
	asm volatile("str %1, %0"
     5ac:	e5823018 	str	r3, [r2, #24]
}
     5b0:	e12fff1e 	bx	lr

000005b4 <nss_gmac_ts_enable>:
{
     5b4:	e92d4070 	push	{r4, r5, r6, lr}
     5b8:	e1a04000 	mov	r4, r0
	addr = (uint32_t)regbase + regoffset;
     5bc:	e5903000 	ldr	r3, [r0]
	asm volatile("ldr %0, %1"
     5c0:	e593203c 	ldr	r2, [r3, #60]	@ 0x3c
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     5c4:	e3822c02 	orr	r2, r2, #512	@ 0x200
	asm volatile("str %1, %0"
     5c8:	e583203c 	str	r2, [r3, #60]	@ 0x3c
	asm volatile("ldr %0, %1"
     5cc:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
     5d0:	e3822001 	orr	r2, r2, #1
	asm volatile("str %1, %0"
     5d4:	e5832700 	str	r2, [r3, #1792]	@ 0x700
	if (gmacdev->aux_clk_freq) {
     5d8:	e590108c 	ldr	r1, [r0, #140]	@ 0x8c
     5dc:	e3510000 	cmp	r1, #0
     5e0:	0a000007 	beq	604 <nss_gmac_ts_enable+0x50>
	uint32_t *nss_base = (uint32_t *)(gmacdev->ctx->nss_base);
     5e4:	e5902284 	ldr	r2, [r0, #644]	@ 0x284
	addr = (uint32_t)regbase + regoffset;
     5e8:	e5920008 	ldr	r0, [r2, #8]
	asm volatile("ldr %0, %1"
     5ec:	e5902014 	ldr	r2, [r0, #20]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     5f0:	e3822102 	orr	r2, r2, #-2147483648	@ 0x80000000
	asm volatile("str %1, %0"
     5f4:	e5802014 	str	r2, [r0, #20]
		nss_gmac_write_reg(gmacdev->mac_base, gmac_ts_sub_sec_incr, (BILLION / gmacdev->aux_clk_freq));
     5f8:	e59f20cc 	ldr	r2, [pc, #204]	@ 6cc <nss_gmac_ts_enable+0x118>
     5fc:	e732f112 	udiv	r2, r2, r1
     600:	e5832704 	str	r2, [r3, #1796]	@ 0x704
     604:	e3a0500a 	mov	r5, #10
     608:	e5835704 	str	r5, [r3, #1796]	@ 0x704
	asm volatile("ldr %0, %1"
     60c:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     610:	e3c22002 	bic	r2, r2, #2
	asm volatile("str %1, %0"
     614:	e5832700 	str	r2, [r3, #1792]	@ 0x700
     618:	e3a02000 	mov	r2, #0
     61c:	e5832710 	str	r2, [r3, #1808]	@ 0x710
     620:	e5832714 	str	r2, [r3, #1812]	@ 0x714
	asm volatile("ldr %0, %1"
     624:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     628:	e3822c02 	orr	r2, r2, #512	@ 0x200
	asm volatile("str %1, %0"
     62c:	e5832700 	str	r2, [r3, #1792]	@ 0x700
	asm volatile("ldr %0, %1"
     630:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
     634:	e3822a01 	orr	r2, r2, #4096	@ 0x1000
	asm volatile("str %1, %0"
     638:	e5832700 	str	r2, [r3, #1792]	@ 0x700
	asm volatile("ldr %0, %1"
     63c:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
     640:	e3822c01 	orr	r2, r2, #256	@ 0x100
	asm volatile("str %1, %0"
     644:	e5832700 	str	r2, [r3, #1792]	@ 0x700
	addr = (uint32_t)regbase + regoffset;
     648:	e5943000 	ldr	r3, [r4]
		timeout--;
     64c:	e2455001 	sub	r5, r5, #1
	asm volatile("ldr %0, %1"
     650:	e5936700 	ldr	r6, [r3, #1792]	@ 0x700
void usleep_range_state(unsigned long min, unsigned long max,
			unsigned int state);

static inline void usleep_range(unsigned long min, unsigned long max)
{
	usleep_range_state(min, max, TASK_UNINTERRUPTIBLE);
     654:	e3a02002 	mov	r2, #2
     658:	e3a01014 	mov	r1, #20
     65c:	e3a0000a 	mov	r0, #10
     660:	ebfffffe 	bl	0 <usleep_range_state>
	} while ((data & gmac_ts_init_mask) && timeout > 0);
     664:	e3160004 	tst	r6, #4
     668:	0a000001 	beq	674 <nss_gmac_ts_enable+0xc0>
     66c:	e3550000 	cmp	r5, #0
     670:	1afffff4 	bne	648 <nss_gmac_ts_enable+0x94>
	if (timeout == 0) {
     674:	e3550000 	cmp	r5, #0
		netdev_info(gmacdev->netdev, "%s: Timestamp enable timed out\n", __func__);
     678:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     67c:	e59f204c 	ldr	r2, [pc, #76]	@ 6d0 <nss_gmac_ts_enable+0x11c>
	if (timeout == 0) {
     680:	1a000003 	bne	694 <nss_gmac_ts_enable+0xe0>
		netdev_info(gmacdev->netdev, "%s: Timestamp enable timed out\n", __func__);
     684:	e59f1048 	ldr	r1, [pc, #72]	@ 6d4 <nss_gmac_ts_enable+0x120>
     688:	ebfffffe 	bl	0 <netdev_info>
		return -1;
     68c:	e3e00000 	mvn	r0, #0
     690:	e8bd8070 	pop	{r4, r5, r6, pc}
	netdev_info(gmacdev->netdev, "%s: Timestamp enabled\n", __func__);
     694:	e59f103c 	ldr	r1, [pc, #60]	@ 6d8 <nss_gmac_ts_enable+0x124>
     698:	ebfffffe 	bl	0 <netdev_info>
     69c:	e5942000 	ldr	r2, [r4]
     6a0:	e5923700 	ldr	r3, [r2, #1792]	@ 0x700
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     6a4:	e3833004 	orr	r3, r3, #4
	asm volatile("str %1, %0"
     6a8:	e5823700 	str	r3, [r2, #1792]	@ 0x700
	gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(TSTAMP);
     6ac:	e594301c 	ldr	r3, [r4, #28]
	set_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags);
     6b0:	e2841018 	add	r1, r4, #24
     6b4:	e3a00007 	mov	r0, #7
	gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(TSTAMP);
     6b8:	e3833002 	orr	r3, r3, #2
     6bc:	e584301c 	str	r3, [r4, #28]
	set_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags);
     6c0:	ebfffffe 	bl	0 <_set_bit>
	return 0;
     6c4:	e3a00000 	mov	r0, #0
};
     6c8:	e8bd8070 	pop	{r4, r5, r6, pc}
     6cc:	3b9aca00 	.word	0x3b9aca00
     6d0:	0000005b 	.word	0x0000005b
     6d4:	00000145 	.word	0x00000145
     6d8:	00000165 	.word	0x00000165

000006dc <nss_gmac_ts_disable>:
	addr = (uint32_t)regbase + regoffset;
     6dc:	e1a01000 	mov	r1, r0
     6e0:	e4912018 	ldr	r2, [r1], #24
	asm volatile("ldr %0, %1"
     6e4:	e5923700 	ldr	r3, [r2, #1792]	@ 0x700
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     6e8:	e3c33004 	bic	r3, r3, #4
	asm volatile("str %1, %0"
     6ec:	e5823700 	str	r3, [r2, #1792]	@ 0x700
	clear_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags);
     6f0:	e3a00007 	mov	r0, #7
     6f4:	eafffffe 	b	0 <_clear_bit>

000006f8 <nss_gmac_flush_tx_fifo>:
	addr = (uint32_t)regbase + regoffset;
     6f8:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     6fc:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     700:	e3833601 	orr	r3, r3, #1048576	@ 0x100000
	asm volatile("str %1, %0"
     704:	e5823018 	str	r3, [r2, #24]
}
     708:	e12fff1e 	bx	lr

0000070c <nss_gmac_mmc_stats_cor_enable>:
	addr = (uint32_t)regbase + regoffset;
     70c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     710:	e5923100 	ldr	r3, [r2, #256]	@ 0x100
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     714:	e3833004 	orr	r3, r3, #4
	asm volatile("str %1, %0"
     718:	e5823100 	str	r3, [r2, #256]	@ 0x100
}
     71c:	e12fff1e 	bx	lr

00000720 <nss_gmac_check_phy_init>:
 * @param[in] pointer to nss_gmac_dev.
 * @return 0 on success. If successful, it updates gmacdev->speed and
 *	   gmacdev->duplex_mode with current speed and duplex mode.
 */
int32_t nss_gmac_check_phy_init(struct nss_gmac_dev *gmacdev)
{
     720:	e92d4ff7 	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr}
     724:	e1a04000 	mov	r4, r0
     728:	e5903018 	ldr	r3, [r0, #24]
	uint32_t phy_link_duplex;

	/*
	 * We cannot have link polling off without forced speed configured!
	 */
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)
     72c:	e3130040 	tst	r3, #64	@ 0x40
     730:	1a000004 	bne	748 <nss_gmac_check_phy_init+0x28>
		&& (gmacdev->forced_speed == SPEED_UNKNOWN)) {
     734:	e5903084 	ldr	r3, [r0, #132]	@ 0x84
     738:	e3730001 	cmn	r3, #1
		netdev_info(gmacdev->netdev,
     73c:	059f225c 	ldreq	r2, [pc, #604]	@ 9a0 <nss_gmac_check_phy_init+0x280>
     740:	059f125c 	ldreq	r1, [pc, #604]	@ 9a4 <nss_gmac_check_phy_init+0x284>
     744:	0a000090 	beq	98c <nss_gmac_check_phy_init+0x26c>
     748:	e5945018 	ldr	r5, [r4, #24]

	/*
	 * First read the PHY speed/duplex if link polling
	 * is enabled
	 */
	if (test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
     74c:	e7e05355 	ubfx	r5, r5, #6, #1
     750:	e3550000 	cmp	r5, #0
     754:	0a000005 	beq	770 <nss_gmac_check_phy_init+0x50>
		phydev = gmacdev->phydev;
     758:	e5945290 	ldr	r5, [r4, #656]	@ 0x290
		if (gmacdev->phydev->is_c45 == false) {
     75c:	e5d531f0 	ldrb	r3, [r5, #496]	@ 0x1f0
     760:	e3130001 	tst	r3, #1
     764:	1a000001 	bne	770 <nss_gmac_check_phy_init+0x50>
			/*
			 * Read the speed/duplex for standard C22 PHYs
			 */
			genphy_read_status(phydev);
     768:	e1a00005 	mov	r0, r5
     76c:	ebfffffe 	bl	0 <genphy_read_status>

	/*
	 * Now find the GMAC speed. Check if we have a forced speed
	 * and duplex configured for the GMAC.
	 */
	if (gmacdev->forced_speed != SPEED_UNKNOWN) {
     770:	e5943084 	ldr	r3, [r4, #132]	@ 0x84
     774:	e3730001 	cmn	r3, #1
		/*
		 * Set the GMAC speed as configured
		 */
		gmacdev->speed = gmacdev->forced_speed;
     778:	1584306c 	strne	r3, [r4, #108]	@ 0x6c
		gmacdev->duplex_mode = gmacdev->forced_duplex;
     77c:	15943088 	ldrne	r3, [r4, #136]	@ 0x88
	if (gmacdev->forced_speed != SPEED_UNKNOWN) {
     780:	1a00006b 	bne	934 <nss_gmac_check_phy_init+0x214>

	/*
	 * Get the GMAC speed from the SGMII PCS for SGMII/QSGMII
	 * interfaces
	 */
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII
     784:	e5943074 	ldr	r3, [r4, #116]	@ 0x74
     788:	e3530013 	cmp	r3, #19
     78c:	13530004 	cmpne	r3, #4
     790:	1a000062 	bne	920 <nss_gmac_check_phy_init+0x200>
	id = gmacdev->sgmii_pcs_chanid;
     794:	e5949010 	ldr	r9, [r4, #16]
				PCS_MODE_CTL_CHn_AUTONEG_RESTART(id));
     798:	e3a08040 	mov	r8, #64	@ 0x40
     79c:	e3a0b002 	mov	fp, #2
	qsgmii_base = ctx->qsgmii_base;
     7a0:	e5943284 	ldr	r3, [r4, #644]	@ 0x284
     7a4:	e593600c 	ldr	r6, [r3, #12]
	previous_linkup_speed = gmacdev->speed;
     7a8:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
     7ac:	e58d3000 	str	r3, [sp]
				PCS_MODE_CTL_CHn_AUTONEG_RESTART(id));
     7b0:	e1a03189 	lsl	r3, r9, #3
     7b4:	e1a08318 	lsl	r8, r8, r3
     7b8:	e58d3004 	str	r3, [sp, #4]
     7bc:	ea000000 	b	7c4 <nss_gmac_check_phy_init+0xa4>
     7c0:	e3a0b001 	mov	fp, #1
	nss_gmac_check_pcs_status(gmacdev);
     7c4:	e1a00004 	mov	r0, r4
     7c8:	ebfffe0c 	bl	0 <nss_gmac_check_pcs_status>
	if (gmacdev->link_state == LINKDOWN) {
     7cc:	e5947064 	ldr	r7, [r4, #100]	@ 0x64
     7d0:	e3570000 	cmp	r7, #0
     7d4:	1a000004 	bne	7ec <nss_gmac_check_phy_init+0xcc>
		if (gmacdev->phydev->link) {
     7d8:	e5943290 	ldr	r3, [r4, #656]	@ 0x290
     7dc:	e5d331f1 	ldrb	r3, [r3, #497]	@ 0x1f1
     7e0:	e3130020 	tst	r3, #32
     7e4:	0a000066 	beq	984 <nss_gmac_check_phy_init+0x264>
			netdev_info(gmacdev->netdev, "SGMII PCS error. Resetting PHY using MDIO\n");
     7e8:	ea00002f 	b	8ac <nss_gmac_check_phy_init+0x18c>
	new_speed = gmacdev->speed;
     7ec:	e594a06c 	ldr	sl, [r4, #108]	@ 0x6c
	asm volatile("ldr %0, %1"
     7f0:	e5963068 	ldr	r3, [r6, #104]	@ 0x68
     7f4:	e1883003 	orr	r3, r8, r3
	asm volatile("str %1, %0"
     7f8:	e5863068 	str	r3, [r6, #104]	@ 0x68
	asm volatile("ldr %0, %1"
     7fc:	e5963068 	ldr	r3, [r6, #104]	@ 0x68
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     800:	e1c33008 	bic	r3, r3, r8
	asm volatile("str %1, %0"
     804:	e5863068 	str	r3, [r6, #104]	@ 0x68
	asm volatile("ldr %0, %1"
     808:	e5963084 	ldr	r3, [r6, #132]	@ 0x84
	timeout = 50;
     80c:	e3a07032 	mov	r7, #50	@ 0x32
	while (!(reg & PCS_CHn_AUTONEG_COMPLETE(id)) && timeout > 0) {
     810:	ea000005 	b	82c <nss_gmac_check_phy_init+0x10c>
     814:	e3a02002 	mov	r2, #2
     818:	e3021ee0 	movw	r1, #12000	@ 0x2ee0
     81c:	e3020710 	movw	r0, #10000	@ 0x2710
		timeout--;
     820:	e2477001 	sub	r7, r7, #1
     824:	ebfffffe 	bl	0 <usleep_range_state>
     828:	e5963084 	ldr	r3, [r6, #132]	@ 0x84
	while (!(reg & PCS_CHn_AUTONEG_COMPLETE(id)) && timeout > 0) {
     82c:	e1180003 	tst	r8, r3
     830:	1a000001 	bne	83c <nss_gmac_check_phy_init+0x11c>
     834:	e3570000 	cmp	r7, #0
     838:	1afffff5 	bne	814 <nss_gmac_check_phy_init+0xf4>
	if (timeout == 0) {
     83c:	e3570000 	cmp	r7, #0
		netdev_info(gmacdev->netdev, "%s: PCS ch %d autoneg timeout\n",
     840:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     844:	e1a03009 	mov	r3, r9
     848:	e59f2158 	ldr	r2, [pc, #344]	@ 9a8 <nss_gmac_check_phy_init+0x288>
	if (timeout == 0) {
     84c:	1a00000a 	bne	87c <nss_gmac_check_phy_init+0x15c>
		netdev_info(gmacdev->netdev, "%s: PCS ch %d autoneg timeout\n",
     850:	e59f1154 	ldr	r1, [pc, #340]	@ 9ac <nss_gmac_check_phy_init+0x28c>
     854:	ebfffffe 	bl	0 <netdev_info>
		if (timeout_count == 2) {
     858:	e35b0001 	cmp	fp, #1
     85c:	1affffd7 	bne	7c0 <nss_gmac_check_phy_init+0xa0>
			gmacdev->link_state = LINKDOWN;
     860:	e5847064 	str	r7, [r4, #100]	@ 0x64
     864:	e5963068 	ldr	r3, [r6, #104]	@ 0x68
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     868:	e59d1004 	ldr	r1, [sp, #4]
     86c:	e3a02010 	mov	r2, #16
     870:	e1833112 	orr	r3, r3, r2, lsl r1
	asm volatile("str %1, %0"
     874:	e5863068 	str	r3, [r6, #104]	@ 0x68
			return;
     878:	ea000041 	b	984 <nss_gmac_check_phy_init+0x264>
	netdev_info(gmacdev->netdev, "%s: PCS ch %d autoneg complete\n",
     87c:	e59f112c 	ldr	r1, [pc, #300]	@ 9b0 <nss_gmac_check_phy_init+0x290>
     880:	ebfffffe 	bl	0 <netdev_info>
	nss_gmac_check_pcs_status(gmacdev);
     884:	e1a00004 	mov	r0, r4
     888:	ebfffddc 	bl	0 <nss_gmac_check_pcs_status>
	if ((gmacdev->link_state == LINKDOWN) || (new_speed != gmacdev->speed)) {
     88c:	e5943064 	ldr	r3, [r4, #100]	@ 0x64
     890:	e3530000 	cmp	r3, #0
     894:	0a000002 	beq	8a4 <nss_gmac_check_phy_init+0x184>
     898:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
     89c:	e15a0003 	cmp	sl, r3
     8a0:	0a000010 	beq	8e8 <nss_gmac_check_phy_init+0x1c8>
		gmacdev->link_state = LINKDOWN;
     8a4:	e3a07000 	mov	r7, #0
     8a8:	e5847064 	str	r7, [r4, #100]	@ 0x64
			netdev_info(gmacdev->netdev, "SGMII PCS error. Resetting PHY using MDIO\n");
     8ac:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     8b0:	e59f10fc 	ldr	r1, [pc, #252]	@ 9b4 <nss_gmac_check_phy_init+0x294>
     8b4:	ebfffffe 	bl	0 <netdev_info>
			phy_write(gmacdev->phydev, MII_BMCR,
     8b8:	e5946290 	ldr	r6, [r4, #656]	@ 0x290
     8bc:	e1a02007 	mov	r2, r7
     8c0:	e5960118 	ldr	r0, [r6, #280]	@ 0x118
     8c4:	e5961148 	ldr	r1, [r6, #328]	@ 0x148
     8c8:	ebfffffe 	bl	0 <mdiobus_read>
				BMCR_RESET | phy_read(gmacdev->phydev, MII_BMCR));
     8cc:	e1e02880 	mvn	r2, r0, lsl #17
			phy_write(gmacdev->phydev, MII_BMCR,
     8d0:	e1a01007 	mov	r1, r7
     8d4:	e1a00006 	mov	r0, r6
				BMCR_RESET | phy_read(gmacdev->phydev, MII_BMCR));
     8d8:	e1e028a2 	mvn	r2, r2, lsr #17
			phy_write(gmacdev->phydev, MII_BMCR,
     8dc:	e6ff2072 	uxth	r2, r2
     8e0:	ebfffde7 	bl	84 <phy_write.isra.0>
		return;
     8e4:	ea000005 	b	900 <nss_gmac_check_phy_init+0x1e0>
	if (previous_linkup_speed != gmacdev->speed) {
     8e8:	e59d3000 	ldr	r3, [sp]
     8ec:	e153000a 	cmp	r3, sl
     8f0:	0a000005 	beq	90c <nss_gmac_check_phy_init+0x1ec>
		nss_gmac_dev_set_speed(gmacdev);
     8f4:	ebfffffe 	bl	2f7c <nss_gmac_dev_set_speed>
		nss_gmac_flush_tx_fifo(gmacdev);
     8f8:	e1a00004 	mov	r0, r4
     8fc:	ebfffffe 	bl	6f8 <nss_gmac_flush_tx_fifo>
		|| gmacdev->phy_mii_type == PHY_INTERFACE_MODE_QSGMII) {
		nss_gmac_check_sgmii_link(gmacdev);
		if (gmacdev->link_state == LINKDOWN) {
     900:	e5943064 	ldr	r3, [r4, #100]	@ 0x64
     904:	e3530000 	cmp	r3, #0
     908:	0a00001d 	beq	984 <nss_gmac_check_phy_init+0x264>
			netdev_info(gmacdev->netdev, "%s: SGMII phy linkup ERROR.\n"
								, __func__);
			return -EIO;
		}

		netdev_info(gmacdev->netdev, "%s: SGMII phy linkup OK.\n",
     90c:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     910:	e59f2088 	ldr	r2, [pc, #136]	@ 9a0 <nss_gmac_check_phy_init+0x280>
     914:	e59f109c 	ldr	r1, [pc, #156]	@ 9b8 <nss_gmac_check_phy_init+0x298>
     918:	ebfffffe 	bl	0 <netdev_info>
								__func__);
		goto out;
     91c:	ea000005 	b	938 <nss_gmac_check_phy_init+0x218>

	/*
	 * Get the GMAC speed from the PHY for RGMII
	 * interfaces
	 */
	if (phydev) {
     920:	e3550000 	cmp	r5, #0
     924:	0a000003 	beq	938 <nss_gmac_check_phy_init+0x218>
		gmacdev->speed = phydev->speed;
     928:	e5953204 	ldr	r3, [r5, #516]	@ 0x204
     92c:	e584306c 	str	r3, [r4, #108]	@ 0x6c
		gmacdev->duplex_mode = phydev->duplex;
     930:	e5953208 	ldr	r3, [r5, #520]	@ 0x208
     934:	e5843068 	str	r3, [r4, #104]	@ 0x68
     938:	e5943018 	ldr	r3, [r4, #24]
	} else {
		phy_link_speed = gmacdev->speed;
		phy_link_duplex = gmacdev->duplex_mode;
	}

	netdev_info(gmacdev->netdev,
     93c:	e59f0078 	ldr	r0, [pc, #120]	@ 9bc <nss_gmac_check_phy_init+0x29c>
     940:	e7e03353 	ubfx	r3, r3, #6, #1
	if (test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags) && phydev) {
     944:	e3550000 	cmp	r5, #0
     948:	03a03000 	moveq	r3, #0
     94c:	12033001 	andne	r3, r3, #1
     950:	e3530000 	cmp	r3, #0
	netdev_info(gmacdev->netdev,
     954:	e59f3064 	ldr	r3, [pc, #100]	@ 9c0 <nss_gmac_check_phy_init+0x2a0>
		phy_link_duplex = phydev->duplex;
     958:	15951208 	ldrne	r1, [r5, #520]	@ 0x208
		phy_link_duplex = gmacdev->duplex_mode;
     95c:	05941068 	ldreq	r1, [r4, #104]	@ 0x68
		phy_link_speed = phydev->speed;
     960:	15952204 	ldrne	r2, [r5, #516]	@ 0x204
		phy_link_speed = gmacdev->speed;
     964:	0594206c 	ldreq	r2, [r4, #108]	@ 0x6c
	netdev_info(gmacdev->netdev,
     968:	e3510001 	cmp	r1, #1
     96c:	e59f1050 	ldr	r1, [pc, #80]	@ 9c4 <nss_gmac_check_phy_init+0x2a4>
     970:	11a03000 	movne	r3, r0
     974:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     978:	ebfffffe 	bl	0 <netdev_info>
			"%d Mbps %s Duplex\n",
			phy_link_speed, (phy_link_duplex == DUPLEX_FULL) ? "Full" : "Half");

	return 0;
     97c:	e3a00000 	mov	r0, #0
     980:	ea000004 	b	998 <nss_gmac_check_phy_init+0x278>
			netdev_info(gmacdev->netdev, "%s: SGMII phy linkup ERROR.\n"
     984:	e59f2014 	ldr	r2, [pc, #20]	@ 9a0 <nss_gmac_check_phy_init+0x280>
     988:	e59f1038 	ldr	r1, [pc, #56]	@ 9c8 <nss_gmac_check_phy_init+0x2a8>
     98c:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
     990:	ebfffffe 	bl	0 <netdev_info>
		return -EIO;
     994:	e3e00004 	mvn	r0, #4
}
     998:	e28dd00c 	add	sp, sp, #12
     99c:	e8bd8ff0 	pop	{r4, r5, r6, r7, r8, r9, sl, fp, pc}
     9a0:	000000a5 	.word	0x000000a5
     9a4:	0000023c 	.word	0x0000023c
     9a8:	000000bd 	.word	0x000000bd
     9ac:	000002a8 	.word	0x000002a8
     9b0:	000002c7 	.word	0x000002c7
     9b4:	0000027d 	.word	0x0000027d
     9b8:	000002e7 	.word	0x000002e7
     9bc:	00000237 	.word	0x00000237
     9c0:	00000232 	.word	0x00000232
     9c4:	00000301 	.word	0x00000301
     9c8:	00000314 	.word	0x00000314

000009cc <nss_gmac_ath_phy_mmd_wr>:
 * @return 0 on success
 */
int32_t nss_gmac_ath_phy_mmd_wr(struct phy_device *phydev,
			uint32_t mmd_dev_addr, uint32_t reg, uint16_t val)
{
	if (IS_ERR(phydev))
     9cc:	e3700a01 	cmn	r0, #4096	@ 0x1000
     9d0:	8a000015 	bhi	a2c <nss_gmac_ath_phy_mmd_wr+0x60>
{
     9d4:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
		return -EINVAL;

	phy_write(phydev, ATH_MII_MMD_ACCESS_CTRL, mmd_dev_addr);
     9d8:	e6ff5071 	uxth	r5, r1
     9dc:	e1a04000 	mov	r4, r0
     9e0:	e1a07002 	mov	r7, r2
     9e4:	e3a0100d 	mov	r1, #13
     9e8:	e1a02005 	mov	r2, r5
     9ec:	e1a06003 	mov	r6, r3
     9f0:	ebfffda3 	bl	84 <phy_write.isra.0>
	phy_write(phydev, ATH_MII_MMD_ACCESS_ADDR_DATA, reg);
     9f4:	e6ff2077 	uxth	r2, r7
     9f8:	e1a00004 	mov	r0, r4
     9fc:	e3a0100e 	mov	r1, #14
     a00:	ebfffd9f 	bl	84 <phy_write.isra.0>
	phy_write(phydev, ATH_MII_MMD_ACCESS_CTRL,
     a04:	e3852901 	orr	r2, r5, #16384	@ 0x4000
     a08:	e1a00004 	mov	r0, r4
     a0c:	e3a0100d 	mov	r1, #13
     a10:	ebfffd9b 	bl	84 <phy_write.isra.0>
		  ath_mmd_acc_ctrl_data_no_incr | mmd_dev_addr);
	phy_write(phydev, ATH_MII_MMD_ACCESS_ADDR_DATA, val);
     a14:	e1a00004 	mov	r0, r4
     a18:	e1a02006 	mov	r2, r6
     a1c:	e3a0100e 	mov	r1, #14
     a20:	ebfffd97 	bl	84 <phy_write.isra.0>

	return 0;
     a24:	e3a00000 	mov	r0, #0
}
     a28:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
		return -EINVAL;
     a2c:	e3e00015 	mvn	r0, #21
}
     a30:	e12fff1e 	bx	lr

00000a34 <nss_gmac_ath_phy_mmd_rd>:
 * @return -EINVAL on failure. Register value on success.
 */
int32_t nss_gmac_ath_phy_mmd_rd(struct phy_device *phydev,
			uint32_t mmd_dev_addr, uint32_t reg)
{
	if (IS_ERR(phydev))
     a34:	e3700a01 	cmn	r0, #4096	@ 0x1000
     a38:	8a000013 	bhi	a8c <nss_gmac_ath_phy_mmd_rd+0x58>
{
     a3c:	e92d4070 	push	{r4, r5, r6, lr}
		return -EINVAL;

	phy_write(phydev, ATH_MII_MMD_ACCESS_CTRL, mmd_dev_addr);
     a40:	e6ff5071 	uxth	r5, r1
     a44:	e1a04000 	mov	r4, r0
     a48:	e1a06002 	mov	r6, r2
     a4c:	e3a0100d 	mov	r1, #13
     a50:	e1a02005 	mov	r2, r5
     a54:	ebfffd8a 	bl	84 <phy_write.isra.0>
	phy_write(phydev, ATH_MII_MMD_ACCESS_ADDR_DATA, reg);
     a58:	e6ff2076 	uxth	r2, r6
     a5c:	e1a00004 	mov	r0, r4
     a60:	e3a0100e 	mov	r1, #14
     a64:	ebfffd86 	bl	84 <phy_write.isra.0>
	phy_write(phydev, ATH_MII_MMD_ACCESS_CTRL,
     a68:	e3852901 	orr	r2, r5, #16384	@ 0x4000
     a6c:	e1a00004 	mov	r0, r4
     a70:	e3a0100d 	mov	r1, #13
     a74:	ebfffd82 	bl	84 <phy_write.isra.0>
     a78:	e5940118 	ldr	r0, [r4, #280]	@ 0x118
     a7c:	e3a0200e 	mov	r2, #14
     a80:	e5941148 	ldr	r1, [r4, #328]	@ 0x148
		  ath_mmd_acc_ctrl_data_no_incr | mmd_dev_addr);
	return phy_read(phydev, ATH_MII_MMD_ACCESS_ADDR_DATA);
}
     a84:	e8bd4070 	pop	{r4, r5, r6, lr}
     a88:	eafffffe 	b	0 <mdiobus_read>
     a8c:	e3e00015 	mvn	r0, #21
     a90:	e12fff1e 	bx	lr

00000a94 <nss_gmac_ath_phy_disable_smart_802az>:
 */
int32_t nss_gmac_ath_phy_disable_smart_802az(struct phy_device *phydev)
{
	uint16_t val = 0;

	if (IS_ERR(phydev))
     a94:	e3700a01 	cmn	r0, #4096	@ 0x1000
     a98:	8a00000c 	bhi	ad0 <nss_gmac_ath_phy_disable_smart_802az+0x3c>
{
     a9c:	e92d4010 	push	{r4, lr}
		return -EINVAL;

	val = nss_gmac_ath_phy_mmd_rd(phydev, ATH_MMD_DEVADDR_3,
     aa0:	e308205d 	movw	r2, #32861	@ 0x805d
     aa4:	e3a01003 	mov	r1, #3
     aa8:	e1a04000 	mov	r4, r0
     aac:	ebfffffe 	bl	a34 <nss_gmac_ath_phy_mmd_rd>
					ath_mmd_smart_eee_ctrl_3);
	val &= ~ath_mmd_smart_eee_ctrl3_lpi_en;
     ab0:	e3c03c01 	bic	r3, r0, #256	@ 0x100
	nss_gmac_ath_phy_mmd_wr(phydev, ATH_MMD_DEVADDR_3,
     ab4:	e308205d 	movw	r2, #32861	@ 0x805d
     ab8:	e6ff3073 	uxth	r3, r3
     abc:	e3a01003 	mov	r1, #3
     ac0:	e1a00004 	mov	r0, r4
     ac4:	ebfffffe 	bl	9cc <nss_gmac_ath_phy_mmd_wr>
					ath_mmd_smart_eee_ctrl_3, val);

	return 0;
     ac8:	e3a00000 	mov	r0, #0
}
     acc:	e8bd8010 	pop	{r4, pc}
		return -EINVAL;
     ad0:	e3e00015 	mvn	r0, #21
}
     ad4:	e12fff1e 	bx	lr

00000ad8 <nss_gmac_ath_phy_disable_802az>:
 */
int32_t nss_gmac_ath_phy_disable_802az(struct phy_device *phydev)
{
	uint16_t val = 0;

	if (IS_ERR(phydev))
     ad8:	e3700a01 	cmn	r0, #4096	@ 0x1000
     adc:	8a00000c 	bhi	b14 <nss_gmac_ath_phy_disable_802az+0x3c>
{
     ae0:	e92d4010 	push	{r4, lr}
		return -EINVAL;

	val = nss_gmac_ath_phy_mmd_rd(phydev, ATH_MMD_DEVADDR_7,
     ae4:	e3a0203c 	mov	r2, #60	@ 0x3c
     ae8:	e3a01007 	mov	r1, #7
     aec:	e1a04000 	mov	r4, r0
     af0:	ebfffffe 	bl	a34 <nss_gmac_ath_phy_mmd_rd>
						ath_mmd_eee_adv);
	val &= ~(ath_mmd_eee_adv_100BT | ath_mmd_eee_adv_1000BT);
     af4:	e3c03006 	bic	r3, r0, #6
	nss_gmac_ath_phy_mmd_wr(phydev, ATH_MMD_DEVADDR_7,
     af8:	e3a0203c 	mov	r2, #60	@ 0x3c
     afc:	e6ff3073 	uxth	r3, r3
     b00:	e3a01007 	mov	r1, #7
     b04:	e1a00004 	mov	r0, r4
     b08:	ebfffffe 	bl	9cc <nss_gmac_ath_phy_mmd_wr>
					ath_mmd_eee_adv, val);

	return 0;
     b0c:	e3a00000 	mov	r0, #0
}
     b10:	e8bd8010 	pop	{r4, pc}
		return -EINVAL;
     b14:	e3e00015 	mvn	r0, #21
}
     b18:	e12fff1e 	bx	lr

00000b1c <nss_gmac_get_mac_addr>:
	addr = (uint32_t)regbase + regoffset;
     b1c:	e590c000 	ldr	ip, [r0]
     b20:	e08cc001 	add	ip, ip, r1
	asm volatile("ldr %0, %1"
     b24:	e59c1000 	ldr	r1, [ip]
			      uint32_t mac_low, uint8_t *mac_addr)
{
	uint32_t data;

	data = nss_gmac_read_reg(gmacdev->mac_base, mac_high);
	mac_addr[5] = (data >> 8) & 0xff;
     b28:	e1a0c421 	lsr	ip, r1, #8
	mac_addr[4] = (data) & 0xff;
     b2c:	e5c31004 	strb	r1, [r3, #4]
	mac_addr[5] = (data >> 8) & 0xff;
     b30:	e5c3c005 	strb	ip, [r3, #5]
     b34:	e5901000 	ldr	r1, [r0]
     b38:	e0811002 	add	r1, r1, r2
     b3c:	e5912000 	ldr	r2, [r1]

	data = nss_gmac_read_reg(gmacdev->mac_base, mac_low);
	mac_addr[3] = (data >> 24) & 0xff;
     b40:	e1a01c22 	lsr	r1, r2, #24
	mac_addr[2] = (data >> 16) & 0xff;
	mac_addr[1] = (data >> 8) & 0xff;
	mac_addr[0] = (data) & 0xff;
     b44:	e5c32000 	strb	r2, [r3]
	mac_addr[3] = (data >> 24) & 0xff;
     b48:	e5c31003 	strb	r1, [r3, #3]
	mac_addr[2] = (data >> 16) & 0xff;
     b4c:	e1a01822 	lsr	r1, r2, #16
     b50:	e5c31002 	strb	r1, [r3, #2]
	mac_addr[1] = (data >> 8) & 0xff;
     b54:	e1a01422 	lsr	r1, r2, #8
     b58:	e5c31001 	strb	r1, [r3, #1]
}
     b5c:	e12fff1e 	bx	lr

00000b60 <nss_gmac_attach>:
 * @return 0 upon success. Error code upon failure.
 * @note This is important function.
 */
int32_t nss_gmac_attach(struct nss_gmac_dev *gmacdev,
			uint32_t reg_base, uint32_t reglen)
{
     b60:	e92d40f7 	push	{r0, r1, r2, r4, r5, r6, r7, lr}
	struct net_device *netdev = NULL;
	netdev = gmacdev->netdev;

	/*Populate the mac and dma base addresses */
	gmacdev->memres = request_mem_region(reg_base, reglen, netdev->name);
     b64:	e3a04000 	mov	r4, #0
	netdev = gmacdev->netdev;
     b68:	e5906090 	ldr	r6, [r0, #144]	@ 0x90
{
     b6c:	e1a05000 	mov	r5, r0
     b70:	e1a07001 	mov	r7, r1
	gmacdev->memres = request_mem_region(reg_base, reglen, netdev->name);
     b74:	e59f0074 	ldr	r0, [pc, #116]	@ bf0 <nss_gmac_attach+0x90>
     b78:	e58d4000 	str	r4, [sp]
     b7c:	e1a03006 	mov	r3, r6
     b80:	ebfffffe 	bl	0 <__request_region>
	if (!gmacdev->memres) {
     b84:	e1500004 	cmp	r0, r4
	gmacdev->memres = request_mem_region(reg_base, reglen, netdev->name);
     b88:	e5850288 	str	r0, [r5, #648]	@ 0x288
		netdev_info(netdev, "Unable to request resource.\n");
     b8c:	059f1060 	ldreq	r1, [pc, #96]	@ bf4 <nss_gmac_attach+0x94>
	if (!gmacdev->memres) {
     b90:	0a000006 	beq	bb0 <nss_gmac_attach+0x50>
		return -EIO;
	}

	/* ioremap addresses */
	gmacdev->mac_base = ioremap(reg_base,
     b94:	e3a01901 	mov	r1, #16384	@ 0x4000
     b98:	e1a00007 	mov	r0, r7
     b9c:	ebfffffe 	bl	0 <ioremap>
						      NSS_GMAC_REG_BLOCK_LEN);
	if (!gmacdev->mac_base) {
     ba0:	e3500000 	cmp	r0, #0
	gmacdev->mac_base = ioremap(reg_base,
     ba4:	e5850000 	str	r0, [r5]
	if (!gmacdev->mac_base) {
     ba8:	1a000004 	bne	bc0 <nss_gmac_attach+0x60>
		netdev_info(netdev, "ioremap fail.\n");
     bac:	e59f1044 	ldr	r1, [pc, #68]	@ bf8 <nss_gmac_attach+0x98>
     bb0:	e1a00006 	mov	r0, r6
     bb4:	ebfffffe 	bl	0 <netdev_info>
		return -EIO;
     bb8:	e3e00004 	mvn	r0, #4
     bbc:	ea000009 	b	be8 <nss_gmac_attach+0x88>
		return -EIO;
	}

	netdev_info(netdev, "ioremap OK. Size 0x%x. reg_base 0x%x. mac_base 0x%p.\n",
     bc0:	e1a03007 	mov	r3, r7
     bc4:	e59f1030 	ldr	r1, [pc, #48]	@ bfc <nss_gmac_attach+0x9c>
     bc8:	e3a02901 	mov	r2, #16384	@ 0x4000
     bcc:	e58d0000 	str	r0, [sp]
     bd0:	e1a00006 	mov	r0, r6
     bd4:	ebfffffe 	bl	0 <netdev_info>
		      NSS_GMAC_REG_BLOCK_LEN, reg_base, gmacdev->mac_base);

	gmacdev->dma_base = gmacdev->mac_base + NSS_GMAC_DMABASE;
     bd8:	e5953000 	ldr	r3, [r5]

	return 0;
     bdc:	e1a00004 	mov	r0, r4
	gmacdev->dma_base = gmacdev->mac_base + NSS_GMAC_DMABASE;
     be0:	e2833a01 	add	r3, r3, #4096	@ 0x1000
     be4:	e5853004 	str	r3, [r5, #4]
}
     be8:	e28dd00c 	add	sp, sp, #12
     bec:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
     bf0:	00000000 	.word	0x00000000
     bf4:	00000359 	.word	0x00000359
     bf8:	00000376 	.word	0x00000376
     bfc:	00000385 	.word	0x00000385

00000c00 <nss_gmac_detach>:
 * @param[in] pointer to nss_gmac_dev to populate mac dma and phy addresses.
 * @return void
 * @note This is important function.
 */
void nss_gmac_detach(struct nss_gmac_dev *gmacdev)
{
     c00:	e92d4070 	push	{r4, r5, r6, lr}
     c04:	e1a04000 	mov	r4, r0
	uint32_t reglen;

	reglen = gmacdev->memres->end - gmacdev->memres->start + 1;
     c08:	e5943288 	ldr	r3, [r4, #648]	@ 0x288
	iounmap((void *)gmacdev->mac_base);
     c0c:	e5900000 	ldr	r0, [r0]
	reglen = gmacdev->memres->end - gmacdev->memres->start + 1;
     c10:	e8930028 	ldm	r3, {r3, r5}
     c14:	e2855001 	add	r5, r5, #1
     c18:	e0455003 	sub	r5, r5, r3
	iounmap((void *)gmacdev->mac_base);
     c1c:	ebfffffe 	bl	0 <iounmap>
	release_mem_region((gmacdev->memres)->start, reglen);
     c20:	e5943288 	ldr	r3, [r4, #648]	@ 0x288
     c24:	e1a02005 	mov	r2, r5
     c28:	e59f0018 	ldr	r0, [pc, #24]	@ c48 <nss_gmac_detach+0x48>
     c2c:	e5931000 	ldr	r1, [r3]
     c30:	ebfffffe 	bl	0 <__release_region>

	gmacdev->memres = NULL;
     c34:	e3a03000 	mov	r3, #0

	gmacdev->mac_base = 0;
     c38:	e5843000 	str	r3, [r4]
	gmacdev->dma_base = 0;
     c3c:	e5843004 	str	r3, [r4, #4]
	gmacdev->memres = NULL;
     c40:	e5843288 	str	r3, [r4, #648]	@ 0x288
}
     c44:	e8bd8070 	pop	{r4, r5, r6, pc}
     c48:	00000000 	.word	0x00000000

00000c4c <nss_gmac_init_rx_desc_base>:
	addr = (uint32_t)regbase + regoffset;
     c4c:	e5903004 	ldr	r3, [r0, #4]
 * @return returns void.
 */
void nss_gmac_init_rx_desc_base(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_write_reg(gmacdev->dma_base, dma_rx_base_addr,
					(uint32_t)gmacdev->rx_desc_dma);
     c50:	e5902024 	ldr	r2, [r0, #36]	@ 0x24
	asm volatile("str %1, %0"
     c54:	e583200c 	str	r2, [r3, #12]
}
     c58:	e12fff1e 	bx	lr

00000c5c <nss_gmac_init_tx_desc_base>:
     c5c:	e5903004 	ldr	r3, [r0, #4]
 * @return returns void.
 */
void nss_gmac_init_tx_desc_base(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_write_reg(gmacdev->dma_base, dma_tx_base_addr,
					(uint32_t)gmacdev->tx_desc_dma);
     c60:	e5902020 	ldr	r2, [r0, #32]
     c64:	e5832010 	str	r2, [r3, #16]
}
     c68:	e12fff1e 	bx	lr

00000c6c <nss_gmac_set_owner_dma>:
 * @param[in] pointer to dma_desc structure.
 * @return returns void.
 */
void nss_gmac_set_owner_dma(struct dma_desc *desc)
{
	desc->status |= desc_own_by_dma;
     c6c:	e5903000 	ldr	r3, [r0]
     c70:	e3833102 	orr	r3, r3, #-2147483648	@ 0x80000000
     c74:	e5803000 	str	r3, [r0]
}
     c78:	e12fff1e 	bx	lr

00000c7c <nss_gmac_set_desc_sof>:
 * @param[in] pointer to dma_desc structure.
 * @return returns void.
 */
void nss_gmac_set_desc_sof(struct dma_desc *desc)
{
	desc->status |= desc_tx_first;
     c7c:	e5903000 	ldr	r3, [r0]
     c80:	e3833201 	orr	r3, r3, #268435456	@ 0x10000000
     c84:	e5803000 	str	r3, [r0]
}
     c88:	e12fff1e 	bx	lr

00000c8c <nss_gmac_set_desc_eof>:
 * @param[in] pointer to dma_desc structure.
 * @return returns void.
 */
void nss_gmac_set_desc_eof(struct dma_desc *desc)
{
	desc->status |= desc_tx_last;
     c8c:	e5903000 	ldr	r3, [r0]
     c90:	e3833202 	orr	r3, r3, #536870912	@ 0x20000000
     c94:	e5803000 	str	r3, [r0]
}
     c98:	e12fff1e 	bx	lr

00000c9c <nss_gmac_is_sof_in_rx_desc>:
 * @param[in] pointer to dma_desc structure.
 * @return returns true if SOF in current descriptor, else returns fail.
 */
bool nss_gmac_is_sof_in_rx_desc(struct dma_desc *desc)
{
	return (desc->status & desc_rx_first) == desc_rx_first;
     c9c:	e5d00001 	ldrb	r0, [r0, #1]
}
     ca0:	e7e000d0 	ubfx	r0, r0, #1, #1
     ca4:	e12fff1e 	bx	lr

00000ca8 <nss_gmac_is_eof_in_rx_desc>:
 * @param[in] pointer to dma_desc structure.
 * @return returns true if SOF in current descriptor, else returns fail.
 */
bool nss_gmac_is_eof_in_rx_desc(struct dma_desc *desc)
{
	return (desc->status & desc_rx_last) == desc_rx_last;
     ca8:	e5d00001 	ldrb	r0, [r0, #1]
}
     cac:	e2000001 	and	r0, r0, #1
     cb0:	e12fff1e 	bx	lr

00000cb4 <nss_gmac_is_da_filter_failed>:
 * @param[in] pointer to dma_desc structure.
 * @return returns true if Failed, false if not.
 */
bool nss_gmac_is_da_filter_failed(struct dma_desc *desc)
{
	return (desc->status & desc_da_filter_fail) == desc_da_filter_fail;
     cb4:	e5d00003 	ldrb	r0, [r0, #3]
}
     cb8:	e7e00350 	ubfx	r0, r0, #6, #1
     cbc:	e12fff1e 	bx	lr

00000cc0 <nss_gmac_is_sa_filter_failed>:
 * @param[in] pointer to dma_desc structure.
 * @return returns true if Failed, false if not.
 */
bool nss_gmac_is_sa_filter_failed(struct dma_desc *desc)
{
	return (desc->status & desc_sa_filter_fail) == desc_sa_filter_fail;
     cc0:	e5d00001 	ldrb	r0, [r0, #1]
}
     cc4:	e7e002d0 	ubfx	r0, r0, #5, #1
     cc8:	e12fff1e 	bx	lr

00000ccc <nss_gmac_is_tx_aborted>:
 * @return returns true if collisions, else returns false.
 */
bool nss_gmac_is_tx_aborted(uint32_t status)
{
	return ((status & desc_tx_late_collision) == desc_tx_late_collision)
	    || ((status & desc_tx_exc_collisions) == desc_tx_exc_collisions);
     ccc:	e3100c03 	tst	r0, #768	@ 0x300

}
     cd0:	13a00001 	movne	r0, #1
     cd4:	03a00000 	moveq	r0, #0
     cd8:	e12fff1e 	bx	lr

00000cdc <nss_gmac_is_tx_carrier_error>:
 * @return returns true if carrier error occurred, else returns false.
 */
bool nss_gmac_is_tx_carrier_error(uint32_t status)
{
	return ((status & desc_tx_lost_carrier) == desc_tx_lost_carrier)
		|| ((status & desc_tx_no_carrier) == desc_tx_no_carrier);
     cdc:	e3100b03 	tst	r0, #3072	@ 0xc00
}
     ce0:	13a00001 	movne	r0, #1
     ce4:	03a00000 	moveq	r0, #0
     ce8:	e12fff1e 	bx	lr

00000cec <nss_gmac_is_tx_underflow_error>:
 * @return returns true if tx underflow occurred, else returns false.
 */
bool nss_gmac_is_tx_underflow_error(uint32_t status)
{
	return (status & desc_tx_underflow) == desc_tx_underflow;
}
     cec:	e7e000d0 	ubfx	r0, r0, #1, #1
     cf0:	e12fff1e 	bx	lr

00000cf4 <nss_gmac_is_tx_lc_error>:
 * @return returns true if tx late collision occurred, else returns false.
 */
bool nss_gmac_is_tx_lc_error(uint32_t status)
{
	return (status & desc_tx_late_collision) == desc_tx_late_collision;
}
     cf4:	e7e004d0 	ubfx	r0, r0, #9, #1
     cf8:	e12fff1e 	bx	lr

00000cfc <nss_gmac_is_rx_frame_damaged>:
 * @return returns true if error else returns false.
 */
bool nss_gmac_is_rx_frame_damaged(uint32_t status)
{
	return ((status & desc_rx_damaged) == desc_rx_damaged)
		|| ((status & desc_rx_collision) == desc_rx_collision);
     cfc:	e3100d21 	tst	r0, #2112	@ 0x840
}
     d00:	13a00001 	movne	r0, #1
     d04:	03a00000 	moveq	r0, #0
     d08:	e12fff1e 	bx	lr

00000d0c <nss_gmac_is_rx_frame_collision>:
 * @return returns true if error else returns false.
 */
bool nss_gmac_is_rx_frame_collision(uint32_t status)
{
	return (status & desc_rx_collision) == desc_rx_collision;
}
     d0c:	e7e00350 	ubfx	r0, r0, #6, #1
     d10:	e12fff1e 	bx	lr

00000d14 <nss_gmac_is_rx_crc>:
 * Check for receive CRC error.
 * Retruns true if rx frame CRC error occurred.
 * @param[in] pointer to dma_desc structure.
 * @return returns true if error else returns false.
 */
bool nss_gmac_is_rx_crc(uint32_t status)
     d14:	e7e000d0 	ubfx	r0, r0, #1, #1
     d18:	e12fff1e 	bx	lr

00000d1c <nss_gmac_is_frame_dribbling_errors>:
 * @return returns true if error else returns false.
 */
bool nss_gmac_is_frame_dribbling_errors(uint32_t status)
{
	return (status & desc_rx_dribbling) == desc_rx_dribbling;
}
     d1c:	e7e00150 	ubfx	r0, r0, #2, #1
     d20:	e12fff1e 	bx	lr

00000d24 <nss_gmac_is_rx_frame_length_errors>:
 * @return returns true if error else returns false.
 */
bool nss_gmac_is_rx_frame_length_errors(uint32_t status)
{
	return (status & desc_rx_length_error) == desc_rx_length_error;
}
     d24:	e7e00650 	ubfx	r0, r0, #12, #1
     d28:	e12fff1e 	bx	lr

00000d2c <nss_gmac_get_desc_data>:
{
	/*
	 * The first time, we map the descriptor as DMA_TO_DEVICE.
	 * Then we only wait for changes from device, so we use DMA_FROM_DEVICE.
	 */
	if (status != 0)
     d2c:	e3510000 	cmp	r1, #0
{
     d30:	e52de004 	push	{lr}		@ (str lr, [sp, #-4]!)
		*status = desc->status;
     d34:	1590e000 	ldrne	lr, [r0]
{
     d38:	e59dc004 	ldr	ip, [sp, #4]
		*status = desc->status;
     d3c:	1581e000 	strne	lr, [r1]

	if (buffer1 != 0)
     d40:	e3520000 	cmp	r2, #0
		*buffer1 = desc->buffer1;
     d44:	15901008 	ldrne	r1, [r0, #8]
     d48:	15821000 	strne	r1, [r2]

	if (length1 != 0)
     d4c:	e3530000 	cmp	r3, #0
		*length1 = (desc->length & desc_size1_mask) >> desc_size1_shift;
     d50:	15902004 	ldrne	r2, [r0, #4]
     d54:	17ec2052 	ubfxne	r2, r2, #0, #13
     d58:	15832000 	strne	r2, [r3]

	if (opaque != 0)
     d5c:	e35c0000 	cmp	ip, #0
		*opaque = desc->reserved1;
     d60:	15903014 	ldrne	r3, [r0, #20]
     d64:	158c3000 	strne	r3, [ip]
}
     d68:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

00000d6c <nss_gmac_enable_dma_rx>:
	addr = (uint32_t)regbase + regoffset;
     d6c:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     d70:	e5923018 	ldr	r3, [r2, #24]
void nss_gmac_enable_dma_rx(struct nss_gmac_dev *gmacdev)
{
	uint32_t data;

	data = nss_gmac_read_reg(gmacdev->dma_base, dma_control);
	data |= dma_rx_start;
     d74:	e3833002 	orr	r3, r3, #2
	asm volatile("str %1, %0"
     d78:	e5823018 	str	r3, [r2, #24]
	nss_gmac_write_reg(gmacdev->dma_base, dma_control, data);
}
     d7c:	e12fff1e 	bx	lr

00000d80 <nss_gmac_enable_dma_tx>:
     d80:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     d84:	e5923018 	ldr	r3, [r2, #24]
void nss_gmac_enable_dma_tx(struct nss_gmac_dev *gmacdev)
{
	uint32_t data;

	data = nss_gmac_read_reg(gmacdev->dma_base, dma_control);
	data |= dma_tx_start;
     d88:	e3833a02 	orr	r3, r3, #8192	@ 0x2000
	asm volatile("str %1, %0"
     d8c:	e5823018 	str	r3, [r2, #24]
	nss_gmac_write_reg(gmacdev->dma_base, dma_control, data);
}
     d90:	e12fff1e 	bx	lr

00000d94 <nss_gmac_take_desc_ownership>:
 * @param[in] pointer to nss_gmac_dev.
 * @return returns void.
 */
void nss_gmac_take_desc_ownership(struct dma_desc *desc)
{
	if (desc) {
     d94:	e3500000 	cmp	r0, #0
		/* Clear the DMA own bit */
		desc->status &= ~desc_own_by_dma;
     d98:	15903000 	ldrne	r3, [r0]
     d9c:	13c33102 	bicne	r3, r3, #-2147483648	@ 0x80000000
     da0:	15803000 	strne	r3, [r0]
	}
}
     da4:	e12fff1e 	bx	lr

00000da8 <nss_gmac_take_desc_ownership_rx>:
 * @return returns void.
 * @note Make sure to disable the transmission before calling this function,
 * otherwise may result in racing situation.
 */
void nss_gmac_take_desc_ownership_rx(struct nss_gmac_dev *gmacdev)
{
     da8:	e92d4010 	push	{r4, lr}
     dac:	e1a01000 	mov	r1, r0
	int32_t i;
	struct dma_desc *desc;

	desc = gmacdev->rx_desc;
     db0:	e590402c 	ldr	r4, [r0, #44]	@ 0x2c
	for (i = 0; i < gmacdev->rx_desc_count; i++)
     db4:	e3a02000 	mov	r2, #0
     db8:	ea000002 	b	dc8 <nss_gmac_take_desc_ownership_rx+0x20>
		nss_gmac_take_desc_ownership(desc + i);
     dbc:	e0840282 	add	r0, r4, r2, lsl #5
	for (i = 0; i < gmacdev->rx_desc_count; i++)
     dc0:	e2822001 	add	r2, r2, #1
		nss_gmac_take_desc_ownership(desc + i);
     dc4:	ebfffffe 	bl	d94 <nss_gmac_take_desc_ownership>
	for (i = 0; i < gmacdev->rx_desc_count; i++)
     dc8:	e5913038 	ldr	r3, [r1, #56]	@ 0x38
     dcc:	e1530002 	cmp	r3, r2
     dd0:	98bd8010 	popls	{r4, pc}
     dd4:	eafffff8 	b	dbc <nss_gmac_take_desc_ownership_rx+0x14>

00000dd8 <nss_gmac_take_desc_ownership_tx>:
 * @return returns void.
 * @note Make sure to disable the transmission before calling this function,
 * otherwise may result in racing situation.
 */
void nss_gmac_take_desc_ownership_tx(struct nss_gmac_dev *gmacdev)
{
     dd8:	e92d4010 	push	{r4, lr}
     ddc:	e1a01000 	mov	r1, r0
	int32_t i;
	struct dma_desc *desc;

	desc = gmacdev->tx_desc;
     de0:	e5904028 	ldr	r4, [r0, #40]	@ 0x28
	for (i = 0; i < gmacdev->tx_desc_count; i++)
     de4:	e3a02000 	mov	r2, #0
     de8:	ea000002 	b	df8 <nss_gmac_take_desc_ownership_tx+0x20>
		nss_gmac_take_desc_ownership(desc + i);
     dec:	e0840282 	add	r0, r4, r2, lsl #5
	for (i = 0; i < gmacdev->tx_desc_count; i++)
     df0:	e2822001 	add	r2, r2, #1
		nss_gmac_take_desc_ownership(desc + i);
     df4:	ebfffffe 	bl	d94 <nss_gmac_take_desc_ownership>
	for (i = 0; i < gmacdev->tx_desc_count; i++)
     df8:	e591303c 	ldr	r3, [r1, #60]	@ 0x3c
     dfc:	e1530002 	cmp	r3, r2
     e00:	98bd8010 	popls	{r4, pc}
     e04:	eafffff8 	b	dec <nss_gmac_take_desc_ownership_tx+0x14>

00000e08 <nss_gmac_disable_dma_tx>:
     e08:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     e0c:	e5923018 	ldr	r3, [r2, #24]
void nss_gmac_disable_dma_tx(struct nss_gmac_dev *gmacdev)
{
	uint32_t data;

	data = nss_gmac_read_reg(gmacdev->dma_base, dma_control);
	data &= (~dma_tx_start);
     e10:	e3c33a02 	bic	r3, r3, #8192	@ 0x2000
	asm volatile("str %1, %0"
     e14:	e5823018 	str	r3, [r2, #24]
	nss_gmac_write_reg(gmacdev->dma_base, dma_control, data);
}
     e18:	e12fff1e 	bx	lr

00000e1c <nss_gmac_disable_dma_rx>:
     e1c:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     e20:	e5923018 	ldr	r3, [r2, #24]
void nss_gmac_disable_dma_rx(struct nss_gmac_dev *gmacdev)
{
	uint32_t data;

	data = nss_gmac_read_reg(gmacdev->dma_base, dma_control);
	data &= (~dma_rx_start);
     e24:	e3c33002 	bic	r3, r3, #2
	asm volatile("str %1, %0"
     e28:	e5823018 	str	r3, [r2, #24]
	nss_gmac_write_reg(gmacdev->dma_base, dma_control, data);
}
     e2c:	e12fff1e 	bx	lr

00000e30 <nss_gmac_disable_mmc_tx_interrupt>:
     e30:	e5903000 	ldr	r3, [r0]
	asm volatile("ldr %0, %1"
     e34:	e5932110 	ldr	r2, [r3, #272]	@ 0x110
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     e38:	e1811002 	orr	r1, r1, r2
	asm volatile("str %1, %0"
     e3c:	e5831110 	str	r1, [r3, #272]	@ 0x110
void nss_gmac_disable_mmc_tx_interrupt(struct nss_gmac_dev *gmacdev,
						uint32_t mask)
{
	nss_gmac_set_reg_bits(gmacdev->mac_base,
			      gmac_mmc_intr_mask_tx, mask);
}
     e40:	e12fff1e 	bx	lr

00000e44 <nss_gmac_disable_mmc_rx_interrupt>:
	addr = (uint32_t)regbase + regoffset;
     e44:	e5903000 	ldr	r3, [r0]
	asm volatile("ldr %0, %1"
     e48:	e593210c 	ldr	r2, [r3, #268]	@ 0x10c
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     e4c:	e1811002 	orr	r1, r1, r2
	asm volatile("str %1, %0"
     e50:	e583110c 	str	r1, [r3, #268]	@ 0x10c
void nss_gmac_disable_mmc_rx_interrupt(struct nss_gmac_dev *gmacdev,
						uint32_t mask)
{
	nss_gmac_set_reg_bits(gmacdev->mac_base,
			      gmac_mmc_intr_mask_rx, mask);
}
     e54:	e12fff1e 	bx	lr

00000e58 <nss_gmac_disable_mmc_ipc_rx_interrupt>:
	addr = (uint32_t)regbase + regoffset;
     e58:	e5903000 	ldr	r3, [r0]
	asm volatile("ldr %0, %1"
     e5c:	e5932200 	ldr	r2, [r3, #512]	@ 0x200
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     e60:	e1811002 	orr	r1, r1, r2
	asm volatile("str %1, %0"
     e64:	e5831200 	str	r1, [r3, #512]	@ 0x200
void nss_gmac_disable_mmc_ipc_rx_interrupt(struct nss_gmac_dev *gmacdev,
					   uint32_t mask)
{
	nss_gmac_set_reg_bits(gmacdev->mac_base,
			      gmac_mmc_rx_ipc_intr_mask, mask);
}
     e68:	e12fff1e 	bx	lr

00000e6c <nss_gmac_enable_rx_chksum_offload>:
 * @param[in] pointer to nss_gmac_dev.
 * @return returns void.
 */
void nss_gmac_enable_rx_chksum_offload(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_set_reg_bits(gmacdev->mac_base,
     e6c:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     e70:	e5923000 	ldr	r3, [r2]
     e74:	e3833b01 	orr	r3, r3, #1024	@ 0x400
	asm volatile("str %1, %0"
     e78:	e5823000 	str	r3, [r2]
			      gmac_config, gmac_rx_ipc_offload);
}
     e7c:	e12fff1e 	bx	lr

00000e80 <nss_gmac_disable_rx_chksum_offload>:
 * @param[in] pointer to nss_gmac_dev.
 * @return returns void.
 */
void nss_gmac_disable_rx_chksum_offload(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_clear_reg_bits(gmacdev->mac_base,
     e80:	e5902000 	ldr	r2, [r0]
	asm volatile("ldr %0, %1"
     e84:	e5923000 	ldr	r3, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     e88:	e3c33b01 	bic	r3, r3, #1024	@ 0x400
	asm volatile("str %1, %0"
     e8c:	e5823000 	str	r3, [r2]
				gmac_config, gmac_rx_ipc_offload);
}
     e90:	e12fff1e 	bx	lr

00000e94 <nss_gmac_rx_tcpip_chksum_drop_enable>:
	addr = (uint32_t)regbase + regoffset;
     e94:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     e98:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
     e9c:	e3c33301 	bic	r3, r3, #67108864	@ 0x4000000
	asm volatile("str %1, %0"
     ea0:	e5823018 	str	r3, [r2, #24]
 */
void nss_gmac_rx_tcpip_chksum_drop_enable(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_clear_reg_bits(gmacdev->dma_base,
				dma_control, dma_disable_drop_tcp_cs);
}
     ea4:	e12fff1e 	bx	lr

00000ea8 <nss_gmac_ipc_offload_init>:
{
     ea8:	e92d4010 	push	{r4, lr}
     eac:	e1a04000 	mov	r4, r0
     eb0:	e5903018 	ldr	r3, [r0, #24]
	if (test_bit(__NSS_GMAC_RXCSUM, &gmacdev->flags)) {
     eb4:	e3130004 	tst	r3, #4
     eb8:	0a000004 	beq	ed0 <nss_gmac_ipc_offload_init+0x28>
		nss_gmac_enable_rx_chksum_offload(gmacdev);
     ebc:	ebfffffe 	bl	e6c <nss_gmac_enable_rx_chksum_offload>
		nss_gmac_rx_tcpip_chksum_drop_enable(gmacdev);
     ec0:	ebfffffe 	bl	e94 <nss_gmac_rx_tcpip_chksum_drop_enable>
		netdev_info(gmacdev->netdev, "%s: enable Rx checksum\n", __func__);
     ec4:	e59f101c 	ldr	r1, [pc, #28]	@ ee8 <nss_gmac_ipc_offload_init+0x40>
     ec8:	e59f201c 	ldr	r2, [pc, #28]	@ eec <nss_gmac_ipc_offload_init+0x44>
     ecc:	ea000002 	b	edc <nss_gmac_ipc_offload_init+0x34>
		nss_gmac_disable_rx_chksum_offload(gmacdev);
     ed0:	ebfffffe 	bl	e80 <nss_gmac_disable_rx_chksum_offload>
		netdev_info(gmacdev->netdev, "%s: disable Rx checksum\n", __func__);
     ed4:	e59f1014 	ldr	r1, [pc, #20]	@ ef0 <nss_gmac_ipc_offload_init+0x48>
     ed8:	e59f200c 	ldr	r2, [pc, #12]	@ eec <nss_gmac_ipc_offload_init+0x44>
     edc:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
}
     ee0:	e8bd4010 	pop	{r4, lr}
		netdev_info(gmacdev->netdev, "%s: disable Rx checksum\n", __func__);
     ee4:	eafffffe 	b	0 <netdev_info>
     ee8:	000003bb 	.word	0x000003bb
     eec:	000000d7 	.word	0x000000d7
     ef0:	000003d3 	.word	0x000003d3

00000ef4 <nss_gmac_mac_init>:
{
     ef4:	e92d4010 	push	{r4, lr}
     ef8:	e1a04000 	mov	r4, r0
	nss_gmac_wd_enable(gmacdev);
     efc:	ebfffffe 	bl	288 <nss_gmac_wd_enable>
	nss_gmac_jab_enable(gmacdev);
     f00:	ebfffffe 	bl	29c <nss_gmac_jab_enable>
	nss_gmac_frame_burst_enable(gmacdev);
     f04:	ebfffffe 	bl	2b0 <nss_gmac_frame_burst_enable>
	nss_gmac_loopback_off(gmacdev);
     f08:	ebfffffe 	bl	3a4 <nss_gmac_loopback_off>
	if (gmacdev->speed == SPEED_1000)
     f0c:	e590306c 	ldr	r3, [r0, #108]	@ 0x6c
     f10:	e3530ffa 	cmp	r3, #1000	@ 0x3e8
     f14:	1a000001 	bne	f20 <nss_gmac_mac_init+0x2c>
		nss_gmac_select_gmii(gmacdev);
     f18:	ebfffffe 	bl	33c <nss_gmac_select_gmii>
     f1c:	ea000000 	b	f24 <nss_gmac_mac_init+0x30>
		nss_gmac_select_mii(gmacdev);
     f20:	ebfffffe 	bl	350 <nss_gmac_select_mii>
	if (gmacdev->duplex_mode == DUPLEX_FULL) {
     f24:	e5943068 	ldr	r3, [r4, #104]	@ 0x68
		nss_gmac_set_full_duplex(gmacdev);
     f28:	e1a00004 	mov	r0, r4
	if (gmacdev->duplex_mode == DUPLEX_FULL) {
     f2c:	e3530001 	cmp	r3, #1
     f30:	1a000004 	bne	f48 <nss_gmac_mac_init+0x54>
		nss_gmac_set_full_duplex(gmacdev);
     f34:	ebfffffe 	bl	3b8 <nss_gmac_set_full_duplex>
		nss_gmac_rx_own_enable(gmacdev);
     f38:	ebfffffe 	bl	37c <nss_gmac_rx_own_enable>
		nss_gmac_retry_disable(gmacdev);
     f3c:	ebfffffe 	bl	3f4 <nss_gmac_retry_disable>
		nss_gmac_enable_crs(gmacdev);
     f40:	ebfffffe 	bl	328 <nss_gmac_enable_crs>
     f44:	ea000003 	b	f58 <nss_gmac_mac_init+0x64>
		nss_gmac_set_half_duplex(gmacdev);
     f48:	ebfffffe 	bl	3cc <nss_gmac_set_half_duplex>
		nss_gmac_rx_own_disable(gmacdev);
     f4c:	ebfffffe 	bl	390 <nss_gmac_rx_own_disable>
		nss_gmac_retry_enable(gmacdev);
     f50:	ebfffffe 	bl	3e0 <nss_gmac_retry_enable>
		nss_gmac_disable_crs(gmacdev);
     f54:	ebfffffe 	bl	314 <nss_gmac_disable_crs>
	nss_gmac_pad_crc_strip_disable(gmacdev);
     f58:	e1a00004 	mov	r0, r4
	nss_gmac_back_off_limit(gmacdev, gmac_backoff_limit0);
     f5c:	e3a01000 	mov	r1, #0
	nss_gmac_pad_crc_strip_disable(gmacdev);
     f60:	ebfffffe 	bl	408 <nss_gmac_pad_crc_strip_disable>
	nss_gmac_back_off_limit(gmacdev, gmac_backoff_limit0);
     f64:	ebfffffe 	bl	41c <nss_gmac_back_off_limit>
	nss_gmac_deferral_check_disable(gmacdev);
     f68:	ebfffffe 	bl	434 <nss_gmac_deferral_check_disable>
			      gmac_addr0_low, gmacdev->netdev->dev_addr);
     f6c:	e5943090 	ldr	r3, [r4, #144]	@ 0x90
	nss_gmac_set_mac_addr(gmacdev, gmac_addr0_high,
     f70:	e3a02044 	mov	r2, #68	@ 0x44
     f74:	e3a01040 	mov	r1, #64	@ 0x40
     f78:	e59331f4 	ldr	r3, [r3, #500]	@ 0x1f4
     f7c:	ebfffffe 	bl	2ec <nss_gmac_twokpe_frame_enable>
	nss_gmac_frame_filter_enable(gmacdev);
     f80:	e1a00004 	mov	r0, r4
	nss_gmac_set_pass_control(gmacdev, gmac_pass_control0);
     f84:	e3a01000 	mov	r1, #0
	nss_gmac_frame_filter_enable(gmacdev);
     f88:	ebfffffe 	bl	498 <nss_gmac_frame_filter_enable>
	nss_gmac_set_pass_control(gmacdev, gmac_pass_control0);
     f8c:	ebfffffe 	bl	4d4 <nss_gmac_set_pass_control>
	nss_gmac_broadcast_enable(gmacdev);
     f90:	ebfffffe 	bl	4ec <nss_gmac_broadcast_enable>
	nss_gmac_src_addr_filter_disable(gmacdev);
     f94:	ebfffffe 	bl	4ac <nss_gmac_src_addr_filter_disable>
	nss_gmac_multicast_enable(gmacdev);
     f98:	ebfffffe 	bl	500 <nss_gmac_multicast_enable>
	gmacdev->netdev->flags |= IFF_ALLMULTI;
     f9c:	e5942090 	ldr	r2, [r4, #144]	@ 0x90
     fa0:	e5923068 	ldr	r3, [r2, #104]	@ 0x68
     fa4:	e3833c02 	orr	r3, r3, #512	@ 0x200
     fa8:	e5823068 	str	r3, [r2, #104]	@ 0x68
	nss_gmac_dst_addr_filter_normal(gmacdev);
     fac:	ebfffffe 	bl	4c0 <nss_gmac_dst_addr_filter_normal>
	nss_gmac_multicast_hash_filter_disable(gmacdev);
     fb0:	ebfffffe 	bl	528 <nss_gmac_multicast_hash_filter_disable>
	nss_gmac_promisc_enable(gmacdev);
     fb4:	ebfffffe 	bl	53c <nss_gmac_promisc_enable>
	nss_gmac_unicast_hash_filter_disable(gmacdev);
     fb8:	ebfffffe 	bl	564 <nss_gmac_unicast_hash_filter_disable>
	nss_gmac_ipc_offload_init(gmacdev);
     fbc:	ebfffffe 	bl	ea8 <nss_gmac_ipc_offload_init>
	nss_gmac_mmc_stats_cor_enable(gmacdev);
     fc0:	e1a00004 	mov	r0, r4
     fc4:	ebfffffe 	bl	70c <nss_gmac_mmc_stats_cor_enable>
	nss_gmac_unicast_pause_frame_detect_disable(gmacdev);
     fc8:	ebfffffe 	bl	578 <nss_gmac_unicast_pause_frame_detect_disable>
	nss_gmac_config_flow_control(gmacdev);
     fcc:	ebfffffe 	bl	208 <nss_gmac_reset_phy+0x28>
	nss_gmac_tx_enable(gmacdev);
     fd0:	e1a00004 	mov	r0, r4
     fd4:	ebfffffe 	bl	470 <nss_gmac_tx_enable>
}
     fd8:	e8bd4010 	pop	{r4, lr}
	nss_gmac_rx_enable(gmacdev);
     fdc:	eafffffe 	b	448 <nss_gmac_rx_enable>

00000fe0 <nss_gmac_rx_tcpip_chksum_drop_disable>:
	addr = (uint32_t)regbase + regoffset;
     fe0:	e5902004 	ldr	r2, [r0, #4]
	asm volatile("ldr %0, %1"
     fe4:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
     fe8:	e3833301 	orr	r3, r3, #67108864	@ 0x4000000
	asm volatile("str %1, %0"
     fec:	e5823018 	str	r3, [r2, #24]
 */
void nss_gmac_rx_tcpip_chksum_drop_disable(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_set_reg_bits(gmacdev->dma_base,
			      dma_control, dma_disable_drop_tcp_cs);
}
     ff0:	e12fff1e 	bx	lr
     ff4:	00000000 	andeq	r0, r0, r0

00000ff8 <nss_gmac_remove>:
	/*
	 * All remove has been done in nss_gmac_exit_network_interfaces
	 * Just return success here
	 */
	return 0;
}
     ff8:	e3a00000 	mov	r0, #0
     ffc:	e12fff1e 	bx	lr

00001000 <nss_gmac_mtnp_show>:
{
    1000:	e92d401f 	push	{r0, r1, r2, r3, r4, lr}
    1004:	e1a0e002 	mov	lr, r2
	addr = (uint32_t)regbase + regoffset;
    1008:	e590c200 	ldr	ip, [r0, #512]	@ 0x200
	timeout = TSTAMP_LOOP_VARIABLE;
    100c:	e3a01ffa 	mov	r1, #1000	@ 0x3e8
		timeout--;
    1010:	e2411001 	sub	r1, r1, #1
	asm volatile("ldr %0, %1"
    1014:	e59c3708 	ldr	r3, [ip, #1800]	@ 0x708
    1018:	e59c270c 	ldr	r2, [ip, #1804]	@ 0x70c
    101c:	e59c4708 	ldr	r4, [ip, #1800]	@ 0x708
	} while (sec != nss_gmac_read_reg(gmacdev->mac_base, gmac_ts_high));
    1020:	e1540003 	cmp	r4, r3
    1024:	1afffff9 	bne	1010 <nss_gmac_mtnp_show+0x10>
	if (timeout == 0) {
    1028:	e3510000 	cmp	r1, #0
    102c:	1a000005 	bne	1048 <nss_gmac_mtnp_show+0x48>
		netdev_info(gmacdev->netdev, "%s: Mtnp show timed out\n", __func__);
    1030:	e5900290 	ldr	r0, [r0, #656]	@ 0x290
    1034:	e59f2044 	ldr	r2, [pc, #68]	@ 1080 <nss_gmac_mtnp_show+0x80>
    1038:	e59f1044 	ldr	r1, [pc, #68]	@ 1084 <nss_gmac_mtnp_show+0x84>
    103c:	ebfffffe 	bl	0 <netdev_info>
		return -1;
    1040:	e3e00000 	mvn	r0, #0
    1044:	ea00000b 	b	1078 <nss_gmac_mtnp_show+0x78>
	ret = snprintf(buf, PAGE_SIZE, "%u %u %u %u\n",
    1048:	e3520000 	cmp	r2, #0
    104c:	e58d2000 	str	r2, [sp]
    1050:	159f1030 	ldrne	r1, [pc, #48]	@ 1088 <nss_gmac_mtnp_show+0x88>
    1054:	01a01002 	moveq	r1, r2
    1058:	03a00001 	moveq	r0, #1
    105c:	13a00000 	movne	r0, #0
    1060:	10411002 	subne	r1, r1, r2
    1064:	e59f2020 	ldr	r2, [pc, #32]	@ 108c <nss_gmac_mtnp_show+0x8c>
    1068:	e1cd00f4 	strd	r0, [sp, #4]
    106c:	e3a01a01 	mov	r1, #4096	@ 0x1000
    1070:	e1a0000e 	mov	r0, lr
    1074:	ebfffffe 	bl	0 <snprintf>
}
    1078:	e28dd010 	add	sp, sp, #16
    107c:	e8bd8010 	pop	{r4, pc}
    1080:	000000f4 	.word	0x000000f4
    1084:	000003ec 	.word	0x000003ec
    1088:	3b9aca00 	.word	0x3b9aca00
    108c:	00000405 	.word	0x00000405

00001090 <nss_gmac_phy_fixup>:
{
    1090:	e92d4070 	push	{r4, r5, r6, lr}
    1094:	e1a05000 	mov	r5, r0
	if (nss_gmac_ath_phy_disable_smart_802az(phydev) != 0)
    1098:	ebfffffe 	bl	a94 <nss_gmac_ath_phy_disable_smart_802az>
		ret = -EFAULT;
    109c:	e2504000 	subs	r4, r0, #0
	if (nss_gmac_ath_phy_disable_802az(phydev) != 0)
    10a0:	e1a00005 	mov	r0, r5
		ret = -EFAULT;
    10a4:	13e0400d 	mvnne	r4, #13
	if (nss_gmac_ath_phy_disable_802az(phydev) != 0)
    10a8:	ebfffffe 	bl	ad8 <nss_gmac_ath_phy_disable_802az>
		ret = -EFAULT;
    10ac:	e3500000 	cmp	r0, #0
}
    10b0:	01a00004 	moveq	r0, r4
    10b4:	13e0000d 	mvnne	r0, #13
    10b8:	e8bd8070 	pop	{r4, r5, r6, pc}

000010bc <nss_gmac_set_features>:
{
    10bc:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(gmacdev == NULL);
    10c0:	e2905d15 	adds	r5, r0, #1344	@ 0x540
    10c4:	1a000000 	bne	10cc <nss_gmac_set_features+0x10>
    10c8:	e7f001f2 	.word	0xe7f001f2
	changed = features ^ netdev->features;
    10cc:	e5906090 	ldr	r6, [r0, #144]	@ 0x90
    10d0:	e5901094 	ldr	r1, [r0, #148]	@ 0x94
	if (!(changed & (NETIF_F_RXCSUM | NETIF_F_HW_CSUM | NETIF_F_GRO)))
    10d4:	e3040008 	movw	r0, #16392	@ 0x4008
	changed = features ^ netdev->features;
    10d8:	e0266002 	eor	r6, r6, r2
    10dc:	e0211003 	eor	r1, r1, r3
	if (!(changed & (NETIF_F_RXCSUM | NETIF_F_HW_CSUM | NETIF_F_GRO)))
    10e0:	e0060000 	and	r0, r6, r0
    10e4:	e2011c01 	and	r1, r1, #256	@ 0x100
    10e8:	e1900001 	orrs	r0, r0, r1
    10ec:	0a000012 	beq	113c <nss_gmac_set_features+0x80>
	if (changed & NETIF_F_RXCSUM) {
    10f0:	e3911000 	orrs	r1, r1, #0
    10f4:	e1a04002 	mov	r4, r2
    10f8:	0a000008 	beq	1120 <nss_gmac_set_features+0x64>
		if (features & NETIF_F_RXCSUM)
    10fc:	e3130c01 	tst	r3, #256	@ 0x100
			set_bit(__NSS_GMAC_RXCSUM, &gmacdev->flags);
    1100:	e3a00002 	mov	r0, #2
    1104:	e2851018 	add	r1, r5, #24
		if (features & NETIF_F_RXCSUM)
    1108:	0a000001 	beq	1114 <nss_gmac_set_features+0x58>
			set_bit(__NSS_GMAC_RXCSUM, &gmacdev->flags);
    110c:	ebfffffe 	bl	0 <_set_bit>
    1110:	ea000000 	b	1118 <nss_gmac_set_features+0x5c>
			clear_bit(__NSS_GMAC_RXCSUM, &gmacdev->flags);
    1114:	ebfffffe 	bl	0 <_clear_bit>
		nss_gmac_ipc_offload_init(gmacdev);
    1118:	e1a00005 	mov	r0, r5
    111c:	ebfffffe 	bl	ea8 <nss_gmac_ipc_offload_init>
	if (changed & NETIF_F_GRO) {
    1120:	e3160901 	tst	r6, #16384	@ 0x4000
    1124:	0a000004 	beq	113c <nss_gmac_set_features+0x80>
		if (!(features & NETIF_F_GRO)) {
    1128:	e2041901 	and	r1, r4, #16384	@ 0x4000
    112c:	e3913000 	orrs	r3, r1, #0
    1130:	1a000001 	bne	113c <nss_gmac_set_features+0x80>
			napi_gro_flush(&gmacdev->napi, false);
    1134:	e28500c8 	add	r0, r5, #200	@ 0xc8
    1138:	ebfffffe 	bl	0 <napi_gro_flush>
}
    113c:	e3a00000 	mov	r0, #0
    1140:	e8bd8070 	pop	{r4, r5, r6, pc}

00001144 <nss_gmac_tstamp_show>:
{
    1144:	e92d4070 	push	{r4, r5, r6, lr}
    1148:	e1a05002 	mov	r5, r2
    114c:	e5902200 	ldr	r2, [r0, #512]	@ 0x200
    1150:	e24dd020 	sub	sp, sp, #32
	timeout = TSTAMP_LOOP_VARIABLE;
    1154:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
		timeout--;
    1158:	e2433001 	sub	r3, r3, #1
    115c:	e5921708 	ldr	r1, [r2, #1800]	@ 0x708
    1160:	e592670c 	ldr	r6, [r2, #1804]	@ 0x70c
    1164:	e5924708 	ldr	r4, [r2, #1800]	@ 0x708
	} while ((ts_hi !=  nss_gmac_read_reg(gmacdev->mac_base, gmac_ts_high)) && timeout > 0);
    1168:	e1540001 	cmp	r4, r1
    116c:	0a000002 	beq	117c <nss_gmac_tstamp_show+0x38>
    1170:	e3530000 	cmp	r3, #0
    1174:	1afffff7 	bne	1158 <nss_gmac_tstamp_show+0x14>
    1178:	ea000001 	b	1184 <nss_gmac_tstamp_show+0x40>
	if (timeout == 0) {
    117c:	e3530000 	cmp	r3, #0
    1180:	1a000005 	bne	119c <nss_gmac_tstamp_show+0x58>
		netdev_info(gmacdev->netdev, "%s: Tstamp show timed out\n", __func__);
    1184:	e5900290 	ldr	r0, [r0, #656]	@ 0x290
    1188:	e59f204c 	ldr	r2, [pc, #76]	@ 11dc <nss_gmac_tstamp_show+0x98>
    118c:	e59f104c 	ldr	r1, [pc, #76]	@ 11e0 <nss_gmac_tstamp_show+0x9c>
    1190:	ebfffffe 	bl	0 <netdev_info>
		return -1;
    1194:	e3e00000 	mvn	r0, #0
    1198:	ea00000d 	b	11d4 <nss_gmac_tstamp_show+0x90>
	ktime_get_real_ts64(&ts64);
    119c:	e28d0010 	add	r0, sp, #16
    11a0:	ebfffffe 	bl	0 <ktime_get_real_ts64>
		ts_hi, ts_lo, (int)ts64.tv_sec, (int)(ts64.tv_nsec / NSEC_PER_USEC));
    11a4:	e59d3018 	ldr	r3, [sp, #24]
    11a8:	e3a02ffa 	mov	r2, #1000	@ 0x3e8
	ret = snprintf(
    11ac:	e3a01a01 	mov	r1, #4096	@ 0x1000
    11b0:	e1a00005 	mov	r0, r5
    11b4:	e58d6000 	str	r6, [sp]
		ts_hi, ts_lo, (int)ts64.tv_sec, (int)(ts64.tv_nsec / NSEC_PER_USEC));
    11b8:	e713f213 	sdiv	r3, r3, r2
	ret = snprintf(
    11bc:	e59f2020 	ldr	r2, [pc, #32]	@ 11e4 <nss_gmac_tstamp_show+0xa0>
    11c0:	e58d3008 	str	r3, [sp, #8]
    11c4:	e59d3010 	ldr	r3, [sp, #16]
    11c8:	e58d3004 	str	r3, [sp, #4]
    11cc:	e1a03004 	mov	r3, r4
    11d0:	ebfffffe 	bl	0 <snprintf>
}
    11d4:	e28dd020 	add	sp, sp, #32
    11d8:	e8bd8070 	pop	{r4, r5, r6, pc}
    11dc:	00000107 	.word	0x00000107
    11e0:	00000412 	.word	0x00000412
    11e4:	0000042d 	.word	0x0000042d

000011e8 <nss_gmac_slam>:
{
    11e8:	e92d4073 	push	{r0, r1, r4, r5, r6, lr}
    11ec:	e1a05002 	mov	r5, r2
	if (!count) {
    11f0:	e2534000 	subs	r4, r3, #0
	uint32_t sec = 0;
    11f4:	e3a02000 	mov	r2, #0
    11f8:	e58d2000 	str	r2, [sp]
	uint32_t nsec = 0;
    11fc:	e58d2004 	str	r2, [sp, #4]
		pr_err("%s: Invalid input buffer.\n", __func__);
    1200:	059f10c8 	ldreq	r1, [pc, #200]	@ 12d0 <nss_gmac_slam+0xe8>
    1204:	059f00c8 	ldreq	r0, [pc, #200]	@ 12d4 <nss_gmac_slam+0xec>
    1208:	0a000009 	beq	1234 <nss_gmac_slam+0x4c>
	if (sscanf(buf, "%u %u", &sec, &nsec) != 2) {
    120c:	e59f10c4 	ldr	r1, [pc, #196]	@ 12d8 <nss_gmac_slam+0xf0>
    1210:	e1a06000 	mov	r6, r0
    1214:	e28d3004 	add	r3, sp, #4
    1218:	e1a0200d 	mov	r2, sp
    121c:	e1a00005 	mov	r0, r5
    1220:	ebfffffe 	bl	0 <sscanf>
    1224:	e3500002 	cmp	r0, #2
    1228:	0a000004 	beq	1240 <nss_gmac_slam+0x58>
		pr_err("%s: Invalid input value.\n", __func__);
    122c:	e59f109c 	ldr	r1, [pc, #156]	@ 12d0 <nss_gmac_slam+0xe8>
    1230:	e59f00a4 	ldr	r0, [pc, #164]	@ 12dc <nss_gmac_slam+0xf4>
    1234:	ebfffffe 	bl	0 <_printk>
		return -EINVAL;
    1238:	e3e00015 	mvn	r0, #21
    123c:	ea000021 	b	12c8 <nss_gmac_slam+0xe0>
	asm volatile("str %1, %0"
    1240:	e59d2000 	ldr	r2, [sp]
	addr = (uint32_t)regbase + regoffset;
    1244:	e5963200 	ldr	r3, [r6, #512]	@ 0x200
    1248:	e5832710 	str	r2, [r3, #1808]	@ 0x710
    124c:	e59d2004 	ldr	r2, [sp, #4]
    1250:	e5832714 	str	r2, [r3, #1812]	@ 0x714
	timeout = TSTAMP_LOOP_VARIABLE;
    1254:	e3a01ffa 	mov	r1, #1000	@ 0x3e8
		timeout--;
    1258:	e2411001 	sub	r1, r1, #1
	asm volatile("ldr %0, %1"
    125c:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
	} while ((data & gmac_ts_init_mask) && timeout > 0);
    1260:	e3120004 	tst	r2, #4
    1264:	0a000002 	beq	1274 <nss_gmac_slam+0x8c>
    1268:	e3510000 	cmp	r1, #0
    126c:	1afffff9 	bne	1258 <nss_gmac_slam+0x70>
    1270:	ea000001 	b	127c <nss_gmac_slam+0x94>
	if (timeout == 0) {
    1274:	e3510000 	cmp	r1, #0
    1278:	1a000005 	bne	1294 <nss_gmac_slam+0xac>
		netdev_info(gmacdev->netdev, "%s: Slam timed out\n", __func__);
    127c:	e5960290 	ldr	r0, [r6, #656]	@ 0x290
    1280:	e59f2048 	ldr	r2, [pc, #72]	@ 12d0 <nss_gmac_slam+0xe8>
    1284:	e59f1054 	ldr	r1, [pc, #84]	@ 12e0 <nss_gmac_slam+0xf8>
    1288:	ebfffffe 	bl	0 <netdev_info>
		return -1;
    128c:	e3e00000 	mvn	r0, #0
    1290:	ea00000c 	b	12c8 <nss_gmac_slam+0xe0>
	nss_gmac_write_reg(gmacdev->mac_base, gmac_ts_control, data | gmac_ts_init_mask);
    1294:	e3822004 	orr	r2, r2, #4
	asm volatile("str %1, %0"
    1298:	e5832700 	str	r2, [r3, #1792]	@ 0x700
		if (maxlen >= p_size)
			return p_len;
	}

	/* Do not check characters beyond the end of p. */
	ret = __real_strnlen(p, maxlen < p_size ? maxlen : p_size);
    129c:	e1a01004 	mov	r1, r4
    12a0:	e1a00005 	mov	r0, r5
    12a4:	ebfffffe 	bl	0 <strnlen>
	if (p_size <= ret && maxlen != ret)
    12a8:	e0544000 	subs	r4, r4, r0
    12ac:	13a04001 	movne	r4, #1
    12b0:	e3700001 	cmn	r0, #1
    12b4:	13a04000 	movne	r4, #0
    12b8:	e3540000 	cmp	r4, #0
    12bc:	0a000001 	beq	12c8 <nss_gmac_slam+0xe0>
		fortify_panic(__func__);
    12c0:	e59f001c 	ldr	r0, [pc, #28]	@ 12e4 <nss_gmac_slam+0xfc>
    12c4:	ebfffffe 	bl	0 <fortify_panic>
}
    12c8:	e28dd008 	add	sp, sp, #8
    12cc:	e8bd8070 	pop	{r4, r5, r6, pc}
    12d0:	0000011c 	.word	0x0000011c
    12d4:	00000455 	.word	0x00000455
    12d8:	00000472 	.word	0x00000472
    12dc:	00000478 	.word	0x00000478
    12e0:	00000494 	.word	0x00000494
    12e4:	0000012a 	.word	0x0000012a

000012e8 <nss_gmac_get_stats64>:
{
    12e8:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
	BUG_ON(gmacdev == NULL);
    12ec:	e2907d15 	adds	r7, r0, #1344	@ 0x540
    12f0:	1a000000 	bne	12f8 <nss_gmac_get_stats64+0x10>
    12f4:	e7f001f2 	.word	0xe7f001f2
	if (!gmacdev->data_plane_ops)
    12f8:	e5903a6c 	ldr	r3, [r0, #2668]	@ 0xa6c
    12fc:	e3530000 	cmp	r3, #0
    1300:	08bd81f0 	popeq	{r4, r5, r6, r7, r8, pc}
    1304:	e1a04000 	mov	r4, r0
	raw_spin_lock(&lock->rlock);
}

static __always_inline void spin_lock_bh(spinlock_t *lock)
{
	raw_spin_lock_bh(&lock->rlock);
    1308:	e2800e7a 	add	r0, r0, #1952	@ 0x7a0
    130c:	e2800008 	add	r0, r0, #8
    1310:	e1a05001 	mov	r5, r1
    1314:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	gmacdev->data_plane_ops->get_stats(gmacdev->data_plane_ctx, &gmacdev->nss_stats);
    1318:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    131c:	e2871fa6 	add	r1, r7, #664	@ 0x298
    1320:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    1324:	e5933020 	ldr	r3, [r3, #32]
    1328:	e12fff33 	blx	r3
	stats->rx_packets = gmacdev->nss_stats.rx_packets;
    132c:	e2841e7e 	add	r1, r4, #2016	@ 0x7e0
	stats->collisions = gmacdev->nss_stats.tx_collisions + gmacdev->nss_stats.rx_late_collision_errors;
    1330:	e2840e85 	add	r0, r4, #2128	@ 0x850
	stats->rx_packets = gmacdev->nss_stats.rx_packets;
    1334:	e1c120d0 	ldrd	r2, [r1]
    1338:	e1c520f0 	strd	r2, [r5]
	stats->rx_bytes = gmacdev->nss_stats.rx_bytes;
    133c:	e14120d8 	ldrd	r2, [r1, #-8]
	stats->rx_length_errors = gmacdev->nss_stats.rx_length_errors;
    1340:	e2841e81 	add	r1, r4, #2064	@ 0x810
	stats->rx_bytes = gmacdev->nss_stats.rx_bytes;
    1344:	e1c521f0 	strd	r2, [r5, #16]
	stats->rx_errors = gmacdev->nss_stats.rx_errors;
    1348:	e2843e7f 	add	r3, r4, #2032	@ 0x7f0
    134c:	e14320d8 	ldrd	r2, [r3, #-8]
    1350:	e1c522f0 	strd	r2, [r5, #32]
	stats->rx_dropped = gmacdev->nss_stats.rx_errors;
    1354:	e1c523f0 	strd	r2, [r5, #48]	@ 0x30
	stats->rx_length_errors = gmacdev->nss_stats.rx_length_errors;
    1358:	e1c120d0 	ldrd	r2, [r1]
    135c:	e1c525f0 	strd	r2, [r5, #80]	@ 0x50
	stats->rx_over_errors = gmacdev->nss_stats.mmc_rx_overflow_errors;
    1360:	e2843d25 	add	r3, r4, #2368	@ 0x940
    1364:	e1c320d0 	ldrd	r2, [r3]
    1368:	e1c525f8 	strd	r2, [r5, #88]	@ 0x58
	stats->rx_crc_errors = gmacdev->nss_stats.mmc_rx_crc_errors;
    136c:	e2843e95 	add	r3, r4, #2384	@ 0x950
    1370:	e1c320d0 	ldrd	r2, [r3]
    1374:	e1c526f0 	strd	r2, [r5, #96]	@ 0x60
	stats->rx_frame_errors = gmacdev->nss_stats.rx_dribble_bit_errors;
    1378:	e14120d8 	ldrd	r2, [r1, #-8]
    137c:	e1c526f8 	strd	r2, [r5, #104]	@ 0x68
	stats->rx_fifo_errors = gmacdev->nss_stats.fifo_overflows;
    1380:	e2843e91 	add	r3, r4, #2320	@ 0x910
    1384:	e14320d8 	ldrd	r2, [r3, #-8]
    1388:	e1c527f0 	strd	r2, [r5, #112]	@ 0x70
	stats->rx_missed_errors = gmacdev->nss_stats.rx_missed;
    138c:	e2843c09 	add	r3, r4, #2304	@ 0x900
    1390:	e1c320d0 	ldrd	r2, [r3]
    1394:	e1c527f8 	strd	r2, [r5, #120]	@ 0x78
	stats->collisions = gmacdev->nss_stats.tx_collisions + gmacdev->nss_stats.rx_late_collision_errors;
    1398:	e5941800 	ldr	r1, [r4, #2048]	@ 0x800
    139c:	e594c804 	ldr	ip, [r4, #2052]	@ 0x804
    13a0:	e5943848 	ldr	r3, [r4, #2120]	@ 0x848
    13a4:	e594284c 	ldr	r2, [r4, #2124]	@ 0x84c
    13a8:	e0933001 	adds	r3, r3, r1
	stats->tx_packets = gmacdev->nss_stats.tx_packets;
    13ac:	e2841d21 	add	r1, r4, #2112	@ 0x840
	stats->collisions = gmacdev->nss_stats.tx_collisions + gmacdev->nss_stats.rx_late_collision_errors;
    13b0:	e0a2200c 	adc	r2, r2, ip
    13b4:	e5853048 	str	r3, [r5, #72]	@ 0x48
    13b8:	e585204c 	str	r2, [r5, #76]	@ 0x4c
	stats->tx_packets = gmacdev->nss_stats.tx_packets;
    13bc:	e1c120d0 	ldrd	r2, [r1]
    13c0:	e1c520f8 	strd	r2, [r5, #8]
	stats->tx_bytes = gmacdev->nss_stats.tx_bytes;
    13c4:	e14120d8 	ldrd	r2, [r1, #-8]
    13c8:	e1c521f8 	strd	r2, [r5, #24]
	stats->tx_errors = gmacdev->nss_stats.tx_errors;
    13cc:	e1c020d0 	ldrd	r2, [r0]
    13d0:	e1c522f8 	strd	r2, [r5, #40]	@ 0x28
	stats->tx_dropped = gmacdev->nss_stats.tx_dropped;
    13d4:	e2843e8b 	add	r3, r4, #2224	@ 0x8b0
    13d8:	e14320d8 	ldrd	r2, [r3, #-8]
    13dc:	e1c523f8 	strd	r2, [r5, #56]	@ 0x38
	stats->tx_carrier_errors = gmacdev->nss_stats.tx_loss_of_carrier_errors + gmacdev->nss_stats.tx_no_carrier_errors;
    13e0:	e5943868 	ldr	r3, [r4, #2152]	@ 0x868
    13e4:	e594286c 	ldr	r2, [r4, #2156]	@ 0x86c
    13e8:	e5941870 	ldr	r1, [r4, #2160]	@ 0x870
    13ec:	e5940874 	ldr	r0, [r4, #2164]	@ 0x874
    13f0:	e0933001 	adds	r3, r3, r1
    13f4:	e0a22000 	adc	r2, r2, r0
	raw_spin_unlock(&lock->rlock);
}

static __always_inline void spin_unlock_bh(spinlock_t *lock)
{
	raw_spin_unlock_bh(&lock->rlock);
    13f8:	e2840e7a 	add	r0, r4, #1952	@ 0x7a0
    13fc:	e5853088 	str	r3, [r5, #136]	@ 0x88
	stats->tx_fifo_errors = gmacdev->nss_stats.tx_underflow_errors;
    1400:	e2843e89 	add	r3, r4, #2192	@ 0x890
	stats->tx_window_errors = gmacdev->nss_stats.tx_late_collision_errors;
    1404:	e2844d22 	add	r4, r4, #2176	@ 0x880
	stats->tx_carrier_errors = gmacdev->nss_stats.tx_loss_of_carrier_errors + gmacdev->nss_stats.tx_no_carrier_errors;
    1408:	e585208c 	str	r2, [r5, #140]	@ 0x8c
	stats->tx_fifo_errors = gmacdev->nss_stats.tx_underflow_errors;
    140c:	e1c320d0 	ldrd	r2, [r3]
    1410:	e2800008 	add	r0, r0, #8
    1414:	e1c529f0 	strd	r2, [r5, #144]	@ 0x90
	stats->tx_window_errors = gmacdev->nss_stats.tx_late_collision_errors;
    1418:	e14420d8 	ldrd	r2, [r4, #-8]
    141c:	e1c52af0 	strd	r2, [r5, #160]	@ 0xa0
}
    1420:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
    1424:	eafffffe 	b	0 <_raw_spin_unlock_bh>

00001428 <nss_gmac_set_rx_mode>:
{
    1428:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(gmacdev == NULL);
    142c:	e2905d15 	adds	r5, r0, #1344	@ 0x540
    1430:	1a000000 	bne	1438 <nss_gmac_set_rx_mode+0x10>
    1434:	e7f001f2 	.word	0xe7f001f2
	if (netdev->flags & IFF_PROMISC) {
    1438:	e5903068 	ldr	r3, [r0, #104]	@ 0x68
    143c:	e3130c01 	tst	r3, #256	@ 0x100
    1440:	0a000002 	beq	1450 <nss_gmac_set_rx_mode+0x28>
		nss_gmac_promisc_enable(gmacdev);
    1444:	e1a00005 	mov	r0, r5
}
    1448:	e8bd4070 	pop	{r4, r5, r6, lr}
		nss_gmac_promisc_enable(gmacdev);
    144c:	eafffffe 	b	53c <nss_gmac_promisc_enable>
    1450:	e1a04000 	mov	r4, r0
		nss_gmac_promisc_disable(gmacdev);
    1454:	e1a00005 	mov	r0, r5
    1458:	ebfffffe 	bl	550 <nss_gmac_promisc_disable>
		if (netdev->flags & IFF_ALLMULTI)
    145c:	e5943068 	ldr	r3, [r4, #104]	@ 0x68
			nss_gmac_multicast_enable(gmacdev);
    1460:	e1a00005 	mov	r0, r5
		if (netdev->flags & IFF_ALLMULTI)
    1464:	e3130c02 	tst	r3, #512	@ 0x200
    1468:	0a000001 	beq	1474 <nss_gmac_set_rx_mode+0x4c>
}
    146c:	e8bd4070 	pop	{r4, r5, r6, lr}
			nss_gmac_multicast_enable(gmacdev);
    1470:	eafffffe 	b	500 <nss_gmac_multicast_enable>
}
    1474:	e8bd4070 	pop	{r4, r5, r6, lr}
			nss_gmac_multicast_disable(gmacdev);
    1478:	eafffffe 	b	514 <nss_gmac_multicast_disable>

0000147c <of_property_read_u32>:
 */
static inline int of_property_read_u32_array(const struct device_node *np,
					     const char *propname,
					     u32 *out_values, size_t sz)
{
	int ret = of_property_read_variable_u32_array(np, propname, out_values,
    147c:	e3a03000 	mov	r3, #0
}

static inline int of_property_read_u32(const struct device_node *np,
				       const char *propname,
				       u32 *out_value)
{
    1480:	e92d4007 	push	{r0, r1, r2, lr}
	int ret = of_property_read_variable_u32_array(np, propname, out_values,
    1484:	e58d3000 	str	r3, [sp]
    1488:	e3a03001 	mov	r3, #1
    148c:	ebfffffe 	bl	0 <of_property_read_variable_u32_array>
	return of_property_read_u32_array(np, propname, out_value, 1);
}
    1490:	e0000fc0 	and	r0, r0, r0, asr #31
    1494:	e28dd00c 	add	sp, sp, #12
    1498:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

0000149c <of_parse_phandle.constprop.0>:
static inline struct device_node *of_parse_phandle(const struct device_node *np,
    149c:	e92d4010 	push	{r4, lr}
    14a0:	e24dd050 	sub	sp, sp, #80	@ 0x50
	if (__of_parse_phandle_with_args(np, phandle_name, NULL, 0,
    14a4:	e3a04000 	mov	r4, #0
    14a8:	e28d3008 	add	r3, sp, #8
    14ac:	e59f1024 	ldr	r1, [pc, #36]	@ 14d8 <of_parse_phandle.constprop.0+0x3c>
    14b0:	e1a02004 	mov	r2, r4
    14b4:	e58d4000 	str	r4, [sp]
    14b8:	e58d3004 	str	r3, [sp, #4]
    14bc:	e1a03004 	mov	r3, r4
    14c0:	ebfffffe 	bl	0 <__of_parse_phandle_with_args>
    14c4:	e1500004 	cmp	r0, r4
	return args.np;
    14c8:	059d0008 	ldreq	r0, [sp, #8]
		return NULL;
    14cc:	11a00004 	movne	r0, r4
}
    14d0:	e28dd050 	add	sp, sp, #80	@ 0x50
    14d4:	e8bd8010 	pop	{r4, pc}
    14d8:	000004a8 	.word	0x000004a8

000014dc <_copy_from_user.constprop.0>:

	if (IS_ENABLED(CONFIG_ALTERNATE_USER_ADDRESS_SPACE) ||
	    !IS_ENABLED(CONFIG_MMU))
		return true;

	return (size <= limit) && (addr <= (limit - size));
    14dc:	e59f3064 	ldr	r3, [pc, #100]	@ 1548 <_copy_from_user.constprop.0+0x6c>
	return raw_copy_to_user(to, from, n);
}

#ifdef INLINE_COPY_FROM_USER
static inline __must_check unsigned long
_copy_from_user(void *to, const void __user *from, unsigned long n)
    14e0:	e92d4070 	push	{r4, r5, r6, lr}
    14e4:	e1a06000 	mov	r6, r0
    14e8:	e1510003 	cmp	r1, r3
    14ec:	8a00000d 	bhi	1528 <_copy_from_user.constprop.0+0x4c>
	 * load the TLS register only once in every function.
	 *
	 * Clang < 13.0.1 gets this wrong for Thumb2 builds:
	 * https://github.com/ClangBuiltLinux/linux/issues/1485
	 */
	cur = __builtin_thread_pointer();
    14f0:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
#ifdef CONFIG_CPU_CP15_MMU
static __always_inline unsigned int get_domain(void)
{
	unsigned int domain;

	asm(
    14f4:	ee135f10 	mrc	15, 0, r5, cr3, cr0, {0}
{
#ifdef CONFIG_CPU_SW_DOMAIN_PAN
	unsigned int old_domain = get_domain();

	/* Set the current domain access to permit user accesses */
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    14f8:	e3c5300c 	bic	r3, r5, #12
    14fc:	e3833004 	orr	r3, r3, #4
	return domain;
}

static __always_inline void set_domain(unsigned int val)
{
	asm volatile(
    1500:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	"mcr	p15, 0, %0, c3, c0	@ set domain"
	  : : "r" (val) : "memory");
	isb();
    1504:	f57ff06f 	isb	sy
raw_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	unsigned int __ua_flags;

	__ua_flags = uaccess_save_and_enable();
	n = arm_copy_from_user(to, from, n);
    1508:	e3a0200c 	mov	r2, #12
    150c:	ebfffffe 	bl	0 <arm_copy_from_user>
    1510:	e1a04000 	mov	r4, r0
	asm volatile(
    1514:	ee035f10 	mcr	15, 0, r5, cr3, cr0, {0}
	isb();
    1518:	f57ff06f 	isb	sy
	if (!should_fail_usercopy() && likely(access_ok(from, n))) {
		instrument_copy_from_user_before(to, from, n);
		res = raw_copy_from_user(to, from, n);
		instrument_copy_from_user_after(to, from, n, res);
	}
	if (unlikely(res))
    151c:	e3500000 	cmp	r0, #0
    1520:	0a000006 	beq	1540 <_copy_from_user.constprop.0+0x64>
    1524:	ea000000 	b	152c <_copy_from_user.constprop.0+0x50>
	unsigned long res = n;
    1528:	e3a0400c 	mov	r4, #12
		memset(to + (n - res), 0, res);
    152c:	e264000c 	rsb	r0, r4, #12
    1530:	e1a02004 	mov	r2, r4
    1534:	e3a01000 	mov	r1, #0
    1538:	e0860000 	add	r0, r6, r0
    153c:	ebfffffe 	bl	0 <memset>
	return res;
}
    1540:	e1a00004 	mov	r0, r4
    1544:	e8bd8070 	pop	{r4, r5, r6, pc}
    1548:	befffff4 	.word	0xbefffff4

0000154c <nss_gmac_cadj>:
{
    154c:	e92d4ff7 	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr}
    1550:	e1a07000 	mov	r7, r0
	int64_t offset = 0;
    1554:	e3a01000 	mov	r1, #0
    1558:	e3a00000 	mov	r0, #0
	if (!count) {
    155c:	e2538000 	subs	r8, r3, #0
	int64_t offset = 0;
    1560:	e1cd00f0 	strd	r0, [sp]
		pr_err("%s: Invalid input buffer.\n", __func__);
    1564:	059f1134 	ldreq	r1, [pc, #308]	@ 16a0 <nss_gmac_cadj+0x154>
    1568:	059f0134 	ldreq	r0, [pc, #308]	@ 16a4 <nss_gmac_cadj+0x158>
    156c:	0a000009 	beq	1598 <nss_gmac_cadj+0x4c>
    1570:	e1a06002 	mov	r6, r2
	if (sscanf(buf, "%lld", &offset) != 1) {
    1574:	e59f112c 	ldr	r1, [pc, #300]	@ 16a8 <nss_gmac_cadj+0x15c>
    1578:	e1a0200d 	mov	r2, sp
    157c:	e1a00006 	mov	r0, r6
    1580:	ebfffffe 	bl	0 <sscanf>
    1584:	e3500001 	cmp	r0, #1
	addr = (uint32_t)regbase + regoffset;
    1588:	0597e200 	ldreq	lr, [r7, #512]	@ 0x200
    158c:	0a000004 	beq	15a4 <nss_gmac_cadj+0x58>
		pr_err("%s: bad offset value.\n", __func__);
    1590:	e59f1108 	ldr	r1, [pc, #264]	@ 16a0 <nss_gmac_cadj+0x154>
    1594:	e59f0110 	ldr	r0, [pc, #272]	@ 16ac <nss_gmac_cadj+0x160>
    1598:	ebfffffe 	bl	0 <_printk>
		return -EINVAL;
    159c:	e3e00015 	mvn	r0, #21
    15a0:	ea00003a 	b	1690 <nss_gmac_cadj+0x144>
	asm volatile("ldr %0, %1"
    15a4:	e59e2708 	ldr	r2, [lr, #1800]	@ 0x708
    15a8:	e59e370c 	ldr	r3, [lr, #1804]	@ 0x70c
    15ac:	e59e0708 	ldr	r0, [lr, #1800]	@ 0x708
	} while ((ts_hi !=  nss_gmac_read_reg(gmacdev->mac_base, gmac_ts_high) && timeout > 0));
    15b0:	e1500002 	cmp	r0, r2
    15b4:	1afffffa 	bne	15a4 <nss_gmac_cadj+0x58>
	newOffset = (((uint64_t) ts_hi * BILLION) + (uint64_t) ts_lo) + offset;
    15b8:	e89d1004 	ldm	sp, {r2, ip}
    15bc:	e59f90ec 	ldr	r9, [pc, #236]	@ 16b0 <nss_gmac_cadj+0x164>
			"mov	%Q0, #0"
			: "+&r" (res)
			: "r" (m), "r" (n)
			: "cc");
	} else {
		asm (	"umull	%Q0, %R0, %Q2, %Q3\n\t"
    15c0:	e28f50d0 	add	r5, pc, #208	@ 0xd0
    15c4:	e1c540d0 	ldrd	r4, [r5]
    15c8:	e0931002 	adds	r1, r3, r2
    15cc:	e2acc000 	adc	ip, ip, #0
    15d0:	e0ac1990 	umlal	r1, ip, r0, r9
    15d4:	e1a0b00c 	mov	fp, ip
    15d8:	e1a0a001 	mov	sl, r1
	register unsigned int tmp asm("ip") = 0;
    15dc:	e3a0c000 	mov	ip, #0
		asm (	"umull	%Q0, %R0, %Q2, %Q3\n\t"
    15e0:	e0810a94 	umull	r0, r1, r4, sl
    15e4:	e1700004 	cmn	r0, r4
    15e8:	e0b11005 	adcs	r1, r1, r5
    15ec:	e2ac0000 	adc	r0, ip, #0
			"umlal	%Q0, %R0, %R1, %R2"
			: "+&r" (res)
			: "r" (m), "r" (n)
			: "cc");
	} else {
		asm (	"umlal	%R0, %Q0, %R2, %Q3\n\t"
    15f0:	e0a01a95 	umlal	r1, r0, r5, sl
    15f4:	e0ac1b94 	umlal	r1, ip, r4, fp
    15f8:	e3a01000 	mov	r1, #0
    15fc:	e09c0000 	adds	r0, ip, r0
    1600:	e2a11000 	adc	r1, r1, #0
    1604:	e0a10b95 	umlal	r0, r1, r5, fp
	nsec = do_div(newOffset, BILLION);
    1608:	e1a03ea0 	lsr	r3, r0, #29
    160c:	e1833181 	orr	r3, r3, r1, lsl #3
    1610:	e062a399 	mls	r2, r9, r3, sl
	asm volatile("str %1, %0"
    1614:	e58e3710 	str	r3, [lr, #1808]	@ 0x710
    1618:	e58e2714 	str	r2, [lr, #1812]	@ 0x714
	timeout = TSTAMP_LOOP_VARIABLE;
    161c:	e3a02ffa 	mov	r2, #1000	@ 0x3e8
		timeout--;
    1620:	e2422001 	sub	r2, r2, #1
	asm volatile("ldr %0, %1"
    1624:	e59e3700 	ldr	r3, [lr, #1792]	@ 0x700
	} while ((data & gmac_ts_init_mask) && timeout > 0);
    1628:	e3130004 	tst	r3, #4
    162c:	0a000002 	beq	163c <nss_gmac_cadj+0xf0>
    1630:	e3520000 	cmp	r2, #0
    1634:	1afffff9 	bne	1620 <nss_gmac_cadj+0xd4>
    1638:	ea000001 	b	1644 <nss_gmac_cadj+0xf8>
	if (timeout == 0) {
    163c:	e3520000 	cmp	r2, #0
    1640:	1a000005 	bne	165c <nss_gmac_cadj+0x110>
		netdev_info(gmacdev->netdev, "%s: Coarse Adjustment timed out\n", __func__);
    1644:	e5970290 	ldr	r0, [r7, #656]	@ 0x290
    1648:	e59f2050 	ldr	r2, [pc, #80]	@ 16a0 <nss_gmac_cadj+0x154>
    164c:	e59f1060 	ldr	r1, [pc, #96]	@ 16b4 <nss_gmac_cadj+0x168>
    1650:	ebfffffe 	bl	0 <netdev_info>
		return -1;
    1654:	e3e00000 	mvn	r0, #0
    1658:	ea00000c 	b	1690 <nss_gmac_cadj+0x144>
	nss_gmac_write_reg(gmacdev->mac_base, gmac_ts_control, data | gmac_ts_init_mask);
    165c:	e3833004 	orr	r3, r3, #4
	asm volatile("str %1, %0"
    1660:	e58e3700 	str	r3, [lr, #1792]	@ 0x700
	ret = __real_strnlen(p, maxlen < p_size ? maxlen : p_size);
    1664:	e1a01008 	mov	r1, r8
    1668:	e1a00006 	mov	r0, r6
    166c:	ebfffffe 	bl	0 <strnlen>
	if (p_size <= ret && maxlen != ret)
    1670:	e0588000 	subs	r8, r8, r0
    1674:	13a08001 	movne	r8, #1
    1678:	e3700001 	cmn	r0, #1
    167c:	13a08000 	movne	r8, #0
    1680:	e3580000 	cmp	r8, #0
    1684:	0a000001 	beq	1690 <nss_gmac_cadj+0x144>
		fortify_panic(__func__);
    1688:	e59f0028 	ldr	r0, [pc, #40]	@ 16b8 <nss_gmac_cadj+0x16c>
    168c:	ebfffffe 	bl	0 <fortify_panic>
}
    1690:	e28dd00c 	add	sp, sp, #12
    1694:	e8bd8ff0 	pop	{r4, r5, r6, r7, r8, r9, sl, fp, pc}
    1698:	36b4a597 	.word	0x36b4a597
    169c:	89705f41 	.word	0x89705f41
    16a0:	00000132 	.word	0x00000132
    16a4:	00000455 	.word	0x00000455
    16a8:	000004b3 	.word	0x000004b3
    16ac:	000004b8 	.word	0x000004b8
    16b0:	3b9aca00 	.word	0x3b9aca00
    16b4:	000004d1 	.word	0x000004d1
    16b8:	0000012a 	.word	0x0000012a

000016bc <nss_gmac_do_common_init.isra.0>:
static int32_t nss_gmac_do_common_init(struct platform_device *pdev)
    16bc:	e92d4030 	push	{r4, r5, lr}
    16c0:	e24dd064 	sub	sp, sp, #100	@ 0x64
	struct resource res_nss_base = {0};
    16c4:	e3a02020 	mov	r2, #32
    16c8:	e3a01000 	mov	r1, #0
    16cc:	e1a0000d 	mov	r0, sp
    16d0:	ebfffffe 	bl	0 <memset>
	struct resource res_qsgmii_base = {0};
    16d4:	e3a02020 	mov	r2, #32
    16d8:	e3a01000 	mov	r1, #0
    16dc:	e08d0002 	add	r0, sp, r2
    16e0:	ebfffffe 	bl	0 <memset>
	struct resource res_clk_ctl_base = {0};
    16e4:	e28d0040 	add	r0, sp, #64	@ 0x40
    16e8:	e3a02020 	mov	r2, #32
    16ec:	e3a01000 	mov	r1, #0
    16f0:	ebfffffe 	bl	0 <memset>
	common_device_node = of_find_node_by_name(NULL, NSS_GMAC_COMMON_DEVICE_NODE);
    16f4:	e59f1188 	ldr	r1, [pc, #392]	@ 1884 <nss_gmac_do_common_init.isra.0+0x1c8>
    16f8:	e3a00000 	mov	r0, #0
    16fc:	ebfffffe 	bl	0 <of_find_node_by_name>
	if (!common_device_node) {
    1700:	e2504000 	subs	r4, r0, #0
    1704:	1a000002 	bne	1714 <nss_gmac_do_common_init.isra.0+0x58>
		pr_info("Cannot find device tree node "NSS_GMAC_COMMON_DEVICE_NODE" \n");
    1708:	e59f0178 	ldr	r0, [pc, #376]	@ 1888 <nss_gmac_do_common_init.isra.0+0x1cc>
    170c:	ebfffffe 	bl	0 <_printk>
		goto nss_gmac_cmn_init_fail;
    1710:	ea000004 	b	1728 <nss_gmac_do_common_init.isra.0+0x6c>
	if (of_address_to_resource(common_device_node, 0, &res_nss_base) != 0) {
    1714:	e1a0200d 	mov	r2, sp
    1718:	e3a01000 	mov	r1, #0
    171c:	ebfffffe 	bl	0 <of_address_to_resource>
    1720:	e3500000 	cmp	r0, #0
    1724:	0a000001 	beq	1730 <nss_gmac_do_common_init.isra.0+0x74>
		ret = -EFAULT;
    1728:	e3e0500d 	mvn	r5, #13
    172c:	ea00004e 	b	186c <nss_gmac_do_common_init.isra.0+0x1b0>
	if (of_address_to_resource(common_device_node, 1, &res_qsgmii_base) != 0) {
    1730:	e28d2020 	add	r2, sp, #32
    1734:	e3a01001 	mov	r1, #1
    1738:	e1a00004 	mov	r0, r4
    173c:	ebfffffe 	bl	0 <of_address_to_resource>
    1740:	e3500000 	cmp	r0, #0
    1744:	1afffff7 	bne	1728 <nss_gmac_do_common_init.isra.0+0x6c>
	if (of_address_to_resource(common_device_node, 2, &res_clk_ctl_base) != 0) {
    1748:	e28d2040 	add	r2, sp, #64	@ 0x40
    174c:	e3a01002 	mov	r1, #2
    1750:	e1a00004 	mov	r0, r4
    1754:	ebfffffe 	bl	0 <of_address_to_resource>
    1758:	e3500000 	cmp	r0, #0
    175c:	1afffff1 	bne	1728 <nss_gmac_do_common_init.isra.0+0x6c>
	ctx.msm_clk_ctl_enabled = !!of_machine_is_compatible("qcom,ipq8064");
    1760:	e59f0124 	ldr	r0, [pc, #292]	@ 188c <nss_gmac_do_common_init.isra.0+0x1d0>
    1764:	ebfffffe 	bl	0 <of_machine_is_compatible>
    1768:	e59f4120 	ldr	r4, [pc, #288]	@ 1890 <nss_gmac_do_common_init.isra.0+0x1d4>
    176c:	e2500000 	subs	r0, r0, #0
    1770:	13a00001 	movne	r0, #1
int adjust_resource(struct resource *res, resource_size_t start,
		    resource_size_t size);
resource_size_t resource_alignment(struct resource *res);
static inline resource_size_t resource_size(const struct resource *res)
{
	return res->end - res->start + 1;
    1774:	e59d1004 	ldr	r1, [sp, #4]
    1778:	e5c40018 	strb	r0, [r4, #24]
	ctx.nss_base = (uint8_t *)ioremap(res_nss_base.start,
    177c:	e59d0000 	ldr	r0, [sp]
    1780:	e2811001 	add	r1, r1, #1
    1784:	e0411000 	sub	r1, r1, r0
    1788:	ebfffffe 	bl	0 <ioremap>
	if (!ctx.nss_base) {
    178c:	e3500000 	cmp	r0, #0
	ctx.nss_base = (uint8_t *)ioremap(res_nss_base.start,
    1790:	e5840008 	str	r0, [r4, #8]
	if (!ctx.nss_base) {
    1794:	1a000003 	bne	17a8 <nss_gmac_do_common_init.isra.0+0xec>
		pr_info("Error mapping NSS GMAC registers\n");
    1798:	e59f00f4 	ldr	r0, [pc, #244]	@ 1894 <nss_gmac_do_common_init.isra.0+0x1d8>
		ret = -EIO;
    179c:	e3e05004 	mvn	r5, #4
		pr_info("Error mapping NSS GMAC registers\n");
    17a0:	ebfffffe 	bl	0 <_printk>
		goto nss_gmac_cmn_init_fail;
    17a4:	ea000030 	b	186c <nss_gmac_do_common_init.isra.0+0x1b0>
    17a8:	e1cd02d0 	ldrd	r0, [sp, #32]
    17ac:	e2811001 	add	r1, r1, #1
	ctx.qsgmii_base = (uint32_t *)ioremap(res_qsgmii_base.start,
    17b0:	e0411000 	sub	r1, r1, r0
    17b4:	ebfffffe 	bl	0 <ioremap>
	if (!ctx.qsgmii_base) {
    17b8:	e3500000 	cmp	r0, #0
	ctx.qsgmii_base = (uint32_t *)ioremap(res_qsgmii_base.start,
    17bc:	e584000c 	str	r0, [r4, #12]
	if (!ctx.qsgmii_base) {
    17c0:	1a000003 	bne	17d4 <nss_gmac_do_common_init.isra.0+0x118>
		pr_info("Error mapping QSGMII registers\n");
    17c4:	e59f00cc 	ldr	r0, [pc, #204]	@ 1898 <nss_gmac_do_common_init.isra.0+0x1dc>
		ret = -EIO;
    17c8:	e3e05004 	mvn	r5, #4
		pr_info("Error mapping QSGMII registers\n");
    17cc:	ebfffffe 	bl	0 <_printk>
		goto nss_gmac_qsgmii_map_err;
    17d0:	ea000021 	b	185c <nss_gmac_do_common_init.isra.0+0x1a0>
    17d4:	e1cd04d0 	ldrd	r0, [sp, #64]	@ 0x40
    17d8:	e2811001 	add	r1, r1, #1
	ctx.clk_ctl_base = (uint32_t *)ioremap(res_clk_ctl_base.start,
    17dc:	e0411000 	sub	r1, r1, r0
    17e0:	ebfffffe 	bl	0 <ioremap>
	if (!ctx.clk_ctl_base) {
    17e4:	e3500000 	cmp	r0, #0
	ctx.clk_ctl_base = (uint32_t *)ioremap(res_clk_ctl_base.start,
    17e8:	e5840010 	str	r0, [r4, #16]
	if (!ctx.clk_ctl_base) {
    17ec:	1a000003 	bne	1800 <nss_gmac_do_common_init.isra.0+0x144>
		pr_info("Error mapping Clk control registers\n");
    17f0:	e59f00a4 	ldr	r0, [pc, #164]	@ 189c <nss_gmac_do_common_init.isra.0+0x1e0>
		ret = -EIO;
    17f4:	e3e05004 	mvn	r5, #4
		pr_info("Error mapping Clk control registers\n");
    17f8:	ebfffffe 	bl	0 <_printk>
		goto nss_gmac_clkctl_map_err;
    17fc:	ea000012 	b	184c <nss_gmac_do_common_init.isra.0+0x190>
	ctx.gmac_ctl_table_header = register_net_sysctl(&init_net,
    1800:	e59f2098 	ldr	r2, [pc, #152]	@ 18a0 <nss_gmac_do_common_init.isra.0+0x1e4>
    1804:	e59f1098 	ldr	r1, [pc, #152]	@ 18a4 <nss_gmac_do_common_init.isra.0+0x1e8>
    1808:	e59f0098 	ldr	r0, [pc, #152]	@ 18a8 <nss_gmac_do_common_init.isra.0+0x1ec>
    180c:	ebfffffe 	bl	0 <register_net_sysctl>
	if (!ctx.gmac_ctl_table_header) {
    1810:	e3500000 	cmp	r0, #0
	ctx.gmac_ctl_table_header = register_net_sysctl(&init_net,
    1814:	e5840030 	str	r0, [r4, #48]	@ 0x30
	if (!ctx.gmac_ctl_table_header) {
    1818:	1a000002 	bne	1828 <nss_gmac_do_common_init.isra.0+0x16c>
		pr_info("Unable to register GMAC sysctl table\n");
    181c:	e59f0088 	ldr	r0, [pc, #136]	@ 18ac <nss_gmac_do_common_init.isra.0+0x1f0>
    1820:	ebfffffe 	bl	0 <_printk>
		goto nss_gmac_clkctl_map_err;
    1824:	ea000007 	b	1848 <nss_gmac_do_common_init.isra.0+0x18c>
	if (nss_gmac_common_init(&ctx) == 0) {
    1828:	e1a00004 	mov	r0, r4
    182c:	ebfffffe 	bl	2dd0 <nss_gmac_common_init>
    1830:	e2505000 	subs	r5, r0, #0
		ctx.common_init_done = true;
    1834:	03a03001 	moveq	r3, #1
    1838:	05c4302c 	strbeq	r3, [r4, #44]	@ 0x2c
	if (nss_gmac_common_init(&ctx) == 0) {
    183c:	0a00000d 	beq	1878 <nss_gmac_do_common_init.isra.0+0x1bc>
	unregister_net_sysctl_table(ctx.gmac_ctl_table_header);
    1840:	e5940030 	ldr	r0, [r4, #48]	@ 0x30
    1844:	ebfffffe 	bl	0 <unregister_net_sysctl_table>
	int32_t ret = -EFAULT;
    1848:	e3e0500d 	mvn	r5, #13
	iounmap(ctx.qsgmii_base);
    184c:	e594000c 	ldr	r0, [r4, #12]
    1850:	ebfffffe 	bl	0 <iounmap>
	ctx.qsgmii_base = NULL;
    1854:	e3a03000 	mov	r3, #0
    1858:	e584300c 	str	r3, [r4, #12]
	iounmap(ctx.nss_base);
    185c:	e5940008 	ldr	r0, [r4, #8]
    1860:	ebfffffe 	bl	0 <iounmap>
	ctx.nss_base = NULL;
    1864:	e3a03000 	mov	r3, #0
    1868:	e5843008 	str	r3, [r4, #8]
	pr_info("%s: platform init fail\n", __func__);
    186c:	e59f103c 	ldr	r1, [pc, #60]	@ 18b0 <nss_gmac_do_common_init.isra.0+0x1f4>
    1870:	e59f003c 	ldr	r0, [pc, #60]	@ 18b4 <nss_gmac_do_common_init.isra.0+0x1f8>
    1874:	ebfffffe 	bl	0 <_printk>
}
    1878:	e1a00005 	mov	r0, r5
    187c:	e28dd064 	add	sp, sp, #100	@ 0x64
    1880:	e8bd8030 	pop	{r4, r5, pc}
    1884:	000004f2 	.word	0x000004f2
    1888:	00000502 	.word	0x00000502
    188c:	00000533 	.word	0x00000533
    1890:	00000000 	.word	0x00000000
    1894:	00000540 	.word	0x00000540
    1898:	00000564 	.word	0x00000564
    189c:	00000586 	.word	0x00000586
    18a0:	00000000 	.word	0x00000000
    18a4:	000005ad 	.word	0x000005ad
    18a8:	00000000 	.word	0x00000000
    18ac:	000005b6 	.word	0x000005b6
    18b0:	00000140 	.word	0x00000140
    18b4:	000005de 	.word	0x000005de

000018b8 <nss_gmac_fadj>:
{
    18b8:	e92d4df3 	push	{r0, r1, r4, r5, r6, r7, r8, sl, fp, lr}
    18bc:	e1a07000 	mov	r7, r0
	uint64_t offset = 0;
    18c0:	e3a01000 	mov	r1, #0
    18c4:	e3a00000 	mov	r0, #0
	if (!count) {
    18c8:	e2538000 	subs	r8, r3, #0
	uint64_t offset = 0;
    18cc:	e1cd00f0 	strd	r0, [sp]
		pr_err("%s: Invalid input buffer.\n", __func__);
    18d0:	059f1110 	ldreq	r1, [pc, #272]	@ 19e8 <nss_gmac_fadj+0x130>
    18d4:	059f0110 	ldreq	r0, [pc, #272]	@ 19ec <nss_gmac_fadj+0x134>
    18d8:	0a000008 	beq	1900 <nss_gmac_fadj+0x48>
    18dc:	e1a06002 	mov	r6, r2
	if (sscanf(buf, "%lld", &offset) != 1) {
    18e0:	e59f1108 	ldr	r1, [pc, #264]	@ 19f0 <nss_gmac_fadj+0x138>
    18e4:	e1a0200d 	mov	r2, sp
    18e8:	e1a00006 	mov	r0, r6
    18ec:	ebfffffe 	bl	0 <sscanf>
    18f0:	e3500001 	cmp	r0, #1
    18f4:	0a000004 	beq	190c <nss_gmac_fadj+0x54>
		pr_err("%s: bad offset value.\n", __func__);
    18f8:	e59f10e8 	ldr	r1, [pc, #232]	@ 19e8 <nss_gmac_fadj+0x130>
    18fc:	e59f00f0 	ldr	r0, [pc, #240]	@ 19f4 <nss_gmac_fadj+0x13c>
    1900:	ebfffffe 	bl	0 <_printk>
		return -EINVAL;
    1904:	e3e00015 	mvn	r0, #21
    1908:	ea000032 	b	19d8 <nss_gmac_fadj+0x120>
	nsec = do_div(offset, BILLION);
    190c:	e1cd00d0 	ldrd	r0, [sp]
	register unsigned int tmp asm("ip") = 0;
    1910:	e3a0c000 	mov	ip, #0
		asm (	"umull	%Q0, %R0, %Q2, %Q3\n\t"
    1914:	e28f50c4 	add	r5, pc, #196	@ 0xc4
    1918:	e1c540d0 	ldrd	r4, [r5]
    191c:	e0832094 	umull	r2, r3, r4, r0
    1920:	e1720004 	cmn	r2, r4
    1924:	e0b33005 	adcs	r3, r3, r5
    1928:	e2ac2000 	adc	r2, ip, #0
		asm (	"umlal	%R0, %Q0, %R2, %Q3\n\t"
    192c:	e1a0a002 	mov	sl, r2
    1930:	e1a0b003 	mov	fp, r3
    1934:	e0aab095 	umlal	fp, sl, r5, r0
    1938:	e0acb194 	umlal	fp, ip, r4, r1
    193c:	e3a0b000 	mov	fp, #0
    1940:	e09ca00a 	adds	sl, ip, sl
    1944:	e2abb000 	adc	fp, fp, #0
    1948:	e0aba195 	umlal	sl, fp, r5, r1
    194c:	e1a02eaa 	lsr	r2, sl, #29
    1950:	e1a03eab 	lsr	r3, fp, #29
    1954:	e182218b 	orr	r2, r2, fp, lsl #3
    1958:	e1cd20f0 	strd	r2, [sp]
    195c:	e59f3094 	ldr	r3, [pc, #148]	@ 19f8 <nss_gmac_fadj+0x140>
    1960:	e0600293 	mls	r0, r3, r2, r0
	addr = (uint32_t)regbase + regoffset;
    1964:	e5973200 	ldr	r3, [r7, #512]	@ 0x200
    1968:	e5832710 	str	r2, [r3, #1808]	@ 0x710
    196c:	e5830714 	str	r0, [r3, #1812]	@ 0x714
	timeout = TSTAMP_LOOP_VARIABLE;
    1970:	e3a01ffa 	mov	r1, #1000	@ 0x3e8
		timeout--;
    1974:	e2411001 	sub	r1, r1, #1
	asm volatile("ldr %0, %1"
    1978:	e5932700 	ldr	r2, [r3, #1792]	@ 0x700
	} while (data & gmac_ts_updt_mask);
    197c:	e3120008 	tst	r2, #8
    1980:	1afffffb 	bne	1974 <nss_gmac_fadj+0xbc>
	if (timeout == 0) {
    1984:	e3510000 	cmp	r1, #0
    1988:	1a000005 	bne	19a4 <nss_gmac_fadj+0xec>
		netdev_info(gmacdev->netdev, "%s: Fine Adjustment timed out\n", __func__);
    198c:	e5970290 	ldr	r0, [r7, #656]	@ 0x290
    1990:	e59f2050 	ldr	r2, [pc, #80]	@ 19e8 <nss_gmac_fadj+0x130>
    1994:	e59f1060 	ldr	r1, [pc, #96]	@ 19fc <nss_gmac_fadj+0x144>
    1998:	ebfffffe 	bl	0 <netdev_info>
		return -1;
    199c:	e3e00000 	mvn	r0, #0
    19a0:	ea00000c 	b	19d8 <nss_gmac_fadj+0x120>
	nss_gmac_write_reg(gmacdev->mac_base, gmac_ts_control, data | gmac_ts_updt_mask);
    19a4:	e3822008 	orr	r2, r2, #8
	asm volatile("str %1, %0"
    19a8:	e5832700 	str	r2, [r3, #1792]	@ 0x700
	ret = __real_strnlen(p, maxlen < p_size ? maxlen : p_size);
    19ac:	e1a01008 	mov	r1, r8
    19b0:	e1a00006 	mov	r0, r6
    19b4:	ebfffffe 	bl	0 <strnlen>
	if (p_size <= ret && maxlen != ret)
    19b8:	e0588000 	subs	r8, r8, r0
    19bc:	13a08001 	movne	r8, #1
    19c0:	e3700001 	cmn	r0, #1
    19c4:	13a08000 	movne	r8, #0
    19c8:	e3580000 	cmp	r8, #0
    19cc:	0a000001 	beq	19d8 <nss_gmac_fadj+0x120>
		fortify_panic(__func__);
    19d0:	e59f0028 	ldr	r0, [pc, #40]	@ 1a00 <nss_gmac_fadj+0x148>
    19d4:	ebfffffe 	bl	0 <fortify_panic>
}
    19d8:	e28dd008 	add	sp, sp, #8
    19dc:	e8bd8df0 	pop	{r4, r5, r6, r7, r8, sl, fp, pc}
    19e0:	36b4a597 	.word	0x36b4a597
    19e4:	89705f41 	.word	0x89705f41
    19e8:	00000158 	.word	0x00000158
    19ec:	00000455 	.word	0x00000455
    19f0:	000004b3 	.word	0x000004b3
    19f4:	000004b8 	.word	0x000004b8
    19f8:	3b9aca00 	.word	0x3b9aca00
    19fc:	000005f8 	.word	0x000005f8
    1a00:	0000012a 	.word	0x0000012a

00001a04 <nss_gmac_set_mac_address>:
{
    1a04:	e92d40f0 	push	{r4, r5, r6, r7, lr}
	BUG_ON(gmacdev == NULL);
    1a08:	e2907d15 	adds	r7, r0, #1344	@ 0x540
{
    1a0c:	e24dd024 	sub	sp, sp, #36	@ 0x24
	BUG_ON(gmacdev == NULL);
    1a10:	0a000002 	beq	1a20 <nss_gmac_set_mac_address+0x1c>
	BUG_ON(gmacdev->netdev != netdev);
    1a14:	e59065d0 	ldr	r6, [r0, #1488]	@ 0x5d0
    1a18:	e1560000 	cmp	r6, r0
    1a1c:	0a000000 	beq	1a24 <nss_gmac_set_mac_address+0x20>
    1a20:	e7f001f2 	.word	0xe7f001f2
	netdev_info(netdev, "%s: AddrFamily: %d, %0x:%0x:%0x:%0x:%0x:%0x\n",
    1a24:	e5d13007 	ldrb	r3, [r1, #7]
    1a28:	e1a04001 	mov	r4, r1
    1a2c:	e59f209c 	ldr	r2, [pc, #156]	@ 1ad0 <nss_gmac_set_mac_address+0xcc>
    1a30:	e59f109c 	ldr	r1, [pc, #156]	@ 1ad4 <nss_gmac_set_mac_address+0xd0>
    1a34:	e58d3014 	str	r3, [sp, #20]
    1a38:	e5d43006 	ldrb	r3, [r4, #6]
    1a3c:	e58d3010 	str	r3, [sp, #16]
    1a40:	e5d43005 	ldrb	r3, [r4, #5]
    1a44:	e58d300c 	str	r3, [sp, #12]
    1a48:	e5d43004 	ldrb	r3, [r4, #4]
    1a4c:	e58d3008 	str	r3, [sp, #8]
    1a50:	e5d43003 	ldrb	r3, [r4, #3]
    1a54:	e58d3004 	str	r3, [sp, #4]
    1a58:	e5d43002 	ldrb	r3, [r4, #2]
    1a5c:	e58d3000 	str	r3, [sp]
    1a60:	e1d430b0 	ldrh	r3, [r4]
    1a64:	ebfffffe 	bl	0 <netdev_info>
 * By definition the broadcast address is also a multicast address.
 */
static inline bool is_multicast_ether_addr(const u8 *addr)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
	u32 a = *(const u32 *)addr;
    1a68:	e5943002 	ldr	r3, [r4, #2]
 */
static inline bool is_valid_ether_addr(const u8 *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
    1a6c:	e2135001 	ands	r5, r3, #1
    1a70:	1a000013 	bne	1ac4 <nss_gmac_set_mac_address+0xc0>
	return ((*(const u32 *)addr) | (*(const u16 *)(addr + 4))) == 0;
    1a74:	e1d420b6 	ldrh	r2, [r4, #6]
	if (!is_valid_ether_addr(addr->sa_data))
    1a78:	e1922003 	orrs	r2, r2, r3
    1a7c:	0a000010 	beq	1ac4 <nss_gmac_set_mac_address+0xc0>
	nss_gmac_set_mac_addr(gmacdev, gmac_addr0_high, gmac_addr0_low,
    1a80:	e2843002 	add	r3, r4, #2
    1a84:	e3a02044 	mov	r2, #68	@ 0x44
    1a88:	e3a01040 	mov	r1, #64	@ 0x40
    1a8c:	e1a00007 	mov	r0, r7
    1a90:	ebfffffe 	bl	2ec <nss_gmac_twokpe_frame_enable>
	nss_gmac_get_mac_addr(gmacdev, gmac_addr0_high, gmac_addr0_low,
    1a94:	e28d301a 	add	r3, sp, #26
    1a98:	e3a02044 	mov	r2, #68	@ 0x44
    1a9c:	e3a01040 	mov	r1, #64	@ 0x40
    1aa0:	e1a00007 	mov	r0, r7
    1aa4:	ebfffffe 	bl	b1c <nss_gmac_get_mac_addr>
		  const void *addr, size_t len);

static inline void
__dev_addr_set(struct net_device *dev, const void *addr, size_t len)
{
	dev_addr_mod(dev, 0, addr, len);
    1aa8:	e28d201a 	add	r2, sp, #26
    1aac:	e1a00006 	mov	r0, r6
    1ab0:	e3a03006 	mov	r3, #6
    1ab4:	e1a01005 	mov	r1, r5
    1ab8:	ebfffffe 	bl	0 <dev_addr_mod>
	return 0;
    1abc:	e1a00005 	mov	r0, r5
}
    1ac0:	ea000000 	b	1ac8 <nss_gmac_set_mac_address+0xc4>
		return -EADDRNOTAVAIL;
    1ac4:	e3e00062 	mvn	r0, #98	@ 0x62
}
    1ac8:	e28dd024 	add	sp, sp, #36	@ 0x24
    1acc:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
    1ad0:	00000166 	.word	0x00000166
    1ad4:	00000617 	.word	0x00000617

00001ad8 <nss_gmac_probe>:
{
    1ad8:	e92d43f0 	push	{r4, r5, r6, r7, r8, r9, lr}
    1adc:	e1a07000 	mov	r7, r0
	if (ctx.common_init_done == false) {
    1ae0:	e59f8884 	ldr	r8, [pc, #2180]	@ 236c <nss_gmac_probe+0x894>
{
    1ae4:	e24dd08c 	sub	sp, sp, #140	@ 0x8c
	if (ctx.common_init_done == false) {
    1ae8:	e5d8302c 	ldrb	r3, [r8, #44]	@ 0x2c
    1aec:	e3530000 	cmp	r3, #0
    1af0:	0a000006 	beq	1b10 <nss_gmac_probe+0x38>
	netdev = alloc_etherdev(sizeof(struct nss_gmac_dev));
    1af4:	e3a02001 	mov	r2, #1
    1af8:	e3a00e53 	mov	r0, #1328	@ 0x530
    1afc:	e1a01002 	mov	r1, r2
    1b00:	ebfffffe 	bl	0 <alloc_etherdev_mqs>
	if (!netdev) {
    1b04:	e2504000 	subs	r4, r0, #0
    1b08:	1a000009 	bne	1b34 <nss_gmac_probe+0x5c>
    1b0c:	ea000003 	b	1b20 <nss_gmac_probe+0x48>
		ret = nss_gmac_do_common_init(pdev);
    1b10:	ebfffee9 	bl	16bc <nss_gmac_do_common_init.isra.0>
		if (ret != 0)
    1b14:	e2506000 	subs	r6, r0, #0
    1b18:	0afffff5 	beq	1af4 <nss_gmac_probe+0x1c>
    1b1c:	ea00020f 	b	2360 <nss_gmac_probe+0x888>
		pr_info("%s: alloc_etherdev() failed\n", __func__);
    1b20:	e59f1848 	ldr	r1, [pc, #2120]	@ 2370 <nss_gmac_probe+0x898>
		return -ENOMEM;
    1b24:	e3e0600b 	mvn	r6, #11
		pr_info("%s: alloc_etherdev() failed\n", __func__);
    1b28:	e59f0844 	ldr	r0, [pc, #2116]	@ 2374 <nss_gmac_probe+0x89c>
    1b2c:	ebfffffe 	bl	0 <_printk>
		return -ENOMEM;
    1b30:	ea00020a 	b	2360 <nss_gmac_probe+0x888>
	netdev->max_mtu = ETH_MAX_MTU;
    1b34:	e30f3fff 	movw	r3, #65535	@ 0xffff
	return (char *)dev + ALIGN(sizeof(struct net_device), NETDEV_ALIGN);
    1b38:	e2845d15 	add	r5, r4, #1344	@ 0x540
	memset((void *)gmacdev, 0, sizeof(struct nss_gmac_dev));
    1b3c:	e3a02e53 	mov	r2, #1328	@ 0x530
    1b40:	e3a01000 	mov	r1, #0
    1b44:	e1a00005 	mov	r0, r5
	netdev->max_mtu = ETH_MAX_MTU;
    1b48:	e58430cc 	str	r3, [r4, #204]	@ 0xcc
	memset((void *)gmacdev, 0, sizeof(struct nss_gmac_dev));
    1b4c:	ebfffffe 	bl	0 <memset>
	mutex_init(&gmacdev->link_mutex);
    1b50:	e59f2820 	ldr	r2, [pc, #2080]	@ 2378 <nss_gmac_probe+0x8a0>
    1b54:	e2840e7b 	add	r0, r4, #1968	@ 0x7b0
    1b58:	e59f181c 	ldr	r1, [pc, #2076]	@ 237c <nss_gmac_probe+0x8a4>
    1b5c:	ebfffffe 	bl	0 <__mutex_init>
	gmacdev->loop_back_mode = NOLOOPBACK;
    1b60:	e3a01000 	mov	r1, #0
	struct resource memres_devtree = {0};
    1b64:	e3a02020 	mov	r2, #32
    1b68:	e28d0048 	add	r0, sp, #72	@ 0x48
	gmacdev->loop_back_mode = NOLOOPBACK;
    1b6c:	e58415b0 	str	r1, [r4, #1456]	@ 0x5b0
	gmacdev->netdev = netdev;
    1b70:	e58445d0 	str	r4, [r4, #1488]	@ 0x5d0
	gmacdev->pdev = pdev;
    1b74:	e58475d4 	str	r7, [r4, #1492]	@ 0x5d4
	np = of_node_get(pdev->dev.of_node);
    1b78:	e59760e8 	ldr	r6, [r7, #232]	@ 0xe8
	struct resource memres_devtree = {0};
    1b7c:	ebfffffe 	bl	0 <memset>
	if (of_property_read_u32(np, "qcom,id", &gmacdev->macid)
    1b80:	e59f17f8 	ldr	r1, [pc, #2040]	@ 2380 <nss_gmac_probe+0x8a8>
    1b84:	e285200c 	add	r2, r5, #12
    1b88:	e1a00006 	mov	r0, r6
    1b8c:	ebfffe3a 	bl	147c <of_property_read_u32>
    1b90:	e3500000 	cmp	r0, #0
    1b94:	0a000002 	beq	1ba4 <nss_gmac_probe+0xcc>
		pr_err("%s: error reading critical device node properties\n", np->name);
    1b98:	e5961000 	ldr	r1, [r6]
    1b9c:	e59f07e0 	ldr	r0, [pc, #2016]	@ 2384 <nss_gmac_probe+0x8ac>
    1ba0:	ea000037 	b	1c84 <nss_gmac_probe+0x1ac>
		|| of_property_read_u32(np, "qcom,phy-mdio-addr",
    1ba4:	e59f17dc 	ldr	r1, [pc, #2012]	@ 2388 <nss_gmac_probe+0x8b0>
    1ba8:	e28d2020 	add	r2, sp, #32
    1bac:	e1a00006 	mov	r0, r6
    1bb0:	ebfffe31 	bl	147c <of_property_read_u32>
    1bb4:	e3500000 	cmp	r0, #0
    1bb8:	1afffff6 	bne	1b98 <nss_gmac_probe+0xc0>
		|| of_property_read_u32(np, "qcom,rgmii-delay",
    1bbc:	e59f17c8 	ldr	r1, [pc, #1992]	@ 238c <nss_gmac_probe+0x8b4>
    1bc0:	e28d2028 	add	r2, sp, #40	@ 0x28
    1bc4:	e1a00006 	mov	r0, r6
    1bc8:	ebfffe2b 	bl	147c <of_property_read_u32>
    1bcc:	e3500000 	cmp	r0, #0
    1bd0:	1afffff0 	bne	1b98 <nss_gmac_probe+0xc0>
		|| of_property_read_u32(np, "qcom,poll-required",
    1bd4:	e59f17b4 	ldr	r1, [pc, #1972]	@ 2390 <nss_gmac_probe+0x8b8>
    1bd8:	e28d2024 	add	r2, sp, #36	@ 0x24
    1bdc:	e1a00006 	mov	r0, r6
    1be0:	ebfffe25 	bl	147c <of_property_read_u32>
    1be4:	e3500000 	cmp	r0, #0
    1be8:	1affffea 	bne	1b98 <nss_gmac_probe+0xc0>
		|| of_property_read_u32(np, "qcom,forced-speed",
    1bec:	e59f17a0 	ldr	r1, [pc, #1952]	@ 2394 <nss_gmac_probe+0x8bc>
    1bf0:	e28d2038 	add	r2, sp, #56	@ 0x38
    1bf4:	e1a00006 	mov	r0, r6
    1bf8:	ebfffe1f 	bl	147c <of_property_read_u32>
    1bfc:	e3500000 	cmp	r0, #0
    1c00:	1affffe4 	bne	1b98 <nss_gmac_probe+0xc0>
		|| of_property_read_u32(np, "qcom,forced-duplex",
    1c04:	e59f178c 	ldr	r1, [pc, #1932]	@ 2398 <nss_gmac_probe+0x8c0>
    1c08:	e28d203c 	add	r2, sp, #60	@ 0x3c
    1c0c:	e1a00006 	mov	r0, r6
    1c10:	ebfffe19 	bl	147c <of_property_read_u32>
    1c14:	e3500000 	cmp	r0, #0
    1c18:	1affffde 	bne	1b98 <nss_gmac_probe+0xc0>
		|| of_property_read_u32(np, "qcom,pcs-chanid",
    1c1c:	e59f1778 	ldr	r1, [pc, #1912]	@ 239c <nss_gmac_probe+0x8c4>
    1c20:	e2842e55 	add	r2, r4, #1360	@ 0x550
    1c24:	e1a00006 	mov	r0, r6
    1c28:	ebfffe13 	bl	147c <of_property_read_u32>
    1c2c:	e2509000 	subs	r9, r0, #0
    1c30:	1affffd8 	bne	1b98 <nss_gmac_probe+0xc0>
	if (of_property_read_u32(np, "qcom,socver", &gmaccfg->socver))
    1c34:	e59f1764 	ldr	r1, [pc, #1892]	@ 23a0 <nss_gmac_probe+0x8c8>
    1c38:	e28d2044 	add	r2, sp, #68	@ 0x44
    1c3c:	e1a00006 	mov	r0, r6
    1c40:	ebfffe0d 	bl	147c <of_property_read_u32>
    1c44:	e3500000 	cmp	r0, #0
	if (of_property_read_u32(np, "qcom,mmds-mask", &gmaccfg->mmds_mask))
    1c48:	e59f1754 	ldr	r1, [pc, #1876]	@ 23a4 <nss_gmac_probe+0x8cc>
    1c4c:	e28d2040 	add	r2, sp, #64	@ 0x40
    1c50:	e1a00006 	mov	r0, r6
		gmaccfg->socver = 0;
    1c54:	158d9044 	strne	r9, [sp, #68]	@ 0x44
	if (of_property_read_u32(np, "qcom,mmds-mask", &gmaccfg->mmds_mask))
    1c58:	ebfffe07 	bl	147c <of_property_read_u32>
    1c5c:	e3500000 	cmp	r0, #0
	gmaccfg->phy_node = of_parse_phandle(np, "phy-handle", 0);
    1c60:	e1a00006 	mov	r0, r6
		gmaccfg->mmds_mask = 0;
    1c64:	13a03000 	movne	r3, #0
    1c68:	158d3040 	strne	r3, [sp, #64]	@ 0x40
	gmaccfg->phy_node = of_parse_phandle(np, "phy-handle", 0);
    1c6c:	ebfffe0a 	bl	149c <of_parse_phandle.constprop.0>
	if (!gmaccfg->phy_node) {
    1c70:	e3500000 	cmp	r0, #0
	gmaccfg->phy_node = of_parse_phandle(np, "phy-handle", 0);
    1c74:	e58d001c 	str	r0, [sp, #28]
	if (!gmaccfg->phy_node) {
    1c78:	1a000003 	bne	1c8c <nss_gmac_probe+0x1b4>
		pr_err("%s: error parsing phy-handle\n", np->name);
    1c7c:	e5961000 	ldr	r1, [r6]
    1c80:	e59f0720 	ldr	r0, [pc, #1824]	@ 23a8 <nss_gmac_probe+0x8d0>
    1c84:	ebfffffe 	bl	0 <_printk>
		return -EFAULT;
    1c88:	ea000024 	b	1d20 <nss_gmac_probe+0x248>
	of_property_read_u32(np, "qcom,aux-clk-freq", &gmacdev->aux_clk_freq);
    1c8c:	e285208c 	add	r2, r5, #140	@ 0x8c
    1c90:	e59f1714 	ldr	r1, [pc, #1812]	@ 23ac <nss_gmac_probe+0x8d4>
    1c94:	e1a00006 	mov	r0, r6
    1c98:	ebfffdf7 	bl	147c <of_property_read_u32>
	of_get_phy_mode(np, &gmaccfg->phy_mii_type);
    1c9c:	e28d102c 	add	r1, sp, #44	@ 0x2c
    1ca0:	e1a00006 	mov	r0, r6
    1ca4:	ebfffffe 	bl	0 <of_get_phy_mode>
	netdev->irq = irq_of_parse_and_map(np, 0);
    1ca8:	e3a01000 	mov	r1, #0
    1cac:	e1a00006 	mov	r0, r6
    1cb0:	ebfffffe 	bl	0 <irq_of_parse_and_map>
	if (!netdev->irq) {
    1cb4:	e3500000 	cmp	r0, #0
	netdev->irq = irq_of_parse_and_map(np, 0);
    1cb8:	e5840190 	str	r0, [r4, #400]	@ 0x190
		pr_err("%s: Can't map interrupt\n", np->name);
    1cbc:	059f06ec 	ldreq	r0, [pc, #1772]	@ 23b0 <nss_gmac_probe+0x8d8>
    1cc0:	05961000 	ldreq	r1, [r6]
	if (!netdev->irq) {
    1cc4:	0affffee 	beq	1c84 <nss_gmac_probe+0x1ac>
	of_get_mac_address(np, gmaccfg->mac_addr);
    1cc8:	e28d1030 	add	r1, sp, #48	@ 0x30
    1ccc:	e1a00006 	mov	r0, r6
    1cd0:	ebfffffe 	bl	0 <of_get_mac_address>
	if (of_address_to_resource(np, 0, &memres_devtree) != 0)
    1cd4:	e28d2048 	add	r2, sp, #72	@ 0x48
    1cd8:	e3a01000 	mov	r1, #0
    1cdc:	e1a00006 	mov	r0, r6
    1ce0:	ebfffffe 	bl	0 <of_address_to_resource>
    1ce4:	e3500000 	cmp	r0, #0
    1ce8:	1a00000c 	bne	1d20 <nss_gmac_probe+0x248>
	netdev->base_addr = memres_devtree.start;
    1cec:	e59d3048 	ldr	r3, [sp, #72]	@ 0x48
    1cf0:	e5843020 	str	r3, [r4, #32]
	gmacdev->phy_mii_type = gmaccfg->phy_mii_type;
    1cf4:	e59d302c 	ldr	r3, [sp, #44]	@ 0x2c
    1cf8:	e58435b4 	str	r3, [r4, #1460]	@ 0x5b4
	gmacdev->phy_base = gmaccfg->phy_mdio_addr;
    1cfc:	e59d3020 	ldr	r3, [sp, #32]
    1d00:	e5843548 	str	r3, [r4, #1352]	@ 0x548
	gmacdev->rgmii_delay = gmaccfg->rgmii_delay;
    1d04:	e59d3028 	ldr	r3, [sp, #40]	@ 0x28
    1d08:	e58435b8 	str	r3, [r4, #1464]	@ 0x5b8
	if (ctx.socver == 0)
    1d0c:	e5983014 	ldr	r3, [r8, #20]
    1d10:	e3530000 	cmp	r3, #0
		ctx.socver = gmaccfg->socver;
    1d14:	059d3044 	ldreq	r3, [sp, #68]	@ 0x44
    1d18:	05883014 	streq	r3, [r8, #20]
	if (ctx.socver == 0)
    1d1c:	ea000003 	b	1d30 <nss_gmac_probe+0x258>
		free_netdev(netdev);
    1d20:	e1a00004 	mov	r0, r4
		return ret;
    1d24:	e3e0600d 	mvn	r6, #13
		free_netdev(netdev);
    1d28:	ebfffffe 	bl	0 <free_netdev>
		return ret;
    1d2c:	ea00018b 	b	2360 <nss_gmac_probe+0x888>
	if (gmaccfg->poll_required) {
    1d30:	e59d3024 	ldr	r3, [sp, #36]	@ 0x24
    1d34:	e3530000 	cmp	r3, #0
    1d38:	0a000005 	beq	1d54 <nss_gmac_probe+0x27c>
		set_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags);
    1d3c:	e2851018 	add	r1, r5, #24
    1d40:	e3a00006 	mov	r0, #6
    1d44:	ebfffffe 	bl	0 <_set_bit>
		gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(LINKPOLL);
    1d48:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
    1d4c:	e3833001 	orr	r3, r3, #1
    1d50:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
	if ((gmaccfg->forced_speed != SPEED_10) && (gmaccfg->forced_speed != SPEED_100)
    1d54:	e59d3038 	ldr	r3, [sp, #56]	@ 0x38
	gmacdev->ctx = &ctx;
    1d58:	e58487c4 	str	r8, [r4, #1988]	@ 0x7c4
	nss_gmac_dev_init(gmacdev);
    1d5c:	e1a00005 	mov	r0, r5
	if ((gmaccfg->forced_speed != SPEED_10) && (gmaccfg->forced_speed != SPEED_100)
    1d60:	e353000a 	cmp	r3, #10
    1d64:	13530064 	cmpne	r3, #100	@ 0x64
    1d68:	13a02001 	movne	r2, #1
    1d6c:	03a02000 	moveq	r2, #0
			&& (gmaccfg->forced_speed != SPEED_1000)) {
    1d70:	e3530ffa 	cmp	r3, #1000	@ 0x3e8
    1d74:	03a02000 	moveq	r2, #0
    1d78:	12022001 	andne	r2, r2, #1
    1d7c:	e3520000 	cmp	r2, #0
		gmacdev->forced_speed = SPEED_UNKNOWN;
    1d80:	13e03000 	mvnne	r3, #0
		gmacdev->forced_speed = gmaccfg->forced_speed;
    1d84:	058435c4 	streq	r3, [r4, #1476]	@ 0x5c4
		gmacdev->forced_duplex = gmaccfg->forced_duplex;
    1d88:	059d303c 	ldreq	r3, [sp, #60]	@ 0x3c
		gmacdev->forced_speed = SPEED_UNKNOWN;
    1d8c:	158435c4 	strne	r3, [r4, #1476]	@ 0x5c4
		gmacdev->forced_duplex = DUPLEX_UNKNOWN;
    1d90:	13a030ff 	movne	r3, #255	@ 0xff
    1d94:	e58435c8 	str	r3, [r4, #1480]	@ 0x5c8
	ctx.nss_gmac[gmacdev->macid] = gmacdev;
    1d98:	e594354c 	ldr	r3, [r4, #1356]	@ 0x54c
    1d9c:	e0888103 	add	r8, r8, r3, lsl #2
    1da0:	e588501c 	str	r5, [r8, #28]
	nss_gmac_dev_init(gmacdev);
    1da4:	ebfffffe 	bl	318c <nss_gmac_dev_init>
			    pdev->resource[0].end - pdev->resource[0].start +
    1da8:	e5972140 	ldr	r2, [r7, #320]	@ 0x140
	if (nss_gmac_attach(gmacdev, netdev->base_addr,
    1dac:	e1a00005 	mov	r0, r5
    1db0:	e5941020 	ldr	r1, [r4, #32]
			    pdev->resource[0].end - pdev->resource[0].start +
    1db4:	e1c220d0 	ldrd	r2, [r2]
    1db8:	e2833001 	add	r3, r3, #1
	if (nss_gmac_attach(gmacdev, netdev->base_addr,
    1dbc:	e0432002 	sub	r2, r3, r2
    1dc0:	ebfffffe 	bl	b60 <nss_gmac_attach>
    1dc4:	e3500000 	cmp	r0, #0
    1dc8:	aa000005 	bge	1de4 <nss_gmac_probe+0x30c>
		netdev_info(netdev, "attach failed for %s\n", netdev->name);
    1dcc:	e59f15e0 	ldr	r1, [pc, #1504]	@ 23b4 <nss_gmac_probe+0x8dc>
    1dd0:	e1a02004 	mov	r2, r4
    1dd4:	e1a00004 	mov	r0, r4
		ret = -EIO;
    1dd8:	e3e06004 	mvn	r6, #4
		netdev_info(netdev, "attach failed for %s\n", netdev->name);
    1ddc:	ebfffffe 	bl	0 <netdev_info>
		goto nss_gmac_attach_fail;
    1de0:	ea00015c 	b	2358 <nss_gmac_probe+0x880>
	prop = of_get_property(np, "mdiobus", NULL);
    1de4:	e59f15cc 	ldr	r1, [pc, #1484]	@ 23b8 <nss_gmac_probe+0x8e0>
    1de8:	e3a02000 	mov	r2, #0
    1dec:	e1a00006 	mov	r0, r6
    1df0:	ebfffffe 	bl	0 <of_get_property>
	if (!prop) {
    1df4:	e3500000 	cmp	r0, #0
		netdev_info(netdev, "cannot get 'mdiobus' property\n");
    1df8:	059f15bc 	ldreq	r1, [pc, #1468]	@ 23bc <nss_gmac_probe+0x8e4>
	if (!prop) {
    1dfc:	0a000005 	beq	1e18 <nss_gmac_probe+0x340>
static __always_inline __u32 __swab32p(const __u32 *p)
{
#ifdef __arch_swab32p
	return __arch_swab32p(p);
#else
	return __swab32(*p);
    1e00:	e5900000 	ldr	r0, [r0]
	mdio_node = of_find_node_by_phandle(be32_to_cpup(prop));
    1e04:	e6bf0f30 	rev	r0, r0
    1e08:	ebfffffe 	bl	0 <of_find_node_by_phandle>
	if (!mdio_node) {
    1e0c:	e3500000 	cmp	r0, #0
    1e10:	1a000004 	bne	1e28 <nss_gmac_probe+0x350>
		netdev_info(netdev, "cannot find mdio node by phandle\n");
    1e14:	e59f15a4 	ldr	r1, [pc, #1444]	@ 23c0 <nss_gmac_probe+0x8e8>
    1e18:	e1a00004 	mov	r0, r4
    1e1c:	ebfffffe 	bl	0 <netdev_info>
		ret = -EIO;
    1e20:	e3e06004 	mvn	r6, #4
    1e24:	ea000149 	b	2350 <nss_gmac_probe+0x878>
	mdio_plat = of_find_device_by_node(mdio_node);
    1e28:	ebfffffe 	bl	0 <of_find_device_by_node>
	if (!mdio_plat) {
    1e2c:	e3500000 	cmp	r0, #0
		netdev_info(netdev, "cannot find platform device from mdio node\n");
    1e30:	059f158c 	ldreq	r1, [pc, #1420]	@ 23c4 <nss_gmac_probe+0x8ec>
	if (!mdio_plat) {
    1e34:	0afffff7 	beq	1e18 <nss_gmac_probe+0x340>
#endif
}

static inline void *dev_get_drvdata(const struct device *dev)
{
	return dev->driver_data;
    1e38:	e5902050 	ldr	r2, [r0, #80]	@ 0x50
	if (!gmacdev->miibus) {
    1e3c:	e3520000 	cmp	r2, #0
	gmacdev->miibus = dev_get_drvdata(&mdio_plat->dev);
    1e40:	e5842a68 	str	r2, [r4, #2664]	@ 0xa68
		netdev_info(netdev, "cannot get mii bus reference from device data\n");
    1e44:	059f157c 	ldreq	r1, [pc, #1404]	@ 23c8 <nss_gmac_probe+0x8f0>
	if (!gmacdev->miibus) {
    1e48:	0afffff2 	beq	1e18 <nss_gmac_probe+0x340>
	netdev_info(netdev, "mdio bus '%s' OK.\n", gmacdev->miibus->id);
    1e4c:	e59f1578 	ldr	r1, [pc, #1400]	@ 23cc <nss_gmac_probe+0x8f4>
    1e50:	e2822008 	add	r2, r2, #8
    1e54:	e1a00004 	mov	r0, r4
    1e58:	ebfffffe 	bl	0 <netdev_info>
	u32 a = *(const u32 *)addr;
    1e5c:	e59d3030 	ldr	r3, [sp, #48]	@ 0x30
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
    1e60:	e2131001 	ands	r1, r3, #1
    1e64:	1a000007 	bne	1e88 <nss_gmac_probe+0x3b0>
	return ((*(const u32 *)addr) | (*(const u16 *)(addr + 4))) == 0;
    1e68:	e1dd23b4 	ldrh	r2, [sp, #52]	@ 0x34
	if (is_valid_ether_addr(gmaccfg->mac_addr)) {
    1e6c:	e1922003 	orrs	r2, r2, r3
    1e70:	0a000004 	beq	1e88 <nss_gmac_probe+0x3b0>
	dev_addr_mod(dev, 0, addr, len);
    1e74:	e3a03006 	mov	r3, #6
    1e78:	e28d2030 	add	r2, sp, #48	@ 0x30
    1e7c:	e1a00004 	mov	r0, r4
    1e80:	ebfffffe 	bl	0 <dev_addr_mod>
}
    1e84:	ea00001d 	b	1f00 <nss_gmac_probe+0x428>
 * Generate a random Ethernet address (MAC) that is not multicast
 * and has the local assigned bit set.
 */
static inline void eth_random_addr(u8 *addr)
{
	get_random_bytes(addr, ETH_ALEN);
    1e88:	e3a01006 	mov	r1, #6
    1e8c:	e28d0048 	add	r0, sp, #72	@ 0x48
    1e90:	ebfffffe 	bl	0 <get_random_bytes>
	addr[0] &= 0xfe;	/* clear multicast bit */
    1e94:	e5dd3048 	ldrb	r3, [sp, #72]	@ 0x48
	dev_addr_mod(dev, 0, addr, len);
    1e98:	e28d2048 	add	r2, sp, #72	@ 0x48
    1e9c:	e3a01000 	mov	r1, #0
    1ea0:	e1a00004 	mov	r0, r4
    1ea4:	e3c33001 	bic	r3, r3, #1
	addr[0] |= 0x02;	/* set local assignment bit (IEEE802) */
    1ea8:	e3833002 	orr	r3, r3, #2
    1eac:	e5cd3048 	strb	r3, [sp, #72]	@ 0x48
    1eb0:	e3a03006 	mov	r3, #6
    1eb4:	ebfffffe 	bl	0 <dev_addr_mod>
{
	u8 addr[ETH_ALEN];

	eth_random_addr(addr);
	__dev_addr_set(dev, addr, ETH_ALEN);
	dev->addr_assign_type = NET_ADDR_RANDOM;
    1eb8:	e3a03001 	mov	r3, #1
		pr_info("GMAC%d(%p) Invalid MAC@ - using %02x:%02x:%02x:%02x:%02x:%02x\n",
    1ebc:	e59f050c 	ldr	r0, [pc, #1292]	@ 23d0 <nss_gmac_probe+0x8f8>
    1ec0:	e5c43180 	strb	r3, [r4, #384]	@ 0x180
    1ec4:	e59431f4 	ldr	r3, [r4, #500]	@ 0x1f4
    1ec8:	e5d33000 	ldrb	r3, [r3]
    1ecc:	e2832005 	add	r2, r3, #5
    1ed0:	e58d2010 	str	r2, [sp, #16]
    1ed4:	e2832004 	add	r2, r3, #4
    1ed8:	e58d200c 	str	r2, [sp, #12]
    1edc:	e2832003 	add	r2, r3, #3
    1ee0:	e58d2008 	str	r2, [sp, #8]
    1ee4:	e2832002 	add	r2, r3, #2
    1ee8:	e58d2004 	str	r2, [sp, #4]
    1eec:	e2832001 	add	r2, r3, #1
    1ef0:	e58d2000 	str	r2, [sp]
    1ef4:	e594154c 	ldr	r1, [r4, #1356]	@ 0x54c
    1ef8:	e1a02005 	mov	r2, r5
    1efc:	ebfffffe 	bl	0 <_printk>
	netdev->watchdog_timeo = 5 * HZ;
    1f00:	e3a03f7d 	mov	r3, #500	@ 0x1f4
	nss_gmac_ethtool_register(netdev);
    1f04:	e1a00004 	mov	r0, r4
	netdev->watchdog_timeo = 5 * HZ;
    1f08:	e5843300 	str	r3, [r4, #768]	@ 0x300
	netdev->netdev_ops = &nss_gmac_netdev_ops;
    1f0c:	e59f34c0 	ldr	r3, [pc, #1216]	@ 23d4 <nss_gmac_probe+0x8fc>
    1f10:	e584307c 	str	r3, [r4, #124]	@ 0x7c
	nss_gmac_ethtool_register(netdev);
    1f14:	ebfffffe 	bl	3d4c <nss_gmac_ethtool_register>
	INIT_DELAYED_WORK(&gmacdev->gmacwork, nss_gmac_open_work);
    1f18:	e3e0301f 	mvn	r3, #31
    1f1c:	e59f14b4 	ldr	r1, [pc, #1204]	@ 23d8 <nss_gmac_probe+0x900>
    1f20:	e3a02602 	mov	r2, #2097152	@ 0x200000
    1f24:	e28500a8 	add	r0, r5, #168	@ 0xa8
    1f28:	e58435d8 	str	r3, [r4, #1496]	@ 0x5d8
    1f2c:	e285309c 	add	r3, r5, #156	@ 0x9c
 * Initializes the list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	WRITE_ONCE(list->next, list);
    1f30:	e58435dc 	str	r3, [r4, #1500]	@ 0x5dc
	WRITE_ONCE(list->prev, list);
    1f34:	e58435e0 	str	r3, [r4, #1504]	@ 0x5e0
    1f38:	e59f349c 	ldr	r3, [pc, #1180]	@ 23dc <nss_gmac_probe+0x904>
    1f3c:	e58435e4 	str	r3, [r4, #1508]	@ 0x5e4
    1f40:	e3a03000 	mov	r3, #0
    1f44:	e58d3000 	str	r3, [sp]
    1f48:	ebfffffe 	bl	0 <init_timer_key>
	snprintf(phy_id, MII_BUS_ID_SIZE + 3, PHY_ID_FMT,
    1f4c:	e5942548 	ldr	r2, [r4, #1352]	@ 0x548
    1f50:	e3a01040 	mov	r1, #64	@ 0x40
    1f54:	e28d0048 	add	r0, sp, #72	@ 0x48
	phyif = gmacdev->phy_mii_type;
    1f58:	e59485b4 	ldr	r8, [r4, #1460]	@ 0x5b4
	snprintf(phy_id, MII_BUS_ID_SIZE + 3, PHY_ID_FMT,
    1f5c:	e5943a68 	ldr	r3, [r4, #2664]	@ 0xa68
    1f60:	e58d2000 	str	r2, [sp]
    1f64:	e59f2474 	ldr	r2, [pc, #1140]	@ 23e0 <nss_gmac_probe+0x908>
    1f68:	e2833008 	add	r3, r3, #8
    1f6c:	ebfffffe 	bl	0 <snprintf>
	if (gmacdev->phy_base != NSS_GMAC_NO_MDIO_PHY) {
    1f70:	e5943548 	ldr	r3, [r4, #1352]	@ 0x548
    1f74:	e3530020 	cmp	r3, #32
    1f78:	0a00000a 	beq	1fa8 <nss_gmac_probe+0x4d0>
		ret = phy_register_fixup((const char *)phy_id,
    1f7c:	e59f3460 	ldr	r3, [pc, #1120]	@ 23e4 <nss_gmac_probe+0x90c>
    1f80:	e3a0184d 	mov	r1, #5046272	@ 0x4d0000
    1f84:	e28d0048 	add	r0, sp, #72	@ 0x48
    1f88:	e59f2458 	ldr	r2, [pc, #1112]	@ 23e8 <nss_gmac_probe+0x910>
    1f8c:	ebfffffe 	bl	0 <phy_register_fixup>
		if (ret	!= 0) {
    1f90:	e2506000 	subs	r6, r0, #0
    1f94:	0a000003 	beq	1fa8 <nss_gmac_probe+0x4d0>
			netdev_info(netdev, "PHY fixup register Error.\n");
    1f98:	e59f144c 	ldr	r1, [pc, #1100]	@ 23ec <nss_gmac_probe+0x914>
    1f9c:	e1a00004 	mov	r0, r4
    1fa0:	ebfffffe 	bl	0 <netdev_info>
			goto nss_gmac_phy_attach_fail;
    1fa4:	ea0000e9 	b	2350 <nss_gmac_probe+0x878>
	if (gmaccfg->phy_node) {
    1fa8:	e59d101c 	ldr	r1, [sp, #28]
    1fac:	e3510000 	cmp	r1, #0
    1fb0:	0a000052 	beq	2100 <nss_gmac_probe+0x628>
		gmacdev->phydev = of_phy_connect(netdev, gmaccfg->phy_node,
    1fb4:	e59435b4 	ldr	r3, [r4, #1460]	@ 0x5b4
		SET_NETDEV_DEV(netdev, &pdev->dev);
    1fb8:	e2877010 	add	r7, r7, #16
		gmacdev->phydev = of_phy_connect(netdev, gmaccfg->phy_node,
    1fbc:	e1a00004 	mov	r0, r4
    1fc0:	e59f2428 	ldr	r2, [pc, #1064]	@ 23f0 <nss_gmac_probe+0x918>
		SET_NETDEV_DEV(netdev, &pdev->dev);
    1fc4:	e5847364 	str	r7, [r4, #868]	@ 0x364
		gmacdev->phydev = of_phy_connect(netdev, gmaccfg->phy_node,
    1fc8:	e58d3000 	str	r3, [sp]
    1fcc:	e3a03000 	mov	r3, #0
    1fd0:	ebfffffe 	bl	0 <of_phy_connect>
		if (!(gmacdev->phydev)) {
    1fd4:	e3500000 	cmp	r0, #0
		gmacdev->phydev = of_phy_connect(netdev, gmaccfg->phy_node,
    1fd8:	e58407d0 	str	r0, [r4, #2000]	@ 0x7d0
		if (!(gmacdev->phydev)) {
    1fdc:	1a000003 	bne	1ff0 <nss_gmac_probe+0x518>
			netdev_err(netdev, "failed to connect to phy device\n");
    1fe0:	e59f140c 	ldr	r1, [pc, #1036]	@ 23f4 <nss_gmac_probe+0x91c>
    1fe4:	e1a00004 	mov	r0, r4
    1fe8:	ebfffffe 	bl	0 <netdev_err>
			goto nss_gmac_phy_attach_fail;
    1fec:	eaffff8b 	b	1e20 <nss_gmac_probe+0x348>
	*supp |= NSS_GMAC_SUPPORTED_FEATURES;
    1ff0:	e590321c 	ldr	r3, [r0, #540]	@ 0x21c
		gmacdev->phydev->irq = PHY_POLL;
    1ff4:	e3e02000 	mvn	r2, #0
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    1ff8:	e59f13f8 	ldr	r1, [pc, #1016]	@ 23f8 <nss_gmac_probe+0x920>
	*supp |= NSS_GMAC_SUPPORTED_FEATURES;
    1ffc:	e3833a06 	orr	r3, r3, #24576	@ 0x6000
    2000:	e38330ef 	orr	r3, r3, #239	@ 0xef
    2004:	e580321c 	str	r3, [r0, #540]	@ 0x21c
	*adv |= NSS_GMAC_ADVERTISED_FEATURES;
    2008:	e5903228 	ldr	r3, [r0, #552]	@ 0x228
    200c:	e38330ef 	orr	r3, r3, #239	@ 0xef
    2010:	e5803228 	str	r3, [r0, #552]	@ 0x228
		gmacdev->phydev->irq = PHY_POLL;
    2014:	e59437d0 	ldr	r3, [r4, #2000]	@ 0x7d0
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    2018:	e1a00004 	mov	r0, r4
		gmacdev->phydev->irq = PHY_POLL;
    201c:	e5832260 	str	r2, [r3, #608]	@ 0x260
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    2020:	e28d2048 	add	r2, sp, #72	@ 0x48
    2024:	ebfffffe 	bl	0 <netdev_info>
		nss_gmac_reset_phy(gmacdev, gmacdev->phy_base);
    2028:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    202c:	e1a00005 	mov	r0, r5
    2030:	ebfffffe 	bl	1e0 <nss_gmac_reset_phy>
		if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII) {
    2034:	e59435b4 	ldr	r3, [r4, #1460]	@ 0x5b4
    2038:	e3530009 	cmp	r3, #9
    203c:	1a00001a 	bne	20ac <nss_gmac_probe+0x5d4>
			netdev_info(netdev, "%s: Program RGMII Tx delay..... \n", __func__);
    2040:	e59f2328 	ldr	r2, [pc, #808]	@ 2370 <nss_gmac_probe+0x898>
    2044:	e1a00004 	mov	r0, r4
    2048:	e59f13ac 	ldr	r1, [pc, #940]	@ 23fc <nss_gmac_probe+0x924>
    204c:	ebfffffe 	bl	0 <netdev_info>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1D, 0x05);
    2050:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2054:	e3a03005 	mov	r3, #5
    2058:	e3a0201d 	mov	r2, #29
    205c:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    2060:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E, 0x100);
    2064:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2068:	e3a03c01 	mov	r3, #256	@ 0x100
    206c:	e3a0201e 	mov	r2, #30
    2070:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    2074:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1D, 0x0B);
    2078:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    207c:	e3a0300b 	mov	r3, #11
    2080:	e3a0201d 	mov	r2, #29
    2084:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    2088:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    208c:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2090:	e3a0201e 	mov	r2, #30
				(0xBC00 | AR8xxx_PHY_RGMII_TX_DELAY_VAL(gmacdev->rgmii_delay)));
    2094:	e59435b8 	ldr	r3, [r4, #1464]	@ 0x5b8
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    2098:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
				(0xBC00 | AR8xxx_PHY_RGMII_TX_DELAY_VAL(gmacdev->rgmii_delay)));
    209c:	e1a03283 	lsl	r3, r3, #5
    20a0:	e2033060 	and	r3, r3, #96	@ 0x60
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    20a4:	e3833b2f 	orr	r3, r3, #48128	@ 0xbc00
    20a8:	ebfffffe 	bl	0 <mdiobus_write>
		      nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_PHYSID1));
    20ac:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    20b0:	e3a02002 	mov	r2, #2
    20b4:	e1a00005 	mov	r0, r5
    20b8:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
    20bc:	e1a03000 	mov	r3, r0
		netdev_info(netdev, "%s MII_PHYSID1 - 0x%04x\n", netdev->name,
    20c0:	e1a02004 	mov	r2, r4
    20c4:	e59f1334 	ldr	r1, [pc, #820]	@ 2400 <nss_gmac_probe+0x928>
    20c8:	e1a00004 	mov	r0, r4
    20cc:	ebfffffe 	bl	0 <netdev_info>
		      nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_PHYSID2));
    20d0:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    20d4:	e3a02003 	mov	r2, #3
    20d8:	e1a00005 	mov	r0, r5
    20dc:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
    20e0:	e1a03000 	mov	r3, r0
		netdev_info(netdev, "%s MII_PHYSID2 - 0x%04x\n", netdev->name,
    20e4:	e59f1318 	ldr	r1, [pc, #792]	@ 2404 <nss_gmac_probe+0x92c>
    20e8:	e1a00004 	mov	r0, r4
    20ec:	e1a02004 	mov	r2, r4
    20f0:	ebfffffe 	bl	0 <netdev_info>
 		phy_attached_info(gmacdev->phydev);
    20f4:	e59407d0 	ldr	r0, [r4, #2000]	@ 0x7d0
    20f8:	ebfffffe 	bl	0 <phy_attached_info>
    20fc:	ea000062 	b	228c <nss_gmac_probe+0x7b4>
    2100:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
		if (test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
    2104:	e3130040 	tst	r3, #64	@ 0x40
    2108:	0a00004b 	beq	223c <nss_gmac_probe+0x764>
		SET_NETDEV_DEV(netdev, &pdev->dev);
    210c:	e2877010 	add	r7, r7, #16
		gmacdev->phydev = phy_connect(netdev, (const char *)phy_id,
    2110:	e59f22d8 	ldr	r2, [pc, #728]	@ 23f0 <nss_gmac_probe+0x918>
    2114:	e1a03008 	mov	r3, r8
    2118:	e28d1048 	add	r1, sp, #72	@ 0x48
    211c:	e1a00004 	mov	r0, r4
		SET_NETDEV_DEV(netdev, &pdev->dev);
    2120:	e5847364 	str	r7, [r4, #868]	@ 0x364
		gmacdev->phydev = phy_connect(netdev, (const char *)phy_id,
    2124:	ebfffffe 	bl	0 <phy_connect>
		if (IS_ERR(gmacdev->phydev)) {
    2128:	e3700a01 	cmn	r0, #4096	@ 0x1000
		gmacdev->phydev = phy_connect(netdev, (const char *)phy_id,
    212c:	e58407d0 	str	r0, [r4, #2000]	@ 0x7d0
			netdev_info(netdev, "PHY %s attach FAIL\n", phy_id);
    2130:	8a00004e 	bhi	2270 <nss_gmac_probe+0x798>
	*supp |= NSS_GMAC_SUPPORTED_FEATURES;
    2134:	e590321c 	ldr	r3, [r0, #540]	@ 0x21c
		gmacdev->phydev->irq = PHY_POLL;
    2138:	e3e02000 	mvn	r2, #0
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    213c:	e59f12b4 	ldr	r1, [pc, #692]	@ 23f8 <nss_gmac_probe+0x920>
	*supp |= NSS_GMAC_SUPPORTED_FEATURES;
    2140:	e3833a06 	orr	r3, r3, #24576	@ 0x6000
    2144:	e38330ef 	orr	r3, r3, #239	@ 0xef
    2148:	e580321c 	str	r3, [r0, #540]	@ 0x21c
	*adv |= NSS_GMAC_ADVERTISED_FEATURES;
    214c:	e5903228 	ldr	r3, [r0, #552]	@ 0x228
    2150:	e38330ef 	orr	r3, r3, #239	@ 0xef
    2154:	e5803228 	str	r3, [r0, #552]	@ 0x228
		gmacdev->phydev->irq = PHY_POLL;
    2158:	e59437d0 	ldr	r3, [r4, #2000]	@ 0x7d0
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    215c:	e1a00004 	mov	r0, r4
		gmacdev->phydev->irq = PHY_POLL;
    2160:	e5832260 	str	r2, [r3, #608]	@ 0x260
		netdev_info(netdev, "PHY %s attach OK\n", phy_id);
    2164:	e28d2048 	add	r2, sp, #72	@ 0x48
    2168:	ebfffffe 	bl	0 <netdev_info>
		nss_gmac_reset_phy(gmacdev, gmacdev->phy_base);
    216c:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2170:	e1a00005 	mov	r0, r5
    2174:	ebfffffe 	bl	1e0 <nss_gmac_reset_phy>
		if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII) {
    2178:	e59435b4 	ldr	r3, [r4, #1460]	@ 0x5b4
    217c:	e3530009 	cmp	r3, #9
    2180:	1a00001a 	bne	21f0 <nss_gmac_probe+0x718>
			netdev_info(netdev, "%s: Program RGMII Tx delay..... \n", __func__);
    2184:	e59f21e4 	ldr	r2, [pc, #484]	@ 2370 <nss_gmac_probe+0x898>
    2188:	e1a00004 	mov	r0, r4
    218c:	e59f1268 	ldr	r1, [pc, #616]	@ 23fc <nss_gmac_probe+0x924>
    2190:	ebfffffe 	bl	0 <netdev_info>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1D, 0x05);
    2194:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2198:	e3a03005 	mov	r3, #5
    219c:	e3a0201d 	mov	r2, #29
    21a0:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    21a4:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E, 0x100);
    21a8:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    21ac:	e3a03c01 	mov	r3, #256	@ 0x100
    21b0:	e3a0201e 	mov	r2, #30
    21b4:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    21b8:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1D, 0x0B);
    21bc:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    21c0:	e3a0300b 	mov	r3, #11
    21c4:	e3a0201d 	mov	r2, #29
    21c8:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
    21cc:	ebfffffe 	bl	0 <mdiobus_write>
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    21d0:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    21d4:	e3a0201e 	mov	r2, #30
				(0xBC00 | AR8xxx_PHY_RGMII_TX_DELAY_VAL(gmacdev->rgmii_delay)));
    21d8:	e59435b8 	ldr	r3, [r4, #1464]	@ 0x5b8
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    21dc:	e5940a68 	ldr	r0, [r4, #2664]	@ 0xa68
				(0xBC00 | AR8xxx_PHY_RGMII_TX_DELAY_VAL(gmacdev->rgmii_delay)));
    21e0:	e1a03283 	lsl	r3, r3, #5
    21e4:	e2033060 	and	r3, r3, #96	@ 0x60
			mdiobus_write(gmacdev->miibus, gmacdev->phy_base, 0x1E,
    21e8:	e3833b2f 	orr	r3, r3, #48128	@ 0xbc00
    21ec:	ebfffffe 	bl	0 <mdiobus_write>
		      nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_PHYSID1));
    21f0:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    21f4:	e3a02002 	mov	r2, #2
    21f8:	e1a00005 	mov	r0, r5
    21fc:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
    2200:	e1a03000 	mov	r3, r0
		netdev_info(netdev, "%s MII_PHYSID1 - 0x%04x\n", netdev->name,
    2204:	e1a02004 	mov	r2, r4
    2208:	e59f11f0 	ldr	r1, [pc, #496]	@ 2400 <nss_gmac_probe+0x928>
    220c:	e1a00004 	mov	r0, r4
    2210:	ebfffffe 	bl	0 <netdev_info>
		      nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_PHYSID2));
    2214:	e5941548 	ldr	r1, [r4, #1352]	@ 0x548
    2218:	e3a02003 	mov	r2, #3
    221c:	e1a00005 	mov	r0, r5
    2220:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
    2224:	e1a03000 	mov	r3, r0
		netdev_info(netdev, "%s MII_PHYSID2 - 0x%04x\n", netdev->name,
    2228:	e59f11d4 	ldr	r1, [pc, #468]	@ 2404 <nss_gmac_probe+0x92c>
    222c:	e1a02004 	mov	r2, r4
    2230:	e1a00004 	mov	r0, r4
    2234:	ebfffffe 	bl	0 <netdev_info>
    2238:	ea000013 	b	228c <nss_gmac_probe+0x7b4>
	} else if (gmacdev->phy_base != NSS_GMAC_NO_MDIO_PHY) {
    223c:	e5943548 	ldr	r3, [r4, #1352]	@ 0x548
    2240:	e3530020 	cmp	r3, #32
    2244:	0a00000e 	beq	2284 <nss_gmac_probe+0x7ac>
		SET_NETDEV_DEV(netdev, gmacdev->miibus->parent);
    2248:	e5943a68 	ldr	r3, [r4, #2664]	@ 0xa68
		gmacdev->phydev = phy_attach(netdev,
    224c:	e1a02008 	mov	r2, r8
    2250:	e28d1048 	add	r1, sp, #72	@ 0x48
    2254:	e1a00004 	mov	r0, r4
		SET_NETDEV_DEV(netdev, gmacdev->miibus->parent);
    2258:	e593356c 	ldr	r3, [r3, #1388]	@ 0x56c
    225c:	e5843364 	str	r3, [r4, #868]	@ 0x364
		gmacdev->phydev = phy_attach(netdev,
    2260:	ebfffffe 	bl	0 <phy_attach>
		if (IS_ERR(gmacdev->phydev)) {
    2264:	e3700a01 	cmn	r0, #4096	@ 0x1000
		gmacdev->phydev = phy_attach(netdev,
    2268:	e58407d0 	str	r0, [r4, #2000]	@ 0x7d0
		if (IS_ERR(gmacdev->phydev)) {
    226c:	9a000006 	bls	228c <nss_gmac_probe+0x7b4>
			netdev_info(netdev, "PHY %s attach FAIL\n", phy_id);
    2270:	e59f1190 	ldr	r1, [pc, #400]	@ 2408 <nss_gmac_probe+0x930>
    2274:	e28d2048 	add	r2, sp, #72	@ 0x48
    2278:	e1a00004 	mov	r0, r4
    227c:	ebfffffe 	bl	0 <netdev_info>
			goto nss_gmac_phy_attach_fail;
    2280:	eafffee6 	b	1e20 <nss_gmac_probe+0x348>
		gmacdev->phydev = ERR_PTR(-ENODEV);
    2284:	e3e03012 	mvn	r3, #18
    2288:	e58437d0 	str	r3, [r4, #2000]	@ 0x7d0
	set_bit(__NSS_GMAC_RXCSUM, &gmacdev->flags);
    228c:	e2851018 	add	r1, r5, #24
    2290:	e3a00002 	mov	r0, #2
    2294:	ebfffffe 	bl	0 <_set_bit>
	nss_gmac_ipc_offload_init(gmacdev);
    2298:	e1a00005 	mov	r0, r5
    229c:	ebfffffe 	bl	ea8 <nss_gmac_ipc_offload_init>
	if (register_netdev(netdev)) {
    22a0:	e1a00004 	mov	r0, r4
    22a4:	ebfffffe 	bl	0 <register_netdev>
    22a8:	e2506000 	subs	r6, r0, #0
    22ac:	0a00000a 	beq	22dc <nss_gmac_probe+0x804>
		netdev_info(netdev, "Error registering netdevice %s\n",
    22b0:	e59f1154 	ldr	r1, [pc, #340]	@ 240c <nss_gmac_probe+0x934>
    22b4:	e1a02004 	mov	r2, r4
    22b8:	e1a00004 	mov	r0, r4
    22bc:	ebfffffe 	bl	0 <netdev_info>
	unregister_netdev(gmacdev->netdev);
    22c0:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
    22c4:	ebfffffe 	bl	0 <unregister_netdev>
	if (!IS_ERR(gmacdev->phydev)) {
    22c8:	e59407d0 	ldr	r0, [r4, #2000]	@ 0x7d0
    22cc:	e3700a01 	cmn	r0, #4096	@ 0x1000
    22d0:	9a00001a 	bls	2340 <nss_gmac_probe+0x868>
		ret = -EFAULT;
    22d4:	e3e0600d 	mvn	r6, #13
    22d8:	ea00001c 	b	2350 <nss_gmac_probe+0x878>
	rtnl_lock();
    22dc:	ebfffffe 	bl	0 <rtnl_lock>
	netdev->features &= ~NETIF_F_GRO;
    22e0:	e5943090 	ldr	r3, [r4, #144]	@ 0x90
	netdev_change_features(netdev);
    22e4:	e1a00004 	mov	r0, r4
	netdev->features &= ~NETIF_F_GRO;
    22e8:	e3c33901 	bic	r3, r3, #16384	@ 0x4000
    22ec:	e5843090 	str	r3, [r4, #144]	@ 0x90
	netdev->wanted_features &= ~NETIF_F_GRO;
    22f0:	e59430a0 	ldr	r3, [r4, #160]	@ 0xa0
    22f4:	e3c33901 	bic	r3, r3, #16384	@ 0x4000
    22f8:	e58430a0 	str	r3, [r4, #160]	@ 0xa0
	netdev_change_features(netdev);
    22fc:	ebfffffe 	bl	0 <netdev_change_features>
	rtnl_unlock();
    2300:	ebfffffe 	bl	0 <rtnl_unlock>
    2304:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
	netdev_info(netdev, "Initialized NSS GMAC%d interface %s: (base = 0x%lx, irq = %d, PhyId = %d, PollLink = %d)\n"
    2308:	e1a00004 	mov	r0, r4
    230c:	e59f10fc 	ldr	r1, [pc, #252]	@ 2410 <nss_gmac_probe+0x938>
    2310:	e7e03353 	ubfx	r3, r3, #6, #1
    2314:	e58d300c 	str	r3, [sp, #12]
    2318:	e5943548 	ldr	r3, [r4, #1352]	@ 0x548
    231c:	e58d3008 	str	r3, [sp, #8]
    2320:	e5943190 	ldr	r3, [r4, #400]	@ 0x190
    2324:	e58d3004 	str	r3, [sp, #4]
    2328:	e5943020 	ldr	r3, [r4, #32]
    232c:	e58d3000 	str	r3, [sp]
    2330:	e594254c 	ldr	r2, [r4, #1356]	@ 0x54c
    2334:	e1a03004 	mov	r3, r4
    2338:	ebfffffe 	bl	0 <netdev_info>
	return 0;
    233c:	ea000007 	b	2360 <nss_gmac_probe+0x888>
		phy_disconnect(gmacdev->phydev);
    2340:	ebfffffe 	bl	0 <phy_disconnect>
		gmacdev->phydev = NULL;
    2344:	e3a03000 	mov	r3, #0
    2348:	e58437d0 	str	r3, [r4, #2000]	@ 0x7d0
    234c:	eaffffe0 	b	22d4 <nss_gmac_probe+0x7fc>
	nss_gmac_detach(gmacdev);
    2350:	e1a00005 	mov	r0, r5
    2354:	ebfffffe 	bl	c00 <nss_gmac_detach>
	free_netdev(netdev);
    2358:	e1a00004 	mov	r0, r4
    235c:	ebfffffe 	bl	0 <free_netdev>
}
    2360:	e1a00006 	mov	r0, r6
    2364:	e28dd08c 	add	sp, sp, #140	@ 0x8c
    2368:	e8bd83f0 	pop	{r4, r5, r6, r7, r8, r9, pc}
    236c:	00000000 	.word	0x00000000
    2370:	0000017f 	.word	0x0000017f
    2374:	00000644 	.word	0x00000644
    2378:	0000003c 	.word	0x0000003c
    237c:	00000663 	.word	0x00000663
    2380:	00000678 	.word	0x00000678
    2384:	00000680 	.word	0x00000680
    2388:	000006b5 	.word	0x000006b5
    238c:	000006c8 	.word	0x000006c8
    2390:	000006d9 	.word	0x000006d9
    2394:	000006ec 	.word	0x000006ec
    2398:	000006fe 	.word	0x000006fe
    239c:	00000711 	.word	0x00000711
    23a0:	00000721 	.word	0x00000721
    23a4:	0000072d 	.word	0x0000072d
    23a8:	0000073c 	.word	0x0000073c
    23ac:	0000075c 	.word	0x0000075c
    23b0:	0000076e 	.word	0x0000076e
    23b4:	00000789 	.word	0x00000789
    23b8:	0000079f 	.word	0x0000079f
    23bc:	000007a7 	.word	0x000007a7
    23c0:	000007c6 	.word	0x000007c6
    23c4:	000007e8 	.word	0x000007e8
    23c8:	00000814 	.word	0x00000814
    23cc:	00000843 	.word	0x00000843
    23d0:	00000856 	.word	0x00000856
    23d4:	00000190 	.word	0x00000190
	...
    23e0:	00000897 	.word	0x00000897
    23e4:	00000000 	.word	0x00000000
    23e8:	ffff0000 	.word	0xffff0000
    23ec:	0000089f 	.word	0x0000089f
    23f0:	00000000 	.word	0x00000000
    23f4:	000008ba 	.word	0x000008ba
    23f8:	000008db 	.word	0x000008db
    23fc:	000008ed 	.word	0x000008ed
    2400:	0000090f 	.word	0x0000090f
    2404:	00000928 	.word	0x00000928
    2408:	00000941 	.word	0x00000941
    240c:	00000955 	.word	0x00000955
    2410:	00000975 	.word	0x00000975

00002414 <nss_gmac_tx_rx_desc_release>:
{
    2414:	e92d4070 	push	{r4, r5, r6, lr}
    2418:	e1a04000 	mov	r4, r0
	struct device *dev = &netdev->dev;
    241c:	e5906090 	ldr	r6, [r0, #144]	@ 0x90
{
    2420:	e24dd018 	sub	sp, sp, #24
	for (i = 0; i < gmacdev->rx_desc_count; i++) {
    2424:	e3a05000 	mov	r5, #0
	struct device *dev = &netdev->dev;
    2428:	e2866d0d 	add	r6, r6, #832	@ 0x340
	for (i = 0; i < gmacdev->rx_desc_count; i++) {
    242c:	ea000016 	b	248c <nss_gmac_tx_rx_desc_release+0x78>
		nss_gmac_get_desc_data(gmacdev->rx_desc + i, &status,
    2430:	e28d3014 	add	r3, sp, #20
    2434:	e28d200c 	add	r2, sp, #12
    2438:	e0800285 	add	r0, r0, r5, lsl #5
    243c:	e28d1008 	add	r1, sp, #8
    2440:	e58d3000 	str	r3, [sp]
    2444:	e28d3010 	add	r3, sp, #16
    2448:	ebfffffe 	bl	d2c <nss_gmac_get_desc_data>
		if ((length1 != 0) && (opaque != 0)) {
    244c:	e59d2010 	ldr	r2, [sp, #16]
    2450:	e3520000 	cmp	r2, #0
    2454:	0a00000b 	beq	2488 <nss_gmac_tx_rx_desc_release+0x74>
    2458:	e59d3014 	ldr	r3, [sp, #20]
    245c:	e3530000 	cmp	r3, #0
    2460:	0a000008 	beq	2488 <nss_gmac_tx_rx_desc_release+0x74>
}

static inline void dma_unmap_single_attrs(struct device *dev, dma_addr_t addr,
		size_t size, enum dma_data_direction dir, unsigned long attrs)
{
	return dma_unmap_page_attrs(dev, addr, size, dir, attrs);
    2464:	e3a03000 	mov	r3, #0
    2468:	e59d100c 	ldr	r1, [sp, #12]
    246c:	e1a00006 	mov	r0, r6
    2470:	e58d3000 	str	r3, [sp]
    2474:	e3a03002 	mov	r3, #2
    2478:	ebfffffe 	bl	0 <dma_unmap_page_attrs>
	__dev_kfree_skb_any(skb, SKB_REASON_DROPPED);
    247c:	e59d0014 	ldr	r0, [sp, #20]
    2480:	e3a01001 	mov	r1, #1
    2484:	ebfffffe 	bl	0 <__dev_kfree_skb_any>
	for (i = 0; i < gmacdev->rx_desc_count; i++) {
    2488:	e2855001 	add	r5, r5, #1
		nss_gmac_get_desc_data(gmacdev->rx_desc + i, &status,
    248c:	e594002c 	ldr	r0, [r4, #44]	@ 0x2c
	for (i = 0; i < gmacdev->rx_desc_count; i++) {
    2490:	e5941038 	ldr	r1, [r4, #56]	@ 0x38
    2494:	e1550001 	cmp	r5, r1
    2498:	3affffe4 	bcc	2430 <nss_gmac_tx_rx_desc_release+0x1c>
}

static inline void dma_free_coherent(struct device *dev, size_t size,
		void *cpu_addr, dma_addr_t dma_handle)
{
	return dma_free_attrs(dev, size, cpu_addr, dma_handle, 0);
    249c:	e3a05000 	mov	r5, #0
    24a0:	e1a02000 	mov	r2, r0
    24a4:	e1a01281 	lsl	r1, r1, #5
    24a8:	e1a00006 	mov	r0, r6
    24ac:	e58d5000 	str	r5, [sp]
    24b0:	e5943024 	ldr	r3, [r4, #36]	@ 0x24
    24b4:	ebfffffe 	bl	0 <dma_free_attrs>
	netdev_info(gmacdev->netdev, "Memory allocated %08x for Rx Descriptors (ring) is given back\n"
    24b8:	e594202c 	ldr	r2, [r4, #44]	@ 0x2c
    24bc:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    24c0:	e59f10b4 	ldr	r1, [pc, #180]	@ 257c <nss_gmac_tx_rx_desc_release+0x168>
    24c4:	ebfffffe 	bl	0 <netdev_info>
	gmacdev->rx_desc_dma = 0;
    24c8:	e5845024 	str	r5, [r4, #36]	@ 0x24
	gmacdev->rx_desc = NULL;
    24cc:	e584502c 	str	r5, [r4, #44]	@ 0x2c
}
    24d0:	ea000016 	b	2530 <nss_gmac_tx_rx_desc_release+0x11c>
		nss_gmac_get_desc_data(gmacdev->tx_desc + i, &status,
    24d4:	e28d3014 	add	r3, sp, #20
    24d8:	e28d200c 	add	r2, sp, #12
    24dc:	e0800285 	add	r0, r0, r5, lsl #5
    24e0:	e28d1008 	add	r1, sp, #8
    24e4:	e58d3000 	str	r3, [sp]
    24e8:	e28d3010 	add	r3, sp, #16
    24ec:	ebfffffe 	bl	d2c <nss_gmac_get_desc_data>
		if ((length1 != 0) && (opaque != 0)) {
    24f0:	e59d2010 	ldr	r2, [sp, #16]
    24f4:	e3520000 	cmp	r2, #0
    24f8:	0a00000b 	beq	252c <nss_gmac_tx_rx_desc_release+0x118>
    24fc:	e59d3014 	ldr	r3, [sp, #20]
    2500:	e3530000 	cmp	r3, #0
    2504:	0a000008 	beq	252c <nss_gmac_tx_rx_desc_release+0x118>
	return dma_unmap_page_attrs(dev, addr, size, dir, attrs);
    2508:	e3a03000 	mov	r3, #0
    250c:	e59d100c 	ldr	r1, [sp, #12]
    2510:	e1a00006 	mov	r0, r6
    2514:	e58d3000 	str	r3, [sp]
    2518:	e3a03001 	mov	r3, #1
    251c:	ebfffffe 	bl	0 <dma_unmap_page_attrs>
    2520:	e59d0014 	ldr	r0, [sp, #20]
    2524:	e3a01001 	mov	r1, #1
    2528:	ebfffffe 	bl	0 <__dev_kfree_skb_any>
	for (i = 0; i < gmacdev->tx_desc_count; i++) {
    252c:	e2855001 	add	r5, r5, #1
		nss_gmac_get_desc_data(gmacdev->tx_desc + i, &status,
    2530:	e5940028 	ldr	r0, [r4, #40]	@ 0x28
	for (i = 0; i < gmacdev->tx_desc_count; i++) {
    2534:	e594103c 	ldr	r1, [r4, #60]	@ 0x3c
    2538:	e1510005 	cmp	r1, r5
    253c:	8affffe4 	bhi	24d4 <nss_gmac_tx_rx_desc_release+0xc0>
	return dma_free_attrs(dev, size, cpu_addr, dma_handle, 0);
    2540:	e3a05000 	mov	r5, #0
    2544:	e1a02000 	mov	r2, r0
    2548:	e1a01281 	lsl	r1, r1, #5
    254c:	e1a00006 	mov	r0, r6
    2550:	e58d5000 	str	r5, [sp]
    2554:	e5943020 	ldr	r3, [r4, #32]
    2558:	ebfffffe 	bl	0 <dma_free_attrs>
	netdev_info(gmacdev->netdev, "Memory allocated %08x for Tx Descriptors (ring) is given back\n"
    255c:	e5942028 	ldr	r2, [r4, #40]	@ 0x28
    2560:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    2564:	e59f1014 	ldr	r1, [pc, #20]	@ 2580 <nss_gmac_tx_rx_desc_release+0x16c>
    2568:	ebfffffe 	bl	0 <netdev_info>
	gmacdev->tx_desc_dma = 0;
    256c:	e5845020 	str	r5, [r4, #32]
	gmacdev->tx_desc = NULL;
    2570:	e5845028 	str	r5, [r4, #40]	@ 0x28
}
    2574:	e28dd018 	add	sp, sp, #24
    2578:	e8bd8070 	pop	{r4, r5, r6, pc}
    257c:	00000a94 	.word	0x00000a94
    2580:	00000ad3 	.word	0x00000ad3

00002584 <nss_gmac_tx_rx_desc_init>:
{
    2584:	e92d4070 	push	{r4, r5, r6, lr}
 */
static inline void nss_gmac_tx_desc_init_ring(struct dma_desc *desc,
					      uint32_t no_of_desc)
{
	struct dma_desc *last_desc = desc + no_of_desc - 1;
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    2588:	e3a01000 	mov	r1, #0
	nss_gmac_tx_desc_init_ring(gmacdev->tx_desc, gmacdev->tx_desc_count);
    258c:	e5905028 	ldr	r5, [r0, #40]	@ 0x28
{
    2590:	e1a04000 	mov	r4, r0
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    2594:	e590203c 	ldr	r2, [r0, #60]	@ 0x3c
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    2598:	e1a00005 	mov	r0, r5
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    259c:	e242637e 	sub	r6, r2, #-134217727	@ 0xf8000001
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    25a0:	e1a02282 	lsl	r2, r2, #5
    25a4:	ebfffffe 	bl	0 <memset>
	last_desc->status = tx_desc_end_of_ring;
    25a8:	e3a03602 	mov	r3, #2097152	@ 0x200000
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    25ac:	e3a01000 	mov	r1, #0
	last_desc->status = tx_desc_end_of_ring;
    25b0:	e7853286 	str	r3, [r5, r6, lsl #5]
	nss_gmac_rx_desc_init_ring(gmacdev->rx_desc, gmacdev->rx_desc_count);
    25b4:	e594602c 	ldr	r6, [r4, #44]	@ 0x2c
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    25b8:	e5945038 	ldr	r5, [r4, #56]	@ 0x38
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    25bc:	e1a00006 	mov	r0, r6
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    25c0:	e245537e 	sub	r5, r5, #-134217727	@ 0xf8000001
    25c4:	e1a05285 	lsl	r5, r5, #5
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    25c8:	e2852020 	add	r2, r5, #32
	last_desc->length = rx_desc_end_of_ring;
    25cc:	e0866005 	add	r6, r6, r5
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    25d0:	ebfffffe 	bl	0 <memset>
	last_desc->length = rx_desc_end_of_ring;
    25d4:	e3a03902 	mov	r3, #32768	@ 0x8000
	nss_gmac_take_desc_ownership_rx(gmacdev);
    25d8:	e1a00004 	mov	r0, r4
    25dc:	e5863004 	str	r3, [r6, #4]
	gmacdev->tx_next_desc = gmacdev->tx_desc;
    25e0:	e5942028 	ldr	r2, [r4, #40]	@ 0x28
	gmacdev->tx_next = 0;
    25e4:	e3a03000 	mov	r3, #0
	gmacdev->busy_tx_desc = 0;
    25e8:	e5843030 	str	r3, [r4, #48]	@ 0x30
	gmacdev->busy_rx_desc = 0;
    25ec:	e5843034 	str	r3, [r4, #52]	@ 0x34
	gmacdev->tx_busy = 0;
    25f0:	e5843040 	str	r3, [r4, #64]	@ 0x40
	gmacdev->tx_busy_desc = gmacdev->tx_desc;
    25f4:	e5842050 	str	r2, [r4, #80]	@ 0x50
	gmacdev->tx_next_desc = gmacdev->tx_desc;
    25f8:	e5842054 	str	r2, [r4, #84]	@ 0x54
	gmacdev->rx_next_desc = gmacdev->rx_desc;
    25fc:	e594202c 	ldr	r2, [r4, #44]	@ 0x2c
	gmacdev->tx_next = 0;
    2600:	e5843044 	str	r3, [r4, #68]	@ 0x44
	gmacdev->rx_busy = 0;
    2604:	e5843048 	str	r3, [r4, #72]	@ 0x48
	gmacdev->rx_next = 0;
    2608:	e584304c 	str	r3, [r4, #76]	@ 0x4c
	gmacdev->rx_busy_desc = gmacdev->rx_desc;
    260c:	e5842058 	str	r2, [r4, #88]	@ 0x58
	gmacdev->rx_next_desc = gmacdev->rx_desc;
    2610:	e584205c 	str	r2, [r4, #92]	@ 0x5c
	nss_gmac_take_desc_ownership_rx(gmacdev);
    2614:	ebfffffe 	bl	da8 <nss_gmac_take_desc_ownership_rx>
	nss_gmac_take_desc_ownership_tx(gmacdev);
    2618:	e1a00004 	mov	r0, r4
}
    261c:	e8bd4070 	pop	{r4, r5, r6, lr}
	nss_gmac_take_desc_ownership_tx(gmacdev);
    2620:	eafffffe 	b	dd8 <nss_gmac_take_desc_ownership_tx>

00002624 <nss_gmac_tstamp_sysfs_create>:
{
    2624:	e92d4010 	push	{r4, lr}
	if (device_create_file(&(dev->dev), &dev_attr_slam) ||
    2628:	e2804d0d 	add	r4, r0, #832	@ 0x340
    262c:	e59f106c 	ldr	r1, [pc, #108]	@ 26a0 <nss_gmac_tstamp_sysfs_create+0x7c>
    2630:	e1a00004 	mov	r0, r4
    2634:	ebfffffe 	bl	0 <device_create_file>
    2638:	e3500000 	cmp	r0, #0
    263c:	0a000002 	beq	264c <nss_gmac_tstamp_sysfs_create+0x28>
}
    2640:	e8bd4010 	pop	{r4, lr}
		pr_err("Failed to create sysfs entries \n");
    2644:	e59f0058 	ldr	r0, [pc, #88]	@ 26a4 <nss_gmac_tstamp_sysfs_create+0x80>
    2648:	eafffffe 	b	0 <_printk>
		device_create_file(&(dev->dev), &dev_attr_cadj) ||
    264c:	e59f1054 	ldr	r1, [pc, #84]	@ 26a8 <nss_gmac_tstamp_sysfs_create+0x84>
    2650:	e1a00004 	mov	r0, r4
    2654:	ebfffffe 	bl	0 <device_create_file>
	if (device_create_file(&(dev->dev), &dev_attr_slam) ||
    2658:	e3500000 	cmp	r0, #0
    265c:	1afffff7 	bne	2640 <nss_gmac_tstamp_sysfs_create+0x1c>
		device_create_file(&(dev->dev), &dev_attr_fadj) ||
    2660:	e59f1044 	ldr	r1, [pc, #68]	@ 26ac <nss_gmac_tstamp_sysfs_create+0x88>
    2664:	e1a00004 	mov	r0, r4
    2668:	ebfffffe 	bl	0 <device_create_file>
		device_create_file(&(dev->dev), &dev_attr_cadj) ||
    266c:	e3500000 	cmp	r0, #0
    2670:	1afffff2 	bne	2640 <nss_gmac_tstamp_sysfs_create+0x1c>
		device_create_file(&(dev->dev), &dev_attr_tstamp) ||
    2674:	e59f1034 	ldr	r1, [pc, #52]	@ 26b0 <nss_gmac_tstamp_sysfs_create+0x8c>
    2678:	e1a00004 	mov	r0, r4
    267c:	ebfffffe 	bl	0 <device_create_file>
		device_create_file(&(dev->dev), &dev_attr_fadj) ||
    2680:	e3500000 	cmp	r0, #0
    2684:	1affffed 	bne	2640 <nss_gmac_tstamp_sysfs_create+0x1c>
		device_create_file(&(dev->dev), &dev_attr_mtnp))
    2688:	e59f1024 	ldr	r1, [pc, #36]	@ 26b4 <nss_gmac_tstamp_sysfs_create+0x90>
    268c:	e1a00004 	mov	r0, r4
    2690:	ebfffffe 	bl	0 <device_create_file>
		device_create_file(&(dev->dev), &dev_attr_tstamp) ||
    2694:	e3500000 	cmp	r0, #0
    2698:	08bd8010 	popeq	{r4, pc}
    269c:	eaffffe7 	b	2640 <nss_gmac_tstamp_sysfs_create+0x1c>
    26a0:	000000d8 	.word	0x000000d8
    26a4:	00000b12 	.word	0x00000b12
    26a8:	000000e8 	.word	0x000000e8
    26ac:	000000f8 	.word	0x000000f8
    26b0:	00000108 	.word	0x00000108
    26b4:	00000118 	.word	0x00000118

000026b8 <nss_gmac_do_ioctl>:
	if (ifr == NULL)
    26b8:	e3510000 	cmp	r1, #0
    26bc:	13500000 	cmpne	r0, #0
    26c0:	1a000002 	bne	26d0 <nss_gmac_do_ioctl+0x18>
    26c4:	ea00004e 	b	2804 <nss_gmac_do_ioctl+0x14c>
		return -EINVAL;
    26c8:	e3e00015 	mvn	r0, #21
    26cc:	ea00004a 	b	27fc <nss_gmac_do_ioctl+0x144>
{
    26d0:	e92d40f0 	push	{r4, r5, r6, r7, lr}
	BUG_ON(gmacdev == NULL);
    26d4:	e2907d15 	adds	r7, r0, #1344	@ 0x540
{
    26d8:	e24dd024 	sub	sp, sp, #36	@ 0x24
    26dc:	e1a04000 	mov	r4, r0
	BUG_ON(gmacdev == NULL);
    26e0:	1a000000 	bne	26e8 <nss_gmac_do_ioctl+0x30>
    26e4:	e7f001f2 	.word	0xe7f001f2
	BUG_ON(gmacdev->netdev != netdev);
    26e8:	e59065d0 	ldr	r6, [r0, #1488]	@ 0x5d0
    26ec:	e1560000 	cmp	r6, r0
    26f0:	0a000000 	beq	26f8 <nss_gmac_do_ioctl+0x40>
    26f4:	e7f001f2 	.word	0xe7f001f2
	if (cmd == SIOCSHWTSTAMP) {
    26f8:	e30839b0 	movw	r3, #35248	@ 0x89b0
    26fc:	e1520003 	cmp	r2, r3
    2700:	1afffff0 	bne	26c8 <nss_gmac_do_ioctl+0x10>
    2704:	e1a05001 	mov	r5, r1

static __always_inline unsigned long __must_check
copy_from_user(void *to, const void __user *from, unsigned long n)
{
	if (check_copy_size(to, n, false))
		n = _copy_from_user(to, from, n);
    2708:	e5911010 	ldr	r1, [r1, #16]
    270c:	e28d0008 	add	r0, sp, #8
    2710:	ebfffb71 	bl	14dc <_copy_from_user.constprop.0>
		if (ret) {
    2714:	e3500000 	cmp	r0, #0
    2718:	1affffea 	bne	26c8 <nss_gmac_do_ioctl+0x10>
		netdev_info(netdev, "Tstamp ioctl, Tx_type: %d, Rx_filter: %d, Flags: %d\n",
    271c:	e59d3008 	ldr	r3, [sp, #8]
    2720:	e1a00006 	mov	r0, r6
    2724:	e59d200c 	ldr	r2, [sp, #12]
    2728:	e59f10dc 	ldr	r1, [pc, #220]	@ 280c <nss_gmac_do_ioctl+0x154>
    272c:	e58d3000 	str	r3, [sp]
    2730:	e59d3010 	ldr	r3, [sp, #16]
    2734:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->data_plane_ctx == netdev) {
    2738:	e59637cc 	ldr	r3, [r6, #1996]	@ 0x7cc
    273c:	e1560003 	cmp	r6, r3
    2740:	1a000003 	bne	2754 <nss_gmac_do_ioctl+0x9c>
		netdev_info(netdev, "%s: NSS Firmware is not up. Cannot enable Timestamping  \n", __func__);
    2744:	e59f20c4 	ldr	r2, [pc, #196]	@ 2810 <nss_gmac_do_ioctl+0x158>
    2748:	e1a00006 	mov	r0, r6
    274c:	e59f10c0 	ldr	r1, [pc, #192]	@ 2814 <nss_gmac_do_ioctl+0x15c>
    2750:	ea00001c 	b	27c8 <nss_gmac_do_ioctl+0x110>
    2754:	e5951010 	ldr	r1, [r5, #16]
    2758:	e28d0014 	add	r0, sp, #20
    275c:	ebfffb5e 	bl	14dc <_copy_from_user.constprop.0>
	if (ret) {
    2760:	e2505000 	subs	r5, r0, #0
    2764:	0a000005 	beq	2780 <nss_gmac_do_ioctl+0xc8>
		netdev_err(netdev, "%s: Unable to copy NSS Firmware into memory \n", __func__);
    2768:	e59f20a0 	ldr	r2, [pc, #160]	@ 2810 <nss_gmac_do_ioctl+0x158>
    276c:	e1a00006 	mov	r0, r6
    2770:	e59f10a0 	ldr	r1, [pc, #160]	@ 2818 <nss_gmac_do_ioctl+0x160>
    2774:	ebfffffe 	bl	0 <netdev_err>
		return -EINVAL;
    2778:	e3e05015 	mvn	r5, #21
    277c:	ea00001d 	b	27f8 <nss_gmac_do_ioctl+0x140>
	if (cfg.rx_filter == HWTSTAMP_FILTER_ALL) {
    2780:	e59d301c 	ldr	r3, [sp, #28]
    2784:	e3530001 	cmp	r3, #1
    2788:	1a000010 	bne	27d0 <nss_gmac_do_ioctl+0x118>
    278c:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
		if (!test_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags)) {
    2790:	e3130080 	tst	r3, #128	@ 0x80
    2794:	1a000017 	bne	27f8 <nss_gmac_do_ioctl+0x140>
			netdev->needed_headroom += 32;
    2798:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
			nss_gmac_tstamp_sysfs_create(netdev);
    279c:	e1a00004 	mov	r0, r4
			netdev->needed_headroom += 32;
    27a0:	e2833020 	add	r3, r3, #32
    27a4:	e1c438bc 	strh	r3, [r4, #140]	@ 0x8c
			nss_gmac_tstamp_sysfs_create(netdev);
    27a8:	ebfffffe 	bl	2624 <nss_gmac_tstamp_sysfs_create>
			if (nss_gmac_ts_enable(gmacdev)) {
    27ac:	e1a00007 	mov	r0, r7
    27b0:	ebfffffe 	bl	5b4 <nss_gmac_ts_enable>
    27b4:	e3500000 	cmp	r0, #0
    27b8:	0a00000e 	beq	27f8 <nss_gmac_do_ioctl+0x140>
				netdev_info(netdev, "%s: Reg write error. Cannot enable Timestamping \n", __func__);
    27bc:	e59f204c 	ldr	r2, [pc, #76]	@ 2810 <nss_gmac_do_ioctl+0x158>
    27c0:	e59f1054 	ldr	r1, [pc, #84]	@ 281c <nss_gmac_do_ioctl+0x164>
    27c4:	e1a00004 	mov	r0, r4
    27c8:	ebfffffe 	bl	0 <netdev_info>
				return -EINVAL;
    27cc:	eaffffe9 	b	2778 <nss_gmac_do_ioctl+0xc0>
	} else if ((cfg.tx_type == HWTSTAMP_TX_OFF) && (cfg.rx_filter != HWTSTAMP_FILTER_ALL)) {
    27d0:	e59d3018 	ldr	r3, [sp, #24]
    27d4:	e3530000 	cmp	r3, #0
    27d8:	1a000006 	bne	27f8 <nss_gmac_do_ioctl+0x140>
    27dc:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
		if (!test_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags)) {
    27e0:	e3130080 	tst	r3, #128	@ 0x80
			netdev_info(netdev, "%s: Timestamp is already disabled \n", __func__);
    27e4:	059f2024 	ldreq	r2, [pc, #36]	@ 2810 <nss_gmac_do_ioctl+0x158>
    27e8:	059f1030 	ldreq	r1, [pc, #48]	@ 2820 <nss_gmac_do_ioctl+0x168>
    27ec:	0afffff4 	beq	27c4 <nss_gmac_do_ioctl+0x10c>
		nss_gmac_ts_disable(gmacdev);
    27f0:	e1a00007 	mov	r0, r7
    27f4:	ebfffffe 	bl	6dc <nss_gmac_ts_disable>
		ret = nss_gmac_tstamp_ioctl(netdev, ifr);
    27f8:	e1a00005 	mov	r0, r5
}
    27fc:	e28dd024 	add	sp, sp, #36	@ 0x24
    2800:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
		return -EINVAL;
    2804:	e3e00015 	mvn	r0, #21
}
    2808:	e12fff1e 	bx	lr
    280c:	00000b35 	.word	0x00000b35
    2810:	00000347 	.word	0x00000347
    2814:	00000b6a 	.word	0x00000b6a
    2818:	00000ba4 	.word	0x00000ba4
    281c:	00000bd2 	.word	0x00000bd2
    2820:	00000c04 	.word	0x00000c04

00002824 <nss_gmac_tstamp_sysfs_remove>:
{
    2824:	e92d4070 	push	{r4, r5, r6, lr}
	device_remove_file(&(dev->dev), &dev_attr_slam);
    2828:	e2804d0d 	add	r4, r0, #832	@ 0x340
    282c:	e59f503c 	ldr	r5, [pc, #60]	@ 2870 <nss_gmac_tstamp_sysfs_remove+0x4c>
    2830:	e1a00004 	mov	r0, r4
    2834:	e28510d8 	add	r1, r5, #216	@ 0xd8
    2838:	ebfffffe 	bl	0 <device_remove_file>
	device_remove_file(&(dev->dev), &dev_attr_cadj);
    283c:	e28510e8 	add	r1, r5, #232	@ 0xe8
    2840:	e1a00004 	mov	r0, r4
    2844:	ebfffffe 	bl	0 <device_remove_file>
	device_remove_file(&(dev->dev), &dev_attr_fadj);
    2848:	e28510f8 	add	r1, r5, #248	@ 0xf8
    284c:	e1a00004 	mov	r0, r4
    2850:	ebfffffe 	bl	0 <device_remove_file>
	device_remove_file(&(dev->dev), &dev_attr_tstamp);
    2854:	e2851f42 	add	r1, r5, #264	@ 0x108
    2858:	e1a00004 	mov	r0, r4
    285c:	ebfffffe 	bl	0 <device_remove_file>
	device_remove_file(&(dev->dev), &dev_attr_mtnp);
    2860:	e2851f46 	add	r1, r5, #280	@ 0x118
    2864:	e1a00004 	mov	r0, r4
}
    2868:	e8bd4070 	pop	{r4, r5, r6, lr}
	device_remove_file(&(dev->dev), &dev_attr_mtnp);
    286c:	eafffffe 	b	0 <device_remove_file>
    2870:	00000000 	.word	0x00000000

00002874 <nss_gmac_exit_network_interfaces>:
/**
 * @brief De-register network interfaces.
 * @return void
 */
void nss_gmac_exit_network_interfaces(void)
{
    2874:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
	uint32_t i;
	struct nss_gmac_dev *gmacdev;

	for (i = 0; i < NSS_MAX_GMACS; i++) {
    2878:	e3a05000 	mov	r5, #0
    287c:	e59f6058 	ldr	r6, [pc, #88]	@ 28dc <nss_gmac_exit_network_interfaces+0x68>
#endif
			nss_gmac_tstamp_sysfs_remove(gmacdev->netdev);
			unregister_netdev(gmacdev->netdev);
			free_netdev(gmacdev->netdev);
			nss_gmac_detach(gmacdev);
			ctx.nss_gmac[i] = NULL;
    2880:	e1a07005 	mov	r7, r5
		gmacdev = ctx.nss_gmac[i];
    2884:	e5964000 	ldr	r4, [r6]
		if (gmacdev) {
    2888:	e3540000 	cmp	r4, #0
    288c:	0a00000d 	beq	28c8 <nss_gmac_exit_network_interfaces+0x54>
			if (!IS_ERR(gmacdev->phydev)) {
    2890:	e5940290 	ldr	r0, [r4, #656]	@ 0x290
    2894:	e3700a01 	cmn	r0, #4096	@ 0x1000
    2898:	8a000001 	bhi	28a4 <nss_gmac_exit_network_interfaces+0x30>
				phy_disconnect(gmacdev->phydev);
    289c:	ebfffffe 	bl	0 <phy_disconnect>
				gmacdev->phydev = NULL;
    28a0:	e5847290 	str	r7, [r4, #656]	@ 0x290
			nss_gmac_tstamp_sysfs_remove(gmacdev->netdev);
    28a4:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    28a8:	ebfffffe 	bl	2824 <nss_gmac_tstamp_sysfs_remove>
			unregister_netdev(gmacdev->netdev);
    28ac:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    28b0:	ebfffffe 	bl	0 <unregister_netdev>
			free_netdev(gmacdev->netdev);
    28b4:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    28b8:	ebfffffe 	bl	0 <free_netdev>
			nss_gmac_detach(gmacdev);
    28bc:	e1a00004 	mov	r0, r4
    28c0:	ebfffffe 	bl	c00 <nss_gmac_detach>
			ctx.nss_gmac[i] = NULL;
    28c4:	e5867000 	str	r7, [r6]
	for (i = 0; i < NSS_MAX_GMACS; i++) {
    28c8:	e2855001 	add	r5, r5, #1
    28cc:	e2866004 	add	r6, r6, #4
    28d0:	e3550004 	cmp	r5, #4
    28d4:	08bd81f0 	popeq	{r4, r5, r6, r7, r8, pc}
    28d8:	eaffffe9 	b	2884 <nss_gmac_exit_network_interfaces+0x10>
    28dc:	0000001c 	.word	0x0000001c

000028e0 <nss_gmac_clear_all_regs>:
	asm volatile("ldr %0, %1"
    28e0:	e5903008 	ldr	r3, [r0, #8]
	asm volatile("str %1, %0"
    28e4:	e3a03000 	mov	r3, #0
    28e8:	e5803008 	str	r3, [r0, #8]
	asm volatile("ldr %0, %1"
    28ec:	e590200c 	ldr	r2, [r0, #12]
	asm volatile("str %1, %0"
    28f0:	e580300c 	str	r3, [r0, #12]
	asm volatile("ldr %0, %1"
    28f4:	e5902010 	ldr	r2, [r0, #16]
	asm volatile("str %1, %0"
    28f8:	e5803010 	str	r3, [r0, #16]
	asm volatile("ldr %0, %1"
    28fc:	e5902014 	ldr	r2, [r0, #20]
	asm volatile("str %1, %0"
    2900:	e5803014 	str	r3, [r0, #20]
	asm volatile("ldr %0, %1"
    2904:	e5902018 	ldr	r2, [r0, #24]
	asm volatile("str %1, %0"
    2908:	e5803018 	str	r3, [r0, #24]
	asm volatile("ldr %0, %1"
    290c:	e5902030 	ldr	r2, [r0, #48]	@ 0x30
	asm volatile("str %1, %0"
    2910:	e5803030 	str	r3, [r0, #48]	@ 0x30
	asm volatile("ldr %0, %1"
    2914:	e5902034 	ldr	r2, [r0, #52]	@ 0x34
	asm volatile("str %1, %0"
    2918:	e5803034 	str	r3, [r0, #52]	@ 0x34
	asm volatile("ldr %0, %1"
    291c:	e5902038 	ldr	r2, [r0, #56]	@ 0x38
	asm volatile("str %1, %0"
    2920:	e5803038 	str	r3, [r0, #56]	@ 0x38
	asm volatile("ldr %0, %1"
    2924:	e590203c 	ldr	r2, [r0, #60]	@ 0x3c
	asm volatile("str %1, %0"
    2928:	e580303c 	str	r3, [r0, #60]	@ 0x3c
	asm volatile("ldr %0, %1"
    292c:	e590202c 	ldr	r2, [r0, #44]	@ 0x2c
	asm volatile("str %1, %0"
    2930:	e580302c 	str	r3, [r0, #44]	@ 0x2c
				NSS_GMAC2_CTL, 0xFFFFFFFF);
	nss_gmac_clear_reg_bits((uint32_t *)nss_base,
				NSS_GMAC3_CTL, 0xFFFFFFFF);
	nss_gmac_clear_reg_bits((uint32_t *)nss_base,
				NSS_QSGMII_CLK_CTL, 0xFFFFFFFF);
}
    2934:	e12fff1e 	bx	lr

00002938 <nss_gmac_ifg_reset>:
 * @return void
 */
static void nss_gmac_ifg_reset(uint32_t gmac_id)
{
	uint32_t val = 0;
	uint32_t *nss_base = (uint32_t *)ctx.nss_base;
    2938:	e59f3024 	ldr	r3, [pc, #36]	@ 2964 <nss_gmac_ifg_reset+0x2c>

	val = nss_gmac_read_reg(nss_base, NSS_GMACn_CTL(gmac_id));
    293c:	e280000c 	add	r0, r0, #12
	addr = (uint32_t)regbase + regoffset;
    2940:	e5932008 	ldr	r2, [r3, #8]
    2944:	e0822100 	add	r2, r2, r0, lsl #2
	asm volatile("ldr %0, %1"
    2948:	e5923000 	ldr	r3, [r2]
	val &= ~(IFG_MASK | GMAC_IFG_LIMIT(IFG_MASK));
    294c:	e3c33c3f 	bic	r3, r3, #16128	@ 0x3f00
    2950:	e3c3303f 	bic	r3, r3, #63	@ 0x3f
	val |= (GMAC_IFG_CTL(GMAC_IFG) | GMAC_IFG_LIMIT(GMAC_IFG));
    2954:	e3833b03 	orr	r3, r3, #3072	@ 0xc00
    2958:	e383300c 	orr	r3, r3, #12
	asm volatile("str %1, %0"
    295c:	e5823000 	str	r3, [r2]
	nss_gmac_write_reg(nss_base, NSS_GMACn_CTL(gmac_id), val);
}
    2960:	e12fff1e 	bx	lr
    2964:	00000000 	.word	0x00000000

00002968 <nss_macsec_pre_init>:
{
    2968:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    296c:	e3a03005 	mov	r3, #5
	uint32_t *nss_base = (uint32_t *)ctx.nss_base;
    2970:	e59f807c 	ldr	r8, [pc, #124]	@ 29f4 <nss_macsec_pre_init+0x8c>
	addr = (uint32_t)regbase + regoffset;
    2974:	e5984008 	ldr	r4, [r8, #8]
    2978:	e584301c 	str	r3, [r4, #28]
    297c:	e5843020 	str	r3, [r4, #32]
    2980:	e5843024 	str	r3, [r4, #36]	@ 0x24
    2984:	e5983010 	ldr	r3, [r8, #16]
    2988:	e3036e28 	movw	r6, #15912	@ 0x3e28
    298c:	e3a02001 	mov	r2, #1
    2990:	e0831006 	add	r1, r3, r6
    2994:	e5812000 	str	r2, [r1]
    2998:	e3035e2c 	movw	r5, #15916	@ 0x3e2c
    299c:	e0831005 	add	r1, r3, r5
    29a0:	e5812000 	str	r2, [r1]
    29a4:	e3037e30 	movw	r7, #15920	@ 0x3e30
    29a8:	e0833007 	add	r3, r3, r7
    29ac:	e5832000 	str	r2, [r3]
	msleep(100);
    29b0:	e3a00064 	mov	r0, #100	@ 0x64
    29b4:	ebfffffe 	bl	0 <msleep>
    29b8:	e5983010 	ldr	r3, [r8, #16]
    29bc:	e3a02000 	mov	r2, #0
    29c0:	e0836006 	add	r6, r3, r6
    29c4:	e5862000 	str	r2, [r6]
    29c8:	e0835005 	add	r5, r3, r5
    29cc:	e5852000 	str	r2, [r5]
    29d0:	e0833007 	add	r3, r3, r7
    29d4:	e5832000 	str	r2, [r3]
	asm volatile("ldr %0, %1"
    29d8:	e5943008 	ldr	r3, [r4, #8]
	val |= (MACSEC_CORE_CLKEN_VAL | MACSEC_GMII_RX_CLKEN_VAL |
    29dc:	e3833477 	orr	r3, r3, #1996488704	@ 0x77000000
    29e0:	e3833607 	orr	r3, r3, #7340032	@ 0x700000
	asm volatile("str %1, %0"
    29e4:	e5843008 	str	r3, [r4, #8]
    29e8:	e3a03077 	mov	r3, #119	@ 0x77
    29ec:	e5843028 	str	r3, [r4, #40]	@ 0x28
}
    29f0:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    29f4:	00000000 	.word	0x00000000

000029f8 <nss_gmac_link_status_set>:
 * @param[in] link_state
 * @return void
 */
static void nss_gmac_link_status_set(uint32_t gmac_id, uint32_t link_state)
{
	struct nss_gmac_dev *gmac_dev = ctx.nss_gmac[gmac_id];
    29f8:	e59f303c 	ldr	r3, [pc, #60]	@ 2a3c <nss_gmac_link_status_set+0x44>
    29fc:	e0833100 	add	r3, r3, r0, lsl #2
    2a00:	e593001c 	ldr	r0, [r3, #28]
	if (gmac_dev == NULL)
    2a04:	e3500000 	cmp	r0, #0
    2a08:	012fff1e 	bxeq	lr
    2a0c:	e5903018 	ldr	r3, [r0, #24]
		return;

	if (!test_bit(__NSS_GMAC_UP, &gmac_dev->flags))
    2a10:	e3130001 	tst	r3, #1
    2a14:	012fff1e 	bxeq	lr
		return;

	if (link_state == LINKDOWN && gmac_dev->link_state == LINKUP)
    2a18:	e3510000 	cmp	r1, #0
    2a1c:	e5903064 	ldr	r3, [r0, #100]	@ 0x64
    2a20:	1a000002 	bne	2a30 <nss_gmac_link_status_set+0x38>
    2a24:	e3530001 	cmp	r3, #1
    2a28:	112fff1e 	bxne	lr
		nss_gmac_linkdown(gmac_dev);
    2a2c:	eafffffe 	b	3a4 <nss_gmac_loopback_off>
	else if (link_state == LINKUP && gmac_dev->link_state == LINKDOWN)
    2a30:	e3530000 	cmp	r3, #0
    2a34:	112fff1e 	bxne	lr
		nss_gmac_linkup(gmac_dev);
    2a38:	eafffffe 	b	4dc4 <nss_gmac_linkup>
    2a3c:	00000000 	.word	0x00000000

00002a40 <nss_macsec_bypass_en_set>:
 * @return void
 */
void nss_macsec_bypass_en_set(uint32_t gmac_id, bool enable)
{
	uint32_t val = 0;
	uint32_t *nss_base = (uint32_t *)ctx.nss_base;
    2a40:	e59f30f4 	ldr	r3, [pc, #244]	@ 2b3c <nss_macsec_bypass_en_set+0xfc>
{
    2a44:	e92d4ff7 	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr}
	struct nss_gmac_dev *gmac_dev = NULL;
	uint32_t link_reset_flag = 0;
	struct nss_gmac_speed_ctx gmac_speed_ctx = {0, 0};

	if ((gmac_id == 0) || (gmac_id > 3))
    2a48:	e240a001 	sub	sl, r0, #1
	struct nss_gmac_speed_ctx gmac_speed_ctx = {0, 0};
    2a4c:	e3a0b000 	mov	fp, #0
	if ((gmac_id == 0) || (gmac_id > 3))
    2a50:	e35a0002 	cmp	sl, #2
	uint32_t *nss_base = (uint32_t *)ctx.nss_base;
    2a54:	e5938008 	ldr	r8, [r3, #8]
	struct nss_gmac_speed_ctx gmac_speed_ctx = {0, 0};
    2a58:	e58db000 	str	fp, [sp]
    2a5c:	e58db004 	str	fp, [sp, #4]
	if ((gmac_id == 0) || (gmac_id > 3))
    2a60:	8a000033 	bhi	2b34 <nss_macsec_bypass_en_set+0xf4>
		return;

	gmac_dev = ctx.nss_gmac[gmac_id];
    2a64:	e0833100 	add	r3, r3, r0, lsl #2
    2a68:	e593501c 	ldr	r5, [r3, #28]
	if (gmac_dev == NULL)
    2a6c:	e155000b 	cmp	r5, fp
    2a70:	0a00002f 	beq	2b34 <nss_macsec_bypass_en_set+0xf4>
    2a74:	e1a04000 	mov	r4, r0
		return;

	mutex_lock(&gmac_dev->link_mutex);
    2a78:	e2850e27 	add	r0, r5, #624	@ 0x270
    2a7c:	e1a06001 	mov	r6, r1
    2a80:	ebfffffe 	bl	0 <mutex_lock>

	/* If gmac is in link up state, it need to simulate link down event
	 * before setting IFG and simulate link up event after the operation
	 */
	if (gmac_dev->link_state == LINKUP)
    2a84:	e5957064 	ldr	r7, [r5, #100]	@ 0x64
    2a88:	e3570001 	cmp	r7, #1
	uint32_t link_reset_flag = 0;
    2a8c:	11a0700b 	movne	r7, fp
	if (gmac_dev->link_state == LINKUP)
    2a90:	1a000003 	bne	2aa4 <nss_macsec_bypass_en_set+0x64>
		link_reset_flag = 1;

	/* simulate a gmac link down event */
	if (link_reset_flag)
		nss_gmac_link_status_set(gmac_id, LINKDOWN);
    2a94:	e1a0100b 	mov	r1, fp
    2a98:	e1a00004 	mov	r0, r4
    2a9c:	ebffffd5 	bl	29f8 <nss_gmac_link_status_set>
    2aa0:	eaffffff 	b	2aa4 <nss_macsec_bypass_en_set+0x64>

	/* Set MACSEC_IFG value */
	if (enable) {
    2aa4:	e3560000 	cmp	r6, #0
    2aa8:	0a000002 	beq	2ab8 <nss_macsec_bypass_en_set+0x78>
		nss_gmac_ifg_reset(gmac_id);
    2aac:	e1a00004 	mov	r0, r4
    2ab0:	ebffffa0 	bl	2938 <nss_gmac_ifg_reset>
    2ab4:	ea000007 	b	2ad8 <nss_macsec_bypass_en_set+0x98>
	} else {
		val = nss_gmac_read_reg(nss_base, NSS_GMACn_CTL(gmac_id));
    2ab8:	e284200c 	add	r2, r4, #12
	addr = (uint32_t)regbase + regoffset;
    2abc:	e0882102 	add	r2, r8, r2, lsl #2
	asm volatile("ldr %0, %1"
    2ac0:	e5923000 	ldr	r3, [r2]
		val &= ~(IFG_MASK | GMAC_IFG_LIMIT(IFG_MASK));
    2ac4:	e3c33c3f 	bic	r3, r3, #16128	@ 0x3f00
    2ac8:	e3c3303f 	bic	r3, r3, #63	@ 0x3f
		val |= (GMAC_IFG_CTL(MACSEC_IFG) | GMAC_IFG_LIMIT(MACSEC_IFG));
    2acc:	e3833c2d 	orr	r3, r3, #11520	@ 0x2d00
    2ad0:	e383302d 	orr	r3, r3, #45	@ 0x2d
	asm volatile("str %1, %0"
    2ad4:	e5823000 	str	r3, [r2]
	asm volatile("ldr %0, %1"
    2ad8:	e5983028 	ldr	r3, [r8, #40]	@ 0x28

	/* Enable/Disable MACSEC for related port */
	val = nss_gmac_read_reg(nss_base, NSS_MACSEC_CTL);
	val |= MACSEC_DP_RST_VAL;
	if (enable)
		val |= (1<<(gmac_id - 1));
    2adc:	e3a02001 	mov	r2, #1
	val |= MACSEC_DP_RST_VAL;
    2ae0:	e3833070 	orr	r3, r3, #112	@ 0x70
	if (enable)
    2ae4:	e3560000 	cmp	r6, #0
		val |= (1<<(gmac_id - 1));
    2ae8:	e1a02a12 	lsl	r2, r2, sl
    2aec:	11823003 	orrne	r3, r2, r3
	else
		val &= ~(1<<(gmac_id - 1));
    2af0:	01c33002 	biceq	r3, r3, r2
	asm volatile("str %1, %0"
    2af4:	e5883028 	str	r3, [r8, #40]	@ 0x28
	nss_gmac_write_reg(nss_base, NSS_MACSEC_CTL, val);

	/* simulate a gmac link up event */
	if (link_reset_flag)
    2af8:	e3570000 	cmp	r7, #0
    2afc:	0a000002 	beq	2b0c <nss_macsec_bypass_en_set+0xcc>
		nss_gmac_link_status_set(gmac_id, LINKUP);
    2b00:	e3a01001 	mov	r1, #1
    2b04:	e1a00004 	mov	r0, r4
    2b08:	ebffffba 	bl	29f8 <nss_gmac_link_status_set>

	mutex_unlock(&gmac_dev->link_mutex);
    2b0c:	e2850e27 	add	r0, r5, #624	@ 0x270
    2b10:	ebfffffe 	bl	0 <mutex_unlock>

	/* Set MACSEC speed */
	gmac_speed_ctx.mac_id = gmac_dev->macid;
    2b14:	e595300c 	ldr	r3, [r5, #12]
	gmac_speed_ctx.speed = gmac_dev->speed;
	blocking_notifier_call_chain(&nss_gmac_notifier_list,
    2b18:	e1a0200d 	mov	r2, sp
    2b1c:	e3a01001 	mov	r1, #1
    2b20:	e59f0018 	ldr	r0, [pc, #24]	@ 2b40 <nss_macsec_bypass_en_set+0x100>
	gmac_speed_ctx.mac_id = gmac_dev->macid;
    2b24:	e58d3000 	str	r3, [sp]
	gmac_speed_ctx.speed = gmac_dev->speed;
    2b28:	e595306c 	ldr	r3, [r5, #108]	@ 0x6c
    2b2c:	e58d3004 	str	r3, [sp, #4]
	blocking_notifier_call_chain(&nss_gmac_notifier_list,
    2b30:	ebfffffe 	bl	0 <blocking_notifier_call_chain>
					NSS_GMAC_SPEED_SET, &gmac_speed_ctx);
}
    2b34:	e28dd00c 	add	sp, sp, #12
    2b38:	e8bd8ff0 	pop	{r4, r5, r6, r7, r8, r9, sl, fp, pc}
    2b3c:	00000000 	.word	0x00000000
    2b40:	00000330 	.word	0x00000330

00002b44 <nss_macsec_pre_exit>:
	struct nss_gmac_dev *gmac_dev = NULL;
	uint32_t gmac_id = 0;
	uint32_t link_reset_flag = 0;

	/* MACSEC reset */
	nss_gmac_write_reg(ctx.clk_ctl_base, MACSEC_CORE1_RESET, 1);
    2b44:	e59f209c 	ldr	r2, [pc, #156]	@ 2be8 <nss_macsec_pre_exit+0xa4>
{
    2b48:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    2b4c:	e3a04001 	mov	r4, #1
	addr = (uint32_t)regbase + regoffset;
    2b50:	e5923010 	ldr	r3, [r2, #16]
    2b54:	e2831c3e 	add	r1, r3, #15872	@ 0x3e00
    2b58:	e5814028 	str	r4, [r1, #40]	@ 0x28
    2b5c:	e2831c3e 	add	r1, r3, #15872	@ 0x3e00
    2b60:	e581402c 	str	r4, [r1, #44]	@ 0x2c
    2b64:	e2833c3e 	add	r3, r3, #15872	@ 0x3e00
    2b68:	e5834030 	str	r4, [r3, #48]	@ 0x30
    2b6c:	e5923008 	ldr	r3, [r2, #8]
    2b70:	e3a01077 	mov	r1, #119	@ 0x77
    2b74:	e5831028 	str	r1, [r3, #40]	@ 0x28
	/* Bypass all MACSECs */
	nss_gmac_write_reg(nss_base, NSS_MACSEC_CTL,
				MACSEC_EXT_BYPASS_EN_MASK | MACSEC_DP_RST_VAL);

	/* Reset GMAC_IFG value */
	for (gmac_id = 1; gmac_id < 4; gmac_id++) {
    2b78:	e282501c 	add	r5, r2, #28
		gmac_dev = ctx.nss_gmac[gmac_id];
    2b7c:	e5b56004 	ldr	r6, [r5, #4]!
		if (gmac_dev == NULL)
    2b80:	e3560000 	cmp	r6, #0
    2b84:	0a000013 	beq	2bd8 <nss_macsec_pre_exit+0x94>
		 * event before setting IFG and simulate link up event after the
		 * operation
		 */
		link_reset_flag = 0;

		mutex_lock(&gmac_dev->link_mutex);
    2b88:	e2867e27 	add	r7, r6, #624	@ 0x270
    2b8c:	e1a00007 	mov	r0, r7
    2b90:	ebfffffe 	bl	0 <mutex_lock>

		if (gmac_dev->link_state == LINKUP)
    2b94:	e5966064 	ldr	r6, [r6, #100]	@ 0x64
    2b98:	e3560001 	cmp	r6, #1
		link_reset_flag = 0;
    2b9c:	13a06000 	movne	r6, #0
		if (gmac_dev->link_state == LINKUP)
    2ba0:	1a000003 	bne	2bb4 <nss_macsec_pre_exit+0x70>
			link_reset_flag = 1;

		/* simulate a gmac link down event */
		if (link_reset_flag)
			nss_gmac_link_status_set(gmac_id, LINKDOWN);
    2ba4:	e3a01000 	mov	r1, #0
    2ba8:	e1a00004 	mov	r0, r4
    2bac:	ebffff91 	bl	29f8 <nss_gmac_link_status_set>
    2bb0:	eaffffff 	b	2bb4 <nss_macsec_pre_exit+0x70>

		nss_gmac_ifg_reset(gmac_id);
    2bb4:	e1a00004 	mov	r0, r4
    2bb8:	ebffff5e 	bl	2938 <nss_gmac_ifg_reset>

		/* simulate a gmac link up event */
		if (link_reset_flag)
    2bbc:	e3560000 	cmp	r6, #0
    2bc0:	0a000002 	beq	2bd0 <nss_macsec_pre_exit+0x8c>
			nss_gmac_link_status_set(gmac_id, LINKUP);
    2bc4:	e3a01001 	mov	r1, #1
    2bc8:	e1a00004 	mov	r0, r4
    2bcc:	ebffff89 	bl	29f8 <nss_gmac_link_status_set>

		mutex_unlock(&gmac_dev->link_mutex);
    2bd0:	e1a00007 	mov	r0, r7
    2bd4:	ebfffffe 	bl	0 <mutex_unlock>
	for (gmac_id = 1; gmac_id < 4; gmac_id++) {
    2bd8:	e2844001 	add	r4, r4, #1
    2bdc:	e3540004 	cmp	r4, #4
    2be0:	08bd81f0 	popeq	{r4, r5, r6, r7, r8, pc}
    2be4:	eaffffe4 	b	2b7c <nss_macsec_pre_exit+0x38>
    2be8:	00000000 	.word	0x00000000

00002bec <nss_gmac_link_state_change_notify_register>:
 * @brief register notifier into gmac module
 * @param[in] struct notifier_block *
 * @return void
 */
void nss_gmac_link_state_change_notify_register(struct notifier_block *nb)
{
    2bec:	e1a01000 	mov	r1, r0
	blocking_notifier_chain_register(&nss_gmac_notifier_list, nb);
    2bf0:	e59f0000 	ldr	r0, [pc]	@ 2bf8 <nss_gmac_link_state_change_notify_register+0xc>
    2bf4:	eafffffe 	b	0 <blocking_notifier_chain_register>
    2bf8:	00000330 	.word	0x00000330

00002bfc <nss_gmac_link_state_change_notify_unregister>:
 * @brief unregister notifier into gmac module
 * @param[in] struct notifier_block *
 * @return void
 */
void nss_gmac_link_state_change_notify_unregister(struct notifier_block *nb)
{
    2bfc:	e1a01000 	mov	r1, r0
	blocking_notifier_chain_unregister(&nss_gmac_notifier_list, nb);
    2c00:	e59f0000 	ldr	r0, [pc]	@ 2c08 <nss_gmac_link_state_change_notify_unregister+0xc>
    2c04:	eafffffe 	b	0 <blocking_notifier_chain_unregister>
    2c08:	00000330 	.word	0x00000330

00002c0c <nss_gmac_qsgmii_dev_init>:
{
    2c0c:	e92d40f7 	push	{r0, r1, r2, r4, r5, r6, r7, lr}
    2c10:	e1a06000 	mov	r6, r0
	uint32_t id = gmacdev->macid;
    2c14:	e590400c 	ldr	r4, [r0, #12]
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII) {
    2c18:	e5902074 	ldr	r2, [r0, #116]	@ 0x74
	uint8_t *nss_base = (uint8_t *)(gmacdev->ctx->nss_base);
    2c1c:	e5903284 	ldr	r3, [r0, #644]	@ 0x284
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII) {
    2c20:	e3520004 	cmp	r2, #4
	uint8_t *nss_base = (uint8_t *)(gmacdev->ctx->nss_base);
    2c24:	e5937008 	ldr	r7, [r3, #8]
	uint32_t *qsgmii_base = (uint32_t *)(gmacdev->ctx->qsgmii_base);
    2c28:	e593500c 	ldr	r5, [r3, #12]
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII) {
    2c2c:	1a000023 	bne	2cc0 <nss_gmac_qsgmii_dev_init+0xb4>
		switch (gmacdev->macid) {
    2c30:	e3540002 	cmp	r4, #2
			netdev_info(gmacdev->netdev, "%s: QSGMII_PHY_QSGMII_CTL(0x%x) - 0x%x\n",
    2c34:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
		switch (gmacdev->macid) {
    2c38:	0a000010 	beq	2c80 <nss_gmac_qsgmii_dev_init+0x74>
    2c3c:	e3540003 	cmp	r4, #3
    2c40:	0a000016 	beq	2ca0 <nss_gmac_qsgmii_dev_init+0x94>
    2c44:	e3540001 	cmp	r4, #1
    2c48:	1a00001c 	bne	2cc0 <nss_gmac_qsgmii_dev_init+0xb4>
			if (SOCINFO_VERSION_MAJOR(gmacdev->ctx->socver) < 2) {
    2c4c:	e5933014 	ldr	r3, [r3, #20]
				qsgmii_tx_param = QSGMII_PHY_TX_DRV_AMP(0xC)
    2c50:	e59f20fc 	ldr	r2, [pc, #252]	@ 2d54 <nss_gmac_qsgmii_dev_init+0x148>
    2c54:	e3530001 	cmp	r3, #1
			nss_gmac_write_reg((uint32_t *)qsgmii_base,
    2c58:	e59f30f8 	ldr	r3, [pc, #248]	@ 2d58 <nss_gmac_qsgmii_dev_init+0x14c>
				qsgmii_tx_param = QSGMII_PHY_TX_DRV_AMP(0xC)
    2c5c:	83a0220d 	movhi	r2, #-805306368	@ 0xd0000000
			nss_gmac_write_reg((uint32_t *)qsgmii_base,
    2c60:	e1823003 	orr	r3, r2, r3
    2c64:	e5853134 	str	r3, [r5, #308]	@ 0x134
	asm volatile("ldr %0, %1"
    2c68:	e5953134 	ldr	r3, [r5, #308]	@ 0x134
			netdev_info(gmacdev->netdev, "%s: QSGMII_PHY_QSGMII_CTL(0x%x) - 0x%x\n",
    2c6c:	e59f20e8 	ldr	r2, [pc, #232]	@ 2d5c <nss_gmac_qsgmii_dev_init+0x150>
    2c70:	e58d3000 	str	r3, [sp]
    2c74:	e3a03f4d 	mov	r3, #308	@ 0x134
    2c78:	e59f10e0 	ldr	r1, [pc, #224]	@ 2d60 <nss_gmac_qsgmii_dev_init+0x154>
    2c7c:	ea00000e 	b	2cbc <nss_gmac_qsgmii_dev_init+0xb0>
	asm volatile("str %1, %0"
    2c80:	e59f30dc 	ldr	r3, [pc, #220]	@ 2d64 <nss_gmac_qsgmii_dev_init+0x158>
    2c84:	e585313c 	str	r3, [r5, #316]	@ 0x13c
	asm volatile("ldr %0, %1"
    2c88:	e595313c 	ldr	r3, [r5, #316]	@ 0x13c
			netdev_info(gmacdev->netdev, "%s: QSGMII_PHY_SGMII_1_CTL(0x%x) - 0x%x\n",
    2c8c:	e59f20c8 	ldr	r2, [pc, #200]	@ 2d5c <nss_gmac_qsgmii_dev_init+0x150>
    2c90:	e58d3000 	str	r3, [sp]
    2c94:	e3a03f4f 	mov	r3, #316	@ 0x13c
    2c98:	e59f10c8 	ldr	r1, [pc, #200]	@ 2d68 <nss_gmac_qsgmii_dev_init+0x15c>
    2c9c:	ea000006 	b	2cbc <nss_gmac_qsgmii_dev_init+0xb0>
	asm volatile("str %1, %0"
    2ca0:	e59f30bc 	ldr	r3, [pc, #188]	@ 2d64 <nss_gmac_qsgmii_dev_init+0x158>
    2ca4:	e5853140 	str	r3, [r5, #320]	@ 0x140
	asm volatile("ldr %0, %1"
    2ca8:	e5953140 	ldr	r3, [r5, #320]	@ 0x140
			netdev_info(gmacdev->netdev, "%s: QSGMII_PHY_SGMII_2_CTL(0x%x) - 0x%x\n",
    2cac:	e59f20a8 	ldr	r2, [pc, #168]	@ 2d5c <nss_gmac_qsgmii_dev_init+0x150>
    2cb0:	e58d3000 	str	r3, [sp]
    2cb4:	e3a03d05 	mov	r3, #320	@ 0x140
    2cb8:	e59f10ac 	ldr	r1, [pc, #172]	@ 2d6c <nss_gmac_qsgmii_dev_init+0x160>
    2cbc:	ebfffffe 	bl	0 <netdev_info>
	if ((gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII)
    2cc0:	e5963074 	ldr	r3, [r6, #116]	@ 0x74
    2cc4:	e3530013 	cmp	r3, #19
    2cc8:	13530004 	cmpne	r3, #4
    2ccc:	03a03001 	moveq	r3, #1
    2cd0:	13a03000 	movne	r3, #0
    2cd4:	1a000005 	bne	2cf0 <nss_gmac_qsgmii_dev_init+0xe4>
		val |= GMACn_QSGMII_RX_CLK(id) | GMACn_QSGMII_TX_CLK(id);
    2cd8:	e2844004 	add	r4, r4, #4
    2cdc:	e3a03001 	mov	r3, #1
    2ce0:	e1a04084 	lsl	r4, r4, #1
    2ce4:	e2442001 	sub	r2, r4, #1
    2ce8:	e1a04413 	lsl	r4, r3, r4
    2cec:	e1843213 	orr	r3, r4, r3, lsl r2
    2cf0:	e597202c 	ldr	r2, [r7, #44]	@ 0x2c
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    2cf4:	e1c23003 	bic	r3, r2, r3
	asm volatile("str %1, %0"
    2cf8:	e587302c 	str	r3, [r7, #44]	@ 0x2c
	asm volatile("ldr %0, %1"
    2cfc:	e597302c 	ldr	r3, [r7, #44]	@ 0x2c
	netdev_info(gmacdev->netdev, "%s: NSS_QSGMII_CLK_CTL(0x%x) - 0x%x\n",
    2d00:	e59f2054 	ldr	r2, [pc, #84]	@ 2d5c <nss_gmac_qsgmii_dev_init+0x150>
    2d04:	e58d3000 	str	r3, [sp]
    2d08:	e3a0302c 	mov	r3, #44	@ 0x2c
    2d0c:	e5960090 	ldr	r0, [r6, #144]	@ 0x90
    2d10:	e59f1058 	ldr	r1, [pc, #88]	@ 2d70 <nss_gmac_qsgmii_dev_init+0x164>
    2d14:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->forced_speed == SPEED_UNKNOWN) {
    2d18:	e5963084 	ldr	r3, [r6, #132]	@ 0x84
    2d1c:	e3730001 	cmn	r3, #1
    2d20:	1a000009 	bne	2d4c <nss_gmac_qsgmii_dev_init+0x140>
					PCS_CHn_SPEED_MASK(gmacdev->macid));
    2d24:	e596300c 	ldr	r3, [r6, #12]
    2d28:	e1a03103 	lsl	r3, r3, #2
    2d2c:	e5952080 	ldr	r2, [r5, #128]	@ 0x80
    2d30:	e3a0100c 	mov	r1, #12
    2d34:	e1c22311 	bic	r2, r2, r1, lsl r3
	asm volatile("str %1, %0"
    2d38:	e5852080 	str	r2, [r5, #128]	@ 0x80
	asm volatile("ldr %0, %1"
    2d3c:	e5952080 	ldr	r2, [r5, #128]	@ 0x80
    2d40:	e3a01002 	mov	r1, #2
    2d44:	e1c23311 	bic	r3, r2, r1, lsl r3
	asm volatile("str %1, %0"
    2d48:	e5853080 	str	r3, [r5, #128]	@ 0x80
}
    2d4c:	e28dd00c 	add	sp, sp, #12
    2d50:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
    2d54:	c8000800 	.word	0xc8000800
    2d58:	0098408f 	.word	0x0098408f
    2d5c:	00000369 	.word	0x00000369
    2d60:	00000c77 	.word	0x00000c77
    2d64:	c09c408f 	.word	0xc09c408f
    2d68:	00000c9f 	.word	0x00000c9f
    2d6c:	00000cc8 	.word	0x00000cc8
    2d70:	00000cf1 	.word	0x00000cf1

00002d74 <nss_gmac_get_phy_profile>:
{
    2d74:	e92d401f 	push	{r0, r1, r2, r3, r4, lr}
	common_device_node = of_find_node_by_name(NULL, NSS_GMAC_COMMON_DEVICE_NODE);
    2d78:	e3a00000 	mov	r0, #0
    2d7c:	e59f1040 	ldr	r1, [pc, #64]	@ 2dc4 <nss_gmac_get_phy_profile+0x50>
    2d80:	ebfffffe 	bl	0 <of_find_node_by_name>
	int ret = of_property_read_variable_u32_array(np, propname, out_values,
    2d84:	e3a03000 	mov	r3, #0
    2d88:	e59f1038 	ldr	r1, [pc, #56]	@ 2dc8 <nss_gmac_get_phy_profile+0x54>
    2d8c:	e28d200c 	add	r2, sp, #12
    2d90:	e58d3000 	str	r3, [sp]
    2d94:	e3a03001 	mov	r3, #1
    2d98:	ebfffffe 	bl	0 <of_property_read_variable_u32_array>
	if (of_property_read_u32(common_device_node, "qcom,gmac-phy-profile", &gmac_phy_profile)) {
    2d9c:	e3500000 	cmp	r0, #0
	return gmac_phy_profile;
    2da0:	a59d000c 	ldrge	r0, [sp, #12]
	if (of_property_read_u32(common_device_node, "qcom,gmac-phy-profile", &gmac_phy_profile)) {
    2da4:	aa000004 	bge	2dbc <nss_gmac_get_phy_profile+0x48>
		if (of_machine_is_compatible("qcom,ipq8064"))
    2da8:	e59f001c 	ldr	r0, [pc, #28]	@ 2dcc <nss_gmac_get_phy_profile+0x58>
    2dac:	ebfffffe 	bl	0 <of_machine_is_compatible>
    2db0:	e16f0f10 	clz	r0, r0
    2db4:	e1a002a0 	lsr	r0, r0, #5
    2db8:	eaffffff 	b	2dbc <nss_gmac_get_phy_profile+0x48>
}
    2dbc:	e28dd014 	add	sp, sp, #20
    2dc0:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)
    2dc4:	00000d16 	.word	0x00000d16
    2dc8:	00000d26 	.word	0x00000d26
    2dcc:	00000d3c 	.word	0x00000d3c

00002dd0 <nss_gmac_common_init>:
{
    2dd0:	e92d4070 	push	{r4, r5, r6, lr}
    2dd4:	e1a05000 	mov	r5, r0
	nss_gmac_clear_all_regs((uint32_t *)ctx->nss_base);
    2dd8:	e5900008 	ldr	r0, [r0, #8]
    2ddc:	ebfffebf 	bl	28e0 <nss_gmac_clear_all_regs>
	addr = (uint32_t)regbase + regoffset;
    2de0:	e595400c 	ldr	r4, [r5, #12]
    2de4:	e59f3120 	ldr	r3, [pc, #288]	@ 2f0c <nss_gmac_common_init+0x13c>
    2de8:	e5843134 	str	r3, [r4, #308]	@ 0x134
    2dec:	e3a03702 	mov	r3, #524288	@ 0x80000
    2df0:	e5843120 	str	r3, [r4, #288]	@ 0x120
	if (ctx->msm_clk_ctl_enabled) {
    2df4:	e5d53018 	ldrb	r3, [r5, #24]
    2df8:	e3530000 	cmp	r3, #0
    2dfc:	0a00000c 	beq	2e34 <nss_gmac_common_init+0x64>
	addr = (uint32_t)regbase + regoffset;
    2e00:	e5953010 	ldr	r3, [r5, #16]
    2e04:	e2832c3e 	add	r2, r3, #15872	@ 0x3e00
    2e08:	e2822024 	add	r2, r2, #36	@ 0x24
	asm volatile("ldr %0, %1"
    2e0c:	e5921000 	ldr	r1, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    2e10:	e3c11001 	bic	r1, r1, #1
	asm volatile("str %1, %0"
    2e14:	e5821000 	str	r1, [r2]
	asm volatile("ldr %0, %1"
    2e18:	e5922000 	ldr	r2, [r2]
	addr = (uint32_t)regbase + regoffset;
    2e1c:	e2833da3 	add	r3, r3, #10432	@ 0x28c0
    2e20:	e283302c 	add	r3, r3, #44	@ 0x2c
    2e24:	e5932000 	ldr	r2, [r3]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    2e28:	e3c224ff 	bic	r2, r2, #-16777216	@ 0xff000000
	asm volatile("str %1, %0"
    2e2c:	e5832000 	str	r2, [r3]
	asm volatile("ldr %0, %1"
    2e30:	e5933000 	ldr	r3, [r3]
    2e34:	e5903028 	ldr	r3, [r0, #40]	@ 0x28
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    2e38:	e3833007 	orr	r3, r3, #7
	asm volatile("str %1, %0"
    2e3c:	e5803028 	str	r3, [r0, #40]	@ 0x28
	asm volatile("ldr %0, %1"
    2e40:	e5903028 	ldr	r3, [r0, #40]	@ 0x28
	if (nss_gmac_get_phy_profile() == NSS_GMAC_PHY_PROFILE_QS) {
    2e44:	ebfffffe 	bl	2d74 <nss_gmac_get_phy_profile>
    2e48:	e3500002 	cmp	r0, #2
    2e4c:	1a00000a 	bne	2e7c <nss_gmac_common_init+0xac>
	asm volatile("str %1, %0"
    2e50:	e3a03001 	mov	r3, #1
    2e54:	e5843128 	str	r3, [r4, #296]	@ 0x128
    2e58:	e5843064 	str	r3, [r4, #100]	@ 0x64
	asm volatile("ldr %0, %1"
    2e5c:	e5942134 	ldr	r2, [r4, #308]	@ 0x134
		val &= ~(QSGMII_PHY_TX_DRV_AMP_MASK
    2e60:	e59f30a8 	ldr	r3, [pc, #168]	@ 2f10 <nss_gmac_common_init+0x140>
    2e64:	e0023003 	and	r3, r2, r3
		val |= (QSGMII_PHY_TX_DRV_AMP(0xD)
    2e68:	e383320d 	orr	r3, r3, #-805306368	@ 0xd0000000
    2e6c:	e3833601 	orr	r3, r3, #1048576	@ 0x100000
	asm volatile("str %1, %0"
    2e70:	e5843134 	str	r3, [r4, #308]	@ 0x134
	asm volatile("ldr %0, %1"
    2e74:	e5943134 	ldr	r3, [r4, #308]	@ 0x134
		goto out;
    2e78:	ea000002 	b	2e88 <nss_gmac_common_init+0xb8>
	asm volatile("str %1, %0"
    2e7c:	e3a03000 	mov	r3, #0
    2e80:	e5843128 	str	r3, [r4, #296]	@ 0x128
    2e84:	e5843064 	str	r3, [r4, #100]	@ 0x64
	asm volatile("ldr %0, %1"
    2e88:	e5943128 	ldr	r3, [r4, #296]	@ 0x128
    2e8c:	e5943064 	ldr	r3, [r4, #100]	@ 0x64
    2e90:	e5943068 	ldr	r3, [r4, #104]	@ 0x68
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    2e94:	e3c33003 	bic	r3, r3, #3
	asm volatile("str %1, %0"
    2e98:	e5843068 	str	r3, [r4, #104]	@ 0x68
	asm volatile("ldr %0, %1"
    2e9c:	e5943068 	ldr	r3, [r4, #104]	@ 0x68
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    2ea0:	e3833002 	orr	r3, r3, #2
	asm volatile("str %1, %0"
    2ea4:	e5843068 	str	r3, [r4, #104]	@ 0x68
	if (ctx->msm_clk_ctl_enabled) {
    2ea8:	e5d53018 	ldrb	r3, [r5, #24]
    2eac:	e3530000 	cmp	r3, #0
    2eb0:	0a00000d 	beq	2eec <nss_gmac_common_init+0x11c>
	addr = (uint32_t)regbase + regoffset;
    2eb4:	e5953010 	ldr	r3, [r5, #16]
    2eb8:	e3036b60 	movw	r6, #15200	@ 0x3b60
    2ebc:	e3e0233f 	mvn	r2, #-67108864	@ 0xfc000000
    2ec0:	e0833006 	add	r3, r3, r6
    2ec4:	e5832000 	str	r2, [r3]
	udelay(100);
    2ec8:	e59f3044 	ldr	r3, [pc, #68]	@ 2f14 <nss_gmac_common_init+0x144>
    2ecc:	e59f0044 	ldr	r0, [pc, #68]	@ 2f18 <nss_gmac_common_init+0x148>
    2ed0:	e5933004 	ldr	r3, [r3, #4]
    2ed4:	e12fff33 	blx	r3
    2ed8:	e5953010 	ldr	r3, [r5, #16]
    2edc:	e3a02000 	mov	r2, #0
    2ee0:	e0833006 	add	r3, r3, r6
    2ee4:	e5832000 	str	r2, [r3]
	asm volatile("ldr %0, %1"
    2ee8:	e5933000 	ldr	r3, [r3]
	asm volatile("str %1, %0"
    2eec:	e59f3028 	ldr	r3, [pc, #40]	@ 2f1c <nss_gmac_common_init+0x14c>
    2ef0:	e5843020 	str	r3, [r4, #32]
	asm volatile("ldr %0, %1"
    2ef4:	e5943020 	ldr	r3, [r4, #32]
    2ef8:	e5943080 	ldr	r3, [r4, #128]	@ 0x80
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    2efc:	e383320f 	orr	r3, r3, #-268435456	@ 0xf0000000
	asm volatile("str %1, %0"
    2f00:	e5843080 	str	r3, [r4, #128]	@ 0x80
}
    2f04:	e3a00000 	mov	r0, #0
    2f08:	e8bd8070 	pop	{r4, r5, r6, pc}
    2f0c:	c898288f 	.word	0xc898288f
    2f10:	03cff3ff 	.word	0x03cff3ff
    2f14:	00000000 	.word	0x00000000
    2f18:	0147adf0 	.word	0x0147adf0
    2f1c:	000f7933 	.word	0x000f7933

00002f20 <nss_gmac_common_deinit>:
{
    2f20:	e92d4010 	push	{r4, lr}
    2f24:	e1a04000 	mov	r4, r0
	nss_gmac_clear_all_regs((uint32_t *)ctx->nss_base);
    2f28:	e5900008 	ldr	r0, [r0, #8]
    2f2c:	ebfffe6b 	bl	28e0 <nss_gmac_clear_all_regs>
	if (ctx->qsgmii_base) {
    2f30:	e594000c 	ldr	r0, [r4, #12]
    2f34:	e3500000 	cmp	r0, #0
    2f38:	0a000002 	beq	2f48 <nss_gmac_common_deinit+0x28>
		iounmap(ctx->qsgmii_base);
    2f3c:	ebfffffe 	bl	0 <iounmap>
		ctx->qsgmii_base = NULL;
    2f40:	e3a03000 	mov	r3, #0
    2f44:	e584300c 	str	r3, [r4, #12]
	if (ctx->clk_ctl_base) {
    2f48:	e5940010 	ldr	r0, [r4, #16]
    2f4c:	e3500000 	cmp	r0, #0
    2f50:	0a000002 	beq	2f60 <nss_gmac_common_deinit+0x40>
		iounmap(ctx->clk_ctl_base);
    2f54:	ebfffffe 	bl	0 <iounmap>
		ctx->clk_ctl_base = NULL;
    2f58:	e3a03000 	mov	r3, #0
    2f5c:	e5843010 	str	r3, [r4, #16]
	if (ctx->nss_base) {
    2f60:	e5940008 	ldr	r0, [r4, #8]
    2f64:	e3500000 	cmp	r0, #0
    2f68:	08bd8010 	popeq	{r4, pc}
		iounmap(ctx->nss_base);
    2f6c:	ebfffffe 	bl	0 <iounmap>
		ctx->nss_base = NULL;
    2f70:	e3a03000 	mov	r3, #0
    2f74:	e5843008 	str	r3, [r4, #8]
}
    2f78:	e8bd8010 	pop	{r4, pc}

00002f7c <nss_gmac_dev_set_speed>:
{
    2f7c:	e92d43f0 	push	{r4, r5, r6, r7, r8, r9, lr}
    2f80:	e24dd014 	sub	sp, sp, #20
	uint32_t id = gmacdev->macid;
    2f84:	e590700c 	ldr	r7, [r0, #12]
{
    2f88:	e1a04000 	mov	r4, r0
	uint32_t *nss_base = (uint32_t *)(gmacdev->ctx->nss_base);
    2f8c:	e5903284 	ldr	r3, [r0, #644]	@ 0x284
    2f90:	e5931008 	ldr	r1, [r3, #8]
	uint32_t *qsgmii_base = (uint32_t *)(gmacdev->ctx->qsgmii_base);
    2f94:	e593500c 	ldr	r5, [r3, #12]
	struct nss_gmac_speed_ctx gmac_speed_ctx = {0, 0};
    2f98:	e3a03000 	mov	r3, #0
    2f9c:	e58d3008 	str	r3, [sp, #8]
    2fa0:	e58d300c 	str	r3, [sp, #12]
	switch (gmacdev->phy_mii_type) {
    2fa4:	e5903074 	ldr	r3, [r0, #116]	@ 0x74
		netdev_info(gmacdev->netdev, "%s: Invalid MII type\n", __func__);
    2fa8:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
	switch (gmacdev->phy_mii_type) {
    2fac:	e3530009 	cmp	r3, #9
    2fb0:	0a00000c 	beq	2fe8 <nss_gmac_dev_set_speed+0x6c>
    2fb4:	e3530013 	cmp	r3, #19
    2fb8:	0a00000a 	beq	2fe8 <nss_gmac_dev_set_speed+0x6c>
    2fbc:	e3530004 	cmp	r3, #4
    2fc0:	1a000010 	bne	3008 <nss_gmac_dev_set_speed+0x8c>
		div = clk_div_sgmii(gmacdev);
    2fc4:	e594206c 	ldr	r2, [r4, #108]	@ 0x6c
	switch (gmacdev->speed) {
    2fc8:	e352000a 	cmp	r2, #10
		div = SGMII_CLK_DIV_10;
    2fcc:	03a02031 	moveq	r2, #49	@ 0x31
	switch (gmacdev->speed) {
    2fd0:	0a000011 	beq	301c <nss_gmac_dev_set_speed+0xa0>
    2fd4:	e2422064 	sub	r2, r2, #100	@ 0x64
    2fd8:	e16f2f12 	clz	r2, r2
    2fdc:	e1a022a2 	lsr	r2, r2, #5
    2fe0:	e1a02102 	lsl	r2, r2, #2
    2fe4:	ea00000c 	b	301c <nss_gmac_dev_set_speed+0xa0>
		div = clk_div_qsgmii(gmacdev);
    2fe8:	e594206c 	ldr	r2, [r4, #108]	@ 0x6c
	switch (gmacdev->speed) {
    2fec:	e352000a 	cmp	r2, #10
		div = QSGMII_CLK_DIV_10;
    2ff0:	03a02063 	moveq	r2, #99	@ 0x63
	switch (gmacdev->speed) {
    2ff4:	0a000008 	beq	301c <nss_gmac_dev_set_speed+0xa0>
		div = QSGMII_CLK_DIV_100;
    2ff8:	e3520064 	cmp	r2, #100	@ 0x64
    2ffc:	13a02001 	movne	r2, #1
    3000:	03a02009 	moveq	r2, #9
    3004:	ea000004 	b	301c <nss_gmac_dev_set_speed+0xa0>
		netdev_info(gmacdev->netdev, "%s: Invalid MII type\n", __func__);
    3008:	e59f2168 	ldr	r2, [pc, #360]	@ 3178 <nss_gmac_dev_set_speed+0x1fc>
    300c:	e59f1168 	ldr	r1, [pc, #360]	@ 317c <nss_gmac_dev_set_speed+0x200>
    3010:	ebfffffe 	bl	0 <netdev_info>
		return -EINVAL;
    3014:	e3e00015 	mvn	r0, #21
    3018:	ea000054 	b	3170 <nss_gmac_dev_set_speed+0x1f4>
	if (((gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII) ||
    301c:	e3530013 	cmp	r3, #19
    3020:	13530004 	cmpne	r3, #4
	if (gmacdev->forced_speed != SPEED_UNKNOWN)
    3024:	e5946084 	ldr	r6, [r4, #132]	@ 0x84
	if (((gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII) ||
    3028:	0a000005 	beq	3044 <nss_gmac_dev_set_speed+0xc8>
		clk |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    302c:	e1a03087 	lsl	r3, r7, #1
    3030:	e3a0c001 	mov	ip, #1
    3034:	e283e009 	add	lr, r3, #9
    3038:	e2833008 	add	r3, r3, #8
    303c:	e1a0331c 	lsl	r3, ip, r3
    3040:	ea000018 	b	30a8 <nss_gmac_dev_set_speed+0x12c>
			&& (force_speed == 1)) {
    3044:	e3760001 	cmn	r6, #1
		clk |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    3048:	e287e004 	add	lr, r7, #4
			&& (force_speed == 1)) {
    304c:	0a000013 	beq	30a0 <nss_gmac_dev_set_speed+0x124>
			pcs_speed = get_pcs_speed(gmacdev);
    3050:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
	switch (gmacdev->speed) {
    3054:	e353000a 	cmp	r3, #10
		speed = PCS_CH_SPEED_10;
    3058:	03a08000 	moveq	r8, #0
	switch (gmacdev->speed) {
    305c:	0a000003 	beq	3070 <nss_gmac_dev_set_speed+0xf4>
		speed = PCS_CH_SPEED_1000;
    3060:	e3530064 	cmp	r3, #100	@ 0x64
    3064:	03a08004 	moveq	r8, #4
    3068:	13a08008 	movne	r8, #8
    306c:	eaffffff 	b	3070 <nss_gmac_dev_set_speed+0xf4>
						PCS_CHn_FORCE_SPEED(id));
    3070:	e1a03107 	lsl	r3, r7, #2
	asm volatile("ldr %0, %1"
    3074:	e595c080 	ldr	ip, [r5, #128]	@ 0x80
    3078:	e3a09002 	mov	r9, #2
    307c:	e18cc319 	orr	ip, ip, r9, lsl r3
	asm volatile("str %1, %0"
    3080:	e585c080 	str	ip, [r5, #128]	@ 0x80
	asm volatile("ldr %0, %1"
    3084:	e595c080 	ldr	ip, [r5, #128]	@ 0x80
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    3088:	e3a0900c 	mov	r9, #12
    308c:	e1ccc319 	bic	ip, ip, r9, lsl r3
	asm volatile("str %1, %0"
    3090:	e585c080 	str	ip, [r5, #128]	@ 0x80
	asm volatile("ldr %0, %1"
    3094:	e595c080 	ldr	ip, [r5, #128]	@ 0x80
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    3098:	e18c3318 	orr	r3, ip, r8, lsl r3
	asm volatile("str %1, %0"
    309c:	e5853080 	str	r3, [r5, #128]	@ 0x80
		clk |= GMACn_GMII_RX_CLK(id) | GMACn_GMII_TX_CLK(id);
    30a0:	e3a0c001 	mov	ip, #1
    30a4:	e1a0371c 	lsl	r3, ip, r7
    30a8:	e1833e1c 	orr	r3, r3, ip, lsl lr
	asm volatile("ldr %0, %1"
    30ac:	e591c008 	ldr	ip, [r1, #8]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    30b0:	e1ccc003 	bic	ip, ip, r3
	asm volatile("str %1, %0"
    30b4:	e581c008 	str	ip, [r1, #8]
	asm volatile("ldr %0, %1"
    30b8:	e591c00c 	ldr	ip, [r1, #12]
	val &= ~GMACn_CLK_DIV(id, GMACn_CLK_DIV_SIZE);
    30bc:	e1a07187 	lsl	r7, r7, #3
    30c0:	e3a0e07f 	mov	lr, #127	@ 0x7f
    30c4:	e1ccc71e 	bic	ip, ip, lr, lsl r7
	val |= GMACn_CLK_DIV(id, div);
    30c8:	e18c2712 	orr	r2, ip, r2, lsl r7
	asm volatile("str %1, %0"
    30cc:	e581200c 	str	r2, [r1, #12]
	asm volatile("ldr %0, %1"
    30d0:	e5912008 	ldr	r2, [r1, #8]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    30d4:	e1833002 	orr	r3, r3, r2
	asm volatile("str %1, %0"
    30d8:	e5813008 	str	r3, [r1, #8]
	asm volatile("ldr %0, %1"
    30dc:	e591300c 	ldr	r3, [r1, #12]
	netdev_info(gmacdev->netdev, "%s:NSS_ETH_CLK_DIV0(0x%x) - 0x%x\n",
    30e0:	e59f2090 	ldr	r2, [pc, #144]	@ 3178 <nss_gmac_dev_set_speed+0x1fc>
    30e4:	e58d3000 	str	r3, [sp]
    30e8:	e3a0300c 	mov	r3, #12
    30ec:	e59f108c 	ldr	r1, [pc, #140]	@ 3180 <nss_gmac_dev_set_speed+0x204>
    30f0:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII
    30f4:	e5943074 	ldr	r3, [r4, #116]	@ 0x74
    30f8:	e3530013 	cmp	r3, #19
    30fc:	13530004 	cmpne	r3, #4
    3100:	1a000011 	bne	314c <nss_gmac_dev_set_speed+0x1d0>
					PCS_MODE_CTL_CHn_AUTONEG_EN(id));
    3104:	e3a03080 	mov	r3, #128	@ 0x80
    3108:	e5952068 	ldr	r2, [r5, #104]	@ 0x68
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    310c:	e1c22713 	bic	r2, r2, r3, lsl r7
	asm volatile("str %1, %0"
    3110:	e5852068 	str	r2, [r5, #104]	@ 0x68
		if (!force_speed) {
    3114:	e3760001 	cmn	r6, #1
    3118:	1a000002 	bne	3128 <nss_gmac_dev_set_speed+0x1ac>
	asm volatile("ldr %0, %1"
    311c:	e5952068 	ldr	r2, [r5, #104]	@ 0x68
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    3120:	e1823713 	orr	r3, r2, r3, lsl r7
	asm volatile("str %1, %0"
    3124:	e5853068 	str	r3, [r5, #104]	@ 0x68
	asm volatile("ldr %0, %1"
    3128:	e5953068 	ldr	r3, [r5, #104]	@ 0x68
		netdev_info(gmacdev->netdev, "%s: qsgmii_base(0x%x) + PCS_MODE_CTL(0x%x): 0x%x\n",
    312c:	e58d3004 	str	r3, [sp, #4]
    3130:	e3a03068 	mov	r3, #104	@ 0x68
    3134:	e59f203c 	ldr	r2, [pc, #60]	@ 3178 <nss_gmac_dev_set_speed+0x1fc>
    3138:	e59f1044 	ldr	r1, [pc, #68]	@ 3184 <nss_gmac_dev_set_speed+0x208>
    313c:	e58d3000 	str	r3, [sp]
    3140:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    3144:	e1a03005 	mov	r3, r5
    3148:	ebfffffe 	bl	0 <netdev_info>
	gmac_speed_ctx.mac_id = gmacdev->macid;
    314c:	e594300c 	ldr	r3, [r4, #12]
	blocking_notifier_call_chain(&nss_gmac_notifier_list,
    3150:	e28d2008 	add	r2, sp, #8
    3154:	e3a01001 	mov	r1, #1
    3158:	e59f0028 	ldr	r0, [pc, #40]	@ 3188 <nss_gmac_dev_set_speed+0x20c>
	gmac_speed_ctx.mac_id = gmacdev->macid;
    315c:	e58d3008 	str	r3, [sp, #8]
	gmac_speed_ctx.speed = gmacdev->speed;
    3160:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
    3164:	e58d300c 	str	r3, [sp, #12]
	blocking_notifier_call_chain(&nss_gmac_notifier_list,
    3168:	ebfffffe 	bl	0 <blocking_notifier_call_chain>
	return 0;
    316c:	e3a00000 	mov	r0, #0
}
    3170:	e28dd014 	add	sp, sp, #20
    3174:	e8bd83f0 	pop	{r4, r5, r6, r7, r8, r9, pc}
    3178:	00000382 	.word	0x00000382
    317c:	00000d49 	.word	0x00000d49
    3180:	00000d5f 	.word	0x00000d5f
    3184:	00000d81 	.word	0x00000d81
    3188:	00000330 	.word	0x00000330

0000318c <nss_gmac_dev_init>:
{
    318c:	e92d47ff 	push	{r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, lr}
    3190:	e1a05000 	mov	r5, r0
	uint32_t id = gmacdev->macid;
    3194:	e590400c 	ldr	r4, [r0, #12]
	uint32_t *nss_base = (uint32_t *)(gmacdev->ctx->nss_base);
    3198:	e5908284 	ldr	r8, [r0, #644]	@ 0x284
	if (ctx->msm_clk_ctl_enabled) {
    319c:	e5d83018 	ldrb	r3, [r8, #24]
	uint32_t *nss_base = (uint32_t *)(gmacdev->ctx->nss_base);
    31a0:	e5987008 	ldr	r7, [r8, #8]
	if (ctx->msm_clk_ctl_enabled) {
    31a4:	e3530000 	cmp	r3, #0
    31a8:	0a000087 	beq	33cc <nss_gmac_dev_init+0x240>
	addr = (uint32_t)regbase + regoffset;
    31ac:	e5983010 	ldr	r3, [r8, #16]
	nss_gmac_set_reg_bits(ctx->clk_ctl_base, GMAC_COREn_CLK_FS(id),
    31b0:	e1a06284 	lsl	r6, r4, #5
    31b4:	e2862df2 	add	r2, r6, #15488	@ 0x3c80
    31b8:	e2822038 	add	r2, r2, #56	@ 0x38
    31bc:	e0821003 	add	r1, r2, r3
    31c0:	e5910000 	ldr	r0, [r1]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    31c4:	e3800008 	orr	r0, r0, #8
	asm volatile("str %1, %0"
    31c8:	e5810000 	str	r0, [r1]
	asm volatile("ldr %0, %1"
    31cc:	e5911000 	ldr	r1, [r1]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_FS(%d)(0x%x): 0x%x\n",
    31d0:	e58d4000 	str	r4, [sp]
    31d4:	e58d2004 	str	r2, [sp, #4]
    31d8:	e59f2338 	ldr	r2, [pc, #824]	@ 3518 <nss_gmac_dev_init+0x38c>
    31dc:	e58d1008 	str	r1, [sp, #8]
    31e0:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    31e4:	e59f1330 	ldr	r1, [pc, #816]	@ 351c <nss_gmac_dev_init+0x390>
    31e8:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    31ec:	e5983010 	ldr	r3, [r8, #16]
	nss_gmac_clear_reg_bits(ctx->clk_ctl_base, GMAC_COREn_CLK_SRC_CTL(id),
    31f0:	e2861df2 	add	r1, r6, #15488	@ 0x3c80
    31f4:	e2811020 	add	r1, r1, #32
    31f8:	e0812003 	add	r2, r1, r3
    31fc:	e5920000 	ldr	r0, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    3200:	e3c00007 	bic	r0, r0, #7
	asm volatile("str %1, %0"
    3204:	e5820000 	str	r0, [r2]
	asm volatile("ldr %0, %1"
    3208:	e5920000 	ldr	r0, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    320c:	e3800002 	orr	r0, r0, #2
	asm volatile("str %1, %0"
    3210:	e5820000 	str	r0, [r2]
	asm volatile("ldr %0, %1"
    3214:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_SRC_CTL(%d)(0x%x): 0x%x\n",
    3218:	e58d4000 	str	r4, [sp]
	asm volatile("str %1, %0"
    321c:	e3a0a000 	mov	sl, #0
    3220:	e98d0006 	stmib	sp, {r1, r2}
    3224:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    3228:	e59f22e8 	ldr	r2, [pc, #744]	@ 3518 <nss_gmac_dev_init+0x38c>
    322c:	e59f12ec 	ldr	r1, [pc, #748]	@ 3520 <nss_gmac_dev_init+0x394>
    3230:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    3234:	e5983010 	ldr	r3, [r8, #16]
	nss_gmac_write_reg(ctx->clk_ctl_base, GMAC_COREn_CLK_SRC0_MD(id), 0);
    3238:	e2862df2 	add	r2, r6, #15488	@ 0x3c80
    323c:	e2822024 	add	r2, r2, #36	@ 0x24
    3240:	e0821003 	add	r1, r2, r3
    3244:	e581a000 	str	sl, [r1]
	nss_gmac_write_reg(ctx->clk_ctl_base, GMAC_COREn_CLK_SRC1_MD(id), 0);
    3248:	e2869df2 	add	r9, r6, #15488	@ 0x3c80
    324c:	e2899028 	add	r9, r9, #40	@ 0x28
    3250:	e0830009 	add	r0, r3, r9
    3254:	e580a000 	str	sl, [r0]
	asm volatile("ldr %0, %1"
    3258:	e591c000 	ldr	ip, [r1]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    325c:	e38cc87f 	orr	ip, ip, #8323072	@ 0x7f0000
	asm volatile("str %1, %0"
    3260:	e581c000 	str	ip, [r1]
	asm volatile("ldr %0, %1"
    3264:	e590c000 	ldr	ip, [r0]
    3268:	e38cc87f 	orr	ip, ip, #8323072	@ 0x7f0000
	asm volatile("str %1, %0"
    326c:	e580c000 	str	ip, [r0]
	asm volatile("ldr %0, %1"
    3270:	e5911000 	ldr	r1, [r1]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_SRC0_MD(%d)(0x%x): 0x%x\n",
    3274:	e58d4000 	str	r4, [sp]
    3278:	e58d2004 	str	r2, [sp, #4]
    327c:	e59f2294 	ldr	r2, [pc, #660]	@ 3518 <nss_gmac_dev_init+0x38c>
    3280:	e58d1008 	str	r1, [sp, #8]
    3284:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    3288:	e59f1294 	ldr	r1, [pc, #660]	@ 3524 <nss_gmac_dev_init+0x398>
    328c:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    3290:	e5983010 	ldr	r3, [r8, #16]
    3294:	e0892003 	add	r2, r9, r3
    3298:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_SRC1_MD(%d)(0x%x): 0x%x\n",
    329c:	e59f1284 	ldr	r1, [pc, #644]	@ 3528 <nss_gmac_dev_init+0x39c>
    32a0:	e88d0210 	stm	sp, {r4, r9}
    32a4:	e58d2008 	str	r2, [sp, #8]
    32a8:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    32ac:	e59f2264 	ldr	r2, [pc, #612]	@ 3518 <nss_gmac_dev_init+0x38c>
    32b0:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    32b4:	e5983010 	ldr	r3, [r8, #16]
	nss_gmac_write_reg(ctx->clk_ctl_base, GMAC_COREn_CLK_SRC0_NS(id), 0);
    32b8:	e2862df2 	add	r2, r6, #15488	@ 0x3c80
    32bc:	e282202c 	add	r2, r2, #44	@ 0x2c
    32c0:	e0821003 	add	r1, r2, r3
	asm volatile("str %1, %0"
    32c4:	e581a000 	str	sl, [r1]
	nss_gmac_write_reg(ctx->clk_ctl_base, GMAC_COREn_CLK_SRC1_NS(id), 0);
    32c8:	e2869df2 	add	r9, r6, #15488	@ 0x3c80
    32cc:	e2899030 	add	r9, r9, #48	@ 0x30
    32d0:	e0830009 	add	r0, r3, r9
    32d4:	e580a000 	str	sl, [r0]
	asm volatile("ldr %0, %1"
    32d8:	e591c000 	ldr	ip, [r1]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    32dc:	e300e142 	movw	lr, #322	@ 0x142
    32e0:	e18cc00e 	orr	ip, ip, lr
	asm volatile("str %1, %0"
    32e4:	e581c000 	str	ip, [r1]
	asm volatile("ldr %0, %1"
    32e8:	e590c000 	ldr	ip, [r0]
    32ec:	e18cc00e 	orr	ip, ip, lr
	asm volatile("str %1, %0"
    32f0:	e580c000 	str	ip, [r0]
	asm volatile("ldr %0, %1"
    32f4:	e5911000 	ldr	r1, [r1]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_SRC0_NS(%d)(0x%x): 0x%x\n",
    32f8:	e58d4000 	str	r4, [sp]
    32fc:	e58d2004 	str	r2, [sp, #4]
    3300:	e59f2210 	ldr	r2, [pc, #528]	@ 3518 <nss_gmac_dev_init+0x38c>
    3304:	e58d1008 	str	r1, [sp, #8]
    3308:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    330c:	e59f1218 	ldr	r1, [pc, #536]	@ 352c <nss_gmac_dev_init+0x3a0>
    3310:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    3314:	e5983010 	ldr	r3, [r8, #16]
    3318:	e0892003 	add	r2, r9, r3
    331c:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_SRC1_NS(%d)(0x%x): 0x%x\n",
    3320:	e59f1208 	ldr	r1, [pc, #520]	@ 3530 <nss_gmac_dev_init+0x3a4>
    3324:	e88d0210 	stm	sp, {r4, r9}
    3328:	e58d2008 	str	r2, [sp, #8]
    332c:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    3330:	e59f21e0 	ldr	r2, [pc, #480]	@ 3518 <nss_gmac_dev_init+0x38c>
    3334:	ebfffffe 	bl	0 <netdev_info>
    3338:	e5983010 	ldr	r3, [r8, #16]
    333c:	e3030c20 	movw	r0, #15392	@ 0x3c20
    3340:	e0832000 	add	r2, r3, r0
    3344:	e5921000 	ldr	r1, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    3348:	e3a0c010 	mov	ip, #16
    334c:	e1c1141c 	bic	r1, r1, ip, lsl r4
	asm volatile("str %1, %0"
    3350:	e5821000 	str	r1, [r2]
	asm volatile("ldr %0, %1"
    3354:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + CLK_HALT_NSSFAB0_NSSFAB1_STATEA(0x%x): 0x%x\n",
    3358:	e59f11d4 	ldr	r1, [pc, #468]	@ 3534 <nss_gmac_dev_init+0x3a8>
    335c:	e88d0005 	stm	sp, {r0, r2}
    3360:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    3364:	e59f21ac 	ldr	r2, [pc, #428]	@ 3518 <nss_gmac_dev_init+0x38c>
    3368:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    336c:	e5983010 	ldr	r3, [r8, #16]
	nss_gmac_clear_reg_bits(ctx->clk_ctl_base, GMAC_COREn_CLK_CTL(id),
    3370:	e2861df2 	add	r1, r6, #15488	@ 0x3c80
    3374:	e2811034 	add	r1, r1, #52	@ 0x34
    3378:	e0812003 	add	r2, r1, r3
    337c:	e5920000 	ldr	r0, [r2]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    3380:	e3c00020 	bic	r0, r0, #32
	asm volatile("str %1, %0"
    3384:	e5820000 	str	r0, [r2]
	asm volatile("ldr %0, %1"
    3388:	e5920000 	ldr	r0, [r2]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    338c:	e3800010 	orr	r0, r0, #16
	asm volatile("str %1, %0"
    3390:	e5820000 	str	r0, [r2]
	asm volatile("ldr %0, %1"
    3394:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_CTL(%d)(0x%x): 0x%x\n",
    3398:	e58d4000 	str	r4, [sp]
	nss_gmac_clear_reg_bits(ctx->clk_ctl_base, GMAC_COREn_RESET(id), 0x1);
    339c:	e2866df2 	add	r6, r6, #15488	@ 0x3c80
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_CTL(%d)(0x%x): 0x%x\n",
    33a0:	e98d0006 	stmib	sp, {r1, r2}
    33a4:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
	nss_gmac_clear_reg_bits(ctx->clk_ctl_base, GMAC_COREn_RESET(id), 0x1);
    33a8:	e286603c 	add	r6, r6, #60	@ 0x3c
	netdev_info(gmacdev->netdev, "%s: ctx->clk_ctl_base(0x%x) + GMAC_COREn_CLK_CTL(%d)(0x%x): 0x%x\n",
    33ac:	e59f2164 	ldr	r2, [pc, #356]	@ 3518 <nss_gmac_dev_init+0x38c>
    33b0:	e59f1180 	ldr	r1, [pc, #384]	@ 3538 <nss_gmac_dev_init+0x3ac>
    33b4:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
    33b8:	e5983010 	ldr	r3, [r8, #16]
    33bc:	e0833006 	add	r3, r3, r6
    33c0:	e5932000 	ldr	r2, [r3]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
    33c4:	e3c22001 	bic	r2, r2, #1
	asm volatile("str %1, %0"
    33c8:	e5832000 	str	r2, [r3]
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII)
    33cc:	e5952074 	ldr	r2, [r5, #116]	@ 0x74
    33d0:	e3a06000 	mov	r6, #0
		val |= GMAC_PHY_RGMII;
    33d4:	e59f3160 	ldr	r3, [pc, #352]	@ 353c <nss_gmac_dev_init+0x3b0>
    33d8:	e59f1160 	ldr	r1, [pc, #352]	@ 3540 <nss_gmac_dev_init+0x3b4>
    33dc:	e3520009 	cmp	r2, #9
    33e0:	01a01003 	moveq	r1, r3
	nss_gmac_write_reg(nss_base, NSS_GMACn_CTL(id), 0x0);
    33e4:	e284300c 	add	r3, r4, #12
    33e8:	e1a03103 	lsl	r3, r3, #2
	addr = (uint32_t)regbase + regoffset;
    33ec:	e0872003 	add	r2, r7, r3
    33f0:	e5826000 	str	r6, [r2]
    33f4:	e5821000 	str	r1, [r2]
	asm volatile("ldr %0, %1"
    33f8:	e5922000 	ldr	r2, [r2]
	netdev_info(gmacdev->netdev, "%s: nss_base(0x%x) + NSS_GMACn_CTL(%d)(0x%x): 0x%x\n",
    33fc:	e59f1140 	ldr	r1, [pc, #320]	@ 3544 <nss_gmac_dev_init+0x3b8>
    3400:	e58d4000 	str	r4, [sp]
    3404:	e58d3004 	str	r3, [sp, #4]
    3408:	e1a03007 	mov	r3, r7
    340c:	e58d2008 	str	r2, [sp, #8]
    3410:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    3414:	e59f212c 	ldr	r2, [pc, #300]	@ 3548 <nss_gmac_dev_init+0x3bc>
    3418:	ebfffffe 	bl	0 <netdev_info>
	gmacdev->speed = SPEED_1000;
    341c:	e3a03ffa 	mov	r3, #1000	@ 0x3e8
	switch (gmacdev->phy_mii_type) {
    3420:	e5951074 	ldr	r1, [r5, #116]	@ 0x74
	gmacdev->speed = SPEED_1000;
    3424:	e585306c 	str	r3, [r5, #108]	@ 0x6c
    3428:	e597300c 	ldr	r3, [r7, #12]
	val &= ~GMACn_CLK_DIV(id, GMACn_CLK_DIV_SIZE);
    342c:	e1a02184 	lsl	r2, r4, #3
    3430:	e3a0007f 	mov	r0, #127	@ 0x7f
	val |= GMACn_CLK_DIV(id, div);
    3434:	e3510009 	cmp	r1, #9
    3438:	13510013 	cmpne	r1, #19
    343c:	03a01001 	moveq	r1, #1
	val &= ~GMACn_CLK_DIV(id, GMACn_CLK_DIV_SIZE);
    3440:	e1c33210 	bic	r3, r3, r0, lsl r2
	val |= GMACn_CLK_DIV(id, div);
    3444:	13a01000 	movne	r1, #0
    3448:	e1833211 	orr	r3, r3, r1, lsl r2
	asm volatile("str %1, %0"
    344c:	e587300c 	str	r3, [r7, #12]
	asm volatile("ldr %0, %1"
    3450:	e597300c 	ldr	r3, [r7, #12]
	netdev_info(gmacdev->netdev, "%s: nss_base(0x%x) + NSS_ETH_CLK_DIV0(0x%x): 0x%x\n",
    3454:	e58d3004 	str	r3, [sp, #4]
    3458:	e3a0300c 	mov	r3, #12
    345c:	e59f10e8 	ldr	r1, [pc, #232]	@ 354c <nss_gmac_dev_init+0x3c0>
    3460:	e59f20e0 	ldr	r2, [pc, #224]	@ 3548 <nss_gmac_dev_init+0x3bc>
    3464:	e58d3000 	str	r3, [sp]
    3468:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    346c:	e1a03007 	mov	r3, r7
    3470:	ebfffffe 	bl	0 <netdev_info>
	if (id == 0 || id == 1) {
    3474:	e3540001 	cmp	r4, #1
		if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII)
    3478:	e5951074 	ldr	r1, [r5, #116]	@ 0x74
    347c:	e3a03001 	mov	r3, #1
	if (id == 0 || id == 1) {
    3480:	8a000001 	bhi	348c <nss_gmac_dev_init+0x300>
		if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII)
    3484:	e3510009 	cmp	r1, #9
    3488:	ea000000 	b	3490 <nss_gmac_dev_init+0x304>
		if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII)
    348c:	e3510004 	cmp	r1, #4
	val = 0;
    3490:	11a03006 	movne	r3, r6
			val |= (1 << id);
    3494:	01a03413 	lsleq	r3, r3, r4
    3498:	e5972014 	ldr	r2, [r7, #20]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
    349c:	e1833002 	orr	r3, r3, r2
	asm volatile("str %1, %0"
    34a0:	e5873014 	str	r3, [r7, #20]
		val |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    34a4:	e3a03001 	mov	r3, #1
	if (gmacdev->phy_mii_type == PHY_INTERFACE_MODE_RGMII)
    34a8:	e3510009 	cmp	r1, #9
		val |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    34ac:	12842004 	addne	r2, r4, #4
    34b0:	01a02314 	lsleq	r2, r4, r3
		val |= GMACn_GMII_RX_CLK(id) | GMACn_GMII_TX_CLK(id);
    34b4:	11a00413 	lslne	r0, r3, r4
		val |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    34b8:	02820009 	addeq	r0, r2, #9
    34bc:	02822008 	addeq	r2, r2, #8
		val |= GMACn_GMII_RX_CLK(id) | GMACn_GMII_TX_CLK(id);
    34c0:	11803213 	orrne	r3, r0, r3, lsl r2
		val |= GMACn_RGMII_RX_CLK(id) | GMACn_RGMII_TX_CLK(id);
    34c4:	01a02213 	lsleq	r2, r3, r2
    34c8:	01823013 	orreq	r3, r2, r3, lsl r0
	asm volatile("ldr %0, %1"
    34cc:	e5970008 	ldr	r0, [r7, #8]
	val |= GMACn_PTP_CLK(id);
    34d0:	e3a0c001 	mov	ip, #1
    34d4:	e2842010 	add	r2, r4, #16
    34d8:	e183321c 	orr	r3, r3, ip, lsl r2
    34dc:	e1833000 	orr	r3, r3, r0
	asm volatile("str %1, %0"
    34e0:	e5873008 	str	r3, [r7, #8]
	if ((gmacdev->phy_mii_type == PHY_INTERFACE_MODE_SGMII)
    34e4:	e3510013 	cmp	r1, #19
    34e8:	13510004 	cmpne	r1, #4
    34ec:	1a000007 	bne	3510 <nss_gmac_dev_init+0x384>
		nss_gmac_qsgmii_dev_init(gmacdev);
    34f0:	e1a00005 	mov	r0, r5
    34f4:	ebfffffe 	bl	2c0c <nss_gmac_qsgmii_dev_init>
		netdev_info(gmacdev->netdev, "SGMII Specific Init for GMAC%d Done!\n", id);
    34f8:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    34fc:	e1a02004 	mov	r2, r4
    3500:	e59f1048 	ldr	r1, [pc, #72]	@ 3550 <nss_gmac_dev_init+0x3c4>
}
    3504:	e28dd010 	add	sp, sp, #16
    3508:	e8bd47f0 	pop	{r4, r5, r6, r7, r8, r9, sl, lr}
		netdev_info(gmacdev->netdev, "SGMII Specific Init for GMAC%d Done!\n", id);
    350c:	eafffffe 	b	0 <netdev_info>
}
    3510:	e28dd010 	add	sp, sp, #16
    3514:	e8bd87f0 	pop	{r4, r5, r6, r7, r8, r9, sl, pc}
    3518:	00000399 	.word	0x00000399
    351c:	00000db3 	.word	0x00000db3
    3520:	00000df4 	.word	0x00000df4
    3524:	00000e3a 	.word	0x00000e3a
    3528:	00000e80 	.word	0x00000e80
    352c:	00000ec6 	.word	0x00000ec6
    3530:	00000f0c 	.word	0x00000f0c
    3534:	00000f52 	.word	0x00000f52
    3538:	00000f9d 	.word	0x00000f9d
    353c:	00090c0c 	.word	0x00090c0c
    3540:	00080c0c 	.word	0x00080c0c
    3544:	00000fdf 	.word	0x00000fdf
    3548:	000003b3 	.word	0x000003b3
    354c:	00001013 	.word	0x00001013
    3550:	00001046 	.word	0x00001046

00003554 <nss_gmac_get_wol>:
 * @param[in] pointer to struct ethtool_wolinfo.
 */
static void nss_gmac_get_wol(struct net_device *netdev,
			     struct ethtool_wolinfo *wol)
{
	wol->supported = 0;
    3554:	e3a03000 	mov	r3, #0
    3558:	e5813004 	str	r3, [r1, #4]
	wol->wolopts = 0;
    355c:	e5813008 	str	r3, [r1, #8]
}
    3560:	e12fff1e 	bx	lr

00003564 <nss_gmac_get_msglevel>:
 * @param[in] pointer to struct net_device.
 */
static uint32_t nss_gmac_get_msglevel(struct net_device *netdev)
{
	return 0;
}
    3564:	e3a00000 	mov	r0, #0
    3568:	e12fff1e 	bx	lr

0000356c <nss_gmac_get_priv_flags>:
static uint32_t nss_gmac_get_priv_flags(struct net_device *netdev)
{
	struct nss_gmac_dev *gmacdev = (struct nss_gmac_dev *)netdev_priv(netdev);

	return (uint32_t)gmacdev->drv_flags;
}
    356c:	e590055c 	ldr	r0, [r0, #1372]	@ 0x55c
    3570:	e12fff1e 	bx	lr

00003574 <nss_gmac_set_settings>:
	BUG_ON(gmacdev == NULL);
    3574:	e2903d15 	adds	r3, r0, #1344	@ 0x540
    3578:	1a000000 	bne	3580 <nss_gmac_set_settings+0xc>
    357c:	e7f001f2 	.word	0xe7f001f2
	if ((gmacdev->forced_speed != SPEED_UNKNOWN)
    3580:	e59025c4 	ldr	r2, [r0, #1476]	@ 0x5c4
    3584:	e3720001 	cmn	r2, #1
    3588:	0a000002 	beq	3598 <nss_gmac_set_settings+0x24>
    358c:	e5902558 	ldr	r2, [r0, #1368]	@ 0x558
	    && (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)))
    3590:	e3120040 	tst	r2, #64	@ 0x40
    3594:	0a000012 	beq	35e4 <nss_gmac_set_settings+0x70>
{
    3598:	e92d4070 	push	{r4, r5, r6, lr}
	phydev = gmacdev->phydev;
    359c:	e59057d0 	ldr	r5, [r0, #2000]	@ 0x7d0
	if (!phydev) {
    35a0:	e3550000 	cmp	r5, #0
    35a4:	0a00000c 	beq	35dc <nss_gmac_set_settings+0x68>
	if (elk->base.autoneg == AUTONEG_ENABLE) {
    35a8:	e5d1200b 	ldrb	r2, [r1, #11]
    35ac:	e1a04001 	mov	r4, r1
		set_bit(__NSS_GMAC_AUTONEG, &gmacdev->flags);
    35b0:	e3a00003 	mov	r0, #3
    35b4:	e2831018 	add	r1, r3, #24
	if (elk->base.autoneg == AUTONEG_ENABLE) {
    35b8:	e3520001 	cmp	r2, #1
    35bc:	1a000001 	bne	35c8 <nss_gmac_set_settings+0x54>
		set_bit(__NSS_GMAC_AUTONEG, &gmacdev->flags);
    35c0:	ebfffffe 	bl	0 <_set_bit>
    35c4:	ea000000 	b	35cc <nss_gmac_set_settings+0x58>
		clear_bit(__NSS_GMAC_AUTONEG, &gmacdev->flags);
    35c8:	ebfffffe 	bl	0 <_clear_bit>
	return phy_ethtool_ksettings_set(phydev, elk);
    35cc:	e1a01004 	mov	r1, r4
    35d0:	e1a00005 	mov	r0, r5
}
    35d4:	e8bd4070 	pop	{r4, r5, r6, lr}
	return phy_ethtool_ksettings_set(phydev, elk);
    35d8:	eafffffe 	b	0 <phy_ethtool_ksettings_set>
}
    35dc:	e3e00000 	mvn	r0, #0
    35e0:	e8bd8070 	pop	{r4, r5, r6, pc}
    35e4:	e3e00000 	mvn	r0, #0
    35e8:	e12fff1e 	bx	lr

000035ec <nss_gmac_get_strings>:
	switch (stringset) {
    35ec:	e3510001 	cmp	r1, #1
{
    35f0:	e92d40f0 	push	{r4, r5, r6, r7, lr}
	switch (stringset) {
    35f4:	0a000002 	beq	3604 <nss_gmac_get_strings+0x18>
    35f8:	e3510002 	cmp	r1, #2
    35fc:	18bd80f0 	popne	{r4, r5, r6, r7, pc}
    3600:	ea00002e 	b	36c0 <nss_gmac_get_strings+0xd4>
    3604:	e59f40c8 	ldr	r4, [pc, #200]	@ 36d4 <nss_gmac_get_strings+0xe8>
    3608:	e2823d21 	add	r3, r2, #2112	@ 0x840
	uint8_t *p = data;
    360c:	e1a0e002 	mov	lr, r2
    3610:	ea00000c 	b	3648 <nss_gmac_get_strings+0x5c>
			memcpy(p, gmac_gstrings_stats[i].stat_string,
    3614:	e1a06004 	mov	r6, r4
    3618:	e1a0500e 	mov	r5, lr
    361c:	e2847020 	add	r7, r4, #32
    3620:	e1a0c006 	mov	ip, r6
    3624:	e2855008 	add	r5, r5, #8
    3628:	e8bc0003 	ldm	ip!, {r0, r1}
    362c:	e15c0007 	cmp	ip, r7
    3630:	e1a0600c 	mov	r6, ip
    3634:	e5050008 	str	r0, [r5, #-8]
    3638:	e5051004 	str	r1, [r5, #-4]
    363c:	1afffff7 	bne	3620 <nss_gmac_get_strings+0x34>
			p += ETH_GSTRING_LEN;
    3640:	e28ee020 	add	lr, lr, #32
		for (i = 0; i < NSS_GMAC_STATS_LEN; i++) {
    3644:	e2844024 	add	r4, r4, #36	@ 0x24
    3648:	e15e0003 	cmp	lr, r3
    364c:	1afffff0 	bne	3614 <nss_gmac_get_strings+0x28>
    3650:	e59fe080 	ldr	lr, [pc, #128]	@ 36d8 <nss_gmac_get_strings+0xec>
    3654:	e2822d29 	add	r2, r2, #2624	@ 0xa40
    3658:	ea00000c 	b	3690 <nss_gmac_get_strings+0xa4>
			memcpy(p, gmac_gstrings_host_stats[i].stat_string,
    365c:	e1a0500e 	mov	r5, lr
    3660:	e1a04003 	mov	r4, r3
    3664:	e28e6020 	add	r6, lr, #32
    3668:	e1a0c005 	mov	ip, r5
    366c:	e2844008 	add	r4, r4, #8
    3670:	e8bc0003 	ldm	ip!, {r0, r1}
    3674:	e15c0006 	cmp	ip, r6
    3678:	e1a0500c 	mov	r5, ip
    367c:	e5040008 	str	r0, [r4, #-8]
    3680:	e5041004 	str	r1, [r4, #-4]
    3684:	1afffff7 	bne	3668 <nss_gmac_get_strings+0x7c>
			p += ETH_GSTRING_LEN;
    3688:	e2833020 	add	r3, r3, #32
		for (i = 0; i < NSS_GMAC_HOST_STATS_LEN; i++) {
    368c:	e28ee024 	add	lr, lr, #36	@ 0x24
    3690:	e1530002 	cmp	r3, r2
    3694:	08bd80f0 	popeq	{r4, r5, r6, r7, pc}
    3698:	eaffffef 	b	365c <nss_gmac_get_strings+0x70>
			memcpy(p, gmac_strings_priv_flags[i],
    369c:	e08c1003 	add	r1, ip, r3
    36a0:	e0820003 	add	r0, r2, r3
    36a4:	e281e020 	add	lr, r1, #32
    36a8:	e4914004 	ldr	r4, [r1], #4
    36ac:	e151000e 	cmp	r1, lr
    36b0:	e4804004 	str	r4, [r0], #4
    36b4:	1afffffb 	bne	36a8 <nss_gmac_get_strings+0xbc>
		for (i = 0; i < NSS_GMAC_PRIV_FLAGS_LEN; i++) {
    36b8:	e2833020 	add	r3, r3, #32
    36bc:	ea000001 	b	36c8 <nss_gmac_get_strings+0xdc>
			memcpy(p, gmac_strings_priv_flags[i],
    36c0:	e59fc014 	ldr	ip, [pc, #20]	@ 36dc <nss_gmac_get_strings+0xf0>
	switch (stringset) {
    36c4:	e3a03000 	mov	r3, #0
		for (i = 0; i < NSS_GMAC_PRIV_FLAGS_LEN; i++) {
    36c8:	e3530060 	cmp	r3, #96	@ 0x60
    36cc:	08bd80f0 	popeq	{r4, r5, r6, r7, pc}
    36d0:	eafffff1 	b	369c <nss_gmac_get_strings+0xb0>
    36d4:	000003c8 	.word	0x000003c8
    36d8:	00000d10 	.word	0x00000d10
    36dc:	00000f50 	.word	0x00000f50

000036e0 <nss_gmac_get_drvinfo>:
			__write_overflow();
	}
	if (size) {
		if (len >= p_size)
			fortify_panic(__func__);
		__underlying_memcpy(p, q, len);
    36e0:	e59f3068 	ldr	r3, [pc, #104]	@ 3750 <nss_gmac_get_drvinfo+0x70>
    36e4:	e2812004 	add	r2, r1, #4
    36e8:	e283001c 	add	r0, r3, #28
    36ec:	e493c004 	ldr	ip, [r3], #4
    36f0:	e1530000 	cmp	r3, r0
    36f4:	e482c004 	str	ip, [r2], #4
    36f8:	1afffffb 	bne	36ec <nss_gmac_get_drvinfo+0xc>
    36fc:	e1d330b0 	ldrh	r3, [r3]
    3700:	e59f004c 	ldr	r0, [pc, #76]	@ 3754 <nss_gmac_get_drvinfo+0x74>
    3704:	e1c230b0 	strh	r3, [r2]
    3708:	e3002c07 	movw	r2, #3079	@ 0xc07
		p[len] = '\0';
    370c:	e3a03000 	mov	r3, #0
		__underlying_memcpy(p, q, len);
    3710:	e19020b2 	ldrh	r2, [r0, r2]
		p[len] = '\0';
    3714:	e5c13022 	strb	r3, [r1, #34]	@ 0x22
		__underlying_memcpy(p, q, len);
    3718:	e1c122b4 	strh	r2, [r1, #36]	@ 0x24
    371c:	e59f2034 	ldr	r2, [pc, #52]	@ 3758 <nss_gmac_get_drvinfo+0x78>
		p[len] = '\0';
    3720:	e5c13027 	strb	r3, [r1, #39]	@ 0x27
    3724:	e5c13067 	strb	r3, [r1, #103]	@ 0x67
	info->n_priv_flags = __NSS_GMAC_PRIV_FLAG_MAX;
    3728:	e3a03004 	mov	r3, #4
		__underlying_memcpy(p, q, len);
    372c:	e5d22002 	ldrb	r2, [r2, #2]
    3730:	e5c12026 	strb	r2, [r1, #38]	@ 0x26
    3734:	e59f2020 	ldr	r2, [pc, #32]	@ 375c <nss_gmac_get_drvinfo+0x7c>
    3738:	e58130b0 	str	r3, [r1, #176]	@ 0xb0
    373c:	e1d200b0 	ldrh	r0, [r2]
    3740:	e5d22002 	ldrb	r2, [r2, #2]
    3744:	e1c106b4 	strh	r0, [r1, #100]	@ 0x64
    3748:	e5c12066 	strb	r2, [r1, #102]	@ 0x66
}
    374c:	e12fff1e 	bx	lr
    3750:	00000fb0 	.word	0x00000fb0
    3754:	000003c8 	.word	0x000003c8
    3758:	00000fcf 	.word	0x00000fcf
    375c:	0000106c 	.word	0x0000106c

00003760 <nss_gmac_nway_reset>:
	BUG_ON(gmacdev == NULL);
    3760:	e3700d15 	cmn	r0, #1344	@ 0x540
    3764:	1a000000 	bne	376c <nss_gmac_nway_reset+0xc>
    3768:	e7f001f2 	.word	0xe7f001f2
    376c:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
	if (!netif_running(netdev))
    3770:	e3130001 	tst	r3, #1
    3774:	0a00000a 	beq	37a4 <nss_gmac_nway_reset+0x44>
    3778:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags))
    377c:	e3130040 	tst	r3, #64	@ 0x40
    3780:	0a000009 	beq	37ac <nss_gmac_nway_reset+0x4c>
    3784:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (!test_bit(__NSS_GMAC_AUTONEG, &gmacdev->flags))
    3788:	e3130008 	tst	r3, #8
    378c:	0a000006 	beq	37ac <nss_gmac_nway_reset+0x4c>
{
    3790:	e92d4010 	push	{r4, lr}
	genphy_restart_aneg(gmacdev->phydev);
    3794:	e59007d0 	ldr	r0, [r0, #2000]	@ 0x7d0
    3798:	ebfffffe 	bl	0 <genphy_restart_aneg>
	return 0;
    379c:	e3a00000 	mov	r0, #0
}
    37a0:	e8bd8010 	pop	{r4, pc}
		return -EAGAIN;
    37a4:	e3e0000a 	mvn	r0, #10
    37a8:	e12fff1e 	bx	lr
		return -EINVAL;
    37ac:	e3e00015 	mvn	r0, #21
}
    37b0:	e12fff1e 	bx	lr

000037b4 <nss_gmac_set_priv_flags>:
{
    37b4:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    37b8:	e1a04000 	mov	r4, r0
	uint32_t changed = flags ^ gmacdev->drv_flags;
    37bc:	e590355c 	ldr	r3, [r0, #1372]	@ 0x55c
{
    37c0:	e1a06001 	mov	r6, r1
	uint32_t changed = flags ^ gmacdev->drv_flags;
    37c4:	e0237001 	eor	r7, r3, r1
	if (changed & NSS_GMAC_PRIV_FLAG(LINKPOLL)) {
    37c8:	e3170001 	tst	r7, #1
    37cc:	0a000010 	beq	3814 <nss_gmac_set_priv_flags+0x60>
    37d0:	e5902558 	ldr	r2, [r0, #1368]	@ 0x558
		if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
    37d4:	e3120040 	tst	r2, #64	@ 0x40
    37d8:	0a00005a 	beq	3948 <nss_gmac_set_priv_flags+0x194>
	struct phy_device *phydev = gmacdev->phydev;
    37dc:	e59007d0 	ldr	r0, [r0, #2000]	@ 0x7d0
		if (IS_ERR(phydev))
    37e0:	e3700a01 	cmn	r0, #4096	@ 0x1000
    37e4:	8a000055 	bhi	3940 <nss_gmac_set_priv_flags+0x18c>
		if (flags & NSS_GMAC_PRIV_FLAG(LINKPOLL)) {
    37e8:	e3110001 	tst	r1, #1
			gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(LINKPOLL);
    37ec:	03c33001 	biceq	r3, r3, #1
    37f0:	0584355c 	streq	r3, [r4, #1372]	@ 0x55c
		if (flags & NSS_GMAC_PRIV_FLAG(LINKPOLL)) {
    37f4:	0a000006 	beq	3814 <nss_gmac_set_priv_flags+0x60>
			gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(LINKPOLL);
    37f8:	e3833001 	orr	r3, r3, #1
    37fc:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
			if (phydev->autoneg == AUTONEG_ENABLE)
    3800:	e5d031f1 	ldrb	r3, [r0, #497]	@ 0x1f1
    3804:	e3130010 	tst	r3, #16
    3808:	0a000001 	beq	3814 <nss_gmac_set_priv_flags+0x60>
				genphy_restart_aneg(phydev);
    380c:	ebfffffe 	bl	0 <genphy_restart_aneg>
    3810:	eaffffff 	b	3814 <nss_gmac_set_priv_flags+0x60>
	if (changed & NSS_GMAC_PRIV_FLAG(TSTAMP)) {
    3814:	e3170002 	tst	r7, #2
    3818:	0a000031 	beq	38e4 <nss_gmac_set_priv_flags+0x130>
		if (flags & NSS_GMAC_PRIV_FLAG(TSTAMP)) {
    381c:	e3160002 	tst	r6, #2
    3820:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
    3824:	0a00001b 	beq	3898 <nss_gmac_set_priv_flags+0xe4>
		  if (!test_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags)) {
    3828:	e3130080 	tst	r3, #128	@ 0x80
    382c:	1a000014 	bne	3884 <nss_gmac_set_priv_flags+0xd0>
			netdev->needed_headroom += 32;
    3830:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
			nss_gmac_tstamp_sysfs_create(netdev);
    3834:	e1a00004 	mov	r0, r4
			netdev->needed_headroom += 32;
    3838:	e2833020 	add	r3, r3, #32
    383c:	e1c438bc 	strh	r3, [r4, #140]	@ 0x8c
			nss_gmac_tstamp_sysfs_create(netdev);
    3840:	ebfffffe 	bl	2624 <nss_gmac_tstamp_sysfs_create>
			if (nss_gmac_ts_enable(gmacdev)) {
    3844:	e2840d15 	add	r0, r4, #1344	@ 0x540
    3848:	ebfffffe 	bl	5b4 <nss_gmac_ts_enable>
    384c:	e3500000 	cmp	r0, #0
    3850:	0a000004 	beq	3868 <nss_gmac_set_priv_flags+0xb4>
				netdev_info(netdev, "%s: Reg write error. Cannot enable Timestamping \n", __func__);
    3854:	e59f20f4 	ldr	r2, [pc, #244]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    3858:	e1a00004 	mov	r0, r4
    385c:	e59f10f0 	ldr	r1, [pc, #240]	@ 3954 <nss_gmac_set_priv_flags+0x1a0>
    3860:	ebfffffe 	bl	0 <netdev_info>
				return -EINVAL;
    3864:	ea000035 	b	3940 <nss_gmac_set_priv_flags+0x18c>
			gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(TSTAMP);
    3868:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Enabled 'Timestamp' flag (needed_headroom: %dx)", __func__, netdev->needed_headroom);
    386c:	e59f20dc 	ldr	r2, [pc, #220]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    3870:	e59f10e0 	ldr	r1, [pc, #224]	@ 3958 <nss_gmac_set_priv_flags+0x1a4>
			gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(TSTAMP);
    3874:	e3833002 	orr	r3, r3, #2
    3878:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Enabled 'Timestamp' flag (needed_headroom: %dx)", __func__, netdev->needed_headroom);
    387c:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
    3880:	ea000015 	b	38dc <nss_gmac_set_priv_flags+0x128>
			netdev_warn(netdev, "%s: Already enabled 'Timestamp' flag", __func__);
    3884:	e59f20c4 	ldr	r2, [pc, #196]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    3888:	e1a00004 	mov	r0, r4
    388c:	e59f10c8 	ldr	r1, [pc, #200]	@ 395c <nss_gmac_set_priv_flags+0x1a8>
    3890:	ebfffffe 	bl	0 <netdev_warn>
    3894:	ea000012 	b	38e4 <nss_gmac_set_priv_flags+0x130>
		  if (!test_bit(__NSS_GMAC_TSTAMP, &gmacdev->flags)) {
    3898:	e3130080 	tst	r3, #128	@ 0x80
    389c:	1a000004 	bne	38b4 <nss_gmac_set_priv_flags+0x100>
			  netdev_warn(netdev, "%s: Timestamp is already disabled \n", __func__);
    38a0:	e59f20a8 	ldr	r2, [pc, #168]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    38a4:	e1a00004 	mov	r0, r4
    38a8:	e59f10b0 	ldr	r1, [pc, #176]	@ 3960 <nss_gmac_set_priv_flags+0x1ac>
    38ac:	ebfffffe 	bl	0 <netdev_warn>
			  return -EINVAL;
    38b0:	ea000022 	b	3940 <nss_gmac_set_priv_flags+0x18c>
		  nss_gmac_ts_disable(gmacdev);
    38b4:	e2840d15 	add	r0, r4, #1344	@ 0x540
    38b8:	ebfffffe 	bl	6dc <nss_gmac_ts_disable>
		  gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(TSTAMP);
    38bc:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
		  nss_gmac_tstamp_sysfs_remove(gmacdev->netdev);
    38c0:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
		  gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(TSTAMP);
    38c4:	e3c33002 	bic	r3, r3, #2
    38c8:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
		  nss_gmac_tstamp_sysfs_remove(gmacdev->netdev);
    38cc:	ebfffffe 	bl	2824 <nss_gmac_tstamp_sysfs_remove>
		  netdev_info(netdev, "%s: Disabled 'Timestamp' flag (needed_headroom: %dx)", __func__, netdev->needed_headroom);
    38d0:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
    38d4:	e59f2074 	ldr	r2, [pc, #116]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    38d8:	e59f1084 	ldr	r1, [pc, #132]	@ 3964 <nss_gmac_set_priv_flags+0x1b0>
    38dc:	e1a00004 	mov	r0, r4
    38e0:	ebfffffe 	bl	0 <netdev_info>
	if (changed & NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR)) {
    38e4:	e3170004 	tst	r7, #4
    38e8:	1a000001 	bne	38f4 <nss_gmac_set_priv_flags+0x140>
	return 0;
    38ec:	e3a00000 	mov	r0, #0
    38f0:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
		if (flags & NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR)) {
    38f4:	e3160004 	tst	r6, #4
			nss_gmac_rx_tcpip_chksum_drop_disable(gmacdev);
    38f8:	e2840d15 	add	r0, r4, #1344	@ 0x540
		if (flags & NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR)) {
    38fc:	0a000006 	beq	391c <nss_gmac_set_priv_flags+0x168>
			nss_gmac_rx_tcpip_chksum_drop_disable(gmacdev);
    3900:	ebfffffe 	bl	fe0 <nss_gmac_rx_tcpip_chksum_drop_disable>
			gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR);
    3904:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Enabled 'ignore Rx csum error' flag", __func__);
    3908:	e59f2040 	ldr	r2, [pc, #64]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    390c:	e59f1054 	ldr	r1, [pc, #84]	@ 3968 <nss_gmac_set_priv_flags+0x1b4>
			gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR);
    3910:	e3833004 	orr	r3, r3, #4
    3914:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Enabled 'ignore Rx csum error' flag", __func__);
    3918:	ea000005 	b	3934 <nss_gmac_set_priv_flags+0x180>
			nss_gmac_rx_tcpip_chksum_drop_enable(gmacdev);
    391c:	ebfffffe 	bl	e94 <nss_gmac_rx_tcpip_chksum_drop_enable>
			gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR);
    3920:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Disabled 'ignore Rx csum error' flag", __func__);
    3924:	e59f2024 	ldr	r2, [pc, #36]	@ 3950 <nss_gmac_set_priv_flags+0x19c>
    3928:	e59f103c 	ldr	r1, [pc, #60]	@ 396c <nss_gmac_set_priv_flags+0x1b8>
			gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(IGNORE_RX_CSUM_ERR);
    392c:	e3c33004 	bic	r3, r3, #4
    3930:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
			netdev_info(netdev, "%s: Disabled 'ignore Rx csum error' flag", __func__);
    3934:	e1a00004 	mov	r0, r4
    3938:	ebfffffe 	bl	0 <netdev_info>
    393c:	eaffffea 	b	38ec <nss_gmac_set_priv_flags+0x138>
			return -EINVAL;
    3940:	e3e00015 	mvn	r0, #21
    3944:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
			return -EOPNOTSUPP;
    3948:	e3e0005e 	mvn	r0, #94	@ 0x5e
}
    394c:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    3950:	00000fd3 	.word	0x00000fd3
    3954:	00001070 	.word	0x00001070
    3958:	000010a2 	.word	0x000010a2
    395c:	000010d6 	.word	0x000010d6
    3960:	000010fb 	.word	0x000010fb
    3964:	0000111f 	.word	0x0000111f
    3968:	00001154 	.word	0x00001154
    396c:	0000117c 	.word	0x0000117c

00003970 <nss_gmac_get_ethtool_stats>:
	if (!gmacdev->data_plane_ops)
    3970:	e5903a6c 	ldr	r3, [r0, #2668]	@ 0xa6c
    3974:	e3530000 	cmp	r3, #0
    3978:	012fff1e 	bxeq	lr
{
    397c:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    3980:	e1a04000 	mov	r4, r0
	raw_spin_lock_bh(&lock->rlock);
    3984:	e2800e7a 	add	r0, r0, #1952	@ 0x7a0
    3988:	e1a05002 	mov	r5, r2
    398c:	e2800008 	add	r0, r0, #8
	gmacdev->data_plane_ops->get_stats(gmacdev->data_plane_ctx, &gmacdev->nss_stats);
    3990:	e2847e7d 	add	r7, r4, #2000	@ 0x7d0
    3994:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    3998:	e2877008 	add	r7, r7, #8
    399c:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    39a0:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    39a4:	e1a01007 	mov	r1, r7
    39a8:	e5933020 	ldr	r3, [r3, #32]
    39ac:	e12fff33 	blx	r3
	for (i = 0; i < NSS_GMAC_STATS_LEN; i++) {
    39b0:	e59fc05c 	ldr	ip, [pc, #92]	@ 3a14 <nss_gmac_get_ethtool_stats+0xa4>
    39b4:	e2452008 	sub	r2, r5, #8
    39b8:	e2853f82 	add	r3, r5, #520	@ 0x208
    39bc:	ea000003 	b	39d0 <nss_gmac_get_ethtool_stats+0x60>
		data[i] = *(uint64_t *)p;
    39c0:	e59c1020 	ldr	r1, [ip, #32]
	for (i = 0; i < NSS_GMAC_STATS_LEN; i++) {
    39c4:	e28cc024 	add	ip, ip, #36	@ 0x24
		data[i] = *(uint64_t *)p;
    39c8:	e18100d7 	ldrd	r0, [r1, r7]
    39cc:	e1e200f8 	strd	r0, [r2, #8]!
	for (i = 0; i < NSS_GMAC_STATS_LEN; i++) {
    39d0:	e1520003 	cmp	r2, r3
    39d4:	1afffff9 	bne	39c0 <nss_gmac_get_ethtool_stats+0x50>
    39d8:	e59f2038 	ldr	r2, [pc, #56]	@ 3a18 <nss_gmac_get_ethtool_stats+0xa8>
    39dc:	e2855fa2 	add	r5, r5, #648	@ 0x288
		data[i] = *(uint64_t *)p;
    39e0:	e300c9e8 	movw	ip, #2536	@ 0x9e8
    39e4:	ea000004 	b	39fc <nss_gmac_get_ethtool_stats+0x8c>
    39e8:	e5921020 	ldr	r1, [r2, #32]
	for (i = NSS_GMAC_STATS_LEN, j = 0; i < NSS_GMAC_STATS_LEN + NSS_GMAC_HOST_STATS_LEN; i++) {
    39ec:	e2822024 	add	r2, r2, #36	@ 0x24
		data[i] = *(uint64_t *)p;
    39f0:	e0841001 	add	r1, r4, r1
    39f4:	e18100dc 	ldrd	r0, [r1, ip]
    39f8:	e1e300f8 	strd	r0, [r3, #8]!
	for (i = NSS_GMAC_STATS_LEN, j = 0; i < NSS_GMAC_STATS_LEN + NSS_GMAC_HOST_STATS_LEN; i++) {
    39fc:	e1530005 	cmp	r3, r5
    3a00:	1afffff8 	bne	39e8 <nss_gmac_get_ethtool_stats+0x78>
	raw_spin_unlock_bh(&lock->rlock);
    3a04:	e2840e7a 	add	r0, r4, #1952	@ 0x7a0
}
    3a08:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
    3a0c:	e2800008 	add	r0, r0, #8
    3a10:	eafffffe 	b	0 <_raw_spin_unlock_bh>
    3a14:	000003c8 	.word	0x000003c8
    3a18:	00000d10 	.word	0x00000d10

00003a1c <nss_gmac_get_pauseparam>:
	BUG_ON(gmacdev == NULL);
    3a1c:	e3700d15 	cmn	r0, #1344	@ 0x540
    3a20:	0a000002 	beq	3a30 <nss_gmac_get_pauseparam+0x14>
	BUG_ON(gmacdev->netdev != netdev);
    3a24:	e59035d0 	ldr	r3, [r0, #1488]	@ 0x5d0
    3a28:	e1530000 	cmp	r3, r0
    3a2c:	0a000000 	beq	3a34 <nss_gmac_get_pauseparam+0x18>
    3a30:	e7f001f2 	.word	0xe7f001f2
	pause->rx_pause = gmacdev->pause & FLOW_CTRL_RX ? 1 : 0;
    3a34:	e59325bc 	ldr	r2, [r3, #1468]	@ 0x5bc
    3a38:	e7e020d2 	ubfx	r2, r2, #1, #1
    3a3c:	e5812008 	str	r2, [r1, #8]
	pause->tx_pause = gmacdev->pause & FLOW_CTRL_TX ? 1 : 0;
    3a40:	e59335bc 	ldr	r3, [r3, #1468]	@ 0x5bc
    3a44:	e2033001 	and	r3, r3, #1
    3a48:	e581300c 	str	r3, [r1, #12]
	pause->autoneg = AUTONEG_ENABLE;
    3a4c:	e3a03001 	mov	r3, #1
    3a50:	e5813004 	str	r3, [r1, #4]
}
    3a54:	e12fff1e 	bx	lr

00003a58 <nss_gmac_get_strset_count>:
	switch (sset) {
    3a58:	e3510001 	cmp	r1, #1
    3a5c:	0a000007 	beq	3a80 <nss_gmac_get_strset_count+0x28>
    3a60:	e3510002 	cmp	r1, #2
    3a64:	0a000007 	beq	3a88 <nss_gmac_get_strset_count+0x30>
		netdev_info(netdev, "%s: Invalid string set\n", __func__);
    3a68:	e59f2020 	ldr	r2, [pc, #32]	@ 3a90 <nss_gmac_get_strset_count+0x38>
    3a6c:	e59f1020 	ldr	r1, [pc, #32]	@ 3a94 <nss_gmac_get_strset_count+0x3c>
{
    3a70:	e92d4010 	push	{r4, lr}
		netdev_info(netdev, "%s: Invalid string set\n", __func__);
    3a74:	ebfffffe 	bl	0 <netdev_info>
    3a78:	e3e0005e 	mvn	r0, #94	@ 0x5e
}
    3a7c:	e8bd8010 	pop	{r4, pc}
		return NSS_GMAC_STATS_LEN + NSS_GMAC_HOST_STATS_LEN;
    3a80:	e3a00052 	mov	r0, #82	@ 0x52
    3a84:	e12fff1e 	bx	lr
	switch (sset) {
    3a88:	e3a00003 	mov	r0, #3
}
    3a8c:	e12fff1e 	bx	lr
    3a90:	00000feb 	.word	0x00000feb
    3a94:	000011a5 	.word	0x000011a5

00003a98 <bitmap_copy.constprop.0>:
	unsigned int len = bitmap_size(nbits);

	if (small_const_nbits(nbits))
		*dst = *src;
	else
		memcpy(dst, src, len);
    3a98:	e5913000 	ldr	r3, [r1]
    3a9c:	e5803000 	str	r3, [r0]
    3aa0:	e5913004 	ldr	r3, [r1, #4]
    3aa4:	e5803004 	str	r3, [r0, #4]
    3aa8:	e5913008 	ldr	r3, [r1, #8]
    3aac:	e5803008 	str	r3, [r0, #8]
}
    3ab0:	e12fff1e 	bx	lr

00003ab4 <nss_gmac_get_settings>:
{
    3ab4:	e92d40f7 	push	{r0, r1, r2, r4, r5, r6, r7, lr}
	u32 lp_advertising = 0;
    3ab8:	e3a03000 	mov	r3, #0
	BUG_ON(gmacdev == NULL);
    3abc:	e2907d15 	adds	r7, r0, #1344	@ 0x540
	u32 lp_advertising = 0;
    3ac0:	e58d3004 	str	r3, [sp, #4]
	BUG_ON(gmacdev == NULL);
    3ac4:	1a000000 	bne	3acc <nss_gmac_get_settings+0x18>
    3ac8:	e7f001f2 	.word	0xe7f001f2
    3acc:	e1a04001 	mov	r4, r1
    3ad0:	e5901558 	ldr	r1, [r0, #1368]	@ 0x558
    3ad4:	e1a05000 	mov	r5, r0
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
    3ad8:	e7e01351 	ubfx	r1, r1, #6, #1
    3adc:	e3510000 	cmp	r1, #0
    3ae0:	1a00000a 	bne	3b10 <nss_gmac_get_settings+0x5c>
		if (gmacdev->forced_speed != SPEED_UNKNOWN) {
    3ae4:	e59035c4 	ldr	r3, [r0, #1476]	@ 0x5c4
    3ae8:	e3730001 	cmn	r3, #1
    3aec:	1a000001 	bne	3af8 <nss_gmac_get_settings+0x44>
			return -EIO;
    3af0:	e3e00004 	mvn	r0, #4
    3af4:	ea000058 	b	3c5c <nss_gmac_get_settings+0x1a8>
			elk->base.speed = gmacdev->forced_speed;
    3af8:	e5843004 	str	r3, [r4, #4]
			elk->base.duplex = gmacdev->forced_duplex;
    3afc:	e59535c8 	ldr	r3, [r5, #1480]	@ 0x5c8
			ethtool_convert_legacy_u32_to_link_mode(elk->link_modes.lp_advertising, 0);
    3b00:	e2840048 	add	r0, r4, #72	@ 0x48
			elk->base.duplex = gmacdev->forced_duplex;
    3b04:	e5c43008 	strb	r3, [r4, #8]
			elk->base.mdio_support = 0;
    3b08:	e5c4100c 	strb	r1, [r4, #12]
			ethtool_convert_legacy_u32_to_link_mode(elk->link_modes.lp_advertising, 0);
    3b0c:	ea000050 	b	3c54 <nss_gmac_get_settings+0x1a0>
	phydev = gmacdev->phydev;
    3b10:	e59067d0 	ldr	r6, [r0, #2000]	@ 0x7d0
	if (phydev->is_c45 == true) {
    3b14:	e5d631f0 	ldrb	r3, [r6, #496]	@ 0x1f0
    3b18:	e3130001 	tst	r3, #1
    3b1c:	13a03002 	movne	r3, #2
    3b20:	1a000004 	bne	3b38 <nss_gmac_get_settings+0x84>
		if (genphy_read_status(phydev) != 0) {
    3b24:	e1a00006 	mov	r0, r6
    3b28:	ebfffffe 	bl	0 <genphy_read_status>
    3b2c:	e3500000 	cmp	r0, #0
    3b30:	03a03001 	moveq	r3, #1
    3b34:	1affffed 	bne	3af0 <nss_gmac_get_settings+0x3c>
	bitmap_copy(elk->link_modes.advertising, phydev->advertising, __ETHTOOL_LINK_MODE_MASK_NBITS);
    3b38:	e2861f8a 	add	r1, r6, #552	@ 0x228
    3b3c:	e284003c 	add	r0, r4, #60	@ 0x3c
		elk->base.mdio_support = ETH_MDIO_SUPPORTS_C45;
    3b40:	e5c4300c 	strb	r3, [r4, #12]
	bitmap_copy(elk->link_modes.advertising, phydev->advertising, __ETHTOOL_LINK_MODE_MASK_NBITS);
    3b44:	ebffffd3 	bl	3a98 <bitmap_copy.constprop.0>
	elk->base.autoneg = phydev->autoneg;
    3b48:	e5d631f1 	ldrb	r3, [r6, #497]	@ 0x1f1
	bitmap_copy(elk->link_modes.supported, phydev->supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
    3b4c:	e2861f87 	add	r1, r6, #540	@ 0x21c
    3b50:	e2840030 	add	r0, r4, #48	@ 0x30
	elk->base.autoneg = phydev->autoneg;
    3b54:	e7e03253 	ubfx	r3, r3, #4, #1
    3b58:	e5c4300b 	strb	r3, [r4, #11]
	elk->base.speed = phydev->speed;
    3b5c:	e5963204 	ldr	r3, [r6, #516]	@ 0x204
    3b60:	e5843004 	str	r3, [r4, #4]
	elk->base.duplex = phydev->duplex;
    3b64:	e5963208 	ldr	r3, [r6, #520]	@ 0x208
    3b68:	e5c43008 	strb	r3, [r4, #8]
	elk->base.port = PORT_TP;
    3b6c:	e3a03000 	mov	r3, #0
    3b70:	e5c43009 	strb	r3, [r4, #9]
	elk->base.phy_address = gmacdev->phy_base;
    3b74:	e5953548 	ldr	r3, [r5, #1352]	@ 0x548
    3b78:	e5c4300a 	strb	r3, [r4, #10]
	elk->base.transceiver = XCVR_EXTERNAL;
    3b7c:	e3a03001 	mov	r3, #1
    3b80:	e5c43010 	strb	r3, [r4, #16]
	bitmap_copy(elk->link_modes.supported, phydev->supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
    3b84:	ebffffc3 	bl	3a98 <bitmap_copy.constprop.0>
	if (phydev->is_c45 == true)
    3b88:	e5d631f0 	ldrb	r3, [r6, #496]	@ 0x1f0
    3b8c:	e3130001 	tst	r3, #1
    3b90:	0a000001 	beq	3b9c <nss_gmac_get_settings+0xe8>
			return 0;
    3b94:	e3a00000 	mov	r0, #0
    3b98:	ea00002f 	b	3c5c <nss_gmac_get_settings+0x1a8>
	ethtool_convert_link_mode_to_legacy_u32(&lp_advertising, elk->link_modes.lp_advertising);
    3b9c:	e2844048 	add	r4, r4, #72	@ 0x48
    3ba0:	e28d0004 	add	r0, sp, #4
    3ba4:	e1a01004 	mov	r1, r4
    3ba8:	ebfffffe 	bl	0 <ethtool_convert_link_mode_to_legacy_u32>
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_LPA);
    3bac:	e5951548 	ldr	r1, [r5, #1352]	@ 0x548
    3bb0:	e3a02005 	mov	r2, #5
    3bb4:	e1a00007 	mov	r0, r7
    3bb8:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
	if (phyreg & LPA_10HALF)
    3bbc:	e3100020 	tst	r0, #32
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_STAT1000);
    3bc0:	e5951548 	ldr	r1, [r5, #1352]	@ 0x548
    3bc4:	e3a0200a 	mov	r2, #10
		lp_advertising |= ADVERTISED_10baseT_Half;
    3bc8:	159d3004 	ldrne	r3, [sp, #4]
    3bcc:	13833001 	orrne	r3, r3, #1
    3bd0:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_10FULL)
    3bd4:	e3100040 	tst	r0, #64	@ 0x40
		lp_advertising |= ADVERTISED_10baseT_Full;
    3bd8:	159d3004 	ldrne	r3, [sp, #4]
    3bdc:	13833002 	orrne	r3, r3, #2
    3be0:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_100HALF)
    3be4:	e3100080 	tst	r0, #128	@ 0x80
		lp_advertising |= ADVERTISED_100baseT_Half;
    3be8:	159d3004 	ldrne	r3, [sp, #4]
    3bec:	13833004 	orrne	r3, r3, #4
    3bf0:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_100FULL)
    3bf4:	e3100c01 	tst	r0, #256	@ 0x100
		lp_advertising |= ADVERTISED_100baseT_Full;
    3bf8:	159d3004 	ldrne	r3, [sp, #4]
    3bfc:	13833008 	orrne	r3, r3, #8
    3c00:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_PAUSE_CAP)
    3c04:	e3100b01 	tst	r0, #1024	@ 0x400
		lp_advertising |= ADVERTISED_Pause;
    3c08:	159d3004 	ldrne	r3, [sp, #4]
    3c0c:	13833a02 	orrne	r3, r3, #8192	@ 0x2000
    3c10:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_PAUSE_ASYM)
    3c14:	e3100b02 	tst	r0, #2048	@ 0x800
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_STAT1000);
    3c18:	e1a00007 	mov	r0, r7
		lp_advertising |= ADVERTISED_Asym_Pause;
    3c1c:	159d3004 	ldrne	r3, [sp, #4]
    3c20:	13833901 	orrne	r3, r3, #16384	@ 0x4000
    3c24:	158d3004 	strne	r3, [sp, #4]
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_STAT1000);
    3c28:	ebfffffe 	bl	178 <nss_gmac_mii_rd_reg>
	if (phyreg & LPA_1000HALF)
    3c2c:	e3100b01 	tst	r0, #1024	@ 0x400
		lp_advertising |= ADVERTISED_1000baseT_Half;
    3c30:	159d3004 	ldrne	r3, [sp, #4]
    3c34:	13833010 	orrne	r3, r3, #16
    3c38:	158d3004 	strne	r3, [sp, #4]
	if (phyreg & LPA_1000FULL)
    3c3c:	e3100b02 	tst	r0, #2048	@ 0x800
	ethtool_convert_legacy_u32_to_link_mode(elk->link_modes.lp_advertising, lp_advertising);
    3c40:	e1a00004 	mov	r0, r4
		lp_advertising |= ADVERTISED_1000baseT_Full;
    3c44:	159d3004 	ldrne	r3, [sp, #4]
    3c48:	13833020 	orrne	r3, r3, #32
    3c4c:	158d3004 	strne	r3, [sp, #4]
	ethtool_convert_legacy_u32_to_link_mode(elk->link_modes.lp_advertising, lp_advertising);
    3c50:	e59d1004 	ldr	r1, [sp, #4]
    3c54:	ebfffffe 	bl	0 <ethtool_convert_legacy_u32_to_link_mode>
	return 0;
    3c58:	eaffffcd 	b	3b94 <nss_gmac_get_settings+0xe0>
}
    3c5c:	e28dd00c 	add	sp, sp, #12
    3c60:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}

00003c64 <nss_gmac_set_pauseparam>:
{
    3c64:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(gmacdev == NULL);
    3c68:	e2905d15 	adds	r5, r0, #1344	@ 0x540
    3c6c:	0a000003 	beq	3c80 <nss_gmac_set_pauseparam+0x1c>
	BUG_ON(gmacdev->netdev != netdev);
    3c70:	e59035d0 	ldr	r3, [r0, #1488]	@ 0x5d0
    3c74:	e1a04000 	mov	r4, r0
    3c78:	e1530000 	cmp	r3, r0
    3c7c:	0a000000 	beq	3c84 <nss_gmac_set_pauseparam+0x20>
    3c80:	e7f001f2 	.word	0xe7f001f2
	gmacdev->pause = 0;
    3c84:	e3a03000 	mov	r3, #0
    3c88:	e58035bc 	str	r3, [r0, #1468]	@ 0x5bc
	if (pause->rx_pause)
    3c8c:	e5913008 	ldr	r3, [r1, #8]
    3c90:	e2533000 	subs	r3, r3, #0
    3c94:	13a03001 	movne	r3, #1
    3c98:	e1a03083 	lsl	r3, r3, #1
    3c9c:	e58035bc 	str	r3, [r0, #1468]	@ 0x5bc
	if (pause->tx_pause)
    3ca0:	e591200c 	ldr	r2, [r1, #12]
    3ca4:	e3520000 	cmp	r2, #0
		gmacdev->pause |= FLOW_CTRL_TX;
    3ca8:	13833001 	orrne	r3, r3, #1
    3cac:	158035bc 	strne	r3, [r0, #1468]	@ 0x5bc
    3cb0:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
    3cb4:	e3130040 	tst	r3, #64	@ 0x40
    3cb8:	1a000011 	bne	3d04 <nss_gmac_set_pauseparam+0xa0>
		if (gmacdev->forced_speed != SPEED_UNKNOWN) {
    3cbc:	e59035c4 	ldr	r3, [r0, #1476]	@ 0x5c4
    3cc0:	e3730001 	cmn	r3, #1
    3cc4:	0a00001e 	beq	3d44 <nss_gmac_set_pauseparam+0xe0>
			if (gmacdev->pause & FLOW_CTRL_TX)
    3cc8:	e59435bc 	ldr	r3, [r4, #1468]	@ 0x5bc
				nss_gmac_tx_pause_enable(gmacdev);
    3ccc:	e1a00005 	mov	r0, r5
			if (gmacdev->pause & FLOW_CTRL_TX)
    3cd0:	e3130001 	tst	r3, #1
    3cd4:	0a000001 	beq	3ce0 <nss_gmac_set_pauseparam+0x7c>
				nss_gmac_tx_pause_enable(gmacdev);
    3cd8:	ebfffffe 	bl	88 <phy_write.isra.0+0x4>
    3cdc:	ea000000 	b	3ce4 <nss_gmac_set_pauseparam+0x80>
				nss_gmac_tx_pause_disable(gmacdev);
    3ce0:	ebfffffe 	bl	104 <nss_gmac_read_phy_reg+0x50>
			if (gmacdev->pause & FLOW_CTRL_RX)
    3ce4:	e59435bc 	ldr	r3, [r4, #1468]	@ 0x5bc
				nss_gmac_rx_pause_enable(gmacdev);
    3ce8:	e1a00005 	mov	r0, r5
			if (gmacdev->pause & FLOW_CTRL_RX)
    3cec:	e3130002 	tst	r3, #2
    3cf0:	0a000001 	beq	3cfc <nss_gmac_set_pauseparam+0x98>
				nss_gmac_rx_pause_enable(gmacdev);
    3cf4:	ebfffffe 	bl	180 <nss_gmac_mii_rd_reg+0x8>
    3cf8:	ea000011 	b	3d44 <nss_gmac_set_pauseparam+0xe0>
				nss_gmac_rx_pause_disable(gmacdev);
    3cfc:	ebfffffe 	bl	1c4 <nss_gmac_mii_wr_reg+0x10>
    3d00:	ea00000f 	b	3d44 <nss_gmac_set_pauseparam+0xe0>
	phydev = gmacdev->phydev;
    3d04:	e59037d0 	ldr	r3, [r0, #2000]	@ 0x7d0
	*advertising &= ~(ADVERTISED_Pause | ADVERTISED_Asym_Pause);
    3d08:	e5932228 	ldr	r2, [r3, #552]	@ 0x228
    3d0c:	e3c21a06 	bic	r1, r2, #24576	@ 0x6000
    3d10:	e5831228 	str	r1, [r3, #552]	@ 0x228
	if (gmacdev->pause & FLOW_CTRL_RX)
    3d14:	e59015bc 	ldr	r1, [r0, #1468]	@ 0x5bc
    3d18:	e3110002 	tst	r1, #2
int genphy_soft_reset(struct phy_device *phydev);
irqreturn_t genphy_handle_interrupt_no_ack(struct phy_device *phydev);

static inline int genphy_config_aneg(struct phy_device *phydev)
{
	return __genphy_config_aneg(phydev, false);
    3d1c:	e3a01000 	mov	r1, #0
		*advertising |=
    3d20:	13822a06 	orrne	r2, r2, #24576	@ 0x6000
    3d24:	15832228 	strne	r2, [r3, #552]	@ 0x228
	if (gmacdev->pause & FLOW_CTRL_TX)
    3d28:	e59025bc 	ldr	r2, [r0, #1468]	@ 0x5bc
    3d2c:	e3120001 	tst	r2, #1
		*advertising |= ADVERTISED_Asym_Pause;
    3d30:	15932228 	ldrne	r2, [r3, #552]	@ 0x228
    3d34:	13822901 	orrne	r2, r2, #16384	@ 0x4000
    3d38:	15832228 	strne	r2, [r3, #552]	@ 0x228
    3d3c:	e59007d0 	ldr	r0, [r0, #2000]	@ 0x7d0
    3d40:	ebfffffe 	bl	0 <__genphy_config_aneg>
}
    3d44:	e3a00000 	mov	r0, #0
    3d48:	e8bd8070 	pop	{r4, r5, r6, pc}

00003d4c <nss_gmac_ethtool_register>:
 * @brief Register ethtool_ops
 * @param[in] pointer to struct net_device
 */
void nss_gmac_ethtool_register(struct net_device *netdev)
{
	netdev->ethtool_ops = &nss_gmac_ethtool_ops;
    3d4c:	e59f3004 	ldr	r3, [pc, #4]	@ 3d58 <nss_gmac_ethtool_register+0xc>
    3d50:	e5803148 	str	r3, [r0, #328]	@ 0x148
}
    3d54:	e12fff1e 	bx	lr
    3d58:	0000034c 	.word	0x0000034c

00003d5c <__raw_spin_unlock>:
	}
}

static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
	smp_mb();
    3d5c:	f57ff05b 	dmb	ish
	lock->tickets.owner++;
    3d60:	e1d030b0 	ldrh	r3, [r0]
    3d64:	e2833001 	add	r3, r3, #1
    3d68:	e1c030b0 	strh	r3, [r0]
	dsb(ishst);
    3d6c:	f57ff04a 	dsb	ishst
	__asm__(SEV);
    3d70:	e320f004 	sev
static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
	spin_release(&lock->dep_map, _RET_IP_);
	do_raw_spin_unlock(lock);
	preempt_enable();
}
    3d74:	e12fff1e 	bx	lr

00003d78 <nss_gmac_slowpath_if_mac_addr>:
 * nss_gmac_slowpath_if_mac_addr()
 */
static int nss_gmac_slowpath_if_mac_addr(void *app_data, uint8_t *addr)
{
	return NSS_GMAC_SUCCESS;
}
    3d78:	e3a00000 	mov	r0, #0
    3d7c:	e12fff1e 	bx	lr

00003d80 <nss_gmac_slowpath_if_change_mtu>:
 * nss_gmac_slowpath_if_change_mtu()
 */
static int nss_gmac_slowpath_if_change_mtu(void *app_data, uint32_t mtu)
{
	return NSS_GMAC_SUCCESS;
}
    3d80:	e3a00000 	mov	r0, #0
    3d84:	e12fff1e 	bx	lr

00003d88 <nss_gmac_slowpath_if_set_features>:
 * nss_gmac_slowpath_if_set_features()
 *	Set the supported net_device features
 */
static void nss_gmac_slowpath_if_set_features(struct net_device *netdev)
{
	netdev->features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM;
    3d88:	e1c029d0 	ldrd	r2, [r0, #144]	@ 0x90
    3d8c:	e3822008 	orr	r2, r2, #8
    3d90:	e3833c01 	orr	r3, r3, #256	@ 0x100
    3d94:	e1c029f0 	strd	r2, [r0, #144]	@ 0x90
	netdev->hw_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM;
    3d98:	e1c029d8 	ldrd	r2, [r0, #152]	@ 0x98
    3d9c:	e3822008 	orr	r2, r2, #8
    3da0:	e3833c01 	orr	r3, r3, #256	@ 0x100
    3da4:	e1c029f8 	strd	r2, [r0, #152]	@ 0x98
	netdev->vlan_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM;
    3da8:	e1c02ad8 	ldrd	r2, [r0, #168]	@ 0xa8
    3dac:	e3822008 	orr	r2, r2, #8
    3db0:	e3833c01 	orr	r3, r3, #256	@ 0x100
    3db4:	e1c02af8 	strd	r2, [r0, #168]	@ 0xa8
	netdev->wanted_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM;
    3db8:	e1c02ad0 	ldrd	r2, [r0, #160]	@ 0xa0
    3dbc:	e3822008 	orr	r2, r2, #8
    3dc0:	e3833c01 	orr	r3, r3, #256	@ 0x100
    3dc4:	e1c02af0 	strd	r2, [r0, #160]	@ 0xa0
}
    3dc8:	e12fff1e 	bx	lr

00003dcc <nss_gmac_slowpath_if_get_stats>:
/*
 * nss_gmac_slowpath_if_get_stats()
 */
static void nss_gmac_slowpath_if_get_stats(void *app_data, struct nss_gmac_stats *stats)
{
}
    3dcc:	e12fff1e 	bx	lr

00003dd0 <nss_gmac_get_netdev_by_macid>:
 * nss_gmac_get_netdev_by_macid()
 *	return the net device of the corrsponding macid if exist
 */
struct net_device *nss_gmac_get_netdev_by_macid(int macid)
{
	struct nss_gmac_dev *gmacdev = ctx.nss_gmac[macid];
    3dd0:	e59f3010 	ldr	r3, [pc, #16]	@ 3de8 <nss_gmac_get_netdev_by_macid+0x18>
    3dd4:	e0833100 	add	r3, r3, r0, lsl #2
    3dd8:	e593001c 	ldr	r0, [r3, #28]

	if (!gmacdev)
    3ddc:	e3500000 	cmp	r0, #0
		return NULL;
	return gmacdev->netdev;
    3de0:	15900090 	ldrne	r0, [r0, #144]	@ 0x90
}
    3de4:	e12fff1e 	bx	lr
    3de8:	00000000 	.word	0x00000000

00003dec <skb_header_pointer>:
	return buffer;
}

static inline void * __must_check
skb_header_pointer(const struct sk_buff *skb, int offset, int len, void *buffer)
{
    3dec:	e92d4010 	push	{r4, lr}
    3df0:	e1a04003 	mov	r4, r3
	return skb->len - skb->data_len;
    3df4:	e590c064 	ldr	ip, [r0, #100]	@ 0x64
    3df8:	e5903068 	ldr	r3, [r0, #104]	@ 0x68
    3dfc:	e04cc003 	sub	ip, ip, r3
	if (likely(hlen - offset >= len))
    3e00:	e04cc001 	sub	ip, ip, r1
    3e04:	e152000c 	cmp	r2, ip
		return (void *)data + offset;
    3e08:	d59040bc 	ldrle	r4, [r0, #188]	@ 0xbc
    3e0c:	d0844001 	addle	r4, r4, r1
	if (likely(hlen - offset >= len))
    3e10:	da000007 	ble	3e34 <skb_header_pointer+0x48>
	if (!skb || unlikely(skb_copy_bits(skb, offset, buffer, len) < 0))
    3e14:	e3500000 	cmp	r0, #0
    3e18:	0a000004 	beq	3e30 <skb_header_pointer+0x44>
    3e1c:	e1a03002 	mov	r3, r2
    3e20:	e1a02004 	mov	r2, r4
    3e24:	ebfffffe 	bl	0 <skb_copy_bits>
    3e28:	e3500000 	cmp	r0, #0
    3e2c:	aa000000 	bge	3e34 <skb_header_pointer+0x48>
		return NULL;
    3e30:	e3a04000 	mov	r4, #0
	return __skb_header_pointer(skb, offset, len, skb->data,
				    skb_headlen(skb), buffer);
}
    3e34:	e1a00004 	mov	r0, r4
    3e38:	e8bd8010 	pop	{r4, pc}

00003e3c <nss_gmac_slowpath_if_close>:
	if (gmacdev->drv_flags & NSS_GMAC_PRIV_FLAG(IRQ_REQUESTED)) {
    3e3c:	e590355c 	ldr	r3, [r0, #1372]	@ 0x55c
    3e40:	e3130008 	tst	r3, #8
    3e44:	0a00000d 	beq	3e80 <nss_gmac_slowpath_if_close+0x44>
{
    3e48:	e92d4010 	push	{r4, lr}
    3e4c:	e1a04000 	mov	r4, r0
		netdev_info(netdev, "Freeing IRQ %d for Mac %d\n", netdev->irq, gmacdev->macid);
    3e50:	e5902190 	ldr	r2, [r0, #400]	@ 0x190
    3e54:	e590354c 	ldr	r3, [r0, #1356]	@ 0x54c
    3e58:	e59f1028 	ldr	r1, [pc, #40]	@ 3e88 <nss_gmac_slowpath_if_close+0x4c>
    3e5c:	ebfffffe 	bl	0 <netdev_info>
		free_irq(netdev->irq, gmacdev);
    3e60:	e5940190 	ldr	r0, [r4, #400]	@ 0x190
    3e64:	e2841d15 	add	r1, r4, #1344	@ 0x540
    3e68:	ebfffffe 	bl	0 <free_irq>
		gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(IRQ_REQUESTED);
    3e6c:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
}
    3e70:	e3a00000 	mov	r0, #0
		gmacdev->drv_flags &= ~NSS_GMAC_PRIV_FLAG(IRQ_REQUESTED);
    3e74:	e3c33008 	bic	r3, r3, #8
    3e78:	e584355c 	str	r3, [r4, #1372]	@ 0x55c
}
    3e7c:	e8bd8010 	pop	{r4, pc}
    3e80:	e3a00000 	mov	r0, #0
    3e84:	e12fff1e 	bx	lr
    3e88:	000011bd 	.word	0x000011bd

00003e8c <nss_gmac_is_in_open_state>:
    3e8c:	e5900558 	ldr	r0, [r0, #1368]	@ 0x558
}
    3e90:	e2000001 	and	r0, r0, #1
    3e94:	e12fff1e 	bx	lr

00003e98 <nss_gmac_slowpath_if_open>:
{
    3e98:	e92d4013 	push	{r0, r1, r4, lr}
	if (gmacdev->drv_flags & NSS_GMAC_PRIV_FLAG(IRQ_REQUESTED))
    3e9c:	e590355c 	ldr	r3, [r0, #1372]	@ 0x55c
    3ea0:	e2133008 	ands	r3, r3, #8
    3ea4:	1a000014 	bne	3efc <nss_gmac_slowpath_if_open+0x64>
	return (char *)dev + ALIGN(sizeof(struct net_device), NETDEV_ALIGN);
    3ea8:	e2802d15 	add	r2, r0, #1344	@ 0x540
    3eac:	e1a04000 	mov	r4, r0
 */
static inline int __must_check
request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)
{
	return request_threaded_irq(irq, handler, NULL, flags, name, dev);
    3eb0:	e59f1050 	ldr	r1, [pc, #80]	@ 3f08 <nss_gmac_slowpath_if_open+0x70>
    3eb4:	e58d2004 	str	r2, [sp, #4]
    3eb8:	e59f204c 	ldr	r2, [pc, #76]	@ 3f0c <nss_gmac_slowpath_if_open+0x74>
    3ebc:	e58d2000 	str	r2, [sp]
    3ec0:	e5900190 	ldr	r0, [r0, #400]	@ 0x190
    3ec4:	e1a02003 	mov	r2, r3
    3ec8:	ebfffffe 	bl	0 <request_threaded_irq>
	if (err) {
    3ecc:	e3500000 	cmp	r0, #0
	gmacdev->drv_flags |= NSS_GMAC_PRIV_FLAG(IRQ_REQUESTED);
    3ed0:	0594355c 	ldreq	r3, [r4, #1372]	@ 0x55c
    3ed4:	03833008 	orreq	r3, r3, #8
    3ed8:	0584355c 	streq	r3, [r4, #1372]	@ 0x55c
	if (err) {
    3edc:	0a000006 	beq	3efc <nss_gmac_slowpath_if_open+0x64>
		netdev_info(netdev, "Mac %d IRQ %d request failed\n", gmacdev->macid, netdev->irq);
    3ee0:	e5943190 	ldr	r3, [r4, #400]	@ 0x190
    3ee4:	e1a00004 	mov	r0, r4
    3ee8:	e594254c 	ldr	r2, [r4, #1356]	@ 0x54c
    3eec:	e59f101c 	ldr	r1, [pc, #28]	@ 3f10 <nss_gmac_slowpath_if_open+0x78>
    3ef0:	ebfffffe 	bl	0 <netdev_info>
    3ef4:	e3a00001 	mov	r0, #1
    3ef8:	ea000000 	b	3f00 <nss_gmac_slowpath_if_open+0x68>
		return NSS_GMAC_SUCCESS;
    3efc:	e3a00000 	mov	r0, #0
}
    3f00:	e28dd008 	add	sp, sp, #8
    3f04:	e8bd8010 	pop	{r4, pc}
    3f08:	00000000 	.word	0x00000000
    3f0c:	000011ed 	.word	0x000011ed
    3f10:	000011f6 	.word	0x000011f6

00003f14 <dma_map_single_attrs.constprop.0>:
static inline dma_addr_t dma_map_single_attrs(struct device *dev, void *ptr,
    3f14:	e92d40f7 	push	{r0, r1, r2, r4, r5, r6, r7, lr}
    3f18:	e1a05000 	mov	r5, r0
	if (dev_WARN_ONCE(dev, is_vmalloc_addr(ptr),
    3f1c:	e1a00001 	mov	r0, r1
static inline dma_addr_t dma_map_single_attrs(struct device *dev, void *ptr,
    3f20:	e1a04001 	mov	r4, r1
    3f24:	e1a06002 	mov	r6, r2
    3f28:	e1a07003 	mov	r7, r3
	if (dev_WARN_ONCE(dev, is_vmalloc_addr(ptr),
    3f2c:	ebfffffe 	bl	0 <is_vmalloc_addr>
    3f30:	e3500000 	cmp	r0, #0
		return DMA_MAPPING_ERROR;
    3f34:	13e00000 	mvnne	r0, #0
	if (dev_WARN_ONCE(dev, is_vmalloc_addr(ptr),
    3f38:	1a00000b 	bne	3f6c <dma_map_single_attrs.constprop.0+0x58>
	return dma_map_page_attrs(dev, virt_to_page(ptr), offset_in_page(ptr),
    3f3c:	e59f3030 	ldr	r3, [pc, #48]	@ 3f74 <dma_map_single_attrs.constprop.0+0x60>
    3f40:	e2841101 	add	r1, r4, #1073741824	@ 0x40000000
    3f44:	e7eb2054 	ubfx	r2, r4, #0, #12
    3f48:	e1a01621 	lsr	r1, r1, #12
    3f4c:	e593c000 	ldr	ip, [r3]
    3f50:	e1a03006 	mov	r3, r6
    3f54:	e58d7000 	str	r7, [sp]
    3f58:	e58d0004 	str	r0, [sp, #4]
    3f5c:	e1a00005 	mov	r0, r5
    3f60:	e08c1281 	add	r1, ip, r1, lsl #5
    3f64:	ebfffffe 	bl	0 <dma_map_page_attrs>
    3f68:	eaffffff 	b	3f6c <dma_map_single_attrs.constprop.0+0x58>
}
    3f6c:	e28dd00c 	add	sp, sp, #12
    3f70:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
    3f74:	00000000 	.word	0x00000000

00003f78 <nss_gmac_start_data_plane>:
    3f78:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (test_bit(__NSS_GMAC_UP, &gmacdev->flags)) {
    3f7c:	e3130001 	tst	r3, #1
    3f80:	0a000001 	beq	3f8c <nss_gmac_start_data_plane+0x14>
		netdev_info(netdev, "This netdev already up, something is wrong\n");
    3f84:	e59f103c 	ldr	r1, [pc, #60]	@ 3fc8 <nss_gmac_start_data_plane+0x50>
    3f88:	eafffffe 	b	0 <netdev_info>
{
    3f8c:	e92d4070 	push	{r4, r5, r6, lr}
    3f90:	e1a04000 	mov	r4, r0
	if (gmacdev->data_plane_ctx == ctx) {
    3f94:	e59037cc 	ldr	r3, [r0, #1996]	@ 0x7cc
    3f98:	e1530001 	cmp	r3, r1
    3f9c:	18bd8070 	popne	{r4, r5, r6, pc}
	struct nss_gmac_global_ctx *global_ctx = gmacdev->ctx;
    3fa0:	e59057c4 	ldr	r5, [r0, #1988]	@ 0x7c4
		netdev_info(netdev, "Data plane cookie matches, let's start the netdev again\n");
    3fa4:	e59f1020 	ldr	r1, [pc, #32]	@ 3fcc <nss_gmac_start_data_plane+0x54>
    3fa8:	ebfffffe 	bl	0 <netdev_info>
 */
static inline bool queue_delayed_work(struct workqueue_struct *wq,
				      struct delayed_work *dwork,
				      unsigned long delay)
{
	return queue_delayed_work_on(WORK_CPU_UNBOUND, wq, dwork, delay);
    3fac:	e2842e5d 	add	r2, r4, #1488	@ 0x5d0
    3fb0:	e3a03064 	mov	r3, #100	@ 0x64
    3fb4:	e5951000 	ldr	r1, [r5]
    3fb8:	e2822008 	add	r2, r2, #8
    3fbc:	e3a00002 	mov	r0, #2
}
    3fc0:	e8bd4070 	pop	{r4, r5, r6, lr}
    3fc4:	eafffffe 	b	0 <queue_delayed_work_on>
    3fc8:	00001214 	.word	0x00001214
    3fcc:	00001240 	.word	0x00001240

00003fd0 <nss_gmac_slowpath_if_link_state>:
	if (link_state) {
    3fd0:	e3510000 	cmp	r1, #0
{
    3fd4:	e92d4070 	push	{r4, r5, r6, lr}
    3fd8:	e1a04000 	mov	r4, r0
	if (link_state) {
    3fdc:	0a00000a 	beq	400c <nss_gmac_slowpath_if_link_state+0x3c>
		napi_enable(&gmacdev->napi);
    3fe0:	e2800c06 	add	r0, r0, #1536	@ 0x600
    3fe4:	e2800008 	add	r0, r0, #8
    3fe8:	ebfffffe 	bl	0 <napi_enable>
		nss_gmac_enable_dma_rx(gmacdev);
    3fec:	e2840d15 	add	r0, r4, #1344	@ 0x540
    3ff0:	ebfffffe 	bl	d6c <nss_gmac_enable_dma_rx>
		nss_gmac_enable_dma_tx(gmacdev);
    3ff4:	e2840d15 	add	r0, r4, #1344	@ 0x540
    3ff8:	ebfffffe 	bl	d80 <nss_gmac_enable_dma_tx>
	addr = (uint32_t)regbase + regoffset;
    3ffc:	e5943544 	ldr	r3, [r4, #1348]	@ 0x544
    4000:	e59f2038 	ldr	r2, [pc, #56]	@ 4040 <nss_gmac_slowpath_if_link_state+0x70>
    4004:	e583201c 	str	r2, [r3, #28]
 */
static inline void nss_gmac_enable_interrupt(struct nss_gmac_dev *gmacdev,
					     uint32_t interrupts)
{
	nss_gmac_write_reg(gmacdev->dma_base, dma_interrupt, interrupts);
}
    4008:	ea00000a 	b	4038 <nss_gmac_slowpath_if_link_state+0x68>
	} else if (gmacdev->link_state == LINKUP) {
    400c:	e59035a4 	ldr	r3, [r0, #1444]	@ 0x5a4
    4010:	e3530001 	cmp	r3, #1
    4014:	1a000007 	bne	4038 <nss_gmac_slowpath_if_link_state+0x68>
static inline void nss_gmac_disable_interrupt(struct nss_gmac_dev *gmacdev,
					      uint32_t interrupts)
{
	uint32_t data = 0;

	data = ~interrupts & readl_relaxed(gmacdev->dma_base + dma_interrupt);
    4018:	e5902544 	ldr	r2, [r0, #1348]	@ 0x544
	asm volatile("ldr %0, %1"
    401c:	e592301c 	ldr	r3, [r2, #28]
    4020:	e3c33801 	bic	r3, r3, #65536	@ 0x10000
    4024:	e3c33041 	bic	r3, r3, #65	@ 0x41
	asm volatile("str %1, %0"
    4028:	e582301c 	str	r3, [r2, #28]
		napi_disable(&gmacdev->napi);
    402c:	e2800c06 	add	r0, r0, #1536	@ 0x600
    4030:	e2800008 	add	r0, r0, #8
    4034:	ebfffffe 	bl	0 <napi_disable>
}
    4038:	e3a00000 	mov	r0, #0
    403c:	e8bd8070 	pop	{r4, r5, r6, pc}
    4040:	00010041 	.word	0x00010041

00004044 <nss_gmac_rx_refill>:
{
    4044:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    4048:	e1a04000 	mov	r4, r0
	int count = NSS_GMAC_RX_DESC_SIZE - gmacdev->busy_rx_desc;
    404c:	e5906034 	ldr	r6, [r0, #52]	@ 0x34
	for (i = 0; i < count; i++) {
    4050:	e3a07000 	mov	r7, #0
	int count = NSS_GMAC_RX_DESC_SIZE - gmacdev->busy_rx_desc;
    4054:	e2666080 	rsb	r6, r6, #128	@ 0x80
	for (i = 0; i < count; i++) {
    4058:	ea00003c 	b	4150 <nss_gmac_rx_refill+0x10c>
		skb = __netdev_alloc_skb(gmacdev->netdev,
    405c:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    4060:	e3a02ea2 	mov	r2, #2592	@ 0xa20
    4064:	e30017ba 	movw	r1, #1978	@ 0x7ba
    4068:	ebfffffe 	bl	0 <__netdev_alloc_skb>
		if (unlikely(skb == NULL)) {
    406c:	e2505000 	subs	r5, r0, #0
    4070:	1a000003 	bne	4084 <nss_gmac_rx_refill+0x40>
			netdev_info(gmacdev->netdev, "Unable to allocate skb, will try next time\n");
    4074:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    4078:	e59f10dc 	ldr	r1, [pc, #220]	@ 415c <nss_gmac_rx_refill+0x118>
}
    407c:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
			netdev_info(gmacdev->netdev, "Unable to allocate skb, will try next time\n");
    4080:	eafffffe 	b	0 <netdev_info>
	skb->tail += len;
    4084:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
		dma_addr = dma_map_single(&gmacdev->netdev->dev, skb->data,
    4088:	e30027ba 	movw	r2, #1978	@ 0x7ba
	skb->data += len;
    408c:	e59510bc 	ldr	r1, [r5, #188]	@ 0xbc
	skb->tail += len;
    4090:	e2833002 	add	r3, r3, #2
	skb->data += len;
    4094:	e2811002 	add	r1, r1, #2
	skb->tail += len;
    4098:	e58530b0 	str	r3, [r5, #176]	@ 0xb0
    409c:	e3a03002 	mov	r3, #2
	skb->data += len;
    40a0:	e58510bc 	str	r1, [r5, #188]	@ 0xbc
    40a4:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    40a8:	e2800d0d 	add	r0, r0, #832	@ 0x340
    40ac:	ebffff98 	bl	3f14 <dma_map_single_attrs.constprop.0>
	BUG_ON(gmacdev->busy_rx_desc >= gmacdev->rx_desc_count);
    40b0:	e1c423d4 	ldrd	r2, [r4, #52]	@ 0x34
    40b4:	e1520003 	cmp	r2, r3
    40b8:	2a00000c 	bcs	40f0 <nss_gmac_rx_refill+0xac>
	BUG_ON(rxdesc != (gmacdev->rx_desc + rxnext));
    40bc:	e594202c 	ldr	r2, [r4, #44]	@ 0x2c
	uint32_t rxnext = gmacdev->rx_next;
    40c0:	e594c04c 	ldr	ip, [r4, #76]	@ 0x4c
	struct dma_desc *rxdesc = gmacdev->rx_next_desc;
    40c4:	e594305c 	ldr	r3, [r4, #92]	@ 0x5c
	BUG_ON(rxdesc != (gmacdev->rx_desc + rxnext));
    40c8:	e082228c 	add	r2, r2, ip, lsl #5
    40cc:	e1530002 	cmp	r3, r2
    40d0:	1a000006 	bne	40f0 <nss_gmac_rx_refill+0xac>
	return (desc->length & desc_size1_mask) == 0;
    40d4:	e5932004 	ldr	r2, [r3, #4]
    40d8:	e7ec1052 	ubfx	r1, r2, #0, #13
	BUG_ON(!nss_gmac_is_desc_empty(rxdesc));
    40dc:	e3510000 	cmp	r1, #0
    40e0:	1a000002 	bne	40f0 <nss_gmac_rx_refill+0xac>
	BUG_ON(nss_gmac_is_desc_owned_by_dma(rxdesc));
    40e4:	e593e000 	ldr	lr, [r3]
    40e8:	e35e0000 	cmp	lr, #0
    40ec:	aa000000 	bge	40f4 <nss_gmac_rx_refill+0xb0>
    40f0:	e7f001f2 	.word	0xe7f001f2
		rxdesc->length |= ((Length1 << desc_size1_shift) & desc_size1_mask);
    40f4:	e3822e7b 	orr	r2, r2, #1968	@ 0x7b0
		rxdesc->data1 = 0;
    40f8:	e1c300f8 	strd	r0, [r3, #8]
		rxdesc->length |= ((Length1 << desc_size1_shift) & desc_size1_mask);
    40fc:	e382200a 	orr	r2, r2, #10
    4100:	e5832004 	str	r2, [r3, #4]
	rxdesc->extstatus = 0;
    4104:	e5831010 	str	r1, [r3, #16]
	rxdesc->reserved1 = Data1;
    4108:	e5835014 	str	r5, [r3, #20]
	rxdesc->timestamplow = 0;
    410c:	e5831018 	str	r1, [r3, #24]
	rxdesc->timestamphigh = 0;
    4110:	e583101c 	str	r1, [r3, #28]
	wmb();
    4114:	f57ff04e 	dsb	st
	rxdesc->status = desc_own_by_dma;
    4118:	e3a02102 	mov	r2, #-2147483648	@ 0x80000000
	gmacdev->rx_next = (rxnext + 1) & (gmacdev->rx_desc_count - 1);
    411c:	e28cc001 	add	ip, ip, #1
	for (i = 0; i < count; i++) {
    4120:	e2877001 	add	r7, r7, #1
	rxdesc->status = desc_own_by_dma;
    4124:	e5832000 	str	r2, [r3]
	gmacdev->rx_next_desc = gmacdev->rx_desc + gmacdev->rx_next;
    4128:	e594202c 	ldr	r2, [r4, #44]	@ 0x2c
	gmacdev->rx_next = (rxnext + 1) & (gmacdev->rx_desc_count - 1);
    412c:	e5943038 	ldr	r3, [r4, #56]	@ 0x38
    4130:	e2433001 	sub	r3, r3, #1
    4134:	e003300c 	and	r3, r3, ip
    4138:	e584304c 	str	r3, [r4, #76]	@ 0x4c
	gmacdev->rx_next_desc = gmacdev->rx_desc + gmacdev->rx_next;
    413c:	e0823283 	add	r3, r2, r3, lsl #5
    4140:	e584305c 	str	r3, [r4, #92]	@ 0x5c
	(gmacdev->busy_rx_desc)++;
    4144:	e5943034 	ldr	r3, [r4, #52]	@ 0x34
    4148:	e2833001 	add	r3, r3, #1
    414c:	e5843034 	str	r3, [r4, #52]	@ 0x34
    4150:	e1570006 	cmp	r7, r6
    4154:	a8bd81f0 	popge	{r4, r5, r6, r7, r8, pc}
    4158:	eaffffbf 	b	405c <nss_gmac_rx_refill+0x18>
    415c:	00001279 	.word	0x00001279

00004160 <nss_gmac_slowpath_if_xmit>:
{
    4160:	e92d4ff1 	push	{r0, r4, r5, r6, r7, r8, r9, sl, fp, lr}
	int nfrags = skb_shinfo(skb)->nr_frags;
    4164:	e59130b4 	ldr	r3, [r1, #180]	@ 0xb4
    4168:	e5d38002 	ldrb	r8, [r3, #2]
	if ((NSS_GMAC_TX_DESC_SIZE - gmacdev->busy_tx_desc) < nfrags + 1)
    416c:	e5903570 	ldr	r3, [r0, #1392]	@ 0x570
    4170:	e2882001 	add	r2, r8, #1
    4174:	e2633080 	rsb	r3, r3, #128	@ 0x80
    4178:	e1530002 	cmp	r3, r2
    417c:	3a00004e 	bcc	42bc <nss_gmac_slowpath_if_xmit+0x15c>
	if (likely(nfrags == 0)) {
    4180:	e3580000 	cmp	r8, #0
    4184:	1a00004e 	bne	42c4 <nss_gmac_slowpath_if_xmit+0x164>
	return skb->len - skb->data_len;
    4188:	e5916064 	ldr	r6, [r1, #100]	@ 0x64
    418c:	e1a04000 	mov	r4, r0
    4190:	e1a05001 	mov	r5, r1
    4194:	e5913068 	ldr	r3, [r1, #104]	@ 0x68
		dma_addr = dma_map_single(&netdev->dev, skb->data, len,
    4198:	e2800d0d 	add	r0, r0, #832	@ 0x340
    419c:	e59110bc 	ldr	r1, [r1, #188]	@ 0xbc
    41a0:	e0466003 	sub	r6, r6, r3
    41a4:	e3a03001 	mov	r3, #1
    41a8:	e1a02006 	mov	r2, r6
    41ac:	ebffff58 	bl	3f14 <dma_map_single_attrs.constprop.0>
    41b0:	e1a07000 	mov	r7, r0
	raw_spin_lock_bh(&lock->rlock);
    41b4:	e2840e7a 	add	r0, r4, #1952	@ 0x7a0
    41b8:	e280000c 	add	r0, r0, #12
    41bc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	BUG_ON(gmacdev->busy_tx_desc > gmacdev->tx_desc_count);
    41c0:	e5942570 	ldr	r2, [r4, #1392]	@ 0x570
    41c4:	e594357c 	ldr	r3, [r4, #1404]	@ 0x57c
    41c8:	e1520003 	cmp	r2, r3
    41cc:	8a00000c 	bhi	4204 <nss_gmac_slowpath_if_xmit+0xa4>
	BUG_ON(txdesc != (gmacdev->tx_desc + txnext));
    41d0:	e5942568 	ldr	r2, [r4, #1384]	@ 0x568
	uint32_t txnext = gmacdev->tx_next;
    41d4:	e5940584 	ldr	r0, [r4, #1412]	@ 0x584
	struct dma_desc *txdesc = gmacdev->tx_next_desc;
    41d8:	e5943594 	ldr	r3, [r4, #1428]	@ 0x594
	BUG_ON(txdesc != (gmacdev->tx_desc + txnext));
    41dc:	e0822280 	add	r2, r2, r0, lsl #5
    41e0:	e1530002 	cmp	r3, r2
    41e4:	1a000006 	bne	4204 <nss_gmac_slowpath_if_xmit+0xa4>
	return (desc->length & desc_size1_mask) == 0;
    41e8:	e5931004 	ldr	r1, [r3, #4]
    41ec:	e7ecc051 	ubfx	ip, r1, #0, #13
	BUG_ON(!nss_gmac_is_desc_empty(txdesc));
    41f0:	e35c0000 	cmp	ip, #0
    41f4:	1a000002 	bne	4204 <nss_gmac_slowpath_if_xmit+0xa4>
	return (desc->status & desc_own_by_dma) == desc_own_by_dma;
    41f8:	e5932000 	ldr	r2, [r3]
	BUG_ON(nss_gmac_is_desc_owned_by_dma(txdesc));
    41fc:	e3520000 	cmp	r2, #0
    4200:	aa000000 	bge	4208 <nss_gmac_slowpath_if_xmit+0xa8>
    4204:	e7f001f2 	.word	0xe7f001f2
	if (Length1 > NSS_GMAC_MAX_DESC_BUFF) {
    4208:	e3560a02 	cmp	r6, #8192	@ 0x2000
		nss_gmac_set_tx_qptr(gmacdev, dma_addr, len, (uint32_t)skb,
    420c:	e5d5e074 	ldrb	lr, [r5, #116]	@ 0x74
	txdesc->reserved1 = Data1;
    4210:	e5835014 	str	r5, [r3, #20]
		      NSS_GMAC_MAX_DESC_BUFF) << desc_size2_shift) & desc_size2_mask;
    4214:	259fa0b8 	ldrcs	sl, [pc, #184]	@ 42d4 <nss_gmac_slowpath_if_xmit+0x174>
		    ((Length1 -
    4218:	2246bd7f 	subcs	fp, r6, #8128	@ 0x1fc0
		txdesc->length |= ((Length1 << desc_size1_shift) & desc_size1_mask);
    421c:	31861001 	orrcc	r1, r6, r1
		    ((Length1 -
    4220:	224bb03f 	subcs	fp, fp, #63	@ 0x3f
    4224:	e20ee060 	and	lr, lr, #96	@ 0x60
		      NSS_GMAC_MAX_DESC_BUFF) << desc_size2_shift) & desc_size2_mask;
    4228:	200aa80b 	andcs	sl, sl, fp, lsl #16
		txdesc->length |=
    422c:	218a1001 	orrcs	r1, sl, r1
    4230:	21e016a1 	mvncs	r1, r1, lsr #13
    4234:	21e01681 	mvncs	r1, r1, lsl #13
		txdesc->data1 = Buffer1 + NSS_GMAC_MAX_DESC_BUFF;
    4238:	e3560a02 	cmp	r6, #8192	@ 0x2000
	txdesc->buffer1 = Buffer1;
    423c:	e9830082 	stmib	r3, {r1, r7}
		txdesc->data1 = Buffer1 + NSS_GMAC_MAX_DESC_BUFF;
    4240:	e2877d7f 	add	r7, r7, #8128	@ 0x1fc0
	txdesc->status |= tx_cntl;
    4244:	e3821203 	orr	r1, r2, #805306368	@ 0x30000000
		txdesc->data1 = Buffer1 + NSS_GMAC_MAX_DESC_BUFF;
    4248:	e287703f 	add	r7, r7, #63	@ 0x3f
    424c:	21a0c007 	movcs	ip, r7
	if (likely(offload_needed))
    4250:	e35e0060 	cmp	lr, #96	@ 0x60
	txdesc->status |= tx_cntl;
    4254:	e5831000 	str	r1, [r3]
	    ((desc->status & (~desc_tx_cis_mask)) | desc_tx_cis_tcp_pseudo_cs);
    4258:	038225c3 	orreq	r2, r2, #817889280	@ 0x30c00000
    425c:	e583c00c 	str	ip, [r3, #12]
	desc->status =
    4260:	05832000 	streq	r2, [r3]
	wmb();
    4264:	f57ff04e 	dsb	st
	txdesc->status |= set_dma;
    4268:	e5932000 	ldr	r2, [r3]
	gmacdev->tx_next = (txnext + 1) & (gmacdev->tx_desc_count - 1);
    426c:	e2800001 	add	r0, r0, #1
	txdesc->status |= set_dma;
    4270:	e3822102 	orr	r2, r2, #-2147483648	@ 0x80000000
    4274:	e5832000 	str	r2, [r3]
	gmacdev->tx_next_desc = gmacdev->tx_desc + gmacdev->tx_next;
    4278:	e5942568 	ldr	r2, [r4, #1384]	@ 0x568
	gmacdev->tx_next = (txnext + 1) & (gmacdev->tx_desc_count - 1);
    427c:	e594357c 	ldr	r3, [r4, #1404]	@ 0x57c
    4280:	e2433001 	sub	r3, r3, #1
    4284:	e0033000 	and	r3, r3, r0
	raw_spin_unlock_bh(&lock->rlock);
    4288:	e2840e7a 	add	r0, r4, #1952	@ 0x7a0
    428c:	e280000c 	add	r0, r0, #12
    4290:	e5843584 	str	r3, [r4, #1412]	@ 0x584
	gmacdev->tx_next_desc = gmacdev->tx_desc + gmacdev->tx_next;
    4294:	e0823283 	add	r3, r2, r3, lsl #5
    4298:	e5843594 	str	r3, [r4, #1428]	@ 0x594
		gmacdev->busy_tx_desc++;
    429c:	e5943570 	ldr	r3, [r4, #1392]	@ 0x570
    42a0:	e2833001 	add	r3, r3, #1
    42a4:	e5843570 	str	r3, [r4, #1392]	@ 0x570
    42a8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	addr = (uint32_t)regbase + regoffset;
    42ac:	e5943544 	ldr	r3, [r4, #1348]	@ 0x544
    42b0:	e3a02000 	mov	r2, #0
    42b4:	e5832004 	str	r2, [r3, #4]
 * @return returns void.
 */
static inline void nss_gmac_resume_dma_tx(struct nss_gmac_dev *gmacdev)
{
	nss_gmac_write_reg(gmacdev->dma_base, dma_tx_poll_demand, 0);
}
    42b8:	ea000002 	b	42c8 <nss_gmac_slowpath_if_xmit+0x168>
		return NETDEV_TX_BUSY;
    42bc:	e3a08010 	mov	r8, #16
    42c0:	ea000000 	b	42c8 <nss_gmac_slowpath_if_xmit+0x168>
	return NSS_GMAC_FAILURE;
    42c4:	e3a08001 	mov	r8, #1
}
    42c8:	e1a00008 	mov	r0, r8
    42cc:	e28dd004 	add	sp, sp, #4
    42d0:	e8bd8ff0 	pop	{r4, r5, r6, r7, r8, r9, sl, fp, pc}
    42d4:	1fff0000 	.word	0x1fff0000

000042d8 <nss_gmac_slowpath_if_pause_on_off>:
static int nss_gmac_slowpath_if_pause_on_off(void *app_data, uint32_t pause_on)
    42d8:	e3a00000 	mov	r0, #0
    42dc:	e12fff1e 	bx	lr

000042e0 <nss_gmac_handle_irq>:
	nss_gmac_clear_interrupt(gmacdev);
    42e0:	e5912004 	ldr	r2, [r1, #4]
{
    42e4:	e92d4010 	push	{r4, lr}
	asm volatile("ldr %0, %1"
    42e8:	e5923014 	ldr	r3, [r2, #20]
	asm volatile("str %1, %0"
    42ec:	e5823014 	str	r3, [r2, #20]
	asm volatile("ldr %0, %1"
    42f0:	e592301c 	ldr	r3, [r2, #28]
	data = ~interrupts & readl_relaxed(gmacdev->dma_base + dma_interrupt);
    42f4:	e3c33801 	bic	r3, r3, #65536	@ 0x10000
    42f8:	e3c33041 	bic	r3, r3, #65	@ 0x41
	asm volatile("str %1, %0"
    42fc:	e582301c 	str	r3, [r2, #28]
	napi_schedule(&gmacdev->napi);
    4300:	e28140c8 	add	r4, r1, #200	@ 0xc8
	if (napi_schedule_prep(n))
    4304:	e1a00004 	mov	r0, r4
    4308:	ebfffffe 	bl	0 <napi_schedule_prep>
    430c:	e3500000 	cmp	r0, #0
    4310:	0a000001 	beq	431c <nss_gmac_handle_irq+0x3c>
		__napi_schedule(n);
    4314:	e1a00004 	mov	r0, r4
    4318:	ebfffffe 	bl	0 <__napi_schedule>
}
    431c:	e3a00001 	mov	r0, #1
    4320:	e8bd8010 	pop	{r4, pc}

00004324 <nss_gmac_poll>:
{
    4324:	e92d4ff7 	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, sl, fp, lr}
    4328:	e1a04000 	mov	r4, r0
	raw_spin_lock(&lock->rlock);
    432c:	e2800f69 	add	r0, r0, #420	@ 0x1a4
    4330:	e1a07001 	mov	r7, r1
    4334:	ebfffffe 	bl	0 <_raw_spin_lock>
	busy = gmacdev->busy_tx_desc;
    4338:	e5148098 	ldr	r8, [r4, #-152]	@ 0xffffff68
	if (!busy) {
    433c:	e3580000 	cmp	r8, #0
    4340:	0a000063 	beq	44d4 <nss_gmac_poll+0x1b0>
	return dma_unmap_page_attrs(dev, addr, size, dir, attrs);
    4344:	e3a06000 	mov	r6, #0
		desc = gmacdev->tx_busy_desc;
    4348:	e514b078 	ldr	fp, [r4, #-120]	@ 0xffffff88
		if (nss_gmac_is_desc_owned_by_dma(desc)) {
    434c:	e59b3000 	ldr	r3, [fp]
    4350:	e3530000 	cmp	r3, #0
    4354:	ba00005e 	blt	44d4 <nss_gmac_poll+0x1b0>
		len = (desc->length & desc_size1_mask) >> desc_size1_shift;
    4358:	e59ba004 	ldr	sl, [fp, #4]
    435c:	e3a03001 	mov	r3, #1
		dma_unmap_single(&gmacdev->netdev->dev, desc->buffer1, len,
    4360:	e5140038 	ldr	r0, [r4, #-56]	@ 0xffffffc8
    4364:	e58d6000 	str	r6, [sp]
    4368:	e59b1008 	ldr	r1, [fp, #8]
    436c:	e7eca05a 	ubfx	sl, sl, #0, #13
    4370:	e1a0200a 	mov	r2, sl
    4374:	e2800d0d 	add	r0, r0, #832	@ 0x340
    4378:	ebfffffe 	bl	0 <dma_unmap_page_attrs>
		status = desc->status;
    437c:	e59b5000 	ldr	r5, [fp]
		if (status & desc_tx_last) {
    4380:	e3150202 	tst	r5, #536870912	@ 0x20000000
    4384:	0a000038 	beq	446c <nss_gmac_poll+0x148>
			skb = (struct sk_buff *)desc->reserved1;
    4388:	e59b0014 	ldr	r0, [fp, #20]
			BUG_ON(!skb);
    438c:	e3500000 	cmp	r0, #0
    4390:	0a0000b8 	beq	4678 <nss_gmac_poll+0x354>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    4394:	e3a01001 	mov	r1, #1
    4398:	ebfffffe 	bl	0 <kfree_skb_reason>
			if (unlikely(status & desc_error)) {
    439c:	e3150902 	tst	r5, #32768	@ 0x8000
    43a0:	0a00001e 	beq	4420 <nss_gmac_poll+0xfc>
				gmacdev->stats.tx_errors++;
    43a4:	e5943100 	ldr	r3, [r4, #256]	@ 0x100
    43a8:	e5942104 	ldr	r2, [r4, #260]	@ 0x104
				gmacdev->stats.tx_carrier_errors += (status & desc_tx_no_carrier) ? 1 : 0;
    43ac:	e5941160 	ldr	r1, [r4, #352]	@ 0x160
    43b0:	e5940164 	ldr	r0, [r4, #356]	@ 0x164
				gmacdev->stats.tx_errors++;
    43b4:	e2933001 	adds	r3, r3, #1
    43b8:	e2a22000 	adc	r2, r2, #0
    43bc:	e5843100 	str	r3, [r4, #256]	@ 0x100
				gmacdev->stats.tx_carrier_errors += (status & desc_tx_lost_carrier) ? 1 : 0;
    43c0:	e7e035d5 	ubfx	r3, r5, #11, #1
				gmacdev->stats.tx_errors++;
    43c4:	e5842104 	str	r2, [r4, #260]	@ 0x104
				gmacdev->stats.tx_carrier_errors += (status & desc_tx_no_carrier) ? 1 : 0;
    43c8:	e7e02555 	ubfx	r2, r5, #10, #1
    43cc:	e0933002 	adds	r3, r3, r2
    43d0:	e0a62006 	adc	r2, r6, r6
    43d4:	e0933001 	adds	r3, r3, r1
    43d8:	e0a22000 	adc	r2, r2, r0
				gmacdev->stats.tx_window_errors += (status & desc_tx_late_collision) ? 1 : 0;
    43dc:	e7e014d5 	ubfx	r1, r5, #9, #1
				gmacdev->stats.tx_carrier_errors += (status & desc_tx_no_carrier) ? 1 : 0;
    43e0:	e5843160 	str	r3, [r4, #352]	@ 0x160
				gmacdev->stats.tx_window_errors += (status & desc_tx_late_collision) ? 1 : 0;
    43e4:	e5943178 	ldr	r3, [r4, #376]	@ 0x178
				gmacdev->stats.tx_fifo_errors += (status & desc_tx_underflow) ? 1 : 0;
    43e8:	e7e050d5 	ubfx	r5, r5, #1, #1
				gmacdev->stats.tx_carrier_errors += (status & desc_tx_no_carrier) ? 1 : 0;
    43ec:	e5842164 	str	r2, [r4, #356]	@ 0x164
				gmacdev->stats.tx_window_errors += (status & desc_tx_late_collision) ? 1 : 0;
    43f0:	e594217c 	ldr	r2, [r4, #380]	@ 0x17c
    43f4:	e0933001 	adds	r3, r3, r1
    43f8:	e2a22000 	adc	r2, r2, #0
    43fc:	e5843178 	str	r3, [r4, #376]	@ 0x178
				gmacdev->stats.tx_fifo_errors += (status & desc_tx_underflow) ? 1 : 0;
    4400:	e5943168 	ldr	r3, [r4, #360]	@ 0x168
				gmacdev->stats.tx_window_errors += (status & desc_tx_late_collision) ? 1 : 0;
    4404:	e584217c 	str	r2, [r4, #380]	@ 0x17c
				gmacdev->stats.tx_fifo_errors += (status & desc_tx_underflow) ? 1 : 0;
    4408:	e594216c 	ldr	r2, [r4, #364]	@ 0x16c
    440c:	e0933005 	adds	r3, r3, r5
    4410:	e2a22000 	adc	r2, r2, #0
    4414:	e5843168 	str	r3, [r4, #360]	@ 0x168
    4418:	e584216c 	str	r2, [r4, #364]	@ 0x16c
    441c:	ea000012 	b	446c <nss_gmac_poll+0x148>
				gmacdev->stats.tx_packets++;
    4420:	e59430e0 	ldr	r3, [r4, #224]	@ 0xe0
	return (status & desc_tx_coll_mask) >> desc_tx_coll_shift;
    4424:	e7e351d5 	ubfx	r5, r5, #3, #4
    4428:	e59420e4 	ldr	r2, [r4, #228]	@ 0xe4
    442c:	e2933001 	adds	r3, r3, #1
    4430:	e2a22000 	adc	r2, r2, #0
    4434:	e58430e0 	str	r3, [r4, #224]	@ 0xe0
				gmacdev->stats.collisions += nss_gmac_get_tx_collision_count(status);
    4438:	e5943120 	ldr	r3, [r4, #288]	@ 0x120
				gmacdev->stats.tx_packets++;
    443c:	e58420e4 	str	r2, [r4, #228]	@ 0xe4
				gmacdev->stats.collisions += nss_gmac_get_tx_collision_count(status);
    4440:	e5942124 	ldr	r2, [r4, #292]	@ 0x124
    4444:	e0933005 	adds	r3, r3, r5
    4448:	e2a22000 	adc	r2, r2, #0
    444c:	e5843120 	str	r3, [r4, #288]	@ 0x120
				gmacdev->stats.tx_bytes += len;
    4450:	e59430f0 	ldr	r3, [r4, #240]	@ 0xf0
				gmacdev->stats.collisions += nss_gmac_get_tx_collision_count(status);
    4454:	e5842124 	str	r2, [r4, #292]	@ 0x124
				gmacdev->stats.tx_bytes += len;
    4458:	e59420f4 	ldr	r2, [r4, #244]	@ 0xf4
    445c:	e093300a 	adds	r3, r3, sl
    4460:	e2a22000 	adc	r2, r2, #0
    4464:	e58430f0 	str	r3, [r4, #240]	@ 0xf0
    4468:	e58420f4 	str	r2, [r4, #244]	@ 0xf4
	BUG_ON(txdesc != (gmacdev->tx_desc + txover));
    446c:	e51410a0 	ldr	r1, [r4, #-160]	@ 0xffffff60
	uint32_t txover = gmacdev->tx_busy;
    4470:	e5140088 	ldr	r0, [r4, #-136]	@ 0xffffff78
	struct dma_desc *txdesc = gmacdev->tx_busy_desc;
    4474:	e5143078 	ldr	r3, [r4, #-120]	@ 0xffffff88
	BUG_ON(txdesc != (gmacdev->tx_desc + txover));
    4478:	e0812280 	add	r2, r1, r0, lsl #5
    447c:	e1530002 	cmp	r3, r2
    4480:	1a00007c 	bne	4678 <nss_gmac_poll+0x354>
	gmacdev->tx_busy = (txover + 1) & (gmacdev->tx_desc_count - 1);
    4484:	e514208c 	ldr	r2, [r4, #-140]	@ 0xffffff74
    4488:	e2800001 	add	r0, r0, #1
		busy--;
    448c:	e2488001 	sub	r8, r8, #1
	} while (busy > 0);
    4490:	e3580000 	cmp	r8, #0
    4494:	e2422001 	sub	r2, r2, #1
    4498:	e0022000 	and	r2, r2, r0
	gmacdev->tx_busy_desc = gmacdev->tx_desc + gmacdev->tx_busy;
    449c:	e0811282 	add	r1, r1, r2, lsl #5
	gmacdev->tx_busy = (txover + 1) & (gmacdev->tx_desc_count - 1);
    44a0:	e5042088 	str	r2, [r4, #-136]	@ 0xffffff78
	gmacdev->tx_busy_desc = gmacdev->tx_desc + gmacdev->tx_busy;
    44a4:	e5041078 	str	r1, [r4, #-120]	@ 0xffffff88
	txdesc->status &= tx_desc_end_of_ring;
    44a8:	e5932000 	ldr	r2, [r3]
	txdesc->length = 0;
    44ac:	e5836004 	str	r6, [r3, #4]
	txdesc->buffer1 = 0;
    44b0:	e5836008 	str	r6, [r3, #8]
	txdesc->data1 = 0;
    44b4:	e583600c 	str	r6, [r3, #12]
	txdesc->status &= tx_desc_end_of_ring;
    44b8:	e2022602 	and	r2, r2, #2097152	@ 0x200000
    44bc:	e5832000 	str	r2, [r3]
	txdesc->reserved1 = 0;
    44c0:	e5836014 	str	r6, [r3, #20]
	(gmacdev->busy_tx_desc)--;
    44c4:	e5143098 	ldr	r3, [r4, #-152]	@ 0xffffff68
    44c8:	e2433001 	sub	r3, r3, #1
    44cc:	e5043098 	str	r3, [r4, #-152]	@ 0xffffff68
    44d0:	caffff9c 	bgt	4348 <nss_gmac_poll+0x24>
	raw_spin_unlock(&lock->rlock);
    44d4:	e2840f69 	add	r0, r4, #420	@ 0x1a4
    44d8:	ebfffe1f 	bl	3d5c <__raw_spin_unlock>
	if (!gmacdev->busy_rx_desc) {
    44dc:	e5140094 	ldr	r0, [r4, #-148]	@ 0xffffff6c
    44e0:	e3500000 	cmp	r0, #0
    44e4:	0a00007a 	beq	46d4 <nss_gmac_poll+0x3b0>
	if (busy > budget)
    44e8:	e1500007 	cmp	r0, r7
    44ec:	e3a09000 	mov	r9, #0
    44f0:	b1a06000 	movlt	r6, r0
    44f4:	a1a06007 	movge	r6, r7
		desc = gmacdev->rx_busy_desc;
    44f8:	e5141070 	ldr	r1, [r4, #-112]	@ 0xffffff90
	return (desc->status & desc_own_by_dma) == desc_own_by_dma;
    44fc:	e5915000 	ldr	r5, [r1]
		if (nss_gmac_is_desc_owned_by_dma(desc)) {
    4500:	e3550000 	cmp	r5, #0
    4504:	ba000070 	blt	46cc <nss_gmac_poll+0x3a8>
		dma_unmap_single(&gmacdev->netdev->dev, desc->buffer1,
    4508:	e5140038 	ldr	r0, [r4, #-56]	@ 0xffffffc8
    450c:	e3a03002 	mov	r3, #2
    4510:	e30027ba 	movw	r2, #1978	@ 0x7ba
		rx_skb = (struct sk_buff *)desc->reserved1;
    4514:	e5918014 	ldr	r8, [r1, #20]
    4518:	e58d9000 	str	r9, [sp]
    451c:	e5911008 	ldr	r1, [r1, #8]
    4520:	e2800d0d 	add	r0, r0, #832	@ 0x340
    4524:	ebfffffe 	bl	0 <dma_unmap_page_attrs>
	return (status & (desc_error | desc_rx_first | desc_rx_last)) ==
    4528:	e2053c83 	and	r3, r5, #33536	@ 0x8300
		if (likely(nss_gmac_is_rx_desc_valid(status))) {
    452c:	e3530c03 	cmp	r3, #768	@ 0x300
    4530:	1a00001b 	bne	45a4 <nss_gmac_poll+0x280>
			gmacdev->stats.rx_packets++;
    4534:	e59430d8 	ldr	r3, [r4, #216]	@ 0xd8
	return (status & desc_frame_length_mask) >> desc_frame_length_shift;
    4538:	e7ed1855 	ubfx	r1, r5, #16, #14
			skb_put(rx_skb, frame_length);
    453c:	e1a00008 	mov	r0, r8
			gmacdev->stats.rx_packets++;
    4540:	e59420dc 	ldr	r2, [r4, #220]	@ 0xdc
			frame_length -= ETH_FCS_LEN;
    4544:	e2411004 	sub	r1, r1, #4
			gmacdev->stats.rx_packets++;
    4548:	e2933001 	adds	r3, r3, #1
    454c:	e2a22000 	adc	r2, r2, #0
    4550:	e58430d8 	str	r3, [r4, #216]	@ 0xd8
			gmacdev->stats.rx_bytes += frame_length;
    4554:	e59430e8 	ldr	r3, [r4, #232]	@ 0xe8
			gmacdev->stats.rx_packets++;
    4558:	e58420dc 	str	r2, [r4, #220]	@ 0xdc
			gmacdev->stats.rx_bytes += frame_length;
    455c:	e59420ec 	ldr	r2, [r4, #236]	@ 0xec
    4560:	e0933001 	adds	r3, r3, r1
    4564:	e0a22fc1 	adc	r2, r2, r1, asr #31
    4568:	e58430e8 	str	r3, [r4, #232]	@ 0xe8
    456c:	e58420ec 	str	r2, [r4, #236]	@ 0xec
			skb_put(rx_skb, frame_length);
    4570:	ebfffffe 	bl	0 <skb_put>
			rx_skb->protocol = eth_type_trans(rx_skb, gmacdev->netdev);
    4574:	e5141038 	ldr	r1, [r4, #-56]	@ 0xffffffc8
    4578:	e1a00008 	mov	r0, r8
    457c:	ebfffffe 	bl	0 <eth_type_trans>
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
    4580:	e5d83074 	ldrb	r3, [r8, #116]	@ 0x74
    4584:	e3a02001 	mov	r2, #1
			napi_gro_receive(&gmacdev->napi, rx_skb);
    4588:	e1a01008 	mov	r1, r8
			rx_skb->protocol = eth_type_trans(rx_skb, gmacdev->netdev);
    458c:	e1c80ab8 	strh	r0, [r8, #168]	@ 0xa8
			napi_gro_receive(&gmacdev->napi, rx_skb);
    4590:	e1a00004 	mov	r0, r4
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
    4594:	e7c63292 	bfi	r3, r2, #5, #2
    4598:	e5c83074 	strb	r3, [r8, #116]	@ 0x74
			napi_gro_receive(&gmacdev->napi, rx_skb);
    459c:	ebfffffe 	bl	0 <napi_gro_receive>
    45a0:	ea00002e 	b	4660 <nss_gmac_poll+0x33c>
			gmacdev->stats.rx_errors++;
    45a4:	e59430f8 	ldr	r3, [r4, #248]	@ 0xf8
    45a8:	e3a01001 	mov	r1, #1
    45ac:	e1a00008 	mov	r0, r8
    45b0:	e59420fc 	ldr	r2, [r4, #252]	@ 0xfc
    45b4:	e2933001 	adds	r3, r3, #1
    45b8:	e2a22000 	adc	r2, r2, #0
    45bc:	e58430f8 	str	r3, [r4, #248]	@ 0xf8
    45c0:	e58420fc 	str	r2, [r4, #252]	@ 0xfc
    45c4:	ebfffffe 	bl	0 <kfree_skb_reason>
			if (status & (desc_rx_crc | desc_rx_collision |
    45c8:	e3013846 	movw	r3, #6214	@ 0x1846
    45cc:	e1150003 	tst	r5, r3
    45d0:	0a000022 	beq	4660 <nss_gmac_poll+0x33c>
				gmacdev->stats.rx_crc_errors += (status & desc_rx_crc) ? 1 : 0;
    45d4:	e5943138 	ldr	r3, [r4, #312]	@ 0x138
    45d8:	e7e010d5 	ubfx	r1, r5, #1, #1
    45dc:	e594213c 	ldr	r2, [r4, #316]	@ 0x13c
    45e0:	e0933001 	adds	r3, r3, r1
				gmacdev->stats.collisions += (status & desc_rx_collision) ? 1 : 0;
    45e4:	e7e01355 	ubfx	r1, r5, #6, #1
				gmacdev->stats.rx_crc_errors += (status & desc_rx_crc) ? 1 : 0;
    45e8:	e2a22000 	adc	r2, r2, #0
    45ec:	e5843138 	str	r3, [r4, #312]	@ 0x138
				gmacdev->stats.collisions += (status & desc_rx_collision) ? 1 : 0;
    45f0:	e5943120 	ldr	r3, [r4, #288]	@ 0x120
				gmacdev->stats.rx_crc_errors += (status & desc_rx_crc) ? 1 : 0;
    45f4:	e584213c 	str	r2, [r4, #316]	@ 0x13c
				gmacdev->stats.collisions += (status & desc_rx_collision) ? 1 : 0;
    45f8:	e5942124 	ldr	r2, [r4, #292]	@ 0x124
    45fc:	e0933001 	adds	r3, r3, r1
				gmacdev->stats.rx_over_errors += (status & desc_rx_damaged) ? 1 : 0;
    4600:	e7e015d5 	ubfx	r1, r5, #11, #1
				gmacdev->stats.collisions += (status & desc_rx_collision) ? 1 : 0;
    4604:	e2a22000 	adc	r2, r2, #0
    4608:	e5843120 	str	r3, [r4, #288]	@ 0x120
				gmacdev->stats.rx_over_errors += (status & desc_rx_damaged) ? 1 : 0;
    460c:	e5943130 	ldr	r3, [r4, #304]	@ 0x130
				gmacdev->stats.collisions += (status & desc_rx_collision) ? 1 : 0;
    4610:	e5842124 	str	r2, [r4, #292]	@ 0x124
				gmacdev->stats.rx_over_errors += (status & desc_rx_damaged) ? 1 : 0;
    4614:	e5942134 	ldr	r2, [r4, #308]	@ 0x134
    4618:	e0933001 	adds	r3, r3, r1
				gmacdev->stats.rx_frame_errors += (status & desc_rx_dribbling) ? 1 : 0;
    461c:	e7e01155 	ubfx	r1, r5, #2, #1
				gmacdev->stats.rx_over_errors += (status & desc_rx_damaged) ? 1 : 0;
    4620:	e2a22000 	adc	r2, r2, #0
    4624:	e5843130 	str	r3, [r4, #304]	@ 0x130
				gmacdev->stats.rx_frame_errors += (status & desc_rx_dribbling) ? 1 : 0;
    4628:	e5943140 	ldr	r3, [r4, #320]	@ 0x140
				gmacdev->stats.rx_over_errors += (status & desc_rx_damaged) ? 1 : 0;
    462c:	e5842134 	str	r2, [r4, #308]	@ 0x134
				gmacdev->stats.rx_frame_errors += (status & desc_rx_dribbling) ? 1 : 0;
    4630:	e5942144 	ldr	r2, [r4, #324]	@ 0x144
    4634:	e0933001 	adds	r3, r3, r1
				gmacdev->stats.rx_length_errors += (status & desc_rx_length_error) ? 1 : 0;
    4638:	e594112c 	ldr	r1, [r4, #300]	@ 0x12c
				gmacdev->stats.rx_frame_errors += (status & desc_rx_dribbling) ? 1 : 0;
    463c:	e2a22000 	adc	r2, r2, #0
    4640:	e5843140 	str	r3, [r4, #320]	@ 0x140
				gmacdev->stats.rx_length_errors += (status & desc_rx_length_error) ? 1 : 0;
    4644:	e7e03655 	ubfx	r3, r5, #12, #1
				gmacdev->stats.rx_frame_errors += (status & desc_rx_dribbling) ? 1 : 0;
    4648:	e5842144 	str	r2, [r4, #324]	@ 0x144
				gmacdev->stats.rx_length_errors += (status & desc_rx_length_error) ? 1 : 0;
    464c:	e5942128 	ldr	r2, [r4, #296]	@ 0x128
    4650:	e0922003 	adds	r2, r2, r3
    4654:	e2a13000 	adc	r3, r1, #0
    4658:	e5842128 	str	r2, [r4, #296]	@ 0x128
    465c:	e584312c 	str	r3, [r4, #300]	@ 0x12c
	BUG_ON(rxdesc != (gmacdev->rx_desc + rxnext));
    4660:	e514109c 	ldr	r1, [r4, #-156]	@ 0xffffff64
	uint32_t rxnext = gmacdev->rx_busy;
    4664:	e5140080 	ldr	r0, [r4, #-128]	@ 0xffffff80
	struct dma_desc *rxdesc = gmacdev->rx_busy_desc;
    4668:	e5143070 	ldr	r3, [r4, #-112]	@ 0xffffff90
	BUG_ON(rxdesc != (gmacdev->rx_desc + rxnext));
    466c:	e0812280 	add	r2, r1, r0, lsl #5
    4670:	e1530002 	cmp	r3, r2
    4674:	0a000000 	beq	467c <nss_gmac_poll+0x358>
    4678:	e7f001f2 	.word	0xe7f001f2
	gmacdev->rx_busy = (rxnext + 1) & (gmacdev->rx_desc_count - 1);
    467c:	e5142090 	ldr	r2, [r4, #-144]	@ 0xffffff70
    4680:	e2800001 	add	r0, r0, #1
		busy--;
    4684:	e2466001 	sub	r6, r6, #1
	} while (busy > 0);
    4688:	e3560000 	cmp	r6, #0
    468c:	e2422001 	sub	r2, r2, #1
    4690:	e0022000 	and	r2, r2, r0
	gmacdev->rx_busy_desc = gmacdev->rx_desc + gmacdev->rx_busy;
    4694:	e0811282 	add	r1, r1, r2, lsl #5
	gmacdev->rx_busy = (rxnext + 1) & (gmacdev->rx_desc_count - 1);
    4698:	e5042080 	str	r2, [r4, #-128]	@ 0xffffff80
	gmacdev->rx_busy_desc = gmacdev->rx_desc + gmacdev->rx_busy;
    469c:	e5041070 	str	r1, [r4, #-112]	@ 0xffffff90
	rxdesc->length &= rx_desc_end_of_ring;
    46a0:	e5932004 	ldr	r2, [r3, #4]
	rxdesc->status = 0;
    46a4:	e5839000 	str	r9, [r3]
	rxdesc->buffer1 = 0;
    46a8:	e5839008 	str	r9, [r3, #8]
	rxdesc->data1 = 0;
    46ac:	e583900c 	str	r9, [r3, #12]
	rxdesc->length &= rx_desc_end_of_ring;
    46b0:	e2022902 	and	r2, r2, #32768	@ 0x8000
    46b4:	e5832004 	str	r2, [r3, #4]
	rxdesc->reserved1 = 0;
    46b8:	e5839014 	str	r9, [r3, #20]
	(gmacdev->busy_rx_desc)--;
    46bc:	e5143094 	ldr	r3, [r4, #-148]	@ 0xffffff6c
    46c0:	e2433001 	sub	r3, r3, #1
    46c4:	e5043094 	str	r3, [r4, #-148]	@ 0xffffff6c
    46c8:	caffff8a 	bgt	44f8 <nss_gmac_poll+0x1d4>
	return budget - busy;
    46cc:	e0476006 	sub	r6, r7, r6
    46d0:	ea000000 	b	46d8 <nss_gmac_poll+0x3b4>
		return 0;
    46d4:	e1a06000 	mov	r6, r0
	nss_gmac_rx_refill(gmacdev);
    46d8:	e24400c8 	sub	r0, r4, #200	@ 0xc8
    46dc:	ebfffe58 	bl	4044 <nss_gmac_rx_refill>
	if (work_done < budget) {
    46e0:	e1570006 	cmp	r7, r6
    46e4:	da000005 	ble	4700 <nss_gmac_poll+0x3dc>
	return napi_complete_done(n, 0);
    46e8:	e3a01000 	mov	r1, #0
    46ec:	e1a00004 	mov	r0, r4
    46f0:	ebfffffe 	bl	0 <napi_complete_done>
	addr = (uint32_t)regbase + regoffset;
    46f4:	e51430c4 	ldr	r3, [r4, #-196]	@ 0xffffff3c
    46f8:	e59f200c 	ldr	r2, [pc, #12]	@ 470c <nss_gmac_poll+0x3e8>
    46fc:	e583201c 	str	r2, [r3, #28]
}
    4700:	e1a00006 	mov	r0, r6
    4704:	e28dd00c 	add	sp, sp, #12
    4708:	e8bd8ff0 	pop	{r4, r5, r6, r7, r8, r9, sl, fp, pc}
    470c:	00010041 	.word	0x00010041

00004710 <nss_gmac_update_prec_stats>:
{
    4710:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
	if (dir == __NSS_GMAC_DIR_RX)
    4714:	e3520001 	cmp	r2, #1
	skb->network_header = skb->data - skb->head;
    4718:	e59140b8 	ldr	r4, [r1, #184]	@ 0xb8
{
    471c:	e1a06002 	mov	r6, r2
    4720:	e24dd038 	sub	sp, sp, #56	@ 0x38
    4724:	e59130bc 	ldr	r3, [r1, #188]	@ 0xbc
    4728:	e1a05000 	mov	r5, r0
    472c:	e1a07001 	mov	r7, r1
	switch (proto) {
    4730:	e3068488 	movw	r8, #25736	@ 0x6488
    4734:	00432004 	subeq	r2, r3, r4
    4738:	01c12abc 	strheq	r2, [r1, #172]	@ 0xac
	return skb->head + skb->network_header;
    473c:	e1d12abc 	ldrh	r2, [r1, #172]	@ 0xac
    4740:	e0844002 	add	r4, r4, r2
	return skb_network_header(skb) - skb->data;
    4744:	e0444003 	sub	r4, r4, r3
	__be16 proto = skb->protocol;
    4748:	e1d13ab8 	ldrh	r3, [r1, #168]	@ 0xa8
	switch (proto) {
    474c:	e1530008 	cmp	r3, r8
    4750:	0a00001d 	beq	47cc <nss_gmac_update_prec_stats+0xbc>
    4754:	8a000004 	bhi	476c <nss_gmac_update_prec_stats+0x5c>
    4758:	e3530008 	cmp	r3, #8
    475c:	0a00000c 	beq	4794 <nss_gmac_update_prec_stats+0x84>
    4760:	e3530081 	cmp	r3, #129	@ 0x81
    4764:	0a00000e 	beq	47a4 <nss_gmac_update_prec_stats+0x94>
    4768:	ea000054 	b	48c0 <nss_gmac_update_prec_stats+0x1b0>
    476c:	e30a2888 	movw	r2, #43144	@ 0xa888
    4770:	e1530002 	cmp	r3, r2
    4774:	0a00000a 	beq	47a4 <nss_gmac_update_prec_stats+0x94>
    4778:	e30d2d86 	movw	r2, #56710	@ 0xdd86
    477c:	e1530002 	cmp	r3, r2
	iph = skb_header_pointer(skb, nh_offset, sizeof(iph_hdr), &iph_hdr);
    4780:	028d3010 	addeq	r3, sp, #16
    4784:	03a02028 	moveq	r2, #40	@ 0x28
    4788:	01a01004 	moveq	r1, r4
	switch (proto) {
    478c:	0a00002b 	beq	4840 <nss_gmac_update_prec_stats+0x130>
    4790:	ea00004a 	b	48c0 <nss_gmac_update_prec_stats+0x1b0>
	iph = skb_header_pointer(skb, nh_offset, sizeof(iph_hdr), &iph_hdr);
    4794:	e28d3010 	add	r3, sp, #16
    4798:	e3a02014 	mov	r2, #20
    479c:	e1a01004 	mov	r1, r4
    47a0:	ea000019 	b	480c <nss_gmac_update_prec_stats+0xfc>
		vlan = skb_header_pointer(skb, nhoff, sizeof(_vlan), &_vlan);
    47a4:	e28d3004 	add	r3, sp, #4
    47a8:	e3a02004 	mov	r2, #4
    47ac:	e1a01004 	mov	r1, r4
    47b0:	e1a00007 	mov	r0, r7
    47b4:	ebfffd8c 	bl	3dec <skb_header_pointer>
		if (!vlan)
    47b8:	e3500000 	cmp	r0, #0
    47bc:	0a00003f 	beq	48c0 <nss_gmac_update_prec_stats+0x1b0>
		proto = vlan->h_vlan_encapsulated_proto;
    47c0:	e1d030b2 	ldrh	r3, [r0, #2]
		nhoff += sizeof(*vlan);
    47c4:	e2844004 	add	r4, r4, #4
		goto inner;
    47c8:	eaffffdf 	b	474c <nss_gmac_update_prec_stats+0x3c>
		pppoeh = skb_header_pointer(skb, nhoff,
    47cc:	e28d3008 	add	r3, sp, #8
    47d0:	e3a02008 	mov	r2, #8
    47d4:	e1a01004 	mov	r1, r4
    47d8:	e1a00007 	mov	r0, r7
    47dc:	ebfffd82 	bl	3dec <skb_header_pointer>
		if (!pppoeh)
    47e0:	e3500000 	cmp	r0, #0
    47e4:	0a000035 	beq	48c0 <nss_gmac_update_prec_stats+0x1b0>
		proto = pppoeh->proto;
    47e8:	e1d030b6 	ldrh	r3, [r0, #6]
		nhoff += PPPOE_SES_HLEN;
    47ec:	e2841008 	add	r1, r4, #8
		switch (proto) {
    47f0:	e3530c21 	cmp	r3, #8448	@ 0x2100
    47f4:	0a000002 	beq	4804 <nss_gmac_update_prec_stats+0xf4>
    47f8:	e3530c57 	cmp	r3, #22272	@ 0x5700
    47fc:	0a00000d 	beq	4838 <nss_gmac_update_prec_stats+0x128>
    4800:	ea00002e 	b	48c0 <nss_gmac_update_prec_stats+0x1b0>
	iph = skb_header_pointer(skb, nh_offset, sizeof(iph_hdr), &iph_hdr);
    4804:	e28d3010 	add	r3, sp, #16
    4808:	e3a02014 	mov	r2, #20
    480c:	e1a00007 	mov	r0, r7
    4810:	ebfffd75 	bl	3dec <skb_header_pointer>
	if (!iph || iph->ihl < 5)
    4814:	e3500000 	cmp	r0, #0
    4818:	0a000028 	beq	48c0 <nss_gmac_update_prec_stats+0x1b0>
    481c:	e5d03000 	ldrb	r3, [r0]
    4820:	e203300f 	and	r3, r3, #15
    4824:	e3530004 	cmp	r3, #4
	*precedence = iph->tos >> NSS_GMAC_DSCP_PREC_SHIFT;
    4828:	85d04001 	ldrbhi	r4, [r0, #1]
    482c:	81a042a4 	lsrhi	r4, r4, #5
	if (!iph || iph->ihl < 5)
    4830:	8a000008 	bhi	4858 <nss_gmac_update_prec_stats+0x148>
    4834:	ea000021 	b	48c0 <nss_gmac_update_prec_stats+0x1b0>
	iph = skb_header_pointer(skb, nh_offset, sizeof(iph_hdr), &iph_hdr);
    4838:	e28d3010 	add	r3, sp, #16
    483c:	e3a02028 	mov	r2, #40	@ 0x28
    4840:	e1a00007 	mov	r0, r7
    4844:	ebfffd68 	bl	3dec <skb_header_pointer>
	if (!iph)
    4848:	e3500000 	cmp	r0, #0
    484c:	0a00001b 	beq	48c0 <nss_gmac_update_prec_stats+0x1b0>
	*precedence = iph->priority >> NSS_GMAC_DSCP6_PREC_SHIFT;
    4850:	e5d04000 	ldrb	r4, [r0]
    4854:	e7e240d4 	ubfx	r4, r4, #1, #3
	raw_spin_lock_bh(&lock->rlock);
    4858:	e2857f9a 	add	r7, r5, #616	@ 0x268
    485c:	e1a00007 	mov	r0, r7
    4860:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		if (dir == __NSS_GMAC_DIR_TX)
    4864:	e3560000 	cmp	r6, #0
    4868:	1a000007 	bne	488c <nss_gmac_update_prec_stats+0x17c>
			gmacdev->nss_host_stats.tx_per_prec[precedence]++;
    486c:	e0854184 	add	r4, r5, r4, lsl #3
    4870:	e59434e8 	ldr	r3, [r4, #1256]	@ 0x4e8
    4874:	e59424ec 	ldr	r2, [r4, #1260]	@ 0x4ec
    4878:	e2933001 	adds	r3, r3, #1
    487c:	e2a22000 	adc	r2, r2, #0
    4880:	e58434e8 	str	r3, [r4, #1256]	@ 0x4e8
    4884:	e58424ec 	str	r2, [r4, #1260]	@ 0x4ec
    4888:	ea000008 	b	48b0 <nss_gmac_update_prec_stats+0x1a0>
		else if (dir == __NSS_GMAC_DIR_RX)
    488c:	e3560001 	cmp	r6, #1
    4890:	1a000006 	bne	48b0 <nss_gmac_update_prec_stats+0x1a0>
			gmacdev->nss_host_stats.rx_per_prec[precedence]++;
    4894:	e0855184 	add	r5, r5, r4, lsl #3
    4898:	e59534a8 	ldr	r3, [r5, #1192]	@ 0x4a8
    489c:	e59524ac 	ldr	r2, [r5, #1196]	@ 0x4ac
    48a0:	e2933001 	adds	r3, r3, #1
    48a4:	e2a22000 	adc	r2, r2, #0
    48a8:	e58534a8 	str	r3, [r5, #1192]	@ 0x4a8
    48ac:	e58524ac 	str	r2, [r5, #1196]	@ 0x4ac
	raw_spin_unlock_bh(&lock->rlock);
    48b0:	e1a00007 	mov	r0, r7
}
    48b4:	e28dd038 	add	sp, sp, #56	@ 0x38
    48b8:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
    48bc:	eafffffe 	b	0 <_raw_spin_unlock_bh>
    48c0:	e28dd038 	add	sp, sp, #56	@ 0x38
    48c4:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}

000048c8 <nss_gmac_receive>:
	BUG_ON(netdev == NULL);
    48c8:	e2503000 	subs	r3, r0, #0
    48cc:	1a000000 	bne	48d4 <nss_gmac_receive+0xc>
    48d0:	e7f001f2 	.word	0xe7f001f2
{
    48d4:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(gmacdev->netdev != netdev);
    48d8:	e59355d0 	ldr	r5, [r3, #1488]	@ 0x5d0
    48dc:	e1550003 	cmp	r5, r3
    48e0:	0a000000 	beq	48e8 <nss_gmac_receive+0x20>
    48e4:	e7f001f2 	.word	0xe7f001f2
    48e8:	e1a04001 	mov	r4, r1
	skb->protocol = eth_type_trans(skb, netdev);
    48ec:	e1a01005 	mov	r1, r5
    48f0:	e1a00004 	mov	r0, r4
    48f4:	e1a06002 	mov	r6, r2
	skb->dev = netdev;
    48f8:	e5845008 	str	r5, [r4, #8]
	skb->protocol = eth_type_trans(skb, netdev);
    48fc:	ebfffffe 	bl	0 <eth_type_trans>
    4900:	e1c40ab8 	strh	r0, [r4, #168]	@ 0xa8
	if (gmacdev->ctx->nss_gmac_per_prec_stats_enable)
    4904:	e59537c4 	ldr	r3, [r5, #1988]	@ 0x7c4
    4908:	e5933034 	ldr	r3, [r3, #52]	@ 0x34
    490c:	e3530000 	cmp	r3, #0
    4910:	0a000003 	beq	4924 <nss_gmac_receive+0x5c>
		nss_gmac_update_prec_stats(gmacdev, skb, __NSS_GMAC_DIR_RX);
    4914:	e3a02001 	mov	r2, #1
    4918:	e1a01004 	mov	r1, r4
    491c:	e2850d15 	add	r0, r5, #1344	@ 0x540
    4920:	ebfffffe 	bl	4710 <nss_gmac_update_prec_stats>
	napi_gro_receive(napi, skb);
    4924:	e1a01004 	mov	r1, r4
    4928:	e1a00006 	mov	r0, r6
}
    492c:	e8bd4070 	pop	{r4, r5, r6, lr}
	napi_gro_receive(napi, skb);
    4930:	eafffffe 	b	0 <napi_gro_receive>

00004934 <nss_gmac_xmit_frames>:
{
    4934:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(skb == NULL);
    4938:	e2505000 	subs	r5, r0, #0
    493c:	0a000014 	beq	4994 <nss_gmac_xmit_frames+0x60>
	if (skb->len < ETH_HLEN) {
    4940:	e5953064 	ldr	r3, [r5, #100]	@ 0x64
    4944:	e1a04001 	mov	r4, r1
	return (char *)dev + ALIGN(sizeof(struct net_device), NETDEV_ALIGN);
    4948:	e2810d15 	add	r0, r1, #1344	@ 0x540
    494c:	e353000d 	cmp	r3, #13
    4950:	8a00000a 	bhi	4980 <nss_gmac_xmit_frames+0x4c>
	netdev_info(netdev, "%s: dropping skb\n", __func__);
    4954:	e59f207c 	ldr	r2, [pc, #124]	@ 49d8 <nss_gmac_xmit_frames+0xa4>
    4958:	e1a00004 	mov	r0, r4
    495c:	e59f1078 	ldr	r1, [pc, #120]	@ 49dc <nss_gmac_xmit_frames+0xa8>
    4960:	ebfffffe 	bl	0 <netdev_info>
	__dev_kfree_skb_any(skb, SKB_REASON_DROPPED);
    4964:	e3a01001 	mov	r1, #1
    4968:	e1a00005 	mov	r0, r5
    496c:	ebfffffe 	bl	0 <__dev_kfree_skb_any>
	netdev->stats.tx_dropped++;
    4970:	e59430f4 	ldr	r3, [r4, #244]	@ 0xf4
    4974:	e2833001 	add	r3, r3, #1
    4978:	e58430f4 	str	r3, [r4, #244]	@ 0xf4
	return NETDEV_TX_OK;
    497c:	ea000013 	b	49d0 <nss_gmac_xmit_frames+0x9c>
	BUG_ON(gmacdev == NULL);
    4980:	e3500000 	cmp	r0, #0
    4984:	0a000002 	beq	4994 <nss_gmac_xmit_frames+0x60>
	BUG_ON(gmacdev->netdev != netdev);
    4988:	e59135d0 	ldr	r3, [r1, #1488]	@ 0x5d0
    498c:	e1530001 	cmp	r3, r1
    4990:	0a000000 	beq	4998 <nss_gmac_xmit_frames+0x64>
    4994:	e7f001f2 	.word	0xe7f001f2
	if (gmacdev->ctx->nss_gmac_per_prec_stats_enable)
    4998:	e59137c4 	ldr	r3, [r1, #1988]	@ 0x7c4
    499c:	e5933034 	ldr	r3, [r3, #52]	@ 0x34
    49a0:	e3530000 	cmp	r3, #0
    49a4:	0a000002 	beq	49b4 <nss_gmac_xmit_frames+0x80>
		nss_gmac_update_prec_stats(gmacdev, skb, __NSS_GMAC_DIR_TX);
    49a8:	e3a02000 	mov	r2, #0
    49ac:	e1a01005 	mov	r1, r5
    49b0:	ebfffffe 	bl	4710 <nss_gmac_update_prec_stats>
	msg_status = gmacdev->data_plane_ops->xmit(gmacdev->data_plane_ctx, skb);
    49b4:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    49b8:	e1a01005 	mov	r1, r5
    49bc:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    49c0:	e5933014 	ldr	r3, [r3, #20]
    49c4:	e12fff33 	blx	r3
	if (likely(msg_status == NSS_GMAC_SUCCESS))
    49c8:	e3500000 	cmp	r0, #0
    49cc:	1affffe0 	bne	4954 <nss_gmac_xmit_frames+0x20>
}
    49d0:	e3a00000 	mov	r0, #0
    49d4:	e8bd8070 	pop	{r4, r5, r6, pc}
    49d8:	00001005 	.word	0x00001005
    49dc:	000012b0 	.word	0x000012b0

000049e0 <nss_gmac_close>:
{
    49e0:	e92d4070 	push	{r4, r5, r6, lr}
	if (!gmacdev)
    49e4:	e2906d15 	adds	r6, r0, #1344	@ 0x540
    49e8:	0a000030 	beq	4ab0 <nss_gmac_close+0xd0>
    49ec:	e1a04000 	mov	r4, r0
	set_bit(__NSS_GMAC_CLOSING, &gmacdev->flags);
    49f0:	e2861018 	add	r1, r6, #24
    49f4:	e3a00001 	mov	r0, #1
    49f8:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
    49fc:	ebfffffe 	bl	0 <_set_bit>
	WRITE_ONCE(dev_queue->trans_start, jiffies);
    4a00:	e59f30b0 	ldr	r3, [pc, #176]	@ 4ab8 <nss_gmac_close+0xd8>
    4a04:	e5941280 	ldr	r1, [r4, #640]	@ 0x280
    4a08:	e5933000 	ldr	r3, [r3]
    4a0c:	e5813048 	str	r3, [r1, #72]	@ 0x48
	smp_mb__before_atomic();
    4a10:	f57ff05b 	dmb	ish
	set_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
    4a14:	e3a00000 	mov	r0, #0
    4a18:	e281104c 	add	r1, r1, #76	@ 0x4c
    4a1c:	ebfffffe 	bl	0 <_set_bit>
	netif_carrier_off(netdev);
    4a20:	e1a00004 	mov	r0, r4
    4a24:	ebfffffe 	bl	0 <netif_carrier_off>
	nss_gmac_rx_disable(gmacdev);
    4a28:	e1a00006 	mov	r0, r6
    4a2c:	ebfffffe 	bl	45c <nss_gmac_rx_disable>
	nss_gmac_tx_disable(gmacdev);
    4a30:	e1a00006 	mov	r0, r6
    4a34:	ebfffffe 	bl	484 <nss_gmac_tx_disable>
    4a38:	e5943544 	ldr	r3, [r4, #1348]	@ 0x544
    4a3c:	e3a02000 	mov	r2, #0
    4a40:	e583201c 	str	r2, [r3, #28]
    4a44:	e5943540 	ldr	r3, [r4, #1344]	@ 0x540
    4a48:	e3e02000 	mvn	r2, #0
    4a4c:	e583203c 	str	r2, [r3, #60]	@ 0x3c
	gmacdev->data_plane_ops->link_state(gmacdev->data_plane_ctx, 0);
    4a50:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    4a54:	e3a01000 	mov	r1, #0
    4a58:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    4a5c:	e5933008 	ldr	r3, [r3, #8]
    4a60:	e12fff33 	blx	r3
	if (!IS_ERR(gmacdev->phydev)) {
    4a64:	e59407d0 	ldr	r0, [r4, #2000]	@ 0x7d0
    4a68:	e3700a01 	cmn	r0, #4096	@ 0x1000
    4a6c:	8a000003 	bhi	4a80 <nss_gmac_close+0xa0>
    4a70:	e5943558 	ldr	r3, [r4, #1368]	@ 0x558
		if (test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags))
    4a74:	e3130040 	tst	r3, #64	@ 0x40
    4a78:	0a000000 	beq	4a80 <nss_gmac_close+0xa0>
			phy_stop(gmacdev->phydev);
    4a7c:	ebfffffe 	bl	0 <phy_stop>
	clear_bit(__NSS_GMAC_UP, &gmacdev->flags);
    4a80:	e2861018 	add	r1, r6, #24
    4a84:	e3a00000 	mov	r0, #0
    4a88:	ebfffffe 	bl	0 <_clear_bit>
	clear_bit(__NSS_GMAC_CLOSING, &gmacdev->flags);
    4a8c:	e2861018 	add	r1, r6, #24
    4a90:	e3a00001 	mov	r0, #1
    4a94:	ebfffffe 	bl	0 <_clear_bit>
	gmacdev->data_plane_ops->close(gmacdev->data_plane_ctx);
    4a98:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    4a9c:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    4aa0:	e5933004 	ldr	r3, [r3, #4]
    4aa4:	e12fff33 	blx	r3
	return 0;
    4aa8:	e3a00000 	mov	r0, #0
    4aac:	e8bd8070 	pop	{r4, r5, r6, pc}
		return -EINVAL;
    4ab0:	e3e00015 	mvn	r0, #21
}
    4ab4:	e8bd8070 	pop	{r4, r5, r6, pc}
    4ab8:	00000000 	.word	0x00000000

00004abc <nss_gmac_override_data_plane>:
{
    4abc:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
	BUG_ON(!gmacdev);
    4ac0:	e2907d15 	adds	r7, r0, #1344	@ 0x540
    4ac4:	1a000000 	bne	4acc <nss_gmac_override_data_plane+0x10>
    4ac8:	e7f001f2 	.word	0xe7f001f2
	if (!dp_ops->open || !dp_ops->close || !dp_ops->link_state
    4acc:	e5913000 	ldr	r3, [r1]
    4ad0:	e1a04000 	mov	r4, r0
    4ad4:	e1a06002 	mov	r6, r2
    4ad8:	e1a05001 	mov	r5, r1
    4adc:	e3530000 	cmp	r3, #0
    4ae0:	0a000014 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
    4ae4:	e5913004 	ldr	r3, [r1, #4]
    4ae8:	e3530000 	cmp	r3, #0
    4aec:	0a000011 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
    4af0:	e5913008 	ldr	r3, [r1, #8]
    4af4:	e3530000 	cmp	r3, #0
    4af8:	0a00000e 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
		|| !dp_ops->mac_addr || !dp_ops->change_mtu || !dp_ops->xmit
    4afc:	e591300c 	ldr	r3, [r1, #12]
    4b00:	e3530000 	cmp	r3, #0
    4b04:	0a00000b 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
    4b08:	e5913010 	ldr	r3, [r1, #16]
    4b0c:	e3530000 	cmp	r3, #0
    4b10:	0a000008 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
    4b14:	e5913014 	ldr	r3, [r1, #20]
    4b18:	e3530000 	cmp	r3, #0
    4b1c:	0a000005 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
		|| !dp_ops->set_features || !dp_ops->pause_on_off) {
    4b20:	e5913018 	ldr	r3, [r1, #24]
    4b24:	e3530000 	cmp	r3, #0
    4b28:	0a000002 	beq	4b38 <nss_gmac_override_data_plane+0x7c>
    4b2c:	e591301c 	ldr	r3, [r1, #28]
    4b30:	e3530000 	cmp	r3, #0
    4b34:	1a000005 	bne	4b50 <nss_gmac_override_data_plane+0x94>
		netdev_info(netdev, "%s: All the op functions must be present, reject this registeration\n",
    4b38:	e1a00004 	mov	r0, r4
    4b3c:	e59f2060 	ldr	r2, [pc, #96]	@ 4ba4 <nss_gmac_override_data_plane+0xe8>
    4b40:	e59f1060 	ldr	r1, [pc, #96]	@ 4ba8 <nss_gmac_override_data_plane+0xec>
    4b44:	ebfffffe 	bl	0 <netdev_info>
		return NSS_GMAC_FAILURE;
    4b48:	e3a00001 	mov	r0, #1
    4b4c:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    4b50:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (test_bit(__NSS_GMAC_UP, &gmacdev->flags)) {
    4b54:	e3130001 	tst	r3, #1
    4b58:	0a000006 	beq	4b78 <nss_gmac_override_data_plane+0xbc>
		nss_gmac_close(netdev);
    4b5c:	ebfffffe 	bl	49e0 <nss_gmac_close>
	netdev->features = 0;
    4b60:	e3a00000 	mov	r0, #0
    4b64:	e3a01000 	mov	r1, #0
    4b68:	e1c409f0 	strd	r0, [r4, #144]	@ 0x90
	netdev->hw_features = 0;
    4b6c:	e1c409f8 	strd	r0, [r4, #152]	@ 0x98
	netdev->wanted_features = 0;
    4b70:	e1c40af0 	strd	r0, [r4, #160]	@ 0xa0
	netdev->vlan_features = 0;
    4b74:	e1c40af8 	strd	r0, [r4, #168]	@ 0xa8
	if (gmacdev->data_plane_ops == &nss_gmac_slowpath_ops)
    4b78:	e5942a6c 	ldr	r2, [r4, #2668]	@ 0xa6c
    4b7c:	e59f3028 	ldr	r3, [pc, #40]	@ 4bac <nss_gmac_override_data_plane+0xf0>
    4b80:	e1520003 	cmp	r2, r3
    4b84:	1a000001 	bne	4b90 <nss_gmac_override_data_plane+0xd4>
		nss_gmac_tx_rx_desc_release(gmacdev);
    4b88:	e1a00007 	mov	r0, r7
    4b8c:	ebfffffe 	bl	2414 <nss_gmac_tx_rx_desc_release>
	gmacdev->first_linkup_done = 0;
    4b90:	e3a00000 	mov	r0, #0
    4b94:	e58405c0 	str	r0, [r4, #1472]	@ 0x5c0
	gmacdev->data_plane_ctx = ctx;
    4b98:	e58467cc 	str	r6, [r4, #1996]	@ 0x7cc
	gmacdev->data_plane_ops = dp_ops;
    4b9c:	e5845a6c 	str	r5, [r4, #2668]	@ 0xa6c
}
    4ba0:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    4ba4:	0000101a 	.word	0x0000101a
    4ba8:	000012c2 	.word	0x000012c2
    4bac:	00000468 	.word	0x00000468

00004bb0 <nss_gmac_restore_data_plane>:
{
    4bb0:	e92d4010 	push	{r4, lr}
    4bb4:	e1a04000 	mov	r4, r0
    4bb8:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (test_bit(__NSS_GMAC_UP, &gmacdev->flags)) {
    4bbc:	e3130001 	tst	r3, #1
    4bc0:	0a000006 	beq	4be0 <nss_gmac_restore_data_plane+0x30>
		nss_gmac_close(netdev);
    4bc4:	ebfffffe 	bl	49e0 <nss_gmac_close>
	netdev->features = 0;
    4bc8:	e3a02000 	mov	r2, #0
    4bcc:	e3a03000 	mov	r3, #0
    4bd0:	e1c429f0 	strd	r2, [r4, #144]	@ 0x90
	netdev->hw_features = 0;
    4bd4:	e1c429f8 	strd	r2, [r4, #152]	@ 0x98
	netdev->wanted_features = 0;
    4bd8:	e1c42af0 	strd	r2, [r4, #160]	@ 0xa0
	netdev->vlan_features = 0;
    4bdc:	e1c42af8 	strd	r2, [r4, #168]	@ 0xa8
	gmacdev->data_plane_ops = &nss_gmac_slowpath_ops;
    4be0:	e59f3008 	ldr	r3, [pc, #8]	@ 4bf0 <nss_gmac_restore_data_plane+0x40>
	gmacdev->data_plane_ctx = netdev;
    4be4:	e58447cc 	str	r4, [r4, #1996]	@ 0x7cc
	gmacdev->data_plane_ops = &nss_gmac_slowpath_ops;
    4be8:	e5843a6c 	str	r3, [r4, #2668]	@ 0xa6c
}
    4bec:	e8bd8010 	pop	{r4, pc}
    4bf0:	00000468 	.word	0x00000468

00004bf4 <nss_gmac_tx_timeout>:
{
    4bf4:	e92d4070 	push	{r4, r5, r6, lr}
	BUG_ON(gmacdev == NULL);
    4bf8:	e2905d15 	adds	r5, r0, #1344	@ 0x540
    4bfc:	1a000000 	bne	4c04 <nss_gmac_tx_timeout+0x10>
    4c00:	e7f001f2 	.word	0xe7f001f2
    4c04:	e1a04000 	mov	r4, r0
	netif_carrier_off(netdev);
    4c08:	ebfffffe 	bl	0 <netif_carrier_off>
	nss_gmac_disable_dma_tx(gmacdev);
    4c0c:	e1a00005 	mov	r0, r5
    4c10:	ebfffffe 	bl	e08 <nss_gmac_disable_dma_tx>
	nss_gmac_flush_tx_fifo(gmacdev);
    4c14:	e1a00005 	mov	r0, r5
    4c18:	ebfffffe 	bl	6f8 <nss_gmac_flush_tx_fifo>
	nss_gmac_enable_dma_tx(gmacdev);
    4c1c:	e1a00005 	mov	r0, r5
    4c20:	ebfffffe 	bl	d80 <nss_gmac_enable_dma_tx>
	netif_carrier_on(netdev);
    4c24:	e1a00004 	mov	r0, r4
    4c28:	ebfffffe 	bl	0 <netif_carrier_on>
	clear_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
    4c2c:	e5941280 	ldr	r1, [r4, #640]	@ 0x280
    4c30:	e3a00000 	mov	r0, #0
}
    4c34:	e8bd4070 	pop	{r4, r5, r6, lr}
    4c38:	e281104c 	add	r1, r1, #76	@ 0x4c
    4c3c:	eafffffe 	b	0 <_clear_bit>

00004c40 <nss_gmac_change_mtu>:
{
    4c40:	e92d40f7 	push	{r0, r1, r2, r4, r5, r6, r7, lr}
	if (!gmacdev) {
    4c44:	e2907d15 	adds	r7, r0, #1344	@ 0x540
    4c48:	1a000007 	bne	4c6c <nss_gmac_change_mtu+0x2c>
		netdev_info(netdev, "%s: Not a GMAC netdevice (%s)\n", __func__, netdev_name(netdev));
    4c4c:	e59f0148 	ldr	r0, [pc, #328]	@ 4d9c <nss_gmac_change_mtu+0x15c>
    4c50:	ebfffffe 	bl	364 <nss_gmac_select_mii+0x14>
    4c54:	e1a03000 	mov	r3, r0
    4c58:	e59f2140 	ldr	r2, [pc, #320]	@ 4da0 <nss_gmac_change_mtu+0x160>
    4c5c:	e59f1140 	ldr	r1, [pc, #320]	@ 4da4 <nss_gmac_change_mtu+0x164>
    4c60:	e59f0134 	ldr	r0, [pc, #308]	@ 4d9c <nss_gmac_change_mtu+0x15c>
    4c64:	ebfffffe 	bl	0 <netdev_info>
		return -EINVAL;
    4c68:	ea00000b 	b	4c9c <nss_gmac_change_mtu+0x5c>
	if (newmtu > NSS_GMAC_JUMBO_MTU) {
    4c6c:	e3510d96 	cmp	r1, #9600	@ 0x2580
    4c70:	e1a04000 	mov	r4, r0
    4c74:	e1a05001 	mov	r5, r1
    4c78:	da000009 	ble	4ca4 <nss_gmac_change_mtu+0x64>
		netdev_info(netdev, "%s: New MTU of %d, for device %s is larger than %d (NSS_GMAC_JUMBO_MTU)\n",
    4c7c:	ebfffffe 	bl	364 <nss_gmac_select_mii+0x14>
    4c80:	e3a03d96 	mov	r3, #9600	@ 0x2580
    4c84:	e59f2114 	ldr	r2, [pc, #276]	@ 4da0 <nss_gmac_change_mtu+0x160>
    4c88:	e59f1118 	ldr	r1, [pc, #280]	@ 4da8 <nss_gmac_change_mtu+0x168>
    4c8c:	e88d0009 	stm	sp, {r0, r3}
    4c90:	e1a03005 	mov	r3, r5
    4c94:	e1a00004 	mov	r0, r4
    4c98:	ebfffffe 	bl	0 <netdev_info>
		return -EINVAL;
    4c9c:	e3e06015 	mvn	r6, #21
    4ca0:	ea00003a 	b	4d90 <nss_gmac_change_mtu+0x150>
    if (!gmacdev->data_plane_ops->change_mtu || gmacdev->data_plane_ops->change_mtu(gmacdev->data_plane_ctx, newmtu)
    4ca4:	e5903a6c 	ldr	r3, [r0, #2668]	@ 0xa6c
    4ca8:	e5933010 	ldr	r3, [r3, #16]
    4cac:	e3530000 	cmp	r3, #0
    4cb0:	1a000008 	bne	4cd8 <nss_gmac_change_mtu+0x98>
		netdev_info(netdev, "%s: Unable to change MTU for (%s)\n", __func__, netdev_name(netdev));
    4cb4:	e1a00004 	mov	r0, r4
		return -EAGAIN;
    4cb8:	e3e0600a 	mvn	r6, #10
		netdev_info(netdev, "%s: Unable to change MTU for (%s)\n", __func__, netdev_name(netdev));
    4cbc:	ebfffffe 	bl	364 <nss_gmac_select_mii+0x14>
    4cc0:	e1a03000 	mov	r3, r0
    4cc4:	e59f20d4 	ldr	r2, [pc, #212]	@ 4da0 <nss_gmac_change_mtu+0x160>
    4cc8:	e1a00004 	mov	r0, r4
    4ccc:	e59f10d8 	ldr	r1, [pc, #216]	@ 4dac <nss_gmac_change_mtu+0x16c>
    4cd0:	ebfffffe 	bl	0 <netdev_info>
		return -EAGAIN;
    4cd4:	ea00002d 	b	4d90 <nss_gmac_change_mtu+0x150>
    if (!gmacdev->data_plane_ops->change_mtu || gmacdev->data_plane_ops->change_mtu(gmacdev->data_plane_ctx, newmtu)
    4cd8:	e59007cc 	ldr	r0, [r0, #1996]	@ 0x7cc
    4cdc:	e12fff33 	blx	r3
    4ce0:	e2506000 	subs	r6, r0, #0
    4ce4:	1afffff2 	bne	4cb4 <nss_gmac_change_mtu+0x74>
  if (newmtu <= NSS_GMAC_NORMAL_FRAME_MTU) {
    4ce8:	e30035dc 	movw	r3, #1500	@ 0x5dc
    4cec:	e1550003 	cmp	r5, r3
    4cf0:	ca000013 	bgt	4d44 <nss_gmac_change_mtu+0x104>
    netdev_info(netdev, "%s: Enabling Normal Frame MTU (Requested MTU [%d])\n",
    4cf4:	e59f20a4 	ldr	r2, [pc, #164]	@ 4da0 <nss_gmac_change_mtu+0x160>
    4cf8:	e1a03005 	mov	r3, r5
    4cfc:	e1a00004 	mov	r0, r4
    4d00:	e59f10a8 	ldr	r1, [pc, #168]	@ 4db0 <nss_gmac_change_mtu+0x170>
    4d04:	ebfffffe 	bl	0 <netdev_info>
    if (strcmp(netdev_name(netdev), "eth1") == 0) {
    4d08:	e1a00004 	mov	r0, r4
    4d0c:	ebfffffe 	bl	364 <nss_gmac_select_mii+0x14>
    4d10:	e59f109c 	ldr	r1, [pc, #156]	@ 4db4 <nss_gmac_change_mtu+0x174>
    4d14:	ebfffffe 	bl	0 <strcmp>
    4d18:	e3500000 	cmp	r0, #0
    4d1c:	1a000003 	bne	4d30 <nss_gmac_change_mtu+0xf0>
      netdev_info(netdev, "%s: Enabling Jumbo Frame MTU for eth1 (Requested MTU [%d])\n",
    4d20:	e59f2078 	ldr	r2, [pc, #120]	@ 4da0 <nss_gmac_change_mtu+0x160>
    4d24:	e1a03005 	mov	r3, r5
    4d28:	e59f1088 	ldr	r1, [pc, #136]	@ 4db8 <nss_gmac_change_mtu+0x178>
    4d2c:	ea000012 	b	4d7c <nss_gmac_change_mtu+0x13c>
      nss_gmac_jumbo_frame_disable(gmacdev);
    4d30:	e1a00007 	mov	r0, r7
    4d34:	ebfffffe 	bl	2d8 <nss_gmac_jumbo_frame_disable>
      nss_gmac_twokpe_frame_disable(gmacdev);
    4d38:	e1a00007 	mov	r0, r7
    4d3c:	ebfffffe 	bl	300 <nss_gmac_twokpe_frame_disable>
    4d40:	ea000011 	b	4d8c <nss_gmac_change_mtu+0x14c>
  } else if (newmtu <= NSS_GMAC_MINI_JUMBO_FRAME_MTU) {
    4d44:	e30037ba 	movw	r3, #1978	@ 0x7ba
    netdev_info(netdev,
    4d48:	e59f2050 	ldr	r2, [pc, #80]	@ 4da0 <nss_gmac_change_mtu+0x160>
  } else if (newmtu <= NSS_GMAC_MINI_JUMBO_FRAME_MTU) {
    4d4c:	e1550003 	cmp	r5, r3
    netdev_info(netdev,
    4d50:	e1a03005 	mov	r3, r5
  } else if (newmtu <= NSS_GMAC_MINI_JUMBO_FRAME_MTU) {
    4d54:	ca000007 	bgt	4d78 <nss_gmac_change_mtu+0x138>
    netdev_info(netdev,
    4d58:	e59f105c 	ldr	r1, [pc, #92]	@ 4dbc <nss_gmac_change_mtu+0x17c>
    4d5c:	e1a00004 	mov	r0, r4
    4d60:	ebfffffe 	bl	0 <netdev_info>
    nss_gmac_jumbo_frame_disable(gmacdev);
    4d64:	e1a00007 	mov	r0, r7
    4d68:	ebfffffe 	bl	2d8 <nss_gmac_jumbo_frame_disable>
    nss_gmac_twokpe_frame_enable(gmacdev);
    4d6c:	e1a00007 	mov	r0, r7
    4d70:	ebfffffe 	bl	2ec <nss_gmac_twokpe_frame_enable>
    4d74:	ea000004 	b	4d8c <nss_gmac_change_mtu+0x14c>
    netdev_info(netdev, "%s: Enabling Jumbo Frame MTU (Requested MTU [%d])\n", __func__, newmtu);
    4d78:	e59f1040 	ldr	r1, [pc, #64]	@ 4dc0 <nss_gmac_change_mtu+0x180>
    4d7c:	e1a00004 	mov	r0, r4
    4d80:	ebfffffe 	bl	0 <netdev_info>
    nss_gmac_jumbo_frame_enable(gmacdev);
    4d84:	e1a00007 	mov	r0, r7
    4d88:	ebfffffe 	bl	2c4 <nss_gmac_jumbo_frame_enable>
	netdev->mtu = newmtu;
    4d8c:	e5845088 	str	r5, [r4, #136]	@ 0x88
}
    4d90:	e1a00006 	mov	r0, r6
    4d94:	e28dd00c 	add	sp, sp, #12
    4d98:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
    4d9c:	fffffac0 	.word	0xfffffac0
    4da0:	00001037 	.word	0x00001037
    4da4:	00001307 	.word	0x00001307
    4da8:	00001326 	.word	0x00001326
    4dac:	0000136f 	.word	0x0000136f
    4db0:	00001392 	.word	0x00001392
    4db4:	000013c6 	.word	0x000013c6
    4db8:	000013cb 	.word	0x000013cb
    4dbc:	00001407 	.word	0x00001407
    4dc0:	0000143f 	.word	0x0000143f

00004dc4 <nss_gmac_linkup>:
{
    4dc4:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    4dc8:	e1a04000 	mov	r4, r0
	struct net_device *netdev = gmacdev->netdev;
    4dcc:	e5905090 	ldr	r5, [r0, #144]	@ 0x90
	if (nss_gmac_check_phy_init(gmacdev) != 0) {
    4dd0:	ebfffffe 	bl	720 <nss_gmac_check_phy_init>
    4dd4:	e3500000 	cmp	r0, #0
		gmacdev->link_state = LINKDOWN;
    4dd8:	1a00003a 	bne	4ec8 <nss_gmac_linkup+0x104>
	gmacdev->link_state = LINKUP;
    4ddc:	e3a06001 	mov	r6, #1
	if (nss_gmac_dev_set_speed(gmacdev) != 0)
    4de0:	e1a00004 	mov	r0, r4
	gmacdev->link_state = LINKUP;
    4de4:	e5846064 	str	r6, [r4, #100]	@ 0x64
	if (nss_gmac_dev_set_speed(gmacdev) != 0)
    4de8:	ebfffffe 	bl	2f7c <nss_gmac_dev_set_speed>
    4dec:	e3500000 	cmp	r0, #0
    4df0:	18bd81f0 	popne	{r4, r5, r6, r7, r8, pc}
	if (gmacdev->first_linkup_done == 0) {
    4df4:	e5947080 	ldr	r7, [r4, #128]	@ 0x80
    4df8:	e3570000 	cmp	r7, #0
    4dfc:	1a000022 	bne	4e8c <nss_gmac_linkup+0xc8>
		nss_gmac_reset(gmacdev);
    4e00:	e1a00004 	mov	r0, r4
    4e04:	ebfffffe 	bl	0 <nss_gmac_check_pcs_status>
	nss_gmac_write_reg(gmacdev->dma_base, dma_interrupt, dma_int_disable);
    4e08:	e5943004 	ldr	r3, [r4, #4]
    4e0c:	e583701c 	str	r7, [r3, #28]
	addr = (uint32_t)regbase + regoffset;
    4e10:	e5942000 	ldr	r2, [r4]
    4e14:	e3e01000 	mvn	r1, #0
    4e18:	e582103c 	str	r1, [r2, #60]	@ 0x3c
	asm volatile("ldr %0, %1"
    4e1c:	e5932014 	ldr	r2, [r3, #20]
	asm volatile("str %1, %0"
    4e20:	e5832014 	str	r2, [r3, #20]
		nss_gmac_init_tx_desc_base(gmacdev);
    4e24:	e1a00004 	mov	r0, r4
    4e28:	ebfffffe 	bl	c5c <nss_gmac_init_tx_desc_base>
		nss_gmac_init_rx_desc_base(gmacdev);
    4e2c:	e1a00004 	mov	r0, r4
    4e30:	ebfffffe 	bl	c4c <nss_gmac_init_rx_desc_base>
		nss_gmac_dma_bus_mode_init(gmacdev, dma_bus_mode_val);
    4e34:	e59f10ec 	ldr	r1, [pc, #236]	@ 4f28 <nss_gmac_linkup+0x164>
    4e38:	e1a00004 	mov	r0, r4
    4e3c:	ebfffffe 	bl	258 <nss_gmac_dma_bus_mode_init>
		nss_gmac_dma_axi_bus_mode_init(gmacdev, dma_axi_bus_mode_val);
    4e40:	e59f10e4 	ldr	r1, [pc, #228]	@ 4f2c <nss_gmac_linkup+0x168>
    4e44:	e1a00004 	mov	r0, r4
    4e48:	ebfffffe 	bl	268 <nss_gmac_dma_axi_bus_mode_init>
		nss_gmac_dma_control_init(gmacdev, dma_omr);
    4e4c:	e59f10dc 	ldr	r1, [pc, #220]	@ 4f30 <nss_gmac_linkup+0x16c>
    4e50:	e1a00004 	mov	r0, r4
    4e54:	ebfffffe 	bl	278 <nss_gmac_dma_control_init>
		nss_gmac_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF);
    4e58:	e3e01000 	mvn	r1, #0
    4e5c:	e1a00004 	mov	r0, r4
    4e60:	ebfffffe 	bl	e30 <nss_gmac_disable_mmc_tx_interrupt>
		nss_gmac_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
    4e64:	e3e01000 	mvn	r1, #0
    4e68:	e1a00004 	mov	r0, r4
    4e6c:	ebfffffe 	bl	e44 <nss_gmac_disable_mmc_rx_interrupt>
		nss_gmac_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);
    4e70:	e3e01000 	mvn	r1, #0
    4e74:	e1a00004 	mov	r0, r4
    4e78:	ebfffffe 	bl	e58 <nss_gmac_disable_mmc_ipc_rx_interrupt>
		nss_gmac_change_mtu(gmacdev->netdev, gmacdev->netdev->mtu);
    4e7c:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
    4e80:	e5901088 	ldr	r1, [r0, #136]	@ 0x88
    4e84:	ebfffffe 	bl	4c40 <nss_gmac_change_mtu>
		gmacdev->first_linkup_done = 1;
    4e88:	e5846080 	str	r6, [r4, #128]	@ 0x80
	nss_gmac_mac_init(gmacdev);
    4e8c:	e1a00004 	mov	r0, r4
    4e90:	ebfffffe 	bl	ef4 <nss_gmac_mac_init>
	if (gmacdev->data_plane_ops->open(gmacdev->data_plane_ctx, gmac_tx_desc,
    4e94:	e594028c 	ldr	r0, [r4, #652]	@ 0x28c
    4e98:	e594352c 	ldr	r3, [r4, #1324]	@ 0x52c
    4e9c:	e5936000 	ldr	r6, [r3]
    4ea0:	e3a03000 	mov	r3, #0
    4ea4:	e1a02003 	mov	r2, r3
    4ea8:	e1a01003 	mov	r1, r3
    4eac:	e12fff36 	blx	r6
    4eb0:	e3500000 	cmp	r0, #0
		netdev_info(netdev, "%s: data plane open command un-successful\n",
    4eb4:	e59f2078 	ldr	r2, [pc, #120]	@ 4f34 <nss_gmac_linkup+0x170>
	if (gmacdev->data_plane_ops->open(gmacdev->data_plane_ctx, gmac_tx_desc,
    4eb8:	0a000005 	beq	4ed4 <nss_gmac_linkup+0x110>
		netdev_info(netdev, "%s: data plane open command un-successful\n",
    4ebc:	e59f1074 	ldr	r1, [pc, #116]	@ 4f38 <nss_gmac_linkup+0x174>
    4ec0:	e1a00005 	mov	r0, r5
    4ec4:	ebfffffe 	bl	0 <netdev_info>
		gmacdev->link_state = LINKDOWN;
    4ec8:	e3a03000 	mov	r3, #0
    4ecc:	e5843064 	str	r3, [r4, #100]	@ 0x64
}
    4ed0:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
	netdev_info(netdev, "%s: data plane open command successfully issued\n",
    4ed4:	e59f1060 	ldr	r1, [pc, #96]	@ 4f3c <nss_gmac_linkup+0x178>
    4ed8:	e1a00005 	mov	r0, r5
    4edc:	ebfffffe 	bl	0 <netdev_info>
    4ee0:	e5943018 	ldr	r3, [r4, #24]
	if (!test_bit(__NSS_GMAC_UP, &gmacdev->flags))
    4ee4:	e3130001 	tst	r3, #1
    4ee8:	0a00000b 	beq	4f1c <nss_gmac_linkup+0x158>
	if (gmacdev->speed == SPEED_1000)
    4eec:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
    4ef0:	e3530ffa 	cmp	r3, #1000	@ 0x3e8
		link |= 0x4;
    4ef4:	03a01005 	moveq	r1, #5
	if (gmacdev->speed == SPEED_1000)
    4ef8:	0a000003 	beq	4f0c <nss_gmac_linkup+0x148>
	link = 0x1;
    4efc:	e3530064 	cmp	r3, #100	@ 0x64
    4f00:	03a01003 	moveq	r1, #3
    4f04:	13a01001 	movne	r1, #1
    4f08:	eaffffff 	b	4f0c <nss_gmac_linkup+0x148>
	gmacdev->data_plane_ops->link_state(gmacdev->data_plane_ctx, link);
    4f0c:	e594028c 	ldr	r0, [r4, #652]	@ 0x28c
    4f10:	e594352c 	ldr	r3, [r4, #1324]	@ 0x52c
    4f14:	e5933008 	ldr	r3, [r3, #8]
    4f18:	e12fff33 	blx	r3
	netif_carrier_on(netdev);
    4f1c:	e1a00005 	mov	r0, r5
}
    4f20:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
	netif_carrier_on(netdev);
    4f24:	eafffffe 	b	0 <netif_carrier_on>
    4f28:	03002082 	.word	0x03002082
    4f2c:	00770008 	.word	0x00770008
    4f30:	0220001c 	.word	0x0220001c
    4f34:	0000104b 	.word	0x0000104b
    4f38:	00001472 	.word	0x00001472
    4f3c:	0000149d 	.word	0x0000149d

00004f40 <nss_gmac_adjust_link>:
    4f40:	e5903558 	ldr	r3, [r0, #1368]	@ 0x558
	if (!test_bit(__NSS_GMAC_UP, &gmacdev->flags))
    4f44:	e3130001 	tst	r3, #1
    4f48:	012fff1e 	bxeq	lr
{
    4f4c:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
    4f50:	e1a04000 	mov	r4, r0
	status = nss_gmac_check_link(gmacdev);
    4f54:	e2800d15 	add	r0, r0, #1344	@ 0x540
    4f58:	ebfffffe 	bl	98 <nss_gmac_check_link>
    4f5c:	e1a07000 	mov	r7, r0
	mutex_lock(&gmacdev->link_mutex);
    4f60:	e2840e7b 	add	r0, r4, #1968	@ 0x7b0
    4f64:	ebfffffe 	bl	0 <mutex_lock>
	if (status == LINKUP && gmacdev->link_state == LINKDOWN)
    4f68:	e3570001 	cmp	r7, #1
    4f6c:	1a000005 	bne	4f88 <nss_gmac_adjust_link+0x48>
    4f70:	e59435a4 	ldr	r3, [r4, #1444]	@ 0x5a4
    4f74:	e3530000 	cmp	r3, #0
    4f78:	1a00000c 	bne	4fb0 <nss_gmac_adjust_link+0x70>
		nss_gmac_linkup(gmacdev);
    4f7c:	e2840d15 	add	r0, r4, #1344	@ 0x540
    4f80:	ebfffffe 	bl	4dc4 <nss_gmac_linkup>
    4f84:	ea000009 	b	4fb0 <nss_gmac_adjust_link+0x70>
	else if (status == LINKDOWN && gmacdev->link_state == LINKUP) {
    4f88:	e3570000 	cmp	r7, #0
    4f8c:	1a000007 	bne	4fb0 <nss_gmac_adjust_link+0x70>
    4f90:	e59435a4 	ldr	r3, [r4, #1444]	@ 0x5a4
    4f94:	e3530001 	cmp	r3, #1
    4f98:	1a000004 	bne	4fb0 <nss_gmac_adjust_link+0x70>
		if (gmacdev->drv_flags & NSS_GMAC_PRIV_FLAG(LINKPOLL)) {
    4f9c:	e594355c 	ldr	r3, [r4, #1372]	@ 0x55c
    4fa0:	e3130001 	tst	r3, #1
    4fa4:	0a000001 	beq	4fb0 <nss_gmac_adjust_link+0x70>
			nss_gmac_linkdown(gmacdev);
    4fa8:	e2840d15 	add	r0, r4, #1344	@ 0x540
    4fac:	ebfffffe 	bl	3a4 <nss_gmac_loopback_off>
	mutex_unlock(&gmacdev->link_mutex);
    4fb0:	e2840e7b 	add	r0, r4, #1968	@ 0x7b0
}
    4fb4:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
	mutex_unlock(&gmacdev->link_mutex);
    4fb8:	eafffffe 	b	0 <mutex_unlock>

00004fbc <nss_gmac_open>:
{
    4fbc:	e92d41ff 	push	{r0, r1, r2, r3, r4, r5, r6, r7, r8, lr}
	if (!gmacdev)
    4fc0:	e2906d15 	adds	r6, r0, #1344	@ 0x540
		return -EINVAL;
    4fc4:	03e00015 	mvneq	r0, #21
	if (!gmacdev)
    4fc8:	0a000092 	beq	5218 <nss_gmac_open+0x25c>
    4fcc:	e1a04000 	mov	r4, r0
	netif_carrier_off(netdev);
    4fd0:	ebfffffe 	bl	0 <netif_carrier_off>
    4fd4:	e5943544 	ldr	r3, [r4, #1348]	@ 0x544
    4fd8:	e3a02000 	mov	r2, #0
    4fdc:	e583201c 	str	r2, [r3, #28]
    4fe0:	e5943540 	ldr	r3, [r4, #1344]	@ 0x540
    4fe4:	e3e02000 	mvn	r2, #0
    4fe8:	e583203c 	str	r2, [r3, #60]	@ 0x3c
	if (!gmacdev->data_plane_ops) {
    4fec:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    4ff0:	e3530000 	cmp	r3, #0
    4ff4:	1a00006a 	bne	51a4 <nss_gmac_open+0x1e8>
		netdev_info(netdev, "%s: offload is not enabled, bring up gmac with slowpath\n",
    4ff8:	e59f2220 	ldr	r2, [pc, #544]	@ 5220 <nss_gmac_open+0x264>
    4ffc:	e1a00004 	mov	r0, r4
	dev = &netdev->dev;
    5000:	e2847d0d 	add	r7, r4, #832	@ 0x340
		netdev_info(netdev, "%s: offload is not enabled, bring up gmac with slowpath\n",
    5004:	e59f1218 	ldr	r1, [pc, #536]	@ 5224 <nss_gmac_open+0x268>
    5008:	ebfffffe 	bl	0 <netdev_info>
	netif_napi_add_weight(dev, napi, poll, weight);
    500c:	e3a03040 	mov	r3, #64	@ 0x40
    5010:	e59f2210 	ldr	r2, [pc, #528]	@ 5228 <nss_gmac_open+0x26c>
    5014:	e28610c8 	add	r1, r6, #200	@ 0xc8
    5018:	e1a00004 	mov	r0, r4
    501c:	ebfffffe 	bl	0 <netif_napi_add_weight>

static __always_inline int
atomic_read(const atomic_t *v)
{
	instrument_atomic_read(v, sizeof(*v));
	return arch_atomic_read(v);
    5020:	e59f3204 	ldr	r3, [pc, #516]	@ 522c <nss_gmac_open+0x270>
    5024:	e5933000 	ldr	r3, [r3]
	if (num_online_cpus() > 1) {
    5028:	e3530001 	cmp	r3, #1
    502c:	9a000002 	bls	503c <nss_gmac_open+0x80>
		dev_set_threaded(dev, true);
    5030:	e3a01001 	mov	r1, #1
    5034:	e1a00004 	mov	r0, r4
    5038:	ebfffffe 	bl	0 <dev_set_threaded>
	gmacdev->rx_desc_count = 0;
    503c:	e3a05000 	mov	r5, #0
		dma_set_coherent_mask(dev, 0xffffffff);
    5040:	e3a03000 	mov	r3, #0
    5044:	e3e02000 	mvn	r2, #0
    5048:	e1a00007 	mov	r0, r7
    504c:	ebfffffe 	bl	0 <dma_set_coherent_mask>
	netdev_info(gmacdev->netdev, "total size of memory required for Rx Descriptors in Ring Mode = 0x%08x\n"
    5050:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
    5054:	e3a02a01 	mov	r2, #4096	@ 0x1000
	gmacdev->rx_desc_count = 0;
    5058:	e5845578 	str	r5, [r4, #1400]	@ 0x578
	netdev_info(gmacdev->netdev, "total size of memory required for Rx Descriptors in Ring Mode = 0x%08x\n"
    505c:	e59f11cc 	ldr	r1, [pc, #460]	@ 5230 <nss_gmac_open+0x274>
    5060:	ebfffffe 	bl	0 <netdev_info>
	return dma_alloc_attrs(dev, size, dma_handle, gfp,
    5064:	e28d200c 	add	r2, sp, #12
    5068:	e3a03d33 	mov	r3, #3264	@ 0xcc0
    506c:	e58d5000 	str	r5, [sp]
    5070:	e3a01a01 	mov	r1, #4096	@ 0x1000
    5074:	e1a00007 	mov	r0, r7
    5078:	ebfffffe 	bl	0 <dma_alloc_attrs>
	if (first_desc == NULL) {
    507c:	e2503000 	subs	r3, r0, #0
		netdev_info(gmacdev->netdev, "Error in Rx Descriptor Memory allocation in Ring mode\n");
    5080:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
	if (first_desc == NULL) {
    5084:	1a000002 	bne	5094 <nss_gmac_open+0xd8>
		netdev_info(gmacdev->netdev, "Error in Rx Descriptor Memory allocation in Ring mode\n");
    5088:	e59f11a4 	ldr	r1, [pc, #420]	@ 5234 <nss_gmac_open+0x278>
    508c:	ebfffffe 	bl	0 <netdev_info>
		return -ENOMEM;
    5090:	ea000014 	b	50e8 <nss_gmac_open+0x12c>
	gmacdev->rx_desc_count = no_of_desc;
    5094:	e3a02080 	mov	r2, #128	@ 0x80
	gmacdev->rx_desc = first_desc;
    5098:	e584356c 	str	r3, [r4, #1388]	@ 0x56c
	gmacdev->rx_desc_count = no_of_desc;
    509c:	e5842578 	str	r2, [r4, #1400]	@ 0x578
	gmacdev->rx_desc_dma = dma_addr;
    50a0:	e59d100c 	ldr	r1, [sp, #12]
    50a4:	e5841564 	str	r1, [r4, #1380]	@ 0x564
	netdev_info(gmacdev->netdev, "Rx Descriptors in Ring Mode: No. of descriptors = %d base = 0x%08x dma = 0x%08x\n",
    50a8:	e58d1000 	str	r1, [sp]
    50ac:	e59f1184 	ldr	r1, [pc, #388]	@ 5238 <nss_gmac_open+0x27c>
    50b0:	ebfffffe 	bl	0 <netdev_info>
	nss_gmac_rx_desc_init_ring(gmacdev->rx_desc, no_of_desc);
    50b4:	e594856c 	ldr	r8, [r4, #1388]	@ 0x56c
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    50b8:	e3a02a01 	mov	r2, #4096	@ 0x1000
    50bc:	e1a01005 	mov	r1, r5
    50c0:	e1a00008 	mov	r0, r8
    50c4:	ebfffffe 	bl	0 <memset>
	last_desc->length = rx_desc_end_of_ring;
    50c8:	e3a03902 	mov	r3, #32768	@ 0x8000
    50cc:	e5883fe4 	str	r3, [r8, #4068]	@ 0xfe4
	gmacdev->rx_next_desc = gmacdev->rx_desc;
    50d0:	e594356c 	ldr	r3, [r4, #1388]	@ 0x56c
	gmacdev->busy_rx_desc = 0;
    50d4:	e5845574 	str	r5, [r4, #1396]	@ 0x574
	gmacdev->rx_busy = 0;
    50d8:	e5845588 	str	r5, [r4, #1416]	@ 0x588
	gmacdev->rx_next = 0;
    50dc:	e584558c 	str	r5, [r4, #1420]	@ 0x58c
	gmacdev->rx_busy_desc = gmacdev->rx_desc;
    50e0:	e5843598 	str	r3, [r4, #1432]	@ 0x598
	gmacdev->rx_next_desc = gmacdev->rx_desc;
    50e4:	e584359c 	str	r3, [r4, #1436]	@ 0x59c
	gmacdev->tx_desc_count = 0;
    50e8:	e3a05000 	mov	r5, #0
	netdev_info(gmacdev->netdev, "Total size of memory required for Tx Descriptors in Ring Mode = 0x%08x\n"
    50ec:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
    50f0:	e3a02a01 	mov	r2, #4096	@ 0x1000
    50f4:	e59f1140 	ldr	r1, [pc, #320]	@ 523c <nss_gmac_open+0x280>
	gmacdev->tx_desc_count = 0;
    50f8:	e584557c 	str	r5, [r4, #1404]	@ 0x57c
	netdev_info(gmacdev->netdev, "Total size of memory required for Tx Descriptors in Ring Mode = 0x%08x\n"
    50fc:	ebfffffe 	bl	0 <netdev_info>
    5100:	e28d200c 	add	r2, sp, #12
    5104:	e3a03d33 	mov	r3, #3264	@ 0xcc0
    5108:	e58d5000 	str	r5, [sp]
    510c:	e3a01a01 	mov	r1, #4096	@ 0x1000
    5110:	e1a00007 	mov	r0, r7
    5114:	ebfffffe 	bl	0 <dma_alloc_attrs>
	if (first_desc == NULL) {
    5118:	e2503000 	subs	r3, r0, #0
		netdev_info(gmacdev->netdev,
    511c:	e59405d0 	ldr	r0, [r4, #1488]	@ 0x5d0
	if (first_desc == NULL) {
    5120:	1a000002 	bne	5130 <nss_gmac_open+0x174>
		netdev_info(gmacdev->netdev,
    5124:	e59f1114 	ldr	r1, [pc, #276]	@ 5240 <nss_gmac_open+0x284>
    5128:	ebfffffe 	bl	0 <netdev_info>
		return -ENOMEM;
    512c:	ea000016 	b	518c <nss_gmac_open+0x1d0>
	gmacdev->tx_desc_count = no_of_desc;
    5130:	e3a02080 	mov	r2, #128	@ 0x80
	gmacdev->tx_desc = first_desc;
    5134:	e5843568 	str	r3, [r4, #1384]	@ 0x568
	gmacdev->tx_desc_count = no_of_desc;
    5138:	e584257c 	str	r2, [r4, #1404]	@ 0x57c
	gmacdev->tx_desc_dma = dma_addr;
    513c:	e59d100c 	ldr	r1, [sp, #12]
    5140:	e5841560 	str	r1, [r4, #1376]	@ 0x560
	netdev_info(gmacdev->netdev, "Tx Descriptors in Ring Mode: No. of descriptors = %d base = 0x%08x dma = 0x%08x\n"
    5144:	e58d1000 	str	r1, [sp]
    5148:	e59f10f4 	ldr	r1, [pc, #244]	@ 5244 <nss_gmac_open+0x288>
    514c:	ebfffffe 	bl	0 <netdev_info>
	nss_gmac_tx_desc_init_ring(gmacdev->tx_desc, gmacdev->tx_desc_count);
    5150:	e5947568 	ldr	r7, [r4, #1384]	@ 0x568
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    5154:	e1a01005 	mov	r1, r5
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    5158:	e594257c 	ldr	r2, [r4, #1404]	@ 0x57c
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    515c:	e1a00007 	mov	r0, r7
	struct dma_desc *last_desc = desc + no_of_desc - 1;
    5160:	e242837e 	sub	r8, r2, #-134217727	@ 0xf8000001
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc));
    5164:	e1a02282 	lsl	r2, r2, #5
    5168:	ebfffffe 	bl	0 <memset>
	last_desc->status = tx_desc_end_of_ring;
    516c:	e3a03602 	mov	r3, #2097152	@ 0x200000
    5170:	e7873288 	str	r3, [r7, r8, lsl #5]
	gmacdev->tx_next_desc = gmacdev->tx_desc;
    5174:	e5943568 	ldr	r3, [r4, #1384]	@ 0x568
	gmacdev->busy_tx_desc = 0;
    5178:	e5845570 	str	r5, [r4, #1392]	@ 0x570
	gmacdev->tx_busy = 0;
    517c:	e5845580 	str	r5, [r4, #1408]	@ 0x580
	gmacdev->tx_next = 0;
    5180:	e5845584 	str	r5, [r4, #1412]	@ 0x584
	gmacdev->tx_busy_desc = gmacdev->tx_desc;
    5184:	e5843590 	str	r3, [r4, #1424]	@ 0x590
	gmacdev->tx_next_desc = gmacdev->tx_desc;
    5188:	e5843594 	str	r3, [r4, #1428]	@ 0x594
		nss_gmac_rx_refill(gmacdev);
    518c:	e1a00006 	mov	r0, r6
    5190:	ebfffbab 	bl	4044 <nss_gmac_rx_refill>
		gmacdev->data_plane_ops = &nss_gmac_slowpath_ops;
    5194:	e59f30ac 	ldr	r3, [pc, #172]	@ 5248 <nss_gmac_open+0x28c>
    5198:	e5843a6c 	str	r3, [r4, #2668]	@ 0xa6c
		gmacdev->data_plane_ctx = gmacdev->netdev;
    519c:	e59435d0 	ldr	r3, [r4, #1488]	@ 0x5d0
    51a0:	e58437cc 	str	r3, [r4, #1996]	@ 0x7cc
	gmacdev->speed = SPEED_100;
    51a4:	e3a03064 	mov	r3, #100	@ 0x64
	nss_gmac_read_version(gmacdev);
    51a8:	e1a00006 	mov	r0, r6
	gmacdev->link_state = LINKDOWN;
    51ac:	e3a05000 	mov	r5, #0
	gmacdev->speed = SPEED_100;
    51b0:	e58435ac 	str	r3, [r4, #1452]	@ 0x5ac
	gmacdev->duplex_mode = DUPLEX_FULL;
    51b4:	e3a03001 	mov	r3, #1
    51b8:	e58435a8 	str	r3, [r4, #1448]	@ 0x5a8
	nss_gmac_read_version(gmacdev);
    51bc:	ebfffffe 	bl	244 <nss_gmac_read_version>
	gmacdev->data_plane_ops->set_features(netdev);
    51c0:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    51c4:	e1a00004 	mov	r0, r4
    51c8:	e5933018 	ldr	r3, [r3, #24]
    51cc:	e12fff33 	blx	r3
	set_bit(__NSS_GMAC_UP, &gmacdev->flags);
    51d0:	e2861018 	add	r1, r6, #24
    51d4:	e3a00000 	mov	r0, #0
    51d8:	ebfffffe 	bl	0 <_set_bit>
	clear_bit(__QUEUE_STATE_DRV_XOFF, &dev_queue->state);
    51dc:	e5941280 	ldr	r1, [r4, #640]	@ 0x280
    51e0:	e3a00000 	mov	r0, #0
    51e4:	e281104c 	add	r1, r1, #76	@ 0x4c
    51e8:	ebfffffe 	bl	0 <_clear_bit>
	nss_gmac_start_up(gmacdev);
    51ec:	e1a00006 	mov	r0, r6
	gmacdev->link_state = LINKDOWN;
    51f0:	e58455a4 	str	r5, [r4, #1444]	@ 0x5a4
	nss_gmac_start_up(gmacdev);
    51f4:	ebfffffe 	bl	3fc <nss_gmac_retry_disable+0x8>
					(uint8_t *)gmacdev->netdev->dev_addr);
    51f8:	e59425d0 	ldr	r2, [r4, #1488]	@ 0x5d0
	gmacdev->data_plane_ops->mac_addr(gmacdev->data_plane_ctx,
    51fc:	e59407cc 	ldr	r0, [r4, #1996]	@ 0x7cc
    5200:	e5943a6c 	ldr	r3, [r4, #2668]	@ 0xa6c
    5204:	e59211f4 	ldr	r1, [r2, #500]	@ 0x1f4
    5208:	e593300c 	ldr	r3, [r3, #12]
    520c:	e12fff33 	blx	r3
	return 0;
    5210:	e1a00005 	mov	r0, r5
    5214:	eaffffff 	b	5218 <nss_gmac_open+0x25c>
}
    5218:	e28dd010 	add	sp, sp, #16
    521c:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    5220:	0000106d 	.word	0x0000106d
    5224:	00001529 	.word	0x00001529
	...
    5230:	00001562 	.word	0x00001562
    5234:	000015aa 	.word	0x000015aa
    5238:	000015e1 	.word	0x000015e1
    523c:	00001632 	.word	0x00001632
    5240:	0000167a 	.word	0x0000167a
    5244:	000016a5 	.word	0x000016a5
    5248:	00000468 	.word	0x00000468

0000524c <nss_gmac_mdiobus_write>:
 * @param[in] Value to write
 * @return 0 on Success
 */
static int32_t nss_gmac_mdiobus_write(struct mii_bus *bus, int32_t phy_id,
						int32_t regnum, uint16_t val)
{
    524c:	e92d4007 	push	{r0, r1, r2, lr}
	struct nss_gmac_dev *gmacdev;

	gmacdev = (struct nss_gmac_dev *)bus->priv;
    5250:	e5900048 	ldr	r0, [r0, #72]	@ 0x48

	nss_gmac_write_phy_reg((uint32_t *)gmacdev->mac_base, phy_id,
    5254:	e590c060 	ldr	ip, [r0, #96]	@ 0x60
    5258:	e58dc000 	str	ip, [sp]
    525c:	e5900000 	ldr	r0, [r0]
    5260:	ebfffffe 	bl	11c <nss_gmac_write_phy_reg>
				regnum, val, gmacdev->mdc_clk_div);

	return 0;
}
    5264:	e3a00000 	mov	r0, #0
    5268:	e28dd00c 	add	sp, sp, #12
    526c:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

00005270 <nss_gmac_mdiobus_read>:
{
    5270:	e92d401f 	push	{r0, r1, r2, r3, r4, lr}
	gmacdev = (struct nss_gmac_dev *)bus->priv;
    5274:	e5900048 	ldr	r0, [r0, #72]	@ 0x48
	status = nss_gmac_read_phy_reg((uint32_t *)gmacdev->mac_base,
    5278:	e5903060 	ldr	r3, [r0, #96]	@ 0x60
    527c:	e58d3000 	str	r3, [sp]
    5280:	e5900000 	ldr	r0, [r0]
    5284:	e28d300e 	add	r3, sp, #14
    5288:	ebfffffe 	bl	b4 <nss_gmac_read_phy_reg>
	if (status != 0)
    528c:	e3500000 	cmp	r0, #0
		data = 0;
    5290:	13a03000 	movne	r3, #0
    5294:	11cd30be 	strhne	r3, [sp, #14]
}
    5298:	e1dd00be 	ldrh	r0, [sp, #14]
    529c:	e28dd014 	add	sp, sp, #20
    52a0:	e49df004 	pop	{pc}		@ (ldr pc, [sp], #4)

000052a4 <nss_gmac_init_mdiobus>:
 * @brief Initialize and register MDIO bus
 * @param[in] pointer to nss_gmac_dev
 * @return 0 on Success
 */
int32_t nss_gmac_init_mdiobus(struct nss_gmac_dev *gmacdev)
{
    52a4:	e92d4070 	push	{r4, r5, r6, lr}
    52a8:	e1a05000 	mov	r5, r0
	return mdiobus_alloc_size(0);
    52ac:	e3a00000 	mov	r0, #0
    52b0:	ebfffffe 	bl	0 <mdiobus_alloc_size>
	struct mii_bus *miibus = NULL;
	struct phy_device *phydev = NULL;

	miibus = mdiobus_alloc();
	if (miibus == NULL)
    52b4:	e2504000 	subs	r4, r0, #0
		return -ENOMEM;
    52b8:	03e0600b 	mvneq	r6, #11
	if (miibus == NULL)
    52bc:	0a000038 	beq	53a4 <nss_gmac_init_mdiobus+0x100>

	miibus->name = "nss gmac mdio bus";
    52c0:	e59f30e4 	ldr	r3, [pc, #228]	@ 53ac <nss_gmac_init_mdiobus+0x108>
	snprintf(miibus->id, MII_BUS_ID_SIZE, "mdiobus%x", gmacdev->macid);
    52c4:	e3a0103d 	mov	r1, #61	@ 0x3d
    52c8:	e2840008 	add	r0, r4, #8
    52cc:	e59f20dc 	ldr	r2, [pc, #220]	@ 53b0 <nss_gmac_init_mdiobus+0x10c>
	miibus->name = "nss gmac mdio bus";
    52d0:	e5843004 	str	r3, [r4, #4]
	snprintf(miibus->id, MII_BUS_ID_SIZE, "mdiobus%x", gmacdev->macid);
    52d4:	e595300c 	ldr	r3, [r5, #12]
    52d8:	ebfffffe 	bl	0 <snprintf>

	miibus->priv = (void *)gmacdev;
	miibus->read = nss_gmac_mdiobus_read;
    52dc:	e59f30d0 	ldr	r3, [pc, #208]	@ 53b4 <nss_gmac_init_mdiobus+0x110>
	miibus->write = nss_gmac_mdiobus_write;
	miibus->reset = nss_gmac_mdiobus_reset;
	mutex_init(&(miibus->mdio_lock));
    52e0:	e2840e55 	add	r0, r4, #1360	@ 0x550
	miibus->priv = (void *)gmacdev;
    52e4:	e5845048 	str	r5, [r4, #72]	@ 0x48
	mutex_init(&(miibus->mdio_lock));
    52e8:	e59f20c8 	ldr	r2, [pc, #200]	@ 53b8 <nss_gmac_init_mdiobus+0x114>
    52ec:	e2800008 	add	r0, r0, #8
    52f0:	e59f10c4 	ldr	r1, [pc, #196]	@ 53bc <nss_gmac_init_mdiobus+0x118>
	miibus->read = nss_gmac_mdiobus_read;
    52f4:	e584304c 	str	r3, [r4, #76]	@ 0x4c
	miibus->write = nss_gmac_mdiobus_write;
    52f8:	e59f30c0 	ldr	r3, [pc, #192]	@ 53c0 <nss_gmac_init_mdiobus+0x11c>
    52fc:	e5843050 	str	r3, [r4, #80]	@ 0x50
	miibus->reset = nss_gmac_mdiobus_reset;
    5300:	e59f30bc 	ldr	r3, [pc, #188]	@ 53c4 <nss_gmac_init_mdiobus+0x120>
    5304:	e5843054 	str	r3, [r4, #84]	@ 0x54
	mutex_init(&(miibus->mdio_lock));
    5308:	ebfffffe 	bl	0 <__mutex_init>
	miibus->parent = &(gmacdev->pdev->dev);
    530c:	e5953094 	ldr	r3, [r5, #148]	@ 0x94

	phy_irq[gmacdev->phy_base] = PHY_POLL;
	miibus->phy_mask = ~((uint32_t)(1 << gmacdev->phy_base));

	if (mdiobus_register(miibus) != 0) {
    5310:	e1a00004 	mov	r0, r4
    5314:	e59f10ac 	ldr	r1, [pc, #172]	@ 53c8 <nss_gmac_init_mdiobus+0x124>
	miibus->parent = &(gmacdev->pdev->dev);
    5318:	e2833010 	add	r3, r3, #16
    531c:	e584356c 	str	r3, [r4, #1388]	@ 0x56c
	miibus->phy_mask = ~((uint32_t)(1 << gmacdev->phy_base));
    5320:	e5952008 	ldr	r2, [r5, #8]
    5324:	e3a03001 	mov	r3, #1
    5328:	e1e03213 	mvn	r3, r3, lsl r2
    532c:	e5843710 	str	r3, [r4, #1808]	@ 0x710
	if (mdiobus_register(miibus) != 0) {
    5330:	ebfffffe 	bl	0 <__mdiobus_register>
    5334:	e2506000 	subs	r6, r0, #0
    5338:	0a000007 	beq	535c <nss_gmac_init_mdiobus+0xb8>
		mdiobus_free(miibus);
    533c:	e1a00004 	mov	r0, r4
		netdev_info(gmacdev->netdev, "%s: mdiobus_reg failed\n", __func__);
		return -EIO;
    5340:	e3e06004 	mvn	r6, #4
		mdiobus_free(miibus);
    5344:	ebfffffe 	bl	0 <mdiobus_free>
		netdev_info(gmacdev->netdev, "%s: mdiobus_reg failed\n", __func__);
    5348:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
    534c:	e59f2078 	ldr	r2, [pc, #120]	@ 53cc <nss_gmac_init_mdiobus+0x128>
    5350:	e59f1078 	ldr	r1, [pc, #120]	@ 53d0 <nss_gmac_init_mdiobus+0x12c>
    5354:	ebfffffe 	bl	0 <netdev_info>
		return -EIO;
    5358:	ea000011 	b	53a4 <nss_gmac_init_mdiobus+0x100>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	miibus->irq = phy_irq;
	phydev = miibus->phy_map[gmacdev->phy_base];
#else
	phydev = mdiobus_get_phy(miibus, gmacdev->phy_base);
    535c:	e5951008 	ldr	r1, [r5, #8]
    5360:	e1a00004 	mov	r0, r4
    5364:	ebfffffe 	bl	0 <mdiobus_get_phy>
#endif
	if (!phydev) {
    5368:	e3500000 	cmp	r0, #0
		mdiobus_unregister(miibus);
		mdiobus_free(miibus);
		return -ENODEV;
	}

	phydev->interface = gmacdev->phy_mii_type;
    536c:	15953074 	ldrne	r3, [r5, #116]	@ 0x74
    5370:	15803200 	strne	r3, [r0, #512]	@ 0x200

	gmacdev->miibus = miibus;
    5374:	15854528 	strne	r4, [r5, #1320]	@ 0x528
	if (!phydev) {
    5378:	1a000009 	bne	53a4 <nss_gmac_init_mdiobus+0x100>
		netdev_info(gmacdev->netdev, "%s: No phy device\n", __func__);
    537c:	e5950090 	ldr	r0, [r5, #144]	@ 0x90
		return -ENODEV;
    5380:	e3e06012 	mvn	r6, #18
		netdev_info(gmacdev->netdev, "%s: No phy device\n", __func__);
    5384:	e59f2040 	ldr	r2, [pc, #64]	@ 53cc <nss_gmac_init_mdiobus+0x128>
    5388:	e59f1044 	ldr	r1, [pc, #68]	@ 53d4 <nss_gmac_init_mdiobus+0x130>
    538c:	ebfffffe 	bl	0 <netdev_info>
		mdiobus_unregister(miibus);
    5390:	e1a00004 	mov	r0, r4
    5394:	ebfffffe 	bl	0 <mdiobus_unregister>
		mdiobus_free(miibus);
    5398:	e1a00004 	mov	r0, r4
    539c:	ebfffffe 	bl	0 <mdiobus_free>
		return -ENODEV;
    53a0:	eaffffff 	b	53a4 <nss_gmac_init_mdiobus+0x100>
	return 0;
}
    53a4:	e1a00006 	mov	r0, r6
    53a8:	e8bd8070 	pop	{r4, r5, r6, pc}
    53ac:	00001743 	.word	0x00001743
    53b0:	00001755 	.word	0x00001755
    53b4:	00000000 	.word	0x00000000
    53b8:	00000040 	.word	0x00000040
    53bc:	0000175f 	.word	0x0000175f
	...
    53cc:	00001092 	.word	0x00001092
    53d0:	00001774 	.word	0x00001774
    53d4:	0000178c 	.word	0x0000178c

000053d8 <nss_gmac_deinit_mdiobus>:
 * @brief De-initialize MDIO bus
 * @param[in] pointer to nss_gmac_dev
 * @return void
 */
void nss_gmac_deinit_mdiobus(struct nss_gmac_dev *gmacdev)
{
    53d8:	e92d4010 	push	{r4, lr}
    53dc:	e1a04000 	mov	r4, r0
	mdiobus_unregister(gmacdev->miibus);
    53e0:	e5900528 	ldr	r0, [r0, #1320]	@ 0x528
    53e4:	ebfffffe 	bl	0 <mdiobus_unregister>
	mdiobus_free(gmacdev->miibus);
    53e8:	e5940528 	ldr	r0, [r4, #1320]	@ 0x528
    53ec:	ebfffffe 	bl	0 <mdiobus_free>
	gmacdev->miibus = NULL;
    53f0:	e3a03000 	mov	r3, #0
    53f4:	e5843528 	str	r3, [r4, #1320]	@ 0x528
}
    53f8:	e8bd8010 	pop	{r4, pc}

000053fc <nss_gmac_per_prec_stats_enable_handler>:
 *	Enable/Disable per-precedence statistics
 */
int nss_gmac_per_prec_stats_enable_handler(struct ctl_table *table, int write,
					   void __user *buffer, size_t *lenp,
					   loff_t *ppos)
{
    53fc:	e92d4037 	push	{r0, r1, r2, r4, r5, lr}
    5400:	e1a05001 	mov	r5, r1
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);
    5404:	e59dc018 	ldr	ip, [sp, #24]
    5408:	e58dc000 	str	ip, [sp]
    540c:	ebfffffe 	bl	0 <proc_dointvec>

	if (!write)
    5410:	e3550000 	cmp	r5, #0
	ret = proc_dointvec(table, write, buffer, lenp, ppos);
    5414:	e1a04000 	mov	r4, r0
	if (!write)
    5418:	0a00000c 	beq	5450 <nss_gmac_per_prec_stats_enable_handler+0x54>
		return ret;

	if (ret) {
    541c:	e3500000 	cmp	r0, #0
    5420:	0a000003 	beq	5434 <nss_gmac_per_prec_stats_enable_handler+0x38>
		pr_err("Errno: -%d.\n", ret);
    5424:	e1a01000 	mov	r1, r0
    5428:	e59f002c 	ldr	r0, [pc, #44]	@ 545c <nss_gmac_per_prec_stats_enable_handler+0x60>
    542c:	ebfffffe 	bl	0 <_printk>
		return ret;
    5430:	ea000006 	b	5450 <nss_gmac_per_prec_stats_enable_handler+0x54>
	}

	switch (ctx.nss_gmac_per_prec_stats_enable) {
    5434:	e59f3024 	ldr	r3, [pc, #36]	@ 5460 <nss_gmac_per_prec_stats_enable_handler+0x64>
    5438:	e5933034 	ldr	r3, [r3, #52]	@ 0x34
    543c:	e3530001 	cmp	r3, #1
    5440:	9a000002 	bls	5450 <nss_gmac_per_prec_stats_enable_handler+0x54>
	case 0:
	case 1:
		break;
	default:
		pr_err("Invalid input. Valid input values: <0|1>\n");
    5444:	e59f0018 	ldr	r0, [pc, #24]	@ 5464 <nss_gmac_per_prec_stats_enable_handler+0x68>
		ret = -1;
    5448:	e3e04000 	mvn	r4, #0
		pr_err("Invalid input. Valid input values: <0|1>\n");
    544c:	ebfffffe 	bl	0 <_printk>
		break;
	}

	return ret;
};
    5450:	e1a00004 	mov	r0, r4
    5454:	e28dd00c 	add	sp, sp, #12
    5458:	e8bd8030 	pop	{r4, r5, pc}
    545c:	0000179f 	.word	0x0000179f
    5460:	00000000 	.word	0x00000000
    5464:	000017ae 	.word	0x000017ae

00005468 <nss_gmac_per_prec_stats_reset_handler>:
 *	Reset per-precedence statistics
 */
int nss_gmac_per_prec_stats_reset_handler(struct ctl_table *table, int write,
				   void __user *buffer, size_t *lenp,
				   loff_t *ppos)
{
    5468:	e92d41f3 	push	{r0, r1, r4, r5, r6, r7, r8, lr}
    546c:	e1a05001 	mov	r5, r1
	int ret, i;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);
    5470:	e59dc020 	ldr	ip, [sp, #32]
    5474:	e58dc000 	str	ip, [sp]
    5478:	ebfffffe 	bl	0 <proc_dointvec>

	if (!write)
    547c:	e3550000 	cmp	r5, #0
	ret = proc_dointvec(table, write, buffer, lenp, ppos);
    5480:	e1a04000 	mov	r4, r0
	if (!write)
    5484:	0a000026 	beq	5524 <nss_gmac_per_prec_stats_reset_handler+0xbc>
		return ret;

	if (ret) {
    5488:	e3500000 	cmp	r0, #0
    548c:	0a000003 	beq	54a0 <nss_gmac_per_prec_stats_reset_handler+0x38>
		pr_err("Errno: -%d.\n", ret);
    5490:	e1a01000 	mov	r1, r0
    5494:	e59f0094 	ldr	r0, [pc, #148]	@ 5530 <nss_gmac_per_prec_stats_reset_handler+0xc8>
    5498:	ebfffffe 	bl	0 <_printk>
		return ret;
    549c:	ea000020 	b	5524 <nss_gmac_per_prec_stats_reset_handler+0xbc>
	}

	switch (ctx.nss_gmac_per_prec_stats_reset) {
    54a0:	e59f508c 	ldr	r5, [pc, #140]	@ 5534 <nss_gmac_per_prec_stats_reset_handler+0xcc>
    54a4:	e5953038 	ldr	r3, [r5, #56]	@ 0x38
    54a8:	e3530000 	cmp	r3, #0
    54ac:	0a00001c 	beq	5524 <nss_gmac_per_prec_stats_reset_handler+0xbc>
    54b0:	e3530001 	cmp	r3, #1
    54b4:	1a000017 	bne	5518 <nss_gmac_per_prec_stats_reset_handler+0xb0>
    54b8:	e2855018 	add	r5, r5, #24
	case 0:
		break;
	case 1: {
		for (i = 0; i < NSS_MAX_GMACS; i++) {
    54bc:	e1a06000 	mov	r6, r0
			if (ctx.nss_gmac[i]) {
    54c0:	e5b50004 	ldr	r0, [r5, #4]!
    54c4:	e3500000 	cmp	r0, #0
    54c8:	0a00000e 	beq	5508 <nss_gmac_per_prec_stats_reset_handler+0xa0>
	raw_spin_lock_bh(&lock->rlock);
    54cc:	e2800f9a 	add	r0, r0, #616	@ 0x268
    54d0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
				spin_lock_bh(&ctx.nss_gmac[i]->stats_lock);
				memset(ctx.nss_gmac[i]->nss_host_stats.tx_per_prec, 0, sizeof(u64) * NSS_GMAC_PRECEDENCE_MAX);
    54d4:	e5957000 	ldr	r7, [r5]
    54d8:	e3a02040 	mov	r2, #64	@ 0x40
    54dc:	e3a01000 	mov	r1, #0
    54e0:	e2870e4e 	add	r0, r7, #1248	@ 0x4e0
    54e4:	e2800008 	add	r0, r0, #8
    54e8:	ebfffffe 	bl	0 <memset>
				memset(ctx.nss_gmac[i]->nss_host_stats.rx_per_prec, 0, sizeof(u64) * NSS_GMAC_PRECEDENCE_MAX);
    54ec:	e2870e4a 	add	r0, r7, #1184	@ 0x4a0
    54f0:	e3a02040 	mov	r2, #64	@ 0x40
    54f4:	e3a01000 	mov	r1, #0
    54f8:	e2800008 	add	r0, r0, #8
    54fc:	ebfffffe 	bl	0 <memset>
	raw_spin_unlock_bh(&lock->rlock);
    5500:	e2870f9a 	add	r0, r7, #616	@ 0x268
    5504:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		for (i = 0; i < NSS_MAX_GMACS; i++) {
    5508:	e2866001 	add	r6, r6, #1
    550c:	e3560004 	cmp	r6, #4
    5510:	1affffea 	bne	54c0 <nss_gmac_per_prec_stats_reset_handler+0x58>
    5514:	ea000002 	b	5524 <nss_gmac_per_prec_stats_reset_handler+0xbc>
			}
		}
		break;
	}
	default:
		pr_err("Invalid input. Valid input value: 1\n");
    5518:	e59f0018 	ldr	r0, [pc, #24]	@ 5538 <nss_gmac_per_prec_stats_reset_handler+0xd0>
		ret = -1;
    551c:	e3e04000 	mvn	r4, #0
		pr_err("Invalid input. Valid input value: 1\n");
    5520:	ebfffffe 	bl	0 <_printk>
		break;
	}

	return ret;
}
    5524:	e1a00004 	mov	r0, r4
    5528:	e28dd008 	add	sp, sp, #8
    552c:	e8bd81f0 	pop	{r4, r5, r6, r7, r8, pc}
    5530:	0000179f 	.word	0x0000179f
    5534:	00000000 	.word	0x00000000
    5538:	000017da 	.word	0x000017da

Disassembly of section .text.unlikely:

00000000 <nss_gmac_reset>:
{
   0:	e92d41f0 	push	{r4, r5, r6, r7, r8, lr}
   4:	e1a04000 	mov	r4, r0
	netdev = gmacdev->netdev;
   8:	e5905090 	ldr	r5, [r0, #144]	@ 0x90
	uint32_t reset_time __attribute__ ((unused)) = jiffies;
   c:	e59f6064 	ldr	r6, [pc, #100]	@ 78 <nss_gmac_reset+0x78>
	netdev_info(netdev, "%s: %s resetting...\n",
  10:	e59f2064 	ldr	r2, [pc, #100]	@ 7c <nss_gmac_reset+0x7c>
  14:	e59f1064 	ldr	r1, [pc, #100]	@ 80 <nss_gmac_reset+0x80>
  18:	e1a00005 	mov	r0, r5
	uint32_t reset_time __attribute__ ((unused)) = jiffies;
  1c:	e5963000 	ldr	r3, [r6]
	netdev_info(netdev, "%s: %s resetting...\n",
  20:	e1a03005 	mov	r3, r5
  24:	ebfffffe 	bl	0 <netdev_info>
	reset_time = jiffies;
  28:	e5967000 	ldr	r7, [r6]
  2c:	e3a02001 	mov	r2, #1
	nss_gmac_write_reg(gmacdev->dma_base, dma_bus_mode, dma_reset_on);
  30:	e5943004 	ldr	r3, [r4, #4]
  34:	e5832000 	str	r2, [r3]
		msleep(DEFAULT_LOOP_VARIABLE);
  38:	e3a0000a 	mov	r0, #10
  3c:	ebfffffe 	bl	0 <msleep>
		    nss_gmac_read_reg(gmacdev->dma_base, dma_bus_mode);
  40:	e5943004 	ldr	r3, [r4, #4]
	asm volatile("ldr %0, %1"
  44:	e5933000 	ldr	r3, [r3]
	} while (data & dma_reset_on);
  48:	e3130001 	tst	r3, #1
  4c:	1afffff9 	bne	38 <nss_gmac_reset+0x38>
	msleep(1000);
  50:	e3a00ffa 	mov	r0, #1000	@ 0x3e8
  54:	ebfffffe 	bl	0 <msleep>
	data = nss_gmac_read_reg(gmacdev->dma_base, dma_bus_mode);
  58:	e5943004 	ldr	r3, [r4, #4]
  5c:	e5933000 	ldr	r3, [r3]
	netdev_info(netdev, "GMAC reset completed in %d jiffies; dma_bus_mode - 0x%x\n", (int)(jiffies - reset_time), data);
  60:	e5962000 	ldr	r2, [r6]
  64:	e1a00005 	mov	r0, r5
  68:	e59f1014 	ldr	r1, [pc, #20]	@ 84 <nss_gmac_reset+0x84>
  6c:	e0422007 	sub	r2, r2, r7
}
  70:	e8bd41f0 	pop	{r4, r5, r6, r7, r8, lr}
	netdev_info(netdev, "GMAC reset completed in %d jiffies; dma_bus_mode - 0x%x\n", (int)(jiffies - reset_time), data);
  74:	eafffffe 	b	0 <netdev_info>
	...
  80:	0000005b 	.word	0x0000005b
  84:	00000070 	.word	0x00000070

00000088 <nss_gmac_tx_pause_enable>:
{
  88:	e92d4010 	push	{r4, lr}
  8c:	e1a04000 	mov	r4, r0
	netdev_info(gmacdev->netdev, "%s: enable Tx flow control\n", __func__);
  90:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
  94:	e59f205c 	ldr	r2, [pc, #92]	@ f8 <nss_gmac_tx_pause_enable+0x70>
  98:	e59f105c 	ldr	r1, [pc, #92]	@ fc <nss_gmac_tx_pause_enable+0x74>
  9c:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->data_plane_ops->pause_on_off(gmacdev->data_plane_ctx, 1)
  a0:	e594028c 	ldr	r0, [r4, #652]	@ 0x28c
  a4:	e3a01001 	mov	r1, #1
  a8:	e594352c 	ldr	r3, [r4, #1324]	@ 0x52c
  ac:	e593301c 	ldr	r3, [r3, #28]
  b0:	e12fff33 	blx	r3
  b4:	e3500000 	cmp	r0, #0
  b8:	0a000004 	beq	d0 <nss_gmac_tx_pause_enable+0x48>
		netdev_warn(gmacdev->netdev, "%s: tx flow control enable failed\n", __func__);
  bc:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
  c0:	e59f2030 	ldr	r2, [pc, #48]	@ f8 <nss_gmac_tx_pause_enable+0x70>
}
  c4:	e8bd4010 	pop	{r4, lr}
		netdev_warn(gmacdev->netdev, "%s: tx flow control enable failed\n", __func__);
  c8:	e59f1030 	ldr	r1, [pc, #48]	@ 100 <nss_gmac_tx_pause_enable+0x78>
  cc:	eafffffe 	b	0 <netdev_warn>
	addr = (uint32_t)regbase + regoffset;
  d0:	e5942004 	ldr	r2, [r4, #4]
  d4:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
  d8:	e3833401 	orr	r3, r3, #16777216	@ 0x1000000
	asm volatile("str %1, %0"
  dc:	e5823018 	str	r3, [r2, #24]
	addr = (uint32_t)regbase + regoffset;
  e0:	e5942000 	ldr	r2, [r4]
	asm volatile("ldr %0, %1"
  e4:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
  e8:	e3833221 	orr	r3, r3, #268435458	@ 0x10000002
  ec:	e38337e2 	orr	r3, r3, #59244544	@ 0x3880000
	asm volatile("str %1, %0"
  f0:	e5823018 	str	r3, [r2, #24]
}
  f4:	e8bd8010 	pop	{r4, pc}
  f8:	0000000f 	.word	0x0000000f
  fc:	000000a9 	.word	0x000000a9
 100:	000000c5 	.word	0x000000c5

00000104 <nss_gmac_tx_pause_disable>:
{
 104:	e92d4010 	push	{r4, lr}
 108:	e1a04000 	mov	r4, r0
	netdev_info(gmacdev->netdev, "%s: disable Tx flow control\n", __func__);
 10c:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 110:	e59f205c 	ldr	r2, [pc, #92]	@ 174 <nss_gmac_tx_pause_disable+0x70>
 114:	e59f105c 	ldr	r1, [pc, #92]	@ 178 <nss_gmac_tx_pause_disable+0x74>
 118:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->data_plane_ops->pause_on_off(gmacdev->data_plane_ctx, 0)
 11c:	e594028c 	ldr	r0, [r4, #652]	@ 0x28c
 120:	e3a01000 	mov	r1, #0
 124:	e594352c 	ldr	r3, [r4, #1324]	@ 0x52c
 128:	e593301c 	ldr	r3, [r3, #28]
 12c:	e12fff33 	blx	r3
 130:	e3500000 	cmp	r0, #0
 134:	0a000004 	beq	14c <nss_gmac_tx_pause_disable+0x48>
		netdev_warn(gmacdev->netdev, "%s: tx flow control disable failed\n", __func__);
 138:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
 13c:	e59f2030 	ldr	r2, [pc, #48]	@ 174 <nss_gmac_tx_pause_disable+0x70>
}
 140:	e8bd4010 	pop	{r4, lr}
		netdev_warn(gmacdev->netdev, "%s: tx flow control disable failed\n", __func__);
 144:	e59f1030 	ldr	r1, [pc, #48]	@ 17c <nss_gmac_tx_pause_disable+0x78>
 148:	eafffffe 	b	0 <netdev_warn>
	addr = (uint32_t)regbase + regoffset;
 14c:	e5942004 	ldr	r2, [r4, #4]
	asm volatile("ldr %0, %1"
 150:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
 154:	e3c33401 	bic	r3, r3, #16777216	@ 0x1000000
	asm volatile("str %1, %0"
 158:	e5823018 	str	r3, [r2, #24]
	addr = (uint32_t)regbase + regoffset;
 15c:	e5942000 	ldr	r2, [r4]
	asm volatile("ldr %0, %1"
 160:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
 164:	e3c33221 	bic	r3, r3, #268435458	@ 0x10000002
 168:	e3c337e2 	bic	r3, r3, #59244544	@ 0x3880000
	asm volatile("str %1, %0"
 16c:	e5823018 	str	r3, [r2, #24]
}
 170:	e8bd8010 	pop	{r4, pc}
 174:	00000028 	.word	0x00000028
 178:	000000e8 	.word	0x000000e8
 17c:	00000105 	.word	0x00000105

00000180 <nss_gmac_rx_pause_enable>:
{
 180:	e92d4010 	push	{r4, lr}
 184:	e1a04000 	mov	r4, r0
	netdev_info(gmacdev->netdev, "%s: enable Rx flow control\n", __func__);
 188:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 18c:	e59f2028 	ldr	r2, [pc, #40]	@ 1bc <nss_gmac_rx_pause_enable+0x3c>
 190:	e59f1028 	ldr	r1, [pc, #40]	@ 1c0 <nss_gmac_rx_pause_enable+0x40>
 194:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
 198:	e5942004 	ldr	r2, [r4, #4]
	asm volatile("ldr %0, %1"
 19c:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
 1a0:	e3833401 	orr	r3, r3, #16777216	@ 0x1000000
	asm volatile("str %1, %0"
 1a4:	e5823018 	str	r3, [r2, #24]
	addr = (uint32_t)regbase + regoffset;
 1a8:	e5942000 	ldr	r2, [r4]
	asm volatile("ldr %0, %1"
 1ac:	e5923018 	ldr	r3, [r2, #24]
	data = bitpos | nss_gmac_read_reg(regbase, regoffset);
 1b0:	e3833004 	orr	r3, r3, #4
	asm volatile("str %1, %0"
 1b4:	e5823018 	str	r3, [r2, #24]
}
 1b8:	e8bd8010 	pop	{r4, pc}
 1bc:	00000042 	.word	0x00000042
 1c0:	00000129 	.word	0x00000129

000001c4 <nss_gmac_rx_pause_disable>:
{
 1c4:	e92d4010 	push	{r4, lr}
 1c8:	e1a04000 	mov	r4, r0
	netdev_info(gmacdev->netdev, "%s: disable Rx flow control\n", __func__);
 1cc:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 1d0:	e59f2028 	ldr	r2, [pc, #40]	@ 200 <nss_gmac_rx_pause_disable+0x3c>
 1d4:	e59f1028 	ldr	r1, [pc, #40]	@ 204 <nss_gmac_rx_pause_disable+0x40>
 1d8:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
 1dc:	e5942004 	ldr	r2, [r4, #4]
	asm volatile("ldr %0, %1"
 1e0:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
 1e4:	e3c33401 	bic	r3, r3, #16777216	@ 0x1000000
	asm volatile("str %1, %0"
 1e8:	e5823018 	str	r3, [r2, #24]
	addr = (uint32_t)regbase + regoffset;
 1ec:	e5942000 	ldr	r2, [r4]
	asm volatile("ldr %0, %1"
 1f0:	e5923018 	ldr	r3, [r2, #24]
	data = ~bitpos & nss_gmac_read_reg(regbase, regoffset);
 1f4:	e3c33004 	bic	r3, r3, #4
	asm volatile("str %1, %0"
 1f8:	e5823018 	str	r3, [r2, #24]
}
 1fc:	e8bd8010 	pop	{r4, pc}
 200:	0000006e 	.word	0x0000006e
 204:	0000017c 	.word	0x0000017c

00000208 <nss_gmac_config_flow_control>:
{
 208:	e92d4010 	push	{r4, lr}
 20c:	e1a04000 	mov	r4, r0
	netdev_info(gmacdev->netdev, "%s:\n", __func__);
 210:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 214:	e59f20bc 	ldr	r2, [pc, #188]	@ 2d8 <nss_gmac_config_flow_control+0xd0>
 218:	e59f10bc 	ldr	r1, [pc, #188]	@ 2dc <nss_gmac_config_flow_control+0xd4>
 21c:	ebfffffe 	bl	0 <netdev_info>
	if (gmacdev->pause == 0) {
 220:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
 224:	e3530000 	cmp	r3, #0
 228:	1a000004 	bne	240 <nss_gmac_config_flow_control+0x38>
		nss_gmac_rx_pause_disable(gmacdev);
 22c:	e1a00004 	mov	r0, r4
 230:	ebfffffe 	bl	1c4 <nss_gmac_rx_pause_disable>
		nss_gmac_tx_pause_disable(gmacdev);
 234:	e1a00004 	mov	r0, r4
}
 238:	e8bd4010 	pop	{r4, lr}
		nss_gmac_tx_pause_disable(gmacdev);
 23c:	eafffffe 	b	104 <nss_gmac_tx_pause_disable>
 240:	e5943018 	ldr	r3, [r4, #24]
	if (!test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags))
 244:	e3130040 	tst	r3, #64	@ 0x40
 248:	08bd8010 	popeq	{r4, pc}
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_LPA);
 24c:	e5941008 	ldr	r1, [r4, #8]
 250:	e3a02005 	mov	r2, #5
 254:	e1a00004 	mov	r0, r4
 258:	ebfffffe 	bl	178 <nss_gmac_tx_pause_disable+0x74>
	if (phyreg & LPA_PAUSE_CAP) {
 25c:	e3100b01 	tst	r0, #1024	@ 0x400
	phyreg = nss_gmac_mii_rd_reg(gmacdev, gmacdev->phy_base, MII_LPA);
 260:	e1a03000 	mov	r3, r0
		netdev_info(gmacdev->netdev,
 264:	e5940090 	ldr	r0, [r4, #144]	@ 0x90
 268:	e59f2068 	ldr	r2, [pc, #104]	@ 2d8 <nss_gmac_config_flow_control+0xd0>
	if (phyreg & LPA_PAUSE_CAP) {
 26c:	0a00000c 	beq	2a4 <nss_gmac_config_flow_control+0x9c>
		netdev_info(gmacdev->netdev,
 270:	e59f1068 	ldr	r1, [pc, #104]	@ 2e0 <nss_gmac_config_flow_control+0xd8>
 274:	ebfffffe 	bl	0 <netdev_info>
		if (gmacdev->pause & FLOW_CTRL_RX)
 278:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
 27c:	e3130002 	tst	r3, #2
 280:	0a000001 	beq	28c <nss_gmac_config_flow_control+0x84>
			nss_gmac_rx_pause_enable(gmacdev);
 284:	e1a00004 	mov	r0, r4
 288:	ebfffffe 	bl	180 <nss_gmac_rx_pause_enable>
		if (gmacdev->pause & FLOW_CTRL_TX)
 28c:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
 290:	e3130001 	tst	r3, #1
 294:	08bd8010 	popeq	{r4, pc}
			nss_gmac_tx_pause_enable(gmacdev);
 298:	e1a00004 	mov	r0, r4
}
 29c:	e8bd4010 	pop	{r4, lr}
			nss_gmac_tx_pause_enable(gmacdev);
 2a0:	eafffffe 	b	88 <nss_gmac_tx_pause_enable>
	if (phyreg & LPA_PAUSE_ASYM) {
 2a4:	e3130b02 	tst	r3, #2048	@ 0x800
 2a8:	0a000004 	beq	2c0 <nss_gmac_config_flow_control+0xb8>
		netdev_info(gmacdev->netdev,
 2ac:	e59f1030 	ldr	r1, [pc, #48]	@ 2e4 <nss_gmac_config_flow_control+0xdc>
 2b0:	ebfffffe 	bl	0 <netdev_info>
		nss_gmac_rx_pause_disable(gmacdev);
 2b4:	e1a00004 	mov	r0, r4
 2b8:	ebfffffe 	bl	1c4 <nss_gmac_rx_pause_disable>
		if (gmacdev->pause & FLOW_CTRL_TX)
 2bc:	eafffff2 	b	28c <nss_gmac_config_flow_control+0x84>
	netdev_info(gmacdev->netdev,
 2c0:	e59f1020 	ldr	r1, [pc, #32]	@ 2e8 <nss_gmac_config_flow_control+0xe0>
 2c4:	ebfffffe 	bl	0 <netdev_info>
	nss_gmac_rx_flow_control_disable(gmacdev);
 2c8:	e1a00004 	mov	r0, r4
 2cc:	ebfffffe 	bl	58c <nss_gmac_rx_flow_control_disable>
}
 2d0:	e8bd4010 	pop	{r4, lr}
	nss_gmac_tx_flow_control_disable(gmacdev);
 2d4:	eafffffe 	b	5a0 <nss_gmac_tx_flow_control_disable>
 2d8:	00000088 	.word	0x00000088
 2dc:	00000199 	.word	0x00000199
 2e0:	0000019e 	.word	0x0000019e
 2e4:	000001cc 	.word	0x000001cc
 2e8:	000001fc 	.word	0x000001fc

000002ec <nss_gmac_set_mac_addr>:
{
 2ec:	e92d40f0 	push	{r4, r5, r6, r7, lr}
 2f0:	e1a04003 	mov	r4, r3
	netdev_info(gmacdev->netdev, "Set addr %02x:%02x:%02x:%02x:%02x:%02x\n",
 2f4:	e5d33005 	ldrb	r3, [r3, #5]
{
 2f8:	e24dd014 	sub	sp, sp, #20
 2fc:	e1a07000 	mov	r7, r0
 300:	e1a06001 	mov	r6, r1
 304:	e1a05002 	mov	r5, r2
	netdev_info(gmacdev->netdev, "Set addr %02x:%02x:%02x:%02x:%02x:%02x\n",
 308:	e59f1050 	ldr	r1, [pc, #80]	@ 360 <nss_gmac_set_mac_addr+0x74>
 30c:	e58d300c 	str	r3, [sp, #12]
 310:	e5d43004 	ldrb	r3, [r4, #4]
 314:	e58d3008 	str	r3, [sp, #8]
 318:	e5d43003 	ldrb	r3, [r4, #3]
 31c:	e58d3004 	str	r3, [sp, #4]
 320:	e5d43002 	ldrb	r3, [r4, #2]
 324:	e58d3000 	str	r3, [sp]
 328:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 32c:	e5d43001 	ldrb	r3, [r4, #1]
 330:	e5d42000 	ldrb	r2, [r4]
 334:	ebfffffe 	bl	0 <netdev_info>
	addr = (uint32_t)regbase + regoffset;
 338:	e5972000 	ldr	r2, [r7]
	data = (mac_addr[5] << 8) | mac_addr[4] | 0x80000000;
 33c:	e1d430b4 	ldrh	r3, [r4, #4]
 340:	e0866002 	add	r6, r6, r2
 344:	e3833102 	orr	r3, r3, #-2147483648	@ 0x80000000
 348:	e5863000 	str	r3, [r6]
	data = (mac_addr[3] << 24) | (mac_addr[2] << 16)
 34c:	e5943000 	ldr	r3, [r4]
 350:	e0855002 	add	r5, r5, r2
 354:	e5853000 	str	r3, [r5]
}
 358:	e28dd014 	add	sp, sp, #20
 35c:	e8bd80f0 	pop	{r4, r5, r6, r7, pc}
 360:	00000331 	.word	0x00000331

00000364 <netdev_name>:

/* netdev_printk helpers, similar to dev_printk */

static inline const char *netdev_name(const struct net_device *dev)
{
	if (!dev->name[0] || strchr(dev->name, '%'))
 364:	e5d02000 	ldrb	r2, [r0]
 368:	e3520000 	cmp	r2, #0
 36c:	0a000008 	beq	394 <netdev_name+0x30>
{
 370:	e92d4010 	push	{r4, lr}
	if (!dev->name[0] || strchr(dev->name, '%'))
 374:	e3a01025 	mov	r1, #37	@ 0x25
 378:	e1a04000 	mov	r4, r0
 37c:	ebfffffe 	bl	0 <strchr>
		return "(unnamed net_device)";
 380:	e59f3018 	ldr	r3, [pc, #24]	@ 3a0 <netdev_name+0x3c>
 384:	e3500000 	cmp	r0, #0
 388:	01a03004 	moveq	r3, r4
	return dev->name;
}
 38c:	e1a00003 	mov	r0, r3
 390:	e8bd8010 	pop	{r4, pc}
		return "(unnamed net_device)";
 394:	e59f3004 	ldr	r3, [pc, #4]	@ 3a0 <netdev_name+0x3c>
}
 398:	e1a00003 	mov	r0, r3
 39c:	e12fff1e 	bx	lr
 3a0:	000011d8 	.word	0x000011d8

000003a4 <nss_gmac_linkdown>:
{
 3a4:	e92d4070 	push	{r4, r5, r6, lr}
 3a8:	e1a04000 	mov	r4, r0
	struct net_device *netdev = gmacdev->netdev;
 3ac:	e5905090 	ldr	r5, [r0, #144]	@ 0x90
	netdev_info(netdev, "Link down\n");
 3b0:	e59f1040 	ldr	r1, [pc, #64]	@ 3f8 <nss_gmac_linkdown+0x54>
 3b4:	e1a00005 	mov	r0, r5
 3b8:	ebfffffe 	bl	0 <netdev_info>
 3bc:	e5943018 	ldr	r3, [r4, #24]
	if (test_bit(__NSS_GMAC_UP, &gmacdev->flags)) {
 3c0:	e3130001 	tst	r3, #1
 3c4:	0a000006 	beq	3e4 <nss_gmac_linkdown+0x40>
		netif_carrier_off(netdev);
 3c8:	e1a00005 	mov	r0, r5
 3cc:	ebfffffe 	bl	0 <netif_carrier_off>
		gmacdev->data_plane_ops->link_state(gmacdev->data_plane_ctx, 0);
 3d0:	e594028c 	ldr	r0, [r4, #652]	@ 0x28c
 3d4:	e3a01000 	mov	r1, #0
 3d8:	e594352c 	ldr	r3, [r4, #1324]	@ 0x52c
 3dc:	e5933008 	ldr	r3, [r3, #8]
 3e0:	e12fff33 	blx	r3
	gmacdev->link_state = LINKDOWN;
 3e4:	e3a03000 	mov	r3, #0
 3e8:	e5843064 	str	r3, [r4, #100]	@ 0x64
	gmacdev->duplex_mode = 0;
 3ec:	e5843068 	str	r3, [r4, #104]	@ 0x68
	gmacdev->speed = 0;
 3f0:	e584306c 	str	r3, [r4, #108]	@ 0x6c
}
 3f4:	e8bd8070 	pop	{r4, r5, r6, pc}
 3f8:	000012a5 	.word	0x000012a5

000003fc <nss_gmac_start_up>:
{
 3fc:	e92d4070 	push	{r4, r5, r6, lr}
 400:	e1a04000 	mov	r4, r0
 404:	e5903018 	ldr	r3, [r0, #24]
			netdev_info(gmacdev->netdev, "%s: start phy 0x%x\n",
 408:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
	if (test_bit(__NSS_GMAC_LINKPOLL, &gmacdev->flags)) {
 40c:	e3130040 	tst	r3, #64	@ 0x40
 410:	0a00000f 	beq	454 <nss_gmac_start_up+0x58>
		if (!IS_ERR(gmacdev->phydev)) {
 414:	e5943290 	ldr	r3, [r4, #656]	@ 0x290
 418:	e3730a01 	cmn	r3, #4096	@ 0x1000
 41c:	8a000008 	bhi	444 <nss_gmac_start_up+0x48>
			netdev_info(gmacdev->netdev, "%s: start phy 0x%x\n",
 420:	e5933164 	ldr	r3, [r3, #356]	@ 0x164
 424:	e59f2050 	ldr	r2, [pc, #80]	@ 47c <nss_gmac_start_up+0x80>
 428:	e59f1050 	ldr	r1, [pc, #80]	@ 480 <nss_gmac_start_up+0x84>
 42c:	ebfffffe 	bl	0 <netdev_info>
			phy_start(gmacdev->phydev);
 430:	e5940290 	ldr	r0, [r4, #656]	@ 0x290
 434:	ebfffffe 	bl	0 <phy_start>
			phy_start_aneg(gmacdev->phydev);
 438:	e5940290 	ldr	r0, [r4, #656]	@ 0x290
}
 43c:	e8bd4070 	pop	{r4, r5, r6, lr}
			phy_start_aneg(gmacdev->phydev);
 440:	eafffffe 	b	0 <phy_start_aneg>
}
 444:	e8bd4070 	pop	{r4, r5, r6, lr}
			netdev_info(gmacdev->netdev, "%s: Invalid PHY device for a link polled interface\n",
 448:	e59f202c 	ldr	r2, [pc, #44]	@ 47c <nss_gmac_start_up+0x80>
 44c:	e59f1030 	ldr	r1, [pc, #48]	@ 484 <nss_gmac_start_up+0x88>
 450:	eafffffe 	b	0 <netdev_info>
	netdev_info(gmacdev->netdev, "%s: Force link up\n", __func__);
 454:	e59f2020 	ldr	r2, [pc, #32]	@ 47c <nss_gmac_start_up+0x80>
 458:	e59f1028 	ldr	r1, [pc, #40]	@ 488 <nss_gmac_start_up+0x8c>
 45c:	ebfffffe 	bl	0 <netdev_info>
	mutex_lock(&gmacdev->link_mutex);
 460:	e2840e27 	add	r0, r4, #624	@ 0x270
 464:	ebfffffe 	bl	0 <mutex_lock>
	nss_gmac_linkup(gmacdev);
 468:	e1a00004 	mov	r0, r4
 46c:	ebfffffe 	bl	4dc4 <nss_gmac_linkup>
	mutex_unlock(&gmacdev->link_mutex);
 470:	e2840e27 	add	r0, r4, #624	@ 0x270
}
 474:	e8bd4070 	pop	{r4, r5, r6, lr}
	mutex_unlock(&gmacdev->link_mutex);
 478:	eafffffe 	b	0 <mutex_unlock>
 47c:	0000105b 	.word	0x0000105b
 480:	000014ce 	.word	0x000014ce
 484:	000014e2 	.word	0x000014e2
 488:	00001516 	.word	0x00001516

0000048c <nss_gmac_open_work>:
/*
 * nss_gmac_open_work()
 *	Schedule delayed work to open the netdev again
 */
void nss_gmac_open_work(struct work_struct *work)
{
 48c:	e92d4010 	push	{r4, lr}
 490:	e1a04000 	mov	r4, r0
	struct nss_gmac_dev *gmacdev = container_of(to_delayed_work(work),
						struct nss_gmac_dev, gmacwork);

	netdev_info(gmacdev->netdev, "Do the network up in delayed queue %s\n",
 494:	e5102008 	ldr	r2, [r0, #-8]
 498:	e59f1010 	ldr	r1, [pc, #16]	@ 4b0 <nss_gmac_open_work+0x24>
 49c:	e1a00002 	mov	r0, r2
 4a0:	ebfffffe 	bl	0 <netdev_info>
							gmacdev->netdev->name);
	nss_gmac_open(gmacdev->netdev);
 4a4:	e5140008 	ldr	r0, [r4, #-8]
}
 4a8:	e8bd4010 	pop	{r4, lr}
	nss_gmac_open(gmacdev->netdev);
 4ac:	eafffffe 	b	4fbc <nss_gmac_open>
 4b0:	000016f6 	.word	0x000016f6

000004b4 <nss_gmac_mdiobus_reset>:
{
 4b4:	e92d4013 	push	{r0, r1, r4, lr}
	gmacdev->mdc_clk_div = MDC_CLK_DIV;
 4b8:	e3a04000 	mov	r4, #0
	gmacdev = (struct nss_gmac_dev *)bus->priv;
 4bc:	e5900048 	ldr	r0, [r0, #72]	@ 0x48
	netdev_info(gmacdev->netdev, "%s: GMAC%d MDC Clk div set to - 0x%x\n",
 4c0:	e59f2020 	ldr	r2, [pc, #32]	@ 4e8 <nss_gmac_mdiobus_reset+0x34>
 4c4:	e59f1020 	ldr	r1, [pc, #32]	@ 4ec <nss_gmac_mdiobus_reset+0x38>
	gmacdev->mdc_clk_div = MDC_CLK_DIV;
 4c8:	e5804060 	str	r4, [r0, #96]	@ 0x60
	netdev_info(gmacdev->netdev, "%s: GMAC%d MDC Clk div set to - 0x%x\n",
 4cc:	e58d4000 	str	r4, [sp]
 4d0:	e590300c 	ldr	r3, [r0, #12]
 4d4:	e5900090 	ldr	r0, [r0, #144]	@ 0x90
 4d8:	ebfffffe 	bl	0 <netdev_info>
}
 4dc:	e1a00004 	mov	r0, r4
 4e0:	e28dd008 	add	sp, sp, #8
 4e4:	e8bd8010 	pop	{r4, pc}
 4e8:	0000107b 	.word	0x0000107b
 4ec:	0000171d 	.word	0x0000171d

Disassembly of section .init.text:

00000000 <nss_gmac_register_driver>:
{
   0:	e92d4010 	push	{r4, lr}
	ctx.common_init_done = false;
   4:	e3a03000 	mov	r3, #0
   8:	e59f4060 	ldr	r4, [pc, #96]	@ 70 <nss_gmac_register_driver+0x70>
			create_singlethread_workqueue(NSS_GMAC_WORKQUEUE_NAME);
   c:	e3a02001 	mov	r2, #1
  10:	e59f105c 	ldr	r1, [pc, #92]	@ 74 <nss_gmac_register_driver+0x74>
  14:	e59f005c 	ldr	r0, [pc, #92]	@ 78 <nss_gmac_register_driver+0x78>
	ctx.msm_clk_ctl_enabled = false;
  18:	e5c43018 	strb	r3, [r4, #24]
	ctx.common_init_done = false;
  1c:	e5c4302c 	strb	r3, [r4, #44]	@ 0x2c
			create_singlethread_workqueue(NSS_GMAC_WORKQUEUE_NAME);
  20:	e59f3054 	ldr	r3, [pc, #84]	@ 7c <nss_gmac_register_driver+0x7c>
  24:	ebfffffe 	bl	0 <alloc_workqueue>
	if (!ctx.gmac_workqueue) {
  28:	e3500000 	cmp	r0, #0
	ctx.gmac_workqueue =
  2c:	e5840000 	str	r0, [r4]
	if (!ctx.gmac_workqueue) {
  30:	1a000004 	bne	48 <nss_gmac_register_driver+0x48>
		pr_info("%s: cannot create workqueue.\n",
  34:	e59f1044 	ldr	r1, [pc, #68]	@ 80 <nss_gmac_register_driver+0x80>
  38:	e59f0044 	ldr	r0, [pc, #68]	@ 84 <nss_gmac_register_driver+0x84>
  3c:	ebfffffe 	bl	0 <_printk>
	return -EIO;
  40:	e3e00004 	mvn	r0, #4
  44:	e8bd8010 	pop	{r4, pc}
	if (platform_driver_register(&nss_gmac_drv) != 0) {
  48:	e59f1038 	ldr	r1, [pc, #56]	@ 88 <nss_gmac_register_driver+0x88>
  4c:	e59f0038 	ldr	r0, [pc, #56]	@ 8c <nss_gmac_register_driver+0x8c>
  50:	ebfffffe 	bl	0 <__platform_driver_register>
  54:	e3500000 	cmp	r0, #0
  58:	08bd8010 	popeq	{r4, pc}
		pr_info("platform drv reg failure\n");
  5c:	e59f002c 	ldr	r0, [pc, #44]	@ 90 <nss_gmac_register_driver+0x90>
  60:	ebfffffe 	bl	0 <_printk>
	destroy_workqueue(ctx.gmac_workqueue);
  64:	e5940000 	ldr	r0, [r4]
  68:	ebfffffe 	bl	0 <destroy_workqueue>
  6c:	eafffff3 	b	40 <nss_gmac_register_driver+0x40>
  70:	00000000 	.word	0x00000000
  74:	000e000a 	.word	0x000e000a
  78:	000009de 	.word	0x000009de
  7c:	000009cf 	.word	0x000009cf
  80:	000002c8 	.word	0x000002c8
  84:	000009e1 	.word	0x000009e1
  88:	00000000 	.word	0x00000000
  8c:	0000006c 	.word	0x0000006c
  90:	00000a01 	.word	0x00000a01

00000094 <init_module>:

/**
 * @brief Module Init
 */
int __init nss_gmac_host_interface_init(void)
{
  94:	e92d4010 	push	{r4, lr}
	pr_info("**********************************************************\n");
  98:	e59f003c 	ldr	r0, [pc, #60]	@ dc <init_module+0x48>
  9c:	ebfffffe 	bl	0 <_printk>
	pr_info("* Driver    :%s\n", nss_gmac_driver_string);
  a0:	e59f1038 	ldr	r1, [pc, #56]	@ e0 <init_module+0x4c>
  a4:	e59f0038 	ldr	r0, [pc, #56]	@ e4 <init_module+0x50>
  a8:	ebfffffe 	bl	0 <_printk>
	pr_info("* Version   :%s\n", nss_gmac_driver_version);
  ac:	e59f1034 	ldr	r1, [pc, #52]	@ e8 <init_module+0x54>
  b0:	e59f0034 	ldr	r0, [pc, #52]	@ ec <init_module+0x58>
  b4:	ebfffffe 	bl	0 <_printk>
	pr_info("* Copyright :%s\n", nss_gmac_copyright);
  b8:	e59f1030 	ldr	r1, [pc, #48]	@ f0 <init_module+0x5c>
  bc:	e59f0030 	ldr	r0, [pc, #48]	@ f4 <init_module+0x60>
  c0:	ebfffffe 	bl	0 <_printk>
	pr_info("**********************************************************\n");
  c4:	e59f0010 	ldr	r0, [pc, #16]	@ dc <init_module+0x48>
  c8:	ebfffffe 	bl	0 <_printk>
#endif

	/*
	 * Initialize the Network dependent services
	 */
	if (nss_gmac_register_driver() != 0)
  cc:	ebfffffe 	bl	0 <nss_gmac_register_driver>
		return -EFAULT;
  d0:	e3500000 	cmp	r0, #0

	return 0;
}
  d4:	13e0000d 	mvnne	r0, #13
  d8:	e8bd8010 	pop	{r4, pc}
  dc:	00000a1d 	.word	0x00000a1d
  e0:	000002e1 	.word	0x000002e1
  e4:	00000a5b 	.word	0x00000a5b
  e8:	00000300 	.word	0x00000300
  ec:	00000a6e 	.word	0x00000a6e
  f0:	00000304 	.word	0x00000304
  f4:	00000a81 	.word	0x00000a81

Disassembly of section .exit.text:

00000000 <nss_gmac_deregister_driver>:
{
   0:	e92d4010 	push	{r4, lr}
	if (ctx.gmac_workqueue) {
   4:	e59f402c 	ldr	r4, [pc, #44]	@ 38 <nss_gmac_deregister_driver+0x38>
	nss_gmac_exit_network_interfaces();
   8:	ebfffffe 	bl	2874 <nss_gmac_exit_network_interfaces>
	platform_driver_unregister(&nss_gmac_drv);
   c:	e59f0028 	ldr	r0, [pc, #40]	@ 3c <nss_gmac_deregister_driver+0x3c>
  10:	ebfffffe 	bl	0 <platform_driver_unregister>
	if (ctx.gmac_workqueue) {
  14:	e5940000 	ldr	r0, [r4]
  18:	e3500000 	cmp	r0, #0
  1c:	0a000002 	beq	2c <nss_gmac_deregister_driver+0x2c>
		destroy_workqueue(ctx.gmac_workqueue);
  20:	ebfffffe 	bl	0 <destroy_workqueue>
		ctx.gmac_workqueue = NULL;
  24:	e3a03000 	mov	r3, #0
  28:	e5843000 	str	r3, [r4]
}
  2c:	e8bd4010 	pop	{r4, lr}
	nss_gmac_common_deinit(&ctx);
  30:	e59f0000 	ldr	r0, [pc]	@ 38 <nss_gmac_deregister_driver+0x38>
  34:	eafffffe 	b	2f20 <nss_gmac_common_deinit>
  38:	00000000 	.word	0x00000000
  3c:	0000006c 	.word	0x0000006c

00000040 <cleanup_module>:
/**
 * @brief Module Exit
 */
void __exit nss_gmac_host_interface_exit(void)
{
	nss_gmac_deregister_driver();
  40:	eafffffe 	b	0 <nss_gmac_deregister_driver>
