/*
 **************************************************************************
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
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
#include <linux/random.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <net/ip6_route.h>
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
#define DEBUG_LEVEL ECM_DB_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_classifier_default.h"
#include "ecm_db.h"

/*
 * Magic number.
 */
#define ECM_DB_LISTENER_INSTANCE_MAGIC 0x9876

/*
 * Listeners
 */
static int ecm_db_listeners_count = 0;			/* Number of listeners allocated */
static struct ecm_db_listener_instance *ecm_db_listeners = NULL;
							/* Event listeners */

/*
 * _ecm_db_listener_ref()
 */
static void _ecm_db_listener_ref(struct ecm_db_listener_instance *li)
{
	DEBUG_CHECK_MAGIC(li, ECM_DB_LISTENER_INSTANCE_MAGIC, "%p: magic failed", li);
	li->refs++;
	DEBUG_ASSERT(li->refs > 0, "%p: ref wrap\n", li);
}

/*
 * ecm_db_listener_ref()
 */
void ecm_db_listener_ref(struct ecm_db_listener_instance *li)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_listener_ref(li);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_listener_ref);

/*
 * ecm_db_listeners_get_and_ref_first()
 *	Obtain a ref to the first listener instance, if any
 */
struct ecm_db_listener_instance *ecm_db_listeners_get_and_ref_first(void)
{
	struct ecm_db_listener_instance *li;
	spin_lock_bh(&ecm_db_lock);
	li = ecm_db_listeners;
	if (li) {
		_ecm_db_listener_ref(li);
	}
	spin_unlock_bh(&ecm_db_lock);
	return li;
}

/*
 * ecm_db_listener_get_and_ref_next()
 *	Return the next listener in the list given a listener
 */
struct ecm_db_listener_instance *ecm_db_listener_get_and_ref_next(struct ecm_db_listener_instance *li)
{
	struct ecm_db_listener_instance *lin;
	DEBUG_CHECK_MAGIC(li, ECM_DB_LISTENER_INSTANCE_MAGIC, "%p: magic failed", li);
	spin_lock_bh(&ecm_db_lock);
	lin = li->next;
	if (lin) {
		_ecm_db_listener_ref(lin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return lin;
}

/*
 * ecm_db_listener_deref()
 *	Release reference to listener.
 *
 * On final reference release listener shall be removed from the database.
 */
int ecm_db_listener_deref(struct ecm_db_listener_instance *li)
{
	struct ecm_db_listener_instance *cli;
	struct ecm_db_listener_instance **cli_prev;

	DEBUG_CHECK_MAGIC(li, ECM_DB_LISTENER_INSTANCE_MAGIC, "%p: magic failed", li);

	spin_lock_bh(&ecm_db_lock);
	li->refs--;
	DEBUG_ASSERT(li->refs >= 0, "%p: ref wrap\n", li);
	if (li->refs > 0) {
		int refs;
		refs = li->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

	/*
	 * Instance is to be removed and destroyed.
	 * Link the listener out of the listener list.
	 */
	cli = ecm_db_listeners;
	cli_prev = &ecm_db_listeners;
	while (cli) {
		if (cli == li) {
			*cli_prev = cli->next;
			break;
		}
		cli_prev = &cli->next;
		cli = cli->next;
	}
	DEBUG_ASSERT(cli, "%p: not found\n", li);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Invoke final callback
	 */
	if (li->final) {
		li->final(li->arg);
	}
	DEBUG_CLEAR_MAGIC(li);
	kfree(li);

	/*
	 * Decrease global listener count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_listeners_count--;
	DEBUG_ASSERT(ecm_db_listeners_count >= 0, "%p: listener count wrap\n", li);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_listener_deref);

/*
 * ecm_db_listener_add()
 *	Add a listener instance into the database.
 */
void ecm_db_listener_add(struct ecm_db_listener_instance *li,
							ecm_db_iface_listener_added_callback_t iface_added,
							ecm_db_iface_listener_removed_callback_t iface_removed,
							ecm_db_node_listener_added_callback_t node_added,
							ecm_db_node_listener_removed_callback_t node_removed,
							ecm_db_host_listener_added_callback_t host_added,
							ecm_db_host_listener_removed_callback_t host_removed,
							ecm_db_mapping_listener_added_callback_t mapping_added,
							ecm_db_mapping_listener_removed_callback_t mapping_removed,
							ecm_db_connection_listener_added_callback_t connection_added,
							ecm_db_connection_listener_removed_callback_t connection_removed,
							ecm_db_listener_final_callback_t final,
							void *arg)
{
	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(li, ECM_DB_LISTENER_INSTANCE_MAGIC, "%p: magic failed\n", li);
	DEBUG_ASSERT(!(li->flags & ECM_DB_LISTENER_FLAGS_INSERTED), "%p: inserted\n", li);
	spin_unlock_bh(&ecm_db_lock);

	li->arg = arg;
	li->final = final;
	li->iface_added = iface_added;
	li->iface_removed = iface_removed;
	li->node_added = node_added;
	li->node_removed = node_removed;
	li->host_added = host_added;
	li->host_removed = host_removed;
	li->mapping_added = mapping_added;
	li->mapping_removed = mapping_removed;
	li->connection_added = connection_added;
	li->connection_removed = connection_removed;

	/*
	 * Add instance into listener list
	 */
	spin_lock_bh(&ecm_db_lock);
	li->flags |= ECM_DB_LISTENER_FLAGS_INSERTED;
	li->next = ecm_db_listeners;
	ecm_db_listeners = li;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_listener_add);

/*
 * ecm_db_listener_alloc()
 *	Allocate a listener instance
 */
struct ecm_db_listener_instance *ecm_db_listener_alloc(void)
{
	struct ecm_db_listener_instance *li;

	li = (struct ecm_db_listener_instance *)kzalloc(sizeof(struct ecm_db_listener_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!li) {
		DEBUG_WARN("Alloc failed\n");
		return NULL;
	}

	li->refs = 1;
	DEBUG_SET_MAGIC(li, ECM_DB_LISTENER_INSTANCE_MAGIC);

	/*
	 * Alloc operation must be atomic to ensure thread and module can be held
	 */
	spin_lock_bh(&ecm_db_lock);

	/*
	 * If the event processing thread is terminating then we cannot create new instances
	 */
	if (ecm_db_terminate_pending) {
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_WARN("Thread terminating\n");
		kfree(li);
		return NULL;
	}

	ecm_db_listeners_count++;
	DEBUG_ASSERT(ecm_db_listeners_count > 0, "%p: listener count wrap\n", li);
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Listener created %p\n", li);
	return li;
}
EXPORT_SYMBOL(ecm_db_listener_alloc);
