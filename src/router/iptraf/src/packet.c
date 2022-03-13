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
#include "ifaces.h"
#include "packet.h"
#include "ipfrag.h"

#define pkt_cast_hdrp_l2off_t(hdr, pkt, off)			\
	do {							\
		pkt->hdr = (struct hdr *) (pkt->pkt_buf + off);	\
	} while (0)

#define pkt_cast_hdrp_l2(hdr, pkt)				\
	pkt_cast_hdrp_l2off_t(hdr, pkt, 0)


#define pkt_cast_hdrp_l3off_t(hdr, pkt, off)				\
	do {								\
		pkt->hdr = (struct hdr *) (pkt->pkt_payload + off);	\
	} while (0)

#define pkt_cast_hdrp_l3(hdr, pkt)					\
		pkt_cast_hdrp_l3off_t(hdr, pkt, 0)

/* code taken from http://www.faqs.org/rfcs/rfc1071.html. See section 4.1 "C"  */
static int verify_ipv4_hdr_chksum(struct iphdr *ip)
{
	register int sum = 0;
	u_short *addr = (u_short *)ip;
	int len = ip->ihl * 4;

	while (len > 1) {
		sum += *addr++;
		len -= 2;
	}

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ((u_short) ~sum) == 0;
}

static int packet_adjust(struct pkt_hdr *pkt)
{
	int retval = 0;

	switch (pkt->from->sll_hatype) {
	case ARPHRD_ETHER:
	case ARPHRD_LOOPBACK:
		pkt_cast_hdrp_l2(ethhdr, pkt);
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += ETH_HLEN;
		pkt->pkt_len -= ETH_HLEN;
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
		pkt_cast_hdrp_l2(fddihdr, pkt);
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += sizeof(struct fddihdr);
		pkt->pkt_len -= sizeof(struct fddihdr);
		break;
	case ARPHRD_INFINIBAND:
		pkt->pkt_payload = pkt->pkt_buf;
		pkt->pkt_payload += 24;
		pkt->pkt_len -= 24;
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

/* initialize all layer3 protocol pointers (we need to initialize all
 * of them, because of case we change pkt->pkt_protocol) */
static void packet_set_l3_hdrp(struct pkt_hdr *pkt)
{
	switch (pkt->pkt_protocol) {
	case ETH_P_IP:
		pkt_cast_hdrp_l3(iphdr, pkt);
		pkt->ip6_hdr = NULL;
		break;
	case ETH_P_IPV6:
		pkt->iphdr = NULL;
		pkt_cast_hdrp_l3(ip6_hdr, pkt);
		break;
	default:
		pkt->iphdr = NULL;
		pkt->ip6_hdr = NULL;
		break;
	}
}

int packet_process(struct pkt_hdr *pkt, unsigned int *total_br,
		   in_port_t *sport, in_port_t *dport,
		   int match_opposite, int v6inv4asv6)
{
	/* move packet pointer (pkt->pkt_payload) past data link header */
	if (packet_adjust(pkt) != 0)
		return INVALID_PACKET;

again:
	packet_set_l3_hdrp(pkt);
	switch (pkt->pkt_protocol) {
	case ETH_P_IP: {
		struct iphdr *ip = pkt->iphdr;
		in_port_t f_sport = 0, f_dport = 0;

		if (!verify_ipv4_hdr_chksum(ip))
			return CHECKSUM_ERROR;

		if ((ip->protocol == IPPROTO_TCP || ip->protocol == IPPROTO_UDP)
		    && (sport != NULL && dport != NULL)) {
			in_port_t sport_tmp, dport_tmp;

			/*
			 * Process TCP/UDP fragments
			 */
			if (ipv4_is_fragmented(ip)) {
				int firstin = 0;

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
				char *ip_payload = (char *) ip + pkt_iph_len(pkt);

				switch (ip->protocol) {
				case IPPROTO_TCP:
					tcp = (struct tcphdr *) ip_payload;
					sport_tmp = ntohs(tcp->source);
					dport_tmp = ntohs(tcp->dest);
					break;
				case IPPROTO_UDP:
					udp = (struct udphdr *) ip_payload;
					sport_tmp = ntohs(udp->source);
					dport_tmp = ntohs(udp->dest);
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

			f_sport = sport_tmp;
			f_dport = dport_tmp;
		}
		/* Process IP filter */
		if ((ofilter.filtercode != 0)
		    &&
		    (!ipfilter
		     (ip->saddr, ip->daddr, f_sport, f_dport, ip->protocol,
		      match_opposite)))
			return PACKET_FILTERED;
		if (v6inv4asv6 && (ip->protocol == IPPROTO_IPV6)) {
			pkt->pkt_protocol = ETH_P_IPV6;
			pkt->pkt_payload += pkt_iph_len(pkt);
			pkt->pkt_len -= pkt_iph_len(pkt);
			goto again;
		}
		break; }
	case ETH_P_IPV6: {
		struct tcphdr *tcp;
		struct udphdr *udp;
		struct ip6_hdr *ip6 = pkt->ip6_hdr;
		char *ip_payload = (char *) ip6 + pkt_iph_len(pkt);

		//TODO: Filter packets
		switch (pkt_ip_protocol(pkt)) {
		case IPPROTO_TCP:
			tcp = (struct tcphdr *) ip_payload;
			if (sport)
				*sport = ntohs(tcp->source);
			if (dport)
				*dport = ntohs(tcp->dest);
			break;
		case IPPROTO_UDP:
			udp = (struct udphdr *) ip_payload;
			if (sport)
				*sport = ntohs(udp->source);
			if (dport)
				*dport = ntohs(udp->dest);
			break;
		default:
			if (sport)
				*sport = 0;
			if (dport)
				*dport = 0;
			break;
		}
		break; }
	case ETH_P_8021Q:
	case ETH_P_QINQ1:	/* ETH_P_QINQx are not officially */
	case ETH_P_QINQ2:	/* registered IDs */
	case ETH_P_QINQ3:
	case ETH_P_8021AD:
		/* strip 802.1Q/QinQ/802.1ad VLAN header */
		pkt->pkt_payload += 4;
		pkt->pkt_len -= 4;
		/* update network protocol */
		pkt->pkt_protocol = ntohs(*((unsigned short *) pkt->pkt_payload));
		goto again;
	default:
		/* not IPv4 and not IPv6: apply non-IP packet filter */
		if (!nonipfilter(pkt->pkt_protocol)) {
			return PACKET_FILTERED;
		}
	}
	return PACKET_OK;
}

int packet_init(struct pkt_hdr *pkt)
{
	pkt->pkt_payload	= NULL;
	pkt->ethhdr		= NULL;
	pkt->fddihdr		= NULL;
	pkt->iphdr		= NULL;
	pkt->ip6_hdr		= NULL;
	pkt->pkt_len		= 0;	/* signalize we have no packet prepared */

	pkt->pkt_buf		= NULL;
	pkt->from		= NULL;

	return 0;	/* all O.K. */
}

void packet_destroy(struct pkt_hdr *pkt __unused)
{
	destroyfraglist();
}

int packet_is_first_fragment(struct pkt_hdr *pkt)
{
	switch (pkt->pkt_protocol) {
	case ETH_P_IP:
		return ipv4_is_first_fragment(pkt->iphdr);
	case ETH_P_IPV6:
		/* FIXME: IPv6 can also be fragmented !!! */
		/* fall through for now */
	default:
		return !0;
	}
}

static char *pkttype_to_string(unsigned int type)
{
	switch(type) {
	case PACKET_HOST:	return "PACKET_HOST";
	case PACKET_BROADCAST:	return "PACKET_BROADCAST";
	case PACKET_MULTICAST:	return "PACKET_MULTICAST";
	case PACKET_OTHERHOST:	return "PACKET_OTHERHOST";
	case PACKET_OUTGOING:	return "PACKET_OUTGOING";
	case PACKET_LOOPBACK:	return "PACKET_LOOPBACK";
	case PACKET_USER:	return "PACKET_USER";
	case PACKET_KERNEL:	return "PACKET_KERNEL";
	default:		return NULL;
	}
}

static char *l2_type_to_string(unsigned int type)
{
	switch(type) {
	case 0:		return "ARPHRD_NETROM";
	case 0xFFFE:	return "ARPHRD_NONE";
	case 0xFFFF:	return "ARPHRD_VOID";
	case 1:		return "ARPHRD_ETHER";
	case 2:		return "ARPHRD_EETHER";
	case 3:		return "ARPHRD_AX25";
	case 4:		return "ARPHRD_PRONET";
	case 5:		return "ARPHRD_CHAOS";
	case 6:		return "ARPHRD_IEEE802";
	case 7:		return "ARPHRD_ARCNET";
	case 8:		return "ARPHRD_APPLETLK";
	case 15:	return "ARPHRD_DLCI";
	case 19:	return "ARPHRD_ATM";
	case 23:	return "ARPHRD_METRICOM";
	case 24:	return "ARPHRD_IEEE1394";
	case 27:	return "ARPHRD_EUI64";
	case 32:	return "ARPHRD_INFINIBAND";
	case 256:	return "ARPHRD_SLIP";
	case 257:	return "ARPHRD_CSLIP";
	case 258:	return "ARPHRD_SLIP6";
	case 259:	return "ARPHRD_CSLIP6";
	case 260:	return "ARPHRD_RSRVD";
	case 264:	return "ARPHRD_ADAPT";
	case 270:	return "ARPHRD_ROSE";
	case 271:	return "ARPHRD_X25";
	case 272:	return "ARPHRD_HWX25";
	case 280:	return "ARPHRD_CAN";
	case 512:	return "ARPHRD_PPP";
	case 513:	return "ARPHRD_CISCO";
	case 516:	return "ARPHRD_LAPB";
	case 517:	return "ARPHRD_DDCMP";
	case 518:	return "ARPHRD_RAWHDLC";
	case 519:	return "ARPHRD_RAWIP";
	case 768:	return "ARPHRD_TUNNEL";
	case 769:	return "ARPHRD_TUNNEL6";
	case 770:	return "ARPHRD_FRAD";
	case 771:	return "ARPHRD_SKIP";
	case 772:	return "ARPHRD_LOOPBACK";
	case 773:	return "ARPHRD_LOCALTLK";
	case 774:	return "ARPHRD_FDDI";
	case 775:	return "ARPHRD_BIF";
	case 776:	return "ARPHRD_SIT";
	case 777:	return "ARPHRD_IPDDP";
	case 778:	return "ARPHRD_IPGRE";
	case 779:	return "ARPHRD_PIMREG";
	case 780:	return "ARPHRD_HIPPI";
	case 781:	return "ARPHRD_ASH";
	case 782:	return "ARPHRD_ECONET";
	case 783:	return "ARPHRD_IRDA";
	case 784:	return "ARPHRD_FCPP";
	case 785:	return "ARPHRD_FCAL";
	case 786:	return "ARPHRD_FCPL";
	case 787:	return "ARPHRD_FCFABRIC";
	case 800:	return "ARPHRD_IEEE802_TR";
	case 801:	return "ARPHRD_IEEE80211";
	case 802:	return "ARPHRD_IEEE80211_PRISM";
	case 803:	return "ARPHRD_IEEE80211_RADIOTAP";
	case 804:	return "ARPHRD_IEEE802154";
	case 805:	return "ARPHRD_IEEE802154_MONITOR";
	case 820:	return "ARPHRD_PHONET";
	case 821:	return "ARPHRD_PHONET_PIPE";
	case 822:	return "ARPHRD_CAIF";
	case 823:	return "ARPHRD_IP6GRE";
	case 824:	return "ARPHRD_NETLINK";
	case 825:	return "ARPHRD_6LOWPAN";
	case 826:	return "ARPHRD_VSOCKMON";
	default:	return NULL;
	}
}

static char *l3_proto_to_string(unsigned short protocol)
{
	switch(protocol) {
	case 0x0001:	return "ETH_P_802_3";
	case 0x0002:	return "ETH_P_AX25";
	case 0x0003:	return "ETH_P_ALL";
	case 0x0004:	return "ETH_P_802_2";
	case 0x0005:	return "ETH_P_SNAP";
	case 0x0006:	return "ETH_P_DDCMP";
	case 0x0007:	return "ETH_P_WAN_PPP";
	case 0x0008:	return "ETH_P_PPP_MP";
	case 0x0009:	return "ETH_P_LOCALTALK";
	case 0x000C:	return "ETH_P_CAN";
	case 0x000D:	return "ETH_P_CANFD";
	case 0x0010:	return "ETH_P_PPPTALK";
	case 0x0011:	return "ETH_P_TR_802_2";
	case 0x0015:	return "ETH_P_MOBITEX";
	case 0x0016:	return "ETH_P_CONTROL";
	case 0x0017:	return "ETH_P_IRDA";
	case 0x0018:	return "ETH_P_ECONET";
	case 0x0019:	return "ETH_P_HDLC";
	case 0x001A:	return "ETH_P_ARCNET";
	case 0x001B:	return "ETH_P_DSA";
	case 0x001C:	return "ETH_P_TRAILER";
	case 0x0060:	return "ETH_P_LOOP";
	case 0x00F5:	return "ETH_P_PHONET";
	case 0x00F6:	return "ETH_P_IEEE802154";
	case 0x00F7:	return "ETH_P_CAIF";
	case 0x00F8:	return "ETH_P_XDSA";
	case 0x00F9:	return "ETH_P_MAP";
	case 0x0200:	return "ETH_P_PUP";
	case 0x0201:	return "ETH_P_PUPAT";
	case 0x0800:	return "ETH_P_IP";
	case 0x0805:	return "ETH_P_X25";
	case 0x0806:	return "ETH_P_ARP";
	case 0x08FF:	return "ETH_P_BPQ";
	case 0x0a00:	return "ETH_P_IEEEPUP";
	case 0x0a01:	return "ETH_P_IEEEPUPAT";
	case 0x22EB:	return "ETH_P_ERSPAN2";
	case 0x22F0:	return "ETH_P_TSN";
	case 0x4305:	return "ETH_P_BATMAN";
	case 0x6000:	return "ETH_P_DEC";
	case 0x6001:	return "ETH_P_DNA_DL";
	case 0x6002:	return "ETH_P_DNA_RC";
	case 0x6003:	return "ETH_P_DNA_RT";
	case 0x6004:	return "ETH_P_LAT";
	case 0x6005:	return "ETH_P_DIAG";
	case 0x6006:	return "ETH_P_CUST";
	case 0x6007:	return "ETH_P_SCA";
	case 0x6558:	return "ETH_P_TEB";
	case 0x8035:	return "ETH_P_RARP";
	case 0x809B:	return "ETH_P_ATALK";
	case 0x80F3:	return "ETH_P_AARP";
	case 0x8100:	return "ETH_P_8021Q";
	case 0x8137:	return "ETH_P_IPX";
	case 0x86DD:	return "ETH_P_IPV6";
	case 0x8808:	return "ETH_P_PAUSE";
	case 0x8809:	return "ETH_P_SLOW";
	case 0x883E:	return "ETH_P_WCCP";
	case 0x8847:	return "ETH_P_MPLS_UC";
	case 0x8848:	return "ETH_P_MPLS_MC";
	case 0x884c:	return "ETH_P_ATMMPOA";
	case 0x8863:	return "ETH_P_PPP_DISC";
	case 0x8864:	return "ETH_P_PPP_SES";
	case 0x886c:	return "ETH_P_LINK_CTL";
	case 0x8884:	return "ETH_P_ATMFATE";
	case 0x888E:	return "ETH_P_PAE";
	case 0x88A2:	return "ETH_P_AOE";
	case 0x88A8:	return "ETH_P_8021AD";
	case 0x88B5:	return "ETH_P_802_EX1";
	case 0x88BE:	return "ETH_P_ERSPAN";
	case 0x88C7:	return "ETH_P_PREAUTH";
	case 0x88CA:	return "ETH_P_TIPC";
	case 0x88CC:	return "ETH_P_LLDP";
	case 0x88E5:	return "ETH_P_MACSEC";
	case 0x88E7:	return "ETH_P_8021AH";
	case 0x88F5:	return "ETH_P_MVRP";
	case 0x88F7:	return "ETH_P_1588";
	case 0x88F8:	return "ETH_P_NCSI";
	case 0x88FB:	return "ETH_P_PRP";
	case 0x8906:	return "ETH_P_FCOE";
	case 0x890D:	return "ETH_P_TDLS";
	case 0x8914:	return "ETH_P_FIP";
	case 0x8915:	return "ETH_P_IBOE";
	case 0x8917:	return "ETH_P_80221";
	case 0x892F:	return "ETH_P_HSR";
	case 0x894F:	return "ETH_P_NSH";
	case 0x9000:	return "ETH_P_LOOPBACK";
	case 0x9100:	return "ETH_P_QINQ1";
	case 0x9200:	return "ETH_P_QINQ2";
	case 0x9300:	return "ETH_P_QINQ3";
	case 0xDADA:	return "ETH_P_EDSA";
	case 0xDADB:	return "ETH_P_DSA_8021Q";
	case 0xED3E:	return "ETH_P_IFE";
	case 0xFBFB:	return "ETH_P_AF_IUCV";
	default:	return NULL;
	}
}

void packet_dump(struct pkt_hdr *pkt, FILE *fp) {
	unsigned i = 0;
	unsigned len;
	char *adr;
	char *str;

	if(pkt == NULL)
		return;

	len = pkt->pkt_caplen;
	if(len == 0)
		return;

	if (pkt->pkt_caplen != pkt->pkt_len)
		fprintf(fp, "length: %zu (out of %zu) bytes\n", pkt->pkt_caplen, pkt->pkt_len);
	else
		fprintf(fp, "length: %zu bytes\n", pkt->pkt_caplen);

	str = pkttype_to_string(pkt->from->sll_pkttype);
	if (str)
		fprintf(fp, "type: %s", str);
	else
		fprintf(fp, "type: %02x", pkt->from->sll_pkttype);

	str = l2_type_to_string(pkt->from->sll_hatype);
	if (str)
		fprintf(fp, ", L2-proto: %s", str);
	else
		fprintf(fp, ", L2-proto: 0x%04x", pkt->from->sll_hatype);

	str = l3_proto_to_string(pkt->pkt_protocol);
	if (str)
		fprintf(fp, ", L3-proto: %s", str);
	else
		fprintf(fp, ", L3-proto: 0x%04x", pkt->pkt_protocol);

	fprintf(fp, "\n");

	while(i < len) {
		adr = (char *)pkt->pkt_buf + i;
		if(i % 16 == 0) {
			if(i > 0)
				fprintf(fp, "\n");
			fprintf(fp, "0x%04x:", i);
		}
		if(i % 8 == 0)
			fprintf(fp, " ");
		fprintf(fp, " ");

		if(adr == (char *)pkt->ethhdr)
			fprintf(fp, "^");
		else if(adr == (char *)pkt->iphdr)
			fprintf(fp, "&");
		else if(adr == (char *)pkt->ip6_hdr)
			fprintf(fp, "*");
		else
			fprintf(fp, " ");

		fprintf(fp, "%02x", *(unsigned char *)adr & 0xff);
		i++;
	}
	fprintf(fp, "\n\n");
	fflush(fp);
}
