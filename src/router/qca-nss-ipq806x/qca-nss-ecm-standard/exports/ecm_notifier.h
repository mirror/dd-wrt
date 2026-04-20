/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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


/**
 * @file ecm_notifier.h
 * ECM notifier subsystem.
 */


#ifndef __ECM_NOTIFIER_H__
#define __ECM_NOTIFIER_H__

/**
 * @addtogroup ecm_notifier_subsystem
 * @{
 */

/**
 * 	ECM connection tuple.
 */
struct ecm_notifier_connection_tuple {
		union {
			struct in_addr in;	/**< IPv4 address in host order. */
			struct in6_addr in6;	/**< IPv6 address in host order. */
		} src;				/**< Source address. */
		union {
			struct in_addr in;	/**< IPv4 address in host order. */
			struct in6_addr in6;	/**< IPv6 address in host order. */
		} dest;				/**< Destination address. */
		uint16_t dst_port;		/**< Destination port in host order. */
		uint16_t src_port;		/**< Source port port in host order. */
		uint8_t protocol;		/**< Next protocol header number. */
		uint8_t ip_ver;			/**< IP version 4 or 6. */
};

/**
 * 	State of the connection in ECM.
 */
enum ecm_notifier_connection_state {
	ECM_NOTIFIER_CONNECTION_STATE_ACCEL,		/**< Connection rule is pushed to NSS. */
	ECM_NOTIFIER_CONNECTION_STATE_ACCEL_PENDING,	/**< Connection is in the process of being accelerated to NSS. */
	ECM_NOTIFIER_CONNECTION_STATE_DECEL,		/**< Connection rule is not pushed to NSS. */
	ECM_NOTIFIER_CONNECTION_STATE_DECEL_PENDING,	/**< Connection is in the process of being decelerated to NSS. */
	ECM_NOTIFIER_CONNECTION_STATE_FAILED,		/**< Connection failed to accelerate. */
	ECM_NOTIFIER_CONNECTION_STATE_INVALID,		/**< Connection is not present in ECM DB. */
	ECM_NOTIFIER_CONNECTION_STATE_MAX		/**< Indicates the last item. */
};

/**
 * 	Event data for a connection notification.
 */
struct ecm_notifier_connection_data {
	struct net_device *to_dev;			/**< First device in ECM 'to' interface hierarchy. */
	struct net_device *from_dev;			/**< First device in ECM 'from' interface hierarchy. */
	struct ecm_notifier_connection_tuple tuple;	/**< Connection tuple information. */
};

/**
 * 	Notifier action for all ECM events.
 */
enum ecm_notifier_action {
	ECM_NOTIFIER_ACTION_CONNECTION_ADDED,		/**< Connection added. */
	ECM_NOTIFIER_ACTION_CONNECTION_REMOVED,		/**< Connection removed. */
	ECM_NOTIFIER_ACTION_MAX				/**< Indicates the last item. */
};

/**
 * Registers a client with the ECM for connection notifications.
 *
 * @param	nb	Notifier block.
 *
 * @return
 * The status of the connection register notification.
 */
extern int ecm_notifier_register_connection_notify(struct notifier_block *nb);

/**
 * Unregisters a client from the ECM for connection notifications.
 *
 * @param	nb	Notifier block.
 *
 * @return
 * The status of the connection deregister notification.
 */
extern int ecm_notifier_unregister_connection_notify(struct notifier_block *nb);

/**
 * Gets the ECM notifier connection state.
 *
 * @param	conn	ECM notifier connection tuple.
 *
 * @return
 * The connection state (#ecm_notifier_connection_state).
 */
extern enum ecm_notifier_connection_state ecm_notifier_connection_state_get(struct ecm_notifier_connection_tuple *conn);

/**
 * @}
 */

#endif
