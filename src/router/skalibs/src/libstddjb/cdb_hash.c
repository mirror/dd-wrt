/* ISC license. */

#include <stdint.h>

#include "cdb-internal.h"

uint32_t cdb_hash (char const *buf, uint32_t len)
{
  uint32_t h = CDB_HASHSTART ;
  while (len--) h = cdb_hashadd(h, *buf++) ;
  return h ;
}
