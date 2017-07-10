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

static void process_rs(int sock, struct Interface *, unsigned char *msg, int len, struct sockaddr_in6 *);
static void process_ra(struct Interface *, unsigned char *msg, int len, struct sockaddr_in6 *);
static int addr_match(struct in6_addr *a1, struct in6_addr *a2, int prefixlen);

void process(int sock, struct Interface *interfaces, unsigned char *msg, int len, struct sockaddr_in6 *addr,
	     struct in6_pktinfo *pkt_info, int hoplimit)
{
	char if_namebuf[IF_NAMESIZE] = {""};
	char *if_name = if_indextoname(pkt_info->ipi6_ifindex, if_namebuf);
	if (!if_name) {
		if_name = "unknown interface";
	}
	dlog(LOG_DEBUG, 4, "%s received a packet", if_name);

	char addr_str[INET6_ADDRSTRLEN];
	addrtostr(&addr->sin6_addr, addr_str, sizeof(addr_str));

	if (!pkt_info) {
		flog(LOG_WARNING, "%s received packet with no pkt_info from %s!", if_name, addr_str);
		return;
	}

	/*
	 * can this happen?
	 */

	if (len < sizeof(struct icmp6_hdr)) {
		flog(LOG_WARNING, "%s received icmpv6 packet with invalid length (%d) from %s", if_name, len, addr_str);
		return;
	}

	struct icmp6_hdr *icmph = (struct icmp6_hdr *)msg;

	if (icmph->icmp6_type != ND_ROUTER_SOLICIT && icmph->icmp6_type != ND_ROUTER_ADVERT) {
		/*
		 *      We just want to listen to RSs and RAs
		 */

		flog(LOG_ERR, "%s icmpv6 filter failed", if_name);
		return;
	}

	if (icmph->icmp6_type == ND_ROUTER_ADVERT) {
		if (len < sizeof(struct nd_router_advert)) {
			flog(LOG_WARNING, "%s received icmpv6 RA packet with invalid length (%d) from %s", if_name, len,
			     addr_str);
			return;
		}

		if (!IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)) {
			flog(LOG_WARNING, "%s received icmpv6 RA packet with non-linklocal source address from %s", if_name,
			     addr_str);
			return;
		}
	}

	if (icmph->icmp6_type == ND_ROUTER_SOLICIT) {
		if (len < sizeof(struct nd_router_solicit)) {
			flog(LOG_WARNING, "%s received icmpv6 RS packet with invalid length (%d) from %s", if_name, len,
			     addr_str);
			return;
		}
	}

	if (icmph->icmp6_code != 0) {
		flog(LOG_WARNING, "%s received icmpv6 RS/RA packet with invalid code (%d) from %s", if_name, icmph->icmp6_code,
		     addr_str);
		return;
	}

	/* get iface by received if_index */
	struct Interface *iface = find_iface_by_index(interfaces, pkt_info->ipi6_ifindex);

	if (iface == NULL) {
		dlog(LOG_WARNING, 4, "%s received icmpv6 RS/RA packet on an unknown interface with index %d", if_name,
		     pkt_info->ipi6_ifindex);
		return;
	}

	if (!iface->state_info.ready && (0 != setup_iface(sock, iface))) {
		flog(LOG_WARNING, "%s received RS or RA on %s but %s is not ready and setup_iface failed", if_name,
		     iface->props.name, iface->props.name);
		return;
	}

	if (hoplimit != 255) {
		flog(LOG_WARNING, "%s received RS or RA with invalid hoplimit %d from %s", if_name, hoplimit, addr_str);
		return;
	}

	if (icmph->icmp6_type == ND_ROUTER_SOLICIT) {
		dlog(LOG_DEBUG, 3, "%s received RS from: %s", if_name, addr_str);
		process_rs(sock, iface, msg, len, addr);
	} else if (icmph->icmp6_type == ND_ROUTER_ADVERT) {
		if (0 == memcmp(&addr->sin6_addr, &iface->props.if_addr, sizeof(iface->props.if_addr))) {
			dlog(LOG_DEBUG, 3, "%s received RA from: %s (myself)", if_name, addr_str);
		} else {
			dlog(LOG_DEBUG, 3, "%s received RA from: %s", if_name, addr_str);
		}
		process_ra(iface, msg, len, addr);
	}
}

static void process_rs(int sock, struct Interface *iface, unsigned char *msg, int len, struct sockaddr_in6 *addr)
{
	/* RFC 7772, section 5.1:
	 * 5.1.  Network-Side Recommendations
	 *    1.  Router manufacturers SHOULD allow network administrators to
	 *        configure the routers to respond to Router Solicitations with
	 *        unicast Router Advertisements if:
	 *        *  The Router Solicitation's source address is not the
	 *           unspecified address, and:
	 *        *  The solicitation contains a valid Source Link-Layer Address
	 *           option.
	 *
	 * However, testing shows that many clients do not set the SLLA option:
	 * https://github.com/reubenhwk/radvd/issues/63#issuecomment-287172252
	 * As of 2017/03/16:
	 * - macOS 10.12.3 sierra - sends SLLA 2 times out of 4
	 * - iOS 10.2.1 (iPhone 5s) - no SLLA
	 * - Android 7.0 (sony xperia phone) - sends SLLA
	 * - Android 5.1 (nexus 7 tablet) - sends SLLA
	 * - Ubuntu 16.04.2 LTS w/ Network Manager, running 4.9 kernel (dell laptop) - no SLLA
	 * - Windows 10 (dell laptop) - no SLLA
	 *
	 * We decide to ignore the SLLA option for now, and only require the
	 * unspecified address check. Clients that did not set the SLLA option will
	 * trigger a neighbour solicit to the solicited-node address trying to
	 * resolve the link-local address to, this would still be less traffic than
	 * the all-nodes multicast.
	 */
	int rfc7772_unicast_response = iface->AdvRASolicitedUnicast && !IN6_IS_ADDR_UNSPECIFIED(&addr->sin6_addr);

	/* validation */
	len -= sizeof(struct nd_router_solicit);

	uint8_t *opt_str = (uint8_t *)(msg + sizeof(struct nd_router_solicit));

	while (len > 0) {
		if (len < 2) {
			flog(LOG_WARNING, "trailing garbage in RS");
			return;
		}

		int const optlen = (opt_str[1] << 3);

		if (optlen == 0) {
			flog(LOG_WARNING, "zero length option in RS");
			return;
		} else if (optlen > len) {
			flog(LOG_WARNING, "option length greater than total length in RS");
			return;
		}

		if (*opt_str == ND_OPT_SOURCE_LINKADDR && IN6_IS_ADDR_UNSPECIFIED(&addr->sin6_addr)) {
			flog(LOG_WARNING,
			     "received icmpv6 RS packet with unspecified source address and there is a lladdr option");
			return;
		}

		len -= optlen;
		opt_str += optlen;
	}

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	double const delay = (MAX_RA_DELAY_SECONDS * rand() / (RAND_MAX + 1.0));

	if (iface->UnicastOnly || rfc7772_unicast_response) {
		send_ra_forall(sock, iface, &addr->sin6_addr);
	} else if (timespecdiff(&ts, &iface->times.last_multicast) / 1000.0 < iface->MinDelayBetweenRAs) {
		/* last RA was sent only a few moments ago, don't send another immediately. */
		double next = iface->MinDelayBetweenRAs - (ts.tv_sec + ts.tv_nsec / 1000000000.0) +
			      (iface->times.last_multicast.tv_sec + iface->times.last_multicast.tv_nsec / 1000000000.0) + delay;
		dlog(LOG_DEBUG, 5, "%s: rate limiting RA's, rescheduling RA %f seconds from now", iface->props.name, next);
		reschedule_iface(iface, next);
	} else {
		/* no RA sent in a while, send a multicast reply */
		send_ra_forall(sock, iface, NULL);
		double next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval);
		reschedule_iface(iface, next);
	}
	dlog(LOG_DEBUG, 2, "%s processed an RS", iface->props.name);
}

/*
 * check router advertisements according to RFC 4861, 6.2.7
 */
static void process_ra(struct Interface *iface, unsigned char *msg, int len, struct sockaddr_in6 *addr)
{
	char addr_str[INET6_ADDRSTRLEN];
	addrtostr(&addr->sin6_addr, addr_str, sizeof(addr_str));

	struct nd_router_advert *radvert = (struct nd_router_advert *)msg;

	if ((radvert->nd_ra_curhoplimit && iface->ra_header_info.AdvCurHopLimit) &&
	    (radvert->nd_ra_curhoplimit != iface->ra_header_info.AdvCurHopLimit)) {
		flog(LOG_WARNING, "our AdvCurHopLimit on %s doesn't agree with %s", iface->props.name, addr_str);
	}

	if ((radvert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED) && !iface->ra_header_info.AdvManagedFlag) {
		flog(LOG_WARNING, "our AdvManagedFlag on %s doesn't agree with %s", iface->props.name, addr_str);
	}

	if ((radvert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER) && !iface->ra_header_info.AdvOtherConfigFlag) {
		flog(LOG_WARNING, "our AdvOtherConfigFlag on %s doesn't agree with %s", iface->props.name, addr_str);
	}

	/* note: we don't check the default router preference here, because they're likely different */

	if ((radvert->nd_ra_reachable && iface->ra_header_info.AdvReachableTime) &&
	    (ntohl(radvert->nd_ra_reachable) != iface->ra_header_info.AdvReachableTime)) {
		flog(LOG_WARNING, "our AdvReachableTime on %s doesn't agree with %s", iface->props.name, addr_str);
	}

	if ((radvert->nd_ra_retransmit && iface->ra_header_info.AdvRetransTimer) &&
	    (ntohl(radvert->nd_ra_retransmit) != iface->ra_header_info.AdvRetransTimer)) {
		flog(LOG_WARNING, "our AdvRetransTimer on %s doesn't agree with %s", iface->props.name, addr_str);
	}

	len -= sizeof(struct nd_router_advert);

	if (len == 0)
		return;

	uint8_t *opt_str = (uint8_t *)(msg + sizeof(struct nd_router_advert));

	while (len > 0) {

		if (len < 2) {
			flog(LOG_ERR, "trailing garbage in RA on %s from %s", iface->props.name, addr_str);
			break;
		}

		int optlen = (opt_str[1] << 3);

		if (optlen == 0) {
			flog(LOG_ERR, "zero length option in RA on %s from %s", iface->props.name, addr_str);
			break;
		} else if (optlen > len) {
			flog(LOG_ERR, "option length (%d) greater than total"
				      " length (%d) in RA on %s from %s",
			     optlen, len, iface->props.name, addr_str);
			break;
		}

		switch (*opt_str) {
		case ND_OPT_MTU: {
			struct nd_opt_mtu *mtu = (struct nd_opt_mtu *)opt_str;
			if (len < sizeof(*mtu))
				return;

			if (iface->AdvLinkMTU && (ntohl(mtu->nd_opt_mtu_mtu) != iface->AdvLinkMTU)) {
				flog(LOG_WARNING, "our AdvLinkMTU on %s doesn't agree with %s", iface->props.name, addr_str);
			}
			break;
		}
		case ND_OPT_PREFIX_INFORMATION: {
			struct nd_opt_prefix_info *pinfo = (struct nd_opt_prefix_info *)opt_str;
			if (len < sizeof(*pinfo))
				return;
			int preferred = ntohl(pinfo->nd_opt_pi_preferred_time);
			int valid = ntohl(pinfo->nd_opt_pi_valid_time);

			struct AdvPrefix *prefix = iface->AdvPrefixList;
			while (prefix) {
				char prefix_str[INET6_ADDRSTRLEN];
				if ((prefix->PrefixLen == pinfo->nd_opt_pi_prefix_len) &&
				    addr_match(&prefix->Prefix, &pinfo->nd_opt_pi_prefix, prefix->PrefixLen)) {
					addrtostr(&prefix->Prefix, prefix_str, sizeof(prefix_str));

					if (!prefix->DecrementLifetimesFlag && valid != prefix->AdvValidLifetime) {
						flog(LOG_WARNING, "our AdvValidLifetime on"
								  " %s for %s doesn't agree with %s",
						     iface->props.name, prefix_str, addr_str);
					}
					if (!prefix->DecrementLifetimesFlag && preferred != prefix->AdvPreferredLifetime) {
						flog(LOG_WARNING, "our AdvPreferredLifetime on"
								  " %s for %s doesn't agree with %s",
						     iface->props.name, prefix_str, addr_str);
					}
				}

				prefix = prefix->next;
			}
			break;
		}
		case ND_OPT_ROUTE_INFORMATION:
			/* not checked: these will very likely vary a lot */
			break;
		case ND_OPT_SOURCE_LINKADDR:
			/* not checked */
			break;
		case ND_OPT_TARGET_LINKADDR:
		case ND_OPT_REDIRECTED_HEADER:
			flog(LOG_ERR, "invalid option %d in RA on %s from %s", (int)*opt_str, iface->props.name, addr_str);
			break;
		/* Mobile IPv6 extensions */
		case ND_OPT_RTR_ADV_INTERVAL:
		case ND_OPT_HOME_AGENT_INFO:
			/* not checked */
			break;
		case ND_OPT_RDNSS_INFORMATION: {
			char rdnss_str[INET6_ADDRSTRLEN];
			struct AdvRDNSS *rdnss = 0;
			struct nd_opt_rdnss_info_local *rdnssinfo = (struct nd_opt_rdnss_info_local *)opt_str;
			if (len < sizeof(*rdnssinfo))
				return;
			int count = rdnssinfo->nd_opt_rdnssi_len;

			/* Check the RNDSS addresses received */
			switch (count) {
			case 7:
				rdnss = iface->AdvRDNSSList;
				if (!check_rdnss_presence(rdnss, &rdnssinfo->nd_opt_rdnssi_addr3)) {
					/* no match found in iface->AdvRDNSSList */
					addrtostr(&rdnssinfo->nd_opt_rdnssi_addr3, rdnss_str, sizeof(rdnss_str));
					flog(LOG_WARNING, "RDNSS address %s received on %s from %s is not advertised by us",
					     rdnss_str, iface->props.name, addr_str);
				}
			/* FALLTHROUGH */
			case 5:
				rdnss = iface->AdvRDNSSList;
				if (!check_rdnss_presence(rdnss, &rdnssinfo->nd_opt_rdnssi_addr2)) {
					/* no match found in iface->AdvRDNSSList */
					addrtostr(&rdnssinfo->nd_opt_rdnssi_addr2, rdnss_str, sizeof(rdnss_str));
					flog(LOG_WARNING, "RDNSS address %s received on %s from %s is not advertised by us",
					     rdnss_str, iface->props.name, addr_str);
				}
			/* FALLTHROUGH */
			case 3:
				rdnss = iface->AdvRDNSSList;
				if (!check_rdnss_presence(rdnss, &rdnssinfo->nd_opt_rdnssi_addr1)) {
					/* no match found in iface->AdvRDNSSList */
					addrtostr(&rdnssinfo->nd_opt_rdnssi_addr1, rdnss_str, sizeof(rdnss_str));
					flog(LOG_WARNING, "RDNSS address %s received on %s from %s is not advertised by us",
					     rdnss_str, iface->props.name, addr_str);
				}

				break;
			default:
				flog(LOG_ERR, "invalid len %i in RDNSS option on %s from %s", count, iface->props.name, addr_str);
			}

			break;
		}
		case ND_OPT_DNSSL_INFORMATION: {
			struct nd_opt_dnssl_info_local *dnsslinfo = (struct nd_opt_dnssl_info_local *)opt_str;
			if (len < sizeof(*dnsslinfo))
				return;

			for (int offset = 0; offset < (dnsslinfo->nd_opt_dnssli_len - 1) * 8;) {
				char suffix[256] = {""};
				if (&dnsslinfo->nd_opt_dnssli_suffixes[offset] - opt_str >= len)
					return;
				int label_len = dnsslinfo->nd_opt_dnssli_suffixes[offset++];

				if (label_len == 0) {
					/*
					 * Ignore empty suffixes. They're
					 * probably just padding...
					 */
					if (suffix[0] == '\0')
						continue;

					if (!check_dnssl_presence(iface->AdvDNSSLList, suffix)) {
						flog(LOG_WARNING,
						     "DNSSL suffix %s received on %s from %s is not advertised by us", suffix,
						     iface->props.name, addr_str);
					}

					suffix[0] = '\0';
					continue;
				}

				/*
				 * 1) must not overflow int: label + 2, offset + label_len
				 * 2) last byte of dnssli_suffix must not overflow opt_str + len
				 */
				if ((sizeof(suffix) - strlen(suffix)) < (label_len + 2) || label_len >= (INT_MAX - 1) ||
				    &dnsslinfo->nd_opt_dnssli_suffixes[offset + label_len] - opt_str >= len ||
				    offset + label_len < offset) {
					flog(LOG_ERR, "oversized suffix in DNSSL option on %s from %s", iface->props.name,
					     addr_str);
					break;
				}

				if (suffix[0] != '\0')
					strcat(suffix, ".");
				strncat(suffix, (char *)&dnsslinfo->nd_opt_dnssli_suffixes[offset], label_len);
				offset += label_len;
			}
			break;
		}
		default:
			dlog(LOG_DEBUG, 1, "unknown option %d in RA on %s from %s", (int)*opt_str, iface->props.name, addr_str);
			break;
		}

		len -= optlen;
		opt_str += optlen;
	}
	dlog(LOG_DEBUG, 2, "processed RA on %s", iface->props.name);
}

static int addr_match(struct in6_addr *a1, struct in6_addr *a2, int prefixlen)
{
	unsigned int pdw = prefixlen >> 0x05; /* num of whole uint32_t in prefix */
	if (pdw) {
		if (memcmp(a1, a2, pdw << 2))
			return 0;
	}

	unsigned int pbi = prefixlen & 0x1f; /* num of bits in incomplete uint32_t in prefix */
	if (pbi) {
		uint32_t w1 = *((uint32_t *)a1 + pdw);
		uint32_t w2 = *((uint32_t *)a2 + pdw);

		uint32_t mask = htonl(((uint32_t)0xffffffff) << (0x20 - pbi));

		if ((w1 ^ w2) & mask)
			return 0;
	}

	return 1;
}
