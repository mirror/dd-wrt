/* ISC license. */

#include <string.h>
#include <skalibs/fmtscan.h>

size_t str_fmt (char *d, char const *s)
{
  return strn_fmt(d, s, strlen(s)) ;
}
