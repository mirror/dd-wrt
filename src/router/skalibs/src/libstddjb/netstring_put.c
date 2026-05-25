/* ISC license. */

#include <errno.h>
#include <skalibs/uint64.h>
#include <skalibs/buffer.h>
#include <skalibs/netstring.h>

int netstring_put (buffer *b, char const *s, size_t len, size_t *written)
{
  char fmt[UINT64_FMT] ;
  size_t n = uint64_fmt(fmt, len) ;
  if (*written > len + n + 2) return (errno = EINVAL, 0) ;
  fmt[n] = ':' ;
  if (*written < n + 1)
  {
    size_t w = *written ;
    int r = buffer_putall(b, fmt, n+1, &w) ;
    if (r < 0) return (*written = w, 0) ;
    *written = n+1 ;
  }
  if (*written < n+1 + len)
  {
    size_t w = *written - (n+1) ;
    int r = buffer_putall(b, s, len, &w) ;
    *written = w + (n+1) ;
    if (r < 0) return (*written = n+1 + w, 0) ;
    *written = n+1 + len ;
  }
  {
    size_t w = 0 ;
    int r = buffer_putall(b, ",", 1, &w) ;
    if (r < 0) return 0 ;
  }
  *written = 0 ;
  return 1 ;
}
