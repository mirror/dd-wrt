/*
 *	BIRD -- Table-to-Table Routing Protocol a.k.a Pipe
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Pipe
 *
 * The Pipe protocol is very simple. It just connects to two routing tables
 * using proto_add_announce_hook() and whenever it receives a rt_notify()
 * about a change in one of the tables, it converts it to a rte_update()
 * in the other one.
 *
 * To avoid pipe loops, Pipe keeps a `being updated' flag in each routing
 * table.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "conf/conf.h"
#include "filter/filter.h"
#include "lib/string.h"

#include "pipe.h"

static void
pipe_send(struct pipe_proto *p, rtable *src_table, rtable *dest, net *n, rte *new, rte *old, ea_list *attrs)
{
  struct proto *src;
  net *nn;
  rte *e;
  rta a;

  if (!new && !old)
    return;

  if (dest->pipe_busy)
    {
      log(L_ERR "Pipe loop detected when sending %I/%d to table %s",
	  n->n.prefix, n->n.pxlen, dest->name);
      return;
    }
  nn = net_get(dest, n->n.prefix, n->n.pxlen);
  if (new)
    {
      memcpy(&a, new->attrs, sizeof(rta));

      if (p->mode == PIPE_OPAQUE)
	{
	  a.proto = &p->p;
	  a.source = RTS_PIPE;
	}

      a.aflags = 0;
      a.eattrs = attrs;
      e = rte_get_temp(&a);
      e->net = nn;
      e->pflags = 0;

      if (p->mode == PIPE_TRANSPARENT)
	{
	  /* Copy protocol specific embedded attributes. */
	  memcpy(&(e->u), &(new->u), sizeof(e->u));
	  e->pref = new->pref;
	  e->pflags = new->pflags;
	}

      src = new->attrs->proto;
    }
  else
    {
      e = NULL;
      src = old->attrs->proto;
    }

  src_table->pipe_busy = 1;
  rte_update(dest, nn, &p->p, (p->mode == PIPE_OPAQUE) ? &p->p : src, e);
  src_table->pipe_busy = 0;
}

static void
pipe_rt_notify_pri(struct proto *P, net *net, rte *new, rte *old, ea_list *attrs)
{
  struct pipe_proto *p = (struct pipe_proto *) P;

  DBG("PIPE %c> %I/%d\n", (new ? '+' : '-'), net->n.prefix, net->n.pxlen);
  pipe_send(p, p->p.table, p->peer, net, new, old, attrs);
}

static void
pipe_rt_notify_sec(struct proto *P, net *net, rte *new, rte *old, ea_list *attrs)
{
  struct pipe_proto *p = ((struct pipe_proto *) P)->phantom;

  DBG("PIPE %c< %I/%d\n", (new ? '+' : '-'), net->n.prefix, net->n.pxlen);
  pipe_send(p, p->peer, p->p.table, net, new, old, attrs);
}

static int
pipe_import_control(struct proto *P, rte **ee, ea_list **ea UNUSED, struct linpool *p UNUSED)
{
  struct proto *pp = (*ee)->sender;

  if (pp == P || pp == &((struct pipe_proto *) P)->phantom->p)
    return -1;	/* Avoid local loops automatically */
  return 0;
}

static int
pipe_reload_routes(struct proto *P)
{
  /*
   * Because the pipe protocol feeds routes from both routing tables
   * together, both directions are reloaded during refeed and 'reload
   * out' command works like 'reload' command. For symmetry, we also
   * request refeed when 'reload in' command is used.
   */
  proto_request_feeding(P);
  return 1;
}

static int
pipe_start(struct proto *P)
{
  struct pipe_proto *p = (struct pipe_proto *) P;
  struct pipe_proto *ph;
  struct announce_hook *a;

  /*
   *  Create a phantom protocol which will represent the remote
   *  end of the pipe (we need to do this in order to get different
   *  filters and announce functions and it unfortunately involves
   *  a couple of magic trickery).
   */
  ph = mb_alloc(P->pool, sizeof(struct pipe_proto));
  memcpy(ph, p, sizeof(struct pipe_proto));
  p->phantom = ph;
  ph->phantom = p;
  ph->p.accept_ra_types = (p->mode == PIPE_OPAQUE) ? RA_OPTIMAL : RA_ANY;
  ph->p.rt_notify = pipe_rt_notify_sec;
  ph->p.proto_state = PS_UP;
  ph->p.core_state = ph->p.core_goal = FS_HAPPY;

  /*
   *  Routes should be filtered in the do_rte_announce() (export
   *  filter for protocols). Reverse direction is handled by putting
   *  specified import filter to out_filter field of the phantom
   *  protocol.
   *
   *  in_filter fields are not important, there is an exception in
   *  rte_update() to ignore it for pipes. We cannot just set
   *  P->in_filter to FILTER_ACCEPT, because that would break other
   *  things (reconfiguration, show-protocols command).
   */
  ph->p.in_filter = FILTER_ACCEPT;
  ph->p.out_filter = P->in_filter;

  /*
   *  Connect the phantom protocol to the peer routing table, but
   *  keep it in the list of connections of the primary protocol,
   *  so that it gets disconnected at the right time and we also
   *  get all routes from both sides during the feeding phase.
   */
  a = proto_add_announce_hook(P, p->peer);
  a->proto = &ph->p;
  rt_lock_table(p->peer);

  return PS_UP;
}

static int
pipe_shutdown(struct proto *P)
{
  struct pipe_proto *p = (struct pipe_proto *) P;

  rt_unlock_table(p->peer);
  return PS_DOWN;
}

static struct proto *
pipe_init(struct proto_config *C)
{
  struct pipe_config *c = (struct pipe_config *) C;
  struct proto *P = proto_new(C, sizeof(struct pipe_proto));
  struct pipe_proto *p = (struct pipe_proto *) P;

  p->peer = c->peer->table;
  p->mode = c->mode;
  P->accept_ra_types = (p->mode == PIPE_OPAQUE) ? RA_OPTIMAL : RA_ANY;
  P->rt_notify = pipe_rt_notify_pri;
  P->import_control = pipe_import_control;
  P->reload_routes = pipe_reload_routes;
  return P;
}

static void
pipe_postconfig(struct proto_config *C)
{
  struct pipe_config *c = (struct pipe_config *) C;

  if (!c->peer)
    cf_error("Name of peer routing table not specified");
  if (c->peer == C->table)
    cf_error("Primary table and peer table must be different");
}

static void
pipe_get_status(struct proto *P, byte *buf)
{
  struct pipe_proto *p = (struct pipe_proto *) P;

  bsprintf(buf, "%c> %s", (p->mode == PIPE_OPAQUE) ? '-' : '=', p->peer->name);
}

static int
pipe_reconfigure(struct proto *P, struct proto_config *new)
{
  struct pipe_proto *p = (struct pipe_proto *) P;
  struct pipe_config *o = (struct pipe_config *) P->cf;
  struct pipe_config *n = (struct pipe_config *) new;

  if ((o->peer->table != n->peer->table) || (o->mode != n->mode))
    return 0;

  /* Update also the filter in the phantom protocol */
  p->phantom->p.out_filter = new->in_filter;
  return 1;
}

struct protocol proto_pipe = {
  name:		"Pipe",
  template:	"pipe%d",
  postconfig:	pipe_postconfig,
  init:		pipe_init,
  start:	pipe_start,
  shutdown:	pipe_shutdown,
  reconfigure:	pipe_reconfigure,
  get_status:	pipe_get_status,
};
