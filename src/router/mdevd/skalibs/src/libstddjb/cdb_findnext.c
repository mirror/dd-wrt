/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/uint32.h>
#include <skalibs/cdb.h>
#include "cdb-internal.h"

int cdb_findnext (cdb const *c, cdb_data *out, char const *key, uint32_t len, cdb_find_state *d)
{
  if (!d->loop)
  {
    uint32_t u = cdb_hash(key, len) ;
    char const *p = cdb_p(c, 8, (u << 3) & 2047) ;
    if (!p) return -1 ;
    uint32_unpack(p + 4, &d->hslots) ;
    if (!d->hslots) return 0 ;
    uint32_unpack(p, &d->hpos) ;
    d->khash = u ;
    u >>= 8 ;
    u %= d->hslots ;
    u <<= 3 ;
    d->kpos = d->hpos + u ;
  }

  while (d->loop < d->hslots)
  {
    uint32_t pos, u ;
    char const *p = cdb_p(c, 8, d->kpos) ;
    if (!p) return -1 ;
    uint32_unpack(p + 4, &pos) ;
    if (!pos) return 0 ;
    d->loop++ ;
    d->kpos += 8 ;
    if (d->kpos == d->hpos + (d->hslots << 3)) d->kpos = d->hpos ;
    uint32_unpack(p, &u) ;
    if (u == d->khash)
    {
      p = cdb_p(c, 8, pos) ;
      if (!p) return -1 ;
      uint32_unpack(p, &u) ;
      if (u == len)
      {
        char const *k = cdb_p(c, len, pos + 8) ;
        if (!k) return -1 ;
        if (!memcmp(key, k, len))
        {
          uint32_unpack(p + 4, &out->len) ;
          out->s = c->map + pos + 8 + len ;
          return 1 ;
        }
      }
    }
  }
  return 0 ;
}
