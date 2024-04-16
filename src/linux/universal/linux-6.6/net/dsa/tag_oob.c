// SPDX-License-Identifier: GPL-2.0-only

/* Copyright (c) 2022, Maxime Chevallier <maxime.chevallier@bootlin.com> */

#include <linux/bitfield.h>
#include <linux/dsa/oob.h>
#include <linux/skbuff.h>

#include "tag.h"

#define OOB_NAME "oob"

static struct sk_buff *oob_tag_xmit(struct sk_buff *skb,
				    struct net_device *dev)
{
	struct dsa_oob_tag_info *tag_info = skb_ext_add(skb, SKB_EXT_DSA_OOB);
	struct dsa_port *dp = dsa_slave_to_port(dev);

	tag_info->port = dp->index;

	return skb;
}

static struct sk_buff *oob_tag_rcv(struct sk_buff *skb,
				   struct net_device *dev)
{
	struct dsa_oob_tag_info *tag_info = skb_ext_find(skb, SKB_EXT_DSA_OOB);

	if (!tag_info)
		return NULL;

	skb->dev = dsa_master_find_slave(dev, 0, tag_info->port);
	if (!skb->dev)
		return NULL;

	return skb;
}

static const struct dsa_device_ops oob_tag_dsa_ops = {
	.name	= OOB_NAME,
	.proto	= DSA_TAG_PROTO_OOB,
	.xmit	= oob_tag_xmit,
	.rcv	= oob_tag_rcv,
};

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DSA tag driver for out-of-band tagging");
MODULE_AUTHOR("Maxime Chevallier <maxime.chevallier@bootlin.com>");
MODULE_ALIAS_DSA_TAG_DRIVER(DSA_TAG_PROTO_OOB, OOB_NAME);

module_dsa_tag_driver(oob_tag_dsa_ops);
