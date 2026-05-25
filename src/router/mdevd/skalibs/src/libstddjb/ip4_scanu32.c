/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/fmtscan.h>

size_t ip4_scanu32 (char const *s, uint32_t *ip)
{
  char pack[4] ;
  size_t r = ip4_scan(s, pack) ;
  if (r) uint32_unpack_big(pack, ip) ;
  return r ;
}
