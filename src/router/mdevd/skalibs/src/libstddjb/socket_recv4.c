/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>

ssize_t socket_recv4 (int s, char *buf, size_t len, char *ip, uint16_t *port)
{
  int e = errno ;
  ssize_t r ;
  struct sockaddr_in sa ;
  socklen_t dummy = sizeof sa ;
  do r = recvfrom(s, buf, len, 0, (struct sockaddr *)&sa, &dummy) ;
  while (r == -1 && errno == EINTR) ;
  if (r == -1) return -1 ;
  errno = e ;
  memcpy(ip, &sa.sin_addr, 4) ;
  *port = uint16_big(sa.sin_port) ;
  return r ;
}
