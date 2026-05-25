/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <stdlib.h>

static int f (void const *a, void const *b, void *c)
{
  int aa = *(int const *)a ;
  int bb = *(int const *)b ;
  (void)c ;
  return aa < bb ? -1 : aa > bb ;
}

int main (void)
{
  char *arg = "blah" ;
  int x[2] = { 2, 1 } ;
  qsort_r(x, 2, sizeof(int), arg, &f) ;
  return 0 ;
}
