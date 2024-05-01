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

/*
 * OVS Classifier.
 * This classifier is called for the connections which are created
 * on OVS interfaces. The connections can be routed or bridged connections.
 * The classifier processes these connection flows and decides if the connection
 * is ready to be accelerated. Moreover, it has an interface to an external classifier
 * to make smart decisions on the flows, such as extracting the VLAN information from the
 * OVS flow tables and gives it to the ECM front end to be passed to the acceleration engine.
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/ctype.h>
#include <net/tcp.h>
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/etherdevice.h>
#include <linux/netfilter/xt_dscp.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_OVS_DEBUG_LEVEL

#include <ovsmgr.h>
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
#include "ecm_classifier_ovs.h"
#include "ecm_classifier_ovs_public.h"
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv4.h"
#ifdef ECM_IPV6_ENABLE
#include "ecm_front_end_ipv6.h"
#endif
#include "ecm_interface.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_OVS_INSTANCE_MAGIC 0x2568

/*
 * struct ecm_classifier_ovs_instance
 * 	State per connection for OVS classifier
 */
struct ecm_classifier_ovs_instance {
	struct ecm_classifier_instance base;			/* Base type */

	uint32_t ci_serial;					/* RO: Serial of the connection */

	struct ecm_classifier_ovs_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_ovs_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	struct ecm_classifier_process_response process_response;
								/* Last process response computed */
	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Operational control.
 */
static int ecm_classifier_ovs_enabled = 1;			/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_ovs_terminate_pending;	/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_ovs_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_ovs_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_ovs_instance *ecm_classifier_ovs_instances = NULL;
								/* list of all active instances */
static int ecm_classifier_ovs_count = 0;			/* Tracks number of instances allocated */

/*
 * Callback object.
 */
static struct ecm_classifier_ovs_callbacks ovs;

/*
 * ecm_classifier_ovs_ref()
 *	Ref
 */
static void ecm_classifier_ovs_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)ci;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->refs++;
	DEBUG_ASSERT(ecvi->refs > 0, "%px: ref wrap\n", ecvi);
	DEBUG_TRACE("%px: ecvi ref %d\n", ecvi, ecvi->refs);
	spin_unlock_bh(&ecm_classifier_ovs_lock);
}

/*
 * ecm_classifier_ovs_deref()
 *	Deref
 */
static int ecm_classifier_ovs_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)ci;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->refs--;
	DEBUG_ASSERT(ecvi->refs >= 0, "%px: refs wrapped\n", ecvi);
	DEBUG_TRACE("%px: ecvi deref %d\n", ecvi, ecvi->refs);
	if (ecvi->refs) {
		int refs = ecvi->refs;
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_ovs_count--;
	DEBUG_ASSERT(ecm_classifier_ovs_count >= 0, "%px: ecm_classifier_ovs_count wrap\n", ecvi);

	/*
	 * UnLink the instance from our list
	 */
	if (ecvi->next) {
		ecvi->next->prev = ecvi->prev;
	}
	if (ecvi->prev) {
		ecvi->prev->next = ecvi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_ovs_instances == ecvi, "%px: list bad %px\n", ecvi, ecm_classifier_ovs_instances);
		ecm_classifier_ovs_instances = ecvi->next;
	}
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final ovs classifier instance\n", ecvi);
	kfree(ecvi);

	return 0;
}

/*
 * ecm_classifier_ovs_interface_get_and_ref()
 *	Gets the OVS bridge port in the specified direction hierarchy.
 */
static inline struct net_device *ecm_classifier_ovs_interface_get_and_ref(struct ecm_db_connection_instance *ci,
									   ecm_db_obj_dir_t dir, bool ovs_port)
{
	int32_t if_first;
	struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t i;

	/*
	 * Get the interface hierarchy.
	 */
	if_first = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, dir);
	if (if_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Failed to get %s interfaces list\n", ci, ecm_db_obj_dir_strings[dir]);
		return NULL;
	}

	/*
	 * Go through the hierarchy and get the OVS port.
	 */
	for (i = ECM_DB_IFACE_HEIRARCHY_MAX - 1; i >= if_first; i--) {
		struct net_device *dev = dev_get_by_index(&init_net,
							  ecm_db_iface_interface_identifier_get(interfaces[i]));
		if (!dev) {
			DEBUG_WARN("%px: Failed to get net device with %d index\n", ci, i);
			continue;
		}

		if (ovs_port) {
			if (ecm_interface_is_ovs_bridge_port(dev)) {
				ecm_db_connection_interfaces_deref(interfaces, if_first);
				DEBUG_TRACE("%px: %s_dev: %s at %d index is an OVS bridge port\n", ci, ecm_db_obj_dir_strings[dir], dev->name, i);
				return dev;
			}

		} else {
			if (ovsmgr_is_ovs_master(dev)) {
				ecm_db_connection_interfaces_deref(interfaces, if_first);
				DEBUG_TRACE("%px: %s_dev: %s at %d index is an OVS bridge dev\n", ci, ecm_db_obj_dir_strings[dir], dev->name, i);
				return dev;
			}
		}

		DEBUG_TRACE("%px: dev: %s index: %d\n", ci, dev->name, i);
		dev_put(dev);
	}

	DEBUG_TRACE("%px: No OVS bridge port on the %s direction\n", ci, ecm_db_obj_dir_strings[dir]);
	ecm_db_connection_interfaces_deref(interfaces, if_first);
	return NULL;
}

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_classifier_ovs_process_multicast()
 * 	Process multicast packet
 */
static void ecm_classifier_ovs_process_multicast(struct ecm_db_connection_instance *ci, struct sk_buff *skb,
							struct ecm_classifier_ovs_instance *ecvi,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_ovs_process_response resp;
	ecm_classifier_ovs_process_callback_t cb;
	struct ecm_db_iface_instance *to_mc_ifaces;
	struct net_device *from_dev = NULL, *indev_master = NULL;
	struct net_device *to_dev[ECM_DB_MULTICAST_IF_MAX] = {NULL};
	struct ovsmgr_dp_flow flow;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	int32_t *to_mc_ifaces_first;
	int if_cnt, i;
	bool valid_ovs_ports = false;

	/*
	 * Classifier is always relevant for multicast
	 * if we have enabled OVS support in ECM.
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	/*
	 * No multicast port found, drop the connection.
	 */
	if_cnt = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &to_mc_ifaces, &to_mc_ifaces_first);
	if (unlikely(!if_cnt)) {
		spin_lock_bh(&ecm_classifier_ovs_lock);
		ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DROP;
		ecvi->process_response.drop = true;
		*process_response = ecvi->process_response;
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		DEBUG_WARN("%px: No multicast 'to' interface found\n", ci);
		goto done;
	}

	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
		int32_t *to_iface_first;
		int j;

		/*
		 * Get to interface list, skip if the list is invalid.
		 */
		to_iface_first = ecm_db_multicast_if_first_get_at_index(to_mc_ifaces_first, i);
		if (*to_iface_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			continue;
		}

		for (j = to_mc_ifaces_first[i]; j < ECM_DB_IFACE_HEIRARCHY_MAX; j++) {
			struct ecm_db_iface_instance **ifaces;
			struct ecm_db_iface_instance *ii_temp;
			struct net_device *to_dev_temp;

			ii_temp = ecm_db_multicast_if_heirarchy_get(to_mc_ifaces, i);
			ifaces = (struct ecm_db_iface_instance **)ecm_db_multicast_if_instance_get_at_index(ii_temp, j);
			to_dev_temp = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(*ifaces));
			if (unlikely(!to_dev_temp)) {
				continue;
			}

			/*
			 * Get the physical OVS port.
			 */
			if (!ecm_interface_is_ovs_bridge_port(to_dev_temp)) {
				dev_put(to_dev_temp);
				continue;
			}

			to_dev[i] = to_dev_temp;
			valid_ovs_ports = true;
		}
	}
	ecm_db_multicast_connection_to_interfaces_deref_all(to_mc_ifaces, to_mc_ifaces_first);

	from_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, true);
	if (!from_dev && !valid_ovs_ports) {
		spin_lock_bh(&ecm_classifier_ovs_lock);
		*process_response = ecvi->process_response;
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		DEBUG_WARN("%px: None of the from/to interfaces are OVS bridge port\n", ci);
		goto done;
	}

	/*
	 * Below are the possible multicast flows.
	 *
	 * 1. L2 Multicast (Bridge)
	 * 	ovsbr (eth0, eth1,eth2)
	 * 	a. sender: eth0, receiver: eth1, eth2
	 *
	 * 2. L3 Multicast (Route)
	 * 	Upstream: br-wan (eth0)
	 * 	Downstream: br-home (eth1, eth2)
	 * 	a. Downstream flow => sender: eth0, receiver: eth1, eth2
	 * 	b. Upstream flow => sender: eth1, receiver: eth0
	 *
	 * 3. L2 + L3 Multicast (Bridge + Route)
	 * 	Upstream: br-wan (eth0)
	 * 	Downstream: br-home (eth1, eth2)
	 * 	a. sender: eth1, receiver: eth0, eth2
	 */

	/*
	 * Is there an external callback to get the ovs value from the packet?
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	cb = ovs.ovs_process;
	if (!cb) {
		/*
		 * Allow acceleration.
		 * Keep the classifier relevant to connection for stats update..
		 */
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		DEBUG_WARN("%px: No external process callback set\n", ci);
		goto allow_accel;
	}
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	memset(&flow, 0, sizeof(struct ovsmgr_dp_flow));

	if (from_dev) {
		flow.indev = from_dev;
		indev_master = ovsmgr_dev_get_master(flow.indev);
		DEBUG_ASSERT(indev_master, "%px: Expected a master\n", ci);
	}

	flow.is_routed = ecm_db_connection_is_routed_get(ci);
	flow.tuple.ip_version = ecm_db_connection_ip_version_get(ci);
	flow.tuple.protocol = ecm_db_connection_protocol_get(ci);
	flow.tuple.src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
	flow.tuple.dst_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, flow.smac);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, flow.dmac);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

	if (flow.tuple.ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, dst_ip);
	} else if (flow.tuple.ip_version == 6) {
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, dst_ip);
	} else {
		DEBUG_ASSERT(NULL, "%px: unexpected ip_version: %d", ci, flow.tuple.ip_version );
	}

	/*
	 * Check if the connection is a "bridge + route"
	 * flow.
	 */
	if (flow.is_routed) {
		/*
		 * Get the ingress VLAN information from ovs-mgr
		 */
		if (from_dev) {
			struct net_device *br_dev;
			ecm_classifier_ovs_result_t result;

			/*
			 * from_dev = eth1
			 * br_dev = ovs-br1
			 */
			br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, false);
			DEBUG_TRACE("%px: processing route flow from_dev = %s, br_dev = %s", ecvi, from_dev->name, br_dev->name);

			/*
			 * We always take the flow from bridge to port, so indev is port device and outdev is bridge device.
			 */
			flow.indev = from_dev;
			flow.outdev = br_dev;

			DEBUG_TRACE("%px: Route Flow Process (from): src MAC: %pM src_dev: %s src: %pI4:%d proto: %d dest: %pI4:%d dest_dev: %s dest MAC: %pM\n",
					&flow, flow.smac, flow.indev->name, &flow.tuple.ipv4.src, flow.tuple.src_port, flow.tuple.protocol,
					&flow.tuple.ipv4.dst, flow.tuple.dst_port, flow.outdev->name, flow.dmac);

			memset(&resp, 0, sizeof(struct ecm_classifier_ovs_process_response));

			/*
			 * Call the external callback and get the result.
			 */
			result = cb(&flow, skb, &resp);

			dev_put(br_dev);

			/*
			 * Handle the result
			 */
			switch (result) {
			case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL:
			case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL:
				/*
				 * Allow accel after setting the external module response.
				 */
				DEBUG_WARN("%px: External callback process succeeded\n", ecvi);
				spin_lock_bh(&ecm_classifier_ovs_lock);
				ecvi->process_response.ingress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
				ecvi->process_response.ingress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
				if (resp.egress_vlan[0].h_vlan_TCI) {
					ecvi->process_response.ingress_vlan_tag[0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
				}

				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;
				if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
					if (resp.egress_vlan[1].h_vlan_TCI) {
						ecvi->process_response.ingress_vlan_tag[1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
					}

					ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
				}
				spin_unlock_bh(&ecm_classifier_ovs_lock);
				DEBUG_TRACE("%px: Multicast ingress vlan tag[0]: %x tag[1]: %x\n", ecvi, ecvi->process_response.ingress_vlan_tag[0],
						ecvi->process_response.ingress_vlan_tag[1]);
				break;

			case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL:
				/*
				 * External callback failed to process VLAN process. So, let's deny the acceleration
				 * and try more with the subsequent packets.
				 */
				DEBUG_WARN("%px: External callback failed to process VLAN tags\n", ecvi);
				spin_lock_bh(&ecm_classifier_ovs_lock);
				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL;
				ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				*process_response = ecvi->process_response;
				spin_unlock_bh(&ecm_classifier_ovs_lock);
				goto done;

			case ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL:
				/*
				 * There is no VLAN tag in the flow. Just allow the acceleration.
				 */
				DEBUG_WARN("%px: External callback didn't find any VLAN relation\n", ecvi);
				break;

			default:
				DEBUG_ASSERT(false, "%px: Unhandled result: %d\n", ci, result);
			}
		}
	}

	/*
	 * During multicast update path, skb is passed as NULL.
	 * We need to fill the ingress_vlan with the same vlan
	 * information that we have stored during connection instance
	 * creation. OVS manager needs this information to provide the
	 * right response.
	 */
	if (!skb) {
		flow.ingress_vlan = ecm_db_multicast_tuple_get_ovs_ingress_vlan(ci->ti);
	}

	/*
	 * Call the external callback and get the result.
	 */
	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
		ecm_classifier_ovs_result_t result = 0;

		if (!to_dev[i]) {
			continue;
		}

		flow.outdev = to_dev[i];

		/*
		 * For routed flows, indev should be the OVS bridge of the
		 * corresponding port.
		 */
		if (flow.is_routed) {
			struct net_device *outdev_master;

			/*
			 * During bridge+route scenario, the indev can be
			 * an ovs bridge port or and ovs bridge. if master device
			 * for indev and outdev are same then it a bridged traffic.
			 * Otherwise, set the indev to master of the egress port.
			 */
			outdev_master = ovsmgr_dev_get_master(flow.outdev);
			DEBUG_ASSERT(outdev_master, "%px: Expected a master\n", ci);
			if (indev_master && (indev_master->ifindex == outdev_master->ifindex)) {
				flow.indev = from_dev;
				ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, flow.smac);
			} else {
				flow.indev = outdev_master;
				ether_addr_copy(flow.smac, flow.indev->dev_addr);
			}
		}

		DEBUG_TRACE("%px: src MAC: %pM src_dev: %s src: %pI4:%d proto: %d dest: %pI4:%d dest_dev: %s dest MAC: %pM\n",
				ci, flow.smac, flow.indev->name, &flow.tuple.ipv4.src, flow.tuple.src_port, flow.tuple.protocol,
				&flow.tuple.ipv4.dst, flow.tuple.dst_port, flow.outdev->name, flow.dmac);

		memset(&resp, 0, sizeof(struct ecm_classifier_ovs_process_response));
		result = cb(&flow, skb, &resp);

		switch(result) {
		case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL_EGRESS:
			DEBUG_TRACE("%px: %s is not a valid OVS port\n", ci, to_dev[i]->name);
			ecm_db_multicast_connection_to_interfaces_clear_at_index(ci, i);
			break;

		case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL:
			/*
			 * If we receive "ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL"
			 * for any of the OVS port, then the connection should be
			 * deleted.
			 */
			DEBUG_TRACE("%px: flow does not exist for the OVS port: %s\n", ci, to_dev[i]->name);
			spin_lock_bh(&ecm_classifier_ovs_lock);
			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL;
			ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
			*process_response = ecvi->process_response;
			spin_unlock_bh(&ecm_classifier_ovs_lock);
			goto done;

		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL:
			DEBUG_TRACE("%px: Acceleration allowed for multicast OVS port: %s\n", ci, to_dev[i]->name);
			break;

		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL:
		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL:
			/*
			 * Allow accel after setting the external module response.
			 * Primary VLAN tag is always present even it is QinQ.
			 */
			DEBUG_WARN("%px: External callback process succeeded\n", ci);

			/*
			 * Initialize the VLAN entries since the ports can join/leave
			 * in-between and that will change the position of an existing port
			 * in the "egress_mc_vlan_tag" array.
			 */
			spin_lock_bh(&ecm_classifier_ovs_lock);
			ecvi->process_response.egress_mc_vlan_tag[i][0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
			ecvi->process_response.egress_mc_vlan_tag[i][1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;
			ecvi->process_response.egress_netdev_index[i] = flow.outdev->ifindex;

			/*
			 * If we have a valid TCI for ingress VLAN then we assume
			 * the flow is bridged. Because for routed flows, ingress VLAN
			 * will not be available.
			 */
			if (resp.ingress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.ingress_vlan_tag[0] = resp.ingress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[0].h_vlan_TCI;
			}

			if (resp.egress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.egress_mc_vlan_tag[i][0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
			}

			if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
				if (resp.ingress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.ingress_vlan_tag[1] = resp.ingress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[1].h_vlan_TCI;
				}

				if (resp.egress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.egress_mc_vlan_tag[i][1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
				}

				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
			}

			spin_unlock_bh(&ecm_classifier_ovs_lock);
			DEBUG_TRACE("%px: Multicast ingress vlan tag[0] : 0x%x tag[1] : 0x%x\n", ci, ecvi->process_response.ingress_vlan_tag[0],
					ecvi->process_response.ingress_vlan_tag[1]);
			DEBUG_TRACE("%px: Multicast egress vlan tag[%d][0] : 0x%x tag[%d][1] : 0x%x\n", ci, i, ecvi->process_response.egress_mc_vlan_tag[i][0],
					i, ecvi->process_response.egress_mc_vlan_tag[i][1]);
			break;

		default:
			DEBUG_ASSERT(false, "Unhandled result: %d\n", result);
		}
	}

	/*
	 * It is possible that after verifying each egress port with ovs manager,
	 * no egress ovs ports are allowed for acceleration.
	 *
	 * Deny the multicast connection as there are no active 'to' interface.
	 */
	if (!ecm_db_multicast_connection_to_interfaces_get_count(ci)) {
		DEBUG_TRACE("%px: No valid multicast 'to' interfaces found\n", ci);
		spin_lock_bh(&ecm_classifier_ovs_lock);
		ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL;
		ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
		*process_response = ecvi->process_response;
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		goto done;
	}

allow_accel:
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

done:
	if (from_dev)
		dev_put(from_dev);

	/*
	 * Deref the ovs net devices
	 */
	if (valid_ovs_ports) {
		for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
			if (to_dev[i]) {
				dev_put(to_dev[i]);
			}
		}
	}

	return;
}
#endif

/*
 * ecm_classifier_ovs_process_route_flow()
 *	Process routed flows.
 */
static void ecm_classifier_ovs_process_route_flow(struct ecm_classifier_ovs_instance *ecvi, struct ecm_db_connection_instance *ci,
							struct sk_buff *skb, struct net_device *from_dev, struct net_device *to_dev,
							struct ecm_classifier_process_response *process_response,
							ecm_classifier_ovs_process_callback_t cb)
{
	ecm_classifier_ovs_result_t result;
	struct ecm_classifier_ovs_process_response resp;
	struct ovsmgr_dp_flow flow;
	struct net_device *br_dev;
	ip_addr_t src_ip, dst_ip;

	memset(&flow, 0, sizeof(flow));

	flow.is_routed = true;
	flow.tuple.ip_version = ecm_db_connection_ip_version_get(ci);
	flow.tuple.protocol = ecm_db_connection_protocol_get(ci);

	/*
	 * For routed flows both from and to side can be OVS bridge port, if there
	 * is a routed flow between two OVS bridges. (e.g: ovs-br1 and ovs-br2)
	 *
	 * Connection Direction:
	 * 				PC1 -----> eth1-ovs-br1 (DUT) ovs-br2-eth2 -----> PC2
							from_dev	to_dev
	 *
	 * 1. SNAT/DNAT Enabled:	FROM				FROM_NAT	TO/TO_NAT
	 * 2. SNAT/DNAT Disabled:	FROM/FROM_NAT					TO/TO_NAT
	 *
	 * Connection Direction:
	 * 				PC1 <----- eth1-ovs-br1 (DUT) ovs-br2-eth2 <----- PC2
	 *						to_dev		from_dev
	 *
	 * 3. SNAT/DNAT Enabled:	TO				TO_NAT		FROM/FROM_NAT
	 * 4. SNAT/DNAT Disabled:	TO/TO_NAT					FROM/FROM_NAT
	 *
	 * 5. Tunnelled packet:  PC1 ------------> eth1---[ovs-br1]---gretap (DUT) eth0----->[gretap device]
	 * 	After the packet is encapsulated, the packet is routed and the
	 * 	flow is not relavant flow.
	 */
	if (from_dev) {
		/*
		 * Case 1/2
		 * from_dev = eth1
		 * br_dev = ovs-br1
		 *
		 * Case 3/4
		 * from_dev = eth2
		 * br_dev = ovs-br2
		 *
		 * case 5
		 * from_dev = greptap
		 * br_dev = NULL
		 */
		br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, false);
		if (!br_dev) {
			DEBUG_WARN("%px: from_dev = %s is a OVS bridge port, bridge interface is not found\n",
					ecvi, from_dev->name);
			goto route_not_relevant;
		}

		DEBUG_TRACE("%px: processing route flow from_dev = %s, br_dev = %s", ecvi, from_dev->name, br_dev->name);

		/*
		 * We always take the flow from bridge to port, so indev is brdige and outdev is port device.
		 */
		flow.indev = br_dev;
		flow.outdev = from_dev;

		flow.tuple.src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO_NAT));
		flow.tuple.dst_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, dst_ip);

		if (flow.tuple.ip_version == 4) {
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, src_ip);
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, dst_ip);
		} else if (flow.tuple.ip_version == 6) {
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, src_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, dst_ip);
		} else {
			DEBUG_ASSERT(NULL, "%px: unexpected ip_version: %d", ecvi, flow.tuple.ip_version );
		}

		ether_addr_copy(flow.smac, br_dev->dev_addr);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, flow.dmac);

		memset(&resp, 0, sizeof(struct ecm_classifier_ovs_process_response));

		/*
		 * Initialize the dscp with the default value.
		 */
		resp.dscp = OVSMGR_INVALID_DSCP;

		DEBUG_TRACE("%px: Route Flow Process (from): src MAC: %pM src_dev: %s src: %pI4:%d proto: %d dest: %pI4:%d dest_dev: %s dest MAC: %pM\n",
				&flow, flow.smac, flow.indev->name, &flow.tuple.ipv4.src, flow.tuple.src_port, flow.tuple.protocol,
				&flow.tuple.ipv4.dst, flow.tuple.dst_port, flow.outdev->name, flow.dmac);
		/*
		 * Call the external callback and get the result.
		 */
		result = cb(&flow, skb, &resp);

		dev_put(br_dev);

		if (resp.dscp != OVSMGR_INVALID_DSCP) {
			/*
			 * Copy DSCP value to the classifier's process response's flow_dscp field,
			 * because this is the from_dev and the direction of the flow is flow direction.
			 */
			spin_lock_bh(&ecm_classifier_ovs_lock);
			ecvi->process_response.flow_dscp = resp.dscp >> XT_DSCP_SHIFT;
			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
			spin_unlock_bh(&ecm_classifier_ovs_lock);
			DEBUG_TRACE("FLOW DSCP : 0x%x\n", ecvi->process_response.flow_dscp);
		}

		/*
		 * Handle the result
		 */
		switch (result) {
		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL:
		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL:
			/*
			 * Allow accel after setting the external module response.
			 */
			DEBUG_WARN("%px: External callback process succeeded\n", ecvi);

			spin_lock_bh(&ecm_classifier_ovs_lock);
			if (resp.egress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.ingress_vlan_tag[0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Ingress vlan tag[0] set : %x\n", ecvi->process_response.ingress_vlan_tag[0]);
			}

			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;

			if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
				if (resp.egress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.ingress_vlan_tag[1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Ingress vlan tag[0] set : %x\n", ecvi->process_response.ingress_vlan_tag[1]);
				}

				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
			}
			spin_unlock_bh(&ecm_classifier_ovs_lock);
			break;

		case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL:
			/*
			 * External callback failed to process VLAN process. So, let's deny the acceleration
			 * and try more with the subsequent packets.
			 */
			DEBUG_WARN("%px: External callback failed to process VLAN tags\n", ecvi);
			goto route_deny_accel;

		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL:

			/*
			 * There is no VLAN tag in the flow. Just allow the acceleration.
			 */
			DEBUG_WARN("%px: External callback didn't find any VLAN relation\n", ecvi);
			break;

		default:
			DEBUG_ASSERT(false, "Unhandled result: %d\n", result);
		}
	}

	if (to_dev) {
		/*
		 * Case 1/2
		 * from_dev = eth2
		 * br_dev = ovs-br2
		 *
		 * Case 3/4
		 * from_dev = eth1
		 * br_dev = ovs-br1
		 */
		br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_TO, false);
		if (!br_dev) {
			DEBUG_WARN("%px: to_dev = %s is a OVS bridge port, bridge interface is not found\n",
					ecvi, to_dev->name);
			goto route_deny_accel;
		}

		DEBUG_TRACE("%px: processing route flow to_dev = %s, br_dev = %s", ecvi, to_dev->name, br_dev->name);

		flow.indev = br_dev;
		flow.outdev = to_dev;

		flow.tuple.src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM_NAT));
		flow.tuple.dst_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

		if (flow.tuple.ip_version == 4) {
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, src_ip);
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, dst_ip);
		} else if (flow.tuple.ip_version == 6) {
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, src_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, dst_ip);
		} else {
			DEBUG_ASSERT(NULL, "%px: unexpected ip_version: %d", ecvi, flow.tuple.ip_version );
		}

		ether_addr_copy(flow.smac, br_dev->dev_addr);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, flow.dmac);

		memset(&resp, 0, sizeof(struct ecm_classifier_ovs_process_response));

		/*
		 * Initialize the dscp with the default value.
		 */
		resp.dscp = OVSMGR_INVALID_DSCP;

		DEBUG_TRACE("%px: Route Flow Process (to): src MAC: %pM src_dev: %s src: %pI4:%d proto: %d dest: %pI4:%d dest_dev: %s dest MAC: %pM\n",
				&flow, flow.smac, flow.indev->name, &flow.tuple.ipv4.src, flow.tuple.src_port, flow.tuple.protocol,
				&flow.tuple.ipv4.dst, flow.tuple.dst_port, flow.outdev->name, flow.dmac);
		/*
		 * Call the external callback and get the result.
		 */
		result = cb(&flow, skb, &resp);

		dev_put(br_dev);

		if (resp.dscp != OVSMGR_INVALID_DSCP) {
			/*
			 * Copy DSCP value to the classifier's process response's return_dscp field,
			 * because this is the to_dev and the direction of the flow is reply direction.
			 */
			spin_lock_bh(&ecm_classifier_ovs_lock);
			ecvi->process_response.return_dscp = resp.dscp >> XT_DSCP_SHIFT;
			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
			spin_unlock_bh(&ecm_classifier_ovs_lock);
			DEBUG_TRACE("RETURN DSCP : 0x%x\n", ecvi->process_response.return_dscp);
		}

		/*
		 * Handle the result
		 */
		switch (result) {
		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL:
		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL:
			/*
			 * Allow accel after setting the external module response.
			 */
			DEBUG_WARN("%px: External callback process succeeded\n", ecvi);

			spin_lock_bh(&ecm_classifier_ovs_lock);
			if (resp.egress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.egress_vlan_tag[0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Egress vlan tag[0] set : %x\n", ecvi->process_response.egress_vlan_tag[0]);
			}

			ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;

			if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
				if (resp.egress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.egress_vlan_tag[1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Ingress vlan tag[0] set : %x\n", ecvi->process_response.ingress_vlan_tag[1]);
				}

				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
			}
			spin_unlock_bh(&ecm_classifier_ovs_lock);
			break;

		case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL:
			/*
			 * External callback failed to process VLAN process. So, let's deny the acceleration
			 * and try more with the subsequent packets.
			 */
			DEBUG_WARN("%px: External callback failed to process VLAN tags\n", ecvi);
			goto route_deny_accel;

		case ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL:

			/*
			 * There is no VLAN tag in the flow. Just allow the acceleration.
			 */
			DEBUG_WARN("%px: External callback didn't find any VLAN relation\n", ecvi);
			break;

		default:
			DEBUG_ASSERT(false, "Unhandled result: %d\n", result);
		}
	}

	/*
	 * Acceleration is permitted
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	return;

route_deny_accel:
	/*
	 * ecm_classifier_ovs_lock MUST be held
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	ecvi->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	return;

route_not_relevant:
	/*
	 * ecm_classifier_ovs_lock MUST be held
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
}

/*
 * ecm_classifier_ovs_process()
 *	Process new packet
 *
 * NOTE: This function would only ever be called if all other classifiers have failed.
 */
static void ecm_classifier_ovs_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
									struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
									struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_ovs_instance *ecvi = (struct ecm_classifier_ovs_instance *)aci;
	ecm_classifier_ovs_result_t result = 0;
	struct ecm_db_connection_instance *ci;
	ecm_classifier_ovs_process_callback_t cb = NULL;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	struct ecm_classifier_ovs_process_response resp;
	struct ovsmgr_dp_flow flow;
	struct net_device *from_dev = NULL;
	struct net_device *to_dev = NULL;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: invalid state magic\n", ecvi);

	/*
	 * Not relevant to the connection if not enabled.
	 */
	if (unlikely(!ecm_classifier_ovs_enabled)) {
		/*
		 * Not relevant.
		 */
		DEBUG_WARN("%px: ovs classifier is not enabled\n", aci);
		goto not_relevant;
	}

	/*
	 * Get connection
	 */
	ci = ecm_db_connection_serial_find_and_ref(ecvi->ci_serial);
	if (!ci) {
		/*
		 * Connection has gone from under us
		 */
		DEBUG_WARN("%px: connection instance gone while processing classifier\n", aci);
		goto not_relevant;
	}

#ifdef ECM_MULTICAST_ENABLE
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	if (ecm_ip_addr_is_multicast(dst_ip)) {
		ecm_classifier_ovs_process_multicast(ci, skb, ecvi, process_response);
		ecm_db_connection_deref(ci);
		return;
	}
#endif

	/*
	 * Get the possible OVS bridge ports. If both are NULL, the classifier is not
	 * relevant to this connection.
	 */
	from_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, true);
	to_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_TO, true);
	if (!from_dev && !to_dev) {
		/*
		 * So, the classifier is not relevant to this connection.
		 */
		DEBUG_WARN("%px: None of the from/to interfaces are OVS bridge port\n", aci);
		ecm_db_connection_deref(ci);
		goto not_relevant;
	}

	/*
	 * Is there an external callback to get the ovs value from the packet?
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	cb = ovs.ovs_process;
	if (!cb) {
		/*
		 * Allow acceleration.
		 * Keep the classifier relevant to connection for stats update..
		 */
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		DEBUG_WARN("%px: No external process callback set\n", aci);
		if (from_dev)
			dev_put(from_dev);

		if (to_dev)
			dev_put(to_dev);

		spin_lock_bh(&ecm_classifier_ovs_lock);
		goto allow_accel;
	}
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	/*
	 * If the flow is a routed flow, set the is_routed flag of the flow.
	 */
	if (ecm_db_connection_is_routed_get(ci)) {
		ecm_classifier_ovs_process_route_flow(ecvi, ci, skb, from_dev, to_dev, process_response, cb);

		if (from_dev)
			dev_put(from_dev);

		if (to_dev)
			dev_put(to_dev);

		ecm_db_connection_deref(ci);
		return;
	}

	memset(&flow, 0, sizeof(struct ovsmgr_dp_flow));

	/*
	 * If the connection is a bridge flow, both devices should be present.
	 * TODO: This is an unexpected situation and should be assertion.
	 * We should also make sure that both devices are on the same OVS bridge.
	 */
	if (!from_dev || !to_dev) {
		DEBUG_ERROR("%px: One of the ports is NULL from_dev: %px to_dev: %px\n", aci, from_dev, to_dev);

		if (from_dev)
			dev_put(from_dev);

		if (to_dev)
			dev_put(to_dev);

		ecm_db_connection_deref(ci);
		goto not_relevant;
	}

	/*
	 * Flow is an OVS bridge flow.
	 */
	flow.tuple.ip_version = ecm_db_connection_ip_version_get(ci);
	flow.tuple.protocol = ecm_db_connection_protocol_get(ci);

	/*
	 * For the flow lookup, we need to use the  proper 5-tuple, in/out dev and
	 * src/dest MAC address. If the packet is coming from the destination side of the connection
	 * (e.g: ACK packets of the TCP connection) these values should be reversed.
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		DEBUG_TRACE("%px: sender is SRC\n", aci);
		flow.indev = from_dev;
		flow.outdev = to_dev;

		flow.tuple.src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		flow.tuple.dst_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, flow.smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, flow.dmac);
	} else {
		DEBUG_TRACE("%px: sender is DEST\n", aci);
		flow.indev = to_dev;
		flow.outdev = from_dev;

		flow.tuple.src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));
		flow.tuple.dst_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, dst_ip);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, flow.smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, flow.dmac);

	}

	if (flow.tuple.ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, dst_ip);
	} else if (flow.tuple.ip_version == 6) {
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, dst_ip);
	} else {
		DEBUG_ASSERT(NULL, "%px: unexpected ip_version: %d", aci, flow.tuple.ip_version );
	}

	memset(&resp, 0, sizeof(struct ecm_classifier_ovs_process_response));

	/*
	 * Set default values for flow and return DSCP.
	 * External module will call the DSCP query twice to
	 * find both directions' values.
	 */
	resp.flow_dscp = OVSMGR_INVALID_DSCP;
	resp.return_dscp = OVSMGR_INVALID_DSCP;

	/*
	 * Call the external callback and get the result.
	 */
	result = cb(&flow, skb, &resp);

	if (from_dev)
		dev_put(from_dev);

	if (to_dev)
		dev_put(to_dev);

	/*
	 * Handle the result
	 */
	switch (result) {
	case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL:
	case ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL:
		/*
		 * Allow accel after setting the external module response.
		 */
		DEBUG_WARN("%px: External callback process succeeded\n", aci);

		spin_lock_bh(&ecm_classifier_ovs_lock);
		ecvi->process_response.ingress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		ecvi->process_response.egress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		ecvi->process_response.ingress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		ecvi->process_response.egress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;

		/*
		 * Primary VLAN tag is always present even it is QinQ.
		 */
		ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;

		/*
		 * If the sender type is source, which means the packet is coming from the originator,
		 * we assign the ECM-ingress<>OVS-ingress, ECM-egress<>OVS-egress values.
		 */
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			if (resp.ingress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.ingress_vlan_tag[0] = resp.ingress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Ingress vlan tag[0] set : 0x%x\n", ecvi->process_response.ingress_vlan_tag[0]);
			}

			if (resp.egress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.egress_vlan_tag[0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Egress vlan tag[0] set : 0x%x\n", ecvi->process_response.egress_vlan_tag[0]);
			}

			if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
				if (resp.ingress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.ingress_vlan_tag[1] = resp.ingress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Ingress vlan tag[0] set : 0x%x\n", ecvi->process_response.ingress_vlan_tag[1]);
				}

				if (resp.egress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.egress_vlan_tag[1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Egress vlan tag[1] set : 0x%x\n", ecvi->process_response.egress_vlan_tag[1]);
				}

				/*
				 * QinQ tag is present. Let's pass this information to the frontend through the process action flag.
				 */
				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
			}
		} else {
			/*
			 * Sender type is destination, which means packet si coming from the counter originator side,
			 * we assign the ECM-ingress<>OVS-egress and ECM-egress<>OVS-ingress values.
			 */
			if (resp.ingress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.egress_vlan_tag[0] = resp.ingress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Egress vlan tag[0] set : 0x%x\n", ecvi->process_response.egress_vlan_tag[0]);
			}

			if (resp.egress_vlan[0].h_vlan_TCI) {
				ecvi->process_response.ingress_vlan_tag[0] = resp.egress_vlan[0].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[0].h_vlan_TCI;
				DEBUG_TRACE("Ingress vlan tag[0] set : 0x%x\n", ecvi->process_response.ingress_vlan_tag[0]);
			}

			if (result == ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL) {
				if (resp.ingress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.egress_vlan_tag[1] = resp.ingress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.ingress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Egress vlan tag[0] set : 0x%x\n", ecvi->process_response.egress_vlan_tag[1]);
				}

				if (resp.egress_vlan[1].h_vlan_TCI) {
					ecvi->process_response.ingress_vlan_tag[1] = resp.egress_vlan[1].h_vlan_encapsulated_proto << 16 | resp.egress_vlan[1].h_vlan_TCI;
					DEBUG_TRACE("Ingress vlan tag[1] set : 0x%x\n", ecvi->process_response.ingress_vlan_tag[1]);
				}

				/*
				 * QinQ tag is present. Let's pass this information to the frontend through the process action flag.
				 */
				ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;
			}
		}

		break;

	case ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL:
		/*
		 * External callback failed to process VLAN process. So, let's deny the acceleration
		 * and try more with the subsequent packets.
		 */
		DEBUG_WARN("%px: External callback failed to process VLAN tags\n", aci);
		goto deny_accel;

	case ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL:

		/*
		 * There is no VLAN tag in the flow. Just allow the acceleration.
		 */
		DEBUG_WARN("%px: External callback didn't find any VLAN relation\n", aci);
		spin_lock_bh(&ecm_classifier_ovs_lock);
		break;

	default:
		DEBUG_ASSERT(false, "Unhandled result: %d\n", result);
	}

	/*
	 * If external module set any of the flow or return DSCP,
	 * we copy them to the classifier's process response based
	 * on the direction of the traffic. The external module
	 * should always set first the flow and then the return values.
	 */
	if ((resp.flow_dscp != OVSMGR_INVALID_DSCP) || (resp.return_dscp != OVSMGR_INVALID_DSCP)) {
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			/*
			 * Copy DSCP values
			 */
			ecvi->process_response.flow_dscp = (resp.flow_dscp != OVSMGR_INVALID_DSCP) ? resp.flow_dscp >> XT_DSCP_SHIFT : 0;
			ecvi->process_response.return_dscp = (resp.return_dscp != OVSMGR_INVALID_DSCP) ? resp.return_dscp >> XT_DSCP_SHIFT : ecvi->process_response.flow_dscp;
			DEBUG_TRACE("FLOW DSCP: 0x%x RETURN DSCP: 0x%x\n",
					ecvi->process_response.flow_dscp, ecvi->process_response.return_dscp);
		} else {
			/*
			 * Copy DSCP values
			 */
			ecvi->process_response.flow_dscp = (resp.return_dscp != OVSMGR_INVALID_DSCP) ? resp.return_dscp >> XT_DSCP_SHIFT : 0;
			ecvi->process_response.return_dscp = (resp.flow_dscp != OVSMGR_INVALID_DSCP) ? resp.flow_dscp >> XT_DSCP_SHIFT : ecvi->process_response.flow_dscp;
			DEBUG_TRACE("FLOW DSCP: 0x%x RETURN DSCP: 0x%x\n",
					ecvi->process_response.flow_dscp, ecvi->process_response.return_dscp);
		}
		ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
	}

allow_accel:
	/*
	 * Acceleration is permitted
	 */
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	ecvi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;

	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
	ecm_db_connection_deref(ci);
	return;

not_relevant:
	/*
	 * ecm_classifier_ovs_lock MUST be held
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
	return;

deny_accel:
	/*
	 * ecm_classifier_ovs_lock MUST be held
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	ecvi->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	ecvi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_classifier_ovs_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_ovs_type_get(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)aci;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
	return ECM_CLASSIFIER_TYPE_OVS;
}

/*
 * ecm_classifier_ovs_reclassify_allowed()
 *	Get whether reclassification is allowed
 */
static bool ecm_classifier_ovs_reclassify_allowed(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)aci;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
	return true;
}

/*
 * ecm_classifier_ovs_reclassify()
 *	Reclassify
 */
static void ecm_classifier_ovs_reclassify(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)aci;
	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);

	/*
	 * Revert back to MAYBE relevant - we will evaluate when we get the next process() call.
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
}

/*
 * ecm_classifier_ovs_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_ovs_last_process_response_get(struct ecm_classifier_instance *aci,
	struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_ovs_instance *ecvi;
	ecvi = (struct ecm_classifier_ovs_instance *)aci;
	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);

	spin_lock_bh(&ecm_classifier_ovs_lock);
	*process_response = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
}

/*
 * ecm_classifier_ovs_stats_sync()
 *	Sync the stats of the OVS flow.
 */
static inline void ecm_classifier_ovs_stats_sync(struct ovsmgr_dp_flow *flow,
					  uint32_t pkts, uint32_t bytes,
					  struct net_device *indev, struct net_device *outdev,
					  uint8_t *smac, uint8_t *dmac,
					  ip_addr_t sip, ip_addr_t dip,
					  uint16_t sport, uint16_t dport, uint16_t tci, uint16_t tpid)
{
	struct ovsmgr_dp_flow_stats stats;

	flow->indev = indev;
	flow->outdev = outdev;

	ether_addr_copy(flow->smac, smac);
	ether_addr_copy(flow->dmac, dmac);

	/*
	 * Set default VLAN tci and vlan encapsulated proto.
	 */
	flow->ingress_vlan.h_vlan_TCI = 0;
	flow->ingress_vlan.h_vlan_encapsulated_proto = 0;

	if (tci) {
		flow->ingress_vlan.h_vlan_TCI = tci;
		flow->ingress_vlan.h_vlan_encapsulated_proto = tpid;
	}

	flow->tuple.src_port = sport;
	flow->tuple.dst_port = dport;

	if (flow->tuple.ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(flow->tuple.ipv4.src, sip);
		ECM_IP_ADDR_TO_NIN4_ADDR(flow->tuple.ipv4.dst, dip);
		DEBUG_TRACE("%px: STATS: src MAC: %pM src_dev: %s src: %pI4:%d proto: %d dest: %pI4:%d dest_dev: %s dest MAC: %pM TCI: %x, TPID: %x\n",
				flow, flow->smac, flow->indev->name, &flow->tuple.ipv4.src, flow->tuple.src_port, flow->tuple.protocol,
				&flow->tuple.ipv4.dst, flow->tuple.dst_port, flow->outdev->name, flow->dmac,
				flow->ingress_vlan.h_vlan_TCI, flow->ingress_vlan.h_vlan_encapsulated_proto);
	} else {
		ECM_IP_ADDR_TO_NIN6_ADDR(flow->tuple.ipv6.src, sip);
		ECM_IP_ADDR_TO_NIN6_ADDR(flow->tuple.ipv6.dst, dip);
		DEBUG_TRACE("%px: STATS: src MAC: %pM src_dev: %s src: %pI6:%d proto: %d dest: %pI6:%d dest_dev: %s dest MAC: %pM, TCI: %x, TPID: %x\n",
				flow, flow->smac, flow->indev->name, &flow->tuple.ipv6.src, flow->tuple.src_port, flow->tuple.protocol,
				&flow->tuple.ipv6.dst, flow->tuple.dst_port, flow->outdev->name, flow->dmac,
				flow->ingress_vlan.h_vlan_TCI, flow->ingress_vlan.h_vlan_encapsulated_proto);
	}

	/*
	 * Set the flow stats and update the flow database.
	 */
	stats.pkts = pkts;
	stats.bytes = bytes;

	ovsmgr_flow_stats_update(flow, &stats);
}

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_classifier_ovs_multicast_sync_to_stats()
 *	Common multicast sync_to function for IPv4 and IPv6.
 */
static void ecm_classifier_ovs_multicast_sync_to_stats(struct ecm_classifier_ovs_instance *ecvi,
					struct ecm_db_connection_instance *ci, struct ecm_classifier_rule_sync *sync)
{
	struct net_device *from_dev;
	struct net_device *to_ovs_port[ECM_DB_MULTICAST_IF_MAX];
	struct net_device *to_ovs_brdev[ECM_DB_MULTICAST_IF_MAX];
	struct net_device *br_dev;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	uint8_t smac[ETH_ALEN];
	uint8_t dmac[ETH_ALEN];
	uint16_t sport;
	uint16_t dport;
	struct ovsmgr_dp_flow flow;
	int if_cnt, i, valid_ifcnt = 0, ifindex;
	uint16_t tpid = 0, tci = 0;

	/*
	 * Get the possible OVS bridge ports.
	 */
	from_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, true);
	if_cnt = ecm_interface_multicast_ovs_to_interface_get_and_ref(ci, to_ovs_port, to_ovs_brdev);
	if (!from_dev && !if_cnt) {
		DEBUG_WARN("%px: None of the from/to interfaces is OVS bridge port\n", ci);
		return;
	}

	memset(&flow, 0, sizeof(flow));

	/*
	 * IP version and protocol are common for routed and bridge flows.
	 */
	flow.tuple.ip_version = ecm_db_connection_ip_version_get(ci);
	flow.tuple.protocol = ecm_db_connection_protocol_get(ci);
	flow.is_routed = ecm_db_connection_is_routed_get(ci);
	if (!flow.is_routed) {
		/* For multicast bridge flows, ovs creates a single rule with multiple
		 * egress interfaces. We need only one egress interface to sync the flow statistics.
		 *
		 * Sync the flow direction.
		 * eth1 to eth2
		 */
		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);

		if (ecvi->process_response.ingress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			tci = ecvi->process_response.ingress_vlan_tag[0] & 0xffff;
			tpid = (ecvi->process_response.ingress_vlan_tag[0] >> 16) & 0xffff;
			DEBUG_TRACE("%px: Ingress VLAN : %x:%x\n", ci, tci, tpid);
		}

		ecm_classifier_ovs_stats_sync(&flow,
				sync->rx_packet_count[ECM_CONN_DIR_FLOW], sync->rx_byte_count[ECM_CONN_DIR_FLOW],
				from_dev, to_ovs_port[0],
				smac, dmac,
				src_ip, dst_ip,
				sport, dport, tci, tpid);

		goto done;
	}

	/*
	 * The 'to' interfaces ports can be part of different OVS bridges.
	 * ovs-br1-(eth0, eth1, eth2)
	 * ovs-br2-(eth3)
	 * ovs-br3 (eth4, eth5)
	 *
	 * Find unique ovs port and ovs bridge combination, as for statistics
	 * update we need only one port from each OVS bridge.
	 *
	 * The logic will derive the below output for the
	 * above mentioned ovs 'to' interface list.
	 * to_ovs_port: eth0, eth3, eth4
	 * to_ovs_brdev: ovs-br1, ovs-br2, ovs-br3
	 *
	 */
	if (if_cnt) {
		ifindex = to_ovs_brdev[0]->ifindex;
		for (i = 1, valid_ifcnt = 1; i < if_cnt; i++) {
			if (ifindex == to_ovs_brdev[i]->ifindex) {
				dev_put(to_ovs_port[i]);
				continue;
			}

			to_ovs_port[valid_ifcnt] = to_ovs_port[i];
			to_ovs_brdev[valid_ifcnt] = to_ovs_brdev[i];
			valid_ifcnt++;
		}
	}

	/*
	 * For routed flows below configurations are possible.
	 *
	 * 1. From interface is OVS bridge port, to interface is
	 * another OVS bridge port
	 * PC1 -----> eth1-ovs-br1--->ovs-br2-eth2----->PC2
	 *
	 * 2. From interface is OVS bridge port, multiple to
	 * OVS bridge port (OVS master is same).
	 * PC1 -----> eth1-ovs-br1--->ovs-br2-eth2,eth3----->PC2
	 *
	 * 3. From interface is OVS bridge port, multiple to
	 * OVS bridge port (OVS masters are different).
	 * PC1 -----> eth1-ovs-br1--->ovs-br2-eth2/ovs-br3-eth3----->PC2
	 */
	if (from_dev) {
		/*
		 * from_dev = eth1
		 * br_dev = ovs-br1
		 */
		br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, false);
		if (!br_dev) {
			DEBUG_WARN("%px: from_dev = %s is a OVS bridge port, bridge interface is not found\n",
					ci, from_dev->name);
			goto done;
		}

		/*
		 * Sync the flow direction.
		 * eth1 to ovs-br1
		 */
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);

		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

		if (ecvi->process_response.ingress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			tci = ecvi->process_response.ingress_vlan_tag[0] & 0xffff;
			tpid = (ecvi->process_response.ingress_vlan_tag[0] >> 16) & 0xffff;
			DEBUG_TRACE("%px: Ingress VLAN : %x:%x\n", ci, tci, tpid);
		}

		ecm_classifier_ovs_stats_sync(&flow,
				sync->rx_packet_count[ECM_CONN_DIR_FLOW], sync->rx_byte_count[ECM_CONN_DIR_FLOW],
				from_dev, br_dev,
				smac, dmac,
				src_ip, dst_ip,
				sport, dport, tci, tpid);
		dev_put(br_dev);
	}

	/*
	 * For routed flow, to side can be multiple OVS bridge ports and
	 * the ports can be part of different OVS bridge master.
	 *
	 * 1. outdev = eth2
	 * indev = ovs-br2
	 * Sync the flow direction.
	 * ovs-br2 to eth2
	 *
	 * 2. outdev = eth3
	 * indev = ovs-br2
	 * Sync the flow direction.
	 * ovs-br2 to eth3
	 *
	 * 3. outdev = eth3
	 * indev = ovs-br3
	 * Sync the flow direction.
	 * ovs-br3 to eth3
	 *
	 */
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	for (i = 0; i < valid_ifcnt; i++) {
		ether_addr_copy(smac, to_ovs_brdev[i]->dev_addr);

		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);

		/*
		 * tci and tpid is 0 for pkts received from ovs bridge
		 */
		tci = tpid = 0;

		ecm_classifier_ovs_stats_sync(&flow,
				sync->rx_packet_count[ECM_CONN_DIR_FLOW], sync->rx_byte_count[ECM_CONN_DIR_FLOW],
				to_ovs_brdev[i], to_ovs_port[i],
				smac, dmac,
				src_ip, dst_ip,
				sport, dport, tci, tpid);
	}

done:
	if (from_dev)
		dev_put(from_dev);

	for (i = 0; i < valid_ifcnt; i++) {
		dev_put(to_ovs_port[i]);
	}
}
#endif

/*
 * ecm_classifier_ovs_sync_to_stats()
 *	Common sync_to function for IPv4 and IPv6.
 */
static void ecm_classifier_ovs_sync_to_stats(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	struct ecm_db_connection_instance *ci;
	struct ovsmgr_dp_flow flow;
	struct net_device *from_dev;
	struct net_device *to_dev;
	struct net_device *br_dev;
	uint8_t smac[ETH_ALEN];
	uint8_t dmac[ETH_ALEN];
	uint16_t sport;
	uint16_t dport;
	uint16_t tpid = 0, tci = 0;

	struct ecm_classifier_ovs_instance *ecvi = (struct ecm_classifier_ovs_instance *)aci;

	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);

	ci = ecm_db_connection_serial_find_and_ref(ecvi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", ecvi, ecvi->ci_serial);
		return;
	}

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Check for multicast connection.
	 */
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	if (ecm_ip_addr_is_multicast(dst_ip)) {
		ecm_classifier_ovs_multicast_sync_to_stats(ecvi, ci, sync);
		ecm_db_connection_deref(ci);
		return;
	}
#endif
	memset(&flow, 0, sizeof(flow));

	/*
	 * Get the possible OVS bridge ports.
	 */
	from_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, true);
	to_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_TO, true);

	/*
	 * IP version and protocol are common for routed and bridge flows.
	 */
	flow.tuple.ip_version = ecm_db_connection_ip_version_get(ci);
	flow.tuple.protocol = ecm_db_connection_protocol_get(ci);
	flow.is_routed = ecm_db_connection_is_routed_get(ci);

	/*
	 * Get the tci and tpid values of the ingress side of the flow.
	 */
	if (ecvi->process_response.ingress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
		tci = ecvi->process_response.ingress_vlan_tag[0] & 0xffff;
		tpid = (ecvi->process_response.ingress_vlan_tag[0] >> 16) & 0xffff;
		DEBUG_TRACE("%px: Ingress VLAN : %x:%x\n", aci, tci, tpid);
	}

	/*
	 * Bridge flow
	 */
	if (!flow.is_routed) {
		/*
		 * Sync the flow direction (eth1 to eth2)
		 *
		 * ECM depends on netdev private flags to identify
		 * if it is an OVS port, if the port is removed from
		 * bridge while traffic is running then the device is
		 * not part of bridge.  Do not update statistics if ports
		 * are removed from bridge.
		 */
		if (!from_dev || !to_dev) {
			ecm_db_connection_deref(ci);
			return;
		}

		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);

		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

		DEBUG_TRACE("%px: Flow direction stats update\n", aci);
		ecm_classifier_ovs_stats_sync(&flow,
				  sync->rx_packet_count[ECM_CONN_DIR_FLOW], sync->rx_byte_count[ECM_CONN_DIR_FLOW],
				  from_dev, to_dev,
				  smac, dmac,
				  src_ip, dst_ip,
				  sport, dport, tci, tpid);

		/*
		 * Sync the return direction (eth2 to eth1)
		 * All the flow parameters are reversed.
		 */
		DEBUG_TRACE("%px: Return direction stats update\n", aci);

		/*
		 * Reset the tci and tpid values and get the egress side of the flow.
		 */
		tci = tpid = 0;
		if (ecvi->process_response.egress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			tci = ecvi->process_response.egress_vlan_tag[0] & 0xffff;
			tpid = (ecvi->process_response.egress_vlan_tag[0] >> 16) & 0xffff;
			DEBUG_TRACE("%px: Egress VLAN : %x:%x\n", aci, tci, tpid);
		}

		ecm_classifier_ovs_stats_sync(&flow,
				  sync->rx_packet_count[ECM_CONN_DIR_RETURN], sync->rx_byte_count[ECM_CONN_DIR_RETURN],
				  to_dev, from_dev,
				  dmac, smac,
				  dst_ip, src_ip,
				  dport, sport, tci, tpid);
		goto done;
	}

	/*
	 * For routed flows both from and to side can be OVS bridge port, if there
	 * is a routed flow between two OVS bridges. (e.g: ovs-br1 and ovs-br2)
	 *
	 * These are the netdevice places and the IP address values based on the
	 * 4 different NAT cases.
	 *
	 * SNAT:
	 * PC1 -----> eth1-ovs-br1--->ovs-br2-eth2 -----> PC2
	 *		(from_dev)	(to_dev)
	 * src_ip			src_ip_nat	dest_ip/dest_ip_nat
	 *
	 * DNAT
	 * PC1 <----- eth1-ovs-br1<---ovs-br2-eth2 <----- PC2
	 *		(to_dev)	(from_dev)
	 * dest_ip			dest_ip_nat	src_ip/src_ip_nat
	 *
	 * Non-NAT - Egress
	 * PC1 -----> eth1-ovs-br1--->ovs-br2-eth2 -----> PC2
	 *		(from_dev)	(to_dev)
	 * src_ip/src_ip_nat				dest_ip/dest_ip_nat
	 *
	 * Non-NAT - Ingress
	 * PC1 <----- eth1-ovs-br1<---ovs-br2-eth2 <----- PC2
	 *		(to_dev)	(from_dev)
	 * dest_ip/dest_ip_nat				src_ip/src_ip_nat
	 */
	if (from_dev) {
		/*
		 * from_dev = eth1/eth2  (can be tagged)
		 * br_dev = ovs-br1/ovs_br2 (untagged)
		 */
		br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_FROM, false);
		if (!br_dev) {
			DEBUG_WARN("%px: from_dev = %s is a OVS bridge port, bridge interface is not found\n",
					aci, from_dev->name);
			goto done;
		}

		/*
		 * Sync the flow direction (eth1/eth2 to ovs-br1/ovs_br2) based on the NAT case.
		 */
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ether_addr_copy(dmac, br_dev->dev_addr);

		/*
		 * If from_dev is a bridge port, dest_ip_nat and dest_port_nat satisfies all the NAT cases.
		 * So, we need to get the ECM_DB_OBJ_DIR_TO_NAT direction's IP and port number from the connection.
		 */
		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO_NAT));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, dst_ip);

		ecm_classifier_ovs_stats_sync(&flow,
				  sync->rx_packet_count[ECM_CONN_DIR_FLOW], sync->rx_byte_count[ECM_CONN_DIR_FLOW],
				  from_dev, br_dev,
				  smac, dmac,
				  src_ip, dst_ip,
				  sport, dport, tci, tpid);

		/*
		 * Sync the return direction (ovs-br1/ovs_br2 to eth1/eth2) based on the NAT case.
		 * All the flow parameters are reversed.
		 */
		ecm_classifier_ovs_stats_sync(&flow,
				  sync->tx_packet_count[ECM_CONN_DIR_FLOW], sync->tx_byte_count[ECM_CONN_DIR_FLOW],
				  br_dev, from_dev,
				  dmac, smac,
				  dst_ip, src_ip,
				  dport, sport, 0, 0);
		dev_put(br_dev);
	}

	if (to_dev) {
		/*
		 * to_dev = eth2/eth1 (can be tagged)
		 * br_dev = ovs-br2/ovs-br1 (untagged)
		 */
		br_dev = ecm_classifier_ovs_interface_get_and_ref(ci, ECM_DB_OBJ_DIR_TO, false);
		if (!br_dev) {
			DEBUG_WARN("%px: to_dev = %s is a OVS bridge port, bridge interface is not found\n",
					aci, to_dev->name);
			goto done;
		}

		/*
		 * Reset the tci and tpid values and get the egress side of the flow.
		 */
		tci = tpid = 0;
		if (ecvi->process_response.egress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			tci = ecvi->process_response.egress_vlan_tag[0] & 0xffff;
			tpid = (ecvi->process_response.egress_vlan_tag[0] >> 16) & 0xffff;
			DEBUG_TRACE("%px: Egress VLAN : %x:%x\n", aci, tci, tpid);
		}

		/*
		 * Sync the flow direction (ovs-br2/ovs_br1 to eth2/eth1) based on the NAT case.
		 */
		ether_addr_copy(smac, br_dev->dev_addr);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);

		/*
		 * If to_dev is a bridge port, src_ip_nat and src_port_nat satisfies all the NAT cases.
		 * So, we need to get the ECM_DB_OBJ_DIR_FROM_NAT direction's IP and port number from the connection.
		 */
		sport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM_NAT));
		dport = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

		ecm_classifier_ovs_stats_sync(&flow,
				  sync->tx_packet_count[ECM_CONN_DIR_RETURN], sync->tx_byte_count[ECM_CONN_DIR_RETURN],
				  br_dev, to_dev,
				  smac, dmac,
				  src_ip, dst_ip,
				  sport, dport, 0, 0);
		/*
		 * Sync the return direction (eth2/eth1 to ovs-br2/ovs-br1) based on the NAT case.
		 * All the flow parameters are reversed.
		 */
		ecm_classifier_ovs_stats_sync(&flow,
				  sync->rx_packet_count[ECM_CONN_DIR_RETURN], sync->rx_byte_count[ECM_CONN_DIR_RETURN],
				  to_dev, br_dev,
				  dmac, smac,
				  dst_ip, src_ip,
				  dport, sport, tci, tpid);
		dev_put(br_dev);
	}

done:
	ecm_db_connection_deref(ci);

	if (from_dev)
		dev_put(from_dev);
	if (to_dev)
		dev_put(to_dev);
}

/*
 * ecm_classifier_ovs_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_ovs_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	/*
	 * Nothing to update.
	 * We only care about flows that are actively being accelerated.
	 */
	if (!(sync->tx_packet_count[ECM_CONN_DIR_FLOW] || sync->tx_packet_count[ECM_CONN_DIR_RETURN])) {
		return;
	}

	/*
	 * Handle only the stats sync. We don't care about the evict or flush syncs.
	 */
	if (sync->reason != ECM_FRONT_END_IPV4_RULE_SYNC_REASON_STATS) {
		return;
	}

	/*
	 * Common sync_to function call.
	 */
	ecm_classifier_ovs_sync_to_stats(aci, sync);
}

/*
 * ecm_classifier_ovs_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_ovs_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_ovs_instance *ecvi __attribute__((unused));

	ecvi = (struct ecm_classifier_ovs_instance *)aci;
	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
}

/*
 * ecm_classifier_ovs_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_ovs_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	/*
	 * Nothing to update.
	 * We only care about flows that are actively being accelerated.
	 */
	if (!(sync->tx_packet_count[ECM_CONN_DIR_FLOW] || sync->tx_packet_count[ECM_CONN_DIR_RETURN])) {
		return;
	}

	/*
	 * Handle only the stats sync. We don't care about the evict or flush syncs.
	 */
	if (sync->reason != ECM_FRONT_END_IPV6_RULE_SYNC_REASON_STATS) {
		return;
	}

	/*
	 * Common sync_to function call.
	 */
	ecm_classifier_ovs_sync_to_stats(aci, sync);
}

/*
 * ecm_classifier_ovs_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_ovs_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_ovs_instance *ecvi __attribute__((unused));

	ecvi = (struct ecm_classifier_ovs_instance *)aci;
	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_ovs_state_get()
 *	Gets the state of this classfier and outputs it to debugfs.
 */
static int ecm_classifier_ovs_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_ovs_instance *ecvi;
	struct ecm_classifier_process_response pr;

	ecvi = (struct ecm_classifier_ovs_instance *)ci;
	DEBUG_CHECK_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC, "%px: magic failed", ecvi);

	if ((result = ecm_state_prefix_add(sfi, "ovs"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_ovs_lock);
	pr = ecvi->process_response;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &pr))) {
		goto done;
	}


	if (pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
#ifdef ECM_MULTICAST_ENABLE
		int i;
#endif
		/*
		 * TODO: Clean up the function later to print classifier
		 * specific data in each classifier’s state_get function.
		 */
		if (pr.ingress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			if ((result = ecm_state_write(sfi, "ingress_vlan_tag[0]", "0x%x", pr.ingress_vlan_tag[0]))) {
				goto done;
			}
		}

		if (pr.ingress_vlan_tag[1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			if ((result = ecm_state_write(sfi, "ingress_vlan_tag[1]", "0x%x", pr.ingress_vlan_tag[1]))) {
				goto done;
			}
		}

		if (pr.egress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			if ((result = ecm_state_write(sfi, "egress_vlan_tag[0]", "0x%x", pr.egress_vlan_tag[0]))) {
				goto done;
			}
		}

		if (pr.egress_vlan_tag[1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			if ((result = ecm_state_write(sfi, "egress_vlan_tag[1]", "0x%x", pr.egress_vlan_tag[1]))) {
				goto done;
			}
		}

#ifdef ECM_MULTICAST_ENABLE
		for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
			struct net_device *dev;

			if (pr.egress_mc_vlan_tag[i][0] == ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
				continue;
			}

			dev = dev_get_by_index(&init_net, pr.egress_netdev_index[i]);
			if (dev) {
				if ((result = ecm_state_write(sfi, "port_egress", "%s", dev->name))) {
					dev_put(dev);
					goto done;
				}
				dev_put(dev);
			}

			if ((result = ecm_state_write(sfi, "port_egress_vlan_tag[0]", "0x%x", pr.egress_mc_vlan_tag[i][0]))) {
				goto done;
			}

			if (pr.egress_mc_vlan_tag[i][1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
				if ((result = ecm_state_write(sfi, "port_egress_vlan_tag[1]", "0x%x", pr.egress_mc_vlan_tag[i][1]))) {
					goto done;
				}
			}
		}
#endif
	}

done:
	ecm_state_prefix_remove(sfi);
	return result;
}
#endif

/*
 * ecm_classifier_ovs_instance_alloc()
 *	Allocate an instance of the ovs classifier
 */
struct ecm_classifier_ovs_instance *ecm_classifier_ovs_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_ovs_instance *ecvi;
#ifdef ECM_MULTICAST_ENABLE
	int i;
#endif

	/*
	 * Allocate the instance
	 */
	ecvi = (struct ecm_classifier_ovs_instance *)kzalloc(sizeof(struct ecm_classifier_ovs_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ecvi) {
		DEBUG_WARN("i%px: Failed to allocate ovs Classifier instance\n", ci);
		return NULL;
	}

	DEBUG_SET_MAGIC(ecvi, ECM_CLASSIFIER_OVS_INSTANCE_MAGIC);
	ecvi->refs = 1;

	/*
	 * Methods generic to all classifiers.
	 */
	ecvi->base.process = ecm_classifier_ovs_process;
	ecvi->base.sync_from_v4 = ecm_classifier_ovs_sync_from_v4;
	ecvi->base.sync_to_v4 = ecm_classifier_ovs_sync_to_v4;
	ecvi->base.sync_from_v6 = ecm_classifier_ovs_sync_from_v6;
	ecvi->base.sync_to_v6 = ecm_classifier_ovs_sync_to_v6;
	ecvi->base.type_get = ecm_classifier_ovs_type_get;
	ecvi->base.reclassify_allowed = ecm_classifier_ovs_reclassify_allowed;
	ecvi->base.reclassify = ecm_classifier_ovs_reclassify;
	ecvi->base.last_process_response_get = ecm_classifier_ovs_last_process_response_get;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ecvi->base.state_get = ecm_classifier_ovs_state_get;
#endif
	ecvi->base.ref = ecm_classifier_ovs_ref;
	ecvi->base.deref = ecm_classifier_ovs_deref;
	ecvi->ci_serial = ecm_db_connection_serial_get(ci);

	ecvi->process_response.process_actions = 0;
	ecvi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	ecvi->process_response.ingress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	ecvi->process_response.egress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	ecvi->process_response.ingress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	ecvi->process_response.egress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;

#ifdef ECM_MULTICAST_ENABLE
	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
		ecvi->process_response.egress_mc_vlan_tag[i][0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		ecvi->process_response.egress_mc_vlan_tag[i][1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	}
#endif

	/*
	 * Final check if we are pending termination
	 */
	spin_lock_bh(&ecm_classifier_ovs_lock);
	if (ecm_classifier_ovs_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		DEBUG_WARN("%px: Terminating\n", ci);
		kfree(ecvi);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	ecvi->next = ecm_classifier_ovs_instances;
	if (ecm_classifier_ovs_instances) {
		ecm_classifier_ovs_instances->prev = ecvi;
	}
	ecm_classifier_ovs_instances = ecvi;

	/*
	 * Increment stats
	 */
	ecm_classifier_ovs_count++;
	DEBUG_ASSERT(ecm_classifier_ovs_count > 0, "%px: ecm_classifier_ovs_count wrap for instance: %px\n", ci, ecvi);
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	DEBUG_INFO("%px: ovs classifier instance alloc: %px\n", ci, ecvi);
	return ecvi;
}
EXPORT_SYMBOL(ecm_classifier_ovs_instance_alloc);

/*
 * ecm_classifier_ovs_register_callbacks()
 */
int ecm_classifier_ovs_register_callbacks(struct ecm_classifier_ovs_callbacks *ovs_cbs)
{
	spin_lock_bh(&ecm_classifier_ovs_lock);

	if (unlikely(!ecm_classifier_ovs_enabled)) {
		spin_unlock_bh(&ecm_classifier_ovs_lock);
		return -1;
	}

	ovs.ovs_process = ovs_cbs->ovs_process;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_ovs_register_callbacks);

/*
 * ecm_classifier_ovs_unregister_callbacks()
 */
void ecm_classifier_ovs_unregister_callbacks(void)
{
	spin_lock_bh(&ecm_classifier_ovs_lock);
	ovs.ovs_process = NULL;
	spin_unlock_bh(&ecm_classifier_ovs_lock);
}
EXPORT_SYMBOL(ecm_classifier_ovs_unregister_callbacks);

/*
 * ecm_classifier_ovs_init()
 */
int ecm_classifier_ovs_init(struct dentry *dentry)
{
	DEBUG_INFO("ovs classifier Module init\n");

	ecm_classifier_ovs_dentry = debugfs_create_dir("ecm_classifier_ovs", dentry);
	if (!ecm_classifier_ovs_dentry) {
		DEBUG_ERROR("Failed to create ecm ovs directory in debugfs\n");
		return -1;
	}

	if (!ecm_debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_ovs_dentry,
					(u32 *)&ecm_classifier_ovs_enabled)) {
		DEBUG_ERROR("Failed to create ovs enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_ovs_dentry);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_ovs_init);

/*
 * ecm_classifier_ovs_exit()
 */
void ecm_classifier_ovs_exit(void)
{
	DEBUG_INFO("ovs classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_ovs_lock);
	ecm_classifier_ovs_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_ovs_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_ovs_dentry) {
		debugfs_remove_recursive(ecm_classifier_ovs_dentry);
	}

}
EXPORT_SYMBOL(ecm_classifier_ovs_exit);
