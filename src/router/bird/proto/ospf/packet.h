/*
 *      BIRD -- OSPF
 *
 *      (c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_PACKET_H_
#define _BIRD_OSPF_PACKET_H_

void ospf_pkt_fill_hdr(struct ospf_iface *ifa, void *buf, u8 h_type);
uint ospf_pkt_maxsize(struct ospf_iface *ifa);
int ospf_rx_hook(sock * sk, int size);
// void ospf_tx_hook(sock * sk);
void ospf_err_hook(sock * sk, int err);
void ospf_verr_hook(sock *sk, int err);
void ospf_send_to_agt(struct ospf_iface *ifa, u8 state);
void ospf_send_to_bdr(struct ospf_iface *ifa);
void ospf_send_to(struct ospf_iface *ifa, ip_addr ip);

static inline void ospf_send_to_all(struct ospf_iface *ifa) { ospf_send_to(ifa, ifa->all_routers); }

static inline void * ospf_tx_buffer(struct ospf_iface *ifa) { return ifa->sk->tbuf; }


#endif /* _BIRD_OSPF_PACKET_H_ */
