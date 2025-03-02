/* Kernel API for join/leave multicast groups and add/del routes
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "iface.h"
#include "kern.h"
#include "log.h"
#include "mrdisc.h"
#include "socket.h"
#include "util.h"

/*
 * Raw IGMP socket used as interface for the IPv4 mrouted API.
 * Receives IGMP packets and upcall messages from the kernel.
 */
static int sd4 = -1;
/*
 * Raw ICMPv6 socket used as interface for the IPv6 mrouted API.
 * Receives MLD packets and upcall messages from the kenrel.
 */
static int sd6 = -1;

/* IPv4 internal virtual interfaces (VIF) descriptor vector */
static struct {
	struct iface *iface;
} vif_list[MAX_MC_VIFS];

/* IPv6 internal virtual interfaces (VIF) descriptor vector */
static struct mif {
	struct iface *iface;
} mif_list[MAX_MC_VIFS];


/*
 * This function handles both ASM and SSM join/leave for IPv4 and IPv6
 * using the RFC 3678 API available on Linux, FreeBSD, and a few other
 * operating systems.
 *
 * On Linux this makes it possible to join a group on an interface that
 * is down and/or has no IP address assigned to it yet.  The latter is
 * one of the most common causes of malfunction on Linux and IPv4 with
 * the old struct ip_mreq API.
 */
#ifdef HAVE_STRUCT_GROUP_REQ	/* Prefer RFC 3678 */
static int group_req(int sd, int cmd, struct mcgroup *mcg)
{
	char source[INET_ADDRSTR_LEN], group[INET_ADDRSTR_LEN];
	struct group_source_req gsr = { 0 };
	struct group_req gr = { 0 };
	size_t len;
	void *arg;
	int op, proto;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (mcg->group.ss_family == AF_INET6)
		proto = IPPROTO_IPV6;
	else
#endif
		proto = IPPROTO_IP;

	if (is_anyaddr(&mcg->source)) {
		if (cmd)	op = MCAST_JOIN_GROUP;
		else		op = MCAST_LEAVE_GROUP;

		arg                = &gr;
		len                = sizeof(gr);
		gr.gr_interface    = mcg->iface->ifindex;
		gr.gr_group        = mcg->group;

		strncpy(source, "*", sizeof(source));
		inet_addr2str(&gr.gr_group, group, sizeof(group));
	} else {
		if (cmd)	op = MCAST_JOIN_SOURCE_GROUP;
		else		op = MCAST_LEAVE_SOURCE_GROUP;

		arg                = &gsr;
		len                = sizeof(gsr);
		gsr.gsr_interface  = mcg->iface->ifindex;
		gsr.gsr_source     = mcg->source;
		gsr.gsr_group      = mcg->group;

		inet_addr2str(&gsr.gsr_source, source, sizeof(source));
		inet_addr2str(&gsr.gsr_group, group, sizeof(group));
	}

	smclog(LOG_DEBUG, "%s group (%s,%s) on ifindex %d and socket %d ...",
	       cmd ? "Join" : "Leave", source, group, mcg->iface->ifindex, sd);

	return setsockopt(sd, proto, op, arg, len);
}

#else  /* Assume we have old style struct ip_mreq */

static int group_req(int sd, int cmd, struct mcgroup *mcg)
{
	char group[INET_ADDRSTR_LEN];
#ifdef HAVE_IPV6_MULTICAST_HOST
	struct ipv6_mreq ipv6mr = { 0 };
#endif
#ifdef HAVE_STRUCT_IP_MREQN
	struct ip_mreqn imr = { 0 };
#else
	struct ip_mreq imr = { 0 };
#endif
	int op, proto;
	size_t len;
	void *arg;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (mcg->group.ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6;

		sin6 = inet_addr6_get(&mcg->group);
		ipv6mr.ipv6mr_multiaddr = sin6->sin6_addr;
		ipv6mr.ipv6mr_interface = mcg->iface->ifindex;

		proto = IPPROTO_IPV6;
		op    = cmd ? IPV6_JOIN_GROUP : IPV6_LEAVE_GROUP;
		arg   = &ipv6mr;
		len   = sizeof(ipv6mr);
	} else
#endif
	{
		imr.imr_multiaddr = *inet_addr_get(&mcg->group);
#ifdef HAVE_STRUCT_IP_MREQN
		imr.imr_ifindex   = mcg->iface->ifindex;
#else
		imr.imr_interface = mcg->iface->inaddr;
#endif

		proto = IPPROTO_IP;
		op    = cmd ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
		arg   = &imr;
		len   = sizeof(imr);
	}

	smclog(LOG_DEBUG, "%s group (*,%s) on ifindex %d ...", cmd ? "Join" : "Leave",
	       inet_addr2str(&mcg->group, group, sizeof(group)), mcg->iface->ifindex);

	return setsockopt(sd, proto, op, arg, len);
}
#endif

int kern_join_leave(int sd, int cmd, struct mcgroup *mcg)
{
	if (group_req(sd, cmd, mcg)) {
		char source[INET_ADDRSTR_LEN] = "*";
		char group[INET_ADDRSTR_LEN];
		int len;

		if (!is_anyaddr(&mcg->source))
			inet_addr2str(&mcg->source, source, sizeof(source));
		inet_addr2str(&mcg->group, group, sizeof(group));
		len = mcg->len == 0 ? 32 : mcg->len;

		smclog(LOG_ERR, "Failed %s group (%s,%s/%d) on sd %d ... %d: %s",
		       cmd ? "joining" : "leaving",
		       source, group, len, sd,
		       errno, strerror(errno));
		return 1;
	}

	return 0;
}

int kern_mroute_init(int table_id, void (*cb)(int, void *), void *arg)
{
	int val = 1;

	if (sd4 < 0) {
		sd4 = socket_create(AF_INET, SOCK_RAW, IPPROTO_IGMP, cb, arg);
		if (sd4 < 0)
			return -1;
	}

#ifdef MRT_TABLE /* Currently only available on Linux  */
	if (table_id != 0) {
		smclog(LOG_INFO, "Setting IPv4 multicast routing table id %d", table_id);
		if (setsockopt(sd4, IPPROTO_IP, MRT_TABLE, &table_id, sizeof(table_id)) < 0) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
	}
#else
	(void)table_id;
#endif

	if (setsockopt(sd4, IPPROTO_IP, MRT_INIT, &val, sizeof(val)))
		goto error;

	/* Enable "PIM" to get WRONGVIF messages */
	if (setsockopt(sd4, IPPROTO_IP, MRT_PIM, &val, sizeof(val)))
		smclog(LOG_ERR, "Failed enabling PIM IGMPMSG_WRONGVIF, ignoring: %s", strerror(errno));

	/* Initialize virtual interface table */
	memset(&vif_list, 0, sizeof(vif_list));

	return 0;
error:
	socket_close(sd4);
	sd4 = -1;

	return -1;
}

int kern_mroute_exit(void)
{
	if (sd4 == -1)
		return errno = EAGAIN;

	/* Drop all kernel routes set by smcroute */
	if (setsockopt(sd4, IPPROTO_IP, MRT_DONE, NULL, 0))
		smclog(LOG_WARNING, "Failed shutting down IPv4 multicast routing socket: %s",
		       strerror(errno));

	socket_close(sd4);
	sd4 = -1;

	return 0;
}

int kern_vif_add(struct iface *iface)
{
	struct vifctl vifc = { 0 };
	size_t i;
	int vif;

	if (!iface)
		return errno = EINVAL;
	if ((iface->flags & IFF_MULTICAST) != IFF_MULTICAST)
		return errno = ENOPROTOOPT;
	if (sd4 == -1)
		return errno = EAGAIN;
	if (iface->vif != NO_VIF)
		return errno = EEXIST;

	/* find a free vif */
	for (i = 0, vif = -1; i < NELEMS(vif_list); i++) {
		if (!vif_list[i].iface) {
			vif = i;
			break;
		}
	}

	if (vif == -1)
		return errno = ENOMEM;

	memset(&vifc, 0, sizeof(vifc));
	vifc.vifc_vifi = vif;
	vifc.vifc_flags = 0;      /* no tunnel, no source routing, register ? */
	vifc.vifc_threshold = iface->threshold;
	vifc.vifc_rate_limit = 0;	/* hopefully no limit */
#ifdef VIFF_USE_IFINDEX		/* Register VIF using ifindex, not lcl_addr, since Linux 2.6.33 */
	vifc.vifc_flags |= VIFF_USE_IFINDEX;
	vifc.vifc_lcl_ifindex = iface->ifindex;
#else
	vifc.vifc_lcl_addr.s_addr = iface->inaddr.s_addr;
#endif
	vifc.vifc_rmt_addr.s_addr = htonl(INADDR_ANY);

	smclog(LOG_DEBUG, "Map iface %-16s => VIF %-2d ifindex %2d flags 0x%04x TTL threshold %u",
	       iface->ifname, vifc.vifc_vifi, iface->ifindex, vifc.vifc_flags, iface->threshold);

	if (setsockopt(sd4, IPPROTO_IP, MRT_ADD_VIF, &vifc, sizeof(vifc)))
		return 1;

	iface->vif = vif;
	vif_list[vif].iface = iface;

	return 0;
}

int kern_vif_del(struct iface *iface)
{
	struct vifctl vifc = { 0 };
	int rc;

	if (sd4 == -1)
		return errno = EAGAIN;
	if (!iface)
		return errno = EINVAL;
	if (iface->vif == NO_VIF)
		return errno = ENOENT;

	smclog(LOG_DEBUG, "Removing  %-16s => VIF %-2d", iface->ifname, iface->vif);

	vifc.vifc_vifi = iface->vif;
#ifdef __linux__
	rc = setsockopt(sd4, IPPROTO_IP, MRT_DEL_VIF, &vifc, sizeof(vifc));
#else
	rc = setsockopt(sd4, IPPROTO_IP, MRT_DEL_VIF, &vifc.vifc_vifi, sizeof(vifc.vifc_vifi));
#endif
	if (!rc) {
		vif_list[iface->vif].iface = NULL;
		iface->vif = -1;
	}

	return rc;
}

static int kern_mroute4(int cmd, struct mroute *route)
{
	char origin[INET_ADDRSTRLEN], group[INET_ADDRSTRLEN];
	int op = cmd ? MRT_ADD_MFC : MRT_DEL_MFC;
	struct mfcctl mfcc = { 0 };
	size_t i;

	if (sd4 == -1) {
		smclog(LOG_DEBUG, "No IPv4 multicast socket");
		return -1;
	}

	memset(&mfcc, 0, sizeof(mfcc));
	mfcc.mfcc_origin   = *inet_addr_get(&route->source);
	mfcc.mfcc_mcastgrp = *inet_addr_get(&route->group);
	mfcc.mfcc_parent   = route->inbound;

	inet_addr2str(&route->source, origin, sizeof(origin));
	inet_addr2str(&route->group, group, sizeof(group));

	/* copy the TTL vector, as many as the kernel supports */
	for (i = 0; i < NELEMS(mfcc.mfcc_ttls); i++)
		mfcc.mfcc_ttls[i] = route->ttl[i];

	if (setsockopt(sd4, IPPROTO_IP, op, &mfcc, sizeof(mfcc))) {
		if (ENOENT == errno)
			smclog(LOG_DEBUG, "failed removing multicast route (%s,%s), does not exist.",
				origin, group);
		else
			smclog(LOG_WARNING, "failed %s IPv4 multicast route (%s,%s): %s",
			       cmd ? "adding" : "removing", origin, group, strerror(errno));
		return 1;
	}

	smclog(LOG_DEBUG, "%s %s -> %s from VIF %d", cmd ? "Add" : "Del",
	       origin, group, route->inbound);

	return 0;
}

static int kern_stats4(struct mroute *route, struct mroute_stats *ms)
{
	struct sioc_sg_req sg_req = { 0 };

	if (sd4 == -1)
		return errno = EAGAIN;

	memset(&sg_req, 0, sizeof(sg_req));
	sg_req.src = *inet_addr_get(&route->source);
	sg_req.grp = *inet_addr_get(&route->group);

	if (ioctl(sd4, SIOCGETSGCNT, &sg_req) < 0) {
		if (ms->ms_wrong_if)
			smclog(LOG_WARNING, "Failed getting MFC stats: %s", strerror(errno));
		return errno;
	}

	ms->ms_pktcnt   = sg_req.pktcnt;
	ms->ms_bytecnt  = sg_req.bytecnt;
	ms->ms_wrong_if = sg_req.wrong_if;

	return 0;
}

#ifdef  HAVE_IPV6_MULTICAST_HOST
#ifdef __linux__
#define IPV6_ALL_MC_FORWARD "/proc/sys/net/ipv6/conf/all/mc_forwarding"

static int proc_set_val(char *file, int val)
{
	int fd, rc = 0;

	fd = open(file, O_WRONLY);
	if (fd < 0)
		return 1;

	if (-1 == write(fd, "1", val))
		rc = 1;

	close(fd);

	return rc;
}
#endif /* Linux only */

int kern_mroute6_init(int table_id, void (*cb)(int, void *), void *arg)
{
	int val = 1;

	if (sd6 < 0) {
		sd6 = socket_create(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6, cb, arg);
		if (sd6 < 0)
			return -1;
	}

#ifdef MRT6_TABLE /* Currently only available on Linux  */
	if (table_id != 0) {
		smclog(LOG_INFO, "Setting IPv6 multicast routing table id %d", table_id);
		if (setsockopt(sd6, IPPROTO_IPV6, MRT6_TABLE, &table_id, sizeof(table_id)) < 0) {
			errno = EPROTONOSUPPORT;
			goto error;
		}
	}
#else
	(void)table_id;
#endif

	if (setsockopt(sd6, IPPROTO_IPV6, MRT6_INIT, &val, sizeof(val)))
		goto error;

	/* Initialize virtual interface table */
	memset(&mif_list, 0, sizeof(mif_list));

#ifdef __linux__
	/*
	 * On Linux pre 2.6.29 kernels net.ipv6.conf.all.mc_forwarding
	 * is not set on MRT6_INIT so we have to do this manually
	 */
	if (proc_set_val(IPV6_ALL_MC_FORWARD, 1)) {
		if (errno != EACCES) {
			smclog(LOG_ERR, "Failed enabling IPv6 multicast forwarding: %s",
			       strerror(errno));
			goto error;
		}
	}
#endif

	return 0;
error:
	socket_close(sd6);
	sd6 = -1;

	return -1;
}

int kern_mroute6_exit(void)
{
	if (sd6 == -1)
		return errno = EAGAIN;

	if (setsockopt(sd6, IPPROTO_IPV6, MRT6_DONE, NULL, 0))
		smclog(LOG_WARNING, "Failed shutting down IPv6 multicast routing socket: %s",
		       strerror(errno));

	socket_close(sd6);
	sd6 = -1;

	return 0;
}

/* Create a virtual interface from @iface so it can be used for IPv6 multicast routing. */
int kern_mif_add(struct iface *iface)
{
	struct mif6ctl mif6c = { 0 };
	int mif = -1;
	size_t i;

	if (sd6 == -1)
		return errno = EAGAIN;
	if (!iface)
		return errno = EINVAL;
	if ((iface->flags & IFF_MULTICAST) != IFF_MULTICAST)
		return errno = ENOPROTOOPT;
	if (iface->mif != NO_VIF)
		return errno = EEXIST;

	/* find a free mif */
	for (i = 0; i < NELEMS(mif_list); i++) {
		if (!mif_list[i].iface) {
			mif = i;
			break;
		}
	}

	if (mif == -1)
		return errno = ENOMEM;

	memset(&mif6c, 0, sizeof(mif6c));
	mif6c.mif6c_mifi = mif;
	mif6c.mif6c_flags = 0;             /* no register */
#ifdef HAVE_MIF6CTL_VIFC_THRESHOLD
	mif6c.vifc_threshold = iface->threshold;
#endif
	mif6c.mif6c_pifi = iface->ifindex; /* physical interface index */
#ifdef HAVE_MIF6CTL_VIFC_RATE_LIMIT
	mif6c.vifc_rate_limit = 0;         /* hopefully no limit */
#endif

	smclog(LOG_DEBUG, "Map iface %-16s => MIF %-2d ifindex %2d flags 0x%04x TTL threshold %u",
	       iface->ifname, mif6c.mif6c_mifi, mif6c.mif6c_pifi, mif6c.mif6c_flags, iface->threshold);

	if (setsockopt(sd6, IPPROTO_IPV6, MRT6_ADD_MIF, &mif6c, sizeof(mif6c)))
		return -1;

	iface->mif = mif;
	mif_list[mif].iface = iface;

	return 0;
}

int kern_mif_del(struct iface *iface)
{
	int rc;

	if (sd6 == -1)
		return errno = EAGAIN;
	if (!iface)
		return errno = EINVAL;
	if (iface->mif == NO_VIF)
		return errno = ENOENT;

	smclog(LOG_DEBUG, "Removing  %-16s => MIF %-2d", iface->ifname, iface->mif);

	rc = setsockopt(sd6, IPPROTO_IPV6, MRT6_DEL_MIF, &iface->mif, sizeof(iface->mif));
	if (!rc) {
		mif_list[iface->mif].iface = NULL;
		iface->mif = -1;
	}

	return rc;
}

static int kern_mroute6(int cmd, struct mroute *route)
{
	char origin[INET_ADDRSTR_LEN], group[INET_ADDRSTR_LEN];
	int op = cmd ? MRT6_ADD_MFC : MRT6_DEL_MFC;
	struct mf6cctl mf6cc = { 0 };
	size_t i;

	if (sd6 == -1)
		return errno = EAGAIN;
	if (!route)
		return errno = EINVAL;

	memset(&mf6cc, 0, sizeof(mf6cc));
	mf6cc.mf6cc_origin   = *inet_addr6_get(&route->source);
	mf6cc.mf6cc_mcastgrp = *inet_addr6_get(&route->group);
	mf6cc.mf6cc_parent   = route->inbound;

	inet_addr2str(&route->source, origin, INET_ADDRSTR_LEN);
	inet_addr2str(&route->group, group, INET_ADDRSTR_LEN);

	IF_ZERO(&mf6cc.mf6cc_ifset);
	for (i = 0; i < NELEMS(route->ttl); i++) {
		if (route->ttl[i]) {
			IF_SET(i, &mf6cc.mf6cc_ifset);
		}
	}

	if (setsockopt(sd6, IPPROTO_IPV6, op, &mf6cc, sizeof(mf6cc))) {
		if (ENOENT == errno)
			smclog(LOG_DEBUG, "failed removing IPv6 multicast route (%s,%s), "
			       "does not exist.", origin, group);
		else
			smclog(LOG_WARNING, "failed %s IPv6 multicast route (%s,%s): %s",
			       cmd ? "adding" : "removing", origin, group, strerror(errno));
		return 1;
	}

	smclog(LOG_DEBUG, "%s %s -> %s from VIF %d", cmd ? "Add" : "Del",
	       origin, group, route->inbound);

	return 0;
}

static int kern_stats6(struct mroute *route, struct mroute_stats *ms)
{
	struct sioc_sg_req6 sg_req = { 0 };

	if (sd6 == -1)
		return errno = EAGAIN;

	sg_req.src = *inet_addr6_get(&route->source);
	sg_req.grp = *inet_addr6_get(&route->group);

	if (ioctl(sd6, SIOCGETSGCNT_IN6, &sg_req) < 0) {
		if (ms->ms_wrong_if)
			smclog(LOG_WARNING, "Failed getting MFC stats: %s", strerror(errno));
		return errno;
	}

	ms->ms_pktcnt   = sg_req.pktcnt;
	ms->ms_bytecnt  = sg_req.bytecnt;
	ms->ms_wrong_if = sg_req.wrong_if;

	return 0;
}
#endif /* HAVE_IPV6_MULTICAST_HOST */

/*
 * Query kernel for route usage statistics
 */
int kern_stats(struct mroute *route, struct mroute_stats *ms)
{
	if (!route || !ms)
		return errno = EINVAL;

#ifdef  HAVE_IPV6_MULTICAST_HOST
	if (route->group.ss_family == AF_INET6)
		return kern_stats6(route, ms);
#endif

	return kern_stats4(route, ms);
}

int kern_mroute_add(struct mroute *route)
{
	if (!route)
		return errno = EINVAL;

#ifdef  HAVE_IPV6_MULTICAST_HOST
	if (route->group.ss_family == AF_INET6)
		return kern_mroute6(1, route);
#endif

	return kern_mroute4(1, route);
}

int kern_mroute_del(struct mroute *route)
{
	if (!route)
		return errno = EINVAL;

#ifdef  HAVE_IPV6_MULTICAST_HOST
	if (route->group.ss_family == AF_INET6)
		return kern_mroute6(0, route);
#endif

	return kern_mroute4(0, route);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
