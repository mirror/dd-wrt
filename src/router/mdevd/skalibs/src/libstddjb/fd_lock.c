/* ISC license. */

#include <unistd.h>
#include <errno.h>

#include <skalibs/error.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int fd_lock (int fd, int w, int nb)
{
  struct flock fl =
  {
    .l_type = w ? F_WRLCK : F_RDLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0
  } ;
  int e = errno ;
  int r ;
  do r = fcntl(fd, nb ? F_SETLK : F_SETLKW, &fl) ;
  while (r == -1 && errno == EINTR) ;
  return r != -1 ? 1 :
    errno == EACCES || error_isagain(errno) ? (errno = e, 0) :
    -1 ;
}
