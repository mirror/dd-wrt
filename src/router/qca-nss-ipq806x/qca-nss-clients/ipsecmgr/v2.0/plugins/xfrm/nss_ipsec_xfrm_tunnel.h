/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_IPSEC_XFRM_TUNNEL_H
#define __NSS_IPSEC_XFRM_TUNNEL_H

/*
 * 20(IPv4 Header) + 8(ESP Header) + 16(Max IV len) + 8(UDP header for NATT tunnels) +
 * 2(NSS_IPSECMGR_ESP_TRAIL_SZ) + 14(NSS_IPSECMGR_ESP_PAD_SZ) + 16(Max ICV len).
 */
#define NSS_IPSEC_XFRM_TUNNEL_V4_MAX_OVERHEAD 84

/*
 * 40(IPv6 Header) + 8(ESP Header) + 16(Max IV len) + 2(NSS_IPSECMGR_ESP_TRAIL_SZ) +
 * 14(NSS_IPSECMGR_ESP_PAD_SZ) + 16(Max ICV len).
 */
#define NSS_IPSEC_XFRM_TUNNEL_V6_MAX_OVERHEAD 96

/*
 * Forward Declarations
 */
struct nss_ipsec_xfrm_drv;
struct nss_ipsec_xfrm_sa;

/*
 * NSS IPSec xfrm Tunnel obj
 */
struct nss_ipsec_xfrm_tunnel {
	struct nss_ipsec_xfrm_drv *drv;		/* Pointer to IPSec xfrm plugin instance */
	struct work_struct free_work;		/* Tunnel free work */
	struct list_head list_entry;		/* Hash DB entry */
	struct net_device *dev;			/* Pointer to corrosponding NSS device */

	xfrm_address_t remote;			/* Remote endpoint Address in networks order */
	xfrm_address_t local;			/* Local endpoint Address in network order */
	atomic_t default_spi;			/* spi idx for the default outer SA */

	struct kref ref;			/* Ref Count */
	uint16_t family;			/* AF_INET or AF_INET6 */
	atomic_t num_sa;			/* Number of Active SAs */
	uint8_t ttl;				/* TTL or hop limit for this tunnel */
};

void nss_ipsec_xfrm_tunnel_deref(struct nss_ipsec_xfrm_tunnel *tun);
struct nss_ipsec_xfrm_tunnel *nss_ipsec_xfrm_tunnel_ref(struct nss_ipsec_xfrm_tunnel *tun);
bool nss_ipsec_xfrm_tunnel_match(struct nss_ipsec_xfrm_tunnel *tun, xfrm_address_t *l, xfrm_address_t *r, uint16_t family);
void nss_ipsec_xfrm_tunnel_dealloc(struct nss_ipsec_xfrm_tunnel *tun);
struct nss_ipsec_xfrm_tunnel *nss_ipsec_xfrm_tunnel_alloc(struct nss_ipsec_xfrm_drv *ctx, xfrm_address_t *l,
							xfrm_address_t *r, uint16_t family);
#endif /* !__NSS_IPSEC_XFRM_TUNNEL_H */
