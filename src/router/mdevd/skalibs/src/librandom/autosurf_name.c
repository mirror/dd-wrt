/* ISC license. */

#include <skalibs/random.h>
#include <skalibs/surf.h>

void autosurf_name (char *s, size_t n)
{
  random_name_from(s, n, &autosurf) ;
}
