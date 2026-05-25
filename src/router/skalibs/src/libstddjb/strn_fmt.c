/* ISC license. */

#include <skalibs/types.h>
#include <skalibs/fmtscan.h>

size_t strn_fmt (char *blah, char const *s, size_t len)
{
  char *d = blah ;
  size_t i ;
  for (i = 0 ; i < len ; i++)
    if ((s[i] >= 32) && ((unsigned char)s[i] < 127)) *d++ = s[i] ;
    else
    {
      *d++ = '\\' ;
      *d++ = '0' ;
      *d++ = 'x' ;
      if ((unsigned char)s[i] < 16) *d++ = '0' ;
      d += uint_xfmt(d, (unsigned char)s[i]) ;
    }
  return d - blah ;
}
