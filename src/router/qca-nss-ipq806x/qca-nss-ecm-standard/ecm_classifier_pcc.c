/*
 **************************************************************************
 * Copyright (c) 2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * Parental Controls Classifier.
 * While not implementing parental controls feature itself.
 * This module provides an interface for customer parental controls systems to interract with the ECM.
 * This ensures that acceleration will not interfere with parental controls logics, especially DPI.
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <linux/ctype.h>
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

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_PCC_DEBUG_LEVEL

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
#include "ecm_front_end_common.h"
#include "ecm_classifier_pcc.h"
#include "ecm_classifier_pcc_public.h"
#include "ecm_front_end_common.h"
#include "ecm_interface.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_PCC_INSTANCE_MAGIC 0x2351

/*
 * struct ecm_classifier_pcc_instance
 * 	State per connection for PCC classifier
 */
struct ecm_classifier_pcc_instance {
	struct ecm_classifier_instance base;			/* Base type */

	ecm_classifier_pcc_result_t accel_permit_state;		/* Permission state for acceleration */
	uint32_t ci_serial;					/* RO: Serial of the connection */
	long process_jiffies_last;				/* Rate limiting the calls to the registrant */
	uint32_t reg_calls_to;					/* #calls to registrant */
	uint32_t reg_calls_from;				/* #calls from registrant */
	uint32_t rate_exceeds;					/* call rate exceeds */
	uint32_t feature_flags;					/* Feature flags */

	struct ecm_classifier_process_response process_response;
								/* Last process response computed */
	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

static DEFINE_SPINLOCK(ecm_classifier_pcc_lock);		/* Concurrency control SMP access */
static int ecm_classifier_pcc_count = 0;			/* Tracks number of instances allocated */
static struct ecm_classifier_pcc_registrant *ecm_classifier_registrant = NULL;
								/* Singleton Parent Controls code */

/*
 * Operational control
 */
static bool ecm_classifier_pcc_enabled = false;			/* Enable / disable state of the classifier function */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_pcc_dentry;

/*
 * ecm_classifier_pcc_register()
 *	Register a new PCC module.
 *
 */
int ecm_classifier_pcc_register(struct ecm_classifier_pcc_registrant *r)
{
	/*
	 * Hold the module of the registrant
	 */
	if (!try_module_get(r->this_module)) {
		return -ESHUTDOWN;
	}

	/*
	 * Hold the registrant we have been given for our purposes.
	 */
	r->ref(r);

	spin_lock_bh(&ecm_classifier_pcc_lock);
	if (ecm_classifier_registrant) {
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		DEBUG_WARN("Registrant already\n");
		module_put(r->this_module);
		r->deref(r);
		return -EALREADY;
	}
	ecm_classifier_registrant = r;
	ecm_classifier_pcc_enabled = true;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Destroy all the connections
	 */
	ecm_db_connection_defunct_all();
	return 0;
}
EXPORT_SYMBOL(ecm_classifier_pcc_register);

/*
 * ecm_classifier_pcc_unregister_begin()
 *	Begin unregistration process
 */
void ecm_classifier_pcc_unregister_begin(struct ecm_classifier_pcc_registrant *r)
{
	struct ecm_classifier_pcc_registrant *reg;

	spin_lock_bh(&ecm_classifier_pcc_lock);
	reg = ecm_classifier_registrant;
	if (!reg) {
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		DEBUG_WARN("No Registrant\n");
		return;
	}
	if (reg != r) {
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		DEBUG_WARN("Unexpected registrant, given: %px, expecting: %px\n", r, reg);
		return;
	}

	ecm_classifier_registrant = NULL;
	ecm_classifier_pcc_enabled = false;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Release our ref upon the registrant that we took when it was registered
	 */
	reg->deref(reg);
	module_put(reg->this_module);

	/*
	 * Destroy all the connections
	 */
	ecm_db_connection_defunct_all();
}
EXPORT_SYMBOL(ecm_classifier_pcc_unregister_begin);

/*
 * ecm_classifier_pcc_decel_by_dev()
 *	Decelerate connection based on a dev.
 */
void ecm_classifier_pcc_decel_by_dev(struct net_device *dev)
{
	if (!dev)
		return;

	ecm_interface_dev_defunct_connections(dev);
}
EXPORT_SYMBOL(ecm_classifier_pcc_decel_by_dev);

/*
 * ecm_classifier_pcc_decel_v4()
 *	Decelerate connection.
 *
 * Big endian parameters apart from protocol
 */
bool ecm_classifier_pcc_decel_v4(uint8_t *src_mac, __be32 src_ip, int src_port,
		uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol)
{
	/*
	 * MAC addresses are not used to decelerate the connection, but in the future
	 * MAC based deceleration can be added to this function.
	 */
	return ecm_db_connection_decel_v4(src_ip, src_port, dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_classifier_pcc_decel_v4);

/*
 * ecm_classifier_pcc_decel_v6()
 *	Decelerate connection.
 *
 * Big endian parameters apart from protocol
 */
bool ecm_classifier_pcc_decel_v6(uint8_t *src_mac, struct in6_addr *src_ip,
		 int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip,
		 int dest_port, int protocol)
{
	/*
	 * MAC addresses are not used to decelerate the connection, but in the future
	 * MAC based deceleration can be added to this function.
	 */
	return ecm_db_connection_decel_v6(src_ip, src_port, dest_ip, dest_port, protocol);
}
EXPORT_SYMBOL(ecm_classifier_pcc_decel_v6);

/*
 * ecm_classifier_pcc_permit_accel_v4()
 *	Permit acceleration.
 *
 * Big endian parameters apart from protocol
 */
void ecm_classifier_pcc_permit_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol)
{
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_pcc_instance *pcci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_src_ip, src_ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_dest_ip, dest_ip);

	DEBUG_INFO("Permit Accel v4, lookup connection using \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_DOT_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_DOT_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_DOT(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_DOT(ecm_dest_ip), dest_port);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_TRACE("Not found\n");
		return;
	}

	/*
	 * Get the PCC classifier
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_PCC);
	if (!classi) {
		DEBUG_TRACE("No PCC classi\n");
		ecm_db_connection_deref(ci);
		return;
	}
	pcci = (struct ecm_classifier_pcc_instance *)classi;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	/*
	 * Set the permitted accel state to PERMITTED
	 * NOTE: When we next see activity on this connection it shall be accelerated (save depending on other classifiers decisions too).
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_PERMITTED;
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	pcci->reg_calls_from++;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	classi->deref(classi);
	ecm_db_connection_deref(ci);
}
EXPORT_SYMBOL(ecm_classifier_pcc_permit_accel_v4);

/*
 * ecm_classifier_pcc_permit_accel_v6()
 *	Permit acceleration
 *
 * Big endian parameters apart from protocol.
 *
 * NOTE: If IPv6 is not supported in ECM this function must still exist as a stub to avoid compilation problems for registrants.
 */
void ecm_classifier_pcc_permit_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol)
{
#ifdef ECM_IPV6_ENABLE
	struct in6_addr in6;
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_pcc_instance *pcci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	in6 = *src_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_src_ip, in6);
	in6 = *dest_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_dest_ip, in6);

	DEBUG_INFO("Permit Accel v6, lookup connection using \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_OCTAL(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_OCTAL(ecm_dest_ip), dest_port);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_TRACE("Not found\n");
		return;
	}

	/*
	 * Get the PCC classifier
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_PCC);
	if (!classi) {
		DEBUG_TRACE("No PCC classi\n");
		ecm_db_connection_deref(ci);
		return;
	}
	pcci = (struct ecm_classifier_pcc_instance *)classi;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	/*
	 * Set the permitted accel state to PERMITTED
	 * NOTE: When we next see activity on this connection it shall be accelerated (save depending on other classifiers decisions too).
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_PERMITTED;
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	pcci->reg_calls_from++;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	classi->deref(classi);
	ecm_db_connection_deref(ci);
#endif
}
EXPORT_SYMBOL(ecm_classifier_pcc_permit_accel_v6);

/*
 * ecm_classifier_pcc_deny_accel_v4()
 *	Deny acceleration
 */
void ecm_classifier_pcc_deny_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol)
{
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_pcc_instance *pcci;
	struct ecm_front_end_connection_instance *feci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_src_ip, src_ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_dest_ip, dest_ip);

	DEBUG_INFO("Deny Accel v4, lookup connection using \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_DOT_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_DOT_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_DOT(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_DOT(ecm_dest_ip), dest_port);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_TRACE("Not found\n");
		return;
	}

	/*
	 * Get the PCC classifier
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_PCC);
	if (!classi) {
		DEBUG_TRACE("No PCC classi\n");
		ecm_db_connection_deref(ci);
		return;
	}
	pcci = (struct ecm_classifier_pcc_instance *)classi;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	/*
	 * Set the permitted accel state to DENIED
	 * NOTE: When we next see activity on this connection it shall be accelerated (save depending on other classifiers decisions too).
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_DENIED;
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	pcci->reg_calls_from++;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Get the front end and issue a deceleration
	 * If the connection is not accelerated anyway this will have no effect
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	feci->decelerate(feci);
	ecm_front_end_connection_deref(feci);

	classi->deref(classi);
	ecm_db_connection_deref(ci);
}
EXPORT_SYMBOL(ecm_classifier_pcc_deny_accel_v4);

/*
 * ecm_classifier_pcc_deny_accel_v6()
 *	Deny acceleration
 *
 * NOTE: If IPv6 is not supported in ECM this function must still exist as a stub to avoid compilation problems for registrants.
 */
void ecm_classifier_pcc_deny_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol)
{
#ifdef ECM_IPV6_ENABLE
	struct in6_addr in6;
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_pcc_instance *pcci;
	struct ecm_front_end_connection_instance *feci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	in6 = *src_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_src_ip, in6);
	in6 = *dest_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_dest_ip, in6);

	DEBUG_INFO("Deny Accel v6, lookup connection using \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_OCTAL(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_OCTAL(ecm_dest_ip), dest_port);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_TRACE("Not found\n");
		return;
	}

	/*
	 * Get the PCC classifier
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_PCC);
	if (!classi) {
		DEBUG_TRACE("No PCC classi\n");
		ecm_db_connection_deref(ci);
		return;
	}
	pcci = (struct ecm_classifier_pcc_instance *)classi;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	/*
	 * Set the permitted accel state to DENIED
	 * NOTE: When we next see activity on this connection it shall be accelerated (save depending on other classifiers decisions too).
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_DENIED;
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	pcci->reg_calls_from++;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Get the front end and issue a deceleration
	 * If the connection is not accelerated anyway this will have no effect
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	feci->decelerate(feci);
	ecm_front_end_connection_deref(feci);

	classi->deref(classi);
	ecm_db_connection_deref(ci);
#endif
}
EXPORT_SYMBOL(ecm_classifier_pcc_deny_accel_v6);

/*
 * ecm_classifier_pcc_unregister_force()
 *	Unregister the registrant, if any
 */
static void ecm_classifier_pcc_unregister_force(struct ecm_classifier_pcc_instance *pcci)
{
	struct ecm_classifier_pcc_registrant *reg;

	spin_lock_bh(&ecm_classifier_pcc_lock);
	reg = ecm_classifier_registrant;
	if (!reg) {
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		return;
	}
	ecm_classifier_registrant = NULL;
	ecm_classifier_pcc_enabled = false;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Release our ref upon the registrant that we took when it was registered
	 */
	DEBUG_INFO("Force unregistration of: %px\n", reg);
	reg->deref(reg);

	/*
	 * Release hold on registrant module
	 */
	module_put(reg->this_module);

	/*
	 * Destroy all the connections
	 */
	ecm_db_connection_defunct_all();
}

/*
 * _ecm_classifier_pcc_ref()
 *	Ref
 */
static void _ecm_classifier_pcc_ref(struct ecm_classifier_pcc_instance *pcci)
{
	pcci->refs++;
	DEBUG_TRACE("%px: pcci ref %d\n", pcci, pcci->refs);
	DEBUG_ASSERT(pcci->refs > 0, "%px: ref wrap\n", pcci);
}

/*
 * ecm_classifier_pcc_ref()
 *	Ref
 */
static void ecm_classifier_pcc_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)ci;

	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
	spin_lock_bh(&ecm_classifier_pcc_lock);
	_ecm_classifier_pcc_ref(pcci);
	spin_unlock_bh(&ecm_classifier_pcc_lock);
}

/*
 * ecm_classifier_pcc_deref()
 *	Deref
 */
static int ecm_classifier_pcc_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)ci;

	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->refs--;
	DEBUG_ASSERT(pcci->refs >= 0, "%px: refs wrapped\n", pcci);
	DEBUG_TRACE("%px: Parental Controls classifier deref %d\n", pcci, pcci->refs);
	if (pcci->refs) {
		int refs = pcci->refs;
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_pcc_count--;
	DEBUG_ASSERT(ecm_classifier_pcc_count >= 0, "%px: ecm_classifier_pcc_count wrap\n", pcci);

	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final Parental Controls classifier instance\n", pcci);
	kfree(pcci);

	return 0;
}

/*
 * ecm_classifier_pcc_get_policing_info()
 *	Get policing related information.
 */
static void ecm_classifier_pcc_get_policing_info(struct ecm_classifier_pcc_info cinfo,
		 int *flow_index, int *return_index)
{
	*flow_index = cinfo.output_params.ap_info.flow_ap_index;
	*return_index = cinfo.output_params.ap_info.return_ap_index;
}

/*
 * ecm_classifier_pcc_get_mirror_info()
 *	Get mirroring related information.
 */
static int ecm_classifier_pcc_get_mirror_info(struct ecm_classifier_pcc_info cinfo,
		 int *flow_mirror_ifindex_ptr, int *return_mirror_ifindex_ptr)
{
	struct net_device *flow_dev = cinfo.output_params.mirror.tuple_mirror_dev;
	struct net_device *return_dev = cinfo.output_params.mirror.tuple_ret_mirror_dev;

	if (!flow_dev && !return_dev) {
		DEBUG_ERROR("No mirror net devices are specified\n");
		return -1;
	}

	/*
	 * Fetch mirror interface information.
	 */
	if (flow_dev) {
		dev_hold(flow_dev);
		*flow_mirror_ifindex_ptr = flow_dev->ifindex;
		dev_put(flow_dev);
	}

	if (return_dev) {
		dev_hold(return_dev);
		*return_mirror_ifindex_ptr = return_dev->ifindex;
		dev_put(return_dev);
	}

	return 0;
}

/*
 * ecm_classifier_pcc_process()
 *	Process new packet
 *
 * NOTE: This function would only ever be called if all other classifiers have failed.
 */
static void ecm_classifier_pcc_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
									struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
									struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_pcc_instance *pcci = (struct ecm_classifier_pcc_instance *)aci;
	ecm_classifier_pcc_result_t accel_permit_state;
	ecm_classifier_pcc_result_t reg_result;
	struct ecm_db_connection_instance *ci;
	long jiffies_now;
	int ip_version;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dest_mac[ETH_ALEN];
	int protocol;
	int src_port;
	int dst_port;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	struct ecm_classifier_pcc_registrant *registrant;
	struct ecm_classifier_pcc_info cinfo = {0};
	int flow_mirror_ifindex = -1;
	int return_mirror_ifindex = -1;
	int flow_acl_index = 0;
	int flow_policer_index = 0;
	int return_acl_index = 0;
	int return_policer_index = 0;
	struct net_device *in_dev = NULL;
	struct net_device *out_dev = NULL;

	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: invalid state magic\n", pcci);

	/*
	 * Get connection
	 */
	ci = ecm_db_connection_serial_find_and_ref(pcci->ci_serial);
	if (!ci) {
		/*
		 * Connection has gone from under us
		 */
		DEBUG_TRACE("%px: pcc no connection\n", pcci);
		spin_lock_bh(&ecm_classifier_pcc_lock);
		goto not_relevant;
	}

	/*
	 * Early detection of DNS server port
	 */
	dst_port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

	spin_lock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Not relevant to the connection if not enabled.
	 */
	if (unlikely(!ecm_classifier_pcc_enabled)) {
		/*
		 * Not relevant.
		 */
		DEBUG_TRACE("%px: pcc not relevant\n", pcci);
		goto not_relevant;
	}

	/*
	 * What is our acceleration permit state?
	 * If it is something other than ECM_CLASSIFIER_PCC_RESULT_NOT_YET then we have a definitive result already.
	 */
	accel_permit_state = pcci->accel_permit_state;
	if (accel_permit_state != ECM_CLASSIFIER_PCC_RESULT_NOT_YET) {
		*process_response = pcci->process_response;
		spin_unlock_bh(&ecm_classifier_pcc_lock);
		ecm_db_connection_deref(ci);
		DEBUG_TRACE("%px: process: no_permit_state: %d\n", pcci, accel_permit_state);
		return;
	}

	/*
	 * If the destination port is to DNS server then we implicitly deny acceleration
	 */
	if (dst_port == 53) {
		/*
		 * By setting the permit state to DENIED we will always deny from this point on
		 */
		pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_DENIED;
		goto deny_accel;
	}

	/*
	 * We need to call to the registrant BUT we cannot do this at a rate that exceeds 1/sec
	 * NOTE: Not worried about wrap around, it's only one second.
	 * As an exception, we try with the first packet.
	 */
	jiffies_now = jiffies;
	if (((jiffies_now - pcci->process_jiffies_last) < HZ) && (ecm_front_end_get_slow_packet_count(ci->feci) > 1)) {
		/*
		 * We cannot permit acceleration just yet
		 * Deny accel but don't change the permit state - we try again later
		 */
		pcci->rate_exceeds++;
		goto deny_accel;
	}
	pcci->process_jiffies_last = jiffies_now;

	/*
	 * We have to call out to our registrant to see if we can get permission to accelerate.
	 * Get our registrant
	 */
	registrant = ecm_classifier_registrant;
	registrant->ref(registrant);

	/*
	 * Bump reg calls made to the registrant.
	 */
	pcci->reg_calls_to++;

	spin_unlock_bh(&ecm_classifier_pcc_lock);

	/*
	 * See if we can hold the registrant module - it may be unloading.
	 */
	if (!try_module_get(registrant->this_module)) {
		/*
		 * Module is unloading.
		 */
		registrant->deref(registrant);

		/*
		 * Force unregistration
		 */
		ecm_classifier_pcc_unregister_force(pcci);

		/*
		 * We are implicitly "not relevant".
		 */
		spin_lock_bh(&ecm_classifier_pcc_lock);
		goto not_relevant;
	}

	/*
	 * Ask the registrant if we may accelerate (big endian)
	 */
	ip_version = ecm_db_connection_ip_version_get(ci);
	protocol = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	src_port = htons(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
	dst_port = htons(dst_port);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_mac);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dest_mac);

	/*
	 * Default is permitted in case ip_version is unsupported here
	 */
	reg_result = ECM_CLASSIFIER_PCC_RESULT_PERMITTED;
	if (ip_version == 4) {
		__be32 src_ip4;
		__be32 dest_ip4;

		ECM_IP_ADDR_TO_NIN4_ADDR(src_ip4, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(dest_ip4, dst_ip);

		/*
		 * get_accel_info_v4 callback has higher priority over
		 * okay_to_accel_v4 callback.
		 * get_accel_info_v4 callback is the advance version of older
		 * okay_to_accel_v4 callback, from which the registrant can not
		 * only can tell the final acceleration decision about the flow but
		 * can also request for additional features like mirroring.
		 * get_accel_info_v4 callback is also backward compatible, means
		 * it can be used by the registrant for only specifying acceleration
		 * decisions.
		 */
		if (registrant->get_accel_info_v4){
			ecm_db_netdevs_get_and_hold(ci, sender, &in_dev, &out_dev);
			cinfo.input_params.dev_info.in_dev = in_dev;
			cinfo.input_params.dev_info.out_dev = out_dev;

			reg_result = registrant->get_accel_info_v4(registrant,
					 src_mac, src_ip4, src_port, dest_mac,
					 dest_ip4, dst_port, protocol, &cinfo);

			pcci->feature_flags = cinfo.feature_flags;
			dev_put(in_dev);
			dev_put(out_dev);
		} else {
			reg_result = registrant->okay_to_accel_v4(registrant,
					 src_mac, src_ip4, src_port, dest_mac,
					 dest_ip4, dst_port, protocol);
			pcci->feature_flags = ECM_CLASSIFIER_PCC_FEATURE_NONE;
		}
	}
#ifdef ECM_IPV6_ENABLE
	if (ip_version == 6) {
		struct in6_addr src_ip6;
		struct in6_addr dest_ip6;
		ECM_IP_ADDR_TO_NIN6_ADDR(src_ip6, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(dest_ip6, dst_ip);

		/*
		 * get_accel_info_v6 callback has higher priority over
		 * okay_to_accel_v6 callback.
		 * get_accel_info_v6 callback is the advance version of older
		 * okay_to_accel_v6 callback, from which the registrant can not
		 * only can tell the final acceleration decision about the flow but
		 * can also request for additional features like mirroring.
		 * get_accel_info_v6 callback is also backward compatible, means
		 * it can be used by the registrant for only specifying acceleration
		 * decisions.
		 */
		if (registrant->get_accel_info_v6){
			ecm_db_netdevs_get_and_hold(ci, sender, &in_dev, &out_dev);
			cinfo.input_params.dev_info.in_dev = in_dev;
			cinfo.input_params.dev_info.out_dev = out_dev;
			reg_result = registrant->get_accel_info_v6(registrant,
					 src_mac, &src_ip6, src_port, dest_mac,
					 &dest_ip6, dst_port, protocol, &cinfo);

			pcci->feature_flags = cinfo.feature_flags;
			dev_put(in_dev);
			dev_put(out_dev);
		} else {
			reg_result = registrant->okay_to_accel_v6(registrant,
					 src_mac, &src_ip6, src_port, dest_mac,
					 &dest_ip6, dst_port, protocol);
			pcci->feature_flags = ECM_CLASSIFIER_PCC_FEATURE_NONE;
		}
	}
#endif

	/*
	 * Release the ref taken for this call
	 */
	registrant->deref(registrant);

	/*
	 * Release the module ref taken.
	 */
	module_put(registrant->this_module);

	/*
	 * Handle the features requested by registrants, if any.
	 */
	if (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
		if (ecm_classifier_pcc_get_mirror_info(cinfo, &flow_mirror_ifindex,
					&return_mirror_ifindex) < 0) {
			spin_lock_bh(&ecm_classifier_pcc_lock);
			goto deny_accel;
		}
	}

	if ((cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL_EGRESS_DEV)) {
		ecm_classifier_pcc_get_policing_info(cinfo, &flow_acl_index, &return_acl_index);
	} else if (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER) {
		ecm_classifier_pcc_get_policing_info(cinfo, &flow_policer_index, &return_policer_index);
	}

	/*
	 * Handle the result
	 */
	switch (reg_result) {
	case ECM_CLASSIFIER_PCC_RESULT_NOT_YET:
		/*
		 * Deny accel but don't change the permit state - we try again later
		 */
		spin_lock_bh(&ecm_classifier_pcc_lock);
		goto deny_accel;
	case ECM_CLASSIFIER_PCC_RESULT_DENIED:
		/*
		 * Deny accel and set the permit state to denied - this connection is denied from this point on.
		 */
		spin_lock_bh(&ecm_classifier_pcc_lock);
		pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_DENIED;
		goto deny_accel;
	case ECM_CLASSIFIER_PCC_RESULT_PERMITTED:
		break;
	default:
		DEBUG_ASSERT(false, "Unhandled result: %d\n", reg_result);
	}

	/*
	 * Acceleration is permitted
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);

	/*
	 * Fill mirror information in the process response.
	 */
	if (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
		pcci->process_response.flow_mirror_ifindex = flow_mirror_ifindex;
		pcci->process_response.return_mirror_ifindex = return_mirror_ifindex;
		pcci->process_response.process_actions |=
			 ECM_CLASSIFIER_PROCESS_ACTION_MIRROR_ENABLED;
	}

	if ((cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL_EGRESS_DEV)) {
		pcci->process_response.rule_id.acl.flow_acl_id = flow_acl_index;
		pcci->process_response.rule_id.acl.return_acl_id = return_acl_index;
		pcci->process_response.process_actions |=  ECM_CLASSIFIER_PROCESS_ACTION_ACL_ENABLED;
	} else if (cinfo.feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER) {
		pcci->process_response.rule_id.policer.flow_policer_id = flow_policer_index;
		pcci->process_response.rule_id.policer.return_policer_id = return_policer_index;
		pcci->process_response.process_actions |=  ECM_CLASSIFIER_PROCESS_ACTION_POLICER_ENABLED;
	}

	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_PERMITTED;
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	*process_response = pcci->process_response;

	spin_unlock_bh(&ecm_classifier_pcc_lock);
	ecm_db_connection_deref(ci);

	return;

not_relevant:

	/*
	 * ecm_classifier_pcc_lock MUST be held
	 */
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
	pcci->process_response.process_actions = 0;
	*process_response = pcci->process_response;
	spin_unlock_bh(&ecm_classifier_pcc_lock);
	if (ci) {
		ecm_db_connection_deref(ci);
	}
	return;

deny_accel:

	/*
	 * ecm_classifier_pcc_lock MUST be held
	 */
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	pcci->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	pcci->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	*process_response = pcci->process_response;
	spin_unlock_bh(&ecm_classifier_pcc_lock);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_classifier_pcc_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_pcc_type_get(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)aci;

	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
	return ECM_CLASSIFIER_TYPE_PCC;
}

/*
 * ecm_classifier_pcc_reclassify_allowed()
 *	Get whether reclassification is allowed
 */
static bool ecm_classifier_pcc_reclassify_allowed(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)aci;

	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
	return true;
}

/*
 * ecm_classifier_pcc_reclassify()
 *	Reclassify
 */
static void ecm_classifier_pcc_reclassify(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	/*
	 * Connection needs to be reset to 'as new'
	 * NOTE: Implicitly the connection would have been decelerated now so we don't need to worry about that.
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_NOT_YET;

	/*
	 * Reset jiffies for rate limiting registrant calls
	 */
	pcci->process_jiffies_last = jiffies;

	spin_unlock_bh(&ecm_classifier_pcc_lock);
}

/*
 * ecm_classifier_pcc_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_pcc_last_process_response_get(struct ecm_classifier_instance *aci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_pcc_instance *pcci;
	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	spin_lock_bh(&ecm_classifier_pcc_lock);
	*process_response = pcci->process_response;
	spin_unlock_bh(&ecm_classifier_pcc_lock);
}

/*
 * ecm_classifier_pcc_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_pcc_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_pcc_instance *pcci __attribute__((unused));

	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
}

/*
 * ecm_classifier_pcc_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_pcc_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_pcc_instance *pcci __attribute__((unused));

	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
}

/*
 * ecm_classifier_pcc_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_pcc_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_pcc_instance *pcci __attribute__((unused));

	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
}

/*
 * ecm_classifier_pcc_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_pcc_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_pcc_instance *pcci __attribute__((unused));

	pcci = (struct ecm_classifier_pcc_instance *)aci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_pcc_state_get()
 *	Return state
 */
static int ecm_classifier_pcc_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_pcc_instance *pcci;
	struct ecm_classifier_process_response process_response;
	ecm_classifier_pcc_result_t accel_permit_state;
	uint32_t reg_calls_to;
	uint32_t reg_calls_from;
	uint32_t rate_exceeds;
	uint32_t feature_flags;

	pcci = (struct ecm_classifier_pcc_instance *)ci;
	DEBUG_CHECK_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC, "%px: magic failed", pcci);

	if ((result = ecm_state_prefix_add(sfi, "pcc"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_pcc_lock);
	accel_permit_state = pcci->accel_permit_state;
	process_response = pcci->process_response;
	reg_calls_to = pcci->reg_calls_to;
	reg_calls_from = pcci->reg_calls_from;
	rate_exceeds = pcci->rate_exceeds;
	feature_flags = pcci->feature_flags;
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	if ((result = ecm_state_write(sfi, "accel_permit_state", "%d", accel_permit_state))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "reg_calls_to", "%d", reg_calls_to))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "reg_calls_from", "%d", reg_calls_from))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "rate_exceeds", "%d", rate_exceeds))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "feature_flags", "0x%x", feature_flags))) {
		return result;
	}

	if (process_response.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_MIRROR_ENABLED) {
		struct net_device *dev;

		if ((dev = dev_get_by_index(&init_net, process_response.flow_mirror_ifindex))) {
			if ((result = ecm_state_write(sfi, "flow_mirror", "%s",
							dev->name))) {
				dev_put(dev);
				return result;
			}
			dev_put(dev);
		}

		if ((dev = dev_get_by_index(&init_net, process_response.return_mirror_ifindex))) {
			if ((result = ecm_state_write(sfi, "return_mirror", "%s",
							dev->name))) {
				dev_put(dev);
				return result;
			}
			dev_put(dev);
		}
	}

	if (process_response.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_ACL_ENABLED) {
		if ((result = ecm_state_write(sfi, "flow_acl_index", "%d", process_response.rule_id.acl.flow_acl_id))) {
			return result;
		}

		if ((result = ecm_state_write(sfi, "return_acl_index", "%d", process_response.rule_id.acl.return_acl_id))) {
			return result;
		}
	}

	if (process_response.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_POLICER_ENABLED) {
		if ((result = ecm_state_write(sfi, "flow_policer_index", "%d", process_response.rule_id.policer.flow_policer_id))) {
			return result;
		}

		if ((result = ecm_state_write(sfi, "return_policer_index", "%d", process_response.rule_id.policer.return_policer_id))) {
			return result;
		}
	}

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
 * ecm_classifier_pcc_instance_alloc()
 *	Allocate an instance of the Parental Controls classifier
 */
struct ecm_classifier_pcc_instance *ecm_classifier_pcc_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_pcc_instance *pcci;
	struct ecm_classifier_instance *cdi;

	/*
	 * Allocate the instance
	 */
	pcci = (struct ecm_classifier_pcc_instance *)kzalloc(sizeof(struct ecm_classifier_pcc_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!pcci) {
		DEBUG_WARN("Failed to allocate Parental Controls Classifier instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(pcci, ECM_CLASSIFIER_PCC_INSTANCE_MAGIC);
	pcci->refs = 1;
	pcci->ci_serial = ecm_db_connection_serial_get(ci);

	/*
	 * We are relevant to the connection at this time
	 */
	pcci->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;

	/*
	 * Don't know yet whether we are allowed to accelerate - need to query the registrant
	 */
	pcci->accel_permit_state = ECM_CLASSIFIER_PCC_RESULT_NOT_YET;

	/*
	 * Reset jiffies for rate limiting registrant calls
	 */
	pcci->process_jiffies_last = jiffies;

	/*
	 * Methods generic to all classifiers.
	 */
	cdi = (struct ecm_classifier_instance *)pcci;
	cdi->process = ecm_classifier_pcc_process;
	cdi->sync_from_v4 = ecm_classifier_pcc_sync_from_v4;
	cdi->sync_to_v4 = ecm_classifier_pcc_sync_to_v4;
	cdi->sync_from_v6 = ecm_classifier_pcc_sync_from_v6;
	cdi->sync_to_v6 = ecm_classifier_pcc_sync_to_v6;
	cdi->type_get = ecm_classifier_pcc_type_get;
	cdi->reclassify_allowed = ecm_classifier_pcc_reclassify_allowed;
	cdi->reclassify = ecm_classifier_pcc_reclassify;
	cdi->last_process_response_get = ecm_classifier_pcc_last_process_response_get;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cdi->state_get = ecm_classifier_pcc_state_get;
#endif
	cdi->ref = ecm_classifier_pcc_ref;
	cdi->deref = ecm_classifier_pcc_deref;

	/*
	 * Increment stats
	 */
	spin_lock_bh(&ecm_classifier_pcc_lock);
	ecm_classifier_pcc_count++;
	DEBUG_ASSERT(ecm_classifier_pcc_count > 0, "%px: ecm_classifier_pcc_count wrap\n", pcci);
	spin_unlock_bh(&ecm_classifier_pcc_lock);

	DEBUG_INFO("Parental Controls classifier instance alloc: %px\n", pcci);
	return pcci;
}
EXPORT_SYMBOL(ecm_classifier_pcc_instance_alloc);

/*
 * ecm_classifier_pcc_init()
 */
int ecm_classifier_pcc_init(struct dentry *dentry)
{
	DEBUG_INFO("Parental Controls classifier Module init\n");

	ecm_classifier_pcc_dentry = debugfs_create_dir("ecm_classifier_pcc", dentry);
	if (!ecm_classifier_pcc_dentry) {
		DEBUG_ERROR("Failed to create ecm pcc directory in debugfs\n");
		return -1;
	}

	if (!ecm_debugfs_create_u32("enabled", S_IRUGO, ecm_classifier_pcc_dentry,
					(u32 *)&ecm_classifier_pcc_enabled)) {
		DEBUG_ERROR("Failed to create pcc enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_pcc_dentry);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_pcc_init);

/*
 * ecm_classifier_pcc_exit()
 */
void ecm_classifier_pcc_exit(void)
{
	DEBUG_INFO("Parental Controls classifier Module exit\n");

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_pcc_dentry) {
		debugfs_remove_recursive(ecm_classifier_pcc_dentry);
	}

}
EXPORT_SYMBOL(ecm_classifier_pcc_exit);
