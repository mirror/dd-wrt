/*
 *      BIRD -- OSPF
 *
 *      (c) 1999--2005 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_IFACE_H_
#define _BIRD_OSPF_IFACE_H_

void ospf_iface_chstate(struct ospf_iface *ifa, u8 state);
void ospf_iface_sm(struct ospf_iface *ifa, int event);
struct ospf_iface *ospf_iface_find(struct proto_ospf *p, struct iface *what);
void ospf_iface_notify(struct proto *p, unsigned flags, struct iface *iface);
void ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a);
void ospf_iface_info(struct ospf_iface *ifa);
void ospf_iface_shutdown(struct ospf_iface *ifa);
void ospf_iface_new(struct proto_ospf *po, struct iface *iface, struct ifa *addr, struct ospf_area_config *ac, struct ospf_iface_patt *ip);
void ospf_iface_change_mtu(struct proto_ospf *po, struct ospf_iface *ifa);
void ospf_set_rxbuf_size(struct ospf_iface *ifa, u32 rxbuf);



#endif /* _BIRD_OSPF_IFACE_H_ */
