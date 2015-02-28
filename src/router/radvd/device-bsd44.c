/*
 *
 *   Authors:
 *    Craig Metz		<cmetz@inner.net>
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

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int update_device_info(int sock, struct Interface *iface)
{
	struct ifaddrs *addresses = 0;

	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface->props.name, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFMTU) failed for %s: %s", iface->props.name, strerror(errno));
		goto ret;
	}

	dlog(LOG_DEBUG, 3, "mtu for %s is %d", iface->props.name, ifr.ifr_mtu);
	iface->sllao.if_maxmtu = ifr.ifr_mtu;

	if (getifaddrs(&addresses) != 0) {
		flog(LOG_ERR, "getifaddrs failed: %s(%d)", strerror(errno), errno);
		goto ret;
	}

	for (struct ifaddrs * ifa = addresses; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, iface->props.name) != 0)
			continue;

		if (ifa->ifa_addr == NULL)
			continue;

		if (ifa->ifa_addr->sa_family != AF_LINK)
			continue;

		struct sockaddr_dl *dl = (struct sockaddr_dl *)ifa->ifa_addr;

		if (dl->sdl_alen > sizeof(iface->props.if_addr)) {
			flog(LOG_ERR, "address length %d too big for %s", dl->sdl_alen, iface->props.name);
			goto ret;
		}

		memcpy(iface->sllao.if_hwaddr, LLADDR(dl), dl->sdl_alen);
		iface->sllao.if_hwaddr_len = dl->sdl_alen << 3;

		switch (dl->sdl_type) {
		case IFT_ETHER:
		case IFT_ISO88023:
			iface->sllao.if_prefix_len = 64;
			break;
		case IFT_FDDI:
			iface->sllao.if_prefix_len = 64;
			break;
		default:
			iface->sllao.if_prefix_len = -1;
			iface->sllao.if_maxmtu = -1;
			break;
		}

		dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->props.name, iface->sllao.if_hwaddr_len);

		dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->props.name, iface->sllao.if_prefix_len);

		if (iface->sllao.if_prefix_len != -1) {
			char zero[sizeof(iface->props.if_addr)];
			memset(zero, 0, dl->sdl_alen);
			if (!memcmp(iface->sllao.if_hwaddr, zero, dl->sdl_alen))
				flog(LOG_WARNING, "WARNING, MAC address on %s is all zero!", iface->props.name);
		}

		struct AdvPrefix *prefix = iface->AdvPrefixList;
		while (prefix) {
			if ((iface->sllao.if_prefix_len != -1) && (iface->sllao.if_prefix_len != prefix->PrefixLen)) {
				flog(LOG_WARNING, "prefix length should be %d for %s", iface->sllao.if_prefix_len, iface->props.name);
			}

			prefix = prefix->next;
		}

		freeifaddrs(addresses);
		return 0;
	}

 ret:
	iface->sllao.if_maxmtu = -1;
	iface->sllao.if_hwaddr_len = -1;
	iface->sllao.if_prefix_len = -1;

	if (addresses != 0)
		freeifaddrs(addresses);

	return -1;
}

int setup_allrouters_membership(int sock, struct Interface *iface)
{
	return 0;
}

int set_interface_linkmtu(const char *iface, uint32_t mtu)
{
	dlog(LOG_DEBUG, 4, "setting LinkMTU (%u) for %s is not supported", mtu, iface);
	return -1;
}

int set_interface_curhlim(const char *iface, uint8_t hlim)
{
	dlog(LOG_DEBUG, 4, "setting CurHopLimit (%u) for %s is not supported", hlim, iface);
	return -1;
}

int set_interface_reachtime(const char *iface, uint32_t rtime)
{
	dlog(LOG_DEBUG, 4, "setting BaseReachableTime (%u) for %s is not supported", rtime, iface);
	return -1;
}

int set_interface_retranstimer(const char *iface, uint32_t rettimer)
{
	dlog(LOG_DEBUG, 4, "setting RetransTimer (%u) for %s is not supported", rettimer, iface);
	return -1;
}

int check_ip6_forwarding(void)
{
	dlog(LOG_DEBUG, 4, "checking ipv6 forwarding not supported");
	return 0;
}
