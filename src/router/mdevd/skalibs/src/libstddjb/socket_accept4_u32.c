/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/socket.h>

int socket_accept4_internal_u32 (int s, uint32_t *ip, uint16_t *port, unsigned int options)
{
  char pack[4] ;
  int fd = socket_accept4_internal(s, pack, port, options) ;
  if (fd >= 0) uint32_unpack_big(pack, ip) ;
  return fd ;
}
