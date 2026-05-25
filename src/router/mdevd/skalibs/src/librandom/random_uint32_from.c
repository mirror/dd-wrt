/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/functypes.h>
#include <skalibs/random.h>

uint32_t random_uint32_from (uint32_t n, randomgen_func_ref f)
{
  uint32_t min, x ;
  if (n < 2) return 0 ;
  min = -n % n ;
  for (;;)
  {
    char tmp[4] ;
    (*f)(tmp, 4) ;
    uint32_unpack_big(tmp, &x) ;
    if (x >= min) break ;
  }
  return x % n ;
}
