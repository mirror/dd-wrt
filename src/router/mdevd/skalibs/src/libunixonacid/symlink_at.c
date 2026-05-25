/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASLINKAT

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#include <skalibs/nonposix.h>

#include <unistd.h>
#include <errno.h>

#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int symlink_at (char const *src, int dirfd, char const *dst)
{
  int e = errno ;
  int r ;
  do r = symlinkat(src, dirfd, dst) ;
  while (r == -1 && errno == EINTR) ;
  if (r >= 0) errno = e ;
  return r ;
}

#else

#include <unistd.h>
#include <errno.h>

#include <skalibs/posixplz.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

struct symlink_s
{
  char const *src ;
  char const *dst ;
} ;

static int do_symlink (void *p)
{
  struct symlink_s *b = p ;
  return symlink(b->src, b->dst) ;
}

static void cancel_symlink (int r, void *p)
{
  struct symlink_s *b = p ;
  (void)r ;
  unlink_void(b->dst) ;
}

int symlink_at (char const *src, int dirfd, char const *dst)
{
  struct symlink_s args = { .src = src, .dst = dst } ;
  return emulate_at(dirfd, &do_symlink, &cancel_symlink, &args) ;
}

#endif
