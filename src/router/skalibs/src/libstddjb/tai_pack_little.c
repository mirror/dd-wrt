/* ISC license. */

#include <skalibs/uint64.h>
#include <skalibs/tai.h>

void tai_pack_little (char *s, tai const *t)
{
  uint64_pack(s, tai_sec(t)) ;
}
