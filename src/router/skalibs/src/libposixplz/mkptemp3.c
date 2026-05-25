/* ISC license. */

#include <sys/stat.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

static int f (char const *fn, mode_t mode, void *data)
{
  unsigned int flags = *(unsigned int *)data ;
  int r = mkfifo(fn, mode) ;
  if (r == -1) return -1 ;
  r = flags & O_NONBLOCK ? flags & O_CLOEXEC ? openc_read(fn) : open_read(fn) : flags & O_CLOEXEC ? openc_readb(fn) : open_readb(fn) ;
  if (r == -1) unlink_void(fn) ;
  return r ;
}

int mkptemp3 (char *s, mode_t mode, unsigned int flags)
{
  mode_t m = umask(0) ;
  int r = mkfiletemp(s, &f, mode, &flags) ;
  umask(m) ;
  return r ;
}
