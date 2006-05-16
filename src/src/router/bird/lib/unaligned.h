/*
 *	Unaligned Data Accesses -- Generic Version, Network Order
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_UNALIGNED_H_
#define _BIRD_UNALIGNED_H_

/*
 *  We don't do any clever tricks with unaligned accesses since it's
 *  virtually impossible to figure out what alignment does the CPU want
 *  (unaligned accesses can be emulated by the OS which makes them work,
 *  but unusably slow). We use memcpy and hope GCC will optimize it out
 *  if possible.
 */

#include "lib/string.h"

static inline u16
get_u16(void *p)
{
  u16 x;
  memcpy(&x, p, 2);
  return ntohs(x);
}

static inline u32
get_u32(void *p)
{
  u32 x;
  memcpy(&x, p, 4);
  return ntohl(x);
}

static inline void
put_u16(void *p, u16 x)
{
  x = htons(x);
  memcpy(p, &x, 2);
}

static inline void
put_u32(void *p, u32 x)
{
  x = htonl(x);
  memcpy(p, &x, 4);
}

#endif
