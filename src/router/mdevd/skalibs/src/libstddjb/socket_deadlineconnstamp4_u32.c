/* ISC license. */

#include <skalibs/uint32.h>
#include <skalibs/socket.h>

int socket_deadlineconnstamp4_u32 (int s, uint32_t ip, uint16_t port, tain const *deadline, tain *stamp)
{
  char pack[4] ;
  uint32_pack_big(pack, ip) ;
  return socket_deadlineconnstamp4(s, pack, port, deadline, stamp) ;
}
