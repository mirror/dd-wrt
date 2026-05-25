/* ISC license. */

#include <string.h>

#include <skalibs/fmtscan.h>
#include <skalibs/ip46.h>

size_t ip46full_scanlist (ip46full *out, size_t max, char const *s, size_t *num)
{
  size_t n = 0, w = 0 ;
  for (; s[w] && (n < max) ; n++)
  {
    ip46full z ;
    size_t i = ip6_scan(s + w, z.ip) ;
    if (i) z.is6 = 1 ;
    else
    {
      i = ip4_scan(s + w, z.ip) ;
      if (!i) break ;
      z.is6 = 0 ;
    }
    out[n] = z ;
    w += i ;
    while (memchr(",; \t\r\n", s[w], 6)) w++ ;
  }
  *num = n ;
  return w ;
}
