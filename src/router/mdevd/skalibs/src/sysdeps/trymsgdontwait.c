/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _XPG4_2
# define _XPG4_2
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/socket.h>

int value = MSG_DONTWAIT ;
