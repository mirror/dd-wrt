/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
 * nss_nludp_st.c
 *	NSS Netlink udp_st Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/timekeeping.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include <net/genetlink.h>
#include <net/sock.h>
#include <net/arp.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/netdevice.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nludp_st_if.h"
#include "nss_nludp_st.h"

#define NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED 0xFFF

/*
 * prototypes
 */
static int nss_nludp_st_ops_uncfg_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_cfg_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_stop(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_start(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_tx_destroy(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_tx_create(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_tx_update_rate(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_rx_mode_set(struct sk_buff *skb, struct genl_info *info);
static int nss_nludp_st_ops_reset_stats(struct sk_buff *skb, struct genl_info *info);

/*
 * multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nludp_st_mcgrp[] = {
	{.name = NSS_NLUDP_ST_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nludp_st_ops[] = {
	{.cmd = NSS_UDP_ST_CFG_RULE_MSG, .doit = nss_nludp_st_ops_cfg_rule,},
	{.cmd = NSS_UDP_ST_UNCFG_RULE_MSG, .doit = nss_nludp_st_ops_uncfg_rule,},
	{.cmd = NSS_UDP_ST_START_MSG, .doit = nss_nludp_st_ops_start,},
	{.cmd = NSS_UDP_ST_STOP_MSG, .doit = nss_nludp_st_ops_stop,},
	{.cmd = NSS_UDP_ST_TX_CREATE_MSG, .doit = nss_nludp_st_ops_tx_create,},
	{.cmd = NSS_UDP_ST_TX_DESTROY_MSG, .doit = nss_nludp_st_ops_tx_destroy,},
	{.cmd = NSS_UDP_ST_TX_UPDATE_RATE_MSG, .doit = nss_nludp_st_ops_tx_update_rate,},
	{.cmd = NSS_UDP_ST_RX_MODE_SET_MSG, .doit = nss_nludp_st_ops_rx_mode_set,},
	{.cmd = NSS_UDP_ST_RESET_STATS_MSG, .doit = nss_nludp_st_ops_reset_stats,},
};

/*
 * udp_st family definition
 */
static struct genl_family nss_nludp_st_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,						/* Auto generate ID */
#endif
	.name = NSS_NLUDP_ST_FAMILY,					/* family name string */
	.hdrsize = sizeof(struct nss_nludp_st_rule),			/* NSS NETLINK udp_st rule */
	.version = NSS_NL_VER,						/* Set it to NSS_NLudp_st version */
	.maxattr = NSS_UDP_ST_MAX_MSG_TYPES,				/* Maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nludp_st_ops,
	.n_ops = ARRAY_SIZE(nss_nludp_st_ops),
	.mcgrps = nss_nludp_st_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nludp_st_mcgrp)
};

/*
 * nss_nludp_st_private
 *	Global structure to handle time sync.
 */
struct nss_nludp_st_private {
	struct timer_list time_sync_timer;		/* Timer to sync linux and nss time */
	struct timer_list test_duration_timer;		/* Timer to keep track of test duration */
	atomic_t disable_time_sync_timer;		/* Flag to disabel time sync timer */
	struct work_struct work;			/* Work struct to send time to linux */
	enum nss_udp_st_mode mode;			/* UDP_ST test mode(default or timestamp) */
	enum nss_udp_st_test_types type;		/* UDP_ST type(RX or TX) */
} global;

/*
 * nss_nludp_st_time_sync()
 *	Sync linux time to NSS FW.
 */
static void nss_nludp_st_time_sync(struct work_struct *work)
{
	struct nss_udp_st_msg num;
	nss_tx_status_t status = NSS_TX_SUCCESS;
	struct nss_ctx_instance *nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_TIME_SYNC_MSG,		/* Time sync respomse message*/
			sizeof(struct nss_udp_st_time_sync),	/* message size */
			NULL,					/* response callback */
			NULL);					/* app context */

	/*
	 * Get epoch time and test type.
	 */
	num.msg.time.time = ktime_get_real_ns();
	num.msg.time.type = global.type;
	status = nss_udp_st_tx(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Failed to sync time", nss_ctx);
	}
}

/*
 * nss_nludp_st_time_sync_timer_cb
 *	Time Sync timer callback handler.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_nludp_st_time_sync_timer_cb(unsigned long data)
#else
static void nss_nludp_st_time_sync_timer_cb(struct timer_list *t)
#endif
{
	unsigned long next_timer_jiffies;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_nludp_st_private *priv = (struct nss_nludp_st_private *)data;
#else
	struct nss_nludp_st_private *priv = from_timer(priv, t, time_sync_timer);
#endif
	/*
	 * Check if stop is called or test duration is over.
	 */
	if (atomic_read(&priv->disable_time_sync_timer)) {
		return;
	}

	/*
	 * Send current time to nss-fw.
	 */
	schedule_work(&priv->work);

	/*
	 * Re-attach the timer
	 */
	next_timer_jiffies = jiffies + msecs_to_jiffies(NSS_UDP_ST_TIME_SYNC_FREQ * 1e3);
	mod_timer(&priv->time_sync_timer, next_timer_jiffies);
}

/*
 * nss_nludp_st_test_duration_timer_cb
 *	Test duration timer callback handler.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_nludp_st_test_duration_timer_cb(unsigned long data)
#else
static void nss_nludp_st_test_duration_timer_cb(struct timer_list *t)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_nludp_st_private *priv = (struct nss_nludp_st_private *)data;
#else
	struct nss_nludp_st_private *priv = from_timer(priv, t, test_duration_timer);
#endif
	atomic_set(&priv->disable_time_sync_timer, 1);
}



/*
 * nss_nludp_st_process_resp_timestamp
 *	Handle NSS-FW -> HLOS response in timestamp mode.
 */
static void nss_nludp_st_process_resp_timestamp(struct nss_udp_st_msg *num)
{
	unsigned long test_duration_jiffies, next_timer_jiffies;
	switch (num->cm.type) {
		case NSS_UDP_ST_START_MSG:
			global.type = num->msg.start.type;

			/*
			 * Start the time syn timer.
			 */
			atomic_set(&global.disable_time_sync_timer, 0);
			next_timer_jiffies = jiffies + msecs_to_jiffies(NSS_UDP_ST_TIME_SYNC_FREQ * 1e3);
			mod_timer(&global.time_sync_timer, next_timer_jiffies);

			/*
			 * Start the test duration timer.
			 */
			test_duration_jiffies = jiffies + msecs_to_jiffies(NSS_UDP_ST_DURATION_MAX * 1e3);
			mod_timer(&global.test_duration_timer, test_duration_jiffies);
			break;

		case NSS_UDP_ST_STOP_MSG:
			/*
			 * Update time sync timer disable flag and delete test duration timer.
			 */
			atomic_set(&global.disable_time_sync_timer, 1);
			del_timer(&global.test_duration_timer);
			break;

		case NSS_UDP_ST_RESET_STATS_MSG:
			/*
			 * Reset the test duration timer.
			 */
			test_duration_jiffies = jiffies + msecs_to_jiffies(NSS_UDP_ST_DURATION_MAX * 1e3);
			mod_timer(&global.test_duration_timer, test_duration_jiffies);
			break;

		default:
			break;
	}
}

/*
 * nss_nludp_st_process_resp()
 *      handle responses
 */
static void nss_nludp_st_process_resp(void *app_data, struct nss_udp_st_msg *num)
{
	struct sk_buff *resp = (struct sk_buff *)app_data;
	struct nss_nludp_st_rule *nl_rule;
	struct nss_udp_st_msg *nl_num;

	nl_rule = nss_nl_get_data(resp);
	nl_num = &nl_rule->num;

	/*
	 * clear NSS common message items that are not useful to uspace
	 */
	num->cm.interface = 0;
	num->cm.cb = (nss_ptr_t)NULL;
	num->cm.app_data = (nss_ptr_t)NULL;

	/*
	 * copy the message response data into the NL message buffer. If, FW
	 * has updated the message then we must updated the same into the NL
	 * message as the NL message buffer is different from what was sent
	 * to FW
	 */
	memcpy(nl_num, num, sizeof(struct nss_udp_st_msg));

	nss_nl_ucast_resp(resp);

	if ((global.mode == NSS_UDP_ST_MODE_TIMESTAMP) && (num->cm.response == NSS_CMN_RESPONSE_ACK)) {
		nss_nludp_st_process_resp_timestamp(num);
	}
}

/*
 * nss_nludp_st_ops_stop
 *      Stop UDP spedtest
 */
static int nss_nludp_st_ops_stop(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_STOP_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract udp_st stop data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -EPERM;
	}


	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_STOP_MSG,			/* rule */
			sizeof(struct nss_udp_st_stop),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */


	memcpy(&num.msg.stop, &nl_rule->num.msg.stop, sizeof(struct nss_udp_st_stop));

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_ops_start
 *	Start UDP spedtest
 */
static int nss_nludp_st_ops_start(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	uint32_t pid;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_START_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract udp_st start data\n", skb);
		return -EINVAL;
	}

	pid = nl_cm->pid;
	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -ENOENT;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_START_MSG,			/* rule */
			sizeof(struct nss_udp_st_start),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	memcpy(&num.msg.start, &nl_rule->num.msg.start, sizeof(struct nss_udp_st_start));

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_ops_tx_update_rate
 *	Update transmit rate.
 */
static int nss_nludp_st_ops_tx_update_rate(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cmn;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cmn = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_TX_UPDATE_RATE_MSG);
	if (!nl_cmn) {
		nss_nl_error("%px: Unable to extract udp_st stop data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cmn, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -EPERM;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_TX_UPDATE_RATE_MSG,		/* rule */
			sizeof(struct nss_udp_st_tx_update_rate),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	memcpy(&num.msg.update_rate, &nl_rule->num.msg.update_rate, sizeof(struct nss_udp_st_tx_update_rate));

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_ops_rx_mode_set
 *	Set Rx mode.
 */
static int nss_nludp_st_ops_rx_mode_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cmn;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cmn = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_RX_MODE_SET_MSG);
	if (!nl_cmn) {
		nss_nl_error("%px: Unable to extract udp_st stop data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cmn, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -EPERM;
	}


	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_RX_MODE_SET_MSG,			/* rule */
			sizeof(struct nss_udp_st_rx_mode),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	memcpy(&num.msg.mode, &nl_rule->num.msg.mode, sizeof(struct nss_udp_st_rx_mode));

	if (num.msg.mode.mode == NSS_UDP_ST_MODE_DEFAULT) {
		global.mode = NSS_UDP_ST_MODE_DEFAULT;
	} else {
		global.mode = NSS_UDP_ST_MODE_TIMESTAMP;

		/*
		 * Get the linux epoch time.
		 */
		num.msg.mode.timestamp = ktime_get_real_ns();
	}

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}
/*
 * nss_nludp_st_ops_reset_stats
 *	Resets UDP speed test statistics.
 */
static int nss_nludp_st_ops_reset_stats(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	uint32_t pid;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_RESET_STATS_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract udp_st reset stats data\n", skb);
		return -EINVAL;
	}

	pid = nl_cm->pid;
	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -ENOENT;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_RESET_STATS_MSG,		/* UDP ST stats reset */
			sizeof(struct nss_udp_st_reset_stats),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_ops_tx_destroy
 *	Destroys UDP speed test Tx node.
 */
static int nss_nludp_st_ops_tx_destroy(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	uint32_t pid;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_TX_DESTROY_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract udp_st tx destroy data\n", skb);
		return -EINVAL;
	}

	pid = nl_cm->pid;
	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -ENOENT;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_TX_DESTROY_MSG,		/* UDP ST tx destroy */
			sizeof(struct nss_udp_st_tx_destroy),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_ops_tx_create
 *	Creates UDP speed test Tx node.
 */
static int nss_nludp_st_ops_tx_create(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	uint32_t pid;
	nss_tx_status_t status;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_TX_CREATE_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract udp_st tx create data\n", skb);
		return -EINVAL;
	}

	pid = nl_cm->pid;
	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -ENOENT;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num,				/* udp_st message */
			NSS_UDP_ST_INTERFACE,			/* interface number */
			NSS_UDP_ST_TX_CREATE_MSG,		/* UDP ST tx create */
			sizeof(struct nss_udp_st_tx_create),	/* message size */
			nss_nludp_st_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	memcpy(&num.msg.create, &nl_rule->num.msg.create, sizeof(struct nss_udp_st_tx_create));

	if (num.msg.create.mode == NSS_UDP_ST_MODE_DEFAULT) {
		global.mode = NSS_UDP_ST_MODE_DEFAULT;
	} else {
		global.mode = NSS_UDP_ST_MODE_TIMESTAMP;

		/*
		 * Get the linux epoch time.
		 */
		num.msg.create.timestamp = ktime_get_real_ns();
	}

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return 0;
}

/*
 * nss_nludp_st_get_neigh_ipv4()
 *	Returns neighbour reference for a given IP address
 */
static struct neighbour *nss_nludp_st_get_neigh_ipv4(uint32_t ip_addr)
{
	struct neighbour *neigh;
	struct rtable *rt;
	struct dst_entry *dst;

	/*
	 * search for route entry
	 */
	rt = ip_route_output(&init_net, ip_addr, 0, 0, 0);
	if (IS_ERR(rt)) {
		return NULL;
	}

	dst = (struct dst_entry *)rt;

	/*
	 * neighbour lookup using IP address in the route table
	 */
	neigh = dst_neigh_lookup(dst, &ip_addr);
	if (likely(neigh)) {
		dst_release(dst);
		return neigh;
	}

	/*
	 * neighbour lookup using IP address, device in the arp table
	 */
	neigh = neigh_lookup(&arp_tbl, &ip_addr, dst->dev);
	if (likely(neigh)) {
		dst_release(dst);
		return neigh;
	}

	/*
	 * dst reference count was held during the lookup
	 */
	dst_release(dst);
	return NULL;
}

/*
 * nss_nludp_st_mac_addr_get_ipv4()
 *	Return the hardware (MAC) address of the given IPv4 address, if any.
 *
 * Returns 0 on success or a negative result on failure.
 * We look up the rtable entry for the address and,
 * from its neighbour structure,obtain the hardware address.
 * This means we will also work if the neighbours are routers too.
 */
static int nss_nludp_st_get_macaddr_ipv4(uint32_t ip_addr, uint8_t mac_addr[])
{
	struct neighbour *neigh;

	/*
	 * handle multicast IP address seperately
	 */
	if (ipv4_is_multicast(htonl(ip_addr))) {
		/*
		 * fixed
		 */
		mac_addr[0] = 0x01;
		mac_addr[1] = 0x00;
		mac_addr[2] = 0x5e;

		/*
		 * from ip_addr
		 */
		mac_addr[3] = (htonl(ip_addr) & 0x7f0000) >> 16;
		mac_addr[4] = (htonl(ip_addr) & 0xff00) >> 8;
		mac_addr[5] = (htonl(ip_addr) & 0xff);

		return 0;
	}

	/*
	 * retrieve the neighbour
	 */
	rcu_read_lock();
	neigh = nss_nludp_st_get_neigh_ipv4(htonl(ip_addr));
	if (!neigh) {
		rcu_read_unlock();
		nss_nl_info("neighbour lookup failed for IP:0x%x\n", htonl(ip_addr));
		return -ENODEV;
	}
	rcu_read_unlock();

	if ((neigh->nud_state & NUD_VALID) == 0) {
		nss_nl_info("neighbour state is invalid for IP:0x%x\n", htonl(ip_addr));
		goto fail;
	}

	if (!neigh->dev) {
		nss_nl_info("neighbour device not found for IP:0x%x\n", htonl(ip_addr));
		goto fail;
	}

	if (is_multicast_ether_addr(neigh->ha)) {
		nss_nl_info( "neighbour MAC address is multicast or broadcast\n");
		goto fail;
	}

	ether_addr_copy(mac_addr, neigh->ha);
	neigh_release(neigh);
	return 0;

fail:
	neigh_release(neigh);
	return -ENODEV;
}

/*
 * nss_nludp_st_get_neigh_ipv6()
 *	Returns neighbour reference for a given IP address
 */
static struct neighbour *nss_nludp_st_get_neigh_ipv6(uint32_t dst_addr[4])
{
	struct neighbour *neigh;
	struct dst_entry *dst;
	struct rt6_info *rt;
	struct in6_addr daddr;

	IPV6_ADDR_TO_IN6_ADDR(daddr, dst_addr);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0))
	rt = rt6_lookup(&init_net, &daddr, NULL, 0, 0);
#else
	rt = rt6_lookup(&init_net, &daddr, NULL, 0, NULL, 0);
#endif
	if (!rt) {
		nss_nl_warn("rt6 info lookup failed\n");
		return NULL;
	}

	dst = (struct dst_entry *)rt;

	/*
	 * neighbour lookup using IP address in the route table
	 */
	neigh = dst_neigh_lookup(dst, &daddr);
	if (likely(neigh)) {
		neigh_hold(neigh);
		dst_release(dst);
		return neigh;
	}
	dst_release(dst);

	nss_nl_warn("dst neigh info lookup failed\n");
	return NULL;
}

/*
 * nss_nludp_st_get_addr_hton()
 *	Convert the ipv6 address from NSS host order to Linux network order.
 */
static inline void nss_nludp_st_get_addr_hton(uint32_t src[4], uint32_t dst[4])
{
	dst[0] = htonl(src[0]);
	dst[1] = htonl(src[1]);
	dst[2] = htonl(src[2]);
	dst[3] = htonl(src[3]);
}

/*
 * nss_nludp_st_mac_addr_get_ipv6()
 *	Return the hardware (MAC) address of the given ipv6 address, if any.
 *
 * Returns 0 on success or a negative result on failure.
 * We look up the rtable entry for the address and,
 * from its neighbour structure,obtain the hardware address.
 * This means we will also work if the neighbours are routers too.
 */
static int nss_nludp_st_get_macaddr_ipv6(uint32_t ip_addr[4], uint8_t mac_addr[])
{
	struct neighbour *neigh;
	struct  in6_addr addr;

	nss_nludp_st_get_addr_hton(ip_addr, addr.s6_addr32);

	if (ipv6_addr_is_multicast(&addr)) {
		/*
		 * fixed
		 */
		mac_addr[0] = 0x33;
		mac_addr[1] = 0x33;

		/*
		 * Last word of the IPv6 address.
		 */
		memcpy(&mac_addr[2], &addr.s6_addr32[3], sizeof(uint32_t));
		return 0;
	}

	rcu_read_lock();

	/*
	 * retrieve the neighbour
	 */
	neigh = nss_nludp_st_get_neigh_ipv6(addr.s6_addr32);
	if (!neigh) {
		rcu_read_unlock();
		nss_nl_info("neighbour lookup failed for %pI6c\n", addr.s6_addr32);
		return -ENODEV;
	}

	rcu_read_unlock();

	if ((neigh->nud_state & NUD_VALID) == 0) {
		nss_nl_info("neighbour state is invalid for %pI6c\n", addr.s6_addr32);
		goto fail;
	}

	if (!neigh->dev) {
		nss_nl_info("neighbour device not found for %pI6c\n", addr.s6_addr32);
		goto fail;
	}

	if (is_multicast_ether_addr(neigh->ha)) {
		nss_nl_info("neighbour MAC address is multicast or broadcast\n");
		goto fail;
	}

	ether_addr_copy(mac_addr, neigh->ha);
	neigh_release(neigh);
	return 0;
fail:
	neigh_release(neigh);
	return -ENODEV;
}

/*
 * nss_nludp_st_destroy_ipv4_rule()
 *	Destroy a nss entry to accelerate the given IPV4 connection
 */
static int nss_nludp_st_destroy_ipv4_rule(struct sk_buff *skb, struct nss_nludp_st_rule *nl_rule)
{
	struct nss_ipv4_rule_destroy_msg *nirdm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv4_msg nim;
	struct sk_buff *resp;
	nss_tx_status_t status;

	nss_ctx = nss_ipv4_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv4 ctx\n", nl_rule);
		return -EPERM;
	}

	nss_nl_info("%px: IPv4 rule: src:%pI4h:%d dst:%pI4h:%d p:%d\n",
		nl_rule, &nl_rule->num.msg.uncfg.src_ip.ip.ipv4, nl_rule->num.msg.uncfg.src_port,
		&nl_rule->num.msg.uncfg.dest_ip.ip.ipv4, nl_rule->num.msg.uncfg.dest_port, IPPROTO_UDP);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	memset(&nim, 0, sizeof(struct nss_ipv4_msg));
	nss_ipv4_msg_init(&nim,
			NSS_IPV4_RX_INTERFACE,
			NSS_IPV4_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv4_rule_destroy_msg),
			NULL,
			NULL);

	nirdm = &nim.msg.rule_destroy;

	/*
	 * Copy over the 5 tuple details.
	 */
	nirdm->tuple.protocol = (uint8_t)IPPROTO_UDP;
	nirdm->tuple.flow_ip = nl_rule->num.msg.uncfg.src_ip.ip.ipv4;
	nirdm->tuple.flow_ident = (uint32_t)nl_rule->num.msg.uncfg.src_port;
	nirdm->tuple.return_ip = nl_rule->num.msg.uncfg.dest_ip.ip.ipv4;
	nirdm->tuple.return_ident = (uint32_t)nl_rule->num.msg.uncfg.dest_port;

	status = nss_ipv4_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Destroy IPv4 message failed %d\n", nss_ctx, status);
		return -EPERM;
	}

	return 0;
}

/*
 * nss_nludp_st_vlan_next_dev_get_and_hold()
 *	Gets and hold the next device of the VLAN.
 *
 * It should be a physical ethernet interface.
 */
static struct net_device *nss_nludp_st_vlan_next_dev_get_and_hold(struct net_device *dev)
{
	struct net_device *next_dev;

	next_dev = vlan_dev_next_dev(dev);
	if (!next_dev) {
		nss_nl_warn("%px: VLAN device's (%s) next dev is NULL\n", dev, dev->name);
		return NULL;
	}

	if (is_vlan_dev(next_dev) || next_dev->type != ARPHRD_ETHER) {
		nss_nl_warn("%px: QinQ or non-ethernet VLAN master (%s) is not supported\n", dev, next_dev->name);
		return NULL;
	}
	dev_hold(next_dev);
	return next_dev;
}

/*
 * nss_nludp_st_ipv4_rawip_iface_config()
 *	Configure the WAN interface as RmNet for IPv4 protocol.
 */
static int nss_nludp_st_ipv4_rawip_iface_config(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm)
{
#ifdef NSS_NETLINK_UDP_ST_NO_RMNET_SUPPORT
	nss_nl_warn("%px: RAWIP is disabled\n", dev);
	return -EINVAL;
#else
	nircm->conn_rule.return_interface_num = nss_rmnet_rx_get_ifnum(dev);
	memset(nircm->conn_rule.return_mac, 0, ETH_ALEN);
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);
	nss_nl_info("%px: Speedtest RmNet WAN interface %s is configured\n", dev, dev->name);
	return 0;
#endif
}

/*
 * nss_nludp_st_ipv4_eth_iface_config()
 *	Configure the WAN interface as Ethernet for IPv4 protocol.
 */
static int nss_nludp_st_ipv4_eth_iface_config(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm)
{
	if (nss_nludp_st_get_macaddr_ipv4(nircm->tuple.return_ip, (uint8_t *)&nircm->conn_rule.return_mac)) {
		nss_nl_warn("%px: Error in updating the return MAC Address\n", dev);
		return -EINVAL;
	}
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);
	nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(dev);
	if (nircm->conn_rule.return_interface_num < 0) {
		nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, dev->name);
		return -EINVAL;
	}
	nss_nl_info("%px: Speedtest Ethernet WAN interface %s is configured\n", dev, dev->name);
	return 0;
}

/*
 * nss_nludp_st_ipv4_set_vlan_tags()
 *	Sets the VLAN tags for IPv4 protocol.
 */
static void nss_nludp_st_ipv4_set_vlan_tags(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm)
{
	/*
	 * Get the primary VLAN info and set in the rule.
	 */
	uint16_t vlan_tag = vlan_dev_vlan_id(dev);
	uint16_t vlan_tpid = ntohs(vlan_dev_vlan_proto(dev));

	nircm->vlan_primary_rule.egress_vlan_tag = (vlan_tpid << 16) | vlan_tag;
	nircm->vlan_primary_rule.ingress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.egress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.ingress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
}

/*
 * nss_nludp_st_ipv4_vlan_iface_config()
 *	Configure the WAN interface as VLAN for IPv4 protocol.
 */
static int nss_nludp_st_ipv4_vlan_iface_config(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm)
{
	struct net_device *next_dev;
	int ret;

	next_dev = nss_nludp_st_vlan_next_dev_get_and_hold(dev);
	if (!next_dev) {
		nss_nl_warn("%px: Unable to get VLAN device's (%s) next dev\n", dev, dev->name);
		return -ENODEV;
	}

	/*
	 * Single VLAN, next_dev should be a physical device.
	 * Copy the real device's number (eth0).
	 */
	nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(next_dev);
	if (nircm->conn_rule.return_interface_num < 0) {
		nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, next_dev->name);
		ret = -EINVAL;
		goto fail;
	}
	dev_put(next_dev);

	/*
	 * Get the return IP address's MAC address.
	 */
	if (nss_nludp_st_get_macaddr_ipv4(nircm->tuple.return_ip, (uint8_t *)&nircm->conn_rule.return_mac)) {
		nss_nl_warn("%px: Error in updating the return MAC Address\n", dev);
		return -EINVAL;
	}

	/*
	 * Get the primary VLAN info and set in the rule.
	 */
	nss_nludp_st_ipv4_set_vlan_tags(dev, nircm);
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;

	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);

	/*
	 * Success. Fall through for release resources
	 */
	nss_nl_info("%px: Speedtest VLAN WAN interface %s is configured\n", dev, dev->name);
	return 0;
fail:
	dev_put(next_dev);
	return ret;
}

/*
 * nss_nludp_st_pppoe_get_chan_and_info()
 *	Gets the PPPoE channel information.
 */
static inline bool nss_nludp_st_pppoe_get_chan_and_info(struct net_device *dev, struct ppp_channel **ppp_chan, struct pppoe_opt *info)
{
	int channel_count;
	int channel_protocol;

	channel_count = ppp_hold_channels(dev, ppp_chan, 1);
	if (channel_count != 1) {
		nss_nl_warn("%px: Unable to get the channel for device: %s\n", dev, dev->name);
		return false;
	}

	channel_protocol = ppp_channel_get_protocol(ppp_chan[0]);
	if (channel_protocol != PX_PROTO_OE) {
		nss_nl_warn("%px: PPP channel protocol is not PPPoE for device: %s\n", dev, dev->name);
		ppp_release_channels(ppp_chan, 1);
		return false;
	}

	if (pppoe_channel_addressing_get(ppp_chan[0], info)) {
		nss_nl_warn("%px: Unable to get the PPPoE session information for device: %s\n", dev, dev->name);
		ppp_release_channels(ppp_chan, 1);
		return false;
	}

	return true;
}

/*
 * nss_nludp_st_ipv4_pppoe_iface_config()
 *	Configure the WAN interface as PPPoE for IPv4 protocol.
 */
static int nss_nludp_st_ipv4_pppoe_iface_config(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm)
{
	struct pppoe_opt info;
	struct ppp_channel *ppp_chan[1];
	int ret = -EINVAL;

	if (!nss_nludp_st_pppoe_get_chan_and_info(dev, ppp_chan, &info)) {
		nss_nl_warn("%px: Unable to get PPPoE channel and info for device: %s\n", dev, dev->name);
		return ret;
	}

	/*
	 * Check if the next device is a VLAN (eth0-eth0.100-pppoe-wan
	 */
	if (is_vlan_dev(info.dev)) {
		/*
		 * Next device is a VLAN device (eth0.100).
		 */
		struct net_device *next_dev;

		/*
		 * Check if we have a single VLAN device.
		 */
		next_dev = nss_nludp_st_vlan_next_dev_get_and_hold(info.dev);
		if (!next_dev) {
			nss_nl_warn("%px: Unable to get PPPoE's VLAN device's (%s) next dev\n", dev, info.dev->name);
			ret = -ENODEV;
			goto fail;
		}

		/*
		 * PPPoE + VLAN (eth0-eth0.100-pppoe-wan)
		 * Copy the physical interface number (eth0)
		 */
		nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(next_dev);
		if (nircm->conn_rule.return_interface_num < 0) {
			nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, next_dev->name);
			dev_put(next_dev);
			goto fail;
		}
		dev_put(next_dev);

		/*
		 * Get the primary VLAN info and set in the rule.
		 */
		nss_nludp_st_ipv4_set_vlan_tags(info.dev, nircm);
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;
	} else {
		/*
		 * PPPoE interface can be created on linux bridge, OVS bridge and LAG devices.
		 * udp_st doesn't support these hierarchies.
		 */
		if ((info.dev->priv_flags & (IFF_EBRIDGE | IFF_OPENVSWITCH))
				|| ((info.dev->flags & IFF_MASTER) && (info.dev->priv_flags & IFF_BONDING))) {
			nss_nl_warn("%px: PPPoE over bridge and LAG interfaces are not supported, dev: %s info.dev: %s\n",
					dev, dev->name, info.dev->name);
			goto fail;
		}

		/*
		 * PPPoE only (eth0-pppoe-wan)
		 * Copy the physical interface number (eth0)
		 */
		nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(info.dev);
		if (nircm->conn_rule.return_interface_num < 0) {
			nss_nl_warn("%px: Unable to get the PPPoE's physical interface's (%s) NSS interface number\n",
					dev, info.dev->name);
			goto fail;
		}
	}

	/*
	 * For PPPoE connections, the other end's MAC address is PPPoE session's remote MAC address
	 */
	ether_addr_copy((uint8_t *)nircm->conn_rule.return_mac, info.pa.remote);
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, info.dev->dev_addr);

	/*
	 * Check if NSS has a valid dynamic interface number for PPPoE interface.
	 * If there is not, NSS cannot handle PPPoE flows in fast path.
	 */
	nircm->pppoe_rule.return_if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (nircm->pppoe_rule.return_if_num < 0) {
		nss_nl_warn("%px: Unable to get the PPPoE interfaces (%s) dynamic interface number\n", dev, dev->name);
		goto fail;
	}
	nircm->pppoe_rule.return_if_exist = 1;
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_PPPOE_VALID;

	/*
	 * Success. Fall through for release resources
	 */
	ret = 0;
	nss_nl_info("%px: Speedtest PPPoE WAN interface %s is configured\n", dev, dev->name);
fail:
	dev_put(info.dev);
	ppp_release_channels(ppp_chan, 1);
	return ret;
}

/*
 * nss_nludp_st_create_ipv4_rule()
 *	Create a nss entry to accelerate the given IPv4 connection
 */
static int nss_nludp_st_create_ipv4_rule(struct sk_buff *skb, struct nss_nludp_st_rule *nl_rule)
{
	struct nss_ipv4_rule_create_msg *nircm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv4_msg nim;
	struct net_device *net_dev;
	struct nss_ipv4_nexthop *nexthop;
	struct sk_buff *resp;
	struct nss_ipv4_src_mac_rule *src_mac;
	nss_tx_status_t status;
	int ret = 0;

	nss_ctx = nss_ipv4_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv4 ctx\n", nl_rule);
		return -EPERM;
	}

	nss_nl_info("%px: IPv4 rule: src:%pI4h:%d dst:%pI4h:%d p:%d\n",
		nl_rule, &nl_rule->num.msg.cfg.src_ip.ip.ipv4, nl_rule->num.msg.cfg.src_port,
		&nl_rule->num.msg.cfg.dest_ip.ip.ipv4, nl_rule->num.msg.cfg.dest_port, IPPROTO_UDP);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	memset(&nim, 0, sizeof(struct nss_ipv4_msg));
	nss_ipv4_msg_init(&nim,
			NSS_IPV4_RX_INTERFACE,
			NSS_IPV4_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv4_rule_create_msg),
			NULL,
			NULL);

	nircm = &nim.msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Copy over the 5 tuple details.
	 */
	nircm->tuple.protocol = (uint8_t)IPPROTO_UDP;
	nircm->tuple.flow_ip = nl_rule->num.msg.cfg.src_ip.ip.ipv4;
	nircm->conn_rule.flow_ip_xlate = nl_rule->num.msg.cfg.src_ip.ip.ipv4;
	nircm->tuple.flow_ident = (uint32_t)nl_rule->num.msg.cfg.src_port;
	nircm->conn_rule.flow_ident_xlate = (uint32_t)nl_rule->num.msg.cfg.src_port;
	nircm->tuple.return_ip = nl_rule->num.msg.cfg.dest_ip.ip.ipv4;
	nircm->conn_rule.return_ip_xlate = nl_rule->num.msg.cfg.dest_ip.ip.ipv4;
	nircm->tuple.return_ident = (uint32_t)nl_rule->num.msg.cfg.dest_port;
	nircm->conn_rule.return_ident_xlate = (uint32_t)nl_rule->num.msg.cfg.dest_port;

	/*
	 * extract netdevice
	 */
	net_dev = dev_get_by_name(&init_net, nl_rule->gmac_ifname);
	if (!net_dev) {
		nss_nl_error("%p: net device(%s) is not available\n", nl_rule, nl_rule->gmac_ifname);
		return -EINVAL;
	}

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = NSS_UDP_ST_INTERFACE;

	/*
	 * Set the MTU values of the flows.
	 */
	nircm->conn_rule.flow_mtu = NSS_NLUDP_ST_MAX_MTU;
	nircm->conn_rule.return_mtu = net_dev->mtu;

	/*
	 * Supported WAN interface hierarchies:
	 *
	 * rmnet <-> DUT (5G interface)
	 * eth0-eth0.100 <-> DUT  (single VLAN)
	 * eth0-pppoe-wan <-> DUT ( PPPoE only)
	 * eth0-eth0.100-pppoe-wan <-> DUT (PPPoE over VLAN)
	 * eth0 <-> DUT (simple physical interface)
	 */
	switch (net_dev->type) {
	case ARPHRD_RAWIP:
		ret = nss_nludp_st_ipv4_rawip_iface_config(net_dev, nircm);
		break;

	case ARPHRD_PPP:
		ret = nss_nludp_st_ipv4_pppoe_iface_config(net_dev, nircm);
		break;

	case ARPHRD_ETHER:
		/*
		 * Bridge and LAG interfaces are not supported.
		 */
		if ((net_dev->priv_flags & (IFF_EBRIDGE | IFF_OPENVSWITCH))
				|| ((net_dev->flags & IFF_MASTER) && (net_dev->priv_flags & IFF_BONDING))) {
			nss_nl_warn("%px: Bridge and LAG interfaces are not supported, dev: %s\n", skb, net_dev->name);
			ret = -EINVAL;
			break;
		}

		if (is_vlan_dev(net_dev)) {
			ret = nss_nludp_st_ipv4_vlan_iface_config(net_dev, nircm);
		} else {
			ret = nss_nludp_st_ipv4_eth_iface_config(net_dev, nircm);
		}
		break;
	default:
		nss_nl_warn("%px: unsupported speedtest interface: %s\n", skb, net_dev->name);
		ret = -EINVAL;
		break;
	}


	if (ret < 0) {
		goto done;
	}

	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_CONN_VALID;

	nexthop = &nircm->nexthop_rule;
	nexthop->flow_nexthop = nircm->conn_rule.flow_interface_num;
	nexthop->return_nexthop = nircm->conn_rule.return_interface_num;

	nss_nl_info("%px: flow_nexthop:%d return_nexthop:%d\n", skb, nexthop->flow_nexthop, nexthop->return_nexthop);

	/*
	 * We want acceleration engine to choose the source MAC address which we
	 * have given to it instead of using the underlying physical interface's MAC address.
	 */
	src_mac = &nircm->src_mac_rule;

	memcpy(src_mac->flow_src_mac, nircm->conn_rule.return_mac, ETH_ALEN);
	memcpy(src_mac->return_src_mac, nircm->conn_rule.flow_mac, ETH_ALEN);

	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_SRC_MAC_VALID;
	src_mac->mac_valid_flags |= (NSS_IPV4_SRC_MAC_FLOW_VALID | NSS_IPV4_SRC_MAC_RETURN_VALID);

	status = nss_ipv4_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create IPv4 message failed %d\n", skb, status);
	}
done:
	dev_put(net_dev);
	return ret;
}

/*
 * nss_nludp_st_destroy_ipv6_rule()
 *	Destroy a nss entry to accelerate the given IPV6 connection
 */
static int nss_nludp_st_destroy_ipv6_rule(struct sk_buff *skb, struct nss_nludp_st_rule *nl_rule)
{
	struct nss_ipv6_rule_destroy_msg *nirdm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv6_msg nim;
	struct sk_buff *resp;
	nss_tx_status_t status;

	nss_ctx = nss_ipv6_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv6 ctx\n", nl_rule);
		return -EPERM;
	}

	nss_nl_info("%px: IPv6 rule: src:%pI4h:%d dst:%pI4h:%d p:%d\n",
		nl_rule, nl_rule->num.msg.uncfg.src_ip.ip.ipv6, nl_rule->num.msg.uncfg.src_port,
		nl_rule->num.msg.uncfg.dest_ip.ip.ipv6, nl_rule->num.msg.uncfg.dest_port, IPPROTO_UDP);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	memset(&nim, 0, sizeof(struct nss_ipv6_msg));
	nss_ipv6_msg_init(&nim,
			NSS_IPV6_RX_INTERFACE,
			NSS_IPV6_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv6_rule_destroy_msg),
			NULL,
			NULL);

	nirdm = &nim.msg.rule_destroy;

	/*
	 * Copy over the 5 tuple details.
	 */
	nirdm->tuple.protocol = (uint8_t)IPPROTO_UDP;
	memcpy(nirdm->tuple.flow_ip, nl_rule->num.msg.uncfg.src_ip.ip.ipv6, sizeof(uint32_t) * 4);
	nirdm->tuple.flow_ident = (uint32_t)nl_rule->num.msg.uncfg.src_port;
	memcpy(nirdm->tuple.return_ip, nl_rule->num.msg.uncfg.dest_ip.ip.ipv6, sizeof(uint32_t) * 4);
	nirdm->tuple.return_ident = (uint32_t)nl_rule->num.msg.uncfg.dest_port;

	status = nss_ipv6_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Destroy IPv6 message failed %d\n", nss_ctx, status);
		return -EPERM;
	}

	return 0;
}

/*
 * nss_nludp_st_ipv6_rawip_iface_config()
 *	Configure the WAN interface as RmNet for IPv6 protocol.
 */
static int nss_nludp_st_ipv6_rawip_iface_config(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm)
{
#ifdef NSS_NETLINK_UDP_ST_NO_RMNET_SUPPORT
	nss_nl_warn("%px: RAWIP is disabled\n", dev);
	return -EINVAL;
#else
	nircm->conn_rule.return_interface_num = nss_rmnet_rx_get_ifnum(dev);
	memset(nircm->conn_rule.return_mac, 0, ETH_ALEN);
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);
	nss_nl_info("%px: Speedtest RmNet WAN interface %s is configured\n", dev, dev->name);
	return 0;
#endif
}

/*
 * nss_nludp_st_ipv6_eth_iface_config()
 *	Configure the WAN interface as Ethernet for IPv6 protocol.
 */
static int nss_nludp_st_ipv6_eth_iface_config(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm)
{
	if (nss_nludp_st_get_macaddr_ipv6(nircm->tuple.return_ip, (uint8_t *)&nircm->conn_rule.return_mac)) {
		nss_nl_warn("%px: Error in updating the return MAC Address\n", dev);
		return -EINVAL;
	}
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);
	nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(dev);
	if (nircm->conn_rule.return_interface_num < 0) {
		nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, dev->name);
		return -EINVAL;
	}
	nss_nl_info("%px: Speedtest Ethernet WAN interface %s is configured\n", dev, dev->name);
	return 0;
}

/*
 * nss_nludp_st_ipv6_set_vlan_tags()
 *	Sets the VLAN tags for IPv6 protocol.
 */
static void nss_nludp_st_ipv6_set_vlan_tags(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm)
{
	/*
	 * Get the primary VLAN info and set in the rule.
	 */
	uint16_t vlan_tag = vlan_dev_vlan_id(dev);
	uint16_t vlan_tpid = ntohs(vlan_dev_vlan_proto(dev));

	nircm->vlan_primary_rule.egress_vlan_tag = (vlan_tpid << 16) | vlan_tag;
	nircm->vlan_primary_rule.ingress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.egress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.ingress_vlan_tag = NSS_NLUDP_ST_VLAN_ID_NOT_CONFIGURED;
}

/*
 * nss_nludp_st_ipv6_vlan_iface_config()
 *	Configure the WAN interface as VLAN for IPv6 protocol.
 */
static int nss_nludp_st_ipv6_vlan_iface_config(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm)
{
	struct net_device *next_dev;
	int ret;

	next_dev = nss_nludp_st_vlan_next_dev_get_and_hold(dev);
	if (!next_dev) {
		nss_nl_warn("%px: Unable to get VLAN device's (%s) next dev\n", dev, dev->name);
		return -ENODEV;
	}

	/*
	 * Single VLAN, next_dev should be a physical device.
	 * Copy the real device's number (eth0).
	 */
	nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(next_dev);
	if (nircm->conn_rule.return_interface_num < 0) {
		nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, next_dev->name);
		ret = -EINVAL;
		goto fail;
	}
	dev_put(next_dev);

	/*
	 * Get the return IP address's MAC address.
	 */
	if (nss_nludp_st_get_macaddr_ipv6(nircm->tuple.return_ip, (uint8_t *)&nircm->conn_rule.return_mac)) {
		nss_nl_warn("%px: Error in updating the return MAC Address\n", dev);
		return -EINVAL;
	}

	/*
	 * Get the primary VLAN info and set in the rule.
	 */
	nss_nludp_st_ipv6_set_vlan_tags(dev, nircm);
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_VLAN_VALID;

	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, dev->dev_addr);

	/*
	 * Success. Fall through for release resources
	 */
	nss_nl_info("%px: Speedtest VLAN WAN interface %s is configured\n", dev, dev->name);
	return 0;
fail:
	dev_put(next_dev);
	return ret;
}

/*
 * nss_nludp_st_ipv6_pppoe_iface_config()
 *	Configure the WAN interface as PPPoE for IPv6 protocol.
 */
static int nss_nludp_st_ipv6_pppoe_iface_config(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm)
{
	struct pppoe_opt info;
	struct ppp_channel *ppp_chan[1];
	int ret = -EINVAL;

	if (!nss_nludp_st_pppoe_get_chan_and_info(dev, ppp_chan, &info)) {
		nss_nl_warn("%px: Unable to get PPPoE channel and info for device: %s\n", dev, dev->name);
		return ret;
	}

	/*
	 * Check if the next device is a VLAN (eth0-eth0.100-pppoe-wan
	 */
	if (is_vlan_dev(info.dev)) {
		/*
		 * Next device is a VLAN device (eth0.100).
		 */
		struct net_device *next_dev;

		/*
		 * Check if we have a single VLAN device.
		 */
		next_dev = nss_nludp_st_vlan_next_dev_get_and_hold(info.dev);
		if (!next_dev) {
			nss_nl_warn("%px: Unable to get PPPoE's VLAN device's (%s) next dev\n", dev, info.dev->name);
			ret = -ENODEV;
			goto fail;
		}

		/*
		 * PPPoE + VLAN (eth0-eth0.100-pppoe-wan)
		 * Copy the physical interface number (eth0)
		 */
		nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(next_dev);
		if (nircm->conn_rule.return_interface_num < 0) {
			nss_nl_warn("%px: Unable to get the NSS interface number of %s\n", dev, next_dev->name);
			dev_put(next_dev);
			goto fail;
		}
		dev_put(next_dev);

		/*
		 * Get the primary VLAN info and set in the rule.
		 */
		nss_nludp_st_ipv6_set_vlan_tags(info.dev, nircm);
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;
	} else {
		/*
		 * PPPoE interface can be created on linux bridge, OVS bridge and LAG devices.
		 * udp_st doesn't support these hierarchies.
		 */
		if ((info.dev->priv_flags & (IFF_EBRIDGE | IFF_OPENVSWITCH))
				|| ((info.dev->flags & IFF_MASTER) && (info.dev->priv_flags & IFF_BONDING))) {
			nss_nl_warn("%px: PPPoE over bridge and LAG interfaces are not supported, dev: %s info.dev: %s\n",
					dev, dev->name, info.dev->name);
			goto fail;
		}

		/*
		 * PPPoE only (eth0-pppoe-wan)
		 * Copy the physical interface number (eth0)
		 */
		nircm->conn_rule.return_interface_num = nss_cmn_get_interface_number_by_dev(info.dev);
		if (nircm->conn_rule.return_interface_num < 0) {
			nss_nl_warn("%px: Unable to get the PPPoE's physical interface's (%s) NSS interface number\n",
					dev, info.dev->name);
			goto fail;
		}
	}

	/*
	 * For PPPoE connections, the other end's MAC address is PPPoE session's remote MAC address
	 */
	ether_addr_copy((uint8_t *)nircm->conn_rule.return_mac, info.pa.remote);
	ether_addr_copy((uint8_t *)nircm->conn_rule.flow_mac, info.dev->dev_addr);

	/*
	 * Check if NSS has a valid dynamic interface number for PPPoE interface.
	 * If there is not, NSS cannot handle PPPoE flows in fast path.
	 */
	nircm->pppoe_rule.return_if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (nircm->pppoe_rule.return_if_num < 0) {
		nss_nl_warn("%px: Unable to get the PPPoE interfaces (%s) dynamic interface number\n", dev, dev->name);
		goto fail;
	}
	nircm->pppoe_rule.return_if_exist = 1;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_PPPOE_VALID;

	/*
	 * Success. Fall through for release resources
	 */
	ret = 0;
	nss_nl_info("%px: Speedtest PPPoE WAN interface %s is configured\n", dev, dev->name);
fail:
	dev_put(info.dev);
	ppp_release_channels(ppp_chan, 1);
	return ret;
}

/*
 * nss_nludp_st_create_ipv6_rule()
 *	Create a nss entry to accelerate the given IPV6 connection
 */
static int nss_nludp_st_create_ipv6_rule(struct sk_buff *skb, struct nss_nludp_st_rule *nl_rule)
{
	struct nss_ipv6_rule_create_msg *nircm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv6_msg nim;
	struct net_device *net_dev;
	struct nss_ipv6_nexthop *nexthop;
	struct sk_buff *resp;
	struct nss_ipv6_src_mac_rule *src_mac;
	nss_tx_status_t status;
	int ret = 0;

	nss_ctx = nss_ipv6_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv6 ctx\n", nl_rule);
		return -EPERM;
	}

	nss_nl_info("%px: Create IPv6 rule: %pI6:%d %pI6:%d p:%d\n",
		nl_rule, nl_rule->num.msg.cfg.src_ip.ip.ipv6, nl_rule->num.msg.cfg.src_port,
		nl_rule->num.msg.cfg.dest_ip.ip.ipv6, nl_rule->num.msg.cfg.dest_port, IPPROTO_UDP);

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%p:unable to save response data from NL buffer\n", nl_rule);
		return -ENOMEM;
	}

	memset(&nim, 0, sizeof(struct nss_ipv6_msg));
	nss_ipv6_msg_init(&nim,
			NSS_IPV6_RX_INTERFACE,
			NSS_IPV6_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv6_rule_create_msg),
			NULL,
			NULL);

	nircm = &nim.msg.rule_create;
	nircm->rule_flags = 0;
	nircm->valid_flags = 0;

	/*
	 * Copy over the 5 tuple information.
	 */
	nircm->tuple.protocol = (uint8_t)IPPROTO_UDP;
	memcpy(nircm->tuple.flow_ip, nl_rule->num.msg.cfg.src_ip.ip.ipv6, sizeof(nircm->tuple.flow_ip));
	memcpy(nircm->tuple.return_ip, nl_rule->num.msg.cfg.dest_ip.ip.ipv6, sizeof(nircm->tuple.return_ip));
	nircm->tuple.flow_ident = (uint32_t)nl_rule->num.msg.cfg.src_port;
	nircm->tuple.return_ident = (uint32_t)nl_rule->num.msg.cfg.dest_port;

	/*
	 * extract netdevice
	 */
	net_dev = dev_get_by_name(&init_net, nl_rule->gmac_ifname);
	if (!net_dev) {
		nss_nl_error("%p: net device(%s) is not available\n", nl_rule, nl_rule->gmac_ifname);
		return -EINVAL;
	}

	/*
	 * Copy over the connection rules and set CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = NSS_UDP_ST_INTERFACE;

	/*
	 * Set the MTU values of the flows.
	 */
	nircm->conn_rule.flow_mtu = NSS_NLUDP_ST_MAX_MTU;
	nircm->conn_rule.return_mtu = net_dev->mtu;

	/*
	 * Supported WAN interface hierarchies:
	 *
	 * rmnet <-> DUT (5G interface)
	 * eth0-eth0.100 <-> DUT  (single VLAN)
	 * eth0-pppoe-wan <-> DUT ( PPPoE only)
	 * eth0-eth0.100-pppoe-wan <-> DUT (PPPoE over VLAN)
	 * eth0 <-> DUT (simple physical interface)
	 */
	switch (net_dev->type) {
	case ARPHRD_RAWIP:
		ret = nss_nludp_st_ipv6_rawip_iface_config(net_dev, nircm);
		break;

	case ARPHRD_PPP:
		ret = nss_nludp_st_ipv6_pppoe_iface_config(net_dev, nircm);
		break;

	case ARPHRD_ETHER:
		/*
		 * Bridge and LAG interfaces are not supported.
		 */
		if ((net_dev->priv_flags & (IFF_EBRIDGE | IFF_OPENVSWITCH))
				|| ((net_dev->flags & IFF_MASTER) && (net_dev->priv_flags & IFF_BONDING))) {
			nss_nl_warn("%px: Bridge and LAG interfaces are not supported, dev: %s\n", skb, net_dev->name);
			ret = -EINVAL;
			break;
		}

		if (is_vlan_dev(net_dev)) {
			ret = nss_nludp_st_ipv6_vlan_iface_config(net_dev, nircm);
		} else {
			ret = nss_nludp_st_ipv6_eth_iface_config(net_dev, nircm);
		}
		break;

	default:
		nss_nl_warn("%px: unsupported speedtest interface: %s\n", skb, net_dev->name);
		ret = -EINVAL;
		break;
	}

	if (ret < 0) {
		goto done;
	}

	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_CONN_VALID;

	nexthop = &nircm->nexthop_rule;
	nexthop->flow_nexthop = nircm->conn_rule.flow_interface_num;
	nexthop->return_nexthop = nircm->conn_rule.return_interface_num;

	nss_nl_info("flow_nexthop:%d return_nexthop:%d\n", nexthop->flow_nexthop, nexthop->return_nexthop);

	/*
	 * We want acceleration engine to choose the source MAC address which we
	 * have given to it instead of using the underlying physical interface's MAC address.
	 */
	src_mac = &nircm->src_mac_rule;

	memcpy(src_mac->flow_src_mac, nircm->conn_rule.return_mac, ETH_ALEN);
	memcpy(src_mac->return_src_mac, nircm->conn_rule.flow_mac, ETH_ALEN);

	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_SRC_MAC_VALID;
	src_mac->mac_valid_flags |= (NSS_IPV6_SRC_MAC_FLOW_VALID | NSS_IPV6_SRC_MAC_RETURN_VALID);

	status = nss_ipv6_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create IPv6 message failed %d\n", nss_ctx, status);
	}
done:
	dev_put(net_dev);
	return ret;
}

/*
 * nss_nludp_st_uncfg_udp_st_rule()
 *	Unconfigures the entry from udp_st
 */
static int nss_nludp_st_uncfg_udp_st_rule(struct sk_buff *skb, struct nss_udp_st_cfg *uncfg)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	nss_tx_status_t status;
	int ret = 0;

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("unable to save response data from NL buffer\n");
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -EPERM;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num, NSS_UDP_ST_INTERFACE,
				NSS_UDP_ST_UNCFG_RULE_MSG,
				sizeof(struct nss_udp_st_cfg),
				nss_nludp_st_process_resp,
				(void *)resp);

	memcpy(&num.msg.uncfg, uncfg, sizeof(struct nss_udp_st_cfg));

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return ret;
}

/*
 * nss_nludp_st_cfg_udp_st_rule()
 *	Configures the entry from udp_st
 */
static int nss_nludp_st_cfg_udp_st_rule(struct sk_buff *skb, struct nss_udp_st_cfg *cfg)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_udp_st_msg num;
	struct sk_buff *resp;
	nss_tx_status_t status;
	int ret = 0;

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("unable to save response data from NL buffer\n");
		return -ENOMEM;
	}

	nss_ctx = nss_udp_st_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("Couldn't get udp_st ctx\n");
		return -EPERM;
	}

	memset(&num, 0, sizeof(struct nss_udp_st_msg));
	nss_udp_st_msg_init(&num, NSS_UDP_ST_INTERFACE,
				NSS_UDP_ST_CFG_RULE_MSG,
				sizeof(struct nss_udp_st_cfg),
				nss_nludp_st_process_resp,
				(void *)resp);
	memcpy(&num.msg.cfg, cfg, sizeof(struct nss_udp_st_cfg));

	status = nss_udp_st_tx_sync(nss_ctx, &num);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create udp_st message failed %d\n", nss_ctx, status);
		return -EBADMSG;
	}

	return ret;
}

/*
 * nss_nludp_st_ops_uncfg_rule
 *	Handler for unconfiguring rules
 */
static int nss_nludp_st_ops_uncfg_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_udp_st_cfg *uncfg;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_UNCFG_RULE_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract unconfigure rule data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);
	uncfg = &nl_rule->num.msg.uncfg;

	/*
	 * Convert the ipv6 address to NSS foramt.
	 */
	if (uncfg->ip_version == NSS_UDP_ST_FLAG_IPV6) {
		nss_nludp_st_swap_addr_ipv6(uncfg->src_ip.ip.ipv6, uncfg->src_ip.ip.ipv6);
		nss_nludp_st_swap_addr_ipv6(uncfg->dest_ip.ip.ipv6, uncfg->dest_ip.ip.ipv6);
	}

	/*
	 * Unconfigure udp_st node in NSS-FW
	 */
	ret = nss_nludp_st_uncfg_udp_st_rule(skb, &nl_rule->num.msg.uncfg);
	if (ret < 0) {
		nss_nl_info("%px: Unable to remove a rule entry for udp st\n", skb);
		return -EBADMSG;
	}

	/*
	 * Destroy rule based on ip version
	 */
	if (uncfg->ip_version == NSS_UDP_ST_FLAG_IPV4) {
		ret = nss_nludp_st_destroy_ipv4_rule(skb, nl_rule);
	} else if (uncfg->ip_version == NSS_UDP_ST_FLAG_IPV6) {
		ret = nss_nludp_st_destroy_ipv6_rule(skb, nl_rule);
	} else {
		goto fail;
	}

	if (ret < 0) {
		nss_nl_error("%px: Unable to delete a rule entry for ipv%d.\n", skb, nl_rule->num.msg.uncfg.ip_version);
		goto fail;
	}

	return ret;
fail:
	/*
	 * num.msg.cfg and num.msg.uncfg are of same type, struct nss_udp_st_cfg
	 */
	nss_nludp_st_cfg_udp_st_rule(skb, &nl_rule->num.msg.uncfg);
	return -EAGAIN;
}

/*
 * nss_nludp_st_ops_cfg_rule()
 *	Handler for configuring rules
 */
static int nss_nludp_st_ops_cfg_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nludp_st_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_udp_st_cfg *cfg;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nludp_st_family, info, NSS_UDP_ST_CFG_RULE_MSG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract configure rule data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nludp_st_rule, cm);
	cfg = &nl_rule->num.msg.cfg;

	/*
	 * Convert the ipv6 address to NSS foramt.
	 */
	if (cfg->ip_version == NSS_UDP_ST_FLAG_IPV6) {
		nss_nludp_st_swap_addr_ipv6(cfg->src_ip.ip.ipv6, cfg->src_ip.ip.ipv6);
		nss_nludp_st_swap_addr_ipv6(cfg->dest_ip.ip.ipv6, cfg->dest_ip.ip.ipv6);
	}

	/*
	 * Configure udp_st node in NSS-FW.
	 */
	ret = nss_nludp_st_cfg_udp_st_rule(skb, &nl_rule->num.msg.cfg);
	if (ret < 0) {
		nss_nl_info("%px: Unable to add a rule entry for udp st\n", skb);
		return -EBADMSG;
	}

	/*
	 * Create rule based on ip version
	 */
	if (cfg->ip_version == NSS_UDP_ST_FLAG_IPV4) {
		ret = nss_nludp_st_create_ipv4_rule(skb, nl_rule);
	} else if (cfg->ip_version == NSS_UDP_ST_FLAG_IPV6) {
		ret = nss_nludp_st_create_ipv6_rule(skb, nl_rule);
	} else {
		goto fail;
	}

	if (ret < 0) {
		nss_nl_error("%px: Unable to add a rule entry for ipv%d.\n", skb, nl_rule->num.msg.cfg.ip_version);
		goto fail;
	}

	return 0;
fail:
	/*
	 * num.msg.cfg and num.msg.uncfg are of same type, struct nss_udp_st_cfg
	 */
	nss_nludp_st_uncfg_udp_st_rule(skb, &nl_rule->num.msg.cfg);
	return -EAGAIN;
}

/*
 * nss_nludp_private_init
 *	Function to handle initialization of global structure
 */
static void nss_nludp_private_init(void) {
	global.mode = NSS_UDP_ST_MODE_DEFAULT;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	init_timer(&global.time_sync_timer);
	global.time_sync_timer.function = nss_nludp_st_time_sync_timer_cb;
	global.time_sync_timer.data = (unsigned long)&global;

	init_timer(&global.test_duration_timer);
	global.test_duration_timer.function = nss_nludp_st_test_duration_timer_cb;
	global.test_duration_timer.data = (unsigned long)&global;
#else
	timer_setup(&global.time_sync_timer, nss_nludp_st_time_sync_timer_cb, 0);
	timer_setup(&global.test_duration_timer, nss_nludp_st_test_duration_timer_cb, 0);
#endif
	INIT_WORK(&global.work, nss_nludp_st_time_sync);

}

/*
 * nss_nludp_st_exit()
 *	handler exit
 */
bool nss_nludp_st_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink udp_st handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nludp_st_family);
	if (error) {
		nss_nl_info_always("unable to unregister udp_st NETLINK family\n");
		return false;
	}

	/*
	 * flush the work queue.
	 */
	flush_work(&global.work);
	return true;
}

/*
 * nss_nludp_st_init()
 *	handler init
 */
bool nss_nludp_st_init(void)
{
	int error;

	nss_nl_info_always("Init NSS netlink udp_st handler\n");

	/*
	 * register Netlink ops with the family
	 */
	error = genl_register_family(&nss_nludp_st_family);
	if (error) {
		nss_nl_info_always("Error: unable to register udp_st family\n");
		return false;
	}

	nss_nludp_private_init();

	return true;
}
