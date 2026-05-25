/* ISC license. */

#include <errno.h>
#include <sys/uio.h>

#include <skalibs/uint32.h>
#include <skalibs/diuint32.h>
#include <skalibs/buffer.h>
#include <skalibs/genalloc.h>
#include <skalibs/siovec.h>
#include <skalibs/cdbmake.h>
#include "cdb-internal.h"
#include "cdbmake-internal.h"

int cdbmake_addv (cdbmaker *c, struct iovec const *kv, unsigned int kn, struct iovec const *dv, unsigned int dn)
{
  size_t keylen = siovec_len(kv, kn) ;
  size_t datalen = siovec_len(dv, dn) ;
  if (keylen > UINT32_MAX || datalen > UINT32_MAX) return (errno = EOVERFLOW, 0) ;

  if (!cdbmake_addbegin(c, keylen, datalen)
   || buffer_putv(&c->b, kv, kn) < keylen
   || buffer_putv(&c->b, dv, dn) < datalen
   || !cdbmake_addend(c, keylen, datalen, cdb_hashv(kv, kn)))
  {
    genalloc_free(diuint32, &c->hplist) ;
    return 0 ;
  }
  return 1 ;
}
