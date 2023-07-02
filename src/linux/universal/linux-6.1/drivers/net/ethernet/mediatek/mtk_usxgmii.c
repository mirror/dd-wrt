/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2022 MediaTek Inc.
 * Author: Henry Yen <henry.yen@mediatek.com>
 *         Daniel Golle <daniel@makrotopia.org>
 */

#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include "mtk_eth_soc.h"

static struct mtk_usxgmii_pcs *pcs_to_mtk_usxgmii_pcs(struct phylink_pcs *pcs)
{
	return container_of(pcs, struct mtk_usxgmii_pcs, pcs);
}

static int mtk_xfi_pextp_init(struct mtk_eth *eth)
{
	struct device *dev = eth->dev;
	struct device_node *r = dev->of_node;
	struct device_node *np;
	int i;

	eth->regmap_pextp = devm_kcalloc(dev, eth->soc->num_devs, sizeof(eth->regmap_pextp), GFP_KERNEL);
	if (!eth->regmap_pextp)
		return -ENOMEM;

	for (i = 0; i < eth->soc->num_devs; i++) {
		np = of_parse_phandle(r, "mediatek,xfi_pextp", i);
		if (!np)
			break;

		eth->regmap_pextp[i] = syscon_node_to_regmap(np);
		if (IS_ERR(eth->regmap_pextp[i]))
			return PTR_ERR(eth->regmap_pextp[i]);
	}

	return 0;
}

static int mtk_xfi_pll_init(struct mtk_eth *eth)
{
	struct device_node *r = eth->dev->of_node;
	struct device_node *np;

	np = of_parse_phandle(r, "mediatek,xfi_pll", 0);
	if (!np)
		return -1;

	eth->usxgmii_pll = syscon_node_to_regmap(np);
	if (IS_ERR(eth->usxgmii_pll))
		return PTR_ERR(eth->usxgmii_pll);

	return 0;
}

static int mtk_toprgu_init(struct mtk_eth *eth)
{
	struct device_node *r = eth->dev->of_node;
	struct device_node *np;

	np = of_parse_phandle(r, "mediatek,toprgu", 0);
	if (!np)
		return -1;

	eth->toprgu = syscon_node_to_regmap(np);
	if (IS_ERR(eth->toprgu))
		return PTR_ERR(eth->toprgu);

	return 0;
}

int mtk_xfi_pll_enable(struct mtk_eth *eth)
{
	u32 val = 0;

	if (!eth->usxgmii_pll)
		return -EINVAL;

	/* Add software workaround for USXGMII PLL TCL issue */
	regmap_write(eth->usxgmii_pll, XFI_PLL_ANA_GLB8, RG_XFI_PLL_ANA_SWWA);

	regmap_read(eth->usxgmii_pll, XFI_PLL_DIG_GLB8, &val);
	val |= RG_XFI_PLL_EN;
	regmap_write(eth->usxgmii_pll, XFI_PLL_DIG_GLB8, val);

	return 0;
}

static int mtk_mac2xgmii_id(struct mtk_eth *eth, int mac_id)
{
	int xgmii_id = mac_id;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V3)) {
		switch (mac_id) {
		case MTK_GMAC1_ID:
		case MTK_GMAC2_ID:
			xgmii_id = 1;
			break;
		case MTK_GMAC3_ID:
			xgmii_id = 0;
			break;
		default:
			xgmii_id = -1;
		}
	}

	return xgmii_id;
}

static int mtk_xgmii2mac_id(struct mtk_eth *eth, int xgmii_id)
{
	int mac_id = xgmii_id;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V3)) {
		switch (xgmii_id) {
		case 0:
			mac_id = 2;
			break;
		case 1:
			mac_id = 1;
			break;
		default:
			mac_id = -1;
		}
	}

	return mac_id;
}


static void mtk_usxgmii_setup_phya_usxgmii(struct mtk_usxgmii_pcs *mpcs)
{
	struct regmap *pextp;

	if (!mpcs->eth)
		return;

	pextp = mpcs->eth->regmap_pextp[mpcs->id];
	if (!pextp)
		return;

	/* Setup operation mode */
	regmap_write(pextp, 0x9024, 0x00C9071C);
	regmap_write(pextp, 0x2020, 0xAA8585AA);
	regmap_write(pextp, 0x2030, 0x0C020707);
	regmap_write(pextp, 0x2034, 0x0E050F0F);
	regmap_write(pextp, 0x2040, 0x00140032);
	regmap_write(pextp, 0x50F0, 0x00C014AA);
	regmap_write(pextp, 0x50E0, 0x3777C12B);
	regmap_write(pextp, 0x506C, 0x005F9CFF);
	regmap_write(pextp, 0x5070, 0x9D9DFAFA);
	regmap_write(pextp, 0x5074, 0x27273F3F);
	regmap_write(pextp, 0x5078, 0xA7883C68);
	regmap_write(pextp, 0x507C, 0x11661166);
	regmap_write(pextp, 0x5080, 0x0E000AAF);
	regmap_write(pextp, 0x5084, 0x08080D0D);
	regmap_write(pextp, 0x5088, 0x02030909);
	regmap_write(pextp, 0x50E4, 0x0C0C0000);
	regmap_write(pextp, 0x50E8, 0x04040000);
	regmap_write(pextp, 0x50EC, 0x0F0F0C06);
	regmap_write(pextp, 0x50A8, 0x506E8C8C);
	regmap_write(pextp, 0x6004, 0x18190000);
	regmap_write(pextp, 0x00F8, 0x01423342);
	/* Force SGDT_OUT off and select PCS */
	regmap_write(pextp, 0x00F4, 0x80201F20);
	/* Force GLB_CKDET_OUT */
	regmap_write(pextp, 0x0030, 0x00050C00);
	/* Force AEQ on */
	regmap_write(pextp, 0x0070, 0x02002800);
	ndelay(1020);
	/* Setup DA default value */
	regmap_write(pextp, 0x30B0, 0x00000020);
	regmap_write(pextp, 0x3028, 0x00008A01);
	regmap_write(pextp, 0x302C, 0x0000A884);
	regmap_write(pextp, 0x3024, 0x00083002);
	regmap_write(pextp, 0x3010, 0x00022220);
	regmap_write(pextp, 0x5064, 0x0F020A01);
	regmap_write(pextp, 0x50B4, 0x06100600);
	regmap_write(pextp, 0x3048, 0x40704000);
	regmap_write(pextp, 0x3050, 0xA8000000);
	regmap_write(pextp, 0x3054, 0x000000AA);
	regmap_write(pextp, 0x306C, 0x00000F00);
	regmap_write(pextp, 0xA060, 0x00040000);
	regmap_write(pextp, 0x90D0, 0x00000001);
	/* Release reset */
	regmap_write(pextp, 0x0070, 0x0200E800);
	udelay(150);
	/* Switch to P0 */
	regmap_write(pextp, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0200C101);
	udelay(15);
	/* Switch to Gen3 */
	regmap_write(pextp, 0x0070, 0x0202C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0202C101);
	udelay(100);
	regmap_write(pextp, 0x30B0, 0x00000030);
	regmap_write(pextp, 0x00F4, 0x80201F00);
	regmap_write(pextp, 0x3040, 0x30000000);
	udelay(400);
}

static void mtk_usxgmii_setup_phya_5gbaser(struct mtk_usxgmii_pcs *mpcs)
{
	struct regmap *pextp;

	if (!mpcs->eth)
		return;

	pextp = mpcs->eth->regmap_pextp[mpcs->id];
	if (!pextp)
		return;

	/* Setup operation mode */
	regmap_write(pextp, 0x9024, 0x00D9071C);
	regmap_write(pextp, 0x2020, 0xAAA5A5AA);
	regmap_write(pextp, 0x2030, 0x0C020707);
	regmap_write(pextp, 0x2034, 0x0E050F0F);
	regmap_write(pextp, 0x2040, 0x00140032);
	regmap_write(pextp, 0x50F0, 0x00C018AA);
	regmap_write(pextp, 0x50E0, 0x3777812B);
	regmap_write(pextp, 0x506C, 0x005C9CFF);
	regmap_write(pextp, 0x5070, 0x9DFAFAFA);
	regmap_write(pextp, 0x5074, 0x273F3F3F);
	regmap_write(pextp, 0x5078, 0xA8883868);
	regmap_write(pextp, 0x507C, 0x14661466);
	regmap_write(pextp, 0x5080, 0x0E001ABF);
	regmap_write(pextp, 0x5084, 0x080B0D0D);
	regmap_write(pextp, 0x5088, 0x02050909);
	regmap_write(pextp, 0x50E4, 0x0C000000);
	regmap_write(pextp, 0x50E8, 0x04000000);
	regmap_write(pextp, 0x50EC, 0x0F0F0C06);
	regmap_write(pextp, 0x50A8, 0x50808C8C);
	regmap_write(pextp, 0x6004, 0x18000000);
	regmap_write(pextp, 0x00F8, 0x00A132A1);
	/* Force SGDT_OUT off and select PCS */
	regmap_write(pextp, 0x00F4, 0x80201F20);
	/* Force GLB_CKDET_OUT */
	regmap_write(pextp, 0x0030, 0x00050C00);
	/* Force AEQ on */
	regmap_write(pextp, 0x0070, 0x02002800);
	ndelay(1020);
	/* Setup DA default value */
	regmap_write(pextp, 0x30B0, 0x00000020);
	regmap_write(pextp, 0x3028, 0x00008A01);
	regmap_write(pextp, 0x302C, 0x0000A884);
	regmap_write(pextp, 0x3024, 0x00083002);
	regmap_write(pextp, 0x3010, 0x00022220);
	regmap_write(pextp, 0x5064, 0x0F020A01);
	regmap_write(pextp, 0x50B4, 0x06100600);
	regmap_write(pextp, 0x3048, 0x40704000);
	regmap_write(pextp, 0x3050, 0xA8000000);
	regmap_write(pextp, 0x3054, 0x000000AA);
	regmap_write(pextp, 0x306C, 0x00000F00);
	regmap_write(pextp, 0xA060, 0x00040000);
	regmap_write(pextp, 0x90D0, 0x00000003);
	/* Release reset */
	regmap_write(pextp, 0x0070, 0x0200E800);
	udelay(150);
	/* Switch to P0 */
	regmap_write(pextp, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0200C101);
	udelay(15);
	/* Switch to Gen3 */
	regmap_write(pextp, 0x0070, 0x0202C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0202C101);
	udelay(100);
	regmap_write(pextp, 0x30B0, 0x00000030);
	regmap_write(pextp, 0x00F4, 0x80201F00);
	regmap_write(pextp, 0x3040, 0x30000000);
	udelay(400);
}

static void mtk_usxgmii_setup_phya_10gbaser(struct mtk_usxgmii_pcs *mpcs)
{
	struct regmap *pextp;

	if (!mpcs->eth)
		return;

	pextp = mpcs->eth->regmap_pextp[mpcs->id];
	if (!pextp)
		return;

	/* Setup operation mode */
	regmap_write(pextp, 0x9024, 0x00C9071C);
	regmap_write(pextp, 0x2020, 0xAA8585AA);
	regmap_write(pextp, 0x2030, 0x0C020707);
	regmap_write(pextp, 0x2034, 0x0E050F0F);
	regmap_write(pextp, 0x2040, 0x00140032);
	regmap_write(pextp, 0x50F0, 0x00C014AA);
	regmap_write(pextp, 0x50E0, 0x3777C12B);
	regmap_write(pextp, 0x506C, 0x005F9CFF);
	regmap_write(pextp, 0x5070, 0x9D9DFAFA);
	regmap_write(pextp, 0x5074, 0x27273F3F);
	regmap_write(pextp, 0x5078, 0xA7883C68);
	regmap_write(pextp, 0x507C, 0x11661166);
	regmap_write(pextp, 0x5080, 0x0E000AAF);
	regmap_write(pextp, 0x5084, 0x08080D0D);
	regmap_write(pextp, 0x5088, 0x02030909);
	regmap_write(pextp, 0x50E4, 0x0C0C0000);
	regmap_write(pextp, 0x50E8, 0x04040000);
	regmap_write(pextp, 0x50EC, 0x0F0F0C06);
	regmap_write(pextp, 0x50A8, 0x506E8C8C);
	regmap_write(pextp, 0x6004, 0x18190000);
	regmap_write(pextp, 0x00F8, 0x01423342);
	/* Force SGDT_OUT off and select PCS */
	regmap_write(pextp, 0x00F4, 0x80201F20);
	/* Force GLB_CKDET_OUT */
	regmap_write(pextp, 0x0030, 0x00050C00);
	/* Force AEQ on */
	regmap_write(pextp, 0x0070, 0x02002800);
	ndelay(1020);
	/* Setup DA default value */
	regmap_write(pextp, 0x30B0, 0x00000020);
	regmap_write(pextp, 0x3028, 0x00008A01);
	regmap_write(pextp, 0x302C, 0x0000A884);
	regmap_write(pextp, 0x3024, 0x00083002);
	regmap_write(pextp, 0x3010, 0x00022220);
	regmap_write(pextp, 0x5064, 0x0F020A01);
	regmap_write(pextp, 0x50B4, 0x06100600);
	regmap_write(pextp, 0x3048, 0x47684100);
	regmap_write(pextp, 0x3050, 0x00000000);
	regmap_write(pextp, 0x3054, 0x00000000);
	regmap_write(pextp, 0x306C, 0x00000F00);
	if (mpcs->id == 0)
		regmap_write(pextp, 0xA008, 0x0007B400);

	regmap_write(pextp, 0xA060, 0x00040000);
	regmap_write(pextp, 0x90D0, 0x00000001);
	/* Release reset */
	regmap_write(pextp, 0x0070, 0x0200E800);
	udelay(150);
	/* Switch to P0 */
	regmap_write(pextp, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0200C101);
	udelay(15);
	/* Switch to Gen3 */
	regmap_write(pextp, 0x0070, 0x0202C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0202C101);
	udelay(100);
	regmap_write(pextp, 0x30B0, 0x00000030);
	regmap_write(pextp, 0x00F4, 0x80201F00);
	regmap_write(pextp, 0x3040, 0x30000000);
	udelay(400);
}

void mtk_sgmii_setup_phya_gen1(struct mtk_eth *eth, int mac_id)
{
	u32 id = mtk_mac2xgmii_id(eth, mac_id);
	struct regmap *pextp;

	if (id >= eth->soc->num_devs)
		return;

	pextp = eth->regmap_pextp[id];
	if (!pextp)
		return;

	/* Setup operation mode */
	regmap_write(pextp, 0x9024, 0x00D9071C);
	regmap_write(pextp, 0x2020, 0xAA8585AA);
	regmap_write(pextp, 0x2030, 0x0C020207);
	regmap_write(pextp, 0x2034, 0x0E05050F);
	regmap_write(pextp, 0x2040, 0x00200032);
	regmap_write(pextp, 0x50F0, 0x00C014BA);
	regmap_write(pextp, 0x50E0, 0x3777C12B);
	regmap_write(pextp, 0x506C, 0x005F9CFF);
	regmap_write(pextp, 0x5070, 0x9D9DFAFA);
	regmap_write(pextp, 0x5074, 0x27273F3F);
	regmap_write(pextp, 0x5078, 0xA7883C68);
	regmap_write(pextp, 0x507C, 0x11661166);
	regmap_write(pextp, 0x5080, 0x0E000EAF);
	regmap_write(pextp, 0x5084, 0x08080E0D);
	regmap_write(pextp, 0x5088, 0x02030B09);
	regmap_write(pextp, 0x50E4, 0x0C0C0000);
	regmap_write(pextp, 0x50E8, 0x04040000);
	regmap_write(pextp, 0x50EC, 0x0F0F0606);
	regmap_write(pextp, 0x50A8, 0x506E8C8C);
	regmap_write(pextp, 0x6004, 0x18190000);
	regmap_write(pextp, 0x00F8, 0x00FA32FA);
	/* Force SGDT_OUT off and select PCS */
	regmap_write(pextp, 0x00F4, 0x80201F21);
	/* Force GLB_CKDET_OUT */
	regmap_write(pextp, 0x0030, 0x00050C00);
	/* Force AEQ on */
	regmap_write(pextp, 0x0070, 0x02002800);
	ndelay(1020);
	/* Setup DA default value */
	regmap_write(pextp, 0x30B0, 0x00000020);
	regmap_write(pextp, 0x3028, 0x00008A01);
	regmap_write(pextp, 0x302C, 0x0000A884);
	regmap_write(pextp, 0x3024, 0x00083002);
	regmap_write(pextp, 0x3010, 0x00011110);
	regmap_write(pextp, 0x3048, 0x40704000);
	regmap_write(pextp, 0x3064, 0x0000C000);
	regmap_write(pextp, 0x3050, 0xA8000000);
	regmap_write(pextp, 0x3054, 0x000000AA);
	regmap_write(pextp, 0x306C, 0x20200F00);
	regmap_write(pextp, 0xA060, 0x00050000);
	regmap_write(pextp, 0x90D0, 0x00000007);
	/* Release reset */
	regmap_write(pextp, 0x0070, 0x0200E800);
	udelay(150);
	/* Switch to P0 */
	regmap_write(pextp, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0200C101);
	udelay(15);
	/* Switch to Gen2 */
	regmap_write(pextp, 0x0070, 0x0201C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0201C101);
	udelay(100);
	regmap_write(pextp, 0x30B0, 0x00000030);
	regmap_write(pextp, 0x00F4, 0x80201F01);
	regmap_write(pextp, 0x3040, 0x30000000);
	udelay(400);
}

void mtk_sgmii_setup_phya_gen2(struct mtk_eth *eth, int mac_id)
{
	u32 id = mtk_mac2xgmii_id(eth, mac_id);
	struct regmap *pextp;

	if (id >= eth->soc->num_devs)
		return;

	pextp = eth->regmap_pextp[id];
	if (!pextp)
		return;

	/* Setup operation mode */
	regmap_write(pextp, 0x9024, 0x00D9071C);
	regmap_write(pextp, 0x2020, 0xAA8585AA);
	regmap_write(pextp, 0x2030, 0x0C020707);
	regmap_write(pextp, 0x2034, 0x0E050F0F);
	regmap_write(pextp, 0x2040, 0x00140032);
	regmap_write(pextp, 0x50F0, 0x00C014AA);
	regmap_write(pextp, 0x50E0, 0x3777C12B);
	regmap_write(pextp, 0x506C, 0x005F9CFF);
	regmap_write(pextp, 0x5070, 0x9D9DFAFA);
	regmap_write(pextp, 0x5074, 0x27273F3F);
	regmap_write(pextp, 0x5078, 0xA7883C68);
	regmap_write(pextp, 0x507C, 0x11661166);
	regmap_write(pextp, 0x5080, 0x0E000AAF);
	regmap_write(pextp, 0x5084, 0x08080D0D);
	regmap_write(pextp, 0x5088, 0x02030909);
	regmap_write(pextp, 0x50E4, 0x0C0C0000);
	regmap_write(pextp, 0x50E8, 0x04040000);
	regmap_write(pextp, 0x50EC, 0x0F0F0C06);
	regmap_write(pextp, 0x50A8, 0x506E8C8C);
	regmap_write(pextp, 0x6004, 0x18190000);
	regmap_write(pextp, 0x00F8, 0x009C329C);
	/* Force SGDT_OUT off and select PCS */
	regmap_write(pextp, 0x00F4, 0x80201F21);
	/* Force GLB_CKDET_OUT */
	regmap_write(pextp, 0x0030, 0x00050C00);
	/* Force AEQ on */
	regmap_write(pextp, 0x0070, 0x02002800);
	ndelay(1020);
	/* Setup DA default value */
	regmap_write(pextp, 0x30B0, 0x00000020);
	regmap_write(pextp, 0x3028, 0x00008A01);
	regmap_write(pextp, 0x302C, 0x0000A884);
	regmap_write(pextp, 0x3024, 0x00083002);
	regmap_write(pextp, 0x3010, 0x00011110);
	regmap_write(pextp, 0x3048, 0x40704000);
	regmap_write(pextp, 0x3050, 0xA8000000);
	regmap_write(pextp, 0x3054, 0x000000AA);
	regmap_write(pextp, 0x306C, 0x22000F00);
	regmap_write(pextp, 0xA060, 0x00050000);
	regmap_write(pextp, 0x90D0, 0x00000005);
	/* Release reset */
	regmap_write(pextp, 0x0070, 0x0200E800);
	udelay(150);
	/* Switch to P0 */
	regmap_write(pextp, 0x0070, 0x0200C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0200C101);
	udelay(15);
	/* Switch to Gen2 */
	regmap_write(pextp, 0x0070, 0x0201C111);
	ndelay(1020);
	regmap_write(pextp, 0x0070, 0x0201C101);
	udelay(100);
	regmap_write(pextp, 0x30B0, 0x00000030);
	regmap_write(pextp, 0x00F4, 0x80201F01);
	regmap_write(pextp, 0x3040, 0x30000000);
	udelay(400);
}

static void mtk_usxgmii_reset(struct mtk_eth *eth, int id)
{
	u32 val = 0;

	if (id >= eth->soc->num_devs || !eth->toprgu)
		return;

	switch (id) {
	case 0:
		/* Enable software reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST_EN, &val);
		val |= SWSYSRST_XFI_PEXPT0_GRST |
		       SWSYSRST_XFI0_GRST;
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST_EN, val);

		/* Assert USXGMII reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST, &val);
		val |= FIELD_PREP(SWSYSRST_UNLOCK_KEY, 0x88) |
		       SWSYSRST_XFI_PEXPT0_GRST |
		       SWSYSRST_XFI0_GRST;
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST, val);

		udelay(100);

		/* De-assert USXGMII reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST, &val);
		val |= FIELD_PREP(SWSYSRST_UNLOCK_KEY, 0x88);
		val &= ~(SWSYSRST_XFI_PEXPT0_GRST |
			 SWSYSRST_XFI0_GRST);
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST, val);

		/* Disable software reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST_EN, &val);
		val &= ~(SWSYSRST_XFI_PEXPT0_GRST |
			 SWSYSRST_XFI0_GRST);
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST_EN, val);
		break;
	case 1:
		/* Enable software reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST_EN, &val);
		val |= SWSYSRST_XFI_PEXPT1_GRST |
		       SWSYSRST_XFI1_GRST;
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST_EN, val);

		/* Assert USXGMII reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST, &val);
		val |= FIELD_PREP(SWSYSRST_UNLOCK_KEY, 0x88) |
		       SWSYSRST_XFI_PEXPT1_GRST |
		       SWSYSRST_XFI1_GRST;
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST, val);

		udelay(100);

		/* De-assert USXGMII reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST, &val);
		val |= FIELD_PREP(SWSYSRST_UNLOCK_KEY, 0x88);
		val &= ~(SWSYSRST_XFI_PEXPT1_GRST |
			 SWSYSRST_XFI1_GRST);
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST, val);

		/* Disable software reset */
		regmap_read(eth->toprgu, TOPRGU_SWSYSRST_EN, &val);
		val &= ~(SWSYSRST_XFI_PEXPT1_GRST |
			 SWSYSRST_XFI1_GRST);
		regmap_write(eth->toprgu, TOPRGU_SWSYSRST_EN, val);
		break;
	}

	mdelay(10);
}

void mtk_sgmii_reset(struct mtk_eth *eth, int mac_id)
{
	u32 xgmii_id = mtk_mac2xgmii_id(eth, mac_id);

	mtk_usxgmii_reset(eth, xgmii_id);
}


static int mtk_usxgmii_pcs_config(struct phylink_pcs *pcs, unsigned int mode,
				  phy_interface_t interface,
				  const unsigned long *advertising,
				  bool permit_pause_to_mac)
{
	struct mtk_usxgmii_pcs *mpcs = pcs_to_mtk_usxgmii_pcs(pcs);
	struct mtk_eth *eth = mpcs->eth;
	unsigned int an_ctrl = 0, link_timer = 0, xfi_mode = 0, adapt_mode = 0;
	bool mode_changed = false;

	if (interface == PHY_INTERFACE_MODE_USXGMII) {
		an_ctrl = FIELD_PREP(USXGMII_AN_SYNC_CNT, 0x1FF) |
			  USXGMII_AN_ENABLE;
		link_timer = FIELD_PREP(USXGMII_LINK_TIMER_IDLE_DETECT, 0x7B) |
			     FIELD_PREP(USXGMII_LINK_TIMER_COMP_ACK_DETECT, 0x7B) |
			     FIELD_PREP(USXGMII_LINK_TIMER_AN_RESTART, 0x7B);
		xfi_mode = FIELD_PREP(USXGMII_XFI_RX_MODE, USXGMII_XFI_RX_MODE_10G) |
			   FIELD_PREP(USXGMII_XFI_TX_MODE, USXGMII_XFI_TX_MODE_10G);
	} else if (interface == PHY_INTERFACE_MODE_10GKR) {
		an_ctrl = FIELD_PREP(USXGMII_AN_SYNC_CNT, 0x1FF);
		link_timer = FIELD_PREP(USXGMII_LINK_TIMER_IDLE_DETECT, 0x7B) |
			     FIELD_PREP(USXGMII_LINK_TIMER_COMP_ACK_DETECT, 0x7B) |
			     FIELD_PREP(USXGMII_LINK_TIMER_AN_RESTART, 0x7B);
		xfi_mode = FIELD_PREP(USXGMII_XFI_RX_MODE, USXGMII_XFI_RX_MODE_10G) |
			   FIELD_PREP(USXGMII_XFI_TX_MODE, USXGMII_XFI_TX_MODE_10G);
		adapt_mode = USXGMII_RATE_UPDATE_MODE;
	} else if (interface == PHY_INTERFACE_MODE_5GBASER) {
		an_ctrl = FIELD_PREP(USXGMII_AN_SYNC_CNT, 0xFF);
		link_timer = FIELD_PREP(USXGMII_LINK_TIMER_IDLE_DETECT, 0x3D) |
			     FIELD_PREP(USXGMII_LINK_TIMER_COMP_ACK_DETECT, 0x3D) |
			     FIELD_PREP(USXGMII_LINK_TIMER_AN_RESTART, 0x3D);
		xfi_mode = FIELD_PREP(USXGMII_XFI_RX_MODE, USXGMII_XFI_RX_MODE_5G) |
			   FIELD_PREP(USXGMII_XFI_TX_MODE, USXGMII_XFI_TX_MODE_5G);
		adapt_mode = USXGMII_RATE_UPDATE_MODE;
	} else
		return -EINVAL;

	adapt_mode |= FIELD_PREP(USXGMII_RATE_ADAPT_MODE, USXGMII_RATE_ADAPT_MODE_X1);

	if (mpcs->interface != interface) {
		mpcs->interface = interface;
		mode_changed = true;
	}

	mtk_xfi_pll_enable(eth);
	mtk_usxgmii_reset(eth, mpcs->id);

	/* Setup USXGMII AN ctrl */
	regmap_update_bits(mpcs->regmap, RG_PCS_AN_CTRL0,
			   USXGMII_AN_SYNC_CNT | USXGMII_AN_ENABLE,
			   an_ctrl);

	regmap_update_bits(mpcs->regmap, RG_PCS_AN_CTRL2,
			   USXGMII_LINK_TIMER_IDLE_DETECT |
			   USXGMII_LINK_TIMER_COMP_ACK_DETECT |
			   USXGMII_LINK_TIMER_AN_RESTART,
			   link_timer);

	/* Gated MAC CK */
	regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
			   USXGMII_MAC_CK_GATED, USXGMII_MAC_CK_GATED);

	/* Enable interface force mode */
	regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
			   USXGMII_IF_FORCE_EN, USXGMII_IF_FORCE_EN);

	/* Setup USXGMII adapt mode */
	regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
			   USXGMII_RATE_UPDATE_MODE | USXGMII_RATE_ADAPT_MODE,
			   adapt_mode);

	/* Setup USXGMII speed */
	regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
			   USXGMII_XFI_RX_MODE | USXGMII_XFI_TX_MODE,
			   xfi_mode);

	udelay(1);

	/* Un-gated MAC CK */
	regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
			   USXGMII_MAC_CK_GATED, 0);

	udelay(1);

	/* Disable interface force mode for the AN mode */
	if (an_ctrl & USXGMII_AN_ENABLE)
		regmap_update_bits(mpcs->regmap, RG_PHY_TOP_SPEED_CTRL1,
				   USXGMII_IF_FORCE_EN, 0);

	/* Setup USXGMIISYS with the determined property */
	if (interface == PHY_INTERFACE_MODE_USXGMII)
		mtk_usxgmii_setup_phya_usxgmii(mpcs);
	else if (interface == PHY_INTERFACE_MODE_10GKR)
		mtk_usxgmii_setup_phya_10gbaser(mpcs);
	else if (interface == PHY_INTERFACE_MODE_5GBASER)
		mtk_usxgmii_setup_phya_5gbaser(mpcs);

	return mode_changed;
}

static void mtk_usxgmii_pcs_get_state(struct phylink_pcs *pcs,
				    struct phylink_link_state *state)
{
	struct mtk_usxgmii_pcs *mpcs = pcs_to_mtk_usxgmii_pcs(pcs);
	struct mtk_eth *eth = mpcs->eth;
	struct mtk_mac *mac = eth->mac[mtk_xgmii2mac_id(eth, mpcs->id)];
	u32 val = 0;

	regmap_read(mpcs->regmap, RG_PCS_AN_CTRL0, &val);
	if (FIELD_GET(USXGMII_AN_ENABLE, val)) {
		/* Refresh LPA by inverting LPA_LATCH */
		regmap_read(mpcs->regmap, RG_PCS_AN_STS0, &val);
		regmap_update_bits(mpcs->regmap, RG_PCS_AN_STS0,
				   USXGMII_LPA_LATCH,
				   !(val & USXGMII_LPA_LATCH));

		regmap_read(mpcs->regmap, RG_PCS_AN_STS0, &val);

		state->interface = mpcs->interface;
		state->link = FIELD_GET(USXGMII_LPA_LINK, val);
		state->duplex = FIELD_GET(USXGMII_LPA_DUPLEX, val);

		switch (FIELD_GET(USXGMII_LPA_SPEED_MASK, val)) {
		case USXGMII_LPA_SPEED_10:
			state->speed = SPEED_10;
			break;
		case USXGMII_LPA_SPEED_100:
			state->speed = SPEED_100;
			break;
		case USXGMII_LPA_SPEED_1000:
			state->speed = SPEED_1000;
			break;
		case USXGMII_LPA_SPEED_2500:
			state->speed = SPEED_2500;
			break;
		case USXGMII_LPA_SPEED_5000:
			state->speed = SPEED_5000;
			break;
		case USXGMII_LPA_SPEED_10000:
			state->speed = SPEED_10000;
			break;
		}
	} else {
		val = mtk_r32(mac->hw, MTK_XGMAC_STS(mac->id));

		if (mac->id == MTK_GMAC2_ID)
			val = val >> 16;

		switch (FIELD_GET(MTK_USXGMII_PCS_MODE, val)) {
		case 0:
			state->speed = SPEED_10000;
			break;
		case 1:
			state->speed = SPEED_5000;
			break;
		case 2:
			state->speed = SPEED_2500;
			break;
		case 3:
			state->speed = SPEED_1000;
			break;
		}

		state->interface = mpcs->interface;
		state->link = FIELD_GET(MTK_USXGMII_PCS_LINK, val);
		state->duplex = DUPLEX_FULL;
	}

	if (state->link == 0)
		mtk_usxgmii_pcs_config(pcs, MLO_AN_INBAND,
				       state->interface, NULL, false);
}

static void mtk_usxgmii_pcs_restart_an(struct phylink_pcs *pcs)
{
	struct mtk_usxgmii_pcs *mpcs = pcs_to_mtk_usxgmii_pcs(pcs);
	unsigned int val = 0;

	if (!mpcs->regmap)
		return;

	regmap_read(mpcs->regmap, RG_PCS_AN_CTRL0, &val);
	val |= USXGMII_AN_RESTART;
	regmap_write(mpcs->regmap, RG_PCS_AN_CTRL0, val);
}

static void mtk_usxgmii_pcs_link_up(struct phylink_pcs *pcs, unsigned int mode,
				    phy_interface_t interface,
				    int speed, int duplex)
{
	/* Reconfiguring USXGMII to ensure the quality of the RX signal
	 * after the line side link up.
	 */
	mtk_usxgmii_pcs_config(pcs, mode,
			       interface, NULL, false);
}

static const struct phylink_pcs_ops mtk_usxgmii_pcs_ops = {
	.pcs_config = mtk_usxgmii_pcs_config,
	.pcs_get_state = mtk_usxgmii_pcs_get_state,
	.pcs_an_restart = mtk_usxgmii_pcs_restart_an,
	.pcs_link_up = mtk_usxgmii_pcs_link_up,
};

int mtk_usxgmii_init(struct mtk_eth *eth)
{
	struct device_node *r = eth->dev->of_node;
	struct device *dev = eth->dev;
	struct device_node *np;
	int i, ret;

	eth->usxgmii_pcs = devm_kcalloc(dev, eth->soc->num_devs, sizeof(eth->usxgmii_pcs), GFP_KERNEL);
	if (!eth->usxgmii_pcs)
		return -ENOMEM;

	for (i = 0; i < eth->soc->num_devs; i++) {
		np = of_parse_phandle(r, "mediatek,usxgmiisys", i);
		if (!np)
			break;

		eth->usxgmii_pcs[i] = devm_kzalloc(dev, sizeof(*eth->usxgmii_pcs), GFP_KERNEL);
		if (!eth->usxgmii_pcs[i])
			return -ENOMEM;

		eth->usxgmii_pcs[i]->id = i;
		eth->usxgmii_pcs[i]->eth = eth;
		eth->usxgmii_pcs[i]->regmap = syscon_node_to_regmap(np);
		if (IS_ERR(eth->usxgmii_pcs[i]->regmap))
			return PTR_ERR(eth->usxgmii_pcs[i]->regmap);

		eth->usxgmii_pcs[i]->pcs.ops = &mtk_usxgmii_pcs_ops;
		eth->usxgmii_pcs[i]->pcs.poll = true;
		eth->usxgmii_pcs[i]->interface = PHY_INTERFACE_MODE_NA;

		of_node_put(np);
	}

	ret = mtk_xfi_pextp_init(eth);
	if (ret)
		return ret;

	ret = mtk_xfi_pll_init(eth);
	if (ret)
		return ret;

	return mtk_toprgu_init(eth);
}

struct phylink_pcs *mtk_usxgmii_select_pcs(struct mtk_eth *eth, int mac_id)
{
	u32 xgmii_id = mtk_mac2xgmii_id(eth, mac_id);

	if (!eth->usxgmii_pcs[xgmii_id]->regmap)
		return NULL;

	return &eth->usxgmii_pcs[xgmii_id]->pcs;
}
