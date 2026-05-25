/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>
#include <skalibs/ip46.h>

#ifdef SKALIBS_IPV6_ENABLED

int socket_local6 (int s, char *ip, uint16_t *port)
{
  struct sockaddr_in6 sa ;
  socklen_t dummy = sizeof sa ;
  if (getsockname(s, (struct sockaddr *)&sa, &dummy) == -1) return -1 ;
  memcpy(ip, sa.sin6_addr.s6_addr, 16) ;
  *port = uint16_big(sa.sin6_port) ;
  return 0 ;
}

#else

#include <errno.h>

int socket_local6 (int s, char *ip, uint16_t *port)
{
  (void)s ;
  (void)ip ;
  (void)port ;
  return (errno = ENOSYS, -1) ;
}

#endif
