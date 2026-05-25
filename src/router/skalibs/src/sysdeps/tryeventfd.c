/* ISC license. */

#include <sys/eventfd.h>

int main (void)
{
  int fd = eventfd(0, EFD_NONBLOCK) ;
  if (fd < 0) return 1 ;
  return 0 ;
}
