/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/djbtime.h>

size_t localtmn_fmt (char *s, localtmn const *l)
{
  char *p = s ;
  p += localtm_fmt(p, &l->tm) ; *p++ = '.' ;
  uint320_fmt(p, l->nano, 9) ; p += 9 ;
  return p - s ;
}
