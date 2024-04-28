/*
 **************************************************************************
 * Copyright (c) 2016,2018, The Linux Foundation. All rights reserved.
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
 * nss_nloam.c
 *	NSS Netlink OAM Handler
 *
 * This nss netlink oam module receives request message from
 * oam adapter and forward it to nss. It also receive responses
 * from nss and forward it to oam adapter.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/version.h>

#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include <nss_oam.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nloam_if.h"

/*
 * prototype declaration for functions
 */
static int nss_nloam_op_none(struct sk_buff *skb, struct genl_info *info);
static int nss_nloam_op_get_req(struct sk_buff *skb, struct genl_info *info);
static void nss_nloam_process_resp(void *app_data, struct nss_oam_msg *nom);
static int nss_nloam_get_fw_pre_req(void *msg, struct nss_oam_msg *nom);
static int nss_nloam_get_fw_post_resp(void *msg, struct nss_oam_msg *nom);

/*
 * global context for nss_nloam
 */
struct nss_nloam_ctx {
	void *nss_ctx;
};

static struct nss_nloam_ctx global_nloam_ctx;

typedef int (*nss_nloam_msg_cb_t)(void *msg, struct nss_oam_msg *nom);

/*
 * message map for message type and rec_id
 */
struct nss_nloam_msg_map {
	uint32_t msg_type;
	int32_t rec_id;

	int8_t cmd_type;
	int8_t res[3];

	nss_nloam_msg_cb_t pre_req;
	nss_nloam_msg_cb_t post_resp;
};

struct nss_nloam_msg_map global_nloam_msg_map[] = {
	{
		.msg_type = NSS_OAM_MSG_TYPE_GET_FW_VER,
		.rec_id = NSS_NLOAM_GET_TYPE_FW_VERSION,
		.cmd_type = NSS_NLOAM_CMD_GET_REQ,
		.pre_req = nss_nloam_get_fw_pre_req,
		.post_resp = nss_nloam_get_fw_post_resp,
	},
};

/*
 * OAM family definition
 */
static struct genl_family nss_nloam_family = {
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
	.name = NSS_NLOAM_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nloam_rule),	/* NLOAM msg size */
	.version = NSS_NL_VER,				/* nss netlink version */
	.maxattr = NSS_NLOAM_CMD_MAX,			/* maximum commands supported */
	.netnsok = false,
	.pre_doit = NULL,
	.post_doit = NULL,
};

static struct genl_multicast_group nss_nloam_mcgrp[] = {
	{.name = NSS_NLOAM_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nloam_ops[] = {
	{.cmd = NSS_NLOAM_CMD_NONE, .doit = nss_nloam_op_none,},
	{.cmd = NSS_NLOAM_CMD_GET_REQ, .doit = nss_nloam_op_get_req,},
};

#define NSS_NLOAM_OPS_SZ ARRAY_SIZE(nss_nloam_ops)
#define NSS_NLOAM_PAYLOAD_SZ sizeof(struct nss_oam_msg) - sizeof(struct nss_cmn_msg)

/*
 * nss_nloam_get_fw_pre_req()
 *	call back for get firmware version request message.
 */
static int nss_nloam_get_fw_pre_req(void *msg, struct nss_oam_msg *nom)
{
	struct nss_nloam_get_req *get_req = (struct nss_nloam_get_req *)msg;

	if (get_req->clear_on_read) {
		nss_nl_error("clear_on_read is invalid for get_fw_version\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_nloam_get_fw_post_resp()
 *	call back for get firmware version response message.
 */
static int nss_nloam_get_fw_post_resp(void *msg, struct nss_oam_msg *nom)
{
	struct nss_nloam_get_resp *get_resp = (struct nss_nloam_get_resp *)msg;
	uint32_t error;
	char *fw_ver;

	fw_ver = nom->msg.fw_ver.string;
	get_resp->length = strlen(fw_ver);

	/*
	 * Check the response status from fw.
	 */
	if (nom->cm.response == NSS_CMN_RESPONSE_EMSG) {
		nss_nl_error("response message with error status\n");
		error = nom->cm.error;
		goto fail;
	}

	/*
	 * Check for max length
	 */
	if (get_resp->length >= NSS_OAM_FW_VERSION_LEN) {
		nss_nl_error("nss fw version exceeds max length\n");
		error = ENOBUFS;
		goto fail;
	}

	strlcpy((char *)get_resp->bin, nom->msg.fw_ver.string, NSS_OAM_FW_VERSION_LEN);
	get_resp->bin[NSS_OAM_FW_VERSION_LEN - 1] = '\0';
	get_resp->error_no = 0;

	return 0;
fail:
	/*
	 * As user app expects bin is a null terminated string
	 * On failure terminating with only '\0' - empty string
	 */
	get_resp->bin[0] = '\0';
	get_resp->error_no = error;
	return -error;
}

/*
 * nss_nloam_find_msg_idx()
 *	Helper function to find the message of a command from msg_map
 */
static struct nss_nloam_msg_map *nss_nloam_find_msg(uint8_t cmd_type, uint32_t rec_id)
{
	struct nss_nloam_msg_map *msg;
	const size_t array_sz = ARRAY_SIZE(global_nloam_msg_map);
	int i = 0;

	for (i = 0, msg = &global_nloam_msg_map[0]; i < array_sz; i++, msg++) {
		if ((msg->cmd_type == cmd_type) && (msg->rec_id == rec_id)) {
			return msg;
		}
	}
	return NULL;
}

/*
 * nss_nloam_op_none()
 *	invalid command handler; ideally nobody should call this
 */
static int nss_nloam_op_none(struct sk_buff *skb, struct genl_info *info)
{
	nss_nl_error("unable to reply generic message\n");
	return -ENOENT;
}

/*
 * nss_nloam_op_get_req()
 *	sends the user data to NSS HLOS driver to forward the request to NSS FW
 */
static int nss_nloam_op_get_req(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nloam_get_req *get_req;
	struct nss_nloam_rule *nl_oam;
	struct nss_nloam_msg_map *msg;
	nss_tx_status_t tx_status;
	struct nss_nlcmn *nl_cm;
	struct nss_oam_msg nom;
	struct sk_buff *resp;
	uint32_t error = 0;
	uint32_t msg_type;
	uint8_t cmd_type;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nloam_family, info, NSS_NLOAM_CMD_SET_REQ);
	if (!nl_cm) {
		nss_nl_error("unable to extract oam req cmd\n");
		error = -EINVAL;
		goto error;
	}

	nl_oam = container_of(nl_cm, struct nss_nloam_rule, cm);
	pid = nl_cm->pid;

	/*
	 * Frame message to send nss according to the recieved request
	 */
	cmd_type = nss_nlcmn_get_cmd(nl_cm);
	get_req = &nl_oam->msg.get_req;

	msg = nss_nloam_find_msg(cmd_type, get_req->rec_id);
	if (!msg) {
		nss_nl_error("%d: invalid OAM CMD (type - %d, rec - %d)\n", pid, cmd_type, get_req->rec_id);
		error = -EINVAL;
		goto error;
	}

	BUG_ON(!msg->pre_req);
	error = msg->pre_req(get_req, &nom);
	if (error) {
		goto error;
	}

	msg_type = msg->msg_type;

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d: unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		goto error;
	}

	/*
	 * Initialize the common message before sending to NSS
	 */
	nss_cmn_msg_init(&nom.cm,				/* common message */
			NSS_OAM_INTERFACE,			/* oam if number */
			msg_type,				/* type */
			NSS_NLOAM_PAYLOAD_SZ,			/* message len */
			nss_nloam_process_resp, 		/* callback */
			(void *)resp);				/* app context */

	/*
	 * Push the Rule to NSS
	 */
	tx_status = nss_oam_tx_msg(global_nloam_ctx.nss_ctx, &nom);
	if (NSS_TX_SUCCESS != tx_status) {
		nss_nl_error("Error in Pushing the oam message\n");
		error = -EBUSY;
		goto free;
	}

	return 0;

free:
	dev_kfree_skb_any(resp);
error:
	return error;
}

/*
 * nss_nloam_process_resp()
 *	Process the response message for a command sent
 */
static void nss_nloam_process_resp(void *app_data, struct nss_oam_msg *nom)
{
	struct sk_buff *resp = (struct sk_buff *)app_data;
	struct nss_nloam_rule *nl_oam_reply;
	struct nss_nloam_msg_map *msg;
	uint8_t cmd_type;

	nl_oam_reply = (struct nss_nloam_rule *) nss_nl_get_data(resp);

	cmd_type = nss_nlcmn_get_cmd(&nl_oam_reply->cm);
	msg = nss_nloam_find_msg(cmd_type, nl_oam_reply->msg.get_req.rec_id);

	if (!msg) {
		nss_nl_error("invalid message reply for get request: %d\n", cmd_type);
		return;
	}

	BUG_ON(!msg->post_resp);
	msg->post_resp(&nl_oam_reply->msg, nom);
	nss_nl_ucast_resp(resp);
}

/*
 * nss_nloam_process_notify()
 *	Process the notify message from nss
 */
static void nss_nloam_process_notify(void *app_data, struct nss_oam_msg *nom)
{
	/*
	 * As of now there is no notify message expected from nss
	 */
	nss_nl_info("handler for nss nloam notify message\n");
}

/*
 * nss_nloam_init()
 *	handler init
 */
bool nss_nloam_init(void)
{
	int error;

	/*
	 * register with the family
	 */
	error = genl_register_family_with_ops_groups(&nss_nloam_family, nss_nloam_ops, nss_nloam_mcgrp);
	if (error != 0) {
		nss_nl_info_always("unable to register OAM family\n");
		return false;
	}

	/*
	 * register device call back handler for OAM from NSS
	 */
	global_nloam_ctx.nss_ctx = nss_oam_notify_register(nss_nloam_process_notify, NULL);
	if (!global_nloam_ctx.nss_ctx) {
		nss_nl_error("Error in retreiving the NSS Context\n");
		goto unreg_ops;
	}

	return true;

unreg_ops:
	genl_unregister_family(&nss_nloam_family);

	return false;
}

/*
 * nss_nloam_exit()
 *	handler exit
 */
bool nss_nloam_exit(void)
{
	nss_nl_info_always("Uninitiallizing the NSS netlink oam handler\n");

	/*
	 * Unregister the device callback handler for oam
	 */
	nss_oam_notify_unregister();

	/*
	 * unregister with the family
	 */
	genl_unregister_family(&nss_nloam_family);

	global_nloam_ctx.nss_ctx = NULL;

	return true;
}
