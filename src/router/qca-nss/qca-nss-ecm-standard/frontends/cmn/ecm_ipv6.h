/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock.  The reverse is NOT SAFE.
 */
extern spinlock_t ecm_ipv6_lock;			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Management thread control
 */
extern bool ecm_ipv6_terminate_pending;		/* True when the user has signalled we should quit */

extern unsigned int ecm_ipv6_ip_process(struct net_device *out_dev, struct net_device *in_dev,
							uint8_t *src_node_addr, uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, bool is_l2_encap,
							struct sk_buff *skb, uint16_t l2_encap_proto);
extern void ecm_ipv6_connection_regenerate(struct ecm_db_connection_instance *ci, ecm_tracker_sender_type_t sender,
							struct net_device *out_dev, struct net_device *out_dev_nat, struct net_device *in_dev,
							struct net_device *in_dev_nat, __be16 *layer4hdr, struct sk_buff *skb);
extern struct ecm_db_node_instance *ecm_ipv6_node_establish_and_ref(struct ecm_front_end_connection_instance *feci,
							struct net_device *dev, ip_addr_t addr, ip_addr_t saddr,
							struct ecm_db_iface_instance *interface_list[], int32_t interface_list_first,
							uint8_t *given_node_addr, struct sk_buff *skb);
extern struct ecm_db_host_instance *ecm_ipv6_host_establish_and_ref(ip_addr_t addr);
extern struct ecm_db_mapping_instance *ecm_ipv6_mapping_establish_and_ref(ip_addr_t addr, int port);
extern bool ecm_ipv6_dev_has_ipaddr(struct net_device *dev);
extern int ecm_ipv6_init(struct dentry *dentry);
extern void ecm_ipv6_exit(void);
