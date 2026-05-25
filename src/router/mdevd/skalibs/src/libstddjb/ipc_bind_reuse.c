/* ISC license. */

#include <skalibs/socket.h>

int ipc_bind_reuse (int s, char const *p)
{
  int fdlock ;
  return ipc_bind_reuse_lock(s, p, &fdlock) ;
}
