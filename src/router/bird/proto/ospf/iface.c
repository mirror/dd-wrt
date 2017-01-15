/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2005 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"
#include "nest/password.h"


const char *ospf_is_names[] = {
  "Down", "Loopback", "Waiting", "PtP", "DROther", "Backup", "DR"
};

const char *ospf_ism_names[] = {
  "InterfaceUp", "WaitTimer", "BackupSeen", "NeighborChange",
  "LoopInd", "UnloopInd", "InterfaceDown"
};

const char *ospf_it[] = { "broadcast", "nbma", "ptp", "ptmp", "virtual link" };


static void
poll_timer_hook(timer * timer)
{
  ospf_send_hello(timer->data, OHS_POLL, NULL);
}

static void
hello_timer_hook(timer * timer)
{
  ospf_send_hello(timer->data, OHS_HELLO, NULL);
}

static void
wait_timer_hook(timer * timer)
{
  struct ospf_iface *ifa = (struct ospf_iface *) timer->data;
  struct ospf_proto *p = ifa->oa->po;

  OSPF_TRACE(D_EVENTS, "Wait timer fired on %s", ifa->ifname);
  ospf_iface_sm(ifa, ISM_WAITF);
}

static inline uint
ifa_tx_length(struct ospf_iface *ifa)
{
  return ifa->cf->tx_length ?: ifa->iface->mtu;
}

static inline uint
ifa_tx_hdrlen(struct ospf_iface *ifa)
{
  uint hlen = SIZE_OF_IP_HEADER;

  /* Relevant just for OSPFv2 */
  if (ifa->autype == OSPF_AUTH_CRYPT)
    hlen += max_mac_length(ifa->passwords);

  return hlen;
}

static inline uint
ifa_bufsize(struct ospf_iface *ifa)
{
  uint bsize = ifa->cf->rx_buffer ?: ifa->iface->mtu;
  return MAX(bsize, ifa->tx_length);
}

static inline uint
ifa_flood_queue_size(struct ospf_iface *ifa)
{
  return ifa->tx_length / 24;
}

int
ospf_iface_assure_bufsize(struct ospf_iface *ifa, uint plen)
{
  plen += ifa->tx_hdrlen;

  if (plen <= ifa->sk->tbsize)
    return 0;

  if (ifa->cf->rx_buffer || (plen > 0xffff))
    return -1;

  plen = BIRD_ALIGN(plen, 1024);
  plen = MIN(plen, 0xffff);
  sk_set_tbsize(ifa->sk, plen);
  return 1;
}


struct nbma_node *
find_nbma_node_(list *nnl, ip_addr ip)
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
  struct ospf_proto *p = ifa->oa->po;

  sock *sk = sk_new(ifa->pool);
  sk->type = SK_IP;
  sk->dport = OSPF_PROTO;
  sk->saddr = ifa->addr->ip;
  sk->iface = ifa->iface;

  sk->tos = ifa->cf->tx_tos;
  sk->priority = ifa->cf->tx_priority;
  sk->rx_hook = ospf_rx_hook;
  // sk->tx_hook = ospf_tx_hook;
  sk->err_hook = ospf_err_hook;
  sk->rbsize = sk->tbsize = ifa_bufsize(ifa);
  sk->data = (void *) ifa;
  sk->flags = SKF_LADDR_RX | (ifa->check_ttl ? SKF_TTL_RX : 0);
  sk->ttl = ifa->cf->ttl_security ? 255 : 1;

  if (sk_open(sk) < 0)
    goto err;

  /* 12 is an offset of the checksum in an OSPFv3 packet */
  if (ospf_is_v3(p))
    if (sk_set_ipv6_checksum(sk, 12) < 0)
      goto err;

  if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_PTP))
  {
    if (ifa->cf->real_bcast)
    {
      ifa->all_routers = ifa->addr->brd;
      ifa->des_routers = IPA_NONE;

      if (sk_setup_broadcast(sk) < 0)
	goto err;
    }
    else
    {
      ifa->all_routers = ospf_is_v2(p) ? IP4_OSPF_ALL_ROUTERS : IP6_OSPF_ALL_ROUTERS;
      ifa->des_routers = ospf_is_v2(p) ? IP4_OSPF_DES_ROUTERS : IP6_OSPF_DES_ROUTERS;

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
  sk_log_error(sk, p->p.name);
  rfree(sk);
  return 0;
}

static inline void
ospf_sk_join_dr(struct ospf_iface *ifa)
{
  if (ifa->sk_dr)
    return;

  if (sk_join_group(ifa->sk, ifa->des_routers) < 0)
    sk_log_error(ifa->sk, ifa->oa->po->p.name);

  ifa->sk_dr = 1;
}

static inline void
ospf_sk_leave_dr(struct ospf_iface *ifa)
{
  if (!ifa->sk_dr)
    return;

  if (sk_leave_group(ifa->sk, ifa->des_routers) < 0)
    sk_log_error(ifa->sk, ifa->oa->po->p.name);

  ifa->sk_dr = 0;
}

void
ospf_open_vlink_sk(struct ospf_proto *p)
{
  sock *sk = sk_new(p->p.pool);
  sk->type = SK_IP;
  sk->dport = OSPF_PROTO;

  /* FIXME: configurable tos/priority ? */
  sk->tos = IP_PREC_INTERNET_CONTROL;
  sk->priority = sk_priority_control;
  sk->err_hook = ospf_verr_hook;

  sk->rbsize = 0;
  sk->tbsize = ospf_is_v2(p) ? IP4_MIN_MTU : IP6_MIN_MTU;
  sk->data = (void *) p;
  sk->flags = 0;

  if (sk_open(sk) < 0)
    goto err;

  /* 12 is an offset of the checksum in an OSPFv3 packet */
  if (ospf_is_v3(p))
    if (sk_set_ipv6_checksum(sk, 12) < 0)
      goto err;

  p->vlink_sk = sk;
  return;

 err:
  sk_log_error(sk, p->p.name);
  log(L_ERR "%s: Cannot open virtual link socket", p->p.name);
  rfree(sk);
}

static void
ospf_iface_down(struct ospf_iface *ifa)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_neighbor *n, *nx;
  struct ospf_iface *iff;

  if (ifa->type != OSPF_IT_VLINK)
  {
    if (ospf_is_v3(ifa->oa->po))
      OSPF_TRACE(D_EVENTS, "Removing interface %s (IID %d) from area %R",
		 ifa->ifname, ifa->instance_id, ifa->oa->areaid);
    else if (ifa->addr->flags & IA_PEER)
      OSPF_TRACE(D_EVENTS, "Removing interface %s (peer %I) from area %R",
		 ifa->ifname, ifa->addr->opposite, ifa->oa->areaid);
    else
      OSPF_TRACE(D_EVENTS, "Removing interface %s (%I/%d) from area %R",
		 ifa->ifname, ifa->addr->prefix, ifa->addr->pxlen, ifa->oa->areaid);

    /* First of all kill all the related vlinks */
    WALK_LIST(iff, p->iface_list)
    {
      if ((iff->type == OSPF_IT_VLINK) && (iff->vifa == ifa))
	ospf_iface_sm(iff, ISM_DOWN);
    }
  }

  WALK_LIST_DELSAFE(n, nx, ifa->neigh_list)
    ospf_neigh_sm(n, INM_KILLNBR);

  if (ifa->hello_timer)
    tm_stop(ifa->hello_timer);

  if (ifa->poll_timer)
    tm_stop(ifa->poll_timer);

  if (ifa->wait_timer)
    tm_stop(ifa->wait_timer);

  ospf_flush2_lsa(p, &ifa->link_lsa);
  ospf_flush2_lsa(p, &ifa->net_lsa);
  ospf_flush2_lsa(p, &ifa->pxn_lsa);

  if (ifa->type == OSPF_IT_VLINK)
  {
    ifa->vifa = NULL;
    ifa->addr = NULL;
    ifa->cost = 0;
    ifa->vip = IPA_NONE;
  }

  ifa->rt_pos_beg = 0;
  ifa->rt_pos_end = 0;
  ifa->px_pos_beg = 0;
  ifa->px_pos_end = 0;
}


void
ospf_iface_remove(struct ospf_iface *ifa)
{
  struct ospf_proto *p = ifa->oa->po;
  int i;

  if (ifa->type == OSPF_IT_VLINK)
    OSPF_TRACE(D_EVENTS, "Removing vlink to %R via area %R", ifa->vid, ifa->voa->areaid);

  /* Release LSAs from flood queue */
  if (!ifa->stub)
    for (i = 0; i < ifa->flood_queue_used; i++)
      ifa->flood_queue[i]->ret_count--;

  ospf_iface_sm(ifa, ISM_DOWN);
  rem_node(NODE ifa);
  rfree(ifa->pool);
}

void
ospf_iface_shutdown(struct ospf_iface *ifa)
{
  if (ifa->state > OSPF_IS_DOWN)
    ospf_send_hello(ifa, OHS_SHUTDOWN, NULL);
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
  struct ospf_proto *p = ifa->oa->po;
  u8 oldstate = ifa->state;

  if (state == oldstate)
    return;

  OSPF_TRACE(D_EVENTS, "Interface %s changed state from %s to %s",
	     ifa->ifname, ospf_is_names[oldstate], ospf_is_names[state]);

  ifa->state = state;

  if ((ifa->type == OSPF_IT_BCAST) && ipa_nonzero(ifa->des_routers) && ifa->sk)
  {
    if ((state == OSPF_IS_BACKUP) || (state == OSPF_IS_DR))
      ospf_sk_join_dr(ifa);
    else
      ospf_sk_leave_dr(ifa);
  }

  if ((oldstate > OSPF_IS_LOOP) && (state <= OSPF_IS_LOOP))
    ospf_iface_down(ifa);

  /* RFC 2328 12.4 Event 2 - iface state change */
  ospf_notify_rt_lsa(ifa->oa);

  /* RFC 5340 4.4.3 Event 1 - iface state change */
  ospf_notify_link_lsa(ifa);

  /* RFC 2328 12.4 Event 3 - iface enters/leaves DR state */
  ospf_notify_net_lsa(ifa);
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
  DBG("SM on %s. Event is '%s'\n", ifa->ifname, ospf_ism_names[event]);

  switch (event)
  {
  case ISM_UP:
    if (ifa->state <= OSPF_IS_LOOP)
    {
      /* Now, nothing should be adjacent */
      if ((ifa->type == OSPF_IT_PTP) ||
	  (ifa->type == OSPF_IT_PTMP) ||
	  (ifa->type == OSPF_IT_VLINK))
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

      ospf_send_hello(ifa, OHS_HELLO, NULL);
    }
    break;

  case ISM_BACKS:
  case ISM_WAITF:
    if (ifa->state == OSPF_IS_WAITING)
      ospf_dr_election(ifa);
    break;

  case ISM_NEICH:
    if (ifa->state >= OSPF_IS_DROTHER)
      ospf_dr_election(ifa);
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
ospf_iface_classify_(struct iface *ifa, struct ifa *addr)
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
  return (type != OSPF_IT_UNDEF) ? type : ospf_iface_classify_(addr->iface, addr);
}


struct ospf_iface *
ospf_iface_find(struct ospf_proto *p, struct iface *what)
{
  struct ospf_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    if ((ifa->iface == what) && (ifa->type != OSPF_IT_VLINK))
      return ifa;

  return NULL;
}

static void
ospf_iface_add(struct object_lock *lock)
{
  struct ospf_iface *ifa = lock->data;
  struct ospf_proto *p = ifa->oa->po;

  /* Open socket if interface is not stub */
  if (! ifa->stub && ! ospf_sk_open(ifa))
  {
    log(L_ERR "%s: Cannot open socket for %s, declaring as stub", p->p.name, ifa->ifname);
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

    ifa->flood_queue_size = ifa_flood_queue_size(ifa);
    ifa->flood_queue = mb_allocz(ifa->pool, ifa->flood_queue_size * sizeof(void *));
  }

  /* Do iface UP, unless there is no link (then wait in LOOP state) */
  if (!ifa->check_link || (ifa->iface->flags & IF_LINK_UP))
    ospf_iface_sm(ifa, ISM_UP);
  else
    ospf_iface_chstate(ifa, OSPF_IS_LOOP);
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
  /* a host address */
  if (addr->flags & IA_HOST)
    return 1;

  /* a loopback iface */
  if (addr->iface->flags & IF_LOOPBACK)
    return 1;

  /*
   * For compatibility reasons on BSD systems, we force OSPF
   * interfaces with non-primary IP prefixes to be stub.
   */
#if defined(OSPFv2) && !defined(CONFIG_MC_PROPER_SRC)
  if (!ip->bsd_secondary && !(addr->flags & IA_PRIMARY))
    return 1;
#endif

  return ip->stub;
}

void
ospf_iface_new(struct ospf_area *oa, struct ifa *addr, struct ospf_iface_patt *ip)
{
  struct ospf_proto *p = oa->po;
  struct iface *iface = addr->iface;
  struct ospf_iface *ifa;
  struct pool *pool;

  if (ospf_is_v3(p))
    OSPF_TRACE(D_EVENTS, "Adding interface %s (IID %d) to area %R",
	       iface->name, ip->instance_id, oa->areaid);
  else if (addr->flags & IA_PEER)
    OSPF_TRACE(D_EVENTS, "Adding interface %s (peer %I) to area %R",
	       iface->name, addr->opposite, oa->areaid);
  else
    OSPF_TRACE(D_EVENTS, "Adding interface %s (%I/%d) to area %R",
	       iface->name, addr->prefix, addr->pxlen, oa->areaid);

  pool = rp_new(p->p.pool, "OSPF Interface");
  ifa = mb_allocz(pool, sizeof(struct ospf_iface));
  ifa->iface = iface;
  ifa->addr = addr;
  ifa->oa = oa;
  ifa->cf = ip;
  ifa->pool = pool;

  ifa->iface_id = iface->index;
  ifa->ifname = iface->name;

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
  ifa->tx_length = ifa_tx_length(ifa);
  ifa->tx_hdrlen = ifa_tx_hdrlen(ifa);
  ifa->check_link = ip->check_link;
  ifa->ecmp_weight = ip->ecmp_weight;
  ifa->check_ttl = (ip->ttl_security == 1);
  ifa->bfd = ip->bfd;
  ifa->autype = ip->autype;
  ifa->passwords = ip->passwords;
  ifa->instance_id = ip->instance_id;

  ifa->ptp_netmask = !(addr->flags & IA_PEER);
  if (ip->ptp_netmask < 2)
    ifa->ptp_netmask = ip->ptp_netmask;


  ifa->type = ospf_iface_classify(ip->type, addr);

  /* Check validity of interface type */
  int old_type = ifa->type;
  u32 if_multi_flag = ip->real_bcast ? IF_BROADCAST : IF_MULTICAST;

  if (ospf_is_v2(p) && (ifa->type == OSPF_IT_BCAST) && (addr->flags & IA_PEER))
    ifa->type = OSPF_IT_PTP;

  if (ospf_is_v2(p) && (ifa->type == OSPF_IT_NBMA) && (addr->flags & IA_PEER))
    ifa->type = OSPF_IT_PTMP;

  if ((ifa->type == OSPF_IT_BCAST) && !(iface->flags & if_multi_flag) && !ifa->stub)
    ifa->type = OSPF_IT_NBMA;

  if ((ifa->type == OSPF_IT_PTP) && !(iface->flags & if_multi_flag) && !ifa->stub)
    ifa->type = OSPF_IT_PTMP;

  if (ifa->type != old_type)
    log(L_WARN "%s: Cannot use interface %s as %s, forcing %s",
	p->p.name, iface->name, ospf_it[old_type], ospf_it[ifa->type]);


  if ((ifa->type == OSPF_IT_PTP) || (ifa->type == OSPF_IT_PTMP))
    ifa->link_lsa_suppression = ip->link_lsa_suppression;

  ifa->state = OSPF_IS_DOWN;
  init_list(&ifa->neigh_list);
  init_list(&ifa->nbma_list);

  struct nbma_node *nb;
  WALK_LIST(nb, ip->nbma_list)
  {
    /* In OSPFv3, addr is link-local while configured neighbors could
       have global IP (although RFC 5340 C.5 says link-local addresses
       should be used). Because OSPFv3 iface is not subnet-specific,
       there is no need for ipa_in_net() check */

    if (ospf_is_v2(p) && !ipa_in_net(nb->ip, addr->prefix, addr->pxlen))
      continue;

    if (ospf_is_v3(p) && !ipa_is_link_local(nb->ip))
      log(L_WARN "%s: Configured neighbor address (%I) should be link-local",
	  p->p.name, nb->ip);

    add_nbma_node(ifa, nb, 0);
  }

  add_tail(&oa->po->iface_list, NODE ifa);

  struct object_lock *lock = olock_new(pool);
  lock->addr = ospf_is_v2(p) ? ifa->addr->prefix : IPA_NONE;
  lock->type = OBJLOCK_IP;
  lock->port = OSPF_PROTO;
  lock->inst = ifa->instance_id;
  lock->iface = iface;
  lock->data = ifa;
  lock->hook = ospf_iface_add;

  olock_acquire(lock);
}

void
ospf_iface_new_vlink(struct ospf_proto *p, struct ospf_iface_patt *ip)
{
  struct ospf_iface *ifa;
  struct pool *pool;

  if (!p->vlink_sk)
    return;

  OSPF_TRACE(D_EVENTS, "Adding vlink to %R via area %R", ip->vid, ip->voa);

  /* Vlink ifname is stored just after the ospf_iface structure */

  pool = rp_new(p->p.pool, "OSPF Vlink");
  ifa = mb_allocz(pool, sizeof(struct ospf_iface) + 16);
  ifa->oa = p->backbone;
  ifa->cf = ip;
  ifa->pool = pool;

  /* Assign iface ID, for vlinks, this is ugly hack */
  u32 vlink_id = p->last_vlink_id++;
  ifa->iface_id = vlink_id + OSPF_VLINK_ID_OFFSET;
  ifa->ifname = (void *) (ifa + 1);
  bsprintf(ifa->ifname, "vlink%d", vlink_id);

  ifa->voa = ospf_find_area(p, ip->voa);
  ifa->vid = ip->vid;
  ifa->sk = p->vlink_sk;

  ifa->helloint = ip->helloint;
  ifa->rxmtint = ip->rxmtint;
  ifa->waitint = ip->waitint;
  ifa->deadint = ip->deadint;
  ifa->inftransdelay = ip->inftransdelay;
  ifa->tx_length = ospf_is_v2(p) ? IP4_MIN_MTU : IP6_MIN_MTU;
  ifa->tx_hdrlen = ifa_tx_hdrlen(ifa);
  ifa->autype = ip->autype;
  ifa->passwords = ip->passwords;
  ifa->instance_id = ip->instance_id;

  ifa->type = OSPF_IT_VLINK;

  ifa->state = OSPF_IS_DOWN;
  init_list(&ifa->neigh_list);
  init_list(&ifa->nbma_list);

  add_tail(&p->iface_list, NODE ifa);

  ifa->hello_timer = tm_new_set(ifa->pool, hello_timer_hook, ifa, 0, ifa->helloint);

  ifa->flood_queue_size = ifa_flood_queue_size(ifa);
  ifa->flood_queue = mb_allocz(ifa->pool, ifa->flood_queue_size * sizeof(void *));
}

static void
ospf_iface_change_timer(timer *tm, uint val)
{
  if (!tm)
    return;

  tm->recurrent = val;

  if (tm->expires)
    tm_start(tm, val);
}

static inline void
ospf_iface_update_flood_queue_size(struct ospf_iface *ifa)
{
  uint old_size = ifa->flood_queue_size;
  uint new_size = ifa_flood_queue_size(ifa);

  if (new_size <= old_size)
    return;

  ifa->flood_queue_size = new_size;
  ifa->flood_queue = mb_realloc(ifa->flood_queue, new_size * sizeof(void *));
  bzero(ifa->flood_queue + old_size, (new_size - old_size) * sizeof(void *));
}

int
ospf_iface_reconfigure(struct ospf_iface *ifa, struct ospf_iface_patt *new)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_iface_patt *old = ifa->cf;
  char *ifname = ifa->ifname;

  /* Type could be changed in ospf_iface_new(),
     but if config values are same then also results are same */
  int old_type = ospf_iface_classify(old->type, ifa->addr);
  int new_type = ospf_iface_classify(new->type, ifa->addr);
  if (old_type != new_type)
    return 0;

  int new_stub = ospf_iface_stubby(new, ifa->addr);
  if (ifa->stub != new_stub)
    return 0;

  /* Change of these options would require to reset the iface socket */
  if ((new->real_bcast != old->real_bcast) ||
      (new->tx_tos != old->tx_tos) ||
      (new->tx_priority != old->tx_priority) ||
      (new->ttl_security != old->ttl_security))
    return 0;

  ifa->cf = new;
  ifa->marked = 0;


  /* HELLO TIMER */
  if (ifa->helloint != new->helloint)
  {
    OSPF_TRACE(D_EVENTS, "Changing hello interval of %s from %d to %d",
	       ifname, ifa->helloint, new->helloint);

    ifa->helloint = new->helloint;
    ospf_iface_change_timer(ifa->hello_timer, ifa->helloint);
  }

  /* RXMT TIMER */
  if (ifa->rxmtint != new->rxmtint)
  {
    OSPF_TRACE(D_EVENTS, "Changing retransmit interval of %s from %d to %d",
	       ifname, ifa->rxmtint, new->rxmtint);

    ifa->rxmtint = new->rxmtint;
    /* FIXME: Update neighbors' timers */
  }

  /* POLL TIMER */
  if (ifa->pollint != new->pollint)
  {
    OSPF_TRACE(D_EVENTS, "Changing poll interval of %s from %d to %d",
	       ifname, ifa->pollint, new->pollint);

    ifa->pollint = new->pollint;
    ospf_iface_change_timer(ifa->poll_timer, ifa->pollint);
  }

  /* WAIT TIMER */
  if (ifa->waitint != new->waitint)
  {
    OSPF_TRACE(D_EVENTS, "Changing wait interval of %s from %d to %d",
	       ifname, ifa->waitint, new->waitint);

    ifa->waitint = new->waitint;
    if (ifa->wait_timer && ifa->wait_timer->expires)
      tm_start(ifa->wait_timer, ifa->waitint);
  }

  /* DEAD TIMER */
  if (ifa->deadint != new->deadint)
  {
    OSPF_TRACE(D_EVENTS, "Changing dead interval of %s from %d to %d",
	       ifname, ifa->deadint, new->deadint);
    ifa->deadint = new->deadint;
  }

  /* INFTRANS */
  if (ifa->inftransdelay != new->inftransdelay)
  {
    OSPF_TRACE(D_EVENTS, "Changing transmit delay of %s from %d to %d",
		     ifname, ifa->inftransdelay, new->inftransdelay);
    ifa->inftransdelay = new->inftransdelay;
  }

  /* AUTHENTICATION */
  if (ifa->autype != new->autype)
  {
    OSPF_TRACE(D_EVENTS, "Changing authentication type of %s", ifname);
    ifa->autype = new->autype;
  }

  /* Update passwords */
  ifa->passwords = new->passwords;

  /* Update header length */
  ifa->tx_hdrlen = ifa_tx_hdrlen(ifa);

  /* Remaining options are just for proper interfaces */
  if (ifa->type == OSPF_IT_VLINK)
    return 1;


  /* COST */
  if (ifa->cost != new->cost)
  {
    OSPF_TRACE(D_EVENTS, "Changing cost of %s from %d to %d",
	       ifname, ifa->cost, new->cost);

    ifa->cost = new->cost;
  }

  /* PRIORITY */
  if (ifa->priority != new->priority)
  {
    OSPF_TRACE(D_EVENTS, "Changing priority of %s from %d to %d",
	       ifname, ifa->priority, new->priority);

    ifa->priority = new->priority;
    ospf_notify_link_lsa(ifa);
  }

  /* STRICT NBMA */
  if (ifa->strictnbma != new->strictnbma)
  {
    OSPF_TRACE(D_EVENTS, "Changing NBMA strictness of %s from %d to %d",
	       ifname, ifa->strictnbma, new->strictnbma);
    ifa->strictnbma = new->strictnbma;
  }

  struct nbma_node *nb, *nbx;

  /* NBMA LIST - remove or update old */
  WALK_LIST_DELSAFE(nb, nbx, ifa->nbma_list)
  {
    struct nbma_node *nb2 = find_nbma_node_(&new->nbma_list, nb->ip);
    if (nb2)
    {
      if (nb->eligible != nb2->eligible)
      {
	OSPF_TRACE(D_EVENTS, "Changing eligibility of NBMA neighbor %I on %s",
		   nb->ip, ifname);
	nb->eligible = nb2->eligible;
      }
    }
    else
    {
      OSPF_TRACE(D_EVENTS, "Removing NBMA neighbor %I on %s",
		       nb->ip, ifname);
      rem_node(NODE nb);
      mb_free(nb);
    }
  }

  /* NBMA LIST - add new */
  WALK_LIST(nb, new->nbma_list)
  {
    /* See related note in ospf_iface_new() */
    if (ospf_is_v2(p) && !ipa_in_net(nb->ip, ifa->addr->prefix, ifa->addr->pxlen))
      continue;

    if (ospf_is_v3(p) && !ipa_is_link_local(nb->ip))
      log(L_WARN "%s: Configured neighbor address (%I) should be link-local",
	  p->p.name, nb->ip);

    if (! find_nbma_node(ifa, nb->ip))
    {
      OSPF_TRACE(D_EVENTS, "Adding NBMA neighbor %I on %s",
		 nb->ip, ifname);
      add_nbma_node(ifa, nb, !!find_neigh_by_ip(ifa, nb->ip));
    }
  }

  int update_buffers = 0;

  /* TX LENGTH */
  if (old->tx_length != new->tx_length)
  {
    OSPF_TRACE(D_EVENTS, "Changing TX length of %s from %d to %d",
	       ifname, old->tx_length, new->tx_length);

    /* ifa cannot be vlink */
    ifa->tx_length = ifa_tx_length(ifa);
    update_buffers = 1;

    if (!ifa->stub)
      ospf_iface_update_flood_queue_size(ifa);
  }

  /* RX BUFFER */
  if (old->rx_buffer != new->rx_buffer)
  {
    OSPF_TRACE(D_EVENTS, "Changing buffer size of %s from %d to %d",
	       ifname, old->rx_buffer, new->rx_buffer);

    /* ifa cannot be vlink */
    update_buffers = 1;
  }

  /* Buffer size depends on both tx_length and rx_buffer options */
  if (update_buffers && ifa->sk)
  {
    uint bsize = ifa_bufsize(ifa);
    sk_set_rbsize(ifa->sk, bsize);
    sk_set_tbsize(ifa->sk, bsize);
  }

  /* LINK */
  if (ifa->check_link != new->check_link)
  {
    OSPF_TRACE(D_EVENTS, "%s link check for %s",
	       new->check_link ? "Enabling" : "Disabling", ifname);
    ifa->check_link = new->check_link;

    /* ifa cannot be vlink */
    if (!(ifa->iface->flags & IF_LINK_UP))
      ospf_iface_sm(ifa, ifa->check_link ? ISM_LOOP : ISM_UNLOOP);
  }

  /* ECMP weight */
  if (ifa->ecmp_weight != new->ecmp_weight)
  {
    OSPF_TRACE(D_EVENTS, "Changing ECMP weight of %s from %d to %d",
	       ifname, (int)ifa->ecmp_weight + 1, (int)new->ecmp_weight + 1);
    ifa->ecmp_weight = new->ecmp_weight;
  }

  /* Link LSA suppression */
  if (((ifa->type == OSPF_IT_PTP) || (ifa->type == OSPF_IT_PTMP)) &&
      (ifa->link_lsa_suppression != new->link_lsa_suppression))
  {
    OSPF_TRACE(D_EVENTS, "Changing link LSA suppression of %s from %d to %d",
	       ifname, ifa->link_lsa_suppression, new->link_lsa_suppression);

    ifa->link_lsa_suppression = new->link_lsa_suppression;
    ospf_notify_link_lsa(ifa);
  }

  /* BFD */
  if (ifa->bfd != new->bfd)
  {
    OSPF_TRACE(D_EVENTS, "%s BFD for %s",
	       new->bfd ? "Enabling" : "Disabling", ifname);
    ifa->bfd = new->bfd;

    struct ospf_neighbor *n;
    WALK_LIST(n, ifa->neigh_list)
      ospf_neigh_update_bfd(n, ifa->bfd);
  }


  /* instance_id is not updated - it is part of key */

  return 1;
}


/*
 * State for matching iface pattterns walk
 *
 * This is significantly different in OSPFv2 and OSPFv3.
 * In OSPFv2, OSPF ifaces are created for each IP prefix (struct ifa)
 * In OSPFv3, OSPF ifaces are created based on real iface (struct iface)
 * We support instance_id for both OSPFv2 (RFC 6549) and OSPFv3.
 *
 * We process one ifa/iface and match it for all configured instance IDs. We
 * maintain bitfields to track whether given instance ID was already matched.
 * We have two bitfields, one global (active) and one per area (ignore), to
 * detect misconfigured cases where one iface with one instance ID matches in
 * multiple areas.
 */

struct ospf_mip_walk {
  u32 active[8];		/* Bitfield of active instance IDs */
  u32 ignore[8];		/* Bitfield of instance IDs matched in current area */
  struct ospf_area *oa;		/* Current area */
  struct ospf_iface_patt *ip;	/* Current iface pattern */
  struct iface *iface;		/* Specified iface (input) */
  struct ifa *a;		/* Specified ifa (input) */
  int warn;			/* Whether iface matched in multiple areas */
};

static int
ospf_walk_matching_iface_patts(struct ospf_proto *p, struct ospf_mip_walk *s)
{
  int id;

  if (s->ip)
    goto step;

  WALK_LIST(s->oa, p->area_list)
  {
    if (s->oa->marked)
      continue;

    WALK_LIST(s->ip, s->oa->ac->patt_list)
    {
      id = s->ip->instance_id;
      if (BIT32_TEST(s->ignore, id))
	continue;

      if (iface_patt_match(&s->ip->i, s->iface, s->a))
      {
	/* Now we matched ifa/iface/instance_id for the first time in current area */
	BIT32_SET(s->ignore, id);

	/* If we already found it in previous areas, ignore it and add warning */
	if (BIT32_TEST(s->active, id))
	  { s->warn = 1; continue; }

	BIT32_SET(s->active, id);
	return 1;
      step:
	;
      }
    }
    BIT32_ZERO(s->ignore, 256);
  }

  if (s->warn)
    log(L_WARN "%s: Interface %s matches for multiple areas", p->p.name, s->iface->name);

  return 0;
}


static struct ospf_iface *
ospf_iface_find_by_key(struct ospf_proto *p, struct ifa *a, int instance_id)
{
  struct ospf_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    if ((ifa->addr == a) && (ifa->instance_id == instance_id) &&
	(ifa->type != OSPF_IT_VLINK))
      return ifa;

  return NULL;
}


void
ospf_ifa_notify2(struct proto *P, uint flags, struct ifa *a)
{
  struct ospf_proto *p = (struct ospf_proto *) P;

  if (a->flags & IA_SECONDARY)
    return;

  if (a->scope <= SCOPE_LINK)
    return;

  /* In OSPFv2, we create OSPF iface for each address. */
  if (flags & IF_CHANGE_UP)
  {
    struct ospf_mip_walk s = { .iface = a->iface, .a = a };
    while (ospf_walk_matching_iface_patts(p, &s))
      ospf_iface_new(s.oa, a, s.ip);
  }

  if (flags & IF_CHANGE_DOWN)
  {
    struct ospf_iface *ifa, *ifx;
    WALK_LIST_DELSAFE(ifa, ifx, p->iface_list)
      if ((ifa->type != OSPF_IT_VLINK) && (ifa->addr == a))
	ospf_iface_remove(ifa);
    /* See a note in ospf_iface_notify() */
  }
}

void
ospf_ifa_notify3(struct proto *P, uint flags, struct ifa *a)
{
  struct ospf_proto *p = (struct ospf_proto *) P;

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
      struct ospf_mip_walk s = { .iface = a->iface };
      while (ospf_walk_matching_iface_patts(p, &s))
	ospf_iface_new(s.oa, a, s.ip);
    }

    if (flags & IF_CHANGE_DOWN)
    {
      struct ospf_iface *ifa, *ifx;
      WALK_LIST_DELSAFE(ifa, ifx, p->iface_list)
	if ((ifa->addr == a) && (ifa->type != OSPF_IT_VLINK))
	  ospf_iface_remove(ifa);
    }
  }
  else
  {
    struct ospf_iface *ifa;
    WALK_LIST(ifa, p->iface_list)
      if (ifa->iface == a->iface)
      {
	/* RFC 5340 4.4.3 Event 5 - prefix added/deleted */
	ospf_notify_link_lsa(ifa);
	ospf_notify_rt_lsa(ifa->oa);
      }
  }
}


static void
ospf_reconfigure_ifaces2(struct ospf_proto *p)
{
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

      struct ospf_mip_walk s = { .iface = iface, .a = a };
      while (ospf_walk_matching_iface_patts(p, &s))
      {
	/* Main inner loop */
	struct ospf_iface *ifa = ospf_iface_find_by_key(p, a, s.ip->instance_id);
	if (ifa)
	{
	  if ((ifa->oa == s.oa) && (ifa->marked < 2) &&
	      ospf_iface_reconfigure(ifa, s.ip))
	    continue;

	  /* Hard restart */
	  log(L_INFO "%s: Restarting interface %s (%I/%d) in area %R",
	      p->p.name, ifa->ifname, a->prefix, a->pxlen, s.oa->areaid);
	  ospf_iface_shutdown(ifa);
	  ospf_iface_remove(ifa);
	}

	ospf_iface_new(s.oa, a, s.ip);
      }
    }
  }
}

static void
ospf_reconfigure_ifaces3(struct ospf_proto *p)
{
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

      struct ospf_mip_walk s = { .iface = iface };
      while (ospf_walk_matching_iface_patts(p, &s))
      {
	/* Main inner loop */
	struct ospf_iface *ifa = ospf_iface_find_by_key(p, a, s.ip->instance_id);
	if (ifa)
	{
	  if ((ifa->oa == s.oa) && (ifa->marked < 2) &&
	      ospf_iface_reconfigure(ifa, s.ip))
	    continue;

	  /* Hard restart */
	  log(L_INFO "%s: Restarting interface %s (IID %d) in area %R",
	      p->p.name, ifa->ifname, ifa->instance_id, s.oa->areaid);
	  ospf_iface_shutdown(ifa);
	  ospf_iface_remove(ifa);
	}

	ospf_iface_new(s.oa, a, s.ip);
      }
    }
  }
}

void
ospf_reconfigure_ifaces(struct ospf_proto *p)
{
  if (ospf_is_v2(p))
    ospf_reconfigure_ifaces2(p);
  else
    ospf_reconfigure_ifaces3(p);
}


static void
ospf_iface_change_mtu(struct ospf_proto *p, struct ospf_iface *ifa)
{
  /* ifa is not vlink */

  OSPF_TRACE(D_EVENTS, "Interface %s changed MTU to %d", ifa->iface->mtu);

  ifa->tx_length = ifa_tx_length(ifa);

  if (!ifa->sk)
    return;

  /* We do not shrink dynamic buffers */
  uint bsize = ifa_bufsize(ifa);
  if (bsize > ifa->sk->rbsize)
    sk_set_rbsize(ifa->sk, bsize);
  if (bsize > ifa->sk->tbsize)
    sk_set_tbsize(ifa->sk, bsize);

  if (!ifa->stub)
    ospf_iface_update_flood_queue_size(ifa);
}

static void
ospf_iface_notify(struct ospf_proto *p, uint flags, struct ospf_iface *ifa)
{
  /* ifa is not vlink */

  if (flags & IF_CHANGE_DOWN)
  {
    ospf_iface_remove(ifa);
    return;
  }

  if (flags & IF_CHANGE_LINK)
    ospf_iface_sm(ifa, (ifa->iface->flags & IF_LINK_UP) ? ISM_UNLOOP : ISM_LOOP);

  if (flags & IF_CHANGE_MTU)
    ospf_iface_change_mtu(p, ifa);
}

void
ospf_if_notify(struct proto *P, uint flags, struct iface *iface)
{
  struct ospf_proto *p = (struct ospf_proto *) P;

  /*
  if (iface->flags & IF_IGNORE)
    return;
  */

  /* Going up means that there are no such ifaces yet */
  if (flags & IF_CHANGE_UP)
    return;

  struct ospf_iface *ifa, *ifx;
  WALK_LIST_DELSAFE(ifa, ifx, p->iface_list)
    if (ifa->iface == iface)
      ospf_iface_notify(p, flags, ifa);

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
    cli_msg(-1015, "Virtual link %s to %R", ifa->ifname, ifa->vid);
    cli_msg(-1015, "\tPeer IP: %I", ifa->vip);
    cli_msg(-1015, "\tTransit area: %R (%u)", ifa->voa->areaid, ifa->voa->areaid);
  }
  else
  {
    if (ospf_is_v3(ifa->oa->po))
      cli_msg(-1015, "Interface %s (IID %d)", ifa->ifname, ifa->instance_id);
    else if (ifa->addr->flags & IA_PEER)
      cli_msg(-1015, "Interface %s (peer %I)", ifa->ifname, ifa->addr->opposite);
    else
      cli_msg(-1015, "Interface %s (%I/%d)", ifa->ifname, ifa->addr->prefix, ifa->addr->pxlen);

    cli_msg(-1015, "\tType: %s%s", ospf_it[ifa->type], more);
    cli_msg(-1015, "\tArea: %R (%u)", ifa->oa->areaid, ifa->oa->areaid);
  }
  cli_msg(-1015, "\tState: %s%s", ospf_is_names[ifa->state], ifa->stub ? " (stub)" : "");
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
    cli_msg(-1015, "\tDesignated router (ID): %R", ifa->drid);
    cli_msg(-1015, "\tDesignated router (IP): %I", ifa->drip);
    cli_msg(-1015, "\tBackup designated router (ID): %R", ifa->bdrid);
    cli_msg(-1015, "\tBackup designated router (IP): %I", ifa->bdrip);
  }
}
