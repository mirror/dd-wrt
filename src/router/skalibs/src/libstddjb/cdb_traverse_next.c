/* ISC license. */

#include <stdint.h>

#include <skalibs/uint32.h>
#include <skalibs/cdb.h>
#include "cdb-internal.h"

int cdb_traverse_next (cdb const *c, cdb_data *key, cdb_data *data, uint32_t *pos)
{
  uint32_t eod ;
  char const *p = cdb_p(c, 4, 0) ;
  if (!p) return -1 ;
  uint32_unpack(p, &eod) ;
  if (eod < 8 || eod - 8 < *pos) return 0 ;
  if (*pos + 8 < *pos) return -1 ;
  p = cdb_p(c, 8, *pos) ;
  if (!p) return -1 ;
  uint32_unpack(p, &key->len) ;
  uint32_unpack(p + 4, &data->len) ;
  key->s = c->map + *pos + 8 ;
  data->s = key->s + key->len ;
  *pos += 8 + key->len + data->len ;
  return 1 ;
}
