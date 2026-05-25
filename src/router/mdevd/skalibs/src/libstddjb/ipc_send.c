/* ISC license. */

#include <skalibs/nonposix.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <skalibs/socket.h>
#include <skalibs/posixishard.h>

ssize_t ipc_send (int fd, char const *s, size_t len, char const *path)
{
  struct sockaddr_un sa ;
  size_t l = strlen(path) ;
  if (l > IPCPATH_MAX) return (errno = EPROTO, -1) ;
  memset(&sa, 0, sizeof sa) ;
  sa.sun_family = AF_UNIX ;
  memcpy(sa.sun_path, path, l+1) ;
  return sendto(fd, s, len, 0, (struct sockaddr *)&sa, sizeof sa) ;
}
