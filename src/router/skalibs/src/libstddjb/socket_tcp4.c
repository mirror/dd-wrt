/* ISC license. */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <skalibs/socket.h>

int socket_tcp4_internal (unsigned int flags)
{
  return socket_internal(AF_INET, SOCK_STREAM, 0, flags) ;
}
