/*
 **************************************************************************
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <net/ip.h>
#include <linux/inet.h>
#include <linux/netfilter_bridge.h>
#include <linux/atomic.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_NOTIFIER_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_db.h"
#include "ecm_front_end_common.h"

#include "ecm_notifier_pvt.h"
#include "exports/ecm_notifier.h"

static atomic_t ecm_notifier_count;
static ATOMIC_NOTIFIER_HEAD(ecm_notifier_connection);

/*
 * ecm_notifier_ci_to_data()
 * 	Convert ci to ecm_notifier_connection_data.
 *
 * This function holds reference to devices (data->from_dev & data->to_dev).
 */
static bool ecm_notifier_ci_to_data(struct ecm_db_connection_instance *ci, struct ecm_notifier_connection_data *data)
{
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	int32_t first_index;
	struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];

	first_index = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_FROM);
	if (first_index == ECM_DB_IFACE_HEIRARCHY_MAX) {\
		DEBUG_WARN("%px: Failed to get 'from' ifaces index\n", ci);
		return false;
	}
	data->from_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(interfaces[first_index]));
	if (!data->from_dev) {
		DEBUG_WARN("%px: Could not locate 'from' interface\n", ci);
		return false;
	}
	ecm_db_connection_interfaces_deref(interfaces, first_index);

	first_index = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_TO);
	if (first_index == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Failed to get 'to' ifaces index\n", ci);
		dev_put(data->from_dev);
		return false;
	}
	data->to_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(interfaces[first_index]));
	if (!data->to_dev) {
		DEBUG_WARN("%px: Could not locate 'to' interface\n", ci);
		dev_put(data->from_dev);
		return false;
	}
	ecm_db_connection_interfaces_deref(interfaces, first_index);

	data->tuple.ip_ver = ecm_db_connection_ip_version_get(ci);
	data->tuple.protocol = ecm_db_connection_protocol_get(ci);
	data->tuple.src_port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
	data->tuple.dst_port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

	switch (data->tuple.ip_ver) {
	case 4:
		ECM_IP_ADDR_TO_HIN4_ADDR(data->tuple.src.in.s_addr, src_ip);
		ECM_IP_ADDR_TO_HIN4_ADDR(data->tuple.dest.in.s_addr, dst_ip);
		break;

	case 6:
		ECM_IP_ADDR_TO_HIN6_ADDR(data->tuple.src.in6, src_ip);
		ECM_IP_ADDR_TO_HIN6_ADDR(data->tuple.dest.in6, dst_ip);
		break;

	default:
		/*
		 * Shouldn't come here.
		 */
		DEBUG_ERROR("%px: Invalid protocol\n", ci);
		dev_put(data->from_dev);
		dev_put(data->to_dev);
		return false;
	}

	return true;
}

/*
 * ecm_notifier_connection_added()
 * 	Send ECM connection added event to notifier chain.
 */
void ecm_notifier_connection_added(void *arg, struct ecm_db_connection_instance *ci)
{
	struct ecm_notifier_connection_data data = {0};

	if (!atomic_read(&ecm_notifier_count)) {
		DEBUG_TRACE("%px: No notifier has registered for event\n", ci);
		return;
	}

	/*
	 * Module has registered for events.
	 */
	if (!ecm_notifier_ci_to_data(ci, &data)) {
		DEBUG_WARN("%px: Failed to get data from connection instance\n", ci);
		return;
	}

	atomic_notifier_call_chain(&ecm_notifier_connection, ECM_NOTIFIER_ACTION_CONNECTION_ADDED, (void*)&data);

	dev_put(data.from_dev);
	dev_put(data.to_dev);
}

/*
 * ecm_notifier_connection_removed()
 * 	Send ECM connection removed event to notifier chain.
 */
void ecm_notifier_connection_removed(void *arg, struct ecm_db_connection_instance *ci)
{
	struct ecm_notifier_connection_data data = {0};

	if (!atomic_read(&ecm_notifier_count)) {
		DEBUG_TRACE("%px: No notifier has registered for event\n", ci);
		return;
	}

	/*
	 * Module has registered for events.
	 */
	if (!ecm_notifier_ci_to_data(ci, &data)) {
		DEBUG_WARN("%px: Failed to get data from connection instance\n", ci);
		return;
	}

	atomic_notifier_call_chain(&ecm_notifier_connection, ECM_NOTIFIER_ACTION_CONNECTION_REMOVED, (void*)&data);

	dev_put(data.from_dev);
	dev_put(data.to_dev);
}

/*
 * ecm_notifier_register_connection_notify()
 * 	Register for ECM connection events.
 */
int ecm_notifier_register_connection_notify(struct notifier_block *nb)
{
	/*
	 * Currently atomic_notifier_chain_register does not return error and assumed to be always success.
	 * so, incrmenting ecm_notifier_count at beginning.
	 */
	atomic_inc(&ecm_notifier_count);

	return atomic_notifier_chain_register(&ecm_notifier_connection, nb);
}
EXPORT_SYMBOL(ecm_notifier_register_connection_notify);

/*
 * ecm_notifier_unregister_connection_notify()
 * 	Unregister for ECM connection events.
 */
int ecm_notifier_unregister_connection_notify(struct notifier_block *nb)
{
	atomic_dec(&ecm_notifier_count);

	return atomic_notifier_chain_unregister(&ecm_notifier_connection, nb);
}
EXPORT_SYMBOL(ecm_notifier_unregister_connection_notify);

/*
 * ecm_notifier_connection_state_fetch()
 * 	Returns the current state for given connection tuple.
 */
enum ecm_notifier_connection_state ecm_notifier_connection_state_get(struct ecm_notifier_connection_tuple *conn)
{
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_state;
	struct ecm_db_connection_instance *ci;
	int host1_port = conn->src_port;
	int host2_port = conn->dst_port;
	int protocol = conn->protocol;
	ip_addr_t host1_addr;
	ip_addr_t host2_addr;

	DEBUG_ASSERT(conn, "Connection tuple is NULL\n");

        switch (conn->ip_ver) {
        case 4:
                ECM_HIN4_ADDR_TO_IP_ADDR(host1_addr, conn->src.in.s_addr);
                ECM_HIN4_ADDR_TO_IP_ADDR(host2_addr, conn->dest.in.s_addr);
		DEBUG_TRACE("%px: lookup src: " ECM_IP_ADDR_DOT_FMT ":%d, "
				"dest: " ECM_IP_ADDR_DOT_FMT ":%d, "
				"protocol %d\n",
				conn,
				ECM_IP_ADDR_TO_DOT(host1_addr),
				host1_port,
				ECM_IP_ADDR_TO_DOT(host2_addr),
				host2_port,
				protocol);
                break;

        case 6:
                ECM_HIN6_ADDR_TO_IP_ADDR(host1_addr, conn->src.in6);
                ECM_HIN6_ADDR_TO_IP_ADDR(host2_addr, conn->dest.in6);
		DEBUG_TRACE("%px: lookup src: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
				"dest: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
				"protocol %d\n",
				conn,
				ECM_IP_ADDR_TO_OCTAL(host1_addr),
				host1_port,
				ECM_IP_ADDR_TO_OCTAL(host2_addr),
				host2_port,
				protocol);
                break;

        default:
                return ECM_NOTIFIER_CONNECTION_STATE_INVALID;
        }

	ci = ecm_db_connection_find_and_ref(host1_addr, host2_addr, protocol, host1_port, host2_port);
	if (!ci) {
		DEBUG_TRACE("%px: database connection not found\n", conn);
		return ECM_NOTIFIER_CONNECTION_STATE_INVALID;
	}

	feci = ecm_db_connection_front_end_get_and_ref(ci);
	if (!feci) {
		DEBUG_TRACE("%px: failed to find front end connection instance\n", ci);
		ecm_db_connection_deref(ci);
		return ECM_NOTIFIER_CONNECTION_STATE_INVALID;
	}

	accel_state = ecm_front_end_connection_accel_state_get(feci);
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);

	switch (accel_state) {
	case ECM_FRONT_END_ACCELERATION_MODE_ACCEL:
		return ECM_NOTIFIER_CONNECTION_STATE_ACCEL;

	case ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING:
		return ECM_NOTIFIER_CONNECTION_STATE_ACCEL_PENDING;

	case ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING:
		return ECM_NOTIFIER_CONNECTION_STATE_DECEL_PENDING;

	case ECM_FRONT_END_ACCELERATION_MODE_DECEL:
		return ECM_NOTIFIER_CONNECTION_STATE_DECEL;

	default:
		DEBUG_TRACE("%px: Marking other state as failed\n", conn);
		break;
	}

	return ECM_NOTIFIER_CONNECTION_STATE_FAILED;
}
EXPORT_SYMBOL(ecm_notifier_connection_state_get);
