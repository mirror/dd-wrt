/* ISC license. */

#include <skalibs/nonposix.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>

int ipc_bind_reuse_lock_perms (int s, char const *p, int *fdlock, unsigned int perms)
{
  unsigned int opt = 1 ;
  size_t len = strlen(p) ;
  int fd ;
  int r ;
  mode_t m ;
  char lockname[len + 6] ;
  memcpy(lockname, p, len) ;
  memcpy(lockname + len, ".lock", 6) ;
  fd = open3(lockname, O_WRONLY | O_NONBLOCK | O_CREAT | O_CLOEXEC, 0600) ;
  if (fd < 0) return -1 ;
  r = fd_lock(fd, 1, 1) ;
  if (r < 0) return -1 ;
  if (!r) return (errno = EBUSY, -1) ;
  r = errno ;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) ;
  errno = r ;
  unlink_void(p) ;
  if (perms) m = umask(~perms & 0777) ;
  if (ipc_bind(s, p) < 0)
  {
    if (perms) umask(m) ;
    return -1 ;
  }
  if (perms) umask(m) ;
  *fdlock = fd ;
  return 0 ;
}
