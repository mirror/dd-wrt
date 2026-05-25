/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include "cdbmake-internal.h"

int cdbmake_posplus (cdbmaker *c, uint32_t len)
{
  uint32_t newpos = c->pos + len ;
  if (newpos < len) return (errno = ENOMEM, 0) ;
  c->pos = newpos ;
  return 1 ;
}
