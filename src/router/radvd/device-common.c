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

/*
 * Saves the first link local address seen on the specified interface to iface->if_addr
 *
 */
int setup_linklocal_addr(struct Interface *iface)
{
	struct ifaddrs *addresses = 0;

	if (getifaddrs(&addresses) != 0) {
		flog(LOG_ERR, "getifaddrs failed on %s: %s", iface->props.name, strerror(errno));
	} else {
		for (struct ifaddrs * ifa = addresses; ifa != NULL; ifa = ifa->ifa_next) {

			if (!ifa->ifa_addr)
				continue;

			if (ifa->ifa_addr->sa_family != AF_INET6)
				continue;

			struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)ifa->ifa_addr;

			/* Skip if it is not a linklocal address */
			uint8_t const ll_prefix[] = { 0xfe, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
			if (memcmp(&(a6->sin6_addr), ll_prefix, sizeof(ll_prefix)) != 0)
				continue;

			/* Skip if it is not the interface we're looking for. */
			if (strcmp(ifa->ifa_name, iface->props.name) != 0)
				continue;

			memcpy(&iface->props.if_addr, &(a6->sin6_addr), sizeof(struct in6_addr));

			freeifaddrs(addresses);

			char addr_str[INET6_ADDRSTRLEN];
			addrtostr(&iface->props.if_addr, addr_str, sizeof(addr_str));
			dlog(LOG_DEBUG, 4, "%s linklocal address: %s", iface->props.name, addr_str);

			return 0;
		}
	}

	if (addresses)
		freeifaddrs(addresses);

	if (iface->IgnoreIfMissing)
		dlog(LOG_DEBUG, 4, "no linklocal address configured on %s", iface->props.name);
	else
		flog(LOG_ERR, "no linklocal address configured on %s", iface->props.name);

	return -1;
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

