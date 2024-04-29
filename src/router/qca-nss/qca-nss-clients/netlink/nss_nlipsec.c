/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2020 The Linux Foundation. All rights reserved.
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
 * nss_nlipsec.c
 *	NSS Netlink IPsec Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/mutex.h>

#include <net/genetlink.h>
#include <net/sock.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>
#include <nss_nl_if.h>
#include <nss_ipsec_cmn.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nlipsec_if.h"
#include "nss_nlipv6_if.h"
#include "nss_nlipv4_if.h"

/*
 * Function prototypes
 */
static int nss_nlipsec_op_create_tunnel(struct sk_buff *skb, struct genl_info *info);
static int nss_nlipsec_op_destroy_tunnel(struct sk_buff *skb, struct genl_info *info);
static int nss_nlipsec_op_add_sa(struct sk_buff *skb, struct genl_info *info);
static int nss_nlipsec_op_delete_sa(struct sk_buff *skb, struct genl_info *info);
// static int nss_nlipsec_op_add_flow(struct sk_buff *skb, struct genl_info *info);
// static int nss_nlipsec_op_delete_flow(struct sk_buff *skb, struct genl_info *info);

/*
 * Hold netdevice references
 */
struct nss_nlipsec_ref {
	struct mutex lock;	/* Mutex for field access */
	int ifindex;		/* Device interface index */
	bool valid;		/* Reference is valid or invalid */
};

/*
 * Local context for the NSS_NLIPSEC
 */
struct nss_nlipsec_ctx {
	atomic_t tunnels;	/* Number tunnels allocated */

	/*
	 * This table stores device reference associated
	 * to the IPsec tunnel that it has created through NETLINK
	 * thus prohibiting any spurious attempts to delete
	 * random net_devices from the Linux kernel
	 */
	struct nss_nlipsec_ref ref_tbl[NSS_NLIPSEC_MAX_TUNNELS];
};

/*
 * Global context
 */
static struct nss_nlipsec_ctx gbl_ctx;

/*
 * Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlipsec_mcgrp[] = {
	{.name = NSS_NLIPSEC_MCAST_GRP},
};

/*
 * Operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlipsec_ops[] = {
	{ /* Create tunnel */
		.cmd = NSS_NLIPSEC_CMD_ADD_TUNNEL,
		.doit = nss_nlipsec_op_create_tunnel,
	},
	{ /* Destroy tunnel */
		.cmd = NSS_NLIPSEC_CMD_DEL_TUNNEL,
		.doit = nss_nlipsec_op_destroy_tunnel,
	},
	{ /* Add Security Association */
		.cmd = NSS_NLIPSEC_CMD_ADD_SA,
		.doit = nss_nlipsec_op_add_sa,
	},
	{ /* Delete Security Association */
		.cmd = NSS_NLIPSEC_CMD_DEL_SA,
		.doit = nss_nlipsec_op_delete_sa,
	},
	// { /* Add flow */
	// 	.cmd = NSS_NLIPSEC_CMD_ADD_FLOW,
	// 	.doit = nss_nlipsec_op_add_flow,
	// },
	// { /* Delete flow */
	// 	.cmd = NSS_NLIPSEC_CMD_DEL_FLOW,
	// 	.doit = nss_nlipsec_op_delete_flow,
	// },
};

/*
 * IPsec family definition
 */
static struct genl_family nss_nlipsec_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,			/* Auto generate ID */
#endif
	.name = NSS_NLIPSEC_FAMILY,		/* Family name string */
	.hdrsize = sizeof(struct nss_nlipsec_rule),/* NSS NETLINK IPsec rule */
	.version = NSS_NL_VER,			/* Set it to NSS_NL version */
	.maxattr = NSS_NLIPSEC_CMD_MAX,		/* Maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlipsec_ops,
	.n_ops = ARRAY_SIZE(nss_nlipsec_ops),
	.mcgrps = nss_nlipsec_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlipsec_mcgrp)
};

/*
 * nss_nlipsec_add_ref()
 *	Add reference to the netdevice object
 */
static inline void nss_nlipsec_add_ref(struct net_device *dev)
{
	struct nss_nlipsec_ref *ref = gbl_ctx.ref_tbl;
	int i;

	for (i = 0; i < NSS_NLIPSEC_MAX_TUNNELS; i++, ref++) {
		mutex_lock(&ref->lock); /* lock_ref */

		if (!ref->valid) {
			ref->ifindex = dev->ifindex;
			ref->valid = true;

			mutex_unlock(&ref->lock); /* unlock_ref */
			return;
		}

		mutex_unlock(&ref->lock); /* unlock_ref */
	}

	BUG_ON(i == NSS_NLIPSEC_MAX_TUNNELS);
}

/*
 * nss_nlipsec_del_ref()
 *	delete netdevice reference
 */
static inline void nss_nlipsec_del_ref(struct nss_nlipsec_ref *ref)
{
	mutex_lock(&ref->lock);	/* lock_ref */

	ref->ifindex = -1;
	ref->valid = false;

	mutex_unlock(&ref->lock); /* unlock_ref */
}

/*
 * nss_nlipsec_find_ref()
 *	find refernce node for the given netdevice
 */
struct nss_nlipsec_ref *nss_nlipsec_find_ref(struct net_device *dev)
{
	struct nss_nlipsec_ref *ref = gbl_ctx.ref_tbl;
	int i;

	for (i = 0; i < NSS_NLIPSEC_MAX_TUNNELS; i++, ref++) {

		mutex_lock(&ref->lock); /* lock_ref */

		if (ref->valid && (dev->ifindex == ref->ifindex)) {
			mutex_unlock(&ref->lock); /* unlock_ref */
			return ref;
		}

		mutex_unlock(&ref->lock); /* unlock_ref */
	}

	return NULL;
}

/*
 * nss_nlipsec_process_event()
 *	Process events from NSS IPsec manager and MCAST it to user
 */
static void nss_nlipsec_process_event(void *ctx, struct nss_ipsecmgr_event *ev)
{
	struct nss_nlipsec_rule *nl_rule;
	struct sk_buff *skb;

	/*
	 * Allocate a new event message
	 */
	skb = nss_nl_new_msg(&nss_nlipsec_family, ev->type);
	if (!skb) {
		nss_nl_error("unable to allocate NSS_NLIPV4 event\n");
		return;
	}

	nl_rule = nss_nl_get_data(skb);

	/*
	 * Initialize the NETLINK common header
	 */
	nss_nlipsec_rule_init(nl_rule, ev->type);

	/*
	 * Copy the contents of the sync message into the NETLINK message
	 */
	memcpy(&nl_rule->rule.event, ev, sizeof(struct nss_ipsecmgr_event));

	nss_nl_mcast_event(&nss_nlipsec_family, skb);
}

/*
 * nss_nlipsec_get_ifnum()
 *	Extract dynamic interface number for inner/outer
 */
int nss_nlipsec_get_ifnum(struct net_device *dev, uint8_t proto, uint16_t dest_port, uint16_t src_port)
{
	enum nss_dynamic_interface_type type;
	int ifnum;

	/*
	 * If the flow is outer, then set the IPsec outer interface type else
	 * set the inner interface type to obtain interface number.
	 */
	switch (proto) {
	case IPPROTO_ESP:
		type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
		break;

	case IPPROTO_UDP:
		if (dest_port == NSS_IPSECMGR_NATT_PORT_DATA)
			type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
		else if (src_port == NSS_IPSECMGR_NATT_PORT_DATA)
			type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
		else
			type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;

		break;

	default:
		type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;
		break;
	}

	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum < 0) {
		nss_nl_error("%px: Failed to find interface number (dev:%s, type:%d)\n", dev, dev->name, type);
		return -1;
	}

	/*
	 * Interface number with core-id
	 */
	return nss_ipsec_cmn_get_ifnum_with_coreid(ifnum);
}

/*
 * nss_nlipsec_get_mtu()
 * 	Provide maximum mtu if it is an outer flow and maintain minimum mtu
 */
int nss_nlipsec_get_mtu(struct net_device *dev, uint8_t ip_ver, uint8_t proto, uint16_t dest_port, uint16_t src_port)
{
	int mtu = dev->mtu;

	/*
	 * If, flow device is IPsec tunnel and protocol is ESP or NAT-T (UDP@4500)
	 * then the operation is Decapsulation. In this we would like to keep the MTU
	 * at the maximum(65536). This would avoid fragmenting the  packet in NSS before
	 * delivering it for IPsec decap. Also, if the device mtu is less than the minimum
	 * mtu, we set it to the minimum mtu.
	 */
	switch (ip_ver) {
	case 4:
		if (proto == IPPROTO_ESP)
			mtu = NSS_NLIPV4_MAX_MTU;
		else if ((proto == IPPROTO_UDP) && (dest_port == NSS_IPSECMGR_NATT_PORT_DATA))
			mtu = NSS_NLIPV4_MAX_MTU;
		else if ((proto == IPPROTO_UDP) && (src_port == NSS_IPSECMGR_NATT_PORT_DATA))
			mtu = NSS_NLIPV4_MAX_MTU;
		else if (dev->mtu < NSS_NLIPV4_MIN_MTU)
			mtu = NSS_NLIPV4_MIN_MTU;

		break;
	case 6:
		if (proto == IPPROTO_ESP)
			mtu = NSS_NLIPV6_MAX_MTU;
		else if (dev->mtu < NSS_NLIPV6_MIN_MTU)
			mtu = NSS_NLIPV6_MIN_MTU;

		break;
	}

	return mtu;
}

/*
 * nss_nlipsec_op_create_tunnel()
 *	Add IPsec tunnel
 */
static int nss_nlipsec_op_create_tunnel(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlipsec_rule *nl_rule;
	struct nss_ipsecmgr_callback cb;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	struct sk_buff *resp;
	uint32_t pid;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipsec_family, info, NSS_NLIPSEC_CMD_ADD_TUNNEL);
	if (!nl_cm) {
		nss_nl_error("unable to extract create tunnel data\n");
		return -EINVAL;
	}

	pid = nl_cm->pid;

	if (atomic_read(&gbl_ctx.tunnels) == NSS_NLIPSEC_MAX_TUNNELS) {
		nss_nl_error("%d: max allowed tunnel reached (%d)\n", pid, NSS_NLIPSEC_MAX_TUNNELS);
		return -EINVAL;
	}

	/*
	 * Create a IPsec tunnel device
	 */
	cb.app_data = &gbl_ctx;
	cb.skb_dev = NULL;
	cb.data_cb = NULL;
	cb.event_cb = nss_nlipsec_process_event;
	dev = nss_ipsecmgr_tunnel_add(&cb);
	if (!dev) {
		nss_nl_error("%d:unable to add IPsec tunnel\n", pid);
		return -ENOMEM;
	}

	/*
	 * Add an internal reference to the tunnel dev
	 */
	atomic_inc(&gbl_ctx.tunnels);
	nss_nlipsec_add_ref(dev);

	/*
	 * Response message to caller
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("unable to copy incoming message\n");
		goto free_dev;
	}

	/*
	 * Overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);

	/*
	 * Init the command
	 */
	nss_nlipsec_rule_init(nl_rule, NSS_NLIPSEC_CMD_ADD_TUNNEL);

	/*
	 * We need to send the  name to the user; copy
	 * the tunnel I/F name into the same rule and send it
	 * as part of the response for the create operation
	 */
	strlcpy(nl_rule->ifname, dev->name, IFNAMSIZ);

	/*
	 * Send to userspace
	 */
	nss_nl_ucast_resp(resp);

	return 0;

free_dev:
	nss_ipsecmgr_tunnel_del(dev);
	return -ENOMEM;
}

/*
 * nss_nlipsec_op_destroy_tunnel()
 *	Delete an IPsec tunnel
 */
static int nss_nlipsec_op_destroy_tunnel(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlipsec_rule *nl_rule;
	struct nss_nlipsec_ref *ref;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	uint32_t pid;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipsec_family, info, NSS_NLIPSEC_CMD_DEL_TUNNEL);
	if (!nl_cm) {
		nss_nl_error("unable to extract destroy tunnel data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlipsec_rule, cm);
	pid = nl_cm->pid;

	if (atomic_read(&gbl_ctx.tunnels) == 0) {
		nss_nl_error("%d: no tunnels available for deletion\n", pid);
		return -EINVAL;
	}
	/*
	 * Get the the Linux net_device object
	 */
	dev = dev_get_by_name(&init_net, nl_rule->ifname);
	if (!dev) {
		nss_nl_error("%d: unable to find netdevice (%s)\n", pid, nl_rule->ifname);
		return -EINVAL;
	}

	/*
	 * Find if we have the local reference
	 */
	ref = nss_nlipsec_find_ref(dev);
	if (!ref) {
		nss_nl_error("%d: (%s) was not created through NL_IPSEC\n", pid, dev->name);
		dev_put(dev);
		return -EINVAL;
	}

	nss_nlipsec_del_ref(ref);
	atomic_dec(&gbl_ctx.tunnels);

	/*
	 * Down the ref_cnt held by nss_nlipsec_destroy_tunnel
	 */
	dev_put(dev);

	/*
	 * Delete the tunnel device
	 */
	nss_ipsecmgr_tunnel_del(dev);

	return 0;
}

/*
 * nss_nlipsec_get_rule()
 *	Extract the rule message
 */
static struct nss_nlipsec_rule *nss_nlipsec_get_rule(struct genl_info *info, enum nss_nlipsec_cmd cmd,
							struct net_device **dev)
{
	struct nss_nlipsec_rule *nl_rule;
	struct nss_nlipsec_ref *ref;
	struct nss_nlcmn *nl_cm;
	uint32_t pid;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipsec_family, info, cmd);
	if (!nl_cm) {
		nss_nl_error("Unable to extract SA data\n");
		return NULL;
	}

	nl_rule = container_of(nl_cm, struct nss_nlipsec_rule, cm);

	pid = nl_cm->pid;

	*dev = dev_get_by_name(&init_net, nl_rule->ifname);
	if (!(*dev)) {
		nss_nl_error("%d: Unable to find Linux net_device(%s)\n", pid, nl_rule->ifname);
		return NULL;
	}

	ref = nss_nlipsec_find_ref(*dev);
	if (!ref) {
		nss_nl_error("%d: (%s) was not created through NL_IPSEC", pid, (*dev)->name);
		dev_put(*dev);
		return NULL;
	}
	return nl_rule;
}

/*
 * nss_nlipsec_op_add_sa()
 *	Add a Security Association for Encapsulation or Decapsulation
 */
static int nss_nlipsec_op_add_sa(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ipsecmgr_sa_data *sa_data;
	struct nss_nlipsec_rule_sa *sa_rule;
	struct nss_nlipsec_rule *nl_rule;
	struct net_device *dev;
	struct sk_buff *resp;
	uint32_t pid, if_num;
	int error = 0;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_ADD_SA, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	/*
	 * Get the SA rule and data from the message
	 */
	sa_rule = &nl_rule->rule.sa;
	sa_data = &nl_rule->rule.sa.data;

	/*
	 * Switch to kernel pointers
	 */
	sa_data->cmn.keys.cipher_key = sa_rule->cipher_key;
	sa_data->cmn.keys.auth_key = sa_rule->auth_key;
	sa_data->cmn.keys.nonce = sa_rule->nonce;

	error = nss_ipsecmgr_sa_add(dev, &sa_rule->tuple, sa_data, &if_num);
	if (error) {
		nss_nl_error("%d: Failed to add SA for net device(%s), error:%d\n", pid, nl_rule->ifname, error);
		goto free_dev;
	}

	/*
	 * Response message to caller
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("unable to copy incoming message\n");
		error = -ENOMEM;
		goto free_dev;
	}

	/*
	 * Overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);

	/*
	 * Init the command
	 */
	nss_nlipsec_rule_init(nl_rule, NSS_NLIPSEC_CMD_ADD_SA);

	/*
	 * We need to send the ifnum to the user; copy
	 * the if_number into the same rule and send it
	 * as part of the response for the create operation
	 */
	nl_rule->ifnum = if_num;

	/*
	 * Send to userspace
	 */
	nss_nl_ucast_resp(resp);

free_dev:
	/*
	 *  dev_put for dev_get done on nss_nlipsec_get_rule
	 */
	dev_put(dev);
	return error;
}

/*
 * nss_nlipsec_op_delete_sa()
 *	Delete a Security Association for Encapsulation or Decapsulation
 */
static int nss_nlipsec_op_delete_sa(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlipsec_rule *nl_rule;
	struct net_device *dev;
	uint32_t pid;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_DEL_SA, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	nss_ipsecmgr_sa_del(dev, &nl_rule->rule.sa.tuple);

	/*
	 *  dev_put for dev_get done on nss_nlipsec_get_rule
	 */
	dev_put(dev);
	return 0;
}

/*
 * nss_nlipsec_op_add_flow()
 *	Add a flow
 */
// static int nss_nlipsec_op_add_flow(struct sk_buff *skb, struct genl_info *info)
// {
// 	struct nss_ipsecmgr_flow_tuple *flow_tuple;
// 	struct nss_ipsecmgr_sa_tuple *sa_tuple;
// 	struct nss_nlipsec_rule *nl_rule;
// 	struct net_device *dev;
// 	uint32_t pid;
// 	int error = 0;
//
// 	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_ADD_FLOW, &dev);
// 	if (!nl_rule) {
// 		nss_nl_error("Failed to extract SA data\n");
// 		return -EINVAL;
// 	}
//
// 	pid = nl_rule->cm.pid;
// 	nss_nl_error("%d: device(%s)", pid, dev->name);
//
// 	flow_tuple = &nl_rule->rule.flow.tuple;
// 	sa_tuple = &nl_rule->rule.flow.sa;
//
// 	//struct nss_ipsecmgr_ref *nss_ipsecmgr_flow_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
// 	error = nss_ipsecmgr_flow_add_sync(dev, flow_tuple, sa_tuple);
// 	if (error) {
// 		nss_nl_error("%d: Failed to add subnet for net_device(%s)", pid, nl_rule->ifname);
// 	}
//
// 	/*
// 	 *  dev_put for dev_get done on nss_nlipsec_get_rule
// 	 */
// 	dev_put(dev);
// 	return error;
// }

/*
 * nss_nlipsec_op_delete_flow()
 *	Delete a flow
 */
// static int nss_nlipsec_op_delete_flow(struct sk_buff *skb, struct genl_info *info)
// {
// 	struct nss_ipsecmgr_flow_tuple *flow_tuple;
// 	struct nss_ipsecmgr_sa_tuple *sa_tuple;
// 	struct nss_nlipsec_rule *nl_rule;
// 	struct net_device *dev;
// 	uint32_t pid;
// 	int error = 0;
//
// 	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_DEL_FLOW, &dev);
// 	if (!nl_rule) {
// 		nss_nl_error("Failed to extract SA data\n");
// 		return -EINVAL;
// 	}
//
// 	pid = nl_rule->cm.pid;
// 	nss_nl_error("%d: device(%s)", pid, dev->name);
//
// 	flow_tuple = &nl_rule->rule.flow.tuple;
// 	sa_tuple = &nl_rule->rule.flow.sa;
//
// 	nss_ipsecmgr_flow_del(dev, flow_tuple, sa_tuple);
//
// 	/*
// 	 *  dev_put for dev_get done on nss_nlipsec_get_rule
// 	 */
// 	dev_put(dev);
// 	return error;
// }

/*
 * nss_nlipsec_init()
 *	Netlink IPsec handler initialization
 */
bool nss_nlipsec_init(void)
{
	struct nss_nlipsec_ref *ref = gbl_ctx.ref_tbl;
	int error;
	int i;

	nss_nl_info_always("Init NSS netlink IPsec handler\n");

	/*
	 * Initialize reference table
	 */
	for (i = 0; i < NSS_NLIPSEC_MAX_TUNNELS; i++, ref++) {
		mutex_init(&ref->lock);
		ref->valid = false;
		ref->ifindex = -1;
	}

	/*
	 * Register with the family
	 */
	error = genl_register_family(&nss_nlipsec_family);
	if (error != 0) {
		nss_nl_info_always("Error: unable to register IPsec family\n");
		return false;
	}

	return true;
}

/*
 * nss_nlipsec_exit()
 *	Netlink IPsec handler exit
 */
bool nss_nlipsec_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink IPsec handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nlipsec_family);
	if (error != 0) {
		nss_nl_info_always("Unregister IPsec NETLINK family failed\n");
		return false;
	}

	return true;
}
