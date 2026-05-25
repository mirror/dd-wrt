/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <skalibs/buffer.h>
#include <skalibs/siovec.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

int skagetlnmaxsep (buffer *b, stralloc *sa, size_t max, char const *sep, size_t seplen)
{
  size_t start = sa->len ;
  for (;;)
  {
    struct iovec v[2] ;
    size_t pos ;
    int r ;
    buffer_rpeek(b, v) ;
    pos = siovec_bytein(v, 2, sep, seplen) ;
    r = pos < buffer_len(b) ; pos += r ;
    if (!stralloc_readyplus(sa, pos)) return -1 ;
    buffer_getnofill(b, sa->s + sa->len, pos) ; sa->len += pos ;
    if (r) return 1 ;
    if (sa->len - start >= max) return (errno = EMSGSIZE, -1) ;
    r = buffer_fill(b) ;
    if (r < 0) return r ;
    if (!r) return (sa->s && (sa->len > start)) ? (errno = EPIPE, -1) : 0 ;
  }
}
