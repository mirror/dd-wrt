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
void ospf_lsupd_receive(struct ospf_lsupd_packet *ps,
			struct ospf_iface *ifa, struct ospf_neighbor *n);
int ospf_lsupd_flood(struct ospf_neighbor *n, struct ospf_lsa_header *hn,
		     struct ospf_lsa_header *hh, struct ospf_iface *iff,
		     struct ospf_area *oa, int rtl);
void ospf_lsupd_flush_nlsa(struct top_hash_entry *en, struct ospf_area *oa);

#endif /* _BIRD_OSPF_LSUPD_H_ */
