/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASOPENAT

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#include <skalibs/nonposix.h>

#include <errno.h>

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int open3_at (int dirfd, char const *file, int flags, unsigned int mode)
{
  int e = errno ;
  int fd ;
  do fd = openat(dirfd, file, flags, mode) ; /* all systems that support openat() have O_CLOEXEC */
  while (fd == -1 && errno == EINTR) ;
  if (fd >= 0) errno = e ;
  return fd ;
}

#else

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

struct open3_s
{
  char const *file ;
  int flags ;
  unsigned int mode ;
} ;

static int do_open3 (void *p)
{
  struct open3_s *b = p ;
  return open3(b->file, b->flags, b->mode) ;
}

static void cancel_open3 (int fd, void *p)
{
  (void)p ;
  fd_close(fd) ;
}

int open3_at (int dirfd, char const *file, int flags, unsigned int mode)
{
  struct open3_s args = { .file = file, .flags = flags, .mode = mode } ;
  return emulate_at(dirfd, &do_open3, &cancel_open3, &args) ;
}

#endif
