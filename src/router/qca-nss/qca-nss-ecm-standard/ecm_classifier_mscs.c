/*
 **************************************************************************
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/skbuff.h>
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
#include <net/esp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
#include <sp_api.h>
#endif

#include <linux/netfilter/xt_dscp.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_MSCS_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_front_end_common.h"
#include "ecm_classifier_mscs.h"
#include "exports/ecm_classifier_mscs_public.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC 0x1234
#define ECM_CLASSIFIER_MSCS_ACCEL_DELAY_PACKETS 0x4
#define ECM_CLASSIFIER_MSCS_UL_ACCEL_DELAY_PACKETS 0x14
#define ECM_CLASSIFIER_MSCS_INVALID_SPI 0xff
#define ECM_CLASSIFIER_MSCS_INVALID_RULE_ID 0xffff

/*
 * MSCS-SCS classifier type.
 */
enum ecm_classifier_mscs_scs_types {
	ECM_CLASSIFIER_MSCS = 1,
	ECM_CLASSIFIER_SCS,
};

/*
 * struct ecm_classifier_mscs_instance
 * 	State to allow tracking of MSCS QoS tag for a connection
 */
struct ecm_classifier_mscs_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_mscs_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_mscs_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	struct ecm_classifier_process_response process_response;/* Last process response computed */
	uint32_t priority[ECM_CONN_DIR_MAX];			/* Priority values for the connections */
	uint8_t packet_seen[ECM_CONN_DIR_MAX];			/* Per direction packet seen flag */
	bool scs_priority_update;				/* SCS rule match flag*/
	bool mscs_priority_update;				/* MSCS rule match flag*/

	int refs;						/* Integer to trap we never go negative */
	enum ecm_classifier_mscs_scs_types classifier_type;	/* Flag for which type of classifier classified the connection */
	uint32_t rule_id;					/* Rule id of the SCS rule match in SPM db */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
	uint16_t slow_ul_pkts;                                  /* count of slow ul packets */
};

/*
 * Operational control
 */
static int ecm_classifier_mscs_enabled = 0;			/* Operational behaviour */

/*
 * Operational control
 */
static int ecm_classifier_mscs_scs_multi_ap_enabled = 0;	/* Operational behaviour */

/*
 * Operational control
 */
static int ecm_classifier_scs_enabled = 0;			/* Operational behaviour */

/*
 * Operational control
 */
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
static int ecm_classifier_mscs_scs_udp_ipsec_port = 4500;	/* Operational behaviour */
#endif

/*
 * Management thread control
 */
static bool ecm_classifier_mscs_terminate_pending;	/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_mscs_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_mscs_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_mscs_instance *ecm_classifier_mscs_instances;
								/* list of all active instances */
static int ecm_classifier_mscs_count;			/* Tracks number of instances allocated */

/*
 * Callback structure to support MSCS peer lookup in external module
 */
static struct ecm_classifier_mscs_callbacks ecm_mscs;

/*
 * ecm_classifier_mscs_ref()
 *	Ref
 */
static void ecm_classifier_mscs_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	cmscsi = (struct ecm_classifier_mscs_instance *)ci;

	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);
	spin_lock_bh(&ecm_classifier_mscs_lock);
	cmscsi->refs++;
	DEBUG_TRACE("%px: cmscsi ref %d\n", cmscsi, cmscsi->refs);
	DEBUG_ASSERT(cmscsi->refs > 0, "%px: ref wrap\n", cmscsi);
	spin_unlock_bh(&ecm_classifier_mscs_lock);
}

/*
 * ecm_classifier_mscs_deref()
 *	Deref
 */
static int ecm_classifier_mscs_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	cmscsi = (struct ecm_classifier_mscs_instance *)ci;

	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);
	spin_lock_bh(&ecm_classifier_mscs_lock);
	cmscsi->refs--;
	DEBUG_ASSERT(cmscsi->refs >= 0, "%px: refs wrapped\n", cmscsi);
	DEBUG_TRACE("%px: MSCS classifier deref %d\n", cmscsi, cmscsi->refs);
	if (cmscsi->refs) {
		int refs = cmscsi->refs;
		spin_unlock_bh(&ecm_classifier_mscs_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_mscs_count--;
	DEBUG_ASSERT(ecm_classifier_mscs_count >= 0, "%px: ecm_classifier_mscs_count wrap\n", cmscsi);

	/*
	 * UnLink the instance from our list
	 */
	if (cmscsi->next) {
		cmscsi->next->prev = cmscsi->prev;
	}
	if (cmscsi->prev) {
		cmscsi->prev->next = cmscsi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_mscs_instances == cmscsi, "%px: list bad %px\n", cmscsi, ecm_classifier_mscs_instances);
		ecm_classifier_mscs_instances = cmscsi->next;
	}
	cmscsi->next = NULL;
	cmscsi->prev = NULL;
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final MSCS classifier instance\n", cmscsi);
	kfree(cmscsi);

	return 0;
}

/*
 * ecm_classifier_mscs_scs_is_bidi_packet_seen()
 *      Return true if both direction packets are seen.
 */
static inline bool ecm_classifier_mscs_scs_is_bidi_packet_seen(struct ecm_classifier_mscs_instance *cmscsi)
{
	return ((cmscsi->packet_seen[ECM_CONN_DIR_FLOW] == true) && (cmscsi->packet_seen[ECM_CONN_DIR_RETURN] == true));
}

/*
 * ecm_classifier_mscs_scs_fill_priority()
 *      Save the priority value in the classifier instance.
 */
static void ecm_classifier_mscs_scs_fill_priority(struct ecm_classifier_mscs_instance *cmscsi,
					ecm_tracker_sender_type_t sender, struct sk_buff *skb,
					bool scs_priority_update, bool mscs_rule_match,
					bool mscs_priority_update, bool scs_rule_match)
{
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		/*
		 * we update flow qos tag when scs rule match happens (as scs has precedence over mscs) or mscs has not applied,
		 * in case of mscs rule match we update bidirectional priority if scs has not updated reverse direction priority.
		 */
		if (scs_rule_match || !mscs_priority_update) {
			cmscsi->priority[ECM_CONN_DIR_FLOW] = skb->priority;
		} else if (mscs_rule_match) {
			cmscsi->priority[ECM_CONN_DIR_FLOW] = skb->priority;
			if (!scs_priority_update) {
				cmscsi->priority[ECM_CONN_DIR_RETURN] = skb->priority;
			}
		}

		cmscsi->packet_seen[ECM_CONN_DIR_FLOW] = true;
	} else {
		/*
		 * we update flow qos tag when scs rule match happens (as scs has precedence over mscs) or mscs has not applied,
		 * in case of mscs rule match we update bidirectional priority if scs has not updated reverse direction priority.
		 */
		if (scs_rule_match || !mscs_priority_update) {
			cmscsi->priority[ECM_CONN_DIR_RETURN] = skb->priority;
		} else if (mscs_rule_match) {
			cmscsi->priority[ECM_CONN_DIR_RETURN] = skb->priority;
			if (!scs_priority_update) {
				cmscsi->priority[ECM_CONN_DIR_FLOW] = skb->priority;
			}
		}

		cmscsi->packet_seen[ECM_CONN_DIR_RETURN] = true;
	}
}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
/*
 * ecm_classifier_mscs_scs_fill_input_params()
 *	Fills input params for SPM
 */
static bool ecm_classifier_mscs_scs_fill_input_params(struct sk_buff *skb,
						uint8_t *smac, uint8_t *dmac,
						struct sp_rule_input_params *flow_input_params,
						struct ecm_db_connection_instance *ci,
						ecm_tracker_sender_type_t sender) {
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	struct udphdr *udphdr;
	struct ip_esp_hdr *esp;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	uint16_t dscp;
	uint16_t version;

	/*
	 * Get the IP version and protocol information.
	 */
	flow_input_params->protocol = ecm_db_connection_protocol_get(ci);
	version = ecm_db_connection_ip_version_get(ci);
	flow_input_params->ip_version_type = version;

	/*
	 * Get the DSCP information from the packets IP header.
	 */
	if (version == 4) {
		if (unlikely(!pskb_may_pull(skb, sizeof(*iph)))) {
			/*
			 * Check for ip header
			 */
			DEBUG_INFO("No ip header in skb\n");
			return false;
		}

		iph = ip_hdr(skb);
		dscp = ipv4_get_dsfield(iph) >> XT_DSCP_SHIFT;
		flow_input_params->dscp = dscp;
        } else if (version == 6) {
		if (unlikely(!pskb_may_pull(skb, sizeof(*ip6h)))) {
			/*
			 * Check for ipv6 header
			 */
			DEBUG_INFO("No ipv6 header in skb\n");
			return false;
		}

		ip6h = ipv6_hdr(skb);
		dscp = ipv6_get_dsfield(ip6h) >> XT_DSCP_SHIFT;
		flow_input_params->dscp = dscp;
	} else {
		DEBUG_INFO("Invalid IP version: %d \n", version);
		return false;
	}

	/*
	 * Get the IP addresses and port information from ECM connection DB.
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
		flow_input_params->src.port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
		flow_input_params->dst.port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);
	} else {
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, src_ip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, dst_ip);
		flow_input_params->src.port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);
		flow_input_params->dst.port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
	}

	if (version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(flow_input_params->src.ip.ipv4_addr, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(flow_input_params->dst.ip.ipv4_addr, dst_ip);
	} else {
		ECM_IP_ADDR_TO_NET_IPV6_ADDR(flow_input_params->src.ip.ipv6_addr, src_ip);
		ECM_IP_ADDR_TO_NET_IPV6_ADDR(flow_input_params->dst.ip.ipv6_addr, dst_ip);
	}

	/*
	 * In case of IPSec / UDP encap IPSec protocol, get the SPI value from the
	 * header UDP / ESP header.
	 */
	flow_input_params->spi = ECM_CLASSIFIER_MSCS_INVALID_SPI;
	if (flow_input_params->protocol == IPPROTO_UDP) {
		/*
		 * Check for udp header
		 */
		if (unlikely(!pskb_may_pull(skb, sizeof(*udphdr)))) {
			DEBUG_INFO("No udp header in skb\n");
			return false;
		}

		/*
		 * TODO : Fetch UDP header using standard functions.
		 */
		if (version == 4) {
			udphdr = (struct udphdr*)((uint8_t *)iph + sizeof(*iph));
		} else {
			udphdr = (struct udphdr*)((uint8_t *)ip6h + sizeof(*ip6h));
		}

		/*
		 * Check for UDP encapsulated IPSEC packet.
		 */
		if (flow_input_params->dst.port == ecm_classifier_mscs_scs_udp_ipsec_port) {
			esp = (struct ip_esp_hdr *)((uint8_t *)udphdr + sizeof(*udphdr));
			flow_input_params->spi = ntohl(esp->spi);
		}

	} else if (flow_input_params->protocol == IPPROTO_ESP) {
		/*
		 * Get the SPI for IPSEC packets.
		 */
		if (version == 4) {
			esp = (struct ip_esp_hdr *)((uint8_t *)iph + sizeof(*iph));
			flow_input_params->spi = ntohl(esp->spi);
		} else {
			esp = (struct ip_esp_hdr *)((uint8_t *)ip6h + sizeof(*ip6h));
			flow_input_params->spi = ntohl(esp->spi);
		}
	}

	ether_addr_copy(flow_input_params->src.mac, smac);
	ether_addr_copy(flow_input_params->dst.mac, dmac);

	return true;
}
#endif

/*
 * ecm_classifier_mscs_process()
 *	Process new data for connection
 */
static void ecm_classifier_mscs_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	ecm_classifier_relevence_t relevance;
	struct ecm_db_connection_instance *ci = NULL;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_mode;
	uint32_t became_relevant = 0;
	ecm_classifier_mscs_process_callback_t cb = NULL;
	ecm_classifier_mscs_result_t result = 0;
	uint8_t smac[ETH_ALEN];
	uint8_t dmac[ETH_ALEN];
	bool mscs_rule_match = false;
	bool scs_rule_match = false;
	struct net_device *src_dev = NULL;
	struct net_device *dest_dev = NULL;
	uint64_t slow_pkts;
	struct ecm_classifier_mscs_get_priority_info get_priority_info = {0};
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	struct sp_rule_input_params flow_input_params;
	struct sp_rule_output_params flow_output_params;
	struct ecm_classifier_mscs_rule_match_info rule_match_info = {0};
	ecm_classifier_mscs_scs_priority_callback_t scs_cb = NULL;
#endif
#ifdef ECM_MULTICAST_ENABLE
	ip_addr_t dst_ip;
#endif

	cmscsi = (struct ecm_classifier_mscs_instance *)aci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);

	/*
	 * Are we yet to decide if this instance is relevant to the connection?
	 */
	spin_lock_bh(&ecm_classifier_mscs_lock);
	relevance = cmscsi->process_response.relevance;

	/*
	 * Are we relevant?
	 */
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		/*
		 * Lock still held
		 */
		goto mscs_classifier_out;
	}

	/*
	 * Set relevance no if both classifiers are disabled.
	 */
	if (!ecm_classifier_mscs_enabled && !ecm_classifier_scs_enabled) {
		/*
		 * Lock still held
		 */
		cmscsi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto mscs_classifier_out;
	}
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	/*
	 * Can we accelerate?
	 */
	ci = ecm_db_connection_serial_find_and_ref(cmscsi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cmscsi, cmscsi->ci_serial);
		spin_lock_bh(&ecm_classifier_mscs_lock);
		cmscsi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto mscs_classifier_out;
	}

	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = ecm_front_end_connection_accel_state_get(feci);
	ecm_front_end_connection_deref(feci);
	protocol = ecm_db_connection_protocol_get(ci);

	/*
	 * MSCS classifier is not applicable for Multicast Traffic
	 */
#ifdef ECM_MULTICAST_ENABLE
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	if (ecm_ip_addr_is_multicast(dst_ip)) {
		DEBUG_TRACE("%px: Multicast Traffic, skip MSCS / SCS classification\n", ci);
		ecm_db_connection_deref(ci);
		spin_lock_bh(&ecm_classifier_mscs_lock);
		cmscsi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto mscs_classifier_out;
	}
#endif

	/*
	 * Get the source mac address for this connection
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		DEBUG_TRACE("%px: sender is SRC\n", aci);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);
	} else {
		DEBUG_TRACE("%px: sender is DEST\n", aci);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, dmac);
	}

	ecm_db_netdevs_get_and_hold(ci, sender, &src_dev, &dest_dev);

	/*
	 * Set the invalid SCS rule id, in case if we do not find any SCS rule.
	 */
	cmscsi->rule_id = ECM_CLASSIFIER_MSCS_INVALID_RULE_ID;

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	/*
	 * Check if SCS classifier is enabled or not.
	 */
	if (ecm_classifier_scs_enabled) {

		if (!ecm_classifier_mscs_scs_fill_input_params(skb, smac, dmac, &flow_input_params, ci, sender)) {
			DEBUG_TRACE("%px: failed to fill in SCS input params\n", ci);
			goto check_mscs_classifier;
		}

		/*
		 * Invoke SPM rule lookup callback for the flow parameters.
		 * Check for MSCS classifier if Rule match fails or peer is not SCS capable.
		 */
		sp_mapdb_apply_scs(skb, &flow_input_params, &flow_output_params);

		if (flow_output_params.priority != SP_RULE_INVALID_PRIORITY) {
			DEBUG_INFO("%px: Found SCS rule in SPM\n", ci);
			/*
			 * Set result true for Multi AP mode.
			 */
			result = true;
			/*
			 * Invoke the WiFi datapath callback registered with MSCS client to check
			 * if SCS priority is valid for WiFi peer corresponding to
			 * destination mac address for Single AP mode.
			 */
			if (!ecm_classifier_mscs_scs_multi_ap_enabled) {
				scs_cb = ecm_mscs.update_skb_priority;
				if (!scs_cb) {
					DEBUG_TRACE("%px: No SCS callback is registered\n", ci);
					goto check_mscs_classifier;
				}

				spin_lock_bh(&ecm_classifier_mscs_lock);
				rule_match_info.rule_id = flow_output_params.rule_id;
				rule_match_info.dst_mac = dmac;
				rule_match_info.src_dev = src_dev;
				rule_match_info.dst_dev = dest_dev;
				result = scs_cb(&rule_match_info);
				spin_unlock_bh(&ecm_classifier_mscs_lock);
			}
		}

		/*
		 * Check the result of the callback. If we have a valid priority and peer is SCS
		 * capable, we set the priority (we do not check MSCS as SCS have higher precedence).
		 */
		if (result) {
			/*
			 * Update skb priority.
			 */
			skb->priority = flow_output_params.priority;
			spin_lock_bh(&ecm_classifier_mscs_lock);
			cmscsi->scs_priority_update = true;
			cmscsi->classifier_type = ECM_CLASSIFIER_SCS;
			scs_rule_match = true;
			cmscsi->rule_id = flow_output_params.rule_id;
			spin_unlock_bh(&ecm_classifier_mscs_lock);

			/*
			 * For IPSEC protocol, we update both side priority values and let it go via slow path.
			 * TODO: FIx the IPSEC acceleration issue.
			 */
			if (protocol == IPPROTO_ESP || (protocol == IPPROTO_UDP &&
				flow_input_params.dst.port == ecm_classifier_mscs_scs_udp_ipsec_port)) {
				ecm_db_connection_deref(ci);
				spin_lock_bh(&ecm_classifier_mscs_lock);
				cmscsi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
				cmscsi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				cmscsi->process_response.flow_qos_tag = skb->priority;
				cmscsi->process_response.return_qos_tag = skb->priority;
				cmscsi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
				goto mscs_classifier_out;
			}

			/*
			 * No need to check MSCS classifier if SCS result is true as SCS
			 * has higher precedence.
			 */
			goto mscs_classifier_exit;
		}


	}

	check_mscs_classifier:
#endif

	/*
	 * Check MSCS classifer.
	 */
	if (ecm_classifier_mscs_enabled) {
		result =  false;
		/*
		 * Check if MSCS multi AP mode is enabled or not -
		 * If yes, we need to query SPM database for rule match.
		 * Else legacy MSCS should work for single AP mode.
		 */
		if (!ecm_classifier_mscs_scs_multi_ap_enabled) {
			/*
			 * Get the WiFi datapath callback registered with MSCS client to check
			 * if MSCS QoS tag is valid for WiFi peer corresponding to
			 * skb->src_mac_addr
			 */
			cb = ecm_mscs.get_peer_priority;
			if (!cb) {
				DEBUG_TRACE("%px: No MSCS callback is registered\n", ci);
				goto mscs_classifier_exit;
			}

			/*
			 * Invoke callback registered to classifier for peer look up
			 */
			get_priority_info.src_mac = smac;
			get_priority_info.dst_mac = dmac;
			get_priority_info.src_dev = src_dev;
			get_priority_info.dst_dev = dest_dev;
			get_priority_info.skb = skb;
			result = cb(&get_priority_info);

			if (result == ECM_CLASSIFIER_MSCS_RESULT_UPDATE_PRIORITY) {
				spin_lock_bh(&ecm_classifier_mscs_lock);
				cmscsi->mscs_priority_update = true;
				mscs_rule_match = true;
				if (!cmscsi->scs_priority_update) {
					cmscsi->classifier_type = ECM_CLASSIFIER_MSCS;
				}
				spin_unlock_bh(&ecm_classifier_mscs_lock);
			}
		}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
		else {
			/*
			 * Invoke SPM rule lookup callback for the flow parameters for Multi AP mode.
			 */
			ether_addr_copy(flow_input_params.src.mac, smac);
			ether_addr_copy(flow_input_params.dst.mac, dmac);

			sp_mapdb_apply_mscs(skb, &flow_input_params, &flow_output_params);

			if (flow_output_params.priority != SP_RULE_INVALID_PRIORITY) {
				DEBUG_INFO("%px: Found MSCS rule in SPM\n", ci);
				skb->priority = flow_output_params.priority;
				spin_lock_bh(&ecm_classifier_mscs_lock);
				cmscsi->mscs_priority_update = true;
				mscs_rule_match = true;
				if (!cmscsi->scs_priority_update) {
					cmscsi->classifier_type = ECM_CLASSIFIER_MSCS;
					cmscsi->rule_id = flow_output_params.rule_id;
				}
				spin_unlock_bh(&ecm_classifier_mscs_lock);
			}
		}
#endif
	}

mscs_classifier_exit:
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = ecm_front_end_connection_accel_state_get(feci);
	slow_pkts = ecm_front_end_get_slow_packet_count(feci);
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);

	if (src_dev->ieee80211_ptr) {
		/*
		 * MSCS classification information comes in UL packets from WLAN side. 
		 * Waiting on slow packets to update right priority in ECM flow entry
		 */
		spin_lock_bh(&ecm_classifier_mscs_lock);
		cmscsi->slow_ul_pkts++;
		spin_unlock_bh(&ecm_classifier_mscs_lock);
	}

	if (ECM_FRONT_END_ACCELERATION_NOT_POSSIBLE(accel_mode)) {
		DEBUG_TRACE("%x: not relevant accel_mode: %d, this is a race condition while ae switch happens from ppe to sfe\n",feci->ci->serial, accel_mode);
	}

	/*
	 * We are relevant to the connection.
	 * Set the process response to its default value, that is, to
	 * allow the acceleration.
	 */
	became_relevant = ecm_db_time_get();

	spin_lock_bh(&ecm_classifier_mscs_lock);
	cmscsi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	cmscsi->process_response.became_relevant = became_relevant;
	cmscsi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;

	/*
	 * Store the priority value in the classifier instance.
	 */
	ecm_classifier_mscs_scs_fill_priority(cmscsi, sender, skb, cmscsi->scs_priority_update, mscs_rule_match, cmscsi->mscs_priority_update, scs_rule_match);

	if (ecm_classifier_scs_enabled) {
		/*
		 * If SCS classifier is enabled, give chance to SCS first for 
		 * ECM_CLASSIFIER_MSCS_ACCEL_DELAY_PACKETS packets to see if it
		 * has to update the priority, if not fall back to MSCS.
		 */
		if (slow_pkts < ECM_CLASSIFIER_MSCS_ACCEL_DELAY_PACKETS) {
			if (!scs_rule_match && cmscsi->process_response.accel_mode != ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
				cmscsi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				goto mscs_classifier_out;
			}
		}
	} else {
		if(cmscsi->slow_ul_pkts < ECM_CLASSIFIER_MSCS_UL_ACCEL_DELAY_PACKETS) {
			/*
			 * Deny acceleration for ECM_CLASSIFIER_MSCS_UL_ACCEL_DELAY_PACKETS
			 * slow uplink packets.
			 */
			cmscsi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
			goto mscs_classifier_out;
		}

		if (result == ECM_CLASSIFIER_MSCS_RESULT_DENY_PRIORITY) {
			/*
			 *  Do not accelerate if Uplink packet is not seen at all.
			 */
			cmscsi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
			goto mscs_classifier_out;
		}
	}

	DEBUG_TRACE("Protocol: %d, Flow Priority: %d, Return priority: %d, sender: %d\n",
			protocol, cmscsi->priority[ECM_CONN_DIR_FLOW],
			cmscsi->priority[ECM_CONN_DIR_RETURN], sender);

	cmscsi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	cmscsi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
	cmscsi->process_response.flow_qos_tag = cmscsi->priority[ECM_CONN_DIR_FLOW];
	cmscsi->process_response.return_qos_tag = cmscsi->priority[ECM_CONN_DIR_RETURN];

mscs_classifier_out:

	/*
	 * Return our process response
	 */
	if(src_dev)
		dev_put(src_dev);

	if(dest_dev)
		dev_put(dest_dev);

	*process_response = cmscsi->process_response;
	spin_unlock_bh(&ecm_classifier_mscs_lock);
}

/*
 * ecm_classifier_mscs_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_mscs_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_mscs_instance *cmscsi __attribute__((unused));

	cmscsi = (struct ecm_classifier_mscs_instance *)aci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed", cmscsi);
}

/*
 * ecm_classifier_mscs_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_mscs_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_mscs_instance *cmscsi __attribute__((unused));

	cmscsi = (struct ecm_classifier_mscs_instance *)aci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed", cmscsi);
}

/*
 * ecm_classifier_mscs_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_mscs_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_mscs_instance *cmscsi __attribute__((unused));

	cmscsi = (struct ecm_classifier_mscs_instance *)aci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed", cmscsi);
}

/*
 * ecm_classifier_mscs_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_mscs_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_mscs_instance *cmscsi __attribute__((unused));

	cmscsi = (struct ecm_classifier_mscs_instance *)aci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed", cmscsi);
}

/*
 * ecm_classifier_mscs_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_mscs_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	cmscsi = (struct ecm_classifier_mscs_instance *)ci;

	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);
	return ECM_CLASSIFIER_TYPE_MSCS;
}

/*
 * ecm_classifier_mscs_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_mscs_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_mscs_instance *cmscsi;

	cmscsi = (struct ecm_classifier_mscs_instance *)ci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);

	spin_lock_bh(&ecm_classifier_mscs_lock);
	*process_response = cmscsi->process_response;
	spin_unlock_bh(&ecm_classifier_mscs_lock);
}

/*
 * ecm_classifier_mscs_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_mscs_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	cmscsi = (struct ecm_classifier_mscs_instance *)ci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);

	return false;
}

/*
 * ecm_classifier_mscs_reclassify()
 *	Reclassify
 */
static void ecm_classifier_mscs_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;
	cmscsi = (struct ecm_classifier_mscs_instance *)ci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed\n", cmscsi);

}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_mscs_state_get()
 *	Return state
 */
static int ecm_classifier_mscs_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_mscs_instance *cmscsi;
	struct ecm_classifier_process_response process_response;

	cmscsi = (struct ecm_classifier_mscs_instance *)ci;
	DEBUG_CHECK_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC, "%px: magic failed", cmscsi);

	if ((result = ecm_state_prefix_add(sfi, "mscs"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_mscs_lock);
	process_response = cmscsi->process_response;
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_classifier_mscs_instance_alloc()
 *	Allocate an instance of the mscs classifier
 */
struct ecm_classifier_mscs_instance *ecm_classifier_mscs_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_mscs_instance *cmscsi;

	/*
	 * Allocate the instance
	 */
	cmscsi = (struct ecm_classifier_mscs_instance *)kzalloc(sizeof(struct ecm_classifier_mscs_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cmscsi) {
		DEBUG_WARN("Failed to allocate mscs instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(cmscsi, ECM_CLASSIFIER_MSCS_INSTANCE_MAGIC);
	cmscsi->refs = 1;
	cmscsi->base.process = ecm_classifier_mscs_process;
	cmscsi->base.sync_from_v4 = ecm_classifier_mscs_sync_from_v4;
	cmscsi->base.sync_to_v4 = ecm_classifier_mscs_sync_to_v4;
	cmscsi->base.sync_from_v6 = ecm_classifier_mscs_sync_from_v6;
	cmscsi->base.sync_to_v6 = ecm_classifier_mscs_sync_to_v6;
	cmscsi->base.type_get = ecm_classifier_mscs_type_get;
	cmscsi->base.last_process_response_get = ecm_classifier_mscs_last_process_response_get;
	cmscsi->base.reclassify_allowed = ecm_classifier_mscs_reclassify_allowed;
	cmscsi->base.reclassify = ecm_classifier_mscs_reclassify;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cmscsi->base.state_get = ecm_classifier_mscs_state_get;
#endif
	cmscsi->base.ref = ecm_classifier_mscs_ref;
	cmscsi->base.deref = ecm_classifier_mscs_deref;
	cmscsi->ci_serial = ecm_db_connection_serial_get(ci);
	cmscsi->process_response.process_actions = 0;
	cmscsi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	spin_lock_bh(&ecm_classifier_mscs_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_mscs_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_mscs_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		kfree(cmscsi);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	cmscsi->next = ecm_classifier_mscs_instances;
	if (ecm_classifier_mscs_instances) {
		ecm_classifier_mscs_instances->prev = cmscsi;
	}
	ecm_classifier_mscs_instances = cmscsi;

	/*
	 * Increment stats
	 */
	ecm_classifier_mscs_count++;
	DEBUG_ASSERT(ecm_classifier_mscs_count > 0, "%px: ecm_classifier_mscs_count wrap\n", cmscsi);
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	DEBUG_INFO("mscs instance alloc: %px\n", cmscsi);
	return cmscsi;
}
EXPORT_SYMBOL(ecm_classifier_mscs_instance_alloc);

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
/*
 * ecm_classifier_mscs_make_defunct_scs_connections()
 *	Defunct the SCS connections with the given rule id as the rule is being updated/
 *	deleted in the SPM db.
 */
static void ecm_classifier_mscs_make_defunct_scs_connections(uint32_t rule_id)
{
	struct ecm_db_connection_instance *ci;
	DEBUG_INFO("Make defunct all connections assigned to SCS\n");
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;
		struct ecm_classifier_instance *eci;
		struct ecm_classifier_mscs_instance *cmscsi;

		eci = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_MSCS);
		if (!eci) {
			goto next_ci;
		}

		cmscsi = (struct ecm_classifier_mscs_instance *)eci;
		if (cmscsi->rule_id == rule_id) {
			DEBUG_INFO("%px Defuncting the connection\n", ci);
			ecm_db_connection_make_defunct(ci);
		}

		eci->deref(eci);
next_ci:
		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
}

/*
 * ecm_classifier_mscs_spm_notifier_callback()
 *	Callback for Service prioritization notification update for SCS classifier.
 */
static int ecm_classifier_mscs_spm_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct sp_rule *r = (struct sp_rule *)data;
	uint32_t valid_flag = r->inner.flags_sawf;

	DEBUG_INFO("SP rule update notification received\n");
	if (r->classifier_type != SP_RULE_TYPE_SCS && r->classifier_type != SP_RULE_TYPE_MSCS) {
		DEBUG_INFO("Not an MSCS/SCS rule notification !\n");
		return NOTIFY_DONE;
	}

	switch(event) {
		case SP_MAPDB_REMOVE_RULE:
		case SP_MAPDB_MODIFY_RULE:
			ecm_classifier_mscs_make_defunct_scs_connections(r->id);
			break;
		case SP_MAPDB_ADD_RULE:
			/*
			 * At add rule notification, defunct already exsisting connections having the
			 * matching ports as the new rule added.
			 */
			if (valid_flag & SP_RULE_FLAG_MATCH_SAWF_SRC_PORT) {
				ecm_db_connection_defunct_by_port(htons(r->inner.src_port), ECM_DB_OBJ_DIR_FROM);
				ecm_db_connection_defunct_by_port(htons(r->inner.src_port), ECM_DB_OBJ_DIR_TO);
				return NOTIFY_DONE;
			}

			if (valid_flag & SP_RULE_FLAG_MATCH_SAWF_DST_PORT) {
				ecm_db_connection_defunct_by_port(htons(r->inner.dst_port), ECM_DB_OBJ_DIR_FROM);
				ecm_db_connection_defunct_by_port(htons(r->inner.dst_port), ECM_DB_OBJ_DIR_TO);
				return NOTIFY_DONE;
			}
			break;
	}

	return NOTIFY_DONE;
}
#endif

/*
 * ecm_classifier_mscs_rul_get_enabled()
 */
static int ecm_classifier_mscs_rule_get_enabled(void *data, u64 *val)
{
	*val = ecm_classifier_mscs_enabled;

	return 0;
}

/*
 * ecm_classifier_mscs_rule_set_enabled()
 */
static int ecm_classifier_mscs_rule_set_enabled(void *data, u64 val)
{
	DEBUG_TRACE("ecm_classifier_mscs_enabled = %u\n", (uint32_t)val);

	if ((val != 0) && (val != 1)) {
		DEBUG_WARN("Invalid value: %u. Valid values are 0 and 1.\n", (uint32_t)val);
		return -EINVAL;
	}

	ecm_classifier_mscs_enabled = (uint32_t)val;

	return 0;
}

/*
 * ecm_classifier_mscs_scs_multi_ap_rule_get_enabled()
 */
static int ecm_classifier_mscs_scs_multi_ap_rule_get_enabled(void *data, u64 *val)
{
	*val = ecm_classifier_mscs_scs_multi_ap_enabled;

	return 0;
}

/*
 * ecm_classifier_mscs_scs_multi_ap_rule_set_enabled(()
 */
static int ecm_classifier_mscs_scs_multi_ap_rule_set_enabled(void *data, u64 val)
{
	DEBUG_TRACE("ecm_classifier_mscs_scs_multi_ap_enabled = %u\n", (uint32_t)val);

	if ((val != 0) && (val != 1)) {
		DEBUG_WARN("Invalid value: %u. Valid values are 0 and 1.\n", (uint32_t)val);
		return -EINVAL;
	}

	ecm_classifier_mscs_scs_multi_ap_enabled = (uint32_t)val;

	return 0;
}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
/*
 * ecm_classifier_mscs_scs_rule_get_enabled()
 */
static int ecm_classifier_mscs_scs_rule_get_enabled(void *data, u64 *val)
{
	*val = ecm_classifier_scs_enabled;

	return 0;
}

/*
 * ecm_classifier_mscs_scs_rule_set_enabled()
 */
static int ecm_classifier_mscs_scs_rule_set_enabled(void *data, u64 val)
{
	DEBUG_TRACE("ecm_classifier_scs_enabled = %u\n", (uint32_t)val);

	if ((val != 0) && (val != 1)) {
		DEBUG_WARN("Invalid value: %u. Valid values are 0 and 1.\n", (uint32_t)val);
		return -EINVAL;
	}

	ecm_classifier_scs_enabled = (uint32_t)val;

	return 0;
}

/*
 * ecm_classifier_mscs_scs_get_udp_ipsec_port()
 */
static int ecm_classifier_mscs_scs_get_udp_ipsec_port(void *data, u64 *val)
{
	*val = ecm_classifier_mscs_scs_udp_ipsec_port;

	return 0;
}

/*
 * ecm_classifier_mscs_scs_set_udp_ipsec_port()
 */
static int ecm_classifier_mscs_scs_set_udp_ipsec_port(void *data, u64 val)
{
	DEBUG_TRACE("ecm_classifier_scs_udp_ipsec_port = %u\n", (uint32_t)val);

	if (val != 5200) {
		DEBUG_WARN("Invalid value: %u. Valid value is 5200.\n", (uint32_t)val);
		return -EINVAL;
	}

	ecm_classifier_mscs_scs_udp_ipsec_port = (uint32_t)val;

	return 0;
}
#endif

/*
 * Debugfs attribute for Emesh Enabled parameter.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_mscs_enabled_fops, ecm_classifier_mscs_rule_get_enabled, ecm_classifier_mscs_rule_set_enabled, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_mscs_scs_multi_ap_enabled_fops, ecm_classifier_mscs_scs_multi_ap_rule_get_enabled, ecm_classifier_mscs_scs_multi_ap_rule_set_enabled, "%llu\n");

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_scs_enabled_fops, ecm_classifier_mscs_scs_rule_get_enabled, ecm_classifier_mscs_scs_rule_set_enabled, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_scs_udp_ipsec_port_fops, ecm_classifier_mscs_scs_get_udp_ipsec_port, ecm_classifier_mscs_scs_set_udp_ipsec_port, "%llu\n");
#endif

/*
 * ecm_interface_ovpn_register
 */
int ecm_classifier_mscs_callback_register(struct ecm_classifier_mscs_callbacks *mscs_cb)
{
	spin_lock_bh(&ecm_classifier_mscs_lock);
	if (ecm_mscs.get_peer_priority) {
		spin_unlock_bh(&ecm_classifier_mscs_lock);
		DEBUG_ERROR("MSCS callbacks are registered\n");
		return -1;
	}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	if (ecm_mscs.update_skb_priority) {
		spin_unlock_bh(&ecm_classifier_mscs_lock);
		DEBUG_ERROR("SCS callbacks are registered\n");
		return -1;
	}
#endif
	ecm_mscs.get_peer_priority = mscs_cb->get_peer_priority;
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	ecm_mscs.update_skb_priority = mscs_cb->update_skb_priority;
#endif
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_mscs_callback_register);

/*
 * ecm_interface_ovpn_unregister
 */
void ecm_classifier_mscs_callback_unregister (void)
{
	spin_lock_bh(&ecm_classifier_mscs_lock);
	ecm_mscs.get_peer_priority = NULL;
#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	ecm_mscs.update_skb_priority = NULL;
#endif
	spin_unlock_bh(&ecm_classifier_mscs_lock);
}
EXPORT_SYMBOL(ecm_classifier_mscs_callback_unregister);

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
/*
 * ecm_classifier_mscs_spm_notifier
 *	Registration for SPM rule update events
 */
static struct notifier_block ecm_classifier_mscs_spm_notifier __read_mostly = {
	.notifier_call = ecm_classifier_mscs_spm_notifier_callback,
};
#endif

/*
 * ecm_classifier_mscs_init()
 */
int ecm_classifier_mscs_init(struct dentry *dentry)
{
	DEBUG_INFO("mscs classifier Module init\n");

	ecm_classifier_mscs_dentry = debugfs_create_dir("ecm_classifier_mscs", dentry);
	if (!ecm_classifier_mscs_dentry) {
		DEBUG_ERROR("Failed to create ecm mscs directory in debugfs\n");
		return -1;
	}

	if (!debugfs_create_file("enabled", S_IRUGO | S_IWUSR, ecm_classifier_mscs_dentry,
				NULL, &ecm_classifier_mscs_enabled_fops)) {
		DEBUG_ERROR("Failed to create ecm mscs classifier enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_mscs_dentry);
		return -1;
	}

	if (!debugfs_create_file("multi_ap_enabled", S_IRUGO | S_IWUSR, ecm_classifier_mscs_dentry,
			NULL, &ecm_classifier_mscs_scs_multi_ap_enabled_fops)) {
		DEBUG_ERROR("Failed to create multi ap enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_mscs_dentry);
		return -1;
	}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	if (!debugfs_create_file("scs_enabled", S_IRUGO | S_IWUSR, ecm_classifier_mscs_dentry,
				NULL, &ecm_classifier_scs_enabled_fops)) {
		DEBUG_ERROR("Failed to create ecm scs classifier enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_mscs_dentry);
		return -1;
	}

	if (!debugfs_create_file("udp_ipsec_port", S_IRUGO | S_IWUSR, ecm_classifier_mscs_dentry,
				NULL, &ecm_classifier_scs_udp_ipsec_port_fops)) {
		DEBUG_ERROR("Failed to create ecm scs udp ipsec port file in debugfs for adding port number\n");
		debugfs_remove_recursive(ecm_classifier_mscs_dentry);
		return -1;
	}

	/*
	 * Register for service prioritization notification update.
	 */
	sp_mapdb_notifier_register(&ecm_classifier_mscs_spm_notifier);
#endif
	return 0;
}
EXPORT_SYMBOL(ecm_classifier_mscs_init);

/*
 * ecm_classifier_mscs_exit()
 */
void ecm_classifier_mscs_exit(void)
{
	DEBUG_INFO("mscs classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_mscs_lock);
	ecm_classifier_mscs_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_mscs_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_mscs_dentry) {
		debugfs_remove_recursive(ecm_classifier_mscs_dentry);
	}

#ifdef ECM_CLASSIFIER_MSCS_SCS_ENABLE
	/*
	 * Unregister service prioritization notification update.
	 */
	sp_mapdb_notifier_unregister(&ecm_classifier_mscs_spm_notifier);
#endif
}
EXPORT_SYMBOL(ecm_classifier_mscs_exit);
