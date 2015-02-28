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

void process_netlink_msg(int sock, struct Interface * ifaces)
{
	int rc = 0;

	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
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

			struct rtattr *rta = IFLA_RTA(NLMSG_DATA(nh));
			int rta_len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
			for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
				if (rta->rta_type == IFLA_OPERSTATE || rta->rta_type == IFLA_LINKMODE) {
					if (ifinfo->ifi_flags & IFF_RUNNING) {
						dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, flags is running", ifname,
						     ifinfo->ifi_index);
					} else {
						dlog(LOG_DEBUG, 3, "netlink: %s, ifindex %d, flags is *NOT* running", ifname,
						     ifinfo->ifi_index);
					}
					++rc;
				}
			}

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

			++rc;

			struct Interface *iface = find_iface_by_index(ifaces, ifaddr->ifa_index);
			if (iface) {
				touch_iface(iface);
			}
		}
	}
}

int netlink_socket(void)
{
	int sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock == -1) {
		flog(LOG_ERR, "Unable to open netlink socket: %s", strerror(errno));
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
