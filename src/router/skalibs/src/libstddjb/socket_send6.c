/* ISC license. */

#include <skalibs/nonposix.h>

#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <skalibs/uint16.h>
#include <skalibs/ip46.h>
#include <skalibs/socket.h>

#ifdef SKALIBS_IPV6_ENABLED

ssize_t socket_send6 (int s, char const *buf, size_t len, char const *ip6, uint16_t port)
{
  int e = errno ;
  ssize_t r ;
  struct sockaddr_in6 sa ;
  memset(&sa, 0, sizeof sa) ;
  sa.sin6_family = AF_INET6 ;
  sa.sin6_port = uint16_big(port) ;
  memcpy(sa.sin6_addr.s6_addr, ip6, 16) ;
  do r = sendto(s, buf, len, 0, (struct sockaddr *)&sa, sizeof sa) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}

#else

ssize_t socket_send6 (int s, char const *buf, size_t len, char const *ip6, uint16_t port)
{
  (void)s ;
  (void)buf ;
  (void)len ;
  (void)ip6 ;
  (void)port ;
  return (errno = ENOSYS, -1) ;
}

#endif
