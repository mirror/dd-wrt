/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/socket.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  int fd ;
  char *s ;
  size_t len ;
  char *path ;
} ;

static int getfd (struct blah_s *blah)
{
  return blah->fd ;
}

static ssize_t get (struct blah_s *blah)
{
  return sanitize_read(ipc_recv(blah->fd, blah->s, blah->len, blah->path)) ;
}

ssize_t ipc_timed_recv (int fd, char *s, size_t len, char *path, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .fd = fd, .s = s, .len = len, .path = path } ;
  return timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
