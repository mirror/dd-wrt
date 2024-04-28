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
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/pkt_sched.h>
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
#define DEBUG_LEVEL ECM_NSS_NON_PORTED_IPV4_DEBUG_LEVEL

#include <nss_api_if.h>

#ifdef ECM_INTERFACE_IPSEC_ENABLE
#include "nss_ipsec_cmn.h"
#endif

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
#include "ecm_nss_non_ported_ipv4.h"
#include "ecm_nss_ipv4.h"
#include "ecm_nss_common.h"
#include "ecm_front_end_common.h"

/*
 * Magic number
 */
#define ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC 0xEC34

/*
 * struct ecm_nss_non_ported_ipv4_connection_instance
 *	A connection specific front end instance for Non-Ported connections
 */
struct ecm_nss_non_ported_ipv4_connection_instance {
	struct ecm_front_end_connection_instance base;		/* Base class */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

static int ecm_nss_non_ported_ipv4_accelerated_count = 0;		/* Number of Non-Ported connections currently offloaded */

#ifdef ECM_INTERFACE_SIT_ENABLE
#ifdef CONFIG_IPV6_SIT_6RD
/*
 * ecm_nss_non_ported_ipv4_sit_set_peer()
 *	It will set the tunnel's peer when the tunnel is a remote any tunnel.
 */
static void ecm_nss_non_ported_ipv4_sit_set_peer(struct ecm_nss_non_ported_ipv4_connection_instance *nnpci, struct sk_buff *skb)
{
	struct nss_tun6rd_msg tun6rdmsg;
	struct nss_tun6rd_set_peer_msg *tun6rdpeer;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_nss_iface;
	int32_t from_ifaces_first;
	const struct ipv6hdr *iph6;
	int32_t interface_number;
	ecm_db_iface_type_t ii_type;
	ip_addr_t addr;
	nss_tx_status_t nss_tx_status;
	struct net_device *dev;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(nnpci->base.ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in from_interfaces list!\n", nnpci);
		return;
	}

	from_nss_iface = from_ifaces[from_ifaces_first];
	ii_type = ecm_db_iface_type_get(from_nss_iface);

	/*
	 * We handle SIT tunnel only here
	 */
	if (ii_type != ECM_DB_IFACE_TYPE_SIT) {
		DEBUG_WARN("%p: This interface is not the sit tunnel\n", nnpci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		return;
	}

	/*
	 * We catch these packets in the tunnel which destination ip address is null
	 */
	if (!ecm_db_iface_sit_daddr_is_null(from_nss_iface)) {
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		return;
	}
	ecm_db_connection_address_get(nnpci->base.ci, ECM_DB_OBJ_DIR_TO, addr);

	/*
	 * This message is sent to the NSS through the INNER interface, but we are now in the process
	 * context of OUTER interface. To get the INNER interface number from the NSS, we use the
	 * actual net device object and the INNER interface type.
	 */
	interface_number = ecm_db_iface_interface_identifier_get(from_nss_iface);
	dev = dev_get_by_index(&init_net, (uint32_t)interface_number);
	if (!dev) {
		DEBUG_WARN("%p: Unable to find the net device with interface index %d\n", nnpci, interface_number);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		return;
	}

	interface_number = ecm_nss_common_get_interface_number_by_dev_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_TUN6RD_INNER);
	dev_put(dev);

	nss_tun6rd_msg_init(&tun6rdmsg, (uint16_t)interface_number, NSS_TUN6RD_ADD_UPDATE_PEER,
			sizeof(struct nss_tun6rd_set_peer_msg), NULL, NULL);

	tun6rdpeer = &tun6rdmsg.msg.peer;
	ECM_IP_ADDR_TO_NIN4_ADDR(tun6rdpeer->dest, addr);
	iph6 = (struct ipv6hdr *)skb_inner_network_header(skb);
	memcpy(tun6rdpeer->ipv6_address, &iph6->daddr, sizeof(struct in6_addr));

	nss_tx_status = nss_tun6rd_tx(nss_tun6rd_get_context(), &tun6rdmsg);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		/*
		 * Nothing to do when faild to xmit the message.
		 */
		DEBUG_WARN("SIT Accelerate set rule failed\n");
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		return;
	}
	DEBUG_TRACE("%p: SIT[%d] set peer\n"
		   "ipv4 destination address:%x\n"
		   "ipv6 destination address::"ECM_IP_ADDR_OCTAL_FMT"\n",
		nnpci, interface_number, tun6rdpeer->dest, ECM_IP_ADDR_TO_OCTAL(tun6rdpeer->ipv6_address));

	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
}
#endif
#endif

/*
 * ecm_nss_non_ported_ipv4_connection_callback()
 *	Callback for handling create ack/nack calls.
 */
static void ecm_nss_non_ported_ipv4_connection_callback(void *app_data, struct nss_ipv4_msg *nim)
{
	struct nss_ipv4_rule_create_msg *__attribute__((unused))nircm = &nim->msg.rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci;
	ecm_front_end_acceleration_mode_t result_mode;
	bool is_defunct = false;

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != NSS_IPV4_TX_CREATE_RULE_MSG) {
		DEBUG_ERROR("%p: non_ported create callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
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
	nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	/*
	 * Record command duration
	 */
	ecm_nss_ipv4_accel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: accelerate response for connection: %p, serial: %u\n", nnpci, feci->ci, serial);
	DEBUG_TRACE("%p: rule_flags: %x, valid_flags: %x\n", nnpci, nircm->rule_flags, nircm->valid_flags);
	DEBUG_TRACE("%p: flow_ip: %pI4h:%d\n", nnpci, &nircm->tuple.flow_ip, nircm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4h:%d\n", nnpci, &nircm->tuple.return_ip, nircm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", nnpci, nircm->tuple.protocol);

	/*
	 * Handle the creation result code.
	 */
	DEBUG_TRACE("%p: response: %d\n", nnpci, nim->cm.response);
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		/*
		 * Creation command failed (specific reason ignored).
		 */
		DEBUG_TRACE("%p: accel nack: %d\n", nnpci, nim->cm.error);
		spin_lock_bh(&feci->lock);
		DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Unexpected mode: %d\n", ci, feci->accel_mode);
		feci->stats.ae_nack++;
		feci->stats.ae_nack_total++;
		if (feci->stats.ae_nack >= feci->stats.ae_nack_limit) {
			/*
			 * Too many NSS rejections
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

		spin_lock_bh(&ecm_nss_ipv4_lock);
		_ecm_nss_ipv4_accel_pending_clear(feci, result_mode);
		spin_unlock_bh(&ecm_nss_ipv4_lock);

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
	if (feci->stats.flush_happened) {
		feci->stats.flush_happened = false;

		/*
		 * Increment the no-action counter.  Our connection was decelerated on us with no action occurring.
		 */
		feci->stats.no_action_seen++;

		spin_lock_bh(&ecm_nss_ipv4_lock);
		_ecm_nss_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		spin_unlock_bh(&ecm_nss_ipv4_lock);

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
	spin_lock_bh(&ecm_nss_ipv4_lock);

	ecm_nss_non_ported_ipv4_accelerated_count++;	/* Protocol specific counter */
	ecm_nss_ipv4_accelerated_count++;		/* General running counter */

	if (!_ecm_nss_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
		/*
		 * Increment the no-action counter, this is reset if offload action is seen
		 */
		feci->stats.no_action_seen++;

		spin_unlock_bh(&ecm_nss_ipv4_lock);
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		feci->deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_INFO("%p: Decelerate was pending\n", ci);

	/*
	 * Check if the pending decelerate was done with the defunct process.
	 * If it was, set the is_defunct flag of the feci to false for re-try.
	 */
	if (feci->is_defunct) {
		is_defunct = feci->is_defunct;
		feci->is_defunct = false;
	}

	spin_unlock_bh(&ecm_nss_ipv4_lock);
	spin_unlock_bh(&feci->lock);

	/*
	 * If the pending decelerate was done through defunct process, we should
	 * re-try it here with the same defunct function, because the purpose of that
	 * process is to remove the connection from the database as well after decelerating it.
	 */
	if (is_defunct) {
		ecm_db_connection_make_defunct(ci);
	} else {
		feci->decelerate(feci);
	}

	/*
	 * Release the connection.
	 */
	feci->deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_nss_non_ported_ipv4_connection_accelerate()
 *	Accelerate a connection
 *
 * GGG TODO Refactor this function into a single function that np, udp and tcp
 * can all use and reduce the amount of code!
 */
static void ecm_nss_non_ported_ipv4_connection_accelerate(struct ecm_front_end_connection_instance *feci, bool is_l2_encap,
									struct ecm_classifier_process_response *pr)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;
	uint16_t regen_occurrances;
	int32_t from_ifaces_first;
	int32_t to_ifaces_first;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_nss_iface;
	struct ecm_db_iface_instance *to_nss_iface;
	int32_t from_nss_iface_id;
	int32_t to_nss_iface_id;
	uint8_t from_nss_iface_address[ETH_ALEN];
	uint8_t to_nss_iface_address[ETH_ALEN];
	ip_addr_t addr;
#if defined(ECM_INTERFACE_L2TPV2_ENABLE) ||  defined(ECM_INTERFACE_PPTP_ENABLE) || defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
	struct net_device *dev __attribute__((unused));
#endif
	struct nss_ipv4_msg *nim;
	struct nss_ipv4_rule_create_msg *nircm;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	nss_tx_status_t nss_tx_status;
	int32_t list_index;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	uint8_t dest_mac_xlate[ETH_ALEN];
	ecm_db_direction_t ecm_dir;
	ecm_front_end_acceleration_mode_t result_mode;
#ifdef ECM_INTERFACE_PPTP_ENABLE
	struct ecm_db_interface_info_pptp pptp_info;
#endif
	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	/*
	 * Test if acceleration is permitted
	 */
	if (!ecm_nss_ipv4_accel_pending_set(feci)) {
		DEBUG_TRACE("%p: Acceleration not permitted: %p\n", feci, feci->ci);
		return;
	}

	nim = (struct nss_ipv4_msg *)kzalloc(sizeof(struct nss_ipv4_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		DEBUG_WARN("%p: no memory for nss ipv4 message structure instance: %p\n", feci, feci->ci);
		ecm_nss_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		return;
	}

	/*
	 * Okay construct an accel command.
	 * Initialise creation structure.
	 * NOTE: We leverage the app_data void pointer to be our 32 bit connection serial number.
	 * When we get it back we re-cast it to a uint32 and do a faster connection lookup.
	 */
	nss_ipv4_msg_init(nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv4_rule_create_msg),
			ecm_nss_non_ported_ipv4_connection_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nircm = &nim->msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	nircm->vlan_primary_rule.ingress_vlan_tag = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_primary_rule.egress_vlan_tag = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.ingress_vlan_tag = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.egress_vlan_tag = ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Get the interface lists of the connection, we must have at least one interface in the list to continue
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in from_interfaces list!\n", nnpci);
		goto non_ported_accel_bad_rule;
	}

	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	if (to_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%p: Accel attempt failed - no interfaces in to_interfaces list!\n", nnpci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * First interface in each must be a known nss interface
	 */
	from_nss_iface = from_ifaces[from_ifaces_first];
	to_nss_iface = to_ifaces[to_ifaces_first];
	from_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(from_nss_iface);
	to_nss_iface_id = ecm_db_iface_ae_interface_identifier_get(to_nss_iface);
	if ((from_nss_iface_id < 0) || (to_nss_iface_id < 0)) {
		DEBUG_TRACE("%p: from_nss_iface_id: %d, to_nss_iface_id: %d\n", nnpci, from_nss_iface_id, to_nss_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Get NSS interface ID of the top interface in heirarchy
	 */
	from_nss_iface = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	to_nss_iface = to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	nircm->nexthop_rule.flow_nexthop = ecm_db_iface_ae_interface_identifier_get(from_nss_iface);
	nircm->nexthop_rule.return_nexthop = ecm_db_iface_ae_interface_identifier_get(to_nss_iface);

	/*
	 * New rule being created
	 */
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_CONN_VALID;

	/*
	 * Set Nexthop interface number valid flag
	 */
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_NEXTHOP_VALID;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	nircm->conn_rule.flow_interface_num = from_nss_iface_id;
	nircm->conn_rule.return_interface_num = to_nss_iface_id;

	/*
	 * Set up the flow and return qos tags
	 */
	nircm->qos_rule.flow_qos_tag = (uint32_t)pr->flow_qos_tag;
	nircm->qos_rule.return_qos_tag = (uint32_t)pr->return_qos_tag;
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_QOS_VALID;

	/*
	 * Set the mtu values. These values will be overwritten if the flow is
	 * a specific tunnel type.
	 */
	nircm->conn_rule.flow_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nircm->conn_rule.return_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	/*
	 * Set the port information. These ports can be overwritten by the PPTP protocol values,
	 * if the flow belongs to a PPTP tunnel.
	 */
	nircm->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nircm->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT);
	nircm->conn_rule.flow_ident_xlate = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT);
	nircm->conn_rule.return_ident_xlate = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	/*
	 * We know that each outward facing interface is known to the NSS and so this connection could be accelerated.
	 * However the lists may also specify other interesting details that must be included in the creation command,
	 * for example, ethernet MAC, VLAN tagging or PPPoE session information.
	 * We get this information by walking from the outer to the innermost interface for each list and examine the interface types.
	 *
	 * Start with the 'from' (src) side.
	 * NOTE: The lists may contain a complex heirarchy of similar type of interface e.g. multiple vlans or tunnels within tunnels.
	 * This NSS cannot handle that - there is no way to describe this in the rule - if we see multiple types that would conflict we have to abort.
	 */
	DEBUG_TRACE("%p: Examine from/src heirarchy list\n", nnpci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_in_dev = NULL;
#endif
#if defined(ECM_INTERFACE_GRE_TAP_ENABLE) ||  defined(ECM_INTERFACE_GRE_TUN_ENABLE) || defined(ECM_INTERFACE_PPTP_ENABLE)
		ip_addr_t saddr;
		ip_addr_t daddr;
#endif
		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", nnpci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {

		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%p: Bridge\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: Bridge - ignore additional\n", nnpci);
				break;
			}
			ecm_db_iface_bridge_address_get(ii, from_nss_iface_address);
			if (is_valid_ether_addr(from_nss_iface_address)) {
				memcpy(nircm->src_mac_rule.flow_src_mac, from_nss_iface_address, ETH_ALEN);
				nircm->src_mac_rule.mac_valid_flags |= NSS_IPV4_SRC_MAC_FLOW_VALID;
				nircm->valid_flags |= NSS_IPV4_RULE_CREATE_SRC_MAC_VALID;
			}

			DEBUG_TRACE("%p: Bridge - mac: %pM\n", nnpci, from_nss_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_ETHERNET:
			DEBUG_TRACE("%p: Ethernet\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Ignore additional mac addresses, these are usually as a result of address propagation
				 * from bridges down to ports etc.
				 */
				DEBUG_TRACE("%p: Ethernet - ignore additional\n", nnpci);
				break;
			}

#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
			dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (dev) {
				if (dev->priv_flags_ext & IFF_EXT_GRE_V4_TAP) {
					/*
					 * Clear QOS_VALID to prevent outer rule from overwriting
					 * inner flow's QoS classification.
					 */
					if (ecm_nss_common_get_interface_type(feci, dev) == NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER) {
						nircm->valid_flags &= ~NSS_IPV4_RULE_CREATE_QOS_VALID;
					}

					ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, saddr);
					ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, daddr);
					if (!ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_GRE_TAP, &(nircm->conn_rule.flow_mtu))) {
						rule_invalid = true;
						DEBUG_WARN("%p: Unable to get mtu value for the GRE TAP interface\n", nnpci);
					}

				}
				dev_put(dev);
			}
#endif

			/*
			 * Can only handle one MAC, the first outermost mac.
			 */
			ecm_db_iface_ethernet_address_get(ii, from_nss_iface_address);
			DEBUG_TRACE("%p: Ethernet - mac: %pM\n", nnpci, from_nss_iface_address);
			break;
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
		case ECM_DB_IFACE_TYPE_GRE_TUN:
			/*
			 * Clear QOS_VALID to prevent outer rule from overwriting
			 * inner flow's QoS classification.
			 */
			dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (dev) {
				if (ecm_nss_common_get_interface_type(feci, dev) == NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER) {
					nircm->valid_flags &= ~NSS_IPV4_RULE_CREATE_QOS_VALID;
				}
				dev_put(dev);
			}

			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, saddr);
			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, daddr);
			if (!ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_GRE_TUN, &(nircm->conn_rule.flow_mtu))) {
				rule_invalid = true;
				DEBUG_WARN("%p: Unable to get mtu value for the GRE TUN interface\n", nnpci);
			}
			break;
#endif

		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%p: PPPoE - additional unsupported\n", nnpci);
				rule_invalid = true;
				break;
			}

			/*
			 * Set the PPPoE rule creation structure.
			 */
			nircm->pppoe_rule.flow_if_num = ecm_db_iface_ae_interface_identifier_get(ii);
			if (nircm->pppoe_rule.flow_if_num < 0) {
				DEBUG_TRACE("%p: PPPoE - acceleration engine flow interface (%d) is not valid\n",
						nnpci, nircm->pppoe_rule.flow_if_num);
				rule_invalid = true;
				break;
			}
			nircm->pppoe_rule.flow_if_exist = 1;
			nircm->valid_flags |= NSS_IPV4_RULE_CREATE_PPPOE_VALID;

			DEBUG_TRACE("%p: PPPoE - exist: %d flow_if_num: %d\n", nnpci,
					nircm->pppoe_rule.flow_if_exist,
					nircm->pppoe_rule.flow_if_num);
#else
			rule_invalid = true;
#endif
			break;
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			DEBUG_TRACE("%p: VLAN\n", nnpci);
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - additional unsupported\n", nnpci);
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
			nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			memcpy(from_nss_iface_address, vlan_info.address, ETH_ALEN);
			if (is_valid_ether_addr(from_nss_iface_address)) {
				DEBUG_TRACE("%p: VLAN use mac: %pM\n", nnpci, from_nss_iface_address);
				interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
				memcpy(nircm->src_mac_rule.flow_src_mac, from_nss_iface_address, ETH_ALEN);
				nircm->src_mac_rule.mac_valid_flags |= NSS_IPV4_SRC_MAC_FLOW_VALID;
				nircm->valid_flags |= NSS_IPV4_RULE_CREATE_SRC_MAC_VALID;
			}
			DEBUG_TRACE("%p: vlan tag: %x\n", nnpci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: VLAN - unsupported\n", nnpci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%p: IPSEC\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: IPSEC - additional unsupported\n", nnpci);
				break;
			}

			nircm->conn_rule.flow_interface_num = ecm_nss_common_ipsec_get_ifnum(from_nss_iface_id);
			nircm->nexthop_rule.flow_nexthop = ecm_nss_common_ipsec_get_ifnum(nircm->nexthop_rule.flow_nexthop);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: IPSEC - unsupported\n", nnpci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_PPTP:
#ifdef ECM_INTERFACE_PPTP_ENABLE
			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, saddr);
			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, daddr);
			if (!ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_PPTP, &(nircm->conn_rule.flow_mtu))) {
				rule_invalid = true;
				DEBUG_WARN("%p: Unable to get mtu value for the PPTP interface\n", nnpci);
				break;
			}

			ecm_db_iface_pptp_session_info_get(ii, &pptp_info);

			nircm->tuple.flow_ident = ntohs(pptp_info.src_call_id);
			nircm->tuple.return_ident = ntohs(pptp_info.dst_call_id);
			nircm->conn_rule.flow_ident_xlate = ntohs(pptp_info.src_call_id);
			nircm->conn_rule.return_ident_xlate = ntohs(pptp_info.dst_call_id);

			nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_NO_SRC_IDENT;
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: PPTP - unsupported\n", nnpci);
#endif
			break;
		default:
			DEBUG_TRACE("%p: Ignoring: %d (%s)\n", nnpci, ii_type, ii_name);
		}

		/*
		 * Seen an interface of this type
		 */
		interface_type_counts[ii_type]++;
	}
	if (rule_invalid) {
		DEBUG_WARN("%p: from/src Rule invalid\n", nnpci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%p: Examine to/dest heirarchy list\n", nnpci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = to_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_out_dev = NULL;
#endif

		ii = to_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%p: list_index: %d, ii: %p, type: %d (%s)\n", nnpci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {
		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%p: Bridge\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: Bridge - ignore additional\n", nnpci);
				break;
			}
			ecm_db_iface_bridge_address_get(ii, to_nss_iface_address);
			if (is_valid_ether_addr(to_nss_iface_address)) {
				memcpy(nircm->src_mac_rule.return_src_mac, to_nss_iface_address, ETH_ALEN);
				nircm->src_mac_rule.mac_valid_flags |= NSS_IPV4_SRC_MAC_RETURN_VALID;
				nircm->valid_flags |= NSS_IPV4_RULE_CREATE_SRC_MAC_VALID;
			}

			DEBUG_TRACE("%p: Bridge - mac: %pM\n", nnpci, to_nss_iface_address);
			break;
		case ECM_DB_IFACE_TYPE_ETHERNET:
			DEBUG_TRACE("%p: Ethernet\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Ignore additional mac addresses, these are usually as a result of address propagation
				 * from bridges down to ports etc.
				 */
				DEBUG_TRACE("%p: Ethernet - ignore additional\n", nnpci);
				break;
			}

			/*
			 * Can only handle one MAC, the first outermost mac.
			 */
			ecm_db_iface_ethernet_address_get(ii, to_nss_iface_address);
			DEBUG_TRACE("%p: Ethernet - mac: %pM\n", nnpci, to_nss_iface_address);
			break;

		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%p: PPPoE - additional unsupported\n", nnpci);
				rule_invalid = true;
				break;
			}

			/*
			 * Set the PPPoE rule creation structure.
			 */
			nircm->pppoe_rule.return_if_num = ecm_db_iface_ae_interface_identifier_get(ii);
			if (nircm->pppoe_rule.return_if_num < 0) {
				DEBUG_TRACE("%p: PPPoE - acceleration engine return interface (%d) is not valid\n",
						nnpci, nircm->pppoe_rule.return_if_num);
				rule_invalid = true;
				break;
			}
			nircm->pppoe_rule.return_if_exist = 1;
			nircm->valid_flags |= NSS_IPV4_RULE_CREATE_PPPOE_VALID;

			DEBUG_TRACE("%p: PPPoE - exist: %d return_if_num: %d\n", nnpci,
					nircm->pppoe_rule.return_if_exist,
					nircm->pppoe_rule.return_if_num);
#else
			rule_invalid = true;
#endif
			break;
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			DEBUG_TRACE("%p: VLAN\n", nnpci);
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: VLAN - additional unsupported\n", nnpci);
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
			nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			memcpy(to_nss_iface_address, vlan_info.address, ETH_ALEN);
			if (is_valid_ether_addr(to_nss_iface_address)) {
				DEBUG_TRACE("%p: VLAN use mac: %pM\n", nnpci, to_nss_iface_address);
				interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
				memcpy(nircm->src_mac_rule.return_src_mac, to_nss_iface_address, ETH_ALEN);
				nircm->src_mac_rule.mac_valid_flags |= NSS_IPV4_SRC_MAC_RETURN_VALID;
				nircm->valid_flags |= NSS_IPV4_RULE_CREATE_SRC_MAC_VALID;
			}

			DEBUG_TRACE("%p: vlan tag: %x\n", nnpci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: VLAN - unsupported\n", nnpci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%p: IPSEC\n", nnpci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%p: IPSEC - additional unsupported\n", nnpci);
				break;
			}

			nircm->conn_rule.return_interface_num = ecm_nss_common_ipsec_get_ifnum(to_nss_iface_id);
			nircm->nexthop_rule.return_nexthop = ecm_nss_common_ipsec_get_ifnum(nircm->nexthop_rule.return_nexthop);
#else
			rule_invalid = true;
			DEBUG_TRACE("%p: IPSEC - unsupported\n", nnpci);
#endif
			break;
		default:
			DEBUG_TRACE("%p: Ignoring: %d (%s)\n", nnpci, ii_type, ii_name);
		}

		/*
		 * Seen an interface of this type
		 */
		interface_type_counts[ii_type]++;
	}
	if (rule_invalid) {
		DEBUG_WARN("%p: to/dest Rule invalid\n", nnpci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Routed or bridged?
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ROUTED;
	} else {
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_BRIDGE_FLOW;
		if (is_l2_encap) {
			nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_L2_ENCAP;
		}
	}

	if (ecm_interface_src_check) {
		DEBUG_INFO("%p: Source interface check flag is enabled\n", nnpci);
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_SRC_INTERFACE_CHECK;
	}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
		nircm->dscp_rule.flow_dscp = pr->flow_dscp;
		nircm->dscp_rule.return_dscp = pr->return_dscp;
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_DSCP_MARKING_VALID;
	}
#endif
	/*
	 * Set protocol
	 */
	nircm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * The flow_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nircm->tuple.flow_ip, addr);

	/*
	 * The return_ip is where the connection is established to, however, in the case of ingress
	 * the return_ip would be the routers WAN IP - i.e. the NAT'ed version.
	 * Getting the NAT'ed version here works for ingress or egress packets, for egress
	 * the NAT'ed version would be the same as the normal address
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nircm->tuple.return_ip, addr);

	/*
	 * When the packet is forwarded to the next interface get the address the source IP of the
	 * packet should be translated to.  For egress this is the NAT'ed from address.
	 * This also works for ingress as the NAT'ed version of the WAN host would be the same as non-NAT'ed
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nircm->conn_rule.flow_ip_xlate, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nircm->conn_rule.return_ip_xlate, addr);

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
	 * Refer to the Example 2 and 3 in ecm_nss_ipv4_ip_process() function for egress
	 * and ingress NAT'ed cases. In these cases, the destination node is the one which has the
	 * ip_dest_addr. So, above we get the mac address of this host and use that mac address
	 * for the destination node address in NAT'ed cases.
	 */
	ecm_dir = ecm_db_connection_direction_get(feci->ci);
	if ((ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT)) {
		memcpy(nircm->conn_rule.return_mac, dest_mac_xlate, ETH_ALEN);
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
		DEBUG_TRACE("%p: sync from: %p, type: %d\n", nnpci, aci, aci->type_get(aci));
		aci->sync_from_v4(aci, &ecrc);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Release the interface lists
	 */
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);

	DEBUG_INFO("%p: Non-Ported Accelerate connection %p\n"
			"Protocol: %d\n"
			"from_mtu: %u\n"
			"to_mtu: %u\n"
			"from_ip: %pI4h:%d\n"
			"to_ip: %pI4h:%d\n"
			"from_ip_xlate: %pI4h:%d\n"
			"to_ip_xlate: %pI4h:%d\n"
			"from_mac: %pM\n"
			"to_mac: %pM\n"
			"src_iface_num: %u\n"
			"dest_iface_num: %u\n"
			"src_nexthop_num: %u\n"
			"dest_nexthop_num: %u\n"
			"ingress_inner_vlan_tag: %u\n"
			"egress_inner_vlan_tag: %u\n"
			"ingress_outer_vlan_tag: %u\n"
			"egress_outer_vlan_tag: %u\n"
			"rule_flags: %x\n"
			"valid_flags: %x\n"
			"pppoe_return_if_exist: %u\n"
			"pppoe_return_if_num: %u\n"
			"pppoe_flow_if_exist: %u\n"
			"pppoe_flow_if_num: %u\n"
			"flow_qos_tag: %x (%u)\n"
			"return_qos_tag: %x (%u)\n"
			"flow_dscp: %x\n"
			"return_dscp: %x\n",
			nnpci,
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
			nircm->nexthop_rule.flow_nexthop,
			nircm->nexthop_rule.return_nexthop,
			nircm->vlan_primary_rule.ingress_vlan_tag,
			nircm->vlan_primary_rule.egress_vlan_tag,
			nircm->vlan_secondary_rule.ingress_vlan_tag,
			nircm->vlan_secondary_rule.egress_vlan_tag,
			nircm->rule_flags,
			nircm->valid_flags,
			nircm->pppoe_rule.return_if_exist,
			nircm->pppoe_rule.return_if_num,
			nircm->pppoe_rule.flow_if_exist,
			nircm->pppoe_rule.flow_if_num,
			nircm->qos_rule.flow_qos_tag, nircm->qos_rule.flow_qos_tag,
			nircm->qos_rule.return_qos_tag, nircm->qos_rule.return_qos_tag,
			nircm->dscp_rule.flow_dscp,
			nircm->dscp_rule.return_dscp);

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
		feci->stats.driver_fail = 0;
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return;
	}

	kfree(nim);

	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%p: Accel mode unexpected: %d\n", nnpci, feci->accel_mode);
	feci->stats.driver_fail_total++;
	feci->stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%p: Accel failed - driver fail limit\n", nnpci);
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	} else {
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	spin_lock_bh(&ecm_nss_ipv4_lock);
	_ecm_nss_ipv4_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_nss_ipv4_lock);

	spin_unlock_bh(&feci->lock);
	return;

non_ported_accel_bad_rule:
	;

	kfree(nim);
	/*
	 * Jump to here when rule data is bad and an offload command cannot be constructed
	 */
	DEBUG_WARN("%p: Accel failed - bad rule\n", nnpci);
	ecm_nss_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE);
}

/*
 * ecm_nss_non_ported_ipv4_connection_destroy_callback()
 *	Callback for handling destroy ack/nack calls.
 */
static void ecm_nss_non_ported_ipv4_connection_destroy_callback(void *app_data, struct nss_ipv4_msg *nim)
{
	struct nss_ipv4_rule_destroy_msg *__attribute__((unused))nirdm = &nim->msg.rule_destroy;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci;

	/*
	 * Is this a response to a destroy message?
	 */
	if (nim->cm.type != NSS_IPV4_TX_DESTROY_RULE_MSG) {
		DEBUG_ERROR("%p: non-ported destroy callback with improper type: %d\n", nim, nim->cm.type);
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
	nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;
	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	/*
	 * Record command duration
	 */
	ecm_nss_ipv4_decel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%p: decelerate response for connection: %p\n", nnpci, feci->ci);
	DEBUG_TRACE("%p: flow_ip: %pI4h:%d\n", nnpci, &nirdm->tuple.flow_ip, nirdm->tuple.flow_ident);
	DEBUG_TRACE("%p: return_ip: %pI4h:%d\n", nnpci, &nirdm->tuple.return_ip, nirdm->tuple.return_ident);
	DEBUG_TRACE("%p: protocol: %d\n", nnpci, nirdm->tuple.protocol);

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

	DEBUG_TRACE("%p: response: %d\n", nnpci, nim->cm.response);
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
	 * Non-Ported acceleration ends
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_non_ported_ipv4_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_nss_non_ported_ipv4_accelerated_count >= 0, "Bad non-ported accel counter\n");
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
 * ecm_nss_non_ported_ipv4_connection_decelerate_msg_send()
 *	Prepares and sends a decelerate message to acceleration engine.
 */
static bool ecm_nss_non_ported_ipv4_connection_decelerate_msg_send(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;
	struct nss_ipv4_msg nim;
	struct nss_ipv4_rule_destroy_msg *nirdm;
	ip_addr_t addr;
	nss_tx_status_t nss_tx_status;
	bool ret;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

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
			ecm_nss_non_ported_ipv4_connection_destroy_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nirdm = &nim.msg.rule_destroy;
	nirdm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nirdm->tuple.flow_ip, addr);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_HIN4_ADDR(nirdm->tuple.return_ip, addr);
	nirdm->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nirdm->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT);

#ifdef ECM_INTERFACE_PPTP_ENABLE
	if (nirdm->tuple.protocol == IPPROTO_GRE) {
		struct ecm_db_interface_info_pptp pptp_info;
		struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_ifaces_first;
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;

		/*
		 * Get the interface lists of the connection, we must have at least one interface in the list to continue
		 */
		from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
		if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("%p: Decel attempt failed - no interfaces in from_interfaces list!\n", nnpci);
			spin_lock_bh(&feci->lock);
			spin_unlock_bh(&feci->lock);
			return false;
		}

		ii = from_ifaces[from_ifaces_first];
		ii_type = ecm_db_iface_type_get(ii);
		DEBUG_TRACE("%p: iface_first: %d, ii: %p, type: %d (%s)\n", nnpci, from_ifaces_first, ii, ii_type, ecm_db_interface_type_to_string(ii_type));

		/*
		 * For PPTP flows, use PPTP local/peer call-id in place of L4 ports
		 */
		if (ECM_DB_IFACE_TYPE_PPTP == ii_type) {
			ecm_db_iface_pptp_session_info_get(ii, &pptp_info);
			nirdm->tuple.flow_ident = ntohs(pptp_info.src_call_id);
			nirdm->tuple.return_ident = ntohs(pptp_info.dst_call_id);
		}
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	}
#endif
	DEBUG_INFO("%p: Non-Ported Connection %p decelerate\n"
			"protocol: %d\n"
			"src_ip: %pI4:%d\n"
			"dest_ip: %pI4:%d\n",
			nnpci, feci->ci, nirdm->tuple.protocol,
			&nirdm->tuple.flow_ip, nirdm->tuple.flow_ident,
			&nirdm->tuple.return_ip, nirdm->tuple.return_ident);

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
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;			/* Reset */
		ret = feci->accel_mode;
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
 * ecm_nss_non_ported_ipv4_connection_decelerate()
 *     Decelerate a connection
 */
static bool ecm_nss_non_ported_ipv4_connection_decelerate(struct ecm_front_end_connection_instance *feci)
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

	return ecm_nss_non_ported_ipv4_connection_decelerate_msg_send(feci);
}

/*
 * ecm_nss_non_ported_ipv4_connection_defunct_callback()
 *	Callback to be called when a non-ported connection has become defunct.
 */
static bool ecm_nss_non_ported_ipv4_connection_defunct_callback(void *arg, int *accel_mode)
{
	bool ret;
	struct ecm_front_end_connection_instance *feci = (struct ecm_front_end_connection_instance *)arg;
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

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

	ret = ecm_nss_non_ported_ipv4_connection_decelerate_msg_send(feci);

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
 * ecm_nss_non_ported_ipv4_connection_accel_state_get()
 *	Get acceleration state
 */
static ecm_front_end_acceleration_mode_t ecm_nss_non_ported_ipv4_connection_accel_state_get(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;
	ecm_front_end_acceleration_mode_t state;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);
	spin_lock_bh(&feci->lock);
	state = feci->accel_mode;
	spin_unlock_bh(&feci->lock);
	return state;
}

/*
 * ecm_nss_non_ported_ipv4_connection_action_seen()
 *	Acceleration action / activity has been seen for this connection.
 *
 * NOTE: Call the action_seen() method when the NSS has demonstrated that it has offloaded some data for a connection.
 */
static void ecm_nss_non_ported_ipv4_connection_action_seen(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);
	DEBUG_INFO("%p: Action seen\n", nnpci);
	spin_lock_bh(&feci->lock);
	feci->stats.no_action_seen = 0;
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_nss_non_ported_ipv4_connection_accel_ceased()
 *	NSS has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an NSS self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_nss_non_ported_ipv4_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	DEBUG_INFO("%p: accel ceased\n", nnpci);

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
	 * Non-Ported acceleration ends
	 */
	spin_lock_bh(&ecm_nss_ipv4_lock);
	ecm_nss_non_ported_ipv4_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_nss_non_ported_ipv4_accelerated_count >= 0, "Bad non-ported accel counter\n");
	ecm_nss_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_nss_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_nss_ipv4_lock);
}

/*
 * ecm_nss_non_ported_ipv4_connection_ref()
 *	Ref a connection front end instance
 */
static void ecm_nss_non_ported_ipv4_connection_ref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);
	spin_lock_bh(&feci->lock);
	feci->refs++;
	DEBUG_TRACE("%p: nnpci ref %d\n", nnpci, feci->refs);
	DEBUG_ASSERT(feci->refs > 0, "%p: ref wrap\n", nnpci);
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_nss_non_ported_ipv4_connection_deref()
 *	Deref a connection front end instance
 */
static int ecm_nss_non_ported_ipv4_connection_deref(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	spin_lock_bh(&feci->lock);
	feci->refs--;
	DEBUG_ASSERT(feci->refs >= 0, "%p: ref wrap\n", nnpci);

	if (feci->refs > 0) {
		int refs = feci->refs;
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%p: nnpci deref %d\n", nnpci, refs);
		return refs;
	}
	spin_unlock_bh(&feci->lock);

	/*
	 * We can now destroy the instance
	 */
	DEBUG_TRACE("%p: nnpci final\n", nnpci);
	DEBUG_CLEAR_MAGIC(nnpci);
	kfree(nnpci);

	return 0;
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_nss_non_ported_ipv4_connection_state_get()
 *	Return the state of this Non ported front end instance
 */
static int ecm_nss_non_ported_ipv4_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	int result;
	bool can_accel;
	ecm_front_end_acceleration_mode_t accel_mode;
	struct ecm_front_end_connection_mode_stats stats;
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)feci;

	DEBUG_CHECK_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", nnpci);

	spin_lock_bh(&feci->lock);
	can_accel = feci->can_accel;
	accel_mode = feci->accel_mode;
	memcpy(&stats, &feci->stats, sizeof(struct ecm_front_end_connection_mode_stats));
	spin_unlock_bh(&feci->lock);

	if ((result = ecm_state_prefix_add(sfi, "front_end_v4.non_ported"))) {
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
 * ecm_nss_non_ported_ipv4_connection_instance_alloc()
 *	Create a front end instance specific for non-ported connection
 */
static struct ecm_nss_non_ported_ipv4_connection_instance *ecm_nss_non_ported_ipv4_connection_instance_alloc(
								struct ecm_db_connection_instance *ci,
								int protocol, bool can_accel)
{
	struct ecm_nss_non_ported_ipv4_connection_instance *nnpci;
	struct ecm_front_end_connection_instance *feci;

	nnpci = (struct ecm_nss_non_ported_ipv4_connection_instance *)kzalloc(sizeof(struct ecm_nss_non_ported_ipv4_connection_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!nnpci) {
		DEBUG_WARN("Non-Ported Front end alloc failed\n");
		return NULL;
	}

	/*
	 * Refs is 1 for the creator of the connection
	 */
	feci = (struct ecm_front_end_connection_instance *)nnpci;
	feci->refs = 1;
	DEBUG_SET_MAGIC(nnpci, ECM_NSS_NON_PORTED_IPV4_CONNECTION_INSTANCE_MAGIC);
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

	feci->protocol = protocol;

	/*
	 * Populate the methods and callbacks
	 */
	feci->ref = ecm_nss_non_ported_ipv4_connection_ref;
	feci->deref = ecm_nss_non_ported_ipv4_connection_deref;
	feci->decelerate = ecm_nss_non_ported_ipv4_connection_decelerate;
	feci->accel_state_get = ecm_nss_non_ported_ipv4_connection_accel_state_get;
	feci->action_seen = ecm_nss_non_ported_ipv4_connection_action_seen;
	feci->accel_ceased = ecm_nss_non_ported_ipv4_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_nss_non_ported_ipv4_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_nss_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_nss_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_nss_common_get_interface_type;
	feci->regenerate = ecm_nss_common_connection_regenerate;

	return nnpci;
}

/*
 * ecm_nss_non_ported_ipv4_process()
 *	Process a protocol that does not have port based identifiers
 */
unsigned int ecm_nss_non_ported_ipv4_process(struct net_device *out_dev, struct net_device *out_dev_nat,
							struct net_device *in_dev, struct net_device *in_dev_nat,
							uint8_t *src_node_addr, uint8_t *src_node_addr_nat,
							uint8_t *dest_node_addr, uint8_t *dest_node_addr_nat,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb,
							struct ecm_tracker_ip_header *ip_hdr,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr, ip_addr_t ip_src_addr_nat, ip_addr_t ip_dest_addr_nat,
							uint16_t l2_encap_proto)
{
	struct ecm_db_connection_instance *ci;
	int protocol;
	int src_port;
	int src_port_nat;
	int dest_port;
	int dest_port_nat;
	ip_addr_t match_addr;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ecm_db_timer_group_t ci_orig_timer_group;
	struct ecm_classifier_process_response prevalent_pr;

	/*
	 * Look up a connection.
	 */
	protocol = (int)orig_tuple->dst.protonum;
#if defined(ECM_INTERFACE_PPTP_ENABLE) || defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
	if ((protocol != IPPROTO_IPV6) && (protocol != IPPROTO_ESP) && (protocol != IPPROTO_GRE)) {
#else
	if ((protocol != IPPROTO_IPV6) && (protocol != IPPROTO_ESP)) {
#endif
		DEBUG_TRACE("Unsupported non-ported protocol: %d, do not process.\n", protocol);
		return NF_ACCEPT;
	}

	src_port = 0;
	src_port_nat = 0;
	dest_port = 0;
	dest_port_nat = 0;

	DEBUG_TRACE("Non ported src: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "), dest: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "), dir %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), ECM_IP_ADDR_TO_DOT(ip_src_addr_nat), ECM_IP_ADDR_TO_DOT(ip_dest_addr),
				ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat), ecm_dir);

	ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);

	/*
	 * If there is no existing connection then create a new one.
	 */
	if (unlikely(!ci)) {
		struct ecm_db_mapping_instance *mi[ECM_DB_OBJ_DIR_MAX];
		struct ecm_db_node_instance *ni[ECM_DB_OBJ_DIR_MAX];
		struct ecm_classifier_default_instance *dci;
		struct ecm_front_end_connection_instance *feci;
		struct ecm_db_connection_instance *nci;
		ecm_classifier_type_t classifier_type;
		int32_t to_list_first;
		struct ecm_db_iface_instance *to_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t to_nat_list_first;
		struct ecm_db_iface_instance *to_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_list_first;
		struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_nat_list_first;
		struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_front_end_interface_construct_instance efeici;

		DEBUG_INFO("New non-ported connection from " ECM_IP_ADDR_DOT_FMT ":%u to " ECM_IP_ADDR_DOT_FMT ":%u protocol: %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), src_port, ECM_IP_ADDR_TO_DOT(ip_dest_addr), dest_port, protocol);

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
			return NF_ACCEPT;
		}
		spin_unlock_bh(&ecm_nss_ipv4_lock);

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
		feci = (struct ecm_front_end_connection_instance *)ecm_nss_non_ported_ipv4_connection_instance_alloc(nci, protocol, can_accel);
		if (!feci) {
			DEBUG_WARN("Failed to allocate front end\n");
			goto fail_1;
		}

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
		from_list_first = ecm_interface_heirarchy_construct(feci, from_list, efeici.from_dev, efeici.from_other_dev, ip_dest_addr, efeici.from_mac_lookup_ip_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, NULL, skb);
		if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from' heirarchy list\n");
			goto fail_3;
		}
		ecm_db_connection_interfaces_reset(nci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);

		DEBUG_TRACE("%p: Create source node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM] = ecm_nss_ipv4_node_establish_and_ref(feci, efeici.from_dev, efeici.from_mac_lookup_ip_addr, from_list, from_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_list, from_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish source node\n");
			goto fail_3;
		}

		DEBUG_TRACE("%p: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM] = ecm_nss_ipv4_mapping_establish_and_ref(ip_src_addr, src_port);
		if (!mi[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish src mapping\n");
			goto fail_4;
		}

		DEBUG_TRACE("%p: Create the 'to' interface heirarchy list\n", nci);
		to_list_first = ecm_interface_heirarchy_construct(feci, to_list, efeici.to_dev, efeici.to_other_dev, ip_src_addr, efeici.to_mac_lookup_ip_addr, ip_dest_addr, 4, protocol, out_dev, is_routed, in_dev, dest_node_addr, src_node_addr, NULL, skb);
		if (to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to' heirarchy list\n");
			goto fail_5;
		}
		ecm_db_connection_interfaces_reset(nci, to_list, to_list_first, ECM_DB_OBJ_DIR_TO);

		DEBUG_TRACE("%p: Create dest node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO] = ecm_nss_ipv4_node_establish_and_ref(feci, efeici.to_dev, efeici.to_mac_lookup_ip_addr, to_list, to_list_first, dest_node_addr, skb);
		ecm_db_connection_interfaces_deref(to_list, to_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO]) {
			DEBUG_WARN("Failed to establish dest node\n");
			goto fail_5;
		}

		DEBUG_TRACE("%p: Create dest mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_TO] = ecm_nss_ipv4_mapping_establish_and_ref(ip_dest_addr, dest_port);
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

		/*
		 * NOTE: For SIT tunnels use the in_dev instead of in_dev_nat
		 */
		DEBUG_TRACE("%p: Create the 'from NAT' interface heirarchy list\n", nci);
		if ((protocol == IPPROTO_IPV6) || (protocol == IPPROTO_ESP)) {
			from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev, is_routed, in_dev, src_node_addr_nat, dest_node_addr_nat, NULL, skb);
		} else {
			from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev, src_node_addr_nat, dest_node_addr_nat, NULL, skb);
		}

		if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from NAT' heirarchy list\n");
			goto fail_7;
		}
		ecm_db_connection_interfaces_reset(nci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);

		DEBUG_TRACE("%p: Create source nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_nss_ipv4_node_establish_and_ref(feci, efeici.from_nat_dev, efeici.from_nat_mac_lookup_ip_addr, from_nat_list, from_nat_list_first, src_node_addr_nat, skb);
		ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish source nat node\n");
			goto fail_7;
		}

		mi[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_nss_ipv4_mapping_establish_and_ref(ip_src_addr_nat, src_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish src nat mapping\n");
			goto fail_8;
		}

		DEBUG_TRACE("%p: Create the 'to NAT' interface heirarchy list\n", nci);
		to_nat_list_first = ecm_interface_heirarchy_construct(feci, to_nat_list, efeici.to_nat_dev, efeici.to_nat_other_dev, ip_src_addr, efeici.to_nat_mac_lookup_ip_addr, ip_dest_addr_nat, 4, protocol, out_dev_nat, is_routed, in_dev, dest_node_addr_nat, src_node_addr_nat, NULL, skb);
		if (to_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to NAT' heirarchy list\n");
			goto fail_9;
		}
		ecm_db_connection_interfaces_reset(nci, to_nat_list, to_nat_list_first, ECM_DB_OBJ_DIR_TO_NAT);

		DEBUG_TRACE("%p: Create dest nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO_NAT] = ecm_nss_ipv4_node_establish_and_ref(feci, efeici.to_nat_dev, efeici.to_nat_mac_lookup_ip_addr, to_nat_list, to_nat_list_first, dest_node_addr_nat, skb);

		ecm_db_connection_interfaces_deref(to_nat_list, to_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest nat node\n");
			goto fail_9;
		}

		mi[ECM_DB_OBJ_DIR_TO_NAT] = ecm_nss_ipv4_mapping_establish_and_ref(ip_dest_addr_nat, dest_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest mapping\n");
			goto fail_10;
		}

		/*
		 * Every connection also needs a default classifier
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

		ecm_db_connection_l2_encap_proto_set(nci, l2_encap_proto);

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
					ecm_nss_non_ported_ipv4_connection_defunct_callback,
					tg, is_routed, nci);

			spin_unlock_bh(&ecm_nss_ipv4_lock);

			ci = nci;
			DEBUG_INFO("%p: New Non-ported protocol %d connection created\n", ci, protocol);
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
		ecm_nss_ipv4_connection_regenerate(ci, sender, out_dev, out_dev_nat, in_dev, in_dev_nat, NULL, skb);
	}

	/*
	 * Iterate the assignments and call to process!
	 * Policy implemented:
	 * 1. Classifiers that say they are not relevant are unassigned and not actioned further.
	 * 2. Any drop command from any classifier is honoured.
	 * 3. Accel is never allowed for non-ported type connections.
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
		aci->process(aci, sender, ip_hdr, skb, &aci_pr);
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

#ifdef ECM_INTERFACE_SIT_ENABLE
#ifdef CONFIG_IPV6_SIT_6RD
	/*
	 * SIT tunnel acceleration needs create a rule to the nss firmware if the
	 *	tunnel's dest ip address is empty,it will get dest ip and the embedded ipv6's dest ip
	 *	address in the packet and send them to the nss firmware to accelerate the
	 *	traffic on the tun6rd interface.
	 */
	if (protocol == IPPROTO_IPV6
			&& prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		struct ecm_front_end_connection_instance *feci;
		DEBUG_TRACE("%p: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		ecm_nss_non_ported_ipv4_sit_set_peer((struct ecm_nss_non_ported_ipv4_connection_instance *)feci, skb);
		feci->deref(feci);
	}
#endif
#endif
	/*
	 * Accelerate?
	 */
	if (prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		struct ecm_front_end_connection_instance *feci;
		DEBUG_TRACE("%p: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		ecm_nss_non_ported_ipv4_connection_accelerate(feci, is_l2_encap, &prevalent_pr);
		feci->deref(feci);
	}
	ecm_db_connection_deref(ci);

	return NF_ACCEPT;
}

/*
 * ecm_nss_non_ported_ipv4_debugfs_init()
 */
bool ecm_nss_non_ported_ipv4_debugfs_init(struct dentry *dentry)
{
	debugfs_create_u32("non_ported_accelerated_count", S_IRUGO, dentry,
					(u32 *)&ecm_nss_non_ported_ipv4_accelerated_count);

	return true;
}
