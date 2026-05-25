/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/bitarray.h>
#include <skalibs/disize.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/siovec.h>
#include <skalibs/unixmessage.h>
#include <skalibs/posixishard.h>

static inline int copyfds (int *dst, int const *src, unsigned int n, unsigned char const *bits)
{
  for (unsigned int i = 0 ; i < n ; i++)
  {
    int fd = src[i] ;
    if (fd < 0) return (errno = EINVAL, 0) ;
    if (bitarray_peek(bits, i)) fd = - fd - 1 ;
    dst[i] = fd ;
  }
  return 1 ;
}

static int reserve_and_copy (unixmessage_sender *b, size_t len, int const *fds, unsigned int nfds, unsigned char const *bits)
{
  disize cur = { .left = b->data.len, .right = genalloc_len(int, &b->fds) } ;
  if (len > UNIXMESSAGE_MAXSIZE || nfds > UNIXMESSAGE_MAXFDS) return (errno = EPROTO, 0) ;
  if (!genalloc_readyplus(disize, &b->offsets, 1)
   || !genalloc_readyplus(int, &b->fds, nfds)
   || !stralloc_readyplus(&b->data, len))
    return 0 ;
  if (!copyfds(genalloc_s(int, &b->fds) + cur.right, fds, nfds, bits)) return 0 ;
  genalloc_setlen(int, &b->fds, cur.right + nfds) ;
  return genalloc_catb(disize, &b->offsets, &cur, 1) ;
}

int unixmessage_put_and_close (unixmessage_sender *b, unixmessage const *m, unsigned char const *bits)
{
  if (!reserve_and_copy(b, m->len, m->fds, m->nfds, bits)) return 0 ;
  memmove(b->data.s + b->data.len, m->s, m->len) ;
  b->data.len += m->len ;
  return 1 ;
}

int unixmessage_putv_and_close (unixmessage_sender *b, unixmessagev const *m, unsigned char const *bits)
{
  size_t len = siovec_len(m->v, m->vlen) ;
  if (!reserve_and_copy(b, len, m->fds, m->nfds, bits)) return 0 ;
  b->data.len += siovec_gather(m->v, m->vlen, b->data.s + b->data.len, len) ;
  return 1 ;
}
