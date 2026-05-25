/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/tai.h>

void tain_pack (char *s, tain const *t)
{
  tai_pack(s, &t->sec) ;
  uint32_pack_big(s+8, t->nano) ;
}
