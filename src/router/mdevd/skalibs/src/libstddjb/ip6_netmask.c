/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>
#include <skalibs/fmtscan.h>

static void apply (char *half, uint8_t netmask)
{
  uint64_t u ;
  uint64_unpack_big(half, &u) ;
  u &= netmask ? ~((1ull << 64 - netmask) - 1) : 0 ;
  uint64_pack_big(half, u) ;
}

int ip6_netmask (char *ip, uint8_t netmask)
{
  if (netmask > 128) return 0 ;
  if (netmask <= 64)
  {
    apply(ip, netmask) ;
    memset(ip + 8, 0, 8) ;
  }
  else apply(ip + 8, netmask - 64) ;
  return 1 ;
}
