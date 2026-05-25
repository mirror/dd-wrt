/* ISC license. */

#include <skalibs/sysdeps.h>
#include <skalibs/nonposix.h>
#include <skalibs/direntry.h>

#ifdef SKALIBS_HASDIRFD

int dir_fd (DIR *dir)
{
  return dirfd(dir) ;
}

#else

 /* Pokes at the internals of DIR - no choice here */

int dir_fd (DIR *dir)
{
  return dir->dd_fd ;
}

#endif
