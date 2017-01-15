/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


const char *ospf_ns_names[] = {
  "Down", "Attempt", "Init", "2-Way", "ExStart", "Exchange", "Loading", "Full"
};

const char *ospf_inm_names[] = {
  "HelloReceived", "Start", "2-WayReceived", "NegotiationDone", "ExchangeDone",
  "BadLSReq", "LoadingDone", "AdjOK?", "SeqNumberMismatch", "1-WayReceived",
  "KillNbr", "InactivityTimer", "LLDown"
};


static int can_do_adj(struct ospf_neighbor *n);
static void inactivity_timer_hook(timer * timer);
static void dbdes_timer_hook(timer *t);
static void lsrq_timer_hook(timer *t);
static void lsrt_timer_hook(timer *t);
static void ackd_timer_hook(timer *t);


static void
init_lists(struct ospf_proto *p, struct ospf_neighbor *n)
{
  s_init_list(&(n->lsrql));
  n->lsrqi = SHEAD(n->lsrql);
  n->lsrqh = ospf_top_new(p, n->pool);

  s_init_list(&(n->lsrtl));
  n->lsrth = ospf_top_new(p, n->pool);
}

static void
release_lsrtl(struct ospf_proto *p, struct ospf_neighbor *n)
{
  struct top_hash_entry *ret, *en;

  WALK_SLIST(ret, n->lsrtl)
  {
    en = ospf_hash_find_entry(p->gr, ret);
    if (en)
      en->ret_count--;
  }
}

/* Resets LSA request and retransmit lists.
 * We do not reset DB summary list iterator here,
 * it is reset during entering EXCHANGE state.
 */
static void
reset_lists(struct ospf_proto *p, struct ospf_neighbor *n)
{
  release_lsrtl(p, n);
  ospf_top_free(n->lsrqh);
  ospf_top_free(n->lsrth);
  ospf_reset_lsack_queue(n);

  tm_stop(n->dbdes_timer);
  tm_stop(n->lsrq_timer);
  tm_stop(n->lsrt_timer);
  tm_stop(n->ackd_timer);

  init_lists(p, n);
}

struct ospf_neighbor *
ospf_neighbor_new(struct ospf_iface *ifa)
{
  struct ospf_proto *p = ifa->oa->po;
  struct pool *pool = rp_new(p->p.pool, "OSPF Neighbor");
  struct ospf_neighbor *n = mb_allocz(pool, sizeof(struct ospf_neighbor));

  n->pool = pool;
  n->ifa = ifa;
  add_tail(&ifa->neigh_list, NODE n);
  n->adj = 0;
  n->csn = 0;
  n->state = NEIGHBOR_DOWN;

  init_lists(p, n);
  s_init(&(n->dbsi), &(p->lsal));

  init_list(&n->ackl[ACKL_DIRECT]);
  init_list(&n->ackl[ACKL_DELAY]);

  n->inactim = tm_new_set(pool, inactivity_timer_hook, n, 0, 0);
  n->dbdes_timer = tm_new_set(pool, dbdes_timer_hook, n, 0, ifa->rxmtint);
  n->lsrq_timer = tm_new_set(pool, lsrq_timer_hook, n, 0, ifa->rxmtint);
  n->lsrt_timer = tm_new_set(pool, lsrt_timer_hook, n, 0, ifa->rxmtint);
  n->ackd_timer = tm_new_set(pool, ackd_timer_hook, n, 0, ifa->rxmtint / 2);

  return (n);
}

static void
ospf_neigh_down(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  struct ospf_proto *p = ifa->oa->po;
  u32 rid = n->rid;

  if ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_PTMP))
  {
    struct nbma_node *nn = find_nbma_node(ifa, n->ip);
    if (nn)
      nn->found = 0;
  }

  s_get(&(n->dbsi));
  release_lsrtl(p, n);
  rem_node(NODE n);
  rfree(n->pool);

  OSPF_TRACE(D_EVENTS, "Neighbor %R on %s removed", rid, ifa->ifname);
}

/**
 * ospf_neigh_chstate - handles changes related to new or lod state of neighbor
 * @n: OSPF neighbor
 * @state: new state
 *
 * Many actions have to be taken acording to a change of state of a neighbor. It
 * starts rxmt timers, call interface state machine etc.
 */
static void
ospf_neigh_chstate(struct ospf_neighbor *n, u8 state)
{
  struct ospf_iface *ifa = n->ifa;
  struct ospf_proto *p = ifa->oa->po;
  u8 old_state = n->state;
  int old_fadj = ifa->fadj;

  if (state == old_state)
    return;

  OSPF_TRACE(D_EVENTS, "Neighbor %R on %s changed state from %s to %s",
	     n->rid, ifa->ifname, ospf_ns_names[old_state], ospf_ns_names[state]);

  n->state = state;

  /* Increase number of partial adjacencies */
  if ((state == NEIGHBOR_EXCHANGE) || (state == NEIGHBOR_LOADING))
    p->padj++;

  /* Decrease number of partial adjacencies */
  if ((old_state == NEIGHBOR_EXCHANGE) || (old_state == NEIGHBOR_LOADING))
    p->padj--;

  /* Increase number of full adjacencies */
  if (state == NEIGHBOR_FULL)
    ifa->fadj++;

  /* Decrease number of full adjacencies */
  if (old_state == NEIGHBOR_FULL)
    ifa->fadj--;

  if (ifa->fadj != old_fadj)
  {
    /* RFC 2328 12.4 Event 4 - neighbor enters/leaves Full state */
    ospf_notify_rt_lsa(ifa->oa);
    ospf_notify_net_lsa(ifa);

    /* RFC 2328 12.4 Event 8 - vlink state change */
    if (ifa->type == OSPF_IT_VLINK)
      ospf_notify_rt_lsa(ifa->voa);
  }

  if (state == NEIGHBOR_EXSTART)
  {
    /* First time adjacency */
    if (n->adj == 0)
      n->dds = random_u32();

    n->dds++;
    n->myimms = DBDES_IMMS;

    tm_start(n->dbdes_timer, 0);
    tm_start(n->ackd_timer, ifa->rxmtint / 2);
  }

  if (state > NEIGHBOR_EXSTART)
    n->myimms &= ~DBDES_I;

  /* Generate NeighborChange event if needed, see RFC 2328 9.2 */
  if ((state == NEIGHBOR_2WAY) && (old_state < NEIGHBOR_2WAY))
    ospf_iface_sm(ifa, ISM_NEICH);
  if ((state < NEIGHBOR_2WAY) && (old_state >= NEIGHBOR_2WAY))
    ospf_iface_sm(ifa, ISM_NEICH);
}

/**
 * ospf_neigh_sm - ospf neighbor state machine
 * @n: neighor
 * @event: actual event
 *
 * This part implements the neighbor state machine as described in 10.3 of
 * RFC 2328. The only difference is that state %NEIGHBOR_ATTEMPT is not
 * used. We discover neighbors on nonbroadcast networks in the
 * same way as on broadcast networks. The only difference is in
 * sending hello packets. These are sent to IPs listed in
 * @ospf_iface->nbma_list .
 */
void
ospf_neigh_sm(struct ospf_neighbor *n, int event)
{
  struct ospf_proto *p = n->ifa->oa->po;

  DBG("Neighbor state machine for %R on %s, event %s\n",
      n->rid, n->ifa->ifname, ospf_inm_names[event]);

  switch (event)
  {
  case INM_START:
    ospf_neigh_chstate(n, NEIGHBOR_ATTEMPT);
    /* NBMA are used different way */
    break;

  case INM_HELLOREC:
    if (n->state < NEIGHBOR_INIT)
      ospf_neigh_chstate(n, NEIGHBOR_INIT);

    /* Restart inactivity timer */
    tm_start(n->inactim, n->ifa->deadint);
    break;

  case INM_2WAYREC:
    if (n->state < NEIGHBOR_2WAY)
      ospf_neigh_chstate(n, NEIGHBOR_2WAY);
    if ((n->state == NEIGHBOR_2WAY) && can_do_adj(n))
      ospf_neigh_chstate(n, NEIGHBOR_EXSTART);
    break;

  case INM_NEGDONE:
    if (n->state == NEIGHBOR_EXSTART)
    {
      ospf_neigh_chstate(n, NEIGHBOR_EXCHANGE);

      /* Reset DB summary list iterator */
      s_get(&(n->dbsi));
      s_init(&(n->dbsi), &p->lsal);

      /* Add MaxAge LSA entries to retransmission list */
      ospf_add_flushed_to_lsrt(p, n);
    }
    else
      bug("NEGDONE and I'm not in EXSTART?");
    break;

  case INM_EXDONE:
    if (!EMPTY_SLIST(n->lsrql))
      ospf_neigh_chstate(n, NEIGHBOR_LOADING);
    else
      ospf_neigh_chstate(n, NEIGHBOR_FULL);
    break;

  case INM_LOADDONE:
    ospf_neigh_chstate(n, NEIGHBOR_FULL);
    break;

  case INM_ADJOK:
    /* Can In build adjacency? */
    if ((n->state == NEIGHBOR_2WAY) && can_do_adj(n))
    {
      ospf_neigh_chstate(n, NEIGHBOR_EXSTART);
    }
    else if ((n->state >= NEIGHBOR_EXSTART) && !can_do_adj(n))
    {
      reset_lists(p, n);
      ospf_neigh_chstate(n, NEIGHBOR_2WAY);
    }
    break;

  case INM_SEQMIS:
  case INM_BADLSREQ:
    if (n->state >= NEIGHBOR_EXCHANGE)
    {
      reset_lists(p, n);
      ospf_neigh_chstate(n, NEIGHBOR_EXSTART);
    }
    break;

  case INM_KILLNBR:
  case INM_LLDOWN:
  case INM_INACTTIM:
    /* No need for reset_lists() */
    ospf_neigh_chstate(n, NEIGHBOR_DOWN);
    ospf_neigh_down(n);
    break;

  case INM_1WAYREC:
    reset_lists(p, n);
    ospf_neigh_chstate(n, NEIGHBOR_INIT);
    break;

  default:
    bug("%s: INM - Unknown event?", p->p.name);
    break;
  }
}

static int
can_do_adj(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  struct ospf_proto *p = ifa->oa->po;
  int i = 0;

  switch (ifa->type)
  {
  case OSPF_IT_PTP:
  case OSPF_IT_PTMP:
  case OSPF_IT_VLINK:
    i = 1;
    break;
  case OSPF_IT_BCAST:
  case OSPF_IT_NBMA:
    switch (ifa->state)
    {
    case OSPF_IS_DOWN:
    case OSPF_IS_LOOP:
      bug("%s: Iface %s in down state?", p->p.name, ifa->ifname);
      break;
    case OSPF_IS_WAITING:
      DBG("%s: Neighbor? on iface %s\n", p->p.name, ifa->ifname);
      break;
    case OSPF_IS_DROTHER:
      if (((n->rid == ifa->drid) || (n->rid == ifa->bdrid))
	  && (n->state >= NEIGHBOR_2WAY))
	i = 1;
      break;
    case OSPF_IS_PTP:
    case OSPF_IS_BACKUP:
    case OSPF_IS_DR:
      if (n->state >= NEIGHBOR_2WAY)
	i = 1;
      break;
    default:
      bug("%s: Iface %s in unknown state?", p->p.name, ifa->ifname);
      break;
    }
    break;
  default:
    bug("%s: Iface %s is unknown type?", p->p.name, ifa->ifname);
    break;
  }
  DBG("%s: Iface %s can_do_adj=%d\n", p->p.name, ifa->ifname, i);
  return i;
}


static inline u32 neigh_get_id(struct ospf_proto *p UNUSED4 UNUSED6, struct ospf_neighbor *n)
{ return ospf_is_v2(p) ? ipa_to_u32(n->ip) : n->rid; }

static struct ospf_neighbor *
elect_bdr(struct ospf_proto *p, list nl)
{
  struct ospf_neighbor *neigh, *n1, *n2;
  u32 nid;

  n1 = NULL;
  n2 = NULL;
  WALK_LIST(neigh, nl)			/* First try those decl. themselves */
  {
    nid = neigh_get_id(p, neigh);

    if (neigh->state >= NEIGHBOR_2WAY)	/* Higher than 2WAY */
      if (neigh->priority > 0)		/* Eligible */
	if (neigh->dr != nid)		/* And not decl. itself DR */
	{
	  if (neigh->bdr == nid)	/* Declaring BDR */
	  {
	    if (n1 != NULL)
	    {
	      if (neigh->priority > n1->priority)
		n1 = neigh;
	      else if (neigh->priority == n1->priority)
		if (neigh->rid > n1->rid)
		  n1 = neigh;
	    }
	    else
	    {
	      n1 = neigh;
	    }
	  }
	  else			/* And NOT declaring BDR */
	  {
	    if (n2 != NULL)
	    {
	      if (neigh->priority > n2->priority)
		n2 = neigh;
	      else if (neigh->priority == n2->priority)
		if (neigh->rid > n2->rid)
		  n2 = neigh;
	    }
	    else
	    {
	      n2 = neigh;
	    }
	  }
	}
  }
  if (n1 == NULL)
    n1 = n2;

  return (n1);
}

static struct ospf_neighbor *
elect_dr(struct ospf_proto *p, list nl)
{
  struct ospf_neighbor *neigh, *n;
  u32 nid;

  n = NULL;
  WALK_LIST(neigh, nl)			/* And now DR */
  {
    nid = neigh_get_id(p, neigh);

    if (neigh->state >= NEIGHBOR_2WAY)	/* Higher than 2WAY */
      if (neigh->priority > 0)		/* Eligible */
	if (neigh->dr == nid)		/* And declaring itself DR */
	{
	  if (n != NULL)
	  {
	    if (neigh->priority > n->priority)
	      n = neigh;
	    else if (neigh->priority == n->priority)
	      if (neigh->rid > n->rid)
		n = neigh;
	  }
	  else
	  {
	    n = neigh;
	  }
	}
  }

  return (n);
}

/**
 * ospf_dr_election - (Backup) Designed Router election
 * @ifa: actual interface
 *
 * When the wait timer fires, it is time to elect (Backup) Designated Router.
 * Structure describing me is added to this list so every electing router has
 * the same list. Backup Designated Router is elected before Designated
 * Router. This process is described in 9.4 of RFC 2328. The function is
 * supposed to be called only from ospf_iface_sm() as a part of the interface
 * state machine.
 */
void
ospf_dr_election(struct ospf_iface *ifa)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_neighbor *neigh, *ndr, *nbdr, me;
  u32 myid = p->router_id;

  DBG("(B)DR election.\n");

  me.state = NEIGHBOR_2WAY;
  me.rid = myid;
  me.priority = ifa->priority;
  me.ip = ifa->addr->ip;

  me.dr  = ospf_is_v2(p) ? ipa_to_u32(ifa->drip) : ifa->drid;
  me.bdr = ospf_is_v2(p) ? ipa_to_u32(ifa->bdrip) : ifa->bdrid;
  me.iface_id = ifa->iface_id;

  add_tail(&ifa->neigh_list, NODE & me);

  nbdr = elect_bdr(p, ifa->neigh_list);
  ndr = elect_dr(p, ifa->neigh_list);

  if (ndr == NULL)
    ndr = nbdr;

  /* 9.4. (4) */
  if (((ifa->drid == myid) && (ndr != &me))
      || ((ifa->drid != myid) && (ndr == &me))
      || ((ifa->bdrid == myid) && (nbdr != &me))
      || ((ifa->bdrid != myid) && (nbdr == &me)))
  {
    me.dr = ndr ? neigh_get_id(p, ndr) : 0;
    me.bdr = nbdr ? neigh_get_id(p, nbdr) : 0;

    nbdr = elect_bdr(p, ifa->neigh_list);
    ndr = elect_dr(p, ifa->neigh_list);

    if (ndr == NULL)
      ndr = nbdr;
  }

  rem_node(NODE & me);


  u32 old_drid = ifa->drid;
  u32 old_bdrid = ifa->bdrid;

  ifa->drid = ndr ? ndr->rid : 0;
  ifa->drip = ndr ? ndr->ip  : IPA_NONE;
  ifa->dr_iface_id = ndr ? ndr->iface_id : 0;

  ifa->bdrid = nbdr ? nbdr->rid : 0;
  ifa->bdrip = nbdr ? nbdr->ip  : IPA_NONE;

  DBG("DR=%R, BDR=%R\n", ifa->drid, ifa->bdrid);

  /* We are part of the interface state machine */
  if (ifa->drid == myid)
    ospf_iface_chstate(ifa, OSPF_IS_DR);
  else if (ifa->bdrid == myid)
    ospf_iface_chstate(ifa, OSPF_IS_BACKUP);
  else
    ospf_iface_chstate(ifa, OSPF_IS_DROTHER);

  /* Review neighbor adjacencies if DR or BDR changed */
  if ((ifa->drid != old_drid) || (ifa->bdrid != old_bdrid))
    WALK_LIST(neigh, ifa->neigh_list)
      if (neigh->state >= NEIGHBOR_2WAY)
	ospf_neigh_sm(neigh, INM_ADJOK);

  /* RFC 2328 12.4 Event 3 - DR change */
  if (ifa->drid != old_drid)
    ospf_notify_rt_lsa(ifa->oa);
}

struct ospf_neighbor *
find_neigh(struct ospf_iface *ifa, u32 rid)
{
  struct ospf_neighbor *n;
  WALK_LIST(n, ifa->neigh_list)
    if (n->rid == rid)
      return n;
  return NULL;
}

struct ospf_neighbor *
find_neigh_by_ip(struct ospf_iface *ifa, ip_addr ip)
{
  struct ospf_neighbor *n;
  WALK_LIST(n, ifa->neigh_list)
    if (ipa_equal(n->ip, ip))
      return n;
  return NULL;
}

static void
inactivity_timer_hook(timer * timer)
{
  struct ospf_neighbor *n = (struct ospf_neighbor *) timer->data;
  struct ospf_proto *p = n->ifa->oa->po;

  OSPF_TRACE(D_EVENTS, "Inactivity timer expired for nbr %R on %s",
	     n->rid, n->ifa->ifname);
  ospf_neigh_sm(n, INM_INACTTIM);
}

static void
ospf_neigh_bfd_hook(struct bfd_request *req)
{
  struct ospf_neighbor *n = req->data;
  struct ospf_proto *p = n->ifa->oa->po;

  if (req->down)
  {
    OSPF_TRACE(D_EVENTS, "BFD session down for nbr %R on %s",
	       n->rid, n->ifa->ifname);
    ospf_neigh_sm(n, INM_INACTTIM);
  }
}

void
ospf_neigh_update_bfd(struct ospf_neighbor *n, int use_bfd)
{
  if (use_bfd && !n->bfd_req)
    n->bfd_req = bfd_request_session(n->pool, n->ip, n->ifa->addr->ip, n->ifa->iface,
				     ospf_neigh_bfd_hook, n);

  if (!use_bfd && n->bfd_req)
  {
    rfree(n->bfd_req);
    n->bfd_req = NULL;
  }
}


static void
dbdes_timer_hook(timer *t)
{
  struct ospf_neighbor *n = t->data;
  struct ospf_proto *p = n->ifa->oa->po;

  // OSPF_TRACE(D_EVENTS, "DBDES timer expired for nbr %R on %s", n->rid, n->ifa->ifname);

  if (n->state == NEIGHBOR_EXSTART)
    ospf_send_dbdes(p, n);

  if ((n->state == NEIGHBOR_EXCHANGE) && (n->myimms & DBDES_MS))
    ospf_rxmt_dbdes(p, n);
}

static void
lsrq_timer_hook(timer *t)
{
  struct ospf_neighbor *n = t->data;
  struct ospf_proto *p = n->ifa->oa->po;

  // OSPF_TRACE(D_EVENTS, "LSRQ timer expired for nbr %R on %s", n->rid, n->ifa->ifname);

  if ((n->state >= NEIGHBOR_EXCHANGE) && !EMPTY_SLIST(n->lsrql))
    ospf_send_lsreq(p, n);
}

static void
lsrt_timer_hook(timer *t)
{
  struct ospf_neighbor *n = t->data;
  struct ospf_proto *p = n->ifa->oa->po;

  // OSPF_TRACE(D_EVENTS, "LSRT timer expired for nbr %R on %s", n->rid, n->ifa->ifname);

  if ((n->state >= NEIGHBOR_EXCHANGE) && !EMPTY_SLIST(n->lsrtl))
    ospf_rxmt_lsupd(p, n);
}

static void
ackd_timer_hook(timer *t)
{
  struct ospf_neighbor *n = t->data;
  struct ospf_proto *p = n->ifa->oa->po;

  ospf_send_lsack(p, n, ACKL_DELAY);
}


void
ospf_sh_neigh_info(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  char *pos = "PtP  ";
  char etime[6];
  int exp, sec, min;

  exp = n->inactim->expires - now;
  sec = exp % 60;
  min = exp / 60;
  if (min > 59)
  {
    bsprintf(etime, "-Inf-");
  }
  else
  {
    bsprintf(etime, "%02u:%02u", min, sec);
  }

  if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_NBMA))
  {
    if (n->rid == ifa->drid)
      pos = "DR   ";
    else if (n->rid == ifa->bdrid)
      pos = "BDR  ";
    else
      pos = "Other";
  }

  cli_msg(-1013, "%-1R\t%3u\t%s/%s\t%-5s\t%-10s %-1I", n->rid, n->priority,
	  ospf_ns_names[n->state], pos, etime, ifa->ifname, n->ip);
}
