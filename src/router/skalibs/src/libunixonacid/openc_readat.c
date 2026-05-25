/* ISC license. */

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int openc_readat (int fd, char const *name)
{
  return open2_at(fd, name, O_RDONLY | O_NONBLOCK | O_CLOEXEC) ;
}
