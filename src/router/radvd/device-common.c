/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
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
#include "includes.h"
#include "radvd.h"
#include "defaults.h"
#include "pathnames.h"

int check_device(int sock, struct Interface *iface)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface->props.name, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFFLAGS) failed on %s: %s", iface->props.name, strerror(errno));
		return -1;
	} else {
		dlog(LOG_ERR, 5, "ioctl(SIOCGIFFLAGS) succeeded on %s", iface->props.name);
	}

	if (!(ifr.ifr_flags & IFF_UP)) {
		dlog(LOG_ERR, 4, "%s is not up", iface->props.name);
		return -1;
	} else {
		dlog(LOG_ERR, 4, "%s is up", iface->props.name);
	}

	if (!(ifr.ifr_flags & IFF_RUNNING)) {
		dlog(LOG_ERR, 4, "%s is not running", iface->props.name);
		return -1;
	} else {
		dlog(LOG_ERR, 4, "%s is running", iface->props.name);
	}

	if (!iface->UnicastOnly && !(ifr.ifr_flags & IFF_MULTICAST)) {
		flog(LOG_INFO, "%s does not support multicast, forcing UnicastOnly", iface->props.name);
		iface->UnicastOnly = 1;
	} else {
		dlog(LOG_ERR, 4, "%s supports multicast", iface->props.name);
	}

	return 0;
}

int get_v4addr(const char *ifn, unsigned int *dst)
{

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		flog(LOG_ERR, "create socket for IPv4 ioctl failed on %s: %s", ifn, strerror(errno));
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifn, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFADDR) failed on %s: %s", ifn, strerror(errno));
		close(fd);
		return -1;
	}

	struct sockaddr_in *addr = (struct sockaddr_in *)(&ifr.ifr_addr);

	dlog(LOG_DEBUG, 3, "%s IPv4 address is: %s", ifn, inet_ntoa(addr->sin_addr));

	*dst = addr->sin_addr.s_addr;

	close(fd);

	return 0;
}


static int cmp_iface_addrs(void const *a, void const *b)
{
	return memcmp(a, b, sizeof(struct in6_addr));
}

/*
 * Return first IPv6 link local addr in if_addr.
 * Return all the IPv6 addresses in if_addrs in ascending
 * order.
 * Return value is -1 if there was no link local addr.
 * otherwise return value is count of addres in if_addrs
 * not including the all zero (unspecified) addr at the
 * end of the list.
 */
int get_iface_addrs(char const *name, struct in6_addr *if_addr, struct in6_addr **if_addrs)
{
	struct ifaddrs *addresses = 0;
	int link_local_set = 0;
	int i = 0;

	if (getifaddrs(&addresses) != 0) {
		flog(LOG_ERR, "getifaddrs failed on %s: %s", name, strerror(errno));
	} else {
		for (struct ifaddrs * ifa = addresses; ifa != NULL; ifa = ifa->ifa_next) {

			if (!ifa->ifa_addr)
				continue;

			if (ifa->ifa_addr->sa_family != AF_INET6)
				continue;

			struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)ifa->ifa_addr;

			/* Skip if it is not the interface we're looking for. */
			if (strcmp(ifa->ifa_name, name) != 0)
				continue;

			*if_addrs = realloc(*if_addrs, (i+1) * sizeof(struct in6_addr));
			(*if_addrs)[i++] = a6->sin6_addr;

			/* Skip if it is not a linklocal address or link locak address already found*/
			uint8_t const ll_prefix[] = { 0xfe, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
			if (link_local_set || 0 != memcmp(&(a6->sin6_addr), ll_prefix, sizeof(ll_prefix)))
				continue;

			if (if_addr)
				memcpy(if_addr, &(a6->sin6_addr), sizeof(struct in6_addr));

			link_local_set = 1;
		}
	}

	if (addresses)
		freeifaddrs(addresses);

	/* last item in the list is all zero (unspecified) address */
	*if_addrs = realloc(*if_addrs, (i+1) * sizeof(struct in6_addr));
	memset(&(*if_addrs)[i], 0, sizeof(struct in6_addr));

	/* Sort the addresses so the output is predictable. */
	qsort(*if_addrs, i, sizeof(struct in6_addr), cmp_iface_addrs);

	if (!link_local_set)
		return -1;

	return i;
}


/*
 * Saves the first link local address seen on the specified interface to iface->if_addr
 * and builds a list of all the other addrs.
 */
int setup_iface_addrs(struct Interface *iface)
{
	int rc = get_iface_addrs(iface->props.name, &iface->props.if_addr, &iface->props.if_addrs);

	if (-1 != rc) {
		iface->props.addrs_count = rc;
		char addr_str[INET6_ADDRSTRLEN];
		addrtostr(&iface->props.if_addr, addr_str, sizeof(addr_str));
		dlog(LOG_DEBUG, 4, "%s linklocal address: %s", iface->props.name, addr_str);
		for (int i = 0; i < rc; ++i) {
			addrtostr(&iface->props.if_addrs[i], addr_str, sizeof(addr_str));
			dlog(LOG_DEBUG, 4, "%s address: %s", iface->props.name, addr_str);
		}
	} else {
		if (iface->IgnoreIfMissing)
			dlog(LOG_DEBUG, 4, "no linklocal address configured on %s", iface->props.name);
		else
			flog(LOG_ERR, "no linklocal address configured on %s", iface->props.name);
	}

	return rc;
}

int update_device_index(struct Interface *iface)
{
	int index = if_nametoindex(iface->props.name);

	if (0 == index) {
		/* Yes, if_nametoindex returns zero on failure.  2014/01/16 */
		flog(LOG_ERR, "%s not found: %s", iface->props.name, strerror(errno));
		return -1;
	}

	if (iface->props.if_index != index) {
		dlog(LOG_DEBUG, 4, "%s if_index changed from %d to %d", iface->props.name, iface->props.if_index, index);
		iface->props.if_index = index;
	}

	return 0;
}

