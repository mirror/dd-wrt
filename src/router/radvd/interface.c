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
#include "defaults.h"
#include "includes.h"
#include "radvd.h"

#define IFACE_SETUP_DELAY 1

void iface_init_defaults(struct Interface *iface)
{
	memset(iface, 0, sizeof(struct Interface));

	iface->state_info.changed = 1;

	iface->IgnoreIfMissing = DFLT_IgnoreIfMissing;
	iface->AdvSendAdvert = DFLT_AdvSendAdv;
	iface->MaxRtrAdvInterval = DFLT_MaxRtrAdvInterval;
	iface->AdvSourceLLAddress = DFLT_AdvSourceLLAddress;
	iface->MinDelayBetweenRAs = DFLT_MinDelayBetweenRAs;
	iface->MinRtrAdvInterval = -1;
	iface->UnicastOnly = DFLT_UnicastOnly;
	iface->AdvRASolicitedUnicast = DFLT_AdvRASolicitedUnicast;

	iface->ra_header_info.AdvDefaultPreference = DFLT_AdvDefaultPreference;
	iface->ra_header_info.AdvDefaultLifetime = -1;
	iface->ra_header_info.AdvReachableTime = DFLT_AdvReachableTime;
	iface->ra_header_info.AdvRetransTimer = DFLT_AdvRetransTimer;
	iface->ra_header_info.AdvCurHopLimit = DFLT_AdvCurHopLimit;
	iface->ra_header_info.AdvHomeAgentFlag = DFLT_AdvHomeAgentFlag;

	iface->mipv6.AdvIntervalOpt = DFLT_AdvIntervalOpt;
	iface->mipv6.AdvHomeAgentInfo = DFLT_AdvHomeAgentInfo;
	iface->mipv6.HomeAgentPreference = DFLT_HomeAgentPreference;
	iface->mipv6.AdvMobRtrSupportFlag = DFLT_AdvMobRtrSupportFlag;
	iface->mipv6.HomeAgentLifetime = -1;

	iface->AdvLinkMTU = DFLT_AdvLinkMTU;
	iface->AdvRAMTU = DFLT_AdvRAMTU;
}

void touch_iface(struct Interface *iface)
{
	iface->state_info.changed = 1;
	iface->state_info.ready = 0;
	iface->state_info.racount = 0;
	reschedule_iface(iface, 0);
}

int setup_iface(int sock, struct Interface *iface)
{
	iface->state_info.changed = 0;
	iface->state_info.ready = 0;

	/* The device index must be setup first so we can search it later */
	if (update_device_index(iface) < 0) {
		return -1;
	}

	/* Check IFF_UP, IFF_RUNNING and IFF_MULTICAST */
	if (check_device(sock, iface) < 0) {
		return -1;
	}

	/* Set iface->max_mtu and iface hardware address */
	if (update_device_info(sock, iface) < 0) {
		return -1;
	}

	/* Make sure the settings in the config file for this interface are ok (this depends
	 * on iface->max_mtu already being set). */
	if (check_iface(iface) < 0) {
		return -1;
	}

	/* Save the first link local address seen on the specified interface to
	 * iface->props.if_addr and keep a list off all addrs in iface->props.if_addrs */
	if (setup_iface_addrs(iface) < 0) {
		return -1;
	}

	/* Check if we a usable RA source address */
	if (iface->props.if_addr_rasrc == NULL) {
		dlog(LOG_DEBUG, 5, "no configured AdvRASrcAddress present, skipping send");
		return -1;
	}

	/* join the allrouters multicast group so we get the solicitations */
	if (setup_allrouters_membership(sock, iface) < 0) {
		return -1;
	}

	iface->state_info.ready = 1;

	dlog(LOG_DEBUG, 4, "%s is ready", iface->props.name);

	return 0;
}

void prefix_init_defaults(struct AdvPrefix *prefix)
{
	memset(prefix, 0, sizeof(struct AdvPrefix));

	prefix->AdvOnLinkFlag = DFLT_AdvOnLinkFlag;
	prefix->AdvAutonomousFlag = DFLT_AdvAutonomousFlag;
	prefix->AdvRouterAddr = DFLT_AdvRouterAddr;
	prefix->AdvValidLifetime = DFLT_AdvValidLifetime;
	prefix->AdvPreferredLifetime = DFLT_AdvPreferredLifetime;
	prefix->DeprecatePrefixFlag = DFLT_DeprecatePrefixFlag;
	prefix->DecrementLifetimesFlag = DFLT_DecrementLifetimesFlag;

	prefix->curr_validlft = prefix->AdvValidLifetime;
	prefix->curr_preferredlft = prefix->AdvPreferredLifetime;
}

void route_init_defaults(struct AdvRoute *route, struct Interface *iface)
{
	memset(route, 0, sizeof(struct AdvRoute));

	route->AdvRouteLifetime = DFLT_AdvRouteLifetime(iface);
	route->AdvRoutePreference = DFLT_AdvRoutePreference;
	route->RemoveRouteFlag = DFLT_RemoveRouteFlag;
}

void rdnss_init_defaults(struct AdvRDNSS *rdnss, struct Interface *iface)
{
	memset(rdnss, 0, sizeof(struct AdvRDNSS));

	rdnss->AdvRDNSSLifetime = DFLT_AdvRDNSSLifetime(iface);
	rdnss->AdvRDNSSNumber = 0;
	rdnss->FlushRDNSSFlag = DFLT_FlushRDNSSFlag;
}

void dnssl_init_defaults(struct AdvDNSSL *dnssl, struct Interface *iface)
{
	memset(dnssl, 0, sizeof(struct AdvDNSSL));

	dnssl->AdvDNSSLLifetime = DFLT_AdvDNSSLLifetime(iface);
	dnssl->FlushDNSSLFlag = DFLT_FlushDNSSLFlag;
}

int check_iface(struct Interface *iface)
{
	int res = 0;
	int MIPv6 = 0;
	struct in6_addr zeroaddr;
	memset(&zeroaddr, 0, sizeof(zeroaddr));

	/* Check if we use Mobile IPv6 extensions */
	if (iface->ra_header_info.AdvHomeAgentFlag || iface->mipv6.AdvHomeAgentInfo || iface->mipv6.AdvIntervalOpt) {
		MIPv6 = 1;
		flog(LOG_INFO, "using Mobile IPv6 extensions");
	}

	/* Check forwarding on interface */
	if (check_ip6_iface_forwarding(iface->props.name) < 1) {
		flog(LOG_WARNING, "IPv6 forwarding on interface seems to be disabled, but continuing anyway");
	}

	struct AdvPrefix *prefix = iface->AdvPrefixList;
	while (!MIPv6 && prefix) {
		if (prefix->AdvRouterAddr) {
			MIPv6 = 1;
		}
		prefix = prefix->next;
	}

	if (iface->MinRtrAdvInterval < 0)
		iface->MinRtrAdvInterval = DFLT_MinRtrAdvInterval(iface);

	if ((iface->MinRtrAdvInterval < (MIPv6 ? MIN_MinRtrAdvInterval_MIPv6 : MIN_MinRtrAdvInterval)) ||
	    (iface->MinRtrAdvInterval > MAX_MinRtrAdvInterval(iface))) {
		flog(LOG_ERR,
		     "MinRtrAdvInterval for %s (%.2f) must be at least %.2f but no more than 3/4 of MaxRtrAdvInterval (%.2f)",
		     iface->props.name, iface->MinRtrAdvInterval,
		     MIPv6 ? MIN_MinRtrAdvInterval_MIPv6 : (int)MIN_MinRtrAdvInterval, MAX_MinRtrAdvInterval(iface));
		res = -1;
	}

	if ((iface->MaxRtrAdvInterval < (MIPv6 ? MIN_MaxRtrAdvInterval_MIPv6 : MIN_MaxRtrAdvInterval)) ||
	    (iface->MaxRtrAdvInterval > MAX_MaxRtrAdvInterval)) {
		flog(LOG_ERR, "MaxRtrAdvInterval for %s (%.2f) must be between %.2f and %d", iface->props.name,
		     iface->MaxRtrAdvInterval, MIPv6 ? MIN_MaxRtrAdvInterval_MIPv6 : (int)MIN_MaxRtrAdvInterval,
		     MAX_MaxRtrAdvInterval);
		res = -1;
	}

	if (iface->MinDelayBetweenRAs < (MIPv6 ? MIN_DELAY_BETWEEN_RAS_MIPv6 : MIN_DELAY_BETWEEN_RAS)) {
		flog(LOG_ERR, "MinDelayBetweenRAs for %s (%.2f) must be at least %.2f", iface->props.name,
		     iface->MinDelayBetweenRAs, MIPv6 ? MIN_DELAY_BETWEEN_RAS_MIPv6 : MIN_DELAY_BETWEEN_RAS);
		res = -1;
	}

	if ((iface->AdvLinkMTU != 0) && ((iface->AdvLinkMTU < MIN_AdvLinkMTU) ||
					 (iface->sllao.if_maxmtu != -1 && (iface->AdvLinkMTU > iface->sllao.if_maxmtu)))) {
		flog(LOG_ERR, "AdvLinkMTU for %s (%u) must be zero or between %u and %u", iface->props.name, iface->AdvLinkMTU,
		     MIN_AdvLinkMTU, iface->sllao.if_maxmtu);
		res = -1;
	}

	if (iface->ra_header_info.AdvReachableTime > MAX_AdvReachableTime) {
		flog(LOG_ERR, "AdvReachableTime for %s (%u) must not be greater than %u", iface->props.name,
		     iface->ra_header_info.AdvReachableTime, MAX_AdvReachableTime);
		res = -1;
	}

	if (iface->ra_header_info.AdvDefaultLifetime < 0)
		iface->ra_header_info.AdvDefaultLifetime = DFLT_AdvDefaultLifetime(iface);

	if ((iface->ra_header_info.AdvDefaultLifetime != 0) &&
	    ((iface->ra_header_info.AdvDefaultLifetime > MAX_AdvDefaultLifetime) ||
	     (iface->ra_header_info.AdvDefaultLifetime < MIN_AdvDefaultLifetime(iface)))) {
		flog(LOG_ERR, "AdvDefaultLifetime for %s (%u) must be zero or between %u and %u", iface->props.name,
		     iface->ra_header_info.AdvDefaultLifetime, (int)MIN_AdvDefaultLifetime(iface), MAX_AdvDefaultLifetime);
		res = -1;
	}

	/* Mobile IPv6 ext */
	if (iface->mipv6.HomeAgentLifetime < 0)
		iface->mipv6.HomeAgentLifetime = DFLT_HomeAgentLifetime(iface);

	/* Mobile IPv6 ext */
	if (iface->mipv6.AdvHomeAgentInfo) {
		if ((iface->mipv6.HomeAgentLifetime > MAX_HomeAgentLifetime) ||
		    (iface->mipv6.HomeAgentLifetime < MIN_HomeAgentLifetime)) {
			flog(LOG_ERR, "HomeAgentLifetime for %s (%u) must be between %u and %u", iface->props.name,
			     iface->mipv6.HomeAgentLifetime, MIN_HomeAgentLifetime, MAX_HomeAgentLifetime);
			res = -1;
		}
	}

	/* Mobile IPv6 ext */
	if (iface->mipv6.AdvHomeAgentInfo && !(iface->ra_header_info.AdvHomeAgentFlag)) {
		flog(LOG_ERR, "AdvHomeAgentFlag for %s must be set with HomeAgentInfo", iface->props.name);
		res = -1;
	}
	if (iface->mipv6.AdvMobRtrSupportFlag && !(iface->mipv6.AdvHomeAgentInfo)) {
		flog(LOG_ERR, "AdvHomeAgentInfo for %s must be set with AdvMobRtrSupportFlag", iface->props.name);
		res = -1;
	}

	/* XXX: need this? prefix = iface->AdvPrefixList; */

	while (prefix) {
		if (prefix->PrefixLen > MAX_PrefixLen) {
			flog(LOG_ERR, "invalid prefix length (%u) for %s", prefix->PrefixLen, iface->props.name);
			res = -1;
		}

		if (prefix->AdvPreferredLifetime > prefix->AdvValidLifetime) {
			flog(LOG_ERR, "AdvValidLifetime for %s (%u) must be "
				      "greater than AdvPreferredLifetime for",
			     iface->props.name, prefix->AdvValidLifetime);
			res = -1;
		}

		prefix = prefix->next;
	}

	struct AdvRoute *route = iface->AdvRouteList;
	while (route) {
		if (route->PrefixLen > MAX_PrefixLen) {
			flog(LOG_ERR, "invalid route prefix length (%u) for %s", route->PrefixLen, iface->props.name);
			res = -1;
		}

		/* For the default route 0::/0, we need to explicitly check the
		 * lifetime against the AdvDefaultLifetime value.
		 *
		 * If exactly one of the two has a zero value, then nodes processing
		 * the RA may have a flap in their default route.
		 *
		 * AdvDefaultLifetime == 0 && route.AdvRouteLifetime > 0:
		 * - default route is deleted and then re-added.
		 * AdvDefaultLifetime > 0 && route.AdvRouteLifetime == 0:
		 * - default route is added and then deleted.
		 */
		if (IN6_IS_ADDR_UNSPECIFIED(&(route->Prefix))) {
			int route_zerolife = (route->AdvRouteLifetime == 0);
			int defaultroute_zerolife = (iface->ra_header_info.AdvDefaultLifetime == 0);
			if (route_zerolife ^ defaultroute_zerolife) {
				flog(
				    LOG_ERR,
				    "route 0::/0 lifetime (%u) conflicts with AdvDefaultLifetime (%u), default routes will flap!",
				    route->AdvRouteLifetime, iface->ra_header_info.AdvDefaultLifetime);
				// res = -1; // In some future version, abort on this configuration error.
			}
		}

		route = route->next;
	}

	return res;
}

struct Interface *find_iface_by_index(struct Interface *iface, int index)
{
	for (; iface; iface = iface->next) {
		if (iface->props.if_index == index) {
			return iface;
		}
	}

	return 0;
}

struct Interface *find_iface_by_name(struct Interface *iface, const char *name)
{
	if (!name) {
		return 0;
	}

	for (; iface; iface = iface->next) {
		if (strcmp(iface->props.name, name) == 0) {
			return iface;
		}
	}

	return 0;
}

struct Interface *find_iface_by_time(struct Interface *iface)
{
	if (!iface) {
		return 0;
	}

	int timeout = next_time_msec(iface);
	struct Interface *next = iface;

	for (iface = iface->next; iface; iface = iface->next) {
		int t = next_time_msec(iface);
		if (timeout > t) {
			timeout = t;
			next = iface;
		}
	}

	return next;
}

void reschedule_iface(struct Interface *iface, double next)
{
#ifdef HAVE_NETLINK
	if (!iface->state_info.changed && !iface->state_info.ready) {
		next = 10 * iface->MaxRtrAdvInterval;
	} else if (next == 0) {
		next = IFACE_SETUP_DELAY;
	} else
#endif
	    if (iface->state_info.racount < MAX_INITIAL_RTR_ADVERTISEMENTS) {
		next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, iface->MaxRtrAdvInterval);
	}

	dlog(LOG_DEBUG, 5, "%s next scheduled RA in %g second(s)", iface->props.name, next);

	iface->times.next_multicast = next_timespec(next);
}

void for_each_iface(struct Interface *ifaces, void (*foo)(struct Interface *, void *), void *data)
{
	for (; ifaces; ifaces = ifaces->next) {
		foo(ifaces, data);
	}
}

static void free_iface_list(struct Interface *iface)
{
	while (iface) {
		struct Interface *next_iface = iface->next;

		dlog(LOG_DEBUG, 4, "freeing interface %s", iface->props.name);

		struct AdvPrefix *prefix = iface->AdvPrefixList;
		while (prefix) {
			struct AdvPrefix *next_prefix = prefix->next;

			free(prefix);
			prefix = next_prefix;
		}

		struct AdvRoute *route = iface->AdvRouteList;
		while (route) {
			struct AdvRoute *next_route = route->next;

			free(route);
			route = next_route;
		}

		struct AdvRDNSS *rdnss = iface->AdvRDNSSList;
		while (rdnss) {
			struct AdvRDNSS *next_rdnss = rdnss->next;

			free(rdnss);
			rdnss = next_rdnss;
		}

		struct AdvDNSSL *dnssl = iface->AdvDNSSLList;
		while (dnssl) {
			struct AdvDNSSL *next_dnssl = dnssl->next;

			for (int i = 0; i < dnssl->AdvDNSSLNumber; i++)
				free(dnssl->AdvDNSSLSuffixes[i]);
			free(dnssl->AdvDNSSLSuffixes);
			free(dnssl);

			dnssl = next_dnssl;
		}

		struct Clients *clients = iface->ClientList;
		while (clients) {
			struct Clients *next_client = clients->next;

			free(clients);
			clients = next_client;
		}

		free(iface->props.if_addrs);

		free(iface);
		iface = next_iface;
	}
}

void free_ifaces(struct Interface *ifaces)
{
	dlog(LOG_DEBUG, 3, "Freeing Interfaces");

	free_iface_list(ifaces);
}
