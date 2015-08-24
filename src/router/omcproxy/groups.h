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

#pragma once

#include <libubox/list.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include <arpa/inet.h>
#include "omcproxy.h"

struct group {
	struct avl_node node;
	struct in6_addr addr;
	struct list_head sources;
	size_t source_count;
	omgp_time_t exclude_until;
	omgp_time_t compat_v2_until;
	omgp_time_t compat_v1_until;
	omgp_time_t next_generic_transmit;
	omgp_time_t next_source_transmit;
	int retransmit;
};

struct group_source {
	struct list_head head;
	struct in6_addr addr;
	omgp_time_t include_until;
	int retransmit;
};

struct groups_config {
	omgp_time_t query_response_interval;
	omgp_time_t query_interval;
	omgp_time_t last_listener_query_interval;
	int robustness;
	int last_listener_query_count;
};

struct groups {
	struct groups_config cfg_v4;
	struct groups_config cfg_v6;
	struct avl_tree groups;
	struct uloop_timeout timer;
	size_t source_limit;
	size_t group_limit;
	void (*cb_query)(struct groups *g, const struct in6_addr *addr,
			const struct list_head *sources, bool suppress);
	void (*cb_update)(struct groups *g, struct group *group, omgp_time_t now);
};


void groups_init(struct groups *groups);
void groups_deinit(struct groups *groups);


enum groups_update {
	UPDATE_UNSPEC,
	UPDATE_IS_INCLUDE	= 1,
	UPDATE_IS_EXCLUDE	= 2,
	UPDATE_TO_IN		= 3,
	UPDATE_TO_EX		= 4,
	UPDATE_ALLOW		= 5,
	UPDATE_BLOCK		= 6,
	UPDATE_REPORT		= 7,
	UPDATE_REPORT_V1	= 8,
	UPDATE_DONE			= 9,
	UPDATE_SET_IN		= 0x11,
	UPDATE_SET_EX		= 0x12,
};

void groups_update_config(struct groups *groups, bool v6,
		omgp_time_t query_response_interval, omgp_time_t query_interval, int robustness);

void groups_update_timers(struct groups *groups,
		const struct in6_addr *groupaddr,
		const struct in6_addr *addrs, size_t len);

void groups_update_state(struct groups *groups,
		const struct in6_addr *groupaddr,
		const struct in6_addr *addrs, size_t len,
		enum groups_update update);

void groups_synthesize_events(struct groups *groups);

// Groups user query API

static inline bool group_is_included(const struct group *group, omgp_time_t time)
{
	return group->exclude_until <= time;
}

static inline bool source_is_included(const struct group_source *source, omgp_time_t time)
{
	return source->include_until > time;
}

#define groups_for_each_group(group, groupsp) \
	avl_for_each_element(&(groupsp)->groups, group, node)

#define group_for_each_source(source, group) \
	list_for_each_entry(source, &(group)->sources, head)

#define group_for_each_active_source(source, group, time) \
	list_for_each_entry(source, &group->sources, head) \
		if (source_is_included(source, time) == group_is_included(group, time))

const struct group* groups_get(struct groups *groups, const struct in6_addr *addr);
bool groups_includes_group(struct groups *groups, const struct in6_addr *addr,
		const struct in6_addr *src, omgp_time_t time);
