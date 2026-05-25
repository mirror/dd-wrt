/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/socket.h>
#include <skalibs/ip46.h>

#ifdef SKALIBS_IPV6_ENABLED

int socket_connect6 (int s, char const *ip6, uint16_t port)
{
  int e = errno ;
  struct sockaddr_in6 sa ;
  int r ;
  memset(&sa, 0, sizeof sa) ;
  sa.sin6_family = AF_INET6 ;
  sa.sin6_port = uint16_big(port) ;
  memcpy(sa.sin6_addr.s6_addr, ip6, 16) ;
  do r = connect(s, (struct sockaddr *)&sa, sizeof sa) ;
  while (r == -1 && errno == EINTR) ;
  if (r == -1 && errno == EALREADY) errno = EINPROGRESS ;
  if (r >= 0) errno = e ;
  return r ;
}

#else

int socket_connect6 (int s, char const *ip6, uint16_t port)
{
  (void)s ;
  (void)ip6 ;
  (void)port ;
  return (errno = ENOSYS, -1) ;
}

#endif
