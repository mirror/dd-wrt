/*
 * Copyright 2015 Steven Barth <steven at midlink.org>
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
#include <libubox/list.h>

#include "querier.h"
#include "client.h"
#include "mrib.h"
#include "proxy.h"

struct proxy {
	struct list_head head;
	int ifindex;
	struct mrib_user mrib;
	struct querier querier;
	enum proxy_flags flags;
};

struct proxy_downlink {
	struct querier_user_iface iface;
	struct mrib_user mrib;
	struct client client;
	enum proxy_flags flags;
};

static struct list_head proxies = LIST_HEAD_INIT(proxies);

// Remove and cleanup a downlink
static void proxy_remove_downlink(struct proxy_downlink *downlink)
{
	mrib_detach_user(&downlink->mrib);
	querier_detach(&downlink->iface);
	client_deinit(&downlink->client);
	free(downlink);
}

// Match scope of a multicast-group against proxy scope-filter
static bool proxy_match_scope(enum proxy_flags flags, const struct in6_addr *addr)
{
	unsigned scope = 0;
	if (IN6_IS_ADDR_V4MAPPED(addr)) {
		if (addr->s6_addr[12] == 239 && addr->s6_addr[13] == 255)
			scope = PROXY_REALMLOCAL;
		else if (addr->s6_addr[12] == 239 && (addr->s6_addr[13] & 0xfc) == 192)
			scope = PROXY_ORGLOCAL;
		else if (addr->s6_addr[12] == 224 && addr->s6_addr[13] == 0 && addr->s6_addr[14] == 0)
			scope = 2;
		else
			scope = PROXY_GLOBAL;
	} else {
		scope = addr->s6_addr[1] & 0xf;
	}
	return scope >= (flags & _PROXY_SCOPEMASK);
}

// Test and set multicast route (called by mrib on detection of new source)
static void proxy_mrib(struct mrib_user *mrib, const struct in6_addr *group,
		const struct in6_addr *source, mrib_filter *filter)
{
	struct proxy *proxy = container_of(mrib, struct proxy, mrib);
	if (!proxy_match_scope(proxy->flags, group))
		return;

	omgp_time_t now = omgp_time();
	struct querier_user *user;
	list_for_each_entry(user, &proxy->querier.ifaces, head) {
		if (groups_includes_group(user->groups, group, source, now)) {
			struct querier_user_iface *iface = container_of(user, struct querier_user_iface, user);
			struct proxy_downlink *downlink = container_of(iface, struct proxy_downlink, iface);
			mrib_filter_add(filter, &downlink->mrib);
		}
	}
}

// Update proxy state (called from querier on change of combined group-state)
static void proxy_trigger(struct querier_user_iface *user, const struct in6_addr *group,
		bool include, const struct in6_addr *sources, size_t len)
{
	struct proxy_downlink *iface = container_of(user, struct proxy_downlink, iface);
	if (proxy_match_scope(iface->flags, group))
		client_set(&iface->client, group, include, sources, len);
}

// Remove proxy with given name
static int proxy_unset(struct proxy *proxyp)
{
	bool found = false;
	struct proxy *proxy, *n;
	list_for_each_entry_safe(proxy, n, &proxies, head) {
		if ((proxyp && proxy == proxyp) ||
				(!proxyp && (proxy->flags & _PROXY_UNUSED))) {
			mrib_detach_user(&proxy->mrib);

			struct querier_user *user, *n;
			list_for_each_entry_safe(user, n, &proxy->querier.ifaces, head) {
				struct querier_user_iface *i = container_of(user, struct querier_user_iface, user);
				proxy_remove_downlink(container_of(i, struct proxy_downlink, iface));
			}

			querier_deinit(&proxy->querier);
			list_del(&proxy->head);
			free(proxy);
			found = true;
		}
	}
	return (found) ? 0 : -ENOENT;
}

// Add / update proxy
int proxy_set(int uplink, const int downlinks[], size_t downlinks_cnt, enum proxy_flags flags)
{
	struct proxy *proxy = NULL, *p;
	list_for_each_entry(p, &proxies, head)
		if (proxy->ifindex == uplink)
			proxy = p;

	if (proxy && (downlinks_cnt == 0 ||
			((proxy->flags & _PROXY_SCOPEMASK) != (flags & _PROXY_SCOPEMASK)))) {
		proxy_unset(proxy);
		proxy = NULL;
	}

	if (downlinks_cnt <= 0)
		return 0;

	if (!proxy) {
		if (!(proxy = calloc(1, sizeof(*proxy))))
			return -ENOMEM;

		if ((flags & _PROXY_SCOPEMASK) == 0)
			flags |= PROXY_GLOBAL;

		proxy->flags = flags;
		proxy->ifindex = uplink;
		querier_init(&proxy->querier);
		list_add(&proxy->head, &proxies);
		if (mrib_attach_user(&proxy->mrib, uplink, proxy_mrib))
			goto err;
	}

	struct querier_user *user, *n;
	list_for_each_entry_safe(user, n, &proxy->querier.ifaces, head) {
		struct querier_user_iface *iface = container_of(user, struct querier_user_iface, user);

		size_t i;
		for (i = 0; i < downlinks_cnt && downlinks[i] == iface->iface->ifindex; ++i);
			if (i == downlinks_cnt)
				proxy_remove_downlink(container_of(iface, struct proxy_downlink, iface));
	}

	for (size_t i = 0; i < downlinks_cnt; ++i) {
		bool found = false;
		struct querier_user *user;
		list_for_each_entry(user, &proxy->querier.ifaces, head) {
			struct querier_user_iface *iface = container_of(user, struct querier_user_iface, user);
			if (iface->iface->ifindex == downlinks[i]) {
				found = true;
				break;
			}
		}

		if (found)
			continue;

		struct proxy_downlink *downlink = calloc(1, sizeof(*downlink));
		if (!downlink)
			goto err;

		if (client_init(&downlink->client, uplink))
			goto downlink_err3;

		if (mrib_attach_user(&downlink->mrib, downlinks[i], NULL))
			goto downlink_err2;

		if (querier_attach(&downlink->iface, &proxy->querier, downlinks[i], proxy_trigger))
			goto downlink_err1;

		downlink->flags = proxy->flags;
		continue;

downlink_err1:
		mrib_detach_user(&downlink->mrib);
downlink_err2:
		client_deinit(&downlink->client);
downlink_err3:
		free(downlink);
		goto err;
	}

	return 0;

err:
	proxy_unset(proxy);
	return -errno;
}

// Mark all flushable proxies as unused
void proxy_update(bool all)
{
	struct proxy *proxy;
	list_for_each_entry(proxy, &proxies, head)
		if (all || (proxy->flags & PROXY_FLUSHABLE))
			proxy->flags |= _PROXY_UNUSED;
}


// Flush all unused proxies
void proxy_flush(void)
{
	proxy_unset(NULL);
}
