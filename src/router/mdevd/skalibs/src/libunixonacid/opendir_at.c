/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>
#include <skalibs/direntry.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>

#ifdef SKALIBS_HASFDOPENDIR

DIR *opendir_at (int dfd, char const *name)
{
  DIR *dir ;
  int fd = openc_readatb(dfd, name) ;
  if (fd == -1) return 0 ;
  dir = fdopendir(fd) ;
  if (!dir) fd_close(fd) ;
  return dir ;
}

#else

#include <unistd.h>
#include <stdlib.h>

DIR *opendir_at (int dfd, char const *name)
{
  DIR *dir ;
  int here = open_read(".") ;
  if (here == -1) return 0 ;
  if (fchdir(dfd) == -1)
  {
    fd_close(here) ;
    return 0 ;
  }
  dir = opendir(name) ;
  if (fchdir(here) == -1) abort() ;
  fd_close(here) ;
  return dir ;
}

#endif
