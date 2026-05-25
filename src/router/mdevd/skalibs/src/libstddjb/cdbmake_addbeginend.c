/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/diuint32.h>
#include <skalibs/buffer.h>
#include <skalibs/genalloc.h>
#include "cdbmake-internal.h"

int cdbmake_addend (cdbmaker *c, uint32_t keylen, uint32_t datalen, uint32_t h)
{
  diuint32 blah = { .left = h, .right = c->pos } ;
  return genalloc_append(diuint32, &c->hplist, &blah) && cdbmake_posplus(c, 8) && cdbmake_posplus(c, keylen) && cdbmake_posplus(c, datalen) ;
}

int cdbmake_addbegin (cdbmaker *c, uint32_t keylen, uint32_t datalen)
{
  char buf[8] ;
  uint32_pack(buf, keylen) ;
  uint32_pack(buf + 4, datalen) ;
  return buffer_put(&c->b, buf, 8) == 8 ;
}
