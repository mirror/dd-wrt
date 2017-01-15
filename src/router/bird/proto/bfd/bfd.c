/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Bidirectional Forwarding Detection
 *
 * The BFD protocol is implemented in three files: |bfd.c| containing the
 * protocol logic and the protocol glue with BIRD core, |packets.c| handling BFD
 * packet processing, RX, TX and protocol sockets. |io.c| then contains generic
 * code for the event loop, threads and event sources (sockets, microsecond
 * timers). This generic code will be merged to the main BIRD I/O code in the
 * future.
 *
 * The BFD implementation uses a separate thread with an internal event loop for
 * handling the protocol logic, which requires high-res and low-latency timing,
 * so it is not affected by the rest of BIRD, which has several low-granularity
 * hooks in the main loop, uses second-based timers and cannot offer good
 * latency. The core of BFD protocol (the code related to BFD sessions,
 * interfaces and packets) runs in the BFD thread, while the rest (the code
 * related to BFD requests, BFD neighbors and the protocol glue) runs in the
 * main thread.
 *
 * BFD sessions are represented by structure &bfd_session that contains a state
 * related to the session and two timers (TX timer for periodic packets and hold
 * timer for session timeout). These sessions are allocated from @session_slab
 * and are accessible by two hash tables, @session_hash_id (by session ID) and
 * @session_hash_ip (by IP addresses of neighbors). Slab and both hashes are in
 * the main protocol structure &bfd_proto. The protocol logic related to BFD
 * sessions is implemented in internal functions bfd_session_*(), which are
 * expected to be called from the context of BFD thread, and external functions
 * bfd_add_session(), bfd_remove_session() and bfd_reconfigure_session(), which
 * form an interface to the BFD core for the rest and are expected to be called
 * from the context of main thread.
 *
 * Each BFD session has an associated BFD interface, represented by structure
 * &bfd_iface. A BFD interface contains a socket used for TX (the one for RX is
 * shared in &bfd_proto), an interface configuration and reference counter.
 * Compared to interface structures of other protocols, these structures are not
 * created and removed based on interface notification events, but according to
 * the needs of BFD sessions. When a new session is created, it requests a
 * proper BFD interface by function bfd_get_iface(), which either finds an
 * existing one in &iface_list (from &bfd_proto) or allocates a new one. When a
 * session is removed, an associated iface is discharged by bfd_free_iface().
 *
 * BFD requests are the external API for the other protocols. When a protocol
 * wants a BFD session, it calls bfd_request_session(), which creates a
 * structure &bfd_request containing approprite information and an notify hook.
 * This structure is a resource associated with the caller's resource pool. When
 * a BFD protocol is available, a BFD request is submitted to the protocol, an
 * appropriate BFD session is found or created and the request is attached to
 * the session. When a session changes state, all attached requests (and related
 * protocols) are notified. Note that BFD requests do not depend on BFD protocol
 * running. When the BFD protocol is stopped or removed (or not available from
 * beginning), related BFD requests are stored in @bfd_wait_list, where waits
 * for a new protocol.
 *
 * BFD neighbors are just a way to statically configure BFD sessions without
 * requests from other protocol. Structures &bfd_neighbor are part of BFD
 * configuration (like static routes in the static protocol). BFD neighbors are
 * handled by BFD protocol like it is a BFD client -- when a BFD neighbor is
 * ready, the protocol just creates a BFD request like any other protocol.
 *
 * The protocol uses a new generic event loop (structure &birdloop) from |io.c|,
 * which supports sockets, timers and events like the main loop. Timers
 * (structure &timer2) are new microsecond based timers, while sockets and
 * events are the same. A birdloop is associated with a thread (field @thread)
 * in which event hooks are executed. Most functions for setting event sources
 * (like sk_start() or tm2_start()) must be called from the context of that
 * thread. Birdloop allows to temporarily acquire the context of that thread for
 * the main thread by calling birdloop_enter() and then birdloop_leave(), which
 * also ensures mutual exclusion with all event hooks. Note that resources
 * associated with a birdloop (like timers) should be attached to the
 * independent resource pool, detached from the main resource tree.
 *
 * There are two kinds of interaction between the BFD core (running in the BFD
 * thread) and the rest of BFD (running in the main thread). The first kind are
 * configuration calls from main thread to the BFD thread (like bfd_add_session()).
 * These calls are synchronous and use birdloop_enter() mechanism for mutual
 * exclusion. The second kind is a notification about session changes from the
 * BFD thread to the main thread. This is done in an asynchronous way, sesions
 * with pending notifications are linked (in the BFD thread) to @notify_list in
 * &bfd_proto, and then bfd_notify_hook() in the main thread is activated using
 * bfd_notify_kick() and a pipe. The hook then processes scheduled sessions and
 * calls hooks from associated BFD requests. This @notify_list (and state fields
 * in structure &bfd_session) is protected by a spinlock in &bfd_proto and
 * functions bfd_lock_sessions() / bfd_unlock_sessions().
 *
 * There are few data races (accessing @p->p.debug from TRACE() from the BFD
 * thread and accessing some some private fields of %bfd_session from
 * bfd_show_sessions() from the main thread, but these are harmless (i hope).
 *
 * TODO: document functions and access restrictions for fields in BFD structures.
 *
 * Supported standards:
 * - RFC 5880 - main BFD standard
 * - RFC 5881 - BFD for IP links
 * - RFC 5882 - generic application of BFD
 * - RFC 5883 - BFD for multihop paths
 */

#include "bfd.h"


#define HASH_ID_KEY(n)		n->loc_id
#define HASH_ID_NEXT(n)		n->next_id
#define HASH_ID_EQ(a,b)		a == b
#define HASH_ID_FN(k)		k

#define HASH_IP_KEY(n)		n->addr
#define HASH_IP_NEXT(n)		n->next_ip
#define HASH_IP_EQ(a,b)		ipa_equal(a,b)
#define HASH_IP_FN(k)		ipa_hash32(k)

static list bfd_proto_list;
static list bfd_wait_list;

const char *bfd_state_names[] = { "AdminDown", "Down", "Init", "Up" };

static void bfd_session_set_min_tx(struct bfd_session *s, u32 val);
static struct bfd_iface *bfd_get_iface(struct bfd_proto *p, ip_addr local, struct iface *iface);
static void bfd_free_iface(struct bfd_iface *ifa);
static inline void bfd_notify_kick(struct bfd_proto *p);


/*
 *	BFD sessions
 */

static void
bfd_session_update_state(struct bfd_session *s, uint state, uint diag)
{
  struct bfd_proto *p = s->ifa->bfd;
  uint old_state = s->loc_state;
  int notify;

  if (state == old_state)
    return;

  TRACE(D_EVENTS, "Session to %I changed state from %s to %s",
	s->addr, bfd_state_names[old_state], bfd_state_names[state]);

  bfd_lock_sessions(p);
  s->loc_state = state;
  s->loc_diag = diag;

  notify = !NODE_VALID(&s->n);
  if (notify)
    add_tail(&p->notify_list, &s->n);
  bfd_unlock_sessions(p);

  if (state == BFD_STATE_UP)
    bfd_session_set_min_tx(s, s->ifa->cf->min_tx_int);

  if (old_state == BFD_STATE_UP)
    bfd_session_set_min_tx(s, s->ifa->cf->idle_tx_int);

  if (notify)
    bfd_notify_kick(p);
}

static void
bfd_session_update_tx_interval(struct bfd_session *s)
{
  u32 tx_int = MAX(s->des_min_tx_int, s->rem_min_rx_int);
  u32 tx_int_l = tx_int - (tx_int / 4);	 // 75 %
  u32 tx_int_h = tx_int - (tx_int / 10); // 90 %

  s->tx_timer->recurrent = tx_int_l;
  s->tx_timer->randomize = tx_int_h - tx_int_l;

  /* Do not set timer if no previous event */
  if (!s->last_tx)
    return;

  /* Set timer relative to last tx_timer event */
  tm2_set(s->tx_timer, s->last_tx + tx_int_l);
}

static void
bfd_session_update_detection_time(struct bfd_session *s, int kick)
{
  btime timeout = (btime) MAX(s->req_min_rx_int, s->rem_min_tx_int) * s->rem_detect_mult;

  if (kick)
    s->last_rx = current_time();

  if (!s->last_rx)
    return;

  tm2_set(s->hold_timer, s->last_rx + timeout);
}

static void
bfd_session_control_tx_timer(struct bfd_session *s, int reset)
{
  // if (!s->opened) goto stop;

  if (s->passive && (s->rem_id == 0))
    goto stop;

  if (s->rem_demand_mode &&
      !s->poll_active &&
      (s->loc_state == BFD_STATE_UP) &&
      (s->rem_state == BFD_STATE_UP))
    goto stop;

  if (s->rem_min_rx_int == 0)
    goto stop;

  /* So TX timer should run */
  if (reset || !tm2_active(s->tx_timer))
  {
    s->last_tx = 0;
    tm2_start(s->tx_timer, 0);
  }

  return;

 stop:
  tm2_stop(s->tx_timer);
  s->last_tx = 0;
}

static void
bfd_session_request_poll(struct bfd_session *s, u8 request)
{
  /* Not sure about this, but doing poll in this case does not make sense */
  if (s->rem_id == 0)
    return;

  s->poll_scheduled |= request;

  if (s->poll_active)
    return;

  s->poll_active = s->poll_scheduled;
  s->poll_scheduled = 0;

  bfd_session_control_tx_timer(s, 1);
}

static void
bfd_session_terminate_poll(struct bfd_session *s)
{
  u8 poll_done = s->poll_active & ~s->poll_scheduled;

  if (poll_done & BFD_POLL_TX)
    s->des_min_tx_int = s->des_min_tx_new;

  if (poll_done & BFD_POLL_RX)
    s->req_min_rx_int = s->req_min_rx_new;

  s->poll_active = s->poll_scheduled;
  s->poll_scheduled = 0;

  /* Timers are updated by caller - bfd_session_process_ctl() */
}

void
bfd_session_process_ctl(struct bfd_session *s, u8 flags, u32 old_tx_int, u32 old_rx_int)
{
  if (s->poll_active && (flags & BFD_FLAG_FINAL))
    bfd_session_terminate_poll(s);

  if ((s->des_min_tx_int != old_tx_int) || (s->rem_min_rx_int != old_rx_int))
    bfd_session_update_tx_interval(s);

  bfd_session_update_detection_time(s, 1);

  /* Update session state */
  int next_state = 0;
  int diag = BFD_DIAG_NOTHING;

  switch (s->loc_state)
  {
  case BFD_STATE_ADMIN_DOWN:
    return;

  case BFD_STATE_DOWN:
    if (s->rem_state == BFD_STATE_DOWN)		next_state = BFD_STATE_INIT;
    else if (s->rem_state == BFD_STATE_INIT)	next_state = BFD_STATE_UP;
    break;

  case BFD_STATE_INIT:
    if (s->rem_state == BFD_STATE_ADMIN_DOWN)	next_state = BFD_STATE_DOWN, diag = BFD_DIAG_NEIGHBOR_DOWN;
    else if (s->rem_state >= BFD_STATE_INIT)	next_state = BFD_STATE_UP;
    break;

  case BFD_STATE_UP:
    if (s->rem_state <= BFD_STATE_DOWN)		next_state = BFD_STATE_DOWN, diag = BFD_DIAG_NEIGHBOR_DOWN;
    break;
  }

  if (next_state)
    bfd_session_update_state(s, next_state, diag);

  bfd_session_control_tx_timer(s, 0);

  if (flags & BFD_FLAG_POLL)
    bfd_send_ctl(s->ifa->bfd, s, 1);
}

static void
bfd_session_timeout(struct bfd_session *s)
{
  struct bfd_proto *p = s->ifa->bfd;

  TRACE(D_EVENTS, "Session to %I expired", s->addr);

  s->rem_state = BFD_STATE_DOWN;
  s->rem_id = 0;
  s->rem_min_tx_int = 0;
  s->rem_min_rx_int = 1;
  s->rem_demand_mode = 0;
  s->rem_detect_mult = 0;
  s->rx_csn_known = 0;

  s->poll_active = 0;
  s->poll_scheduled = 0;

  bfd_session_update_state(s, BFD_STATE_DOWN, BFD_DIAG_TIMEOUT);

  bfd_session_control_tx_timer(s, 1);
}

static void
bfd_session_set_min_tx(struct bfd_session *s, u32 val)
{
  /* Note that des_min_tx_int <= des_min_tx_new */

  if (val == s->des_min_tx_new)
    return;

  s->des_min_tx_new = val;

  /* Postpone timer update if des_min_tx_int increases and the session is up */
  if ((s->loc_state != BFD_STATE_UP) || (val < s->des_min_tx_int))
  {
    s->des_min_tx_int = val;
    bfd_session_update_tx_interval(s);
  }

  bfd_session_request_poll(s, BFD_POLL_TX);
}

static void
bfd_session_set_min_rx(struct bfd_session *s, u32 val)
{
  /* Note that req_min_rx_int >= req_min_rx_new */

  if (val == s->req_min_rx_new)
    return;

  s->req_min_rx_new = val;

  /* Postpone timer update if req_min_rx_int decreases and the session is up */
  if ((s->loc_state != BFD_STATE_UP) || (val > s->req_min_rx_int))
  {
    s->req_min_rx_int = val;
    bfd_session_update_detection_time(s, 0);
  }

  bfd_session_request_poll(s, BFD_POLL_RX);
}

struct bfd_session *
bfd_find_session_by_id(struct bfd_proto *p, u32 id)
{
  return HASH_FIND(p->session_hash_id, HASH_ID, id);
}

struct bfd_session *
bfd_find_session_by_addr(struct bfd_proto *p, ip_addr addr)
{
  return HASH_FIND(p->session_hash_ip, HASH_IP, addr);
}

static void
bfd_tx_timer_hook(timer2 *t)
{
  struct bfd_session *s = t->data;

  s->last_tx = current_time();
  bfd_send_ctl(s->ifa->bfd, s, 0);
}

static void
bfd_hold_timer_hook(timer2 *t)
{
  bfd_session_timeout(t->data);
}

static u32
bfd_get_free_id(struct bfd_proto *p)
{
  u32 id;
  for (id = random_u32(); 1; id++)
    if (id && !bfd_find_session_by_id(p, id))
      break;

  return id;
}

static struct bfd_session *
bfd_add_session(struct bfd_proto *p, ip_addr addr, ip_addr local, struct iface *iface)
{
  birdloop_enter(p->loop);

  struct bfd_iface *ifa = bfd_get_iface(p, local, iface);

  struct bfd_session *s = sl_alloc(p->session_slab);
  bzero(s, sizeof(struct bfd_session));

  s->addr = addr;
  s->ifa = ifa;
  s->loc_id = bfd_get_free_id(p);

  HASH_INSERT(p->session_hash_id, HASH_ID, s);
  HASH_INSERT(p->session_hash_ip, HASH_IP, s);


  /* Initialization of state variables - see RFC 5880 6.8.1 */
  s->loc_state = BFD_STATE_DOWN;
  s->rem_state = BFD_STATE_DOWN;
  s->des_min_tx_int = s->des_min_tx_new = ifa->cf->idle_tx_int;
  s->req_min_rx_int = s->req_min_rx_new = ifa->cf->min_rx_int;
  s->rem_min_rx_int = 1;
  s->detect_mult = ifa->cf->multiplier;
  s->passive = ifa->cf->passive;
  s->tx_csn = random_u32();

  s->tx_timer = tm2_new_init(p->tpool, bfd_tx_timer_hook, s, 0, 0);
  s->hold_timer = tm2_new_init(p->tpool, bfd_hold_timer_hook, s, 0, 0);
  bfd_session_update_tx_interval(s);
  bfd_session_control_tx_timer(s, 1);

  init_list(&s->request_list);
  s->last_state_change = now;

  TRACE(D_EVENTS, "Session to %I added", s->addr);

  birdloop_leave(p->loop);

  return s;
}

/*
static void
bfd_open_session(struct bfd_proto *p, struct bfd_session *s, ip_addr local, struct iface *ifa)
{
  birdloop_enter(p->loop);

  s->opened = 1;

  bfd_session_control_tx_timer(s);

  birdloop_leave(p->loop);
}

static void
bfd_close_session(struct bfd_proto *p, struct bfd_session *s)
{
  birdloop_enter(p->loop);

  s->opened = 0;

  bfd_session_update_state(s, BFD_STATE_DOWN, BFD_DIAG_PATH_DOWN);
  bfd_session_control_tx_timer(s);

  birdloop_leave(p->loop);
}
*/

static void
bfd_remove_session(struct bfd_proto *p, struct bfd_session *s)
{
  ip_addr ip = s->addr;

  /* Caller should ensure that request list is empty */

  birdloop_enter(p->loop);

  /* Remove session from notify list if scheduled for notification */
  /* No need for bfd_lock_sessions(), we are already protected by birdloop_enter() */
  if (NODE_VALID(&s->n))
    rem_node(&s->n);

  bfd_free_iface(s->ifa);

  rfree(s->tx_timer);
  rfree(s->hold_timer);

  HASH_REMOVE(p->session_hash_id, HASH_ID, s);
  HASH_REMOVE(p->session_hash_ip, HASH_IP, s);

  sl_free(p->session_slab, s);

  TRACE(D_EVENTS, "Session to %I removed", ip);

  birdloop_leave(p->loop);
}

static void
bfd_reconfigure_session(struct bfd_proto *p, struct bfd_session *s)
{
  birdloop_enter(p->loop);

  struct bfd_iface_config *cf = s->ifa->cf;

  u32 tx = (s->loc_state == BFD_STATE_UP) ? cf->min_tx_int : cf->idle_tx_int;
  bfd_session_set_min_tx(s, tx);
  bfd_session_set_min_rx(s, cf->min_rx_int);
  s->detect_mult = cf->multiplier;
  s->passive = cf->passive;

  bfd_session_control_tx_timer(s, 0);

  birdloop_leave(p->loop);

  TRACE(D_EVENTS, "Session to %I reconfigured", s->addr);
}


/*
 *	BFD interfaces
 */

static struct bfd_iface_config bfd_default_iface = {
  .min_rx_int = BFD_DEFAULT_MIN_RX_INT,
  .min_tx_int = BFD_DEFAULT_MIN_TX_INT,
  .idle_tx_int = BFD_DEFAULT_IDLE_TX_INT,
  .multiplier = BFD_DEFAULT_MULTIPLIER
};

static inline struct bfd_iface_config *
bfd_find_iface_config(struct bfd_config *cf, struct iface *iface)
{
  struct bfd_iface_config *ic;

  ic = iface ? (void *) iface_patt_find(&cf->patt_list, iface, NULL) : cf->multihop;

  return ic ? ic : &bfd_default_iface;
}

static struct bfd_iface *
bfd_get_iface(struct bfd_proto *p, ip_addr local, struct iface *iface)
{
  struct bfd_iface *ifa;

  WALK_LIST(ifa, p->iface_list)
    if (ipa_equal(ifa->local, local) && (ifa->iface == iface))
      return ifa->uc++, ifa;

  struct bfd_config *cf = (struct bfd_config *) (p->p.cf);
  struct bfd_iface_config *ic = bfd_find_iface_config(cf, iface);

  ifa = mb_allocz(p->tpool, sizeof(struct bfd_iface));
  ifa->local = local;
  ifa->iface = iface;
  ifa->cf = ic;
  ifa->bfd = p;

  ifa->sk = bfd_open_tx_sk(p, local, iface);
  ifa->uc = 1;

  add_tail(&p->iface_list, &ifa->n);

  return ifa;
}

static void
bfd_free_iface(struct bfd_iface *ifa)
{
  if (!ifa || --ifa->uc)
    return;

  if (ifa->sk)
  {
    sk_stop(ifa->sk);
    rfree(ifa->sk);
  }

  rem_node(&ifa->n);
  mb_free(ifa);
}

static void
bfd_reconfigure_iface(struct bfd_proto *p, struct bfd_iface *ifa, struct bfd_config *nc)
{
  struct bfd_iface_config *nic = bfd_find_iface_config(nc, ifa->iface);
  ifa->changed = !!memcmp(nic, ifa->cf, sizeof(struct bfd_iface_config));

  /* This should be probably changed to not access ifa->cf from the BFD thread */
  birdloop_enter(p->loop);
  ifa->cf = nic;
  birdloop_leave(p->loop);
}


/*
 *	BFD requests
 */

static void
bfd_request_notify(struct bfd_request *req, u8 state, u8 diag)
{
  u8 old_state = req->state;

  if (state == old_state)
    return;

  req->state = state;
  req->diag = diag;
  req->old_state = old_state;
  req->down = (old_state == BFD_STATE_UP) && (state == BFD_STATE_DOWN);

  if (req->hook)
    req->hook(req);
}

static int
bfd_add_request(struct bfd_proto *p, struct bfd_request *req)
{
  struct bfd_session *s = bfd_find_session_by_addr(p, req->addr);
  u8 state, diag;

  if (!s)
    s = bfd_add_session(p, req->addr, req->local, req->iface);

  rem_node(&req->n);
  add_tail(&s->request_list, &req->n);
  req->session = s;

  bfd_lock_sessions(p);
  state = s->loc_state;
  diag = s->loc_diag;
  bfd_unlock_sessions(p);

  bfd_request_notify(req, state, diag);

  return 1;
}

static void
bfd_submit_request(struct bfd_request *req)
{
  node *n;

  WALK_LIST(n, bfd_proto_list)
    if (bfd_add_request(SKIP_BACK(struct bfd_proto, bfd_node, n), req))
      return;

  rem_node(&req->n);
  add_tail(&bfd_wait_list, &req->n);
  req->session = NULL;
  bfd_request_notify(req, BFD_STATE_ADMIN_DOWN, 0);
}

static void
bfd_take_requests(struct bfd_proto *p)
{
  node *n, *nn;

  WALK_LIST_DELSAFE(n, nn, bfd_wait_list)
    bfd_add_request(p, SKIP_BACK(struct bfd_request, n, n));
}

static void
bfd_drop_requests(struct bfd_proto *p)
{
  node *n;

  HASH_WALK(p->session_hash_id, next_id, s)
  {
    /* We assume that p is not in bfd_proto_list */
    WALK_LIST_FIRST(n, s->request_list)
      bfd_submit_request(SKIP_BACK(struct bfd_request, n, n));
  }
  HASH_WALK_END;
}

static struct resclass bfd_request_class;

struct bfd_request *
bfd_request_session(pool *p, ip_addr addr, ip_addr local, struct iface *iface,
		    void (*hook)(struct bfd_request *), void *data)
{
  struct bfd_request *req = ralloc(p, &bfd_request_class);

  /* Hack: self-link req->n, we will call rem_node() on it */
  req->n.prev = req->n.next = &req->n;

  req->addr = addr;
  req->local = local;
  req->iface = iface;

  bfd_submit_request(req);

  req->hook = hook;
  req->data = data;

  return req;
}

static void
bfd_request_free(resource *r)
{
  struct bfd_request *req = (struct bfd_request *) r;
  struct bfd_session *s = req->session;

  rem_node(&req->n);

  /* Remove the session if there is no request for it. Skip that if
     inside notify hooks, will be handled by bfd_notify_hook() itself */

  if (s && EMPTY_LIST(s->request_list) && !s->notify_running)
    bfd_remove_session(s->ifa->bfd, s);
}

static void
bfd_request_dump(resource *r)
{
  struct bfd_request *req = (struct bfd_request *) r;

  debug("(code %p, data %p)\n", req->hook, req->data);
}

static struct resclass bfd_request_class = {
  "BFD request",
  sizeof(struct bfd_request),
  bfd_request_free,
  bfd_request_dump,
  NULL,
  NULL
};


/*
 *	BFD neighbors
 */

static void
bfd_neigh_notify(struct neighbor *nb)
{
  struct bfd_proto *p = (struct bfd_proto *) nb->proto;
  struct bfd_neighbor *n = nb->data;

  if (!n)
    return;

  if ((nb->scope > 0) && !n->req)
  {
    ip_addr local = ipa_nonzero(n->local) ? n->local : nb->ifa->ip;
    n->req = bfd_request_session(p->p.pool, n->addr, local, nb->iface, NULL, NULL);
  }

  if ((nb->scope <= 0) && n->req)
  {
    rfree(n->req);
    n->req = NULL;
  }
}

static void
bfd_start_neighbor(struct bfd_proto *p, struct bfd_neighbor *n)
{
  n->active = 1;

  if (n->multihop)
  {
    n->req = bfd_request_session(p->p.pool, n->addr, n->local, NULL, NULL, NULL);
    return;
  }

  struct neighbor *nb = neigh_find2(&p->p, &n->addr, n->iface, NEF_STICKY);
  if (!nb)
  {
    log(L_ERR "%s: Invalid remote address %I%J", p->p.name, n->addr, n->iface);
    return;
  }

  if (nb->data)
  {
    log(L_ERR "%s: Duplicate neighbor %I", p->p.name, n->addr);
    return;
  }

  n->neigh = nb;
  nb->data = n;

  if (nb->scope > 0)
    bfd_neigh_notify(nb);
  else
    TRACE(D_EVENTS, "Waiting for %I%J to become my neighbor", n->addr, n->iface);
}

static void
bfd_stop_neighbor(struct bfd_proto *p UNUSED, struct bfd_neighbor *n)
{
  if (n->neigh)
    n->neigh->data = NULL;
  n->neigh = NULL;

  rfree(n->req);
  n->req = NULL;
}

static inline int
bfd_same_neighbor(struct bfd_neighbor *x, struct bfd_neighbor *y)
{
  return ipa_equal(x->addr, y->addr) && ipa_equal(x->local, y->local) &&
    (x->iface == y->iface) && (x->multihop == y->multihop);
}

static void
bfd_reconfigure_neighbors(struct bfd_proto *p, struct bfd_config *new)
{
  struct bfd_config *old = (struct bfd_config *) (p->p.cf);
  struct bfd_neighbor *on, *nn;

  WALK_LIST(on, old->neigh_list)
  {
    WALK_LIST(nn, new->neigh_list)
      if (bfd_same_neighbor(nn, on))
      {
	nn->neigh = on->neigh;
	if (nn->neigh)
	  nn->neigh->data = nn;

	nn->req = on->req;
	nn->active = 1;
	return;
      }

    bfd_stop_neighbor(p, on);
  }

  WALK_LIST(nn, new->neigh_list)
    if (!nn->active)
      bfd_start_neighbor(p, nn);
}


/*
 *	BFD notify socket
 */

/* This core notify code should be replaced after main loop transition to birdloop */

int pipe(int pipefd[2]);
void pipe_drain(int fd);
void pipe_kick(int fd);

static int
bfd_notify_hook(sock *sk, uint len UNUSED)
{
  struct bfd_proto *p = sk->data;
  struct bfd_session *s;
  list tmp_list;
  u8 state, diag;
  node *n, *nn;

  pipe_drain(sk->fd);

  bfd_lock_sessions(p);
  init_list(&tmp_list);
  add_tail_list(&tmp_list, &p->notify_list);
  init_list(&p->notify_list);
  bfd_unlock_sessions(p);

  WALK_LIST_FIRST(s, tmp_list)
  {
    bfd_lock_sessions(p);
    rem_node(&s->n);
    state = s->loc_state;
    diag = s->loc_diag;
    bfd_unlock_sessions(p);

    /* FIXME: convert to btime and move to bfd_session_update_state() */
    s->last_state_change = now;

    s->notify_running = 1;
    WALK_LIST_DELSAFE(n, nn, s->request_list)
      bfd_request_notify(SKIP_BACK(struct bfd_request, n, n), state, diag);
    s->notify_running = 0;

    /* Remove the session if all requests were removed in notify hooks */
    if (EMPTY_LIST(s->request_list))
      bfd_remove_session(p, s);
  }

  return 0;
}

static inline void
bfd_notify_kick(struct bfd_proto *p)
{
  pipe_kick(p->notify_ws->fd);
}

static void
bfd_noterr_hook(sock *sk, int err)
{
  struct bfd_proto *p = sk->data;
  log(L_ERR "%s: Notify socket error: %m", p->p.name, err);
}

static void
bfd_notify_init(struct bfd_proto *p)
{
  int pfds[2];
  sock *sk;

  int rv = pipe(pfds);
  if (rv < 0)
    die("pipe: %m");

  sk = sk_new(p->p.pool);
  sk->type = SK_MAGIC;
  sk->rx_hook = bfd_notify_hook;
  sk->err_hook = bfd_noterr_hook;
  sk->fd = pfds[0];
  sk->data = p;
  if (sk_open(sk) < 0)
    die("bfd: sk_open failed");
  p->notify_rs = sk;

  /* The write sock is not added to any event loop */
  sk = sk_new(p->p.pool);
  sk->type = SK_MAGIC;
  sk->fd = pfds[1];
  sk->data = p;
  sk->flags = SKF_THREAD;
  if (sk_open(sk) < 0)
    die("bfd: sk_open failed");
  p->notify_ws = sk;
}


/*
 *	BFD protocol glue
 */

void
bfd_init_all(void)
{
  init_list(&bfd_proto_list);
  init_list(&bfd_wait_list);
}

static struct proto *
bfd_init(struct proto_config *c)
{
  struct proto *p = proto_new(c, sizeof(struct bfd_proto));

  p->neigh_notify = bfd_neigh_notify;

  return p;
}

static int
bfd_start(struct proto *P)
{
  struct bfd_proto *p = (struct bfd_proto *) P;
  struct bfd_config *cf = (struct bfd_config *) (P->cf);

  p->loop = birdloop_new();
  p->tpool = rp_new(NULL, "BFD thread root");
  pthread_spin_init(&p->lock, PTHREAD_PROCESS_PRIVATE);

  p->session_slab = sl_new(P->pool, sizeof(struct bfd_session));
  HASH_INIT(p->session_hash_id, P->pool, 8);
  HASH_INIT(p->session_hash_ip, P->pool, 8);

  init_list(&p->iface_list);

  init_list(&p->notify_list);
  bfd_notify_init(p);

  add_tail(&bfd_proto_list, &p->bfd_node);

  birdloop_enter(p->loop);
  p->rx_1 = bfd_open_rx_sk(p, 0);
  p->rx_m = bfd_open_rx_sk(p, 1);
  birdloop_leave(p->loop);

  bfd_take_requests(p);

  struct bfd_neighbor *n;
  WALK_LIST(n, cf->neigh_list)
    bfd_start_neighbor(p, n);

  birdloop_start(p->loop);

  return PS_UP;
}


static int
bfd_shutdown(struct proto *P)
{
  struct bfd_proto *p = (struct bfd_proto *) P;
  struct bfd_config *cf = (struct bfd_config *) (P->cf);

  rem_node(&p->bfd_node);

  birdloop_stop(p->loop);

  struct bfd_neighbor *n;
  WALK_LIST(n, cf->neigh_list)
    bfd_stop_neighbor(p, n);

  bfd_drop_requests(p);

  /* FIXME: This is hack */
  birdloop_enter(p->loop);
  rfree(p->tpool);
  birdloop_leave(p->loop);

  birdloop_free(p->loop);

  return PS_DOWN;
}

static int
bfd_reconfigure(struct proto *P, struct proto_config *c)
{
  struct bfd_proto *p = (struct bfd_proto *) P;
  // struct bfd_config *old = (struct bfd_config *) (P->cf);
  struct bfd_config *new = (struct bfd_config *) c;
  struct bfd_iface *ifa;

  birdloop_mask_wakeups(p->loop);

  WALK_LIST(ifa, p->iface_list)
    bfd_reconfigure_iface(p, ifa, new);

  HASH_WALK(p->session_hash_id, next_id, s)
  {
    if (s->ifa->changed)
      bfd_reconfigure_session(p, s);
  }
  HASH_WALK_END;

  bfd_reconfigure_neighbors(p, new);

  birdloop_unmask_wakeups(p->loop);

  return 1;
}

/* Ensure one instance */
struct bfd_config *bfd_cf;

static void
bfd_preconfig(struct protocol *P UNUSED, struct config *c UNUSED)
{
  bfd_cf = NULL;
}

static void
bfd_copy_config(struct proto_config *dest, struct proto_config *src UNUSED)
{
  struct bfd_config *d = (struct bfd_config *) dest;
  // struct bfd_config *s = (struct bfd_config *) src;

  /* We clean up patt_list and neigh_list, neighbors and ifaces are non-sharable */
  init_list(&d->patt_list);
  init_list(&d->neigh_list);
}

void
bfd_show_sessions(struct proto *P)
{
  byte tbuf[TM_DATETIME_BUFFER_SIZE];
  struct bfd_proto *p = (struct bfd_proto *) P;
  uint state, diag UNUSED;
  u32 tx_int, timeout;
  const char *ifname;

  if (p->p.proto_state != PS_UP)
  {
    cli_msg(-1020, "%s: is not up", p->p.name);
    cli_msg(0, "");
    return;
  }

  cli_msg(-1020, "%s:", p->p.name);
  cli_msg(-1020, "%-25s %-10s %-10s %-10s  %8s %8s",
	  "IP address", "Interface", "State", "Since", "Interval", "Timeout");


  HASH_WALK(p->session_hash_id, next_id, s)
  {
    /* FIXME: this is thread-unsafe, but perhaps harmless */
    state = s->loc_state;
    diag = s->loc_diag;
    ifname = (s->ifa && s->ifa->iface) ? s->ifa->iface->name : "---";
    tx_int = s->last_tx ? (MAX(s->des_min_tx_int, s->rem_min_rx_int) TO_MS) : 0;
    timeout = (MAX(s->req_min_rx_int, s->rem_min_tx_int) TO_MS) * s->rem_detect_mult;

    state = (state < 4) ? state : 0;
    tm_format_datetime(tbuf, &config->tf_proto, s->last_state_change);

    cli_msg(-1020, "%-25I %-10s %-10s %-10s  %3u.%03u  %3u.%03u",
	    s->addr, ifname, bfd_state_names[state], tbuf,
	    tx_int / 1000, tx_int % 1000, timeout / 1000, timeout % 1000);
  }
  HASH_WALK_END;

  cli_msg(0, "");
}


struct protocol proto_bfd = {
  .name =		"BFD",
  .template =		"bfd%d",
  .config_size =	sizeof(struct bfd_config),
  .init =		bfd_init,
  .start =		bfd_start,
  .shutdown =		bfd_shutdown,
  .reconfigure =	bfd_reconfigure,
  .preconfig = 		bfd_preconfig,
  .copy_config =	bfd_copy_config,
};
