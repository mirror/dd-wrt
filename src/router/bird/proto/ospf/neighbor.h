/*
 *      BIRD -- OSPF
 *
 *      (c) 1999 - 2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_NEIGHBOR_H_
#define _BIRD_OSPF_NEIGHBOR_H_

struct ospf_neighbor *ospf_neighbor_new(struct ospf_iface *ifa);
void ospf_neigh_sm(struct ospf_neighbor *n, int event);
void bdr_election(struct ospf_iface *ifa);
struct ospf_neighbor *find_neigh(struct ospf_iface *ifa, u32 rid);
struct ospf_neighbor *find_neigh_by_ip(struct ospf_iface *ifa, ip_addr ip);
void ospf_neigh_remove(struct ospf_neighbor *n);
void ospf_neigh_update_bfd(struct ospf_neighbor *n, int use_bfd);
void ospf_sh_neigh_info(struct ospf_neighbor *n);

#endif /* _BIRD_OSPF_NEIGHBOR_H_ */
