/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#define _XPG4_2
#define _XPG6

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main (void)
{
  int s, r ;
  struct in6_addr foo ;
  struct sockaddr_in6 bar ;
  memset(&foo, 0, sizeof(struct sockaddr_in6)) ;
  bar.sin6_addr = foo ;
  bar.sin6_port = 1026 ;
  s = socket(AF_INET6, SOCK_STREAM, 0) ;
  if (s < 0) return 111 ;
  do r = connect(s, (struct sockaddr *)&bar, sizeof bar) ;
  while (r == -1 && errno == EINTR) ;
  if (r == -1 && errno == EALREADY) errno = EINPROGRESS ;
  return 0 ;
}
