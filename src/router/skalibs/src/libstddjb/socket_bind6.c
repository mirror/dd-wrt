/* ISC license. */

#include <skalibs/nonposix.h>

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>
#include <skalibs/ip46.h>

#ifdef SKALIBS_IPV6_ENABLED

int socket_bind6 (int s, char const *ip6, uint16_t port)
{
  struct sockaddr_in6 sa ;
  memset(&sa, 0, sizeof sa) ;
  sa.sin6_family = AF_INET6 ;
  sa.sin6_port = uint16_big(port) ;
  memcpy(sa.sin6_addr.s6_addr, ip6, 16) ;
  return bind(s, (struct sockaddr *)&sa, sizeof sa) ;
}

#else

int socket_bind6 (int s, char const *ip6, uint16_t port)
{
  (void)s ;
  (void)ip6 ;
  (void)port ;
  return (errno = ENOSYS, -1) ;
}

#endif
