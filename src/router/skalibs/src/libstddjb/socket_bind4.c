/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>

int socket_bind4 (int s, char const *ip, uint16_t port)
{
  struct sockaddr_in sa ;
  memset(&sa, 0, sizeof sa) ;
  sa.sin_family = AF_INET ;
  sa.sin_port = uint16_big(port) ;
  memcpy(&sa.sin_addr.s_addr, ip, 4) ;
  return bind(s, (struct sockaddr *)&sa, sizeof sa) ;
}
