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
void ospf_if_notify(struct proto *p, unsigned flags, struct iface *iface);
void ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a);
void ospf_iface_info(struct ospf_iface *ifa);
void ospf_iface_new(struct ospf_area *oa, struct ifa *addr, struct ospf_iface_patt *ip);
void ospf_iface_new_vlink(struct proto_ospf *po, struct ospf_iface_patt *ip);
void ospf_iface_remove(struct ospf_iface *ifa);
void ospf_iface_shutdown(struct ospf_iface *ifa);
int ospf_iface_reconfigure(struct ospf_iface *ifa, struct ospf_iface_patt *new);
void ospf_ifaces_reconfigure(struct ospf_area *oa, struct ospf_area_config *nac);

int ospf_iface_assure_bufsize(struct ospf_iface *ifa, uint plen);

void ospf_open_vlink_sk(struct proto_ospf *po);

struct nbma_node *find_nbma_node_in(list *nnl, ip_addr ip);

static inline struct nbma_node *
find_nbma_node(struct ospf_iface *ifa, ip_addr ip)
{ return find_nbma_node_in(&ifa->nbma_list, ip); }

#endif /* _BIRD_OSPF_IFACE_H_ */
