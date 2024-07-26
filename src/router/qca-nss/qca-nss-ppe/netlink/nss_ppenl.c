/*
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/if.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/version.h>
#include <net/genetlink.h>

#include "nss_ppenl.h"
#include "nss_ppenl_acl.h"
#include "nss_ppenl_cmn_if.h"
#include "nss_ppenl_acl_if.h"
#include "nss_ppenl_policer.h"
#include "nss_ppenl_policer_if.h"

/*
 * nss_ppenl.c
 *	NSS PPE Netlink manager
 */

/*
 * NSS PPE NL family definition
 */
struct nss_ppenl_family {
	uint8_t *name;		/* name of the family */
	nss_ppenl_fn_t entry;	/* entry function of the family */
	nss_ppenl_fn_t exit;	/* exit function of the family */
	bool valid;		/* valid or invalid */
};

/*
 * Family handler table
 */
static struct nss_ppenl_family family_handlers[] = {
	{
		/*
		 * NSS_PPENL_ACL
		 */
		.name = NSS_PPENL_ACL_FAMILY,		/* ACL FAMILY */
		.entry = NSS_PPENL_ACL_INIT,		/* Init */
		.exit = NSS_PPENL_ACL_EXIT,		/* exit */
		.valid = CONFIG_NSS_PPENL_ACL		/* 1 or 0 */
	},
	{
		/*
		 * NSS_PPENL_POLICER
		 */
		.name = NSS_PPENL_POLICER_FAMILY,	/* Policer Family */
		.entry = NSS_PPENL_POLICER_INIT,	/* Init */
		.exit = NSS_PPENL_POLICER_EXIT,	/* exit */
		.valid = CONFIG_NSS_PPENL_POLICER	/* 1 or 0 */
	},
};

#define NSS_PPENL_FAMILY_HANDLER_SZ ARRAY_SIZE(family_handlers)

/*
 * nss_ppenl_copy_msg()
 *	copy a existing NETLINK message into a new one
 *
 * NOTE: this returns the new SKB/message
 * 	This will be used for response
 */
struct sk_buff *nss_ppenl_copy_msg(struct sk_buff *orig)
{
	struct sk_buff *copy;
	struct nss_ppenl_cmn *cm;

	cm = nss_ppenl_get_data(orig);

	copy = skb_copy(orig, GFP_KERNEL);
	if (!copy) {
		nss_ppenl_warn("%d:unable to copy incoming message of len(%d)\n", cm->pid, orig->len);
		return NULL;
	}

	return copy;
}

/*
 * nss_ppenl_get_data()
 *	Returns start of payload data
 */
void  *nss_ppenl_get_data(struct sk_buff *skb)
{
	return genlmsg_data(NLMSG_DATA(skb->data));
}

/*
 * nss_ppenl_ucast_resp()
 *	send the response to the user (PID)
 *
 * NOTE: this assumes the socket to be available for reception
 */
int nss_ppenl_ucast_resp(struct sk_buff *skb)
{
	struct nss_ppenl_cmn *cm;
	struct net *net;

	cm = genlmsg_data(NLMSG_DATA(skb->data));

	net = (struct net *)cm->sock_data;
	cm->sock_data = 0;

	/*
	 * End the message as no more updates are left to happen
	 * After this message is assumed to be read-only
	 */
	genlmsg_end(skb, cm);
	return genlmsg_unicast(net, skb, cm->pid);
}

/*
 * nss_ppenl_get_msg()
 *	verifies and returns the message pointer
 */
struct nss_ppenl_cmn *nss_ppenl_get_msg(struct genl_family *family, struct genl_info *info, uint16_t cmd)
{
	struct nss_ppenl_cmn *cm;
	uint32_t pid;

	pid =  info->snd_portid;

	/*
	 * validate the common message header version & magic
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	cm = info->userhdr;
#else
	cm = genl_info_userhdr(info);
#endif
	if (nss_ppenl_cmn_chk_ver(cm, family->version) == false) {
		nss_ppenl_warn("%d, %s: version mismatch (%d)\n", pid, family->name, cm->version);
		return NULL;
	}

	/*
	 * check if the message len arrived matches with expected len
	 */
	if (nss_ppenl_cmn_get_cmd_len(cm) != family->hdrsize) {
		nss_ppenl_warn("%d, %s: invalid command len (%d)\n", pid, family->name, nss_ppenl_cmn_get_cmd_len(cm));
		return NULL;
	}

	cm->pid = pid;
	cm->sock_data = (nss_ptr_t)genl_info_net(info);

	return cm;
}

/*
 * nss_ppenl_init()
 *	init module
 */
static int __init nss_ppenl_init(void)
{
	struct nss_ppenl_family *family = NULL;
	int i = 0;
	bool status;

	nss_ppenl_info("NSS Netlink manager loaded: %s\n", NSS_CLIENT_BUILD_ID);

	/*
	 * initialize the handler families, the intention to init the
	 * families that are marked active
	 */
	family = &family_handlers[0];

	for (i = 0; i < NSS_PPENL_FAMILY_HANDLER_SZ; i++, family++) {
		/*
		 * Check if the family exists
		 */
		if (family->valid) {
			BUG_ON(!family->entry);
			status = family->entry();
			if (status) {
				nss_ppenl_info("attaching family:%s\n", family->name);
			} else {
				return -1;
			}
		} else {
			nss_ppenl_info("skipping family:%s\n", family->name);
			nss_ppenl_info("valid = %d, entry = %d\n", family->valid, !!family->entry);
		}
	}

	return 0;
}

/*
 * nss_ppenl_exit()
 *	deinit module
 */
static void __exit nss_ppenl_exit(void)
{
	struct nss_ppenl_family *family = NULL;
	int i = 0;
	bool status;

	nss_ppenl_info("NSS Netlink manager unloaded\n");

	/*
	 * initialize the handler families
	 */
	family = &family_handlers[0];

	for (i = 0; i < NSS_PPENL_FAMILY_HANDLER_SZ; i++, family++) {
		if (family->valid) {
			BUG_ON(!family->exit);
			status = family->exit();
			if (status) {
				nss_ppenl_info("attaching family:%s\n", family->name);
			}
		} else {
			nss_ppenl_info("skipping family:%s\n", family->name);
			nss_ppenl_info("valid = %d, entry = %d\n", family->valid, !!family->entry);
		}
	}
}

module_init(nss_ppenl_init);
module_exit(nss_ppenl_exit);

MODULE_DESCRIPTION("NSS PPE Netlink");
MODULE_LICENSE("Dual BSD/GPL");
