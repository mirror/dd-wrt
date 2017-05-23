/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *    Reuben Hawkins		<reubenhwk@gmail.com>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "radvd.h"
#include "log.h"
#include "netlink.h"

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef SOL_NETLINK
#define SOL_NETLINK	270
#endif
#ifndef RTM_SETLINK
#define RTM_SETLINK (RTM_BASE+3)
#endif

#ifndef NLA_F_NESTED
#define NLA_F_NESTED		(1 << 15)
#endif

struct iplink_req {
	struct nlmsghdr		n;
	struct ifinfomsg	i;
	char			buf[1024];
};

int netlink_get_device_addr_len(struct Interface *iface)
{
	struct iplink_req req = {};
	struct iovec iov = { &req, sizeof(req) };
	struct sockaddr_nl sa = {};
	struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	int sock, len, addr_len = -1;
	unsigned short type;
	char answer[32768];
	struct rtattr *tb;

	/* nl_pid (for linux kernel) and nl_groups (unicast) should be zero */
	sa.nl_family = AF_NETLINK;
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_GETLINK;
	req.i.ifi_index = iface->props.if_index;

	sock = netlink_socket();
	if (sock == -1)
		return -1;

	len = sendmsg(sock, &msg, 0);
	if (len == -1) {
		flog(LOG_ERR, "netlink: sendmsg for addr_len failed: %s", strerror(errno));
		goto out;
	}

	iov.iov_base = answer;
	iov.iov_len = sizeof(answer);
	len = recvmsg(sock, &msg, 0);
	if (len == -1) {
		flog(LOG_ERR, "netlink: recvmsg for addr_len failed: %s", strerror(errno));
		goto out;
	}

	if (len < NLMSG_LENGTH(sizeof(struct ifinfomsg)))
		goto out;
	len -= NLMSG_LENGTH(sizeof(struct ifinfomsg));

	tb = (struct rtattr *)(answer + NLMSG_LENGTH(sizeof(struct ifinfomsg)));
	while (RTA_OK(tb, len)) {
		type = tb->rta_type & ~NLA_F_NESTED;
		if (type == IFLA_ADDRESS) {
			addr_len = RTA_PAYLOAD(tb);
			break;
		}
		tb = RTA_NEXT(tb, len);
	}

out:
	close(sock);

	return addr_len;
}

void process_netlink_msg(int sock, struct Interface * ifaces)
{
	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = {
			.msg_name = (void*)&sa,
			.msg_namelen = sizeof(sa),
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = NULL,
			.msg_controllen = 0,
			.msg_flags = 0
	};
	int len = recvmsg(sock, &msg, 0);
	if (len == -1) {
		flog(LOG_ERR, "netlink: recvmsg failed: %s", strerror(errno));
	}

	for (struct nlmsghdr * nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
		char ifnamebuf[IF_NAMESIZE];
		/* The end of multipart message. */
		if (nh->nlmsg_type == NLMSG_DONE)
			return;

		if (nh->nlmsg_type == NLMSG_ERROR) {
			flog(LOG_ERR, "netlink: unknown error");
			abort();
		}

		/* Continue with parsing payload. */
		if (nh->nlmsg_type == RTM_NEWLINK || nh->nlmsg_type == RTM_DELLINK || nh->nlmsg_type == RTM_SETLINK) {
			struct ifinfomsg *ifinfo = (struct ifinfomsg *)NLMSG_DATA(nh);
			const char *ifname = if_indextoname(ifinfo->ifi_index, ifnamebuf);

#ifdef IFLA_LINKINFO
			struct rtattr *rta = IFLA_RTA(NLMSG_DATA(nh));
			int rta_len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
			for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
				if (rta->rta_type == IFLA_OPERSTATE || rta->rta_type == IFLA_LINKMODE) {
#endif
					if (ifinfo->ifi_flags & IFF_RUNNING) {
						dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, flags is running", ifname,
						     ifinfo->ifi_index);
					} else {
						dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, flags is *NOT* running", ifname,
						     ifinfo->ifi_index);
					}
#ifdef IFLA_LINKINFO
				}
			}
#endif
			/* Reinit the interfaces which need it. */
			struct Interface *iface;
			switch (nh->nlmsg_type) {
			case RTM_NEWLINK:
				iface = find_iface_by_name(ifaces, ifname);
				break;
			default:
				iface = find_iface_by_index(ifaces, ifinfo->ifi_index);
				break;
			}
			if (iface) {
				touch_iface(iface);
			}

		} else if (nh->nlmsg_type == RTM_NEWADDR || nh->nlmsg_type == RTM_DELADDR) {
			struct ifaddrmsg *ifaddr = (struct ifaddrmsg *)NLMSG_DATA(nh);
			const char *ifname = if_indextoname(ifaddr->ifa_index, ifnamebuf);

			switch (nh->nlmsg_type) {

			case RTM_DELADDR:
				dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, address deleted", ifname, ifaddr->ifa_index);
				break;

			case RTM_NEWADDR:
				dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, new address", ifname, ifaddr->ifa_index);
				break;

			default:
				flog(LOG_ERR, "netlink: unhandled event: %d", nh->nlmsg_type);
				break;
			}

			struct Interface *iface = find_iface_by_index(ifaces, ifaddr->ifa_index);
			if (iface) {
				struct in6_addr *if_addrs = NULL;
				int count = get_iface_addrs(iface->props.name, NULL, &if_addrs);

				if (count != iface->props.addrs_count ||
					0 != memcmp(if_addrs, iface->props.if_addrs, count * sizeof(struct in6_addr))) {
					dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, addresses are different",
						ifname, ifaddr->ifa_index);
					touch_iface(iface);
				} else {
					dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, addresses are the same",
						ifname, ifaddr->ifa_index);
				}
				free(if_addrs);
			}

		}
	}
}

int netlink_socket(void)
{
	int sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock == -1) {
		flog(LOG_ERR, "Unable to open netlink socket: %s", strerror(errno));
		return -1;
	}
#if defined SOL_NETLINK && defined NETLINK_NO_ENOBUFS
	else if (setsockopt(sock, SOL_NETLINK, NETLINK_NO_ENOBUFS, (int[]){1}, sizeof(int)) < 0) {
		flog(LOG_ERR, "Unable to setsockopt NETLINK_NO_ENOBUFS: %s", strerror(errno));
	}
#endif
	struct sockaddr_nl snl;
	memset(&snl, 0, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = RTMGRP_LINK | RTMGRP_IPV6_IFADDR;

	int rc = bind(sock, (struct sockaddr *)&snl, sizeof(snl));
	if (rc == -1) {
		flog(LOG_ERR, "Unable to bind netlink socket: %s", strerror(errno));
		close(sock);
		sock = -1;
	}

	return sock;
}
