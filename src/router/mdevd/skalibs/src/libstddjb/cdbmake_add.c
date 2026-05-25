/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/diuint32.h>
#include <skalibs/buffer.h>
#include <skalibs/genalloc.h>
#include <skalibs/cdbmake.h>
#include "cdb-internal.h"
#include "cdbmake-internal.h"

int cdbmake_add (cdbmaker *c, char const *key, uint32_t keylen, char const *data, uint32_t datalen)
{
  if (!cdbmake_addbegin(c, keylen, datalen)
   || buffer_put(&c->b, key, keylen) < keylen
   || buffer_put(&c->b, data, datalen) < datalen
   || !cdbmake_addend(c, keylen, datalen, cdb_hash(key, keylen)))
  {
    genalloc_free(diuint32, &c->hplist) ;
    return 0 ;
  }
  return 1 ;
}
