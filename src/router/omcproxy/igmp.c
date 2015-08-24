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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>

#include "querier.h"

// Test if multicast-group is valid and relevant
static inline bool igmp_is_valid_group(in_addr_t group)
{
	return IN_MULTICAST(be32_to_cpu(group));
}

// Handle an IGMP-record from an IGMP-packet (called by igmp_receive)
static ssize_t igmp_handle_record(struct groups *groups, const uint8_t *data, size_t len)
{
	struct igmpv3_grec *r = (struct igmpv3_grec*)data;

	if (len < sizeof(*r))
		return -1;

	size_t nsrc = ntohs(r->grec_nsrcs);
	size_t read = sizeof(*r) + nsrc * sizeof(struct in_addr) + r->grec_auxwords * 4;

	if (len < read)
		return -1;

	if (r->grec_type >= UPDATE_IS_INCLUDE && r->grec_type <= UPDATE_BLOCK &&
			igmp_is_valid_group(r->grec_mca)) {
		struct in6_addr addr, sources[nsrc];
		querier_map(&addr, r->grec_mca);

		for (size_t i = 0; i < nsrc; ++i)
			querier_map(&sources[i], r->grec_src[i]);

		groups_update_state(groups, &addr, sources, nsrc, r->grec_type);
	}

	return read;
}

// Receive and parse an IGMP-packet (called by uloop as callback)
void igmp_handle(struct mrib_querier *mrib, const struct igmphdr *igmp, size_t len,
		const struct sockaddr_in *from)
{
	struct querier_iface *q = container_of(mrib, struct querier_iface, mrib);
	omgp_time_t now = omgp_time();
	char addrbuf[INET_ADDRSTRLEN];
	struct in6_addr group;

	querier_map(&group, igmp->group);
	inet_ntop(AF_INET, &from->sin_addr, addrbuf, sizeof(addrbuf));

	if (igmp->type == IGMP_HOST_MEMBERSHIP_QUERY) {
		struct igmpv3_query *query = (struct igmpv3_query*)igmp;

		if (len != sizeof(*igmp) && ((size_t)len < sizeof(*query) ||
				(size_t)len < sizeof(*query) + ntohs(query->nsrcs) * sizeof(struct in_addr)))
			return;

		if (query->group && !igmp_is_valid_group(query->group))
			return;

		// Setup query target address
		struct in_addr addr;
		if (mrib_igmp_source(mrib, &addr))
			return;

		bool suppress = false;
		size_t nsrc = 0;
		int robustness = 2;
		omgp_time_t mrd = 10000;
		omgp_time_t query_interval = 125000;

		if (igmp->code)
			mrd = 100 * ((len == sizeof(*igmp)) ? igmp->code : querier_qqi(igmp->code));

		if ((size_t)len > sizeof(*igmp)) {
			if (query->qrv)
				robustness = query->qrv;

			if (query->qqic)
				query_interval = querier_qqi(query->qqic) * 1000;

			suppress = query->suppress;
			nsrc = ntohs(query->nsrcs);
		}

		if (!suppress && query->group) {
			struct in6_addr sources[nsrc];
			for (size_t i = 0; i < nsrc; ++i)
				querier_map(&sources[i], query->srcs[i]);

			groups_update_timers(&q->groups, &group, sources, nsrc);
		}

		int election = memcmp(&from->sin_addr, &addr, sizeof(from->sin_addr));
		L_INFO("%s: detected other querier %s with priority %d on %d",
				__FUNCTION__, addrbuf, election, q->ifindex);

		// TODO: we ignore IGMPv1/v2 queriers for now, since a lot of them are dumb switches

		if (election < 0 && !query->group && len > sizeof(*igmp)) {
			groups_update_config(&q->groups, false, mrd, query_interval, robustness);

			q->igmp_other_querier = true;
			q->igmp_next_query = now + (q->groups.cfg_v4.query_response_interval / 2) +
				(q->groups.cfg_v4.robustness * q->groups.cfg_v4.query_interval);
		}
	} else if (igmp->type == IGMPV3_HOST_MEMBERSHIP_REPORT) {
		struct igmpv3_report *report = (struct igmpv3_report*)igmp;

		if ((size_t)len <= sizeof(*report))
			return;

		uint8_t *ibuf = (uint8_t*)igmp;
		size_t count = ntohs(report->ngrec);
		size_t offset = sizeof(*report);

		while (count > 0 && offset < len) {
			ssize_t read = igmp_handle_record(&q->groups, &ibuf[offset], len - offset);
			if (read < 0)
				break;

			offset += read;
			--count;
		}
	} else if (igmp->type == IGMPV2_HOST_MEMBERSHIP_REPORT ||
			igmp->type == IGMP_HOST_LEAVE_MESSAGE ||
			igmp->type == IGMP_HOST_MEMBERSHIP_REPORT) {

		if (len != sizeof(*igmp) || !igmp_is_valid_group(igmp->group))
			return;

		groups_update_state(&q->groups, &group, NULL, 0,
				(igmp->type == IGMPV2_HOST_MEMBERSHIP_REPORT) ? UPDATE_REPORT :
				(igmp->type == IGMP_HOST_MEMBERSHIP_REPORT) ? UPDATE_REPORT_V1 : UPDATE_DONE);
	}

	uloop_timeout_set(&q->timeout, 0);
}

// Send generic / group-specific / group-and-source specific IGMP-query
int igmp_send_query(struct querier_iface *q,
		const struct in6_addr *group,
		const struct list_head *sources,
		bool suppress)
{
	uint8_t qqic = querier_qqic(((group) ? q->groups.cfg_v4.last_listener_query_interval :
			q->groups.cfg_v4.query_response_interval) / 100);
	struct {
		struct igmpv3_query q;
		struct in_addr srcs[QUERIER_MAX_SOURCE];
	} query = {.q = {
		.type = IGMP_HOST_MEMBERSHIP_QUERY,
		.code = qqic,
		.qrv = q->groups.cfg_v4.robustness,
		.suppress = suppress,
		.qqic = querier_qqic(q->groups.cfg_v4.query_interval / 1000),
	}};

	struct group_source *s;
	size_t cnt = 0;
	if (sources) {
		list_for_each_entry(s, sources, head) {
			if (cnt >= QUERIER_MAX_SOURCE) {
				L_WARN("%s: maximum source count (%d) exceeded",
						__FUNCTION__, QUERIER_MAX_SOURCE);
				break;
			}

			query.q.srcs[cnt] = querier_unmap(&s->addr);
		}
	}
	query.q.nsrcs = htons(cnt);

	struct sockaddr_in dest = { .sin_family = AF_INET, .sin_addr = {htonl(0xe0000001U)}};
	if (group) {
		query.q.group = querier_unmap(group);
		dest.sin_addr.s_addr = query.q.group;
	}

	return mrib_send_igmp(&q->mrib, &query.q,
			sizeof(query.q) + cnt * sizeof(query.srcs[0]), &dest);
}

