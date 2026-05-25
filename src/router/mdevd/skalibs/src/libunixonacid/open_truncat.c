/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int open_truncat (int fd, char const *name)
{
  return open3_at(fd, name, O_WRONLY | O_NONBLOCK | O_TRUNC | O_CREAT, 0666) ;
}
