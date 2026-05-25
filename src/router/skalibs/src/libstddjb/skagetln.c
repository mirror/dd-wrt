/* ISC license. */

#include <errno.h>
#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int skagetln (buffer *b, stralloc *sa, char sep)
{
  size_t start = sa->len ;
  for (;;)
  {
    ssize_t r = skagetln_nofill(b, sa, sep) ;
    if (r) return r ;
    r = buffer_fill(b) ;
    if (r < 0) return r ;
    if (!r) return (sa->s && (sa->len > start)) ? (errno = EPIPE, -1) : 0 ;
  }
}
