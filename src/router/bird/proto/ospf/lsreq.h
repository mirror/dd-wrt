/*
 *      BIRD -- OSPF
 *
 *      (c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_LSREQ_H_
#define _BIRD_OSPF_LSREQ_H_

void ospf_lsreq_send(struct ospf_neighbor *n);
void ospf_lsreq_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
			struct ospf_neighbor *n);

#endif /* _BIRD_OSPF_LSREQ_H_ */
