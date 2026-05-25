/* ISC license. */

#include <stdlib.h>
#include <skalibs/alloc.h>

void *alloc (size_t n)
{
  return malloc(n ? n : 1) ;
}
