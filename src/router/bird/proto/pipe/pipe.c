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
pipe_rt_notify(struct proto *P, rtable *src_table, net *n, rte *new, rte *old, ea_list *attrs)
{
  struct pipe_proto *p = (struct pipe_proto *) P;
  rtable *dest = (src_table == P->table) ? p->peer : P->table; /* The other side of the pipe */
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
      a.hostentry = NULL;
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

static int
pipe_import_control(struct proto *P, rte **ee, ea_list **ea UNUSED, struct linpool *p UNUSED)
{
  struct proto *pp = (*ee)->sender;

  if (pp == P)
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
  struct announce_hook *a;

  /* Clean up the secondary stats */
  bzero(&p->peer_stats, sizeof(struct proto_stats));

  /* Lock the peer table, unlock is handled in pipe_cleanup() */
  rt_lock_table(p->peer);

  /* Connect the protocol also to the peer routing table. */
  a = proto_add_announce_hook(P, p->peer);

  return PS_UP;
}

static void
pipe_cleanup(struct proto *P)
{
  struct pipe_proto *p = (struct pipe_proto *) P;
  rt_unlock_table(p->peer);
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
  P->rt_notify = pipe_rt_notify;
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

static int
pipe_reconfigure(struct proto *P, struct proto_config *new)
{
  // struct pipe_proto *p = (struct pipe_proto *) P;
  struct pipe_config *o = (struct pipe_config *) P->cf;
  struct pipe_config *n = (struct pipe_config *) new;

  if ((o->peer->table != n->peer->table) || (o->mode != n->mode))
    return 0;

  return 1;
}

static void
pipe_copy_config(struct proto_config *dest, struct proto_config *src)
{
  /* Just a shallow copy, not many items here */
  proto_copy_rest(dest, src, sizeof(struct pipe_config));
}

static void
pipe_get_status(struct proto *P, byte *buf)
{
  struct pipe_proto *p = (struct pipe_proto *) P;

  bsprintf(buf, "%c> %s", (p->mode == PIPE_OPAQUE) ? '-' : '=', p->peer->name);
}


struct protocol proto_pipe = {
  name:		"Pipe",
  template:	"pipe%d",
  preference:	DEF_PREF_PIPE,
  postconfig:	pipe_postconfig,
  init:		pipe_init,
  start:	pipe_start,
  cleanup:	pipe_cleanup,
  reconfigure:	pipe_reconfigure,
  copy_config:  pipe_copy_config,
  get_status:	pipe_get_status,
};
