/* ISC license. */

#include <unistd.h>

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  int fd ;
  char *s ;
  size_t len ;
  size_t w ;
} ;

static int getfd (struct blah_s *blah)
{
  return blah->fd ;
}

static ssize_t get (struct blah_s *blah)
{
  ssize_t r = sanitize_read(fd_read(blah->fd, blah->s + blah->w, blah->len - blah->w)) ;
  if (r > 0) blah->w += r ;
  return r ;
}

size_t timed_read (int fd, char *s, size_t len, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .fd = fd, .s = s, .len = len, .w = 0 } ;
  timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
  return blah.w ;
}
