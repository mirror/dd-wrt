/*
 **************************************************************************
 * Copyright (c) 2016-2018, 2020-2021 The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <nss_dp_dev.h>
#include "qcom_dev.h"

#define QCOM_STAT(m)		offsetof(struct nss_dp_hal_gmac_stats, m)
#define QCOM_MIB_STAT(m)	offsetof(fal_mib_counter_t, m)

/*
 * Ethtool stats pointer structure
 */
struct qcom_ethtool_stats {
	uint8_t stat_string[ETH_GSTRING_LEN];
	uint32_t stat_offset;
};

/*
 * Array of strings describing data plane statistics
 */
static const struct qcom_ethtool_stats qcom_gstrings_stats[] = {
#if defined(NSS_DP_EDMA_V2)
	/*
	 * Per GMAC DMA driver statistics are
	 * supported only for IPQ95xx, IPQ53XX.
	 */
	{"rx_bytes", QCOM_STAT(rx_bytes)},
	{"rx_packets", QCOM_STAT(rx_packets)},
	{"rx_dropped", QCOM_STAT(rx_dropped)},
	{"rx_fraglist_packets", QCOM_STAT(rx_fraglist_packets)},
	{"rx_nr_frag_packets", QCOM_STAT(rx_nr_frag_packets)},
	{"rx_nr_frag_headroom_err", QCOM_STAT(rx_nr_frag_headroom_err)},
	{"tx_bytes", QCOM_STAT(tx_bytes)},
	{"tx_packets", QCOM_STAT(tx_packets)},
	{"tx_dropped", QCOM_STAT(tx_dropped)},
	{"tx_nr_frag_packets", QCOM_STAT(tx_nr_frag_packets)},
	{"tx_fraglist_packets", QCOM_STAT(tx_fraglist_packets)},
	{"tx_fraglist_nr_frags_packets", QCOM_STAT(tx_fraglist_with_nr_frags_packets)},
	{"tx_tso_packets", QCOM_STAT(tx_tso_packets)},
	{"tx_tso_drop_packets", QCOM_STAT(tx_tso_drop_packets)},
	{"tx_gso_packets", QCOM_STAT(tx_gso_packets)},
	{"tx_gso_drop_packets", QCOM_STAT(tx_gso_drop_packets)},
	{"tx_queue_stopped_cpu0", QCOM_STAT(tx_queue_stopped[0])},
	{"tx_queue_stopped_cpu1", QCOM_STAT(tx_queue_stopped[1])},
	{"tx_queue_stopped_cpu2", QCOM_STAT(tx_queue_stopped[2])},
	{"tx_queue_stopped_cpu3", QCOM_STAT(tx_queue_stopped[3])},
#endif
};

/*
 * Array of strings describing mib statistics
 */
static const struct qcom_ethtool_stats qcom_gstrings_mib_stats[] = {
	{"rx_broadcast", QCOM_MIB_STAT(RxBroad)},
	{"rx_pause", QCOM_MIB_STAT(RxPause)},
	{"rx_unicast", QCOM_MIB_STAT(RxUniCast)},
	{"rx_multicast", QCOM_MIB_STAT(RxMulti)},
	{"rx_fcserr", QCOM_MIB_STAT(RxFcsErr)},
	{"rx_alignerr", QCOM_MIB_STAT(RxAllignErr)},
	{"rx_runt", QCOM_MIB_STAT(RxRunt)},
	{"rx_frag", QCOM_MIB_STAT(RxFragment)},
	{"rx_jmbfcserr", QCOM_MIB_STAT(RxJumboFcsErr)},
	{"rx_jmbalignerr", QCOM_MIB_STAT(RxJumboAligenErr)},
	{"rx_pkt64", QCOM_MIB_STAT(Rx64Byte)},
	{"rx_pkt65to127", QCOM_MIB_STAT(Rx128Byte)},
	{"rx_pkt128to255", QCOM_MIB_STAT(Rx256Byte)},
	{"rx_pkt256to511", QCOM_MIB_STAT(Rx512Byte)},
	{"rx_pkt512to1023", QCOM_MIB_STAT(Rx1024Byte)},
	{"rx_pkt1024to1518", QCOM_MIB_STAT(Rx1518Byte)},
	{"rx_pkt1519tox", QCOM_MIB_STAT(RxMaxByte)},
	{"rx_toolong", QCOM_MIB_STAT(RxTooLong)},
	{"rx_pktgoodbyte", QCOM_MIB_STAT(RxGoodByte)},
	{"rx_pktbadbyte", QCOM_MIB_STAT(RxBadByte)},
	{"rx_overflow", QCOM_MIB_STAT(RxOverFlow)},
	{"tx_broadcast", QCOM_MIB_STAT(TxBroad)},
	{"tx_pause", QCOM_MIB_STAT(TxPause)},
	{"tx_multicast", QCOM_MIB_STAT(TxMulti)},
	{"tx_underrun", QCOM_MIB_STAT(TxUnderRun)},
	{"tx_pkt64", QCOM_MIB_STAT(Tx64Byte)},
	{"tx_pkt65to127", QCOM_MIB_STAT(Tx128Byte)},
	{"tx_pkt128to255", QCOM_MIB_STAT(Tx256Byte)},
	{"tx_pkt256to511", QCOM_MIB_STAT(Tx512Byte)},
	{"tx_pkt512to1023", QCOM_MIB_STAT(Tx1024Byte)},
	{"tx_pkt1024to1518", QCOM_MIB_STAT(Tx1518Byte)},
	{"tx_pkt1519tox", QCOM_MIB_STAT(TxMaxByte)},
	{"tx_oversize", QCOM_MIB_STAT(TxOverSize)},
	{"tx_pktbyte_h", QCOM_MIB_STAT(TxByte)},
	{"tx_collisions", QCOM_MIB_STAT(TxCollision)},
	{"tx_abortcol", QCOM_MIB_STAT(TxAbortCol)},
	{"tx_multicol", QCOM_MIB_STAT(TxMultiCol)},
	{"tx_singlecol", QCOM_MIB_STAT(TxSingalCol)},
	{"tx_exesdeffer", QCOM_MIB_STAT(TxExcDefer)},
	{"tx_deffer", QCOM_MIB_STAT(TxDefer)},
	{"tx_latecol", QCOM_MIB_STAT(TxLateCol)},
	{"tx_unicast", QCOM_MIB_STAT(TxUniCast)},
};

/*
 * Array of strings describing private flag names
 */
static const char * const qcom_strings_priv_flags[] = {
	"linkpoll",
	"tstamp",
	"tsmode",
};

#define QCOM_STATS_LEN		ARRAY_SIZE(qcom_gstrings_stats)
#define QCOM_MIB_STATS_LEN	ARRAY_SIZE(qcom_gstrings_mib_stats)
#define QCOM_PRIV_FLAGS_LEN	ARRAY_SIZE(qcom_strings_priv_flags)

/*
 * qcom_rx_flow_control()
 */
static void qcom_rx_flow_control(struct nss_gmac_hal_dev *nghd, bool enabled)
{
	if (enabled)
		qcom_set_rx_flow_ctrl(nghd);
	else
		qcom_clear_rx_flow_ctrl(nghd);
}

/*
 * qcom_tx_flow_control()
 */
static void qcom_tx_flow_control(struct nss_gmac_hal_dev *nghd, bool enabled)
{
	if (enabled)
		qcom_set_tx_flow_ctrl(nghd);
	else
		qcom_clear_tx_flow_ctrl(nghd);
}

/*
 * qcom_get_mib_stats()
 */
static int32_t qcom_get_mib_stats(struct nss_gmac_hal_dev *nghd)
{
	if (qcom_get_stats(nghd))
		return -1;

	return 0;
}

/*
 * qcom_set_maxframe()
 */
static int32_t qcom_set_maxframe(struct nss_gmac_hal_dev *nghd,
				 uint32_t maxframe)
{
	/*
	 * Check for maximum allowable MTU.
	 */
	BUG_ON(nghd == NULL);

	if (maxframe > QCOM_HAL_MAX_MTU_SIZE) {
		netdev_warn(nghd->netdev, "Maximum allowed MTU: %d\n", QCOM_HAL_MAX_MTU_SIZE);
		return -1;
	}

	return 0;
}

/*
 * qcom_get_maxframe()
 */
static int32_t qcom_get_maxframe(struct nss_gmac_hal_dev *nghd)
{
	int ret;
	uint32_t mtu;

	ret = fal_port_max_frame_size_get(0, nghd->mac_id, &mtu);

	if (!ret)
		return mtu;

	return ret;
}

/*
 * qcom_get_netdev_stats()
 */
static int32_t qcom_get_netdev_stats(struct nss_gmac_hal_dev *nghd,
		struct rtnl_link_stats64 *stats)
{
	struct qcom_hal_dev *qhd = (struct qcom_hal_dev *)nghd;
	fal_mib_counter_t *hal_stats = &(qhd->stats);

	if (qcom_get_mib_stats(nghd))
		return -1;

	stats->rx_packets = hal_stats->RxUniCast + hal_stats->RxBroad
				+ hal_stats->RxMulti;
	stats->tx_packets = hal_stats->TxUniCast + hal_stats->TxBroad
				+ hal_stats->TxMulti;
	stats->rx_bytes = hal_stats->RxGoodByte;
	stats->tx_bytes = hal_stats->TxByte;

	/* RX errors */
	stats->rx_crc_errors = hal_stats->RxFcsErr + hal_stats->RxJumboFcsErr;
	stats->rx_frame_errors = hal_stats->RxAllignErr +
				 hal_stats->RxJumboAligenErr;
	stats->rx_fifo_errors = hal_stats->RxRunt;
	stats->rx_errors = stats->rx_crc_errors + stats->rx_frame_errors +
			   stats->rx_fifo_errors;

	stats->rx_dropped = hal_stats->RxTooLong + stats->rx_errors;

	/* TX errors */
	stats->tx_fifo_errors = hal_stats->TxUnderRun;
	stats->tx_aborted_errors = hal_stats->TxAbortCol;
	stats->tx_errors = stats->tx_fifo_errors + stats->tx_aborted_errors;

	stats->collisions = hal_stats->TxCollision;
	stats->multicast = hal_stats->RxMulti;

	return 0;
}

/*
 * qcom_get_strset_count()
 *	Get string set count for ethtool operations
 */
int32_t qcom_get_strset_count(struct nss_gmac_hal_dev *nghd, int32_t sset)
{
	struct net_device *netdev = nghd->netdev;

	switch (sset) {
	case ETH_SS_STATS:
		return (QCOM_STATS_LEN + QCOM_MIB_STATS_LEN);
	case ETH_SS_PRIV_FLAGS:
		return QCOM_PRIV_FLAGS_LEN;
	}

	netdev_dbg(netdev, "%s: Invalid string set\n", __func__);
	return -EPERM;
}

/*
 * qcom_get_strings()
 *	Get strings
 */
int32_t qcom_get_strings(struct nss_gmac_hal_dev *nghd, int32_t sset,
						uint8_t *data)
{
	struct net_device *netdev = nghd->netdev;
	int i;

	switch (sset) {
	case ETH_SS_STATS:
		for (i = 0; i < QCOM_STATS_LEN; i++) {
			memcpy(data, qcom_gstrings_stats[i].stat_string,
				strlen(qcom_gstrings_stats[i].stat_string));
			data += ETH_GSTRING_LEN;
		}

		for (i = 0; i < QCOM_MIB_STATS_LEN; i++) {
			memcpy(data, qcom_gstrings_mib_stats[i].stat_string,
				strlen(qcom_gstrings_mib_stats[i].stat_string));
			data += ETH_GSTRING_LEN;
		}
		break;

	case ETH_SS_PRIV_FLAGS:
		for (i = 0; i < QCOM_PRIV_FLAGS_LEN; i++) {
			memcpy(data, qcom_strings_priv_flags[i],
				strlen(qcom_strings_priv_flags[i]));
			data += ETH_GSTRING_LEN;
		}
		break;

	default:
		netdev_dbg(netdev, "%s: Invalid string set\n", __func__);
		return -EPERM;
	}

	return 0;
}

/*
 * qcom_get_eth_stats()
 */
static int32_t qcom_get_eth_stats(struct nss_gmac_hal_dev *nghd, uint64_t *data, struct nss_dp_gmac_stats *stats)
{
	struct qcom_hal_dev *qhd = (struct qcom_hal_dev *)nghd;
	fal_mib_counter_t *mib_stats = &(qhd->stats);
	uint8_t *p;
	int i, i_mib;

	/*
	 * Populate data plane statistics.
	 */
	for (i = 0; i < QCOM_STATS_LEN; i++) {
		p = ((uint8_t *)(stats)
			+ qcom_gstrings_stats[i].stat_offset);
		data[i] = *(uint64_t *)p;
	}

	/*
	 * Get MIB statistics
	 */
	if (qcom_get_mib_stats(nghd)) {
		return -1;
	}

	/*
	 * Populate MIB statistics
	 */
	for (i_mib = 0; i_mib < QCOM_MIB_STATS_LEN; i_mib++) {
		p = (uint8_t *)mib_stats
			+ qcom_gstrings_mib_stats[i_mib].stat_offset;
		i = QCOM_STATS_LEN + i_mib;
		data[i] = *(uint32_t *)p;
	}

	return 0;
}

/*
 * qcom_send_pause_frame()
 */
static void qcom_send_pause_frame(struct nss_gmac_hal_dev *nghd)
{
	qcom_set_ctrl2_test_pause(nghd);
}

/*
 * qcom_stop_pause_frame()
 */
static void qcom_stop_pause_frame(struct nss_gmac_hal_dev *nghd)
{
	qcom_reset_ctrl2_test_pause(nghd);
}

/*
 * qcom_init()
 */
static void *qcom_init(struct nss_gmac_hal_platform_data *gmacpdata)
{
	struct qcom_hal_dev *qhd = NULL;
	struct net_device *ndev = NULL;
	struct nss_dp_dev *dp_priv = NULL;
	struct resource *res;

	ndev = gmacpdata->netdev;
	dp_priv = netdev_priv(ndev);

	res = platform_get_resource(dp_priv->pdev, IORESOURCE_MEM, 0);
	if (!res) {
		netdev_dbg(ndev, "Resource get failed.\n");
		return NULL;
	}

	qhd = (struct qcom_hal_dev *)devm_kzalloc(&dp_priv->pdev->dev,
				sizeof(struct qcom_hal_dev), GFP_KERNEL);
	if (!qhd) {
		netdev_dbg(ndev, "kzalloc failed. Returning...\n");
		return NULL;
	}

	qhd->nghd.mac_reg_len = resource_size(res);
	qhd->nghd.memres = devm_request_mem_region(&dp_priv->pdev->dev,
							res->start,
							resource_size(res),
							ndev->name);
	if (!qhd->nghd.memres) {
		netdev_dbg(ndev, "Request mem region failed. Returning.\n");
		devm_kfree(&dp_priv->pdev->dev, qhd);
		return NULL;
	}

	/* Save netdev context in QCOM HAL context */
	qhd->nghd.netdev = gmacpdata->netdev;
	qhd->nghd.mac_id = gmacpdata->macid;

	/* Populate the mac base addresses */
	qhd->nghd.mac_base = devm_ioremap(&dp_priv->pdev->dev,
			res->start, resource_size(res));
	if (!qhd->nghd.mac_base) {
		netdev_dbg(ndev, "ioremap fail.\n");
		devm_release_mem_region(&dp_priv->pdev->dev,
				qhd->nghd.memres->start,
				qhd->nghd.mac_reg_len);
		devm_kfree(&dp_priv->pdev->dev, qhd);
		return NULL;
	}

	spin_lock_init(&qhd->nghd.slock);

	netdev_dbg(ndev, "ioremap OK.Size 0x%x Ndev base 0x%lx macbase 0x%px\n",
			gmacpdata->reg_len,
			ndev->base_addr,
			qhd->nghd.mac_base);

	/* Reset MIB Stats */
	if (fal_mib_port_flush_counters(0, qhd->nghd.mac_id)) {
		netdev_dbg(ndev, "MIB stats Reset fail.\n");
	}

	return (struct nss_gmac_hal_dev *)qhd;
}

/*
 * qcom_get_mac_address()
 */
static void qcom_get_mac_address(struct nss_gmac_hal_dev *nghd,
				 uint8_t *macaddr)
{
	uint32_t data = hal_read_relaxed_reg(nghd->mac_base, QCOM_MAC_ADDR0);
	macaddr[5] = (data >> 8) & 0xff;
	macaddr[4] = (data) & 0xff;

	data = hal_read_relaxed_reg(nghd->mac_base, QCOM_MAC_ADDR1);
	macaddr[0] = (data >> 24) & 0xff;
	macaddr[1] = (data >> 16) & 0xff;
	macaddr[2] = (data >> 8) & 0xff;
	macaddr[3] = (data) & 0xff;
}

/*
 * qcom_set_mac_address()
 */
static void qcom_set_mac_address(struct nss_gmac_hal_dev *nghd,
				uint8_t *macaddr)
{
	uint32_t data = (macaddr[5] << 8) | macaddr[4];
	hal_write_relaxed_reg(nghd->mac_base, QCOM_MAC_ADDR0, data);
	data = (macaddr[0] << 24) | (macaddr[1] << 16)
		| (macaddr[2] << 8) | macaddr[3];
	hal_write_relaxed_reg(nghd->mac_base, QCOM_MAC_ADDR1, data);
}

/*
 * qcom_exit()
 */
static void qcom_exit(struct nss_gmac_hal_dev *nghd)
{
	struct nss_dp_dev *dp_priv = NULL;
	struct qcom_hal_dev *qhd = (struct qcom_hal_dev *)nghd;

	netdev_dbg(nghd->netdev, "Freeing up dev memory.\n");

	dp_priv = netdev_priv(nghd->netdev);
	devm_iounmap(&dp_priv->pdev->dev,
			(void *)nghd->mac_base);
	devm_release_mem_region(&dp_priv->pdev->dev,
			(nghd->memres)->start,
			nghd->mac_reg_len);

	nghd->memres = NULL;
	nghd->mac_base = NULL;

	devm_kfree(&dp_priv->pdev->dev, qhd);
}

/*
 * MAC hal_ops base structure
 */
struct nss_gmac_hal_ops qcom_gmac_ops = {
	.init = &qcom_init,
	.start = NULL,
	.stop = NULL,
	.exit = &qcom_exit,
	.setmacaddr = &qcom_set_mac_address,
	.getmacaddr = &qcom_get_mac_address,
	.rxflowcontrol = &qcom_rx_flow_control,
	.txflowcontrol = &qcom_tx_flow_control,
	.getstats = &qcom_get_mib_stats,
	.setmaxframe = &qcom_set_maxframe,
	.getmaxframe = &qcom_get_maxframe,
	.getndostats = &qcom_get_netdev_stats,
	.getssetcount = &qcom_get_strset_count,
	.getstrings = &qcom_get_strings,
	.getethtoolstats = &qcom_get_eth_stats,
	.sendpause = &qcom_send_pause_frame,
	.stoppause = &qcom_stop_pause_frame,
};
