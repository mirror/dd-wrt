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

#ifndef __NSS_NLCAPWAP_API_H__
#define __NSS_NLCAPWAP_API_H__

/** @addtogroup chapter_nlcapwap
 This chapter describes APIs for Control and Provisioning of Wireless Access Points (CAPWAP)
 in the user space. These APIs are wrapper functions for CAPWAP family specific operations.
*/

/** @addtogroup nss_nlcapwap_datatypes @{ */

/**
 * Response callback for CAPWAP.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule CAPWAP rule.
 * @param[in] resp_ctx  User data per callback.
 *
 * @return
 * None.
 */
typedef void (*nss_nlcapwap_resp_t)(void *user_ctx, struct nss_nlcapwap_rule *rule, void *resp_ctx);

/**
 * Event callback for CAPWAP.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule CAPWAP rule.
 *
 * @return
 * None.
 */
typedef void (*nss_nlcapwap_event_t)(void *user_ctx, struct nss_nlcapwap_rule *rule);

/**
 * NSS NL CAPWAP response.
 */
struct nss_nlcapwap_resp {
	void *data;			/**< Response context. */
	nss_nlcapwap_resp_t cb;		/**< Response callback. */
};

/**
 * NSS NL CAPWAP context.
 */
struct nss_nlcapwap_ctx {
	struct nss_nlsock_ctx sock;	/**< NSS socket context. */
	nss_nlcapwap_event_t event;	/**< NSS event callback function. */
};

/** @} *//* end_addtogroup nss_nlcapwap_datatypes */
/** @addtogroup nss_nlcapwap_functions @{ */

/**
 * Opens NSS NL CAPWAP socket.
 *
 * @param[in] ctx NSS NL socket context allocated by the caller.
 * @param[in] user_ctx User context stored per socket.
 * @param[in] event_cb Event callback handler.
 *
 * @return
 * Status of the open call.
 */
int nss_nlcapwap_sock_open(struct nss_nlcapwap_ctx *ctx, void *user_ctx, nss_nlcapwap_event_t event_cb);

/**
 * Closes NSS NL CAPWAP socket.
 *
 * @param[in] ctx NSS NL socket context allocated by the caller.
 *
 * @return
 * None.
 */
void nss_nlcapwap_sock_close(struct nss_nlcapwap_ctx *ctx);

/**
 * Sends a CAPWAP rule synchronously to NSS NETLINK.
 *
 * @param[in] ctx NSS NL socket context allocated by the caller.
 * @param[in] rule CAPWAP rule.
 * @param[in] cb Response callback handler.
 * @param[in] data Data received from sender.
 *
 * @return
 * Send status:
 * - 0 -- Success.
 * - Negative version error (-ve) -- Failure.
 */
int nss_nlcapwap_sock_send(struct nss_nlcapwap_ctx *ctx, struct nss_nlcapwap_rule *rule, nss_nlcapwap_resp_t cb, void *data);

/**
 * Initializes create rule message.
 *
 * @param[in] rule CAPWAP rule.
 * @param[in] type Type of command.
 *
 * @return
 * None.
 */
void nss_nlcapwap_init_rule(struct nss_nlcapwap_rule *rule, enum nss_nlcapwap_cmd_type type);

/** @} *//* end_addtogroup nss_nlcapwap_functions */

#endif /* __NSS_NLCAPWAP_API_H__ */
