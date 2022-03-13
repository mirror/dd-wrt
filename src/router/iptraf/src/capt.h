#ifndef IPTRAF_NG_CAPT_H
#define IPTRAF_NG_CAPT_H

/*
 * Number of bytes from captured packet to move into a buffer.
 * 96 bytes should be enough for the IP header, TCP/UDP/ICMP/whatever header
 * with reasonable numbers of options.
 *
 * keep it aligned to 16 !!!
 */
#define MAX_PACKET_SIZE 96

#include "list.h"

struct capt {
	int		fd;
	unsigned long	dropped;

	struct list_head promisc;

	void		*priv;

	bool		(*have_packet)(struct capt *capt);
	int		(*get_packet)(struct capt *capt, struct pkt_hdr *pkt);
	int		(*put_packet)(struct capt *capt, struct pkt_hdr *pkt);
	unsigned long	(*get_dropped)(struct capt *capt);
	void		(*cleanup)(struct capt *capt);
};

int capt_get_socket(struct capt *capt);
void capt_put_socket(struct capt *capt);
int capt_init(struct capt *capt, char *ifname);
void capt_destroy(struct capt *capt);
unsigned long capt_get_dropped(struct capt *capt);
int capt_get_packet(struct capt *capt, struct pkt_hdr *pkt, int *ch, WINDOW *win);
int capt_put_packet(struct capt *capt, struct pkt_hdr *pkt);

#endif	/* IPTRAF_NG_CAPT_H */
