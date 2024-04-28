/*
 **************************************************************************
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * ecm_conntrack_notifier.c
 * 	Conntrack notifier functionality.
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

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CONNTRACK_NOTIFIER_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_tracker_datagram.h"
#include "ecm_db.h"
#include "ecm_front_end_ipv4.h"
#include "ecm_front_end_ipv6.h"
#ifdef ECM_CLASSIFIER_NL_ENABLE
#include "ecm_classifier_nl.h"
#endif

/*
 * Locking of the classifier - concurrency control
 */
static DEFINE_SPINLOCK(ecm_conntrack_notifier_lock __attribute__((unused)));	/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_conntrack_notifier_dentry;

/*
 * General operational control
 */
static int ecm_conntrack_notifier_stopped = 0;	/* When non-zero further traffic will not be processed */

/*
 * ecm_conntrack_ipv6_event_destroy()
 *	Handles conntrack destroy events
 */
static void ecm_conntrack_ipv6_event_destroy(struct nf_conn *ct)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Destroy event for ct: %p\n", ct);

	ci = ecm_db_connection_ipv6_from_ct_get_and_ref(ct);
	if (!ci) {
		DEBUG_TRACE("%p: not found\n", ct);
		return;
	}
	DEBUG_INFO("%p: Connection defunct %p\n", ct, ci);

	/*
	 * Force destruction of the connection by making it defunct
	 */
	ecm_db_connection_make_defunct(ci);
	ecm_db_connection_deref(ci);
}

#if defined(CONFIG_NF_CONNTRACK_MARK)
/*
 * ecm_conntrack_ipv6_event_mark()
 *	Handles conntrack mark events
 */
static void ecm_conntrack_ipv6_event_mark(struct nf_conn *ct)
{
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *__attribute__((unused))cls;

	DEBUG_INFO("Mark event for ct: %p\n", ct);

	/*
	 * Ignore transitions to zero
	 */
	if (ct->mark == 0) {
		return;
	}

	ci = ecm_db_connection_ipv6_from_ct_get_and_ref(ct);
	if (!ci) {
		DEBUG_TRACE("%p: not found\n", ct);
		return;
	}

#ifdef ECM_CLASSIFIER_NL_ENABLE
	/*
	 * As of now, only the Netlink classifier is interested in conmark changes
	 * GGG TODO Add a classifier method to propagate this information to any and all types of classifier.
	 */
	cls = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_NL);
	if (cls) {
		ecm_classifier_nl_process_mark((struct ecm_classifier_nl_instance *)cls, ct->mark);
		cls->deref(cls);
	}
#endif
	/*
	 * All done
	 */
	ecm_db_connection_deref(ci);
}
#endif

/*
 * ecm_conntrack_ipv6_event()
 *	Callback event invoked when conntrack connection state changes, currently we handle destroy events to quickly release state
 */
int ecm_conntrack_ipv6_event(unsigned long events, struct nf_conn *ct)
{
	/*
	 * If operations have stopped then do not process event
	 */
	if (unlikely(ecm_front_end_ipv6_stopped)) {
		DEBUG_WARN("Ignoring event - stopped\n");
		return NOTIFY_DONE;
	}

	if (!ct) {
		DEBUG_WARN("Error: no ct\n");
		return NOTIFY_DONE;
	}

	/*
	 * handle destroy events
	 */
	if (events & (1 << IPCT_DESTROY)) {
		DEBUG_TRACE("%p: Event is destroy\n", ct);
		ecm_conntrack_ipv6_event_destroy(ct);
	}

#if defined(CONFIG_NF_CONNTRACK_MARK)
	/*
	 * handle mark change events
	 */
	if (events & (1 << IPCT_MARK)) {
		DEBUG_TRACE("%p: Event is mark\n", ct);
		ecm_conntrack_ipv6_event_mark(ct);
	}
#endif
	return NOTIFY_DONE;
}
EXPORT_SYMBOL(ecm_conntrack_ipv6_event);

/*
 * ecm_conntrack_ipv4_event_destroy()
 *	Handles conntrack destroy events
 */
static void ecm_conntrack_ipv4_event_destroy(struct nf_conn *ct)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Destroy event for ct: %p\n", ct);

	ci = ecm_db_connection_ipv4_from_ct_get_and_ref(ct);
	if (!ci) {
		DEBUG_TRACE("%p: not found\n", ct);
		return;
	}
	DEBUG_INFO("%p: Connection defunct %p\n", ct, ci);

	/*
	 * Force destruction of the connection by making it defunct
	 */
	ecm_db_connection_make_defunct(ci);
	ecm_db_connection_deref(ci);
}

#if defined(CONFIG_NF_CONNTRACK_MARK)
/*
 * ecm_conntrack_ipv4_event_mark()
 *	Handles conntrack mark events
 */
static void ecm_conntrack_ipv4_event_mark(struct nf_conn *ct)
{
	struct ecm_db_connection_instance *ci;
	struct ecm_classifier_instance *__attribute__((unused))cls;

	DEBUG_INFO("Mark event for ct: %p\n", ct);

	/*
	 * Ignore transitions to zero
	 */
	if (ct->mark == 0) {
		return;
	}

	ci = ecm_db_connection_ipv4_from_ct_get_and_ref(ct);
	if (!ci) {
		DEBUG_TRACE("%p: not found\n", ct);
		return;
	}

#ifdef ECM_CLASSIFIER_NL_ENABLE
	/*
	 * As of now, only the Netlink classifier is interested in conmark changes
	 * GGG TODO Add a classifier method to propagate this information to any and all types of classifier.
	 */
	cls = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_NL);
	if (cls) {
		ecm_classifier_nl_process_mark((struct ecm_classifier_nl_instance *)cls, ct->mark);
		cls->deref(cls);
	}
#endif

	/*
	 * All done
	 */
	ecm_db_connection_deref(ci);
}
#endif

/*
 * ecm_conntrack_ipv4_event()
 *	Callback event invoked when conntrack connection state changes, currently we handle destroy events to quickly release state
 */
int ecm_conntrack_ipv4_event(unsigned long events, struct nf_conn *ct)
{
	/*
	 * If operations have stopped then do not process event
	 */
	if (unlikely(ecm_front_end_ipv4_stopped)) {
		DEBUG_WARN("Ignoring event - stopped\n");
		return NOTIFY_DONE;
	}

	if (!ct) {
		DEBUG_WARN("Error: no ct\n");
		return NOTIFY_DONE;
	}

	/*
	 * handle destroy events
	 */
	if (events & (1 << IPCT_DESTROY)) {
		DEBUG_TRACE("%p: Event is destroy\n", ct);
		ecm_conntrack_ipv4_event_destroy(ct);
	}

#if defined(CONFIG_NF_CONNTRACK_MARK)
	/*
	 * handle mark change events
	 */
	if (events & (1 << IPCT_MARK)) {
		DEBUG_TRACE("%p: Event is mark\n", ct);
		ecm_conntrack_ipv4_event_mark(ct);
	}
#endif
	return NOTIFY_DONE;
}
EXPORT_SYMBOL(ecm_conntrack_ipv4_event);

#ifdef CONFIG_NF_CONNTRACK_EVENTS
/*
 * ecm_conntrack_event()
 *	Callback event invoked when conntrack connection state changes, currently we handle destroy events to quickly release state
 */
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
static int ecm_conntrack_event(struct notifier_block *this, unsigned long events, void *ptr)
#else
static int ecm_conntrack_event(unsigned int events, struct nf_ct_event *item)
#endif
{
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
	struct nf_ct_event *item = (struct nf_ct_event *)ptr;
#endif
	struct nf_conn *ct = item->ct;

	/*
	 * If operations have stopped then do not process event
	 */
	spin_lock_bh(&ecm_conntrack_notifier_lock);
	if (unlikely(ecm_conntrack_notifier_stopped)) {
		DEBUG_WARN("Ignoring event - stopped\n");
		spin_unlock_bh(&ecm_conntrack_notifier_lock);
		return NOTIFY_DONE;
	}
	spin_unlock_bh(&ecm_conntrack_notifier_lock);

	if (!ct) {
		DEBUG_WARN("Error: no ct\n");
		return NOTIFY_DONE;
	}

	/*
	 * Only interested if this is IPv4 or IPv6.
	 */
	if (nf_ct_l3num(ct) == AF_INET) {
		return ecm_conntrack_ipv4_event(events, ct);
	}
#ifdef ECM_IPV6_ENABLE
	if (nf_ct_l3num(ct) == AF_INET6) {
		return ecm_conntrack_ipv6_event(events, ct);
	}
#endif
	return NOTIFY_DONE;
}

#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
/*
 * struct notifier_block ecm_conntrack_notifier
 *	Netfilter conntrack event system to monitor connection tracking changes
 */
static struct notifier_block ecm_conntrack_notifier = {
	.notifier_call	= ecm_conntrack_event,
};
#else
/*
 * struct nf_ct_event_notifier ecm_conntrack_notifier
 *	Netfilter conntrack event system to monitor connection tracking changes
 */
static struct nf_ct_event_notifier ecm_conntrack_notifier = {
	.fcn	= ecm_conntrack_event,
};
#endif
#endif

/*
 * ecm_conntrack_notifier_stop()
 */
void ecm_conntrack_notifier_stop(int num)
{
	ecm_conntrack_notifier_stopped = num;
}
EXPORT_SYMBOL(ecm_conntrack_notifier_stop);

/*
 * ecm_conntrack_notifier_init()
 */
int ecm_conntrack_notifier_init(struct dentry *dentry)
{
	int result __attribute__((unused));
	DEBUG_INFO("ECM Conntrack Notifier init\n");

	ecm_conntrack_notifier_dentry = debugfs_create_dir("ecm_conntrack_notifier", dentry);
	if (!ecm_conntrack_notifier_dentry) {
		DEBUG_ERROR("Failed to create ecm conntrack notifier directory in debugfs\n");
		return -1;
	}

	debugfs_create_u32("stop", S_IRUGO | S_IWUSR, ecm_conntrack_notifier_dentry,
					(u32 *)&ecm_conntrack_notifier_stopped);

#ifdef CONFIG_NF_CONNTRACK_EVENTS
	/*
	 * Eventing subsystem is available so we register a notifier hook to get fast notifications of expired connections
	 */
	result = nf_conntrack_register_chain_notifier(&init_net, &ecm_conntrack_notifier);
	if (result < 0) {
		DEBUG_ERROR("Can't register nf notifier hook.\n");
		debugfs_remove_recursive(ecm_conntrack_notifier_dentry);
		return result;
	}
#endif

	return 0;
}
EXPORT_SYMBOL(ecm_conntrack_notifier_init);

/*
 * ecm_conntrack_notifier_exit()
 */
void ecm_conntrack_notifier_exit(void)
{
	DEBUG_INFO("ECM Conntrack Notifier exit\n");
#ifdef CONFIG_NF_CONNTRACK_EVENTS
	nf_conntrack_unregister_chain_notifier(&init_net, &ecm_conntrack_notifier);
#endif
	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_conntrack_notifier_dentry) {
		debugfs_remove_recursive(ecm_conntrack_notifier_dentry);
	}
}
EXPORT_SYMBOL(ecm_conntrack_notifier_exit);
