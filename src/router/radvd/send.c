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
static void build_ra(struct safe_buffer * sb, struct Interface const * iface);

static int ensure_iface_setup(int sock, struct Interface *iface);
static void decrement_lifetime(const time_t secs, uint32_t * lifetime);
static void update_iface_times(struct Interface * iface);

static void add_ra_header(struct safe_buffer * sb, struct ra_header_info const * ra_header_info, int cease_adv);
static void add_prefix(struct safe_buffer * sb, struct AdvPrefix const * prefix, int cease_adv);
static void add_route(struct safe_buffer * sb, struct AdvRoute const *route, int cease_adv);
static void add_rdnss(struct safe_buffer * sb, struct AdvRDNSS const *rdnss, int cease_adv);
static size_t serialize_domain_names(struct safe_buffer * safe_buffer, struct AdvDNSSL const *dnssl);
static void add_dnssl(struct safe_buffer * sb, struct AdvDNSSL const *dnssl, int cease_adv);
static void add_mtu(struct safe_buffer * sb, uint32_t AdvLinkMTU);
static void add_sllao(struct safe_buffer * sb, struct sllao const *sllao);
static void add_mipv6_rtr_adv_interval(struct safe_buffer * sb, double MaxRtrAdvInterval);
static void add_mipv6_home_agent_info(struct safe_buffer * sb, struct mipv6 const * mipv6);
static void add_lowpanco(struct safe_buffer * sb, struct AdvLowpanCo const *lowpanco);
static void add_abro(struct safe_buffer * sb, struct AdvAbro const *abroo);

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
	for (struct Clients * current = iface->ClientList; current; current = current->next) {
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
		char address_text[INET6_ADDRSTRLEN] = { "" };
		inet_ntop(AF_INET6, dest, address_text, INET6_ADDRSTRLEN);
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

static void decrement_lifetime(const time_t secs, uint32_t * lifetime)
{
	if (*lifetime > secs) {
		*lifetime -= secs;
	} else {
		*lifetime = 0;
	}
}

static void update_iface_times(struct Interface * iface)
{
	struct timespec last_time = iface->times.last_ra_time;
	clock_gettime(CLOCK_MONOTONIC, &iface->times.last_ra_time);
	time_t secs_since_last_ra = timespecdiff(&iface->times.last_ra_time, &last_time);

	if (secs_since_last_ra < 0) {
		secs_since_last_ra = 0;
		flog(LOG_WARNING, "clock_gettime(CLOCK_MONOTONIC) went backwards!");
	}

	struct AdvPrefix *prefix = iface->AdvPrefixList;
	while (prefix) {
		if (prefix->enabled && (!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0)) {
			if (!(iface->state_info.cease_adv && prefix->DeprecatePrefixFlag)) {
				if (prefix->DecrementLifetimesFlag) {

					decrement_lifetime(secs_since_last_ra, &prefix->curr_validlft);
					decrement_lifetime(secs_since_last_ra, &prefix->curr_preferredlft);

					if (prefix->curr_preferredlft == 0) {
						char pfx_str[INET6_ADDRSTRLEN];
						addrtostr(&prefix->Prefix, pfx_str, sizeof(pfx_str));
						dlog(LOG_DEBUG, 3, "Will cease advertising %s/%u%%%s, preferred lifetime is 0", pfx_str, prefix->PrefixLen, iface->props.name);
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

static void add_ra_header(struct safe_buffer * sb, struct ra_header_info const * ra_header_info, int cease_adv)
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
		radvert.nd_ra_router_lifetime = !check_ip6_forwarding()? htons(ra_header_info->AdvDefaultLifetime) : 0;
	}
	radvert.nd_ra_flags_reserved |= (ra_header_info->AdvDefaultPreference << ND_OPT_RI_PRF_SHIFT) & ND_OPT_RI_PRF_MASK;

	radvert.nd_ra_reachable = htonl(ra_header_info->AdvReachableTime);
	radvert.nd_ra_retransmit = htonl(ra_header_info->AdvRetransTimer);

	safe_buffer_append(sb, &radvert, sizeof(radvert));
}

static void add_prefix(struct safe_buffer * sb, struct AdvPrefix const *prefix, int cease_adv)
{
	while (prefix) {
		if (prefix->enabled && (!prefix->DecrementLifetimesFlag || prefix->curr_preferredlft > 0)) {
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
				pinfo.nd_opt_pi_valid_time = htonl(MIN_AdvValidLifetime);
				pinfo.nd_opt_pi_preferred_time = 0;
			} else {
				pinfo.nd_opt_pi_valid_time = htonl(prefix->curr_validlft);
				pinfo.nd_opt_pi_preferred_time = htonl(prefix->curr_preferredlft);
			}

			memcpy(&pinfo.nd_opt_pi_prefix, &prefix->Prefix, sizeof(struct in6_addr));

			safe_buffer_append(sb, &pinfo, sizeof(pinfo));
		}

		prefix = prefix->next;
	}
}



/* *INDENT-OFF* */
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
/* *INDENT-ON* */
static size_t serialize_domain_names(struct safe_buffer * safe_buffer, struct AdvDNSSL const *dnssl)
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

static void add_route(struct safe_buffer * sb, struct AdvRoute const *route, int cease_adv)
{
	while (route) {
		struct nd_opt_route_info_local rinfo;

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

		safe_buffer_append(sb, &rinfo, sizeof(rinfo));

		route = route->next;
	}
}

static void add_rdnss(struct safe_buffer * sb, struct AdvRDNSS const *rdnss, int cease_adv)
{
	while (rdnss) {
		struct nd_opt_rdnss_info_local rdnssinfo;

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

		safe_buffer_append(sb, &rdnssinfo, sizeof(rdnssinfo) - (3 - rdnss->AdvRDNSSNumber) * sizeof(struct in6_addr));

		rdnss = rdnss->next;
	}
}

static void add_dnssl(struct safe_buffer * safe_buffer, struct AdvDNSSL const *dnssl, int cease_adv)
{
	while (dnssl) {

		/* *INDENT-OFF* */
		/*
		 * Snippet from RFC 6106...
		 *
		 *    5.2. DNS Search List Option
		 * 
		 * 
		 *    The DNSSL option contains one or more domain names of DNS suffixes.
		 *    All of the domain names share the same Lifetime value.  If it is
		 *    desirable to have different Lifetime values, multiple DNSSL options
		 *    can be used.  Figure 2 shows the format of the DNSSL option.
		 * 
		 *       0                   1                   2                   3
		 *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *      |     Type      |     Length    |           Reserved            |
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *      |                           Lifetime                            |
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *      |                                                               |
		 *      :                Domain Names of DNS Search List                :
		 *      |                                                               |
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * 
		 *               Figure 2: DNS Search List (DNSSL) Option Format
		 * 
		 *   Fields:
		 *     Type          8-bit identifier of the DNSSL option type as assigned
		 *                   by the IANA: 31
		 * 
		 *     Length        8-bit unsigned integer.  The length of the option
		 *                   (including the Type and Length fields) is in units of
		 *                   8 octets.  The minimum value is 2 if at least one
		 *                   domain name is contained in the option.  The Length
		 *                   field is set to a multiple of 8 octets to accommodate
		 *                   all the domain names in the field of Domain Names of
		 *                   DNS Search List.
		 * 
		 *     Lifetime      32-bit unsigned integer.  The maximum time, in
		 *                   seconds (relative to the time the packet is sent),
		 *                   over which this DNSSL domain name MAY be used for
		 *                   name resolution.  The Lifetime value has the same
		 *                   semantics as with the RDNSS option.  That is, Lifetime
		 *                   SHOULD be bounded as follows:
		 *                   MaxRtrAdvInterval <= Lifetime <= 2*MaxRtrAdvInterval.
		 *                   A value of all one bits (0xffffffff) represents
		 *                   infinity.  A value of zero means that the DNSSL
		 *                   domain name MUST no longer be used.
		 * 
		 *     Domain Names of DNS Search List
		 *                   One or more domain names of DNS Search List that MUST
		 *                   be encoded using the technique described in Section
		 *                   3.1 of [RFC1035].
		 * 
		 */
		/* *INDENT-ON* */
		struct nd_opt_dnssl_info_local dnsslinfo;

		memset(&dnsslinfo, 0, sizeof(dnsslinfo));

		size_t const domain_name_bytes = serialize_domain_names(0, dnssl);
		size_t const bytes = sizeof(dnsslinfo) + domain_name_bytes;
		
		dnsslinfo.nd_opt_dnssli_type = ND_OPT_DNSSL_INFORMATION;
		dnsslinfo.nd_opt_dnssli_len = (bytes + 7) / 8;
		dnsslinfo.nd_opt_dnssli_reserved = 0;

		if (cease_adv && dnssl->FlushDNSSLFlag) {
			dnsslinfo.nd_opt_dnssli_lifetime = 0;
		} else {
			dnsslinfo.nd_opt_dnssli_lifetime = htonl(dnssl->AdvDNSSLLifetime);
		}

		size_t const padding = dnsslinfo.nd_opt_dnssli_len * 8 - bytes;

		safe_buffer_append(safe_buffer, &dnsslinfo, sizeof(dnsslinfo));
		serialize_domain_names(safe_buffer, dnssl);
		safe_buffer_pad(safe_buffer, padding);

		dnssl = dnssl->next;
	}


}

/*
 * add Source Link-layer Address option
 */
static void add_sllao(struct safe_buffer * sb, struct sllao const *sllao)
{
	/* *INDENT-OFF* */
	/*
	4.6.1.  Source/Target Link-layer Address

	      0                   1                   2                   3
	      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	     |     Type      |    Length     |    Link-Layer Address ...
	     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	   Fields:

	      Type
			     1 for Source Link-layer Address
			     2 for Target Link-layer Address

	      Length         The length of the option (including the type and
			     length fields) in units of 8 octets.  For example,
			     the length for IEEE 802 addresses is 1 [IPv6-
			     ETHER].

	      Link-Layer Address
			     The variable length link-layer address.

			     The content and format of this field (including
			     byte and bit ordering) is expected to be specified
			     in specific documents that describe how IPv6
			     operates over different link layers.  For instance,
			     [IPv6-ETHER].

	 */
	/* *INDENT-ON* */

	/* +2 for the ND_OPT_SOURCE_LINKADDR and the length (each occupy one byte) */
	size_t const sllao_bytes = (sllao->if_hwaddr_len / 8) + 2;
	size_t const sllao_len = (sllao_bytes + 7) / 8;

	uint8_t buff[2] = {ND_OPT_SOURCE_LINKADDR, (uint8_t)sllao_len};
	safe_buffer_append(sb, buff, sizeof(buff));

	/* if_hwaddr_len is in bits, so divide by 8 to get the byte count. */
	safe_buffer_append(sb, sllao->if_hwaddr, sllao->if_hwaddr_len / 8);
	safe_buffer_pad(sb, sllao_len * 8 - sllao_bytes);
}

static void add_mtu(struct safe_buffer * sb, uint32_t AdvLinkMTU)
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
static void add_mipv6_rtr_adv_interval(struct safe_buffer * sb, double MaxRtrAdvInterval)
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
static void add_mipv6_home_agent_info(struct safe_buffer * sb, struct mipv6 const * mipv6)
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
static void add_lowpanco(struct safe_buffer * sb, struct AdvLowpanCo const *lowpanco)
{
	struct nd_opt_6co co;

	memset(&co, 0, sizeof(co));

	co.nd_opt_6co_type = ND_OPT_6CO;
	co.nd_opt_6co_len = 3;
	co.nd_opt_6co_context_len = lowpanco->ContextLength;
	co.nd_opt_6co_c = lowpanco->ContextCompressionFlag;
	co.nd_opt_6co_cid = lowpanco->AdvContextID;
	co.nd_opt_6co_valid_lifetime = lowpanco->AdvLifeTime;
	co.nd_opt_6co_con_prefix = lowpanco->AdvContextPrefix;

	safe_buffer_append(sb, &co, sizeof(co));
}

static void add_abro(struct safe_buffer * sb, struct AdvAbro const *abroo)
{
	struct nd_opt_abro abro;

	memset(&abro, 0, sizeof(abro));

	abro.nd_opt_abro_type = ND_OPT_ABRO;
	abro.nd_opt_abro_len = 3;
	abro.nd_opt_abro_ver_low = abroo->Version[1];
	abro.nd_opt_abro_ver_high = abroo->Version[0];
	abro.nd_opt_abro_valid_lifetime = abroo->ValidLifeTime;
	abro.nd_opt_abro_6lbr_address = abroo->LBRaddress;

	safe_buffer_append(sb, &abro, sizeof(abro));
}


static void build_ra(struct safe_buffer * sb, struct Interface const * iface)
{
	add_ra_header(sb, &iface->ra_header_info, iface->state_info.cease_adv);

	if (iface->AdvPrefixList) {
		add_prefix(sb, iface->AdvPrefixList, iface->state_info.cease_adv);
	}

	if (iface->AdvRouteList) {
		add_route(sb, iface->AdvRouteList, iface->state_info.cease_adv);
	}

	if (iface->AdvRDNSSList) {
		add_rdnss(sb, iface->AdvRDNSSList, iface->state_info.cease_adv);
	}

	if (iface->AdvDNSSLList) {
		add_dnssl(sb, iface->AdvDNSSLList, iface->state_info.cease_adv);
	}

	if (iface->AdvLinkMTU != 0) {
		add_mtu(sb, iface->AdvLinkMTU);
	}

	if (iface->AdvSourceLLAddress && iface->sllao.if_hwaddr_len > 0) {
		add_sllao(sb, &iface->sllao);
	}

	if (iface->mipv6.AdvIntervalOpt) {
		add_mipv6_rtr_adv_interval(sb, iface->MaxRtrAdvInterval);
	}

	if (iface->mipv6.AdvHomeAgentInfo
	    && (iface->mipv6.AdvMobRtrSupportFlag || iface->mipv6.HomeAgentPreference != 0
		|| iface->mipv6.HomeAgentLifetime != iface->ra_header_info.AdvDefaultLifetime)) {
		add_mipv6_home_agent_info(sb, &iface->mipv6);
	}

	if (iface->AdvLowpanCoList) {
		add_lowpanco(sb, iface->AdvLowpanCoList);
	}

	if (iface->AdvAbroList) {
		add_abro(sb, iface->AdvAbroList);
	}
}

static int send_ra(int sock, struct Interface *iface, struct in6_addr const *dest)
{
	if (!iface->AdvSendAdvert) {
		dlog(LOG_DEBUG, 2, "AdvSendAdvert is off for %s", iface->props.name);
		return 0;
	}

	if (dest == NULL) {
		static uint8_t const all_hosts_addr[] = { 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
		dest = (struct in6_addr const *)all_hosts_addr;
		clock_gettime(CLOCK_MONOTONIC, &iface->times.last_multicast);
	}

	update_iface_times(iface);

	char address_text[INET6_ADDRSTRLEN] = { "" };
	inet_ntop(AF_INET6, dest, address_text, INET6_ADDRSTRLEN);
	dlog(LOG_DEBUG, 5, "sending RA to %s on %s", address_text, iface->props.name);

	struct safe_buffer safe_buffer = SAFE_BUFFER_INIT;

	build_ra(&safe_buffer, iface);

	int err = really_send(sock, dest, &iface->props, &safe_buffer);

	safe_buffer_free(&safe_buffer);

	if (err < 0) {
		if (!iface->IgnoreIfMissing || !(errno == EINVAL || errno == ENODEV))
			flog(LOG_WARNING, "sendmsg: %s", strerror(errno));
		else
			dlog(LOG_DEBUG, 3, "sendmsg: %s", strerror(errno));
		return -1;
	}

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

	char __attribute__ ((aligned(8))) chdr[CMSG_SPACE(sizeof(struct in6_pktinfo))];
	memset(chdr, 0, sizeof(chdr));
	struct cmsghdr *cmsg = (struct cmsghdr *)chdr;

	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_PKTINFO;

	struct in6_pktinfo *pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
	pkt_info->ipi6_ifindex = props->if_index;
	memcpy(&pkt_info->ipi6_addr, &props->if_addr, sizeof(struct in6_addr));

#ifdef HAVE_SIN6_SCOPE_ID
	if (IN6_IS_ADDR_LINKLOCAL(&addr.sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&addr.sin6_addr))
		addr.sin6_scope_id = props->if_index;
#endif

	struct msghdr mhdr;
	memset(&mhdr, 0, sizeof(mhdr));
	mhdr.msg_name = (caddr_t) & addr;
	mhdr.msg_namelen = sizeof(struct sockaddr_in6);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *)cmsg;
	mhdr.msg_controllen = sizeof(chdr);

	return sendmsg(sock, &mhdr, 0);
}
