/* ISC license. */

#include <skalibs/cdb.h>
#include "cdb-internal.h"

char const *cdb_p (cdb const *c, uint32_t len, uint32_t pos)
{
  return pos <= c->size && len <= c->size - pos ? c->map + pos : 0 ;
}
