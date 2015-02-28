/*
 *	BIRD -- OSPF
 *
 *	(c) 1999 - 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

char *ospf_ns[] = { "    down",
  " attempt",
  "    init",
  "    2way",
  " exstart",
  "exchange",
  " loading",
  "    full"
};

const char *ospf_inm[] =
  { "hello received", "neighbor start", "2-way received",
  "negotiation done", "exstart done", "bad ls request", "load done",
  "adjacency ok?", "sequence mismatch", "1-way received", "kill neighbor",
  "inactivity timer", "line down"
};

static void neigh_chstate(struct ospf_neighbor *n, u8 state);
static struct ospf_neighbor *electbdr(list nl);
static struct ospf_neighbor *electdr(list nl);
static void neighbor_timer_hook(timer * timer);
static void rxmt_timer_hook(timer * timer);
static void ackd_timer_hook(timer * t);

static void
init_lists(struct ospf_neighbor *n)
{
  s_init_list(&(n->lsrql));
  n->lsrqh = ospf_top_new(n->pool);
  s_init(&(n->lsrqi), &(n->lsrql));

  s_init_list(&(n->lsrtl));
  n->lsrth = ospf_top_new(n->pool);
  s_init(&(n->lsrti), &(n->lsrtl));
}

/* Resets LSA request and retransmit lists.
 * We do not reset DB summary list iterator here, 
 * it is reset during entering EXCHANGE state.
 */
static void
reset_lists(struct ospf_neighbor *n)
{
  ospf_top_free(n->lsrqh);
  ospf_top_free(n->lsrth);
  init_lists(n);
}

struct ospf_neighbor *
ospf_neighbor_new(struct ospf_iface *ifa)
{
  struct proto *p = (struct proto *) (ifa->oa->po);
  struct proto_ospf *po = ifa->oa->po;
  struct pool *pool = rp_new(p->pool, "OSPF Neighbor");
  struct ospf_neighbor *n = mb_allocz(pool, sizeof(struct ospf_neighbor));

  n->pool = pool;
  n->ifa = ifa;
  add_tail(&ifa->neigh_list, NODE n);
  n->adj = 0;
  n->csn = 0;
  n->state = NEIGHBOR_DOWN;

  init_lists(n);
  s_init(&(n->dbsi), &(po->lsal));

  n->inactim = tm_new(pool);
  n->inactim->data = n;
  n->inactim->randomize = 0;
  n->inactim->hook = neighbor_timer_hook;
  n->inactim->recurrent = 0;
  DBG("%s: Installing inactivity timer.\n", p->name);

  n->rxmt_timer = tm_new(pool);
  n->rxmt_timer->data = n;
  n->rxmt_timer->randomize = 0;
  n->rxmt_timer->hook = rxmt_timer_hook;
  n->rxmt_timer->recurrent = ifa->rxmtint;
  tm_start(n->rxmt_timer, n->ifa->rxmtint);
  DBG("%s: Installing rxmt timer.\n", p->name);

  n->ackd_timer = tm_new(pool);
  n->ackd_timer->data = n;
  n->ackd_timer->randomize = 0;
  n->ackd_timer->hook = ackd_timer_hook;
  n->ackd_timer->recurrent = ifa->rxmtint / 2;
  init_list(&n->ackl[ACKL_DIRECT]);
  init_list(&n->ackl[ACKL_DELAY]);
  tm_start(n->ackd_timer, n->ifa->rxmtint / 2);
  DBG("%s: Installing ackd timer.\n", p->name);

  return (n);
}

/**
 * neigh_chstate - handles changes related to new or lod state of neighbor
 * @n: OSPF neighbor
 * @state: new state
 *
 * Many actions have to be taken acording to a change of state of a neighbor. It
 * starts rxmt timers, call interface state machine etc.
 */

static void
neigh_chstate(struct ospf_neighbor *n, u8 state)
{
  u8 oldstate;

  oldstate = n->state;

  if (oldstate != state)
  {
    struct ospf_iface *ifa = n->ifa;
    struct proto_ospf *po = ifa->oa->po;
    struct proto *p = &po->proto;

    n->state = state;

    OSPF_TRACE(D_EVENTS, "Neighbor %I changes state from \"%s\" to \"%s\".",
	       n->ip, ospf_ns[oldstate], ospf_ns[state]);

    if ((state == NEIGHBOR_2WAY) && (oldstate < NEIGHBOR_2WAY))
      ospf_iface_sm(ifa, ISM_NEICH);
    if ((state < NEIGHBOR_2WAY) && (oldstate >= NEIGHBOR_2WAY))
      ospf_iface_sm(ifa, ISM_NEICH);

    if (oldstate == NEIGHBOR_FULL)	/* Decrease number of adjacencies */
    {
      ifa->fadj--;
      schedule_rt_lsa(ifa->oa);
      if (ifa->type == OSPF_IT_VLINK) schedule_rt_lsa(ifa->voa);
      schedule_net_lsa(ifa);
    }

    if (state == NEIGHBOR_FULL)	/* Increase number of adjacencies */
    {
      ifa->fadj++;
      schedule_rt_lsa(ifa->oa);
      if (ifa->type == OSPF_IT_VLINK) schedule_rt_lsa(ifa->voa);
      schedule_net_lsa(ifa);
    }
    if (state == NEIGHBOR_EXSTART)
    {
      if (n->adj == 0)		/* First time adjacency */
      {
	n->dds = random_u32();
      }
      n->dds++;
      n->myimms.byte = 0;
      n->myimms.bit.ms = 1;
      n->myimms.bit.m = 1;
      n->myimms.bit.i = 1;
    }
    if (state > NEIGHBOR_EXSTART)
      n->myimms.bit.i = 0;
  }
}

static struct ospf_neighbor *
electbdr(list nl)
{
  struct ospf_neighbor *neigh, *n1, *n2;
  u32 nid;

  n1 = NULL;
  n2 = NULL;
  WALK_LIST(neigh, nl)			/* First try those decl. themselves */
  {
#ifdef OSPFv2
    nid = ipa_to_u32(neigh->ip);
#else /* OSPFv3 */
    nid = neigh->rid;
#endif

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
electdr(list nl)
{
  struct ospf_neighbor *neigh, *n;
  u32 nid;

  n = NULL;
  WALK_LIST(neigh, nl)			/* And now DR */
  {
#ifdef OSPFv2
    nid = ipa_to_u32(neigh->ip);
#else /* OSPFv3 */
    nid = neigh->rid;
#endif

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

static int
can_do_adj(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa;
  struct proto *p;
  int i;

  ifa = n->ifa;
  p = (struct proto *) (ifa->oa->po);
  i = 0;

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
      bug("%s: Iface %s in down state?", p->name, ifa->ifname);
      break;
    case OSPF_IS_WAITING:
      DBG("%s: Neighbor? on iface %s\n", p->name, ifa->ifname);
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
      bug("%s: Iface %s in unknown state?", p->name, ifa->ifname);
      break;
    }
    break;
  default:
    bug("%s: Iface %s is unknown type?", p->name, ifa->ifname);
    break;
  }
  DBG("%s: Iface %s can_do_adj=%d\n", p->name, ifa->ifname, i);
  return i;
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
  struct proto_ospf *po = n->ifa->oa->po;
  struct proto *p = &po->proto;

  DBG("Neighbor state machine for neighbor %I, event '%s'\n", n->ip,
	     ospf_inm[event]);

  switch (event)
  {
  case INM_START:
    neigh_chstate(n, NEIGHBOR_ATTEMPT);
    /* NBMA are used different way */
    break;
  case INM_HELLOREC:
    switch (n->state)
    {
    case NEIGHBOR_ATTEMPT:
    case NEIGHBOR_DOWN:
      neigh_chstate(n, NEIGHBOR_INIT);
    default:
      tm_start(n->inactim, n->ifa->deadint);	/* Restart inactivity timer */
      break;
    }
    break;
  case INM_2WAYREC:
    if (n->state < NEIGHBOR_2WAY)
      neigh_chstate(n, NEIGHBOR_2WAY);
    if ((n->state == NEIGHBOR_2WAY) && can_do_adj(n))
      neigh_chstate(n, NEIGHBOR_EXSTART);
    break;
  case INM_NEGDONE:
    if (n->state == NEIGHBOR_EXSTART)
    {
      neigh_chstate(n, NEIGHBOR_EXCHANGE);

      /* Reset DB summary list iterator */
      s_get(&(n->dbsi));
      s_init(&(n->dbsi), &po->lsal);

      while (!EMPTY_LIST(n->ackl[ACKL_DELAY]))
      {
	struct lsah_n *no;
	no = (struct lsah_n *) HEAD(n->ackl[ACKL_DELAY]);
	rem_node(NODE no);
	mb_free(no);
      }
    }
    else
      bug("NEGDONE and I'm not in EXSTART?");
    break;
  case INM_EXDONE:
    neigh_chstate(n, NEIGHBOR_LOADING);
    break;
  case INM_LOADDONE:
    neigh_chstate(n, NEIGHBOR_FULL);
    break;
  case INM_ADJOK:
    switch (n->state)
    {
    case NEIGHBOR_2WAY:
      /* Can In build adjacency? */
      if (can_do_adj(n))
      {
	neigh_chstate(n, NEIGHBOR_EXSTART);
      }
      break;
    default:
      if (n->state >= NEIGHBOR_EXSTART)
	if (!can_do_adj(n))
	{
	  reset_lists(n);
	  neigh_chstate(n, NEIGHBOR_2WAY);
	}
      break;
    }
    break;
  case INM_SEQMIS:
  case INM_BADLSREQ:
    if (n->state >= NEIGHBOR_EXCHANGE)
    {
      reset_lists(n);
      neigh_chstate(n, NEIGHBOR_EXSTART);
    }
    break;
  case INM_KILLNBR:
  case INM_LLDOWN:
  case INM_INACTTIM:
    reset_lists(n);
    neigh_chstate(n, NEIGHBOR_DOWN);
    break;
  case INM_1WAYREC:
    reset_lists(n);
    neigh_chstate(n, NEIGHBOR_INIT);
    break;
  default:
    bug("%s: INM - Unknown event?", p->name);
    break;
  }
}

/**
 * bdr_election - (Backup) Designed Router election
 * @ifa: actual interface
 *
 * When the wait timer fires, it is time to elect (Backup) Designated Router.
 * Structure describing me is added to this list so every electing router
 * has the same list. Backup Designated Router is elected before Designated
 * Router. This process is described in 9.4 of RFC 2328.
 */
void
bdr_election(struct ospf_iface *ifa)
{
  struct proto_ospf *po = ifa->oa->po;
  u32 myid = po->router_id;
  struct ospf_neighbor *neigh, *ndr, *nbdr, me;
  int doadj;

  DBG("(B)DR election.\n");

  me.state = NEIGHBOR_2WAY;
  me.rid = myid;
  me.priority = ifa->priority;
  me.ip = ifa->addr->ip;

#ifdef OSPFv2
  me.dr = ipa_to_u32(ifa->drip);
  me.bdr = ipa_to_u32(ifa->bdrip);
#else /* OSPFv3 */
  me.dr = ifa->drid;
  me.bdr = ifa->bdrid;
  me.iface_id = ifa->iface_id;
#endif

  add_tail(&ifa->neigh_list, NODE & me);

  nbdr = electbdr(ifa->neigh_list);
  ndr = electdr(ifa->neigh_list);

  if (ndr == NULL)
    ndr = nbdr;

  /* 9.4. (4) */
  if (((ifa->drid == myid) && (ndr != &me))
      || ((ifa->drid != myid) && (ndr == &me))
      || ((ifa->bdrid == myid) && (nbdr != &me))
      || ((ifa->bdrid != myid) && (nbdr == &me)))
  {
#ifdef OSPFv2
    me.dr = ndr ? ipa_to_u32(ndr->ip) : 0;
    me.bdr = nbdr ? ipa_to_u32(nbdr->ip) : 0;
#else /* OSPFv3 */
    me.dr = ndr ? ndr->rid : 0;
    me.bdr = nbdr ? nbdr->rid : 0;
#endif

    nbdr = electbdr(ifa->neigh_list);
    ndr = electdr(ifa->neigh_list);

    if (ndr == NULL)
      ndr = nbdr;
  }

  u32 odrid = ifa->drid;
  u32 obdrid = ifa->bdrid;
 
  ifa->drid = ndr ? ndr->rid : 0;
  ifa->drip = ndr ? ndr->ip  : IPA_NONE;
  ifa->bdrid = nbdr ? nbdr->rid : 0;
  ifa->bdrip = nbdr ? nbdr->ip  : IPA_NONE;

#ifdef OSPFv3
  ifa->dr_iface_id = ndr ? ndr->iface_id : 0;
#endif

  DBG("DR=%R, BDR=%R\n", ifa->drid, ifa->bdrid);

  doadj = ((ifa->drid != odrid) || (ifa->bdrid != obdrid));

  if (myid == ifa->drid)
    ospf_iface_chstate(ifa, OSPF_IS_DR);
  else
  {
    if (myid == ifa->bdrid)
      ospf_iface_chstate(ifa, OSPF_IS_BACKUP);
    else
      ospf_iface_chstate(ifa, OSPF_IS_DROTHER);
  }

  rem_node(NODE & me);

  if (doadj)
  {
    WALK_LIST(neigh, ifa->neigh_list)
    {
      ospf_neigh_sm(neigh, INM_ADJOK);
    }
  }
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

/* Neighbor is inactive for a long time. Remove it. */
static void
neighbor_timer_hook(timer * timer)
{
  struct ospf_neighbor *n = (struct ospf_neighbor *) timer->data;
  struct ospf_iface *ifa = n->ifa;
  struct proto *p = &ifa->oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Inactivity timer fired on interface %s for neighbor %I.",
	     ifa->ifname, n->ip);
  ospf_neigh_remove(n);
}

void
ospf_neigh_remove(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  struct proto *p = &ifa->oa->po->proto;

  if ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_PTMP))
  {
    struct nbma_node *nn = find_nbma_node(ifa, n->ip);
    if (nn)
      nn->found = 0;
  }

  s_get(&(n->dbsi));
  neigh_chstate(n, NEIGHBOR_DOWN);
  rem_node(NODE n);
  rfree(n->pool);
  OSPF_TRACE(D_EVENTS, "Deleting neigbor.");
}

static void
ospf_neigh_bfd_hook(struct bfd_request *req)
{
  struct ospf_neighbor *n = req->data;
  struct proto *p = &n->ifa->oa->po->proto;

  if (req->down)
  {
    OSPF_TRACE(D_EVENTS, "BFD session down for %I on %s",
	       n->ip, n->ifa->ifname);

    ospf_neigh_remove(n);
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


void
ospf_sh_neigh_info(struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  char *pos = "other";
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

  if (n->rid == ifa->drid)
    pos = "dr   ";
  else if (n->rid == ifa->bdrid)
    pos = "bdr  ";
  else if ((n->ifa->type == OSPF_IT_PTP) || (n->ifa->type == OSPF_IT_PTMP) ||
	   (n->ifa->type == OSPF_IT_VLINK))
    pos = "ptp  ";

  cli_msg(-1013, "%-1R\t%3u\t%s/%s\t%-5s\t%-10s %-1I", n->rid, n->priority,
	  ospf_ns[n->state], pos, etime, ifa->ifname, n->ip);
}

static void
rxmt_timer_hook(timer * timer)
{
  struct ospf_neighbor *n = (struct ospf_neighbor *) timer->data;
  // struct proto *p = &n->ifa->oa->po->proto;
  struct top_hash_entry *en;

  DBG("%s: RXMT timer fired on interface %s for neigh: %I.\n",
      p->name, n->ifa->ifname, n->ip);

  if(n->state < NEIGHBOR_EXSTART) return;

  if (n->state == NEIGHBOR_EXSTART)
  {
    ospf_dbdes_send(n, 1);
    return;
  }

  if ((n->state == NEIGHBOR_EXCHANGE) && n->myimms.bit.ms)	/* I'm master */
    ospf_dbdes_send(n, 0);


  if (n->state < NEIGHBOR_FULL)	
    ospf_lsreq_send(n);	/* EXCHANGE or LOADING */
  else
  {
    if (!EMPTY_SLIST(n->lsrtl))	/* FULL */
    {
      list uplist;
      slab *upslab;
      struct l_lsr_head *llsh;

      init_list(&uplist);
      upslab = sl_new(n->pool, sizeof(struct l_lsr_head));

      WALK_SLIST(en, n->lsrtl)
      {
	if ((SNODE en)->next == (SNODE en))
	  bug("RTList is cycled");
	llsh = sl_alloc(upslab);
	llsh->lsh.id = en->lsa.id;
	llsh->lsh.rt = en->lsa.rt;
	llsh->lsh.type = en->lsa.type;
	DBG("Working on ID: %R, RT: %R, Type: %u\n",
	    en->lsa.id, en->lsa.rt, en->lsa.type);
	add_tail(&uplist, NODE llsh);
      }
      ospf_lsupd_send_list(n, &uplist);
      rfree(upslab);
    }
  }
}

static void
ackd_timer_hook(timer * t)
{
  struct ospf_neighbor *n = t->data;
  ospf_lsack_send(n, ACKL_DELAY);
}
