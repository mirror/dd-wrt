/*
 **************************************************************************
 * Copyright (c) 2014-2019 The Linux Foundation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/pkt_sched.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/mroute.h>
#include <linux/vmalloc.h>

#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <net/arp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#ifdef ECM_INTERFACE_VLAN_ENABLE
#include <linux/../../net/8021q/vlan.h>
#include <linux/if_vlan.h>
#endif

/*
 * General operational control
 */
int ecm_front_end_ipv4_mc_stopped = 0;	/* When non-zero further traffic will not be processed */

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_NSS_MULTICAST_IPV4_DEBUG_LEVEL

#include <nss_api_if.h>
#include <mc_ecm.h>

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
#include "ecm_classifier_default.h"
#include "ecm_interface.h"
#include "ecm_nss_ipv4.h"
#include "ecm_nss_multicast_ipv4.h"
#include "ecm_nss_common.h"
#include "ecm_front_end_common.h"

/*
 * Magic numbers
 */
#define ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC 0xED12

/*
 * struct ecm_nss_ipv4_multicast_connection_instance
 *	A connection specific front end instance for MULTICAST connections
 */
struct ecm_nss_multicast_ipv4_connection_instance {
	struct ecm_front_end_connection_instance base;		/* Base class */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

static int ecm_nss_multicast_ipv4_accelerated_count = 0;
						/* Array of Number of TCP and UDP connections currently offloaded */

/*
 * ecm_nss_multicast_connection_to_interface_heirarchy_construct()
 * 	Create destination interface heirarchy for a routed/bridge multicast connection
 */
static int32_t ecm_nss_multicast_connection_to_interface_heirarchy_construct(struct ecm_front_end_connection_instance *feci,
								      struct ecm_db_iface_instance *interfaces, ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr,
								      struct net_device *in_dev, struct net_device *brdev, uint8_t max_if, uint32_t *dst_dev,
								      uint32_t *to_list_first, uint8_t *src_node_addr, bool is_routed,
								      __be16 *layer4hdr, struct sk_buff *skb)
{
	int interface_instance_cnt;
	if (is_routed) {
		interface_instance_cnt = ecm_interface_multicast_heirarchy_construct_routed(feci, interfaces, in_dev, ip_src_addr, ip_dest_addr, max_if, dst_dev, to_list_first, false, layer4hdr, skb);
	} else {
		interface_instance_cnt = ecm_interface_multicast_heirarchy_construct_bridged(feci, interfaces, brdev, ip_src_addr, ip_dest_addr, max_if, dst_dev, to_list_first, src_node_addr, layer4hdr, skb);
	}

	return interface_instance_cnt;
}

/*
 * ecm_nss_multicast_ipv4_connection_update_callback()
 * 	Callback for handling Ack/Nack for update accelerate rules
 */
static void ecm_nss_multicast_ipv4_connection_update_callback(void *app_data, struct nss_ipv4_msg *nim)
{
	struct nss_ipv4_mc_rule_create_msg *nircm = &nim->msg.mc_rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_nss_multicast_ipv4_connection_instance *nmci;

	/*TODO: If the response is NACK then decelerate the flow and flushes all rules */
	DEBUG_TRACE("%p: update callback, response received from FW : %u\n", nim, nim->cm.response);

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != NSS_IPV4_TX_CREATE_MC_RULE_MSG) {
		DEBUG_ERROR("%p: multicast update callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Is this a response to a update rule message?
	 */
	if ( !(nircm->rule_flags & NSS_IPV4_MC_RULE_CREATE_FLAG_MC_UPDATE)) {
		DEBUG_ERROR("%p: multicast update callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%p: multicast update callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: Update accelerate response for connection: %p, serial: %u\n", nmci, feci->ci, serial);
	DEBUG_TRACE("%p: valid_flags: %x\n", nmci, nircm->valid_flags);
	DEBUG_TRACE("%p: flow_ip: %pI4h:%d\n", nmci, &nircm->tuple.flow_ip, nircm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4h:%d\n", nmci, &nircm->tuple.return_ip, nircm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", nmci, nircm->tuple.protocol);

	/*
	 * Release the connection.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
	return;
}

/*
 * ecm_nss_multicast_ipv4_connection_create_callback()
 * 	callback for handling create ack/nack calls for multicast create commands.
 */
static void ecm_nss_multicast_ipv4_connection_create_callback(void *app_data, struct nss_ipv4_msg *nim)
{
	struct nss_ipv4_mc_rule_create_msg *__attribute__((unused))nircm = &nim->msg.mc_rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_nss_multicast_ipv4_connection_instance *nmci;

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != NSS_IPV4_TX_CREATE_MC_RULE_MSG) {
		DEBUG_ERROR("%p: udp create callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%p: create callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: accelerate response for connection: %p, serial: %u\n", nmci, feci->ci, serial);
	DEBUG_TRACE("%p: valid_flags: %x\n", nmci, nircm->valid_flags);
	DEBUG_TRACE("%p: flow_ip: %pI4h:%d\n", nmci, &nircm->tuple.flow_ip, nircm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4h:%d\n", nmci, &nircm->tuple.return_ip, nircm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", nmci, nircm->tuple.protocol);

	/*
	 * Handle the creation result code.
	 */
	DEBUG_TRACE("%p: response: %d\n", nmci, nim->cm.response);
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		/*
		 * Creation command failed (specific reason ignored).
		 */
		DEBUG_TRACE("%p: accel nack: %d\n", nmci, nim->cm.error);
		spin_lock_bh(&feci->lock);
		DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Unexpected mode: %d\n", ci, feci->accel_mode);
		nmci->base.stats.ae_nack++;
		nmci->base.stats.ae_nack_total++;
		if (nmci->base.stats.ae_nack >= nmci->base.stats.ae_nack_limit) {
			/*
			 * Too many NSS rejections
			 */
			feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_ACCEL_ENGINE;
		} else {
			/*
			 * Revert to decelerated
			 */
			feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
		}

		/*
		 * Clear any decelerate pending flag since we aren't accelerated anyway we can just clear this whether it is set or not
		 */
		nmci->base.stats.decelerate_pending = false;

		/*
		 * If connection is now defunct then set mode to ensure no further accel attempts occur
		 */
		if (feci->is_defunct) {
			feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT;
		}
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Unexpected mode: %d\n", ci, feci->accel_mode);

	/*
	 * If a flush occured before we got the ACK then our acceleration was effectively cancelled on us
	 * GGG TODO This is a workaround for a NSS message OOO quirk, this should eventually be removed.
	 */
	if (nmci->base.stats.flush_happened) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
		nmci->base.stats.flush_happened = false;

		/*
		 * We are decelerated, clear any pending flag as that is meaningless now.
		 */
		nmci->base.stats.decelerate_pending = false;

		/*
		 * Increement the no-action counter.  Our connectin was decelerated on us with no action occurring.
		 */
		nmci->base.stats.no_action_seen++;
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	/*
	 * We got an ACK - we are accelerated.
	 */
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_ACCEL;

	/*
	 * Create succeeded, declare that we are accelerated.
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_multicast_ipv4_accelerated_count++;	/* Protocol specific counter */
	ecm_nss_ipv4_accelerated_count++;		/* General running counter */
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	/*
	 * Clear any nack count
	 */
	nmci->base.stats.ae_nack = 0;

	/*
	 * Decelerate may have been attempted while we were accel pending.
	 * If decelerate is pending then we need to begin deceleration :-(
	 */
	if (!nmci->base.stats.decelerate_pending) {
		/*
		 * Increement the no-action counter, this is reset if offload action is seen
		 */
		nmci->base.stats.no_action_seen++;

		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_INFO("%p: Decelerate was pending\n", ci);
	nmci->base.stats.decelerate_pending = false;
	spin_unlock_bh(&feci->lock);

	feci->decelerate(feci);

	/*
	 * Release the connection.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_nss_multicast_ipv4_connection_update_accelerate()
 * 	Push destination interface list updates for a multicast connection
 * 	to NSS
 *
 * 	This function receives a list of interfaces that have either left or joined the connection,
 * 	and sends a 'multicast update' command to NSS to inform about these interface state changes.
 */
static int ecm_nss_multicast_ipv4_connection_update_accelerate(struct ecm_front_end_connection_instance *feci,
							       struct ecm_multicast_if_update *rp)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	uint16_t regen_occurrances;
	struct ecm_db_iface_instance *to_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_nss_iface;
	struct nss_ipv4_mc_rule_create_msg *create;
	struct nss_ipv4_msg *nim;
	ip_addr_t addr;
	int32_t *to_ifaces_first;
	int32_t *to_ii_first;
	int32_t vif;
	int32_t ret;
	int32_t valid_vif_idx = 0;
	int32_t from_ifaces_first;
	int32_t to_nss_iface_id = 0;
	int32_t from_nss_iface_id = 0;
	uint8_t to_nss_iface_address[ETH_ALEN];
	nss_tx_status_t nss_tx_status;
	int32_t list_index;
	int32_t to_mtu = 0;
	int from_iface_bridge_identifier = 0;
	int to_iface_bridge_identifier = 0;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	uint8_t dest_mac[ETH_ALEN];

	DEBUG_INFO("%p: Accel conn: %p\n", nmci, feci->ci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	nim = (struct nss_ipv4_msg *)kzalloc(sizeof(struct nss_ipv4_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		return -1;
	}

	nss_ipv4_msg_init(nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_MC_RULE_MSG,
			sizeof(struct nss_ipv4_mc_rule_create_msg),
			ecm_nss_multicast_ipv4_connection_update_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	create = &nim->msg.mc_rule_create;

	/*
	 * Construct an accel command.
	 */
	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(feci->ci, &to_ifaces, &to_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in to_interfaces list!\n", nmci);
		kfree(nim);
		return -1;
	}

	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in from_interfaces list!\n", nmci);
		ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
		kfree(nim);
		return -1;
	}

	create->ingress_vlan_tag[0] =   ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	create->ingress_vlan_tag[1] =   ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Set the source NSS interface identifier
	 */
	from_nss_iface = from_ifaces[from_ifaces_first];
	from_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(from_nss_iface);
	if (from_nss_iface_id < 0) {
                DEBUG_TRACE("%p: from_nss_iface_id: %d\n", nmci, from_nss_iface_id);
		spin_lock_bh(&feci->lock);
		if (feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL) {
			feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_NO_ACTION;
		}
		spin_unlock_bh(&feci->lock);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
		kfree(nim);
		return -1;
        }

	create->src_interface_num = from_nss_iface_id;
	from_nss_iface = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	from_iface_bridge_identifier = ecm_db_iface_interface_identifier_get(from_nss_iface);
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination interface
	 * information
	 */
	DEBUG_TRACE("%p: Examine to/dest heirarchy list\n", nmci);
	rule_invalid = false;

	/*
	 * Loop through the list of interface updates
	 */
	for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
#ifdef ECM_INTERFACE_VLAN_ENABLE
		create->if_rule[vif].egress_vlan_tag[0] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
		create->if_rule[vif].egress_vlan_tag[1] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
#endif

		/*
		 * If there is no state change for an interface at this index,
		 * then ignore
		 */
		if (!(rp->if_join_idx[vif] || rp->if_leave_idx[vif])) {
			continue;
		}

		ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, vif);

		/*
		 * We have an update for this interface. Construct the interface information
		 */
		to_nss_iface_id = -1;
		memset(interface_type_counts, 0, sizeof(interface_type_counts));
		to_ii_first = ecm_db_multicast_if_first_get_at_index(to_ifaces_first, vif);

		for (list_index = *to_ii_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
			struct ecm_db_iface_instance *ii;
			ecm_db_iface_type_t ii_type;
			char *ii_name;

			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, list_index);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii = *ifaces;
			ii_type = ecm_db_iface_type_get(ii);
			ii_name = ecm_db_interface_type_to_string(ii_type);
			DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", nmci, list_index, ii, ii_type, ii_name);

			/*
			 * Extract information from this interface type if it is applicable to the rule.
			 * Conflicting information may cause accel to be unsupported.
			 */
			switch (ii_type) {
#ifdef ECM_INTERFACE_VLAN_ENABLE
				struct ecm_db_interface_info_vlan vlan_info;
#endif

			case ECM_DB_IFACE_TYPE_BRIDGE:
				DEBUG_TRACE("%p: Bridge\n", nmci);
				if (interface_type_counts[ii_type] != 0) {

					/*
					 * Cannot cascade bridges
					 */
					rule_invalid = true;
					DEBUG_TRACE("%p: Bridge - ignore additional\n", nmci);
					break;
				}
				ecm_db_iface_bridge_address_get(ii, to_nss_iface_address);
				to_iface_bridge_identifier = ecm_db_iface_interface_identifier_get(ii);
				DEBUG_TRACE("%p: Bridge - mac: %pM\n", nmci, to_nss_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_ETHERNET:
				 DEBUG_TRACE("%p: Ethernet\n", nmci);
				if (interface_type_counts[ii_type] != 0) {
					/*
					 * Ignore additional mac addresses, these are usually as a result of address propagation
					 * from bridges down to ports etc.
					 */
					DEBUG_TRACE("%p: Ethernet - ignore additional\n", nmci);
					break;
				}

				/*
				 * Can only handle one MAC, the first outermost mac.
				 */
				ecm_db_iface_ethernet_address_get(ii, to_nss_iface_address);
				to_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);
				to_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(ii);
				if (to_nss_iface_id < 0) {
					DEBUG_TRACE("%p: to_nss_iface_id: %d\n", nmci, to_nss_iface_id);
					ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
					kfree(nim);
					return -1;
			        }

				DEBUG_TRACE("%p: Ethernet - mac: %pM\n", nmci, to_nss_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
				/*
				 * More than one PPPoE in the list is not valid!
				 */
				if (interface_type_counts[ii_type] != 0) {
					DEBUG_TRACE("%p: PPPoE - additional unsupported\n", nmci);
					rule_invalid = true;
					break;
				}

				/*
				 * Set the PPPoE rule creation structure.
				 */
				create->if_rule[valid_vif_idx].pppoe_if_num = ecm_db_iface_ae_interface_identifier_get(ii);
				if (create->if_rule[valid_vif_idx].pppoe_if_num < 0) {
					DEBUG_TRACE("%p: PPPoE - acceleration engine interface (%d) is not valid\n",
							nmci, create->if_rule[valid_vif_idx].pppoe_if_num);
					rule_invalid = true;
					break;
				}
				create->if_rule[valid_vif_idx].valid_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_PPPOE_VALID;
				DEBUG_TRACE("%p: PPPoE - exist pppoe_if_num: %d\n", nmci,
							create->if_rule[valid_vif_idx].pppoe_if_num);
#else
				DEBUG_TRACE("%p: PPPoE - unsupported\n", nmci);
				rule_invalid = true;
#endif
				break;
			case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
				DEBUG_TRACE("%p: VLAN\n", nmci);
				if (interface_type_counts[ii_type] > 1) {
					/*
					 * Can only support two vlans
					 */
					rule_invalid = true;
					DEBUG_TRACE("%p: VLAN - additional unsupported\n", nmci);
					break;
				}
				ecm_db_iface_vlan_info_get(ii, &vlan_info);
				create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]] = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);

				/*
				 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
				 */
				if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
					memcpy(to_nss_iface_address, vlan_info.address, ETH_ALEN);
					interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
					DEBUG_TRACE("%p: VLAN use mac: %pM\n", nmci, to_nss_iface_address);
				}
				create->if_rule[valid_vif_idx].valid_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;
				DEBUG_TRACE("%p: vlan tag: %x\n", nmci, create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]]);
#else
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - unsupported\n", nmci);
#endif
				break;
			default:
				DEBUG_TRACE("%p: Ignoring: %d (%s)\n", nmci, ii_type, ii_name);
			}

			/*
			 * Seen an interface of this type
			 */
			interface_type_counts[ii_type]++;
		}

		if (rule_invalid) {
			DEBUG_WARN("%p: to/dest Rule invalid\n", nmci);
			ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
			kfree(nim);
			return -1;
		}

		/*
		 * Is this a valid interface?
		 */
		if (to_nss_iface_id != -1) {
			bool is_bridge;
			create->if_rule[valid_vif_idx].if_num = to_nss_iface_id;
			create->if_rule[valid_vif_idx].if_mtu = to_mtu;
			if (rp->if_join_idx[vif]) {
				/*
				 * The interface has joined the group
				 */
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_JOIN;
			} else if (rp->if_leave_idx[vif]) {
				/*
				 * The interface has left the group
				 */
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_LEAVE;
			}

			is_bridge = !ecm_db_connection_is_routed_get(feci->ci);

			/*
			 * Do not set the ROUTED flag for pure bridged interfaces
			 */
			if (is_bridge) {
				uint8_t from_nss_iface_address[ETH_ALEN];
				ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)from_nss_iface_address);
				memcpy(create->if_rule[valid_vif_idx].if_mac, from_nss_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_BRIDGE_FLOW;
			} else {
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_ROUTED_FLOW;
				memcpy(create->if_rule[valid_vif_idx].if_mac, to_nss_iface_address, ETH_ALEN);
			}

			valid_vif_idx++;
		}
	}

	/*
	 * Set number of interface updates in the update rule
	 */
	create->if_count = valid_vif_idx;

	/*
	 * Set the UPDATE flag
	 */
	create->rule_flags |= NSS_IPV4_MC_RULE_CREATE_FLAG_MC_UPDATE;

	/*
	 * Set protocol
	 */
	create->tuple.protocol = IPPROTO_UDP;

	/*
	 * The src_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(create->tuple.flow_ip, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(create->tuple.return_ip, addr);

	/*
	 * Same approach as above for port information
	 */
	create->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	create->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	/*
	 * Destination Node(MAC) address. This address will be same for all to side intefaces
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac);
	memcpy(create->dest_mac, dest_mac, ETH_ALEN);

	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);

	for (vif = 0; vif < valid_vif_idx ; vif++) {
		DEBUG_TRACE("ACCEL UPDATE %p: UDP Accelerate connection %p\n"
				"Rule flag: %x\n"
				"Vif: %d\n"
				"Protocol: %d\n"
				"to_mtu: %u\n"
				"from_ip: %pI4h:%d\n"
				"to_ip: %pI4h:%d\n"
				"to_mac: %pM\n"
				"dest_iface_num: %u\n"
				"out_vlan[0] %x\n"
				"out_vlan[1] %x\n",
				nmci,
				feci->ci,
				create->if_rule[vif].rule_flags,
				vif,
				create->tuple.protocol,
				create->if_rule[vif].if_mtu,
				&create->tuple.flow_ip, create->tuple.flow_ident,
				&create->tuple.return_ip, create->tuple.return_ident,
				create->if_rule[vif].if_mac,
				create->if_rule[vif].if_num,
				create->if_rule[vif].egress_vlan_tag[0],
				create->if_rule[vif].egress_vlan_tag[1]);

	}

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the nss.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%p: connection:%p regen occurred - aborting accel rule.\n", feci, feci->ci);
		kfree(nim);
		return -1;
	}

	/*
	 * Ref the connection before issuing an NSS rule
	 * This ensures that when the NSS responds to the command - which may even be immediately -
	 * the callback function can trust the correct ref was taken for its purpose.
	 * NOTE: remember that this will also implicitly hold the feci.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Call the rule create function
	 */
	nss_tx_status = nss_ipv4_tx(ecm_nss_ipv4_nss_ipv4_mgr, nim);
	if (nss_tx_status == NSS_TX_SUCCESS) {
		spin_lock_bh(&feci->lock);
		nmci->base.stats.driver_fail = 0;		/* Reset */
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return 0;
	}

	/*
	 * Revert accel mode if necessary
	 */
	DEBUG_WARN("%p: ACCEL UPDATE attempt failed\n", nmci);

	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	kfree(nim);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	nmci->base.stats.driver_fail_total++;
	nmci->base.stats.driver_fail++;
	if (nmci->base.stats.driver_fail >= nmci->base.stats.driver_fail_limit) {
		DEBUG_WARN("%p: Accel failed - driver fail limit\n", nmci);
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	}
	spin_unlock_bh(&feci->lock);
	return -1;
}

/*
 * ecm_nss_multicast_ipv4_connection_accelerate()
 * 	Accelerate a multicast UDP connection
 */
static void ecm_nss_multicast_ipv4_connection_accelerate(struct ecm_front_end_connection_instance *feci,
									struct ecm_classifier_process_response *pr)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	uint16_t regen_occurrances;
	struct ecm_db_iface_instance *to_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_nss_iface;
	struct ecm_db_iface_instance *from_nat_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct nss_ipv4_mc_rule_create_msg *create;
	struct nss_ipv4_msg *nim;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int32_t *to_ifaces_first;
	int32_t *to_ii_first;
	int32_t from_ifaces_first;
	int32_t from_nss_iface_id;
	uint32_t from_nat_ifaces_first;
	int32_t to_nss_iface_id;
	int32_t from_nat_ifaces_identifier = 0;
	uint8_t to_nss_iface_address[ETH_ALEN];
	ip_addr_t addr;
	int from_iface_bridge_identifier = 0;
	int to_iface_bridge_identifier = 0;
	int aci_index;
	int vif;
	int ret;
	int assignment_count;
	nss_tx_status_t nss_tx_status;
	int32_t list_index;
	int32_t valid_vif_idx = 0;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	uint8_t dest_mac[ETH_ALEN];
	bool rule_invalid;
	ecm_front_end_acceleration_mode_t result_mode;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	/*
	 * Can this connection be accelerated at all?
	 */
	if (!ecm_nss_ipv4_accel_pending_set(feci)) {
		DEBUG_TRACE("%p: Acceleration denied: %p\n", feci, feci->ci);
		return;
	}

	/*
	 * Construct an accel command.
	 * Initialise Multicast create structure.
	 * NOTE: We leverage the app_data void pointer to be our 32 bit connection serial number.
	 * When we get it back we re-cast it to a uint32 and do a faster connection lookup.
	 */
	nim = (struct nss_ipv4_msg *)kzalloc(sizeof(struct nss_ipv4_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		return;
	}

	nss_ipv4_msg_init(nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_MC_RULE_MSG,
			sizeof(struct nss_ipv4_mc_rule_create_msg),
			 ecm_nss_multicast_ipv4_connection_create_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	create = &nim->msg.mc_rule_create;

	/*
	 * Populate the multicast creation structure
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in from_interfaces list!\n", nmci);
		kfree(nim);
		return;
	}

	create->ingress_vlan_tag[0] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	create->ingress_vlan_tag[1] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	from_nss_iface = from_ifaces[from_ifaces_first];
	from_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(from_nss_iface);
	if (from_nss_iface_id < 0) {
                DEBUG_TRACE("%p: from_nss_iface_id: %d\n", nmci, from_nss_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		kfree(nim);
		return;
        }

	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; list_index < ECM_DB_IFACE_HEIRARCHY_MAX; list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;

		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", nmci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {
#ifdef ECM_INTERFACE_VLAN_ENABLE
			struct ecm_db_interface_info_vlan vlan_info;
#endif
		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%p: Bridge\n", nmci);
			from_iface_bridge_identifier = ecm_db_iface_interface_identifier_get(ii);
			break;
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			DEBUG_TRACE("%p: VLAN\n", nmci);
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - additional unsupported\n", nmci);
				break;
			}
			ecm_db_iface_vlan_info_get(ii, &vlan_info);
			create->ingress_vlan_tag[interface_type_counts[ii_type]] = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);
			create->valid_flags |= NSS_IPV4_MC_RULE_CREATE_FLAG_INGRESS_VLAN_VALID;
			DEBUG_TRACE("%p: vlan tag: %x\n", nmci, create->ingress_vlan_tag[interface_type_counts[ii_type]]);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: VLAN - unsupported\n", nmci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%p: PPPoE - additional unsupported\n", nmci);
				rule_invalid = true;
				break;
			}

			/*
			 * Set the PPPoE rule creation structure.
			 */
			create->valid_flags |= NSS_IPV4_MC_RULE_CREATE_FLAG_INGRESS_PPPOE;
			DEBUG_TRACE("%p: PPPoE - ingress interface is valid\n", nmci);
#else
			rule_invalid = true;
#endif
			break;
		default:
			DEBUG_TRACE("%p: Ignoring: %d (%s)\n", nmci, ii_type, ii_name);
		}
		interface_type_counts[ii_type]++;
	}

	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(feci->ci, &to_ifaces, &to_ifaces_first);
	if (!ret) {
		DEBUG_WARN("%p: Accel attempt failed - no multicast interfaces in to_interfaces list!\n", nmci);
		kfree(nim);
		return;
	}

	from_nat_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_nat_ifaces, ECM_DB_OBJ_DIR_FROM_NAT);
	from_nat_ifaces_identifier = ecm_db_iface_interface_identifier_get(from_nat_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1]);
	ecm_db_connection_interfaces_deref(from_nat_ifaces, from_nat_ifaces_first);
	rule_invalid = false;

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%p: Examine to/dest heirarchy list\n", nmci);
	for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
		int32_t found_nat_ii_match = 0;
		int32_t to_mtu = 0;

		to_nss_iface_id = -1;

		create->if_rule[vif].egress_vlan_tag[0] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
		create->if_rule[vif].egress_vlan_tag[1] = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;

		ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, vif);
		to_ii_first = ecm_db_multicast_if_first_get_at_index(to_ifaces_first, vif);
		memset(interface_type_counts, 0, sizeof(interface_type_counts));

		for (list_index = *to_ii_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
			struct ecm_db_iface_instance *ii;
			int32_t ii_identifier;
			ecm_db_iface_type_t ii_type;
			char *ii_name;
#ifdef ECM_INTERFACE_VLAN_ENABLE
			struct ecm_db_interface_info_vlan vlan_info;
			struct net_device *vlan_out_dev = NULL;
			uint32_t vlan_prio = 0;
#endif

			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, list_index);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii = *ifaces;
			ii_type = ecm_db_iface_type_get(ii);
			ii_name = ecm_db_interface_type_to_string(ii_type);
			ii_identifier = ecm_db_iface_interface_identifier_get(ii);

			/*
			 * Find match for NAT interface in Multicast destination interface list.
			 * If found match, set the found_nat_ii_match flag here.
			 */
			if (ii_identifier == from_nat_ifaces_identifier) {
				found_nat_ii_match = 1;
			}

			DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", nmci, list_index, ii, ii_type, ii_name);

			/*
			 * Extract information from this interface type if it is applicable to the rule.
			 * Conflicting information may cause accel to be unsupported.
			 */
			switch (ii_type) {
			case ECM_DB_IFACE_TYPE_BRIDGE:

				/*
				 * TODO: Find and set the bridge/route flag for this interface
				 */
				DEBUG_TRACE("%p: Bridge\n", nmci);
				if (interface_type_counts[ii_type] != 0) {
					/*
					 * Cannot cascade bridges
					 */
					rule_invalid = true;
					DEBUG_TRACE("%p: Bridge - ignore additional\n", nmci);
					break;
				}
				ecm_db_iface_bridge_address_get(ii, to_nss_iface_address);
				to_iface_bridge_identifier = ecm_db_iface_interface_identifier_get(ii);
				DEBUG_TRACE("%p: Bridge - mac: %pM\n", nmci, to_nss_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_ETHERNET:
				DEBUG_TRACE("%p: Ethernet\n", nmci);
				if (interface_type_counts[ii_type] != 0) {
					/*
					 * Ignore additional mac addresses, these are usually as a result of address propagation
					 * from bridges down to ports etc.
					 */
					DEBUG_TRACE("%p: Ethernet - ignore additional\n", nmci);
					break;
				}

				/*
				 * Can only handle one MAC, the first outermost mac.
				 */
				ecm_db_iface_ethernet_address_get(ii, to_nss_iface_address);
				to_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);
				to_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(ii);
				if (to_nss_iface_id < 0) {
					DEBUG_TRACE("%p: to_nss_iface_id: %d\n", nmci, to_nss_iface_id);
					ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
					kfree(nim);
					return;
			        }

				DEBUG_TRACE("%p: Ethernet - mac: %pM, mtu %d\n", nmci, to_nss_iface_address, to_mtu);
				break;
			case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
				/*
				 * More than one PPPoE in the list is not valid!
				 */
				if (interface_type_counts[ii_type] != 0) {
					DEBUG_TRACE("%p: PPPoE - additional unsupported\n", nmci);
					rule_invalid = true;
					break;
				}

				/*
				 * Set the PPPoE rule creation structure.
				 */
				create->if_rule[valid_vif_idx].pppoe_if_num = ecm_db_iface_ae_interface_identifier_get(ii);
				if (create->if_rule[valid_vif_idx].pppoe_if_num < 0) {
					DEBUG_TRACE("%p: PPPoE - acceleration engine interface (%d) is not valid\n",
							nmci, create->if_rule[valid_vif_idx].pppoe_if_num);
					rule_invalid = true;
					break;
				}
				create->if_rule[valid_vif_idx].valid_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_PPPOE_VALID;
				DEBUG_TRACE("%p: PPPoE - exist if_num: %d\n", nmci,
							create->if_rule[valid_vif_idx].pppoe_if_num);
#else
				DEBUG_TRACE("%p: PPPoE - unsupported\n", nmci);
				rule_invalid = true;
#endif
				break;
			case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
				DEBUG_TRACE("%p: VLAN\n", nmci);
				if (interface_type_counts[ii_type] > 1) {
					/*
					 * Can only support two vlans
					 */
					rule_invalid = true;
					DEBUG_TRACE("%p: VLAN - additional unsupported\n", nmci);
					break;
				}

				ecm_db_iface_vlan_info_get(ii, &vlan_info);
				create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]] = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);

				vlan_out_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
				if (vlan_out_dev) {
					vlan_prio = vlan_dev_get_egress_prio(vlan_out_dev, pr->flow_qos_tag);
					create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]] |= vlan_prio;
					dev_put(vlan_out_dev);
					vlan_out_dev = NULL;
				}

				/*
				 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
				 */
				if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
					memcpy(to_nss_iface_address, vlan_info.address, ETH_ALEN);
					interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
					DEBUG_TRACE("%p: VLAN use mac: %pM\n", nmci, to_nss_iface_address);
				}
				create->if_rule[valid_vif_idx].valid_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;
				DEBUG_TRACE("%p: vlan tag: %x\n", nmci, create->if_rule[vif].egress_vlan_tag[interface_type_counts[ii_type]]);
#else
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - unsupported\n", nmci);
#endif
				break;
			default:
				DEBUG_TRACE("%p: Ignoring: %d (%s)\n", nmci, ii_type, ii_name);
			}

			/*
			 * Seen an interface of this type
			 */
			interface_type_counts[ii_type]++;
		}

		if (rule_invalid) {
			DEBUG_WARN("%p: to/dest Rule invalid\n", nmci);
			ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
			kfree(nim);
			return;
		}

		/*
		 * Populate the interface details for a valid interface in the multicast destination
		 * interface list.
		 */
		if (to_nss_iface_id != -1) {
			uint32_t xlate_src_ip, src_ip;
			bool is_bridge;
			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, addr);
			ECM_IP_ADDR_TO_HIN4_ADDR(xlate_src_ip, addr);
			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
			ECM_IP_ADDR_TO_HIN4_ADDR(src_ip, addr);

			/*
			 * Set a rule for NAT if found_nat_ii_match flag is set
			 * and source IP of packet is not matching with host IP address.
			 */
			if (found_nat_ii_match && (xlate_src_ip != src_ip)) {
				create->if_rule[valid_vif_idx].xlate_src_ip = xlate_src_ip;
				create->if_rule[valid_vif_idx].xlate_src_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT);
				create->if_rule[valid_vif_idx].valid_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_NAT_VALID;
			} else {
				create->if_rule[valid_vif_idx].xlate_src_ip = src_ip;
				create->if_rule[valid_vif_idx].xlate_src_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
			}
			create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_JOIN;
			create->if_rule[valid_vif_idx].if_num = to_nss_iface_id;
			create->if_rule[valid_vif_idx].if_mtu = to_mtu;
			is_bridge = !ecm_db_connection_is_routed_get(feci->ci);

			/*
			 * Identify if the destination interface blongs to pure bridge or routed flow.
			 */
			if (is_bridge) {
				uint8_t from_nss_iface_address[ETH_ALEN];
				ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)from_nss_iface_address);
				memcpy(create->if_rule[valid_vif_idx].if_mac, from_nss_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_BRIDGE_FLOW;
			} else {
				memcpy(create->if_rule[valid_vif_idx].if_mac, to_nss_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= NSS_IPV4_MC_RULE_CREATE_IF_FLAG_ROUTED_FLOW;
			}

			valid_vif_idx++;
		}
	}

	create->if_count = valid_vif_idx;
	create->src_interface_num = from_nss_iface_id;

	/*
	 * Set up the flow qos tags
	 */
	create->qos_tag = (uint32_t)pr->flow_qos_tag;

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
			create->egress_dscp = pr->flow_dscp;
			create->valid_flags |= NSS_IPV4_MC_RULE_CREATE_FLAG_DSCP_MARKING_VALID;
	}
#endif
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac);
	memcpy(create->dest_mac, dest_mac, ETH_ALEN);

	/*
	 * Set protocol
	 */
	create->tuple.protocol = IPPROTO_UDP;

	/*
	 * The src_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(create->tuple.flow_ip, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(create->tuple.return_ip, addr);

	/*
	 * Same approach as above for port information
	 */
	create->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	create->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	/*
	 * Sync our creation command from the assigned classifiers to get specific additional creation rules.
	 * NOTE: These are called in ascending order of priority and so the last classifier (highest) shall
	 * override any preceding classifiers.
	 * This also gives the classifiers a chance to see that acceleration is being attempted.
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(feci->ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_instance *aci;
		struct ecm_classifier_rule_create ecrc;
		/*
		 * NOTE: The current classifiers do not sync anything to the underlying accel engines.
		 * In the future, if any of the classifiers wants to pass any parameter, these parameters
		 * should be received via this object and copied to the accel engine's create object (nircm).
		*/
		aci = assignments[aci_index];
		DEBUG_TRACE("%p: sync from: %p, type: %d\n", nmci, aci, aci->type_get(aci));
		aci->sync_from_v4(aci, &ecrc);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Release the interface lists
	 */
	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);

	for (vif = 0; vif < valid_vif_idx ; vif++) {
		DEBUG_TRACE("%p: UDP Accelerate connection %p\n"
			"Vif: %d\n"
			"Protocol: %d\n"
			"to_mtu: %u\n"
			"from_ip: %pI4h:%d\n"
			"to_ip: %pI4h:%d\n"
			"xlate_ip: %pI4h:%d\n"
			"to_mac: %pM\n"
			"dest_iface_num: %u\n"
			"in_vlan[0] %x\n"
			"in_vlan[1] %x\n"
			"out_vlan[0] %x\n"
			"out_vlan[1] %x\n",
			nmci,
			feci->ci,
			vif,
			create->tuple.protocol,
			create->if_rule[vif].if_mtu,
			&create->tuple.flow_ip, create->tuple.flow_ident,
			&create->tuple.return_ip, create->tuple.return_ident,
			&create->if_rule[vif].xlate_src_ip, create->if_rule[vif].xlate_src_ident,
			create->if_rule[vif].if_mac,
			create->if_rule[vif].if_num,
			create->ingress_vlan_tag[0],
			create->ingress_vlan_tag[1],
			create->if_rule[vif].egress_vlan_tag[0],
			create->if_rule[vif].egress_vlan_tag[1]);
	}

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the nss.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 * Remember that the connection is marked as "accel pending state" so if a regen is flagged immediately
	 * after this check passes, the connection will be decelerated and refreshed very quickly.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%p: connection:%p regen occurred - aborting accel rule.\n", feci, feci->ci);
		ecm_nss_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		kfree(nim);
		return;
	}

	/*
	 * Ref the connection before issuing an NSS rule
	 * This ensures that when the NSS responds to the command - which may even be immediately -
	 * the callback function can trust the correct ref was taken for its purpose.
	 * NOTE: remember that this will also implicitly hold the feci.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Call the rule create function
	 */
	nss_tx_status = nss_ipv4_tx(ecm_nss_ipv4_nss_ipv4_mgr, nim);
	if (nss_tx_status == NSS_TX_SUCCESS) {
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		nmci->base.stats.driver_fail = 0; /* Reset */
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return;
	}

	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: accel mode unexpected: %d\n", nmci, feci->accel_mode);
	nmci->base.stats.driver_fail_total++;
	nmci->base.stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%p: Accel failed - driver fail limit\n", nmci);
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	} else {
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	spin_lock_bh(&ecm_nss_ipv4_lock);
	_ecm_nss_ipv4_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	spin_unlock_bh(&feci->lock);

	kfree(nim);

	return;
}

/*
 * ecm_nss_multicast_ipv4_connection_destroy_callback()
 *	Callback for handling destroy ack/nack calls.
 */
static void ecm_nss_multicast_ipv4_connection_destroy_callback(void *app_data, struct nss_ipv4_msg *nim)
{
	struct nss_ipv4_rule_destroy_msg * __attribute__((unused))nirdm = &nim->msg.rule_destroy;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_nss_multicast_ipv4_connection_instance *nmci;

	/*
	 * Is this a response to a destroy message?
	 */
	if (nim->cm.type != NSS_IPV4_TX_DESTROY_RULE_MSG) {
		DEBUG_ERROR("%p: ported destroy callback with improper type: %d\n", nim, nim->cm.type);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%p: destroy callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	/*
	 * Record command duration
	 */
	ecm_nss_ipv4_decel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: decelerate response for connection: %p\n", nmci, feci->ci);
	DEBUG_TRACE("%p: flow_ip: %pI4h:%d\n", nmci, &nirdm->tuple.flow_ip, nirdm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4h:%d\n", nmci, &nirdm->tuple.return_ip, nirdm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", nmci, nirdm->tuple.protocol);

	/*
	 * Drop decel pending counter
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_nss_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	spin_lock_bh(&feci->lock);

	/*
	 * If decel is not still pending then it's possible that the NSS ended acceleration by some other reason e.g. flush
	 * In which case we cannot rely on the response we get here.
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING) {
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connections.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_TRACE("%p: response: %d\n", nmci, nim->cm.response);
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DECEL;
	} else {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	/*
	 * If connection became defunct then set mode so that no further accel/decel attempts occur.
	 */
	if (feci->is_defunct) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT;
	}

	spin_unlock_bh(&feci->lock);

	/*
	 * Mcast acceleration ends
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_multicast_ipv4_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_nss_multicast_ipv4_accelerated_count >= 0, "Bad udp accel counter\n");
	ecm_nss_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_nss_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	/*
	 * Release the connections.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_nss_multicast_ipv4_connection_decelerate_msg_send()
 *	Prepares and sends a decelerate message to acceleration engine.
 */
static bool ecm_nss_multicast_ipv4_connection_decelerate_msg_send(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	struct nss_ipv4_msg nim;
	struct nss_ipv4_rule_destroy_msg *nirdm;
	ip_addr_t src_addr;
	ip_addr_t group_addr;
	nss_tx_status_t nss_tx_status;
	bool ret;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	/*
	 * Increment the decel pending counter
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_ipv4_pending_decel_count++;
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	/*
	 * Prepare deceleration message
	 */
	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv4_rule_destroy_msg),
			ecm_nss_multicast_ipv4_connection_destroy_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nirdm = &nim.msg.rule_destroy;
	nirdm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nirdm->tuple.flow_ip, src_addr);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, group_addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nirdm->tuple.return_ip, group_addr);
	nirdm->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nirdm->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	DEBUG_INFO("%p: Mcast Connection %p decelerate\n"
			"protocol: %d\n"
			"src_ip: %pI4:%d\n"
			"dest_ip: %pI4:%d\n",
			nmci, feci->ci, nirdm->tuple.protocol,
			&nirdm->tuple.flow_ip, nirdm->tuple.flow_ident,
			&nirdm->tuple.return_ip, nirdm->tuple.return_ident);

	/*
	 * Right place to free multicast destination interfaces list.
	 */
	ecm_db_multicast_connection_to_interfaces_clear(feci->ci);

	/*
	 * Take a ref to the feci->ci so that it will persist until we get a response from the NSS.
	 * NOTE: This will implicitly hold the feci too.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Destroy the NSS connection cache entry.
	 */
	nss_tx_status = nss_ipv4_tx(ecm_nss_ipv4_nss_ipv4_mgr, &nim);
	if (nss_tx_status == NSS_TX_SUCCESS) {
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;			/* Reset */
		spin_unlock_bh(&feci->lock);
		return true;
	}

	/*
	 * TX failed
	 */
	ret = ecm_front_end_destroy_failure_handle(feci);

	/*
	 * Release the ref take, NSS driver did not accept our command.
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * Could not send the request, decrement the decel pending counter
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_nss_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	return ret;
}

/*
 * ecm_nss_multicast_ipv4_connection_decelerate()
 *     Decelerate a connection
 */
static bool ecm_nss_multicast_ipv4_connection_decelerate(struct ecm_front_end_connection_instance *feci)
{
	/*
	 * Check if accel mode is OK for the deceleration.
	 */
	spin_lock_bh(&feci->lock);
	if (!ecm_front_end_common_connection_decelerate_accel_mode_check(feci)) {
		spin_unlock_bh(&feci->lock);
		return false;
	}
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING;
	spin_unlock_bh(&feci->lock);

	return ecm_nss_multicast_ipv4_connection_decelerate_msg_send(feci);
}

/*
 * ecm_nss_multicast_ipv4_connection_defunct_callback()
 *	Callback to be called when a Mcast connection has become defunct.
 */
static bool ecm_nss_multicast_ipv4_connection_defunct_callback(void *arg, int *accel_mode)
{
	bool ret;
	struct ecm_front_end_connection_instance *feci = (struct ecm_front_end_connection_instance *)arg;
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	spin_lock_bh(&feci->lock);
	/*
	 * Check if the connection can be defuncted.
	 */
	if (!ecm_front_end_common_connection_defunct_check(feci)) {
		*accel_mode = feci->accel_mode;
		spin_unlock_bh(&feci->lock);
		return false;
	}

	/*
	 * If none of the cases matched above, this means the connection is in the
	 * accel mode, so we force a deceleration.
	 * NOTE: If the mode is accel pending then the decel will be actioned when that is completed.
	 */
	if (!ecm_front_end_common_connection_decelerate_accel_mode_check(feci)) {
		*accel_mode = feci->accel_mode;
		spin_unlock_bh(&feci->lock);
		return false;
	}
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING;
	spin_unlock_bh(&feci->lock);

	/*
	 * If none of the cases matched above, this means the connection is in the
	 * accel mode, so we force a deceleration.
	 * NOTE: If the mode is accel pending then the decel will be actioned when that is completed.
	 */
	ret = ecm_nss_multicast_ipv4_connection_decelerate_msg_send(feci);

	/*
	 * Copy the accel_mode which is returned from the decelerate message function. This value
	 * will be used in the caller to decide releasing the final reference of the connection.
	 * But if this function reaches to here, the caller care about the ret. If ret is true,
	 * the reference will be released regardless of the accel_mode. If ret is false, accel_mode
	 * will be in the ACCEL state (for destroy re-try) and this state will not be used in the
	 * caller's decision. It looks for ACCEL_FAIL states.
	 */
	spin_lock_bh(&feci->lock);
	*accel_mode = feci->accel_mode;
	spin_unlock_bh(&feci->lock);

	return ret;
}

/*
 * ecm_nss_multicast_ipv4_connection_accel_state_get()
 *	Get acceleration state
 */
static ecm_front_end_acceleration_mode_t ecm_nss_multicast_ipv4_connection_accel_state_get(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;
	ecm_front_end_acceleration_mode_t state;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);
	spin_lock_bh(&feci->lock);
	state = feci->accel_mode;
	spin_unlock_bh(&feci->lock);
	return state;
}

/*
 * ecm_nss_multicast_ipv4_connection_action_seen()
 *	Acceleration action / activity has been seen for this connection.
 *
 * NOTE: Call the action_seen() method when the NSS has demonstrated that it has offloaded some data for a connection.
 */
static void ecm_nss_multicast_ipv4_connection_action_seen(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);
	DEBUG_INFO("%p: Action seen\n", nmci);
	spin_lock_bh(&feci->lock);
	feci->stats.no_action_seen = 0;
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_nss_multicast_ipv4_connection_accel_ceased()
 *	NSS has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an NSS self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_nss_multicast_ipv4_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);
	DEBUG_INFO("%p: accel ceased\n", nmci);

	spin_lock_bh(&feci->lock);

	/*
	 * If we are in accel-pending state then the NSS has issued a flush out-of-order
	 * with the ACK/NACK we are actually waiting for.
	 * To work around this we record a "flush has already happened" and will action it when we finally get that ACK/NACK.
	 * GGG TODO This should eventually be removed when the NSS honours messaging sequence.
	 */
	if (feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING) {
		feci->stats.flush_happened = true;
		feci->stats.flush_happened_total++;
		spin_unlock_bh(&feci->lock);
		return;
	}

	/*
	 * If connection is no longer accelerated by the time we get here just ignore the command
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL) {
		spin_unlock_bh(&feci->lock);
		return;
	}

	/*
	 * If the no_action_seen counter was not reset then acceleration ended without any offload action
	 */
	if (feci->stats.no_action_seen) {
		feci->stats.no_action_seen_total++;
	}

	/*
	 * If the no_action_seen indicates successive cessations of acceleration without any offload action occuring
	 * then we fail out this connection
	 */
	if (feci->stats.no_action_seen >= feci->stats.no_action_seen_limit) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_NO_ACTION;
	} else {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}
	spin_unlock_bh(&feci->lock);

	/*
	 * Mcast acceleration ends
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_multicast_ipv4_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_nss_multicast_ipv4_accelerated_count >= 0, "Bad Mcast accel counter\n");
	ecm_nss_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_nss_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_nss_ipv4_lock);
}

/*
 * ecm_nss_multicast_ipv4_connection_ref()
 *	Ref a connection front end instance
 */
static void ecm_nss_multicast_ipv4_connection_ref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);
	spin_lock_bh(&feci->lock);
	feci->refs++;
	DEBUG_TRACE("%p: nmci ref %d\n", feci, feci->refs);
	DEBUG_ASSERT(feci->refs > 0, "%p: ref wrap\n", feci);
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_nss_multicast_ipv4_connection_deref()
 *	Deref a connection front end instance
 */
static int ecm_nss_multicast_ipv4_connection_deref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	spin_lock_bh(&feci->lock);
	feci->refs--;
	DEBUG_ASSERT(feci->refs >= 0, "%p: ref wrap\n", feci);

	if (feci->refs > 0) {
		int refs = feci->refs;
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%p: nmci deref %d\n", nmci, refs);
		return refs;
	}
	spin_unlock_bh(&feci->lock);

	/*
	 * We can now destroy the instance
	 */
	DEBUG_TRACE("%p: nmci final\n", nmci);
	DEBUG_CLEAR_MAGIC(nmci);
	kfree(nmci);
	return 0;
}

/*
 * ecm_nss_multicast_ipv4_connection_regenerate()
 *	Re-generate a connection.
 *
 * Re-generating a connection involves re-evaluating the interface lists in case interface heirarchies have changed.
 * It also involves the possible triggering of classifier re-evaluation but only if all currently assigned
 * classifiers permit this operation.
 */
static void ecm_nss_multicast_ipv4_connection_regenerate(struct ecm_db_connection_instance *ci, ecm_tracker_sender_type_t sender,
							struct net_device *out_dev, struct net_device *in_dev, struct net_device *in_dev_nat)
{
	int i;
	bool reclassify_allowed;
	struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_list_first;
	struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_nat_list_first;
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	int protocol;
	bool is_routed;
	uint8_t src_node_addr[ETH_ALEN];
	uint8_t dest_node_addr[ETH_ALEN];
	uint8_t src_node_addr_nat[ETH_ALEN];
	int assignment_count;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	struct ecm_front_end_connection_instance *feci;
	__be16 layer4hdr[2] = {0, 0};
	__be16 port = 0;

	/*
	 * Update the interface lists - these may have changed, e.g. LAG path change etc.
	 * NOTE: We never have to change the usual mapping->host->node_iface arrangements for each side of the connection (to/from sides)
	 * This is because if these interfaces change then the connection is dead anyway.
	 * But a LAG slave might change the heirarchy the connection is using but the LAG master is still sane.
	 * If any of the new interface heirarchies cannot be created then simply set empty-lists as this will deny
	 * acceleration and ensure that a bad rule cannot be created.
	 * IMPORTANT: The 'sender' defines who has sent the packet that triggered this re-generation
	 */
	protocol = ecm_db_connection_protocol_get(ci);

	is_routed = ecm_db_connection_is_routed_get(ci);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, ip_src_addr);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, ip_src_addr_nat);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, ip_dest_addr);

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_node_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, src_node_addr_nat);

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dest_node_addr);

	feci = ecm_db_connection_front_end_get_and_ref(ci);

	port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
	layer4hdr[0] = htons(port);
	port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));
	layer4hdr[1] = htons(port);

	DEBUG_TRACE("%p: Update the 'from' interface heirarchy list\n", ci);
	from_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_list, ip_dest_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, NULL);
	if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		goto ecm_multicast_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_list, from_list_first);

	DEBUG_TRACE("%p: Update the 'from NAT' interface heirarchy list\n", ci);
	from_nat_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_nat_list, ip_dest_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr_nat, dest_node_addr, layer4hdr, NULL);
	if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		goto ecm_multicast_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);
	ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);

	feci->deref(feci);

	/*
	 * Get list of assigned classifiers to reclassify.
	 * Remember: This also includes our default classifier too.
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);

	/*
	 * All of the assigned classifiers must permit reclassification.
	 */
	reclassify_allowed = true;
	for (i = 0; i < assignment_count; ++i) {
		DEBUG_TRACE("%p: Calling to reclassify: %p, type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
		if (!assignments[i]->reclassify_allowed(assignments[i])) {
			DEBUG_TRACE("%p: reclassify denied: %p, by type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
			reclassify_allowed = false;
			break;
		}
	}

	/*
	 * Now we action any classifier re-classify
	 */
	if (!reclassify_allowed) {
		/*
		 * Regeneration came to a successful conclusion even though reclassification was denied
		 */
		DEBUG_WARN("%p: re-classify denied\n", ci);
		goto ecm_multicast_ipv4_regen_done;
	}

	/*
	 * Reclassify
	 */
	DEBUG_INFO("%p: reclassify\n", ci);
	if (!ecm_classifier_reclassify(ci, assignment_count, assignments)) {
		/*
		 * We could not set up the classifiers to reclassify, it is safer to fail out and try again next time
		 */
		DEBUG_WARN("%p: Regeneration: reclassify failed\n", ci);
		goto ecm_multicast_ipv4_regen_done;
	}
	DEBUG_INFO("%p: reclassify success\n", ci);

ecm_multicast_ipv4_regen_done:

	/*
	 * Release the assignments
	 */
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Re-generation of state is successful.
	 */
	ecm_db_connection_regeneration_completed(ci);

	return;

ecm_multicast_ipv4_retry_regen:
	feci->deref(feci);
	ecm_db_connection_regeneration_failed(ci);
	return;
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_nss_multicast_ipv4_connection_state_get()
 *	Return state of this multicast front end instance
 */
static int ecm_nss_multicast_ipv4_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	int result;
	bool can_accel;
	ecm_front_end_acceleration_mode_t accel_mode;
	struct ecm_front_end_connection_mode_stats stats;
	struct ecm_nss_multicast_ipv4_connection_instance *nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nmci);

	spin_lock_bh(&feci->lock);
	can_accel = feci->can_accel;
	accel_mode = feci->accel_mode;
	memcpy(&stats, &feci->stats, sizeof(struct ecm_front_end_connection_mode_stats));
	spin_unlock_bh(&feci->lock);

	if ((result = ecm_state_prefix_add(sfi, "front_end_v4.multicast"))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "can_accel", "%d", can_accel))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "accel_mode", "%d", accel_mode))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "decelerate_pending", "%d", stats.decelerate_pending))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "flush_happened_total", "%d", stats.flush_happened_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen_total", "%d", stats.no_action_seen_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen", "%d", stats.no_action_seen))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen_limit", "%d", stats.no_action_seen_limit))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail_total", "%d", stats.driver_fail_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail", "%d", stats.driver_fail))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail_limit", "%d", stats.driver_fail_limit))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack_total", "%d", stats.ae_nack_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack", "%d", stats.ae_nack))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack_limit", "%d", stats.ae_nack_limit))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_nss_multicast_ipv4_connection_instance_alloc()
 *	Create a front end instance specific for Mcast connection
 */
static struct ecm_nss_multicast_ipv4_connection_instance *ecm_nss_multicast_ipv4_connection_instance_alloc(
								struct ecm_db_connection_instance *ci,
								bool can_accel)
{
	struct ecm_nss_multicast_ipv4_connection_instance *nmci;
	struct ecm_front_end_connection_instance *feci;

	nmci = (struct ecm_nss_multicast_ipv4_connection_instance *)kzalloc(sizeof(struct ecm_nss_multicast_ipv4_connection_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!nmci) {
		DEBUG_WARN("Mcast Front end alloc failed\n");
		return NULL;
	}

	/*
	 * Refs is 1 for the creator of the connection
	 */
	feci = (struct ecm_front_end_connection_instance *)nmci;
	feci->refs = 1;
	DEBUG_SET_MAGIC(nmci, ECM_NSS_MULTICAST_IPV4_CONNECTION_INSTANCE_MAGIC);
	spin_lock_init(&feci->lock);

	feci->can_accel = can_accel;
	feci->accel_mode = (can_accel)? ECM_FRONT_END_ACCELERATION_MODE_DECEL : ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED;
	spin_lock_bh(&ecm_nss_ipv4_lock);
	feci->stats.no_action_seen_limit = ecm_nss_ipv4_no_action_limit_default;
	feci->stats.driver_fail_limit = ecm_nss_ipv4_driver_fail_limit_default;
	feci->stats.ae_nack_limit = ecm_nss_ipv4_nack_limit_default;
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	/*
	 * Copy reference to connection - no need to ref ci as ci maintains a ref to this instance instead (this instance persists for as long as ci does)
	 */
	feci->ci = ci;

	feci->ip_version = 4;

	/*
	 * Populate the methods and callbacks
	 */
	feci->ref = ecm_nss_multicast_ipv4_connection_ref;
	feci->deref = ecm_nss_multicast_ipv4_connection_deref;
	feci->decelerate = ecm_nss_multicast_ipv4_connection_decelerate;
	feci->accel_state_get = ecm_nss_multicast_ipv4_connection_accel_state_get;
	feci->action_seen = ecm_nss_multicast_ipv4_connection_action_seen;
	feci->accel_ceased = ecm_nss_multicast_ipv4_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_nss_multicast_ipv4_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_nss_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_nss_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_nss_common_get_interface_type;
	feci->regenerate = ecm_nss_common_connection_regenerate;

	return nmci;
}

/*
 * ecm_nss_multicast_ipv4_node_establish_and_ref()
 *	Returns a reference to a node, possibly creating one if necessary.
 *
 * The given_node_addr will be used if provided.
 *
 * Returns NULL on failure.
 *
 * TODO: This function will be removed later and the one in the ecm_nss_ipv4.c file will be used
 *	instead of this one when the multicast code is fixed to use the new interface hierarchy
 *	construction model.
 */
static struct ecm_db_node_instance *ecm_nss_multicast_ipv4_node_establish_and_ref(struct ecm_front_end_connection_instance *feci,
							struct net_device *dev, ip_addr_t addr,
							struct ecm_db_iface_instance *interface_list[], int32_t interface_list_first,
							uint8_t *given_node_addr, struct sk_buff *skb)
{
	struct ecm_db_node_instance *ni;
	struct ecm_db_node_instance *nni;
	struct ecm_db_iface_instance *ii;
	int i;
	bool done;
	uint8_t node_addr[ETH_ALEN];
#if defined(ECM_INTERFACE_L2TPV2_ENABLE) || defined(ECM_INTERFACE_PPTP_ENABLE)
	ip_addr_t local_ip, remote_ip;
#endif

	DEBUG_INFO("Establish node for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));

	/*
	 * The node is the datalink address, typically a MAC address.
	 * However the node address to use is not always obvious and depends on the interfaces involved.
	 * For example if the interface is PPPoE then we use the MAC of the PPPoE server as we cannot use normal ARP resolution.
	 * Not all hosts have a node address, where there is none, a suitable alternative should be located and is typically based on 'addr'
	 * or some other datalink session information.
	 * It should be, at a minimum, something that ties the host with the interface.
	 *
	 * Iterate from 'inner' to 'outer' interfaces - discover what the node is.
	 */
	memset(node_addr, 0, ETH_ALEN);
	done = false;
	if (given_node_addr) {
		memcpy(node_addr, given_node_addr, ETH_ALEN);
		done = true;
		DEBUG_TRACE("Using given node address: %pM\n", node_addr);
	}

	for (i = ECM_DB_IFACE_HEIRARCHY_MAX - 1; (!done) && (i >= interface_list_first); i--) {
		ecm_db_iface_type_t type;
		ip_addr_t gw_addr = ECM_IP_ADDR_NULL;
		bool on_link = false;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
		struct ecm_db_interface_info_pppol2tpv2 pppol2tpv2_info;
#endif
#ifdef ECM_INTERFACE_PPTP_ENABLE
		struct ecm_db_interface_info_pptp pptp_info;
#endif
		type = ecm_db_iface_type_get(interface_list[i]);
		DEBUG_INFO("Lookup node address, interface @ %d is type: %d\n", i, type);

		switch (type) {

		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * Node address is the address of the remote PPPoE server
			 */
			ecm_db_iface_pppoe_session_info_get(interface_list[i], &pppoe_info);
			memcpy(node_addr, pppoe_info.remote_mac, ETH_ALEN);
			done = true;
			break;
#else
			DEBUG_TRACE("PPPoE interface unsupported\n");
			return NULL;
#endif

		case ECM_DB_IFACE_TYPE_SIT:
		case ECM_DB_IFACE_TYPE_TUNIPIP6:
			done = true;
			break;

		case ECM_DB_IFACE_TYPE_PPPOL2TPV2:
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			ecm_db_iface_pppol2tpv2_session_info_get(interface_list[i], &pppol2tpv2_info);
			ECM_HIN4_ADDR_TO_IP_ADDR(local_ip, pppol2tpv2_info.ip.saddr);
			ECM_HIN4_ADDR_TO_IP_ADDR(remote_ip, pppol2tpv2_info.ip.daddr);
			if (ECM_IP_ADDR_MATCH(local_ip, addr)) {
				if (unlikely(!ecm_interface_mac_addr_get(local_ip, node_addr, &on_link, gw_addr))) {
					DEBUG_TRACE("failed to obtain node address for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(local_ip));
					return NULL;
				}

			} else {
				if (unlikely(!ecm_interface_mac_addr_get(remote_ip, node_addr, &on_link, gw_addr))) {
					DEBUG_TRACE("failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(remote_ip));
					return NULL;
				}
			}

			done = true;
			break;
#else
			DEBUG_TRACE("PPPoL2TPV2 interface unsupported\n");
			return NULL;
#endif

		case ECM_DB_IFACE_TYPE_PPTP:
#ifdef ECM_INTERFACE_PPTP_ENABLE
			ecm_db_iface_pptp_session_info_get(interface_list[i], &pptp_info);
			ECM_HIN4_ADDR_TO_IP_ADDR(local_ip, pptp_info.src_ip);
			ECM_HIN4_ADDR_TO_IP_ADDR(remote_ip, pptp_info.dst_ip);
			if (ECM_IP_ADDR_MATCH(local_ip, addr)) {
				if (unlikely(!ecm_interface_mac_addr_get(local_ip, node_addr, &on_link, gw_addr))) {
					DEBUG_TRACE("failed to obtain node address for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(local_ip));
					return NULL;
				}

			} else {
				if (unlikely(!ecm_interface_mac_addr_get(remote_ip, node_addr, &on_link, gw_addr))) {
					DEBUG_TRACE("failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(remote_ip));
					return NULL;
				}
			}

			done = true;
			break;
#else
			DEBUG_TRACE("PPTP interface unsupported\n");
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			/*
			 * VLAN handled same along with ethernet, lag, bridge etc.
			 */
#else
			DEBUG_TRACE("VLAN interface unsupported\n");
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_ETHERNET:
		case ECM_DB_IFACE_TYPE_LAG:
		case ECM_DB_IFACE_TYPE_BRIDGE:
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
			if (!ecm_interface_mac_addr_get(addr, node_addr, &on_link, gw_addr)) {
				DEBUG_TRACE("failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
				ecm_interface_send_arp_request(dev, addr, on_link, gw_addr);

				/*
				 * Unable to get node address at this time.
				 */
				return NULL;
			}

			if (is_multicast_ether_addr(node_addr)) {
				DEBUG_TRACE("multicast node address for host " ECM_IP_ADDR_DOT_FMT ", node_addr: %pM\n", ECM_IP_ADDR_TO_DOT(addr), node_addr);
				return NULL;
			}

			/*
			 * Because we are iterating from inner to outer interface, this interface is the
			 * innermost one that has a node address - take this one.
			 */
			done = true;
			break;
		default:
			/*
			 * Don't know how to handle these.
			 * Just copy some part of the address for now, but keep iterating the interface list
			 * in the hope something recognisable will be seen!
			 * GGG TODO We really need to roll out support for all interface types we can deal with ASAP :-(
			 */
			memcpy(node_addr, (uint8_t *)addr, ETH_ALEN);
		}
	}
	if (!done) {
		DEBUG_INFO("Failed to establish node for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
		return NULL;
	}

	/*
	 * Establish iface
	 */
	ii = ecm_interface_establish_and_ref(feci, dev, skb);
	if (!ii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Locate the node
	 */
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		DEBUG_TRACE("%p: node established\n", ni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	/*
	 * No node - create one
	 */
	nni = ecm_db_node_alloc();
	if (!nni) {
		DEBUG_WARN("Failed to establish node\n");
		ecm_db_iface_deref(ii);
		return NULL;
	}

	/*
	 * Add node into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		spin_unlock_bh(&ecm_nss_ipv4_lock);
		ecm_db_node_deref(nni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	ecm_db_node_add(nni, ii, node_addr, NULL, nni);
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	/*
	 * Don't need iface instance now
	 */
	ecm_db_iface_deref(ii);

	DEBUG_TRACE("%p: node established\n", nni);
	return nni;
}

/*
 * ecm_nss_multicast_ipv4_connection_process()
 *	Process a UDP multicast flow
 */
unsigned int ecm_nss_multicast_ipv4_connection_process(struct net_device *out_dev,
							struct net_device *in_dev,
							uint8_t *src_node_addr,
							uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, struct sk_buff *skb,
							struct ecm_tracker_ip_header *iph,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple)
{
	int vif, if_cnt;
	uint32_t dst_dev[ECM_DB_MULTICAST_IF_MAX];
	struct udphdr *udp_hdr;
	struct udphdr udp_hdr_buff;
	int src_port;
	int dest_port;
	int src_port_nat = 0;
	struct net_device *in_dev_nat = NULL;
	struct net_device *out_dev_master = NULL;
	struct ecm_db_multicast_tuple_instance *tuple_instance;
	struct ecm_db_connection_instance *ci;
	ip_addr_t match_addr;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ecm_db_direction_t ecm_dir = ECM_DB_DIRECTION_INGRESS_NAT;
	ecm_db_timer_group_t ci_orig_timer_group;
	struct ecm_classifier_process_response prevalent_pr;
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	__be32 ip_src;
	__be32 ip_grp;
	bool br_dev_found_in_mfc = false;
	int protocol = (int)orig_tuple->dst.protonum;
	__be16 *layer4hdr = NULL;

	if (protocol != IPPROTO_UDP) {
		DEBUG_WARN("Invalid Protocol %d in skb %p\n", protocol, skb);
		return NF_ACCEPT;
	}

	/*
	 * Extract UDP header to obtain port information
	 */
	udp_hdr = ecm_tracker_udp_check_header_and_read(skb, iph, &udp_hdr_buff);
	if (unlikely(!udp_hdr)) {
		DEBUG_WARN("Invalid UDP header in skb %p\n", skb);
		return NF_ACCEPT;
	}

	/*
	 * Return if source dev is any tunnel type
	 * TODO: Add support for multicast over tunnels
	 */
	if ((in_dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) ||
	    (in_dev->type == ARPHRD_SIT) || (in_dev->type == ARPHRD_PPP) ||
	    (in_dev->type == ARPHRD_TUNNEL6)) {
		DEBUG_TRACE("Net device: %p is TUNNEL type: %d\n", in_dev, in_dev->type);
		return NF_ACCEPT;
	}

	layer4hdr = (__be16*)udp_hdr;
	/*
	 * Now extract information, if we have conntrack then use that (which would already be in the tuples)
	 */
	if (unlikely(!ct)) {
		orig_tuple->src.u.udp.port = udp_hdr->source;
		orig_tuple->dst.u.udp.port = udp_hdr->dest;
		reply_tuple->src.u.udp.port = udp_hdr->dest;
		reply_tuple->dst.u.udp.port = udp_hdr->source;
	}

	ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple->src.u3.ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple->dst.u3.ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple->dst.u3.ip);

	ECM_IP_ADDR_TO_NIN4_ADDR(ip_src, ip_src_addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(ip_grp, ip_dest_addr);

	/*
	 * Extract Port numbers
	 */
	src_port = ntohs(orig_tuple->src.u.udp.port);
	dest_port = ntohs(orig_tuple->dst.u.udp.port);
	src_port_nat = ntohs(reply_tuple->dst.u.udp.port);

	/*
	 * Initialize in_dev_nat to default in_dev.
	 */
	in_dev_nat = in_dev;

	/*
	 * Query MFC/Bridge to access the destination interface list.
	 */
	memset(dst_dev, 0, sizeof(dst_dev));
	if_cnt =  ipmr_find_mfc_entry(&init_net, ip_src, ip_grp, ECM_DB_MULTICAST_IF_MAX, dst_dev);

	/*
	 * Skip PPP acceleration
	 */
	if (ecm_interface_multicast_is_iface_type(dst_dev, if_cnt, ARPHRD_PPP)) {
		DEBUG_TRACE("%p: Packet is of type PPP; skip it\n", skb);
		return NF_ACCEPT;
	}

	if (is_routed) {
		/*
		 * This is a routed flow, hence look for a valid MFC rule
		 */
		if (if_cnt <= 0) {
			DEBUG_WARN("Not found a valid vif count %d\n", if_cnt);
			return NF_ACCEPT;
		}

		/*
		 * Check for the presence of a bridge device in the destination
		 * interface list given to us by MFC
		 */
		br_dev_found_in_mfc = ecm_interface_multicast_check_for_br_dev(dst_dev, if_cnt);
	} else {
		if (if_cnt > 0) {
			/*
			 *  In case of Bridge + Route there is chance that Bridge post routing hook called first and
			 *  is_route flag is false. To make sure this is a routed flow, query the MFC and if MFC if_cnt
			 *  is not Zero than this is a routed flow.
			 */
			is_routed = true;
			br_dev_found_in_mfc = ecm_interface_multicast_check_for_br_dev(dst_dev, if_cnt);
		} else {
			out_dev_master =  ecm_interface_get_and_hold_dev_master(out_dev);
			DEBUG_ASSERT(out_dev_master, "Expected a master\n");

			/*
			 * Packet flow is pure bridge. Try to query the snooper for the destination
			 * interface list
			 */
			if_cnt = mc_bridge_ipv4_get_if(out_dev_master, ip_src, ip_grp, ECM_DB_MULTICAST_IF_MAX, dst_dev);
			if (if_cnt <= 0) {
				DEBUG_WARN("Not found a valid MCS if count %d\n", if_cnt);
				goto done;
			}

			/*
			 * The source interface could have joined the group as well.
			 * In such cases, the destination interface list returned by
			 * the snooper would include the source interface as well.
			 * We need to filter the source interface from the list in such cases.
			 */
			if_cnt = ecm_interface_multicast_check_for_src_ifindex(dst_dev, if_cnt, in_dev->ifindex);
			if (if_cnt <= 0) {
				DEBUG_WARN("Not found a valid MCS if count %d\n", if_cnt);
				goto done;
			}
		}
	}

	/*
	 * In pure bridge flow, do not process further if TTL is less than two.
	 */
	if (!is_routed) {
		if (iph->ttl < 2) {
			DEBUG_TRACE("%p: Ignoring, Multicast IPv4 Header has TTL one\n", skb);
			goto done;
		}
	}

	/*
	 * Work out if this packet involves NAT or not.
	 * If it does involve NAT then work out if this is an ingressing or egressing packet.
	 */
	if (!ECM_IP_ADDR_MATCH(ip_src_addr, ip_src_addr_nat)) {
		int nat_if_found = 0;
		bool from_local_addr;

		/*
		 * Egressing NAT
		 */
		in_dev_nat = ecm_interface_dev_find_by_addr(ip_src_addr_nat, &from_local_addr);
		if (!in_dev_nat) {
			goto done;
		}

		/*
		 * Try to find the egress NAT interface in the list of destination
		 * interfaces
		 */
		for (vif = 0; vif < if_cnt; vif++) {
			if (dst_dev[vif] == in_dev_nat->ifindex) {
				nat_if_found = 1;
				break;
			}
		}
		dev_put(in_dev_nat);

		/*
		 * We have not found the NAT interface in the MFC destination interface
		 * list for a NAT flow, which should not happen.
		 */
		if (!nat_if_found) {
			goto done;
		}
	}

	DEBUG_TRACE("MCAST UDP src: " ECM_IP_ADDR_DOT_FMT ":%d, dest: " ECM_IP_ADDR_DOT_FMT ":%d, dir %d\n",
			ECM_IP_ADDR_TO_DOT(ip_src_addr), src_port, ECM_IP_ADDR_TO_DOT(ip_dest_addr), dest_port, ecm_dir);

	/*
	 * Look up a connection
	 */
	ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);

	if (unlikely(!ci)) {
		struct ecm_db_mapping_instance *mi[ECM_DB_OBJ_DIR_MAX];
		struct ecm_db_node_instance *ni[ECM_DB_OBJ_DIR_MAX];
		struct ecm_classifier_default_instance *dci;
		struct ecm_front_end_connection_instance *feci;
		struct ecm_db_connection_instance *nci;
		struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_db_iface_instance *to_list;
		struct ecm_db_iface_instance *to_list_single;
		struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		ecm_classifier_type_t classifier_type;
		int32_t from_list_first;
		int32_t interface_idx_cnt = 0;
		int32_t from_nat_list_first;
		int32_t *to_list_first;
		int32_t *to_first;
		int ret;
		uint8_t dest_mac_addr[ETH_ALEN];

		DEBUG_TRACE("New UDP connection from " ECM_IP_ADDR_DOT_FMT ":%u to " ECM_IP_ADDR_DOT_FMT ":%u\n", ECM_IP_ADDR_TO_DOT(ip_src_addr), \
				src_port, ECM_IP_ADDR_TO_DOT(ip_dest_addr), dest_port);

		/*
		 * Before we attempt to create the connection are we being terminated?
		 */
		spin_lock_bh(&ecm_nss_ipv4_lock);
		if (ecm_nss_ipv4_terminate_pending) {
			spin_unlock_bh(&ecm_nss_ipv4_lock);
			DEBUG_WARN("Terminating\n");

			/*
			 * As we are terminating we just allow the packet to pass - it's no longer our concern
			 */
			goto done;
		}
		spin_unlock_bh(&ecm_nss_ipv4_lock);

		/*
		 * Now allocate the new connection
		 */
		nci = ecm_db_connection_alloc();
		if (!nci) {
			DEBUG_WARN("Failed to allocate connection\n");
			goto done;
		}

		/*
		 * Connection must have a front end instance associated with it
		 */
		feci = (struct ecm_front_end_connection_instance *)ecm_nss_multicast_ipv4_connection_instance_alloc(nci, can_accel);
		if (!feci) {
			ecm_db_connection_deref(nci);
			DEBUG_WARN("Failed to allocate front end\n");
			goto done;
		}

		/*
		 * Create a tuple instance
		 */
		tuple_instance = ecm_db_multicast_tuple_instance_alloc(ip_src_addr, ip_dest_addr, src_port, dest_port);
		if (!tuple_instance) {
			ecm_db_connection_deref(nci);
			DEBUG_WARN("Failed to allocate tuple instance\n");
			goto done;
		}

		/*
		 * Create Destination MAC address using IP multicast destination address
		 */
		ecm_translate_multicast_mac(ip_dest_addr, dest_mac_addr);

		/*
		 * Get the src and destination mappings.
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 */
		DEBUG_TRACE("%p: Create the 'from' interface heirarchy list\n", nci);
		from_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_list, ip_dest_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, skb);
		if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			DEBUG_WARN("Failed to obtain 'from' heirarchy list\n");
			goto done;
		}
		ecm_db_connection_interfaces_reset(nci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);

		DEBUG_TRACE("%p: Establish source node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM] = ecm_nss_multicast_ipv4_node_establish_and_ref(feci, in_dev, ip_src_addr, from_list, from_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_list, from_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("%p: Failed to establish source node\n", nci);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			goto done;
		}

		DEBUG_TRACE("%p: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM] = ecm_nss_ipv4_mapping_establish_and_ref(ip_src_addr, src_port);
		if (!mi[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("%p: Failed to establish src mapping\n", nci);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			goto done;
		}

		to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
		if (!to_list) {
			DEBUG_WARN("%p: Failed to alloc memory for multicast dest interface hierarchies\n", nci);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			goto done;
		}

		to_list_first = (int32_t *)kzalloc(sizeof(int32_t *) * ECM_DB_MULTICAST_IF_MAX, GFP_ATOMIC | __GFP_NOWARN);
		if (!to_list_first) {
			DEBUG_WARN("%p: Failed to alloc memory for multicast dest interfaces first list\n", nci);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			goto done;
		}

		/*
		 * Create the multicast 'to' interface heirarchy
		 */
		DEBUG_TRACE("%p: Create the multicast  'to' interface heirarchy list\n", nci);
		for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
			to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
			*to_first = ECM_DB_IFACE_HEIRARCHY_MAX;
		}

		interface_idx_cnt = ecm_nss_multicast_connection_to_interface_heirarchy_construct(feci, to_list, ip_src_addr, ip_dest_addr, in_dev,
												  out_dev_master, if_cnt, dst_dev, to_list_first,
												  src_node_addr, is_routed, layer4hdr, skb);
		if (interface_idx_cnt == 0) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("%p: Failed to obtain mutlicast 'to' heirarchy list\n", nci);
			goto done;
		}
		ret = ecm_db_multicast_connection_to_interfaces_reset(nci, to_list, to_list_first);
		if (ret < 0) {
			for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
				to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, vif);
				ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
				ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
			}

			feci->deref(feci);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("%p: Failed to obtain mutlicast 'to' heirarchy list\n", nci);
			goto done;
		}

		DEBUG_TRACE("%p: Create destination node\n", nci);
		ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list);
		ni[ECM_DB_OBJ_DIR_TO] = ecm_nss_multicast_ipv4_node_establish_and_ref(feci, out_dev, ip_dest_addr, to_list_temp, *to_list_first, dest_mac_addr, skb);

		/*
		 * De-ref the Multicast destination interface list
		 */
		for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
			to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, vif);
			ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
			to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
			ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
		}

		if (!ni[ECM_DB_OBJ_DIR_TO]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish destination node\n");
			goto done;
		}
		ni[ECM_DB_OBJ_DIR_TO_NAT] = ni[ECM_DB_OBJ_DIR_TO];

		DEBUG_TRACE("%p: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_TO] = ecm_nss_ipv4_mapping_establish_and_ref(ip_dest_addr, dest_port);
		if (!mi[ECM_DB_OBJ_DIR_TO]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish dst mapping\n");
			goto done;
		}
		mi[ECM_DB_OBJ_DIR_TO_NAT] = mi[ECM_DB_OBJ_DIR_TO];

		DEBUG_TRACE("%p: Create the 'from NAT' interface heirarchy list\n", nci);
		from_nat_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_nat_list, ip_dest_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr, dest_node_addr, (__be16 *)&udp_hdr, skb);
		if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to obtain 'from NAT' heirarchy list\n");
			goto done;
		}
		ecm_db_connection_interfaces_reset(nci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);

		ni[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_nss_multicast_ipv4_node_establish_and_ref(feci, in_dev_nat, ip_src_addr_nat, from_nat_list, from_nat_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM_NAT]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to obtain 'from NAT' node\n");
			goto done;
		}

		DEBUG_TRACE("%p: Create from NAT mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_nss_ipv4_mapping_establish_and_ref(ip_src_addr_nat, src_port_nat);

		if (!mi[ECM_DB_OBJ_DIR_FROM_NAT]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish from nat mapping\n");
			goto done;
		}

		/*
		 * Every connection also needs a default classifier
		 */
		dci = ecm_classifier_default_instance_alloc(nci, protocol, ecm_dir, src_port, dest_port);
		if (!dci) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			feci->deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to allocate default classifier\n");
			goto done;
		}
		ecm_db_connection_classifier_assign(nci, (struct ecm_classifier_instance *)dci);

		/*
		 * Every connection starts with a full complement of classifiers assigned.
		 * NOTE: Default classifier is a special case considered previously
		 */
		for (classifier_type = ECM_CLASSIFIER_TYPE_DEFAULT + 1; classifier_type < ECM_CLASSIFIER_TYPES; ++classifier_type) {
			struct ecm_classifier_instance *aci = ecm_classifier_assign_classifier(nci, classifier_type);
			if (aci) {
				aci->deref(aci);
			} else {
				dci->base.deref((struct ecm_classifier_instance *)dci);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
				feci->deref(feci);
				ecm_db_connection_deref(nci);
				ecm_db_multicast_tuple_instance_deref(tuple_instance);
				kfree(to_list);
				kfree(to_list_first);
				DEBUG_WARN("Failed to allocate classifiers assignments\n");
				goto done;
			}
		}

		ecm_db_front_end_instance_ref_and_set(nci, feci);

		/*
		 * Now add the connection into the database.
		 * NOTE: In an SMP situation such as ours there is a possibility that more than one packet for the same
		 * connection is being processed simultaneously.
		 * We *could* end up creating more than one connection instance for the same actual connection.
		 * To guard against this we now perform a mutex'd lookup of the connection + add once more - another cpu may have created it before us.
		 */
		spin_lock_bh(&ecm_nss_ipv4_lock);
		ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);
		if (ci) {
			/*
			 * Another cpu created the same connection before us - use the one we just found
			 */
			spin_unlock_bh(&ecm_nss_ipv4_lock);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			ecm_db_connection_deref(nci);
		} else {
			struct ecm_tracker_instance *ti;
			ecm_db_timer_group_t tg;
			ecm_tracker_sender_state_t src_state;
			ecm_tracker_sender_state_t dest_state;
			ecm_tracker_connection_state_t state;

			/*
			 * Ask tracker for timer group to set the connection to initially.
			 */
			ti = dci->tracker_get_and_ref(dci);
			ti->state_get(ti, &src_state, &dest_state, &state, &tg);
			ti->deref(ti);

			/*
			 * Add the new connection we created into the database
			 * NOTE: assign to a short timer group for now - it is the assigned classifiers responsibility to do this
			 */
			ecm_db_connection_add(nci, mi, ni,
					4, protocol, ecm_dir,
					NULL /* final callback */,
					ecm_nss_multicast_ipv4_connection_defunct_callback,
					tg, is_routed, nci);

			/*
			 * Add the tuple instance and attach it with connection instance
			 */
			ecm_db_multicast_tuple_instance_add(tuple_instance, nci);
			if (br_dev_found_in_mfc) {
				ecm_db_multicast_tuple_instance_flags_set(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

			spin_unlock_bh(&ecm_nss_ipv4_lock);

			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			ci = nci;
			DEBUG_INFO("%p: New UDP multicast connection created\n", ci);
		}

		/*
		 * No longer need referenecs to the objects we created
		 */
		dci->base.deref((struct ecm_classifier_instance *)dci);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
		feci->deref(feci);
		kfree(to_list);
		kfree(to_list_first);
	} else {
		bool is_dest_interface_list_empty;

		/*
		 * At this pont the feci->accel_mode is DEACEL because the MC connection has expired
		 * and we had received a callback from MFC which had freed the multicast destination
		 * interface heirarchy. In this case, we reconstruct the multicastdestination interface
		 * heirarchy and re-accelerate the connection.
		 */
		is_dest_interface_list_empty = ecm_db_multicast_connection_to_interfaces_set_check(ci);
		if (!is_dest_interface_list_empty) {
			struct ecm_db_iface_instance *to_list;
			struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
			struct ecm_db_iface_instance *to_list_single;
			int32_t *to_list_first;
			int32_t *to_first;
			int32_t i, interface_idx_cnt;
			int ret;
			struct ecm_front_end_connection_instance *feci;

			to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list) {
				ecm_db_connection_deref(ci);
				goto done;
			}

			to_list_first = (int32_t *)kzalloc(sizeof(int32_t *) * ECM_DB_MULTICAST_IF_MAX, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list_first) {
				ecm_db_connection_deref(ci);
				kfree(to_list);
				goto done;
			}

			/*
			 * Reconstruct the multicast destination interface heirarchy.
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++ ) {
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, i);
				*to_first = ECM_DB_IFACE_HEIRARCHY_MAX;
			}

			feci = ecm_db_connection_front_end_get_and_ref(ci);
			interface_idx_cnt = ecm_nss_multicast_connection_to_interface_heirarchy_construct(feci, to_list, ip_src_addr, ip_dest_addr, in_dev,
													  out_dev_master, if_cnt, dst_dev, to_list_first,
													  src_node_addr, is_routed, (__be16 *)&udp_hdr, skb);
			feci->deref(feci);
			if (interface_idx_cnt == 0) {
				DEBUG_WARN("Failed to reconstruct 'to mc' heirarchy list\n");
				ecm_db_connection_deref(ci);
				kfree(to_list);
				kfree(to_list_first);
				goto done;
			}

			ret = ecm_db_multicast_connection_to_interfaces_reset(ci, to_list, to_list_first);

			/*
			 * De-ref the destination interface list
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
				ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, i);
				ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
			}

			/*
			 * if a bridge dev is present in the MFC destination then set the
			 * ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG in tuple_instance
			 */
			if (br_dev_found_in_mfc) {
				struct ecm_db_multicast_tuple_instance *tuple_instance;
				tuple_instance = ecm_db_multicast_connection_find_and_ref(ip_src_addr, ip_dest_addr);
				if (!tuple_instance) {
					ecm_db_connection_deref(ci);
					kfree(to_list);
					kfree(to_list_first);
					goto done;
				}

				ecm_db_multicast_tuple_instance_flags_set(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
				ecm_db_multicast_connection_deref(tuple_instance);
			}
			kfree(to_list);
			kfree(to_list_first);

			/*
			 * If ret is less than zero than connection reset could not find memory for
			 * to_mcast_interfaces. Deref the CI and retrun.
			 */
			if (ret < 0) {
				ecm_db_connection_deref(ci);
				goto done;
			}
		}
	}

	/*
	 * Keep connection alive as we have seen activity
	 */
	if (!ecm_db_connection_defunct_timer_touch(ci)) {
		ecm_db_connection_deref(ci);
		goto done;
	}

	/*
	 * Identify which side of the connection is sending
	 */
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, match_addr);
	if (ECM_IP_ADDR_MATCH(ip_src_addr, match_addr)) {
		sender = ECM_TRACKER_SENDER_TYPE_SRC;
	} else {
		sender = ECM_TRACKER_SENDER_TYPE_DEST;
	}

	/*
	 * Do we need to action generation change?
	 */
	if (unlikely(ecm_db_connection_regeneration_required_check(ci))) {
		ecm_nss_multicast_ipv4_connection_regenerate(ci, sender, out_dev, in_dev, in_dev_nat);
	}

	/*
	 * Iterate the assignments and call to process!
	 * Policy implemented:
	 * 1. Classifiers that say they are not relevant are unassigned and not actioned further.
	 * 2. Any drop command from any classifier is honoured.
	 * 3. All classifiers must action acceleration for accel to be honoured, any classifiers not sure of their relevance will stop acceleration.
	 * 4. Only the highest priority classifier, that actions it, will have its qos tag honoured.
	 * 5. Only the highest priority classifier, that actions it, will have its timer group honoured.
	 */
	DEBUG_TRACE("%p: process begin, skb: %p\n", ci, skb);
	prevalent_pr.drop = false;
	prevalent_pr.flow_qos_tag = skb->priority;
	prevalent_pr.return_qos_tag = skb->priority;
	prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	prevalent_pr.timer_group = ci_orig_timer_group = ecm_db_connection_timer_group_get(ci);
	prevalent_pr.process_actions = 0;
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_process_response aci_pr;
		struct ecm_classifier_instance *aci;

		aci = assignments[aci_index];
		DEBUG_TRACE("%p: process: %p, type: %d\n", ci, aci, aci->type_get(aci));
		aci->process(aci, sender, iph, skb, &aci_pr);
		DEBUG_TRACE("%p: aci_pr: process actions: %x, became relevant: %u, relevance: %d, drop: %d, "
				"flow_qos_tag: %u, return_qos_tag: %u, accel_mode: %x, timer_group: %d\n",
				ci, aci_pr.process_actions, aci_pr.became_relevant, aci_pr.relevance, aci_pr.drop,
				aci_pr.flow_qos_tag, aci_pr.return_qos_tag, aci_pr.accel_mode, aci_pr.timer_group);

		if (aci_pr.relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
			ecm_classifier_type_t aci_type;

			/*
			 * This classifier can be unassigned - PROVIDED it is not the default classifier
			 */
			aci_type = aci->type_get(aci);
			if (aci_type == ECM_CLASSIFIER_TYPE_DEFAULT) {
				continue;
			}

			DEBUG_INFO("%p: Classifier not relevant, unassign: %d", ci, aci_type);
			ecm_db_connection_classifier_unassign(ci, aci);
			continue;
		}

		/*
		 * Yes or Maybe relevant.
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DROP) {
			/*
			 * Drop command from any classifier is actioned.
			 */
			DEBUG_TRACE("%p: wants drop: %p, type: %d, skb: %p\n", ci, aci, aci->type_get(aci), skb);
			prevalent_pr.drop |= aci_pr.drop;
		}

		/*
		 * Accel mode permission
		 */
		if (aci_pr.relevance == ECM_CLASSIFIER_RELEVANCE_MAYBE) {
			/*
			 * Classifier not sure of its relevance - cannot accel yet
			 */
			DEBUG_TRACE("%p: accel denied by maybe: %p, type: %d\n", ci, aci, aci->type_get(aci));
			prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
		} else {
			if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE) {
				if (aci_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_NO) {
					DEBUG_TRACE("%p: accel denied: %p, type: %d\n", ci, aci, aci->type_get(aci));
					prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				}
				/* else yes or don't care about accel */
			}
		}

		/*
		 * Timer group (the last classifier i.e. the highest priority one) will 'win'
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_TIMER_GROUP) {
			DEBUG_TRACE("%p: timer group: %p, type: %d, group: %d\n", ci, aci, aci->type_get(aci), aci_pr.timer_group);
			prevalent_pr.timer_group = aci_pr.timer_group;
		}

		/*
		 * Qos tag (the last classifier i.e. the highest priority one) will 'win'
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG) {
			DEBUG_TRACE("%p: aci: %p, type: %d, flow qos tag: %u, return qos tag: %u\n",
					ci, aci, aci->type_get(aci), aci_pr.flow_qos_tag, aci_pr.return_qos_tag);
			prevalent_pr.flow_qos_tag = aci_pr.flow_qos_tag;
			prevalent_pr.return_qos_tag = aci_pr.return_qos_tag;
		}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
		/*
		 * If any classifier denied DSCP remarking then that overrides every classifier
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY) {
			DEBUG_TRACE("%p: aci: %p, type: %d, DSCP remark denied\n",
					ci, aci, aci->type_get(aci));
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY;
			prevalent_pr.process_actions &= ~ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
		}

		/*
		 * DSCP remark action, but only if it has not been denied by any classifier
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
			if (!(prevalent_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY)) {
				DEBUG_TRACE("%p: aci: %p, type: %d, DSCP remark wanted, flow_dscp: %u, return dscp: %u\n",
						ci, aci, aci->type_get(aci), aci_pr.flow_dscp, aci_pr.return_dscp);
				prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
				prevalent_pr.flow_dscp = aci_pr.flow_dscp;
				prevalent_pr.return_dscp = aci_pr.return_dscp;
			}
		}
#endif
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Change timer group?
	 */
	if (ci_orig_timer_group != prevalent_pr.timer_group) {
		DEBUG_TRACE("%p: change timer group from: %d to: %d\n", ci, ci_orig_timer_group, prevalent_pr.timer_group);
		ecm_db_connection_defunct_timer_reset(ci, prevalent_pr.timer_group);
	}

	/*
	 * Drop?
	 */
	if (prevalent_pr.drop) {
		DEBUG_TRACE("%p: drop: %p\n", ci, skb);
		ecm_db_connection_data_totals_update_dropped(ci, (sender == ECM_TRACKER_SENDER_TYPE_SRC)? true : false, skb->len, 1);
		ecm_db_connection_deref(ci);
		goto done;
	}
	ecm_db_multicast_connection_data_totals_update(ci, false, skb->len, 1);

	/*
	 * Accelerate?
	 */
	if (prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		struct ecm_front_end_connection_instance *feci;
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		ecm_nss_multicast_ipv4_connection_accelerate(feci, &prevalent_pr);
		feci->deref(feci);
	}
	ecm_db_connection_deref(ci);

done:
	if (out_dev_master) {
		dev_put(out_dev_master);
	}
	return NF_ACCEPT;
}

/*
 * ecm_nss_multicast_ipv4_get_accelerated_count()
 */
ssize_t ecm_nss_multicast_ipv4_get_accelerated_count(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	ssize_t count;
	int num;

	/*
	 * Operate under our locks
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	num = ecm_nss_multicast_ipv4_accelerated_count;
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
	return count;
}

/*
 * ecm_br_multicast_update_event_callback()
 * 	Callback received from bridge multicast snooper module in the
 * 	following events:
 * 	a.) Updates to a muticast group due to IGMP JOIN/LEAVE.
 *
 * 	group: IP multicast group address
 * 	brdev: bridge netdevice
 *
 * This function does the following:
 * a.) Lookup the tuple_instance table to find any flows through the bridge for the group
 * b.) Query the MCS bridge using the source and group address, to receive a list
 *     of bridge slaves for the flow
 * c.) Process the updates by matching the received bridge slave list with existing
 *     destination interfaces heirarchy for the CI, and decide whether to delete an
 *     exiting interface heirarchy or add a new heirarchy.
 */
static void ecm_br_multicast_update_event_callback(struct net_device *brdev, uint32_t group)
{
	struct ecm_db_multicast_tuple_instance *tuple_instance;
	struct ecm_db_multicast_tuple_instance *tuple_instance_next;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_multicast_if_update mc_update;
	struct ecm_db_iface_instance *ii;
	struct ecm_db_iface_instance *to_list;
	struct ecm_db_iface_instance *to_list_single;
	struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	ecm_db_iface_type_t ii_type;
	ip_addr_t dest_ip;
	ip_addr_t grp_ip;
	ip_addr_t src_ip;
	int32_t to_list_first[ECM_DB_MULTICAST_IF_MAX];
	int32_t from_ifaces_first;
	int32_t from_iface_identifier;
	int i, ret;
	uint32_t mc_dst_dev[ECM_DB_MULTICAST_IF_MAX];
	uint32_t if_cnt;
	int32_t if_num;
	uint32_t mc_flags = 0;
	uint32_t mc_max_dst = ECM_DB_MULTICAST_IF_MAX;
	uint8_t src_node_addr[ETH_ALEN];
	bool if_update;
	uint32_t tuple_instance_flags;
	bool is_routed;
	__be16 layer4hdr[2] = {0, 0};
	__be16 port = 0;
	int mc_to_interface_count = 0;

	DEBUG_TRACE("ecm_br_multicast_update_event_callback 0x%x\n", group);

	ECM_HIN4_ADDR_TO_IP_ADDR(dest_ip, htonl(group));

	/*
	 * Get the first entry for the group in the tuple_instance table
	 */
	tuple_instance = ecm_db_multicast_connection_get_and_ref_first(dest_ip);
	if (!tuple_instance) {
		DEBUG_TRACE("ecm_br_multicast_update_event_callback: no multicast tuple entry found\n");
		return;
	}

	while (tuple_instance) {
		/*
		 * We now have a 5-tuple which has been accelerated. Query the MCS bridge to receive a list
		 * of interfaces left or joined a group for a source.
		 */
		memset(mc_dst_dev, 0, sizeof(mc_dst_dev));

		/*
		 * Get the group IP address stored in tuple_instance and match this with
		 * the group IP received from MCS update callback.
		 */
		ecm_db_multicast_tuple_instance_group_ip_get(tuple_instance, grp_ip);
		if (!ECM_IP_ADDR_MATCH(grp_ip, dest_ip)) {
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
			continue;
		}

		/*
		 * Get the source IP address for this entry for the group
		 */
		ecm_db_multicast_tuple_instance_source_ip_get(tuple_instance, src_ip);

		/*
		 * Query bridge snooper for the destination list when given the group and source
		 * if, 	if_num < 0   mc_bridge_ipv4_get_if has encountered with some error, return immediately
		 * 	if_num = 0  All slaves has left the group. Deacel the flow.
		 * 	if_num > 0   An interface leave/Join the group. Process the leave/join interface request.
		 */
		if_num = mc_bridge_ipv4_get_if(brdev, htonl(src_ip[0]), htonl(dest_ip[0]), mc_max_dst, mc_dst_dev);
		if (if_num < 0) {
			DEBUG_TRACE("No valid bridge slaves for the group/source\n");

			/*
			 * This may a valid case when all the interface has left a multicast group.
			 * In this case the MCS will return if_num 0, But we may have an oudated
			 * interface in multicast interface heirarchy list. At next step we have to
			 * check whether the DB instance is present or not.
			 */
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
			continue;
		}

		/*
		 * Get a DB connection instance for the 5-tuple
		 */
		ci = ecm_db_multicast_connection_get_from_tuple(tuple_instance);
		DEBUG_TRACE("MCS-cb: src_ip = 0x%x, dest_ip = 0x%x, Num if = %d\n", src_ip[0], dest_ip[0], if_num);
		is_routed = ecm_db_connection_is_routed_get(ci);

		/*
		 * The source interface could have joined the group as well.
		 * In such cases, the destination interface list returned by
		 * the snooper would include the source interface as well.
		 * We need to filter the source interface from the list in such cases.
		 */
		if (if_num > 0) {
			/*
			 * Get the interface lists of the connection, we must have at least one interface in the list to continue
			 */
			from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
			if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
				DEBUG_WARN("%p: MCS Snooper Update: no interfaces in from_interfaces list!\n", ci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			ii = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
			ii_type = ecm_db_iface_type_get(ii);
			if (ii_type == ECM_DB_IFACE_TYPE_BRIDGE) {
				ii = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 2];
			}

			from_iface_identifier = ecm_db_iface_interface_identifier_get(ii);
			if_num = ecm_interface_multicast_check_for_src_ifindex(mc_dst_dev, if_num, from_iface_identifier);
			ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		}

		/*
		 * All bridge slaves has left the group. If flow is pure bridge, Decel the connection and return.
		 * If flow is routed, let MFC callback handle this.
		 */
		if (if_num == 0) {
			/*
			 * If there are no routed interfaces, then decelerate. Else
			 * we first send an update message to the firmware for the
			 * interface that have left, before issuing a decelerate
			 * at a later point via the MFC callback. This is because
			 * there might be a few seconds delay before MFC issues
			 * the delete callback
			 */
			if (!is_routed) {
				/*
				 * Decelerate the flow
				 */
				feci = ecm_db_connection_front_end_get_and_ref(ci);
				feci->decelerate(feci);
				feci->deref(feci);

				/*
				 * Get next multicast connection instance
				 */
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}
		}

		memset(&mc_update, 0, sizeof(mc_update));
		spin_lock_bh(&ecm_nss_ipv4_lock);

		/*
		 * Find out changes to the destination interfaces heirarchy
		 * of the connection. We try to find out the interfaces that
		 * have joined new, and the existing interfaces in the list
		 * that have left seperately.
		 */
		if_update = ecm_interface_multicast_find_updates_to_iface_list(ci, &mc_update, mc_flags, true, mc_dst_dev, if_num);
		if (!if_update) {
			/*
			 * No updates to this multicast flow. Move on to the next
			 * flow for the same group
			 */
			spin_unlock_bh(&ecm_nss_ipv4_lock);
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
			continue;
		}

		spin_unlock_bh(&ecm_nss_ipv4_lock);
		DEBUG_TRACE("BRIDGE UPDATE callback ===> leave_cnt %d, join_cnt %d\n", mc_update.if_leave_cnt, mc_update.if_join_cnt);

		feci = ecm_db_connection_front_end_get_and_ref(ci);

		/*
		 * Do we have any new interfaces that have joined?
		 */
		if (mc_update.if_join_cnt > 0) {
			to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list) {
				feci->deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			/*
			 * Initialize the heirarchy's indices for the 'to_list'
			 * which will hold the interface heirarchies for the new joinees
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				to_list_first[i] = ECM_DB_IFACE_HEIRARCHY_MAX;
			}

			ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_node_addr);

			/*
			 * Create the interface heirarchy list for the new interfaces. We append this list later to
			 * the existing list of destination interfaces.
			 */
			port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
			layer4hdr[0] = htons(port);
			port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));
			layer4hdr[1] = htons(port);

			if_cnt = ecm_interface_multicast_heirarchy_construct_bridged(feci, to_list, brdev, src_ip, dest_ip, mc_update.if_join_cnt, mc_update.join_dev, to_list_first, src_node_addr, layer4hdr, NULL);
			if (if_cnt == 0) {
				DEBUG_WARN("Failed to obtain 'to_mcast_update' heirarchy list\n");
				feci->decelerate(feci);
				feci->deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				kfree(to_list);
				break;
			}

			/*
			 * Append the interface heirarchy array of the new joinees to the existing destination list
			 */
			ecm_db_multicast_connection_to_interfaces_update(ci, to_list, to_list_first, mc_update.if_join_idx);

			/*
			 * In routed + Bridge mode, if there is a group leave request arrives for the last
			 * slave of the bridge then MFC will clear ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG
			 * in tuple_instance. If the bridge slave joins again then we need to set the flag again
			 * in tuple_instance here.
			 */
			tuple_instance_flags = ecm_db_multicast_tuple_instance_flags_get(tuple_instance);
			if (is_routed && !(tuple_instance_flags & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG)) {
				ecm_db_multicast_tuple_instance_flags_set(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

			/*
			 * De-ref the updated destination interface list
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				if (mc_update.if_join_idx[i]) {
					to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
					ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
					ecm_db_connection_interfaces_deref(to_list_temp, to_list_first[i]);
				}
			}

			kfree(to_list);
		} else if (mc_update.if_leave_cnt > 0) {
			/*
			 * If these are the last interface set leaving the to interface
			 * list of the connection, then decelerate the connection
			 */
			mc_to_interface_count = ecm_db_multicast_connection_to_interfaces_get_count(ci);
			if (mc_update.if_leave_cnt == mc_to_interface_count) {
				DEBUG_INFO("%p: Decelerating the flow as there are no to interfaces in the multicast group: 0x%x\n", feci, dest_ip[0]);
				feci->decelerate(feci);
				feci->deref(feci);

				/*
				 * Get next multicast connection instance
				 */
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}
		}

		/*
		 * Push the updates to NSS
		 */
		DEBUG_TRACE("%p: Update accel\n", ci);
		if ((feci->accel_mode <= ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED) ||
				(feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
			DEBUG_TRACE("%p: Ignoring wrong mode accel for conn: %p\n", feci, feci->ci);
			feci->deref(feci);
			ecm_db_multicast_connection_deref(tuple_instance);
			return;
		}

		/*
		 * Update the new rules in FW. If returns error decelerate the connection
		 * and flush all ECM rules.
		 */
		ret = ecm_nss_multicast_ipv4_connection_update_accelerate(feci, &mc_update);
		if (ret < 0) {
			feci->decelerate(feci);
			feci->deref(feci);
			ecm_db_multicast_connection_deref(tuple_instance);
			return;
		}

		feci->deref(feci);

		/*
		 * Release the interfaces that may have left the connection
		 */
		for (i = 0; i < ECM_DB_MULTICAST_IF_MAX && mc_update.if_leave_cnt; i++) {
			/*
			 * Is this entry marked? If yes, then the corresponding entry
			 * in the 'to_mcast_interfaces' array in the ci has left the
			 * connection
			 */
			if (mc_update.if_leave_idx[i]) {
				/*
				 * Release the interface heirarchy for this
				 * interface since it has left the group
				 */
				ecm_db_multicast_connection_to_interfaces_clear_at_index(ci, i);
				mc_update.if_leave_cnt--;
			}
		}

		tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
		ecm_db_multicast_connection_deref(tuple_instance);
		tuple_instance = tuple_instance_next;
	}

	return;
}

/*
 * ecm_mfc_update_event_callback()
 * 	Callback called by Linux kernel multicast routing module in the
 * 	following events:
 * 	a.) Updates to destination interface lists for a multicast route due to
 * 	    IGMP JOIN/LEAVE
 * 	b.) Multicast route delete event
 *
 * 	group: IP multicast group address
 *	origin: IP source for the multicast route
 *	to_dev_idx[]: Array of ifindex for destination interfaces for the route
 *	max_to_dev: size of the array
 *	op: UPDATE or DELETE event
 *
 * This function does the following:
 * a.) Lookup the tuple_instance table to find any accelerated flows for the group/source,
 *     and access their CI
 * b.) Process the updates by matching the received destination interface list with existing
 *     destination interfaces heirarchy for the CI, and decide whether to delete an
 *     exiting interface heirarchy or add a new heirarchy.
 */
static void ecm_mfc_update_event_callback(__be32 group, __be32 origin, uint32_t max_to_dev, uint32_t to_dev_idx[], uint8_t op)
{
	struct ecm_db_connection_instance *ci;
	struct ecm_db_multicast_tuple_instance *tuple_instance;
	struct ecm_db_multicast_tuple_instance *tuple_instance_next;
	struct ecm_multicast_if_update mc_update;
	int32_t vif_cnt;
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	ip_addr_t src_addr;
	ip_addr_t grp_addr;
	uint32_t tuple_instance_flags = 0;
	__be16 layer4hdr[2] = {0, 0};
	__be16 port = 0;

	ECM_HIN4_ADDR_TO_IP_ADDR(src_ip, htonl(origin));
	ECM_HIN4_ADDR_TO_IP_ADDR(dest_ip, htonl(group));

	DEBUG_TRACE("origin : group " ECM_IP_ADDR_DOT_FMT ":" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(src_ip), ECM_IP_ADDR_TO_DOT(dest_ip));

	/*
	 * Access the 5-tuple information from the tuple_instance table, using the
	 * source and group addresses
	 */
	tuple_instance = ecm_db_multicast_connection_find_and_ref(src_ip, dest_ip);
	if (!tuple_instance) {
		DEBUG_TRACE("MFC_EVENT: Tuple info is not found\n");
		return;
	}

	switch (op) {
	case IPMR_MFC_EVENT_UPDATE:
	{
		struct ecm_front_end_connection_instance *feci;
		struct ecm_db_iface_instance *to_list;
		struct ecm_db_iface_instance *to_list_single;
		struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
		uint32_t to_list_first[ECM_DB_MULTICAST_IF_MAX];
		uint32_t i, ret;
		uint32_t mc_flag = 0;
		bool if_update = false;
		bool found_br_dev = false;

		/*
		 * Find the connections belongs to the same source and group address and
		 * apply the updates received from event handler
		 */
		while (tuple_instance) {
			/*
			 * Get the source/group IP address for this multicast tuple and match
			 * with the source and group IP received from the event handler. If
			 * there is not match found jumps to the next multicast tuple.
			 */
			ecm_db_multicast_tuple_instance_source_ip_get(tuple_instance, src_addr);
			ecm_db_multicast_tuple_instance_group_ip_get(tuple_instance, grp_addr);
			if (!(ECM_IP_ADDR_MATCH(src_addr, src_ip) && ECM_IP_ADDR_MATCH(grp_addr, dest_ip))) {
				DEBUG_TRACE("%p: Multicast tuple not matched, try next multicast tuple %d\n", tuple_instance, op);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			/*
			 * Get the DB connection instance using the tuple_instance, ref to 'ci'
			 * has been already taken by ecm_db_multicast_connection_find_and_ref()
			 */
			ci = ecm_db_multicast_connection_get_from_tuple(tuple_instance);
			DEBUG_TRACE("%p: update the multicast flow: %p\n", ci, tuple_instance);

			/*
			 * Check if this multicast connection has a bridge
			 * device in multicast destination interface list.
			 */
			tuple_instance_flags = ecm_db_multicast_tuple_instance_flags_get(tuple_instance);
			if ((tuple_instance_flags & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG)) {
				mc_flag |= ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG;
			}

			memset(&mc_update, 0, sizeof(mc_update));
			spin_lock_bh(&ecm_nss_ipv4_lock);

			/*
			 * Find out if any change in destination interfaces heirarchy
			 * list. We try to find out the interfaces that
			 * have joined new, and the existing interfaces in the list
			 * that have left seperately.
			 */
			if_update = ecm_interface_multicast_find_updates_to_iface_list(ci, &mc_update, mc_flag, false, to_dev_idx, max_to_dev);
			if (!if_update) {
				DEBUG_TRACE("%p: no update, check next multicast tuple: %p\n", ci, tuple_instance);
				spin_unlock_bh(&ecm_nss_ipv4_lock);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			found_br_dev = ecm_interface_multicast_check_for_br_dev(to_dev_idx, max_to_dev);
			if (!found_br_dev) {
				ecm_db_multicast_tuple_instance_flags_clear(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

			spin_unlock_bh(&ecm_nss_ipv4_lock);
			DEBUG_TRACE("%p: MFC update callback leave_cnt %d, join_cnt %d\n", ci, mc_update.if_leave_cnt, mc_update.if_join_cnt);

			feci = ecm_db_connection_front_end_get_and_ref(ci);

			/*
			 * Do we have any new interfaces that have joined?
			 */
			if (mc_update.if_join_cnt > 0) {
				to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
				if (!to_list) {
					feci->deref(feci);
					ecm_db_multicast_connection_deref(tuple_instance);
					return;
				}

				/*
				 * Initialize the heirarchy's indices for the 'to_list'
				 * which will hold the interface heirarchies for the new joinees
				 */
				for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
					to_list_first[i] = ECM_DB_IFACE_HEIRARCHY_MAX;
				}

				/*
				 * Create the interface heirarchy list for the new interfaces. We append this list later to
				 * the existing list of destination interfaces.
				 */
				port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
				layer4hdr[0] = htons(port);
				port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));
				layer4hdr[1] = htons(port);

				vif_cnt = ecm_interface_multicast_heirarchy_construct_routed(feci, to_list, NULL, src_ip, dest_ip, mc_update.if_join_cnt, mc_update.join_dev, to_list_first, true, layer4hdr, NULL);
				if (vif_cnt == 0) {
					DEBUG_WARN("Failed to obtain 'to_mcast_update' heirarchy list\n");
					feci->decelerate(feci);
					feci->deref(feci);
					ecm_db_multicast_connection_deref(tuple_instance);
					kfree(to_list);
					return;
				}

				/*
				 * Append the interface heirarchy array of the new joinees to the existing destination list
				 */
				ecm_db_multicast_connection_to_interfaces_update(ci, to_list, to_list_first, mc_update.if_join_idx);

				/*
				 * De-ref the updated destination interface list
				 */
				for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
					if (mc_update.if_join_idx[i]) {
						to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
						ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
						ecm_db_connection_interfaces_deref(to_list_temp, to_list_first[i]);
					}
				}

				kfree(to_list);
			}

			/*
			 * Push the updates to NSS
			 */
			DEBUG_TRACE("%p: Update accel %p\n", ci, tuple_instance);
			if ((feci->accel_mode <= ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED) ||
					(feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
				DEBUG_TRACE("%p: Ignoring wrong mode accel for conn: %p\n", feci, feci->ci);
				feci->deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			/*
			 * Update the new rules in FW. If returns error decelerate the connection
			 * and flush Multicast rule.
			 */
			ret = ecm_nss_multicast_ipv4_connection_update_accelerate(feci, &mc_update);
			if (ret < 0) {
				feci->decelerate(feci);
				feci->deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			feci->deref(feci);

			/*
			 * Release the interfaces that may have left the connection
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX && mc_update.if_leave_cnt; i++) {
				/*
				 * Is this entry marked? If yes, then the corresponding entry
				 * in the 'to_mcast_interfaces' array in the ci has left the
				 * connection
				 */
				if (!mc_update.if_leave_idx[i]) {
					continue;
				}

				/*
				 * Release the interface heirarchy for this
				 * interface since it has left the group
				 */
				ecm_db_multicast_connection_to_interfaces_clear_at_index(ci, i);
				mc_update.if_leave_cnt--;
			}

			/*
			 * Move on to the next flow for the same source and group
			 */
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
		}
		break;
	}
	case IPMR_MFC_EVENT_DELETE:
	{
		struct ecm_front_end_connection_instance *feci;

		/*
		 * MFC reports that the multicast connection has died.
		 * Find the connections belongs to the same source/group address,
		 * Decelerate connections and free the frontend instance
		 */
		while (tuple_instance) {
			/*
			 * Get the source/group IP address for this multicast tuple and match
			 * with the source and group IP received from the event handler. If
			 * there is not match found jumps to the next multicast tuple.
			 */
			ecm_db_multicast_tuple_instance_source_ip_get(tuple_instance, src_addr);
			ecm_db_multicast_tuple_instance_group_ip_get(tuple_instance, grp_addr);
			if (!(ECM_IP_ADDR_MATCH(src_addr, src_ip) && ECM_IP_ADDR_MATCH(grp_addr, dest_ip))) {
				DEBUG_TRACE("%p: Multicast tuple not matched, try next tuple %d\n", tuple_instance, op);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			/*
			 * Get the DB connection instance using the tuple_instance, ref to 'ci'
			 * has been already taken by ecm_db_multicast_connection_find_and_ref()
			 */
			ci = ecm_db_multicast_connection_get_from_tuple(tuple_instance);
			DEBUG_TRACE("%p:%d delete the multicast flow: %p\n", ci, op, tuple_instance);

			/*
			 * Get the front end instance
			 */
			feci = ecm_db_connection_front_end_get_and_ref(ci);
			feci->decelerate(feci);
			feci->deref(feci);

			/*
			 * Move on to the next flow for the same source and group
			 */
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
		}
		break;
	}
	default:
		DEBUG_TRACE("Wrong op, shouldn't be here\n");
		break;
	}

	return;
}

/*
 * ecm_nss_multicast_ipv4_debugfs_init()
 */
bool ecm_nss_multicast_ipv4_debugfs_init(struct dentry *dentry)
{
	struct dentry *multicast_dentry;

	multicast_dentry = debugfs_create_u32("multicast_accelerated_count", S_IRUGO, dentry,
						&ecm_nss_multicast_ipv4_accelerated_count);
	if (!multicast_dentry) {
		DEBUG_ERROR("Failed to create ecm nss ipv4 multicast_accelerated_count file in debugfs\n");
		return false;
	}

	return true;
}

/*
 * ecm_nss_multicast_ipv4_stop()
 */
void ecm_nss_multicast_ipv4_stop(int num)
{
	ecm_front_end_ipv4_mc_stopped = num;
}

/*
 * ecm_nss_multicast_ipv4_init()
 * 	Register the callbacks for MCS snooper and MFC update
 */
int ecm_nss_multicast_ipv4_init(struct dentry *dentry)
{
	debugfs_create_u32("ecm_nss_multicast_ipv4_stop", S_IRUGO | S_IWUSR, dentry,
					(u32 *)&ecm_front_end_ipv4_mc_stopped);

	/*
	 * Register multicast update callback to MCS snooper
	 */
	mc_bridge_ipv4_update_callback_register(ecm_br_multicast_update_event_callback);

	/*
	 * Register multicast update callbacks to MFC
	 */
	ipmr_register_mfc_event_offload_callback(ecm_mfc_update_event_callback);
	return 0;
}

/*
 * ecm_nss_multicast_ipv4_exit()
 * 	De-register the callbacks for MCS snooper and MFC update
 */
void ecm_nss_multicast_ipv4_exit(void)
{
	/*
	 * De-register multicast update callbacks to
	 * MFC and MCS snooper
	 */
	ipmr_unregister_mfc_event_offload_callback();
	mc_bridge_ipv4_update_callback_deregister();
}
