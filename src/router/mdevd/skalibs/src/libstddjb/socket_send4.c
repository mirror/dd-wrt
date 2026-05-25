/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>

ssize_t socket_send4 (int s, char const *buf, size_t len, char const *ip, uint16_t port)
{
  int e = errno ;
  ssize_t r ;
  struct sockaddr_in sa ;
  memset(&sa, 0, sizeof sa) ;
  sa.sin_family = AF_INET ;
  sa.sin_port = uint16_big(port) ;
  memcpy(&sa.sin_addr, ip, 4) ;
  do r = sendto(s, buf, len, 0, (struct sockaddr *)&sa, sizeof sa) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}
