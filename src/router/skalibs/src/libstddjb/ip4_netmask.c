/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/fmtscan.h>

int ip4_netmask (char *ip, uint8_t netmask)
{
  uint32_t u ;
  if (netmask > 32) return 0 ;
  uint32_unpack_big(ip, &u) ;
  u &= netmask ? ~((1u << 32 - netmask) - 1) : 0 ;
  uint32_pack_big(ip, u) ;
  return 1 ;
}
