/* ISC license. */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <skalibs/djbunix.h>
#include <skalibs/ip46.h>
#include <skalibs/socket.h>

#ifdef SKALIBS_IPV6_ENABLED

int socket_tcp6_internal (unsigned int flags)
{
  int fd = socket_internal(AF_INET6, SOCK_STREAM, 0, flags) ;
  if (fd < 0) return fd ;
  {
    int option = 1 ;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option)) < 0)
    {
      fd_close(fd) ;
      return -1 ;
    }
  }
  return fd ;
}

#else

int socket_tcp6_internal (unsigned int flags)
{
  (void)flags ;
  return (errno = ENOSYS, -1) ;
}

#endif
