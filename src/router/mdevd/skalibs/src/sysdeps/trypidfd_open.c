/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

static long a = SYS_pidfd_open ;
