/* ISC license. */

/*
   This header MUST be paired with skalibs/nonposix.h,
   which must be included before system headers.
*/

#ifndef SKALIBS_NSIG_H
#define SKALIBS_NSIG_H

#include <signal.h>

#ifndef NSIG
# ifdef _NSIG
#  define NSIG _NSIG
# elif defined(SIGMAX)
#  define NSIG (SIGMAX + 1)
# elif defined(_SIGMAX)
#  define NSIG(_SIGMAX + 1)
# else
#  define NSIG 65
# endif
#endif

/*
  Some systems (FreeBSD, Darwin) incorrectly define NSIG as 32
  (their highest signal number) when it should be 33 (highest plus one).
  OpenBSD gets this right so we can't use SKALIBS_BSD_SUCKS.
  The heuristic we use is: if NSIG is a power of two, it's wrong.
*/

#if NSIG & (NSIG - 1)
# define SKALIBS_NSIG NSIG
#else
# define SKALIBS_NSIG (NSIG+1)
#endif

#endif
