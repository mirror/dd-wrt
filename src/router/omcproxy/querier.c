/*
 * Author: Steven Barth <steven at midlink.org>
 *
 * Copyright 2015 Deutsche Telekom AG
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <libubox/ustream.h>
#include <libubox/usock.h>
#include <libubox/list.h>

#include "querier.h"

static struct list_head ifaces = LIST_HEAD_INIT(ifaces);


// Handle querier update event from a querier-interface
static void querier_announce_iface(struct querier_user_iface *user, omgp_time_t now, const struct group *group, bool enabled)
{
	bool include = true;
	size_t cnt = 0;
	struct in6_addr sources[group->source_count];

	if (enabled) {
		struct group_source *source;
		group_for_each_active_source(source, group, now)
			sources[cnt++] = source->addr;

		include = group_is_included(group, now);
	}

	if (user->user_cb)
		user->user_cb(user, &group->addr, include, sources, cnt);
}

// Handle changes from a querier for a given group (called by a group-state as callback)
static void querier_announce_change(struct groups *groups, struct group *group, omgp_time_t now)
{
	struct querier_iface *iface = container_of(groups, struct querier_iface, groups);

	// Only recognize changes to non-link-local groups
	struct querier_user_iface *user;
	list_for_each_entry(user, &iface->users, head)
		querier_announce_iface(user, now, group, true);
}

// Send query for a group + sources (called by a group-state as callback)
static void querier_send_query(struct groups *groups, const struct in6_addr *group,
		const struct list_head *sources, bool suppress)
{
	struct querier_iface *iface = container_of(groups, struct querier_iface, groups);
	char addrbuf[INET6_ADDRSTRLEN] = "::";
	inet_ntop(AF_INET6, group, addrbuf, sizeof(addrbuf));

	L_DEBUG("%s: sending %s-specific query for %s on %d (S: %d)", __FUNCTION__,
			(!sources) ? "group" : "source", addrbuf, iface->ifindex, suppress);

	bool v4 = IN6_IS_ADDR_V4MAPPED(group);
	if (v4 && !iface->igmp_other_querier)
		igmp_send_query(iface, group, sources, suppress);
	else if (!v4 && !iface->mld_other_querier)
		mld_send_query(iface, group, sources, suppress);
}

// Expire interface timers and send queries (called by timer as callback)
static void querier_iface_timer(struct uloop_timeout *timeout)
{
	struct querier_iface *iface = container_of(timeout, struct querier_iface, timeout);
	omgp_time_t now = omgp_time();
	omgp_time_t next_event = now + 3600 * OMGP_TIME_PER_SECOND;

	if (iface->igmp_next_query <= now) {
		// If the other querier is gone, reset interface config
		if (iface->igmp_other_querier) {
			iface->groups.cfg_v4 = iface->cfg;
			iface->igmp_other_querier = false;
		}

		igmp_send_query(iface, NULL, NULL, false);
		L_DEBUG("%s: sending generic IGMP-query on %d (S: 0)", __FUNCTION__, iface->ifindex);

		if (iface->igmp_startup_tries > 0)
			--iface->igmp_startup_tries;

		iface->igmp_next_query = now + ((iface->igmp_startup_tries > 0) ?
						(iface->groups.cfg_v4.query_interval / 4) :
						iface->groups.cfg_v4.query_interval);
	}

	if (iface->igmp_next_query < next_event)
		next_event = iface->igmp_next_query;

	if (iface->mld_next_query <= now) {
		// If the other querier is gone, reset interface config
		if (iface->mld_other_querier) {
			iface->groups.cfg_v6 = iface->cfg;
			iface->mld_other_querier = false;
		}

		mld_send_query(iface, NULL, NULL, false);
		L_DEBUG("%s: sending generic MLD-query on %d (S: 0)", __FUNCTION__, iface->ifindex);

		if (iface->mld_startup_tries > 0)
			--iface->mld_startup_tries;

		iface->mld_next_query = now + ((iface->mld_startup_tries > 0) ?
						(iface->groups.cfg_v6.query_interval / 4) :
						iface->groups.cfg_v6.query_interval);
	}

	if (iface->mld_next_query < next_event)
		next_event = iface->mld_next_query;

	uloop_timeout_set(&iface->timeout, (next_event > now) ? next_event - now : 0);
}


// Calculate QQI from QQIC
int querier_qqi(uint8_t qqic)
{
	return (qqic & 0x80) ? (((qqic & 0xf) | 0x10) << (((qqic >> 4) & 0x7) + 3)) : qqic;
}

// Calculate MRD from MRC
int querier_mrd(uint16_t mrc)
{
	mrc = ntohs(mrc);
	return (mrc & 0x8000) ? (((mrc & 0xfff) | 0x1000) << (((mrc >> 12) & 0x7) + 3)) : mrc;
}

// Calculate QQIC from QQI
uint8_t querier_qqic(int qqi)
{
	if (qqi >= 128) {
		int exp = 3;

		while ((qqi >> exp) > 0x1f && exp <= 10)
			++exp;

		if (exp > 10)
			qqi = 0xff;
		else
			qqi = 0x80 | ((exp - 3) << 4) | ((qqi >> exp) & 0xf);
	}
	return qqi;
}

// Calculate MRC from MRD
uint16_t querier_mrc(int mrd)
{
	if (mrd >= 32768) {
		int exp = 3;

		while ((mrd >> exp) > 0x1fff && exp <= 10)
			++exp;

		if (exp > 10)
			mrd = 0xffff;
		else
			mrd = 0x8000 | ((exp - 3) << 12) | ((mrd >> exp) & 0xfff);
	}
	return htons(mrd);
}

// Attach an interface to a querier-instance
int querier_attach(struct querier_user_iface *user,
		struct querier *querier, int ifindex, querier_iface_cb *cb)
{
	struct querier_iface *c, *iface = NULL;
	list_for_each_entry(c, &ifaces, head) {
		if (c->ifindex == ifindex) {
			iface = c;
			break;
		}
	}

	omgp_time_t now = omgp_time();
	int res = 0;
	if (!iface) {
		if (!(iface = calloc(1, sizeof(*iface)))) {
			res = -errno;
			goto out;
		}

		list_add(&iface->head, &ifaces);
		INIT_LIST_HEAD(&iface->users);

		iface->ifindex = ifindex;
		iface->timeout.cb = querier_iface_timer;
		uloop_timeout_set(&iface->timeout, 0);

		groups_init(&iface->groups);
		iface->groups.source_limit = QUERIER_MAX_SOURCE;
		iface->groups.group_limit = QUERIER_MAX_GROUPS;
		iface->groups.cb_update = querier_announce_change;
		iface->groups.cb_query = querier_send_query;
		iface->cfg = iface->groups.cfg_v6;
		iface->igmp_startup_tries = iface->groups.cfg_v4.robustness;
		iface->mld_startup_tries = iface->groups.cfg_v6.robustness;

		if ((res = mrib_attach_querier(&iface->mrib, ifindex, igmp_handle, mld_handle)))
			goto out;
	}

out:
	if (iface) {
		list_add(&user->head, &iface->users);
		user->iface = iface;

		list_add(&user->user.head, &querier->ifaces);
		user->user_cb = cb;
		user->user.querier = querier;
		user->user.groups = &iface->groups;

		struct group *group;
		groups_for_each_group(group, &iface->groups)
			querier_announce_iface(user, now, group, true);
	}

	if (res)
		querier_detach(user);
	return res;
}

// Detach an interface from a querier-instance
void querier_detach(struct querier_user_iface *user)
{
	struct querier_iface *iface = user->iface;
	list_del(&user->user.head);
	list_del(&user->head);

	omgp_time_t now = omgp_time();
	struct group *group;
	groups_for_each_group(group, &iface->groups)
		querier_announce_iface(user, now, group, false);

	if (list_empty(&iface->users)) {
		uloop_timeout_cancel(&iface->timeout);
		groups_deinit(&iface->groups);
		mrib_detach_querier(&iface->mrib);
		list_del(&iface->head);
		free(iface);
	}
}

// Initialize querier-instance
int querier_init(struct querier *querier)
{
	memset(querier, 0, sizeof(*querier));
	INIT_LIST_HEAD(&querier->ifaces);
	return 0;
}

// Cleanup querier-instance
void querier_deinit(struct querier *querier)
{
	struct querier_user *user, *n;
	list_for_each_entry_safe(user, n, &querier->ifaces, head)
		querier_detach(container_of(user, struct querier_user_iface, user));
}
