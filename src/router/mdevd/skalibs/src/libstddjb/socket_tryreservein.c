/* ISC license. */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <skalibs/socket.h>

void socket_tryreservein (int s, unsigned int size)
{
  while (size >= 1024)
  {
    if (!setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof size)) return ;
    size -= (size >> 5) ;
  }
}
