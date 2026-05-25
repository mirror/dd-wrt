/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#include <skalibs/socket.h>
#include <skalibs/posixishard.h>

int ipc_bind (int s, char const *p)
{
  struct sockaddr_un sa ;
  size_t l = strlen(p) ;
  if (l > IPCPATH_MAX) return (errno = ENAMETOOLONG, -1) ;
  memset(&sa, 0, sizeof sa) ;
  sa.sun_family = AF_UNIX ;
  memcpy(sa.sun_path, p, l+1) ;
  return bind(s, (struct sockaddr *)&sa, sizeof sa) ;
}
