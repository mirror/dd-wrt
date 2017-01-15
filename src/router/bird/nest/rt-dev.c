/*
 *	BIRD -- Direct Device Routes
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Direct
 *
 * The Direct protocol works by converting all ifa_notify() events it receives
 * to rte_update() calls for the corresponding network.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/rt-dev.h"
#include "conf/conf.h"
#include "lib/resource.h"
#include "lib/string.h"

static void
dev_ifa_notify(struct proto *p, unsigned c, struct ifa *ad)
{
  struct rt_dev_config *P = (void *) p->cf;

  if (!EMPTY_LIST(P->iface_list) &&
      !iface_patt_find(&P->iface_list, ad->iface, ad->iface->addr))
    /* Empty list is automagically treated as "*" */
    return;

  if (ad->flags & IA_SECONDARY)
    return;

  if (ad->scope <= SCOPE_LINK)
    return;

  if (c & IF_CHANGE_DOWN)
    {
      net *n;

      DBG("dev_if_notify: %s:%I going down\n", ad->iface->name, ad->ip);
      n = net_find(p->table, ad->prefix, ad->pxlen);
      if (!n)
	{
	  DBG("dev_if_notify: device shutdown: prefix not found\n");
	  return;
	}

      /* Use iface ID as local source ID */
      struct rte_src *src = rt_get_source(p, ad->iface->index);
      rte_update2(p->main_ahook, n, NULL, src);
    }
  else if (c & IF_CHANGE_UP)
    {
      rta *a;
      net *n;
      rte *e;

      DBG("dev_if_notify: %s:%I going up\n", ad->iface->name, ad->ip);

      if (P->check_link && !(ad->iface->flags & IF_LINK_UP))
	return;

      /* Use iface ID as local source ID */
      struct rte_src *src = rt_get_source(p, ad->iface->index);

      rta a0 = {
	.src = src,
	.source = RTS_DEVICE,
	.scope = SCOPE_UNIVERSE,
	.cast = RTC_UNICAST,
	.dest = RTD_DEVICE,
	.iface = ad->iface
      };

      a = rta_lookup(&a0);
      n = net_get(p->table, ad->prefix, ad->pxlen);
      e = rte_get_temp(a);
      e->net = n;
      e->pflags = 0;
      rte_update2(p->main_ahook, n, e, src);
    }
}

static void
dev_if_notify(struct proto *p, uint c, struct iface *iface)
{
  struct rt_dev_config *cf = (void *) p->cf;

  if (c & (IF_CHANGE_UP | IF_CHANGE_DOWN))
    return;

  if ((c & IF_CHANGE_LINK) && cf->check_link)
  {
    uint ac = (iface->flags & IF_LINK_UP) ? IF_CHANGE_UP : IF_CHANGE_DOWN;

    struct ifa *a;
    WALK_LIST(a, iface->addrs)
      dev_ifa_notify(p, ac, a);
  }
}


static struct proto *
dev_init(struct proto_config *c)
{
  struct proto *p = proto_new(c, sizeof(struct proto));

  p->if_notify = dev_if_notify;
  p->ifa_notify = dev_ifa_notify;
  return p;
}

static int
dev_reconfigure(struct proto *p, struct proto_config *new)
{
  struct rt_dev_config *o = (struct rt_dev_config *) p->cf;
  struct rt_dev_config *n = (struct rt_dev_config *) new;

  return iface_patts_equal(&o->iface_list, &n->iface_list, NULL) &&
    (o->check_link == n->check_link);
}

static void
dev_copy_config(struct proto_config *dest, struct proto_config *src)
{
  struct rt_dev_config *d = (struct rt_dev_config *) dest;
  struct rt_dev_config *s = (struct rt_dev_config *) src;

  /*
   * We copy iface_list as ifaces can be shared by more direct protocols.
   * Copy suffices to be is shallow, because new nodes can be added, but
   * old nodes cannot be modified (although they contain internal lists).
   */
  cfg_copy_list(&d->iface_list, &s->iface_list, sizeof(struct iface_patt));

  d->check_link = s->check_link;
}

struct protocol proto_device = {
  .name = 		"Direct",
  .template = 		"direct%d",
  .preference = 	DEF_PREF_DIRECT,
  .config_size =	sizeof(struct rt_dev_config),
  .init = 		dev_init,
  .reconfigure = 	dev_reconfigure,
  .copy_config = 	dev_copy_config
};
