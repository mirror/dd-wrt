/* ISC license. */

#include <sys/uio.h>

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/siovec.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  int fd ;
  struct iovec *v ;
  unsigned int vlen ;
  size_t w ;
} ;

static int getfd (struct blah_s *blah)
{
  return blah->fd ;
}

static ssize_t get (struct blah_s *blah)
{
  ssize_t r = sanitize_read(fd_readv(blah->fd, blah->v, blah->vlen)) ;
  if (r > 0) { blah->w += r ; siovec_seek(blah->v, blah->vlen, r) ; }
  return r ;
}

size_t timed_readv (int fd, struct iovec *v, unsigned int vlen, tain const *deadline, tain *stamp)
{
  if (!vlen || !siovec_len(v, vlen)) return 0 ;
  struct iovec vv[vlen] ;
  struct blah_s blah = { .fd = fd, .v = vv, .vlen = vlen, .w = 0 } ;
  for (unsigned int i = 0 ; i < vlen ; i++) vv[i] = v[i] ;
  timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
  return blah.w ;
}
