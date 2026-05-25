/* ISC license. */

#include <skalibs/bitarray.h>

#define B0(n) n, n+1, n+1, n+2
#define B1(n) B0(n), B0(n+1), B0(n+1), B0(n+2)
#define B2(n) B1(n), B1(n+1), B1(n+1), B1(n+2)
#define B3(n) B2(n), B2(n+1), B2(n+1), B2(n+2)

size_t bitarray_countones (unsigned char const *c, size_t n)
{
  static unsigned char const table[256] = { B3(0) } ;
  size_t len = bitarray_div8(n) ;
  size_t total = 0 ;
  size_t i = 0 ;
  if (n & 7) len-- ;
  for (; i < len ; i++) total += table[c[i]] ;
  if (n & 7) total += table[c[i] & ((1 << (n & 7)) - 1)] ;
  return total ;
}
