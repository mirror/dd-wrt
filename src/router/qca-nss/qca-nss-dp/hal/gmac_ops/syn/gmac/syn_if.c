/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <fal/fal_mib.h>
#include <fal/fal_port_ctrl.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <nss_dp_arch.h>
#include "syn_dev.h"
#include "syn_mac_reg.h"

#define SYN_STAT(m)	offsetof(struct nss_dp_hal_gmac_stats, m)
#define SYN_MIB_STAT(m)	offsetof(fal_mib_counter_t, m)
#define HW_ERR_SIZE	sizeof(uint64_t)

/*
 * Array to store ethtool statistics
 */
struct syn_ethtool_stats {
	uint8_t stat_string[ETH_GSTRING_LEN];
	uint64_t stat_offset;
};

/*
 * Array of strings describing statistics
 */
static const struct syn_ethtool_stats syn_gstrings_stats[] = {
	{"rx_bytes", SYN_STAT(rx_stats.rx_bytes)},
	{"rx_packets", SYN_STAT(rx_stats.rx_packets)},
	{"rx_errors", SYN_STAT(rx_stats.rx_errors)},
	{"rx_missed", SYN_STAT(rx_stats.rx_missed)},
	{"rx_descriptor_errors", SYN_STAT(rx_stats.rx_descriptor_errors)},
	{"rx_late_collision_errors", SYN_STAT(rx_stats.rx_late_collision_errors)},
	{"rx_dribble_bit_errors", SYN_STAT(rx_stats.rx_dribble_bit_errors)},
	{"rx_length_errors", SYN_STAT(rx_stats.rx_length_errors)},
	{"rx_ip_header_errors", SYN_STAT(rx_stats.rx_ip_header_errors)},
	{"rx_ip_payload_errors", SYN_STAT(rx_stats.rx_ip_payload_errors)},
	{"rx_no_buffer_errors", SYN_STAT(rx_stats.rx_no_buffer_errors)},
	{"rx_transport_csum_bypassed", SYN_STAT(rx_stats.rx_transport_csum_bypassed)},
	{"rx_fifo_overflows", SYN_STAT(rx_stats.rx_fifo_overflows)},
	{"rx_overflow_errors", SYN_STAT(rx_stats.rx_overflow_errors)},
	{"rx_crc_errors", SYN_STAT(rx_stats.rx_crc_errors)},
	{"rx_scatter_bytes", SYN_STAT(rx_stats.rx_scatter_bytes)},
	{"rx_scatter_packets", SYN_STAT(rx_stats.rx_scatter_packets)},
	{"rx_scatter_errors", SYN_STAT(rx_stats.rx_scatter_errors)},
	{"tx_bytes", SYN_STAT(tx_stats.tx_bytes)},
	{"tx_packets", SYN_STAT(tx_stats.tx_packets)},
	{"tx_collisions", SYN_STAT(tx_stats.tx_collisions)},
	{"tx_errors", SYN_STAT(tx_stats.tx_errors)},
	{"tx_jabber_timeout_errors", SYN_STAT(tx_stats.tx_jabber_timeout_errors)},
	{"tx_frame_flushed_errors", SYN_STAT(tx_stats.tx_frame_flushed_errors)},
	{"tx_loss_of_carrier_errors", SYN_STAT(tx_stats.tx_loss_of_carrier_errors)},
	{"tx_no_carrier_errors", SYN_STAT(tx_stats.tx_no_carrier_errors)},
	{"tx_late_collision_errors", SYN_STAT(tx_stats.tx_late_collision_errors)},
	{"tx_excessive_collision_errors", SYN_STAT(tx_stats.tx_excessive_collision_errors)},
	{"tx_excessive_deferral_errors", SYN_STAT(tx_stats.tx_excessive_deferral_errors)},
	{"tx_underflow_errors", SYN_STAT(tx_stats.tx_underflow_errors)},
	{"tx_ip_header_errors", SYN_STAT(tx_stats.tx_ip_header_errors)},
	{"tx_ip_payload_errors", SYN_STAT(tx_stats.tx_ip_payload_errors)},
	{"tx_dropped", SYN_STAT(tx_stats.tx_dropped)},
	{"tx_ts_create_errors", SYN_STAT(tx_stats.tx_ts_create_errors)},
	{"tx_desc_not_avail", SYN_STAT(tx_stats.tx_desc_not_avail)},
	{"tx_pkts_requeued", SYN_STAT(tx_stats.tx_packets_requeued)},
	{"tx_nr_frags_pkts", SYN_STAT(tx_stats.tx_nr_frags_pkts)},
	{"tx_fraglist_pkts", SYN_STAT(tx_stats.tx_fraglist_pkts)},
	{"pmt_interrupts", SYN_STAT(hw_errs[0])},
	{"mmc_interrupts", SYN_STAT(hw_errs[0]) + (1 * HW_ERR_SIZE)},
	{"line_interface_interrupts", SYN_STAT(hw_errs[0]) + (2 * HW_ERR_SIZE)},
	{"fatal_bus_error_interrupts", SYN_STAT(hw_errs[0]) + (3 * HW_ERR_SIZE)},
	{"rx_buffer_unavailable_interrupts", SYN_STAT(hw_errs[0]) + (4 * HW_ERR_SIZE)},
	{"rx_process_stopped_interrupts", SYN_STAT(hw_errs[0]) + (5 * HW_ERR_SIZE)},
	{"tx_underflow_interrupts", SYN_STAT(hw_errs[0]) + (6 * HW_ERR_SIZE)},
	{"rx_overflow_interrupts", SYN_STAT(hw_errs[0]) + (7 * HW_ERR_SIZE)},
	{"tx_jabber_timeout_interrutps", SYN_STAT(hw_errs[0]) + (8 * HW_ERR_SIZE)},
	{"tx_process_stopped_interrutps", SYN_STAT(hw_errs[0]) + (9 * HW_ERR_SIZE)},
};

/*
 * Array of strings describing statistics
 */
static const struct syn_ethtool_stats syn_gstrings_mib_stats[] = {
	{"rx_broadcast", SYN_MIB_STAT(RxBroad)},
	{"rx_pause", SYN_MIB_STAT(RxPause)},
	{"rx_multicast", SYN_MIB_STAT(RxMulti)},
	{"rx_fcserr", SYN_MIB_STAT(RxFcsErr)},
	{"rx_alignerr", SYN_MIB_STAT(RxAllignErr)},
	{"rx_runt", SYN_MIB_STAT(RxRunt)},
	{"rx_frag", SYN_MIB_STAT(RxFragment)},
	{"rx_pkt64", SYN_MIB_STAT(Rx64Byte)},
	{"rx_pkt65to127", SYN_MIB_STAT(Rx128Byte)},
	{"rx_pkt128to255", SYN_MIB_STAT(Rx256Byte)},
	{"rx_pkt256to511", SYN_MIB_STAT(Rx512Byte)},
	{"rx_pkt512to1023", SYN_MIB_STAT(Rx1024Byte)},
	{"rx_pkt1024to1518", SYN_MIB_STAT(Rx1518Byte)},
	{"rx_pkt1519tox", SYN_MIB_STAT(RxMaxByte)},
	{"rx_toolong", SYN_MIB_STAT(RxTooLong)},
	{"rx_pktgoodbyte", SYN_MIB_STAT(RxGoodByte)},
	{"rx_pktbadbyte", SYN_MIB_STAT(RxBadByte)},
	{"rx_overflow", SYN_MIB_STAT(RxOverFlow)},
	{"filtered", SYN_MIB_STAT(Filtered)},
	{"tx_broadcast", SYN_MIB_STAT(TxBroad)},
	{"tx_pause", SYN_MIB_STAT(TxPause)},
	{"tx_multicast", SYN_MIB_STAT(TxMulti)},
	{"tx_underrun", SYN_MIB_STAT(TxUnderRun)},
	{"tx_pkt64", SYN_MIB_STAT(Tx64Byte)},
	{"tx_pkt65to127", SYN_MIB_STAT(Tx128Byte)},
	{"tx_pkt128to255", SYN_MIB_STAT(Tx256Byte)},
	{"tx_pkt256to511", SYN_MIB_STAT(Tx512Byte)},
	{"tx_pkt512to1023", SYN_MIB_STAT(Tx1024Byte)},
	{"tx_pkt1024to1518", SYN_MIB_STAT(Tx1518Byte)},
	{"tx_pkt1519tox", SYN_MIB_STAT(TxMaxByte)},
	{"tx_oversize", SYN_MIB_STAT(TxOverSize)},
	{"tx_pktbyte_h", SYN_MIB_STAT(TxByte)},
	{"tx_collisions", SYN_MIB_STAT(TxCollision)},
	{"tx_abortcol", SYN_MIB_STAT(TxAbortCol)},
	{"tx_multicol", SYN_MIB_STAT(TxMultiCol)},
	{"tx_singlecol", SYN_MIB_STAT(TxSingalCol)},
	{"tx_exesdeffer", SYN_MIB_STAT(TxExcDefer)},
	{"tx_deffer", SYN_MIB_STAT(TxDefer)},
	{"tx_latecol", SYN_MIB_STAT(TxLateCol)},
	{"rx_unicast", SYN_MIB_STAT(RxUniCast)},
	{"tx_unicast", SYN_MIB_STAT(TxUniCast)},
	{"rx_jumbofcserr", SYN_MIB_STAT(RxJumboFcsErr)},
	{"rx_jumboalignerr", SYN_MIB_STAT(RxJumboAligenErr)},
	{"rx_14to63", SYN_MIB_STAT(Rx14To63)},
	{"rx_toolongbyte", SYN_MIB_STAT(RxTooLongByte)},
	{"rx_runtbyte", SYN_MIB_STAT(RxRuntByte)},
};

#define SYN_STATS_LEN	ARRAY_SIZE(syn_gstrings_stats)
#define SYN_STATS_MIB_STATS_LEN	ARRAY_SIZE(syn_gstrings_mib_stats)

/*
 * syn_enable_mac_cst()
 *	Enable stripping of MAC padding/FCS
 */
static void syn_enable_mac_cst(struct nss_gmac_hal_dev *nghd)
{
	printk(KERN_INFO "enable stripping of mac padding/fcs start %d", hal_check_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_ENABLE));
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_ENABLE);
	printk(KERN_INFO "enable stripping of mac padding/fcs end %d", hal_check_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_ENABLE));
}

/*
 * syn_disable_mac_cst()
 *	Disable stripping of MAC padding/FCS
 */
static struct nss_gmac_hal_dev *s_nghd;
static void syn_disable_mac_cst(struct nss_gmac_hal_dev *nghd)
{
	printk(KERN_INFO "disable stripping of mac padding/fcs start %d", hal_check_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_ENABLE));
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_DISABLE);
	printk(KERN_INFO "disable stripping of mac padding/fcs end %d", hal_check_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_CST_ENABLE));
}

void disable_mac_cst(void)
{
	syn_enable_mac_cst(s_nghd);
}

/*
 * syn_set_rx_flow_ctrl()
 */
static void syn_set_rx_flow_ctrl(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_FLOW_CONTROL,
			SYN_MAC_FC_RX_FLOW_CONTROL);
}

/*
 * syn_clear_rx_flow_ctrl()
 */
static void syn_clear_rx_flow_ctrl(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_FLOW_CONTROL,
			SYN_MAC_FC_RX_FLOW_CONTROL);

}

/*
 * syn_set_tx_flow_ctrl()
 */
static void syn_set_tx_flow_ctrl(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_FLOW_CONTROL,
			SYN_MAC_FC_TX_FLOW_CONTROL);
}

/*
 * syn_send_tx_pause_frame()
 */
static void syn_send_tx_pause_frame(struct nss_gmac_hal_dev *nghd)
{
	syn_set_tx_flow_ctrl(nghd);
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_FLOW_CONTROL,
			SYN_MAC_FC_SEND_PAUSE_FRAME);
}

/*
 * syn_clear_tx_flow_ctrl()
 */
static void syn_clear_tx_flow_ctrl(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_FLOW_CONTROL,
			SYN_MAC_FC_TX_FLOW_CONTROL);
}

/*
 * syn_rx_enable()
 */
static void syn_rx_enable(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_RX);
}

/*
 * syn_tx_enable()
 */
static void syn_tx_enable(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_TX);
}

/*
 * syn_rx_disable()
 */
static void syn_rx_disable(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_RX);
}

/*
 * syn_tx_disable()
 */
static void syn_tx_disable(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_CONFIGURATION, SYN_MAC_TX);
}

/************Ip checksum offloading APIs*************/

/*
 * syn_enable_rx_chksum_offload()
 *	Enable IPv4 header and IPv4/IPv6 TCP/UDP checksum calculation by GMAC.
 */
static void syn_enable_rx_chksum_offload(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base,
			      SYN_MAC_CONFIGURATION, SYN_MAC_RX_IPC_OFFLOAD);
}

/*
 * syn_disable_rx_chksum_offload()
 *	Disable the IP checksum offloading in receive path.
 */
static void syn_disable_rx_chksum_offload(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base,
				SYN_MAC_CONFIGURATION, SYN_MAC_RX_IPC_OFFLOAD);
}

/*******************Ip checksum offloading APIs**********************/

/*
 * syn_ipc_offload_init()
 *	Initialize IPC Checksum offloading.
 */
static void syn_ipc_offload_init(struct nss_gmac_hal_dev *nghd)
{
	struct nss_dp_dev *dp_priv;
	dp_priv = netdev_priv(nghd->netdev);

	if (test_bit(__NSS_DP_RXCSUM, &dp_priv->flags)) {
		/*
		 * Enable the offload engine in the receive path
		 */
		syn_enable_rx_chksum_offload(nghd);
		printk(KERN_INFO "%s: enable Rx checksum\n", __func__);
	} else {
		syn_disable_rx_chksum_offload(nghd);
		printk(KERN_INFO "%s: disable Rx checksum\n", __func__);
	}
}

/*
 * syn_disable_mac_interrupt()
 *	Disable all the interrupts.
 */
static void syn_disable_mac_interrupt(struct nss_gmac_hal_dev *nghd)
{
	hal_write_relaxed_reg(nghd->mac_base, SYN_INTERRUPT_MASK, 0xffffffff);
}

/*
 * syn_disable_mmc_tx_interrupt()
 *	Disable the MMC Tx interrupt.
 *
 * The MMC tx interrupts are masked out as per the mask specified.
 */
static void syn_disable_mmc_tx_interrupt(struct nss_gmac_hal_dev *nghd,
						uint32_t mask)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MMC_TX_INTERRUPT_MASK, mask);
}

/*
 * syn_disable_mmc_rx_interrupt()
 *	Disable the MMC Rx interrupt.
 *
 * The MMC rx interrupts are masked out as per the mask specified.
 */
static void syn_disable_mmc_rx_interrupt(struct nss_gmac_hal_dev *nghd,
						uint32_t mask)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MMC_RX_INTERRUPT_MASK, mask);
}

/*
 * syn_disable_mmc_ipc_rx_interrupt()
 *	Disable the MMC ipc rx checksum offload interrupt.
 *
 * The MMC ipc rx checksum offload interrupts are masked out as
 * per the mask specified.
 */
static void syn_disable_mmc_ipc_rx_interrupt(struct nss_gmac_hal_dev *nghd,
					   uint32_t mask)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MMC_IPC_RX_INTR_MASK, mask);
}

/*
 * syn_disable_interrupt_all()
 *	Disable all the interrupts.
 */
static void syn_disable_interrupt_all(struct nss_gmac_hal_dev *nghd)
{
	syn_disable_mac_interrupt(nghd);
	syn_disable_mmc_tx_interrupt(nghd, 0xFFFFFFFF);
	syn_disable_mmc_rx_interrupt(nghd, 0xFFFFFFFF);
	syn_disable_mmc_ipc_rx_interrupt(nghd, 0xFFFFFFFF);
}

/*
 * syn_broadcast_enable()
 *	Enables Broadcast frames.
 *
 * When enabled Address filtering module passes all incoming broadcast frames.
 */
static void syn_broadcast_enable(struct nss_gmac_hal_dev *nghd)
{
	hal_clear_reg_bits(nghd->mac_base, SYN_MAC_FRAME_FILTER, SYN_MAC_BROADCAST);
}

/*
 * syn_multicast_enable()
 *	Enables Multicast frames.
 *
 * When enabled all multicast frames are passed.
 */
static void syn_multicast_enable(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_FRAME_FILTER, SYN_MAC_MULTICAST_FILTER);
}

/*
 * syn_promisc_enable()
 *	Enables promiscous mode.
 *
 * When enabled Address filter modules pass all incoming frames
 * regardless of their Destination and source addresses.
 */
static void syn_promisc_enable(struct nss_gmac_hal_dev *nghd)
{
	hal_set_reg_bits(nghd->mac_base, SYN_MAC_FRAME_FILTER,
				SYN_MAC_PROMISCUOUS_MODE_ON);
}

/*
 * syn_rx_flow_control()
 */
static void syn_rx_flow_control(struct nss_gmac_hal_dev *nghd,
							bool enabled)
{
	BUG_ON(nghd == NULL);

	if (enabled) {
		syn_set_rx_flow_ctrl(nghd);
	} else {
		syn_clear_rx_flow_ctrl(nghd);
	}
}

/*
 * syn_tx_flow_control()
 */
static void syn_tx_flow_control(struct nss_gmac_hal_dev *nghd,
							bool enabled)
{
	BUG_ON(nghd == NULL);

	if (enabled) {
		syn_set_tx_flow_ctrl(nghd);
	} else {
		syn_clear_tx_flow_ctrl(nghd);
	}
}

/*
 * syn_get_max_frame_size()
 */
static int32_t syn_get_max_frame_size(struct nss_gmac_hal_dev *nghd)
{
	int ret;
	uint32_t mtu;

	BUG_ON(nghd == NULL);

	ret = fal_port_max_frame_size_get(0, nghd->mac_id, &mtu);

	if (!ret) {
		return mtu;
	}

	return ret;
}

/*
 * syn_set_max_frame_size()
 */
static int32_t syn_set_max_frame_size(struct nss_gmac_hal_dev *nghd,
							uint32_t val)
{
	uint16_t frame_sz;
	BUG_ON(nghd == NULL);

	if (val > SYN_HAL_MAX_MTU_SIZE) {
		netdev_warn(nghd->netdev, "Maximum allowed MTU: %d\n",
							SYN_HAL_MAX_MTU_SIZE);
		return -1;
	}

	frame_sz = val + SYN_HAL_MTU_L2_OVERHEAD;
	return fal_port_max_frame_size_set(0, nghd->mac_id, frame_sz);
}

/*
 * syn_get_mib_stats()
 */
int syn_get_mib_stats(struct nss_gmac_hal_dev *nghd, fal_mib_counter_t *stats)
{
	if (fal_mib_counter_get(0, nghd->mac_id, stats) < 0) {
		return -1;
	}

	return 0;
}

/*
 * syn_get_netdev_stats()
 */
static int32_t syn_get_netdev_stats(struct nss_gmac_hal_dev *nghd,
		struct rtnl_link_stats64 *stats)
{
	fal_mib_counter_t hal_stats;

	BUG_ON(nghd == NULL);

	memset(&hal_stats, 0, sizeof(fal_mib_counter_t));
	if (syn_get_mib_stats(nghd, &hal_stats)) {
		return -1;
	}

	stats->rx_packets = hal_stats.RxUniCast + hal_stats.RxBroad
				+ hal_stats.RxMulti;
	stats->tx_packets = hal_stats.TxUniCast + hal_stats.TxBroad
				+ hal_stats.TxMulti;
	stats->rx_bytes = hal_stats.RxGoodByte;
	stats->tx_bytes = hal_stats.TxByte;
	stats->collisions = hal_stats.TxCollision;
	stats->multicast = hal_stats.RxMulti;

	/*
	 * RX errors
	 */
	stats->rx_crc_errors = hal_stats.RxFcsErr + hal_stats.RxJumboFcsErr;
	stats->rx_frame_errors = hal_stats.RxAllignErr +
				 hal_stats.RxJumboAligenErr + hal_stats.RxRunt;
	stats->rx_fifo_errors = hal_stats.RxOverFlow;
	stats->rx_errors = stats->rx_crc_errors + stats->rx_frame_errors +
			   stats->rx_fifo_errors;

	stats->rx_dropped = hal_stats.RxTooLong + stats->rx_errors;

	/*
	 * TX errors
	 */
	stats->tx_fifo_errors = hal_stats.TxUnderRun + hal_stats.RxOverFlow;
	stats->tx_aborted_errors = hal_stats.TxAbortCol;
	stats->tx_errors = stats->tx_fifo_errors + stats->tx_aborted_errors;

	return 0;
}

/*
 * syn_get_eth_stats()
 */
static int32_t syn_get_eth_stats(struct nss_gmac_hal_dev *nghd,
					uint64_t *data,
					struct nss_dp_gmac_stats *stats)
{
	int i, i_mib;
	fal_mib_counter_t mib_stats;
	uint8_t *p = NULL;

	BUG_ON(nghd == NULL);

	/*
	 * Populate data plane statistics.
	 */
	for (i = 0; i < SYN_STATS_LEN; i++) {
		p = ((uint8_t *)(stats) + syn_gstrings_stats[i].stat_offset);
		data[i] = *(uint64_t *)p;
	}

	/*
	 * Get MIB statistics
	 */
	memset(&mib_stats, 0, sizeof(fal_mib_counter_t));
	if (syn_get_mib_stats(nghd, &mib_stats)) {
		return -1;
	}

	/*
	 * Populate MIB statistics
	 */
	for (i_mib = 0; i_mib < SYN_STATS_MIB_STATS_LEN; i_mib++) {
		p = ((uint8_t *)(&mib_stats) +
				syn_gstrings_mib_stats[i_mib].stat_offset);
		i = SYN_STATS_LEN + i_mib;
		data[i] = *(uint64_t *)p;
	}

	return 0;
}

/*
 * syn_get_strset_count()
 */
static int32_t syn_get_strset_count(struct nss_gmac_hal_dev *nghd,
					int32_t sset)
{
	struct net_device *netdev;

	BUG_ON(nghd == NULL);

	netdev = nghd->netdev;

	switch (sset) {
	case ETH_SS_STATS:
		return (SYN_STATS_LEN + SYN_STATS_MIB_STATS_LEN);
	}

	netdev_dbg(netdev, "%s: Invalid string set\n", __func__);
	return -EPERM;
}

/*
 * syn_get_strings()
 */
static int32_t syn_get_strings(struct nss_gmac_hal_dev *nghd,
					int32_t stringset, uint8_t *data)
{
	struct net_device *netdev;
	int i;

	BUG_ON(nghd == NULL);

	netdev = nghd->netdev;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < SYN_STATS_LEN; i++) {
			memcpy(data, syn_gstrings_stats[i].stat_string,
					ETH_GSTRING_LEN);
			data += ETH_GSTRING_LEN;
		}

		for (i = 0; i < SYN_STATS_MIB_STATS_LEN; i++) {
			memcpy(data, syn_gstrings_mib_stats[i].stat_string,
					ETH_GSTRING_LEN);
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
 * syn_send_pause_frame()
 */
static void syn_send_pause_frame(struct nss_gmac_hal_dev *nghd)
{
	BUG_ON(nghd == NULL);

	syn_send_tx_pause_frame(nghd);
}

/*
 * syn_set_mac_address()
 */
static void syn_set_mac_address(struct nss_gmac_hal_dev *nghd,
							uint8_t *macaddr)
{
	uint32_t data;

	BUG_ON(nghd == NULL);

	if (!macaddr) {
		netdev_warn(nghd->netdev, "macaddr is not valid.\n");
		return;
	}

	data = (macaddr[5] << 8) | macaddr[4] | SYN_MAC_ADDR_HIGH_AE;
	hal_write_relaxed_reg(nghd->mac_base, SYN_MAC_ADDR0_HIGH, data);
	data = (macaddr[3] << 24) | (macaddr[2] << 16) | (macaddr[1] << 8)
		| macaddr[0];
	hal_write_relaxed_reg(nghd->mac_base, SYN_MAC_ADDR0_LOW, data);
}

/*
 * syn_get_mac_address()
 */
static void syn_get_mac_address(struct nss_gmac_hal_dev *nghd,
							uint8_t *macaddr)
{
	uint32_t data;

	BUG_ON(nghd == NULL);

	if (!macaddr) {
		netdev_warn(nghd->netdev, "macaddr is not valid.\n");
		return;
	}

	data = hal_read_relaxed_reg(nghd->mac_base, SYN_MAC_ADDR0_HIGH);
	macaddr[5] = (data >> 8) & 0xff;
	macaddr[4] = (data) & 0xff;

	data = hal_read_relaxed_reg(nghd->mac_base, SYN_MAC_ADDR0_LOW);
	macaddr[3] = (data >> 24) & 0xff;
	macaddr[2] = (data >> 16) & 0xff;
	macaddr[1] = (data >> 8) & 0xff;
	macaddr[0] = (data) & 0xff;
}

/*
 * syn_gmac_clk_enable
 *	Function to enable GCC_SNOC_GMAC_AXI_CLK.
 *
 * These clocks are required for GMAC operations.
 */
static void syn_gmac_clk_enable(struct nss_gmac_hal_dev *nghd)
{
	struct net_device *ndev = nghd->netdev;
	struct nss_dp_dev *dp_priv = netdev_priv(ndev);
	struct platform_device *pdev = dp_priv->pdev;
	struct device *dev = &pdev->dev;
	struct clk *gmac_clk = NULL;
	int err;

	gmac_clk = devm_clk_get(dev, SYN_GMAC_SNOC_GMAC_AXI_CLK);
	if (IS_ERR(gmac_clk)) {
		pr_err("%s: cannot get clock: %s\n", __func__,
						SYN_GMAC_SNOC_GMAC_AXI_CLK);
		return;
	}

	err = clk_prepare_enable(gmac_clk);
	if (err) {
		pr_err("%s: cannot enable clock: %s, err: %d\n", __func__,
						SYN_GMAC_SNOC_GMAC_AXI_CLK, err);
		return;
	}
}

/*
 * syn_start()
 */
static int32_t syn_start(struct nss_gmac_hal_dev *nghd)
{
	struct nss_dp_dev *dp_dev;

	BUG_ON(nghd == NULL);

	syn_tx_enable(nghd);
	syn_rx_enable(nghd);
	dp_dev = (struct nss_dp_dev *)netdev_priv(nghd->netdev);

	/*
	 * TODO: Enable MAC CST stripping for NSS mode as well. We need
	 * NSS change for that.
	 */
	if (!(dp_dev->drv_flags & NSS_DP_PRIV_FLAG(INIT_OVERRIDE))) {
		syn_enable_mac_cst(nghd);
	}

	netdev_dbg(nghd->netdev, "%s: mac_base:0x%px MAC Config:0x%x\n",
		__func__, nghd->mac_base,
		hal_read_relaxed_reg(nghd->mac_base, SYN_MAC_CONFIGURATION));

	return 0;
}

/*
 * syn_stop()
 */
static int32_t syn_stop(struct nss_gmac_hal_dev *nghd)
{
	struct nss_dp_dev *dp_dev;

	BUG_ON(nghd == NULL);

	syn_tx_disable(nghd);
	syn_rx_disable(nghd);
	dp_dev = (struct nss_dp_dev *)netdev_priv(nghd->netdev);

	/*
	 * TODO: Disable MAC CST stripping for NSS mode as well. We need
	 * NSS change for that.
	 */
	if (!(dp_dev->drv_flags & NSS_DP_PRIV_FLAG(INIT_OVERRIDE))) {
		syn_disable_mac_cst(nghd);
	}


	netdev_dbg(nghd->netdev, "%s: mac_base:0x%px MAC Config:0x%x\n",
		__func__, nghd->mac_base,
		hal_read_relaxed_reg(nghd->mac_base, SYN_MAC_CONFIGURATION));

	return 0;
}

/*
 * syn_init()
 */
static void *syn_init(struct nss_gmac_hal_platform_data *gmacpdata)
{
	struct syn_hal_dev *shd = NULL;
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

	shd = (struct syn_hal_dev *)devm_kzalloc(&dp_priv->pdev->dev,
					sizeof(struct syn_hal_dev),
					GFP_KERNEL);
	if (!shd) {
		netdev_dbg(ndev, "kzalloc failed. Returning...\n");
		return NULL;
	}

	shd->nghd.mac_reg_len = resource_size(res);
	shd->nghd.memres = devm_request_mem_region(&dp_priv->pdev->dev,
							res->start,
							resource_size(res),
							ndev->name);
	if (!shd->nghd.memres) {
		netdev_dbg(ndev, "Request mem region failed. Returning.\n");
		devm_kfree(&dp_priv->pdev->dev, shd);
		return NULL;
	}

	/*
	 * Save netdev context in syn HAL context
	 */
	shd->nghd.netdev = gmacpdata->netdev;
	shd->nghd.mac_id = gmacpdata->macid;
	shd->nghd.duplex_mode = DUPLEX_FULL;

	set_bit(__NSS_DP_RXCSUM, &dp_priv->flags);

	/*
	 * Populate the mac base addresses
	 */
	shd->nghd.mac_base =
		devm_ioremap(&dp_priv->pdev->dev, res->start,
							resource_size(res));
	if (!shd->nghd.mac_base) {
		netdev_dbg(ndev, "ioremap fail.\n");
		devm_release_mem_region(&dp_priv->pdev->dev,
				shd->nghd.memres->start,
				shd->nghd.mac_reg_len);
		devm_kfree(&dp_priv->pdev->dev, shd);
		return NULL;
	}

	spin_lock_init(&shd->nghd.slock);

	netdev_dbg(ndev, "ioremap OK.Size 0x%x Ndev base 0x%lx macbase 0x%px\n",
			gmacpdata->reg_len,
			ndev->base_addr,
			shd->nghd.mac_base);

	syn_disable_interrupt_all(&shd->nghd);

	/*
	 * Enable SoC specific GMAC clocks.
	 */
	syn_gmac_clk_enable(&shd->nghd);

	syn_ipc_offload_init(&shd->nghd);
	syn_promisc_enable(&shd->nghd);
	syn_broadcast_enable(&shd->nghd);
	syn_multicast_enable(&shd->nghd);
	s_nghd = &shd->nghd;
//	syn_disable_mac_cst(&shd->nghd);

	/*
	 * Reset MIB Stats
	 */
	if (fal_mib_port_flush_counters(0, shd->nghd.mac_id)) {
		netdev_dbg(ndev, "MIB stats Reset fail.\n");
	}

	return (struct nss_gmac_hal_dev *)shd;
}

/*
 * syn_exit()
 */
static void syn_exit(struct nss_gmac_hal_dev *nghd)
{
	struct nss_dp_dev *dp_priv = NULL;
	struct syn_hal_dev *shd = (struct syn_hal_dev *)nghd;

	netdev_dbg(nghd->netdev, "Freeing up dev memory.\n");

	dp_priv = netdev_priv(nghd->netdev);
	devm_iounmap(&dp_priv->pdev->dev,
			(void *)nghd->mac_base);
	devm_release_mem_region(&dp_priv->pdev->dev,
			(nghd->memres)->start,
			nghd->mac_reg_len);

	nghd->memres = NULL;
	nghd->mac_base = NULL;

	devm_kfree(&dp_priv->pdev->dev, shd);
}

/*
 * MAC hal_ops base structure
 */
struct nss_gmac_hal_ops syn_gmac_ops = {
	.init = &syn_init,
	.start =  &syn_start,
	.stop = &syn_stop,
	.exit = &syn_exit,
	.setmacaddr = &syn_set_mac_address,
	.getmacaddr = &syn_get_mac_address,
	.rxflowcontrol = &syn_rx_flow_control,
	.txflowcontrol = &syn_tx_flow_control,
	.setmaxframe = &syn_set_max_frame_size,
	.getmaxframe = &syn_get_max_frame_size,
	.getndostats = &syn_get_netdev_stats,
	.getssetcount = &syn_get_strset_count,
	.getstrings = &syn_get_strings,
	.getethtoolstats = &syn_get_eth_stats,
	.sendpause = &syn_send_pause_frame,
};
