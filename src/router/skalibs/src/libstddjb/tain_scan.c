/* ISC license. */

#include <skalibs/fmtscan.h>
#include <skalibs/tai.h>

size_t tain_scan (char const *s, tain *a)
{
  char pack[TAIN_PACK] ;
  size_t r = ucharn_scan(s, pack, TAIN_PACK) ;
  if (r) tain_unpack(pack, a) ;
  return r ;
}
