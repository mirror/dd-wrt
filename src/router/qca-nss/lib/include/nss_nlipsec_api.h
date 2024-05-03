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

#ifndef __NSS_NLIPSEC_API_H__
#define __NSS_NLIPSEC_API_H__

/** @addtogroup chapter_nlipsec
 This chapter describes IP security (IPsec) APIs in the user space.
 These APIs are wrapper functions for IPsec family specific operations.
*/

/** @addtogroup nss_nlipsec_datatypes @{ */

/**
 * Response callback for IPsec.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule IPsec rule.
 * @param[in] resp_ctx User data per callback.
 *
 * @return
 * None.
 */
typedef void (*nss_nlipsec_resp_t)(void *user_ctx, struct nss_nlipsec_rule *rule, void *resp_ctx);

/**
 * Event callback for IPsec.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule IPsec rule.
 *
 * @return
 * None.
 */
typedef void (*nss_nlipsec_event_t)(void *user_ctx, struct nss_nlipsec_rule *rule);

/**
 * NSS NL IPsec response.
 */
struct nss_nlipsec_resp {
	void *data;		/**< Response context. */
	nss_nlipsec_resp_t cb;	/**< Response callback. */
};

/**
 * NSS NL IPsec context.
 */
struct nss_nlipsec_ctx {
	struct nss_nlsock_ctx sock;	/**< NSS socket context. */
	nss_nlipsec_event_t event;	/**< NSS event callback function. */
};

/** @} *//* end_addtogroup nss_nlipsec_datatypes */
/** @addtogroup nss_nlipsec_functions @{ */

/**
 * Opens NSS NL IPsec socket.
 *
 * @param[in] ctx  NSS NL socket context allocated by the caller.
 * @param[in] user_ctx  User context stored per socket.
 * @param[in] event_cb Event callback handler.
 *
 * @return
 * Status of the open call.
 */
int nss_nlipsec_sock_open(struct nss_nlipsec_ctx *ctx, void *user_ctx, nss_nlipsec_event_t event_cb);

/**
 * Closes NSS NL IPsec socket.
 *
 * @param[in] ctx NSS NL context.
 *
 * @return
 * None.
 */
void nss_nlipsec_sock_close(struct nss_nlipsec_ctx *ctx);

/**
 * Sends NSS IPsec rule message synchronously via netlink.
 *
 * @param[in] ctx NSS NL IPsec context.
 * @param[in] rule IPsec rule.
 * @param[in] cb Response callback handler.
 * @param[in] data Response data per callback.
 *
 * @return
 * Send status:
 * - 0 -- Success.
 * - Negative version error (-ve) -- Failure.
 */
int nss_nlipsec_sock_send(struct nss_nlipsec_ctx *ctx, struct nss_nlipsec_rule *rule, nss_nlipsec_resp_t cb, void *data);

/**
 * IPsec initialization command.
 *
 * @param[in] rule IPsec rule.
 * @param[in] type Command type.
 *
 * @return
 * None.
 */
void nss_nlipsec_init_cmd(struct nss_nlipsec_rule *rule, enum nss_nlipsec_cmd type);

/** @} *//* end_addtogroup nss_nlipsec_functions */

#endif /* __NSS_NLIPV4_API_H__ */
