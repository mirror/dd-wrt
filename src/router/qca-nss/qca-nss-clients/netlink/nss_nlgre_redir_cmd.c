/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2020, The Linux Foundation. All rights reserved.
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
 * nss_nlgre_redir.c
 * 	NSS Netlink gre_redir Handler
 */
#include <linux/version.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nlcmn_if.h>
#include <nss_nl_if.h>
#include <nss_nlgre_redir_if.h>
#include "nss_nl.h"
#include "nss_nlgre_redir.h"
#include "nss_nlgre_redir_cmd.h"
#include "nss_nlgre_redir_cmn.h"
#include "nss_nlgre_redir_lag.h"
#include "nss_nlipv6_if.h"
#include "nss_nlipv4_if.h"

/*
 * To get lock on deploy_mode
 */
static DEFINE_SPINLOCK(lock);

/*
 * Variable to keep track of mode we are operating
 */
static enum nss_nlgre_redir_cmd_deploy_mode deploy_mode;

/*
 * prototypes
 */
static int nss_nlgre_redir_cmd_ops_tun_create(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_tun_destroy(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_map(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_unmap(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_set_next(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_add_hash(struct sk_buff *skb, struct genl_info *info);
static int nss_nlgre_redir_cmd_ops_del_hash(struct sk_buff *skb, struct genl_info *info);

/*
 * nss_nlgre_redir_cmd_mcgrp
 *	Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlgre_redir_family_mcgrp[] = {
         {.name = NSS_NLGRE_REDIR_MCAST_GRP},
};

/*
 * nss_nlgre_redir_ops
 * 	Operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlgre_redir_ops[] = {
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN, .doit = nss_nlgre_redir_cmd_ops_tun_create,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN, .doit = nss_nlgre_redir_cmd_ops_tun_destroy,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_MAP, .doit = nss_nlgre_redir_cmd_ops_map,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_UNMAP, .doit = nss_nlgre_redir_cmd_ops_unmap,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP, .doit = nss_nlgre_redir_cmd_ops_set_next,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_ADD_HASH, .doit = nss_nlgre_redir_cmd_ops_add_hash,},
	{.cmd = NSS_NLGRE_REDIR_CMD_TYPE_DEL_HASH, .doit = nss_nlgre_redir_cmd_ops_del_hash,},
};

/*
 * nss_nlgre_redir_cmd_family
 * 	Gre_redir family definition
 */
struct genl_family nss_nlgre_redir_cmd_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.name = NSS_NLGRE_REDIR_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlgre_redir_rule),	/* NSS NETLINK gre_redir rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLGRE_REDIR version */
	.maxattr = NSS_NLGRE_REDIR_CMD_TYPE_MAX,	/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlgre_redir_ops,
	.n_ops = ARRAY_SIZE(nss_nlgre_redir_ops),
	.mcgrps = nss_nlgre_redir_family_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlgre_redir_family_mcgrp)
};

/*
 * nss_nlgre_redir_cmd_get_deploy_mode()
 * 	Returns deploy_mode value
 */
static inline enum nss_nlgre_redir_cmd_deploy_mode nss_nlgre_redir_cmd_get_deploy_mode(void)
{
	enum nss_nlgre_redir_cmd_deploy_mode ret_deploy_mode;
	spin_lock(&lock);
	ret_deploy_mode = deploy_mode;
	spin_unlock(&lock);
	return ret_deploy_mode;
}

/*
 * nss_nlgre_redir_cmd_set_deploy_mode()
 * 	Sets the value of deploy_mode to parameter passed
 */
static inline void nss_nlgre_redir_cmd_set_deploy_mode(enum nss_nlgre_redir_cmd_deploy_mode param_deploy_mode)
{
	spin_lock(&lock);
	deploy_mode = param_deploy_mode;
	spin_unlock(&lock);
}

/*
 * nss_nlgre_redir_cmd_ops_tun_create()
 * 	Handler for tunnel create
 */
static int nss_nlgre_redir_cmd_ops_tun_create(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract create tunnel data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Create tunnel based on value of lag_enable
	 */
	if (!nl_rule->msg.create.lag_enable) {
		nss_nlgre_redir_cmd_set_deploy_mode(NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_NON_LAG);
		ret = nss_nlgre_redir_create_tun(&nl_rule->msg.create);
		if (ret < 0) {
			nss_nl_error("Unable to create tunnel\n");
			return -EAGAIN;
		}

		goto done;
	}

	/*
	 * Create a lag tunnel
	 */
	nss_nlgre_redir_cmd_set_deploy_mode(NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_LAG);
	ret = nss_nlgre_redir_lag_create_tun(&nl_rule->msg.create);
	if (ret < 0) {
		nss_nl_error("Unable to create lag tunnel\n");
		return -EAGAIN;
	}
done:
	nss_nl_info("Successfully created tunnel\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_tun_destroy()
 * 	Handler to destroy tunnel
 */
static int nss_nlgre_redir_cmd_ops_tun_destroy(struct sk_buff *skb, struct genl_info *info)
{
	enum nss_nlgre_redir_cmd_deploy_mode deploy_mode;
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract destroy tunnel data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Get the dev reference
	 */
	dev = dev_get_by_name(&init_net, nl_rule->msg.destroy.netdev);
	if (!dev) {
		nss_nl_error("Invalid parameters: %s\n", nl_rule->msg.destroy.netdev);
		return -ENODEV;
	}

	dev_put(dev);

	/*
	 * Destroy the non-lag tunnel
	 */
	deploy_mode = nss_nlgre_redir_cmd_get_deploy_mode();
	if (deploy_mode != NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_LAG) {
		ret = nss_nlgre_redir_destroy_tun(dev);
		if (ret < 0) {
			nss_nl_error("Unable to destroy tunnel: %s\n", nl_rule->msg.destroy.netdev);
			dev_put(dev);
			return -EAGAIN;
		}

		goto done;
	}

	/*
	 * Destroy the lag tunnel
	 */
	ret = nss_nlgre_redir_lag_destroy_tun(dev);
	if (ret < 0) {
		nss_nl_error("Unable to destroy tunnel: %s\n", nl_rule->msg.destroy.netdev);
		dev_put(dev);
		return -EAGAIN;
	}

done:
	nss_nl_info("Successfully destroyed gretun = %s tunnel\n", nl_rule->msg.destroy.netdev);
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_map()
 * 	Handler for map command
 */
static int nss_nlgre_redir_cmd_ops_map(struct sk_buff *skb, struct genl_info *info)
{
	enum nss_nlgre_redir_cmd_deploy_mode deploy_mode;
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_MAP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract map interface data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Map the interface
	 */
	deploy_mode = nss_nlgre_redir_cmd_get_deploy_mode();
	if (deploy_mode != NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_LAG) {
		ret = nss_nlgre_redir_map_interface(&nl_rule->msg.map);
		if(ret < 0) {
			nss_nl_error("Unable to map nss interface\n");
			return -EAGAIN;
		}

		goto done;
	}

	ret = nss_nlgre_redir_lag_map_interface(&nl_rule->msg.map);
	if (ret < 0) {
		nss_nl_error("Unable to map nss interface\n");
		return -EAGAIN;
	}
done:
	nss_nl_info("Successfully mapped nss interface.\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_unmap()
 * 	Handler for unmap command
 */
static int nss_nlgre_redir_cmd_ops_unmap(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_UNMAP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract unmap data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Unmap the interface
	 */
	ret = nss_nlgre_redir_cmn_unmap_interface(&nl_rule->msg.unmap);
	if(ret < 0) {
		nss_nl_error("Unable to unmap nss interface\n");
		return -EAGAIN;
	}

	nss_nl_info("Successfully unmapped the nss interface.\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_set_next()
 * 	Handler for set_next command
 */
static int nss_nlgre_redir_cmd_ops_set_next(struct sk_buff *skb, struct genl_info *info)
{
	enum nss_nlgre_redir_cmd_deploy_mode deploy_mode;
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP);
	if (!nl_cm) {
		nss_nl_error("Unable to extract set_next_hop data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Set the next hop of ath0 as wifi_offld_inner of gre_redir node
	 */
	deploy_mode = nss_nlgre_redir_cmd_get_deploy_mode();
	if (deploy_mode != NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_LAG) {
		ret = nss_nlgre_redir_set_next_hop(&nl_rule->msg.snext);
		if (ret < 0) {
			nss_nl_error("Unable to set next hop\n");
			return -EAGAIN;
		}

		goto done;
	}

	/*
	 * Set the next hop of ath0 as lag US node's inner interface
	 */
	ret = nss_nlgre_redir_lag_set_next_hop(&nl_rule->msg.snext);
	if (ret < 0) {
		nss_nl_error("Unable to set the next hop\n");
		return -EAGAIN;
	}

done:
	nss_nl_info("Successfully set the next hop\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_add_hash()
 * 	Handler for adding hash a value
 */
static int nss_nlgre_redir_cmd_ops_add_hash(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_ADD_HASH);
	if (!nl_cm) {
		nss_nl_error("Unable to add a new hash value.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);
	ret = nss_nlgre_redir_lag_add_hash(&nl_rule->msg.hash_ops);
	if(ret < 0) {
		nss_nl_error("Unable to add hash value.\n");
		return -EINVAL;
	}

	nss_nl_info("Successfully added a hash value.\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_ops_del_hash()
 * 	Handler for deleting a hash value
 */
static int nss_nlgre_redir_cmd_ops_del_hash(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlgre_redir_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int ret;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlgre_redir_cmd_family, info, NSS_NLGRE_REDIR_CMD_TYPE_DEL_HASH);
	if (!nl_cm) {
		nss_nl_error("Unable to delete the hash value.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlgre_redir_rule, cm);

	/*
	 * Delete hash value corresponding to smac and dmac
	 */
	ret = nss_nlgre_redir_lag_del_hash(&nl_rule->msg.hash_ops);
	if(ret < 0) {
		nss_nl_error("Unable to delete hash value.\n");
		return -EINVAL;
	}

	nss_nl_info("Successfully deleted the hash entry.\n");
	return 0;
}

/*
 * nss_nlgre_redir_cmd_get_ifnum()
 * 	Get the interface number corresponding to netdev
 */
int nss_nlgre_redir_cmd_get_ifnum(struct net_device *dev, uint8_t proto)
{
	enum nss_dynamic_interface_type type;
	int ifnum;

	switch (proto) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
		type = NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_WIFI_OFFL_INNER;
		break;
	case IPPROTO_GRE:
		type = NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER;
		break;
	default:
		nss_nl_error("Invalid protocol %d\n", proto);
		return -1;
	}

	/*
	 * Get the interface number depending upon the dev and type
	 */
	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum < 0) {
		nss_nl_error("%px: Failed to find interface number (dev:%s, type:%d)\n", dev, dev->name, type);
		return -1;
	}

	return ifnum;
}

/*
 * nss_nlgre_redir_cmd_get_mtu()
 * 	Returns the mtu based on the device passed
 */
int nss_nlgre_redir_cmd_get_mtu(struct net_device *dev, uint8_t iptype, int ifnum)
{
	enum nss_dynamic_interface_type type;
	struct nss_ctx_instance *nss_ctx;
	int mtu = dev->mtu;

	nss_ctx = nss_gre_redir_get_context();
	type = nss_dynamic_interface_get_type(nss_ctx, ifnum);
	switch (iptype) {
	case NSS_GRE_REDIR_IP_HDR_TYPE_IPV4:
		if (type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER) {
			mtu = NSS_NLIPV4_MAX_MTU;
		} else if (mtu < NSS_NLIPV4_MIN_MTU) {
			mtu = NSS_NLIPV4_MIN_MTU;
		}

		break;
	case NSS_GRE_REDIR_IP_HDR_TYPE_IPV6:
		if (type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_OUTER) {
			mtu = NSS_NLIPV6_MAX_MTU;
		} else if (mtu < NSS_NLIPV6_MIN_MTU) {
			mtu = NSS_NLIPV6_MIN_MTU;
		}

		break;
	}

	return mtu;
}
