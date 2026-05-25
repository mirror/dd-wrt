/* ISC license. */

#include <skalibs/cdb.h>

int cdb_find (cdb const *c, cdb_data *out, char const *key, uint32_t len)
{
  cdb_find_state cfs = CDB_FIND_STATE_ZERO ;
  return cdb_findnext(c, out, key, len, &cfs) ;
}
