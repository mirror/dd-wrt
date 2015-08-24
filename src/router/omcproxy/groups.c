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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "groups.h"


// Group comparator for AVL-tree
static int compare_groups(const void *k1, const void *k2, __unused void *ptr)
{
	return memcmp(k1, k2, sizeof(struct in6_addr));
}

// Remove a source-definition for a group
static void querier_remove_source(struct group *group, struct group_source *source)
{
	--group->source_count;
	list_del(&source->head);
	free(source);
}

// Clear all sources of a certain group
static void querier_clear_sources(struct group *group)
{
	struct group_source *s, *n;
	list_for_each_entry_safe(s, n, &group->sources, head)
		querier_remove_source(group, s);
}

// Remove a group and all associated sources from the group state
static void querier_remove_group(struct groups *groups, struct group *group, omgp_time_t now)
{
	querier_clear_sources(group);
	group->exclude_until = 0;

	if (groups->cb_update)
		groups->cb_update(groups, group, now);

	avl_delete(&groups->groups, &group->node);
	free(group);
}

// Expire a group and / or its associated sources depending on the current time
static omgp_time_t expire_group(struct groups *groups, struct group *group,
		omgp_time_t now, omgp_time_t next_event)
{
	struct groups_config *cfg = IN6_IS_ADDR_V4MAPPED(&group->addr) ? &groups->cfg_v4 : &groups->cfg_v6;
	omgp_time_t llqi = now + cfg->last_listener_query_interval;
	omgp_time_t llqt = now + (cfg->last_listener_query_interval * cfg->last_listener_query_count);

	// Handle group and source-specific query retransmission
	struct list_head suppressed = LIST_HEAD_INIT(suppressed);
	struct list_head unsuppressed = LIST_HEAD_INIT(unsuppressed);
	struct group_source *s, *s2;

	if (group->next_source_transmit > 0 && group->next_source_transmit <= now) {
		group->next_source_transmit = 0;

		list_for_each_entry_safe(s, s2, &group->sources, head) {
			if (s->retransmit > 0) {
				list_move_tail(&s->head, (s->include_until > llqt) ? &suppressed : &unsuppressed);
				--s->retransmit;
			}

			if (s->retransmit > 0)
				group->next_source_transmit = llqi;
		}
	}

	if (group->next_source_transmit > 0 && group->next_source_transmit < next_event)
		next_event = group->next_source_transmit;

	// Handle group-specific query retransmission
	if (group->retransmit > 0 && group->next_generic_transmit <= now) {
		group->next_generic_transmit = 0;

		if (groups->cb_query)
			groups->cb_query(groups, &group->addr, NULL, group->exclude_until > llqt);

		--group->retransmit;

		if (group->retransmit > 0)
			group->next_generic_transmit = llqi;

		// Skip suppresed source-specific query (RFC 3810 7.6.3.2)
		list_splice_init(&suppressed, &group->sources);
	}

	if (group->next_generic_transmit > 0 && group->next_generic_transmit < next_event)
		next_event = group->next_generic_transmit;

	if (!list_empty(&suppressed)) {
		if (groups->cb_query)
				groups->cb_query(groups, &group->addr, &suppressed, true);

		list_splice(&suppressed, &group->sources);
	}

	if (!list_empty(&unsuppressed)) {
		if (groups->cb_query)
				groups->cb_query(groups, &group->addr, &unsuppressed, false);

		list_splice(&unsuppressed, &group->sources);
	}

	// Handle source and group expiry
	bool changed = false;
	if (group->exclude_until > 0) {
		if (group_is_included(group, now)) {
			// Leaving exclude mode
			group->exclude_until = 0;
			changed = true;
		} else if (group->exclude_until < next_event) {
			next_event = group->exclude_until;
		}
	}

	list_for_each_entry_safe(s, s2, &group->sources, head) {
		if (s->include_until > 0) {
			if (!source_is_included(s, now)) {
				s->include_until = 0;
				changed = true;
			} else if (s->include_until < next_event) {
				next_event = s->include_until;
			}
		}

		if (group->exclude_until == 0 && s->include_until == 0)
			querier_remove_source(group, s);
	}

	if (group->exclude_until == 0 && group->source_count == 0)
		querier_remove_group(groups, group, now);
	else if (changed && groups->cb_update)
		groups->cb_update(groups, group, now);

	return next_event;
}

// Rearm the global groups-timer if the next event is before timer expiration
static void rearm_timer(struct groups *groups, int msecs)
{
	int remain = uloop_timeout_remaining(&groups->timer);
	if (remain < 0 || remain >= msecs)
		uloop_timeout_set(&groups->timer, msecs);
}

// Expire all groups of a group-state (called by timer as callback)
static void expire_groups(struct uloop_timeout *t)
{
	struct groups *groups = container_of(t, struct groups, timer);
	omgp_time_t now = omgp_time();
	omgp_time_t next_event = now + 3600 * OMGP_TIME_PER_SECOND;

	struct group *group, *n;
	avl_for_each_element_safe(&groups->groups, group, node, n)
		next_event = expire_group(groups, group, now, next_event);

	rearm_timer(groups, (next_event > now) ? next_event - now : 0);
}

// Initialize a group-state
void groups_init(struct groups *groups)
{
	avl_init(&groups->groups, compare_groups, false, NULL);
	groups->timer.cb = expire_groups;

	groups_update_config(groups, false, 10 * OMGP_TIME_PER_SECOND,
			125 * OMGP_TIME_PER_SECOND, 2);
	groups_update_config(groups, true, 10 * OMGP_TIME_PER_SECOND,
				125 * OMGP_TIME_PER_SECOND, 2);
}

// Cleanup a group-state
void groups_deinit(struct groups *groups)
{
	omgp_time_t now = omgp_time();
	struct group *group, *safe;
	avl_for_each_element_safe(&groups->groups, group, node, safe)
		querier_remove_group(groups, group, now);
	uloop_timeout_cancel(&groups->timer);
}

// Get group-object for a given group, create if requested
static struct group* groups_get_group(struct groups *groups,
		const struct in6_addr *addr, bool *created)
{
	struct group *group = avl_find_element(&groups->groups, addr, group, node);
	if (!group && created && (group = calloc(1, sizeof(*group)))) {
		group->addr = *addr;
		group->node.key = &group->addr;
		avl_insert(&groups->groups, &group->node);

		INIT_LIST_HEAD(&group->sources);
		*created = true;
	} else if (created) {
		*created = false;
	}
	return group;
}

// Get source-object for a given source, create if requested
static struct group_source* groups_get_source(struct groups *groups,
		struct group *group, const struct in6_addr *addr, bool *created)
{
	struct group_source *c, *source = NULL;
	group_for_each_source(c, group)
		if (IN6_ARE_ADDR_EQUAL(&c->addr, addr))
			source = c;

	if (!source && created && group->source_count < groups->source_limit &&
			(source = calloc(1, sizeof(*source)))) {
		source->addr = *addr;
		list_add_tail(&source->head, &group->sources);
		++group->source_count;
		*created = true;
	} else if (created) {
		*created = false;
	}

	return source;
}

// Update the IGMP/MLD timers of a group-state
void groups_update_config(struct groups *groups, bool v6,
		omgp_time_t query_response_interval, omgp_time_t query_interval, int robustness)
{
	struct groups_config *cfg = v6 ? &groups->cfg_v6 : &groups->cfg_v4;
	cfg->query_response_interval = query_response_interval;
	cfg->query_interval = query_interval;
	cfg->robustness = robustness;
	cfg->last_listener_query_count = cfg->robustness;
	cfg->last_listener_query_interval = 1 * OMGP_TIME_PER_SECOND;
}

// Update timers for a given group (called when receiving queries from other queriers)
void groups_update_timers(struct groups *groups,
		const struct in6_addr *groupaddr,
		const struct in6_addr *addrs, size_t len)
{
	char addrbuf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, groupaddr, addrbuf, sizeof(addrbuf));
	struct group *group = groups_get_group(groups, groupaddr, NULL);
	if (!group) {
		L_WARN("%s: failed to update timer: no such group %s", __FUNCTION__, addrbuf);
		return;
	}

	struct groups_config *cfg = IN6_IS_ADDR_V4MAPPED(&group->addr) ? &groups->cfg_v4 : &groups->cfg_v6;
	omgp_time_t now = omgp_time();
	omgp_time_t llqt = now + (cfg->last_listener_query_count * cfg->last_listener_query_interval);

	if (len == 0) {
		if (group->exclude_until > llqt)
			group->exclude_until = llqt;
	} else {
		for (size_t i = 0; i < len; ++i) {
			struct group_source *source = groups_get_source(groups, group, &addrs[i], NULL);
			if (!source) {
				L_WARN("%s: failed to update timer: unknown sources for group %s", __FUNCTION__, addrbuf);
				continue;
			}

			if (source->include_until > llqt)
				source->include_until = llqt;
		}
	}

	rearm_timer(groups, llqt - now);
}

// Update state of a given group (on reception of node's IGMP/MLD packets)
void groups_update_state(struct groups *groups,
		const struct in6_addr *groupaddr,
		const struct in6_addr *addrs, size_t len,
		enum groups_update update)
{
	bool created = false, changed = false;
	char addrbuf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, groupaddr, addrbuf, sizeof(addrbuf));
	L_DEBUG("%s: %s (+%d sources) => %d", __FUNCTION__, addrbuf, (int)len, update);

	struct group *group = groups_get_group(groups, groupaddr, &created);
	if (!group) {
		L_ERR("querier_state: failed to allocate group for %s", addrbuf);
		return;
	}

	if (created)
		changed = true;

	omgp_time_t now = omgp_time();
	omgp_time_t next_event = OMGP_TIME_MAX;
	struct groups_config *cfg = IN6_IS_ADDR_V4MAPPED(&group->addr) ? &groups->cfg_v4 : &groups->cfg_v6;

	// Backwards compatibility modes
	if (group->compat_v2_until > now || group->compat_v1_until > now) {
		if (update == UPDATE_BLOCK)
			return;

		if (group->compat_v1_until > now && (update == UPDATE_DONE || update == UPDATE_TO_IN))
			return;

		if (update == UPDATE_TO_EX)
			len = 0;
	}

	if (update == UPDATE_REPORT || update == UPDATE_REPORT_V1 || update == UPDATE_DONE) {
		omgp_time_t compat_until = now + cfg->query_response_interval +
				(cfg->robustness * cfg->query_interval);

		if (update == UPDATE_REPORT_V1)
			group->compat_v1_until = compat_until;
		else if (update == UPDATE_REPORT)
			group->compat_v2_until = compat_until;

		update = (update == UPDATE_DONE) ? UPDATE_TO_IN : UPDATE_IS_EXCLUDE;
		len = 0;
	}

	bool include = group->exclude_until <= now;
	bool is_include = update == UPDATE_IS_INCLUDE || update == UPDATE_TO_IN || update == UPDATE_ALLOW;

	int llqc = cfg->last_listener_query_count;
	omgp_time_t mali = now + (cfg->robustness * cfg->query_interval) + cfg->query_response_interval;
	omgp_time_t llqt = now + (cfg->last_listener_query_interval * llqc);

	// RFC 3810 7.4
	struct list_head saved = LIST_HEAD_INIT(saved);
	struct list_head queried = LIST_HEAD_INIT(queried);
	for (size_t i = 0; i < len; ++i) {
		bool *create = (include && update == UPDATE_BLOCK) ? NULL : &created;
		struct group_source *source = groups_get_source(groups, group, &addrs[i], create);

		if (include && update == UPDATE_BLOCK) {
			if (source)
				list_move_tail(&source->head, &queried);
		} else {
			bool query = false;
			if (!source) {
				groups_update_state(groups, groupaddr, NULL, 0, false);
				L_WARN("querier: failed to allocate source for %s, fallback to ASM", addrbuf);
				return;
			}

			if (created)
				changed = true;
			else if (include && update == UPDATE_TO_EX)
				query = true;

			if (source->include_until <= now && update == UPDATE_SET_IN) {
				source->include_until = mali;
				changed = true;
			} else if (source->include_until > now && update == UPDATE_SET_EX) {
				source->include_until = now;
				changed = true;
			}

			if (!include && (update == UPDATE_BLOCK || update == UPDATE_TO_EX) &&
					(created || source->include_until > now))
				query = true;

			if ((is_include || (!include && created))) {
				if (source->include_until <= now)
					changed = true;

				source->include_until = (is_include || update == UPDATE_IS_EXCLUDE)
						? mali : group->exclude_until;

				if (next_event > mali)
					next_event = mali;
			}

			if (query)
				list_move_tail(&source->head, &queried);
			else if (update == UPDATE_IS_EXCLUDE || update == UPDATE_TO_EX ||
					update == UPDATE_SET_EX || update == UPDATE_SET_IN)
				list_move_tail(&source->head, &saved);
		}
	}

	if (update == UPDATE_IS_EXCLUDE || update == UPDATE_TO_EX || update == UPDATE_SET_EX) {
		if (include || !list_empty(&group->sources))
			changed = true;

		querier_clear_sources(group);
		list_splice(&saved, &group->sources);
		group->exclude_until = mali;

		if (next_event > mali)
			next_event = mali;
	}

	if (update == UPDATE_SET_IN) {
		if (!include || !list_empty(&group->sources)) {
			changed = true;
			next_event = now;
		}

		querier_clear_sources(group);
		list_splice(&saved, &group->sources);
		group->exclude_until = now;
	}

	// Prepare queries
	if (update == UPDATE_TO_IN) {
		struct group_source *source, *n;
		list_for_each_entry_safe(source, n, &group->sources, head) {
			if (source->include_until <= now)
				continue;

			size_t i;
			for (i = 0; i < len && !IN6_ARE_ADDR_EQUAL(&source->addr, &addrs[i]); ++i);
			if (i == len)
				list_move_tail(&source->head, &queried);
		}
	}

	if (!list_empty(&queried)) {
		struct group_source *source;
		list_for_each_entry(source, &queried, head) {
			if (source->include_until > llqt)
				source->include_until = llqt;

			group->next_source_transmit = now;
			source->retransmit = llqc;
		}

		next_event = now;
		list_splice(&queried, &group->sources);
	}

	if (!include && update == UPDATE_TO_IN) {
		if (group->exclude_until > llqt)
			group->exclude_until = llqt;

		group->next_generic_transmit = now;
		group->retransmit = llqc;
		next_event = now;
	}

	if (changed && groups->cb_update)
		groups->cb_update(groups, group, now);

	if (group_is_included(group, now) && group->source_count == 0)
		next_event = now;

	if (next_event < OMGP_TIME_MAX)
		rearm_timer(groups, next_event - now);

	if (changed)
		L_DEBUG("%s: %s => %s (+%d sources)", __FUNCTION__, addrbuf,
				(group_is_included(group, now)) ? "included" : "excluded",
				(int)group->source_count);

}

// Get group object of a given group
const struct group* groups_get(struct groups *groups, const struct in6_addr *addr)
{
	return groups_get_group(groups, addr, NULL);
}

// Test if a group (and source) is requested in the current group state
// (i.e. for deciding if it should be routed / forwarded)
bool groups_includes_group(struct groups *groups, const struct in6_addr *addr,
		const struct in6_addr *src, omgp_time_t time)
{
	struct group *group = groups_get_group(groups, addr, NULL);
	if (group) {
		if (!src && (!group_is_included(group, time) || group->source_count > 0))
			return true;

		struct group_source *source = groups_get_source(groups, group, src, NULL);
		if ((!group_is_included(group, time) && (!source || source_is_included(source, time))) ||
				(group_is_included(group, time) && source && source_is_included(source, time)))
			return true;
	}
	return false;
}
