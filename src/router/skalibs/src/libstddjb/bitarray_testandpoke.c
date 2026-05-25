/* ISC license. */

#include <skalibs/bitarray.h>

int bitarray_testandpoke (unsigned char *s, size_t n, int h)
{
  unsigned char mask = 1 << (n & 7) ;
  unsigned char c = s[n>>3] ;
  s[n>>3] = h ? c | mask : c & ~mask ;
  return (c & mask) ? 1 : 0 ;
}
