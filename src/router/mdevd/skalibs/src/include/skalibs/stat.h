/* ISC license. */

#ifndef SKALIBS_STAT_H
#define SKALIBS_STAT_H

 /*
    This header must always be paired with skalibs/bsdsnowflake.h
    (which must always come first).
    If you #include <sys/stat.h> before bsdsnowflake, the
    workaround will not work.
 */

#include <skalibs/sysdeps.h>

#include <sys/stat.h>

#if !defined(SKALIBS_HASSTATIM) && defined(SKALIBS_HASSTATIMESPEC)

#define st_atim st_atimespec
#define st_mtim st_mtimespec
#define st_ctim st_ctimespec

#endif

#endif
