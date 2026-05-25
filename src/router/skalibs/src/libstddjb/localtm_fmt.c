/* ISC license. */

#include <time.h>
#include <skalibs/types.h>
#include <skalibs/djbtime.h>

size_t localtm_fmt (char *s, struct tm const *l)
{
  char *p = s ;
  p += uint_fmt(p, 1900 + l->tm_year) ; *p++ = '-' ;
  uint0_fmt(p, 1 + l->tm_mon, 2) ; p += 2 ; *p++ = '-' ;
  uint0_fmt(p, l->tm_mday, 2) ; p += 2 ; *p++ = ' ' ;
  uint0_fmt(p, l->tm_hour, 2) ; p += 2 ; *p++ = ':' ;
  uint0_fmt(p, l->tm_min, 2) ; p += 2 ; *p++ = ':' ;
  uint0_fmt(p, l->tm_sec, 2) ; p += 2 ;
  return p - s ;
}
