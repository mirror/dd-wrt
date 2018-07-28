#ifndef IPTRAF_NG_IPFRAG_H
#define IPTRAF_NG_IPFRAG_H

/***

ipfrag.h - IP fragmentation hander definitions

***/

static inline unsigned int ipv4_frag_offset(struct iphdr *ip)
{
	return (ntohs(ip->frag_off) & 0x1fff) * 8;
}

static inline int ipv4_is_first_fragment(struct iphdr *ip)
{
	return (ntohs(ip->frag_off) & 0x1fff) == 0;
}

static inline int ipv4_is_fragmented(struct iphdr *ip)
{
	return (ntohs(ip->frag_off) & 0x3fff) != 0;
}

static inline int ipv4_more_fragments(struct iphdr *ip)
{
	return (ntohs(ip->frag_off) & 0x2000) != 0;
}

void destroyfraglist(void);
unsigned int processfragment(struct iphdr *packet, in_port_t *sport,
			     in_port_t *dport, int *firstin);

#endif	/* IPTRAF_NG_IPFRAG_H */
