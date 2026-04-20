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

#ifndef __NSS_NLQRFS_API_H__
#define __NSS_NLQRFS_API_H__

/** @addtogroup chapter_nlqrfs
 This chapter describes QRFS APIs in the userspace.
 These APIs are wrapper functions for QRFS family specific operations.
*/

/** @addtogroup nss_nlqrfs_datatypes @{ */

/**
 * Response callback for QRFS.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule qrfs rule
 * @param[in] resp_ctx User data per callback.
 *
 * @return
 * None.
 */
typedef void (*nss_nlqrfs_resp_t)(void *user_ctx, struct nss_nlqrfs_rule *rule, void *resp_ctx);

/**
 * Event callback for QRFS.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule QRFS rule.
 *
 * @return
 * None.
 */
typedef void (*nss_nlqrfs_event_t)(void *user_ctx, struct nss_nlqrfs_rule *rule);

/**
 * NSS NL qrfs response.
 */
struct nss_nlqrfs_resp {
	void *data;		/**< Response context. */
	nss_nlqrfs_resp_t cb;	/**< Response callback. */
};

/**
 * NSS NL qrfs context.
 */
struct nss_nlqrfs_ctx {
	struct nss_nlsock_ctx sock;	/**< NSS socket context. */
	nss_nlqrfs_event_t event;	/**< NSS event callback function. */
};

/** @} *//* end_addtogroup nss_nlqrfs_datatypes */
/** @addtogroup nss_nlqrfs_functions @{ */

/**
 * Open NSS NL QRFS socket.
 *
 * @param[in] ctx NSS NL socket context, allocated by the caller.
 * @param[in] user_ctx User context stored per socket.
 * @param[in] event_cb Event callback handler.
 *
 * @return
 * Status of the open call.
 */
int nss_nlqrfs_sock_open(struct nss_nlqrfs_ctx *ctx, void *user_ctx, nss_nlqrfs_event_t event_cb);

/**
 * Closes NSS NL QRFS socket.
 *
 * @param[in] ctx NSS NL context.
 *
 * @return
 * None.
 */
void nss_nlqrfs_sock_close(struct nss_nlqrfs_ctx *ctx);

/**
 * Sends an QRFS rule asynchronously to the NSS NETLINK.
 *
 * @param[in] ctx NSS NL qrfs NL context.
 * @param[in] rule QRFS rule.
 * @param[in] cb Response callback handler.
 * @param[in] data Response data per callback.
 *
 * @return
 * Send status:
 * - 0 -- Success.
 * - Negative version error (-ve) -- Failure.
 */
int nss_nlqrfs_sock_send(struct nss_nlqrfs_ctx *ctx, struct nss_nlqrfs_rule *rule, nss_nlqrfs_resp_t cb, void *data);

/**
 * Initialize the QRFS rule message
 *
 * @param[in] rule QRFS rule.
 * @param[in] type Command type.
 *
 * @return
 * None.
 */
void nss_nlqrfs_init_rule(struct nss_nlqrfs_rule *rule, enum nss_qrfs_msg_types type);

/** @} *//* end_addtogroup nss_nlqrfs_functions */

#endif /* __NSS_NLQRFS_API_H__ */
