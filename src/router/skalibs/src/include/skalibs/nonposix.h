/* ISC license. */

#ifndef SKALIBS_NONPOSIX_H
#define SKALIBS_NONPOSIX_H


 /* Drop all pretense of standardness: some libc headers are *more*
    broken when you define standard feature test macros than when
    you don't (I'm looking at you, FreeBSD). */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE


#if defined(sun) || defined(__sun)

 /* Solaris: the socket API is not POSIX unless you enable this */

#ifndef _XPG4_2
#define _XPG4_2
#endif
#ifndef _XPG6
#define _XPG6
#endif


 /* Solaris: for settimeofday() and similar. Notice how you
    have to define by hand a macro with double underscores. */

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#endif /* sun || __sun */


#if defined(__linux__) || defined(__GNU__) || defined(__midipix__)

 /* GNU (Linux or Hurd): most extensions are unavailable unless
    you enable _GNU_SOURCE. Some Linux interfaces are also
    unavailable without it. */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#else /* __linux__ || __GNU__ */

 /* Various BSDs and others: _BSD_SOURCE opens up a lot of extensions.
    We guard this under not-GNU because recent glibcs scream their
    heads off if you define _BSD_SOURCE. Stay classy, GNU. */

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#endif /* __linux__ || __GNU__ */


#ifdef __NetBSD__

 /* NetBSD: of course they had to have their own macros too. */

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE
#endif
#ifndef _INCOMPLETE_XOPEN_C063
#define _INCOMPLETE_XOPEN_C063
#endif

#endif /* __NetBSD__ */


 /* old versions of BSD and some broken GNU toolchains:
      system headers are not self-contained,
      starting with sys/types.h normally always works. */

#include <sys/types.h>

#endif /* SKALIBS_NONPOSIX_H */
