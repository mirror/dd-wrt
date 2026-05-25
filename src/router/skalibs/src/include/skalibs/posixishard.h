/* ISC license. */

#ifndef SKALIBS_POSIXISHARD_H
#define SKALIBS_POSIXISHARD_H

#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>

#include <skalibs/gccattributes.h>
#include <skalibs/stat.h>

 /*
    This header must be used with some broken OSes who have
    serious trouble implementing even easy parts of POSIX.
    It's supposed to be included *after* system headers, so
    it won't catch behaviour such as "this macro must be defined
    prior to including system headers in order to define that
    symbol". If that's what you need, include skalibs/nonposix.h
    instead, which must happen *before* system headers.
 */

extern size_t strnlen (char const *, size_t) gccattr_pure ;

#ifndef EPROTO
#define EPROTO EPROTOTYPE
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifdef PATH_MAX
#define SKALIBS_PATH_MAX PATH_MAX
#else
#define SKALIBS_PATH_MAX 4096
#endif

#ifdef NAME_MAX
#define SKALIBS_NAME_MAX NAME_MAX
#else
#define SKALIBS_NAME_MAX 255
#endif

#endif
