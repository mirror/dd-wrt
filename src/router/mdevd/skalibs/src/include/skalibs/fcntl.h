/* ISC license. */

#ifndef SKALIBS_FCNTL_H
#define SKALIBS_FCNTL_H

#include <fcntl.h>

 /*
    Old MacOS X doesn't have O_CLOEXEC.
    We define it to something completely out of the way so we
    can still use it in userspace.
    Workarounds in syscalls will be enabled via the
    SKALIBS_HASOCLOEXEC sysdep.
 */

#ifndef O_CLOEXEC
#define O_CLOEXEC 0x40000000
#endif

#endif
