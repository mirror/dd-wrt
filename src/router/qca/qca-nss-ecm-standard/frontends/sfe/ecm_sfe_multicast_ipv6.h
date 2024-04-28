/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation.  All rights reserved.
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
 * ecm_sfe_multicast_ipv6_connection_process()
 *	Process a UDP multicast flow
 */
static inline unsigned int ecm_sfe_multicast_ipv6_connection_process(struct net_device *out_dev,
							struct net_device *in_dev,
							uint8_t *src_node_addr,
							uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, struct sk_buff *skb,
							struct ecm_tracker_ip_header *iph,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple)
{
	/*
	 * Now sfe don't support multicast, just return
	 */
	return NF_ACCEPT;
}

/*
 * ecm_sfe_multicast_ipv6_debugfs_init()
 *	create debugfs for ecm sfe ipv4 multicast
 */
static inline bool ecm_sfe_multicast_ipv6_debugfs_init(struct dentry *dentry)
{
	/*
	 * Now sfe don't support multicast, just return
	 */
	return true;
}

/*
 * ecm_sfe_multicast_ipv6_init()
 * 	Register the callbacks for MCS snooper and MFC update
 */
static inline void ecm_sfe_multicast_ipv6_init(void)
{
	/*
	 * Now sfe don't support multicast, just return
	 */
	return;
}

/*
 * ecm_sfe_multicast_ipv6_exit()
 * 	De-register the callbacks for MCS snooper and MFC update
 */
static inline void ecm_sfe_multicast_ipv6_exit(void)
{
	/*
	 * Now sfe don't support multicast, just return
	 */
	return;
}
