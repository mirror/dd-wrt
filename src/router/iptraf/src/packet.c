/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

packet.c - routines to open the raw socket, read socket data and
           adjust the initial packet pointer

***/

#include "iptraf-ng-compat.h"

#include "deskman.h"
#include "error.h"
#include "options.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "ipfilter.h"
#include "isdntab.h"
#include "ifaces.h"
#include "packet.h"
#include "ipfrag.h"

/* Reimplement again
 * Removed PPP, LINK_ISDN
 */

extern int daemonized;

/*
int isdnfd;
struct isdntab isdntable;
*/

/* code taken from http://www.faqs.org/rfcs/rfc1071.html. See section 4.1 "C"  */
static int in_cksum(u_short * addr, int len)
{
	register int sum = 0;

	while (len > 1) {
		sum += *(u_short *) addr++;
		len -= 2;
	}

	if (len > 0)
		sum += *(unsigned char *) addr;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (u_short) (~sum);
}

static int packet_adjust(struct pkt_hdr *pkt)
{
	int retval = 0;

	switch (pkt->pkt_hatype) {
	case ARPHRD_ETHER:
	case ARPHRD_LOOPBACK:
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += ETH_HLEN;
		pkt->pkt_len -= ETH_HLEN;
		if (pkt->pkt_protocol == ETH_P_8021Q) {
			/* strip 0x8100 802.1Q VLAN Extended Header  */
			pkt->pkt_payload += 4;
			pkt->pkt_len -= 4;
			/* update network protocol */
			pkt->pkt_protocol = ntohs(*((unsigned short *) pkt->pkt_payload));
		}
		break;
	case ARPHRD_SLIP:
	case ARPHRD_CSLIP:
	case ARPHRD_SLIP6:
	case ARPHRD_CSLIP6:
	case ARPHRD_PPP:
	case ARPHRD_TUNNEL:
	case ARPHRD_SIT:
	case ARPHRD_NONE:
	case ARPHRD_IPGRE:
		pkt->pkt_payload = pkt->pkt_buf;
		break;
	case ARPHRD_FRAD:
	case ARPHRD_DLCI:
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += 4;
		pkt->pkt_len -= 4;
		break;
	case ARPHRD_FDDI:
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += sizeof(struct fddihdr);
		pkt->pkt_len -= sizeof(struct fddihdr);
		break;
	default:
		/* return a NULL packet to signal an unrecognized link */
		/* protocol to the caller.  Hopefully, this switch statement */
		/* will grow. */
		pkt->pkt_payload = NULL;
		retval = -1;
		break;
	}
	return retval;
}

/* IPTraf input function; reads both keystrokes and network packets. */
int packet_get(int fd, struct pkt_hdr *pkt, int *ch, WINDOW *win)
{
	struct pollfd pfds[2];
	nfds_t nfds = 0;
	int ss;

	/* Monitor raw socket */
	pfds[0].fd = fd;
	pfds[0].events = POLLIN;
	nfds++;

	/* Monitor stdin only if in interactive, not daemon mode. */
	if (!daemonized) {
		pfds[1].fd = 0;
		pfds[1].events = POLLIN;
		nfds++;
	}
	do {
		ss = poll(pfds, nfds, DEFAULT_UPDATE_DELAY / 1000);
	} while ((ss == -1) && (errno == EINTR));

	pkt->pkt_len = 0;	/* signalize we have no packet prepared */
	if ((ss > 0) && (pfds[0].revents & POLLIN) != 0) {
		struct sockaddr_ll from;
		socklen_t fromlen = sizeof(struct sockaddr_ll);
		ssize_t len;

		len = recvfrom(fd, pkt->pkt_buf, pkt->pkt_bufsize,
			       MSG_TRUNC | MSG_DONTWAIT,
			       (struct sockaddr *) &from, &fromlen);
		if (len > 0) {
			pkt->pkt_len = len;
			pkt->pkt_caplen = len;
			if (pkt->pkt_caplen > pkt->pkt_bufsize)
				pkt->pkt_caplen = pkt->pkt_bufsize;
			pkt->pkt_payload = NULL;
			pkt->pkt_protocol = ntohs(from.sll_protocol);
			pkt->pkt_ifindex = from.sll_ifindex;
			pkt->pkt_hatype = from.sll_hatype;
			pkt->pkt_pkttype = from.sll_pkttype;
		} else
			ss = len;
	}

	*ch = ERR;	/* signalize we have no key ready */
	if (!daemonized && (ss > 0) && ((pfds[1].revents & POLLIN) != 0))
		*ch = wgetch(win);

	return ss;
}

int packet_process(struct pkt_hdr *pkt, unsigned int *total_br,
		   unsigned int *sport, unsigned int *dport,
		   struct filterstate *filter, int match_opposite,
		   int v6inv4asv6)
{
	/* move packet pointer (pkt->pkt_payload) past data link header */
	if (packet_adjust(pkt) != 0)
		return INVALID_PACKET;

again:	if (pkt->pkt_protocol == ETH_P_IP) {
		struct iphdr *ip;
		int hdr_check;
		register int ip_checksum;
		register int iphlen;
		unsigned int f_sport = 0, f_dport = 0;

		/*
		 * At this point, we're now processing IP packets.  Start by getting
		 * IP header and length.
		 */
		ip = (struct iphdr *) (pkt->pkt_payload);
		iphlen = ip->ihl * 4;

		/*
		 * Compute and verify IP header checksum.
		 */

		ip_checksum = ip->check;
		ip->check = 0;
		hdr_check = in_cksum((u_short *) ip, iphlen);

		if ((hdr_check != ip_checksum))
			return CHECKSUM_ERROR;

		if ((ip->protocol == IPPROTO_TCP || ip->protocol == IPPROTO_UDP)
		    && (sport != NULL && dport != NULL)) {
			unsigned int sport_tmp, dport_tmp;

			/*
			 * Process TCP/UDP fragments
			 */
			if ((ntohs(ip->frag_off) & 0x3fff) != 0) {
				int firstin;

				/*
				 * total_br contains total byte count of all fragments
				 * not yet retrieved.  Will differ only if fragments
				 * arrived before the first fragment, in which case
				 * the total accumulated fragment sizes will be returned
				 * once the first fragment arrives.
				 */

				if (total_br != NULL)
					*total_br =
					    processfragment(ip, &sport_tmp,
							    &dport_tmp,
							    &firstin);

				if (!firstin)
					return MORE_FRAGMENTS;
			} else {
				struct tcphdr *tcp;
				struct udphdr *udp;
				char *ip_payload = (char *) ip + iphlen;

				switch (ip->protocol) {
				case IPPROTO_TCP:
					tcp = (struct tcphdr *) ip_payload;
					sport_tmp = tcp->source;
					dport_tmp = tcp->dest;
					break;
				case IPPROTO_UDP:
					udp = (struct udphdr *) ip_payload;
					sport_tmp = udp->source;
					dport_tmp = udp->dest;
					break;
				default:
					sport_tmp = 0;
					dport_tmp = 0;
					break;
				}

				if (total_br != NULL)
					*total_br = pkt->pkt_len;
			}

			if (sport != NULL)
				*sport = sport_tmp;

			if (dport != NULL)
				*dport = dport_tmp;

			/*
			 * Process IP filter
			 */
			f_sport = ntohs(sport_tmp);
			f_dport = ntohs(dport_tmp);
		}
		if ((filter->filtercode != 0)
		    &&
		    (!ipfilter
		     (ip->saddr, ip->daddr, f_sport, f_dport, ip->protocol,
		      match_opposite, &(filter->fl))))
			return PACKET_FILTERED;
		if (v6inv4asv6 && (ip->protocol == IPPROTO_IPV6)) {
			pkt->pkt_protocol = ETH_P_IPV6;
			pkt->pkt_payload += iphlen;
			pkt->pkt_len -= iphlen;
			goto again;
		}
		return PACKET_OK;
	} else if (pkt->pkt_protocol == ETH_P_IPV6) {
		struct tcphdr *tcp;
		struct udphdr *udp;
		struct ip6_hdr *ip6 = (struct ip6_hdr *) pkt->pkt_payload;
		char *ip_payload = (char *) ip6 + 40;

		//TODO: Filter packets
		switch (ip6->ip6_nxt) {	/* FIXME: extension headers ??? */
		case IPPROTO_TCP:
			tcp = (struct tcphdr *) ip_payload;
			if (sport)
				*sport = tcp->source;
			if (dport)
				*dport = tcp->dest;
			break;
		case IPPROTO_UDP:
			udp = (struct udphdr *) ip_payload;
			if (sport)
				*sport = udp->source;
			if (dport)
				*dport = udp->dest;
			break;
		default:
			if (sport)
				*sport = 0;
			if (dport)
				*dport = 0;
			break;
		}
	} else {
		/* not IPv4 and not IPv6: apply non-IP packet filter */
		if (!nonipfilter(filter, pkt->pkt_protocol)) {
			return PACKET_FILTERED;
		}
	}
	return PACKET_OK;
}

void pkt_cleanup(void)
{
	// close(isdnfd);
	// isdnfd = -1;
	destroyfraglist();
	// destroy_isdn_table(&isdntable);
}
