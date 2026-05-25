/* ISC license. */

#include <unistd.h>

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  int fd ;
  char const *s ;
  size_t len ;
  size_t w ;
} ;

static int getfd (struct blah_s *blah)
{
  return blah->fd ;
}

static int isnonempty (struct blah_s *blah)
{
  return blah->w < blah->len ;
}

static int flush (struct blah_s *blah)
{
  ssize_t r = fd_write(blah->fd, blah->s + blah->w, blah->len - blah->w) ;
  if (r > 0) blah->w += r ;
  return r > 0 ;
}

size_t timed_write (int fd, char const *s, size_t len, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .fd = fd, .s = s, .len = len, .w = 0 } ;
  timed_flush(&blah, (init_func_ref)&getfd, (init_func_ref)&isnonempty, (init_func_ref)&flush, deadline, stamp) ;
  return blah.w ;
}
