/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2019 The Linux Foundation. All rights reserved.
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
#include "nss_crypto_defines.h"
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nlipsec_if.h"
#include "nss_nlipv6_if.h"

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
 * IPsec family definition
 */
static struct genl_family nss_nlipsec_family = {
	.id = GENL_ID_GENERATE,			/* Auto generate ID */
	.name = NSS_NLIPSEC_FAMILY,		/* Family name string */
	.hdrsize = sizeof(struct nss_nlipsec_rule),/* NSS NETLINK IPsec rule */
	.version = NSS_NL_VER,			/* Set it to NSS_NL version */
	.maxattr = NSS_NLIPSEC_CMD_MAX,		/* Maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
};

/*
 * Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlipsec_mcgrp[] = {
	{.name = NSS_NLIPSEC_MCAST_GRP},
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
	memcpy(&nl_rule->data.event, ev, sizeof(struct nss_ipsecmgr_event));

	nss_nl_mcast_event(&nss_nlipsec_family, skb);
}

/*
 * nss_nlipsec_op_create_tunnel()
 *	Add IPsec tunnel
 */
static int nss_nlipsec_op_create_tunnel(struct sk_buff *skb,
					struct genl_info *info)
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
	nl_cm = nss_nl_get_msg(&nss_nlipsec_family, info,
			       NSS_NLIPSEC_CMD_CREATE_TUNNEL);
	if (!nl_cm) {
		nss_nl_error("unable to extract create tunnel data\n");
		return -EINVAL;
	}

	pid = nl_cm->pid;

	if (atomic_read(&gbl_ctx.tunnels) == NSS_NLIPSEC_MAX_TUNNELS) {
		nss_nl_error("%d: max allowed tunnel reached (%d)\n", pid,
			     NSS_NLIPSEC_MAX_TUNNELS);
		return -EINVAL;
	}

	/*
	 * Create a IPsec tunnel device
	 */
	cb.app_data = &gbl_ctx;
	cb.skb_dev = NULL; /* FIXME: passing NULL ???? */
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
	 * Response message
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
	nss_nlipsec_rule_init(nl_rule, NSS_NLIPSEC_CMD_CREATE_TUNNEL);

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
static int nss_nlipsec_op_destroy_tunnel(struct sk_buff *skb,
					 struct genl_info *info)
{
	struct nss_nlipsec_rule *nl_rule;
	struct nss_nlipsec_ref *ref;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	uint32_t pid;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipsec_family, info,
			       NSS_NLIPSEC_CMD_DESTROY_TUNNEL);
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
		nss_nl_error("%d: unable to find netdevice (%s)\n", pid,
			     nl_rule->ifname);
		return -EINVAL;
	}

	/*
	 * Find if we have the local reference
	 */
	ref = nss_nlipsec_find_ref(dev);
	if (!ref) {
		nss_nl_error("%d: (%s) was not created through NL_IPSEC\n",
			     pid, dev->name);
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
static struct nss_nlipsec_rule *nss_nlipsec_get_rule(struct genl_info *info,
						     enum nss_nlipsec_cmd cmd,
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
		nss_nl_error("%d: Unable to find Linux net_device(%s)\n", pid,
			     nl_rule->ifname);
		return NULL;
	}

	ref = nss_nlipsec_find_ref(*dev);
	if (!ref) {
		nss_nl_error("%d: (%s) was not created through NL_IPSEC",
			     pid, (*dev)->name);
		dev_put(*dev);
		return NULL;
	}
	nss_nlipsec_del_ref(ref);
	return nl_rule;
}

/*
 * nss_nlipsec_op_add_sa()
 *	Add a Security Association for Encapsulation or Decapsulation
 */
static int nss_nlipsec_op_add_sa(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlipsec_rule *nl_rule;
	struct nss_ipsecmgr_sa *sa;
	struct net_device *dev;
	uint32_t pid, if_num;
	int error;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_ADD_SA, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	/*
	 * Get the SA data from the message
	 */
	sa = &nl_rule->data.sa;

	/* TODO: need to add verify the SA */

	if (nss_ipsecmgr_sa_add(dev, &nl_rule->outer, sa, &if_num)) {
		nss_nl_error("%d: Failed to add SA for net device(%s)\n",
			     pid, nl_rule->ifname);
		error = -EINVAL;
		goto done;
	}

	return 0;

done:
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
	struct nss_ipsecmgr_sa *sa;
	struct net_device *dev;
	uint32_t pid;
	int err = 0;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_DEL_SA, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	sa = &nl_rule->data.sa;

	/* TODO: need to verify before destroying */

	nss_ipsecmgr_sa_del(dev, &nl_rule->outer);

	dev_put(dev);
	return err;
}

/*
 * nss_nlipsec_op_add_flow()
 *	Add a flow
 */
static int nss_nlipsec_op_add_flow(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ipsecmgr_flow_inner *inner;
	struct nss_ipsecmgr_flow_outer *outer;
	struct nss_nlipsec_rule *nl_rule;
	struct net_device *dev;
	uint32_t pid;
	int error = 0;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_ADD_FLOW, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	/* TODO: verify the flow parameters */

	inner = &nl_rule->data.flow;
	outer = &nl_rule->outer;

	if (nss_ipsecmgr_flow_add(dev, inner, outer)) {
		nss_nl_error("%d: Failed to add subnet for net_device(%s)",
			     pid, nl_rule->ifname);
		error = -EINVAL;
	}

	dev_put(dev);
	return error;
}

/*
 * nss_nlipsec_op_delete_flow()
 *	Delete a flow
 */
static int nss_nlipsec_op_delete_flow(struct sk_buff *skb,
				      struct genl_info *info)
{
	struct nss_ipsecmgr_flow_inner *inner;
	struct nss_ipsecmgr_flow_outer *outer;
	struct nss_nlipsec_rule *nl_rule;
	struct net_device *dev;
	uint32_t pid;
	int error = 0;

	nl_rule = nss_nlipsec_get_rule(info, NSS_NLIPSEC_CMD_DEL_FLOW, &dev);
	if (!nl_rule) {
		nss_nl_error("Failed to extract SA data\n");
		return -EINVAL;
	}

	pid = nl_rule->cm.pid;
	nss_nl_error("%d: device(%s)", pid, dev->name);

	/* TODO: verify the flow parameters */

	inner = &nl_rule->data.flow;
	outer = &nl_rule->outer;

	nss_ipsecmgr_flow_del(dev, inner, outer);

	dev_put(dev);
	return error;
}

/*
 * Operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlipsec_ops[] = {
	{ /* Create tunnel */
		.cmd = NSS_NLIPSEC_CMD_CREATE_TUNNEL,
		.doit = nss_nlipsec_op_create_tunnel,
	},
	{ /* Destroy tunnel */
		.cmd = NSS_NLIPSEC_CMD_DESTROY_TUNNEL,
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
	{ /* Add flow */
		.cmd = NSS_NLIPSEC_CMD_ADD_FLOW,
		.doit = nss_nlipsec_op_add_flow,
	},
	{ /* Delete flow */
		.cmd = NSS_NLIPSEC_CMD_DEL_FLOW,
		.doit = nss_nlipsec_op_delete_flow,
	},
};

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
	error = genl_register_family_with_ops_groups(&nss_nlipsec_family,
						     nss_nlipsec_ops,
						     nss_nlipsec_mcgrp);
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
