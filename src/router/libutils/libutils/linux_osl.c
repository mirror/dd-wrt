/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_osl.c,v 1.5 2005/03/07 08:35:32 kanki Exp $
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include <linux_osl.h>
#include <time.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>

int ifconfig(char *name, int flags, char *addr, char *netmask)
{
	if (!strcmp(name, "wwan0")) {
		return 0;
	}
	// char *down="down";
	// if (flags == IFUP)
	// down = "up";
	cprintf("ifconfig %s = %s/%s\n", name, addr, netmask);
	if (!ifexists(name)) {
		cprintf("interface %s does not exists, ignoring\n", name);
		return -1;
	}
	// if (addr==NULL)
	// addr="0.0.0.0";
	// int ret;
	// if (netmask==NULL)
	// {
	// ret = eval("ifconfig",name,addr,down);
	// }else
	// {
	// ret = eval("ifconfig",name,addr,"netmask",netmask,down);
	// }
	int s;
	struct ifreq ifr;
	struct in_addr in_addr, in_netmask, in_broadaddr;

	cprintf("ifconfig(): name=[%s] flags=[%s] addr=[%s] netmask=[%s]\n",
		name, flags == IFUP ? "IFUP" : "0", addr, netmask);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		goto err2; // override socket close
	cprintf("ifconfig(): socket opened\n");

	strlcpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	cprintf("ifconfig(): set interface name\n");
	if (flags) {
		ifr.ifr_flags = flags;
		if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig(): interface flags configured\n");
	if (addr) {
		inet_aton(addr, &in_addr);
		sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig() ip configured\n");

	if (addr && netmask) {
		inet_aton(netmask, &in_netmask);
		sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto err;

		in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) |
				      ~in_netmask.s_addr;
		sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
		ifr.ifr_broadaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig() mask configured\n");

	close(s);
	cprintf("ifconfig() done()\n");
	return 0;

err:
	cprintf("ifconfig() done with error\n");
	close(s);
err2:
#ifndef HAVE_SILENCE
	perror(name);
#endif
	return errno;
	// return ret;
}

struct iface {
	struct iface *next;

	char *ifname;

	// The address of the HTTP server
	struct in_addr inaddr;

	struct net_connection *http_connection;
	struct net_connection *ssdp_connection;
};

#if !defined(FALSE) || !defined(TRUE)
#define TRUE 1
#define FALSE (!TRUE)
#endif

static short osl_ifflags(const char *ifname); // - tofu

struct in_addr *osl_ifaddr(const char *ifname, struct in_addr *inaddr)
{
	int sockfd;
	struct ifreq ifreq;

	if (!(osl_ifflags(ifname) & IFF_UP))
		return NULL;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return NULL;
	}
	strlcpy(ifreq.ifr_name, ifname, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
		inaddr = NULL;
	} else {
		memcpy(inaddr,
		       &(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr),
		       sizeof(struct in_addr));
	}
	close(sockfd);
	return inaddr;
}

static short osl_ifflags(const char *ifname)
{
	int sockfd;
	struct ifreq ifreq;
	short flags = 0;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return flags;
	}

	strlcpy(ifreq.ifr_name, ifname, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifreq) < 0) {
		flags = 0;
	} else {
		flags = ifreq.ifr_flags;
	}
	close(sockfd);
	return flags;
}

#if 0
int osl_join_multicast(struct iface *pif, int fd, unsigned int ipaddr, ushort port)
{
	struct ip_mreqn mcreqn;
	struct ifreq ifreq;
	struct sockaddr_in mcaddr;
	int success = FALSE;
	int flag;

	do {

		// make sure this interface is capable of MULTICAST...
		bzero(&ifreq, sizeof(ifreq));
		strcpy(ifreq.ifr_name, pif->ifname);
		if (ioctl(fd, SIOCGIFFLAGS, (int)&ifreq))
			break;

		if ((ifreq.ifr_flags & IFF_MULTICAST) == 0)
			break;

		// bind the socket to an address and port.
		flag = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));

		bzero(&mcaddr, sizeof(mcaddr));
		// memcpy(&mcaddr.sin_addr, &pif->inaddr, sizeof(mcaddr.sin_addr));
		mcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		mcaddr.sin_family = AF_INET;
		mcaddr.sin_port = htons(port);
		if (bind(fd, (struct sockaddr *)&mcaddr, sizeof(mcaddr)))
			break;

		// join the multicast group.
		bzero(&ifreq, sizeof(ifreq));
		strcpy(ifreq.ifr_name, pif->ifname);
		if (ioctl(fd, SIOCGIFINDEX, &ifreq))
			break;

		bzero(&mcreqn, sizeof(mcreqn));
		mcreqn.imr_multiaddr.s_addr = ipaddr;
		// mcreqn.imr_interface.s_addr = mcaddr.sin_addr.s_addr;
		// if we get to use struct ip_mreqn, delete the previous line and
		// uncomment the next two
		mcreqn.imr_address.s_addr = mcaddr.sin_addr.s_addr;
		mcreqn.imr_ifindex = ifreq.ifr_ifindex;
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mcreqn, sizeof(mcreqn)))
			break;

		// restrict multicast messages sent on this socket 
		// to only go out this interface and no other
		// (doesn't say anything about multicast receives.)
		// 
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&pif->inaddr, sizeof(pif->inaddr)))
			break;

		success = TRUE;

	}
	while (0);

	// TRUE == success, FALSE otherwise.
	return success;
}
#endif
char *safe_snprintf(char *str, int *len, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(str, *len, fmt, ap);
	va_end(ap);

	if (n > 0) {
		str += n;
		*len -= n;
	} else if (n < 0) {
		*len = 0;
	}

	return str;
}
