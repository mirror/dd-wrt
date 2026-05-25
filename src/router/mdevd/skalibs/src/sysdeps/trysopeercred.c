/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _XPG4_2
#define _XPG4_2
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/socket.h>
#include <sys/un.h>

int main (void)
{
  int s ;
  struct ucred dummy ;
  socklen_t len = sizeof(struct ucred) ;
  return getsockopt(s, SOL_SOCKET, SO_PEERCRED, &dummy, &len) ;
}
