/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _XPG4_2
# define _XPG4_2
#endif

#ifndef _XPG_6
# define _XPG6
#endif

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <sys/socket.h>

int main (void)
{
  int flag = MSG_CMSG_CLOEXEC ;
  return 0 ;
}
