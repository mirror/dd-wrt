/* ISC license. */

#include <skalibs/socket.h>

int ipc_bind_reuse_perms (int s, char const *p, unsigned int perms)
{
  int fdlock ;
  return ipc_bind_reuse_lock_perms(s, p, &fdlock, perms) ;
}
