/* ISC license. */

#include <skalibs/socket.h>
#include <skalibs/ip46.h>

#ifdef SKALIBS_IPV6_ENABLED

ssize_t socket_recv46 (int s, char *buf, size_t len, ip46 *ip, uint16_t *port, int h)
{
  ssize_t r = h ? socket_recv6(s, buf, len, ip->ip, port) : socket_recv4(s, buf, len, ip->ip, port) ;
  if (r == -1) return -1 ;
  ip->is6 = !!h ;
  return r ;
}

#else

ssize_t socket_recv46 (int s, char *buf, size_t len, ip46 *ip, uint16_t *port, int h)
{
  (void)h ;
  return socket_recv4(s, buf, len, ip->ip, port) ;
}

#endif
