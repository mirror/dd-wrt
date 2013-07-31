/*
 *      BIRD -- OSPF
 *
 *      (c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_LSACK_H_
#define _BIRD_OSPF_LSACK_H_

struct lsah_n
{
  node n;
  struct ospf_lsa_header lsa;
};

void ospf_lsack_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
			struct ospf_neighbor *n);
void ospf_lsack_send(struct ospf_neighbor *n, int queue);
void ospf_lsack_enqueue(struct ospf_neighbor *n, struct ospf_lsa_header *h,
			int queue);

#endif /* _BIRD_OSPF_LSACK_H_ */
