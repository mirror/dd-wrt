/*
 *	BIRD -- Unix Routing Table Syncing
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/route.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "lib/unix.h"
#include "lib/krt.h"
#include "lib/string.h"

int
krt_capable(rte *e)
{
  rta *a = e->attrs;

#ifdef CONFIG_AUTO_ROUTES
  if (a->source == RTS_DEVICE)
    return 0;
#endif
  return
    a->cast == RTC_UNICAST &&
    (a->dest == RTD_ROUTER
     || a->dest == RTD_DEVICE
#ifdef RTF_REJECT
     || a->dest == RTD_UNREACHABLE
#endif
     );
}

static void
krt_ioctl(int ioc, rte *e, char *name)
{
  net *net = e->net;
  struct rtentry re;
  rta *a = e->attrs;

  bzero(&re, sizeof(re));
  fill_in_sockaddr((struct sockaddr_in *) &re.rt_dst, net->n.prefix, 0);
  fill_in_sockaddr((struct sockaddr_in *) &re.rt_genmask, ipa_mkmask(net->n.pxlen), 0);
  re.rt_flags = RTF_UP;
  if (net->n.pxlen == 32)
    re.rt_flags |= RTF_HOST;
  switch (a->dest)
    {
    case RTD_ROUTER:
      fill_in_sockaddr((struct sockaddr_in *) &re.rt_gateway, a->gw, 0);
      re.rt_flags |= RTF_GATEWAY;
      break;
    case RTD_DEVICE:
      if (!a->iface)
	return;
      re.rt_dev = a->iface->name;
      break;
#ifdef RTF_REJECT
    case RTD_UNREACHABLE:
      re.rt_flags |= RTF_REJECT;
      break;
#endif
    default:
      bug("krt set: unknown flags, but not filtered");
    }

  if (ioctl(if_scan_sock, ioc, &re) < 0)
    log(L_ERR "%s(%I/%d): %m", name, net->n.prefix, net->n.pxlen);
}

void
krt_set_notify(struct krt_proto *p, net *net, rte *new, rte *old)
{
  if (old)
    {
      DBG("krt_remove_route(%I/%d)\n", net->n.prefix, net->n.pxlen);
      krt_ioctl(SIOCDELRT, old, "SIOCDELRT");
    }
  if (new)
    {
      DBG("krt_add_route(%I/%d)\n", net->n.prefix, net->n.pxlen);
      krt_ioctl(SIOCADDRT, new, "SIOCADDRT");
    }
}

void
krt_set_start(struct krt_proto *x, int first)
{
  if (if_scan_sock < 0)
    bug("krt set: missing socket");
}

void
krt_set_construct(struct krt_config *c)
{
}

void
krt_set_shutdown(struct krt_proto *x, int last)
{
} 
