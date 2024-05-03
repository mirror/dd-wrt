/*
 **************************************************************************
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
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

#include <nss_nlbase.h>
#include <nss_nlsock_api.h>
#include <nss_nlgre_redir_api.h>

/*
 * nss_nlgre_redir_sock_cb()
 *	Callback func for netlink socket
 */
int nss_nlgre_redir_sock_cb(struct nl_msg *msg, void *arg)
{
	pid_t pid = getpid();

	struct nss_nlgre_redir_rule *rule = nss_nlsock_get_data(msg);

	if (!rule) {
		nss_nlsock_log_error("%d:failed to get NSS NL gre_redir header\n", pid);
		return NL_SKIP;
	}

	uint8_t cmd = nss_nlcmn_get_cmd(&rule->cm);

	switch (cmd) {
	case NSS_NLGRE_REDIR_CMD_TYPE_CREATE_TUN:
	case NSS_NLGRE_REDIR_CMD_TYPE_DESTROY_TUN:
	case NSS_NLGRE_REDIR_CMD_TYPE_MAP:
	case NSS_NLGRE_REDIR_CMD_TYPE_UNMAP:
	case NSS_NLGRE_REDIR_CMD_TYPE_SET_NEXT_HOP:
	case NSS_NLGRE_REDIR_CMD_TYPE_ADD_HASH:
	case NSS_NLGRE_REDIR_CMD_TYPE_DEL_HASH:
		return NL_OK;

	default:
		nss_nlsock_log_error("%d:unsupported message cmd type(%d)\n", pid, cmd);
		return NL_SKIP;
	}
}

/*
 * nss_nlgre_redir_sock_open()
 *	Opens the NSS gre_redir NL socket for usage
 */
int nss_nlgre_redir_sock_open(struct nss_nlgre_redir_ctx *ctx, void *user_ctx, nss_nlgre_redir_event_t event_cb)
{
	pid_t pid = getpid();
	int error;

	if (!ctx) {
		nss_nlsock_log_error("%d: invalid parameters passed\n", pid);
		return -EINVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	nss_nlsock_set_family(&ctx->sock, NSS_NLGRE_REDIR_FAMILY);
	nss_nlsock_set_user_ctx(&ctx->sock, user_ctx);

	/*
	 * try opening the socket with Linux
	 */
	error = nss_nlsock_open(&ctx->sock, nss_nlgre_redir_sock_cb);
	if (error) {
		nss_nlsock_log_error("%d:unable to open NSS gre_redir socket, error(%d)\n", pid, error);
		goto fail;
	}

	return 0;
fail:
	memset(ctx, 0, sizeof(*ctx));
	return error;
}

/*
 * nss_nlgre_redir_sock_close()
 *	Close the NSS gre_redir NL socket
 */
void nss_nlgre_redir_sock_close(struct nss_nlgre_redir_ctx *ctx)
{
	nss_nlsock_close(&ctx->sock);
	memset(ctx, 0, sizeof(struct nss_nlgre_redir_ctx));
}

/*
 * nss_nlgre_redir_sock_send()
 *	Send the gre_redir message synchronously through the socket
 */
int nss_nlgre_redir_sock_send(struct nss_nlgre_redir_ctx *ctx, struct nss_nlgre_redir_rule *rule, nss_nlgre_redir_resp_t cb, void *data)
{
	int32_t family_id = ctx->sock.family_id;
	struct nss_nlgre_redir_resp *resp;
	pid_t pid = getpid();
	bool has_resp = false;
	int error = 0;

	if (!rule) {
		nss_nlsock_log_error("%d:invalid NSS gre_redir rule\n", pid);
		return -EINVAL;
	}

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
		nss_nlsock_log_error("%d:failed to send NSS gre_redir rule, error(%d)\n", pid, error);
	}

	return error;
}

/*
 * nss_nlgre_redir_init_rule()
 *	Initialize the gre_redir rule
 */
void nss_nlgre_redir_init_rule(struct nss_nlgre_redir_rule *rule, enum nss_nlgre_redir_cmd_type type)
{
	nss_nlgre_redir_rule_init(rule, type);
}
