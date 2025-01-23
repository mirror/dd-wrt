/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
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
#include "netlink.h"

static int really_send(int sock, struct in6_addr const *dest, struct properties const *props, struct safe_buffer const *sb);
static int send_ra(int sock, struct Interface *iface, struct in6_addr const *dest);
static struct safe_buffer_list *build_ra_options(struct Interface const *iface, struct in6_addr const *dest);

static int ensure_iface_setup(int sock, struct Interface *iface);
static void decrement_lifetime(const time_t secs, uint32_t *lifetime);
static void update_iface_times(struct Interface *iface);

// Option helpers
static size_t serialize_domain_names(struct safe_buffer *safe_buffer, struct AdvDNSSL const *dnssl);
static int get_prefix_lifetimes(struct AdvPrefix const *prefix, unsigned int *valid_lft, unsigned int *preferred_lft);
static void limit_prefix_lifetimes(struct AdvPrefix *prefix);

// Options that only need a single block
static void add_ra_header(struct safe_buffer *sb, struct ra_header_info const *ra_header_info, int cease_adv);
static void add_ra_option_prefix(struct safe_buffer *sb, struct AdvPrefix const *prefix, int cease_adv);
static void add_ra_option_mtu(struct safe_buffer *sb, uint32_t AdvLinkMTU);
static void add_ra_option_sllao(struct safe_buffer *sb, struct sllao const *sllao);
static void add_ra_option_mipv6_rtr_adv_interval(struct safe_buffer *sb, double MaxRtrAdvInterval);
static void add_ra_option_mipv6_home_agent_info(struct safe_buffer *sb, struct mipv6 const *mipv6);
static void add_ra_option_lowpanco(struct safe_buffer *sb, struct AdvLowpanCo const *lowpanco);
static void add_ra_option_abro(struct safe_buffer *sb, struct AdvAbro const *abroo);
static void add_ra_option_capport(struct safe_buffer *sb, const char *captive_portal);

// Options that generate 0 or more blocks
static struct safe_buffer_list *add_ra_options_prefix(struct safe_buffer_list *sbl, struct Interface const *iface,
						      char const *ifname, struct AdvPrefix const *prefix, int cease_adv,
						      struct in6_addr const *dest);
static struct safe_buffer_list *add_ra_options_nat64prefix(struct safe_buffer_list *sbl, struct NAT64Prefix const *prefix);
static struct safe_buffer_list *add_ra_options_route(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvRoute const *route, int cease_adv, struct in6_addr const *dest);
static struct safe_buffer_list *add_ra_options_rdnss(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvRDNSS const *rdnss, int cease_adv, struct in6_addr const *dest);
static struct safe_buffer_list *add_ra_options_dnssl(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvDNSSL const *dnssl, int cease_adv, struct in6_addr const *dest);

// Scheduling of options per RFC7772
static int schedule_helper(struct in6_addr const *dest, struct Interface const *iface, int option_lifetime);
static int schedule_option_prefix(struct in6_addr const *dest, struct Interface const *iface, struct AdvPrefix const *prefix);
static int schedule_option_route(struct in6_addr const *dest, struct Interface const *iface, struct AdvRoute const *route);
static int schedule_option_rdnss(struct in6_addr const *dest, struct Interface const *iface, struct AdvRDNSS const *rdnss);
static int schedule_option_dnssl(struct in6_addr const *dest, struct Interface const *iface, struct AdvDNSSL const *dnssl);
static int schedule_option_mtu(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_sllao(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_mipv6_rtr_adv_interval(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_mipv6_home_agent_info(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_lowpanco(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_abro(struct in6_addr const *dest, struct Interface const *iface);
static int schedule_option_capport(struct in6_addr const *dest, struct Interface const *iface);

#ifdef UNIT_TEST
#include "test/send.c"
#endif

/*
 * Sends an advertisement for all specified clients of this interface
 * (or via broadcast, if there are no restrictions configured).
 *
 * If a destination address is given, the RA will be sent to the destination
 * address only, but only if it was configured.
 *
 */
int send_ra_forall(int sock, struct Interface *iface, struct in6_addr *dest)
{
	/* when netlink is not available (disabled or BSD), ensure_iface_setup is necessary. */
	if (ensure_iface_setup(sock, iface) < 0) {
		dlog(LOG_DEBUG, 3, "not sending RA for %s, interface is not ready", iface->props.name);
		return -1;
	}

	// Ignore unicast request/response - otherwise rapid unicast
	// requests during startup can cause multicast/broadcast RAs to *NOT* be
	// sent on the desired schedule.
	// racount is consumed in interface.c to calculate when to send the
	// next non-unicast RA.
	if (iface->state_info.racount < MAX_INITIAL_RTR_ADVERTISEMENTS && dest == NULL)
		iface->state_info.racount++;

	/* If no list of clients was specified for this interface, we broadcast */
	if (iface->ClientList == NULL) {
		if (dest == NULL && iface->UnicastOnly) {
			dlog(LOG_DEBUG, 5, "no client list, no destination, unicast only...doing nothing");
			return 0;
		}
		return send_ra(sock, iface, dest);
	}

	/* If clients are configured, send the advertisement to all of them via unicast */
	for (struct Clients *current = iface->ClientList; current; current = current->next) {
		/* If a non-authorized client sent a solicitation, ignore it (logging later) */
		if (dest != NULL && memcmp(dest, &current->Address, sizeof(struct in6_addr)) != 0)
			continue;

		/* Clients that should be ignored */
		if (current->ignored) {
			/* Don't allow fallback to UnrestrictedUnicast for direct queries */
			if (dest != NULL)
				return 0;

			continue;
		}

		send_ra(sock, iface, &(current->Address));

		/* If we should only send the RA to a specific address, we are done */
		if (dest != NULL)
			return 0;
	}

	if (dest == NULL)
		return 0;

	/* Reply with advertisement to unlisted clients */
	if (iface->UnrestrictedUnicast) {
		return send_ra(sock, iface, dest);
	}

	/* If we refused a client's solicitation, log it if debugging is high enough */
	if (get_debuglevel() >= 5) {
		char address_text[INET6_ADDRSTRLEN] = {""};
		addrtostr(dest, address_text, INET6_ADDRSTRLEN);
		dlog(LOG_DEBUG, 5, "Not answering request from %s, not configured", address_text);
	}

	return 0;
}

/********************************************************************************
*       support functions                                                       *
********************************************************************************/

static int ensure_iface_setup(int sock, struct Interface *iface)
{
#ifdef HAVE_NETLINK
	if (iface->state_info.changed)
		setup_iface(sock, iface);
#else
	setup_iface(sock, iface);
#endif

	return (iface->state_info.ready ? 0 : -1);
}

static void decrement_lifetime(const time_t secs, uint32_t *lifetime)
{
	if (*lifetime > secs) {
		*lifetime -= secs;
	} else {
		*lifetime = 0;
	}
}

static void update_iface_times(struct Interface *iface)
{
	struct timespec last_time = iface->times.last_ra_time;
	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_ra_time);
	time_t secs_since_last_ra = timespecdiff(&iface->times.last_ra_time, &last_time) / 1000;

	if (secs_since_last_ra < 0) {
		secs_since_last_ra = 0;
		flog(LOG_WARNING, "clock_gettime(CLOCK_MONOTONIC) went backwards!");
	}

	struct AdvPrefix *prefix = iface->AdvPrefixList;
	while (prefix) {
		if (!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0) {
			if (!(iface->state_info.cease_adv && prefix->DeprecatePrefixFlag)) {
				if (prefix->DecrementLifetimesFlag) {

					decrement_lifetime(secs_since_last_ra, &prefix->curr_validlft);
					decrement_lifetime(secs_since_last_ra, &prefix->curr_preferredlft);

					if (prefix->curr_preferredlft == 0) {
						char pfx_str[INET6_ADDRSTRLEN];
						addrtostr(&prefix->Prefix, pfx_str, sizeof(pfx_str));
						dlog(LOG_DEBUG, 3, "Will cease advertising %s/%u%%%s, preferred lifetime is 0",
						     pfx_str, prefix->PrefixLen, iface->props.name);
					}
				}
			}
		}
		prefix = prefix->next;
	}
}

/********************************************************************************
*       add_ra_*                                                                *
********************************************************************************/

static void add_ra_header(struct safe_buffer *sb, struct ra_header_info const *ra_header_info, int cease_adv)
{
	struct nd_router_advert radvert;

	memset(&radvert, 0, sizeof(radvert));

	radvert.nd_ra_type = ND_ROUTER_ADVERT;
	radvert.nd_ra_code = 0;
	radvert.nd_ra_cksum = 0;
	radvert.nd_ra_curhoplimit = ra_header_info->AdvCurHopLimit;
	radvert.nd_ra_flags_reserved = (ra_header_info->AdvManagedFlag) ? ND_RA_FLAG_MANAGED : 0;
	radvert.nd_ra_flags_reserved |= (ra_header_info->AdvOtherConfigFlag) ? ND_RA_FLAG_OTHER : 0;
	/* Mobile IPv6 ext */
	radvert.nd_ra_flags_reserved |= (ra_header_info->AdvHomeAgentFlag) ? ND_RA_FLAG_HOME_AGENT : 0;

	radvert.nd_ra_router_lifetime = cease_adv ? 0 : htons(ra_header_info->AdvDefaultLifetime);
	radvert.nd_ra_flags_reserved |= (ra_header_info->AdvDefaultPreference << ND_OPT_RI_PRF_SHIFT) & ND_OPT_RI_PRF_MASK;

	radvert.nd_ra_reachable = htonl(ra_header_info->AdvReachableTime);
	radvert.nd_ra_retransmit = htonl(ra_header_info->AdvRetransTimer);

	safe_buffer_append(sb, &radvert, sizeof(radvert));
}

static void add_ra_option_prefix(struct safe_buffer *sb, struct AdvPrefix const *prefix, int cease_adv)
{
	struct nd_opt_prefix_info pinfo;

	memset(&pinfo, 0, sizeof(pinfo));

	pinfo.nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
	pinfo.nd_opt_pi_len = 4;
	pinfo.nd_opt_pi_prefix_len = prefix->PrefixLen;

	pinfo.nd_opt_pi_flags_reserved = (prefix->AdvOnLinkFlag) ? ND_OPT_PI_FLAG_ONLINK : 0;
	pinfo.nd_opt_pi_flags_reserved |= (prefix->AdvAutonomousFlag) ? ND_OPT_PI_FLAG_AUTO : 0;
	/* Mobile IPv6 ext */
	pinfo.nd_opt_pi_flags_reserved |= (prefix->AdvRouterAddr) ? ND_OPT_PI_FLAG_RADDR : 0;

	if (cease_adv && prefix->DeprecatePrefixFlag) {
		/* RFC4862, 5.5.3, step e) */
		if (prefix->curr_validlft < MIN_AdvValidLifetime) {
			pinfo.nd_opt_pi_valid_time = htonl(prefix->curr_validlft);
		} else {
			pinfo.nd_opt_pi_valid_time = htonl(MIN_AdvValidLifetime);
		}
		pinfo.nd_opt_pi_preferred_time = 0;
	} else {
		pinfo.nd_opt_pi_valid_time = htonl(prefix->curr_validlft);
		pinfo.nd_opt_pi_preferred_time = htonl(prefix->curr_preferredlft);
	}

	memcpy(&pinfo.nd_opt_pi_prefix, &prefix->Prefix, sizeof(struct in6_addr));

	safe_buffer_append(sb, &pinfo, sizeof(pinfo));
}

static int get_prefix_lifetimes (struct AdvPrefix const *prefix, unsigned int *valid_lft, unsigned int *preferred_lft) {
	unsigned int preferred = 0;
	unsigned int valid = 0;
	int ret = 0;

#ifdef HAVE_NETLINK
	/* Retrieve valid and current lifetimes of the prefix */
	ret = netlink_get_address_lifetimes (prefix, &preferred, &valid);
#endif

	*valid_lft = valid;
	*preferred_lft = preferred;
	return ret;
}

static void limit_prefix_lifetimes(struct AdvPrefix *prefix) {
  unsigned int valid, preferred;
  int ret = get_prefix_lifetimes (prefix, &valid, &preferred);
  /* Retrieve valid and current lifetimes of the prefix */
  if(ret) {
    prefix->curr_validlft = min(valid, prefix->curr_validlft);
    prefix->curr_preferredlft = min(preferred, prefix->curr_preferredlft);
  }
}

static void add_ra_option_nat64prefix(struct safe_buffer *sb, struct NAT64Prefix const *prefix)
{
	struct nd_opt_nat64prefix_info pinfo;
	uint8_t prefix_length_code = 0;

	memset(&pinfo, 0, sizeof(pinfo));

	pinfo.nd_opt_pi_type = ND_OPT_PREF64;
	pinfo.nd_opt_pi_len = 2;
	/*
	   PLC (Prefix Length Code):  3-bit unsigned integer.  This field
          encodes the NAT64 Prefix Length defined in [RFC6052].  The PLC
          field values 0, 1, 2, 3, 4, and 5 indicate the NAT64 prefix length
          of 96, 64, 56, 48, 40, and 32 bits, respectively.  The receiver
          MUST ignore the PREF64 option if the Prefix Length Code field is
          not set to one of those values.
	*/
	switch (prefix->PrefixLen) {
	case 96:
		prefix_length_code = 0;
		break;
	case 64:
		prefix_length_code = 1;
		break;
	case 56:
		prefix_length_code = 2;
		break;
	case 48:
		prefix_length_code = 3;
		break;
	case 40:
		prefix_length_code = 4;
		break;
	case 32:
		prefix_length_code = 5;
		break;
	}

	/*
       Scaled Lifetime:  13-bit unsigned integer.  The maximum time in units
          of 8 seconds over which this NAT64 prefix MAY be used.  See
          Section 4.1 for the Scaled Lifetime field processing rules.

       Router vendors SHOULD allow administrators to specify nonzero
         lifetime values that are not divisible by 8.  In such cases, the
         router SHOULD round the provided value up to the nearest integer that
         is divisible by 8 and smaller than 65536, then divide the result by 8
         (or perform a logical right shift by 3) and set the Scaled Lifetime
         field to the resulting value.  If a nonzero lifetime value that is to
         be divided by 8 (or subjected to a logical right shift by 3) is less
         than 8, then the Scaled Lifetime field SHOULD be set to 1.  This last
         step ensures that lifetimes under 8 seconds are encoded as a nonzero
         Scaled Lifetime.
	*/
	pinfo.nd_opt_pi_lifetime_preflen = htons(
		((prefix->curr_validlft + 7) & 0xFFF8) |
		(prefix_length_code & 0x7));

	/* Only copy 96 bits of the prefix */
	memcpy(&pinfo.nd_opt_pi_nat64prefix, &prefix->Prefix, 12);

	safe_buffer_append(sb, &pinfo, sizeof(pinfo));
}

static struct safe_buffer_list *add_auto_prefixes_6to4(struct safe_buffer_list *sbl, struct Interface const *iface,
						       char const *ifname, struct AdvPrefix const *prefix, int cease_adv,
						       struct in6_addr const *dest)
{
#ifdef HAVE_IFADDRS_H
	struct AdvPrefix xprefix = *prefix;
	unsigned int dst;

	if (get_v4addr(prefix->if6to4, &dst) < 0) {
		flog(LOG_ERR, "Base6to4interface %s has no IPv4 addresses", prefix->if6to4);
	} else {
		memcpy(xprefix.Prefix.s6_addr + 2, &dst, sizeof(dst));
		*((uint16_t *)(xprefix.Prefix.s6_addr)) = htons(0x2002);
		xprefix.PrefixLen = 64;

		char pfx_str[INET6_ADDRSTRLEN];
		addrtostr(&xprefix.Prefix, pfx_str, sizeof(pfx_str));
		dlog(LOG_DEBUG, 3, "auto-selected prefix %s/%d on interface %s", pfx_str, xprefix.PrefixLen, ifname);

		/** We want to get the lowest value out of the configured lifetime (from /etc/radvd.conf) and the maximum lifetime on
		 *  any address that is part of that prefix in the kernel to avoid advertising a prefix that might expire too soon */
		// TODO: audit clobbers of prefixes based on original config?
		limit_prefix_lifetimes(&xprefix);

		if (cease_adv || schedule_option_prefix(dest, iface, &xprefix)) {
			sbl = safe_buffer_list_append(sbl);
			add_ra_option_prefix(sbl->sb, &xprefix, cease_adv);
		}
	}
#endif
	return sbl;
}

static struct safe_buffer_list *add_auto_prefixes(struct safe_buffer_list *sbl, struct Interface const *iface, char const *ifname,
						  struct AdvPrefix const *prefix, int cease_adv, struct in6_addr const *dest)
{
#ifdef HAVE_IFADDRS_H
	struct AdvPrefix xprefix;
	struct ifaddrs *ifap = 0, *ifa = 0;

	if (getifaddrs(&ifap) != 0)
		flog(LOG_ERR, "getifaddrs failed: %s", strerror(errno));

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {

		if (strncmp(ifa->ifa_name, ifname, IFNAMSIZ))
			continue;
		
		if (ifa->ifa_addr == NULL) {
			flog(LOG_WARNING, "ifa_addr == NULL for dev %s !? Ignoring in add_auto_prefixes", ifname);
			continue;
                }
		
		if (ifa->ifa_addr->sa_family != AF_INET6)
			continue;

		struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
		struct sockaddr_in6 *mask = (struct sockaddr_in6 *)ifa->ifa_netmask;

		if (IN6_IS_ADDR_LINKLOCAL(&s6->sin6_addr))
			continue;

		struct in6_addr prefix6 = get_prefix6(&s6->sin6_addr, &mask->sin6_addr);

		int ignore = 0;
		for (struct AutogenIgnorePrefix *current = iface->IgnorePrefixList; current; current = current->next) {
			struct in6_addr candidatePrefix6 = get_prefix6(&current->Prefix, &current->Mask);

			if (memcmp(&prefix6, &candidatePrefix6, sizeof(struct in6_addr)) == 0 &&
			    memcmp(&mask->sin6_addr, &current->Mask, sizeof(struct in6_addr)) == 0) {
				ignore = 1;
				break;
			}
		}

		if (ignore)
			continue;

		xprefix = *prefix;
		xprefix.Prefix = prefix6;
		xprefix.PrefixLen = count_mask(mask);

		char pfx_str[INET6_ADDRSTRLEN];
		addrtostr(&xprefix.Prefix, pfx_str, sizeof(pfx_str));
		dlog(LOG_DEBUG, 3, "auto-selected prefix %s/%d on interface %s", pfx_str, xprefix.PrefixLen, ifname);

		/** We want to get the lowest value out of the configured lifetime (from /etc/radvd.conf) and the maximum lifetime on
		 *  any address that is part of that prefix in the kernel to avoid advertising a prefix that might expire too soon */
		// TODO: audit clobbers of prefixes based on original config?
		limit_prefix_lifetimes(&xprefix);

		if (cease_adv || schedule_option_prefix(dest, iface, &xprefix)) {
			sbl = safe_buffer_list_append(sbl);
			add_ra_option_prefix(sbl->sb, &xprefix, cease_adv);
		}
	}

	if (ifap)
		freeifaddrs(ifap);
#endif
	return sbl;
}

static struct safe_buffer_list *add_ra_options_nat64prefix(struct safe_buffer_list *sbl, struct NAT64Prefix const *prefix)
{
	while (prefix) {
		sbl = safe_buffer_list_append(sbl);
		add_ra_option_nat64prefix(sbl->sb, prefix);

		prefix = prefix->next;
	}

	return sbl;
}

static struct safe_buffer_list *add_ra_options_prefix(struct safe_buffer_list *sbl, struct Interface const *iface,
						      char const *ifname, struct AdvPrefix const *prefix, int cease_adv,
						      struct in6_addr const *dest)
{
	while (prefix) {
		if (!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0) {
			struct in6_addr zero = {};
			if (prefix->if6to4[0] || prefix->if6[0] || 0 == memcmp(&prefix->Prefix, &zero, sizeof(zero))) {
				if (prefix->if6to4[0]) {
					dlog(LOG_DEBUG, 4, "if6to4 auto prefix detected on iface %s", ifname);
					sbl = add_auto_prefixes_6to4(sbl, iface, prefix->if6to4, prefix, cease_adv, dest);
				}
				if (prefix->if6[0]) {
					dlog(LOG_DEBUG, 4, "if6 auto prefix detected on iface %s", ifname);
					sbl = add_auto_prefixes(sbl, iface, prefix->if6, prefix, cease_adv, dest);
				}
				if (0 == memcmp(&prefix->Prefix, &zero, sizeof(zero))) {
					dlog(LOG_DEBUG, 4, "::/64 auto prefix detected on iface %s", ifname);
					sbl = add_auto_prefixes(sbl, iface, iface->props.name, prefix, cease_adv, dest);
				}
			} else {
				if (cease_adv || schedule_option_prefix(dest, iface, prefix)) {
					sbl = safe_buffer_list_append(sbl);

		            /** We want to get the lowest value out of the configured lifetime (from /etc/radvd.conf) and the maximum lifetime on
		             *  any address that is part of that prefix in the kernel to avoid advertising a prefix that might expire too soon */
					// TODO: audit clobbers of prefixes based on original config?
					struct AdvPrefix xprefix = *prefix;
					limit_prefix_lifetimes(&xprefix);
					add_ra_option_prefix(sbl->sb, &xprefix, cease_adv);
				}
			}
		}

		prefix = prefix->next;
	}
	return sbl;
}

/* clang-format off */
/*
 * Domain Names of DNS Search List
 *   One or more domain names of DNS Search List that MUST
 *   be encoded using the technique described in Section
 *   3.1 of [RFC1035].  By this technique, each domain
 *   name is represented as a sequence of labels ending in
 *   a zero octet, defined as domain name representation.
 *   For more than one domain name, the corresponding
 *   domain name representations are concatenated as they
 *   are.  Note that for the simple decoding, the domain
 *   names MUST NOT be encoded in a compressed form, as
 *   described in Section 4.1.4 of [RFC1035].  Because the
 *   size of this field MUST be a multiple of 8 octets,
 *   for the minimum multiple including the domain name
 *   representations, the remaining octets other than the
 *   encoding parts of the domain name representations
 *   MUST be padded with zeros.
 */
/* clang-format on */
static size_t serialize_domain_names(struct safe_buffer *safe_buffer, struct AdvDNSSL const *dnssl)
{
	size_t len = 0;

	for (int i = 0; i < dnssl->AdvDNSSLNumber; i++) {
		char *label = dnssl->AdvDNSSLSuffixes[i];

		while (label[0] != '\0') {
			unsigned char label_len;

			if (strchr(label, '.') == NULL)
				label_len = (unsigned char)strlen(label);
			else
				label_len = (unsigned char)(strchr(label, '.') - label);

			// +8 is for null & padding, only allocate once.
			safe_buffer_resize(safe_buffer, safe_buffer->used + sizeof(label_len) + label_len + 8);
			len += safe_buffer_append(safe_buffer, &label_len, sizeof(label_len));
			len += safe_buffer_append(safe_buffer, label, label_len);

			label += label_len;

			if (label[0] == '.') {
				label++;
			}

			if (label[0] == '\0') {
				char zero = 0;
				len += safe_buffer_append(safe_buffer, &zero, sizeof(zero));
			}
		}
	}
	return len;
}

static struct safe_buffer_list *add_ra_options_route(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvRoute const *route, int cease_adv, struct in6_addr const *dest)
{
	while (route) {
		struct nd_opt_route_info_local rinfo;

		if (!cease_adv && !schedule_option_route(dest, iface, route)) {
			route = route->next;
			continue;
		}

		memset(&rinfo, 0, sizeof(rinfo));

		rinfo.nd_opt_ri_type = ND_OPT_ROUTE_INFORMATION;
		if (route->PrefixLen == 0) {
			rinfo.nd_opt_ri_len = 1;
		} else if (route->PrefixLen > 0 && route->PrefixLen <= 64) {
			rinfo.nd_opt_ri_len = 2;
		} else if (route->PrefixLen > 64 && route->PrefixLen <= 128) {
			rinfo.nd_opt_ri_len = 3;
		}
		rinfo.nd_opt_ri_prefix_len = route->PrefixLen;

		rinfo.nd_opt_ri_flags_reserved = (route->AdvRoutePreference << ND_OPT_RI_PRF_SHIFT) & ND_OPT_RI_PRF_MASK;
		if (cease_adv && route->RemoveRouteFlag) {
			rinfo.nd_opt_ri_lifetime = 0;
		} else {
			rinfo.nd_opt_ri_lifetime = htonl(route->AdvRouteLifetime);
		}

		memcpy(&rinfo.nd_opt_ri_prefix, &route->Prefix, sizeof(struct in6_addr));

		sbl = safe_buffer_list_append(sbl);
		safe_buffer_append(sbl->sb, &rinfo, rinfo.nd_opt_ri_len * 8);

		route = route->next;
	}
	return sbl;
}

static struct safe_buffer_list *add_ra_options_rdnss(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvRDNSS const *rdnss, int cease_adv, struct in6_addr const *dest)
{
	while (rdnss) {
		struct nd_opt_rdnss_info_local rdnssinfo;
		if (!cease_adv && !schedule_option_rdnss(dest, iface, rdnss)) {
			rdnss = rdnss->next;
			continue;
		}

		memset(&rdnssinfo, 0, sizeof(rdnssinfo));

		size_t const bytes = sizeof(rdnssinfo) + sizeof(struct in6_addr) * rdnss->AdvRDNSSNumber;
		size_t const nd_opt_dnssli_len = bytes/8; // deliberate size_t, not uint_8; padding is NOT required for RDNSS
		// dnsslinfo.nd_opt_rdnssi_len is uint8 count of 8-octet groups; min 3, max 255
		// too many DNS servers could exceed it
		// https://datatracker.ietf.org/doc/html/rfc8106#section-5.1
		if(nd_opt_dnssli_len > 255) {
			flog(LOG_ERR,
				"Skipping option: RDNSS too long (%ld) for RA, must be <= %d bytes including header.",
				bytes, (255*8));
			rdnss = rdnss->next;
			continue;
		}

		rdnssinfo.nd_opt_rdnssi_type = ND_OPT_RDNSS_INFORMATION;
		rdnssinfo.nd_opt_rdnssi_len = (uint8_t) nd_opt_dnssli_len;
		rdnssinfo.nd_opt_rdnssi_pref_flag_reserved = 0;

		if (cease_adv && rdnss->FlushRDNSSFlag) {
			rdnssinfo.nd_opt_rdnssi_lifetime = 0;
		} else {
			rdnssinfo.nd_opt_rdnssi_lifetime = htonl(rdnss->AdvRDNSSLifetime);
		}

		sbl = safe_buffer_list_append(sbl);
		safe_buffer_resize(sbl->sb, sbl->sb->used + bytes);
		safe_buffer_append(sbl->sb, &rdnssinfo, sizeof(rdnssinfo));
		for (int i = 0; i < rdnss->AdvRDNSSNumber; i++) {
			safe_buffer_append(sbl->sb, &rdnss->AdvRDNSSAddr[i], sizeof(struct in6_addr));
		}
		// padding is only required for DNSSL

		rdnss = rdnss->next;
	}

	return sbl;
}

static struct safe_buffer_list *add_ra_options_dnssl(struct safe_buffer_list *sbl, struct Interface const *iface,
						     struct AdvDNSSL const *dnssl, int cease_adv, struct in6_addr const *dest)
{
	struct safe_buffer *serialized_domains = new_safe_buffer();
	while (dnssl) {

		struct nd_opt_dnssl_info_local dnsslinfo;

		if (!cease_adv && !schedule_option_dnssl(dest, iface, dnssl)) {
			dnssl = dnssl->next;
			continue;
		}

		memset(&dnsslinfo, 0, sizeof(dnsslinfo));

		serialized_domains->used = 0;
		size_t const domain_name_bytes = serialize_domain_names(serialized_domains, dnssl);
		size_t const bytes = sizeof(dnsslinfo) + domain_name_bytes;
		size_t const nd_opt_dnssli_len = (bytes + 7) / 8; // deliberate size_t, not uint_8
		// dnsslinfo.nd_opt_dnssli_len is uint8 count of 8-octet groups; min 3, max 255
		// too many long serialized domains could exceed it
		// https://datatracker.ietf.org/doc/html/rfc8106#section-5.2
		size_t const bytes_with_padding = nd_opt_dnssli_len * 8;
		if(nd_opt_dnssli_len > 255) {
			flog(LOG_ERR,
				"Skipping option: DNSSL too long (%ld) for RA, must be <= %d bytes including header and padding.",
				bytes_with_padding, (255*8));
			dnssl = dnssl->next;
			continue;
		}

		dnsslinfo.nd_opt_dnssli_type = ND_OPT_DNSSL_INFORMATION;
		dnsslinfo.nd_opt_dnssli_len = (uint8_t) nd_opt_dnssli_len;
		dnsslinfo.nd_opt_dnssli_reserved = 0;

		if (cease_adv && dnssl->FlushDNSSLFlag) {
			dnsslinfo.nd_opt_dnssli_lifetime = 0;
		} else {
			dnsslinfo.nd_opt_dnssli_lifetime = htonl(dnssl->AdvDNSSLLifetime);
		}

		size_t const padding = bytes_with_padding - bytes;

		sbl = safe_buffer_list_append(sbl);
		safe_buffer_resize(sbl->sb, sbl->sb->used + sizeof(dnsslinfo) + domain_name_bytes + padding);
		safe_buffer_append(sbl->sb, &dnsslinfo, sizeof(dnsslinfo));
		safe_buffer_append(sbl->sb, serialized_domains->buffer, serialized_domains->used);
		safe_buffer_pad(sbl->sb, padding);
		// abort();

		dnssl = dnssl->next;
	}
	safe_buffer_free(serialized_domains);
	return sbl;
}

/*
 * add Source Link-layer Address option
 */
static void add_ra_option_sllao(struct safe_buffer *sb, struct sllao const *sllao)
{
	/* +2 for the ND_OPT_SOURCE_LINKADDR and the length (each occupy one byte) */
	size_t const sllao_bytes = (sllao->if_hwaddr_len / 8) + 2;
	size_t const sllao_len = (sllao_bytes + 7) / 8;

	uint8_t buff[2] = {ND_OPT_SOURCE_LINKADDR, (uint8_t)sllao_len};
	safe_buffer_append(sb, buff, sizeof(buff));

	/* if_hwaddr_len is in bits, so divide by 8 to get the byte count. */
	safe_buffer_append(sb, sllao->if_hwaddr, sllao->if_hwaddr_len / 8);
	safe_buffer_pad(sb, sllao_len * 8 - sllao_bytes);
}

static void add_ra_option_mtu(struct safe_buffer *sb, uint32_t AdvLinkMTU)
{
	struct nd_opt_mtu mtu;

	memset(&mtu, 0, sizeof(mtu));

	mtu.nd_opt_mtu_type = ND_OPT_MTU;
	mtu.nd_opt_mtu_len = 1;
	mtu.nd_opt_mtu_reserved = 0;
	mtu.nd_opt_mtu_mtu = htonl(AdvLinkMTU);

	safe_buffer_append(sb, &mtu, sizeof(mtu));
}

/*
 * Mobile IPv6 ext: Advertisement Interval Option to support
 * movement detection of mobile nodes
 */
static void add_ra_option_mipv6_rtr_adv_interval(struct safe_buffer *sb, double MaxRtrAdvInterval)
{
	uint32_t ival = 1000;

	if (MaxRtrAdvInterval < Cautious_MaxRtrAdvInterval) {
		ival *= MaxRtrAdvInterval + Cautious_MaxRtrAdvInterval_Leeway;
	} else {
		ival *= MaxRtrAdvInterval;
	}

	struct AdvInterval a_ival;

	memset(&a_ival, 0, sizeof(a_ival));

	a_ival.type = ND_OPT_RTR_ADV_INTERVAL;
	a_ival.length = 1;
	a_ival.reserved = 0;
	a_ival.adv_ival = htonl(ival);

	safe_buffer_append(sb, &a_ival, sizeof(a_ival));
}

/*
 * Mobile IPv6 ext: Home Agent Information Option to support
 * Dynamic Home Agent Address Discovery
 */
static void add_ra_option_mipv6_home_agent_info(struct safe_buffer *sb, struct mipv6 const *mipv6)
{
	struct HomeAgentInfo ha_info;

	memset(&ha_info, 0, sizeof(ha_info));

	ha_info.type = ND_OPT_HOME_AGENT_INFO;
	ha_info.length = 1;
	ha_info.flags_reserved = (mipv6->AdvMobRtrSupportFlag) ? ND_OPT_HAI_FLAG_SUPPORT_MR : 0;
	ha_info.preference = htons(mipv6->HomeAgentPreference);
	ha_info.lifetime = htons(mipv6->HomeAgentLifetime);

	safe_buffer_append(sb, &ha_info, sizeof(ha_info));
}

/*
 * Add 6co option
 */
static void add_ra_option_lowpanco(struct safe_buffer *sb, struct AdvLowpanCo const *lowpanco)
{
	struct nd_opt_6co co;

	memset(&co, 0, sizeof(co));

	co.nd_opt_6co_type = ND_OPT_6CO;
	co.nd_opt_6co_len = 3;
	co.nd_opt_6co_context_len = lowpanco->ContextLength;
	co.nd_opt_6co_res_c_cid = ((lowpanco->ContextCompressionFlag ? 1 : 0) << 4)
				| (lowpanco->AdvContextID & 0x0F);
	co.nd_opt_6co_valid_lifetime = htons(lowpanco->AdvLifeTime);
	co.nd_opt_6co_con_prefix = lowpanco->AdvContextPrefix;

	safe_buffer_append(sb, &co, sizeof(co));
}

static void add_ra_option_abro(struct safe_buffer *sb, struct AdvAbro const *abroo)
{
	struct nd_opt_abro abro;

	memset(&abro, 0, sizeof(abro));

	abro.nd_opt_abro_type = ND_OPT_ABRO;
	abro.nd_opt_abro_len = 3;
	abro.nd_opt_abro_ver_low = htons(abroo->Version[1]);
	abro.nd_opt_abro_ver_high = htons(abroo->Version[0]);
	abro.nd_opt_abro_valid_lifetime = htons(abroo->ValidLifeTime);
	abro.nd_opt_abro_6lbr_address = abroo->LBRaddress;

	safe_buffer_append(sb, &abro, sizeof(abro));
}

static void add_ra_option_capport(struct safe_buffer *sb, const char *captive_portal)
{
	/* +2 for the ND_OPT_CAPTIVE_PORTAL and the length (each occupy one byte) */
	size_t const capport_strlen = strlen(captive_portal);
	size_t const capport_bytes = capport_strlen + 2;
	size_t const capport_len = (capport_bytes + 7) / 8;

	uint8_t buff[2] = {ND_OPT_CAPTIVE_PORTAL, (uint8_t)capport_len};
	safe_buffer_append(sb, buff, sizeof(buff));

	safe_buffer_append(sb, captive_portal, capport_strlen);
	safe_buffer_pad(sb, (capport_len * 8) - capport_bytes);
}

static struct safe_buffer_list *build_ra_options(struct Interface const *iface, struct in6_addr const *dest)
{
	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer_list *cur = sbl;

	if (iface->AdvPrefixList) {
		cur =
		    add_ra_options_prefix(cur, iface, iface->props.name, iface->AdvPrefixList, iface->state_info.cease_adv, dest);
	}

	if (iface->NAT64PrefixList) {
		cur = add_ra_options_nat64prefix(cur, iface->NAT64PrefixList);
	}

	if (iface->AdvRouteList) {
		cur = add_ra_options_route(cur, iface, iface->AdvRouteList, iface->state_info.cease_adv, dest);
	}

	if (iface->AdvRDNSSList) {
		cur = add_ra_options_rdnss(cur, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, dest);
	}

	if (iface->AdvDNSSLList) {
		cur = add_ra_options_dnssl(cur, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, dest);
	}

	if (iface->AdvLinkMTU != 0 && schedule_option_mtu(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_mtu(cur->sb, iface->AdvLinkMTU);
	}

	if (iface->AdvSourceLLAddress && iface->sllao.if_hwaddr_len > 0 && schedule_option_sllao(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_sllao(cur->sb, &iface->sllao);
	}

	if (iface->mipv6.AdvIntervalOpt && schedule_option_mipv6_rtr_adv_interval(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_mipv6_rtr_adv_interval(cur->sb, iface->MaxRtrAdvInterval);
	}

	if (iface->mipv6.AdvHomeAgentInfo && schedule_option_mipv6_home_agent_info(dest, iface) &&
	    (iface->mipv6.AdvMobRtrSupportFlag || iface->mipv6.HomeAgentPreference != 0 ||
	     iface->mipv6.HomeAgentLifetime != iface->ra_header_info.AdvDefaultLifetime)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_mipv6_home_agent_info(cur->sb, &iface->mipv6);
	}

	if (iface->AdvLowpanCoList && schedule_option_lowpanco(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_lowpanco(cur->sb, iface->AdvLowpanCoList);
	}

	if (iface->AdvAbroList && schedule_option_abro(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_abro(cur->sb, iface->AdvAbroList);
	}

	if (iface->AdvCaptivePortalAPI != NULL && schedule_option_capport(dest, iface)) {
		cur->next = new_safe_buffer_list();
		cur = cur->next;
		add_ra_option_capport(cur->sb, iface->AdvCaptivePortalAPI);
	}

	// Return the root of the list
	return sbl;
}

static int send_ra(int sock, struct Interface *iface, struct in6_addr const *dest)
{
	if (!iface->AdvSendAdvert) {
		dlog(LOG_DEBUG, 2, "AdvSendAdvert is off for %s", iface->props.name);
		return 0;
	}

	if (dest == NULL) {
		static uint8_t const all_hosts_addr[] = {0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
		dest = (struct in6_addr const *)all_hosts_addr;
		clock_gettime(CLOCK_MONOTONIC, &iface->times.last_multicast);
	}

	update_iface_times(iface);

	char dest_text[INET6_ADDRSTRLEN] = {""};
	char src_text[INET6_ADDRSTRLEN] = {""};
	addrtostr(dest, dest_text, INET6_ADDRSTRLEN);
	addrtostr(iface->props.if_addr_rasrc, src_text, INET6_ADDRSTRLEN);

	// Build RA header
	struct safe_buffer *ra_hdr = new_safe_buffer();
	// if forwarding is disabled, send zero router lifetime
	// the check_ip6 function is hoisted here to enable testing of add_ra_header
	int cease_adv = iface->state_info.cease_adv || check_ip6_forwarding();
	add_ra_header(ra_hdr, &iface->ra_header_info, cease_adv);
	// Build RA option list
	struct safe_buffer_list *ra_opts = build_ra_options(iface, dest);

	// Send out one or more RAs, all in the form of (hdr+options),
	// such that none of the RAs exceed the link MTU
	// (max size is pre-computed in iface->props.max_ra_option_size)

	struct safe_buffer_list *cur = ra_opts;
	struct safe_buffer *sb = new_safe_buffer();
	do {
		unsigned long int option_count = 0;
		sb->used = 0;
		// Duplicate the RA header
		safe_buffer_append(sb, ra_hdr->buffer, ra_hdr->used);
		// Copy in as many RA options as we can fit.
		while (NULL != cur) {
			if (sb->used == 0) {
				dlog(LOG_DEBUG, 5, "send_ra: Saw empty buffer!");
				cur = cur->next;
				continue;
			}
			// Not enough room for the next option in our buffer, just send the buffer now.
			if (sb->used + cur->sb->used > iface->props.max_ra_option_size) {
				// But make sure we send at least one option in each RA
				// TODO: a future improvement would be to optimize packing of
				// the options in the minimal number of RAs, such that each one
				// does not exceed the MTU where possible.
				if (option_count > 0)
					break;
			}
			// It's possible that a single option is larger than the MTU, so
			// fragmentation will always happen in that case.
			// One known case is a very long DNSSL, which is documented in
			// RFC6106 errata #4864
			// In this case, the RA will contain a single option, consisting of
			// ONLY the DNSSL, without other options. RFC6980-conforming nodes
			// should then ignore the DNSSL.
			if (cur->sb->used > iface->props.max_ra_option_size) {
				flog(LOG_WARNING,
				     "send_ra: RA option (type=%hhd) length %lu exceeds max RA option size %u, fragmenting anyway (violates RFC6980 section 2)",
				     (unsigned char)(cur->sb->buffer[0]), cur->sb->used, iface->props.max_ra_option_size);
			}
			// Add this option to the buffer.
			safe_buffer_append(sb, cur->sb->buffer, cur->sb->used);
			option_count++;
			/*
			if(cur->sb->used > 0 && (unsigned char)(cur->sb->buffer[0]) == ND_OPT_DNSSL_INFORMATION) {
				abort();
			}
			*/
			cur = cur->next;
		}

		// RA built, now send it.
		dlog(LOG_DEBUG, 5, "sending RA to %s on %s (%s), %lu options (using %zd/%u bytes)", dest_text, iface->props.name,
		     src_text, option_count, sb->used, iface->props.max_ra_option_size);
		int err = really_send(sock, dest, &iface->props, sb);
		if (err < 0) {
			if (!iface->IgnoreIfMissing || !(errno == EINVAL || errno == ENODEV))
				flog(LOG_WARNING, "sendmsg: %s", strerror(errno));
			else
				dlog(LOG_DEBUG, 3, "sendmsg: %s", strerror(errno));
			safe_buffer_free(sb);
			safe_buffer_list_free(ra_opts);
			safe_buffer_free(ra_hdr);
			return -1;
		}

	} while (NULL != cur);

	safe_buffer_free(sb);
	safe_buffer_list_free(ra_opts);
	safe_buffer_free(ra_hdr);

	return 0;
}

static int really_send(int sock, struct in6_addr const *dest, struct properties const *props, struct safe_buffer const *sb)
{
	struct sockaddr_in6 addr;
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(IPPROTO_ICMPV6);
	memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));

	struct iovec iov;
	iov.iov_len = sb->used;
	iov.iov_base = (caddr_t)sb->buffer;

	char __attribute__((aligned(8))) chdr[CMSG_SPACE(sizeof(struct in6_pktinfo))];
	memset(chdr, 0, sizeof(chdr));
	struct cmsghdr *cmsg = (struct cmsghdr *)chdr;

	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_PKTINFO;

	struct in6_pktinfo *pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
	pkt_info->ipi6_ifindex = props->if_index;
	memcpy(&pkt_info->ipi6_addr, props->if_addr_rasrc, sizeof(struct in6_addr));

#ifdef HAVE_SIN6_SCOPE_ID
	if (IN6_IS_ADDR_LINKLOCAL(&addr.sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&addr.sin6_addr))
		addr.sin6_scope_id = props->if_index;
#endif

	struct msghdr mhdr;
	memset(&mhdr, 0, sizeof(mhdr));
	mhdr.msg_name = (caddr_t)&addr;
	mhdr.msg_namelen = sizeof(struct sockaddr_in6);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *)cmsg;
	mhdr.msg_controllen = sizeof(chdr);

	return sendmsg(sock, &mhdr, 0);
}

static int schedule_option_prefix(struct in6_addr const *dest, struct Interface const *iface, struct AdvPrefix const *prefix)
{
	return schedule_helper(dest, iface, prefix->curr_preferredlft);
}

static int schedule_option_route(struct in6_addr const *dest, struct Interface const *iface, struct AdvRoute const *route)
{
	return schedule_helper(dest, iface, route->AdvRouteLifetime);
}

static int schedule_option_rdnss(struct in6_addr const *dest, struct Interface const *iface, struct AdvRDNSS const *rdnss)
{
	return schedule_helper(dest, iface, rdnss->AdvRDNSSLifetime);
}

static int schedule_option_dnssl(struct in6_addr const *dest, struct Interface const *iface, struct AdvDNSSL const *dnssl)
{
	return schedule_helper(dest, iface, dnssl->AdvDNSSLLifetime);
}

static int schedule_option_mtu(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->ra_header_info.AdvDefaultLifetime);
}

static int schedule_option_sllao(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->ra_header_info.AdvDefaultLifetime);
}

static int schedule_option_mipv6_rtr_adv_interval(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->ra_header_info.AdvDefaultLifetime);
}

static int schedule_option_mipv6_home_agent_info(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->mipv6.HomeAgentLifetime);
}

static int schedule_option_lowpanco(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->AdvLowpanCoList->AdvLifeTime);
}

static int schedule_option_abro(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->AdvAbroList->ValidLifeTime);
}

static int schedule_option_capport(struct in6_addr const *dest, struct Interface const *iface)
{
	return schedule_helper(dest, iface, iface->ra_header_info.AdvDefaultLifetime);
}

static int schedule_helper(struct in6_addr const *dest, struct Interface const *iface, int option_lifetime)
{
	return 1;
	// 1.
	// Cases to schedule a complete RA blast:
	// - Server received a RS
	// - We're in the initial-RAs phase of startup
	// - The (unicast) destination was first seen very recently, and probably
	//   has NOT got a full set of RAs yet.

	// 2.
	// If the dest has existed for a while
	// (unicast destination): spread RAs out to at least 1/N of the option lifetime
	// (multicast destination): spread RAs out to at least 1/N of the option lifetime
};
