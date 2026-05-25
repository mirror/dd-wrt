/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/random.h>

void random_unsort_from (char *s, size_t n, size_t chunksize, randomgen_func_ref f)
{
  char tmp[chunksize] ;
  while (n--)
  {
    uint32_t i = random_uint32_from(n+1, f) ;
    memcpy(tmp, s + i * chunksize, chunksize) ;
    memcpy(s + i * chunksize, s + n * chunksize, chunksize) ;
    memcpy(s + n * chunksize, tmp, chunksize) ;
  }
}
