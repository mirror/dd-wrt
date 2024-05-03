/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_NLUDP_ST_API_H__
#define __NSS_NLUDP_ST_API_H__

/** @addtogroup chapter_udp_st
 A User Datagram Protocol (UDP) speed test measures downlink and uplink performance of
 a device. The NSS Netlink UDP speed test can transmit and receive speed.

 For a transmission test, the UDP speed test creates the transmission node that allocates
 buffers that are encapsulated with IP and UDP headers. These buffers are then sent out of
 the device to measure transmission performance of the device. This test will create IPv4 or
 IPv6 rules from the user space to measure transmission performance of Network Processing Unit
 (NPU) acceleration. It will also configure the UDP Speed transmission node to create packets
 for the IP rule provided.

 For a receive test, the UDP speed test only creates IPv4 or Ipv6 rules to measure received
 performance of the NPU. Users should only test either transmission performance or receive
 performance. This test receives buffers that measure receive performance of the device. No
 packet verification is done during a receive test.

 For each trasnmission and receive speed test, a stop command will be issued to stop the test.
 Test statistics will be provided in a response message each time a UDP speed test is stopped.
 The transmission node can be destroyed by sending a destroy message to the firmware that will
 deallocate the resources assigned for the node.
*/

/** @addtogroup nss_udp_st_datatypes @{ */

/**
 * Response callback for UDP speed test.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule UDP speed test rule associated with the response.
 * @param[in] resp_ctx User data per callback.
 *
 * @return
 * None.
 */
typedef void (*nss_nludp_st_resp_t)(void *user_ctx, struct nss_nludp_st_rule *rule, void *resp_ctx);

/**
 * Event callback for UDP speed test.
 *
 * @param[in] user_ctx User context (provided at socket open).
 * @param[in] rule UDP speed test rule associated with the event.
 *
 * @return
 * None.
 */
typedef void (*nss_nludp_st_event_t)(void *user_ctx, struct nss_nludp_st_rule *rule);

/**
 * NSS NL UDP speed test response.
 */
struct nss_nludp_st_resp {
	void *data;		/**< Response context. */
	nss_nludp_st_resp_t cb;	/**< Response callback. */
};

/**
 * NSS NL UDP speed test context.
 */
struct nss_nludp_st_ctx {
	struct nss_nlsock_ctx sock;	/**< NSS socket context. */
	nss_nludp_st_event_t event;	/**< NSS event callback function. */
};

/** @} *//* end_addtogroup nss_udp_st_datatypes */

/** @addtogroup nss_udp_st_functions @{ */

/**
 * Opens NSS NL UDP speed test socket.
 *
 * @param[in] ctx NSS NL socket context allocated by the caller.
 * @param[in] user_ctx User context stored per socket.
 * @param[in] event_cb Event callback handler.
 *
 * @return
 * Send status:
 * - 0 -- Success.
 * - -ve -- Failure.
 */
int nss_nludp_st_sock_open(struct nss_nludp_st_ctx *ctx, void *user_ctx, nss_nludp_st_event_t event_cb);

/**
 * Closes NSS NL UDP speed test socket.
 *
 * @param[in] ctx NSS NL context.
 *
 * @return
 * None.
 */
void nss_nludp_st_sock_close(struct nss_nludp_st_ctx *ctx);

/**
 * Sends UDP speed test rule synchronously to NSS Netlink.
 *
 * @param[in] ctx NSS UDP speed test NL context.
 * @param[in] rule UDP speed test rule to use.
 * @param[in] cb Response callback handler.
 * @param[in] data Data received from sender.
 *
 * @return
 * Send status:
 * - 0 -- Success.
 * - -ve -- Failure.
 */
int nss_nludp_st_sock_send(struct nss_nludp_st_ctx *ctx, struct nss_nludp_st_rule *rule, nss_nludp_st_resp_t cb, void *data);

/**
 * Initializes UDP speed test rule message.
 *
 * @param[in] rule UDP speed test rule.
 * @param[in] type Type of command.
 *
 * @return
 * None.
 */
void nss_nludp_st_init_rule(struct nss_nludp_st_rule *rule, enum nss_udp_st_message_types type);

/** @} *//* end_addtogroup nss_udp_st_functions */
#endif /* !__NSS_NLUDP_ST_API_H__*/
