/* ISC license. */

#include <string.h>
#include <skalibs/fmtscan.h>

size_t ip6_scanlist (char *out, size_t max, char const *s, size_t *num)
{
  size_t n = 0, w = 0 ;
  for (; s[w] && (n < max) ; n++)
  {
    char ip[16] ;
    size_t i = ip6_scan(s + w, ip) ;
    if (!i) break ;
    memcpy(out + (n << 4), ip, 16) ;
    w += i ;
    while (memchr(",; \t\r\n", s[w], 6)) w++ ;
  }
  *num = n ;
  return w ;
}
