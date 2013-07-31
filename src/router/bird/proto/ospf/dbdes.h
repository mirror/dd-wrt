/*
 *      BIRD -- OSPF
 *
 *      (c) 1999 - 2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_DBDES_H_
#define _BIRD_OSPF_DBDES_H_

void ospf_dbdes_send(struct ospf_neighbor *n, int next);
void ospf_dbdes_receive(struct ospf_packet *ps, struct ospf_iface *ifa,
			struct ospf_neighbor *n);

#endif /* _BIRD_OSPF_DBDES_H_ */
