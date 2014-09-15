/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2009-2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/if_vlan.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>

#include <mt7620.h>
#include "ralink_soc_eth.h"
#include "gsw_mt7620a.h"

#define MT7620A_CDMA_CSG_CFG	0x400
#define MT7620_DMA_VID		(MT7620A_CDMA_CSG_CFG | 0x30)
#define MT7620A_DMA_2B_OFFSET	BIT(31)
#define MT7620A_RESET_FE	BIT(21)
#define MT7620A_RESET_ESW	BIT(23)
#define MT7620_L4_VALID		BIT(23)

#define SYSC_REG_RESET_CTRL     0x34
#define MAX_RX_LENGTH           1536

#define CDMA_ICS_EN		BIT(2)
#define CDMA_UCS_EN		BIT(1)
#define CDMA_TCS_EN		BIT(0)

#define GDMA_ICS_EN		BIT(22)
#define GDMA_TCS_EN		BIT(21)
#define GDMA_UCS_EN		BIT(20)

static const u32 rt5350_reg_table[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = RT5350_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = RT5350_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = RT5350_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = RT5350_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = RT5350_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = RT5350_TX_CTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = RT5350_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = RT5350_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = RT5350_RX_CALC_IDX0,
	[FE_REG_FE_INT_ENABLE] = RT5350_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = RT5350_FE_INT_STATUS,
	[FE_REG_FE_DMA_VID_BASE] = MT7620_DMA_VID,
};

static void mt7620_fe_reset(void)
{
	rt_sysc_w32(MT7620A_RESET_FE | MT7620A_RESET_ESW, SYSC_REG_RESET_CTRL);
	rt_sysc_w32(0, SYSC_REG_RESET_CTRL);
}

static void mt7620_fwd_config(struct fe_priv *priv)
{
	int i;

	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc. */
	for (i = 0; i < 16; i += 2)
		fe_w32(((i + 1) << 16) + i, MT7620_DMA_VID + (i * 2));

	fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~7, MT7620A_GDMA1_FWD_CFG);
	fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) | (GDMA_ICS_EN | GDMA_TCS_EN | GDMA_UCS_EN), MT7620A_GDMA1_FWD_CFG);
	fe_w32(fe_r32(MT7620A_CDMA_CSG_CFG) | (CDMA_ICS_EN | CDMA_UCS_EN | CDMA_TCS_EN), MT7620A_CDMA_CSG_CFG);
}

static void mt7620_tx_dma(struct fe_priv *priv, int idx, struct sk_buff *skb)
{
	unsigned int nr_frags = 0;
	unsigned int len = 0;

	if (skb) {
		nr_frags = skb_shinfo(skb)->nr_frags;
		len = skb->len - skb->data_len;
	}

	if (!skb)
		priv->tx_dma[idx].txd2 = TX_DMA_LSO | TX_DMA_DONE;
	else if (!nr_frags)
		priv->tx_dma[idx].txd2 = TX_DMA_LSO | TX_DMA_PLEN0(len);
	else
		priv->tx_dma[idx].txd2 = TX_DMA_PLEN0(len);

	if(skb && vlan_tx_tag_present(skb))
		priv->tx_dma[idx].txd4 = 0x80 | (vlan_tx_tag_get(skb) >> 13) << 4 | (vlan_tx_tag_get(skb) & 0xF);
	else
		priv->tx_dma[idx].txd4 = 0;
}

static void mt7620_rx_dma(struct fe_priv *priv, int idx, int len)
{
	priv->rx_dma[idx].rxd2 = RX_DMA_PLEN0(len);
}

#ifdef CONFIG_INET_LRO
static int
mt7620_get_skb_header(struct sk_buff *skb, void **iphdr, void **tcph,
			u64 *hdr_flags, void *_priv)
{
	struct iphdr *iph = NULL;
	int vhdr_len = 0;

	/*
	 * Make sure that this packet is Ethernet II, is not VLAN
	 * tagged, is IPv4, has a valid IP header, and is TCP.
	 */
	if (skb->protocol == 0x0081)
		vhdr_len = VLAN_HLEN;

	iph = (struct iphdr *)(skb->data + vhdr_len);
	if(iph->protocol != IPPROTO_TCP)
		return -1;

	*iphdr = iph;
	*tcph = skb->data + (iph->ihl << 2) + vhdr_len;
	*hdr_flags = LRO_IPV4 | LRO_TCP;

	return 0;
}
#endif

static void mt7620_init_data(struct fe_soc_data *data)
{
	if (mt7620_get_eco() >= 5)
		data->tso = 1;
}

static struct fe_soc_data mt7620_data = {
	.mac = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
	.init_data = mt7620_init_data,
	.reset_fe = mt7620_fe_reset,
	.set_mac = mt7620_set_mac,
	.fwd_config = mt7620_fwd_config,
	.tx_dma = mt7620_tx_dma,
	.rx_dma = mt7620_rx_dma,
	.switch_init = mt7620_gsw_probe,
	.switch_config = mt7620_gsw_config,
	.port_init = mt7620_port_init,
	.min_pkt_len = 0,
	.reg_table = rt5350_reg_table,
	.pdma_glo_cfg = FE_PDMA_SIZE_16DWORDS | MT7620A_DMA_2B_OFFSET,
	.rx_dly_int = RT5350_RX_DLY_INT,
	.tx_dly_int = RT5350_TX_DLY_INT,
	.checksum_bit = MT7620_L4_VALID,
	.has_carrier = mt7620a_has_carrier,
	.mdio_read = mt7620_mdio_read,
	.mdio_write = mt7620_mdio_write,
	.mdio_adjust_link = mt7620_mdio_link_adjust,
#ifdef CONFIG_INET_LRO
	.get_skb_header = mt7620_get_skb_header,
#endif
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "ralink,mt7620a-eth", .data = &mt7620_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
