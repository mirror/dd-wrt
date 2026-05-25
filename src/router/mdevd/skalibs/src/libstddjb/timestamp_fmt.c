/* ISC license. */

#include <skalibs/tai.h>

size_t timestamp_fmt (char *s, tain const *a)
{
  *s = '@' ;
  return 1 + tain_fmt(s+1, a) ;
}
