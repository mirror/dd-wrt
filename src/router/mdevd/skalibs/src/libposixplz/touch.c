/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASFUTIMENS

#include <skalibs/nonposix.h>
#include <time.h>
#include <sys/stat.h>
#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

int touch (char const *file)
{
  int r ;
  int fd = open_create(file) ;
  if (fd < 0) return 0 ;
  r = futimens(fd, 0) >= 0 ;
  fd_close(fd) ;
  return r ;
}

#else
#ifdef SKALIBS_HASFUTIMES

#include <skalibs/nonposix.h>
#include <sys/time.h>
#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

int touch (char const *file)
{
  int r ;
  int fd = open_create(file) ;
  if (fd < 0) return 0 ;
  r = futimes(fd, 0) >= 0 ;
  fd_close(fd) ;
  return r ;
}

#else

#include <sys/time.h>
#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

int touch (char const *file)
{
  int fd = open_create(file) ;
  if (fd < 0) return 0 ;
  fd_close(fd) ;
  return utimes(file, 0) >= 0 ;
}

#endif
#endif
