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
  u32 options;			
  /* router-LSA style options (for ORT_ROUTER), with V,E,B bits.
     In OSPFv2, ASBRs from another areas (that we know from rt-summary-lsa),
     have just ORTA_ASBR in options, their real options are unknown */
#define ORTA_ASBR OPT_RT_E
#define ORTA_ABR  OPT_RT_B
  struct ospf_area *oa;
  u32 metric1;
  u32 metric2;
  ip_addr nh;			/* Next hop */
  struct ospf_iface *ifa;	/* Outgoing interface */
  struct top_hash_entry *ar;	/* Advertising router (or ABR) */
  u32 tag;
  u32 rid;			/* Router ID of real advertising router */
  /* For ext-LSA from different area, 'ar' is a type 1 LSA of ABR.
     Router ID of real advertising router is stored in 'rid'. */
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
