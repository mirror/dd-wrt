/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>

#include <skalibs/socket.h>

int socket_bind4_reuse (int s, char const *ip, uint16_t port)
{
  static unsigned int const opt = 1 ;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) ;
  return socket_bind4(s, ip, port) ;
}
