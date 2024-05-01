/*
 **************************************************************************
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @file ecm_front_end_common_public.h
 *	ECM common frontend public APIs and data structures.
 */

#ifndef __ECM_FE_COMMON_PUBLIC_H__
#define __ECM_FE_COMMON_PUBLIC_H__

/**
 * This structure collects 5-tuple information to send it to
 * WLAN driver via the registered FSE (Flow Search Engine) callback.
 */
struct ecm_front_end_fse_info {
	union {
		__be32 v4_addr;			/**< Source IPv4 address. */
		struct in6_addr v6_addr;	/**< Source IPv6 address. */
	} src;
	union {
		__be32 v4_addr;			/**< Destination IPv4 address. */
		struct in6_addr v6_addr;	/**< Destination IPv6 address. */
	} dest;

	uint8_t ip_version;			/**< IP version. */
	uint8_t protocol;			/**< Protocol number. */
	uint16_t src_port;			/**< Source port. */
	uint16_t dest_port;			/**< Destination port. */
	uint8_t src_mac[ETH_ALEN];		/**< Source MAC. */
	uint8_t dest_mac[ETH_ALEN];		/**< Destination MAC. */
	struct net_device *src_dev;		/**< Source netdevice. */
	struct net_device *dest_dev;		/**< Destination netdevice. */
};

/**
 * Callback to which WLAN driver will register to create FSE flow rule from ECM frontend.
 */
typedef bool (*ecm_front_end_create_fse_rule_callback_t)(struct ecm_front_end_fse_info *fse_info);

/**
 * Callback to which WLAN driver will register to destroy FSE flow rule from ECM frontend.
 */
typedef bool (*ecm_front_end_destroy_fse_rule_callback_t)(struct ecm_front_end_fse_info *fse_info);

/**
 * Data structure for FSE callbacks.
 */
struct ecm_front_end_fse_callbacks {
	ecm_front_end_create_fse_rule_callback_t create_fse_rule;	/**< Callback to create FSE rule from ECM frontend. */
	ecm_front_end_destroy_fse_rule_callback_t destroy_fse_rule;	/**< Callback to destroy FSE rule from ECM frontend. */
};

/**
 * registers FSE callbacks with Front end FSE ops.
 *
 * @param fe_fse_cb FSE callback pointer.
 *
 * @return
 * 0 if success, error value if fails.
 */
int ecm_front_end_fse_callbacks_register(struct ecm_front_end_fse_callbacks *fe_fse_cb);

/**
 * Unregisters FSE callbacks from Front end FSE ops.
 *
 * @return
 * None.
 */
void ecm_front_end_fse_callbacks_unregister(void);

#endif /* __ECM_FE_COMMON_PUBLIC_H__ */
