/*
 *   Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License v2 as published by
 *   the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <linux/fib_rules.h>

#include "relayd.h"

static struct uloop_fd rtnl_sock;
static unsigned int rtnl_seq, rtnl_dump_seq;
int route_table = 16800;

static void rtnl_flush(void)
{
	int fd;

	fd = open("/proc/sys/net/ipv4/route/flush", O_WRONLY);
	if (fd < 0)
		return;

	write(fd, "-1", 2);
	close(fd);
}

enum {
	RULE_F_ADD = (1 << 0),
	RULE_F_DEFGW_WORKAROUND = (1 << 1),
};

static int get_route_table(struct relayd_interface *rif)
{
	if (rif)
		return rif->rt_table;
	else
		return local_route_table;
}

static void
rtnl_rule_request(struct relayd_interface *rif, int flags)
{
	static struct {
		struct nlmsghdr nl;
		struct rtmsg rt;
		struct {
			struct rtattr rta;
			int table;
		} __packed table;
		struct {
			struct rtattr rta;
			char ifname[IFNAMSIZ + 1];
		} __packed dev;
	} __packed req = {
		.rt = {
			.rtm_family = AF_INET,
			.rtm_table = RT_TABLE_UNSPEC,
			.rtm_scope = RT_SCOPE_UNIVERSE,
			.rtm_protocol = RTPROT_BOOT,
		},
		.table.rta = {
			.rta_type = FRA_TABLE,
			.rta_len = sizeof(req.table),
		},
	};
	const char *ifname = "lo";
	int padding = sizeof(req.dev.ifname);

	if (rif)
		ifname = rif->ifname;

	if (!(flags & RULE_F_DEFGW_WORKAROUND)) {
		req.dev.rta.rta_type = FRA_IFNAME;
		padding -= strlen(ifname) + 1;
		strcpy(req.dev.ifname, ifname);
		req.dev.rta.rta_len = sizeof(req.dev.rta) + strlen(ifname) + 1;
	} else {
		req.dev.rta.rta_type = FRA_PRIORITY;
		req.dev.rta.rta_len = sizeof(req.dev.rta) + sizeof(uint32_t);
		padding -= sizeof(uint32_t);
		*((uint32_t *) &req.dev.ifname) = 1;
	}
	req.table.table = get_route_table(rif);
	req.nl.nlmsg_len = sizeof(req) - padding;

	req.nl.nlmsg_flags = NLM_F_REQUEST;
	if (flags & RULE_F_ADD) {
		req.nl.nlmsg_type = RTM_NEWRULE;
		req.nl.nlmsg_flags |= NLM_F_CREATE | NLM_F_EXCL;

		req.rt.rtm_type = RTN_UNICAST;
	} else {
		req.nl.nlmsg_type = RTM_DELRULE;
		req.rt.rtm_type = RTN_UNSPEC;
	}

	send(rtnl_sock.fd, &req, req.nl.nlmsg_len, 0);
	rtnl_flush();
}

struct rtnl_addr {
	struct rtattr rta;
	uint8_t ipaddr[4];
} __packed;

static struct rtnl_addr *
rtnl_add_addr(struct rtnl_addr *addr, int *len, int type, const uint8_t *ipaddr)
{
	addr->rta.rta_type = type;
	memcpy(addr->ipaddr, ipaddr, 4);
	*len += sizeof(*addr);
	return addr + 1;
}

static void
rtnl_route_request(struct relayd_interface *rif, struct relayd_host *host,
		   struct relayd_route *route, bool add)
{
	static struct {
		struct nlmsghdr nl;
		struct rtmsg rt;
		struct {
			struct rtattr rta;
			int table;
		} __packed table;
		struct {
			struct rtattr rta;
			int ifindex;
		} __packed dev;
		struct rtnl_addr addr[3];
	} __packed req = {
		.rt = {
			.rtm_family = AF_INET,
			.rtm_dst_len = 32,
			.rtm_table = RT_TABLE_MAIN,
		},
		.table.rta = {
			.rta_type = RTA_TABLE,
			.rta_len = sizeof(req.table),
		},
		.dev.rta = {
			.rta_type = RTA_OIF,
			.rta_len = sizeof(req.dev),
		},
		.addr[0].rta.rta_len = sizeof(struct rtnl_addr),
		.addr[1].rta.rta_len = sizeof(struct rtnl_addr),
		.addr[2].rta.rta_len = sizeof(struct rtnl_addr),
	};
	int pktlen = sizeof(req) - sizeof(req.addr);
	struct rtnl_addr *addr = &req.addr[0];
	const char *ifname = "loopback";

	req.dev.ifindex = host->rif->sll.sll_ifindex;
	req.table.table = get_route_table(rif);

	req.nl.nlmsg_flags = NLM_F_REQUEST;
	if (add) {
		req.nl.nlmsg_type = RTM_NEWROUTE;
		req.nl.nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;

		req.rt.rtm_protocol = RTPROT_BOOT;
		if (route) {
			req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
		} else {
			req.rt.rtm_scope = RT_SCOPE_LINK;
		}
		req.rt.rtm_type = RTN_UNICAST;
	} else {
		req.nl.nlmsg_type = RTM_DELROUTE;
		req.rt.rtm_scope = RT_SCOPE_NOWHERE;
	}

	if (rif)
		ifname = rif->ifname;

	if (route) {
		DPRINTF(2, "%s: add route to "IP_FMT"/%d via "IP_FMT" (%s)\n", ifname,
			IP_BUF(route->dest), route->mask, IP_BUF(host->ipaddr),
			host->rif->ifname);

		req.rt.rtm_dst_len = route->mask;
		if (route->mask)
			addr = rtnl_add_addr(addr, &pktlen, RTA_DST, route->dest);
		addr = rtnl_add_addr(addr, &pktlen, RTA_GATEWAY, host->ipaddr);
	} else {
		DPRINTF(2, "%s: add host route to "IP_FMT" (%s)\n", ifname,
			IP_BUF(host->ipaddr), host->rif->ifname);
		addr = rtnl_add_addr(addr, &pktlen, RTA_DST, host->ipaddr);
		req.rt.rtm_dst_len = 32;
	}

	/* local route */
	if (!rif)
		addr = rtnl_add_addr(addr, &pktlen, RTA_PREFSRC, local_addr);

	req.nl.nlmsg_len = pktlen;
	if (route)
		rtnl_rule_request(rif, RULE_F_DEFGW_WORKAROUND | RULE_F_ADD);
	send(rtnl_sock.fd, &req, pktlen, 0);
	if (route)
		rtnl_rule_request(rif, RULE_F_DEFGW_WORKAROUND);
	rtnl_flush();
}

void
rtnl_route_set(struct relayd_host *host, struct relayd_route *route, bool add)
{
	struct relayd_interface *rif;

	list_for_each_entry(rif, &interfaces, list) {
		if (rif == host->rif)
			continue;

		rtnl_route_request(rif, host, route, add);
	}
	if (local_route_table)
		rtnl_route_request(NULL, host, route, add);
}

void relayd_add_interface_routes(struct relayd_interface *rif)
{
	rif->rt_table = route_table++;
	rtnl_rule_request(rif, RULE_F_ADD);
}

void relayd_del_interface_routes(struct relayd_interface *rif)
{
	rtnl_rule_request(rif, 0);
}

#ifndef NDA_RTA
#define NDA_RTA(r) \
    ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif

static void rtnl_parse_newneigh(struct nlmsghdr *h)
{
	struct relayd_interface *rif = NULL;
	struct ndmsg *r = NLMSG_DATA(h);
	const uint8_t *lladdr = NULL;
	const uint8_t *ipaddr = NULL;
	struct rtattr *rta;
	int len;

	if (r->ndm_family != AF_INET)
		return;

	list_for_each_entry(rif, &interfaces, list) {
		if (rif->sll.sll_ifindex == r->ndm_ifindex)
			goto found_interface;
	}
	return;

found_interface:
	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*r));
	for (rta = NDA_RTA(r); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch(rta->rta_type) {
		case NDA_LLADDR:
			lladdr = RTA_DATA(rta);
			break;
		case NDA_DST:
			ipaddr = RTA_DATA(rta);
			break;
		default:
			break;
		}
	}

	if (!lladdr || !ipaddr || (r->ndm_state & (NUD_INCOMPLETE|NUD_FAILED)))
		return;

	if (!memcmp(lladdr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
		return;

	DPRINTF(1, "%s: Found ARP cache entry for host "IP_FMT" ("MAC_FMT")\n",
		rif->ifname, IP_BUF(ipaddr), MAC_BUF(lladdr));
	relayd_refresh_host(rif, lladdr, ipaddr);
}

static void rtnl_parse_packet(void *data, int len)
{
	struct nlmsghdr *h;

	for (h = data; NLMSG_OK(h, len); h = NLMSG_NEXT(h, len)) {
		if (h->nlmsg_type == NLMSG_DONE ||
		    h->nlmsg_type == NLMSG_ERROR)
			return;

		if (h->nlmsg_seq != rtnl_dump_seq)
			continue;

		if (h->nlmsg_type == RTM_NEWNEIGH)
			rtnl_parse_newneigh(h);
	}
}

static void rtnl_cb(struct uloop_fd *fd, unsigned int events)
{
	struct sockaddr_nl nladdr;
	static uint8_t buf[16384];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf),
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	do {
		int len;

		len = recvmsg(rtnl_sock.fd, &msg, 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;

			return;
		}

		if (!len)
			break;

		if (nladdr.nl_pid != 0)
			continue;

		rtnl_parse_packet(buf, len);
	} while (1);
}

static void rtnl_dump_request(int nlmsg_type)
{
	static struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST,
			.nlmsg_pid = 0,
		},
		.g.rtgen_family = AF_INET,
	};
	req.nlh.nlmsg_type = nlmsg_type;
	req.nlh.nlmsg_seq = rtnl_seq;
	send(rtnl_sock.fd, &req, sizeof(req), 0);
	rtnl_seq++;
}

int relayd_rtnl_init(void)
{
	struct sockaddr_nl snl_local;

	rtnl_sock.fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rtnl_sock.fd < 0) {
		perror("socket(AF_NETLINK)");
		return -1;
	}

	snl_local.nl_family = AF_NETLINK;

	if (bind(rtnl_sock.fd, (struct sockaddr *) &snl_local, sizeof(struct sockaddr_nl)) < 0) {
		perror("bind");
		close(rtnl_sock.fd);
		return -1;
	}

	rtnl_sock.cb = rtnl_cb;
	uloop_fd_add(&rtnl_sock, ULOOP_READ | ULOOP_EDGE_TRIGGER);

	rtnl_seq = time(NULL);
	rtnl_dump_seq = rtnl_seq;
	rtnl_dump_request(RTM_GETNEIGH);
	rtnl_rule_request(NULL, RULE_F_ADD);

	return 0;
}

void relayd_rtnl_done(void)
{
	rtnl_rule_request(NULL, 0);
	uloop_fd_delete(&rtnl_sock);
	close(rtnl_sock.fd);
}
