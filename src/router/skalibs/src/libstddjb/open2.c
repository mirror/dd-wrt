/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <sys/stat.h>
#include <errno.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

#ifdef SKALIBS_HASOCLOEXEC

int open2 (char const *s, unsigned int flags)
{
  int e = errno ;
  int r ;
  do r = open(s, flags) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}

#else

int open2 (char const *s, unsigned int flags)
{
  int e = errno ;
  int fd ;
  do fd = open(s, flags & ~O_CLOEXEC) ;
  while (fd == -1 && errno == EINTR) ;
  if (fd == -1) return -1 ;
  if (flags & O_CLOEXEC && coe(fd) == -1)
  {
    fd_close(fd) ;
    return -1 ;
  }
  errno = e ;
  return fd ;
}

#endif
