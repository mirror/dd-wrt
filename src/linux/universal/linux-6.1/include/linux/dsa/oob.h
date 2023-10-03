/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (C) 2022 Maxime Chevallier <maxime.chevallier@bootlin.com>
 */

#ifndef _NET_DSA_OOB_H
#define _NET_DSA_OOB_H

#include <linux/skbuff.h>

struct dsa_oob_tag_info {
	u16 port;
};

int dsa_oob_tag_push(struct sk_buff *skb, struct dsa_oob_tag_info *ti);
int dsa_oob_tag_pop(struct sk_buff *skb, struct dsa_oob_tag_info *ti);
#endif
