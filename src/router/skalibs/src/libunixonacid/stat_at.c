/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASOPENAT

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#include <skalibs/bsdsnowflake.h>
#include <skalibs/nonposix.h>


#include <skalibs/stat.h>
#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>

int stat_at (int dirfd, char const *file, struct stat *st)
{
  return fstatat(dirfd, file, st, 0) ;
}

int lstat_at (int dirfd, char const *file, struct stat *st)
{
  return fstatat(dirfd, file, st, AT_SYMLINK_NOFOLLOW) ;
}

#else

#include <skalibs/bsdsnowflake.h>
#include <skalibs/nonposix.h>

#include <errno.h>

#include <skalibs/stat.h>
#include <skalibs/fcntl.h>
#include <skalibs/unix-transactional.h>
#include "at-internal.h"

struct stat_s
{
  char const *file ;
  struct stat *st ;
} ;

static int do_stat (void *p)
{
  struct stat_s *b = p ;
  return stat(b->file, b->st) ;
}

static int do_lstat (void *p)
{
  struct stat_s *b = p ;
  return lstat(b->file, b->st) ;
}

static void cancel_stat (int r, void *p)
{
  (void)r ;
  (void)p ;
}

int stat_at (int dirfd, char const *file, struct stat *st)
{
  struct stat_s args = { .file = file, .st = st } ;
  return emulate_at(dirfd, &do_stat, &cancel_stat, &args) ;
}

int lstat_at (int dirfd, char const *file, struct stat *st)
{
  struct stat_s args = { .file = file, .st = st } ;
  return emulate_at(dirfd, &do_lstat, &cancel_stat, &args) ;
}

#endif
