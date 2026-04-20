/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2020-2021, The Linux Foundation. All rights reserved.
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

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <linux/netfilter/nf_conntrack_zones_common.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <net/genetlink.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_NL_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_classifier_nl.h"
#include "ecm_db.h"
#include "ecm_front_end_ipv4.h"
#ifdef ECM_IPV6_ENABLE
#include "ecm_front_end_ipv6.h"
#endif

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_NL_INSTANCE_MAGIC 0xFE12

#define ECM_CLASSIFIER_NL_F_ACCEL	(1 << 0) /* acceleration requested */
#define ECM_CLASSIFIER_NL_F_ACCEL_OK	(1 << 1) /* acceleration confirmed */
#define ECM_CLASSIFIER_NL_F_CLOSED	(1 << 2) /* close event issued */

/*
 * struct ecm_classifier_nl_instance
 * 	State to allow tracking of dynamic qos for a connection
 */
struct ecm_classifier_nl_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_nl_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_nl_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	struct ecm_classifier_process_response process_response;/* Last process response computed */
	int refs;						/* Integer to trap we never go negative */
	unsigned int flags;					/* See ECM_CLASSIFIER_NL_F_* */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Operational control
 */
static bool ecm_classifier_nl_enabled = false;		/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_nl_terminate_pending = false;		/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_nl_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_nl_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_nl_instance *ecm_classifier_nl_instances = NULL;
									/* list of all active instances */
static int ecm_classifier_nl_count = 0;					/* Tracks number of instances allocated */

/*
 * Listener for db events
 */
struct ecm_db_listener_instance *ecm_classifier_nl_li = NULL;

/*
 * Generic Netlink family and multicast group names
 */
static struct genl_multicast_group ecm_cl_nl_genl_mcgrp[] = {
	{
		.name = ECM_CL_NL_GENL_MCGRP,
	},
};

static struct genl_family ecm_cl_nl_genl_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = ECM_CL_NL_GENL_NAME,
	.version = ECM_CL_NL_GENL_VERSION,
	.maxattr = ECM_CL_NL_GENL_ATTR_MAX,
};

/*
 * helper for sending basic genl commands requiring only a tuple attribute
 *
 * TODO: implement a message queue serviced by a thread to allow automatic
 *	 retries for accel_ok and closed messages.
 */
static int
ecm_classifier_nl_send_genl_msg(enum ECM_CL_NL_GENL_CMD cmd,
				struct ecm_cl_nl_genl_attr_tuple *tuple)
{
	int ret;
	int buf_len;
	int total_len;
	void *msg_head;
	struct sk_buff *skb;

	/*
	 * Calculate our packet payload size.
	 * Start with our family header.
	 */
	buf_len = ecm_cl_nl_genl_family.hdrsize;

	/*
	 * Add the nla_total_size of each attribute we're going to nla_put().
	 */
	buf_len += nla_total_size(sizeof(*tuple));

	/*
	 * Lastly we need to add space for the NL message header since
	 * genlmsg_new only accounts for the GENL header and not the
	 * outer NL header. To do this, we use a NL helper function which
	 * calculates the total size of a netlink message given a payload size.
	 * Note this value does not include the GENL header, but that's
	 * added automatically by genlmsg_new.
	 */
	total_len = nlmsg_total_size(buf_len);
	skb = genlmsg_new(total_len, GFP_ATOMIC);
	if (!skb) {
		DEBUG_WARN("failed to alloc nlmsg\n");
		return -ENOMEM;
	}

	msg_head = genlmsg_put(skb,
			       0, /* netlink PID */
			       0, /* sequence number */
			       &ecm_cl_nl_genl_family,
			       0, /* flags */
			       cmd);
	if (!msg_head) {
		DEBUG_WARN("failed to add genl headers\n");
		nlmsg_free(skb);
		return -ENOMEM;
	}

	ret = nla_put(skb, ECM_CL_NL_GENL_ATTR_TUPLE, sizeof(*tuple), tuple);
	if (ret != 0) {
		DEBUG_WARN("failed to put tuple into genl msg: %d\n", ret);
		nlmsg_free(skb);
		return ret;
	}

	ret = genlmsg_end(skb, msg_head);
	if (ret < 0) {
		DEBUG_WARN("failed to finalize genl msg: %d\n", ret);
		nlmsg_free(skb);
		return ret;
	}

	/* genlmsg_multicast frees the skb in both success and error cases */
	ret = genlmsg_multicast(&ecm_cl_nl_genl_family,
				skb,
				0,
				0,
				GFP_ATOMIC);
	if (ret != 0) {
		DEBUG_WARN("genl multicast failed: %d\n", ret);
		return ret;
	}

	return 0;
}

/*
 * ecm_cl_nl_genl_attr_tuple_encode()
 *	Helper function to convert connection IP info into a genl_attr_tuple
 */
static int ecm_cl_nl_genl_attr_tuple_encode(struct ecm_cl_nl_genl_attr_tuple *tuple,
				 int ip_version,
				 int proto,
				 ip_addr_t src_ip,
				 int src_port,
				 ip_addr_t dst_ip,
				 int dst_port)
{
	memset(tuple, 0, sizeof(*tuple));
	tuple->proto = (uint8_t)proto;
	tuple->src_port = htons((uint16_t)src_port);
	tuple->dst_port = htons((uint16_t)dst_port);
	if (ip_version == 4) {
		tuple->af = AF_INET;
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple->src_ip.in.s_addr, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple->dst_ip.in.s_addr, dst_ip);
		return 0;
	}
#ifdef ECM_IPV6_ENABLE
	if (ip_version == 6) {
		tuple->af = AF_INET6;
		ECM_IP_ADDR_TO_NIN6_ADDR(tuple->src_ip.in6, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(tuple->dst_ip.in6, dst_ip);
		return 0;
	}
#endif
	return -EAFNOSUPPORT;
}

/*
 * ecm_cl_nl_genl_attr_tuple_decode()
 *	Helper function to convert a genl_attr_tuple into connection IP info
 */
static int ecm_cl_nl_genl_attr_tuple_decode(struct ecm_cl_nl_genl_attr_tuple *tuple,
				 int *proto,
				 ip_addr_t src_ip,
				 int *src_port,
				 ip_addr_t dst_ip,
				 int *dst_port)
{
	*proto = tuple->proto;
	*src_port = ntohs(tuple->src_port);
	*dst_port = ntohs(tuple->dst_port);
	if (AF_INET == tuple->af) {
		ECM_NIN4_ADDR_TO_IP_ADDR(src_ip, tuple->src_ip.in.s_addr);
		ECM_NIN4_ADDR_TO_IP_ADDR(dst_ip, tuple->dst_ip.in.s_addr);
		return 0;
	}
#ifdef ECM_IPV6_ENABLE
	if (AF_INET6 == tuple->af) {
		ECM_NIN6_ADDR_TO_IP_ADDR(src_ip, tuple->src_ip.in6);
		ECM_NIN6_ADDR_TO_IP_ADDR(dst_ip, tuple->dst_ip.in6);
		return 0;
	}
#endif
	return -EAFNOSUPPORT;
}

/*
 * ecm_classifier_nl_genl_msg_ACCEL_OK()
 *	Indicates that Accelleration is okay to the netlink channel
 */
static void ecm_classifier_nl_genl_msg_ACCEL_OK(struct ecm_classifier_nl_instance *cnli)
{
	struct ecm_db_connection_instance *ci;
	int ret;
	int ip_version;
	int proto;
	int src_port;
	int dst_port;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	struct ecm_cl_nl_genl_attr_tuple tuple;

	/*
	 * Lookup the associated connection
	 */
	ci = ecm_db_connection_serial_find_and_ref(cnli->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cnli, cnli->ci_serial);
		return;
	}

	spin_lock_bh(&ecm_classifier_nl_lock);

	/* if we've already issued an ACCEL_OK on this connection,
	   do not send it again */
	if (cnli->flags & ECM_CLASSIFIER_NL_F_ACCEL_OK) {
		spin_unlock_bh(&ecm_classifier_nl_lock);
		ecm_db_connection_deref(ci);
		return;
	}

	spin_unlock_bh(&ecm_classifier_nl_lock);

	proto = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	src_port = (uint16_t)ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	dst_port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

	ip_version = ecm_db_connection_ip_version_get(ci);
	ecm_db_connection_deref(ci);

	ret = ecm_cl_nl_genl_attr_tuple_encode(&tuple,
						ip_version,
						proto,
						src_ip,
						src_port,
						dst_ip,
						dst_port);
	if (ret != 0) {
		DEBUG_WARN("failed to encode genl_attr_tuple: %d\n", ret);
		return;
	}

	ret = ecm_classifier_nl_send_genl_msg(ECM_CL_NL_GENL_CMD_ACCEL_OK,
					      &tuple);
	if (ret != 0) {
		DEBUG_WARN("failed to send ACCEL_OK: %px, serial %u\n",
			   cnli, cnli->ci_serial);
		return;
	}

	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->flags |= ECM_CLASSIFIER_NL_F_ACCEL_OK;
	spin_unlock_bh(&ecm_classifier_nl_lock);
}

/*
 * ecm_classifier_nl_genl_msg_closed()
 *	Invoke this when the connection has been closed and it has been accelerated previously.
 *
 * GGG TODO The purpose of this is not clear, esp. wrt. "accel ok" message.
 * DO NOT CALL THIS UNLESS ECM_CLASSIFIER_NL_F_ACCEL_OK has been set.
 */
static void ecm_classifier_nl_genl_msg_closed(struct ecm_db_connection_instance *ci, struct ecm_classifier_nl_instance *cnli,
					int proto, ip_addr_t src_ip, ip_addr_t dst_ip, int src_port, int dst_port)
{
	int ip_version;
	int ret;
	struct ecm_cl_nl_genl_attr_tuple tuple;

	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->flags |= ECM_CLASSIFIER_NL_F_CLOSED;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	ip_version = ecm_db_connection_ip_version_get(ci);
	ret = ecm_cl_nl_genl_attr_tuple_encode(&tuple,
						ip_version,
						proto,
						src_ip,
						src_port,
						dst_ip,
						dst_port);
	if (ret != 0) {
		DEBUG_WARN("failed to encode genl_attr_tuple: %d\n", ret);
		return;
	}

	ecm_classifier_nl_send_genl_msg(ECM_CL_NL_GENL_CMD_CONNECTION_CLOSED, &tuple);
}

/*
 * ignore ecm_classifier_messages ACCEL_OK and CLOSED
 */
static int ecm_classifier_nl_genl_msg_DUMP(struct sk_buff *skb,
					   struct netlink_callback *cb)
{
	return 0;
}

/*
 * ecm_classifier_nl_genl_msg_ACCEL()
 *	handles a ECM_CL_NL_ACCEL message
 */
static int ecm_classifier_nl_genl_msg_ACCEL(struct sk_buff *skb,
					    struct genl_info *info)
{
	int ret;
	struct nlattr *na;
	struct ecm_cl_nl_genl_attr_tuple *tuple;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_nl_instance *cnli;

	/* the netlink message comes to us in network order, but ECM
	   stores addresses in host order */
	int proto;
	int src_port;
	int dst_port;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;

	/*
	 * Check if we are enabled
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	if (!ecm_classifier_nl_enabled) {
		spin_unlock_bh(&ecm_classifier_nl_lock);
		return -ECONNREFUSED;
	}
	spin_unlock_bh(&ecm_classifier_nl_lock);

	na = info->attrs[ECM_CL_NL_GENL_ATTR_TUPLE];
	tuple = nla_data(na);

	ret = ecm_cl_nl_genl_attr_tuple_decode(tuple,
					       &proto,
					       src_ip,
					       &src_port,
					       dst_ip,
					       &dst_port);
	if (ret != 0) {
		DEBUG_WARN("failed to decode genl_attr_tuple: %d\n", ret);
		return ret;
	}

	/*
	 * Locate the connection using the tuple given
	 */
	DEBUG_TRACE("ACCEL: Lookup connection "
		    ECM_IP_ADDR_OCTAL_FMT ":%d <> "
		    ECM_IP_ADDR_OCTAL_FMT ":%d "
		    "protocol %d\n",
		    ECM_IP_ADDR_TO_OCTAL(src_ip),
		    src_port,
		    ECM_IP_ADDR_TO_OCTAL(dst_ip),
		    dst_port,
		    tuple->proto);
	ci = ecm_db_connection_find_and_ref(src_ip,
					    dst_ip,
					    proto,
					    src_port,
					    dst_port);
	if (!ci) {
		DEBUG_WARN("database connection not found\n");
		return -ENOENT;
	}
	DEBUG_TRACE("Connection found: %px\n", ci);

	/*
	 * Get the NL classifier for this connection
	 */
	cnli = (struct ecm_classifier_nl_instance *)
		ecm_db_connection_assigned_classifier_find_and_ref(ci,
			ECM_CLASSIFIER_TYPE_NL);
	if (!cnli) {
		ecm_db_connection_deref(ci);
		return -EUNATCH;
	}

	/*
	 * Allow acceleration of the connection.  This will be done as
	 * packets are processed in the usual way.
	 */
	DEBUG_TRACE("Permit accel: %px\n", ci);
	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->process_response.accel_mode =
		ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	cnli->flags |= ECM_CLASSIFIER_NL_F_ACCEL;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	cnli->base.deref((struct ecm_classifier_instance *)cnli);
	ecm_db_connection_deref(ci);

	return 0;
}

/*
 * ecm_classifier_nl_ref()
 *	Ref
 */
static void ecm_classifier_nl_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;
	cnli = (struct ecm_classifier_nl_instance *)ci;

	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);
	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->refs++;
	DEBUG_TRACE("%px: cnli ref %d\n", cnli, cnli->refs);
	DEBUG_ASSERT(cnli->refs > 0, "%px: ref wrap\n", cnli);
	spin_unlock_bh(&ecm_classifier_nl_lock);
}

/*
 * ecm_classifier_nl_deref()
 *	Deref
 */
static int ecm_classifier_nl_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;
	cnli = (struct ecm_classifier_nl_instance *)ci;

	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);

	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->refs--;
	DEBUG_ASSERT(cnli->refs >= 0, "%px: refs wrapped\n", cnli);
	DEBUG_TRACE("%px: Netlink classifier deref %d\n", cnli, cnli->refs);
	if (cnli->refs) {
		int refs = cnli->refs;
		spin_unlock_bh(&ecm_classifier_nl_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_nl_count--;
	DEBUG_ASSERT(ecm_classifier_nl_count >= 0, "%px: ecm_classifier_nl_count wrap\n", cnli);

	/*
	 * UnLink the instance from our list
	 */
	if (cnli->next) {
		cnli->next->prev = cnli->prev;
	}
	if (cnli->prev) {
		cnli->prev->next = cnli->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_nl_instances == cnli, "%px: list bad %px\n", cnli, ecm_classifier_nl_instances);
		ecm_classifier_nl_instances = cnli->next;
	}
	cnli->next = NULL;
	cnli->prev = NULL;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final Netlink classifier instance\n", cnli);
	kfree(cnli);

	return 0;
}

#if defined(CONFIG_NF_CONNTRACK_MARK)
void
ecm_classifier_nl_process_mark(struct ecm_classifier_nl_instance *cnli,
			       uint32_t mark)
{
	bool updated;
	ecm_front_end_acceleration_mode_t accel_mode;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;

	updated = false;

	spin_lock_bh(&ecm_classifier_nl_lock);

	/*
	 * If the mark is different to either of the current flow or return qos tags then we override them.
	 * NOTE: This will force a change of the skb priority and also drive through these qos tags in any acceleration rule.
	 */
	if ((mark != cnli->process_response.flow_qos_tag) || (mark != cnli->process_response.return_qos_tag)) {
		cnli->process_response.flow_qos_tag = mark;
		cnli->process_response.return_qos_tag = mark;
		cnli->process_response.process_actions |=
			ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
		updated = true;
	}
	spin_unlock_bh(&ecm_classifier_nl_lock);

	if (!updated) {
		return;
	}

	/*
	 * we need to make sure to propagate the new mark to the
	 * accel engine if the connection has been accelerated.  to do that,
	 * since there's no way to directly update an offload rule,
	 * we simply decelerate the connection which should result
	 * in a re-acceleration when the next packet is processed
	 * by the front end, thereby applying the new mark.
	 */

	/*
	 * Lookup the associated connection
	 */
	ci = ecm_db_connection_serial_find_and_ref(cnli->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cnli, cnli->ci_serial);
		return;
	}
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = ecm_front_end_connection_accel_state_get(feci);
	if ((accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL)
			|| (accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING)) {
		DEBUG_TRACE("%px: mark changed on offloaded connection, decelerate. new mark: 0x%08x\n",
			    cnli, mark);
		feci->decelerate(feci);
	} else {
		DEBUG_TRACE("%px: mark changed on non-offloaded connection. new mark: 0x%08x\n",
			    cnli, mark);
	}
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);
}
EXPORT_SYMBOL(ecm_classifier_nl_process_mark);
#endif

/*
 * ecm_classifier_nl_process()
 *	Process new data for connection
 */
static void ecm_classifier_nl_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_nl_instance *cnli;
	ecm_classifier_relevence_t relevance;
	bool enabled;
	struct ecm_db_connection_instance *ci;
	ecm_front_end_acceleration_mode_t accel_mode;
	uint32_t became_relevant = 0;

	cnli = (struct ecm_classifier_nl_instance *)aci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);

	/*
	 * Have we decided our relevance?  If so return our state.
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	relevance = cnli->process_response.relevance;
	if (relevance != ECM_CLASSIFIER_RELEVANCE_MAYBE) {
		*process_response = cnli->process_response;
		spin_unlock_bh(&ecm_classifier_nl_lock);
		return;
	}

	/*
	 * Decide upon relevance
	 */
	enabled = ecm_classifier_nl_enabled;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	/*
	 * If classifier is enabled, the connection is routed and the front end says it can accel then we are "relevant".
	 * Any other condition and we are not and will stop analysing this connection.
	 */
	relevance = ECM_CLASSIFIER_RELEVANCE_NO;
	ci = ecm_db_connection_serial_find_and_ref(cnli->ci_serial);
	if (ci) {
		struct ecm_front_end_connection_instance *feci;
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		accel_mode = ecm_front_end_connection_accel_state_get(feci);
		ecm_front_end_connection_deref(feci);
		if (enabled && ECM_FRONT_END_ACCELERATION_POSSIBLE(accel_mode) && ecm_db_connection_is_routed_get(ci)) {
			relevance = ECM_CLASSIFIER_RELEVANCE_YES;
			became_relevant = ecm_db_time_get();
		}
		ecm_db_connection_deref(ci);
	}

	/*
	 * Return process response
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->process_response.relevance = relevance;
	cnli->process_response.became_relevant = became_relevant;
	*process_response = cnli->process_response;
	spin_unlock_bh(&ecm_classifier_nl_lock);
}

/*
 * ecm_classifier_nl_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_nl_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_nl_instance *cnli;

	if (!(sync->tx_packet_count[ECM_CONN_DIR_FLOW] || sync->tx_packet_count[ECM_CONN_DIR_RETURN])) {
		/*
		 * Nothing to update.
		 * We only care about flows that are actively being accelerated.
		 */
		return;
	}

	cnli = (struct ecm_classifier_nl_instance *)aci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed", cnli);

	switch(sync->reason) {
	case ECM_FRONT_END_IPV4_RULE_SYNC_REASON_FLUSH:
		/* do nothing */
		DEBUG_TRACE("%px: nl_sync_to_v4: SYNC_FLUSH\n", cnli);
		break;
	case ECM_FRONT_END_IPV4_RULE_SYNC_REASON_EVICT:
		/* do nothing */
		DEBUG_TRACE("%px: nl_sync_to_v4: SYNC_EVICT\n", cnli);
		break;
	case ECM_FRONT_END_IPV4_RULE_SYNC_REASON_DESTROY:
		DEBUG_TRACE("%px: nl_sync_to_v4: SYNC_DESTROY\n", cnli);
		break;
	case ECM_FRONT_END_IPV4_RULE_SYNC_REASON_STATS:
		DEBUG_TRACE("%px: nl_sync_to_v4: SYNC_STATS\n", cnli);
		ecm_classifier_nl_genl_msg_ACCEL_OK(cnli);
		break;
	default:
		DEBUG_TRACE("%px: nl_sync_to_v4: unsupported reason\n", cnli);
		break;
	}
}

/*
 * ecm_classifier_nl_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_nl_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_nl_instance *cnli;

	cnli = (struct ecm_classifier_nl_instance *)aci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed", cnli);
}

/*
 * ecm_classifier_nl_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_nl_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_nl_instance *cnli;

	cnli = (struct ecm_classifier_nl_instance *)aci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed", cnli);

	if (!(sync->tx_packet_count[ECM_CONN_DIR_FLOW] || sync->tx_packet_count[ECM_CONN_DIR_RETURN])) {
		/*
		 * No traffic has been accelerated.
		 * Nothing to update. We only care about flows that are actively being accelerated.
		 */
		DEBUG_TRACE("%px: No traffic\n", cnli);
		return;
	}

	/*
	 * If this sync does NOT contain a final sync then we have seen traffic
	 * and that acceleration is continuing - acceleration is OK
	 */
	switch(sync->reason) {
	case ECM_FRONT_END_IPV6_RULE_SYNC_REASON_FLUSH:
		/* do nothing */
		DEBUG_TRACE("%px: nl_sync_to_v6: SYNC_FLUSH\n", cnli);
		break;
	case ECM_FRONT_END_IPV6_RULE_SYNC_REASON_EVICT:
		/* do nothing */
		DEBUG_TRACE("%px: nl_sync_to_v6: SYNC_EVICT\n", cnli);
		break;
	case ECM_FRONT_END_IPV6_RULE_SYNC_REASON_DESTROY:
		/* do nothing */
		DEBUG_TRACE("%px: nl_sync_to_v6: SYNC_DESTROY\n", cnli);
		break;
	case ECM_FRONT_END_IPV6_RULE_SYNC_REASON_STATS:
		DEBUG_TRACE("%px: nl_sync_to_v6: SYNC_STATS\n", cnli);
		ecm_classifier_nl_genl_msg_ACCEL_OK(cnli);
		break;
	default:
		DEBUG_TRACE("%px: nl_sync_to_v6: unsupported reason\n", cnli);
		break;
	}
}

/*
 * ecm_classifier_nl_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_nl_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_nl_instance *cnli;

	cnli = (struct ecm_classifier_nl_instance *)aci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed", cnli);

}

/*
 * ecm_classifier_nl_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_nl_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;
	cnli = (struct ecm_classifier_nl_instance *)ci;

	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);
	return ECM_CLASSIFIER_TYPE_NL;
}

/*
 * ecm_classifier_nl_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_nl_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_nl_instance *cnli;

	cnli = (struct ecm_classifier_nl_instance *)ci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);

	spin_lock_bh(&ecm_classifier_nl_lock);
	*process_response = cnli->process_response;
	spin_unlock_bh(&ecm_classifier_nl_lock);
}

/*
 * ecm_classifier_nl_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_nl_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;
	cnli = (struct ecm_classifier_nl_instance *)ci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);

	return true;
}

/*
 * ecm_classifier_nl_reclassify()
 *	Reclassify
 */
static void ecm_classifier_nl_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;
	cnli = (struct ecm_classifier_nl_instance *)ci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed\n", cnli);

	/*
	 * Revert back to MAYBE relevant - we will evaluate when we get the next process() call.
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	spin_unlock_bh(&ecm_classifier_nl_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_nl_state_get()
 *	Return state
 */
static int ecm_classifier_nl_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_nl_instance *cnli;
	struct ecm_classifier_process_response process_response;

	cnli = (struct ecm_classifier_nl_instance *)ci;
	DEBUG_CHECK_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC, "%px: magic failed", cnli);

	if ((result = ecm_state_prefix_add(sfi, "nl"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_nl_lock);
	process_response = cnli->process_response;
	spin_unlock_bh(&ecm_classifier_nl_lock);

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
 * ecm_classifier_nl_ct_get_and_ref()
 * 	Returns the conntrack entry for an ecm_db_connection_instance.
 * @param ci The connection instance to be used for the conntrack search.
 * @return struct nf_conn * A pointer and a reference to a matching conntrack
 *                          entry, or NULL if no such entry is found.
 * @pre The ci instance is not NULL.
 * @note This function takes a reference to the associated conntrack entry if
 *       it is found, and the caller must nf_ct_put() this entry when done.
 * @note FIXME: The param ci should be const, but none of the called functions
 *       are declared const.  This would be a larger change.
 */
static struct nf_conn *ecm_classifier_nl_ct_get_and_ref(struct ecm_db_connection_instance *ci)
{
	int ip_version;
	int proto;
	int src_port;
	int dst_port;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;
	struct nf_conntrack_tuple_hash *h;
	struct nf_conntrack_tuple tuple = {};

	DEBUG_ASSERT(ci != NULL, "ci was NULL for ct lookup");
	ip_version = ecm_db_connection_ip_version_get(ci);
	proto = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	src_port = (uint16_t)ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, dst_ip);
	dst_port = (uint16_t)ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO_NAT);

	if (ip_version == 4) {
		tuple.src.l3num = AF_INET;
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple.src.u3.ip, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(tuple.dst.u3.ip, dst_ip);
		goto ip_check_done;
	}
#ifdef ECM_IPV6_ENABLE
	if (ip_version == 6) {
		tuple.src.l3num = AF_INET6;
		ECM_IP_ADDR_TO_NIN6_ADDR(tuple.src.u3.in6, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(tuple.dst.u3.in6, dst_ip);
		goto ip_check_done;
	}
#endif
	return NULL;

ip_check_done:
	tuple.dst.protonum = proto;
	tuple.src.u.all = htons(src_port);
	tuple.dst.u.all = htons(dst_port);

	h = nf_conntrack_find_get(&init_net, &nf_ct_zone_dflt, &tuple);
	if (!h) {
		return NULL;
	}

	return nf_ct_tuplehash_to_ctrack(h);
}

/*
 * ecm_classifier_nl_connection_added()
 *	Invoked when a connection is added to the DB
 */
static void ecm_classifier_nl_connection_added(void *arg, struct ecm_db_connection_instance *ci)
{
	struct nf_conn *ct;
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_nl_instance *cnli;
	uint32_t serial __attribute__((unused)) = ecm_db_connection_serial_get(ci);
	DEBUG_INFO("%px: NL Listener: conn added with serial: %u\n", ci, serial);

	/*
	 * Only handle events if there is an NL classifier attached
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_NL);
	if (!classi) {
		DEBUG_TRACE("%px: Connection added ignored - no NL classifier\n", ci);
		return;
	}
	cnli = (struct ecm_classifier_nl_instance *)classi;
	DEBUG_TRACE("%px: added conn, serial: %u, NL classifier: %px\n", ci,
		    serial, classi);

	ct = ecm_classifier_nl_ct_get_and_ref(ci);
	if (!ct) {
		DEBUG_TRACE("%px: Connection add skipped - no associated CT entry.\n", ci);
		goto classi;
	}
	DEBUG_TRACE("%px: added conn, serial: %u, NL classifier: %px, CT: %px\n",
		    ci, serial, classi, ct);

#if defined(CONFIG_NF_CONNTRACK_MARK)
	spin_lock_bh(&ecm_classifier_nl_lock);
	cnli->process_response.flow_qos_tag = ct->mark;
	cnli->process_response.return_qos_tag = ct->mark;
	spin_unlock_bh(&ecm_classifier_nl_lock);
#endif
	nf_ct_put(ct);

classi:
	classi->deref(classi);

	return;
}

/*
 * ecm_classifier_nl_connection_removed()
 *	Invoked when a connection is removed from the DB
 */
static void ecm_classifier_nl_connection_removed(void *arg, struct ecm_db_connection_instance *ci)
{
	uint32_t serial __attribute__((unused)) = ecm_db_connection_serial_get(ci);
	struct ecm_classifier_instance *classi;
	struct ecm_classifier_nl_instance *cnli;
	bool accel_ok;
	int proto;
	int src_port;
	int dst_port;
	ip_addr_t src_ip;
	ip_addr_t dst_ip;

	DEBUG_INFO("%px: NL Listener: conn removed with serial: %u\n", ci, serial);

	/*
	 * Only handle events if there is an NL classifier attached
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_NL);
	if (!classi) {
		DEBUG_TRACE("%px: Connection removed ignored - no NL classifier\n", ci);
		return;
	}

	cnli = (struct ecm_classifier_nl_instance *)classi;
	DEBUG_INFO("%px: removed conn with serial: %u, NL classifier: %px\n", ci, serial, cnli);

	/*
	 * If the connection was accelerated OK then issue a close
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	accel_ok = (cnli->flags & ECM_CLASSIFIER_NL_F_ACCEL_OK)? true : false;
	spin_unlock_bh(&ecm_classifier_nl_lock);
	if (!accel_ok) {
		DEBUG_INFO("%px: cnli: %px, accel not ok\n", ci, cnli);
		classi->deref(classi);
		return;
	}

	proto = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	src_port = (uint16_t)ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);
	dst_port = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

	DEBUG_INFO("%px: NL classifier: %px, issue Close\n", ci, cnli);
	ecm_classifier_nl_genl_msg_closed(ci, cnli, proto, src_ip, dst_ip, src_port, dst_port);

	classi->deref(classi);
}

/*
 * ecm_classifier_nl_instance_alloc()
 *	Allocate an instance of the Netlink classifier
 */
struct ecm_classifier_nl_instance *ecm_classifier_nl_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_nl_instance *cnli;

	/*
	 * Allocate the instance
	 */
	cnli = (struct ecm_classifier_nl_instance *)kzalloc(sizeof(struct ecm_classifier_nl_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cnli) {
		DEBUG_WARN("Failed to allocate Netlink instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(cnli, ECM_CLASSIFIER_NL_INSTANCE_MAGIC);
	cnli->refs = 1;
	cnli->base.process = ecm_classifier_nl_process;
	cnli->base.sync_from_v4 = ecm_classifier_nl_sync_from_v4;
	cnli->base.sync_to_v4 = ecm_classifier_nl_sync_to_v4;
	cnli->base.sync_from_v6 = ecm_classifier_nl_sync_from_v6;
	cnli->base.sync_to_v6 = ecm_classifier_nl_sync_to_v6;
	cnli->base.type_get = ecm_classifier_nl_type_get;
	cnli->base.last_process_response_get = ecm_classifier_nl_last_process_response_get;
	cnli->base.reclassify_allowed = ecm_classifier_nl_reclassify_allowed;
	cnli->base.reclassify = ecm_classifier_nl_reclassify;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cnli->base.state_get = ecm_classifier_nl_state_get;
#endif
	cnli->base.ref = ecm_classifier_nl_ref;
	cnli->base.deref = ecm_classifier_nl_deref;
	cnli->ci_serial = ecm_db_connection_serial_get(ci);

	/*
	 * Classifier initially denies acceleration.
	 */
	cnli->process_response.flow_qos_tag = 0;
	cnli->process_response.return_qos_tag = 0;
	cnli->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	cnli->process_response.process_actions =
		ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	cnli->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;

	spin_lock_bh(&ecm_classifier_nl_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_nl_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_nl_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		kfree(cnli);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	cnli->next = ecm_classifier_nl_instances;
	if (ecm_classifier_nl_instances) {
		ecm_classifier_nl_instances->prev = cnli;
	}
	ecm_classifier_nl_instances = cnli;

	/*
	 * Increment stats
	 */
	ecm_classifier_nl_count++;
	DEBUG_ASSERT(ecm_classifier_nl_count > 0, "%px: ecm_classifier_nl_count wrap\n", cnli);
	spin_unlock_bh(&ecm_classifier_nl_lock);

	DEBUG_INFO("Netlink instance alloc: %px\n", cnli);
	return cnli;
}
EXPORT_SYMBOL(ecm_classifier_nl_instance_alloc);

/*
 * ecm_classifier_nl_set_command()
 *	Set Netlink command to accel/decel connection.
 */
static ssize_t ecm_classifier_nl_set_command(struct file *file,
						const char __user *user_buf,
						size_t sz, loff_t *ppos)

{
#define ECM_CLASSIFIER_NL_SET_IP_COMMAND_FIELDS 7
	char *cmd_buf;
	int field_count;
	char *field_ptr;
	char *fields[ECM_CLASSIFIER_NL_SET_IP_COMMAND_FIELDS];
	char cmd;
	uint32_t serial;
	ip_addr_t src_ip;
	uint32_t src_port;
	int proto;
	ip_addr_t dest_ip;
	uint32_t dest_port;
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_nl_instance *cnli;
	struct ecm_front_end_connection_instance *feci;

	/*
	 * Check if we are enabled
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	if (!ecm_classifier_nl_enabled) {
		spin_unlock_bh(&ecm_classifier_nl_lock);
		return -EINVAL;
	}
	spin_unlock_bh(&ecm_classifier_nl_lock);

	/*
	 * buf is formed as:
	 * [0]   [1]      [2]      [3]        [4]     [5]       [6]
	 * <CMD>/<SERIAL>/<src_ip>/<src_port>/<proto>/<dest_ip>/<dest_port>
	 * CMD:
	 *	F = Accelerate based on IP address, <SERIAL> unused
	 *	f = Decelerate based on IP address, <SERIAL> unused
	 *	S = Accelerate based on serial, <SERIAL> only becomes relevant
	 *	s = Decelerate based on serial, <SERIAL> only becomes relevant
	 */
	cmd_buf = (char *)kzalloc(sz + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		return -ENOMEM;
	}

	sz = simple_write_to_buffer(cmd_buf, sz, ppos, user_buf, sz);

	/*
	 * Split the buffer into its fields
	 */
	field_count = 0;
	field_ptr = cmd_buf;
	fields[field_count] = strsep(&field_ptr, "/");
	while (fields[field_count] != NULL) {
		DEBUG_TRACE("FIELD %d: %s\n", field_count, fields[field_count]);
		field_count++;
		if (field_count == ECM_CLASSIFIER_NL_SET_IP_COMMAND_FIELDS) {
			break;
		}
		fields[field_count] = strsep(&field_ptr, "/");
	}

	if (field_count != ECM_CLASSIFIER_NL_SET_IP_COMMAND_FIELDS) {
		DEBUG_WARN("invalid field count %d\n", field_count);
		kfree(cmd_buf);
		return -EINVAL;
	}

	sscanf(fields[0], "%c", &cmd);
	sscanf(fields[1], "%u", &serial);
	ecm_string_to_ip_addr(src_ip, fields[2]);
	sscanf(fields[3], "%u", &src_port);
	sscanf(fields[4], "%d", &proto);
	ecm_string_to_ip_addr(dest_ip, fields[5]);
	sscanf(fields[6], "%u", &dest_port);

	kfree(cmd_buf);

	/*
	 * Locate the connection using the serial or tuple given
	 */
	switch (cmd) {
	case 'F':
	case 'f':
		DEBUG_TRACE("Lookup connection " ECM_IP_ADDR_OCTAL_FMT ":%d <> " ECM_IP_ADDR_OCTAL_FMT ":%d protocol %d\n",
				ECM_IP_ADDR_TO_OCTAL(src_ip), src_port, ECM_IP_ADDR_TO_OCTAL(dest_ip), dest_port, proto);
		ci = ecm_db_connection_find_and_ref(src_ip, dest_ip, proto, src_port, dest_port);
		break;
	case 'S':
	case 's':
		DEBUG_TRACE("Lookup connection using serial: %u\n", serial);
		ci = ecm_db_connection_serial_find_and_ref(serial);
		break;
	default:
		DEBUG_WARN("invalid cmd %c\n", cmd);
		return -EINVAL;
	}

	if (!ci) {
		DEBUG_WARN("database connection not found\n");
		return -ENOMEM;
	}
	DEBUG_TRACE("Connection found: %px\n", ci);

	/*
	 * Get the NL classifier
	 */
	cnli = (struct ecm_classifier_nl_instance *)ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_NL);
	if (!cnli) {
		ecm_db_connection_deref(ci);
		return -ENOMEM;
	}

	/*
	 * Now action the command
	 */
	switch (cmd) {
	case 's':
	case 'f':
		/*
		 * Decelerate the connection, NL is denying further accel until it says so.
		 */
		DEBUG_TRACE("Force decel: %px\n", ci);
		spin_lock_bh(&ecm_classifier_nl_lock);
		cnli->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
		spin_unlock_bh(&ecm_classifier_nl_lock);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		feci->decelerate(feci);
		ecm_front_end_connection_deref(feci);
		break;
	case 'S':
	case 'F':
		/*
		 * Allow acceleration of the connection.  This will be done as packets are processed in the usual way.
		 */
		DEBUG_TRACE("Permit accel: %px\n", ci);
		spin_lock_bh(&ecm_classifier_nl_lock);
		cnli->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
		cnli->flags |= ECM_CLASSIFIER_NL_F_ACCEL;
		spin_unlock_bh(&ecm_classifier_nl_lock);
		break;
	}

	cnli->base.deref((struct ecm_classifier_instance *)cnli);
	ecm_db_connection_deref(ci);

	return sz;
}

/*
 * ecm_classifier_nl_rule_get_enabled()
 */
static int ecm_classifier_nl_rule_get_enabled(void *data, u64 *val)
{
	DEBUG_TRACE("get enabled\n");

	*val = ecm_classifier_nl_enabled;

	return 0;
}

/*
 * ecm_classifier_nl_rule_set_enabled()
 */
static int ecm_classifier_nl_rule_set_enabled(void *data, u64 val)
{
	bool prev_state;

	/*
	 * Boolean-ise
	 */
	if (val) {
		val = true;
	}
	DEBUG_TRACE("ecm_classifier_nl_enabled = %d\n", (int)val);

	/*
	 * Operate under our locks
	 */
	spin_lock_bh(&ecm_classifier_nl_lock);
	prev_state = ecm_classifier_nl_enabled;
	ecm_classifier_nl_enabled = (bool)val;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	/*
	 * If there is a change in operating state, and that change is that we are now disabled
	 * then we need to re-generate all connections relevant to this classifier type
	 */
	if (!val && (prev_state ^ (bool)val)) {
		/*
		 * Change in state to become disabled.
		 */
#ifdef ECM_DB_CTA_TRACK_ENABLE
		ecm_db_connection_regenerate_by_assignment_type(ECM_CLASSIFIER_TYPE_NL);
#else
		ecm_db_regeneration_needed();
#endif
	}
	return 0;
}

/*
 * Debugfs attribute for the nl classifier enabled flag.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_classifier_nl_enabled_fops, ecm_classifier_nl_rule_get_enabled, ecm_classifier_nl_rule_set_enabled, "%llu\n");

/*
 * File operations for nl classifier command.
 */
static struct file_operations ecm_classifier_nl_cmd_fops = {
	.write = ecm_classifier_nl_set_command,
};

/*
 * Generic Netlink attr checking policies
 */
static struct nla_policy
ecm_cl_nl_genl_policy[ECM_CL_NL_GENL_ATTR_COUNT] = {
	[ECM_CL_NL_GENL_ATTR_TUPLE] = {
		.type = NLA_UNSPEC,
		.len = sizeof(struct ecm_cl_nl_genl_attr_tuple), },
};

/*
 * Generic Netlink message-to-handler mapping
 */
static struct genl_ops ecm_cl_nl_genl_ops[] = {
	{
		.cmd = ECM_CL_NL_GENL_CMD_ACCEL,
		.flags = 0,
		.policy = ecm_cl_nl_genl_policy,
		.doit = ecm_classifier_nl_genl_msg_ACCEL,
		.dumpit = NULL,
	},
	{
		.cmd = ECM_CL_NL_GENL_CMD_ACCEL_OK,
		.flags = 0,
		.policy = ecm_cl_nl_genl_policy,
		.doit = NULL,
		.dumpit = ecm_classifier_nl_genl_msg_DUMP,
	},
	{
		.cmd = ECM_CL_NL_GENL_CMD_CONNECTION_CLOSED,
		.flags = 0,
		.policy = ecm_cl_nl_genl_policy,
		.doit = NULL,
		.dumpit = ecm_classifier_nl_genl_msg_DUMP,
	},
};

static int ecm_classifier_nl_register_genl(void)
{

	return genl_register_family_with_ops_groups(&ecm_cl_nl_genl_family,
						      ecm_cl_nl_genl_ops,
						      ecm_cl_nl_genl_mcgrp);
}

static void ecm_classifier_nl_unregister_genl(void)
{
	genl_unregister_family(&ecm_cl_nl_genl_family);
}

/*
 * ecm_classifier_nl_rules_init()
 */
int ecm_classifier_nl_rules_init(struct dentry *dentry)
{
	int result;
	DEBUG_INFO("Netlink classifier Module init\n");

	ecm_classifier_nl_dentry = debugfs_create_dir("ecm_classifier_nl", dentry);
	if (!ecm_classifier_nl_dentry) {
		DEBUG_ERROR("Failed to create ecm nl classifier directory in debugfs\n");
		return -1;
	}

	if (!debugfs_create_file("enabled", S_IRUGO | S_IWUSR, ecm_classifier_nl_dentry,
					NULL, &ecm_classifier_nl_enabled_fops)) {
		DEBUG_ERROR("Failed to create ecm nl classifier enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_nl_dentry);
		return -1;
	}

	if (!debugfs_create_file("cmd", S_IRUGO | S_IWUSR, ecm_classifier_nl_dentry,
					NULL, &ecm_classifier_nl_cmd_fops)) {
		DEBUG_ERROR("Failed to create ecm nl classifier cmd file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_nl_dentry);
		return -1;
	}

	result = ecm_classifier_nl_register_genl();
	if (result) {
		DEBUG_ERROR("Failed to register genl sockets\n");
		return result;
	}

	/*
	 * Allocate listener instance to listen for db events
	 */
	ecm_classifier_nl_li = ecm_db_listener_alloc();
	if (!ecm_classifier_nl_li) {
		DEBUG_ERROR("Failed to allocate listener\n");
		return -1;
	}

	/*
	 * Add the listener into the database
	 * NOTE: Ref the thread count for the listener
	 */
	ecm_db_listener_add(ecm_classifier_nl_li,
			NULL /* ecm_classifier_nl_iface_added */,
			NULL /* ecm_classifier_nl_iface_removed */,
			NULL /* ecm_classifier_nl_node_added */,
			NULL /* ecm_classifier_nl_node_removed */,
			NULL /* ecm_classifier_nl_host_added */,
			NULL /* ecm_classifier_nl_host_removed */,
			NULL /* ecm_classifier_nl_mapping_added */,
			NULL /* ecm_classifier_nl_mapping_removed */,
			ecm_classifier_nl_connection_added,
			ecm_classifier_nl_connection_removed,
			NULL /* ecm_classifier_nl_listener_final */,
			ecm_classifier_nl_li);

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_nl_rules_init);

/*
 * ecm_classifier_nl_rules_exit()
 */
void ecm_classifier_nl_rules_exit(void)
{
	DEBUG_INFO("Netlink classifier Module exit\n");

	/*
	 * Release our ref to the listener.
	 * This will cause it to be unattached to the db listener list.
	 * GGG TODO THIS IS TOTALLY BROKEN (DUE TO REF COUNT HANDLING NOT BEING HONOURED)
	 */
	if (ecm_classifier_nl_li) {
		ecm_db_listener_deref(ecm_classifier_nl_li);
		ecm_classifier_nl_li = NULL;
	}

	spin_lock_bh(&ecm_classifier_nl_lock);
	ecm_classifier_nl_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_nl_lock);

	ecm_classifier_nl_unregister_genl();

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_nl_dentry) {
		debugfs_remove_recursive(ecm_classifier_nl_dentry);
	}
}
EXPORT_SYMBOL(ecm_classifier_nl_rules_exit);
