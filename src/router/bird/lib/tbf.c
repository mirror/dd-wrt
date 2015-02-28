/*
 *	BIRD Library -- Token Bucket Filter
 *
 *	(c) 2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"

void
tbf_update(struct tbf *f)
{
  bird_clock_t delta = now - f->timestamp;

  if (delta == 0)
    return;

  f->timestamp = now;

  if ((0 < delta) && (delta < f->burst))
  {
    u32 next = f->count + delta * f->rate;
    f->count = MIN(next, f->burst);
  }
  else
    f->count = f->burst;
}
