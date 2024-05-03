/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include <nss_nlbase.h>
#include <nss_nlsock_api.h>
#include <nss_nlqrfs_api.h>

/*
 * nss_nlqrfs_sock_cb()
 *	NSS NL qrfs callback
 */
int nss_nlqrfs_sock_cb(struct nl_msg *msg, void *arg)
{
	pid_t pid = getpid();

	struct nss_nlqrfs_rule *rule = nss_nlsock_get_data(msg);
	if (!rule) {
		nss_nlsock_log_error("%d:failed to get NSS NL qrfs header\n", pid);
		return NL_SKIP;
	}

	uint8_t cmd = nss_nlcmn_get_cmd(&rule->cm);

	switch (cmd) {
	case NSS_QRFS_MSG_FLOW_ADD:
		nss_nlsock_log_info("add rule success\n");
		return NL_OK;
	case NSS_QRFS_MSG_FLOW_DELETE:
		nss_nlsock_log_info("delete rule success\n");
		return NL_OK;
	default:
		nss_nlsock_log_error("%d:unsupported message cmd type(%d)\n", pid, cmd);
		return NL_SKIP;
	}
}

/*
 * nss_nlqrfs_sock_open()
 *	 Opens the NSS qrfs NL socket for usage
 */
int nss_nlqrfs_sock_open(struct nss_nlqrfs_ctx *ctx, void *user_ctx, nss_nlqrfs_event_t event_cb)
{
	int error;
	pid_t pid = getpid();

	if (!ctx) {
		nss_nlsock_log_error("%d: invalid parameters passed\n", pid);
		return -EINVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	nss_nlsock_set_family(&ctx->sock, NSS_NLQRFS_FAMILY);
	nss_nlsock_set_user_ctx(&ctx->sock, user_ctx);

	/*
	 * try opening the socket with Linux
	 */
	error = nss_nlsock_open(&ctx->sock, nss_nlqrfs_sock_cb);
	if (error) {
		nss_nlsock_log_error("%d:unable to open NSS qrfs socket, error(%d)\n", pid, error);
	}

	return error;
}

/*
 * nss_nlqrfs_sock_close()
 *	close the NSS QRFS NL socket
 */
void nss_nlqrfs_sock_close(struct nss_nlqrfs_ctx *ctx)
{
	if (!ctx) {
		nss_nlsock_log_error("ctx is NULL\n");
		return;
	}

	nss_nlsock_close(&ctx->sock);
	memset(ctx, 0, sizeof(struct nss_nlqrfs_ctx));
}

/*
 * nss_nlqrfs_sock_send()
 *	register callback and send the QRFS message synchronously through the socket
 */
int nss_nlqrfs_sock_send(struct nss_nlqrfs_ctx *ctx, struct nss_nlqrfs_rule *rule, nss_nlqrfs_resp_t cb, void *data)
{
	int32_t family_id;
	struct nss_nlqrfs_resp *resp;
	pid_t pid = getpid();
	bool has_resp = false;
	int error;

	if(!ctx) {
		nss_nlsock_log_error("%d: Empty socket context\n", pid);
		return -EINVAL;
	}

	if (!rule) {
		nss_nlsock_log_error("%d: Invalid NSS QRFS rule\n", pid);
		return -EINVAL;
	}

	family_id = ctx->sock.family_id;

	if (cb) {
		nss_nlcmn_set_cb_owner(&rule->cm, family_id);

		resp = nss_nlcmn_get_cb_data(&rule->cm, family_id);
		assert(resp);

		resp->data = data;
		resp->cb = cb;
		has_resp = true;
	}
	error = nss_nlsock_send(&ctx->sock, &rule->cm, rule, has_resp);
	if (error) {
		nss_nlsock_log_error("%d: Failed to send NSS QRFS rule, error(%d)\n", pid, error);
	}

	return error;
}

/*
 * nss_nlqrfs_init_cmd()
 *	init the rule message
 */
void nss_nlqrfs_init_rule(struct nss_nlqrfs_rule *rule, enum nss_qrfs_msg_types type)
{
	nss_nlqrfs_rule_init(rule, type);
}
