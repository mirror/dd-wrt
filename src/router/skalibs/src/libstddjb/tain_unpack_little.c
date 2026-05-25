/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/tai.h>

void tain_unpack_little (char const *s, tain *t)
{
  uint32_unpack(s, &t->nano) ;
  tai_unpack_little(s+4, &t->sec) ;
}
