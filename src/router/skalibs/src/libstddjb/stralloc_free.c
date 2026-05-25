/* ISC license. */

#include <skalibs/alloc.h>
#include <skalibs/stralloc.h>

void stralloc_free (stralloc *sa)
{
  alloc_free(sa->s) ;
  *sa = stralloc_zero ;
}
