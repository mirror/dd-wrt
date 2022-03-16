/*
 *	BIRD -- OSPF Topological Database
 *
 *	(c) 1999       Martin Mares <mj@ucw.cz>
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "lib/string.h"

#include "ospf.h"


#define HASH_DEF_ORDER 6
#define HASH_HI_MARK *4
#define HASH_HI_STEP 2
#define HASH_HI_MAX 16
#define HASH_LO_MARK /5
#define HASH_LO_STEP 2
#define HASH_LO_MIN 8

static inline void * lsab_flush(struct ospf_proto *p);
static inline void lsab_reset(struct ospf_proto *p);


/**
 * ospf_install_lsa - install new LSA into database
 * @p: OSPF protocol instance
 * @lsa: LSA header
 * @type: type of LSA
 * @domain: domain of LSA
 * @body: pointer to LSA body
 *
 * This function ensures installing new LSA received in LS update into LSA
 * database. Old instance is replaced. Several actions are taken to detect if
 * new routing table calculation is necessary. This is described in 13.2 of RFC
 * 2328. This function is for received LSA only, locally originated LSAs are
 * installed by ospf_originate_lsa().
 *
 * The LSA body in @body is expected to be mb_allocated by the caller and its
 * ownership is transferred to the LSA entry structure.
 */
struct top_hash_entry *
ospf_install_lsa(struct ospf_proto *p, struct ospf_lsa_header *lsa, u32 type, u32 domain, void *body)
{
  struct top_hash_entry *en;
  int change = 0;

  en = ospf_hash_get(p->gr, domain, lsa->id, lsa->rt, type);

  if (!SNODE_VALID(en))
    s_add_tail(&p->lsal, SNODE en);

  if ((en->lsa_body == NULL) ||			/* No old LSA */
      (en->lsa.length != lsa->length) ||
      (en->lsa.type_raw != lsa->type_raw) ||	/* Check for OSPFv2 options */
      (en->lsa.age == LSA_MAXAGE) ||
      (lsa->age == LSA_MAXAGE) ||
      memcmp(en->lsa_body, body, lsa->length - sizeof(struct ospf_lsa_header)))
    change = 1;

  if ((en->lsa.age == LSA_MAXAGE) && (lsa->age == LSA_MAXAGE))
    change = 0;

  mb_free(en->lsa_body);
  en->lsa_body = body;
  en->lsa = *lsa;
  en->init_age = en->lsa.age;
  en->inst_time = now;

  /*
   * We do not set en->mode. It is either default LSA_M_BASIC, or in a special
   * case when en is local but flushed, there is postponed LSA, self-originated
   * LSA is received and ospf_install_lsa() is called from ospf_advance_lse(),
   * then we have en->mode from the postponed LSA origination.
   */

  OSPF_TRACE(D_EVENTS, "Installing LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x, Age: %u",
	     en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn, en->lsa.age);

  if (change)
    ospf_schedule_rtcalc(p);

  return en;
}

/**
 * ospf_advance_lsa - handle received unexpected self-originated LSA
 * @p: OSPF protocol instance
 * @en: current LSA entry or NULL
 * @lsa: new LSA header
 * @type: type of LSA
 * @domain: domain of LSA
 * @body: pointer to LSA body
 *
 * This function handles received unexpected self-originated LSA (@lsa, @body)
 * by either advancing sequence number of the local LSA instance (@en) and
 * propagating it, or installing the received LSA and immediately flushing it
 * (if there is no local LSA; i.e., @en is NULL or MaxAge).
 *
 * The LSA body in @body is expected to be mb_allocated by the caller and its
 * ownership is transferred to the LSA entry structure or it is freed.
 */
void
ospf_advance_lsa(struct ospf_proto *p, struct top_hash_entry *en, struct ospf_lsa_header *lsa, u32 type, u32 domain, void *body)
{
  /* RFC 2328 13.4 */

  if (en && (en->lsa.age < LSA_MAXAGE))
  {
    if (lsa->sn != LSA_MAXSEQNO)
    {
      /*
       * We simply advance current LSA to have higher seqnum than received LSA.
       * The received LSA is ignored and the advanced LSA is propagated instead.
       *
       * Although this is an origination of distinct LSA instance and therefore
       * should be limited by MinLSInterval, we do not enforce it here. Fast
       * reaction is needed and we are already limited by MinLSArrival.
       */

      mb_free(body);

      en->lsa.sn = lsa->sn + 1;
      en->lsa.age = 0;
      en->init_age = 0;
      en->inst_time = now;
      lsa_generate_checksum(&en->lsa, en->lsa_body);

      OSPF_TRACE(D_EVENTS, "Advancing LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x",
		 en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);
    }
    else
    {
      /*
       * Received LSA has maximal sequence number, so we cannot simply override
       * it. We have to install it to the database, immediately flush it to
       * implement sequence number wrapping, and schedule our current LSA to be
       * originated after the received instance is flushed.
       */

      if (en->next_lsa_body == NULL)
      {
	/* Schedule current LSA */
	en->next_lsa_blen = en->lsa.length - sizeof(struct ospf_lsa_header);
	en->next_lsa_body = en->lsa_body;
	en->next_lsa_opts = ospf_is_v2(p) ? lsa_get_options(&en->lsa) : 0;
      }
      else
      {
	/* There is already scheduled LSA, so we just free current one */
	mb_free(en->lsa_body);
      }

      en->lsa_body = body;
      en->lsa = *lsa;
      en->lsa.age = LSA_MAXAGE;
      en->init_age = lsa->age;
      en->inst_time = now;

      OSPF_TRACE(D_EVENTS, "Resetting LSA:  Type: %04x, Id: %R, Rt: %R, Seq: %08x",
		 en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);
      OSPF_TRACE(D_EVENTS, "Postponing LSA: Type: %04x, Id: %R, Rt: %R",
		 en->lsa_type, en->lsa.id, en->lsa.rt);
    }
  }
  else
  {
    /*
     * We do not have received LSA in the database. We have to flush the
     * received LSA. It has to be installed in the database to secure
     * retransmissions. Note that the received LSA may already be MaxAge.
     * Also note that en->next_lsa_* may be defined.
     */

    lsa->age = LSA_MAXAGE;
    en = ospf_install_lsa(p, lsa, type, domain, body);
  }

  /*
   * We flood the updated LSA. Although in some cases the to-be-flooded LSA is
   * the same as the received LSA, and therefore we should propagate it as
   * regular received LSA (send the acknowledgement instead of the update to
   * the neighbor we received it from), we cheat a bit here.
   */

  ospf_flood_lsa(p, en, NULL);
}


static int
ospf_do_originate_lsa(struct ospf_proto *p, struct top_hash_entry *en, void *lsa_body, u16 lsa_blen, u16 lsa_opts)
{
  /* Enforce MinLSInterval */
  if ((en->init_age == 0) && en->inst_time && ((en->inst_time + MINLSINTERVAL) > now))
    return 0;

  /* Handle wrapping sequence number */
  if (en->lsa.sn == LSA_MAXSEQNO)
  {
    /* Prepare to flush old LSA */
    if (en->lsa.age != LSA_MAXAGE)
    {
      OSPF_TRACE(D_EVENTS, "Resetting LSA:  Type: %04x, Id: %R, Rt: %R, Seq: %08x",
		 en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);

      en->lsa.age = LSA_MAXAGE;
      ospf_flood_lsa(p, en, NULL);
      return 0;
    }

    /* Already flushing */
    if ((p->padj != 0) || (en->ret_count != 0))
      return 0;

    /* Flush done, just clean up seqnum, lsa_body is freed below */
    en->lsa.sn = LSA_ZEROSEQNO;
  }

  /*
   * lsa.type_raw is initialized by ospf_hash_get() to OSPFv3 LSA type.
   * lsa_set_options() implicitly converts it to OSPFv2 LSA type, assuming that
   * old type is just new type masked by 0xff.  That is not universally true,
   * but it holds for all OSPFv2 types currently supported by BIRD.
   */

  if (ospf_is_v2(p))
    lsa_set_options(&en->lsa, lsa_opts);

  mb_free(en->lsa_body);
  en->lsa_body = lsa_body;
  en->lsa.length = sizeof(struct ospf_lsa_header) + lsa_blen;
  en->lsa.sn++;
  en->lsa.age = 0;
  en->init_age = 0;
  en->inst_time = now;
  lsa_generate_checksum(&en->lsa, en->lsa_body);

  OSPF_TRACE(D_EVENTS, "Originating LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x",
	     en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);

  ospf_flood_lsa(p, en, NULL);

  if (en->mode == LSA_M_BASIC)
    ospf_schedule_rtcalc(p);

  return 1;
}

/**
 * ospf_originate_lsa - originate new LSA
 * @p: OSPF protocol instance
 * @lsa: New LSA specification
 *
 * This function prepares a new LSA, installs it into the LSA database and
 * floods it. If the new LSA cannot be originated now (because the old instance
 * was originated within MinLSInterval, or because the LSA seqnum is currently
 * wrapping), the origination is instead scheduled for later. If the new LSA is
 * equivalent to the current LSA, the origination is skipped. In all cases, the
 * corresponding LSA entry is returned. The new LSA is based on the LSA
 * specification (@lsa) and the LSA body from lsab buffer of @p, which is
 * emptied after the call. The opposite of this function is ospf_flush_lsa().
 */
struct top_hash_entry *
ospf_originate_lsa(struct ospf_proto *p, struct ospf_new_lsa *lsa)
{
  struct top_hash_entry *en;
  void *lsa_body = p->lsab;
  u16 lsa_blen = p->lsab_used;
  u16 lsa_length = sizeof(struct ospf_lsa_header) + lsa_blen;

  en = ospf_hash_get(p->gr, lsa->dom, lsa->id, p->router_id, lsa->type);

  if (!SNODE_VALID(en))
    s_add_tail(&p->lsal, SNODE en);

  if (!en->nf || !en->lsa_body)
    en->nf = lsa->nf;

  if (en->nf != lsa->nf)
  {
    log(L_ERR "%s: LSA ID collision for %I/%d",
	p->p.name, lsa->nf->fn.prefix, lsa->nf->fn.pxlen);

    en = NULL;
    goto drop;
  }

  if (en->mode != lsa->mode)
    en->mode = lsa->mode;

  if (en->next_lsa_body)
  {
    /* Ignore the new LSA if it is the same as the scheduled one */
    if ((lsa_blen == en->next_lsa_blen) &&
	!memcmp(lsa_body, en->next_lsa_body, lsa_blen) &&
	(!ospf_is_v2(p) || (lsa->opts == en->next_lsa_opts)))
      goto drop;

    /* Free scheduled LSA */
    mb_free(en->next_lsa_body);
    en->next_lsa_body = NULL;
    en->next_lsa_blen = 0;
    en->next_lsa_opts = 0;
  }

  /* Ignore the the new LSA if is the same as the current one */
  if ((en->lsa.age < LSA_MAXAGE) &&
      (lsa_length == en->lsa.length) &&
      !memcmp(lsa_body, en->lsa_body, lsa_blen) &&
      (!ospf_is_v2(p) || (lsa->opts == lsa_get_options(&en->lsa))))
    goto drop;

  lsa_body = lsab_flush(p);

  if (! ospf_do_originate_lsa(p, en, lsa_body, lsa_blen, lsa->opts))
  {
    OSPF_TRACE(D_EVENTS, "Postponing LSA: Type: %04x, Id: %R, Rt: %R",
	       en->lsa_type, en->lsa.id, en->lsa.rt);

    en->next_lsa_body = lsa_body;
    en->next_lsa_blen = lsa_blen;
    en->next_lsa_opts = lsa->opts;
  }

  return en;

 drop:
  lsab_reset(p);
  return en;
}

static void
ospf_originate_next_lsa(struct ospf_proto *p, struct top_hash_entry *en)
{
  /* Called by ospf_update_lsadb() to handle scheduled origination */

  if (! ospf_do_originate_lsa(p, en, en->next_lsa_body, en->next_lsa_blen, en->next_lsa_opts))
    return;

  en->next_lsa_body = NULL;
  en->next_lsa_blen = 0;
  en->next_lsa_opts = 0;
}

static void
ospf_refresh_lsa(struct ospf_proto *p, struct top_hash_entry *en)
{
  /*
   * Called by ospf_update_lsadb() for periodic LSA refresh.
   *
   * We know that lsa.age < LSA_MAXAGE and lsa.rt is our router ID. We can also
   * assume that there is no scheduled LSA, because inst_time is deep in past,
   * therefore ospf_originate_next_lsa() called before would either succeed or
   * switched lsa.age to LSA_MAXAGE.
   */

  OSPF_TRACE(D_EVENTS, "Refreshing LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x",
	     en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);

  ASSERT(en->next_lsa_body == NULL);

  /* Handle wrapping sequence number */
  if (en->lsa.sn == LSA_MAXSEQNO)
  {
    /* Copy LSA body as next LSA to get automatic origination after flush is finished */
    en->next_lsa_blen = en->lsa.length - sizeof(struct ospf_lsa_header);
    en->next_lsa_body = mb_alloc(p->p.pool, en->next_lsa_blen);
    memcpy(en->next_lsa_body, en->lsa_body, en->next_lsa_blen);
    en->next_lsa_opts = ospf_is_v2(p) ? lsa_get_options(&en->lsa) : 0;

    en->lsa.age = LSA_MAXAGE;
    ospf_flood_lsa(p, en, NULL);
    return;
  }

  en->lsa.sn++;
  en->lsa.age = 0;
  en->init_age = 0;
  en->inst_time = now;
  lsa_generate_checksum(&en->lsa, en->lsa_body);
  ospf_flood_lsa(p, en, NULL);
}

/**
 * ospf_flush_lsa - flush LSA from OSPF domain
 * @p: OSPF protocol instance
 * @en: LSA entry to flush
 *
 * This function flushes @en from the OSPF domain by setting its age to
 * %LSA_MAXAGE and flooding it. That also triggers subsequent events in LSA
 * lifecycle leading to removal of the LSA from the LSA database (e.g. the LSA
 * content is freed when flushing is acknowledged by neighbors). The function
 * does nothing if the LSA is already being flushed. LSA entries are not
 * immediately removed when being flushed, the caller may assume that @en still
 * exists after the call. The function is the opposite of ospf_originate_lsa()
 * and is supposed to do the right thing even in cases of postponed
 * origination.
 */
void
ospf_flush_lsa(struct ospf_proto *p, struct top_hash_entry *en)
{
  en->nf = NULL;

  if (en->next_lsa_body)
  {
    mb_free(en->next_lsa_body);
    en->next_lsa_body = NULL;
    en->next_lsa_blen = 0;
    en->next_lsa_opts = 0;
  }

  if (en->lsa.age == LSA_MAXAGE)
    return;

  OSPF_TRACE(D_EVENTS, "Flushing LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x",
	     en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);

  en->lsa.age = LSA_MAXAGE;
  ospf_flood_lsa(p, en, NULL);

  if (en->mode == LSA_M_BASIC)
    ospf_schedule_rtcalc(p);

  en->mode = LSA_M_BASIC;
}

static void
ospf_clear_lsa(struct ospf_proto *p, struct top_hash_entry *en)
{
  /*
   * Called by ospf_update_lsadb() as part of LSA flushing process.
   * Flushed LSA was acknowledged by neighbors and we can free its content.
   * The log message is for 'remove' - we hide empty LSAs from users.
   */

  OSPF_TRACE(D_EVENTS, "Removing LSA: Type: %04x, Id: %R, Rt: %R, Seq: %08x",
	     en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn);

  if (en->lsa.sn == LSA_MAXSEQNO)
    en->lsa.sn = LSA_ZEROSEQNO;

  mb_free(en->lsa_body);
  en->lsa_body = NULL;
}

static void
ospf_remove_lsa(struct ospf_proto *p, struct top_hash_entry *en)
{
  /*
   * Called by ospf_update_lsadb() as part of LSA flushing process.
   * Both lsa_body and next_lsa_body are NULL.
   */

  s_rem_node(SNODE en);
  ospf_hash_delete(p->gr, en);
}

/**
 * ospf_update_lsadb - update LSA database
 * @p: OSPF protocol instance
 *
 * This function is periodicaly invoked from ospf_disp(). It does some periodic
 * or postponed processing related to LSA entries. It originates postponed LSAs
 * scheduled by ospf_originate_lsa(), It continues in flushing processes started
 * by ospf_flush_lsa(). It also periodically refreshs locally originated LSAs --
 * when the current instance is older %LSREFRESHTIME, a new instance is originated.
 * Finally, it also ages stored LSAs and flushes ones that reached %LSA_MAXAGE.
 *
 * The RFC 2328 says that a router should periodically check checksums of all
 * stored LSAs to detect hardware problems. This is not implemented.
 */
void
ospf_update_lsadb(struct ospf_proto *p)
{
  struct top_hash_entry *en, *nxt;
  bird_clock_t real_age;

  WALK_SLIST_DELSAFE(en, nxt, p->lsal)
  {
    if (en->next_lsa_body)
      ospf_originate_next_lsa(p, en);

    real_age = en->init_age + (now - en->inst_time);

    if (en->lsa.age == LSA_MAXAGE)
    {
      if (en->lsa_body && (p->padj == 0) && (en->ret_count == 0))
	ospf_clear_lsa(p, en);

      if ((en->lsa_body == NULL) && (en->next_lsa_body == NULL) &&
	  ((en->lsa.rt != p->router_id) || (real_age >= LSA_MAXAGE)))
	ospf_remove_lsa(p, en);

      continue;
    }

    if ((en->lsa.rt == p->router_id) && (real_age >= LSREFRESHTIME))
    {
      ospf_refresh_lsa(p, en);
      continue;
    }

    if (real_age >= LSA_MAXAGE)
    {
      ospf_flush_lsa(p, en);
      continue;
    }

    en->lsa.age = real_age;
  }
}


static inline u32
ort_to_lsaid(struct ospf_proto *p UNUSED4 UNUSED6, ort *nf)
{
  /*
   * In OSPFv2, We have to map IP prefixes to u32 in such manner that resulting
   * u32 interpreted as IP address is a member of given prefix. Therefore, /32
   * prefix have to be mapped on itself.  All received prefixes have to be
   * mapped on different u32s.
   *
   * We have an assumption that if there is nontrivial (non-/32) network prefix,
   * then there is not /32 prefix for the first and the last IP address of the
   * network (these are usually reserved, therefore it is not an important
   * restriction).  The network prefix is mapped to the first or the last IP
   * address in the manner that disallow collisions - we use the IP address that
   * cannot be used by the parent prefix.
   *
   * For example:
   * 192.168.0.0/24 maps to 192.168.0.255
   * 192.168.1.0/24 maps to 192.168.1.0
   * because 192.168.0.0 and 192.168.1.255 might be used by 192.168.0.0/23 .
   *
   * Appendig E of RFC 2328 suggests different algorithm, that tries to maximize
   * both compatibility and subnetting. But as it is not possible to have both
   * reliably and the suggested algorithm was unnecessary complicated and it
   * does crazy things like changing LSA ID for a network because different
   * network appeared, we choose a different way.
   *
   * In OSPFv3, it is simpler. There is not a requirement for membership of the
   * result in the input network, so we just use a hash-based unique ID of a
   * routing table entry for a route that originated given LSA. For ext-LSA, it
   * is an imported route in the nest's routing table (p->table). For summary-LSA,
   * it is a 'source' route in the protocol internal routing table (p->rtf).
   */

  if (ospf_is_v3(p))
    return nf->fn.uid;

  u32 id = ipa_to_u32(nf->fn.prefix);
  int pxlen = nf->fn.pxlen;

  if ((pxlen == 0) || (pxlen == 32))
    return id;

  if (id & (1 << (32 - pxlen)))
    return id;
  else
    return id | ~u32_mkmask(pxlen);
}


static void *
lsab_alloc(struct ospf_proto *p, uint size)
{
  uint offset = p->lsab_used;
  p->lsab_used += size;
  if (p->lsab_used > p->lsab_size)
  {
    p->lsab_size = MAX(p->lsab_used, 2 * p->lsab_size);
    p->lsab = p->lsab ? mb_realloc(p->lsab, p->lsab_size):
      mb_alloc(p->p.pool, p->lsab_size);
  }
  return ((byte *) p->lsab) + offset;
}

static inline void *
lsab_allocz(struct ospf_proto *p, uint size)
{
  void *r = lsab_alloc(p, size);
  bzero(r, size);
  return r;
}

static inline void *
lsab_flush(struct ospf_proto *p)
{
  void *r = mb_alloc(p->p.pool, p->lsab_used);
  memcpy(r, p->lsab, p->lsab_used);
  p->lsab_used = 0;
  return r;
}

static inline void
lsab_reset(struct ospf_proto *p)
{
  p->lsab_used = 0;
}

static inline void *
lsab_offset(struct ospf_proto *p, uint offset)
{
  return ((byte *) p->lsab) + offset;
}

static inline void * UNUSED
lsab_end(struct ospf_proto *p)
{
  return ((byte *) p->lsab) + p->lsab_used;
}


/*
 *	Router-LSA handling
 *	Type = LSA_T_RT
 */

static int
configured_stubnet(struct ospf_area *oa, struct ifa *a)
{
  /* Does not work for IA_PEER addresses, but it is not called on these */
  struct ospf_stubnet_config *sn;
  WALK_LIST(sn, oa->ac->stubnet_list)
  {
    if (sn->summary)
    {
      if (ipa_in_net(a->prefix, sn->px.addr, sn->px.len) && (a->pxlen >= sn->px.len))
	return 1;
    }
    else
    {
      if (ipa_equal(a->prefix, sn->px.addr) && (a->pxlen == sn->px.len))
	return 1;
    }
  }

  return 0;
}

static int
bcast_net_active(struct ospf_iface *ifa)
{
  struct ospf_neighbor *neigh;

  if (ifa->state == OSPF_IS_WAITING)
    return 0;

  WALK_LIST(neigh, ifa->neigh_list)
  {
    if (neigh->state == NEIGHBOR_FULL)
    {
      if (neigh->rid == ifa->drid)
	return 1;

      if (ifa->state == OSPF_IS_DR)
	return 1;
    }
  }

  return 0;
}

static inline u32
get_rt_options(struct ospf_proto *p, struct ospf_area *oa, int bitv)
{
  u32 opts = 0;

  if (p->areano > 1)
    opts |= OPT_RT_B;

  if ((p->areano > 1) && oa_is_nssa(oa) && oa->ac->translator)
    opts |= OPT_RT_NT;

  if (p->asbr && !oa_is_stub(oa))
    opts |= OPT_RT_E;

  if (bitv)
    opts |= OPT_RT_V;

  return opts;
}

static inline void
add_rt2_lsa_link(struct ospf_proto *p, u8 type, u32 id, u32 data, u16 metric)
{
  struct ospf_lsa_rt2_link *ln = lsab_alloc(p, sizeof(struct ospf_lsa_rt2_link));
  ln->type = type;
  ln->id = id;
  ln->data = data;
  ln->metric = metric;
  ln->no_tos = 0;
}

static void
prepare_rt2_lsa_body(struct ospf_proto *p, struct ospf_area *oa)
{
  struct ospf_iface *ifa;
  int i = 0, bitv = 0;
  struct ospf_neighbor *neigh;

  ASSERT(p->lsab_used == 0);
  lsab_allocz(p, sizeof(struct ospf_lsa_rt));
  /* ospf_lsa_rt header will be filled later */

  WALK_LIST(ifa, p->iface_list)
  {
    int net_lsa = 0;
    u32 link_cost = p->stub_router ? 0xffff : ifa->cost;

    if ((ifa->type == OSPF_IT_VLINK) && (ifa->voa == oa) &&
	(!EMPTY_LIST(ifa->neigh_list)))
    {
      neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
      if ((neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	bitv = 1;
    }

    if ((ifa->oa != oa) || (ifa->state == OSPF_IS_DOWN))
      continue;

    ifa->rt_pos_beg = i;

    /* RFC 2328 - 12.4.1.1-4 */
    switch (ifa->type)
    {
    case OSPF_IT_PTP:
    case OSPF_IT_PTMP:
      WALK_LIST(neigh, ifa->neigh_list)
	if (neigh->state == NEIGHBOR_FULL)
	{
	  /*
	   * ln->data should be ifa->iface_id in case of no/ptp
	   * address (ifa->addr->flags & IA_PEER) on PTP link (see
	   * RFC 2328 12.4.1.1.), but the iface ID value has no use,
	   * while using IP address even in this case is here for
	   * compatibility with some broken implementations that use
	   * this address as a next-hop.
	   */
	  add_rt2_lsa_link(p, LSART_PTP, neigh->rid, ipa_to_u32(ifa->addr->ip), link_cost);
	  i++;
	}
      break;

    case OSPF_IT_BCAST:
    case OSPF_IT_NBMA:
      if (bcast_net_active(ifa))
      {
	add_rt2_lsa_link(p, LSART_NET, ipa_to_u32(ifa->drip), ipa_to_u32(ifa->addr->ip), link_cost);
	i++;
	net_lsa = 1;
      }
      break;

    case OSPF_IT_VLINK:
      neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
      if ((!EMPTY_LIST(ifa->neigh_list)) && (neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	add_rt2_lsa_link(p, LSART_VLNK, neigh->rid, ipa_to_u32(ifa->addr->ip), link_cost), i++;
      break;

    default:
      log(L_BUG "OSPF: Unknown interface type");
      break;
    }

    ifa->rt_pos_end = i;

    /* Now we will originate stub area if there is no primary */
    if (net_lsa ||
	(ifa->type == OSPF_IT_VLINK) ||
	((ifa->addr->flags & IA_PEER) && ! ifa->cf->stub) ||
	configured_stubnet(oa, ifa->addr))
      continue;

      /* Host or network stub entry */
    if ((ifa->addr->flags & IA_HOST) ||
	(ifa->state == OSPF_IS_LOOP) ||
	(ifa->type == OSPF_IT_PTMP))
      add_rt2_lsa_link(p, LSART_STUB, ipa_to_u32(ifa->addr->ip), 0xffffffff, 0);
    else
      add_rt2_lsa_link(p, LSART_STUB, ipa_to_u32(ifa->addr->prefix), u32_mkmask(ifa->addr->pxlen), ifa->cost);
    i++;

    ifa->rt_pos_end = i;
  }

  struct ospf_stubnet_config *sn;
  WALK_LIST(sn, oa->ac->stubnet_list)
    if (!sn->hidden)
      add_rt2_lsa_link(p, LSART_STUB, ipa_to_u32(sn->px.addr), u32_mkmask(sn->px.len), sn->cost), i++;

  struct ospf_lsa_rt *rt = p->lsab;
  /* Store number of links in lower half of options */
  rt->options = get_rt_options(p, oa, bitv) | (u16) i;
}

static inline void
add_rt3_lsa_link(struct ospf_proto *p, u8 type, struct ospf_iface *ifa, u32 nif, u32 id)
{
  struct ospf_lsa_rt3_link *ln = lsab_alloc(p, sizeof(struct ospf_lsa_rt3_link));
  ln->type = type;
  ln->padding = 0;
  ln->metric = ifa->cost;
  ln->lif = ifa->iface_id;
  ln->nif = nif;
  ln->id = id;
}

static void
prepare_rt3_lsa_body(struct ospf_proto *p, struct ospf_area *oa)
{
  struct ospf_iface *ifa;
  struct ospf_neighbor *neigh;
  int bitv = 0;
  int i = 0;

  ASSERT(p->lsab_used == 0);
  lsab_allocz(p, sizeof(struct ospf_lsa_rt));
  /* ospf_lsa_rt header will be filled later */

  WALK_LIST(ifa, p->iface_list)
  {
    if ((ifa->type == OSPF_IT_VLINK) && (ifa->voa == oa) &&
	(!EMPTY_LIST(ifa->neigh_list)))
    {
      neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
      if ((neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	bitv = 1;
    }

    if ((ifa->oa != oa) || (ifa->state == OSPF_IS_DOWN))
      continue;

    ifa->rt_pos_beg = i;

    /* RFC 5340 - 4.4.3.2 */
    switch (ifa->type)
    {
    case OSPF_IT_PTP:
    case OSPF_IT_PTMP:
      WALK_LIST(neigh, ifa->neigh_list)
	if (neigh->state == NEIGHBOR_FULL)
	  add_rt3_lsa_link(p, LSART_PTP, ifa, neigh->iface_id, neigh->rid), i++;
      break;

    case OSPF_IT_BCAST:
    case OSPF_IT_NBMA:
      if (bcast_net_active(ifa))
	add_rt3_lsa_link(p, LSART_NET, ifa, ifa->dr_iface_id, ifa->drid), i++;
      break;

    case OSPF_IT_VLINK:
      neigh = (struct ospf_neighbor *) HEAD(ifa->neigh_list);
      if ((!EMPTY_LIST(ifa->neigh_list)) && (neigh->state == NEIGHBOR_FULL) && (ifa->cost <= 0xffff))
	add_rt3_lsa_link(p, LSART_VLNK, ifa, neigh->iface_id, neigh->rid), i++;
      break;

    default:
      log(L_BUG "OSPF: Unknown interface type");
      break;
    }

    ifa->rt_pos_end = i;
  }

  struct ospf_lsa_rt *rt = p->lsab;
  rt->options = get_rt_options(p, oa, bitv) | (oa->options & LSA_OPTIONS_MASK);
}

static void
ospf_originate_rt_lsa(struct ospf_proto *p, struct ospf_area *oa)
{
  struct ospf_new_lsa lsa = {
    .type = LSA_T_RT,
    .dom  = oa->areaid,
    .id   = ospf_is_v2(p) ? p->router_id : 0,
    .opts = oa->options
  };

  OSPF_TRACE(D_EVENTS, "Updating router state for area %R", oa->areaid);

  if (ospf_is_v2(p))
    prepare_rt2_lsa_body(p, oa);
  else
    prepare_rt3_lsa_body(p, oa);

  oa->rt = ospf_originate_lsa(p, &lsa);
}


/*
 *	Net-LSA handling
 *	Type = LSA_T_NET
 */

static void
prepare_net2_lsa_body(struct ospf_proto *p, struct ospf_iface *ifa)
{
  struct ospf_lsa_net *net;
  struct ospf_neighbor *n;
  int nodes = ifa->fadj + 1;
  u16 i = 1;

  ASSERT(p->lsab_used == 0);
  net = lsab_alloc(p, sizeof(struct ospf_lsa_net) + 4 * nodes);

  net->optx = u32_mkmask(ifa->addr->pxlen);
  net->routers[0] = p->router_id;

  WALK_LIST(n, ifa->neigh_list)
  {
    if (n->state == NEIGHBOR_FULL)
    {
      net->routers[i] = n->rid;
      i++;
    }
  }
  ASSERT(i == nodes);
}

static void
prepare_net3_lsa_body(struct ospf_proto *p, struct ospf_iface *ifa)
{
  struct ospf_lsa_net *net;
  int nodes = ifa->fadj + 1;
  u32 options = 0;
  u16 i = 1;

  ASSERT(p->lsab_used == 0);
  net = lsab_alloc(p, sizeof(struct ospf_lsa_net) + 4 * nodes);

  net->routers[0] = p->router_id;

  struct ospf_neighbor *n;
  WALK_LIST(n, ifa->neigh_list)
  {
    if (n->state == NEIGHBOR_FULL)
    {
      /* In OSPFv3, we would like to merge options from Link LSAs of added neighbors */

      struct top_hash_entry *en =
	ospf_hash_find(p->gr, ifa->iface_id, n->iface_id, n->rid, LSA_T_LINK);

      if (en)
	options |= ((struct ospf_lsa_link *) en->lsa_body)->options;

      net->routers[i] = n->rid;
      i++;
    }
  }
  ASSERT(i == nodes);

  net->optx = options & LSA_OPTIONS_MASK;
}

static void
ospf_originate_net_lsa(struct ospf_proto *p, struct ospf_iface *ifa)
{
  struct ospf_new_lsa lsa = {
    .type = LSA_T_NET,
    .dom  = ifa->oa->areaid,
    .id   = ospf_is_v2(p) ? ipa_to_u32(ifa->addr->ip) : ifa->iface_id,
    .opts = ifa->oa->options,
    .ifa  = ifa
  };

  OSPF_TRACE(D_EVENTS, "Updating network state for %s (Id: %R)", ifa->ifname, lsa.id);

  if (ospf_is_v2(p))
    prepare_net2_lsa_body(p, ifa);
  else
    prepare_net3_lsa_body(p, ifa);

  ifa->net_lsa = ospf_originate_lsa(p, &lsa);
}


/*
 *	(Net|Rt)-summary-LSA handling
 *	(a.k.a. Inter-Area-(Prefix|Router)-LSA)
 *	Type = LSA_T_SUM_NET, LSA_T_SUM_RT
 */

static inline void
prepare_sum2_lsa_body(struct ospf_proto *p, uint pxlen, u32 metric)
{
  struct ospf_lsa_sum2 *sum;

  sum = lsab_allocz(p, sizeof(struct ospf_lsa_sum2));
  sum->netmask = u32_mkmask(pxlen);
  sum->metric = metric;
}

static inline void
prepare_sum3_net_lsa_body(struct ospf_proto *p, ort *nf, u32 metric)
{
  struct ospf_lsa_sum3_net *sum;

  sum = lsab_allocz(p, sizeof(struct ospf_lsa_sum3_net) + IPV6_PREFIX_SPACE(nf->fn.pxlen));
  sum->metric = metric;
  put_ipv6_prefix(sum->prefix, nf->fn.prefix, nf->fn.pxlen, 0, 0);
}

static inline void
prepare_sum3_rt_lsa_body(struct ospf_proto *p, u32 drid, u32 metric, u32 options)
{
  struct ospf_lsa_sum3_rt *sum;

  sum = lsab_allocz(p, sizeof(struct ospf_lsa_sum3_rt));
  sum->options = options;
  sum->metric = metric;
  sum->drid = drid;
}

void
ospf_originate_sum_net_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, int metric)
{
  struct ospf_new_lsa lsa = {
    .type = LSA_T_SUM_NET,
    .mode = LSA_M_RTCALC,
    .dom  = oa->areaid,
    .id   = ort_to_lsaid(p, nf),
    .opts = oa->options,
    .nf   = nf
  };

  if (ospf_is_v2(p))
    prepare_sum2_lsa_body(p, nf->fn.pxlen, metric);
  else
    prepare_sum3_net_lsa_body(p, nf, metric);

  ospf_originate_lsa(p, &lsa);
}

void
ospf_originate_sum_rt_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, int metric, u32 options)
{
  struct ospf_new_lsa lsa = {
    .type = LSA_T_SUM_RT,
    .mode = LSA_M_RTCALC,
    .dom  = oa->areaid,
    .id   = ipa_to_rid(nf->fn.prefix),	/* Router ID of ASBR, irrelevant for OSPFv3 */
    .opts = oa->options
  };

  if (ospf_is_v2(p))
    prepare_sum2_lsa_body(p, 0, metric);
  else
    prepare_sum3_rt_lsa_body(p, lsa.id, metric, options & LSA_OPTIONS_MASK);

  ospf_originate_lsa(p, &lsa);
}


/*
 *	AS-external-LSA and NSSA-LSA handling
 *	Type = LSA_T_EXT, LSA_T_NSSA
 */

static inline void
prepare_ext2_lsa_body(struct ospf_proto *p, uint pxlen,
		      u32 metric, u32 ebit, ip_addr fwaddr, u32 tag)
{
  struct ospf_lsa_ext2 *ext;

  ext = lsab_allocz(p, sizeof(struct ospf_lsa_ext2));
  ext->metric = metric & LSA_METRIC_MASK;
  ext->netmask = u32_mkmask(pxlen);
  ext->fwaddr = ipa_to_u32(fwaddr);
  ext->tag = tag;

  if (ebit)
    ext->metric |= LSA_EXT2_EBIT;
}

static inline void
prepare_ext3_lsa_body(struct ospf_proto *p, ort *nf,
		      u32 metric, u32 ebit, ip_addr fwaddr, u32 tag, int pbit)
{
  struct ospf_lsa_ext3 *ext;
  int bsize = sizeof(struct ospf_lsa_ext3)
    + IPV6_PREFIX_SPACE(nf->fn.pxlen)
    + (ipa_nonzero(fwaddr) ? 16 : 0)
    + (tag ? 4 : 0);

  ext = lsab_allocz(p, bsize);
  ext->metric = metric & LSA_METRIC_MASK;
  u32 *buf = ext->rest;

  buf = put_ipv6_prefix(buf, nf->fn.prefix, nf->fn.pxlen, pbit ? OPT_PX_P : 0, 0);

  if (ebit)
    ext->metric |= LSA_EXT3_EBIT;

  if (ipa_nonzero(fwaddr))
  {
    ext->metric |= LSA_EXT3_FBIT;
    buf = put_ipv6_addr(buf, fwaddr);
  }

  if (tag)
  {
    ext->metric |= LSA_EXT3_TBIT;
    *buf++ = tag;
  }
}

/**
 * originate_ext_lsa - new route received from nest and filters
 * @p: OSPF protocol instance
 * @oa: ospf_area for which LSA is originated
 * @nf: network prefix and mask
 * @mode: the mode of the LSA (LSA_M_EXPORT or LSA_M_RTCALC)
 * @metric: the metric of a route
 * @ebit: E-bit for route metric (bool)
 * @fwaddr: the forwarding address
 * @tag: the route tag
 * @pbit: P-bit for NSSA LSAs (bool), ignored for external LSAs
 *
 * If I receive a message that new route is installed, I try to originate an
 * external LSA. If @oa is an NSSA area, NSSA-LSA is originated instead.
 * @oa should not be a stub area. @src does not specify whether the LSA
 * is external or NSSA, but it specifies the source of origination -
 * the export from ospf_rt_notify(), or the NSSA-EXT translation.
 */
void
ospf_originate_ext_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf, u8 mode,
		       u32 metric, u32 ebit, ip_addr fwaddr, u32 tag, int pbit)
{
  struct ospf_new_lsa lsa = {
    .type = oa ? LSA_T_NSSA : LSA_T_EXT,
    .mode = mode, /* LSA_M_EXPORT or LSA_M_RTCALC */
    .dom  = oa ? oa->areaid : 0,
    .id   = ort_to_lsaid(p, nf),
    .opts = oa ? (pbit ? OPT_P : 0) : OPT_E,
    .nf   = nf
  };

  if (ospf_is_v2(p))
    prepare_ext2_lsa_body(p, nf->fn.pxlen, metric, ebit, fwaddr, tag);
  else
    prepare_ext3_lsa_body(p, nf, metric, ebit, fwaddr, tag, oa && pbit);

  ospf_originate_lsa(p, &lsa);
}

static struct top_hash_entry *
ospf_hash_find_(struct top_graph *f, u32 domain, u32 lsa, u32 rtr, u32 type);

static void
ospf_flush_ext_lsa(struct ospf_proto *p, struct ospf_area *oa, ort *nf)
{
  struct top_hash_entry *en;

  u32 type = oa ? LSA_T_NSSA : LSA_T_EXT;
  u32 dom = oa ? oa->areaid : 0;
  u32 id = ort_to_lsaid(p, nf);

  en = ospf_hash_find_(p->gr, dom, id, p->router_id, type);

  if (!en || (en->nf != nf))
    return;

  ospf_flush_lsa(p, en);
}

static inline int
use_gw_for_fwaddr(struct ospf_proto *p, ip_addr gw, struct iface *iface)
{
  struct ospf_iface *ifa;

  if (ipa_zero(gw) || ipa_is_link_local(gw))
    return 0;

  WALK_LIST(ifa, p->iface_list)
    if ((ifa->iface == iface) &&
	(!ospf_is_v2(p) || ipa_in_net(gw, ifa->addr->prefix, ifa->addr->pxlen)))
      return 1;

  return 0;
}

static inline ip_addr
find_surrogate_fwaddr(struct ospf_proto *p, struct ospf_area *oa)
{
  struct ospf_iface *ifa;
  struct ifa *a, *cur_addr = NULL;
  int np, cur_np = 0;

  /* RFC 3101 2.3 - surrogate forwarding address selection */

  WALK_LIST(ifa, p->iface_list)
  {
    if ((ifa->oa != oa) ||
	(ifa->type == OSPF_IT_VLINK))
      continue;

    if (ospf_is_v2(p))
    {
      a = ifa->addr;
      if (a->flags & IA_PEER)
	continue;

      np = (a->flags & IA_HOST) ? 3 : (ifa->stub ? 2 : 1);
      if (np > cur_np)
      {
	cur_addr = a;
	cur_np = np;
      }
    }
    else /* OSPFv3 */
    {
      WALK_LIST(a, ifa->iface->addrs)
      {
	if ((a->flags & IA_SECONDARY) ||
	    (a->flags & IA_PEER) ||
	    (a->scope <= SCOPE_LINK))
	  continue;

	np = (a->flags & IA_HOST) ? 3 : (ifa->stub ? 2 : 1);
	if (np > cur_np)
	{
	  cur_addr = a;
	  cur_np = np;
	}
      }
    }
  }

  return cur_addr ? cur_addr->ip : IPA_NONE;
}

void
ospf_rt_notify(struct proto *P, rtable *tbl UNUSED, net *n, rte *new, rte *old UNUSED, ea_list *ea)
{
  struct ospf_proto *p = (struct ospf_proto *) P;
  struct ospf_area *oa = NULL;	/* non-NULL for NSSA-LSA */
  ort *nf;

  /*
   * There are several posibilities:
   * 1) router in regular area - originate external LSA with global scope
   * 2) router in NSSA area - originate area-specific NSSA-LSA
   * 3) router in stub area - cannot export routes
   * 4) area border router - same as (1), it is attached to backbone
   */

  if ((p->areano == 1) && oa_is_nssa(HEAD(p->area_list)))
    oa = HEAD(p->area_list);

  if (!new)
  {
    nf = (ort *) fib_find(&p->rtf, &n->n.prefix, n->n.pxlen);

    if (!nf || !nf->external_rte)
      return;

    ospf_flush_ext_lsa(p, oa, nf);
    nf->external_rte = 0;

    /* Old external route might blocked some NSSA translation */
    if ((p->areano > 1) && rt_is_nssa(nf) && nf->n.oa->translate)
      ospf_schedule_rtcalc(p);

    return;
  }

  ASSERT(p->asbr);

  /* Get route attributes */
  rta *a = new->attrs;
  u32 m1 = ea_get_int(ea, EA_OSPF_METRIC1, LSINFINITY);
  u32 m2 = ea_get_int(ea, EA_OSPF_METRIC2, 10000);
  int ebit = (m1 == LSINFINITY);
  u32 metric = ebit ? m2 : m1;
  u32 tag = ea_get_int(ea, EA_OSPF_TAG, 0);
  ip_addr fwd = IPA_NONE;


  if ((a->dest == RTD_ROUTER) && use_gw_for_fwaddr(p, a->gw, a->iface))
    fwd = a->gw;

  /* NSSA-LSA with P-bit set must have non-zero forwarding address */
  if (oa && ipa_zero(fwd))
  {
    fwd = find_surrogate_fwaddr(p, oa);

    if (ipa_zero(fwd))
    {
      log(L_ERR "%s: Cannot find forwarding address for NSSA-LSA %I/%d",
	  p->p.name, n->n.prefix, n->n.pxlen);
      return;
    }
  }

  nf = (ort *) fib_get(&p->rtf, &n->n.prefix, n->n.pxlen);
  ospf_originate_ext_lsa(p, oa, nf, LSA_M_EXPORT, metric, ebit, fwd, tag, 1);
  nf->external_rte = 1;
}


/*
 *	Link-LSA handling (assume OSPFv3)
 *	Type = LSA_T_LINK
 */

static inline void
lsab_put_prefix(struct ospf_proto *p, ip_addr prefix, u32 pxlen, u32 cost)
{
  void *buf = lsab_alloc(p, IPV6_PREFIX_SPACE(pxlen));
  u8 flags = (pxlen < MAX_PREFIX_LENGTH) ? 0 : OPT_PX_LA;
  put_ipv6_prefix(buf, prefix, pxlen, flags, cost);
}

static void
prepare_link_lsa_body(struct ospf_proto *p, struct ospf_iface *ifa)
{
  struct ospf_lsa_link *ll;
  int i = 0;

  ASSERT(p->lsab_used == 0);
  ll = lsab_allocz(p, sizeof(struct ospf_lsa_link));
  ll->options = ifa->oa->options | (ifa->priority << 24);
  ll->lladdr = ipa_to_ip6(ifa->addr->ip);
  ll = NULL; /* buffer might be reallocated later */

  struct ifa *a;
  WALK_LIST(a, ifa->iface->addrs)
  {
    if ((a->flags & IA_SECONDARY) ||
	(a->scope < SCOPE_SITE))
      continue;

    lsab_put_prefix(p, a->prefix, a->pxlen, 0);
    i++;
  }

  ll = p->lsab;
  ll->pxcount = i;
}

static void
ospf_originate_link_lsa(struct ospf_proto *p, struct ospf_iface *ifa)
{
  if (ospf_is_v2(p))
    return;

  struct ospf_new_lsa lsa = {
    .type = LSA_T_LINK,
    .dom  = ifa->iface_id,
    .id   = ifa->iface_id,
    .ifa  = ifa
  };

  OSPF_TRACE(D_EVENTS, "Updating link state for %s (Id: %R)", ifa->ifname, lsa.id);

  prepare_link_lsa_body(p, ifa);

  ifa->link_lsa = ospf_originate_lsa(p, &lsa);
}


/*
 *	Prefix-Rt-LSA handling (assume OSPFv3)
 *	Type = LSA_T_PREFIX, referred type = LSA_T_RT
 */

static void
prepare_prefix_rt_lsa_body(struct ospf_proto *p, struct ospf_area *oa)
{
  struct ospf_config *cf = (struct ospf_config *) (p->p.cf);
  struct ospf_iface *ifa;
  struct ospf_lsa_prefix *lp;
  int host_addr = 0;
  int net_lsa;
  int i = 0;

  ASSERT(p->lsab_used == 0);
  lp = lsab_allocz(p, sizeof(struct ospf_lsa_prefix));
  lp->ref_type = LSA_T_RT;
  lp->ref_id = 0;
  lp->ref_rt = p->router_id;
  lp = NULL; /* buffer might be reallocated later */

  WALK_LIST(ifa, p->iface_list)
  {
    if ((ifa->oa != oa) || (ifa->type == OSPF_IT_VLINK) || (ifa->state == OSPF_IS_DOWN))
      continue;

    ifa->px_pos_beg = i;

    if ((ifa->type == OSPF_IT_BCAST) ||
	(ifa->type == OSPF_IT_NBMA))
      net_lsa = bcast_net_active(ifa);
    else
      net_lsa = 0;

    struct ifa *a;
    WALK_LIST(a, ifa->iface->addrs)
    {
      if ((a->flags & IA_SECONDARY) ||
	  (a->flags & IA_PEER) ||
	  (a->scope <= SCOPE_LINK))
	continue;

      if (((a->pxlen < MAX_PREFIX_LENGTH) && net_lsa) ||
	  configured_stubnet(oa, a))
	continue;

      if ((a->flags & IA_HOST) ||
	  (ifa->state == OSPF_IS_LOOP) ||
	  (ifa->type == OSPF_IT_PTMP))
      {
	lsab_put_prefix(p, a->ip, MAX_PREFIX_LENGTH, 0);
	host_addr = 1;
      }
      else
	lsab_put_prefix(p, a->prefix, a->pxlen, ifa->cost);
      i++;
    }

    ifa->px_pos_end = i;
  }

  struct ospf_stubnet_config *sn;
  WALK_LIST(sn, oa->ac->stubnet_list)
    if (!sn->hidden)
    {
      lsab_put_prefix(p, sn->px.addr, sn->px.len, sn->cost);
      if (sn->px.len == MAX_PREFIX_LENGTH)
	host_addr = 1;
      i++;
    }

  /* If there are some configured vlinks, find some global address
     (even from another area), which will be used as a vlink endpoint. */
  if (!EMPTY_LIST(cf->vlink_list) && !host_addr)
  {
    WALK_LIST(ifa, p->iface_list)
    {
      if ((ifa->type == OSPF_IT_VLINK) || (ifa->state == OSPF_IS_DOWN))
	continue;

      struct ifa *a;
      WALK_LIST(a, ifa->iface->addrs)
      {
	if ((a->flags & IA_SECONDARY) || (a->scope <= SCOPE_LINK))
	  continue;

	/* Found some IP */
	lsab_put_prefix(p, a->ip, MAX_PREFIX_LENGTH, 0);
	i++;
	goto done;
      }
    }
  }

 done:
  lp = p->lsab;
  lp->pxcount = i;
}

static void
ospf_originate_prefix_rt_lsa(struct ospf_proto *p, struct ospf_area *oa)
{
  if (ospf_is_v2(p))
    return;

  struct ospf_new_lsa lsa = {
    .type = LSA_T_PREFIX,
    .dom  = oa->areaid,
    .id   = 0
  };

  prepare_prefix_rt_lsa_body(p, oa);

  ospf_originate_lsa(p, &lsa);
}


/*
 *	Prefix-Net-LSA handling (assume OSPFv3)
 *	Type = LSA_T_PREFIX, referred type = LSA_T_NET
 */

static inline int
prefix_space(u32 *buf)
{
  int pxl = *buf >> 24;
  return IPV6_PREFIX_SPACE(pxl);
}

static inline int
prefix_same(u32 *b1, u32 *b2)
{
  int pxl1 = *b1 >> 24;
  int pxl2 = *b2 >> 24;
  int pxs, i;

  if (pxl1 != pxl2)
    return 0;

  pxs = IPV6_PREFIX_WORDS(pxl1);
  for (i = 1; i < pxs; i++)
    if (b1[i] != b2[i])
      return 0;

  return 1;
}

static inline u32 *
prefix_advance(u32 *buf)
{
  int pxl = *buf >> 24;
  return buf + IPV6_PREFIX_WORDS(pxl);
}

/* FIXME eliminate items with LA bit set? see 4.4.3.9 */
static void
add_prefix(struct ospf_proto *p, u32 *px, int offset, int *pxc)
{
  u32 *pxl = lsab_offset(p, offset);
  int i;
  for (i = 0; i < *pxc; pxl = prefix_advance(pxl), i++)
    if (prefix_same(px, pxl))
    {
      /* Options should be logically OR'ed together */
      *pxl |= (*px & 0x00FF0000);
      return;
    }

  ASSERT(pxl == lsab_end(p));

  int pxspace = prefix_space(px);
  pxl = lsab_alloc(p, pxspace);
  memcpy(pxl, px, pxspace);
  *pxl &= 0xFFFF0000;	/* Set metric to zero */
  (*pxc)++;
}

static void
add_link_lsa(struct ospf_proto *p, struct ospf_lsa_link *ll, int offset, int *pxc)
{
  u32 *pxb = ll->rest;
  uint j;

  for (j = 0; j < ll->pxcount; pxb = prefix_advance(pxb), j++)
  {
    u8 pxlen = (pxb[0] >> 24);
    u8 pxopts = (pxb[0] >> 16);

    /* Skip NU or LA prefixes */
    if (pxopts & (OPT_PX_NU | OPT_PX_LA))
      continue;

    /* Skip link-local prefixes */
    if ((pxlen >= 10) && ((pxb[1] & 0xffc00000) == 0xfe800000))
      continue;

    add_prefix(p, pxb, offset, pxc);
  }
}

static void
prepare_prefix_net_lsa_body(struct ospf_proto *p, struct ospf_iface *ifa)
{
  struct ospf_lsa_prefix *lp;
  struct ospf_neighbor *n;
  struct top_hash_entry *en;
  int pxc, offset;

  ASSERT(p->lsab_used == 0);
  lp = lsab_allocz(p, sizeof(struct ospf_lsa_prefix));
  lp->ref_type = LSA_T_NET;
  lp->ref_id = ifa->net_lsa->lsa.id;
  lp->ref_rt = p->router_id;
  lp = NULL; /* buffer might be reallocated later */

  pxc = 0;
  offset = p->lsab_used;

  /* Find all Link LSAs associated with the link and merge their prefixes */
  if (en = ifa->link_lsa)
    add_link_lsa(p, en->next_lsa_body ?: en->lsa_body, offset, &pxc);

  WALK_LIST(n, ifa->neigh_list)
    if ((n->state == NEIGHBOR_FULL) &&
	(en = ospf_hash_find(p->gr, ifa->iface_id, n->iface_id, n->rid, LSA_T_LINK)))
      add_link_lsa(p, en->lsa_body, offset, &pxc);

  lp = p->lsab;
  lp->pxcount = pxc;
}

static void
ospf_originate_prefix_net_lsa(struct ospf_proto *p, struct ospf_iface *ifa)
{
  if (ospf_is_v2(p))
    return;

  struct ospf_new_lsa lsa = {
    .type = LSA_T_PREFIX,
    .dom  = ifa->oa->areaid,
    .id   = ifa->iface_id,
    .ifa  = ifa
  };

  prepare_prefix_net_lsa_body(p, ifa);

  ifa->pxn_lsa = ospf_originate_lsa(p, &lsa);
}

static inline int breaks_minlsinterval(struct top_hash_entry *en)
{ return en && (en->lsa.age < LSA_MAXAGE) && ((en->inst_time + MINLSINTERVAL) > now); }

void
ospf_update_topology(struct ospf_proto *p)
{
  struct ospf_area *oa;
  struct ospf_iface *ifa;

  WALK_LIST(oa, p->area_list)
  {
    if (oa->update_rt_lsa)
    {
      /*
       * Generally, MinLSInterval is enforced in ospf_do_originate_lsa(), but
       * origination of (prefix) router LSA is a special case. We do not want to
       * prepare a new router LSA body and then postpone it in en->next_lsa_body
       * for later origination, because there are side effects (updates of
       * rt_pos_* and px_pos_* in ospf_iface structures) during that, which may
       * confuse routing table calculation if executed after LSA body
       * preparation but before final LSA origination (as rtcalc would use
       * current rt_pos_* indexes but the old router LSA body).
       *
       * Here, we ensure that MinLSInterval is observed and we do not even try
       * to originate these LSAs if it is not. Therefore, origination, when
       * requested, will succeed unless there is also a seqnum wrapping, which
       * is not a problem because in that case rtcalc is blocked by MaxAge.
       */

      if (breaks_minlsinterval(oa->rt) || breaks_minlsinterval(oa->pxr_lsa))
	continue;

      ospf_originate_rt_lsa(p, oa);
      ospf_originate_prefix_rt_lsa(p, oa);
      oa->update_rt_lsa = 0;
    }
  }

  WALK_LIST(ifa, p->iface_list)
  {
    if (ifa->type == OSPF_IT_VLINK)
      continue;

    if (ifa->update_link_lsa)
    {
      if ((ifa->state > OSPF_IS_LOOP) && !ifa->link_lsa_suppression)
	ospf_originate_link_lsa(p, ifa);
      else
	ospf_flush2_lsa(p, &ifa->link_lsa);

      ifa->update_link_lsa = 0;
    }

    if (ifa->update_net_lsa)
    {
      if ((ifa->state == OSPF_IS_DR) && (ifa->fadj > 0))
      {
	ospf_originate_net_lsa(p, ifa);
	ospf_originate_prefix_net_lsa(p, ifa);
      }
      else
      {
	ospf_flush2_lsa(p, &ifa->net_lsa);
	ospf_flush2_lsa(p, &ifa->pxn_lsa);
      }

      ifa->update_net_lsa = 0;
    }
  }
}


static void
ospf_top_ht_alloc(struct top_graph *f)
{
  f->hash_size = 1 << f->hash_order;
  f->hash_mask = f->hash_size - 1;
  if (f->hash_order > HASH_HI_MAX - HASH_HI_STEP)
    f->hash_entries_max = ~0;
  else
    f->hash_entries_max = f->hash_size HASH_HI_MARK;
  if (f->hash_order < HASH_LO_MIN + HASH_LO_STEP)
    f->hash_entries_min = 0;
  else
    f->hash_entries_min = f->hash_size HASH_LO_MARK;
  DBG("Allocating OSPF hash of order %d: %d hash_entries, %d low, %d high\n",
      f->hash_order, f->hash_size, f->hash_entries_min, f->hash_entries_max);
  f->hash_table =
    mb_alloc(f->pool, f->hash_size * sizeof(struct top_hash_entry *));
  bzero(f->hash_table, f->hash_size * sizeof(struct top_hash_entry *));
}

static inline void
ospf_top_ht_free(struct top_hash_entry **h)
{
  mb_free(h);
}

static inline u32
ospf_top_hash_u32(u32 a)
{
  /* Shamelessly stolen from IP address hashing in ipv4.h */
  a ^= a >> 16;
  a ^= a << 10;
  return a;
}

static uint
ospf_top_hash(struct top_graph *f, u32 domain, u32 lsaid, u32 rtrid, u32 type)
{
  /* In OSPFv2, we don't know Router ID when looking for network LSAs.
     In OSPFv3, we don't know LSA ID when looking for router LSAs.
     In both cases, there is (usually) just one (or small number)
     appropriate LSA, so we just clear unknown part of key. */

  return (((f->ospf2 && (type == LSA_T_NET)) ? 0 : ospf_top_hash_u32(rtrid)) +
	  ((!f->ospf2 && (type == LSA_T_RT)) ? 0 : ospf_top_hash_u32(lsaid)) +
	  type + domain) & f->hash_mask;

  /*
  return (ospf_top_hash_u32(lsaid) + ospf_top_hash_u32(rtrid) +
	  type + areaid) & f->hash_mask;
  */
}

/**
 * ospf_top_new - allocated new topology database
 * @p: OSPF protocol instance
 * @pool: pool for allocation
 *
 * This dynamically hashed structure is used for keeping LSAs. Mainly it is used
 * for the LSA database of the OSPF protocol, but also for LSA retransmission
 * and request lists of OSPF neighbors.
 */
struct top_graph *
ospf_top_new(struct ospf_proto *p UNUSED4 UNUSED6, pool *pool)
{
  struct top_graph *f;

  f = mb_allocz(pool, sizeof(struct top_graph));
  f->pool = pool;
  f->hash_slab = sl_new(f->pool, sizeof(struct top_hash_entry));
  f->hash_order = HASH_DEF_ORDER;
  ospf_top_ht_alloc(f);
  f->hash_entries = 0;
  f->hash_entries_min = 0;
  f->ospf2 = ospf_is_v2(p);
  return f;
}

void
ospf_top_free(struct top_graph *f)
{
  rfree(f->hash_slab);
  ospf_top_ht_free(f->hash_table);
  mb_free(f);
}

static void
ospf_top_rehash(struct top_graph *f, int step)
{
  struct top_hash_entry **n, **oldt, **newt, *e, *x;
  uint oldn, oldh;

  oldn = f->hash_size;
  oldt = f->hash_table;
  DBG("re-hashing topology hash from order %d to %d\n", f->hash_order,
      f->hash_order + step);
  f->hash_order += step;
  ospf_top_ht_alloc(f);
  newt = f->hash_table;

  for (oldh = 0; oldh < oldn; oldh++)
  {
    e = oldt[oldh];
    while (e)
    {
      x = e->next;
      n = newt + ospf_top_hash(f, e->domain, e->lsa.id, e->lsa.rt, e->lsa_type);
      e->next = *n;
      *n = e;
      e = x;
    }
  }
  ospf_top_ht_free(oldt);
}

static struct top_hash_entry *
ospf_hash_find_(struct top_graph *f, u32 domain, u32 lsa, u32 rtr, u32 type)
{
  struct top_hash_entry *e;
  e = f->hash_table[ospf_top_hash(f, domain, lsa, rtr, type)];

  while (e && (e->lsa.id != lsa || e->lsa.rt != rtr ||
	       e->lsa_type != type || e->domain != domain))
    e = e->next;

  return e;
}

struct top_hash_entry *
ospf_hash_find(struct top_graph *f, u32 domain, u32 lsa, u32 rtr, u32 type)
{
  struct top_hash_entry *e = ospf_hash_find_(f, domain, lsa, rtr, type);

  /* Hide hash entry with empty lsa_body */
  return (e && e->lsa_body) ? e : NULL;
}

/* In OSPFv2, lsa.id is the same as lsa.rt for router LSA. In OSPFv3, we don't know
   lsa.id when looking for router LSAs. We return matching LSA with smallest lsa.id. */
struct top_hash_entry *
ospf_hash_find_rt(struct top_graph *f, u32 domain, u32 rtr)
{
  struct top_hash_entry *rv = NULL;
  struct top_hash_entry *e;
  /* We can put rtr for lsa.id to hash fn, it is ignored in OSPFv3 */
  e = f->hash_table[ospf_top_hash(f, domain, rtr, rtr, LSA_T_RT)];

  while (e)
  {
    if (e->lsa.rt == rtr && e->lsa_type == LSA_T_RT && e->domain == domain && e->lsa_body)
    {
      if (f->ospf2 && (e->lsa.id == rtr))
	return e;
      if (!f->ospf2 && (!rv || e->lsa.id < rv->lsa.id))
	rv = e;
    }
    e = e->next;
  }

  return rv;
}

/*
 * ospf_hash_find_rt3_first() and ospf_hash_find_rt3_next() are used exclusively
 * for lsa_walk_rt_init(), lsa_walk_rt(), therefore they skip MaxAge entries.
 */
static inline struct top_hash_entry *
find_matching_rt3(struct top_hash_entry *e, u32 domain, u32 rtr)
{
  while (e && (e->lsa.rt != rtr || e->lsa_type != LSA_T_RT ||
	       e->domain != domain || e->lsa.age == LSA_MAXAGE))
    e = e->next;
  return e;
}

struct top_hash_entry *
ospf_hash_find_rt3_first(struct top_graph *f, u32 domain, u32 rtr)
{
  struct top_hash_entry *e;
  e = f->hash_table[ospf_top_hash(f, domain, 0, rtr, LSA_T_RT)];
  return find_matching_rt3(e, domain, rtr);
}

struct top_hash_entry *
ospf_hash_find_rt3_next(struct top_hash_entry *e)
{
  return find_matching_rt3(e->next, e->domain, e->lsa.rt);
}

/* In OSPFv2, we don't know Router ID when looking for network LSAs.
   There should be just one, so we find any match. */
struct top_hash_entry *
ospf_hash_find_net2(struct top_graph *f, u32 domain, u32 id)
{
  struct top_hash_entry *e;
  e = f->hash_table[ospf_top_hash(f, domain, id, 0, LSA_T_NET)];

  while (e && (e->lsa.id != id || e->lsa_type != LSA_T_NET ||
	       e->domain != domain || e->lsa_body == NULL))
    e = e->next;

  return e;
}


struct top_hash_entry *
ospf_hash_get(struct top_graph *f, u32 domain, u32 lsa, u32 rtr, u32 type)
{
  struct top_hash_entry **ee;
  struct top_hash_entry *e;

  ee = f->hash_table + ospf_top_hash(f, domain, lsa, rtr, type);
  e = *ee;

  while (e && (e->lsa.id != lsa || e->lsa.rt != rtr ||
	       e->lsa_type != type || e->domain != domain))
    e = e->next;

  if (e)
    return e;

  e = sl_alloc(f->hash_slab);
  bzero(e, sizeof(struct top_hash_entry));

  e->color = OUTSPF;
  e->dist = LSINFINITY;
  e->lsa.type_raw = type;
  e->lsa.id = lsa;
  e->lsa.rt = rtr;
  e->lsa.sn = LSA_ZEROSEQNO;
  e->lsa_type = type;
  e->domain = domain;
  e->next = *ee;
  *ee = e;
  if (f->hash_entries++ > f->hash_entries_max)
    ospf_top_rehash(f, HASH_HI_STEP);
  return e;
}

void
ospf_hash_delete(struct top_graph *f, struct top_hash_entry *e)
{
  struct top_hash_entry **ee = f->hash_table +
    ospf_top_hash(f, e->domain, e->lsa.id, e->lsa.rt, e->lsa_type);

  while (*ee)
  {
    if (*ee == e)
    {
      *ee = e->next;
      sl_free(f->hash_slab, e);
      if (f->hash_entries-- < f->hash_entries_min)
	ospf_top_rehash(f, -HASH_LO_STEP);
      return;
    }
    ee = &((*ee)->next);
  }
  bug("ospf_hash_delete() called for invalid node");
}

/*
static void
ospf_dump_lsa(struct top_hash_entry *he, struct proto *p)
{

  struct ospf_lsa_rt *rt = NULL;
  struct ospf_lsa_rt_link *rr = NULL;
  struct ospf_lsa_net *ln = NULL;
  u32 *rts = NULL;
  u32 i, max;

  OSPF_TRACE(D_EVENTS, "- %1x %-1R %-1R %4u 0x%08x 0x%04x %-1R",
	     he->lsa.type, he->lsa.id, he->lsa.rt, he->lsa.age, he->lsa.sn,
	     he->lsa.checksum, he->domain);


  switch (he->lsa.type)
    {
    case LSA_T_RT:
      rt = he->lsa_body;
      rr = (struct ospf_lsa_rt_link *) (rt + 1);

      for (i = 0; i < lsa_rt_items(&he->lsa); i++)
	OSPF_TRACE(D_EVENTS, "  - %1x %-1R %-1R %5u",
		   rr[i].type, rr[i].id, rr[i].data, rr[i].metric);
      break;

    case LSA_T_NET:
      ln = he->lsa_body;
      rts = (u32 *) (ln + 1);

      for (i = 0; i < lsa_net_items(&he->lsa); i++)
	OSPF_TRACE(D_EVENTS, "  - %-1R", rts[i]);
      break;

    default:
      break;
    }
}

void
ospf_top_dump(struct top_graph *f, struct proto *p)
{
  uint i;
  OSPF_TRACE(D_EVENTS, "Hash entries: %d", f->hash_entries);

  for (i = 0; i < f->hash_size; i++)
  {
    struct top_hash_entry *e;
    for (e = f->hash_table[i]; e != NULL; e = e->next)
      ospf_dump_lsa(e, p);
  }
}
*/
