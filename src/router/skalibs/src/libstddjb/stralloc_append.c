/* ISC license. */

#include <skalibs/stralloc.h>

int stralloc_append (stralloc *sa, char c)
{
  return stralloc_catb(sa, &c, 1) ;
}
