/*
 **************************************************************************
 * Copyright 2018, The Linux Foundation. All rights reserved.
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
 * @file nss_nlcryptov2.c
 *	NSS Netlink Crypto Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>

#include <linux/netlink.h>
#include <linux/version.h>
#include <net/genetlink.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include "nss_nlcmn_if.h"
#include "nss_crypto_api.h"
#include "nss_nlcryptov2_if.h"
#include "nss_nlcryptov2.h"
#include "nss_crypto_cmn.h"
#include "nss_nl.h"

/*
 * @brief prototypes
 */
void nss_nlcryptov2_attach(void * app_data, struct nss_crypto_user *user);
void nss_nlcryptov2_detach(void * app_data, struct nss_crypto_user *user);

/*
 * NSS NETLINK crypto context
 */
struct nss_nlcryptov2 {
	struct nss_crypto_user_ctx ctx;
	struct nss_crypto_user *user;
};

/*
 * global context
 */
static struct nss_nlcryptov2 g_nlcryptov2 = {
	.ctx = {
		.name = "nss_nlcryptov2",
		.attach = nss_nlcryptov2_attach,
		.detach = nss_nlcryptov2_detach,
		.hdr_pool_sz = NSS_NLCRYPTOV2_HDR_POOL_SZ,
		.default_hdr_sz = NSS_NLCRYPTOV2_DEFAULT_HDR_SZ,
		.timeout_ticks = NSS_NLCRYPTOV2_TIMEOUT
	},
};

/*
 * @brief  Crypto family definition
 */
static struct genl_family nss_nlcryptov2_family = {
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
	.name = NSS_NLCRYPTOV2_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlcryptov2_rule),	/* NSS NETLINK Crypto rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLIPv4 version */
	.maxattr = NSS_NLCRYPTOV2_CMD_MAX,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
};

#define NSS_NLCRYPTO_OPS_SZ ARRAY_SIZE(nss_nlcryptov2_ops)

/*
 * @brief nss_nlcrypto_op_session_info()
 * 	get session information
 */
static int nss_nlcryptov2_op_session_info(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

/*
 * @brief nss_nlcrypto_op_session_create()
 * 	session add handler
 */
static int nss_nlcryptov2_op_session_create(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcryptov2_create_session *create_msg;
	struct nss_nlcryptov2_resp_session *reply;
	struct nss_crypto_session_data data;
	struct nss_nlcryptov2_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	int32_t session_idx = -1;
	int32_t status;
	struct sk_buff *resp;
	uint32_t pid;

	memset(&data, 0, sizeof(struct nss_crypto_session_data));
	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcryptov2_family, info, NSS_NLCRYPTOV2_CMD_CREATE_SESSION);
	if (!nl_cm) {
		nss_nl_error("%p: unable to extract rule create data\n", info);
		return -ENOENT;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcryptov2_rule, cm);

	pid = nl_cm->pid;
	create_msg = &nl_rule->msg.create;

	data.algo = create_msg->algo;
	data.cipher_key = create_msg->cipher_key;
	data.auth_key   = create_msg->auth_key;
	data.sec_key_offset = create_msg->sec_key_offset;
	data.sec_key = create_msg->sec_key;

	status = nss_crypto_session_alloc(g_nlcryptov2.user, &data, &session_idx);
	if (status < 0) {
		nss_nl_error("pid[%d]: unable to allocate crypto session (%d)\n", pid, status);
		return status;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("pid[%d]: unable to save response data from NL buffer\n", pid);
		return -ENOMEM;
	}

	/*
	 * overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);
	nss_nlcryptov2_rule_init(nl_rule, NSS_NLCRYPTOV2_CMD_RESP_SESSION);

	/*
	 * Fill up the info message
	 */
	reply = &nl_rule->msg.resp;
	memset(reply, 0, sizeof(struct nss_nlcryptov2_resp_session));

	/*
	 * copy the info message
	 */
	reply->session_idx = session_idx;

	/*
	 * unicast the response to user
	 */
	nss_nl_ucast_resp(resp);

	return 0;
}

/*
 * @brief nss_nlcrypto_op_session_destroy()
 * 	delete an existing session
 *
 */
static int nss_nlcryptov2_op_session_destroy(struct sk_buff *skb_msg, struct genl_info *info)
{
	struct nss_nlcryptov2_destroy_session *destroy;
	struct nss_nlcryptov2_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcryptov2_family, info, NSS_NLCRYPTOV2_CMD_DESTROY_SESSION);
	if (!nl_cm) {
		nss_nl_error("%p: unable to extract rule create data\n", info);
		return -ENOENT;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcryptov2_rule, cm);
	pid = nl_cm->pid;

	destroy = &nl_rule->msg.destroy;

	nss_crypto_session_free(g_nlcryptov2.user, destroy->session_idx);
	return 0;
}

/*
 *  *  @brief operation table called by the generic netlink layer based on the command
 *   */
static struct genl_ops nss_nlcryptov2_ops[] = {
	{.cmd = NSS_NLCRYPTOV2_CMD_CREATE_SESSION, .doit = nss_nlcryptov2_op_session_create,},
	{.cmd = NSS_NLCRYPTOV2_CMD_DESTROY_SESSION, .doit = nss_nlcryptov2_op_session_destroy,},
	{.cmd = NSS_NLCRYPTOV2_CMD_RESP_SESSION, .doit = nss_nlcryptov2_op_session_info,}
};

/*
 * @brief nss_nlcrypto_attach()
 * 	crypto user attach
 */
void nss_nlcryptov2_attach(void *app_data, struct nss_crypto_user *user)
{
	g_nlcryptov2.user = user;
}

/*
 * @brief nss_nlcrypto_detach()
 * 	crypto user detach; triggered by unregister
 */
void nss_nlcryptov2_detach(void *app_data, struct nss_crypto_user *user)
{
	g_nlcryptov2.user = NULL;
}

/*
 * @brief nss_nlcryptov2_init()
 * 	handler init
 */
bool nss_nlcryptov2_init(void)
{
	int status;

	nss_nl_info_always("initiallizing the NSS netlink crypto handler\n");

	/*
	 * register with the family
	 */
	status = genl_register_family_with_ops(&nss_nlcryptov2_family, nss_nlcryptov2_ops);
	if (status) {
		return false;
	}

	nss_crypto_register_user(&g_nlcryptov2.ctx, &g_nlcryptov2);

	return true;
}

/*
 * @brief nss_nlcryptov2_exit()
 * 	handler exit
 */
bool nss_nlcryptov2_exit(void)
{
	int status;

	nss_nl_info_always("Uninitializing the NSS netlink crypto handler\n");

	/*
	 * unregister with the family
	 */
	status = genl_unregister_family(&nss_nlcryptov2_family);
	if (status != 0) {
		return false;
	}

	nss_crypto_unregister_user(g_nlcryptov2.user);

	return true;
}
