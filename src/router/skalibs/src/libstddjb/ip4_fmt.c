/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/fmtscan.h>

size_t ip4_fmt (char *s, char const *ip)
{
  size_t len = 0 ;
  unsigned int j = 0 ;
  for (; j < 4 ; j++)
  {
    size_t i = uint32_fmt(s, (unsigned char)ip[j]) ;
    len += i ;
    if (s) s += i ;
    if (j == 3) break ;
    if (s) *s++ = '.' ;
    ++len ;
  }
  return len ;
}
