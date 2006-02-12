/*
 *   $Id: radvdump.c,v 1.7 2001/12/31 11:40:56 psavola Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *    Marko Myllynen		<myllynen@lut.fi>
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s),
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

char usage_str[] = "[-vhfe] [-d level]";

#ifdef HAVE_GETOPT_LONG
struct option prog_opt[] = {
	{"debug", 1, 0, 'd'},
	{"file-format", 0, 0, 'f'},
	{"exclude-defaults", 0, 0, 'e'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{NULL, 0, 0, 0}
};
#endif

char *pname;
int sock = -1;

void version(void);
void usage(void);
void print_ra(unsigned char *, int, struct sockaddr_in6 *, int, int);
void print_ff(unsigned char *, int, struct sockaddr_in6 *, int, int, int);

int
main(int argc, char *argv[])
{
	unsigned char msg[MSG_SIZE];
	int c, len, hoplimit;
	int fform = 0;
	int edefs = 0;
	struct sockaddr_in6 rcv_addr;
        struct in6_pktinfo *pkt_info = NULL;
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
#endif

	pname = ((pname=strrchr(argv[0],'/')) != NULL)?pname+1:argv[0];

	/* parse args */
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "d:fehv", prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, "d:fehv")) > 0)
#endif
	{
		switch (c) {
		case 'd':
			set_debuglevel(atoi(optarg));
			break;
		case 'f':
			fform = 1;
			break;
		case 'e':
			edefs = 1;
			break;
		case 'v':
			version();
			break;
		case 'h':
			usage();
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname,
				prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}
	
	if (log_open(L_STDERR, pname, NULL, 0) < 0)
		exit(1);

	/* get a raw socket for sending and receiving ICMPv6 messages */
	sock = open_icmpv6_socket();
	if (sock < 0)
		exit(1);
		
	for(;;)
	{
	        len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit);
   	     	if (len > 0)
       	 	{
			struct icmp6_hdr *icmph;
	
			/*
			 * can this happen?
			 */

			if (len < sizeof(struct icmp6_hdr))
			{
				log(LOG_WARNING, "received icmpv6 packet with invalid length: %d",
					len);
				exit(1);
			}

			icmph = (struct icmp6_hdr *) msg;

			if (icmph->icmp6_type != ND_ROUTER_SOLICIT &&
			    icmph->icmp6_type != ND_ROUTER_ADVERT)
			{
				/*
				 *	We just want to listen to RSs and RAs
				 */
			
				log(LOG_ERR, "icmpv6 filter failed");
				exit(1);
			}

			dlog(LOG_DEBUG, 4, "receiver if_index: %d", pkt_info->ipi6_ifindex);

			if (icmph->icmp6_type == ND_ROUTER_SOLICIT)
			{
				/* not yet */	
			}
			else if (icmph->icmp6_type == ND_ROUTER_ADVERT)
			{
				if (!fform)
					print_ra(msg, len, &rcv_addr, hoplimit, pkt_info->ipi6_ifindex);
				else
					print_ff(msg, len, &rcv_addr, hoplimit, pkt_info->ipi6_ifindex, edefs);
			}
        	}
		else if (len == 0)
       	 	{
       	 		log(LOG_ERR, "received zero lenght packet");
       	 		exit(1);
        	}
        	else
        	{
			log(LOG_ERR, "recv_rs_ra: %s", strerror(errno));
			exit(1);
        	}
        }                                                                                            

	exit(0);
}

void
print_ra(unsigned char *msg, int len, struct sockaddr_in6 *addr, int hoplimit, int if_index)
{
	struct nd_router_advert *radvert;
	char addr_str[INET6_ADDRSTRLEN];
	uint8_t *opt_str;
	int i;
	char if_name[IFNAMSIZ];

	if (get_debuglevel() > 2)
	{
		int j;
		char hd[64];
		
		dlog(LOG_DEBUG, 3, "Hexdump of packet (length %d):", len);
		
		for (j = 0; j < len; j++)
		{
			sprintf(hd+3*(j%16), "%02X ", (unsigned int) msg[j]);
				
			if (!((j+1)%16) || (j+1) == len)
			{
				dlog(LOG_DEBUG, 3, hd);
			}
		}
		
		dlog(LOG_DEBUG, 4, "Hexdump printed");
	}

	print_addr(&addr->sin6_addr, addr_str);
	printf("Router advertisement from %s (hoplimit %d)\n", addr_str, hoplimit);
	if_indextoname(if_index, if_name);
	printf("Received by interface %s\n", if_name);

	radvert = (struct nd_router_advert *) msg;

	printf("\t# Note: {Min,Max}RtrAdvInterval cannot be obtained with radvdump\n");

	printf("\tAdvCurHopLimit: %d\n", (int) radvert->nd_ra_curhoplimit);

	printf("\tAdvManagedFlag: %s\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED)?"on":"off");

	printf("\tAdvOtherConfigFlag: %s\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER)?"on":"off");

	/* Mobile IPv6 ext */
	printf("\tAdvHomeAgentFlag: %s\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT)?"on":"off");

	printf("\tAdvReachableTime: %lu\n", (unsigned long)ntohl(radvert->nd_ra_reachable));

	printf("\tAdvRetransTimer: %lu\n", (unsigned long)ntohl(radvert->nd_ra_retransmit));

	len -= sizeof(struct nd_router_advert);

	if (len == 0)
		return;
		
	opt_str = (uint8_t *)(msg + sizeof(struct nd_router_advert));
		
	while (len > 0)
	{
		int optlen;
		struct nd_opt_prefix_info *pinfo;
		struct nd_opt_mtu *mtu;
		struct AdvInterval *a_ival;
		struct HomeAgentInfo *ha_info;
		char prefix_str[INET6_ADDRSTRLEN];

		if (len < 2)
		{
			log(LOG_ERR, "trailing garbage in RA from %s", 
				addr_str);
			break;
		}
		
		optlen = (opt_str[1] << 3);

		dlog(LOG_DEBUG,4, "option type %d, length %d", (int)*opt_str,
			optlen);

		if (optlen == 0) 
		{
			log(LOG_ERR, "zero length option in RA");
			break;
		} 
		else if (optlen > len)
		{
			log(LOG_ERR, "option length greater than total"
				" length in RA (type %d, optlen %d, len %d)", 
				(int)*opt_str, optlen, len);
			break;
		} 		

		switch (*opt_str)
		{
		case ND_OPT_MTU:
			mtu = (struct nd_opt_mtu *)opt_str;

			printf("\tAdvLinkMTU: %lu\n", (unsigned long)ntohl(mtu->nd_opt_mtu_mtu));
			break;
		case ND_OPT_PREFIX_INFORMATION:
			pinfo = (struct nd_opt_prefix_info *) opt_str;
			
			print_addr(&pinfo->nd_opt_pi_prefix, prefix_str);	
				
			printf("\tPrefix %s/%d\n", prefix_str, pinfo->nd_opt_pi_prefix_len);
	
	
			if (ntohl(pinfo->nd_opt_pi_valid_time) == 0xffffffff)
			{		
				printf("\t\tAdvValidLifetime: infinity (0xffffffff)\n");
			}
			else
			{
				printf("\t\tAdvValidLifetime: %lu\n", (unsigned long) ntohl(pinfo->nd_opt_pi_valid_time));
			}
			if (ntohl(pinfo->nd_opt_pi_preferred_time) == 0xffffffff)
			{
				printf("\t\tAdvPreferredLifetime: infinity (0xffffffff)\n");
			}
			else
			{
				printf("\t\tAdvPreferredLifetime: %lu\n", (unsigned long) ntohl(pinfo->nd_opt_pi_preferred_time));
			}
			printf("\t\tAdvOnLink: %s\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK)?"on":"off");

			printf("\t\tAdvAutonomous: %s\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO)?"on":"off");

			/* Mobile IPv6 ext */
			printf("\t\tAdvRouterAddr: %s\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_RADDR)?"on":"off");

			break;
		case ND_OPT_SOURCE_LINKADDR:
			printf("\tAdvSourceLLAddress: ");
			
			for (i = 2; i < optlen; i++)
			{
				printf("%02X ", (unsigned int) opt_str[i]);
			}
			
			printf("\n");
			break;
		/* Mobile IPv6 ext */
		case ND_OPT_RTR_ADV_INTERVAL:
			a_ival = (struct AdvInterval *)opt_str;

			printf("\tAdvIntervalOpt:\n");
			printf("\t\tAdvInterval: %lu", (unsigned long)ntohl(a_ival->adv_ival) / 1000);
			printf("\n");
			break;
		/* Mobile IPv6 ext */
		case ND_OPT_HOME_AGENT_INFO:
			ha_info = (struct HomeAgentInfo *)opt_str;

			printf("\tAdvHomeAgentInfo:\n");
			printf("\t\tHomeAgentPreference: %hu", (unsigned short)ntohs(ha_info->preference));
			printf("\n");
			printf("\t\tHomeAgentLifetime: %hu", (unsigned short)ntohs(ha_info->lifetime));
			printf("\n");
			break;
		case ND_OPT_TARGET_LINKADDR:
		case ND_OPT_REDIRECTED_HEADER:
			log(LOG_ERR, "invalid option %d in RA", (int)*opt_str);
			break;
		default:
			dlog(LOG_DEBUG, 1, "unknown option %d in RA",
				(int)*opt_str);
			break;
		}
		
		len -= optlen;
		opt_str += optlen;
	}
	
	fflush(stdout);
}

void
print_ff(unsigned char *msg, int len, struct sockaddr_in6 *addr, int hoplimit, int if_index, int edefs)
{
	struct nd_router_advert *radvert;
	char addr_str[INET6_ADDRSTRLEN];
	uint8_t *opt_str;
	int orig_len = len;
	char if_name[IFNAMSIZ];

	print_addr(&addr->sin6_addr, addr_str);
	printf("#\n# radvd configuration generated by radvdump %s\n", VERSION);
	printf("# based on Router Advertisement from %s\n", addr_str);
	if_indextoname(if_index, if_name);
	printf("# received by interface %s\n", if_name);
	printf("#\n\ninterface %s\n{\n\tAdvSendAdvert on;\n", if_name);

	printf("\t# Note: {Min,Max}RtrAdvInterval cannot be obtained with radvdump\n");

	radvert = (struct nd_router_advert *) msg;

	if (!edefs || DFLT_AdvManagedFlag != (ND_RA_FLAG_MANAGED == (radvert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED)))
	printf("\tAdvManagedFlag %s;\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED)?"on":"off");

	if (!edefs || DFLT_AdvOtherConfigFlag != (ND_RA_FLAG_OTHER == (radvert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER)))
	printf("\tAdvOtherConfigFlag %s;\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER)?"on":"off");

	if (!edefs || DFLT_AdvReachableTime != (unsigned long)ntohl(radvert->nd_ra_reachable))
	printf("\tAdvReachableTime %lu;\n", (unsigned long)ntohl(radvert->nd_ra_reachable));

	if (!edefs || DFLT_AdvRetransTimer != (unsigned long)ntohl(radvert->nd_ra_retransmit))
	printf("\tAdvRetransTimer %lu;\n", (unsigned long)ntohl(radvert->nd_ra_retransmit));

	if (!edefs || DFLT_AdvCurHopLimit != (int) radvert->nd_ra_curhoplimit)
	printf("\tAdvCurHopLimit %d;\n", (int) radvert->nd_ra_curhoplimit);

	/* Mobile IPv6 ext */
	if (!edefs || DFLT_AdvHomeAgentFlag != (ND_RA_FLAG_HOME_AGENT == (radvert->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT)))
	printf("\tAdvHomeAgentFlag %s;\n", 
		(radvert->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT)?"on":"off");

	len -= sizeof(struct nd_router_advert);

	if (len == 0)
		return;
		
	opt_str = (uint8_t *)(msg + sizeof(struct nd_router_advert));
		
	while (len > 0)
	{
		int optlen;
		struct nd_opt_mtu *mtu;
		struct AdvInterval *a_ival;
		struct HomeAgentInfo *ha_info;

		if (len < 2)
		{
			log(LOG_ERR, "trailing garbage in RA from %s", 
				addr_str);
			break;
		}
		
		optlen = (opt_str[1] << 3);

		if (optlen == 0) 
		{
			log(LOG_ERR, "zero length option in RA");
			break;
		} 
		else if (optlen > len)
		{
			log(LOG_ERR, "option length greater than total"
				" length in RA (type %d, optlen %d, len %d)", 
				(int)*opt_str, optlen, len);
			break;
		} 		

		switch (*opt_str)
		{
		case ND_OPT_MTU:
			mtu = (struct nd_opt_mtu *)opt_str;

			if (!edefs || DFLT_AdvLinkMTU != (unsigned long)ntohl(mtu->nd_opt_mtu_mtu))
			printf("\tAdvLinkMTU %lu;\n", (unsigned long)ntohl(mtu->nd_opt_mtu_mtu));
			break;
		case ND_OPT_SOURCE_LINKADDR:
			/* XXX: !DFLT depends on current DFLT_ value */
			if (!edefs || !DFLT_AdvSourceLLAddress)
			printf("\tAdvSourceLLAddress on;\n");
			break;
		/* Mobile IPv6 ext */
		case ND_OPT_RTR_ADV_INTERVAL:
			a_ival = (struct AdvInterval *)opt_str;

			/* XXX: !DFLT depends on current DFLT_ value */
			if (!edefs || !DFLT_AdvIntervalOpt)
			printf("\tAdvIntervalOpt on;\n");
			break;
		/* Mobile IPv6 ext */
		case ND_OPT_HOME_AGENT_INFO:
			ha_info = (struct HomeAgentInfo *)opt_str;

			/* XXX: !DFLT depends on current DFLT_ value */
			if (!edefs || !DFLT_AdvHomeAgentInfo)
			printf("\tAdvHomeAgentInfo on;\n");

			if (!edefs || DFLT_HomeAgentPreference != (unsigned short)ntohs(ha_info->preference))
			printf("\tHomeAgentPreference %hu;\n", (unsigned short)ntohs(ha_info->preference));
			/* Hum.. */
			if (!edefs || (3*DFLT_MaxRtrAdvInterval) != (unsigned short)ntohs(ha_info->lifetime))
			printf("\tHomeAgentLifetime %hu;\n", (unsigned short)ntohs(ha_info->lifetime));
			break;
		case ND_OPT_TARGET_LINKADDR:
		case ND_OPT_REDIRECTED_HEADER:
			log(LOG_ERR, "invalid option %d in RA", (int)*opt_str);
			break;
		case ND_OPT_PREFIX_INFORMATION:
			break;
		default:
			dlog(LOG_DEBUG, 1, "unknown option %d in RA",
				(int)*opt_str);
			break;
		}
		
		len -= optlen;
		opt_str += optlen;
	}

	orig_len -= sizeof(struct nd_router_advert);

	if (orig_len == 0)
		return;

	opt_str = (uint8_t *)(msg + sizeof(struct nd_router_advert));
		
	while (orig_len > 0)
	{
		int optlen;
		struct nd_opt_prefix_info *pinfo;
		char prefix_str[INET6_ADDRSTRLEN];

		if (orig_len < 2)
		{
			log(LOG_ERR, "trailing garbage in RA from %s", 
				addr_str);
			break;
		}
		
		optlen = (opt_str[1] << 3);

		if (optlen == 0) 
		{
			log(LOG_ERR, "zero length option in RA");
			break;
		} 
		else if (optlen > orig_len)
		{
			log(LOG_ERR, "option length greater than total"
				" length in RA (type %d, optlen %d, len %d)", 
				(int)*opt_str, optlen, orig_len);
			break;
		} 		

		switch (*opt_str)
		{
		case ND_OPT_PREFIX_INFORMATION:
			pinfo = (struct nd_opt_prefix_info *) opt_str;
			
			print_addr(&pinfo->nd_opt_pi_prefix, prefix_str);	
				
			printf("\n\tprefix %s/%d\n\t{\n", prefix_str, pinfo->nd_opt_pi_prefix_len);

			if (ntohl(pinfo->nd_opt_pi_valid_time) == 0xffffffff)
			{		
				if (!edefs || DFLT_AdvValidLifetime != 0xffffffff)
				printf("\t\tAdvValidLifetime infinity; # (0xffffffff)\n");
			}
			else
			{
				if (!edefs || DFLT_AdvValidLifetime != (unsigned long)ntohl(pinfo->nd_opt_pi_valid_time))
				printf("\t\tAdvValidLifetime %lu;\n", (unsigned long)ntohl(pinfo->nd_opt_pi_valid_time));
			}
			if (ntohl(pinfo->nd_opt_pi_preferred_time) == 0xffffffff)
			{
				if (!edefs || DFLT_AdvPreferredLifetime != 0xffffffff)
				printf("\t\tAdvPreferredLifetime infinity; # (0xffffffff)\n");
			}
			else
			{
				if (!edefs || DFLT_AdvPreferredLifetime != (unsigned long)ntohl(pinfo->nd_opt_pi_preferred_time))
				printf("\t\tAdvPreferredLifetime %lu;\n", (unsigned long)ntohl(pinfo->nd_opt_pi_preferred_time));
			}

			if (!edefs || DFLT_AdvOnLinkFlag != (ND_OPT_PI_FLAG_ONLINK == (pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK)))
			printf("\t\tAdvOnLink %s;\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK)?"on":"off");

			if (!edefs || DFLT_AdvAutonomousFlag != (ND_OPT_PI_FLAG_AUTO == (pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO)))
			printf("\t\tAdvAutonomous %s;\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO)?"on":"off");

			/* Mobile IPv6 ext */
			if (!edefs || DFLT_AdvRouterAddr != (ND_OPT_PI_FLAG_RADDR == (pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_RADDR)))
			printf("\t\tAdvRouterAddr %s;\n", 
				(pinfo->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_RADDR)?"on":"off");

			printf("\t}; # End of prefix definition\n\n");
			break;
		default:
			break;
		}
		orig_len -= optlen;
		opt_str += optlen;
	}

	printf("}; # End of interface definition\n");

	fflush(stdout);
}

void
version(void)
{
	fprintf(stderr,"Version: %s\n\n", VERSION);
	fprintf(stderr,"Please send bug reports and suggestions to %s\n",
		CONTACT_EMAIL);
	exit(1);	
}

void
usage(void)
{
	fprintf(stderr,"usage: %s %s\n", pname, usage_str);
	exit(1);	
}
