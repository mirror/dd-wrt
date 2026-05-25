/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/tai.h>

void tai_pack (char *s, tai const *t)
{
  uint64_pack_big(s, tai_sec(t)) ;
}
