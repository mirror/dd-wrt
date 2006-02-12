/*
 *   $Id: process.c,v 1.5 2001/11/14 19:58:11 lutchann Exp $
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
 *   may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>

static void process_rs(int, struct Interface *, struct sockaddr_in6 *);
static void process_ra(struct Interface *, unsigned char *msg, int len,
	struct sockaddr_in6 *);
static int  addr_match(struct in6_addr *a1, struct in6_addr *a2,
	int prefixlen);

void
process(int sock, struct Interface *ifacel, unsigned char *msg, int len, 
	struct sockaddr_in6 *addr, struct in6_pktinfo *pkt_info, int hoplimit)
{
	struct Interface *iface;
	struct icmp6_hdr *icmph;
	char addr_str[INET6_ADDRSTRLEN];

	if ( ! pkt_info )
	{
		log(LOG_WARNING, "received packet with no pkt_info!" );
		return;
	}

	/*
	 * can this happen?
	 */

	if (len < sizeof(struct icmp6_hdr))
	{
		log(LOG_WARNING, "received icmpv6 packet with invalid length: %d",
			len);
		return;
	}

	icmph = (struct icmp6_hdr *) msg;

	if (icmph->icmp6_type != ND_ROUTER_SOLICIT &&
	    icmph->icmp6_type != ND_ROUTER_ADVERT)
	{
		/*
		 *	We just want to listen to RSs and RAs
		 */
		
		log(LOG_ERR, "icmpv6 filter failed");
		return;
	}

	if (icmph->icmp6_type == ND_ROUTER_ADVERT)
	{
		if (len < sizeof(struct nd_router_advert)) {
			log(LOG_WARNING, "received icmpv6 RA packet with invalid length: %d",
				len);
			return;
		}

		if (!IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)) {
			log(LOG_WARNING, "received icmpv6 RA packet with non-linklocal source address");
			return;
		}
	}			
	
	if (icmph->icmp6_type == ND_ROUTER_SOLICIT)
	{
		if (len < sizeof(struct nd_router_solicit)) {
			log(LOG_WARNING, "received icmpv6 RS packet with invalid length: %d",
				len);
			return;
		}
		
		/* 
		 * XXX: RFC2461 6.1.1: if RS src addr is the unspecified address,
		 *	and there is a lladdr option, the packet MUST be dropped
		 */
	}			

	if (icmph->icmp6_code != 0)
	{
		log(LOG_WARNING, "received icmpv6 RS/RA packet with invalid code: %d",
			icmph->icmp6_code);
		return;
	}
	
	dlog(LOG_DEBUG, 4, "if_index %d", pkt_info->ipi6_ifindex);

	/* get iface by received if_index */

	for (iface = ifacel; iface; iface=iface->next)
	{
		if (iface->if_index == pkt_info->ipi6_ifindex)
		{
			break;
		}
	}

	if (iface == NULL)
	{
		dlog(LOG_DEBUG, 2, "received packet from unknown interface: %d",
			pkt_info->ipi6_ifindex);
		return;
	}
	
	if (hoplimit != 255)
	{
		print_addr(&addr->sin6_addr, addr_str);
		log(LOG_WARNING, "received RS or RA with invalid hoplimit %d from %s",
			hoplimit, addr_str);
		return;
	}
	
	if (!iface->AdvSendAdvert)
	{
		dlog(LOG_DEBUG, 2, "AdvSendAdvert is off for %s", iface->Name);
		return;
	}

	dlog(LOG_DEBUG, 4, "found Interface: %s", iface->Name);

	if (icmph->icmp6_type == ND_ROUTER_SOLICIT)
	{
		process_rs(sock, iface, addr);
	}
	else if (icmph->icmp6_type == ND_ROUTER_ADVERT)
	{
		process_ra(iface, msg, len, addr);
	}
}

static void
process_rs(int sock, struct Interface *iface, struct sockaddr_in6 *addr)
{
	int delay;
	double next;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	delay = (int) (MAX_RA_DELAY_TIME*rand()/(RAND_MAX+1.0));
	dlog(LOG_DEBUG, 3, "random mdelay for %s: %d", iface->Name, delay);
	mdelay(delay);
 	
	if (iface->UnicastOnly
		|| tv.tv_sec - iface->last_multicast < MIN_DELAY_BETWEEN_RAS)
	{
		/*
		 *	unicast reply
		 */

		send_ra(sock, iface, &addr->sin6_addr);
	}
	else
	{
		/*
		 *	multicast reply
		 */

		clear_timer(&iface->tm);
		send_ra(sock, iface, NULL);
		
		next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval); 
		set_timer(&iface->tm, next);
	}
}

/*
 * check router advertisements according to RFC 2461, 6.2.7
 */
static void
process_ra(struct Interface *iface, unsigned char *msg, int len, 
	struct sockaddr_in6 *addr)
{
	struct nd_router_advert *radvert;
	char addr_str[INET6_ADDRSTRLEN];
	uint8_t *opt_str;

	print_addr(&addr->sin6_addr, addr_str);

	radvert = (struct nd_router_advert *) msg;

	if ((radvert->nd_ra_curhoplimit && iface->AdvCurHopLimit) && 
	   (radvert->nd_ra_curhoplimit != iface->AdvCurHopLimit))
	{
		log(LOG_WARNING, "our AdvCurHopLimit on %s doesn't agree with %s",
			iface->Name, addr_str);
	}

	if ((radvert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED) && !iface->AdvManagedFlag)
	{
		log(LOG_WARNING, "our AdvManagedFlag on %s doesn't agree with %s",
			iface->Name, addr_str);
	}
	
	if ((radvert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER) && !iface->AdvOtherConfigFlag)
	{
		log(LOG_WARNING, "our AdvOtherConfigFlag on %s doesn't agree with %s",
			iface->Name, addr_str);
	}

	if ((radvert->nd_ra_reachable && iface->AdvReachableTime) &&
	   (ntohl(radvert->nd_ra_reachable) != iface->AdvReachableTime))
	{
		log(LOG_WARNING, "our AdvReachableTime on %s doesn't agree with %s",
			iface->Name, addr_str);
	}
	
	if ((radvert->nd_ra_retransmit && iface->AdvRetransTimer) &&
	   (ntohl(radvert->nd_ra_retransmit) != iface->AdvRetransTimer))
	{
		log(LOG_WARNING, "our AdvRetransTimer on %s doesn't agree with %s",
			iface->Name, addr_str);
	}

	len -= sizeof(struct nd_router_advert);

	if (len == 0)
		return;
		
	opt_str = (uint8_t *)(msg + sizeof(struct nd_router_advert));
		
	while (len > 0)
	{
		int optlen;
		struct nd_opt_prefix_info *pinfo;
		struct nd_opt_mtu *mtu;
		struct AdvPrefix *prefix;
		char prefix_str[INET6_ADDRSTRLEN];
		uint32_t preferred, valid;

		if (len < 2)
		{
			log(LOG_ERR, "trailing garbage in RA on %s from %s", 
				iface->Name, addr_str);
			break;
		}
		
		optlen = (opt_str[1] << 3);

		if (optlen == 0) 
		{
			log(LOG_ERR, "zero length option in RA on %s from %s",
				iface->Name, addr_str);
			break;
		} 
		else if (optlen > len)
		{
			log(LOG_ERR, "option length greater than total"
				" length in RA on %s from %s",
				iface->Name, addr_str);
			break;
		} 		

		switch (*opt_str)
		{
		case ND_OPT_MTU:
			mtu = (struct nd_opt_mtu *)opt_str;

			if (iface->AdvLinkMTU && (ntohl(mtu->nd_opt_mtu_mtu) != iface->AdvLinkMTU))
			{
				log(LOG_WARNING, "our AdvLinkMTU on %s doesn't agree with %s",
					iface->Name, addr_str);
			}
			break;
		case ND_OPT_PREFIX_INFORMATION:
			pinfo = (struct nd_opt_prefix_info *) opt_str;
			preferred = ntohl(pinfo->nd_opt_pi_preferred_time);
			valid = ntohl(pinfo->nd_opt_pi_valid_time);
			
			prefix = iface->AdvPrefixList;
			while (prefix)
			{
				if (prefix->enabled &&
				    (prefix->PrefixLen == pinfo->nd_opt_pi_prefix_len) &&
				    addr_match(&prefix->Prefix, &pinfo->nd_opt_pi_prefix,
				    	 prefix->PrefixLen))
				{
					print_addr(&prefix->Prefix, prefix_str);

					if (valid != prefix->AdvValidLifetime)
					{
						log(LOG_WARNING, "our AdvValidLifetime on"
						 " %s for %s doesn't agree with %s",
						 iface->Name,
						 prefix_str,
						 addr_str
						 );
					}
					if (preferred != prefix->AdvPreferredLifetime)
					{
						log(LOG_WARNING, "our AdvPreferredLifetime on"
						 " %s for %s doesn't agree with %s",
						 iface->Name,
						 prefix_str,
						 addr_str
						 );
					}
				}

				prefix = prefix->next;
			}			
			break;
		case ND_OPT_SOURCE_LINKADDR:
			/* not checked */
			break;
		case ND_OPT_TARGET_LINKADDR:
		case ND_OPT_REDIRECTED_HEADER:
			log(LOG_ERR, "invalid option %d in RA on %s from %s",
				(int)*opt_str, iface->Name, addr_str);
			break;
		/* Mobile IPv6 extensions */
		case ND_OPT_RTR_ADV_INTERVAL:
		case ND_OPT_HOME_AGENT_INFO:
			/* not checked */
			break;
		default:
			dlog(LOG_DEBUG, 1, "unknown option %d in RA on %s from %s",
				(int)*opt_str, iface->Name, addr_str);
			break;
		}
		
		len -= optlen;
		opt_str += optlen;
	}
}

static int 
addr_match(struct in6_addr *a1, struct in6_addr *a2, int prefixlen)
{
	int pdw;
	int pbi;

	pdw = prefixlen >> 0x05;  /* num of whole uint32_t in prefix */
	pbi = prefixlen &  0x1f;  /* num of bits in incomplete uint32_t in prefix */

	if (pdw) 
	{
		if (memcmp(a1, a2, pdw << 2))
			return 0;
	}

	if (pbi) 
	{
		uint32_t w1, w2;
		uint32_t mask;

		w1 = *((uint32_t *)a1 + pdw);
		w2 = *((uint32_t *)a2 + pdw);

		mask = htonl(((uint32_t) 0xffffffff) << (0x20 - pbi));

		if ((w1 ^ w2) & mask)
			return 0;
	}

	return 1;
}
