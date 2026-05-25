/* ISC license. */

#include <stddef.h>

#include <skalibs/fmtscan.h>
#include <skalibs/ip46.h>

size_t ip46full_scan (char const *s, ip46full *ip)
{
  size_t len = ip6_scan(s, ip->ip) ;
  if (len) ip->is6 = 1 ;
  else
  {
    len = ip4_scan(s, ip->ip) ;
    if (len) ip->is6 = 0 ;
  }
  return len ;
}
