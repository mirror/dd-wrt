/*
 **************************************************************************
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#define NSS_IFB_MSG_TIMEOUT (600*HZ)	/* 1 min timeout */

/*
 * nss_ifb_if_config
 *	Types of IFB configuration messages.
 */
enum nss_ifb_if_config {
	NSS_IFB_SET_IGS_NODE,
	NSS_IFB_CLEAR_IGS_NODE,
	NSS_IFB_SET_NEXTHOP,
	NSS_IFB_RESET_NEXTHOP,
	NSS_IFB_OPEN,
	NSS_IFB_CLOSE,
};

/*
 * nss_ifb_info
 *	IFB and its mapped interface's bind structure.
 */
struct nss_ifb_info {
	struct net_device *ifb_dev;	/* IFB device */
	struct net_device *map_dev;	/* Device mapped to an IFB device */
	struct list_head map_list;	/* Internal list */
	bool is_mapped;			/* Flag to indicate whether mapping is valid or not */
};

/*
 * nss_ifb_igs_ip_pre_routing_hook()
 *	Copy class-id to Linux CT structure.
 *
 * Copy class-id from tc_index field of skb in ingress QoS fields inside
 * DSCP CT extention structure.
 */
extern unsigned int nss_ifb_igs_ip_pre_routing_hook(void *priv, struct sk_buff *skb,
		 const struct nf_hook_state *state);

/*
 * nss_ifb_list_del()
 *	API to delete member in ifb list.
 */
extern void nss_ifb_list_del(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_is_mapped()
 *	Returns the map status of the given ifb bind structure.
 */
extern bool nss_ifb_is_mapped(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_config_msg_tx()
 *	Send IFB configure message to an IFB mapped interface.
 */
extern int32_t nss_ifb_config_msg_tx(struct net_device *dev, int32_t ifb_num,
		 enum nss_ifb_if_config config, void *cb);

/*
 * nss_ifb_config_msg_tx_sync()
 *	Send IFB configure message to an IFB mapped interface and wait for the response.
 */
extern int32_t nss_ifb_config_msg_tx_sync(struct net_device *dev, int32_t ifb_num,
		 enum nss_ifb_if_config config, void *cb);

/*
 * nss_ifb_reset_nexthop()
 *	Send RESET NEXTHOP configure message to an IFB mapped interface.
 */
extern bool nss_ifb_reset_nexthop(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_clear_igs_node()
 *	Send CLEAR configure message to an IFB mapped interface.
 */
extern bool nss_ifb_clear_igs_node(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_down()
 *	Send interface's DOWN configure message to an IFB interface.
 */
extern bool nss_ifb_down(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_up()
 *	Send interface's UP configure message to an IFB interface.
 */
extern bool nss_ifb_up(struct nss_ifb_info *ifb_info);

/*
 * nss_ifb_init()
 *	Initialization API.
 */
extern void nss_ifb_init(void);

/*
 * nss_ifb_find_map_dev()
 *	Find and return the IFB mapped netdev in the ifb list.
 */
extern struct nss_ifb_info *nss_ifb_find_map_dev(struct net_device *dev);

/*
 * nss_ifb_find_dev()
 *	Find and return the IFB netdev in the ifb list.
 */
extern struct nss_ifb_info *nss_ifb_find_dev(struct net_device *dev);

/*
 * nss_ifb_bind()
 *	API to bind an IFB device with its requested mapped interface.
 */
extern int32_t nss_ifb_bind(struct nss_ifb_info *ifb_info, struct net_device *from_dev,
		struct net_device *to_dev);

/*
 * nss_ifb_delete_if()
 *	Delete an IFB interface in NSS Firmware.
 */
extern void nss_ifb_delete_if(int32_t if_num);

/*
 * nss_ifb_create_if()
 *	Create an IFB interface in NSS Firmware.
 */
extern int32_t nss_ifb_create_if(struct net_device *dev);
