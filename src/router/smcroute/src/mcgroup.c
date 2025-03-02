/* Multicast group management (join/leave) API
 *
 * Copyright (C) 2001-2005  Carsten Schill <carsten@cschill.de>
 * Copyright (C) 2006-2009  Julien BLACHE <jb@jblache.org>
 * Copyright (C) 2009       Todd Hayton <todd.hayton@gmail.com>
 * Copyright (C) 2009-2011  Micha Lenk <micha@debian.org>
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"
#include "queue.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#ifdef HAVE_LINUX_FILTER_H
#include <linux/filter.h>
#endif
#include <sys/resource.h>

#include "log.h"
#include "ipc.h"
#include "util.h"
#include "iface.h"
#include "socket.h"
#include "mcgroup.h"
#include "kern.h"

/*
 * Track IGMP join, any-source and source specific
 */
static TAILQ_HEAD(kmcglist, mcgroup) kern_list = TAILQ_HEAD_INITIALIZER(kern_list);
static TAILQ_HEAD(cmcglist, mcgroup) conf_list = TAILQ_HEAD_INITIALIZER(conf_list);

#ifdef HAVE_LINUX_FILTER_H
/*
 * Extremely simple "drop everything" filter for Linux so we do not get
 * a copy each packet of every routed group we join.
 */
static struct sock_filter filter[] = {
	{ 0x6, 0, 0, 0x00000000 },
};

static struct sock_fprog fprog = {
	sizeof(filter) / sizeof(filter[0]),
	filter
};
#endif /* HAVE_LINUX_FILTER_H */

/*
 * Linux net.ipv4.igmp_max_memberships defaults to 20, but empiricism
 * suggests we can only ever get 10 from each socket on Linux 5.11.
 * We therefore calibrate for the current system this using ENOBUFS.
 */
#define MAX_GROUPS 20
static int max_groups = MAX_GROUPS;

struct mc_sock {
	TAILQ_ENTRY(mc_sock) link;

	int family;			/* address family */
	int sd;				/* socket for join/leave ops */
	int cnt;			/* max 20 on linux */
};

TAILQ_HEAD(mcslist, mc_sock) mc_sock_list= TAILQ_HEAD_INITIALIZER(mc_sock_list);

static int alloc_mc_sock(int family)
{
	struct mc_sock *entry;

	TAILQ_FOREACH(entry, &mc_sock_list, link) {
		if (entry->cnt < max_groups && entry->family == family)
			break;
	}

	if (!entry) {
		entry = malloc(sizeof(struct mc_sock));
		if (!entry) {
			smclog(LOG_ERR, "Out of memory in %s()", __func__);
			return -1;
		}

		entry->family = family;
		entry->cnt = 0;
		entry->sd = socket_create(family, SOCK_DGRAM, 0, NULL, NULL);
		if (entry->sd == -1) {
			smclog(LOG_ERR, "Failed creating mc socket: %s", strerror(errno));
			free(entry);
			return -1;
		}

#ifdef HAVE_LINUX_FILTER_H
		if (setsockopt(entry->sd, SOL_SOCKET, SO_ATTACH_FILTER, &fprog, sizeof(fprog)) < 0)
			smclog(LOG_WARNING, "failed setting IPv4 socket filter, continuing anyway");
#endif

		TAILQ_INSERT_TAIL(&mc_sock_list, entry, link);
	}

	entry->cnt++;
	smclog(LOG_DEBUG, "Group socket %d count %d of MAX %d", entry->sd, entry->cnt, max_groups);

	return entry->sd;
}

static void free_mc_sock(int sd)
{
	struct mc_sock *entry, *tmp;

	TAILQ_FOREACH_SAFE(entry, &mc_sock_list, link, tmp) {
		if (entry->sd == sd)
			break;
	}

	if (entry) {
		if (--entry->cnt == 0) {
			TAILQ_REMOVE(&mc_sock_list, entry, link);
			socket_close(entry->sd);
			free(entry);
		}
	}
}

static struct iface *match_valid_iface(const char *ifname, struct ifmatch *state)
{
	struct iface *iface = iface_match_by_name(ifname, 0, state);

	if (!iface && !state->match_count)
		smclog(LOG_DEBUG, "unknown interface %s", ifname);

	return iface;
}

static void list_add(int sd, struct mcgroup *mcg)
{
	struct mcgroup *entry;

	entry = malloc(sizeof(*entry));
	if (!entry) {
		smclog(LOG_ERR, "Failed adding mgroup to list: %s", strerror(errno));
		return;
	}

	*entry    = *mcg;
	entry->sd = sd;

	TAILQ_INSERT_TAIL(&kern_list, entry, link);
}

static void list_rem(int sd, struct mcgroup *mcg)
{
	struct mcgroup *entry, *tmp;

	(void)sd;
	TAILQ_FOREACH_SAFE(entry, &kern_list, link, tmp) {
		if (entry->iface->ifindex != mcg->iface->ifindex)
			continue;

		if (inet_addr_cmp(&entry->source, &mcg->source) ||
		    inet_addr_cmp(&entry->group, &mcg->group))
			continue;

		TAILQ_REMOVE(&kern_list, entry, link);
		free_mc_sock(entry->sd);
		free(entry);
	}
}

void mcgroup_init(void)
{
	struct rlimit rlim;

	if (getrlimit(RLIMIT_NOFILE, &rlim)) {
		smclog(LOG_ERR, "Failed reading RLIMIT_NOFILE");
		return;
	}

	smclog(LOG_DEBUG, "NOFILE: current %lu max %lu", rlim.rlim_cur, rlim.rlim_max);
	rlim.rlim_cur = rlim.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rlim)) {
		smclog(LOG_ERR, "Failed setting RLIMIT_NOFILE soft limit to %lu: %s",
		       rlim.rlim_max, strerror(errno));
		return;
	}
	smclog(LOG_DEBUG, "NOFILE: set new current %ld max %ld", rlim.rlim_cur, rlim.rlim_max);
}

/*
 * Close IPv4/IPv6 multicast sockets to kernel to leave any joined groups
 */
void mcgroup_exit(void)
{
	struct mcgroup *entry, *tmp;

	TAILQ_FOREACH_SAFE(entry, &conf_list, link, tmp) {
		TAILQ_REMOVE(&conf_list, entry, link);
		free(entry);
	}
	TAILQ_FOREACH_SAFE(entry, &kern_list, link, tmp) {
		TAILQ_REMOVE(&kern_list, entry, link);
		free_mc_sock(entry->sd);
		free(entry);
	}
}

static struct mcgroup *find_conf(const char *ifname, inet_addr_t *source, inet_addr_t *group, int len)
{
	struct mcgroup *entry;

	TAILQ_FOREACH(entry, &conf_list, link) {
		if (strcmp(entry->ifname, ifname))
			continue;
		if (inet_addr_cmp(&entry->source, source))
			continue;
		if (inet_addr_cmp(&entry->group, group) || entry->len != len)
			continue;

		return entry;
	}

	return NULL;
}

static struct mcgroup *find_kern(struct mcgroup *mcg)
{
	struct mcgroup *entry;

	TAILQ_FOREACH(entry, &kern_list, link) {
		if (strcmp(entry->ifname, mcg->ifname))
			continue;
		if (inet_addr_cmp(&entry->source, &mcg->source))
			continue;
		if (inet_addr_cmp(&entry->group, &mcg->group))
			continue;

		return entry;
	}

	return NULL;
}

int mcgroup_action(int cmd, const char *ifname, inet_addr_t *source, int src_len, inet_addr_t *group, int len)
{
	char src[INET_ADDRSTR_LEN] = "*", grp[INET_ADDRSTR_LEN];
	struct mcgroup *mcg;
	struct ifmatch state;
	int rc = 0;
	int sd;

	if (!is_anyaddr(source))
		inet_addr2str(source, src, sizeof(src));
	inet_addr2str(group, grp, sizeof(grp));

	mcg = find_conf(ifname, source, group, len);
	if (mcg) {
		if (cmd) {
			if (mcg->unused) {
				mcg->unused = 0;
				return 0;
			}

			smclog(LOG_INFO, "Already joined (%s,%s) on %s", src, grp, ifname);
			errno = EALREADY;
			return 1;
		}
	} else {
		if (!cmd) {
			smclog(LOG_INFO, "No group (%s,%s) on %s to leave", src, grp, ifname);
			errno = ENOENT;
			return 1;
		}

		mcg = calloc(1, sizeof(*mcg));
		if (!mcg) {
			smclog(LOG_ERR, "Out of memory joining (%s,%s) on %s", src, grp, ifname);
			return 1;
		}

		strlcpy(mcg->ifname, ifname, sizeof(mcg->ifname));
		mcg->source  = *source;
		mcg->src_len = src_len;
		mcg->group   = *group;
		mcg->len     = len;

		TAILQ_INSERT_TAIL(&conf_list, mcg, link);
	}

	iface_match_init(&state);
	while ((mcg->iface = match_valid_iface(ifname, &state))) {
		struct inet_iter siter;

		inet_iter_init(&siter, &mcg->source, mcg->src_len);
		while (inet_iterator(&siter, &mcg->source)) {
			struct inet_iter giter;

			inet_iter_init(&giter, &mcg->group, mcg->len);
			while (inet_iterator(&giter, &mcg->group)) {
				if (!cmd) {
					struct mcgroup *kmcg;

					kmcg = find_kern(mcg);
					if (!kmcg)
						continue;

					sd = kmcg->sd;
				} else {
				retry:
					sd = alloc_mc_sock(group->ss_family);
				}

				if (sd == -1) {
					smclog(LOG_ERR, "Failed %s (%s,%s) on %s: %s",
					       cmd ? "joining" : "leaving",
					       src, grp, ifname, strerror(errno));
					continue;
				}

				if (kern_join_leave(sd, cmd, mcg)) {
					if (cmd) {
						switch (errno) {
						case EADDRINUSE:
							 /* Already joined, ignore */
							continue;

						case ENOBUFS:
							smclog(LOG_WARNING, "Out of groups on socket "
							       "adjusting max_groups %d.", max_groups);
							/*
							 * Maxed out net.ipv4.igmp_max_msf
							 * or net.ipv4.igmp_max_memberships
							 * Linux only.
							 */
							max_groups--;
							goto retry;

						default:
							break;
						}
					}
					rc++;
					break;
				}

				if (cmd)
					list_add(sd, mcg);
				else
					list_rem(sd, mcg);
			}

			mcg->group = giter.orig;
		}

		mcg->source = siter.orig;
	}

	if (!cmd) {
		TAILQ_REMOVE(&conf_list, mcg, link);
		free_mc_sock(mcg->sd);
		free(mcg);
	}

	if (!state.match_count)
		return 1;

	return rc;
}

/*
 * Called on SIGHUP/reload.  Mark all known configured groups as
 * 'unused', let mcgroup_action() unmark and mcgroup_reload_end()
 * take care to remove groups that still have the 'unused' flag.
 */
void mcgroup_reload_beg(void)
{
	struct mcgroup *entry;

	TAILQ_FOREACH(entry, &conf_list, link)
		entry->unused = 1;
}

void mcgroup_reload_end(void)
{
	struct mcgroup *entry, *tmp;
	struct iface *iface;
	int first = 1;

	while ((iface = iface_iterator(first))) {
		char  dummy[IFNAMSIZ];

		first = 0;
		if (iface->unused || !if_indextoname(iface->ifindex, dummy))
			mcgroup_prune(iface->ifname);
	}

	TAILQ_FOREACH_SAFE(entry, &conf_list, link, tmp) {
		if (!entry->unused)
			continue;

		mcgroup_action(0, entry->ifname, &entry->source, entry->src_len, &entry->group, entry->len);
	}
}

/*
 * When an interface is removed from the system, or its flags are
 * changed to exclude the MULTICAST flag, we must prune groups.
 */
void mcgroup_prune(char *ifname)
{
	struct mcgroup *entry, *tmp;

	TAILQ_FOREACH_SAFE(entry, &conf_list, link, tmp) {
		if (strcmp(entry->ifname, ifname))
			continue;

		mcgroup_action(0, entry->ifname, &entry->source, entry->src_len, &entry->group, entry->len);
	}
}

static int show_mcgroup(int sd, struct mcgroup *entry)
{
	int max_len = inet_max_len(&entry->group);
	char sg[INET_ADDRSTR_LEN * 2 + 10 + 3];
	char src[INET_ADDRSTR_LEN] = "*";
	char grp[INET_ADDRSTR_LEN];
	char line[256];

	if (!is_anyaddr(&entry->source))
		inet_addr2str(&entry->source, src, sizeof(src));
	inet_addr2str(&entry->group, grp, sizeof(grp));

	snprintf(sg, sizeof(sg), "(%s", src);
	if (entry->src_len != max_len)
		snprintf(line, sizeof(line), "/%u, ", entry->src_len);
	else
		snprintf(line, sizeof(line), ", ");
	strlcat(sg, line, sizeof(sg));

	if (entry->len != max_len)
		snprintf(line, sizeof(line), "%s/%u)", grp, entry->len);
	else
		snprintf(line, sizeof(line), "%s)", grp);
	strlcat(sg, line, sizeof(sg));

	snprintf(line, sizeof(line), "%-42s %s\n", sg, entry->ifname);
	if (ipc_send(sd, line, strlen(line)) < 0) {
		smclog(LOG_ERR, "Failed sending reply to client: %s", strerror(errno));
		return -1;
	}

	return 0;
}

/* Write all joined IGMP/MLD groups to client socket */
int mcgroup_show(int sd, int detail)
{
	char *conf_str = "Group Memberships Table_\n";
	char *kern_str = "Kernel Group Membership Table_\n";
	struct mcgroup *entry;
	char line[256];

	if (TAILQ_EMPTY(&conf_list))
		return 0;

	ipc_send(sd, conf_str, strlen(conf_str));
	snprintf(line, sizeof(line), "%-42s %-16s=\n", "GROUP (S,G)", "IIF");
	ipc_send(sd, line, strlen(line));

	TAILQ_FOREACH(entry, &conf_list, link) {
		if (show_mcgroup(sd, entry) < 0)
		    return 1;
	}

	if (!detail)
		return 0;

	ipc_send(sd, kern_str, strlen(kern_str));
	snprintf(line, sizeof(line), "%-42s %-16s=\n", "GROUP (S,G)", "IIF");
	TAILQ_FOREACH(entry, &kern_list, link) {
		if (show_mcgroup(sd, entry) < 0)
		    return 1;
	}

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
