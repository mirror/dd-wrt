/* ISC license. */

#include <sys/uio.h>
#include <stdint.h>
#include <errno.h>
#include <skalibs/uint32.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/textmessage.h>

int textmessage_receive (textmessage_receiver *tr, struct iovec *v)
{
  if (tr->indata.len == tr->wanted)
  {
    uint32_t u ;
    char pack[4] ;
    if (buffer_len(&tr->in) < 4)
    {
      ssize_t r = sanitize_read(buffer_fill(&tr->in)) ;
      if (r <= 0) return r ;
      if (buffer_len(&tr->in) < 4) return (errno = EWOULDBLOCK, 0) ;
    }
    buffer_getnofill(&tr->in, pack, 4) ;
    uint32_unpack_big(pack, &u) ;
    if (u > tr->max) return (errno = EMSGSIZE, -1) ;
    if (!stralloc_ready(&tr->indata, u)) return -1 ;
    tr->wanted = u ;
    tr->indata.len = 0 ;
  }

  {
    int r = buffer_getall(&tr->in, tr->indata.s, tr->wanted, &tr->indata.len) ;
    if (r <= 0) return r ;
  }

  v->iov_base = tr->indata.s ;
  v->iov_len = tr->indata.len ;
  return 1 ;
}
