/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "nss_core.h"
#include "nss_data_plane_hal.h"

static DEFINE_SPINLOCK(nss_data_plane_hal_gmac_stats_lock);

/*
 * nss_data_plane_hal_get_stats()
 *	Called by nss-dp to get GMAC stats
 */
static void nss_data_plane_hal_get_stats(struct nss_dp_data_plane_ctx *dpc,
					struct nss_dp_gmac_stats *stats)
{
	struct nss_data_plane_param *dp = (struct nss_data_plane_param *)dpc;

	spin_lock_bh(&nss_data_plane_hal_gmac_stats_lock);
	memcpy(stats, &dp->gmac_stats, sizeof(*stats));
	spin_unlock_bh(&nss_data_plane_hal_gmac_stats_lock);
}

/*
 * nss_data_plane_hal_add_dp_ops()
 */
void nss_data_plane_hal_add_dp_ops(struct nss_dp_data_plane_ops *dp_ops)
{
	dp_ops->get_stats = nss_data_plane_hal_get_stats;
}

/*
 * nss_data_plane_hal_register()
 */
void nss_data_plane_hal_register(struct nss_ctx_instance *nss_ctx)
{
}

/*
 * nss_data_plane_hal_unregister()
 */
void nss_data_plane_hal_unregister(struct nss_ctx_instance *nss_ctx)
{
}

/*
 * nss_data_plane_hal_set_features
 */
void nss_data_plane_hal_set_features(struct nss_dp_data_plane_ctx *dpc)
{
	dpc->dev->features |= NSS_DATA_PLANE_SUPPORTED_FEATURES;
	dpc->dev->hw_features |= NSS_DATA_PLANE_SUPPORTED_FEATURES;
	dpc->dev->wanted_features |= NSS_DATA_PLANE_SUPPORTED_FEATURES;

	/*
	 * We advertise checksum offload for VLANs.
	 * Synopsys GMAC does not support checksum offload for QinQ VLANs.
	 * However, we are dependent on netdev ops ndo_features_check to block
	 * QinQ VLAN TSO/checksum offload.
	 */
	dpc->dev->vlan_features |= NSS_DATA_PLANE_SUPPORTED_FEATURES;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	dpc->dev->vlan_features &= ~NETIF_F_UFO;
#endif
}

/*
 * nss_data_plane_hal_stats_sync()
 */
void nss_data_plane_hal_stats_sync(struct nss_data_plane_param *ndpp,
					struct nss_phys_if_stats *stats)
{
	struct nss_dp_hal_gmac_stats *gmac_stats = &ndpp->gmac_stats.stats;

	spin_lock_bh(&nss_data_plane_hal_gmac_stats_lock);

	gmac_stats->rx_stats.rx_bytes += stats->if_stats.rx_bytes;
	gmac_stats->rx_stats.rx_packets += stats->if_stats.rx_packets;
	gmac_stats->rx_stats.rx_errors += stats->estats.rx_errors;
	gmac_stats->rx_stats.rx_descriptor_errors += stats->estats.rx_descriptor_errors;
	gmac_stats->rx_stats.rx_late_collision_errors += stats->estats.rx_late_collision_errors;
	gmac_stats->rx_stats.rx_dribble_bit_errors += stats->estats.rx_dribble_bit_errors;
	gmac_stats->rx_stats.rx_length_errors += stats->estats.rx_length_errors;
	gmac_stats->rx_stats.rx_ip_header_errors += stats->estats.rx_ip_header_errors;
	gmac_stats->rx_stats.rx_ip_payload_errors += stats->estats.rx_ip_payload_errors;
	gmac_stats->rx_stats.rx_no_buffer_errors += stats->estats.rx_no_buffer_errors;
	gmac_stats->rx_stats.rx_transport_csum_bypassed += stats->estats.rx_transport_csum_bypassed;
	gmac_stats->rx_stats.rx_missed += stats->estats.rx_missed;
	gmac_stats->rx_stats.rx_fifo_overflows += stats->estats.fifo_overflows;
	gmac_stats->rx_stats.rx_scatter_errors += stats->estats.rx_scatter_errors;

	gmac_stats->tx_stats.tx_bytes += stats->if_stats.tx_bytes;
	gmac_stats->tx_stats.tx_packets += stats->if_stats.tx_packets;
	gmac_stats->tx_stats.tx_collisions += stats->estats.tx_collisions;
	gmac_stats->tx_stats.tx_errors += stats->estats.tx_errors;
	gmac_stats->tx_stats.tx_jabber_timeout_errors += stats->estats.tx_jabber_timeout_errors;
	gmac_stats->tx_stats.tx_frame_flushed_errors += stats->estats.tx_frame_flushed_errors;
	gmac_stats->tx_stats.tx_loss_of_carrier_errors += stats->estats.tx_loss_of_carrier_errors;
	gmac_stats->tx_stats.tx_no_carrier_errors += stats->estats.tx_no_carrier_errors;
	gmac_stats->tx_stats.tx_late_collision_errors += stats->estats.tx_late_collision_errors;
	gmac_stats->tx_stats.tx_excessive_collision_errors += stats->estats.tx_excessive_collision_errors;
	gmac_stats->tx_stats.tx_excessive_deferral_errors += stats->estats.tx_excessive_deferral_errors;
	gmac_stats->tx_stats.tx_underflow_errors += stats->estats.tx_underflow_errors;
	gmac_stats->tx_stats.tx_ip_header_errors += stats->estats.tx_ip_header_errors;
	gmac_stats->tx_stats.tx_ip_payload_errors += stats->estats.tx_ip_payload_errors;
	gmac_stats->tx_stats.tx_dropped += stats->estats.tx_dropped;
	gmac_stats->tx_stats.tx_ts_create_errors += stats->estats.tx_ts_create_errors;

	gmac_stats->hw_errs[0] += stats->estats.hw_errs[0];
	gmac_stats->hw_errs[1] += stats->estats.hw_errs[1];
	gmac_stats->hw_errs[2] += stats->estats.hw_errs[2];
	gmac_stats->hw_errs[3] += stats->estats.hw_errs[3];
	gmac_stats->hw_errs[4] += stats->estats.hw_errs[4];
	gmac_stats->hw_errs[5] += stats->estats.hw_errs[5];
	gmac_stats->hw_errs[6] += stats->estats.hw_errs[6];
	gmac_stats->hw_errs[7] += stats->estats.hw_errs[7];
	gmac_stats->hw_errs[8] += stats->estats.hw_errs[8];
	gmac_stats->hw_errs[9] += stats->estats.hw_errs[9];

	spin_unlock_bh(&nss_data_plane_hal_gmac_stats_lock);
}

/*
 * nss_data_plane_hal_get_mtu_sz()
 */
uint16_t nss_data_plane_hal_get_mtu_sz(uint16_t mtu)
{
	/*
	 * Return MTU value as is.
	 */
	return mtu;
}
