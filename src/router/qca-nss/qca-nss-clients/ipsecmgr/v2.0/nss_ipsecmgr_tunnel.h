/*
 * ********************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **********************************************************************************
 */

#ifndef __NSS_IPSECMGR_TUNNEL_H
#define __NSS_IPSECMGR_TUNNEL_H

#define NSS_IPSECMGR_CHK_POW2(x) (__builtin_constant_p(x) && !(~(x - 1) & (x >> 1)))

#define NSS_IPSECMGR_MAX_NAME (NSS_IPSECMGR_MAX_KEY_NAME + 64)
#define NSS_IPSECMGR_NODE_STATS_SZ 512
#define NSS_IPSECMGR_NODE_CONFIG_RETRY_TIMEOUT msecs_to_jiffies(500) /* msecs */

#define NSS_IPSECMGR_DEFAULT_TUN_NAME "ipsecdummy"

/*
 * IPsec manager private context
 */
struct nss_ipsecmgr_tunnel {
	struct list_head list;			/* List node */
	struct nss_ipsecmgr_ref ref;		/* SA objects under the tunnel */
	struct net_device *dev;			/* back pointer to tunnel device */

	size_t stats_buf_sz;			/* Size of statistics buffer */
	struct dentry *dentry;			/* DebugFS entry */
	struct list_head ctx_db;		/* Context database */
	struct nss_ipsecmgr_callback cb;	/* Callback entry */
	struct nss_ipsecmgr_sa *tx_sa;		/* Default SA for tunnel transmit */
};

/*
 * Initialize the metadata in the header of SKB, and return the pointer to the start of metadata payload
 */
static inline void *nss_ipsecmgr_tunnel_get_mdata(struct sk_buff *skb)
{
	struct nss_ipsec_cmn_mdata *mo;
	uint8_t *data = skb->data;
	uint16_t len, max_len;
	uint8_t *mo_ptr;

	/*
	 * Here we go backwards by the size of metadata + alignment
	 * Then PTR_ALIGN will optionally shift it forward based
	 * on the current alignment
	 */
	max_len = sizeof(*mo) + NSS_IPSEC_CMN_MDATA_ALIGN_SZ;
	mo_ptr = PTR_ALIGN(data - max_len, NSS_IPSEC_CMN_MDATA_ALIGN_SZ);

	len = data - mo_ptr;
	mo = (struct nss_ipsec_cmn_mdata *)skb_push(skb, len);

	nss_ipsec_cmn_mdata_init(mo, len);
	return (void *)&mo->data;
}

#endif
