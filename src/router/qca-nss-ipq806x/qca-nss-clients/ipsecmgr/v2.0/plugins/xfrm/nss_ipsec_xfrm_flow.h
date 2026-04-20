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

#ifndef __NSS_IPSEC_XFRM_FLOW_H
#define __NSS_IPSEC_XFRM_FLOW_H

/*
 * NSS IPSec xfrm Flow obj
 */
struct nss_ipsec_xfrm_flow {
	struct list_head list_entry; 		/* Hash DB Entry */
	struct nss_ipsec_xfrm_drv *drv; 	/* Pointer to nss_ipsec_xfrm_drv object */
	struct nss_ipsecmgr_flow_tuple tuple;	/* Cached Parameters for this Flow object*/

	struct kref ref; 			/* Reference count */
	atomic_t allow_accel;			/* Acceleration is allowed for this flow or not(currently unused) */
	struct xfrm_policy *pol; 		/* Reference to xfrm stack's policy object */
	struct nss_ipsec_xfrm_sa *sa; 		/* Reference to the SA object */
};

void nss_ipsec_xfrm_flow_hdr2tuple(struct sk_buff *skb, bool natt, struct nss_ipsecmgr_flow_tuple *tuple);
bool nss_ipsec_xfrm_flow_match(struct nss_ipsec_xfrm_flow *flow, struct nss_ipsecmgr_flow_tuple *tuple);

void nss_ipsec_xfrm_flow_deref(struct nss_ipsec_xfrm_flow *flow);
struct nss_ipsec_xfrm_flow *nss_ipsec_xfrm_flow_ref(struct nss_ipsec_xfrm_flow *flow);

bool nss_ipsec_xfrm_flow_update(struct nss_ipsec_xfrm_flow *flow, struct nss_ipsec_xfrm_sa *new_sa);
void nss_ipsec_xfrm_flow_dealloc(struct nss_ipsec_xfrm_flow *flow);
struct nss_ipsec_xfrm_flow *nss_ipsec_xfrm_flow_alloc(struct nss_ipsec_xfrm_drv *ctx,
							struct nss_ipsecmgr_flow_tuple *tuple,
							struct nss_ipsec_xfrm_sa *sa);
#endif /* !__NSS_IPSEC_XFRM_FLOW_H */
