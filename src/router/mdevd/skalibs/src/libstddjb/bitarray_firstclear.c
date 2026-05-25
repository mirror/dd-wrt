/* ISC license. */

#include <skalibs/bitarray.h>

size_t bitarray_firstclear (unsigned char const *s, size_t max)
{
  size_t n = bitarray_div8(max) ;
  size_t i = 0 ;
  for (; i < n ; i++) if (s[i] != 0xffU) break ;
  if (i == n) return max ;
  i <<= 3 ;
  while ((i < max) && bitarray_peek(s, i)) i++ ;
  return i ;
}
