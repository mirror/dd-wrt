/* ISC license. */

#include <skalibs/socket.h>
#include <skalibs/ip46.h>

#ifdef SKALIBS_IPV6_ENABLED

ssize_t socket_recvnb46 (int s, char *buf, size_t len, ip46 *ip, uint16_t *port, int h, tain const *deadline, tain *stamp)
{
  ssize_t r = h ? socket_recvnb6(s, buf, len, ip->ip, port, deadline, stamp) : socket_recvnb4(s, buf, len, ip->ip, port, deadline, stamp) ;
  if (r == -1) return -1 ;
  ip->is6 = !!h ;
  return r ;
}

#else

ssize_t socket_recvnb46 (int s, char *buf, size_t len, ip46 *ip, uint16_t *port, int h, tain const *deadline, tain *stamp)
{
  (void)h ;
  return socket_recvnb4(s, buf, len, ip->ip, port, deadline, stamp) ;
}

#endif
