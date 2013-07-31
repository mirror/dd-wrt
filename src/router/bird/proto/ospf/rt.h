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
  /*
   * For ORT_ROUTER routes, options field are router-LSA style
   * options, with V,E,B bits. In OSPFv2, ASBRs from another areas
   * (that we know from rt-summary-lsa) have just ORTA_ASBR in
   * options, their real options are unknown.
   */
#define ORTA_ASBR OPT_RT_E
#define ORTA_ABR  OPT_RT_B
  /*
   * For ORT_NET routes, the field is almost unused with one
   * exception: ORTA_PREF for external routes means that the route is
   * preferred in AS external route selection according to 16.4.1. -
   * it is intra-area path using non-backbone area. In other words,
   * the forwarding address (or ASBR if forwarding address is zero) is
   * intra-area (type == RTS_OSPF) and its area is not a backbone.
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
   * We use fn.x0 to mark persistent rt entries, that are needed for summary
   * LSAs that don't have 'proper' rt entry (area networks + default to stubs)
   * to keep uid stable (used for LSA ID in OSPFv3 - see fibnode_to_lsaid()).
   *
   * We use fn.x1 to note whether the external route was originated
   * from the route export (in ospf_rt_notify()) or from the NSSA
   * route translation (in check_nssa_lsa()).
   *
   * old_* values are here to represent the last route update. old_rta
   * is cached (we keep reference), mainly for multipath nexthops.
   * old_rta == NULL means route wasn not in the last update, in that
   * case other old_* values are not valid.
   */
  struct fib_node fn;
  orta n;
  u32 old_metric1, old_metric2, old_tag, old_rid;
  rta *old_rta;
}
ort;

static inline int rt_is_nssa(ort *nf)
{ return nf->n.options & ORTA_NSSA; }

#define EXT_EXPORT 1
#define EXT_NSSA 2

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
 * - nodes may be invalid (fn.type == 0), in that case other invariants don't hold
 * - n.metric1 may be at most a small multiple of LSINFINITY,
 *   therefore sums do not overflow
 * - n.oa is always non-NULL
 * - n.nhs is always non-NULL with one exception - configured stubnet
 *   nodes (in po->rtf).
 * - oa->rtr does not contain calculating router itself
 *
 * There are three types of nexthops in nhs fields:
 * - gateway nexthops (non-NULL iface, gw != IPA_NONE)
 * - device nexthops (non-NULL iface, gw == IPA_NONE)
 * - dummy vlink nexthops (NULL iface, gw == IPA_NONE)
 * These three types don't mix, nhs field contains either
 * one device, one vlink node, or one/more gateway nodes.
 */

void ospf_rt_spf(struct proto_ospf *po);
void ospf_rt_initort(struct fib_node *fn);


#endif /* _BIRD_OSPF_RT_H_ */
