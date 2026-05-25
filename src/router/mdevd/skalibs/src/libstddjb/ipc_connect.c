/* ISC license. */

#include <skalibs/nonposix.h>

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <skalibs/socket.h>
#include <skalibs/posixishard.h>

int ipc_connect (int s, char const *p)
{
  struct sockaddr_un sa ;
  size_t l = strlen(p) ;
  if (l > IPCPATH_MAX) return (errno = EPROTO, 0) ;
  memset(&sa, 0, sizeof sa) ;
  sa.sun_family = AF_UNIX ;
  memcpy(sa.sun_path, p, l+1) ;
  if (connect(s, (struct sockaddr *)&sa, sizeof sa) == -1)
  {
    if (errno == EINTR) errno = EINPROGRESS ;
    return 0 ;
  }
  return 1 ;
}
