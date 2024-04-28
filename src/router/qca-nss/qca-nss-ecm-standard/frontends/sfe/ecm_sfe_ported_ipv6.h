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

extern unsigned int ecm_sfe_ported_ipv6_process(struct net_device *out_dev,
							struct net_device *in_dev,
							uint8_t *src_node_addr,
							uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb,
							struct ecm_tracker_ip_header *iph,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr);
extern bool ecm_sfe_ported_ipv6_debugfs_init(struct dentry *dentry);

