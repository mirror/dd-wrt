/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/djbtime.h>

size_t localtmn_scan (char const *s, localtmn *l)
{
  localtmn m ;
  size_t n = localtm_scan(s, &m.tm) ;
  if (!n) return 0 ;
  s += n ;
  if (*s++ != '.') m.nano = 0 ;
  else
  {
    size_t b = uint32_scan(s, &m.nano) ;
    if (!b) return 0 ;
    n += b ;
  }
  *l = m ;
  return n ;
}
