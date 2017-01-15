/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
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
  u16 lsa_type;			/* lsa.type processed and converted to common values (LSA_T_*) */
  u16 init_age;			/* Initial value for lsa.age during inst_time */
  u32 domain;			/* Area ID for area-wide LSAs, Iface ID for link-wide LSAs */
  //  struct ospf_area *oa;
  void *lsa_body;		/* May be NULL if LSA was flushed but hash entry was kept */
  void *next_lsa_body;		/* For postponed LSA origination */
  u16 next_lsa_blen;		/* For postponed LSA origination */
  u16 next_lsa_opts;		/* For postponed LSA origination */
  bird_clock_t inst_time;	/* Time of installation into DB */
  struct ort *nf;		/* Reference fibnode for sum and ext LSAs, NULL for otherwise */
  struct mpnh *nhs;		/* Computed nexthops - valid only in ospf_rt_spf() */
  ip_addr lb;			/* In OSPFv2, link back address. In OSPFv3, any global address in the area useful for vlinks */
  u32 lb_id;			/* Interface ID of link back iface (for bcast or NBMA networks) */
  u32 dist;			/* Distance from the root */
  int ret_count;		/* Number of retransmission lists referencing the entry */
  u8 color;
#define OUTSPF 0
#define CANDIDATE 1
#define INSPF 2
  u8 mode;			/* LSA generated during RT calculation (LSA_RTCALC or LSA_STALE)*/
  u8 nhs_reuse;			/* Whether nhs nodes can be reused during merging.
				   See a note in rt.c:add_cand() */
};


/* Prevents ospf_hash_find() to ignore the entry, for p->lsrqh and p->lsrth */
#define LSA_BODY_DUMMY ((void *) 1)

/*
 * LSA entry life cycle
 *
 * LSA entries are created by ospf_originate_lsa() (for locally originated LSAs)
 * or ospf_install_lsa() (for LSAs received from neighbors). A regular (like
 * newly originated) LSA entry has defined lsa_body nad lsa.age < %LSA_MAXAGE.
 * When the LSA is requested to be flushed by ospf_flush_lsa(), the lsa.age is
 * set to %LSA_MAXAGE and flooded. Flush process is finished asynchronously,
 * when (at least) flooding is acknowledged by neighbors. This is detected in
 * ospf_update_lsadb(), then ospf_clear_lsa() is called to free the LSA body but
 * the LSA entry is kept. Such LSA does not formally exist, we keep an empty
 * entry (until regular timeout) to know inst_time and lsa.sn in the case of
 * later reorigination. After the timeout, LSA is removed by ospf_remove_lsa().
 *
 * When LSA origination is requested (by ospf_originate_lsa()). but it is not
 * possible to do that immediately (because of MinLSInterval or because the
 * sequence number is wrapping), The new LSA is scheduled for later origination
 * in next_lsa_* fields of the LSA entry. The later origination is handled by
 * ospf_originate_next_lsa() called from ospf_update_lsadb(). We can see that
 * both real origination and final flush is asynchronous to ospf_originate_lsa()
 * and ospf_flush_lsa().
 *
 * LSA entry therefore could be in three basic states:
 * R - regular (lsa.age < %LSA_MAXAGE, lsa_body != NULL)
 * F - flushing (lsa.age == %LSA_MAXAGE, lsa_body != NULL)
 * E - empty (lsa.age == %LSA_MAXAGE, lsa_body == NULL)
 *
 * And these states are doubled based on whether the next LSA is scheduled
 * (next_lsa_body != NULL, -n suffix) or not (next_lsa_body == NULL). We also
 * use X for a state of non-existentce. We have this basic state graph
 * (transitions from any state to R are omitted for clarity):
 *
 *  X --> R ---> F ---> E --> X
 *        | \  / |      |
 *        |  \/  |      |
 *        |  /\  |      |
 *        | /  \ |      |
 *        Rn --> Fn --> En
 *
 * The transitions are:
 *
 * any state -> R		- new LSA origination requested and executed
 * R -> Rn, F -> Fn, E -> En	- new LSA origination requested and postponed
 * R -> Fn			- new LSA origination requested, seqnum wrapping
 * Rn,Fn,En -> R		- postponed LSA finally originated
 * R -> R			- LSA refresh done
 * R -> Fn			- LSA refresh with seqnum wrapping
 * R -> F, Rn -> Fn		- LSA age timeout
 * R,Rn,Fn -> F, En -> E	- LSA flush requested
 * F -> E, Fn -> En		- LSA flush done (acknowledged)
 * E -> X			- LSA real age timeout (or immediate for received LSA)
 *
 * The 'origination requested' and 'flush requested' transitions are triggered
 * and done by ospf_originate_lsa() and ospf_flush_lsa(), the rest is handled
 * asynchronously by ospf_update_lsadb().
 *
 * The situation is significantly simpler for non-local (received) LSAs - there
 * is no postponed origination and after flushing is done, LSAs are immediately
 * removed, so it is just X -> R -> F -> X, or X -> F -> X (when MaxAge LSA is
 * received).
 *
 * There are also some special cases related to handling of received unknown
 * self-originated LSAs in ospf_advance_lsa():
 * X -> F		- LSA is received and immediately flushed
 * R,Rn -> Fn		- LSA with MaxSeqNo received and flushed, current LSA scheduled
 */


#define LSA_M_BASIC	0
#define LSA_M_EXPORT	1
#define LSA_M_RTCALC	2
#define LSA_M_STALE	3

/*
 * LSA entry modes:
 *
 * LSA_M_BASIC - The LSA is explicitly originated using ospf_originate_lsa() and
 * explicitly flushed using ospf_flush_lsa(). When the LSA is changed, the
 * routing table calculation is scheduled. This is also the mode used for LSAs
 * received from neighbors. Example: Router-LSAs, Network-LSAs.
 *
 * LSA_M_EXPORT - like LSA_M_BASIC, but the routing table calculation does not
 * depend on the LSA. Therefore, the calculation is not scheduled when the LSA
 * is changed. Example: AS-external-LSAs for exported routes.
 *
 * LSA_M_RTCALC - The LSA has to be requested using ospf_originate_lsa() during
 * each routing table calculation, otherwise it is flushed automatically at the
 * end of the calculation. The LSA is a result of the calculation and not a
 * source for it. Therefore, the calculation is not scheduled when the LSA is
 * changed. Example: Summary-LSAs.
 *
 * LSA_M_STALE - Temporary state for LSA_M_RTCALC that is not requested during
 * the current routing table calculation.
 *
 *
 * Note that we do not schedule the routing table calculation when the age of
 * LSA_M_BASIC LSA is changed to MaxAge because of the sequence number wrapping,
 * As it will be switched back to a regular one ASAP.
 */


struct top_graph
{
  pool *pool;			/* Pool we allocate from */
  slab *hash_slab;		/* Slab for hash entries */
  struct top_hash_entry **hash_table;	/* Hashing (modelled a`la fib) */
  uint ospf2;			/* Whether it is for OSPFv2 or OSPFv3 */
  uint hash_size;
  uint hash_order;
  uint hash_mask;
  uint hash_entries;
  uint hash_entries_min, hash_entries_max;
};

struct ospf_new_lsa
{
  u16 type;
  u8  mode;
  u32 dom;
  u32 id;
  u16 opts;
  u16 length;
  struct ospf_iface *ifa;
  struct ort *nf;
};

struct top_graph *ospf_top_new(struct ospf_proto *p, pool *pool);
void ospf_top_free(struct top_graph *f);

struct top_hash_entry * ospf_install_lsa(struct ospf_proto *p, struct ospf_lsa_header *lsa, u32 type, u32 domain, void *body);
struct top_hash_entry * ospf_originate_lsa(struct ospf_proto *p, struct ospf_new_lsa *lsa);
void ospf_advance_lsa(struct ospf_proto *p, struct top_hash_entry *en, struct ospf_lsa_header *lsa, u32 type, u32 domain, void *body);
void ospf_flush_lsa(struct ospf_proto *p, struct top_hash_entry *en);
void ospf_update_lsadb(struct ospf_proto *p);

static inline void ospf_flush2_lsa(struct ospf_proto *p, struct top_hash_entry **en)
{ if (*en) { ospf_flush_lsa(p, *en); *en = NULL; } }

void ospf_originate_sum_net_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, int metric);
void ospf_originate_sum_rt_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, int metric, u32 options);
void ospf_originate_ext_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, u8 mode, u32 metric, u32 ebit, ip_addr fwaddr, u32 tag, int pbit);

void ospf_rt_notify(struct proto *P, rtable *tbl, net *n, rte *new, rte *old, ea_list *attrs);
void ospf_update_topology(struct ospf_proto *p);

struct top_hash_entry *ospf_hash_find(struct top_graph *, u32 domain, u32 lsa, u32 rtr, u32 type);
struct top_hash_entry *ospf_hash_get(struct top_graph *, u32 domain, u32 lsa, u32 rtr, u32 type);
void ospf_hash_delete(struct top_graph *, struct top_hash_entry *);

static inline struct top_hash_entry * ospf_hash_find_entry(struct top_graph *f, struct top_hash_entry *en)
{ return ospf_hash_find(f, en->domain, en->lsa.id, en->lsa.rt, en->lsa_type); }

static inline struct top_hash_entry * ospf_hash_get_entry(struct top_graph *f, struct top_hash_entry *en)
{ return ospf_hash_get(f, en->domain, en->lsa.id, en->lsa.rt, en->lsa_type); }

struct top_hash_entry * ospf_hash_find_rt(struct top_graph *f, u32 domain, u32 rtr);
struct top_hash_entry * ospf_hash_find_rt3_first(struct top_graph *f, u32 domain, u32 rtr);
struct top_hash_entry * ospf_hash_find_rt3_next(struct top_hash_entry *e);

struct top_hash_entry * ospf_hash_find_net2(struct top_graph *f, u32 domain, u32 id);

/* In OSPFv2, id is network IP prefix (lsa.id) while lsa.rt field is unknown
   In OSPFv3, id is lsa.rt of DR while nif is neighbor iface id (lsa.id) */
static inline struct top_hash_entry *
ospf_hash_find_net(struct top_graph *f, u32 domain, u32 id, u32 nif)
{
  return f->ospf2 ?
    ospf_hash_find_net2(f, domain, id) :
    ospf_hash_find(f, domain, nif, id, LSA_T_NET);
}


#endif /* _BIRD_OSPF_TOPOLOGY_H_ */
