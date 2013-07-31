/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2005 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

char *ospf_is[] = { "down", "loop", "waiting", "ptp", "drother",
  "backup", "dr"
};

char *ospf_ism[] = { "interface up", "wait timer fired", "backup seen",
  "neighbor change", "loop indicated", "unloop indicated", "interface down"
};

char *ospf_it[] = { "broadcast", "nbma", "ptp", "ptmp", "virtual link" };

static void
poll_timer_hook(timer * timer)
{
  ospf_hello_send(timer->data, OHS_POLL, NULL);
}

static void
hello_timer_hook(timer * timer)
{
  ospf_hello_send(timer->data, OHS_HELLO, NULL);
}

static void
wait_timer_hook(timer * timer)
{
  struct ospf_iface *ifa = (struct ospf_iface *) timer->data;
  struct proto *p = &ifa->oa->po->proto;

  OSPF_TRACE(D_EVENTS, "Wait timer fired on interface %s.", ifa->iface->name);
  ospf_iface_sm(ifa, ISM_WAITF);
}

static void ospf_iface_change_mtu(struct proto_ospf *po, struct ospf_iface *ifa);

u32
rxbufsize(struct ospf_iface *ifa)
{
  switch(ifa->rxbuf)
  {
    case OSPF_RXBUF_NORMAL:
      return (ifa->iface->mtu * 2);
      break;
    case OSPF_RXBUF_LARGE:
      return OSPF_MAX_PKT_SIZE;
      break;
    default:
      return ifa->rxbuf;
      break;
  }
}

struct nbma_node *
find_nbma_node_in(list *nnl, ip_addr ip)
{
  struct nbma_node *nn;
  WALK_LIST(nn, *nnl)
    if (ipa_equal(nn->ip, ip))
      return nn;
  return NULL;
}

static int
ospf_sk_open(struct ospf_iface *ifa)
{
  sock *sk = sk_new(ifa->pool);
  sk->type = SK_IP;
  sk->dport = OSPF_PROTO;
  sk->saddr = IPA_NONE;

  sk->tos = ifa->cf->tx_tos;
  sk->priority = ifa->cf->tx_priority;
  sk->rx_hook = ospf_rx_hook;
  sk->tx_hook = ospf_tx_hook;
  sk->err_hook = ospf_err_hook;
  sk->iface = ifa->iface;
  sk->rbsize = rxbufsize(ifa);
  sk->tbsize = rxbufsize(ifa);
  sk->data = (void *) ifa;
  sk->flags = SKF_LADDR_RX | (ifa->check_ttl ? SKF_TTL_RX : 0);
  sk->ttl = ifa->cf->ttl_security ? 255 : -1;

  if (sk_open(sk) != 0)
    goto err;

#ifdef OSPFv3
  /* 12 is an offset of the checksum in an OSPF packet */
  if (sk_set_ipv6_checksum(sk, 12) < 0)
    goto err;
#endif

  /*
   * For OSPFv2: When sending a packet, it is important to have a
   * proper source address. We expect that when we send one-hop
   * unicast packets, OS chooses a source address according to the
   * destination address (to be in the same prefix). We also expect
   * that when we send multicast packets, OS uses the source address
   * from sk->saddr registered to OS by sk_setup_multicast(). This
   * behavior is needed to implement multiple virtual ifaces (struct
   * ospf_iface) on one physical iface and is signalized by
   * CONFIG_MC_PROPER_SRC.
   *
   * If this behavior is not available (for example on BSD), we create
   * non-stub iface just for the primary IP address (see
   * ospf_iface_stubby()) and we expect OS to use primary IP address
   * as a source address for both unicast and multicast packets.
   *
   * FIXME: the primary IP address is currently just the
   * lexicographically smallest address on an interface, it should be
   * signalized by sysdep code which one is really the primary.
   */

  sk->saddr = ifa->addr->ip;
  if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_PTP))
  {
    if (ifa->cf->real_bcast)
    {
      ifa->all_routers = ifa->addr->brd;

      if (sk_set_broadcast(sk, 1) < 0)
        goto err;
    }
    else
    {
      ifa->all_routers = AllSPFRouters;
      sk->ttl = ifa->cf->ttl_security ? 255 : 1;

      if (sk_setup_multicast(sk) < 0)
        goto err;

      if (sk_join_group(sk, ifa->all_routers) < 0)
        goto err;
    }
  }

  ifa->sk = sk;
  ifa->sk_dr = 0;
  return 1;

 err:
  rfree(sk);
  return 0;
}

static inline void
ospf_sk_join_dr(struct ospf_iface *ifa)
{
  if (ifa->sk_dr)
    return;

  sk_join_group(ifa->sk, AllDRouters);
  ifa->sk_dr = 1;
}

static inline void
ospf_sk_leave_dr(struct ospf_iface *ifa)
{
  if (!ifa->sk_dr)
    return;

  sk_leave_group(ifa->sk, AllDRouters);
  ifa->sk_dr = 0;
}

static void
ospf_iface_down(struct ospf_iface *ifa)
{
  struct ospf_neighbor *n, *nx;
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  struct ospf_iface *iff;

  if (ifa->type != OSPF_IT_VLINK)
  {
#ifdef OSPFv2
    OSPF_TRACE(D_EVENTS, "Removing interface %s (%I/%d) from area %R",
	       ifa->iface->name, ifa->addr->prefix, ifa->addr->pxlen, ifa->oa->areaid);
#else
    OSPF_TRACE(D_EVENTS, "Removing interface %s (IID %d) from area %R",
	       ifa->iface->name, ifa->instance_id, ifa->oa->areaid);
#endif

    /* First of all kill all the related vlinks */
    WALK_LIST(iff, po->iface_list)
    {
      if ((iff->type == OSPF_IT_VLINK) && (iff->vifa == ifa))
	ospf_iface_sm(iff, ISM_DOWN);
    }
  }

  WALK_LIST_DELSAFE(n, nx, ifa->neigh_list)
  {
    OSPF_TRACE(D_EVENTS, "Removing neighbor %I", n->ip);
    ospf_neigh_remove(n);
  }

  if (ifa->hello_timer)
    tm_stop(ifa->hello_timer);

  if (ifa->poll_timer)
    tm_stop(ifa->poll_timer);

  if (ifa->wait_timer)
    tm_stop(ifa->wait_timer);

  if (ifa->type == OSPF_IT_VLINK)
  {
    ifa->vifa = NULL;
    ifa->iface = NULL;
    ifa->addr = NULL;
    ifa->sk = NULL;
    ifa->cost = 0;
    ifa->vip = IPA_NONE;
  }

  ifa->rt_pos_beg = 0;
  ifa->rt_pos_end = 0;
#ifdef OSPFv3
  ifa->px_pos_beg = 0;
  ifa->px_pos_end = 0;
#endif
}


void
ospf_iface_remove(struct ospf_iface *ifa)
{
  struct proto *p = &ifa->oa->po->proto;
  if (ifa->type == OSPF_IT_VLINK)
    OSPF_TRACE(D_EVENTS, "Removing vlink to %R via area %R", ifa->vid, ifa->voa->areaid);

  ospf_iface_sm(ifa, ISM_DOWN);
  rem_node(NODE ifa);
  rfree(ifa->pool);
}

void
ospf_iface_shutdown(struct ospf_iface *ifa)
{
  if (ifa->state > OSPF_IS_DOWN)
    ospf_hello_send(ifa, OHS_SHUTDOWN, NULL);
}

/**
 * ospf_iface_chstate - handle changes of interface state
 * @ifa: OSPF interface
 * @state: new state
 *
 * Many actions must be taken according to interface state changes. New network
 * LSAs must be originated, flushed, new multicast sockets to listen for messages for
 * %ALLDROUTERS have to be opened, etc.
 */
void
ospf_iface_chstate(struct ospf_iface *ifa, u8 state)
{
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  u8 oldstate = ifa->state;

  if (oldstate == state)
    return;

  ifa->state = state;

  if (ifa->type == OSPF_IT_VLINK)
    OSPF_TRACE(D_EVENTS, "Changing state of virtual link %R from %s to %s",
	       ifa->vid, ospf_is[oldstate], ospf_is[state]);
  else
    OSPF_TRACE(D_EVENTS, "Changing state of iface %s from %s to %s",
	       ifa->iface->name, ospf_is[oldstate], ospf_is[state]);

  if ((ifa->type == OSPF_IT_BCAST) && !ifa->cf->real_bcast && ifa->sk)
  {
    if ((state == OSPF_IS_BACKUP) || (state == OSPF_IS_DR))
      ospf_sk_join_dr(ifa);
    else
      ospf_sk_leave_dr(ifa);
  }

  if ((oldstate == OSPF_IS_DR) && (ifa->net_lsa != NULL))
  {
    ifa->net_lsa->lsa.age = LSA_MAXAGE;
    if (state >= OSPF_IS_WAITING)
      ospf_lsupd_flush_nlsa(po, ifa->net_lsa);

    if (can_flush_lsa(po))
      flush_lsa(ifa->net_lsa, po);
    ifa->net_lsa = NULL;
  }

  if ((oldstate > OSPF_IS_LOOP) && (state <= OSPF_IS_LOOP))
    ospf_iface_down(ifa);

  schedule_rt_lsa(ifa->oa);
  // FIXME flushling of link LSA
}

/**
 * ospf_iface_sm - OSPF interface state machine
 * @ifa: OSPF interface
 * @event: event comming to state machine
 *
 * This fully respects 9.3 of RFC 2328 except we have slightly
 * different handling of %DOWN and %LOOP state. We remove intefaces
 * that are %DOWN. %DOWN state is used when an interface is waiting
 * for a lock. %LOOP state is used when an interface does not have a
 * link.
 */
void
ospf_iface_sm(struct ospf_iface *ifa, int event)
{
  DBG("SM on %s %s. Event is '%s'\n", (ifa->type == OSPF_IT_VLINK) ? "vlink" : "iface",
    ifa->iface ? ifa->iface->name : "(none)" , ospf_ism[event]);

  switch (event)
  {
  case ISM_UP:
    if (ifa->state <= OSPF_IS_LOOP)
    {
      /* Now, nothing should be adjacent */
      if ((ifa->type == OSPF_IT_PTP) || (ifa->type == OSPF_IT_PTMP) || (ifa->type == OSPF_IT_VLINK))
      {
	ospf_iface_chstate(ifa, OSPF_IS_PTP);
      }
      else
      {
	if (ifa->priority == 0)
	  ospf_iface_chstate(ifa, OSPF_IS_DROTHER);
	else
	{
	  ospf_iface_chstate(ifa, OSPF_IS_WAITING);
	  if (ifa->wait_timer)
	    tm_start(ifa->wait_timer, ifa->waitint);
	}
      }

      if (ifa->hello_timer)
	tm_start(ifa->hello_timer, ifa->helloint);

      if (ifa->poll_timer)
	tm_start(ifa->poll_timer, ifa->pollint);

      ospf_hello_send(ifa, OHS_HELLO, NULL);
      schedule_link_lsa(ifa);
    }
    break;

  case ISM_BACKS:
  case ISM_WAITF:
    if (ifa->state == OSPF_IS_WAITING)
    {
      bdr_election(ifa);
    }
    break;

  case ISM_NEICH:
    if ((ifa->state == OSPF_IS_DROTHER) || (ifa->state == OSPF_IS_DR) ||
	(ifa->state == OSPF_IS_BACKUP))
    {
      bdr_election(ifa);
      schedule_rt_lsa(ifa->oa);
    }
    break;

  case ISM_LOOP:
    if ((ifa->state > OSPF_IS_LOOP) && ifa->check_link)
      ospf_iface_chstate(ifa, OSPF_IS_LOOP);
    break;

  case ISM_UNLOOP:
    /* Immediate go UP */
    if (ifa->state == OSPF_IS_LOOP)
      ospf_iface_sm(ifa, ISM_UP);
    break;

  case ISM_DOWN:
    ospf_iface_chstate(ifa, OSPF_IS_DOWN);
    break;

  default:
    bug("OSPF_I_SM - Unknown event?");
    break;
  }

}

static u8
ospf_iface_classify_int(struct iface *ifa, struct ifa *addr)
{
  if (ipa_nonzero(addr->opposite))
    return (ifa->flags & IF_MULTICAST) ? OSPF_IT_PTP :  OSPF_IT_PTMP;

  if ((ifa->flags & (IF_MULTIACCESS | IF_MULTICAST)) ==
      (IF_MULTIACCESS | IF_MULTICAST))
    return OSPF_IT_BCAST;

  if ((ifa->flags & (IF_MULTIACCESS | IF_MULTICAST)) == IF_MULTIACCESS)
    return OSPF_IT_NBMA;

  return OSPF_IT_PTP;
}

static inline u8
ospf_iface_classify(u8 type, struct ifa *addr)
{
  return (type != OSPF_IT_UNDEF) ? type : ospf_iface_classify_int(addr->iface, addr);
}


struct ospf_iface *
ospf_iface_find(struct proto_ospf *p, struct iface *what)
{
  struct ospf_iface *i;

  WALK_LIST(i, p->iface_list) if ((i->iface == what) && (i->type != OSPF_IT_VLINK))
    return i;
  return NULL;
}

static void
ospf_iface_add(struct object_lock *lock)
{
  struct ospf_iface *ifa = lock->data;
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;

  /* Open socket if interface is not stub */
  if (! ifa->stub && ! ospf_sk_open(ifa))
  {
    log(L_ERR "%s: Socket open failed on interface %s, declaring as stub", p->name, ifa->iface->name);
    ifa->ioprob = OSPF_I_SK;
    ifa->stub = 1;
  }

  if (! ifa->stub)
  {
    ifa->hello_timer = tm_new_set(ifa->pool, hello_timer_hook, ifa, 0, ifa->helloint);

    if (ifa->type == OSPF_IT_NBMA)
      ifa->poll_timer = tm_new_set(ifa->pool, poll_timer_hook, ifa, 0, ifa->pollint);

    if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_NBMA))
      ifa->wait_timer = tm_new_set(ifa->pool, wait_timer_hook, ifa, 0, 0);
  }

  /* Do iface UP, unless there is no link and we use link detection */
  ospf_iface_sm(ifa, (ifa->check_link && !(ifa->iface->flags & IF_LINK_UP)) ? ISM_LOOP : ISM_UP);
}

static inline void
add_nbma_node(struct ospf_iface *ifa, struct nbma_node *src, int found)
{
  struct nbma_node *n = mb_alloc(ifa->pool, sizeof(struct nbma_node));
  add_tail(&ifa->nbma_list, NODE n);
  n->ip = src->ip;
  n->eligible = src->eligible;
  n->found = found;
}

static int
ospf_iface_stubby(struct ospf_iface_patt *ip, struct ifa *addr)
{
  if (! addr)
    return 0;

  /* a host/loopback address */
  if (addr->flags & IA_HOST)
    return 1;

  /*
   * We cannot properly support multiple OSPF ifaces on real iface
   * with multiple prefixes, therefore we force OSPF ifaces with
   * non-primary IP prefixes to be stub.
   */
#if defined(OSPFv2) && !defined(CONFIG_MC_PROPER_SRC)
  if (! (addr->flags & IA_PRIMARY))
    return 1;
#endif

  return ip->stub;
}

void
ospf_iface_new(struct ospf_area *oa, struct ifa *addr, struct ospf_iface_patt *ip)
{
  struct proto *p = &oa->po->proto;
  struct iface *iface = addr ? addr->iface : NULL;
  struct pool *pool;

  struct ospf_iface *ifa;
  struct nbma_node *nb;
  struct object_lock *lock;

  if (ip->type == OSPF_IT_VLINK)
    OSPF_TRACE(D_EVENTS, "Adding vlink to %R via area %R", ip->vid, ip->voa);
  else
  {
#ifdef OSPFv2
    OSPF_TRACE(D_EVENTS, "Adding interface %s (%I/%d) to area %R",
	       iface->name, addr->prefix, addr->pxlen, oa->areaid);
#else
    OSPF_TRACE(D_EVENTS, "Adding interface %s (IID %d) to area %R",
	       iface->name, ip->instance_id, oa->areaid);
#endif
  }

  pool = rp_new(p->pool, "OSPF Interface");
  ifa = mb_allocz(pool, sizeof(struct ospf_iface));
  ifa->iface = iface;
  ifa->addr = addr;
  ifa->oa = oa;
  ifa->cf = ip;
  ifa->pool = pool;

  ifa->cost = ip->cost;
  ifa->rxmtint = ip->rxmtint;
  ifa->inftransdelay = ip->inftransdelay;
  ifa->priority = ip->priority;
  ifa->helloint = ip->helloint;
  ifa->pollint = ip->pollint;
  ifa->strictnbma = ip->strictnbma;
  ifa->waitint = ip->waitint;
  ifa->deadint = ip->deadint;
  ifa->stub = ospf_iface_stubby(ip, addr);
  ifa->ioprob = OSPF_I_OK;
  ifa->rxbuf = ip->rxbuf;
  ifa->check_link = ip->check_link;
  ifa->ecmp_weight = ip->ecmp_weight;
  ifa->check_ttl = (ip->ttl_security == 1);

#ifdef OSPFv2
  ifa->autype = ip->autype;
  ifa->passwords = ip->passwords;
  ifa->ptp_netmask = addr ? !(addr->flags & IA_PEER) : 0;
  if (ip->ptp_netmask < 2)
    ifa->ptp_netmask = ip->ptp_netmask;
#endif

#ifdef OSPFv3
  ifa->instance_id = ip->instance_id;
#endif

  ifa->type = ospf_iface_classify(ip->type, addr);

  /* Check validity of interface type */
  int old_type = ifa->type;
  u32 if_multi_flag = ip->real_bcast ? IF_BROADCAST : IF_MULTICAST;

#ifdef OSPFv2
  if ((ifa->type == OSPF_IT_BCAST) && (addr->flags & IA_PEER))
    ifa->type = OSPF_IT_PTP;

  if ((ifa->type == OSPF_IT_NBMA) && (addr->flags & IA_PEER))
    ifa->type = OSPF_IT_PTMP;
#endif

  if ((ifa->type == OSPF_IT_BCAST) && !(iface->flags & if_multi_flag))
    ifa->type = OSPF_IT_NBMA;

  if ((ifa->type == OSPF_IT_PTP) && !(iface->flags & if_multi_flag))
    ifa->type = OSPF_IT_PTMP;

  if (ifa->type != old_type)
    log(L_WARN "%s: Cannot use interface %s as %s, forcing %s",
	p->name, iface->name, ospf_it[old_type], ospf_it[ifa->type]);

  /* Assign iface ID, for vlinks, this is ugly hack */
  ifa->iface_id = (ifa->type != OSPF_IT_VLINK) ? iface->index : oa->po->last_vlink_id++;

  init_list(&ifa->neigh_list);
  init_list(&ifa->nbma_list);

  WALK_LIST(nb, ip->nbma_list)
  {
    /* In OSPFv3, addr is link-local while configured neighbors could
       have global IP (although RFC 5340 C.5 says link-local addresses
       should be used). Because OSPFv3 iface is not subnet-specific,
       there is no need for ipa_in_net() check */

#ifdef OSPFv2
    if (!ipa_in_net(nb->ip, addr->prefix, addr->pxlen))
      continue;
#else
    if (!ipa_has_link_scope(nb->ip))
      log(L_WARN "In OSPFv3, configured neighbor address (%I) should be link-local", nb->ip);
#endif

    add_nbma_node(ifa, nb, 0);
  }

  ifa->state = OSPF_IS_DOWN;
  add_tail(&oa->po->iface_list, NODE ifa);

  if (ifa->type == OSPF_IT_VLINK)
  {
    ifa->voa = ospf_find_area(oa->po, ip->voa);
    ifa->vid = ip->vid;

    ifa->hello_timer = tm_new_set(ifa->pool, hello_timer_hook, ifa, 0, ifa->helloint);

    return;			/* Don't lock, don't add sockets */
  }

  /*
   * In some cases we allow more ospf_ifaces on one physical iface.
   * In OSPFv2, if they use different IP address prefix.
   * In OSPFv3, if they use different instance_id.
   * Therefore, we store such info to lock->addr field.
   */

  lock = olock_new(pool);
#ifdef OSPFv2
  lock->addr = ifa->addr->prefix;
#else /* OSPFv3 */
  lock->addr = _MI(0,0,0,ifa->instance_id);
#endif
  lock->type = OBJLOCK_IP;
  lock->port = OSPF_PROTO;
  lock->iface = iface;
  lock->data = ifa;
  lock->hook = ospf_iface_add;

  olock_acquire(lock);
}

static void
ospf_iface_change_timer(timer *tm, unsigned val)
{
  if (!tm)
    return;

  tm->recurrent = val;

  if (tm->expires)
    tm_start(tm, val);
}

int
ospf_iface_reconfigure(struct ospf_iface *ifa, struct ospf_iface_patt *new)
{
  struct proto *p = &ifa->oa->po->proto;
  struct nbma_node *nb, *nbx;
  char *ifname = (ifa->type != OSPF_IT_VLINK) ? ifa->iface->name : "vlink";

  /* Type could be changed in ospf_iface_new(),
     but if config values are same then also results are same */
  int old_type = ospf_iface_classify(ifa->cf->type, ifa->addr);
  int new_type = ospf_iface_classify(new->type, ifa->addr);
  if (old_type != new_type)
    return 0;

  int new_stub = ospf_iface_stubby(new, ifa->addr);
  if (ifa->stub != new_stub)
    return 0;

  /* Change of these options would require to reset the iface socket */
  if ((new->real_bcast != ifa->cf->real_bcast) ||
      (new->tx_tos != ifa->cf->tx_tos) ||
      (new->tx_priority != ifa->cf->tx_priority) ||
      (new->ttl_security != ifa->cf->ttl_security))
    return 0;

  ifa->cf = new;
  ifa->marked = 0;


  /* HELLO TIMER */
  if (ifa->helloint != new->helloint)
  {
    OSPF_TRACE(D_EVENTS, "Changing hello interval on interface %s from %d to %d",
	       ifname, ifa->helloint, new->helloint);

    ifa->helloint = new->helloint;
    ospf_iface_change_timer(ifa->hello_timer, ifa->helloint);
  }

  /* RXMT TIMER */
  if (ifa->rxmtint != new->rxmtint)
  {
    OSPF_TRACE(D_EVENTS, "Changing retransmit interval on interface %s from %d to %d",
	       ifname, ifa->rxmtint, new->rxmtint);

    ifa->rxmtint = new->rxmtint;
  }

  /* POLL TIMER */
  if (ifa->pollint != new->pollint)
  {
    OSPF_TRACE(D_EVENTS, "Changing poll interval on interface %s from %d to %d",
	       ifname, ifa->pollint, new->pollint);

    ifa->pollint = new->pollint;
    ospf_iface_change_timer(ifa->poll_timer, ifa->pollint);
  }

  /* WAIT TIMER */
  if (ifa->waitint != new->waitint)
  {
    OSPF_TRACE(D_EVENTS, "Changing wait interval on interface %s from %d to %d",
	       ifname, ifa->waitint, new->waitint);

    ifa->waitint = new->waitint;
    if (ifa->wait_timer && ifa->wait_timer->expires)
      tm_start(ifa->wait_timer, ifa->waitint);
  }

  /* DEAD TIMER */
  if (ifa->deadint != new->deadint)
  {
    OSPF_TRACE(D_EVENTS, "Changing dead interval on interface %s from %d to %d",
	       ifname, ifa->deadint, new->deadint);
    ifa->deadint = new->deadint;
  }

  /* INFTRANS */
  if (ifa->inftransdelay != new->inftransdelay)
  {
    OSPF_TRACE(D_EVENTS, "Changing transmit delay on interface %s from %d to %d",
		     ifname, ifa->inftransdelay, new->inftransdelay);
    ifa->inftransdelay = new->inftransdelay;
  }

#ifdef OSPFv2	
  /* AUTHENTICATION */
  if (ifa->autype != new->autype)
  {
    OSPF_TRACE(D_EVENTS, "Changing authentication type on interface %s", ifname);
    ifa->autype = new->autype;
  }

  /* Update passwords */
  ifa->passwords = new->passwords;
#endif

  /* Remaining options are just for proper interfaces */
  if (ifa->type == OSPF_IT_VLINK)
    return 1;


  /* COST */
  if (ifa->cost != new->cost)
  {
    OSPF_TRACE(D_EVENTS, "Changing cost on interface %s from %d to %d",
	       ifname, ifa->cost, new->cost);

    ifa->cost = new->cost;
  }

  /* PRIORITY */
  if (ifa->priority != new->priority)
  {
    OSPF_TRACE(D_EVENTS, "Changing priority on interface %s from %d to %d",
	       ifname, ifa->priority, new->priority);
    ifa->priority = new->priority;
  }

  /* STRICT NBMA */
  if (ifa->strictnbma != new->strictnbma)
  {
    OSPF_TRACE(D_EVENTS, "Changing NBMA strictness on interface %s", ifname);
    ifa->strictnbma = new->strictnbma;
  }

  /* NBMA LIST - remove or update old */
  WALK_LIST_DELSAFE(nb, nbx, ifa->nbma_list)
  {
    struct nbma_node *nb2 = find_nbma_node_in(&new->nbma_list, nb->ip);
    if (nb2)
    {
      if (nb->eligible != nb2->eligible)
      {
	OSPF_TRACE(D_EVENTS, "Changing eligibility of neighbor %I on interface %s",
		   nb->ip, ifname);
	nb->eligible = nb2->eligible;
      }
    }
    else
    {
      OSPF_TRACE(D_EVENTS, "Removing NBMA neighbor %I on interface %s",
		       nb->ip, ifname);
      rem_node(NODE nb);
      mb_free(nb);
    }
  }

  /* NBMA LIST - add new */
  WALK_LIST(nb, new->nbma_list)
  {
    /* See related note in ospf_iface_new() */
#ifdef OSPFv2
    if (!ipa_in_net(nb->ip, ifa->addr->prefix, ifa->addr->pxlen))
      continue;
#else
    if (!ipa_has_link_scope(nb->ip))
      log(L_WARN "In OSPFv3, configured neighbor address (%I) should be link-local", nb->ip);
#endif

    if (! find_nbma_node(ifa, nb->ip))
    {
      OSPF_TRACE(D_EVENTS, "Adding NBMA neighbor %I on interface %s",
		 nb->ip, ifname);
      add_nbma_node(ifa, nb, !!find_neigh_by_ip(ifa, nb->ip));
    }
  }

  /* RX BUFF */
  if (ifa->rxbuf != new->rxbuf)
  {
    OSPF_TRACE(D_EVENTS, "Changing rxbuf interface %s from %d to %d",
	       ifname, ifa->rxbuf, new->rxbuf);
    ifa->rxbuf = new->rxbuf;
    ospf_iface_change_mtu(ifa->oa->po, ifa);
  }

  /* LINK */
  if (ifa->check_link != new->check_link)
  {
    OSPF_TRACE(D_EVENTS, "%s link check on interface %s",
	       new->check_link ? "Enabling" : "Disabling", ifname);
    ifa->check_link = new->check_link;

    if (!(ifa->iface->flags & IF_LINK_UP))
      ospf_iface_sm(ifa, ifa->check_link ? ISM_LOOP : ISM_UNLOOP);
  }

  /* ECMP weight */
  if (ifa->ecmp_weight != new->ecmp_weight)
  {
    OSPF_TRACE(D_EVENTS, "Changing ECMP weight of interface %s from %d to %d",
	       ifname, (int)ifa->ecmp_weight + 1, (int)new->ecmp_weight + 1);
    ifa->ecmp_weight = new->ecmp_weight;
  }

  /* instance_id is not updated - it is part of key */

  return 1;
}


#ifdef OSPFv2

static inline struct ospf_iface_patt *
ospf_iface_patt_find(struct ospf_area_config *ac, struct ifa *a)
{
  return (struct ospf_iface_patt *) iface_patt_find(&ac->patt_list, a->iface, a);
}

void
ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (a->flags & IA_SECONDARY)
    return;

  if (a->scope <= SCOPE_LINK)
    return;

  /* In OSPFv2, we create OSPF iface for each address. */
  if (flags & IF_CHANGE_UP)
  {
    int done = 0;
    struct ospf_area *oa;
    WALK_LIST(oa, po->area_list)
    {
      struct ospf_iface_patt *ip;
      if (ip = ospf_iface_patt_find(oa->ac, a))
      {
	if (!done)
	  ospf_iface_new(oa, a, ip);
	done++;
      }
    }

    if (done > 1)
      log(L_WARN "%s: Interface %s (IP %I) matches for multiple areas", p->name,  a->iface->name, a->ip);
  }

  if (flags & IF_CHANGE_DOWN)
  {
    struct ospf_iface *ifa, *ifx;
    WALK_LIST_DELSAFE(ifa, ifx, po->iface_list)
    {
      if ((ifa->type != OSPF_IT_VLINK) && (ifa->addr == a))
	ospf_iface_remove(ifa);
      /* See a note in ospf_iface_notify() */
    }
  }
}

static struct ospf_iface *
ospf_iface_find_by_key(struct ospf_area *oa, struct ifa *a)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, oa->po->iface_list)
    if ((ifa->addr == a) && (ifa->oa == oa) && (ifa->type != OSPF_IT_VLINK))
      return ifa;

  return NULL;
}

void
ospf_ifaces_reconfigure(struct ospf_area *oa, struct ospf_area_config *nac)
{
  struct ospf_iface_patt *ip;
  struct iface *iface;
  struct ifa *a;

  WALK_LIST(iface, iface_list)
  {
    if (! (iface->flags & IF_UP))
      continue;

    WALK_LIST(a, iface->addrs)
    {
      if (a->flags & IA_SECONDARY)
	continue;

      if (a->scope <= SCOPE_LINK)
	continue;

      if (ip = ospf_iface_patt_find(oa->ac, a))
      {
	/* Main inner loop */
	struct ospf_iface *ifa = ospf_iface_find_by_key(oa, a);
	if (ifa)
	{
	  if (ospf_iface_reconfigure(ifa, ip))
	    continue;

	  /* Hard restart */
	  ospf_iface_shutdown(ifa);
	  ospf_iface_remove(ifa);
	}
	
	ospf_iface_new(oa, a, ip);
      }
    }
  }
}


#else /* OSPFv3 */

struct ospf_iface_patt *
ospf_iface_patt_find(struct ospf_area_config *ac, struct iface *iface, int iid)
{
  struct ospf_iface_patt *pt, *res = NULL;

  WALK_LIST(pt, ac->patt_list)
    if ((pt->instance_id >= iid) && (iface_patt_match(&pt->i, iface, NULL)) &&
	(!res || (pt->instance_id < res->instance_id)))
      res = pt;

  return res;
}

void
ospf_ifa_notify(struct proto *p, unsigned flags, struct ifa *a)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

  if (a->flags & IA_SECONDARY)
    return;

  if (a->scope < SCOPE_LINK)
    return;

  /* In OSPFv3, we create OSPF iface for link-local address,
     other addresses are used for link-LSA. */
  if (a->scope == SCOPE_LINK)
  {
    if (flags & IF_CHANGE_UP)
    {
      int done0 = 0;
      struct ospf_area *oa;

      WALK_LIST(oa, po->area_list)
      {
	int iid = 0;

	struct ospf_iface_patt *ip;
	while (ip = ospf_iface_patt_find(oa->ac, a->iface, iid))
	{
	  ospf_iface_new(oa, a, ip);
	  if (ip->instance_id == 0)
	    done0++;
	  iid = ip->instance_id + 1;
	}
      }

      if (done0 > 1)
	log(L_WARN "%s: Interface %s matches for multiple areas",
	    p->name,  a->iface->name);
    }

    if (flags & IF_CHANGE_DOWN)
    {
      struct ospf_iface *ifa, *ifx;
      WALK_LIST_DELSAFE(ifa, ifx, po->iface_list)
      {
	if ((ifa->type != OSPF_IT_VLINK) && (ifa->addr == a))
	  ospf_iface_remove(ifa);
	/* See a note in ospf_iface_notify() */
      }
    }
  }
  else
  {
    struct ospf_iface *ifa;
    WALK_LIST(ifa, po->iface_list)
    {
      if (ifa->iface == a->iface)
      {
	schedule_rt_lsa(ifa->oa);
	/* Event 5 from RFC5340 4.4.3. */
	schedule_link_lsa(ifa);
	return;
      }
    }
  }
}

static struct ospf_iface *
ospf_iface_find_by_key(struct ospf_area *oa, struct ifa *a, int iid)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, oa->po->iface_list)
    if ((ifa->addr == a) && (ifa->oa == oa) && (ifa->instance_id == iid) && (ifa->type != OSPF_IT_VLINK))
      return ifa;

  return NULL;
}

void
ospf_ifaces_reconfigure(struct ospf_area *oa, struct ospf_area_config *nac)
{
  struct ospf_iface_patt *ip;
  struct iface *iface;
  struct ifa *a;

  WALK_LIST(iface, iface_list)
  {
    if (! (iface->flags & IF_UP))
      continue;

    WALK_LIST(a, iface->addrs)
    {
      if (a->flags & IA_SECONDARY)
	continue;

      if (a->scope != SCOPE_LINK)
	continue;

      int iid = 0;
      while (ip = ospf_iface_patt_find(nac, iface, iid))
      {
	iid = ip->instance_id + 1;

	/* Main inner loop */
	struct ospf_iface *ifa = ospf_iface_find_by_key(oa, a, ip->instance_id);
	if (ifa)
	{
	  if (ospf_iface_reconfigure(ifa, ip))
	    continue;

	  /* Hard restart */
	  ospf_iface_shutdown(ifa);
	  ospf_iface_remove(ifa);
	}

	ospf_iface_new(oa, a, ip);
      }
    }
  }
}

#endif

static void
ospf_iface_change_mtu(struct proto_ospf *po, struct ospf_iface *ifa)
{
  struct proto *p = &po->proto;
  struct ospf_packet *op;
  struct ospf_neighbor *n;
  OSPF_TRACE(D_EVENTS, "Changing MTU on interface %s", ifa->iface->name);

  if (ifa->sk)
  {
    ifa->sk->rbsize = rxbufsize(ifa);
    ifa->sk->tbsize = rxbufsize(ifa);
    sk_reallocate(ifa->sk);
  }

  WALK_LIST(n, ifa->neigh_list)
  {
    op = (struct ospf_packet *) n->ldbdes;
    n->ldbdes = mb_allocz(n->pool, ifa->iface->mtu);

    if (ntohs(op->length) <= ifa->iface->mtu)	/* If the packet in old buffer is bigger, let it filled by zeros */
      memcpy(n->ldbdes, op, ifa->iface->mtu);	/* If the packet is old is same or smaller, copy it */

    mb_free(op);
  }
}

static void
ospf_iface_notify(struct proto_ospf *po, unsigned flags, struct ospf_iface *ifa)
{
  if (flags & IF_CHANGE_DOWN)
  {
    ospf_iface_remove(ifa);
    return;
  }

  if (flags & IF_CHANGE_LINK)
    ospf_iface_sm(ifa, (ifa->iface->flags & IF_LINK_UP) ? ISM_UNLOOP : ISM_LOOP);

  if (flags & IF_CHANGE_MTU)
    ospf_iface_change_mtu(po, ifa);
}

void
ospf_if_notify(struct proto *p, unsigned flags, struct iface *iface)
{
  struct proto_ospf *po = (struct proto_ospf *) p;

  /*
  if (iface->flags & IF_IGNORE)
    return;
  */

  /* Going up means that there are no such ifaces yet */
  if (flags & IF_CHANGE_UP)
    return;

  struct ospf_iface *ifa, *ifx;
  WALK_LIST_DELSAFE(ifa, ifx, po->iface_list)
    if ((ifa->type != OSPF_IT_VLINK) && (ifa->iface == iface))
      ospf_iface_notify(po, flags, ifa);

  /* We use here that even shutting down iface also shuts down
     the vlinks, but vlinks are not freed and stays in the
     iface_list even when down */
}

void
ospf_iface_info(struct ospf_iface *ifa)
{
  char *more = "";

  if (ifa->strictnbma &&
      ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_PTMP)))
    more = " (strict)";

  if (ifa->cf->real_bcast &&
      ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_PTP)))
    more = " (real)";

  if (ifa->type == OSPF_IT_VLINK)
  {
    cli_msg(-1015, "Virtual link to %R:", ifa->vid);
    cli_msg(-1015, "\tPeer IP: %I", ifa->vip);
    cli_msg(-1015, "\tTransit area: %R (%u)", ifa->voa->areaid,
	    ifa->voa->areaid);
    cli_msg(-1015, "\tInterface: \"%s\"",
	    (ifa->iface ? ifa->iface->name : "(none)"));
  }
  else
  {
#ifdef OSPFv2
    if (ifa->addr->flags & IA_PEER)
      cli_msg(-1015, "Interface %s (peer %I)", ifa->iface->name, ifa->addr->opposite);
    else
      cli_msg(-1015, "Interface %s (%I/%d)", ifa->iface->name, ifa->addr->prefix, ifa->addr->pxlen);
#else /* OSPFv3 */
    cli_msg(-1015, "Interface %s (IID %d)", ifa->iface->name, ifa->instance_id);
#endif
    cli_msg(-1015, "\tType: %s%s", ospf_it[ifa->type], more);
    cli_msg(-1015, "\tArea: %R (%u)", ifa->oa->areaid, ifa->oa->areaid);
  }
  cli_msg(-1015, "\tState: %s%s", ospf_is[ifa->state], ifa->stub ? " (stub)" : "");
  cli_msg(-1015, "\tPriority: %u", ifa->priority);
  cli_msg(-1015, "\tCost: %u", ifa->cost);
  if (ifa->oa->po->ecmp)
    cli_msg(-1015, "\tECMP weight: %d", ((int) ifa->ecmp_weight) + 1);
  cli_msg(-1015, "\tHello timer: %u", ifa->helloint);

  if (ifa->type == OSPF_IT_NBMA)
  {
    cli_msg(-1015, "\tPoll timer: %u", ifa->pollint);
  }
  cli_msg(-1015, "\tWait timer: %u", ifa->waitint);
  cli_msg(-1015, "\tDead timer: %u", ifa->deadint);
  cli_msg(-1015, "\tRetransmit timer: %u", ifa->rxmtint);
  if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_NBMA))
  {
    cli_msg(-1015, "\tDesigned router (ID): %R", ifa->drid);
    cli_msg(-1015, "\tDesigned router (IP): %I", ifa->drip);
    cli_msg(-1015, "\tBackup designed router (ID): %R", ifa->bdrid);
    cli_msg(-1015, "\tBackup designed router (IP): %I", ifa->bdrip);
  }
}

