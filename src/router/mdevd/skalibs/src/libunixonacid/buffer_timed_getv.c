/* ISC license. */

#include <sys/uio.h>

#include <skalibs/functypes.h>
#include <skalibs/buffer.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  buffer *b ;
  struct iovec *v ;
  unsigned int n ;
  size_t w ;
} ;

static int getfd (struct blah_s *blah)
{
  return buffer_fd(blah->b) ;
}

static ssize_t get (struct blah_s *blah)
{
  return buffer_getvall(blah->b, blah->v, blah->n, &blah->w) ;
}

size_t buffer_timed_getv (buffer *b, struct iovec *v, unsigned int n, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .b = b, .v = v, .n = n, .w = 0 } ;
  timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
  return blah.w ;
}
