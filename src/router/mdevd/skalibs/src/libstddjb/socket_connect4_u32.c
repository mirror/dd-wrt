/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/socket.h>

int socket_connect4_u32 (int s, uint32_t ip, uint16_t port)
{
  char pack[4] ;
  uint32_pack_big(pack, ip) ;
  return socket_connect4(s, pack, port) ;
}
