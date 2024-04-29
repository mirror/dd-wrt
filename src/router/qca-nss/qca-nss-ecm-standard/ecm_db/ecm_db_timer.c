/*
 **************************************************************************
 * Copyright (c) 2014-2018, 2020-2021, The Linux Foundation. All rights reserved.
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
 * Check the configured HZ value.
 */
#if HZ > 100000
#error "Bad HZ value"
#endif

/*
 * Timers and cleanup
 */
uint32_t ecm_db_time = 0;					/* Time in seconds since start */
								/* Timer groups */
static struct timer_list ecm_db_timer;				/* Timer to drive timer groups */

struct ecm_db_timer_group ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_MAX];

/*
 * _ecm_db_timer_group_entry_remove()
 *	Remove the entry from its timer group, returns false if the entry has already expired.
 */
bool _ecm_db_timer_group_entry_remove(struct ecm_db_timer_group_entry *tge)
{
	struct ecm_db_timer_group *timer_group;

	/*
	 * If not in a timer group then it is already removed
	 */
	if (tge->group == ECM_DB_TIMER_GROUPS_MAX) {
		return false;
	}

	/*
	 * Remove the connection from its current group
	 */
	timer_group = &ecm_db_timer_groups[tge->group];

	/*
	 * Somewhere in the list?
	 */
	if (tge->prev) {
		tge->prev->next = tge->next;
	} else {
		/*
		 * First in the group
		 */
		DEBUG_ASSERT(timer_group->head == tge, "%px: bad head, expecting %px, got %px\n", timer_group, tge, timer_group->head);
		timer_group->head = tge->next;
	}

	if (tge->next) {
		tge->next->prev = tge->prev;
	} else {
		/*
		 * No next so this must be the last item - we need to adjust the tail pointer
		 */
		DEBUG_ASSERT(timer_group->tail == tge, "%px: bad tail, expecting %px got %px\n", timer_group, tge, timer_group->tail);
		timer_group->tail = tge->prev;
	}

	/*
	 * No longer a part of a timer group
	 */
	tge->group = ECM_DB_TIMER_GROUPS_MAX;
	return true;
}

/*
 * ecm_db_timer_group_entry_remove()
 *	Remove the connection from its timer group, returns false if the entry has already expired.
 */
bool ecm_db_timer_group_entry_remove(struct ecm_db_timer_group_entry *tge)
{
	bool res;
	spin_lock_bh(&ecm_db_lock);
	res = _ecm_db_timer_group_entry_remove(tge);
	spin_unlock_bh(&ecm_db_lock);
	return res;
}
EXPORT_SYMBOL(ecm_db_timer_group_entry_remove);

/*
 * _ecm_db_timer_group_entry_set()
 *	Set the timer group to which this entry will be a member
 */
void _ecm_db_timer_group_entry_set(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg)
{
	struct ecm_db_timer_group *timer_group;

	DEBUG_ASSERT(tge->group == ECM_DB_TIMER_GROUPS_MAX, "%px: already set\n", tge);

	/*
	 * Set group
	 */
	tge->group = tg;
	timer_group = &ecm_db_timer_groups[tge->group];
	tge->timeout = timer_group->time + ecm_db_time;

	/*
	 * Insert into a timer group at the head (as this is now touched)
	 */
	tge->prev = NULL;
	tge->next = timer_group->head;
	if (!timer_group->head) {
		/*
		 * As there is no head there is also no tail so we need to set that
		 */
		timer_group->tail = tge;
	} else {
		/*
		 * As there is a head already there must be a tail.  Since we insert before
		 * the current head we don't adjust the tail.
		 */
		timer_group->head->prev = tge;
	}
	timer_group->head = tge;
}

/*
 * ecm_db_timer_group_entry_reset()
 *	Re-set the timer group to which this entry will be a member.
 *
 * Returns false if the timer cannot be reset because it has expired
 */
bool ecm_db_timer_group_entry_reset(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg)
{
	spin_lock_bh(&ecm_db_lock);

	/*
	 * Remove it from its current group, if any
	 */
	if (!_ecm_db_timer_group_entry_remove(tge)) {
		spin_unlock_bh(&ecm_db_lock);
		return false;
	}

	/*
	 * Set new group
	 */
	_ecm_db_timer_group_entry_set(tge, tg);
	spin_unlock_bh(&ecm_db_lock);
	return true;
}
EXPORT_SYMBOL(ecm_db_timer_group_entry_reset);

/*
 * ecm_db_timer_group_entry_set()
 *	Set the timer group to which this entry will be a member
 */
void ecm_db_timer_group_entry_set(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_timer_group_entry_set(tge, tg);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_timer_group_entry_set);

/*
 * ecm_db_timer_group_entry_init()
 *	Initialise a timer entry ready for setting
 */
void ecm_db_timer_group_entry_init(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_entry_callback_t fn, void *arg)
{
	memset(tge, 0, sizeof(struct ecm_db_timer_group_entry));
	tge->group = ECM_DB_TIMER_GROUPS_MAX;
	tge->arg = arg;
	tge->fn = fn;
}
EXPORT_SYMBOL(ecm_db_timer_group_entry_init);

/*
 * ecm_db_timer_group_entry_touch()
 *	Update the timeout, if the timer is not running this has no effect.
 * It returns false if the timer is not running.
 */
bool ecm_db_timer_group_entry_touch(struct ecm_db_timer_group_entry *tge)
{
	struct ecm_db_timer_group *timer_group;

	spin_lock_bh(&ecm_db_lock);

	/*
	 * If not in a timer group then do nothing
	 */
	if (tge->group == ECM_DB_TIMER_GROUPS_MAX) {
		spin_unlock_bh(&ecm_db_lock);
		return false;
	}

	/*
	 * Update time to live
	 */
	timer_group = &ecm_db_timer_groups[tge->group];

	/*
	 * Link out of its current position.
	 */
	if (!tge->prev) {
		/*
		 * Already at the head, just update the time
		 */
		tge->timeout = timer_group->time + ecm_db_time;
		spin_unlock_bh(&ecm_db_lock);
		return true;
	}

	/*
	 * tge->prev is not null, so:
	 * 1) it is in a timer list
	 * 2) is not at the head of the list
	 * 3) there is a head already (so more than one item on the list)
	 * 4) there is a prev pointer.
	 * Somewhere in the group list - unlink it.
	 */
	tge->prev->next = tge->next;

	if (tge->next) {
		tge->next->prev = tge->prev;
	} else {
		/*
		 * Since there is no next this must be the tail
		 */
		DEBUG_ASSERT(timer_group->tail == tge, "%px: bad tail, expecting %px got %px\n", timer_group, tge, timer_group->tail);
		timer_group->tail = tge->prev;
	}

	/*
	 * Link in to head.
	 */
	tge->timeout = timer_group->time + ecm_db_time;
	tge->prev = NULL;
	tge->next = timer_group->head;
	timer_group->head->prev = tge;
	timer_group->head = tge;
	spin_unlock_bh(&ecm_db_lock);
	return true;
}
EXPORT_SYMBOL(ecm_db_timer_group_entry_touch);

/*
 * ecm_db_timer_groups_check()
 *	Check for expired group entries, returns the number that have expired
 */
static uint32_t ecm_db_timer_groups_check(uint32_t time_now)
{
	ecm_db_timer_group_t i;
	uint32_t expired = 0;

	DEBUG_TRACE("Timer groups check start %u\n", time_now);

	/*
	 * Examine all timer groups for expired entries.
	 */
	for (i = 0; i < ECM_DB_TIMER_GROUPS_MAX; ++i) {
		struct ecm_db_timer_group *timer_group;

		/*
		 * The group tail tracks the oldest entry so that is what we examine.
		 */
		timer_group = &ecm_db_timer_groups[i];
		spin_lock_bh(&ecm_db_lock);
		while (timer_group->tail) {
			struct ecm_db_timer_group_entry *tge;

			tge = timer_group->tail;
			if (tge->timeout > time_now) {
				/*
				 * Not expired - and no further will be as they are in order
				 */
				break;
			}

			/*
			 * Has expired - remove the entry from the list and invoke the callback
			 * NOTE: We know the entry is at the tail of the group
			 */
			if (tge->prev) {
				tge->prev->next = NULL;
			} else {
				/*
				 * First in the group
				 */
				DEBUG_ASSERT(timer_group->head == tge, "%px: bad head, expecting %px got %px\n", timer_group, tge, timer_group->head);
				timer_group->head = NULL;
			}
			timer_group->tail = tge->prev;
			tge->group = ECM_DB_TIMER_GROUPS_MAX;
			spin_unlock_bh(&ecm_db_lock);
			expired++;
			DEBUG_TRACE("%px: Expired\n", tge);
			tge->fn(tge->arg);
			spin_lock_bh(&ecm_db_lock);
		}
		spin_unlock_bh(&ecm_db_lock);
	}

	spin_lock_bh(&ecm_db_lock);
	time_now = ecm_db_time;
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Timer groups check end %u, expired count %u\n", time_now, expired);
	return expired;
}

/*
 * ecm_db_time_get()
 *	Return database time, in seconds since the database started.
 */
uint32_t ecm_db_time_get(void)
{
	uint32_t time_now;
	spin_lock_bh(&ecm_db_lock);
	time_now = ecm_db_time;
	spin_unlock_bh(&ecm_db_lock);
	return time_now;
}
EXPORT_SYMBOL(ecm_db_time_get);

/*
 * ecm_db_timer_callback()
 *	Manage expiration of connections
 * NOTE: This is softirq context
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void ecm_db_timer_callback(unsigned long data)
#else
static void ecm_db_timer_callback(struct timer_list *tm)
#endif
{
	uint32_t timer;

	/*
	 * Increment timer.
	 */
	spin_lock_bh(&ecm_db_lock);
	timer = ++ecm_db_time;
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Garbage timer tick %d\n", timer);

	/*
	 * Check timer groups
	 */
	ecm_db_timer_groups_check(timer);

	/*
	 * Set the timer for the next second
	 */
	ecm_db_timer.expires += HZ;
	if (ecm_db_timer.expires <= jiffies) {
		DEBUG_WARN("losing time %lu, jiffies = %lu\n", ecm_db_timer.expires, jiffies);
		ecm_db_timer.expires = jiffies + HZ;
	}
	add_timer(&ecm_db_timer);
}

/*
 * ecm_db_timer_init()
 */
void ecm_db_timer_init(void)
{
	DEBUG_INFO("ECM database timer init\n");

	/*
	 * Set a timer to manage cleanup of expired connections
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	init_timer(&ecm_db_timer);
	ecm_db_timer.function = ecm_db_timer_callback;
	ecm_db_timer.data = 0;
#else
	timer_setup(&ecm_db_timer, ecm_db_timer_callback, 0);
#endif
	ecm_db_timer.expires = jiffies + HZ;
	add_timer(&ecm_db_timer);

	/*
	 * Initialise timer groups with time values
	 */
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CLASSIFIER_DETERMINE_GENERIC_TIMEOUT].time = ECM_DB_CLASSIFIER_DETERMINE_GENERIC_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CLASSIFIER_DETERMINE_GENERIC_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CLASSIFIER_DETERMINE_GENERIC_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_GENERIC_TIMEOUT].time = ECM_DB_CONNECTION_GENERIC_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_GENERIC_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_GENERIC_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_IGMP_TIMEOUT].time = ECM_DB_CONNECTION_IGMP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_IGMP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_IGMP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_UDP_GENERIC_TIMEOUT].time = ECM_DB_CONNECTION_UDP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_UDP_GENERIC_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_UDP_GENERIC_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_UDP_WKP_TIMEOUT].time = ECM_DB_CONNECTION_UDP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_UDP_WKP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_UDP_WKP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ICMP_TIMEOUT].time = ECM_DB_CONNECTION_ICMP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ICMP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_ICMP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_SHORT_TIMEOUT].time = ECM_DB_CONNECTION_TCP_SHORT_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_SHORT_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_TCP_SHORT_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_RESET_TIMEOUT].time = ECM_DB_CONNECTION_TCP_RST_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_RESET_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_TCP_RESET_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_CLOSED_TIMEOUT].time = ECM_DB_CONNECTION_TCP_CLOSED_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_CLOSED_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_TCP_CLOSED_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_LONG_TIMEOUT].time = ECM_DB_CONNECTION_TCP_LONG_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_TCP_LONG_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_TCP_LONG_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_PPTP_DATA_TIMEOUT].time = ECM_DB_CONNECTION_PPTP_DATA_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_PPTP_DATA_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_PPTP_DATA_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTCP_TIMEOUT].time = ECM_DB_CONNECTION_RTCP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTCP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_RTCP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_TIMEOUT].time = ECM_DB_CONNECTION_TCP_LONG_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_FAST_TIMEOUT].time = ECM_DB_CONNECTION_RTSP_FAST_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_FAST_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_FAST_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_SLOW_TIMEOUT].time = ECM_DB_CONNECTION_RTSP_SLOW_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_SLOW_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_RTSP_SLOW_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_DNS_TIMEOUT].time = ECM_DB_CONNECTION_DNS_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_DNS_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_DNS_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_FTP_TIMEOUT].time = ECM_DB_CONNECTION_FTP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_FTP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_FTP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_BITTORRENT_TIMEOUT].time = ECM_DB_CONNECTION_BITTORRENT_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_BITTORRENT_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_BITTORRENT_TIMEOUT;

	/*
	 * H323 timeout value is 8 hours (8h * 60m * 60s == 28800 seconds).
	 */
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_H323_TIMEOUT].time = ECM_DB_CONNECTION_H323_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_H323_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_H323_TIMEOUT;

	/*
	 * IKE Timeout (seconds) = 15 hours
	 */
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_IKE_TIMEOUT].time = ECM_DB_CONNECTION_IKE_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_IKE_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_IKE_TIMEOUT;

	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ESP_TIMEOUT].time = ECM_DB_CONNECTION_ESP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ESP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_ESP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ESP_PENDING_TIMEOUT].time = ECM_DB_CONNECTION_ESP_PENDING_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_ESP_PENDING_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_ESP_PENDING_TIMEOUT;

	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_SDP_TIMEOUT].time = ECM_DB_CONNECTION_SDP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_SDP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_SDP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_SIP_TIMEOUT].time = ECM_DB_CONNECTION_SIP_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_SIP_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_SIP_TIMEOUT;

	/*
	 * Defunct re-try timeout (5 seconds)
	 */
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_DEFUNCT_RETRY_TIMEOUT].time = ECM_DB_CONNECTION_DEFUNCT_RETRY_TIMEOUT;
	ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_CONNECTION_DEFUNCT_RETRY_TIMEOUT].tg = ECM_DB_TIMER_GROUPS_CONNECTION_DEFUNCT_RETRY_TIMEOUT;
}

/*
 * ecm_db_timer_exit()
 */
void ecm_db_timer_exit(void)
{
	DEBUG_INFO("ECM database timer exit\n");

	/*
	 * Destroy garbage timer
	 * Timer must be cancelled outside of holding db lock - if the
	 * timer callback runs on another CPU we would deadlock
	 * as we would wait for the callback to finish and it would wait
	 * indefinately for the lock to be released!
	 */
	del_timer_sync(&ecm_db_timer);
}
