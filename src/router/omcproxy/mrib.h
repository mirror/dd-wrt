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

#include <netinet/in.h>
#include <libubox/list.h>
#include <sys/socket.h>

#include <netinet/icmp6.h>

#define icmp6_filter icmpv6_filter
#include <linux/igmp.h>
#include <linux/icmpv6.h>
#undef icmp6_filter

#define MRIB_DEFAULT_LIFETIME 125

#define IPV6_ALL_NODES_INIT		{ { { 0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x1 } } }
#define INADDR_ALLIGMPV3RTRS_GROUP	cpu_to_be32(0xe0000016U)

typedef uint32_t mrib_filter;
struct mrib_iface;
struct mrib_user;
struct mrib_querier;

typedef void(mrib_cb)(struct mrib_user *user, const struct in6_addr *group,
		const struct in6_addr *source, mrib_filter *filter);

typedef void(mrib_igmp_cb)(struct mrib_querier *mrib, const struct igmphdr *igmp, size_t len,
		const struct sockaddr_in *from);

typedef void(mrib_mld_cb)(struct mrib_querier *mrib, const struct mld_hdr *mld, size_t len,
		const struct sockaddr_in6 *from);

struct mrib_user {
	struct list_head head;
	struct mrib_iface *iface;
	mrib_cb *cb_newsource;
};

struct mrib_querier {
	struct list_head head;
	struct mrib_iface *iface;
	mrib_igmp_cb *cb_igmp;
	mrib_mld_cb *cb_mld;
};

// Register a new user to mrib
int mrib_attach_user(struct mrib_user *user, int ifindex, mrib_cb *cb_newsource);

// Deregister a user from mrib
void mrib_detach_user(struct mrib_user *user);

// Register a querier to mrib
int mrib_attach_querier(struct mrib_querier *querier, int ifindex, mrib_igmp_cb *cb_igmp, mrib_mld_cb *cb_mld);

// Deregister a querier from mrib
void mrib_detach_querier(struct mrib_querier *querier);

// Flush state for a multicast route
int mrib_flush(struct mrib_user *user, const struct in6_addr *group, uint8_t group_plen, const struct in6_addr *source);

// Add interface to filter
int mrib_filter_add(mrib_filter *filter, struct mrib_user *user);

// Send IGMP-packet
int mrib_send_igmp(struct mrib_querier *querier, struct igmpv3_query *igmp, size_t len,
		const struct sockaddr_in *dest);

// Send MLD-packet
int mrib_send_mld(struct mrib_querier *querier, struct mld_hdr *mld, size_t len,
		const struct sockaddr_in6 *dest);

// Get source address
int mrib_mld_source(struct mrib_querier *q, struct in6_addr *source);
int mrib_igmp_source(struct mrib_querier *q, struct in_addr *source);
