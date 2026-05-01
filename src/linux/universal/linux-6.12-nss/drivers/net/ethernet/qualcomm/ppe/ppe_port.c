// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* PPE Port MAC initialization and PPE port MAC functions. */

#include <linux/clk.h>
#include <linux/of_net.h>
#include <linux/pcs/pcs-qcom-ipq9574.h>
#include <linux/phylink.h>
#include <linux/reset.h>
#include <linux/regmap.h>
#include <linux/rtnetlink.h>

#include "edma_port.h"
#include "ppe.h"
#include "ppe_port.h"
#include "ppe_regs.h"

/* PPE MAC max frame size which including 4bytes FCS */
#define PPE_PORT_MAC_MAX_FRAME_SIZE		0x3000

/* PPE BM port start for PPE MAC ports */
#define PPE_BM_PORT_MAC_START			7

/* Poll interval time to poll GMAC MIBs for overflow protection,
 * the time should ensure that the 32bit GMAC packet counter
 * register would not overflow within this time at line rate
 * speed for 64B packet size.
 */
#define PPE_GMIB_POLL_INTERVAL_MS		120000

#define PPE_MAC_MIB_DESC(_s, _o, _n)		\
	{					\
		.size = (_s),			\
		.offset = (_o),			\
		.name = (_n),			\
	}

/* PPE MAC MIB description */
struct ppe_mac_mib_info {
	u32 size;
	u32 offset;
	const char *name;
};

/* PPE GMAC MIB statistics type */
enum ppe_gmib_stats_type {
	gmib_rx_broadcast,
	gmib_rx_pause,
	gmib_rx_multicast,
	gmib_rx_fcserr,
	gmib_rx_alignerr,
	gmib_rx_runt,
	gmib_rx_frag,
	gmib_rx_jumbofcserr,
	gmib_rx_jumboalignerr,
	gmib_rx_pkt64,
	gmib_rx_pkt65to127,
	gmib_rx_pkt128to255,
	gmib_rx_pkt256to511,
	gmib_rx_pkt512to1023,
	gmib_rx_pkt1024to1518,
	gmib_rx_pkt1519tomax,
	gmib_rx_toolong,
	gmib_rx_bytes_g,
	gmib_rx_bytes_b,
	gmib_rx_unicast,
	gmib_tx_broadcast,
	gmib_tx_pause,
	gmib_tx_multicast,
	gmib_tx_underrun,
	gmib_tx_pkt64,
	gmib_tx_pkt65to127,
	gmib_tx_pkt128to255,
	gmib_tx_pkt256to511,
	gmib_tx_pkt512to1023,
	gmib_tx_pkt1024to1518,
	gmib_tx_pkt1519tomax,
	gmib_tx_bytes,
	gmib_tx_collisions,
	gmib_tx_abortcol,
	gmib_tx_multicol,
	gmib_tx_singlecol,
	gmib_tx_excdeffer,
	gmib_tx_deffer,
	gmib_tx_latecol,
	gmib_tx_unicast,
};

/* PPE XGMAC MIB statistics type */
enum ppe_xgmib_stats_type {
	xgmib_tx_bytes,
	xgmib_tx_frames,
	xgmib_tx_broadcast_g,
	xgmib_tx_multicast_g,
	xgmib_tx_pkt64,
	xgmib_tx_pkt65to127,
	xgmib_tx_pkt128to255,
	xgmib_tx_pkt256to511,
	xgmib_tx_pkt512to1023,
	xgmib_tx_pkt1024tomax,
	xgmib_tx_unicast,
	xgmib_tx_multicast,
	xgmib_tx_broadcast,
	xgmib_tx_underflow_err,
	xgmib_tx_bytes_g,
	xgmib_tx_frames_g,
	xgmib_tx_pause,
	xgmib_tx_vlan_g,
	xgmib_tx_lpi_usec,
	xgmib_tx_lpi_tran,
	xgmib_rx_frames,
	xgmib_rx_bytes,
	xgmib_rx_bytes_g,
	xgmib_rx_broadcast_g,
	xgmib_rx_multicast_g,
	xgmib_rx_crc_err,
	xgmib_rx_runt_err,
	xgmib_rx_jabber_err,
	xgmib_rx_undersize_g,
	xgmib_rx_oversize_g,
	xgmib_rx_pkt64,
	xgmib_rx_pkt65to127,
	xgmib_rx_pkt128to255,
	xgmib_rx_pkt256to511,
	xgmib_rx_pkt512to1023,
	xgmib_rx_pkt1024tomax,
	xgmib_rx_unicast_g,
	xgmib_rx_len_err,
	xgmib_rx_outofrange_err,
	xgmib_rx_pause,
	xgmib_rx_fifo_overflow,
	xgmib_rx_vlan,
	xgmib_rx_wdog_err,
	xgmib_rx_lpi_usec,
	xgmib_rx_lpi_tran,
	xgmib_rx_drop_frames,
	xgmib_rx_drop_bytes,
};

/* PPE port clock and reset name */
static const char * const ppe_port_clk_rst_name[] = {
	[PPE_PORT_CLK_RST_MAC] = "port_mac",
	[PPE_PORT_CLK_RST_RX] = "port_rx",
	[PPE_PORT_CLK_RST_TX] = "port_tx",
};

/* PPE GMAC MIB statistics description information */
static const struct ppe_mac_mib_info gmib_info[] = {
	PPE_MAC_MIB_DESC(4, GMAC_RXBROAD_ADDR, "rx_broadcast"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPAUSE_ADDR, "rx_pause"),
	PPE_MAC_MIB_DESC(4, GMAC_RXMULTI_ADDR, "rx_multicast"),
	PPE_MAC_MIB_DESC(4, GMAC_RXFCSERR_ADDR, "rx_fcserr"),
	PPE_MAC_MIB_DESC(4, GMAC_RXALIGNERR_ADDR, "rx_alignerr"),
	PPE_MAC_MIB_DESC(4, GMAC_RXRUNT_ADDR, "rx_runt"),
	PPE_MAC_MIB_DESC(4, GMAC_RXFRAG_ADDR, "rx_frag"),
	PPE_MAC_MIB_DESC(4, GMAC_RXJUMBOFCSERR_ADDR, "rx_jumbofcserr"),
	PPE_MAC_MIB_DESC(4, GMAC_RXJUMBOALIGNERR_ADDR, "rx_jumboalignerr"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT64_ADDR, "rx_pkt64"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT65TO127_ADDR, "rx_pkt65to127"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT128TO255_ADDR, "rx_pkt128to255"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT256TO511_ADDR, "rx_pkt256to511"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT512TO1023_ADDR, "rx_pkt512to1023"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT1024TO1518_ADDR, "rx_pkt1024to1518"),
	PPE_MAC_MIB_DESC(4, GMAC_RXPKT1519TOX_ADDR, "rx_pkt1519tomax"),
	PPE_MAC_MIB_DESC(4, GMAC_RXTOOLONG_ADDR, "rx_toolong"),
	PPE_MAC_MIB_DESC(8, GMAC_RXBYTE_G_ADDR, "rx_bytes_g"),
	PPE_MAC_MIB_DESC(8, GMAC_RXBYTE_B_ADDR, "rx_bytes_b"),
	PPE_MAC_MIB_DESC(4, GMAC_RXUNI_ADDR, "rx_unicast"),
	PPE_MAC_MIB_DESC(4, GMAC_TXBROAD_ADDR, "tx_broadcast"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPAUSE_ADDR, "tx_pause"),
	PPE_MAC_MIB_DESC(4, GMAC_TXMULTI_ADDR, "tx_multicast"),
	PPE_MAC_MIB_DESC(4, GMAC_TXUNDERRUN_ADDR, "tx_underrun"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT64_ADDR, "tx_pkt64"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT65TO127_ADDR, "tx_pkt65to127"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT128TO255_ADDR, "tx_pkt128to255"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT256TO511_ADDR, "tx_pkt256to511"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT512TO1023_ADDR, "tx_pkt512to1023"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT1024TO1518_ADDR, "tx_pkt1024to1518"),
	PPE_MAC_MIB_DESC(4, GMAC_TXPKT1519TOX_ADDR, "tx_pkt1519tomax"),
	PPE_MAC_MIB_DESC(8, GMAC_TXBYTE_ADDR, "tx_bytes"),
	PPE_MAC_MIB_DESC(4, GMAC_TXCOLLISIONS_ADDR, "tx_collisions"),
	PPE_MAC_MIB_DESC(4, GMAC_TXABORTCOL_ADDR, "tx_abortcol"),
	PPE_MAC_MIB_DESC(4, GMAC_TXMULTICOL_ADDR, "tx_multicol"),
	PPE_MAC_MIB_DESC(4, GMAC_TXSINGLECOL_ADDR, "tx_singlecol"),
	PPE_MAC_MIB_DESC(4, GMAC_TXEXCESSIVEDEFER_ADDR, "tx_excdeffer"),
	PPE_MAC_MIB_DESC(4, GMAC_TXDEFER_ADDR, "tx_deffer"),
	PPE_MAC_MIB_DESC(4, GMAC_TXLATECOL_ADDR, "tx_latecol"),
	PPE_MAC_MIB_DESC(4, GMAC_TXUNI_ADDR, "tx_unicast"),
};

/* PPE XGMAC MIB statistics description information */
static const struct ppe_mac_mib_info xgmib_info[] = {
	PPE_MAC_MIB_DESC(8, XGMAC_TXBYTE_GB_ADDR, "tx_bytes"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT_GB_ADDR, "tx_frames"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXBROAD_G_ADDR, "tx_broadcast_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXMULTI_G_ADDR, "tx_multicast_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT64_GB_ADDR, "tx_pkt64"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT65TO127_GB_ADDR, "tx_pkt65to127"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT128TO255_GB_ADDR, "tx_pkt128to255"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT256TO511_GB_ADDR, "tx_pkt256to511"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT512TO1023_GB_ADDR, "tx_pkt512to1023"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT1024TOMAX_GB_ADDR, "tx_pkt1024tomax"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXUNI_GB_ADDR, "tx_unicast"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXMULTI_GB_ADDR, "tx_multicast"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXBROAD_GB_ADDR, "tx_broadcast"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXUNDERFLOW_ERR_ADDR, "tx_underflow_err"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXBYTE_G_ADDR, "tx_bytes_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPKT_G_ADDR, "tx_frames_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXPAUSE_ADDR, "tx_pause"),
	PPE_MAC_MIB_DESC(8, XGMAC_TXVLAN_G_ADDR, "tx_vlan_g"),
	PPE_MAC_MIB_DESC(4, XGMAC_TXLPI_USEC_ADDR, "tx_lpi_usec"),
	PPE_MAC_MIB_DESC(4, XGMAC_TXLPI_TRAN_ADDR, "tx_lpi_tran"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT_GB_ADDR, "rx_frames"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXBYTE_GB_ADDR, "rx_bytes"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXBYTE_G_ADDR, "rx_bytes_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXBROAD_G_ADDR, "rx_broadcast_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXMULTI_G_ADDR, "rx_multicast_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXCRC_ERR_ADDR, "rx_crc_err"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXRUNT_ERR_ADDR, "rx_runt_err"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXJABBER_ERR_ADDR, "rx_jabber_err"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXUNDERSIZE_G_ADDR, "rx_undersize_g"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXOVERSIZE_G_ADDR, "rx_oversize_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT64_GB_ADDR, "rx_pkt64"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT65TO127_GB_ADDR, "rx_pkt65to127"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT128TO255_GB_ADDR, "rx_pkt128to255"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT256TO511_GB_ADDR, "rx_pkt256to511"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT512TO1023_GB_ADDR, "rx_pkt512to1023"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPKT1024TOMAX_GB_ADDR, "rx_pkt1024tomax"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXUNI_G_ADDR, "rx_unicast_g"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXLEN_ERR_ADDR, "rx_len_err"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXOUTOFRANGE_ADDR, "rx_outofrange_err"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXPAUSE_ADDR, "rx_pause"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXFIFOOVERFLOW_ADDR, "rx_fifo_overflow"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXVLAN_GB_ADDR, "rx_vlan"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXWATCHDOG_ERR_ADDR, "rx_wdog_err"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXLPI_USEC_ADDR, "rx_lpi_usec"),
	PPE_MAC_MIB_DESC(4, XGMAC_RXLPI_TRAN_ADDR, "rx_lpi_tran"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXDISCARD_GB_ADDR, "rx_drop_frames"),
	PPE_MAC_MIB_DESC(8, XGMAC_RXDISCARDBYTE_GB_ADDR, "rx_drop_bytes"),
};

/* Get GMAC MIBs from registers and accumulate to PPE port GMIB stats array */
static void ppe_port_gmib_update(struct ppe_port *ppe_port)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	const struct ppe_mac_mib_info *mib;
	int port = ppe_port->port_id;
	u32 reg, val;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(gmib_info); i++) {
		mib = &gmib_info[i];
		reg = PPE_PORT_GMAC_ADDR(port) + mib->offset;

		ret = regmap_read(ppe_dev->regmap, reg, &val);
		if (ret) {
			dev_warn(ppe_dev->dev, "%s: %d\n", __func__, ret);
			continue;
		}

		ppe_port->gmib_stats[i] += val;
		if (mib->size == 8) {
			ret = regmap_read(ppe_dev->regmap, reg + 4, &val);
			if (ret) {
				dev_warn(ppe_dev->dev, "%s: %d\n",
					 __func__, ret);
				continue;
			}

			ppe_port->gmib_stats[i] += (u64)val << 32;
		}
	}
}

/* Polling task to read GMIB statistics to avoid GMIB 32bit register overflow */
static void ppe_port_gmib_stats_poll(struct work_struct *work)
{
	struct ppe_port *ppe_port = container_of(work, struct ppe_port,
						 gmib_read.work);
	spin_lock(&ppe_port->gmib_stats_lock);
	ppe_port_gmib_update(ppe_port);
	spin_unlock(&ppe_port->gmib_stats_lock);

	schedule_delayed_work(&ppe_port->gmib_read,
			      msecs_to_jiffies(PPE_GMIB_POLL_INTERVAL_MS));
}

/* Get the XGMAC MIB counter based on the specific MIB stats type */
static u64 ppe_port_xgmib_get(struct ppe_port *ppe_port,
			      enum ppe_xgmib_stats_type xgmib_type)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	const struct ppe_mac_mib_info *mib;
	int port = ppe_port->port_id;
	u32 reg, val;
	u64 data = 0;
	int ret;

	mib = &xgmib_info[xgmib_type];
	reg = PPE_PORT_XGMAC_ADDR(port) + mib->offset;

	ret = regmap_read(ppe_dev->regmap, reg, &val);
	if (ret) {
		dev_warn(ppe_dev->dev, "%s: %d\n", __func__, ret);
		goto data_return;
	}

	data = val;
	if (mib->size == 8) {
		ret = regmap_read(ppe_dev->regmap, reg + 4, &val);
		if (ret) {
			dev_warn(ppe_dev->dev, "%s: %d\n", __func__, ret);
			goto data_return;
		}

		data |= (u64)val << 32;
	}

data_return:
	return data;
}

/**
 * ppe_port_get_sset_count() - Get PPE port statistics string count
 * @ppe_port: PPE port
 * @sset: string set ID
 *
 * Description: Get the MAC statistics string count for the PPE port
 * specified by @ppe_port.
 *
 * Return: The count of the statistics string.
 */
int ppe_port_get_sset_count(struct ppe_port *ppe_port, int sset)
{
	if (sset != ETH_SS_STATS)
		return 0;

	if (ppe_port->mac_type == PPE_MAC_TYPE_GMAC)
		return ARRAY_SIZE(gmib_info);
	else
		return ARRAY_SIZE(xgmib_info);
}

/**
 * ppe_port_get_strings() - Get PPE port statistics strings
 * @ppe_port: PPE port
 * @stringset: string set ID
 * @data: pointer to statistics strings
 *
 * Description: Get the MAC statistics stings for the PPE port
 * specified by @ppe_port. The strings are stored in the buffer
 * indicated by @data which used in the ethtool ops.
 */
void ppe_port_get_strings(struct ppe_port *ppe_port, u32 stringset, u8 *data)
{
	int i;

	if (stringset != ETH_SS_STATS)
		return;

	if (ppe_port->mac_type == PPE_MAC_TYPE_GMAC) {
		for (i = 0; i < ARRAY_SIZE(gmib_info); i++)
			strscpy(data + i * ETH_GSTRING_LEN, gmib_info[i].name,
				ETH_GSTRING_LEN);
	} else {
		for (i = 0; i < ARRAY_SIZE(xgmib_info); i++)
			strscpy(data + i * ETH_GSTRING_LEN, xgmib_info[i].name,
				ETH_GSTRING_LEN);
	}
}

/**
 * ppe_port_get_ethtool_stats() - Get PPE port ethtool statistics
 * @ppe_port: PPE port
 * @data: pointer to statistics data
 *
 * Description: Get the MAC statistics for the PPE port specified
 * by @ppe_port. The statistics are stored in the buffer indicated
 * by @data which used in the ethtool ops.
 */
void ppe_port_get_ethtool_stats(struct ppe_port *ppe_port, u64 *data)
{
	int i;

	if (ppe_port->mac_type == PPE_MAC_TYPE_GMAC) {
		spin_lock(&ppe_port->gmib_stats_lock);

		ppe_port_gmib_update(ppe_port);
		for (i = 0; i < ARRAY_SIZE(gmib_info); i++)
			data[i] = ppe_port->gmib_stats[i];

		spin_unlock(&ppe_port->gmib_stats_lock);
	} else {
		for (i = 0; i < ARRAY_SIZE(xgmib_info); i++)
			data[i] = ppe_port_xgmib_get(ppe_port, i);
	}
}

/**
 * ppe_port_get_stats64() - Get PPE port statistics
 * @ppe_port: PPE port
 * @s: statistics pointer
 *
 * Description: Get the MAC statistics for the PPE port specified
 * by @ppe_port.
 */
void ppe_port_get_stats64(struct ppe_port *ppe_port,
			  struct rtnl_link_stats64 *s)
{
	if (ppe_port->mac_type == PPE_MAC_TYPE_GMAC) {
		u64 *src = ppe_port->gmib_stats;

		spin_lock(&ppe_port->gmib_stats_lock);

		ppe_port_gmib_update(ppe_port);

		s->rx_packets = src[gmib_rx_unicast] +
			src[gmib_rx_broadcast] + src[gmib_rx_multicast];

		s->tx_packets = src[gmib_tx_unicast] +
			src[gmib_tx_broadcast] + src[gmib_tx_multicast];

		s->rx_bytes = src[gmib_rx_bytes_g];
		s->tx_bytes = src[gmib_tx_bytes];
		s->multicast = src[gmib_rx_multicast];

		s->rx_crc_errors = src[gmib_rx_fcserr] + src[gmib_rx_frag];
		s->rx_frame_errors = src[gmib_rx_alignerr];
		s->rx_errors = s->rx_crc_errors + s->rx_frame_errors;
		s->rx_dropped = src[gmib_rx_toolong] + s->rx_errors;

		s->tx_fifo_errors = src[gmib_tx_underrun];
		s->tx_aborted_errors = src[gmib_tx_abortcol];
		s->tx_errors = s->tx_fifo_errors + s->tx_aborted_errors;
		s->collisions = src[gmib_tx_collisions];

		spin_unlock(&ppe_port->gmib_stats_lock);
	} else {
		s->multicast = ppe_port_xgmib_get(ppe_port, xgmib_rx_multicast_g);

		s->rx_packets = s->multicast;
		s->rx_packets += ppe_port_xgmib_get(ppe_port, xgmib_rx_unicast_g);
		s->rx_packets += ppe_port_xgmib_get(ppe_port, xgmib_rx_broadcast_g);

		s->tx_packets = ppe_port_xgmib_get(ppe_port, xgmib_tx_frames);
		s->rx_bytes = ppe_port_xgmib_get(ppe_port, xgmib_rx_bytes);
		s->tx_bytes = ppe_port_xgmib_get(ppe_port, xgmib_tx_bytes);

		s->rx_crc_errors = ppe_port_xgmib_get(ppe_port, xgmib_rx_crc_err);
		s->rx_fifo_errors = ppe_port_xgmib_get(ppe_port, xgmib_rx_fifo_overflow);

		s->rx_length_errors = ppe_port_xgmib_get(ppe_port, xgmib_rx_len_err);
		s->rx_errors = s->rx_crc_errors +
			s->rx_fifo_errors + s->rx_length_errors;
		s->rx_dropped = s->rx_errors;

		s->tx_fifo_errors = ppe_port_xgmib_get(ppe_port, xgmib_tx_underflow_err);
		s->tx_errors = s->tx_packets -
			ppe_port_xgmib_get(ppe_port, xgmib_tx_frames_g);
	}
}

/**
 * ppe_port_set_mac_address() - Set PPE port MAC address
 * @ppe_port: PPE port
 * @addr: MAC address
 *
 * Description: Set MAC address for the given PPE port.
 *
 * Return: 0 upon success or a negative error upon failure.
 */
int ppe_port_set_mac_address(struct ppe_port *ppe_port, const u8 *addr)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int port = ppe_port->port_id;
	u32 reg, val;
	int ret;

	if (ppe_port->mac_type == PPE_MAC_TYPE_GMAC) {
		reg = PPE_PORT_GMAC_ADDR(port);
		val = (addr[5] << 8) | addr[4];
		ret = regmap_write(ppe_dev->regmap, reg + GMAC_GOL_ADDR0_ADDR, val);
		if (ret)
			return ret;

		val = (addr[0] << 24) | (addr[1] << 16) |
		      (addr[2] << 8) | addr[3];
		ret = regmap_write(ppe_dev->regmap, reg + GMAC_GOL_ADDR1_ADDR, val);
		if (ret)
			return ret;
	} else {
		reg = PPE_PORT_XGMAC_ADDR(port);
		val = (addr[5] << 8) | addr[4] | XGMAC_ADDR_EN;
		ret = regmap_write(ppe_dev->regmap, reg + XGMAC_ADDR0_H_ADDR, val);
		if (ret)
			return ret;

		val = (addr[3] << 24) | (addr[2] << 16) |
		      (addr[1] << 8) | addr[0];
		ret = regmap_write(ppe_dev->regmap, reg + XGMAC_ADDR0_L_ADDR, val);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * ppe_port_set_mac_eee() - Set EEE configuration for PPE port MAC
 * @ppe_port: PPE port
 * @eee: EEE settings
 *
 * Description: Set port MAC EEE settings for the given PPE port.
 *
 * Return: 0 upon success or a negative error upon failure.
 */
int ppe_port_set_mac_eee(struct ppe_port *ppe_port, struct ethtool_keee *eee)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int port = ppe_port->port_id;
	u32 val;
	int ret;

	ret = regmap_read(ppe_dev->regmap, PPE_LPI_EN_ADDR, &val);
	if (ret)
		return ret;

	if (eee->tx_lpi_enabled)
		val |= PPE_LPI_PORT_EN(port);
	else
		val &= ~PPE_LPI_PORT_EN(port);

	ret = regmap_write(ppe_dev->regmap, PPE_LPI_EN_ADDR, val);

	return ret;
}

/**
 * ppe_port_set_maxframe() - Set port maximum frame size
 * @ppe_port: PPE port structure
 * @maxframe_size: Maximum frame size supported by PPE port
 *
 * Description: Set MTU of network interface specified by @ppe_port.
 *
 * Return: 0 upon success or a negative error upon failure.
 */
int ppe_port_set_maxframe(struct ppe_port *ppe_port, int maxframe_size)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	u32 reg, val, mru_mtu_val[3];
	int port = ppe_port->port_id;
	int ret;

	/* The max frame size should be MTU added by ETH_HLEN in PPE. */
	maxframe_size += ETH_HLEN;

	/* MAC takes cover the FCS for the calculation of frame size. */
	if (maxframe_size > PPE_PORT_MAC_MAX_FRAME_SIZE - ETH_FCS_LEN)
		return -EINVAL;

	reg = PPE_MC_MTU_CTRL_TBL_ADDR + PPE_MC_MTU_CTRL_TBL_INC * port;
	val = FIELD_PREP(PPE_MC_MTU_CTRL_TBL_MTU, maxframe_size);
	ret = regmap_update_bits(ppe_dev->regmap, reg,
				 PPE_MC_MTU_CTRL_TBL_MTU,
				 val);
	if (ret)
		return ret;

	reg = PPE_MRU_MTU_CTRL_TBL_ADDR + PPE_MRU_MTU_CTRL_TBL_INC * port;
	ret = regmap_bulk_read(ppe_dev->regmap, reg,
			       mru_mtu_val, ARRAY_SIZE(mru_mtu_val));
	if (ret)
		return ret;

	PPE_MRU_MTU_CTRL_SET_MRU(mru_mtu_val, maxframe_size);
	PPE_MRU_MTU_CTRL_SET_MTU(mru_mtu_val, maxframe_size);

	return regmap_bulk_write(ppe_dev->regmap, reg,
				 mru_mtu_val, ARRAY_SIZE(mru_mtu_val));
}

/* PPE port and MAC reset */
static int ppe_port_mac_reset(struct ppe_port *ppe_port)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret;

	ret = reset_control_assert(ppe_port->rstcs[PPE_PORT_CLK_RST_MAC]);
	if (ret)
		goto error;

	ret = reset_control_assert(ppe_port->rstcs[PPE_PORT_CLK_RST_RX]);
	if (ret)
		goto error;

	ret = reset_control_assert(ppe_port->rstcs[PPE_PORT_CLK_RST_TX]);
	if (ret)
		goto error;

	/* 150ms delay is required by hardware to reset PPE port and MAC */
	msleep(150);

	ret = reset_control_deassert(ppe_port->rstcs[PPE_PORT_CLK_RST_MAC]);
	if (ret)
		goto error;

	ret = reset_control_deassert(ppe_port->rstcs[PPE_PORT_CLK_RST_RX]);
	if (ret)
		goto error;

	ret = reset_control_deassert(ppe_port->rstcs[PPE_PORT_CLK_RST_TX]);
	if (ret)
		goto error;

	return ret;

error:
	dev_err(ppe_dev->dev, "%s: port %d reset fail %d\n",
		__func__, ppe_port->port_id, ret);
	return ret;
}

/* PPE port MAC configuration for phylink */
static void ppe_port_mac_config(struct phylink_config *config,
				unsigned int mode,
				const struct phylink_link_state *state)
{
	struct ppe_port *ppe_port = container_of(config, struct ppe_port,
						 phylink_config);
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int port = ppe_port->port_id;
	enum ppe_mac_type mac_type;
	u32 val, mask;
	int ret;

	switch (state->interface) {
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10G_QXGMII:
		mac_type = PPE_MAC_TYPE_XGMAC;
		break;
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_PSGMII:
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		mac_type = PPE_MAC_TYPE_GMAC;
		break;
	default:
		dev_err(ppe_dev->dev, "%s: Unsupport interface %s\n",
			__func__, phy_modes(state->interface));
		return;
	}

	/* Reset Port MAC for GMAC */
	if (mac_type == PPE_MAC_TYPE_GMAC) {
		ret = ppe_port_mac_reset(ppe_port);
		if (ret)
			goto err_mac_config;
	}

	/* Port mux to select GMAC or XGMAC */
	mask = PPE_PORT_SEL_XGMAC(port);
	val = mac_type == PPE_MAC_TYPE_GMAC ? 0 : mask;
	ret = regmap_update_bits(ppe_dev->regmap,
				 PPE_PORT_MUX_CTRL_ADDR,
				 mask, val);
	if (ret)
		goto err_mac_config;

	ppe_port->mac_type = mac_type;

	return;

err_mac_config:
	dev_err(ppe_dev->dev, "%s: port %d MAC config fail %d\n",
		__func__, port, ret);
}

/* PPE port GMAC link up configuration */
static int ppe_port_gmac_link_up(struct ppe_port *ppe_port, int speed,
				 int duplex, bool tx_pause, bool rx_pause)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 reg, val;

	/* Set GMAC speed */
	switch (speed) {
	case SPEED_1000:
		val = GMAC_SPEED_1000;
		break;
	case SPEED_100:
		val = GMAC_SPEED_100;
		break;
	case SPEED_10:
		val = GMAC_SPEED_10;
		break;
	default:
		dev_err(ppe_dev->dev, "%s: Invalid GMAC speed %s\n",
			__func__, phy_speed_to_str(speed));
		return -EINVAL;
	}

	reg = PPE_PORT_GMAC_ADDR(port);
	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_SPEED_ADDR,
				 GMAC_SPEED_M, val);
	if (ret)
		return ret;

	/* Set duplex, flow control and enable GMAC */
	val = GMAC_TRXEN;
	if (duplex == DUPLEX_FULL)
		val |= GMAC_DUPLEX_FULL;
	if (tx_pause)
		val |= GMAC_TXFCEN;
	if (rx_pause)
		val |= GMAC_RXFCEN;

	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_ENABLE_ADDR,
				 GMAC_ENABLE_ALL, val);

	return ret;
}

/* PPE port XGMAC link up configuration */
static int ppe_port_xgmac_link_up(struct ppe_port *ppe_port,
				  phy_interface_t interface,
				  int speed, int duplex,
				  bool tx_pause, bool rx_pause)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 reg, val;

	/* Set XGMAC TX speed and enable TX */
	switch (speed) {
	case SPEED_10000:
		if (interface == PHY_INTERFACE_MODE_USXGMII)
			val = XGMAC_SPEED_10000_USXGMII;
		else
			val = XGMAC_SPEED_10000;
		break;
	case SPEED_5000:
		val = XGMAC_SPEED_5000;
		break;
	case SPEED_2500:
		if (interface == PHY_INTERFACE_MODE_USXGMII ||
		    interface == PHY_INTERFACE_MODE_10G_QXGMII)
			val = XGMAC_SPEED_2500_USXGMII;
		else
			val = XGMAC_SPEED_2500;
		break;
	case SPEED_1000:
		val = XGMAC_SPEED_1000;
		break;
	case SPEED_100:
		val = XGMAC_SPEED_100;
		break;
	case SPEED_10:
		val = XGMAC_SPEED_10;
		break;
	default:
		dev_err(ppe_dev->dev, "%s: Invalid XGMAC speed %s\n",
			__func__, phy_speed_to_str(speed));
		return -EINVAL;
	}

	reg = PPE_PORT_XGMAC_ADDR(port);
	val |= XGMAC_TXEN;
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_TX_CONFIG_ADDR,
				 XGMAC_SPEED_M | XGMAC_TXEN, val);
	if (ret)
		return ret;

	/* Set XGMAC TX flow control */
	val = FIELD_PREP(XGMAC_PAUSE_TIME_M, FIELD_MAX(XGMAC_PAUSE_TIME_M));
	val |= tx_pause ? XGMAC_TXFCEN : 0;
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_TX_FLOW_CTRL_ADDR,
				 XGMAC_PAUSE_TIME_M | XGMAC_TXFCEN, val);
	if (ret)
		return ret;

	/* Set XGMAC RX flow control */
	val = rx_pause ? XGMAC_RXFCEN : 0;
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_RX_FLOW_CTRL_ADDR,
				 XGMAC_RXFCEN, val);
	if (ret)
		return ret;

	/* Enable XGMAC RX*/
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_RX_CONFIG_ADDR,
				 XGMAC_RXEN, XGMAC_RXEN);

	return ret;
}

/* PPE port MAC link up configuration for phylink */
static void ppe_port_mac_link_up(struct phylink_config *config,
				 struct phy_device *phy,
				 unsigned int mode,
				 phy_interface_t interface,
				 int speed, int duplex,
				 bool tx_pause, bool rx_pause)
{
	struct ppe_port *ppe_port = container_of(config, struct ppe_port,
						 phylink_config);
	enum ppe_mac_type mac_type = ppe_port->mac_type;
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 reg, val;

	/* Start GMIB statistics polling */
	schedule_delayed_work(&ppe_port->gmib_read, 0);

	if (mac_type == PPE_MAC_TYPE_GMAC)
		ret = ppe_port_gmac_link_up(ppe_port,
					    speed, duplex, tx_pause, rx_pause);
	else
		ret = ppe_port_xgmac_link_up(ppe_port, interface,
					     speed, duplex, tx_pause, rx_pause);
	if (ret)
		goto err_port_mac_link_up;

	/* Set PPE port BM flow control */
	reg = PPE_BM_PORT_FC_MODE_ADDR +
		PPE_BM_PORT_FC_MODE_INC * (port + PPE_BM_PORT_MAC_START);
	val = tx_pause ? PPE_BM_PORT_FC_MODE_EN : 0;
	ret = regmap_update_bits(ppe_dev->regmap, reg,
				 PPE_BM_PORT_FC_MODE_EN, val);
	if (ret)
		goto err_port_mac_link_up;

	/* Enable PPE port TX */
	reg = PPE_PORT_BRIDGE_CTRL_ADDR + PPE_PORT_BRIDGE_CTRL_INC * port;
	ret = regmap_update_bits(ppe_dev->regmap, reg,
				 PPE_PORT_BRIDGE_TXMAC_EN,
				 PPE_PORT_BRIDGE_TXMAC_EN);
	if (ret)
		goto err_port_mac_link_up;

	return;

err_port_mac_link_up:
	dev_err(ppe_dev->dev, "%s: port %d link up fail %d\n",
		__func__, port, ret);
}

/* PPE port MAC link down configuration for phylink */
static void ppe_port_mac_link_down(struct phylink_config *config,
				   unsigned int mode,
				   phy_interface_t interface)
{
	struct ppe_port *ppe_port = container_of(config, struct ppe_port,
						 phylink_config);
	enum ppe_mac_type mac_type = ppe_port->mac_type;
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 reg;

	/* Stop GMIB statistics polling */
	cancel_delayed_work_sync(&ppe_port->gmib_read);

	/* Disable PPE port TX */
	reg = PPE_PORT_BRIDGE_CTRL_ADDR + PPE_PORT_BRIDGE_CTRL_INC * port;
	ret = regmap_update_bits(ppe_dev->regmap, reg,
				 PPE_PORT_BRIDGE_TXMAC_EN, 0);
	if (ret)
		goto err_port_mac_link_down;

	/* Disable PPE MAC */
	if (mac_type == PPE_MAC_TYPE_GMAC) {
		reg = PPE_PORT_GMAC_ADDR(port) + GMAC_ENABLE_ADDR;
		ret = regmap_update_bits(ppe_dev->regmap, reg, GMAC_TRXEN, 0);
		if (ret)
			goto err_port_mac_link_down;
	} else {
		reg = PPE_PORT_XGMAC_ADDR(port);
		ret = regmap_update_bits(ppe_dev->regmap,
					 reg + XGMAC_RX_CONFIG_ADDR,
					 XGMAC_RXEN, 0);
		if (ret)
			goto err_port_mac_link_down;

		ret = regmap_update_bits(ppe_dev->regmap,
					 reg + XGMAC_TX_CONFIG_ADDR,
					 XGMAC_TXEN, 0);
		if (ret)
			goto err_port_mac_link_down;
	}

	return;

err_port_mac_link_down:
	dev_err(ppe_dev->dev, "%s: port %d link down fail %d\n",
		__func__, port, ret);
}

/* PPE port MAC PCS selection for phylink */
static
struct phylink_pcs *ppe_port_mac_select_pcs(struct phylink_config *config,
					    phy_interface_t interface)
{
	struct ppe_port *ppe_port = container_of(config, struct ppe_port,
						 phylink_config);
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 val;

	/* PPE port5 can connects with PCS0 or PCS1. In PSGMII
	 * mode, it selects PCS0; otherwise, it selects PCS1.
	 */
	if (port == 5) {
		val = interface == PHY_INTERFACE_MODE_PSGMII ?
			0 : PPE_PORT5_SEL_PCS1;
		ret = regmap_update_bits(ppe_dev->regmap,
					 PPE_PORT_MUX_CTRL_ADDR,
					 PPE_PORT5_SEL_PCS1, val);
		if (ret) {
			dev_err(ppe_dev->dev, "%s: port5 select PCS fail %d\n",
				__func__, ret);
			return NULL;
		}
	}

	return ppe_port->pcs;
}

static const struct phylink_mac_ops ppe_phylink_ops = {
	.mac_config = ppe_port_mac_config,
	.mac_link_up = ppe_port_mac_link_up,
	.mac_link_down = ppe_port_mac_link_down,
	.mac_select_pcs = ppe_port_mac_select_pcs,
};

/**
 * ppe_port_phylink_setup() - Set phylink instance for the given PPE port
 * @ppe_port: PPE port
 * @netdev: Netdevice
 *
 * Description: Wrapper function to help setup phylink for the PPE port
 * specified by @ppe_port and associated with the net device @netdev.
 *
 * Return: 0 upon success or a negative error upon failure.
 */
int ppe_port_phylink_setup(struct ppe_port *ppe_port, struct net_device *netdev)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	struct device_node *pcs_node;
	int ret;

	/* Create PCS */
	pcs_node = of_parse_phandle(ppe_port->np, "pcs-handle", 0);
	if (!pcs_node)
		return -ENODEV;

	ppe_port->pcs = ipq_pcs_get(pcs_node);
	of_node_put(pcs_node);
	if (IS_ERR(ppe_port->pcs)) {
		dev_err(ppe_dev->dev, "%s: port %d failed to create PCS\n",
			__func__, ppe_port->port_id);
		return PTR_ERR(ppe_port->pcs);
	}

	/* Port phylink capability */
	ppe_port->phylink_config.dev = &netdev->dev;
	ppe_port->phylink_config.type = PHYLINK_NETDEV;
	ppe_port->phylink_config.mac_capabilities = MAC_ASYM_PAUSE |
		MAC_SYM_PAUSE | MAC_10 | MAC_100 | MAC_1000 |
		MAC_2500FD | MAC_5000FD | MAC_10000FD;
	__set_bit(PHY_INTERFACE_MODE_QSGMII,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_PSGMII,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_SGMII,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_1000BASEX,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_2500BASEX,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_USXGMII,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_10GBASER,
		  ppe_port->phylink_config.supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_10G_QXGMII,
		  ppe_port->phylink_config.supported_interfaces);

	/* Create phylink */
	ppe_port->phylink = phylink_create(&ppe_port->phylink_config,
					   of_fwnode_handle(ppe_port->np),
					   ppe_port->interface,
					   &ppe_phylink_ops);
	if (IS_ERR(ppe_port->phylink)) {
		dev_err(ppe_dev->dev, "%s: port %d failed to create phylink\n",
			__func__, ppe_port->port_id);
		ret = PTR_ERR(ppe_port->phylink);
		goto err_free_pcs;
	}

	/* Connect phylink */
	ret = phylink_of_phy_connect(ppe_port->phylink, ppe_port->np, 0);
	if (ret) {
		dev_err(ppe_dev->dev, "%s: port %d failed to connect phylink\n",
			__func__, ppe_port->port_id);
		goto err_free_phylink;
	}

	return 0;

err_free_phylink:
	phylink_destroy(ppe_port->phylink);
	ppe_port->phylink = NULL;
err_free_pcs:
	ipq_pcs_put(ppe_port->pcs);
	ppe_port->pcs = NULL;
	return ret;
}

/**
 * ppe_port_phylink_destroy() - Destroy phylink instance for the given PPE port
 * @ppe_port: PPE port
 *
 * Description: Wrapper function to help destroy phylink for the PPE port
 * specified by @ppe_port.
 */
void ppe_port_phylink_destroy(struct ppe_port *ppe_port)
{
	/* Destroy phylink */
	if (ppe_port->phylink) {
		rtnl_lock();
		phylink_disconnect_phy(ppe_port->phylink);
		rtnl_unlock();
		phylink_destroy(ppe_port->phylink);
		ppe_port->phylink = NULL;
	}

	/* Destroy PCS */
	if (ppe_port->pcs) {
		ipq_pcs_put(ppe_port->pcs);
		ppe_port->pcs = NULL;
	}
}

/* PPE port clock initialization */
static int ppe_port_clock_init(struct ppe_port *ppe_port)
{
	struct device_node *port_node = ppe_port->np;
	struct reset_control *rstc;
	struct clk *clk;
	int i, j, ret;

	for (i = 0; i < PPE_PORT_CLK_RST_MAX; i++) {
		/* Get PPE port resets which will be used to reset PPE
		 * port and MAC.
		 */
		rstc = of_reset_control_get_exclusive(port_node,
						      ppe_port_clk_rst_name[i]);
		if (IS_ERR(rstc)) {
			ret =  PTR_ERR(rstc);
			goto err_rst;
		}

		clk = of_clk_get_by_name(port_node, ppe_port_clk_rst_name[i]);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			goto err_clk_get;
		}

		ret = clk_prepare_enable(clk);
		if (ret)
			goto err_clk_en;

		ppe_port->clks[i] = clk;
		ppe_port->rstcs[i] = rstc;
	}

	return 0;

err_clk_en:
	clk_put(clk);
err_clk_get:
	reset_control_put(rstc);
err_rst:
	for (j = 0; j < i; j++) {
		clk_disable_unprepare(ppe_port->clks[j]);
		clk_put(ppe_port->clks[j]);
		reset_control_put(ppe_port->rstcs[j]);
	}

	return ret;
}

/* PPE port clock deinitialization */
static void ppe_port_clock_deinit(struct ppe_port *ppe_port)
{
	int i;

	for (i = 0; i < PPE_PORT_CLK_RST_MAX; i++) {
		clk_disable_unprepare(ppe_port->clks[i]);
		clk_put(ppe_port->clks[i]);
		reset_control_put(ppe_port->rstcs[i]);
	}
}

/* PPE port MAC hardware init configuration */
static int ppe_port_mac_hw_init(struct ppe_port *ppe_port)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	int ret, port = ppe_port->port_id;
	u32 reg, val;

	/* GMAC RX and TX are initialized as disabled */
	reg = PPE_PORT_GMAC_ADDR(port);
	ret = regmap_update_bits(ppe_dev->regmap,
				 reg + GMAC_ENABLE_ADDR, GMAC_TRXEN, 0);
	if (ret)
		return ret;

	/* GMAC max frame size configuration */
	val = FIELD_PREP(GMAC_JUMBO_SIZE_M, PPE_PORT_MAC_MAX_FRAME_SIZE);
	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_JUMBO_SIZE_ADDR,
				 GMAC_JUMBO_SIZE_M, val);
	if (ret)
		return ret;

	val = FIELD_PREP(GMAC_MAXFRAME_SIZE_M, PPE_PORT_MAC_MAX_FRAME_SIZE);
	val |= FIELD_PREP(GMAC_TX_THD_M, 0x1);
	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_CTRL_ADDR,
				 GMAC_CTRL_MASK, val);
	if (ret)
		return ret;

	val = FIELD_PREP(GMAC_HIGH_IPG_M, 0xc);
	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_DBG_CTRL_ADDR,
				 GMAC_HIGH_IPG_M, val);
	if (ret)
		return ret;

	/* Enable and reset GMAC MIB counters and set as read clear
	 * mode, the GMAC MIB counters will be cleared after reading.
	 */
	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_MIB_CTRL_ADDR,
				 GMAC_MIB_CTRL_MASK, GMAC_MIB_CTRL_MASK);
	if (ret)
		return ret;

	ret = regmap_update_bits(ppe_dev->regmap, reg + GMAC_MIB_CTRL_ADDR,
				 GMAC_MIB_RST, 0);
	if (ret)
		return ret;

	/* XGMAC RX and TX disabled and max frame size configuration */
	reg = PPE_PORT_XGMAC_ADDR(port);
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_TX_CONFIG_ADDR,
				 XGMAC_TXEN | XGMAC_JD, XGMAC_JD);
	if (ret)
		return ret;

	val = FIELD_PREP(XGMAC_GPSL_M, PPE_PORT_MAC_MAX_FRAME_SIZE);
	val |= XGMAC_GPSLEN;
	val |= XGMAC_CST;
	val |= XGMAC_ACS;
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_RX_CONFIG_ADDR,
				 XGMAC_RX_CONFIG_MASK, val);
	if (ret)
		return ret;

	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_WD_TIMEOUT_ADDR,
				 XGMAC_WD_TIMEOUT_MASK, XGMAC_WD_TIMEOUT_VAL);
	if (ret)
		return ret;

	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_PKT_FILTER_ADDR,
				 XGMAC_PKT_FILTER_MASK, XGMAC_PKT_FILTER_VAL);
	if (ret)
		return ret;

	/* Enable and reset XGMAC MIB counters */
	ret = regmap_update_bits(ppe_dev->regmap, reg + XGMAC_MMC_CTRL_ADDR,
				 XGMAC_MCF | XGMAC_CNTRST, XGMAC_CNTRST);

	return ret;
}

/* PPE port MAC MIB work task initialization */
static int ppe_port_mac_mib_work_init(struct ppe_port *ppe_port)
{
	struct ppe_device *ppe_dev = ppe_port->ppe_dev;
	u64 *gstats;

	gstats = devm_kzalloc(ppe_dev->dev,
			      sizeof(*gstats) * ARRAY_SIZE(gmib_info),
			      GFP_KERNEL);
	if (!gstats)
		return -ENOMEM;

	ppe_port->gmib_stats = gstats;

	spin_lock_init(&ppe_port->gmib_stats_lock);
	INIT_DELAYED_WORK(&ppe_port->gmib_read,
			  ppe_port_gmib_stats_poll);

	return 0;
}

/**
 * ppe_port_mac_init() - Initialization of PPE ports for the PPE device
 * @ppe_dev: PPE device
 *
 * Description: Initialize the PPE MAC ports on the PPE device specified
 * by @ppe_dev.
 *
 * Return: 0 upon success or a negative error upon failure.
 */
int ppe_port_mac_init(struct ppe_device *ppe_dev)
{
	struct device_node *ports_node, *port_node;
	int port, num, ret, j, i = 0;
	struct ppe_ports *ppe_ports;
	phy_interface_t phy_mode;

	ports_node = of_get_child_by_name(ppe_dev->dev->of_node,
					  "ethernet-ports");
	if (!ports_node) {
		dev_err(ppe_dev->dev, "Failed to get ports node\n");
		return -ENODEV;
	}

	num = of_get_available_child_count(ports_node);

	ppe_ports = devm_kzalloc(ppe_dev->dev,
				 struct_size(ppe_ports, port, num),
				 GFP_KERNEL);
	if (!ppe_ports) {
		ret = -ENOMEM;
		goto err_ports_node;
	}

	ppe_dev->ports = ppe_ports;
	ppe_ports->num = num;

	for_each_available_child_of_node(ports_node, port_node) {
		ret = of_property_read_u32(port_node, "reg", &port);
		if (ret) {
			dev_err(ppe_dev->dev, "Failed to get port id\n");
			goto err_port_node;
		}

		ret = of_get_phy_mode(port_node, &phy_mode);
		if (ret) {
			dev_err(ppe_dev->dev, "Failed to get phy mode\n");
			goto err_port_node;
		}

		ppe_ports->port[i].ppe_dev = ppe_dev;
		ppe_ports->port[i].port_id = port;
		ppe_ports->port[i].np = port_node;
		ppe_ports->port[i].interface = phy_mode;

		ret = ppe_port_clock_init(&ppe_ports->port[i]);
		if (ret) {
			dev_err(ppe_dev->dev, "Failed to initialize port clocks\n");
			goto err_port_clk;
		}

		ret = ppe_port_mac_hw_init(&ppe_ports->port[i]);
		if (ret) {
			dev_err(ppe_dev->dev, "Failed to initialize MAC hardware\n");
			goto err_port_node;
		}

		ret = ppe_port_mac_mib_work_init(&ppe_ports->port[i]);
		if (ret) {
			dev_err(ppe_dev->dev, "Failed to initialize MAC MIB work\n");
			goto err_port_node;
		}

		ret = edma_port_setup(&ppe_ports->port[i]);
		if (ret) {
			dev_err(ppe_dev->dev, "QCOM EDMA port setup failed\n");
			i--;
			goto err_port_setup;
		}

		i++;
	}

	of_node_put(ports_node);
	return 0;

err_port_setup:
	/* Destroy edma ports created till now */
	while (i >= 0) {
		edma_port_destroy(&ppe_ports->port[i]);
		i--;
	}

err_port_clk:
	for (j = 0; j < i; j++)
		ppe_port_clock_deinit(&ppe_ports->port[j]);
err_port_node:
	of_node_put(port_node);
err_ports_node:
	of_node_put(ports_node);
	return ret;
}

/**
 * ppe_port_mac_deinit() - Deinitialization of PPE ports for the PPE device
 * @ppe_dev: PPE device
 *
 * Description: Deinitialize the PPE MAC ports on the PPE device specified
 * by @ppe_dev.
 */
void ppe_port_mac_deinit(struct ppe_device *ppe_dev)
{
	struct ppe_port *ppe_port;
	int i;

	for (i = 0; i < ppe_dev->ports->num; i++) {
		ppe_port = &ppe_dev->ports->port[i];

		/* Destroy all phylinks and edma ports */
		edma_port_destroy(ppe_port);

		ppe_port_clock_deinit(ppe_port);
	}
}
