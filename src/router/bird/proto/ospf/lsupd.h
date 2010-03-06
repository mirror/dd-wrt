/*
 *      BIRD -- OSPF
 *
 *      (c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_LSUPD_H_
#define _BIRD_OSPF_LSUPD_H_

void ospf_dump_lsahdr(struct proto *p, struct ospf_lsa_header *lsa_n);
void ospf_dump_common(struct proto *p, struct ospf_packet *op);
void ospf_lsupd_send_list(struct ospf_neighbor *n, list * l);
void ospf_lsupd_receive(struct ospf_packet *ps_i,
			struct ospf_iface *ifa, struct ospf_neighbor *n);
int ospf_lsupd_flood(struct proto_ospf *po,
		     struct ospf_neighbor *n, struct ospf_lsa_header *hn,
		     struct ospf_lsa_header *hh, u32 domain, int rtl);
void ospf_lsupd_flush_nlsa(struct proto_ospf *po, struct top_hash_entry *en);
int ospf_lsa_flooding_allowed(struct ospf_lsa_header *lsa, u32 domain, struct ospf_iface *ifa);


#endif /* _BIRD_OSPF_LSUPD_H_ */
