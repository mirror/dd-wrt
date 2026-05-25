/* ISC license. */

#include "cdb-internal.h"

uint32_t cdb_hashadd (uint32_t h, uint8_t c)
{
  h += (h << 5) ;
  return h ^ c ;
}
