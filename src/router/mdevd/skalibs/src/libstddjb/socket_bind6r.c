/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>

#include <skalibs/socket.h>

int socket_bind6_reuse (int s, char const *ip6, uint16_t port)
{
  static unsigned int const opt = 1 ;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) ;
  return socket_bind6(s, ip6, port) ;
}
