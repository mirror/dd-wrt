/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int openc_truncat (int fd, char const *name)
{
  return open3_at(fd, name, O_WRONLY | O_NONBLOCK | O_TRUNC | O_CREAT | O_CLOEXEC, 0666) ;
}
