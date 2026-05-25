/* ISC license. */

#include <errno.h>
#include <skalibs/error.h>
#include <skalibs/socket.h>

int socket_deadlineconnstamp (int s, char const *ip, uint16_t port, tain const *deadline, tain *stamp)
{
  int e = errno ;
  if (socket_connect4(s, ip, port) >= 0) return 1 ;
  if (!error_isagain(errno) && !error_isalready(errno)) return 0 ;
  errno = e ;
  return socket_waitconn(s, deadline, stamp) ;
}
