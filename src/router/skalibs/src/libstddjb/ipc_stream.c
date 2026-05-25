/* ISC license. */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <skalibs/socket.h>

int ipc_stream_internal (unsigned int flags)
{
  return socket_internal(AF_UNIX, SOCK_STREAM, 0, flags) ;
}
