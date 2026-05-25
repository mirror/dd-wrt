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
} ;

static int getfd (struct blah_s *blah)
{
  return blah->fd ;
}

static int isnonempty (struct blah_s *blah)
{
  return !!siovec_len(blah->v, blah->vlen) ;
}

static int flush (struct blah_s *blah)
{
  ssize_t r = fd_writev(blah->fd, blah->v, blah->vlen) ;
  if (r > 0) siovec_seek(blah->v, blah->vlen, r) ;
  return r > 0 ;
}

size_t timed_writev (int fd, struct iovec const *srcv, unsigned int vlen, tain const *deadline, tain *stamp)
{
  size_t len = siovec_len(srcv, vlen) ;
  if (!len) return 0 ;
  struct iovec v[vlen] ;
  struct blah_s blah = { .fd = fd, .v = v, .vlen = vlen } ;
  for (unsigned int i = 0 ; i < vlen ; i++) v[i] = srcv[i] ;
  timed_flush(&blah, (init_func_ref)&getfd, (init_func_ref)&isnonempty, (init_func_ref)&flush, deadline, stamp) ;
  return len - siovec_len(v, vlen) ;
}
