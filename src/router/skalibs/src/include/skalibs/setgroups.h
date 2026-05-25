/* ISC license. */

#ifndef SKALIBS_SETGROUPS_H
#define SKALIBS_SETGROUPS_H

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASSETGROUPS

 /*
    setgroups() is defined by a lot of OSes.
    However, they don't agree on what it should do:
    some change the primary gid, others don't, etc. It's a mess.
    Never use setgroups(). Use the functions below instead.
 */

#include <unistd.h>

extern int setgroups_and_gid (gid_t, size_t, gid_t const *) ;
extern int setgroups_with_egid (size_t, gid_t const *) ;
extern int skalibs_setgroups (size_t, gid_t const *) ;

#else

 /* No setgroups() at all? not much we can do. */

#include <errno.h>

#define setgroups_and_gid(g, n, tab) (errno = ENOSYS, -1)
#define setgroups_with_egid(n, tab) (errno = ENOSYS, -1)
#define skalibs_setgroups(n, tab) (errno = ENOSYS, -1)

#endif

#endif
