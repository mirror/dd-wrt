/* ISC license. */

#include <skalibs/sysdeps.h>

#if !defined(SKALIBS_HASOPENAT) || !defined(SKALIBS_HASLINKAT)

#include <skalibs/bsdsnowflake.h>

#include <errno.h>
#include <fcntl.h>

#include <skalibs/functypes.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

int emulate_at (int dirfd, init_func_ref f, deinit_func_ref g, void *p)
{
  int r ;
#ifdef SKALIBS_HASODIRECTORY
  int fdhere = open2(".", O_RDONLY | O_DIRECTORY) ;
#else
  int fdhere = open_readb(".") ;
#endif
  if (fdhere < 0) return -1 ;
  if (fd_chdir(dirfd) < 0) goto errclose ;
  r = (*f)(p) ;
  if (r < 0)
  {
    int e = errno ;
    fd_chdir(fdhere) ;
    errno = e ;
    goto errclose ;
  }
  if (fd_chdir(fdhere) < 0)
  {
    (*g)(r, p) ;
    goto errclose ;
  }
  fd_close(fdhere) ;
  return r ;

 errclose:
  fd_close(fdhere) ;
  return -1 ;
}

#endif
