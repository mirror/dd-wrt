/* ISC license. */

#include <skalibs/bitarray.h>

size_t bitarray_firstset (unsigned char const *s, size_t max)
{
  size_t n = bitarray_div8(max) ;
  size_t i = 0 ;
  for (; i < n ; i++) if (s[i]) break ;
  if (i == n) return max ;
  i <<= 3 ;
  while ((i < max) && !bitarray_peek(s, i)) i++ ;
  return i ;
}
