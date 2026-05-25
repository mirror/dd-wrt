/* ISC license. */

#include <skalibs/bitarray.h>

void bitarray_xor (unsigned char *c, unsigned char const *a, unsigned char const *b, size_t n)
{
  size_t i = bitarray_div8(n) ;
  while (i--) c[i] = a[i] ^ b[i] ;
}
