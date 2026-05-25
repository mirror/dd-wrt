/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASOPENAT

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#include <skalibs/nonposix.h>

#include <errno.h>

#include <skalibs/stat.h>
#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int open2_at (int dirfd, char const *file, int flags)
{
  int e = errno ;
  int fd ;
  do fd = openat(dirfd, file, flags) ;  /* all systems supporting openat() have O_CLOEXEC */
  while (fd == -1 && errno == EINTR) ;
  if (fd >= 0) errno = e ;
  return fd ;
}

#else

#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

struct open2_s
{
  char const *file ;
  int flags ;
} ;

static int do_open2 (void *p)
{
  struct open2_s *b = p ;
  return open2(b->file, b->flags) ;
}

static void cancel_open2 (int fd, void *p)
{
  (void)p ;
  fd_close(fd) ;
}

int open2_at (int dirfd, char const *file, int flags)
{
  struct open2_s args = { .file = file, .flags = flags } ;
  return emulate_at(dirfd, &do_open2, &cancel_open2, &args) ;
}

#endif
