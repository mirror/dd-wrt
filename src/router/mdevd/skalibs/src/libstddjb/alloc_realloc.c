/* ISC license. */

#include <stdlib.h>
#include <skalibs/alloc.h>

int alloc_realloc (char **x, size_t n)
{
  char *y = n ? realloc(*x, n) : alloc(0) ;
  if (!y) return 0 ;
  if (!n) free(*x) ;
  *x = y ;
  return 1 ;
}
