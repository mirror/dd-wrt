/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <sys/socket.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>

#ifdef SKALIBS_HASACCEPT4

int socket_internal (int domain, int type, int protocol, unsigned int flags)
{
  return socket(domain, type | ((flags & O_NONBLOCK) ? SOCK_NONBLOCK : 0) | ((flags & O_CLOEXEC) ? SOCK_CLOEXEC : 0), protocol) ;
}

#else

int socket_internal (int domain, int type, int protocol, unsigned int flags)
{
  int s = socket(domain, type, protocol) ;
  if (s == -1) return -1 ;
  if ((((flags & O_NONBLOCK) ? ndelay_on(s) : ndelay_off(s)) < 0)
   || (((flags & O_CLOEXEC) ? coe(s) : uncoe(s)) < 0))
  {
    fd_close(s) ;
    return -1 ;
  }
  return s ;
}

#endif
