/*
 *	BIRD -- *BSD Table Scanning
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/route.h>

#undef LOCAL_DEBUG
#define LOCAL_DEBUG

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

void
krt_sysctl_scan(struct proto *p, pool *pool, byte **buf, int *bl, int cmd)
{
  byte *next;
  int obl, needed, mib[6], on;
  struct ks_msg *m;

  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = BIRD_PF;
  mib[4] = cmd;
  mib[5] = 0;

  if( sysctl(mib, 6 , NULL , &needed, NULL, 0) < 0)
  {
    die("RT scan...");
  }

  obl = *bl;

  while(needed > *bl) *bl *= 2;
  while(needed < (*bl/2)) *bl /= 2;

  if( (obl!=*bl) || !*buf)
  {
    if(*buf) mb_free(*buf);
    if( (*buf = mb_alloc(pool, *bl)) == NULL ) die("RT scan buf alloc");
  }

  on = needed;

  if( sysctl(mib, 6 , *buf, &needed, NULL, 0) < 0)
  {
    if(on != needed) return; 	/* The buffer size changed since last sysctl */
    die("RT scan 2");
  }

  for (next = *buf; next < (*buf + needed); next += m->rtm.rtm_msglen)
  {
    m = (struct ks_msg *)next;
    krt_read_msg(p, m, 1);
  }
}

void
krt_scan_fire(struct krt_proto *p)
{
  static byte *buf = NULL;
  static int bl = 32768;
  krt_sysctl_scan((struct proto *)p , p->krt_pool, &buf, &bl, NET_RT_DUMP);
}

void
krt_if_scan(struct kif_proto *p)
{
  static byte *buf = NULL;
  static int bl = 4096;
  struct proto *P = (struct proto *)p;
  if_start_update();
  krt_sysctl_scan(P, P->pool, &buf, &bl, NET_RT_IFLIST);
  if_end_update();
}

