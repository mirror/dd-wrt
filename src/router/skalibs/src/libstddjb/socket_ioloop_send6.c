/* ISC license. */

#include <skalibs/socket.h>

ssize_t socket_ioloop_send6 (int fd, char *s, size_t len, char *ip, uint16_t *port)
{
  return socket_send6(fd, s, len, ip, *port) ;
}
