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

static int really_send(int sock, struct in6_addr const *dest, struct properties const *props, struct safe_buffer const *sb);
static int send_ra(int sock, struct Interface *iface, struct in6_addr const *dest);
static struct safe_buffer_list *build_ra_options(struct Interface const *iface, struct in6_addr const *dest);

static int ensure_iface_setup(int sock, struct Interface *iface);
static void decrement_lifetime(const time_t secs, uint32_t *lifetime);
static void update_iface_times(struct Interface *iface);

// Option helpers
static size_t serialize_domain_names(struct safe_buffer *safe_buffer, struct AdvDNSSL const *dnssl);

// Options that only need a single block
static void add_ra_header(struct safe_buffer *sb, struct ra_header_info const *ra_header_info, int cease_adv);
static void add_ra_option_prefix(struct safe_buffer *sb, struct AdvPrefix const *prefix, int cease_adv);
static void add_ra_option_mtu(struct safe_buffer *sb, uint32_t AdvLinkMTU);
static void add_ra_option_sllao(struct safe_buffer *sb, struct sllao const *sllao);
static void add_ra_option_mipv6_rtr_adv_interval(struct safe_buffer *sb, double MaxRtrAdvInterval);
static void add_ra_option_mipv6_home_agent_info(struct safe_buffer *sb, struct mipv6 const *mipv6);
static void add_ra_option_lowpanco(struct safe_buffer *sb, struct AdvLowpanCo const *lowpanco);
static void add_ra_option_abro(struct safe_buffer *sb, struct AdvAbro const *abroo);

// Options that generate 0 or more blocks
static struct safe_buffer_list *add_ra_options_prefix(struct safe_buffer_list *sbl, struct Interface const *iface,
						      char const *ifname, struct AdvPrefix const *prefix, int cease_adv,
						      struct in6_addr const *dest);
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

	if (iface->state_info.racount < MAX_INITIAL_RTR_ADVERTISEMENTS)
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

		send_ra(sock, iface, &(current->Address));

		/* If we should only send the RA to a specific address, we are done */
		if (dest != NULL)
			return 0;
	}

	if (dest == NULL)
		return 0;

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
		if ((!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0)) {
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

	if (cease_adv) {
		radvert.nd_ra_router_lifetime = 0;
	} else {
		/* if forwarding is disabled, send zero router lifetime */
		radvert.nd_ra_router_lifetime = !check_ip6_forwarding() ? htons(ra_header_info->AdvDefaultLifetime) : 0;
	}
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

		/* TODO: Something must be done with these. */
		(void)xprefix.curr_validlft;
		(void)xprefix.curr_preferredlft;

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

		if (ifa->ifa_addr->sa_family != AF_INET6)
			continue;

		struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
		struct sockaddr_in6 *mask = (struct sockaddr_in6 *)ifa->ifa_netmask;

		if (IN6_IS_ADDR_LINKLOCAL(&s6->sin6_addr))
			continue;

		xprefix = *prefix;
		xprefix.Prefix = get_prefix6(&s6->sin6_addr, &mask->sin6_addr);
		xprefix.PrefixLen = count_mask(mask);

		char pfx_str[INET6_ADDRSTRLEN];
		addrtostr(&xprefix.Prefix, pfx_str, sizeof(pfx_str));
		dlog(LOG_DEBUG, 3, "auto-selected prefix %s/%d on interface %s", pfx_str, xprefix.PrefixLen, ifname);

		/* TODO: Something must be done with these. */
		(void)xprefix.curr_validlft;
		(void)xprefix.curr_preferredlft;

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

static struct safe_buffer_list *add_ra_options_prefix(struct safe_buffer_list *sbl, struct Interface const *iface,
						      char const *ifname, struct AdvPrefix const *prefix, int cease_adv,
						      struct in6_addr const *dest)
{
	while (prefix) {
		if ((!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0)) {
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
					add_ra_option_prefix(sbl->sb, prefix, cease_adv);
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
		/* XXX: the prefixes are allowed to be sent in smaller chunks as well */
		rinfo.nd_opt_ri_len = 3;
		rinfo.nd_opt_ri_prefix_len = route->PrefixLen;

		rinfo.nd_opt_ri_flags_reserved = (route->AdvRoutePreference << ND_OPT_RI_PRF_SHIFT) & ND_OPT_RI_PRF_MASK;
		if (cease_adv && route->RemoveRouteFlag) {
			rinfo.nd_opt_ri_lifetime = 0;
		} else {
			rinfo.nd_opt_ri_lifetime = htonl(route->AdvRouteLifetime);
		}

		memcpy(&rinfo.nd_opt_ri_prefix, &route->Prefix, sizeof(struct in6_addr));

		sbl = safe_buffer_list_append(sbl);
		safe_buffer_append(sbl->sb, &rinfo, sizeof(rinfo));

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

		rdnssinfo.nd_opt_rdnssi_type = ND_OPT_RDNSS_INFORMATION;
		rdnssinfo.nd_opt_rdnssi_len = 1 + 2 * rdnss->AdvRDNSSNumber;
		rdnssinfo.nd_opt_rdnssi_pref_flag_reserved = 0;

		if (cease_adv && rdnss->FlushRDNSSFlag) {
			rdnssinfo.nd_opt_rdnssi_lifetime = 0;
		} else {
			rdnssinfo.nd_opt_rdnssi_lifetime = htonl(rdnss->AdvRDNSSLifetime);
		}

		memcpy(&rdnssinfo.nd_opt_rdnssi_addr1, &rdnss->AdvRDNSSAddr1, sizeof(struct in6_addr));
		memcpy(&rdnssinfo.nd_opt_rdnssi_addr2, &rdnss->AdvRDNSSAddr2, sizeof(struct in6_addr));
		memcpy(&rdnssinfo.nd_opt_rdnssi_addr3, &rdnss->AdvRDNSSAddr3, sizeof(struct in6_addr));

		sbl = safe_buffer_list_append(sbl);
		safe_buffer_append(sbl->sb, &rdnssinfo,
				   sizeof(rdnssinfo) - (3 - rdnss->AdvRDNSSNumber) * sizeof(struct in6_addr));

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
		if (bytes > (256 * 8)) {
			flog(LOG_ERR, "DNSSL too long for RA option, must be < 2048 bytes.  Exiting.");
			exit(1);
		}

		dnsslinfo.nd_opt_dnssli_type = ND_OPT_DNSSL_INFORMATION;
		dnsslinfo.nd_opt_dnssli_len = (bytes + 7) / 8;
		dnsslinfo.nd_opt_dnssli_reserved = 0;

		if (cease_adv && dnssl->FlushDNSSLFlag) {
			dnsslinfo.nd_opt_dnssli_lifetime = 0;
		} else {
			dnsslinfo.nd_opt_dnssli_lifetime = htonl(dnssl->AdvDNSSLLifetime);
		}

		size_t const padding = dnsslinfo.nd_opt_dnssli_len * 8 - bytes;

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

static struct safe_buffer_list *build_ra_options(struct Interface const *iface, struct in6_addr const *dest)
{
	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer_list *cur = sbl;

	if (iface->AdvPrefixList) {
		cur =
		    add_ra_options_prefix(cur, iface, iface->props.name, iface->AdvPrefixList, iface->state_info.cease_adv, dest);
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
	add_ra_header(ra_hdr, &iface->ra_header_info, iface->state_info.cease_adv);
	// Build RA option list
	struct safe_buffer_list *ra_opts = build_ra_options(iface, dest);

	// Send out one or more RAs, all in the form of (hdr+options),
	// such that none of the RAs exceed the link MTU
	// (max size is pre-computed in iface->props.max_ra_option_size)

	struct safe_buffer_list *cur = ra_opts;
	struct safe_buffer *sb = new_safe_buffer();
	unsigned long int total_seen_options = 0;
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
			// Ok, it's more than 0 bytes in length
			total_seen_options++;
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
				     "send_ra: RA option (type=%hhd) too long for MTU, fragmenting anyway (violates RFC6980)",
				     (unsigned char)(cur->sb->buffer[0]));
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

		if (option_count == 0 && total_seen_options > 0) {
			// If option_count == 0 and total_seen_options==0 we make sure to
			// send ONE RA out, so that clients get the RA header fields.
		} else if (option_count == 0 && total_seen_options > 0) {
			// None of the RA options are scheduled for this window.
			dlog(LOG_DEBUG, 5,
			     "No RA options scheduled in this pass, staying quiet; already sent at least one RA packet");
			break;
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
