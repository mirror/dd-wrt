/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASLINKAT

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#include <skalibs/nonposix.h>

#include <unistd.h>

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int access_at (int dirfd, char const *file, int amode, unsigned int flag)
{
  return faccessat(dirfd, file, amode, flag ? AT_EACCESS : 0) ;
}

#else

#include <errno.h>
#include <unistd.h>

#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

struct access_s
{
  char const *file ;
  int amode ;
} ;

static int do_access (void *p)
{
  struct access_s *b = p ;
  return access(b->file, b->amode) ;
}

static void cancel_access (int r, void *p)
{
  (void)r ;
  (void)p ;
}

int access_at (int dirfd, char const *file, int amode, unsigned int flag)
{
  struct access_s args = { .file = file, .amode = amode } ;
  if (flag && (getuid() != geteuid() || getgid() != getegid()))
    return (errno = ENOSYS, 1) ;
  return emulate_at(dirfd, &do_access, &cancel_access, &args) ;
}

#endif
