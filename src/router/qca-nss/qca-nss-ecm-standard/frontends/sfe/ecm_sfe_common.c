/*
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/inet.h>
#include <net/ipv6.h>
#include <linux/etherdevice.h>
#include <net/sch_generic.h>
#define DEBUG_LEVEL ECM_SFE_COMMON_DEBUG_LEVEL

#include <sfe_api.h>

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_datagram.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_interface.h"
#include "ecm_front_end_common.h"
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_ipv6.h"
#include "ecm_sfe_common.h"
#include "exports/ecm_sfe_common_public.h"


/*
 * Callback object to support SFE frontend interaction with external code
 */
struct ecm_sfe_common_callbacks ecm_sfe_cb;

/*
 * Sysctl table
 */
static struct ctl_table_header *ecm_sfe_ctl_tbl_hdr;

static int ecm_sfe_fast_xmit_enable = 1;

/*
 * ecm_sfe_common_fast_xmit_check()
 *	Check the fast transmit feasibility.
 *
 * It only check device related attribute:
 */
static bool ecm_sfe_common_fast_xmit_check(s32 interface_num) {

	struct net_device *dev;

	/*
	 * Return failure if user has disabled SFE fast_xmit
	 */
	if (!ecm_sfe_fast_xmit_enable) {
		return false;
	}

	dev = dev_get_by_index(&init_net, interface_num);
	if (!dev) {
		DEBUG_INFO("device-ifindex[%d] is not present\n", interface_num);
		return false;
	}

	BUG_ON(!rcu_read_lock_bh_held());

#ifdef ECM_INTERFACE_IPSEC_ENABLE
	if (dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
		DEBUG_INFO("Fast xmit is not enabled for ipsec device[%s]\n", dev->name);
		dev_put(dev);
		return false;
	}
#endif
	dev_put(dev);
	return true;
}

/*
 * ecm_sfe_common_qdisc_check()
 * 	Check whether the ifindex has a qdisc attached
 */
static bool ecm_sfe_common_qdisc_check(s32 interface_num)
{
	struct net_device *dev;
	struct netdev_queue *txq;
	int i;
	struct Qdisc *q;
#if 0 //defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
	struct mini_Qdisc *miniq;
#endif

	dev = dev_get_by_index(&init_net, interface_num);
	if (!dev) {
		DEBUG_INFO("device-ifindex[%d] is not present\n", interface_num);
		return false;
	}

	BUG_ON(!rcu_read_lock_bh_held());

	/*
	 * It assume that the qdisc attribute won't change after traffic
	 * running, if the qdisc changed, we need flush all of the rule.
	 */
	for (i = 0; i < dev->real_num_tx_queues; i++) {
		txq = netdev_get_tx_queue(dev, i);
		q = rcu_dereference_bh(txq->qdisc);
		if (q->enqueue) {
			DEBUG_INFO("Qdisc is present for device[%s]\n", dev->name);
			dev_put(dev);
			return true;
		}
	}

#if 0 //defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
	miniq = rcu_dereference_bh(dev->miniq_egress);
	if (miniq) {
		DEBUG_INFO("Egress needed\n");
		dev_put(dev);
		return true;
	}
#endif

	dev_put(dev);

	return false;
}

/*
 * ecm_sfe_feature_check()
 *	Check some specific features for SFE acceleration
 */
bool ecm_sfe_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr, bool is_routed)
{
	if (!is_routed && !sfe_is_l2_feature_enabled()) {
		return false;
	}

	return ecm_front_end_feature_check(skb, ip_hdr);
}

/*
 * ecm_sfe_common_is_l2_iface_supported()
 *	Check if full offload can be supported in SFE engine for the given L2 interface and interface type
 */
bool ecm_sfe_common_is_l2_iface_supported(ecm_db_iface_type_t ii_type, int cur_heirarchy_index, int first_heirarchy_index)
{
	/*
	 * If extended feature is not supported, we dont need to check interface heirarchy.
	 */
	if (!sfe_is_l2_feature_enabled()) {
		DEBUG_TRACE("There is no support for extended features\n");
		return false;
	}

	switch (ii_type) {
	case ECM_DB_IFACE_TYPE_BRIDGE:
	case ECM_DB_IFACE_TYPE_OVS_BRIDGE:

		/*
		 * Below checks ensure that bridge slave interface is not a subinterce and top interface is bridge interface.
		 * This means that we support only ethX-br-lan in the herirarchy.
		 * We can remove all these checks if all l2 features are supported.
		 */

		if (cur_heirarchy_index != (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
			DEBUG_TRACE("Top interface is not bridge, current index=%d\n", cur_heirarchy_index);
			goto fail;
		}
		return true;

	case ECM_DB_IFACE_TYPE_MACVLAN:
		return true;

	default:
		break;
	}

fail:
	return false;
}

/*
 * ecm_sfe_common_fast_xmit_set()
 *	Configure the qdisc and fast transmit settings in the rule.
 *
 * Note: We configure SFE to use full L2 offload in case a single qdisc or no qdisc is enabled in the xmit interface
 * hierarchy for the direction. This configuration sequence works as below.
 *
 * Step 1.) Based on the qdisc checks, we first decide the destination interface for the direction as below:
 *   a.) More than one qdisc in heirarchy - use top interface and disable L2 forwarding by disabling bottom
 *   	interface setting.
 *   b.) Single qdisc or no qdisc in heirarchy - enable L2 offload (set bottom interface flag). If qdisc is found,
 *   we also configure the interface number on which qdisc is enabled, in the qdisc rule. SFE will use bottom
 *   interface and qdisc interface settings to enable full L2 offload along with qdisc processing.
 *
 * Step 2.) Fast transmit setting is enabled on the destination interface only when no qdisc
 * 	is found in the hierarchy.
 */
void ecm_sfe_common_fast_xmit_set(uint16_t *rule_flags, uint16_t *valid_flags, struct sfe_qdisc_rule *qdisc_rule, struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX], struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX], int32_t from_interfaces_first, int32_t to_interfaces_first)
{
	s32 interface_num;
	bool qdisc_found = false;
	bool status;
	int list_index;

	rcu_read_lock_bh();

	/*
	 * Check if a single qdisc is enabled in the interface heirarchy. If yes, configure qdisc rule
	 */
	qdisc_rule->flow_qdisc_interface = -1;
	for (list_index = from_interfaces_first; list_index < ECM_DB_IFACE_HEIRARCHY_MAX; list_index++) {
		interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[list_index]);
		status = ecm_sfe_common_qdisc_check(interface_num);
		if (status) {
			if (!qdisc_found) {
				qdisc_found = true;
				qdisc_rule->valid_flags |= SFE_QDISC_RULE_FLOW_VALID;
				qdisc_rule->flow_qdisc_interface = interface_num;
			} else {
				qdisc_rule->valid_flags &= ~SFE_QDISC_RULE_FLOW_VALID;
				qdisc_rule->flow_qdisc_interface = -1;
				break;
			}
		}
	}

	/*
	 * We have found more than one qdisc enabled in the interface heirarchy.
	 * So strip the bottom interface flag for this case.
	 */
	if (qdisc_found && qdisc_rule->flow_qdisc_interface == -1) {
		*rule_flags &= ~SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
	}

	/*
	 * Check if we can enable fast transmit for the destination interface (top/bottom)
	 */
	interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1]);
        if (*rule_flags & SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE) {
		interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[from_interfaces_first]);
	}

	if (!qdisc_found && ecm_sfe_common_fast_xmit_check(interface_num)) {
		*rule_flags |= SFE_RULE_CREATE_FLAG_RETURN_TRANSMIT_FAST;
	}

	qdisc_found = false;

	/*
	 * Check if a single qdisc is enabled in the interface heirarchy. If yes, configure qdisc rule
	 */
	qdisc_rule->return_qdisc_interface = -1;
	for (list_index = to_interfaces_first; list_index < ECM_DB_IFACE_HEIRARCHY_MAX; list_index++) {
		interface_num = ecm_db_iface_interface_identifier_get(to_ifaces[list_index]);
		status = ecm_sfe_common_qdisc_check(interface_num);
		if (status) {
			if (!qdisc_found) {
				qdisc_found = true;
				qdisc_rule->return_qdisc_interface = interface_num;
				qdisc_rule->valid_flags |= SFE_QDISC_RULE_RETURN_VALID;
			} else {
				qdisc_rule->valid_flags &= ~SFE_QDISC_RULE_RETURN_VALID;
				qdisc_rule->return_qdisc_interface = -1;
				break;
			}
		}
	}

	/*
	 * We have found more than one qdisc enabled in the interface heirarchy.
	 * So strip the bottom interface flag for this case.
	 */
	if (qdisc_found && qdisc_rule->return_qdisc_interface == -1) {
		*rule_flags &= ~SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
	}

	/*
	 * Check if we can enable fast transmit for the destination interface (top/bottom)
	 */
	interface_num = ecm_db_iface_interface_identifier_get(to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);
        if (*rule_flags & SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE) {
		interface_num = ecm_db_iface_interface_identifier_get(to_ifaces[to_interfaces_first]);
	}

	if (!qdisc_found && ecm_sfe_common_fast_xmit_check(interface_num)) {
		*rule_flags |= SFE_RULE_CREATE_FLAG_FLOW_TRANSMIT_FAST;
	}

	/*
	 * Set the Qdisc rule valid flag if qdisc is present in any direction.
	 */
	if ((qdisc_rule->valid_flags & SFE_QDISC_RULE_FLOW_VALID) || (qdisc_rule->valid_flags & SFE_QDISC_RULE_RETURN_VALID)) {
		*valid_flags |= SFE_RULE_CREATE_QDISC_RULE_VALID;
	}

	rcu_read_unlock_bh();
}

/*
 * ecm_sfe_fast_xmit_enable_handler()
 *	Fast transmit sysctl node handler.
 */
int ecm_sfe_fast_xmit_enable_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	if ((ecm_sfe_fast_xmit_enable != 0) && (ecm_sfe_fast_xmit_enable != 1)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		return -EINVAL;
	}

	return ret;
}

/*
 * ecm_sfe_ipv4_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_sfe_ipv4_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_sfe_ipv4_accelerated_count == sfe_ipv4_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for SFE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_sfe_ipv4_accelerated_count);
		return true;
	}

	return false;
}

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_sfe_ipv6_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_sfe_ipv6_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_sfe_ipv6_accelerated_count == sfe_ipv6_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for SFE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_sfe_ipv6_accelerated_count);
		return true;
	}

	return false;
}

#endif

static struct ctl_table ecm_sfe_sysctl_tbl[] = {
	{
		.procname	= "sfe_fast_xmit_enable",
		.data		= &ecm_sfe_fast_xmit_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_sfe_fast_xmit_enable_handler,
	},
	{}
};

/*
 * ecm_sfe_sysctl_tbl_init()
 * 	Register sysctl for SFE
 */
int ecm_sfe_sysctl_tbl_init()
{
	ecm_sfe_ctl_tbl_hdr = register_sysctl(ECM_FRONT_END_SYSCTL_PATH, ecm_sfe_sysctl_tbl);
	if (!ecm_sfe_ctl_tbl_hdr) {
		DEBUG_WARN("Unable to register ecm_sfe_sysctl_tbl");
		return -EINVAL;
	}

	return 0;
}

/*
 * ecm_sfe_sysctl_tbl_exit()
 * 	Unregister sysctl for SFE
 */
void ecm_sfe_sysctl_tbl_exit()
{
	if (ecm_sfe_ctl_tbl_hdr) {
		unregister_sysctl_table(ecm_sfe_ctl_tbl_hdr);
	}
}

/*
 * ecm_sfe_common_init_fe_info()
 *	Initialize common fe info
 */
void ecm_sfe_common_init_fe_info(struct ecm_front_end_common_fe_info *info)
{
	info->from_stats_bitmap = 0;
	info->to_stats_bitmap = 0;
	info->front_end_flags = 0;
}

/*
 * ecm_sfe_common_update_rule()
 *	Updates the frontend specifc data.
 *
 * Currently, only updates the mark values of the connection and updates the SFE AE.
 */
void ecm_sfe_common_update_rule(struct ecm_front_end_connection_instance *feci, enum ecm_rule_update_type type, void *arg)
{

	switch (type) {
	case ECM_RULE_UPDATE_TYPE_CONNMARK:
	{
		struct nf_conn *ct = (struct nf_conn *)arg;
		struct sfe_connection_mark mark;
		ip_addr_t src_addr;
		ip_addr_t dest_addr;
		int aci_index;
		int assignment_count;
		struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];

		if (ecm_front_end_connection_accel_state_get(feci) != ECM_FRONT_END_ACCELERATION_MODE_ACCEL) {
			DEBUG_WARN("%px: connection is not in accelerated mode\n", feci);
			return;
		}

		/*
		 * Get connection information
		 */
		mark.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);
		mark.src_port = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
		mark.dest_port = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));
		ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_addr);
		ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, dest_addr);
		mark.mark = ct->mark;

		DEBUG_TRACE("%px: Update the mark value for the SFE connection\n", feci);

		if (feci->ip_version == 4) {
			ECM_IP_ADDR_TO_NIN4_ADDR(mark.src_ip[0], src_addr);
			ECM_IP_ADDR_TO_NIN4_ADDR(mark.dest_ip[0], dest_addr);
			sfe_ipv4_mark_rule_update(&mark);
			DEBUG_TRACE("%px: src_ip: %pI4 dest_ip: %pI4 src_port: %d dest_port: %d protocol: %d\n",
				    feci, &mark.src_ip[0], &mark.dest_ip[0],
				    ntohs(mark.src_port), ntohs(mark.dest_port), mark.protocol);
		} else {
			ECM_IP_ADDR_TO_SFE_IPV6_ADDR(mark.src_ip, src_addr);
			ECM_IP_ADDR_TO_SFE_IPV6_ADDR(mark.dest_ip, dest_addr);
			sfe_ipv6_mark_rule_update(&mark);
			DEBUG_TRACE("%px: src_ip: " ECM_IP_ADDR_OCTAL_FMT "dest_ip: " ECM_IP_ADDR_OCTAL_FMT
				    " src_port: %d dest_port: %d protocol: %d\n",
				    feci, ECM_IP_ADDR_TO_OCTAL(src_addr), ECM_IP_ADDR_TO_OCTAL(dest_addr),
				    ntohs(mark.src_port), ntohs(mark.dest_port), mark.protocol);
		}

		/*
		 * Get the assigned classifiers and call their update callbacks. If they are interested in this type of
		 * update, they will handle the event.
		 */
		assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(feci->ci, assignments);
		for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
			struct ecm_classifier_instance *aci;
			aci = assignments[aci_index];
			if (aci->update) {
				aci->update(aci, type, ct);
			}
		}
		ecm_db_connection_assignments_release(assignment_count, assignments);

		break;
	}
	default:
		DEBUG_WARN("%px: unsupported update rule type: %d\n", feci, type);
		break;
	}
}

/*
 * ecm_sfe_common_tuple_set()
 *	Sets the SFE common tuple object with the ECM connection rule paramaters.
 *
 * This tuple object will be used by external module to make decision on L2 acceleration.
 */
void ecm_sfe_common_tuple_set(struct ecm_front_end_connection_instance *feci,
			      int32_t from_iface_id, int32_t to_iface_id,
			      struct ecm_sfe_common_tuple *tuple)
{
	ip_addr_t saddr;
	ip_addr_t daddr;

	tuple->protocol = ecm_db_connection_protocol_get(feci->ci);
	tuple->ip_ver = feci->ip_version;

	tuple->src_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
        tuple->dest_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	tuple->src_ifindex = from_iface_id;
	tuple->dest_ifindex = to_iface_id;

	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, saddr);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, daddr);

	if (feci->ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple->src_addr[0], saddr);
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple->dest_addr[0], daddr);
	} else {
		ECM_IP_ADDR_TO_SFE_IPV6_ADDR(tuple->src_addr, saddr);
		ECM_IP_ADDR_TO_SFE_IPV6_ADDR(tuple->dest_addr, daddr);
	}
}

/*
 * ecm_sfe_common_defunct_ipv4_connection()
 *	Defunct an IPv4 5-tuple connection.
 */
bool ecm_sfe_common_defunct_ipv4_connection(__be32 src_ip, int src_port,
					    __be32 dest_ip, int dest_port, int protocol)
{
	return ecm_db_connection_decel_v4(src_ip, src_port, dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_sfe_common_defunct_ipv4_connection);

/*
 * ecm_sfe_common_defunct_ipv6_connection()
 *	Defunct an IPv6 5-tuple connection.
 */
bool ecm_sfe_common_defunct_ipv6_connection(struct in6_addr *src_ip, int src_port,
					    struct in6_addr *dest_ip, int dest_port, int protocol)
{
	return ecm_db_connection_decel_v6(src_ip, src_port, dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_sfe_common_defunct_ipv6_connection);

/*
 * ecm_sfe_common_defunct_by_protocol()
 *	Defunct the connections by the protocol type (e.g:TCP, UDP)
 */
void ecm_sfe_common_defunct_by_protocol(int protocol)
{
	ecm_db_connection_defunct_by_protocol(protocol);
}
EXPORT_SYMBOL(ecm_sfe_common_defunct_by_protocol);

/*
 * ecm_sfe_common_defunct_by_port()
 *	Defunct the connections associated with this port in the direction
 * relative to the ECM's connection direction as well.
 *
 * TODO:
 *	For now, all the connections from/to this port number are defuncted.
 *	Directional defunct can be implemented later, but there is a trade of here:
 *	For each connection in the database, the connection's from/to interfaces will
 *	be checked with the wan_name and direction will be determined and then the connection
 *	will be defuncted if there is a match with this port number. This process may be heavier
 *	than defuncting all the connections from/to this port number. So, the direction and  wan_name
 *	are optional for this API for now.
 */
void ecm_sfe_common_defunct_by_port(int port, int direction, char *wan_name)
{
	ecm_db_connection_defunct_by_port(htons(port), ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_defunct_by_port(htons(port), ECM_DB_OBJ_DIR_TO);
}
EXPORT_SYMBOL(ecm_sfe_common_defunct_by_port);

/*
 * ecm_sfe_common_callbacks_register()
 *	Registers SFE common callbacks.
 */
int ecm_sfe_common_callbacks_register(struct ecm_sfe_common_callbacks *sfe_cb)
{
	if (!sfe_cb || !sfe_cb->l2_accel_check) {
		DEBUG_ERROR("SFE L2 acceleration check callback is NULL\n");
		return -EINVAL;
	}

	rcu_assign_pointer(ecm_sfe_cb.l2_accel_check, sfe_cb->l2_accel_check);
	synchronize_rcu();

	return 0;
}
EXPORT_SYMBOL(ecm_sfe_common_callbacks_register);

/*
 * ecm_sfe_common_callbacks_unregister()
 *	Unregisters SFE common callbacks.
 */
void ecm_sfe_common_callbacks_unregister(void)
{
	rcu_assign_pointer(ecm_sfe_cb.l2_accel_check, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(ecm_sfe_common_callbacks_unregister);
