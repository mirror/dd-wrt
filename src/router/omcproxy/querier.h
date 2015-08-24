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
#include <libubox/uloop.h>
#include <libubox/avl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <stdbool.h>

#include "mrib.h"
#include "groups.h"

struct querier_iface {
	struct list_head head;
	struct list_head users;
	struct uloop_timeout timeout;
	struct groups_config cfg;

	struct uloop_fd igmp_fd;
	omgp_time_t igmp_next_query;
	bool igmp_other_querier;
	int igmp_startup_tries;

	struct uloop_fd mld_fd;
	omgp_time_t mld_next_query;
	bool mld_other_querier;
	int mld_startup_tries;

	struct mrib_querier mrib;
	struct groups groups;
	int ifindex;
};

struct querier;
struct querier_user;
struct querier_user_iface;

typedef void (querier_iface_cb)(struct querier_user_iface *user, const struct in6_addr *group,
		bool include, const struct in6_addr *sources, size_t len);

struct querier_user {
	struct list_head head;
	struct groups *groups;
	struct querier *querier;
};

struct querier_user_iface {
	struct list_head head;
	struct querier_user user;
	struct querier_iface *iface;
	querier_iface_cb *user_cb;
};


/* External API */
int querier_init(struct querier *querier);
void querier_deinit(struct querier *querier);

int querier_attach(struct querier_user_iface *user, struct querier *querier,
		int ifindex, querier_iface_cb *cb);
void querier_detach(struct querier_user_iface *user);


/* Internal API */

struct querier {
	struct list_head ifaces;
};

#define QUERIER_MAX_SOURCE 75
#define QUERIER_MAX_GROUPS 256
#define QUERIER_SUPPRESS (1 << 3)

static inline in_addr_t querier_unmap(const struct in6_addr *addr6)
{
	return addr6->s6_addr32[3];
}

static inline void querier_map(struct in6_addr *addr6, in_addr_t addr4)
{
	addr6->s6_addr32[0] = 0;
	addr6->s6_addr32[1] = 0;
	addr6->s6_addr32[2] = cpu_to_be32(0xffff);
	addr6->s6_addr32[3] = addr4;
}

void querier_announce(struct querier_user *user, omgp_time_t now, const struct group *group, bool enabled);
void querier_synthesize_events(struct querier *querier);

int querier_qqi(uint8_t qqic);
int querier_mrd(uint16_t mrc);
uint8_t querier_qqic(int qi);
uint16_t querier_mrc(int mrd);


void igmp_handle(struct mrib_querier *mrib, const struct igmphdr *igmp, size_t len,
		const struct sockaddr_in *from);
int igmp_send_query(struct querier_iface *q,
		const struct in6_addr *group,
		const struct list_head *sources,
		bool suppress);


void mld_handle(struct mrib_querier *mrib, const struct mld_hdr *hdr, size_t len,
		const struct sockaddr_in6 *from);
ssize_t mld_send_query(struct querier_iface *q,
		const struct in6_addr *group,
		const struct list_head *sources,
		bool suppress);

