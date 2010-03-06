/*
 *	BIRD -- Linux Routing Table Scanning
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/route.h>

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "lib/timer.h"
#include "lib/unix.h"
#include "lib/krt.h"
#include "lib/string.h"

static int krt_scan_fd = -1;

struct iface *
krt_temp_iface(struct krt_proto *p, char *name)
{
  struct iface *i;

  WALK_LIST(i, p->scan.temp_ifs)
    if (!strcmp(i->name, name))
      return i;
  i = mb_allocz(p->p.pool, sizeof(struct iface));
  strcpy(i->name, name);
  add_tail(&p->scan.temp_ifs, &i->n);
  return i;
}

static void
krt_parse_entry(byte *ent, struct krt_proto *p)
{
  u32 dest0, gw0, mask0;
  ip_addr dest, gw, mask;
  unsigned int flags;
  int masklen;
  net *net;
  byte *iface = ent;
  rta a;
  rte *e;

  if (sscanf(ent, "%*s\t%x\t%x\t%x\t%*d\t%*d\t%*d\t%x\t", &dest0, &gw0, &flags, &mask0) != 4)
    {
      log(L_ERR "krt read: unable to parse `%s'", ent);
      return;
    }
  while (*ent != '\t')
    ent++;
  *ent = 0;

  dest = ipa_from_u32(dest0);
  ipa_ntoh(dest);
  gw = ipa_from_u32(gw0);
  ipa_ntoh(gw);
  mask = ipa_from_u32(mask0);
  ipa_ntoh(mask);
  if ((masklen = ipa_mklen(mask)) < 0)
    {
      log(L_ERR "krt read: invalid netmask %08x", mask0);
      return;
    }
  DBG("Got %I/%d via %I flags %x\n", dest, masklen, gw, flags);

  if (!(flags & RTF_UP))
    {
      DBG("Down.\n");
      return;
    }
  if (flags & RTF_HOST)
    masklen = 32;
  if (flags & (RTF_DYNAMIC | RTF_MODIFIED)) /* Redirect route */
    {
      log(L_WARN "krt: Ignoring redirect to %I/%d via %I", dest, masklen, gw);
      return;
    }

  net = net_get(p->p.table, dest, masklen);

  a.proto = &p->p;
  a.source = RTS_INHERIT;
  a.scope = SCOPE_UNIVERSE;
  a.cast = RTC_UNICAST;
  a.flags = a.aflags = 0;
  a.from = IPA_NONE;
  a.iface = NULL;
  a.eattrs = NULL;

  if (flags & RTF_GATEWAY)
    {
      neighbor *ng = neigh_find(&p->p, &gw, 0);
      if (ng && ng->scope)
	a.iface = ng->iface;
      else
	{
	  log(L_WARN "Kernel told us to use non-neighbor %I for %I/%d", gw, net->n.prefix, net->n.pxlen);
	  return;
	}
      a.dest = RTD_ROUTER;
      a.gw = gw;
    }
  else if (flags & RTF_REJECT)
    {
      a.dest = RTD_UNREACHABLE;
      a.gw = IPA_NONE;
    }
  else if (isalpha(iface[0]))
    {
      a.dest = RTD_DEVICE;
      a.gw = IPA_NONE;
      a.iface = krt_temp_iface(p, iface);
    }
  else
    {
      log(L_WARN "Kernel reporting unknown route type to %I/%d", net->n.prefix, net->n.pxlen);
      return;
    }

  e = rte_get_temp(&a);
  e->net = net;
  e->u.krt.src = KRT_SRC_UNKNOWN;
  krt_got_route(p, e);
}

void
krt_scan_fire(struct krt_proto *p)
{
  byte buf[32768];
  int l, seen_hdr;

  if (krt_scan_fd < 0)
    {
      krt_scan_fd = open("/proc/net/route", O_RDONLY);
      if (krt_scan_fd < 0)
	die("/proc/net/route: %m");
    }
  else if (lseek(krt_scan_fd, 0, SEEK_SET) < 0)
    {
      log(L_ERR "krt seek: %m");
      return;
    }
  seen_hdr = 0;
  while ((l = read(krt_scan_fd, buf, sizeof(buf))) > 0)
    {
      byte *z = buf;
      if (l & 127)
	{
	  log(L_ERR "krt read: misaligned entry: l=%d", l);
	  return;
	}
      while (l >= 128)
	{
	  if (seen_hdr++)
	    krt_parse_entry(z, p);
	  z += 128;
	  l -= 128;
	}
    }
  if (l < 0)
    {
      log(L_ERR "krt read: %m");
      return;
    }
  DBG("KRT scan done, seen %d lines\n", seen_hdr);
}

void
krt_scan_construct(struct krt_config *c)
{
}

void
krt_scan_preconfig(struct config *c)
{
}

void
krt_scan_postconfig(struct krt_config *c)
{
}

void
krt_scan_start(struct krt_proto *x, int first)
{
  init_list(&x->scan.temp_ifs);
}

void
krt_scan_shutdown(struct krt_proto *x, int last)
{
}
