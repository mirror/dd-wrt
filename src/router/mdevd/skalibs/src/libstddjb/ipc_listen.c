/* ISC license. */

#include <skalibs/nonposix.h> 
#include <sys/socket.h>
#include <skalibs/socket.h>

int ipc_listen (int s, int backlog)
{
  return listen(s, backlog) ;
}
