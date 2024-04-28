/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_ovpnmgr_tun.h
 */
#ifndef _NSS_OVPNMGR_TUN_H
#define _NSS_OVPNMGR_TUN_H

#define NSS_OVPNMGR_TUN_NAME "ovpn%d"

/*
 * nss_ovpnmgr_tun_ctx_stats
 *	Tunnel statistics received from NSS firmware.
 */
struct nss_ovpnmgr_tun_ctx_stats {
	uint64_t exception[NSS_QVPN_EXCEPTION_EVENT_MAX];	/* Exception statistics. */
	uint64_t fail_crypto[NSS_CRYPTO_CMN_RESP_ERROR_MAX];	/* Crypto response errors. */
	uint64_t fail_offload[NSS_QVPN_PKT_DROP_EVENT_MAX];	/* Packets drop events.*/
	uint64_t rx_dropped[NSS_MAX_NUM_PRI];			/* Packets dropped on receive due to queue full. */
	uint64_t rx_packets;					/* Number of packets received. */
	uint64_t rx_bytes;					/* Number of bytes received. */
	uint64_t tx_packets;					/* Number of packets transmitted. */
	uint64_t tx_bytes;					/* Number of bytes transmitted. */
	uint64_t host_pkt_drop;					/* Packet drops in host. */
};

/*
 * nss_ovpnmgr_tun_ctx
 *	Tunnel context.
 */
struct nss_ovpnmgr_tun_ctx {
	struct nss_ctx_instance *nss_ctx;	/* OVPN nss context. */
	struct nss_ovpnmgr_crypto_ctx active;	/* Crypto context for active key. */
	struct nss_ovpnmgr_crypto_ctx expiring;	/* Crypto context for expiring key. */
	struct nss_ovpnmgr_tun_ctx_stats stats;	/* Tunnel statistics. */
	int32_t ifnum;				/* Dynamic interface for OVPN encapsulation. */
	uint32_t di_type;			/* Dynamic interface type. */
};

/*
 * nss_ovpnmgr_tun
 *	Tunnel instance.
 */
struct nss_ovpnmgr_tun {
	struct list_head list;					/* List of OVPN Tunnels created by the application. */
	struct nss_ovpnmgr_app *app;				/* Application reference. */
	struct nss_ovpnmgr_tun_tuple tun_hdr;			/* OVPN tunnel header parameters. */
	struct nss_ovpnmgr_tun_config tun_cfg;			/* OVPN tunnel header parameters. */
	struct nss_ovpnmgr_tun_ctx inner;			/* Inner tunnel context. */
	struct nss_ovpnmgr_tun_ctx outer;			/* Outer tunnel context. */
	struct nss_qvpn_hdr_configure_msg ovpn_hdr;		/* OVPN header for QVPN configuration. */
	struct nss_ctx_instance *nss_ctx;			/* OVPN NSS context. */
	struct nss_ovpnmgr_tun_stats stats;			/* OVPN tunnel statistics. */
	struct list_head route_list;				/* List of Routes configured in this tunnel. */
	uint32_t tunnel_id;					/* Tunnel id assigned during tunnel addition. */
	struct net_device *dev;					/* Tunnel netdev. */
};

static inline void nss_ovpnmgr_ipv6_addr_ntohl(uint32_t *host, uint32_t *net)
{
	host[0] = ntohl(net[0]);
	host[1] = ntohl(net[1]);
	host[2] = ntohl(net[2]);
	host[3] = ntohl(net[3]);
}

void nss_ovpnmgr_tun_get_stats(struct net_device *app_dev, struct rtnl_link_stats64 *stats);
#endif /* _NSS_OVPNMGR_TUN_H */
