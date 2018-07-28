#ifndef IPTRAF_NG_PACKET_H
#define IPTRAF_NG_PACKET_H

/***

packet.h - external declarations for packet.c

***/

/*
 * Number of bytes from captured packet to move into a buffer.
 * 96 bytes should be enough for the IP header, TCP/UDP/ICMP/whatever header
 * with reasonable numbers of options.
 */
#define MAX_PACKET_SIZE 96

#define INVALID_PACKET 0
#define PACKET_OK 1
#define CHECKSUM_ERROR 2
#define PACKET_FILTERED 3
#define MORE_FRAGMENTS 4

struct pkt_hdr {
	char	       *pkt_buf;
	size_t		pkt_bufsize;
	char	       *pkt_payload;
	size_t		pkt_caplen;	/* bytes captured */
	size_t		pkt_len;	/* bytes on-the-wire */
	unsigned short	pkt_protocol;	/* Physical layer protocol: ETH_P_* */

	struct iovec	iov;
	struct sockaddr_ll *from;
	struct msghdr  *msg;

	struct ethhdr  *ethhdr;
	struct fddihdr *fddihdr;
	struct iphdr   *iphdr;
	struct ip6_hdr *ip6_hdr;
};

static inline __u8 pkt_iph_len(const struct pkt_hdr *pkt)
{
	switch (pkt->pkt_protocol) {
	case ETH_P_IP:
		return pkt->iphdr->ihl * 4;
	case ETH_P_IPV6:
		return 40;
	default:
		return 0;
	}
}

static inline __u8 pkt_ip_protocol(const struct pkt_hdr *p)
{
	switch (p->pkt_protocol) {
	case ETH_P_IP:
		return p->iphdr->protocol;
	case ETH_P_IPV6:
		return p->ip6_hdr->ip6_nxt; /* FIXME: extension headers ??? */
	};
	return 0;
}

int packet_get(int fd, struct pkt_hdr *pkt, int *ch, WINDOW *win);
int packet_process(struct pkt_hdr *pkt, unsigned int *total_br,
		   in_port_t *sport, in_port_t *dport,
		   int match_opposite, int v6inv4asv6);
int packet_init(struct pkt_hdr *pkt);
void packet_destroy(struct pkt_hdr *pkt);
unsigned int packet_get_dropped(int fd);
int packet_is_first_fragment(struct pkt_hdr *pkt);

#endif	/* IPTRAF_NG_PACKET_H */
