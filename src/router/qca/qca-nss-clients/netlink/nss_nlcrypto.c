/*
 **************************************************************************
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
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
 * @file nss_nlcrypto.c
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
#include "nss_crypto_defines.h"
#include "nss_nlcrypto_if.h"
#include "nss_nl.h"

/*
 * @brief prototypes
 */
static int nss_nlcrypto_op_session_create(struct sk_buff *skb_msg, struct genl_info *info);
static int nss_nlcrypto_op_session_create_nokey(struct sk_buff *skb_msg, struct genl_info *info);
static int nss_nlcrypto_op_session_update(struct sk_buff *skb_msg, struct genl_info *info);
static int nss_nlcrypto_op_session_destroy(struct sk_buff *skb_msg, struct genl_info *info);
static int nss_nlcrypto_op_session_info(struct sk_buff *skb_msg, struct genl_info *info);

/*
 * NSS NETLINK crypto context
 */
struct nss_nlcrypto_ctx {
	nss_crypto_handle_t crypto_hdl;
};

/*
 * @brief  Crypto family definition
 */
static struct genl_family nss_nlcrypto_family = {
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
	.name = NSS_NLCRYPTO_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlcrypto_rule),	/* NSS NETLINK Crypto rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLIPv4 version */
	.maxattr = NSS_NLCRYPTO_CMD_MAX,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
};

/*
 *  @brief operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlcrypto_ops[] = {
	{.cmd = NSS_NLCRYPTO_CMD_CREATE_SESSION, .doit = nss_nlcrypto_op_session_create,},	/* session create */
	{.cmd = NSS_NLCRYPTO_CMD_CREATE_SESSION_NOKEY, .doit = nss_nlcrypto_op_session_create_nokey,},	/* session create */
	{.cmd = NSS_NLCRYPTO_CMD_UPDATE_SESSION, .doit = nss_nlcrypto_op_session_update,},	/* session update */
	{.cmd = NSS_NLCRYPTO_CMD_DESTROY_SESSION, .doit = nss_nlcrypto_op_session_destroy,},	/* session delete */
	{.cmd = NSS_NLCRYPTO_CMD_INFO_SESSION, .doit = nss_nlcrypto_op_session_info,},	/* sessoin info */
};


#define NSS_NLCRYPTO_OPS_SZ ARRAY_SIZE(nss_nlcrypto_ops)
#define NSS_NLCRYPTO_MAX_IDX_LOCATION 0x2000 	/* 8K memory size */

/*
 * global context
 */
static struct nss_nlcrypto_ctx gbl_ctx;

/*
 * @brief nss_nlcrypto_validate_cipher()
 * 	validate the cipher parameters
 */
static inline bool nss_nlcrypto_validate_cipher(struct nss_crypto_key *cipher)
{
	enum nss_crypto_cipher algo = cipher->algo;
	uint16_t key_len = cipher->key_len;
	uint16_t max_key_len = 0;

	switch (algo) {
	case NSS_CRYPTO_CIPHER_AES_CBC:
	case NSS_CRYPTO_CIPHER_AES_CTR:
		max_key_len = NSS_CRYPTO_MAX_KEYLEN_AES;
		break;

	case NSS_CRYPTO_CIPHER_DES:
		max_key_len =  NSS_CRYPTO_MAX_KEYLEN_DES;
		break;

	case NSS_CRYPTO_CIPHER_NULL:
		max_key_len = 0;
		break;

	default:
		return false;
	}

	/*
	 * Key length shouldn't be more than max key length
	 */
	return !(key_len > max_key_len);
}

/*
 * @brief nss_nlcrypto_validate_auth()
 * 	validate the auth parameters
 */
static inline bool nss_nlcrypto_validate_auth(struct nss_crypto_key *auth)
{
	enum nss_crypto_auth algo = auth->algo;
	uint16_t key_len = auth->key_len;
	uint16_t max_key_len = 0;

	switch (algo) {
		case NSS_CRYPTO_AUTH_SHA1_HMAC:
			max_key_len = NSS_CRYPTO_MAX_KEYLEN_SHA1;
			break;

		case NSS_CRYPTO_AUTH_SHA256_HMAC:
			max_key_len =  NSS_CRYPTO_MAX_KEYLEN_SHA256;
			break;

		case NSS_CRYPTO_AUTH_NULL:
			max_key_len = 0;
			break;

		default:
			return false;

	}

	/*
	 * Key length shouldn't be more than max key length
	 */
	return !(key_len > max_key_len);

}

/*
 * @brief nss_nlcrypto_validate_key_index()
 * 	validate the auth parameters
 */
static inline bool nss_nlcrypto_validate_key_index(struct nss_crypto_key *crypto)
{
	uint32_t max_index = NSS_NLCRYPTO_MAX_IDX_LOCATION - crypto->key_len;
	if (crypto->index > max_index) {
		return false;
	}

	return true;
}

/*
 * @brief nss_nlcrypto_validate_param()
 * 	validate the  parameters
 */
static inline bool nss_nlcrypto_validate_param(struct nss_crypto_params *param)
{
	/* check the skip lengths */
	if (param->auth_skip > param->cipher_skip) {
		return false;
	}

	switch (param->req_type) {
	/* pure encryption */
	case NSS_CRYPTO_REQ_TYPE_ENCRYPT:
		return true;
	/* pure decryption */
	case NSS_CRYPTO_REQ_TYPE_DECRYPT:
		return true;
	/* encryption and authentication */
	case (NSS_CRYPTO_REQ_TYPE_ENCRYPT | NSS_CRYPTO_REQ_TYPE_AUTH):
		return true;
	/* decryption and authentication */
	case (NSS_CRYPTO_REQ_TYPE_DECRYPT | NSS_CRYPTO_REQ_TYPE_AUTH):
		return true;
	/* otherwise */
	default:
		return false;

	}

	return false;
}

/*
 * @brief nss_nlcrypto_compose_info()
 * 	Compose the info message
 */
static inline void nss_crypto_copy_info(struct nss_nlcrypto_info_session *reply, uint32_t session_idx)
{
	reply->session_idx = session_idx;
	reply->cipher.algo = nss_crypto_get_cipher(session_idx);
	reply->auth.algo = nss_crypto_get_auth(session_idx);
}

/*
 * @brief nss_nlcrypto_op_session_create()
 * 	session add handler
 */
static int nss_nlcrypto_op_session_create(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcrypto_create_session *create_msg;
	struct nss_nlcrypto_info_session *reply;
	struct nss_crypto_key *cipher = NULL;
	struct nss_crypto_key *auth = NULL;
	struct nss_nlcrypto_rule *nl_rule;
	nss_crypto_status_t status;
	struct nss_nlcmn *nl_cm;
	int32_t session_idx = -1;
	struct sk_buff *resp;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcrypto_family, info, NSS_NLCRYPTO_CMD_CREATE_SESSION);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule create data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcrypto_rule, cm);

	pid = nl_cm->pid;
	create_msg = &nl_rule->msg.create;

	cipher = &create_msg->cipher;
	auth   = &create_msg->auth;

	/* validate if cipher specific parameters are correct */
	if (nss_nlcrypto_validate_cipher(cipher) == false) {
		nss_nl_error("cipher validation failed: algo(%d), key_len(%d)\n",
				 cipher->algo, cipher->key_len);
		return -EINVAL;
	}

	/* validate if auth specific parameters are correct */
	if (nss_nlcrypto_validate_auth(auth) == false) {
		nss_nl_error("auth validation failed:  algo(%d), key_len(%d)\n",
				auth->algo,
				auth->key_len);
		return -EINVAL;
	}

	/*
	 * Link the key pointer in crypto driver structure to the array
	 */
	cipher->key = create_msg->cipher_key;
	auth->key   = create_msg->auth_key;

	status = nss_crypto_session_alloc(gbl_ctx.crypto_hdl, cipher, auth, &session_idx);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_nl_error("%d:session alloc failed\n", pid);
		return -EINVAL;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d:unable to save response data from NL buffer\n", pid);
		return -ENOMEM;
	}

	/*
	 * overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);
	nss_nlcrypto_rule_init(nl_rule, NSS_NLCRYPTO_CMD_INFO_SESSION);

	/*
	 * Fill up the info message
	 */
	reply = &nl_rule->msg.info;
	memset(reply, 0, sizeof(struct nss_nlcrypto_info_session));

	/*
	 * copy the info message
	 */
	nss_crypto_copy_info(reply, session_idx);

	/*
	 * unicast the response to user
	 */
	nss_nl_ucast_resp(resp);

	return 0;
}
/*
 * @brief nss_nlcrypto_op_session_create_nokey()
 * 	handler for session add without key
 */
static int nss_nlcrypto_op_session_create_nokey(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcrypto_create_session *create_msg;
	struct nss_nlcrypto_info_session *reply;
	struct nss_crypto_key *cipher;
	struct nss_crypto_key *auth;
	struct nss_nlcrypto_rule *nl_rule;
	nss_crypto_status_t status;
	struct nss_nlcmn *nl_cm;
	int32_t session_idx = -1;
	struct sk_buff *resp;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcrypto_family, info, NSS_NLCRYPTO_CMD_CREATE_SESSION_NOKEY);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule create data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcrypto_rule, cm);

	pid = nl_cm->pid;
	create_msg = &nl_rule->msg.create;

	cipher = &create_msg->cipher;
	auth   = &create_msg->auth;

	/* validate if cipher specific parameters are correct */
	if (nss_nlcrypto_validate_cipher(cipher) == false) {
		nss_nl_error("cipher validation failed: algo(%d), key_len(%d)\n",
				 cipher->algo, cipher->key_len);
		return -EINVAL;
	}

	/* validate if auth specific parameters are correct */
	if (nss_nlcrypto_validate_auth(auth) == false) {
		nss_nl_error("auth validation failed:  algo(%d), key_len(%d)\n",
				auth->algo,
				auth->key_len);
		return -EINVAL;
	}

	if (nss_nlcrypto_validate_key_index(cipher) == false) {
		nss_nl_error("cipher validation failed: key_idx(%d)\n", cipher->index);
		return -EINVAL;
	}

	if (nss_nlcrypto_validate_key_index(auth) == false) {
		nss_nl_error("auth validation failed: key_idx(%d)\n", auth->index);
		return -EINVAL;
	}

	status = nss_crypto_session_alloc_nokey(gbl_ctx.crypto_hdl, cipher, auth, &session_idx);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_nl_error("%d:session alloc failed\n", pid);
		return -EINVAL;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d:unable to save response data from NL buffer\n", pid);
		return -ENOMEM;
	}

	/*
	 * overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);
	nss_nlcrypto_rule_init(nl_rule, NSS_NLCRYPTO_CMD_INFO_SESSION);

	/*
	 * Fill up the info message
	 */
	reply = &nl_rule->msg.info;
	memset(reply, 0, sizeof(struct nss_nlcrypto_info_session));

	/*
	 * copy the info message
	 */
	nss_crypto_copy_info(reply, session_idx);

	/*
	 * unicast the response to user
	 */
	nss_nl_ucast_resp(resp);

	return 0;
}

/*
 * @brief nss_nlcrypto_op_session_update()
 * 	session update handler
 */
static int nss_nlcrypto_op_session_update(struct sk_buff *skb_msg, struct genl_info *info)
{
	struct nss_nlcrypto_rule *nl_rule;
	struct nss_crypto_params *param;
	struct nss_nlcmn *nl_cm;
	uint32_t session_idx;
	uint32_t pid;


	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcrypto_family, info, NSS_NLCRYPTO_CMD_UPDATE_SESSION);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule create data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcrypto_rule, cm);
	pid = nl_cm->pid;

	session_idx = nl_rule->msg.update.session_idx;

	param = &nl_rule->msg.update.param;
	if (nss_nlcrypto_validate_param(param) == false) {
		nss_nl_error("param validation failed:  req_type(%d), auth_skip(%d), cipher_skip(%d)\n",
				param->req_type,param->auth_skip, param->cipher_skip);
		return -EINVAL;
	}

	return nss_crypto_session_update(gbl_ctx.crypto_hdl, session_idx, param);
}

/*
 * @brief nss_nlcrypto_op_session_destroy()
 * 	delete an existing session
 *
 */
static int nss_nlcrypto_op_session_destroy(struct sk_buff *skb_msg, struct genl_info *info)
{
	struct nss_nlcrypto_destroy_session *destroy;
	struct nss_nlcrypto_rule *nl_rule;
	nss_crypto_status_t status;
	struct nss_nlcmn *nl_cm;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcrypto_family, info, NSS_NLCRYPTO_CMD_DESTROY_SESSION);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule create data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcrypto_rule, cm);
	pid = nl_cm->pid;

	destroy = &nl_rule->msg.destroy;

	if (nss_crypto_get_cipher(destroy->session_idx) == NSS_CRYPTO_CIPHER_NONE) {
		nss_nl_error("invalid session index, cipher unknown:%d\n", destroy->session_idx);
		return -EINVAL;
	}

	if (nss_crypto_get_auth(destroy->session_idx) == NSS_CRYPTO_AUTH_NONE) {
		nss_nl_error("invalid session index, auth unknown:%d\n", destroy->session_idx);
		return -EINVAL;
	}

	status = nss_crypto_session_free(gbl_ctx.crypto_hdl, destroy->session_idx);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_nl_error("unable to delete the session:%d\n", destroy->session_idx);
		return -EINVAL;
	}

	return 0;
}

/*
 * @brief nss_nlcrypto_op_session_info()
 * 	get session information
 */
static int nss_nlcrypto_op_session_info(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcrypto_info_session *reply;
	struct nss_nlcrypto_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	uint32_t session_idx;
	struct sk_buff *resp;
	uint32_t pid;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcrypto_family, info, NSS_NLCRYPTO_CMD_INFO_SESSION);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule info data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcrypto_rule, cm);
	pid = nl_cm->pid;

	/*
	 * since we need to populate the same information in the session_info
	 * we will reuse the op for response
	 */
	session_idx  = nl_rule->msg.info.session_idx;

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d:unable to save response data from NL buffer\n", pid);
		return -ENOMEM;
	}

	/*
	 * overload the nl_rule with the new response address
	 */
	nl_rule = nss_nl_get_data(resp);

	/*
	 * Fill up the info message
	 */
	reply = &nl_rule->msg.info;
	memset(reply, 0, sizeof(struct nss_nlcrypto_info_session));

	/*
	 * copy the info message
	 */
	nss_crypto_copy_info(reply, session_idx);

	/*
	 * unicast the response to user
	 */
	nss_nl_ucast_resp(resp);
	return 0;
}
/*
 * @brief nss_nlcrypto_attach()
 * 	crypto user attach
 */
nss_crypto_user_ctx_t nss_nlcrypto_attach(nss_crypto_handle_t crypto)
{
	gbl_ctx.crypto_hdl = crypto;

	return (nss_crypto_user_ctx_t)&nss_nlcrypto_family;
}

/*
 * @brief nss_nlcrypto_detach()
 * 	crypto user detach; triggered by unregister
 */
void nss_nlcrypto_detach(nss_crypto_user_ctx_t ctx)
{
	gbl_ctx.crypto_hdl = NULL;
}

/*
 * @brief nss_nlcrypto_init()
 * 	handler init
 */
bool nss_nlcrypto_init(void)
{
	int status;

	nss_nl_info_always("initiallizing the NSS netlink crypto handler\n");

	/*
	 * register with the family
	 */
	status = genl_register_family_with_ops(&nss_nlcrypto_family, nss_nlcrypto_ops, NSS_NLCRYPTO_OPS_SZ);
	if (!status) {
		return false;
	}

	nss_crypto_register_user(nss_nlcrypto_attach, nss_nlcrypto_detach, NSS_NLCRYPTO_FAMILY);

	return true;
}

/*
 * @brief nss_nlcrypto_exit()
 * 	handler exit
 */
bool nss_nlcrypto_exit(void)
{
	int status;

	nss_nl_info_always("Uninitializing the NSS netlink crypto handler\n");

	/*
	 * unregister with the family
	 */
	status = genl_unregister_family(&nss_nlcrypto_family);
	if (status != 0) {
		return false;
	}

	nss_crypto_unregister_user(gbl_ctx.crypto_hdl);

	return true;
}
