/* ISC license. */

#include <skalibs/tai.h>

size_t timestamp_scan (char const *s, tain *a)
{
  size_t r ;
  if (*s != '@') return 0 ;
  r = tain_scan(s+1, a) ;
  return r ? r+1 : 0 ;
}
