/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_OSPF_TOPOLOGY_H_
#define _BIRD_OSPF_TOPOLOGY_H_

struct top_hash_entry
{				/* Index for fast mapping (type,rtrid,LSid)->vertex */
  snode n;
  node cn;			/* For adding into list of candidates
				   in intra-area routing table calculation */
  struct top_hash_entry *next;	/* Next in hash chain */
  struct ospf_lsa_header lsa;
  u32 domain;			/* Area ID for area-wide LSAs, Iface ID for link-wide LSAs */
  //  struct ospf_area *oa;
  void *lsa_body;
  bird_clock_t inst_t;		/* Time of installation into DB */
  struct mpnh *nhs;		/* Computed nexthops - valid only in ospf_rt_spf() */
  ip_addr lb;			/* In OSPFv2, link back address. In OSPFv3, any global address in the area useful for vlinks */
#ifdef OSPFv3
  u32 lb_id;			/* Interface ID of link back iface (for bcast or NBMA networks) */
#endif
  u32 dist;			/* Distance from the root */
  u16 ini_age;
  u8 color;
#define OUTSPF 0
#define CANDIDATE 1
#define INSPF 2
  u8 nhs_reuse;			/* Whether nhs nodes can be reused during merging.
				   See a note in rt.c:merge_nexthops() */
};

struct top_graph
{
  pool *pool;			/* Pool we allocate from */
  slab *hash_slab;		/* Slab for hash entries */
  struct top_hash_entry **hash_table;	/* Hashing (modelled a`la fib) */
  unsigned int hash_size;
  unsigned int hash_order;
  unsigned int hash_mask;
  unsigned int hash_entries;
  unsigned int hash_entries_min, hash_entries_max;
};

struct top_graph *ospf_top_new(pool *);
void ospf_top_free(struct top_graph *);
void ospf_top_dump(struct top_graph *, struct proto *);
u32 ospf_lsa_domain(u32 type, struct ospf_iface *ifa);
struct top_hash_entry *ospf_hash_find_header(struct top_graph *f, u32 domain,
					     struct ospf_lsa_header *h);
struct top_hash_entry *ospf_hash_get_header(struct top_graph *f, u32 domain,
					    struct ospf_lsa_header *h);

struct top_hash_entry *ospf_hash_find(struct top_graph *, u32 domain, u32 lsa, u32 rtr,
				      u32 type);
struct top_hash_entry *ospf_hash_get(struct top_graph *, u32 domain, u32 lsa, u32 rtr,
				     u32 type);
void ospf_hash_delete(struct top_graph *, struct top_hash_entry *);
void originate_rt_lsa(struct ospf_area *oa);
void update_rt_lsa(struct ospf_area *oa);
void originate_net_lsa(struct ospf_iface *ifa);
void update_net_lsa(struct ospf_iface *ifa);
void update_link_lsa(struct ospf_iface *ifa);
int can_flush_lsa(struct proto_ospf *po);

void originate_sum_net_lsa(struct ospf_area *oa, struct fib_node *fn, int metric);
void originate_sum_rt_lsa(struct ospf_area *oa, struct fib_node *fn, int metric, u32 options UNUSED);
void flush_sum_lsa(struct ospf_area *oa, struct fib_node *fn, int type);
void originate_ext_lsa(struct ospf_area *oa, struct fib_node *fn, int src, u32 metric, ip_addr fwaddr, u32 tag, int pbit);
void flush_ext_lsa(struct ospf_area *oa, struct fib_node *fn, int nssa);


#ifdef OSPFv2
struct top_hash_entry * ospf_hash_find_net(struct top_graph *f, u32 domain, u32 lsa);

static inline struct top_hash_entry *
ospf_hash_find_rt(struct top_graph *f, u32 domain, u32 rtr)
{
  return ospf_hash_find(f, domain, rtr, rtr, LSA_T_RT);
}

#else /* OSPFv3 */
struct top_hash_entry * ospf_hash_find_rt(struct top_graph *f, u32 domain, u32 rtr);
struct top_hash_entry * ospf_hash_find_rt_first(struct top_graph *f, u32 domain, u32 rtr);
struct top_hash_entry * ospf_hash_find_rt_next(struct top_hash_entry *e);
#endif


#endif /* _BIRD_OSPF_TOPOLOGY_H_ */
