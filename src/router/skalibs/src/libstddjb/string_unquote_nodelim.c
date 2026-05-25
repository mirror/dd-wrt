/* ISC license. */

#include <errno.h>
#include <skalibs/skamisc.h>

ssize_t string_unquote_nodelim (char *d, char const *s, size_t len)
{
  size_t rr, ww ;
  if (!string_unquote_withdelim(d, &ww, s, len, &rr, 0, 0)) return -1 ;
  return (ssize_t)ww ;
}
