/*
 *      BIRD -- OSPF
 *
 *      (c) 2000--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_OSPF_RT_H_
#define _BIRD_OSPF_RT_H_


#define ORT_NET 0
#define ORT_ROUTER 1

typedef struct orta
{
  u8 type;			/* RTS_OSPF_* */
  u8 nhs_reuse;			/* Whether nhs nodes can be reused during merging.
				   See a note in rt.c:add_cand() */
  u32 options;
  /*
   * For ORT_ROUTER routes, options field are router-LSA style
   * options, with V,E,B bits. In OSPFv2, ASBRs from another areas
   * (that we know from rt-summary-lsa) have just ORTA_ASBR in
   * options, their real options are unknown.
   */
#define ORTA_ASBR OPT_RT_E
#define ORTA_ABR  OPT_RT_B
  /*
   * For ORT_NET routes, there are just several flags for external routes:
   *
   * ORTA_PREF for external routes means that the route is preferred in AS
   * external route selection according to 16.4.1. - it is intra-area path using
   * non-backbone area. In other words, the forwarding address (or ASBR if
   * forwarding address is zero) is intra-area (type == RTS_OSPF) and its area
   * is not a backbone.
   *
   * ORTA_NSSA means that the entry represents an NSSA route, and ORTA_PROP
   * means that the NSSA route has propagate-bit set. These flags are used in
   * NSSA translation.
   */
#define ORTA_PREF 0x80000000
#define ORTA_NSSA 0x40000000
#define ORTA_PROP 0x20000000

  u32 metric1;
  u32 metric2;
  u32 tag;
  u32 rid;			/* Router ID of real advertising router */
  struct ospf_area *oa;
  struct ospf_area *voa;	/* Used when route is replaced in ospf_rt_sum_tr(),
				   NULL otherwise */
  struct mpnh *nhs;		/* Next hops computed during SPF */
  struct top_hash_entry *en;	/* LSA responsible for this orta */
}
orta;

typedef struct ort
{
  /*
   * Most OSPF routing table entries are for computed OSPF routes, these have
   * defined n.type. There are also few other cases: entries for configured area
   * networks (these have area_net field set) and entries for external routes
   * exported to OSPF (these have external_rte field set). These entries are
   * kept even if they do not contain 'proper' rt entry. That is needed to keep
   * allocated stable UID numbers (fn.uid), which are used as LSA IDs in OSPFv3
   * (see fibnode_to_lsaid()) for related LSAs (network summary LSAs in the
   * first case, external or NSSA LSAs in the second case). Entries for external
   * routes also have a second purpose - to prevent NSSA translation of received
   * NSSA routes if regular external routes were already originated for the same
   * network (see check_nssa_lsa()).
   *
   * old_* values are here to represent the last route update. old_rta is cached
   * (we keep reference), mainly for multipath nexthops.  old_rta == NULL means
   * route was not in the last update, in that case other old_* values are not
   * valid.
   */
  struct fib_node fn;
  orta n;
  u32 old_metric1, old_metric2, old_tag, old_rid;
  rta *old_rta;
  u8 external_rte;
  u8 area_net;
  u8 keep;
}
ort;

static inline int rt_is_nssa(ort *nf)
{ return nf->n.options & ORTA_NSSA; }


/*
 * Invariants for structs top_hash_entry (nodes of LSA db)
 * enforced by SPF calculation for final nodes (color == INSPF):
 * - only router, network and AS-external LSAs
 * - lsa.age < LSA_MAXAGE
 * - dist < LSINFINITY (or 2*LSINFINITY for ext-LSAs)
 * - nhs is non-NULL unless the node is oa->rt (calculating router itself)
 * - beware, nhs is not valid after SPF calculation
 *
 * Invariants for structs orta nodes of fib tables po->rtf, oa->rtr:
 * - nodes may be invalid (n.type == 0), in that case other invariants don't hold
 * - n.metric1 may be at most a small multiple of LSINFINITY,
 *   therefore sums do not overflow
 * - n.oa is always non-NULL
 * - n.nhs is always non-NULL unless it is configured stubnet
 * - n.en is non-NULL for external routes, NULL for intra/inter area routes.
 * - oa->rtr does not contain calculating router itself
 *
 * There are four types of nexthops in nhs fields:
 * - gateway nexthops (non-NULL iface, gw != IPA_NONE)
 * - device nexthops (non-NULL iface, gw == IPA_NONE)
 * - dummy vlink nexthops (NULL iface, gw == IPA_NONE)
 * - configured stubnets (nhs is NULL, only RTS_OSPF orta nodes in po->rtf)
 *
 * Dummy vlink nexthops and configured stubnets cannot be mixed with
 * regular ones, nhs field contains either list of gateway+device nodes,
 * one vlink node, or NULL for configured stubnet.
 *
 * Dummy vlink nexthops can appear in both network (rtf) and backbone area router
 * (rtr) tables for regular and inter-area routes, but only if areano > 1. They are
 * replaced in ospf_rt_sum_tr() and removed in ospf_rt_abr1(), therefore cannot
 * appear in ASBR pre-selection and external routes processing.
 */

void ospf_rt_spf(struct ospf_proto *p);
void ospf_rt_initort(struct fib_node *fn);


#endif /* _BIRD_OSPF_RT_H_ */
