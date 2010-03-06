/*
 *      BIRD -- OSPF
 *
 *      (c) 1999 - 2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_HELLO_H_
#define _BIRD_OSPF_HELLO_H_

void ospf_hello_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
			struct ospf_neighbor *n, ip_addr faddr);
void ospf_hello_send(timer *timer, int poll, struct ospf_neighbor *dirn);

#endif /* _BIRD_OSPF_HELLO_H_ */
