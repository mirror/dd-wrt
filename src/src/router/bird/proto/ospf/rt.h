/*
 *      BIRD -- OSPF
 *
 *      (c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_RT_H_
#define _BIRD_OSPF_RT_H_

#define ORT_UNDEF -1
#define ORT_ROUTER 1
#define ORT_NET 0

typedef struct orta
{
  int type;
  int capa;
#define ORTA_ASBR 1
#define ORTA_ABR 2
  struct ospf_area *oa;
  u32 metric1;
  u32 metric2;
  ip_addr nh;			/* Next hop */
  struct ospf_iface *ifa;	/* Outgoing interface */
  struct top_hash_entry *ar;	/* Advertising router */
  u32 tag;
}
orta;

typedef struct ort
{
  struct fib_node fn;
  orta n;
  orta o;
  struct ort *efn;		/* For RFC1583 */
}
ort;

void ospf_rt_spf(struct proto_ospf *po);
void ospf_rt_initort(struct fib_node *fn);


#endif /* _BIRD_OSPF_RT_H_ */
