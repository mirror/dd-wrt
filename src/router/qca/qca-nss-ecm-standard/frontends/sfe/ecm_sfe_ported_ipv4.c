/*
 **************************************************************************
 * Copyright (c) 2015-2019 The Linux Foundation.  All rights reserved.
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
#include <linux/kthread.h>
#include <linux/pkt_sched.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>

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
#include <linux/netfilter/nf_conntrack_tftp.h>
#ifdef ECM_INTERFACE_VLAN_ENABLE
#include <linux/../../net/8021q/vlan.h>
#include <linux/if_vlan.h>
#endif

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_SFE_PORTED_IPV4_DEBUG_LEVEL

#include <sfe_drv.h>

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
#include "ecm_sfe_ported_ipv4.h"
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_common.h"
#include "ecm_front_end_common.h"

/*
 * Magic numbers
 */
#define ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC 0xED56

/*
 * Protocol type that ported file supports.
 */
enum ecm_sfe_ported_ipv4_proto_types {
	ECM_SFE_PORTED_IPV4_PROTO_TCP = 0,
	ECM_SFE_PORTED_IPV4_PROTO_UDP,
	ECM_SFE_PORTED_IPV4_PROTO_MAX

};

/*
 * struct ecm_sfe_ipv4_ported_connection_instance
 *	A connection specific front end instance for PORTED connections
 */
struct ecm_sfe_ported_ipv4_connection_instance {
	struct ecm_front_end_connection_instance base;		/* Base class */
	uint8_t ported_accelerated_count_index;			/* Index value of accelerated count array (UDP or TCP) */
#ifdef CONFIG_XFRM
	enum ecm_sfe_ipsec_state flow_ipsec_state;	/* Flow traffic need ipsec process or not */
	enum ecm_sfe_ipsec_state return_ipsec_state;	/* Return traffic need ipsec process or not */
#endif
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

static int ecm_sfe_ported_ipv4_accelerated_count[ECM_SFE_PORTED_IPV4_PROTO_MAX] = {0};
						/* Array of Number of TCP and UDP connections currently offloaded */

/*
 * Expose what should be a static flag in the TCP connection tracker.
 */
#ifdef ECM_OPENWRT_SUPPORT
extern int nf_ct_tcp_no_window_check;
#endif
extern int nf_ct_tcp_be_liberal;

/*
 * ecm_sfe_ported_ipv4_connection_callback()
 *	Callback for handling create ack/nack calls.
 */
static void ecm_sfe_ported_ipv4_connection_callback(void *app_data, struct sfe_ipv4_msg *nim)
{
	struct sfe_ipv4_rule_create_msg * __attribute__((unused)) nircm = &nim->msg.rule_create;
	uint32_t serial = (uint32_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_sfe_ported_ipv4_connection_instance *npci;
	ecm_front_end_acceleration_mode_t result_mode;

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != SFE_TX_CREATE_RULE_MSG) {
		DEBUG_ERROR("%p: ported create callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
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
	npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	/*
	 * Record command duration
	 */
	ecm_sfe_ipv4_accel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: accelerate response for connection: %p, serial: %u\n", npci, feci->ci, serial);
	DEBUG_TRACE("%p: rule_flags: %x, valid_flags: %x\n", npci, nircm->rule_flags, nircm->valid_flags);
	DEBUG_TRACE("%p: flow_ip: %pI4n:%d\n", npci, &nircm->tuple.flow_ip, nircm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4n:%d\n", npci, &nircm->tuple.return_ip, nircm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", npci, nircm->tuple.protocol);

	/*
	 * Handle the creation result code.
	 */
	DEBUG_TRACE("%p: response: %d\n", npci, nim->cm.response);
	if (nim->cm.response != SFE_CMN_RESPONSE_ACK) {
		/*
		 * Creation command failed (specific reason ignored).
		 */
		DEBUG_TRACE("%p: accel nack: %d\n", npci, nim->cm.error);
		spin_lock_bh(&feci->lock);
		DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Unexpected mode: %d\n", ci, feci->accel_mode);
		feci->stats.ae_nack++;
		feci->stats.ae_nack_total++;
		if (feci->stats.ae_nack >= feci->stats.ae_nack_limit) {
			/*
			 * Too many SFE rejections
			 */
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_ACCEL_ENGINE;
		} else {
			/*
			 * Revert to decelerated
			 */
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
		}

		/*
		 * If connection is now defunct then set mode to ensure no further accel attempts occur
		 */
		if (feci->is_defunct) {
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT;
		}

		spin_lock_bh(&ecm_sfe_ipv4_lock);
		_ecm_sfe_ipv4_accel_pending_clear(feci, result_mode);
		spin_unlock_bh(&ecm_sfe_ipv4_lock);

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
	 * GGG TODO This is a workaround for a SFE message OOO quirk, this should eventually be removed.
	 */
	if (feci->stats.flush_happened) {
		feci->stats.flush_happened = false;

		/*
		 * Increment the no-action counter.  Our connection was decelerated on us with no action occurring.
		 */
		feci->stats.no_action_seen++;

		spin_lock_bh(&ecm_sfe_ipv4_lock);
		_ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		spin_unlock_bh(&ecm_sfe_ipv4_lock);

		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	/*
	 * Create succeeded
	 */

	/*
	 * Clear any nack count
	 */
	feci->stats.ae_nack = 0;

	/*
	 * Clear the "accelerate pending" state and move to "accelerated" state bumping
	 * the accelerated counters to match our new state.
	 *
	 * Decelerate may have been attempted while we were "pending accel" and
	 * this function will return true if that was the case.
	 * If decelerate was pending then we need to begin deceleration :-(
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);

	ecm_sfe_ported_ipv4_accelerated_count[npci->ported_accelerated_count_index]++;	/* Protocol specific counter */
	ecm_sfe_ipv4_accelerated_count++;		/* General running counter */

	if (!_ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
		/*
		 * Increment the no-action counter, this is reset if offload action is seen
		 */
		feci->stats.no_action_seen++;

		spin_unlock_bh(&ecm_sfe_ipv4_lock);
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_INFO("%p: Decelerate was pending\n", ci);

	spin_unlock_bh(&ecm_sfe_ipv4_lock);
	spin_unlock_bh(&feci->lock);

	feci->decelerate(feci);

	/*
	 * Release the connection.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_sfe_ported_ipv4_connection_accelerate()
 *	Accelerate a connection
 */
static void ecm_sfe_ported_ipv4_connection_accelerate(struct ecm_front_end_connection_instance *feci,
									struct ecm_classifier_process_response *pr, bool is_l2_encap,
									struct nf_conn *ct)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;
	uint16_t regen_occurrances;
	int protocol;
	int32_t from_ifaces_first;
	int32_t to_ifaces_first;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_sfe_iface;
	struct ecm_db_iface_instance *to_sfe_iface;
	int32_t from_sfe_iface_id;
	int32_t to_sfe_iface_id;
	uint8_t from_sfe_iface_address[ETH_ALEN];
	uint8_t to_sfe_iface_address[ETH_ALEN];
	ip_addr_t addr;
	struct sfe_ipv4_msg nim;
	struct sfe_ipv4_rule_create_msg *nircm;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	sfe_tx_status_t sfe_tx_status;
	int32_t list_index;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	uint8_t dest_mac_xlate[ETH_ALEN];
	ecm_db_direction_t ecm_dir;
	ecm_front_end_acceleration_mode_t result_mode;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	/*
	 * Test if acceleration is permitted
	 */
	if (!ecm_sfe_ipv4_accel_pending_set(feci)) {
		DEBUG_TRACE("%p: Acceleration not permitted: %p\n", feci, feci->ci);
		return;
	}

	/*
	 * Okay construct an accel command.
	 * Initialise creation structure.
	 * NOTE: We leverage the app_data void pointer to be our 32 bit connection serial number.
	 * When we get it back we re-cast it to a uint32 and do a faster connection lookup.
	 */
	memset(&nim, 0, sizeof(struct sfe_ipv4_msg));
	sfe_ipv4_msg_init(&nim, SFE_SPECIAL_INTERFACE_IPV4, SFE_TX_CREATE_RULE_MSG,
			sizeof(struct sfe_ipv4_rule_create_msg),
			ecm_sfe_ported_ipv4_connection_callback,
			(void *)ecm_db_connection_serial_get(feci->ci));

	nircm = &nim.msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	nircm->vlan_primary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_primary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Get the interface lists of the connection, we must have at least one interface in the list to continue
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in from_interfaces list!\n", feci);
		goto ported_accel_bad_rule;
	}

	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	if (to_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in to_interfaces list!\n", npci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * First interface in each must be a known sfe interface
	 */
	from_sfe_iface = from_ifaces[from_ifaces_first];
	to_sfe_iface = to_ifaces[to_ifaces_first];
	from_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(from_sfe_iface);
	to_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(to_sfe_iface);
	if ((from_sfe_iface_id < 0) || (to_sfe_iface_id < 0)) {
		DEBUG_TRACE("%p: from_sfe_iface_id: %d, to_sfe_iface_id: %d\n", npci, from_sfe_iface_id, to_sfe_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * New rule being created
	 */
	nircm->valid_flags |= SFE_RULE_CREATE_CONN_VALID;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	nircm->conn_rule.flow_interface_num = from_sfe_iface_id;
	nircm->conn_rule.return_interface_num = to_sfe_iface_id;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the inner facing addresses from the heirarchy interface lists we got above.
	 */
	nim.msg.rule_create.conn_rule.flow_top_interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);
	nim.msg.rule_create.conn_rule.return_top_interface_num = ecm_db_iface_interface_identifier_get(to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);

	/*
	 * We know that each outward facing interface is known to the SFE and so this connection could be accelerated.
	 * However the lists may also specify other interesting details that must be included in the creation command,
	 * for example, ethernet MAC, VLAN tagging or PPPoE session information.
	 * We get this information by walking from the outer to the innermost interface for each list and examine the interface types.
	 *
	 * Start with the 'from' (src) side.
	 * NOTE: The lists may contain a complex heirarchy of similar type of interface e.g. multiple vlans or tunnels within tunnels.
	 * This SFE cannot handle that - there is no way to describe this in the rule - if we see multiple types that would conflict we have to abort.
	 */
	DEBUG_TRACE("%p: Examine from/src heirarchy list\n", npci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_in_dev = NULL;
#endif

		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", npci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {
		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%p: Bridge\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: Bridge - ignore additional\n", npci);
				break;
			}
			ecm_db_iface_bridge_address_get(ii, from_sfe_iface_address);
			DEBUG_TRACE("%p: Bridge - mac: %pM\n", npci, from_sfe_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_ETHERNET:
			DEBUG_TRACE("%p: Ethernet\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Ignore additional mac addresses, these are usually as a result of address propagation
				 * from bridges down to ports etc.
				 */
				DEBUG_TRACE("%p: Ethernet - ignore additional\n", npci);
				break;
			}

			/*
			 * Can only handle one MAC, the first outermost mac.
			 */
			ecm_db_iface_ethernet_address_get(ii, from_sfe_iface_address);
			DEBUG_TRACE("%p: Ethernet - mac: %pM\n", npci, from_sfe_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%p: PPPoE - additional unsupported\n", npci);
				rule_invalid = true;
				break;
			}

			/*
			 * Copy pppoe session info to the creation structure.
			 */
			ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);

			nircm->pppoe_rule.flow_pppoe_session_id = pppoe_info.pppoe_session_id;
			memcpy(nircm->pppoe_rule.flow_pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
			nircm->valid_flags |= SFE_RULE_CREATE_PPPOE_VALID;

			DEBUG_TRACE("%p: PPPoE - session: %x, mac: %pM\n", npci,
					nircm->pppoe_rule.flow_pppoe_session_id,
					nircm->pppoe_rule.flow_pppoe_remote_mac);
#else
			rule_invalid = true;
#endif
			break;
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			DEBUG_TRACE("%p: VLAN\n", npci);
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - additional unsupported\n", npci);
				break;
			}
			ecm_db_iface_vlan_info_get(ii, &vlan_info);
			vlan_value = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);

			/*
			 * Look up the vlan device and incorporate the vlan priority into the vlan_value
			 */
			vlan_in_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (vlan_in_dev) {
				vlan_value |= vlan_dev_get_egress_prio(vlan_in_dev, pr->return_qos_tag);
				dev_put(vlan_in_dev);
				vlan_in_dev = NULL;
			}

			/*
			 * Primary or secondary (QinQ) VLAN?
			 */
			if (interface_type_counts[ii_type] == 0) {
				nircm->vlan_primary_rule.ingress_vlan_tag = vlan_value;
			} else {
				nircm->vlan_secondary_rule.ingress_vlan_tag = vlan_value;
			}
			nircm->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
				memcpy(from_sfe_iface_address, vlan_info.address, ETH_ALEN);
				interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
				DEBUG_TRACE("%p: VLAN use mac: %pM\n", npci, from_sfe_iface_address);
			}
			DEBUG_TRACE("%p: vlan tag: %x\n", npci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: VLAN - unsupported\n", npci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%p: IPSEC\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: IPSEC - additional unsupported\n", npci);
				break;
			}
			nircm->conn_rule.flow_interface_num = SFE_SPECIAL_INTERFACE_IPSEC;
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: IPSEC - unsupported\n", npci);
#endif
			break;
		default:
			DEBUG_TRACE("%p: Ignoring: %d (%s)\n", npci, ii_type, ii_name);
		}

		/*
		 * Seen an interface of this type
		 */
		interface_type_counts[ii_type]++;
	}
	if (rule_invalid) {
		DEBUG_WARN("%p: from/src Rule invalid\n", npci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%p: Examine to/dest heirarchy list\n", npci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = to_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_out_dev = NULL;
#endif

		ii = to_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", npci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {
		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%p: Bridge\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: Bridge - ignore additional\n", npci);
				break;
			}
			ecm_db_iface_bridge_address_get(ii, to_sfe_iface_address);
			DEBUG_TRACE("%p: Bridge - mac: %pM\n", npci, to_sfe_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_ETHERNET:
			DEBUG_TRACE("%p: Ethernet\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Ignore additional mac addresses, these are usually as a result of address propagation
				 * from bridges down to ports etc.
				 */
				DEBUG_TRACE("%p: Ethernet - ignore additional\n", npci);
				break;
			}

			/*
			 * Can only handle one MAC, the first outermost mac.
			 */
			ecm_db_iface_ethernet_address_get(ii, to_sfe_iface_address);
			DEBUG_TRACE("%p: Ethernet - mac: %pM\n", npci, to_sfe_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%p: PPPoE - additional unsupported\n", npci);
				rule_invalid = true;
				break;
			}

			/*
			 * Copy pppoe session info to the creation structure.
			 */
			ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);
			nircm->pppoe_rule.return_pppoe_session_id = pppoe_info.pppoe_session_id;
			memcpy(nircm->pppoe_rule.return_pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
			nircm->valid_flags |= SFE_RULE_CREATE_PPPOE_VALID;

			DEBUG_TRACE("%p: PPPoE - session: %x, mac: %pM\n", npci,
				    nircm->pppoe_rule.return_pppoe_session_id,
				    nircm->pppoe_rule.return_pppoe_remote_mac);
#else
			rule_invalid = true;
#endif
			break;
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			DEBUG_TRACE("%p: VLAN\n", npci);
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - additional unsupported\n", npci);
				break;
			}
			ecm_db_iface_vlan_info_get(ii, &vlan_info);
			vlan_value = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);

			/*
			 * Look up the vlan device and incorporate the vlan priority into the vlan_value
			 */
			vlan_out_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (vlan_out_dev) {
				vlan_value |= vlan_dev_get_egress_prio(vlan_out_dev, pr->flow_qos_tag);
				dev_put(vlan_out_dev);
				vlan_out_dev = NULL;
			}

			/*
			 * Primary or secondary (QinQ) VLAN?
			 */
			if (interface_type_counts[ii_type] == 0) {
				nircm->vlan_primary_rule.egress_vlan_tag = vlan_value;
			} else {
				nircm->vlan_secondary_rule.egress_vlan_tag = vlan_value;
			}
			nircm->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
				memcpy(to_sfe_iface_address, vlan_info.address, ETH_ALEN);
				interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
				DEBUG_TRACE("%p: VLAN use mac: %pM\n", npci, to_sfe_iface_address);
			}
			DEBUG_TRACE("%p: vlan tag: %x\n", npci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: VLAN - unsupported\n", npci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%p: IPSEC\n", npci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: IPSEC - additional unsupported\n", npci);
				break;
			}
			nircm->conn_rule.return_interface_num = SFE_SPECIAL_INTERFACE_IPSEC;
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: IPSEC - unsupported\n", npci);
#endif
			break;
		default:
			DEBUG_TRACE("%p: Ignoring: %d (%s)\n", npci, ii_type, ii_name);
		}

		/*
		 * Seen an interface of this type
		 */
		interface_type_counts[ii_type]++;
	}
	if (rule_invalid) {
		DEBUG_WARN("%p: from/src Rule invalid\n", npci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * Routed or bridged?
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_ROUTED;
	} else {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_BRIDGE_FLOW;
		if (is_l2_encap) {
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_L2_ENCAP;
		}
	}

	/*
	 * Set up the flow and return qos tags
	 */
	nircm->qos_rule.flow_qos_tag = (uint32_t)pr->flow_qos_tag;
	nircm->qos_rule.return_qos_tag = (uint32_t)pr->return_qos_tag;
	nircm->valid_flags |= SFE_RULE_CREATE_QOS_VALID;

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
		nircm->dscp_rule.flow_dscp = pr->flow_dscp;
		nircm->dscp_rule.return_dscp = pr->return_dscp;
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= SFE_RULE_CREATE_DSCP_MARKING_VALID;
	}
#endif

#ifdef CONFIG_XFRM
	nircm->direction_rule.flow_accel = (npci->flow_ipsec_state != ECM_SFE_IPSEC_STATE_TO_ENCRYPT);
	nircm->direction_rule.return_accel = (npci->return_ipsec_state != ECM_SFE_IPSEC_STATE_TO_ENCRYPT);
	if (!nircm->direction_rule.flow_accel || !nircm->direction_rule.return_accel) {
		nircm->valid_flags |= SFE_RULE_CREATE_DIRECTION_VALID;
	}
#endif
	protocol = ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Set protocol
	 */
	nircm->tuple.protocol = (int32_t)protocol;

	/*
	 * The flow_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->tuple.flow_ip, addr);

	/*
	 * The return_ip is where the connection is established to, however, in the case of ingress
	 * the return_ip would be the routers WAN IP - i.e. the NAT'ed version.
	 * Getting the NAT'ed version here works for ingress or egress packets, for egress
	 * the NAT'ed version would be the same as the normal address
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->tuple.return_ip, addr);

	/*
	 * When the packet is forwarded to the next interface get the address the source IP of the
	 * packet should be translated to.  For egress this is the NAT'ed from address.
	 * This also works for ingress as the NAT'ed version of the WAN host would be the same as non-NAT'ed
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->conn_rule.flow_ip_xlate, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->conn_rule.return_ip_xlate, addr);

	/*
	 * Same approach as above for port information
	 */
	nircm->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	nircm->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));
	nircm->conn_rule.flow_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT));
	nircm->conn_rule.return_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO));

	/*
	 * Get mac addresses.
	 * The src_mac is the mac address of the node that established the connection.
	 * This will work whether the from_node is LAN (egress) or WAN (ingress).
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)nircm->conn_rule.flow_mac);

	/*
	 * The dest_mac is more complex.  For egress it is the node address of the 'to' side of the connection.
	 * For ingress it is the node adress of the NAT'ed 'to' IP.
	 * Essentially it is the MAC of node associated with create.dest_ip and this is "to nat" side.
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, (uint8_t *)nircm->conn_rule.return_mac);

	/*
	 * The dest_mac_xlate is the mac address to replace the pkt.dst_mac when a packet is sent to->from
	 * For bridged connections this does not change.
	 * For routed connections this is the mac of the 'to' node side of the connection.
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac_xlate);
	} else {
		/*
		 * Bridge flows preserve the MAC addressing
		 */
		memcpy(dest_mac_xlate, (uint8_t *)nircm->conn_rule.return_mac, ETH_ALEN);
	}

	/*
	 * Refer to the Example 2 and 3 in ecm_sfe_ipv4_ip_process() function for egress
	 * and ingress NAT'ed cases. In these cases, the destination node is the one which has the
	 * ip_dest_addr. So, above we get the mac address of this host and use that mac address
	 * for the destination node address in NAT'ed cases.
	 */
	ecm_dir = ecm_db_connection_direction_get(feci->ci);
	if ((ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT)) {
		memcpy(nircm->conn_rule.return_mac, dest_mac_xlate, ETH_ALEN);
	}

	/*
	 * Get MTU information
	 */
	nircm->conn_rule.flow_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nircm->conn_rule.return_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	if (protocol == IPPROTO_TCP) {
		/*
		 * Need window scaling and remarking information if available
		 * Start by looking up the conntrack connection
		 */
		if (!ct) {
			/*
			 * No conntrack so no need to check window sequence space
			 */
			DEBUG_TRACE("%p: TCP Accel no ct from conn %p to get window data\n", npci, feci->ci);
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK;
		} else {
			int flow_dir;
			int return_dir;

			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
			ecm_front_end_flow_and_return_directions_get(ct, addr, 4, &flow_dir, &return_dir);

			DEBUG_TRACE("%p: TCP Accel Get window data from ct %p for conn %p\n", npci, ct, feci->ci);
			spin_lock_bh(&ct->lock);
			nircm->tcp_rule.flow_window_scale = ct->proto.tcp.seen[flow_dir].td_scale;
			nircm->tcp_rule.flow_max_window = ct->proto.tcp.seen[flow_dir].td_maxwin;
			nircm->tcp_rule.flow_end = ct->proto.tcp.seen[flow_dir].td_end;
			nircm->tcp_rule.flow_max_end = ct->proto.tcp.seen[flow_dir].td_maxend;
			nircm->tcp_rule.return_window_scale = ct->proto.tcp.seen[return_dir].td_scale;
			nircm->tcp_rule.return_max_window = ct->proto.tcp.seen[return_dir].td_maxwin;
			nircm->tcp_rule.return_end = ct->proto.tcp.seen[return_dir].td_end;
			nircm->tcp_rule.return_max_end = ct->proto.tcp.seen[return_dir].td_maxend;
#ifdef ECM_OPENWRT_SUPPORT
			if (nf_ct_tcp_be_liberal || nf_ct_tcp_no_window_check
#else
			if (nf_ct_tcp_be_liberal
#endif
					|| (ct->proto.tcp.seen[flow_dir].flags & IP_CT_TCP_FLAG_BE_LIBERAL)
					|| (ct->proto.tcp.seen[return_dir].flags & IP_CT_TCP_FLAG_BE_LIBERAL)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK;
			} else {
#ifdef CONFIG_XFRM
				/*
				 * TCP window check will fail if TCP is accelerated in one direction but not in another
				 */
				if (nircm->direction_rule.flow_accel != nircm->direction_rule.return_accel) {
					spin_unlock_bh(&ct->lock);
					DEBUG_WARN("Can't accelerate ipsec TCP flow in uni-direction because TCP window checking is on\n");
					ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
					ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
					goto ported_accel_bad_rule;
				}
#endif
			}
			spin_unlock_bh(&ct->lock);
		}

		nircm->valid_flags |= SFE_RULE_CREATE_TCP_VALID;
	}

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
		DEBUG_TRACE("%p: sync from: %p, type: %d\n", npci, aci, aci->type_get(aci));
		aci->sync_from_v4(aci, &ecrc);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Release the interface lists
	 */
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);

	DEBUG_INFO("%p: Ported Accelerate connection %p\n"
			"Protocol: %d\n"
			"from_mtu: %u\n"
			"to_mtu: %u\n"
			"from_ip: %pI4n:%d\n"
			"to_ip: %pI4n:%d\n"
			"from_ip_xlate: %pI4n:%d\n"
			"to_ip_xlate: %pI4n:%d\n"
			"from_mac: %pM\n"
			"to_mac: %pM\n"
			"src_iface_num: %u\n"
			"dest_iface_num: %u\n"
			"ingress_inner_vlan_tag: %u\n"
			"egress_inner_vlan_tag: %u\n"
			"ingress_outer_vlan_tag: %u\n"
			"egress_outer_vlan_tag: %u\n"
			"rule_flags: %x\n"
			"valid_flags: %x\n"
			"return_pppoe_session_id: %u\n"
			"return_pppoe_remote_mac: %pM\n"
			"flow_pppoe_session_id: %u\n"
			"flow_pppoe_remote_mac: %pM\n"
			"flow_qos_tag: %x (%u)\n"
			"return_qos_tag: %x (%u)\n"
			"flow_window_scale: %u\n"
			"flow_max_window: %u\n"
			"flow_end: %u\n"
			"flow_max_end: %u\n"
			"return_window_scale: %u\n"
			"return_max_window: %u\n"
			"return_end: %u\n"
			"return_max_end: %u\n"
			"flow_dscp: %x\n"
			"return_dscp: %x\n",
			npci,
			feci->ci,
			nircm->tuple.protocol,
			nircm->conn_rule.flow_mtu,
			nircm->conn_rule.return_mtu,
			&nircm->tuple.flow_ip, nircm->tuple.flow_ident,
			&nircm->tuple.return_ip, nircm->tuple.return_ident,
			&nircm->conn_rule.flow_ip_xlate, nircm->conn_rule.flow_ident_xlate,
			&nircm->conn_rule.return_ip_xlate, nircm->conn_rule.return_ident_xlate,
			nircm->conn_rule.flow_mac,
			nircm->conn_rule.return_mac,
			nircm->conn_rule.flow_interface_num,
			nircm->conn_rule.return_interface_num,
			nircm->vlan_primary_rule.ingress_vlan_tag,
			nircm->vlan_primary_rule.egress_vlan_tag,
			nircm->vlan_secondary_rule.ingress_vlan_tag,
			nircm->vlan_secondary_rule.egress_vlan_tag,
			nircm->rule_flags,
			nircm->valid_flags,
			nircm->pppoe_rule.return_pppoe_session_id,
			nircm->pppoe_rule.return_pppoe_remote_mac,
			nircm->pppoe_rule.flow_pppoe_session_id,
			nircm->pppoe_rule.flow_pppoe_remote_mac,
			nircm->qos_rule.flow_qos_tag, nircm->qos_rule.flow_qos_tag,
			nircm->qos_rule.return_qos_tag, nircm->qos_rule.return_qos_tag,
			nircm->tcp_rule.flow_window_scale,
			nircm->tcp_rule.flow_max_window,
			nircm->tcp_rule.flow_end,
			nircm->tcp_rule.flow_max_end,
			nircm->tcp_rule.return_window_scale,
			nircm->tcp_rule.return_max_window,
			nircm->tcp_rule.return_end,
			nircm->tcp_rule.return_max_end,
			nircm->dscp_rule.flow_dscp,
			nircm->dscp_rule.return_dscp);

	if (protocol == IPPROTO_TCP) {

		DEBUG_INFO("flow_window_scale: %u\n"
			"flow_max_window: %u\n"
			"flow_end: %u\n"
			"flow_max_end: %u\n"
			"return_window_scale: %u\n"
			"return_max_window: %u\n"
			"return_end: %u\n"
			"return_max_end: %u\n",
			nircm->tcp_rule.flow_window_scale,
			nircm->tcp_rule.flow_max_window,
			nircm->tcp_rule.flow_end,
			nircm->tcp_rule.flow_max_end,
			nircm->tcp_rule.return_window_scale,
			nircm->tcp_rule.return_max_window,
			nircm->tcp_rule.return_end,
			nircm->tcp_rule.return_max_end);
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
		ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		return;
	}

	/*
	 * Ref the connection before issuing an SFE rule
	 * This ensures that when the SFE responds to the command - which may even be immediately -
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
	sfe_tx_status = sfe_drv_ipv4_tx(ecm_sfe_ipv4_drv_mgr, &nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;
		spin_unlock_bh(&feci->lock);
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
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Accel mode unexpected: %d\n", feci, feci->accel_mode);
	feci->stats.driver_fail_total++;
	feci->stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%p: Accel failed - driver fail limit\n", npci);
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	} else {
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	spin_lock_bh(&ecm_sfe_ipv4_lock);
	_ecm_sfe_ipv4_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	spin_unlock_bh(&feci->lock);
	return;

ported_accel_bad_rule:
	;

	/*
	 * Jump to here when rule data is bad and an offload command cannot be constructed
	 */
	DEBUG_WARN("%p: Accel failed - bad rule\n", npci);
	ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE);
}

/*
 * ecm_sfe_ported_ipv4_connection_destroy_callback()
 *	Callback for handling destroy ack/nack calls.
 */
static void ecm_sfe_ported_ipv4_connection_destroy_callback(void *app_data, struct sfe_ipv4_msg *nim)
{
	struct sfe_ipv4_rule_destroy_msg * __attribute__((unused))nirdm = &nim->msg.rule_destroy;
	uint32_t serial = (uint32_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_sfe_ported_ipv4_connection_instance *npci;

	/*
	 * Is this a response to a destroy message?
	 */
	if (nim->cm.type != SFE_TX_DESTROY_RULE_MSG) {
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
	npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	/*
	 * Record command duration
	 */
	ecm_sfe_ipv4_decel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: decelerate response for connection: %p\n", npci, feci->ci);
	DEBUG_TRACE("%p: flow_ip: %pI4n:%d\n", npci, &nirdm->tuple.flow_ip, nirdm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4n:%d\n", npci, &nirdm->tuple.return_ip, nirdm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", npci, nirdm->tuple.protocol);

	/*
	 * Drop decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	spin_lock_bh(&feci->lock);

	/*
	 * If decel is not still pending then it's possible that the SFE ended acceleration by some other reason e.g. flush
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

	DEBUG_TRACE("%p: response: %d\n", npci, nim->cm.response);
	if (nim->cm.response != SFE_CMN_RESPONSE_ACK) {
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
	 * Ported acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ported_ipv4_accelerated_count[npci->ported_accelerated_count_index]--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_ported_ipv4_accelerated_count[npci->ported_accelerated_count_index] >= 0, "Bad udp accel counter\n");
	ecm_sfe_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Release the connections.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_sfe_ported_ipv4_connection_decelerate_msg_send()
 *	Prepares and sends a decelerate message to acceleration engine.
 */
static bool ecm_sfe_ported_ipv4_connection_decelerate_msg_send(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;
	struct sfe_ipv4_msg nim;
	struct sfe_ipv4_rule_destroy_msg *nirdm;
	ip_addr_t addr;
	sfe_tx_status_t sfe_tx_status;
	bool ret;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	/*
	 * Increment the decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count++;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Prepare deceleration message
	 */
	sfe_ipv4_msg_init(&nim, SFE_SPECIAL_INTERFACE_IPV4, SFE_TX_DESTROY_RULE_MSG,
			sizeof(struct sfe_ipv4_rule_destroy_msg),
			ecm_sfe_ported_ipv4_connection_destroy_callback,
			(void *)ecm_db_connection_serial_get(feci->ci));

	nirdm = &nim.msg.rule_destroy;
	nirdm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nirdm->tuple.flow_ip, addr);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nirdm->tuple.return_ip, addr);
	nirdm->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	nirdm->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));

	DEBUG_INFO("%p: Ported Connection %p decelerate\n"
			"protocol: %d\n"
			"src_ip: %pI4:%d\n"
			"dest_ip: %pI4:%d\n",
			npci, feci->ci, nirdm->tuple.protocol,
			&nirdm->tuple.flow_ip, nirdm->tuple.flow_ident,
			&nirdm->tuple.return_ip, nirdm->tuple.return_ident);

	/*
	 * Take a ref to the feci->ci so that it will persist until we get a response from the SFE.
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
	 * Destroy the SFE connection cache entry.
	 */
	sfe_tx_status = sfe_drv_ipv4_tx(ecm_sfe_ipv4_drv_mgr, &nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;
		spin_unlock_bh(&feci->lock);
		return true;
	}

	/*
	 * TX failed
	 */
	ret = ecm_front_end_destroy_failure_handle(feci);

	/*
	 * Release the ref take, SFE driver did not accept our command.
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * Could not send the request, decrement the decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	return ret;
}

/*
 * ecm_sfe_ported_ipv4_connection_decelerate()
 *     Decelerate a connection
 */
static bool ecm_sfe_ported_ipv4_connection_decelerate(struct ecm_front_end_connection_instance *feci)
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

	return ecm_sfe_ported_ipv4_connection_decelerate_msg_send(feci);
}

/*
 * ecm_sfe_ported_ipv4_connection_defunct_callback()
 *	Callback to be called when a ported connection has become defunct.
 */
static bool ecm_sfe_ported_ipv4_connection_defunct_callback(void *arg, int *accel_mode)
{
	bool ret;
	struct ecm_front_end_connection_instance *feci = (struct ecm_front_end_connection_instance *)arg;
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

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

	ret = ecm_sfe_ported_ipv4_connection_decelerate_msg_send(feci);

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
 * ecm_sfe_ported_ipv4_connection_accel_state_get()
 *	Get acceleration state
 */
static ecm_front_end_acceleration_mode_t ecm_sfe_ported_ipv4_connection_accel_state_get(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;
	ecm_front_end_acceleration_mode_t state;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);
	spin_lock_bh(&feci->lock);
	state = feci->accel_mode;
	spin_unlock_bh(&feci->lock);
	return state;
}

/*
 * ecm_sfe_ported_ipv4_connection_action_seen()
 *	Acceleration action / activity has been seen for this connection.
 *
 * NOTE: Call the action_seen() method when the SFE has demonstrated that it has offloaded some data for a connection.
 */
static void ecm_sfe_ported_ipv4_connection_action_seen(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);
	DEBUG_INFO("%p: Action seen\n", npci);
	spin_lock_bh(&feci->lock);
	feci->stats.no_action_seen = 0;
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_sfe_ported_ipv4_connection_accel_ceased()
 *	SFE has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an SFE self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_sfe_ported_ipv4_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);
	DEBUG_INFO("%p: accel ceased\n", npci);

	spin_lock_bh(&feci->lock);

	/*
	 * If we are in accel-pending state then the SFE has issued a flush out-of-order
	 * with the ACK/NACK we are actually waiting for.
	 * To work around this we record a "flush has already happened" and will action it when we finally get that ACK/NACK.
	 * GGG TODO This should eventually be removed when the SFE honours messaging sequence.
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
	 * Ported acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ported_ipv4_accelerated_count[npci->ported_accelerated_count_index]--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_ported_ipv4_accelerated_count[npci->ported_accelerated_count_index] >= 0, "Bad ported accel counter\n");
	ecm_sfe_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);
}

/*
 * ecm_sfe_ported_ipv4_connection_ref()
 *	Ref a connection front end instance
 */
static void ecm_sfe_ported_ipv4_connection_ref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);
	spin_lock_bh(&feci->lock);
	feci->refs++;
	DEBUG_TRACE("%p: npci ref %d\n", feci, feci->refs);
	DEBUG_ASSERT(feci->refs > 0, "%p: ref wrap\n", feci);
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_sfe_ported_ipv4_connection_deref()
 *	Deref a connection front end instance
 */
static int ecm_sfe_ported_ipv4_connection_deref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	spin_lock_bh(&feci->lock);
	feci->refs--;
	DEBUG_ASSERT(feci->refs >= 0, "%p: ref wrap\n", feci);

	if (feci->refs > 0) {
		int refs = feci->refs;
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%p: npci deref %d\n", npci, refs);
		return refs;
	}
	spin_unlock_bh(&feci->lock);

	/*
	 * We can now destroy the instance
	 */
	DEBUG_TRACE("%p: npci final\n", npci);
	DEBUG_CLEAR_MAGIC(npci);
	kfree(npci);
	return 0;
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_sfe_ported_ipv4_connection_state_get()
 *	Return state of this ported front end instance
 */
static int ecm_sfe_ported_ipv4_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	int result;
	bool can_accel;
	ecm_front_end_acceleration_mode_t accel_mode;
	struct ecm_front_end_connection_mode_stats stats;
	struct ecm_sfe_ported_ipv4_connection_instance *npci = (struct ecm_sfe_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", npci);

	spin_lock_bh(&feci->lock);
	can_accel = feci->can_accel;
	accel_mode = feci->accel_mode;
	memcpy(&stats, &feci->stats, sizeof(struct ecm_front_end_connection_mode_stats));
	spin_unlock_bh(&feci->lock);

	if ((result = ecm_state_prefix_add(sfi, "front_end_v4.ported"))) {
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
 * ecm_sfe_ported_ipv4_connection_instance_alloc()
 *	Create a front end instance specific for ported connection
 */
static struct ecm_sfe_ported_ipv4_connection_instance *ecm_sfe_ported_ipv4_connection_instance_alloc(
								struct ecm_db_connection_instance *ci,
								int protocol,
								bool can_accel)
{
	struct ecm_sfe_ported_ipv4_connection_instance *npci;
	struct ecm_front_end_connection_instance *feci;

	npci = (struct ecm_sfe_ported_ipv4_connection_instance *)kzalloc(sizeof(struct ecm_sfe_ported_ipv4_connection_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!npci) {
		DEBUG_WARN("Ported Front end alloc failed\n");
		return NULL;
	}

	/*
	 * Refs is 1 for the creator of the connection
	 */
	feci = (struct ecm_front_end_connection_instance *)npci;
	feci->refs = 1;
	DEBUG_SET_MAGIC(npci, ECM_SFE_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC);
	spin_lock_init(&feci->lock);

	feci->can_accel = can_accel;
	feci->accel_mode = (can_accel)? ECM_FRONT_END_ACCELERATION_MODE_DECEL : ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED;
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	feci->stats.no_action_seen_limit = ecm_sfe_ipv4_no_action_limit_default;
	feci->stats.driver_fail_limit = ecm_sfe_ipv4_driver_fail_limit_default;
	feci->stats.ae_nack_limit = ecm_sfe_ipv4_nack_limit_default;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Copy reference to connection - no need to ref ci as ci maintains a ref to this instance instead (this instance persists for as long as ci does)
	 */
	feci->ci = ci;

	feci->ip_version = 4;

	/*
	 * Populate the methods and callbacks
	 */
	feci->ref = ecm_sfe_ported_ipv4_connection_ref;
	feci->deref = ecm_sfe_ported_ipv4_connection_deref;
	feci->decelerate = ecm_sfe_ported_ipv4_connection_decelerate;
	feci->accel_state_get = ecm_sfe_ported_ipv4_connection_accel_state_get;
	feci->action_seen = ecm_sfe_ported_ipv4_connection_action_seen;
	feci->accel_ceased = ecm_sfe_ported_ipv4_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_sfe_ported_ipv4_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_sfe_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_sfe_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_sfe_common_get_interface_type;
	feci->regenerate = ecm_sfe_common_connection_regenerate;

	if (protocol == IPPROTO_TCP) {
		npci->ported_accelerated_count_index = ECM_SFE_PORTED_IPV4_PROTO_TCP;
	} else if (protocol == IPPROTO_UDP) {
		npci->ported_accelerated_count_index = ECM_SFE_PORTED_IPV4_PROTO_UDP;
	} else {
		DEBUG_WARN("%p: Wrong protocol: %d\n", npci, protocol);
		DEBUG_CLEAR_MAGIC(npci);
		kfree(npci);
		return NULL;
	}

	return npci;
}

/*
 * ecm_sfe_ported_ipv4_process()
 *	Process a ported packet
 */
unsigned int ecm_sfe_ported_ipv4_process(struct net_device *out_dev, struct net_device *out_dev_nat,
							struct net_device *in_dev, struct net_device *in_dev_nat,
							uint8_t *src_node_addr, uint8_t *src_node_addr_nat,
							uint8_t *dest_node_addr, uint8_t *dest_node_addr_nat,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb,
							struct ecm_tracker_ip_header *iph,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr,
							ip_addr_t ip_src_addr_nat, ip_addr_t ip_dest_addr_nat)
{
	struct tcphdr *tcp_hdr;
	struct tcphdr tcp_hdr_buff;
	struct udphdr *udp_hdr;
	struct udphdr udp_hdr_buff;
	int src_port;
	int src_port_nat;
	int dest_port;
	int dest_port_nat;
	struct ecm_db_connection_instance *ci;
	ip_addr_t match_addr;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ecm_db_timer_group_t ci_orig_timer_group;
	struct ecm_classifier_process_response prevalent_pr;
	int protocol = (int)orig_tuple->dst.protonum;
	__be16 *layer4hdr = NULL;

	/*
	 * Unconfirmed connection may be dropped by Linux at the final step,
	 * So we don't allow acceleration for the unconfirmed connections.
	 */
	if (likely(ct) && !nf_ct_is_confirmed(ct)) {
		DEBUG_WARN("%p: Unconfirmed connection\n", ct);
		return NF_ACCEPT;
	}

	if (protocol == IPPROTO_TCP) {
		/*
		 * Don't try to manage a non-established connection.
		 */
		if (likely(ct) && !test_bit(IPS_ASSURED_BIT, &ct->status)) {
			DEBUG_WARN("%p: Non-established TCP connection\n", ct);
			return NF_ACCEPT;
		}

		/*
		 * Extract TCP header to obtain port information
		 */
		tcp_hdr = ecm_tracker_tcp_check_header_and_read(skb, iph, &tcp_hdr_buff);
		if (unlikely(!tcp_hdr)) {
			DEBUG_WARN("TCP packet header %p\n", skb);
			return NF_ACCEPT;
		}

		layer4hdr = (__be16 *)tcp_hdr;

		/*
		 * Now extract information, if we have conntrack then use that (which would already be in the tuples)
		 */
		if (unlikely(!ct)) {
			orig_tuple->src.u.tcp.port = tcp_hdr->source;
			orig_tuple->dst.u.tcp.port = tcp_hdr->dest;
			reply_tuple->src.u.tcp.port = tcp_hdr->dest;
			reply_tuple->dst.u.tcp.port = tcp_hdr->source;
		}

		/*
		 * Extract transport port information
		 * Refer to the ecm_sfe_ipv4_process() for information on how we extract this information.
		 */
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
				src_port = ntohs(orig_tuple->src.u.tcp.port);
				dest_port = ntohs(orig_tuple->dst.u.tcp.port);
				dest_port_nat = ntohs(reply_tuple->src.u.tcp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
				src_port = ntohs(orig_tuple->src.u.tcp.port);
				dest_port_nat = ntohs(orig_tuple->dst.u.tcp.port);
				dest_port = ntohs(reply_tuple->src.u.tcp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
				src_port = ntohs(orig_tuple->src.u.tcp.port);
				dest_port = ntohs(orig_tuple->dst.u.tcp.port);
				dest_port_nat = ntohs(reply_tuple->src.u.tcp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else {
				DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
			}
		} else {
			if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
				dest_port = ntohs(orig_tuple->src.u.tcp.port);
				src_port = ntohs(orig_tuple->dst.u.tcp.port);
				src_port_nat = ntohs(reply_tuple->src.u.tcp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
				dest_port = ntohs(orig_tuple->src.u.tcp.port);
				src_port_nat = ntohs(orig_tuple->dst.u.tcp.port);
				src_port = ntohs(reply_tuple->src.u.tcp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
				dest_port = ntohs(orig_tuple->src.u.tcp.port);
				src_port = ntohs(orig_tuple->dst.u.tcp.port);
				src_port_nat = ntohs(reply_tuple->src.u.tcp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.tcp.port);
			} else {
				DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
			}
		}

		DEBUG_TRACE("TCP src: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "):%d(%d), dest: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "):%d(%d), dir %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), ECM_IP_ADDR_TO_DOT(ip_src_addr_nat), src_port, src_port_nat, ECM_IP_ADDR_TO_DOT(ip_dest_addr),
				ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat), dest_port, dest_port_nat, ecm_dir);
	} else if (protocol == IPPROTO_UDP) {
		/*
		 * Extract UDP header to obtain port information
		 */
		udp_hdr = ecm_tracker_udp_check_header_and_read(skb, iph, &udp_hdr_buff);
		if (unlikely(!udp_hdr)) {
			DEBUG_WARN("Invalid UDP header in skb %p\n", skb);
			return NF_ACCEPT;
		}

		layer4hdr = (__be16 *)udp_hdr;

		/*
		 * Now extract information, if we have conntrack then use that (which would already be in the tuples)
		 */
		if (unlikely(!ct)) {
			orig_tuple->src.u.udp.port = udp_hdr->source;
			orig_tuple->dst.u.udp.port = udp_hdr->dest;
			reply_tuple->src.u.udp.port = udp_hdr->dest;
			reply_tuple->dst.u.udp.port = udp_hdr->source;
		}

		/*
		 * Extract transport port information
		 * Refer to the ecm_sfe_ipv4_process() for information on how we extract this information.
		 */
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
				src_port = ntohs(orig_tuple->src.u.udp.port);
				dest_port = ntohs(orig_tuple->dst.u.udp.port);
				dest_port_nat = ntohs(reply_tuple->src.u.udp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
				src_port = ntohs(orig_tuple->src.u.udp.port);
				dest_port_nat = ntohs(orig_tuple->dst.u.udp.port);
				dest_port = ntohs(reply_tuple->src.u.udp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
				src_port = ntohs(orig_tuple->src.u.udp.port);
				dest_port = ntohs(orig_tuple->dst.u.udp.port);
				dest_port_nat = ntohs(reply_tuple->src.u.udp.port);
				src_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else {
				DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
			}
		} else {
			if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
				dest_port = ntohs(orig_tuple->src.u.udp.port);
				src_port = ntohs(orig_tuple->dst.u.udp.port);
				src_port_nat = ntohs(reply_tuple->src.u.udp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
				dest_port = ntohs(orig_tuple->src.u.udp.port);
				src_port_nat = ntohs(orig_tuple->dst.u.udp.port);
				src_port = ntohs(reply_tuple->src.u.udp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
				dest_port = ntohs(orig_tuple->src.u.udp.port);
				src_port = ntohs(orig_tuple->dst.u.udp.port);
				src_port_nat = ntohs(reply_tuple->src.u.udp.port);
				dest_port_nat = ntohs(reply_tuple->dst.u.udp.port);
			} else {
				DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
			}
		}

		if (ct && nfct_help(ct) && (dest_port == TFTP_PORT)) {
			DEBUG_TRACE("%p: Connection has helper but protocol is TFTP, it can be accelerated\n", ct);
			can_accel = true;
		}

		DEBUG_TRACE("UDP src: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "):%d(%d), dest: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "):%d(%d), dir %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), ECM_IP_ADDR_TO_DOT(ip_src_addr_nat), src_port, src_port_nat, ECM_IP_ADDR_TO_DOT(ip_dest_addr),
				ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat), dest_port, dest_port_nat, ecm_dir);
	} else {
		DEBUG_WARN("Wrong protocol: %d\n", protocol);
		return NF_ACCEPT;
	}

	/*
	 * Look up a connection
	 */
	ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);

	/*
	 * If there is no existing connection then create a new one.
	 */
	if (unlikely(!ci)) {
		struct ecm_db_mapping_instance *mi[ECM_DB_OBJ_DIR_MAX];
		struct ecm_db_node_instance *ni[ECM_DB_OBJ_DIR_MAX];
		struct ecm_classifier_default_instance *dci;
		struct ecm_db_connection_instance *nci;
		ecm_classifier_type_t classifier_type;
		struct ecm_front_end_connection_instance *feci;
		int32_t to_list_first;
		struct ecm_db_iface_instance *to_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t to_nat_list_first;
		struct ecm_db_iface_instance *to_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_list_first;
		struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_nat_list_first;
		struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_front_end_interface_construct_instance efeici;

		DEBUG_INFO("New ported connection from " ECM_IP_ADDR_DOT_FMT ":%u to " ECM_IP_ADDR_DOT_FMT ":%u protocol: %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), src_port, ECM_IP_ADDR_TO_DOT(ip_dest_addr), dest_port, protocol);

		/*
		 * Before we attempt to create the connection are we being terminated?
		 */
		spin_lock_bh(&ecm_sfe_ipv4_lock);
		if (ecm_sfe_ipv4_terminate_pending) {
			spin_unlock_bh(&ecm_sfe_ipv4_lock);
			DEBUG_WARN("Terminating\n");

			/*
			 * As we are terminating we just allow the packet to pass - it's no longer our concern
			 */
			return NF_ACCEPT;
		}
		spin_unlock_bh(&ecm_sfe_ipv4_lock);

		/*
		 * Does this connection have a conntrack entry?
		 */
		if (ct) {
			if (protocol == IPPROTO_TCP) {
				/*
				 * No point in establishing a connection for one that is closing
				 */
				spin_lock_bh(&ct->lock);
				if (ct->proto.tcp.state >= TCP_CONNTRACK_FIN_WAIT && ct->proto.tcp.state <= TCP_CONNTRACK_CLOSE) {
					spin_unlock_bh(&ct->lock);
					DEBUG_TRACE("%p: Connection in termination state %#X\n", ct, ct->proto.tcp.state);
					return NF_ACCEPT;
				}
				spin_unlock_bh(&ct->lock);
			}
		}

		/*
		 * Now allocate the new connection
		 */
		nci = ecm_db_connection_alloc();
		if (!nci) {
			DEBUG_WARN("Failed to allocate connection\n");
			return NF_ACCEPT;
		}

		/*
		 * Connection must have a front end instance associated with it
		 */
		feci = (struct ecm_front_end_connection_instance *)ecm_sfe_ported_ipv4_connection_instance_alloc(nci, protocol, can_accel);
		if (!feci) {
			DEBUG_WARN("Failed to allocate front end\n");
			goto fail_1;
		}

#ifdef CONFIG_XFRM
		/*
		 * Packet has been decrypted by ipsec, mark it in connection.
		 */
		if (unlikely(skb_ext_exist(skb, SKB_EXT_SEC_PATH))) {
			((struct ecm_sfe_ported_ipv4_connection_instance *)feci)->flow_ipsec_state = ECM_SFE_IPSEC_STATE_WAS_DECRYPTED;
			((struct ecm_sfe_ported_ipv4_connection_instance *)feci)->return_ipsec_state = ECM_SFE_IPSEC_STATE_TO_ENCRYPT;
		}
#endif
		if (!ecm_front_end_ipv4_interface_construct_set_and_hold(skb, sender, ecm_dir, is_routed,
							in_dev, out_dev,
							ip_src_addr, ip_src_addr_nat,
							ip_dest_addr, ip_dest_addr_nat,
							&efeici)) {
			DEBUG_WARN("ECM front end ipv4 interface construct set failed for routed traffic\n");
			goto fail_2;
		}


		/*
		 * Get the src and destination mappings.
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 * GGG TODO rework terms of "src/dest" - these need to be named consistently as from/to as per database terms.
		 * GGG TODO The empty list checks should not be needed, mapping_establish_and_ref() should fail out if there is no list anyway.
		 */
		DEBUG_TRACE("%p: Create the 'from' interface heirarchy list\n", nci);
		from_list_first = ecm_interface_heirarchy_construct(feci, from_list, efeici.from_dev, efeici.from_other_dev, ip_dest_addr, efeici.from_mac_lookup_ip_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, skb);
		if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from' heirarchy list\n");
			goto fail_3;
		}
		ecm_db_connection_interfaces_reset(nci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);

		DEBUG_TRACE("%p: Create source node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM] = ecm_sfe_ipv4_node_establish_and_ref(feci, efeici.from_dev, efeici.from_mac_lookup_ip_addr, from_list, from_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_list, from_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish source node\n");
			goto fail_3;
		}

		DEBUG_TRACE("%p: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM] = ecm_sfe_ipv4_mapping_establish_and_ref(ip_src_addr, src_port);
		if (!mi[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish src mapping\n");
			goto fail_4;
		}

		DEBUG_TRACE("%p: Create the 'to' interface heirarchy list\n", nci);
		to_list_first = ecm_interface_heirarchy_construct(feci, to_list, efeici.to_dev, efeici.to_other_dev, ip_src_addr, efeici.to_mac_lookup_ip_addr, ip_dest_addr, 4, protocol, out_dev, is_routed, in_dev, dest_node_addr, src_node_addr, layer4hdr, skb);
		if (to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to' heirarchy list\n");
			goto fail_5;
		}
		ecm_db_connection_interfaces_reset(nci, to_list, to_list_first, ECM_DB_OBJ_DIR_TO);

		DEBUG_TRACE("%p: Create dest node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO] = ecm_sfe_ipv4_node_establish_and_ref(feci, efeici.to_dev, efeici.to_mac_lookup_ip_addr, to_list, to_list_first, dest_node_addr, skb);
		ecm_db_connection_interfaces_deref(to_list, to_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO]) {
			DEBUG_WARN("Failed to establish dest node\n");
			goto fail_5;
		}

		DEBUG_TRACE("%p: Create dest mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_TO] = ecm_sfe_ipv4_mapping_establish_and_ref(ip_dest_addr, dest_port);
		if (!mi[ECM_DB_OBJ_DIR_TO]) {
			DEBUG_WARN("Failed to establish dest mapping\n");
			goto fail_6;
		}

		/*
		 * Get the src and destination NAT mappings
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 * GGG TODO rework terms of "src/dest" - these need to be named consistently as from/to as per database terms.
		 * GGG TODO The empty list checks should not be needed, mapping_establish_and_ref() should fail out if there is no list anyway.
		 */
		DEBUG_TRACE("%p: Create the 'from NAT' interface heirarchy list\n", nci);
		from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr_nat, dest_node_addr_nat, layer4hdr, skb);
		if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from NAT' heirarchy list\n");
			goto fail_7;
		}
		ecm_db_connection_interfaces_reset(nci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);

		DEBUG_TRACE("%p: Create source nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_sfe_ipv4_node_establish_and_ref(feci, efeici.from_nat_dev, efeici.from_nat_mac_lookup_ip_addr, from_nat_list, from_nat_list_first, src_node_addr_nat, skb);
		ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish source nat node\n");
			goto fail_7;
		}

		mi[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_sfe_ipv4_mapping_establish_and_ref(ip_src_addr_nat, src_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish src nat mapping\n");
			goto fail_8;
		}

		DEBUG_TRACE("%p: Create the 'to NAT' interface heirarchy list\n", nci);
		to_nat_list_first = ecm_interface_heirarchy_construct(feci, to_nat_list, efeici.to_nat_dev, efeici.to_nat_other_dev, ip_src_addr, efeici.to_nat_mac_lookup_ip_addr, ip_dest_addr_nat, 4, protocol, out_dev_nat, is_routed, in_dev, dest_node_addr_nat, src_node_addr_nat, layer4hdr, skb);
		if (to_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to NAT' heirarchy list\n");
			goto fail_9;
		}
		ecm_db_connection_interfaces_reset(nci, to_nat_list, to_nat_list_first, ECM_DB_OBJ_DIR_TO_NAT);

		DEBUG_TRACE("%p: Create dest nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO_NAT] = ecm_sfe_ipv4_node_establish_and_ref(feci, efeici.to_nat_dev, efeici.to_nat_mac_lookup_ip_addr, to_nat_list, to_nat_list_first, dest_node_addr_nat, skb);

		ecm_db_connection_interfaces_deref(to_nat_list, to_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest nat node\n");
			goto fail_9;
		}

		mi[ECM_DB_OBJ_DIR_TO_NAT] = ecm_sfe_ipv4_mapping_establish_and_ref(ip_dest_addr_nat, dest_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest mapping\n");
			goto fail_10;
		}

		/*
		 * Every connection also needs a default classifier which is considered 'special' to be assigned
		 */
		dci = ecm_classifier_default_instance_alloc(nci, protocol, ecm_dir, src_port, dest_port);
		if (!dci) {
			DEBUG_WARN("Failed to allocate default classifier\n");
			goto fail_11;
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
				DEBUG_WARN("Failed to allocate classifiers assignments\n");
				goto fail_12;
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
		spin_lock_bh(&ecm_sfe_ipv4_lock);
		ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);
		if (ci) {
			/*
			 * Another cpu created the same connection before us - use the one we just found
			 */
			spin_unlock_bh(&ecm_sfe_ipv4_lock);
			ecm_db_connection_deref(nci);
		} else {
			ecm_db_timer_group_t tg;
			ecm_tracker_sender_state_t src_state;
			ecm_tracker_sender_state_t dest_state;
			ecm_tracker_connection_state_t state;
			struct ecm_tracker_instance *ti;

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
					ecm_sfe_ported_ipv4_connection_defunct_callback,
					tg, is_routed, nci);

			spin_unlock_bh(&ecm_sfe_ipv4_lock);

			ci = nci;
			DEBUG_INFO("%p: New ported connection created\n", ci);
		}

		/*
		 * No longer need referenecs to the objects we created
		 */
		dci->base.deref((struct ecm_classifier_instance *)dci);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO_NAT]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		feci->deref(feci);

		goto done;
fail_12:
		dci->base.deref((struct ecm_classifier_instance *)dci);
fail_11:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO_NAT]);
fail_10:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO_NAT]);
fail_9:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
fail_8:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
fail_7:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
fail_6:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
fail_5:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
fail_4:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
fail_3:
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
fail_2:
		feci->deref(feci);
fail_1:
		ecm_db_connection_deref(nci);
		return NF_ACCEPT;
done:
		;
	}

	/*
	 * Keep connection alive as we have seen activity
	 */
	if (!ecm_db_connection_defunct_timer_touch(ci)) {
		ecm_db_connection_deref(ci);
		return NF_ACCEPT;
	}

	/*
	 * Identify which side of the connection is sending.
	 * NOTE: This may be different than what sender is at the moment
	 * given the connection we have located.
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
		ecm_sfe_ipv4_connection_regenerate(ci, sender, out_dev, out_dev_nat, in_dev, in_dev_nat, layer4hdr, skb);
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
	prevalent_pr.process_actions = 0;
	prevalent_pr.drop = false;
	prevalent_pr.flow_qos_tag = skb->priority;
	prevalent_pr.return_qos_tag = skb->priority;
	prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	prevalent_pr.timer_group = ci_orig_timer_group = ecm_db_connection_timer_group_get(ci);

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
		return NF_ACCEPT;
	}
	ecm_db_connection_data_totals_update(ci, (sender == ECM_TRACKER_SENDER_TYPE_SRC)? true : false, skb->len, 1);

	/*
	 * Assign qos tag
	 * GGG TODO Should we use sender to identify whether to use flow or return qos tag?
	 */
	skb->priority = prevalent_pr.flow_qos_tag;
	DEBUG_TRACE("%p: skb priority: %u\n", ci, skb->priority);

	/*
	 * Accelerate?
	 */
	if (prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		struct ecm_front_end_connection_instance *feci;
		DEBUG_TRACE("%p: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		ecm_sfe_ported_ipv4_connection_accelerate(feci, &prevalent_pr, is_l2_encap, ct);
		feci->deref(feci);
	}
	ecm_db_connection_deref(ci);

	return NF_ACCEPT;
}

/*
 * ecm_sfe_ported_ipv4_debugfs_init()
 */
bool ecm_sfe_ported_ipv4_debugfs_init(struct dentry *dentry)
{
	struct dentry *udp_dentry;

	udp_dentry = debugfs_create_u32("udp_accelerated_count", S_IRUGO, dentry,
						&ecm_sfe_ported_ipv4_accelerated_count[ECM_SFE_PORTED_IPV4_PROTO_UDP]);
	if (!udp_dentry) {
		DEBUG_ERROR("Failed to create ecm sfe ipv4 udp_accelerated_count file in debugfs\n");
		return false;
	}

	debugfs_create_u32("tcp_accelerated_count", S_IRUGO, dentry,
					&ecm_sfe_ported_ipv4_accelerated_count[ECM_SFE_PORTED_IPV4_PROTO_TCP]);

	return true;
}

