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
void ospf_hello_send(struct ospf_iface *ifa, int kind, struct ospf_neighbor *dirn);

#define OHS_HELLO    0
#define OHS_POLL     1
#define OHS_SHUTDOWN 2

#endif /* _BIRD_OSPF_HELLO_H_ */
