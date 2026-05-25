/* ISC license. */

#include <stdint.h>

#include "cdb-internal.h"

uint32_t cdb_hashv (struct iovec const *v, unsigned int n)
{
  uint32_t h = CDB_HASHSTART ;
  for (unsigned int i = 0 ; i < n ; i++)
    for (size_t j = 0 ; j < v[i].iov_len ; j++)
      h = cdb_hashadd(h, ((uint8_t const *)v[i].iov_base)[j]) ;
  return h ;
}
