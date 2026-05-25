/* ISC license. */

#include <sys/types.h>
#include <errno.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

#define N 4096

int slurpn (int fd, stralloc *sa, size_t max)
{
  for (;;)
  {
    ssize_t r ;
    size_t n = max && sa->len + N > max ? max - sa->len : N ;
    if (!n) return (errno = ENOBUFS, 0) ;
    if (!stralloc_readyplus(sa, n)) return 0 ;
    r = fd_read(fd, sa->s + sa->len, n) ;
    switch (r)
    {
      case -1 : return 0 ;
      case 0 : return 1 ;
      default : sa->len += r ;
    }
  }
}
