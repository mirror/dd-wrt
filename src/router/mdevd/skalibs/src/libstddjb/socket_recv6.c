/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#include <skalibs/uint16.h>
#include <skalibs/ip46.h>
#include <skalibs/socket.h>

#ifdef SKALIBS_IPV6_ENABLED

ssize_t socket_recv6 (int s, char *buf, size_t len, char *ip6, uint16_t *port)
{
  int e = errno ;
  struct sockaddr_in6 sa ;
  socklen_t dummy = sizeof sa ;
  ssize_t r ;
  do r = recvfrom(s, buf, len, 0, (struct sockaddr *)&sa, &dummy) ;
  while (r == -1 && errno == EINTR) ;
  if (r == -1) return -1 ;
  errno = e ;
  memcpy(ip6, sa.sin6_addr.s6_addr, 16) ;
  *port = uint16_big(sa.sin6_port) ;
  return r ;
}

#else

ssize_t socket_recv6 (int s, char *buf, size_t len, char *ip6, uint16_t *port)
{
  (void)s ;
  (void)buf ;
  (void)len ;
  (void)ip6 ;
  (void)port ;
  return (errno = ENOSYS, -1) ;
}

#endif
