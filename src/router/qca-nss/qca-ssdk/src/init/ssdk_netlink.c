/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "ssdk_netlink.h"
#include <linux/version.h>
#include <net/genetlink.h>
#include "ssdk_init.h"
#include "ssdk_plat.h"

struct nla_policy ssdk_genl_policy[] = {
	[SSDK_ATTR_IFNAME] = { .type = NLA_NUL_STRING, .len = IFNAMSIZ - 1},
	[SSDK_ATTR_MACADDR] = { .type = NLA_BINARY, .len = ETH_ALEN},
	[SSDK_ATTR_MACPOLL] = { .type = NLA_U8},
};

static int ssdk_new_mac(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

static int ssdk_expire_mac(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

static int ssdk_poll_mac(struct sk_buff *skb, struct genl_info *info)
{
	unsigned int port_id = 0, dev_id = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	if (!priv)
	{
		return -ENOENT;
	}

	if (info->attrs[SSDK_ATTR_IFNAME])
	{
		port_id = ssdk_ifname_to_port(dev_id,
			(char *) nla_data(info->attrs[SSDK_ATTR_IFNAME]));
	}
	if (port_id < SSDK_PHYSICAL_PORT1 || port_id > SSDK_PHYSICAL_PORT6)
	{
		SSDK_ERROR("do not support poll mac on port %d\n", port_id);
		return -EINVAL;
	}
	if (info->attrs[SSDK_ATTR_MACPOLL])
	{
		if (nla_get_u8(info->attrs[SSDK_ATTR_MACPOLL]) == 1)
		{
			SSDK_DEBUG("enable fdb polling task: port_id %d\n", port_id);
			qca_fdb_sw_sync_work_start(priv, BIT(port_id));
		}
		if (nla_get_u8(info->attrs[SSDK_ATTR_MACPOLL]) == 0)
		{
			SSDK_DEBUG("disable fdb polling task: port_id %d\n", port_id);
			qca_fdb_sw_sync_work_stop(priv, BIT(port_id));
		}
	}

	return 0;
}

struct genl_ops ssdk_genl_ops[] = {
	{
		.cmd = SSDK_COMMAND_NEW_MAC,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
		.policy = ssdk_genl_policy,
#endif
		.doit = ssdk_new_mac,
	},
	{
		.cmd = SSDK_COMMAND_EXPIRE_MAC,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
		.policy = ssdk_genl_policy,
#endif
		.doit = ssdk_expire_mac,
	},
	{
		.cmd = SSDK_COMMAND_POLL_MAC,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
		.policy = ssdk_genl_policy,
#endif
		.doit = ssdk_poll_mac,
	},
};

struct genl_multicast_group ssdk_genl_mcgrp[] = {
    { .name = SSDK_GENL_MCAST_GRP_NAME },
};


/*
 * ssdk family definition
 */
struct genl_family ssdk_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.hdrsize = 0,
	.name = SSDK_GENL_FAMILY_NAME,
	.version = SSDK_GENL_VERSION,
	.maxattr = SSDK_ATTR_MAX,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 9, 0))
	.policy = ssdk_genl_policy,
#endif
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = ssdk_genl_ops,
	.n_ops = ARRAY_SIZE(ssdk_genl_ops),
	.mcgrps = ssdk_genl_mcgrp,
	.n_mcgrps = ARRAY_SIZE(ssdk_genl_mcgrp)
};


int ssdk_genl_notify_mac_info(char cmd, char *ifname, unsigned char *addr)
{
	struct sk_buff *skb;
	void *msg_head;
	int error, payload;

	SSDK_DEBUG("Newing message.\n");
	payload = nla_total_size(IFNAMSIZ) +
				nla_total_size(ETH_ALEN) +
				nla_total_size(sizeof(u32));

	skb = genlmsg_new(payload, GFP_ATOMIC);
	if (!skb)
	{
		SSDK_ERROR("genlmsg_new() failed.\n");
		return -1;
	}

	SSDK_DEBUG("Putting message.\n");
	if (cmd == SSDK_COMMAND_NEW_MAC ||
		cmd == SSDK_COMMAND_EXPIRE_MAC)
	{
		msg_head = genlmsg_put(skb, 0, 0, &ssdk_family, 0, cmd);
	}
	else
	{
		SSDK_ERROR("genlmsg_put(): not support this command type.\n");
		kfree_skb(skb);
		return -1;
	}
	if (msg_head == NULL)
	{
		SSDK_ERROR("genlmsg_put() failed.\n");
		kfree_skb(skb);
		return -1;
	}

	SSDK_DEBUG("Nla_putting ifname.\n");
	error = nla_put_string(skb, SSDK_ATTR_IFNAME, ifname);
	if (error)
	{
		SSDK_ERROR("nla_put_string() failed: %d\n", error);
		kfree_skb(skb);
		return -1;
	}
	SSDK_DEBUG("Nla_putting macaddr.\n");
	error = nla_put(skb, SSDK_ATTR_MACADDR, ETH_ALEN, addr);
	if (error)
	{
		SSDK_ERROR("nla_put() failed: %d\n", error);
		kfree_skb(skb);
		return -1;
	}

	SSDK_DEBUG("Ending message.\n");
	genlmsg_end(skb, msg_head);

	error = genlmsg_multicast(&ssdk_family, skb, 0, 0, GFP_ATOMIC);
	if (error != 0 && error != -3)
	{
		SSDK_ERROR("genlmsg_multicast() failed: %d\n", error);
		return -1;
	}
	SSDK_DEBUG("Multicasting message Success to group id %u\n", ssdk_family.mcgrp_offset);
	return 0;
}

int ssdk_genl_init(void)
{
	int error;

	error = genl_register_family(&ssdk_family);
	if (error)
	{
		SSDK_ERROR("ssdk genl family register fail: %d\n", error);
	}
	return error;
}

int ssdk_genl_deinit(void)
{
	int error;

	error = genl_unregister_family(&ssdk_family);
	if (error)
	{
		SSDK_ERROR("ssdk genl family unregister fail: %d\n", error);
	}
	return error;
}

/**
 * @}
 */
