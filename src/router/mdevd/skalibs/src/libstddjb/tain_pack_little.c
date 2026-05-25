/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/tai.h>

void tain_pack_little (char *s, tain const *t)
{
  uint32_pack(s, t->nano) ;
  tai_pack_little(s+4, &t->sec) ;
}
