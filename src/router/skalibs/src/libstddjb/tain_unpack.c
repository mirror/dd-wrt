/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/tai.h>

void tain_unpack (char const *s, tain *t)
{
  tai_unpack(s, &t->sec) ;
  uint32_unpack_big(s+8, &t->nano) ;
}
