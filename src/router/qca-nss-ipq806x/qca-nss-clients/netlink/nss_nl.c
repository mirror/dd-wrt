/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2021 The Linux Foundation. All rights reserved.
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

#include <linux/if.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/version.h>
#include <net/genetlink.h>

#include <nss_api_if.h>
#include <nss_capwap.h>
#include <nss_capwapmgr.h>
#include <nss_cmn.h>
#include <nss_ipsecmgr.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlcapwap.h"
#include "nss_nlcapwap_if.h"
#include "nss_nlcmn_if.h"
#include "nss_nldtls.h"
#include "nss_nldtls_if.h"
#include "nss_nlgre_redir_if.h"
#include "nss_nlgre_redir_family.h"
#include "nss_nlipsec.h"
#include "nss_nlipsec_if.h"
#include "nss_nlipv4.h"
#include "nss_nlipv4_if.h"
#include "nss_nlipv6.h"
#include "nss_nlipv6_if.h"
#include "nss_nloam.h"
#include "nss_nloam_if.h"
#include "nss_nlethrx.h"
#include "nss_nlethrx_if.h"
#include "nss_nledma.h"
#include "nss_nledma_if.h"
#include "nss_nlcapwap.h"
#include "nss_nlcapwap_if.h"
#include "nss_nldynamic_interface.h"
#include "nss_nldynamic_interface_if.h"
#include "nss_nln2h.h"
#include "nss_nln2h_if.h"
#include "nss_nlc2c_tx.h"
#include "nss_nlc2c_tx_if.h"
#include "nss_nlc2c_rx.h"
#include "nss_nlc2c_rx_if.h"
#include "nss_nlipv4_reasm.h"
#include "nss_nlipv4_reasm_if.h"
#include "nss_nlipv6_reasm.h"
#include "nss_nlipv6_reasm_if.h"
#include "nss_nlwifili.h"
#include "nss_nlwifili_if.h"
#include "nss_nllso_rx.h"
#include "nss_nllso_rx_if.h"
#include "nss_nlmap_t.h"
#include "nss_nlmap_t_if.h"
#include "nss_nlpppoe.h"
#include "nss_nlpppoe_if.h"
#include "nss_nll2tpv2.h"
#include "nss_nll2tpv2_if.h"
#include "nss_nlpptp.h"
#include "nss_nlpptp_if.h"
#include "nss_nludp_st.h"
#include "nss_nludp_st_if.h"
#include "nss_nlqrfs.h"
#include "nss_nlqrfs_if.h"

/*
 * nss_nl.c
 *	NSS Netlink manager
 */

/*
 * NSS NL family definition
 */
struct nss_nl_family {
	uint8_t *name;		/* name of the family */
	nss_nl_fn_t entry;	/* entry function of the family */
	nss_nl_fn_t exit;	/* exit function of the family */
	bool valid;		/* valid or invalid */
};

/*
 * Family handler table
 */
static struct nss_nl_family family_handlers[] = {
	{
		/*
		 * NSS_NLIPV4
		 */
		.name = NSS_NLIPV4_FAMILY,		/* ipv4 */
		.entry = NSS_NLIPV4_INIT,		/* init */
		.exit = NSS_NLIPV4_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLIPV4		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLIPSEC
		 */
		.name = NSS_NLIPSEC_FAMILY,		/* ipsec */
		.entry = NSS_NLIPSEC_INIT,		/* init */
		.exit = NSS_NLIPSEC_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLIPSEC		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLOAM
		 */
		.name = NSS_NLOAM_FAMILY,		/* oam */
		.entry = NSS_NLOAM_INIT,		/* init */
		.exit = NSS_NLOAM_EXIT,			/* exit */
		.valid = CONFIG_NSS_NLOAM		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLIPV6
		 */
		.name = NSS_NLIPV6_FAMILY,		/* ipv6 */
		.entry = NSS_NLIPV6_INIT,		/* init */
		.exit = NSS_NLIPV6_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLIPV6		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLGRE_REDIR
		 */
		.name = NSS_NLGRE_REDIR_FAMILY,		/* gre_redir */
		.entry = NSS_NLGRE_REDIR_FAMILY_INIT,	/* init */
		.exit = NSS_NLGRE_REDIR_FAMILY_EXIT,	/* exit */
		.valid = CONFIG_NSS_NLGRE_REDIR_FAMILY	/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLCAPWAP
		 */
		.name = NSS_NLCAPWAP_FAMILY,		/* capwap */
		.entry = NSS_NLCAPWAP_INIT,		/* init */
		.exit = NSS_NLCAPWAP_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLCAPWAP		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLDTLS
		 */
		.name = NSS_NLDTLS_FAMILY,		/* dtls */
		.entry = NSS_NLDTLS_INIT,		/* init */
		.exit = NSS_NLDTLS_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLDTLS		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLETHRX
		 */
		.name = NSS_NLETHRX_FAMILY,		/* ethrx */
		.entry = NSS_NLETHRX_INIT,		/* init */
		.exit = NSS_NLETHRX_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLETHRX		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLEDMA
		 */
		.name = NSS_NLEDMA_FAMILY,		/* edma */
		.entry = NSS_NLEDMA_INIT,		/* init */
		.exit = NSS_NLEDMA_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLEDMA		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLDYNAMIC_INTERFACE
		 */
		.name = NSS_NLDYNAMIC_INTERFACE_FAMILY,	/* dynamic interface */
		.entry = NSS_NLDYNAMIC_INTERFACE_INIT,	/* init */
		.exit = NSS_NLDYNAMIC_INTERFACE_EXIT,	/* exit */
		.valid = CONFIG_NSS_NLDYNAMIC_INTERFACE	/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLN2H
		 */
		.name = NSS_NLN2H_FAMILY,		/* n2h */
		.entry = NSS_NLN2H_INIT,		/* init */
		.exit = NSS_NLN2H_EXIT,			/* exit */
		.valid = CONFIG_NSS_NLN2H		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLC2C_TX
		 */
		.name = NSS_NLC2C_TX_FAMILY,		/* c2c_tx */
		.entry = NSS_NLC2C_TX_INIT,		/* init */
		.exit = NSS_NLC2C_TX_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLC2C_TX		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLC2C_RX
		 */
		.name = NSS_NLC2C_RX_FAMILY,		/* c2c_rx */
		.entry = NSS_NLC2C_RX_INIT,		/* init */
		.exit = NSS_NLC2C_RX_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLC2C_RX		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLIPV4_REASM
		 */
		.name = NSS_NLIPV4_REASM_FAMILY,	/* ipv4_reasm */
		.entry = NSS_NLIPV4_REASM_INIT,		/* init */
		.exit = NSS_NLIPV4_REASM_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLIPV4_REASM	/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLIPV6_REASM
		 */
		.name = NSS_NLIPV6_REASM_FAMILY,	/* ipv6_reasm */
		.entry = NSS_NLIPV6_REASM_INIT,		/* init */
		.exit = NSS_NLIPV6_REASM_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLIPV6_REASM	/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLWIFILI
		 */
		.name = NSS_NLWIFILI_FAMILY,		/* wifili */
		.entry = NSS_NLWIFILI_INIT,		/* init */
		.exit = NSS_NLWIFILI_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLWIFILI		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLLSO_RX
		 */
		.name = NSS_NLLSO_RX_FAMILY,		/* lso_rx */
		.entry = NSS_NLLSO_RX_INIT,		/* init */
		.exit = NSS_NLLSO_RX_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLLSO_RX		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLMAP_T
		 */
		.name = NSS_NLMAP_T_FAMILY,		/* map_t */
		.entry = NSS_NLMAP_T_INIT,		/* init */
		.exit = NSS_NLMAP_T_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLMAP_T		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLPPPOE
		 */
		.name = NSS_NLPPPOE_FAMILY,		/* pppoe */
		.entry = NSS_NLPPPOE_INIT,		/* init */
		.exit = NSS_NLPPPOE_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLPPPOE		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLL2TPV2
		 */
		.name = NSS_NLL2TPV2_FAMILY,		/* l2tpv2 */
		.entry = NSS_NLL2TPV2_INIT,		/* init */
		.exit = NSS_NLL2TPV2_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLL2TPV2		/* 1 or 0 */
	},
	{
		/*
		 * NSS_NLPPTP
		 */
		.name = NSS_NLPPTP_FAMILY,		/* pptp */
		.entry = NSS_NLPPTP_INIT,		/* init */
		.exit = NSS_NLPPTP_EXIT,		/* exit */
		.valid = CONFIG_NSS_NLPPTP		/* 1 or 0 */
	},
	{
                /*
                 * NSS_NLUDP_ST
                 */
                .name = NSS_NLUDP_ST_FAMILY,             /* udp_st */
                .entry = NSS_NLUDP_ST_INIT,              /* init */
                .exit = NSS_NLUDP_ST_EXIT,               /* exit */
                .valid = CONFIG_NSS_NLUDP_ST             /* 1 or 0 */
        },
	{
                /*
                 * NSS_NLQRFS
                 */
                .name = NSS_NLQRFS_FAMILY,             /* qrfs */
                .entry = NSS_NLQRFS_INIT,              /* init */
                .exit = NSS_NLQRFS_EXIT,               /* exit */
                .valid = CONFIG_NSS_NLQRFS             /* 1 or 0 */
        },


};

#define NSS_NL_FAMILY_HANDLER_SZ ARRAY_SIZE(family_handlers)

/*
 * nss_nl_alloc_msg()
 *	allocate NETLINK message
 *
 * NOTE: this returns the SKB/message
 */
struct sk_buff *nss_nl_new_msg(struct genl_family *family, uint8_t cmd)
{
	struct sk_buff *skb;
	struct nss_nlcmn *cm;
	int len;

	/*
	 * aligned length
	 */
	len = nla_total_size(family->hdrsize);

	/*
	 * allocate NL message
	 */
	skb = genlmsg_new(len, GFP_ATOMIC);
	if (!skb) {
		nss_nl_error("%s: unable to allocate notifier SKB\n", family->name);
		return NULL;
	}

	/*
	 * append the generic message header
	 */
	cm = genlmsg_put(skb, 0 /*pid*/, 0 /*seq*/, family, 0 /*flags*/, cmd);
	if (!cm) {
		nss_nl_error("%s: no space to put generic header\n", family->name);
		nlmsg_free(skb);
		return NULL;
	}

	/*
	 * Kernel PID=0
	 */
	cm->pid = 0;

	return skb;
}

/*
 * nss_nl_copy_msg()
 *	copy a existing NETLINK message into a new one
 *
 * NOTE: this returns the new SKB/message
 */
struct sk_buff *nss_nl_copy_msg(struct sk_buff *orig)
{
	struct sk_buff *copy;
	struct nss_nlcmn *cm;

	cm = nss_nl_get_data(orig);

	copy = skb_copy(orig, GFP_KERNEL);
	if (!copy) {
		nss_nl_error("%d:unable to copy incoming message of len(%d)\n", cm->pid, orig->len);
		return NULL;
	}

	return copy;
}

/*
 * nss_nl_get_data()
 *	Returns start of payload data
 */
void  *nss_nl_get_data(struct sk_buff *skb)
{
	return genlmsg_data(NLMSG_DATA(skb->data));
}

/*
 * nss_nl_mcast_event()
 *	mcast the event to the user listening on the MCAST group ID
 *
 * Note: It will free the message buffer if there is no space left to end
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
int nss_nl_mcast_event(struct genl_family *family, struct sk_buff *skb)
{
	struct nss_nlcmn *cm;

	cm = genlmsg_data(NLMSG_DATA(skb->data));

	/*
	 * End the message as no more updates are left to happen.
	 * After this, the message is assunmed to be read-only
	 */
	genlmsg_end(skb, cm);

	return genlmsg_multicast(family, skb, cm->pid, 0, GFP_ATOMIC);
}
#else
int nss_nl_mcast_event(struct genl_multicast_group *grp, struct sk_buff *skb)
{
	struct nss_nlcmn *cm;

	cm = genlmsg_data(NLMSG_DATA(skb->data));

	/*
	 * End the message as no more updates are left to happen.
	 * After this, the message is assunmed to be read-only
	 */
	genlmsg_end(skb, cm);

	return genlmsg_multicast(skb, cm->pid, grp->id, GFP_ATOMIC);
}
#endif

/*
 * nss_nl_ucast_resp()
 *	send the response to the user (PID)
 *
 * NOTE: this assumes the socket to be available for reception
 */
int nss_nl_ucast_resp(struct sk_buff *skb)
{
	struct nss_nlcmn *cm;
	struct net *net;

	cm = genlmsg_data(NLMSG_DATA(skb->data));

	net = (struct net *)cm->sock_data;
	cm->sock_data = 0;

	/*
	 * End the message as no more updates are left to happen
	 * After this message is assumed to be read-only
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	genlmsg_end(skb, cm);
#else
	if (genlmsg_end(skb, cm) < 0) {
		nss_nl_error("%d: unable to close generic ucast message\n", cm->pid);
		nlmsg_free(skb);
		return -ENOMEM;
	}
#endif

	return genlmsg_unicast(net, skb, cm->pid);
}

/*
 * nss_nl_get_msg()
 *	verifies and returns the message pointer
 */
struct nss_nlcmn *nss_nl_get_msg(struct genl_family *family, struct genl_info *info, uint16_t cmd)
{
	struct nss_nlcmn *cm;
	uint32_t pid;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0))
	pid =  info->snd_pid;
#else
	pid =  info->snd_portid;
#endif
	/*
	 * validate the common message header version & magic
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	cm = info->userhdr;
#else
	cm = genl_info_userhdr(info);
#endif
	if (nss_nlcmn_chk_ver(cm, family->version) == false) {
		nss_nl_error("%d, %s: version mismatch (%d)\n", pid, family->name, cm->version);
		return NULL;
	}

	/*
	 * check if the message len arrived matches with expected len
	 */
	if (nss_nlcmn_get_len(cm) != family->hdrsize) {
		nss_nl_error("%d, %s: invalid command len (%d)\n", pid, family->name, nss_nlcmn_get_len(cm));
		return NULL;
	}

	cm->pid = pid;
	cm->sock_data = (nss_ptr_t)genl_info_net(info);

	return cm;
}

/*
 * nss_nl_init()
 *	init module
 */
static int __init nss_nl_init(void)
{
	struct nss_nl_family *family = NULL;
	int i = 0;

#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_nl_info_always("NSS Netlink manager loaded: %s\n", NSS_CLIENT_BUILD_ID);

	/*
	 * initialize the handler families, the intention to init the
	 * families that are marked active
	 */
	family = &family_handlers[0];

	for (i = 0; i < NSS_NL_FAMILY_HANDLER_SZ; i++, family++) {
		/*
		 * Check if the family exists
		 */
		if (!family->valid || !family->entry) {
			nss_nl_info_always("skipping family:%s\n", family->name);
			nss_nl_info_always("valid = %d, entry = %d\n", family->valid, !!family->entry);
			continue;
		}

		nss_nl_info_always("attaching family:%s\n", family->name);

		family->entry();
	}

	return 0;
}

/*
 * nss_nl_exit()
 *	deinit module
 */
static void __exit nss_nl_exit(void)
{
	struct nss_nl_family *family = NULL;
	int i = 0;

#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	nss_nl_info_always("NSS Netlink manager unloaded\n");

	/*
	 * initialize the handler families
	 */
	family = &family_handlers[0];

	for (i = 0; i < NSS_NL_FAMILY_HANDLER_SZ; i++, family++) {
		/*
		 * Check if the family exists
		 */
		if (!family->valid || !family->exit) {
			nss_nl_info_always("skipping family:%s\n", family->name);
			nss_nl_info_always("valid = %d, exit = %d\n", family->valid, !!family->exit);
			continue;
		}

		nss_nl_info_always("detaching family:%s\n", family->name);

		family->exit();
	}
}

module_init(nss_nl_init);
module_exit(nss_nl_exit);

MODULE_DESCRIPTION("NSS NETLINK");
MODULE_LICENSE("Dual BSD/GPL");
