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

#ifndef __NSS_IPSEC_XFRM_SA_H
#define __NSS_IPSEC_XFRM_SA_H

/*
 * Max replay_window size in bytes.
 * The max window size suppoted by NSS is actually 48 bytes.
 * We round it up to the next highest power of 2.
 */
#define NSS_IPSEC_XFRM_SA_MAX_REPLAY_WIN_SZ 64

/*
 * NSS IPSec xfrm SA obj
 */
struct nss_ipsec_xfrm_sa {
	struct nss_ipsecmgr_sa_info sa_info;	/* Cached crypto information for this SA */
	struct nss_ipsecmgr_sa_tuple tuple;	/* Cached SA tuple for this SA object*/
	struct nss_ipsecmgr_sa_data data;	/* Cached SA parameters for this SA object */
	struct nss_ipsec_xfrm_tunnel *tun; 	/* Reference to the parent tun object */
	struct nss_ipsec_xfrm_drv *drv; 	/* Pointer to nss_ipsec_xfrm_drv */
	enum nss_ipsecmgr_sa_type type;		/* Encap or decap */
	atomic_t ecm_accel_outer;		/* Outer flow IPv4 rule is accelerated or not */
	struct kref ref; 			/* Reference count */
};

void nss_ipsec_xfrm_sa_deref(struct nss_ipsec_xfrm_sa *sa);
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref(struct nss_ipsec_xfrm_sa *sa);

/*
 * All these API(s) will hold the SA reference; once the usage is over the reference needs to
 * be dropped
 */
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref_by_state(struct xfrm_state *x);
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref_by_spi(uint32_t spi_index, uint32_t family);

bool nss_ipsec_xfrm_sa_sp_set(struct nss_ipsec_xfrm_sa *sa, struct sk_buff *skb);

void nss_ipsec_xfrm_sa_deinit(struct xfrm_state *x);
void nss_ipsec_xfrm_sa_dealloc(struct nss_ipsec_xfrm_sa *sa, struct xfrm_state *x);
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_alloc(struct nss_ipsec_xfrm_tunnel *tun, struct xfrm_state *x);
#endif /* !__NSS_IPSEC_XFRM_SA_H */
