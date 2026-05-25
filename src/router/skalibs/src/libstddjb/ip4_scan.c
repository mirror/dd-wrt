/* ISC license. */

#include <stdint.h>

#include <skalibs/uint16.h>
#include <skalibs/fmtscan.h>

size_t ip4_scan (char const *s, char *ip)
{
  size_t len = 0 ;
  uint16_t u ;
  for (unsigned int j = 0 ; j < 4 ; j++)
  {
    size_t i = uint16_scan(s + len, &u) ;
    if (!i || u > 0xffu) return 0 ;
    ip[j] = u ;
    len += i ;
    if (j == 3) break ;
    if (s[len++] != '.') return 0 ;
  }
  return len ;
}
