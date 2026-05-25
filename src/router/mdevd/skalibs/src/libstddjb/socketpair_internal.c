/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <errno.h>

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>

#ifdef SKALIBS_HASACCEPT4

int socketpair_internal (int domain, int type, int protocol, unsigned int flags, int *sv)
{
  return socketpair(domain, type | ((flags & O_NONBLOCK) ? SOCK_NONBLOCK : 0) | ((flags & O_CLOEXEC) ? SOCK_CLOEXEC : 0), protocol, sv) ;
}

#else

int socketpair_internal (int domain, int type, int protocol, unsigned int flags, int *sv)
{
  int fd[2] ;
  if (socketpair(domain, type, protocol, fd) < 0) return -1 ;
  if (flags & O_NONBLOCK)
  {
    if (ndelay_on(fd[0]) < 0) goto err ;
    if (ndelay_on(fd[1]) < 0) goto err ;
  }
  else
  {
    if (ndelay_off(fd[0]) < 0) goto err ;
    if (ndelay_off(fd[1]) < 0) goto err ;
  }
  if (flags & O_CLOEXEC)
  {
    if (coe(fd[0]) < 0) goto err ;
    if (coe(fd[1]) < 0) goto err ;
  }
  else
  {
    if (uncoe(fd[0]) < 0) goto err ;
    if (uncoe(fd[1]) < 0) goto err ;
  }
  sv[0] = fd[0] ;
  sv[1] = fd[1] ;
  return 0 ;

 err:
  fd_close(fd[1]) ;
  fd_close(fd[0]) ;
  return -1 ;
}

#endif
