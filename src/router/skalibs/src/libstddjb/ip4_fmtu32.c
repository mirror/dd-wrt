/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/fmtscan.h>

size_t ip4_fmtu32 (char *s, uint32_t ip)
{
  char pack[4] ;
  uint32_pack_big(pack, ip) ;
  return ip4_fmt(s, pack) ;
}
