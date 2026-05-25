/* ISC license. */

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <skalibs/socket.h>

int socket_tcpdelay (int s)
{
  static int const val = 0 ;
  return setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int)) ;
}
