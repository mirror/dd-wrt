/* ISC license. */

#include <string.h>
#include <errno.h>
#include <skalibs/skamisc.h>

int string_unquote (char *d, size_t *w, char const *s, size_t len, size_t *r)
{
  if (!len-- || (*s++ != '"')) return (errno = EINVAL, 0) ;
  {
    size_t rr, ww ;
    char tmp[len ? len : 1] ;
    if (!string_unquote_withdelim(tmp, &ww, s, len, &rr, "\"", 1)) return 0 ;
    if (rr == len) return (errno = EPIPE, 0) ;
    memcpy(d, tmp, ww) ;
    *w = ww ;
    *r = rr + 2 ;
  }
  return 1 ;
}
