/* ISC license. */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <skalibs/bytestr.h>
#include <skalibs/socket.h>

int ipc_local (int s, char *p, size_t l, int *trunc)
{
  struct sockaddr_un sa ;
  socklen_t dummy = sizeof sa ;
  memset(&sa, 0, sizeof sa) ;
  if (getsockname(s, (struct sockaddr *)&sa, &dummy) == -1) return -1 ;
  dummy = byte_chr(sa.sun_path, dummy, 0) ;
  *trunc = 1 ;
  if (!l) return 0 ;
  if (l < (dummy + 1)) dummy = l - 1 ;
  else *trunc = 0 ;
  memcpy(p, sa.sun_path, dummy) ;
  p[dummy] = 0 ;
  return 0 ;
}
