/* ISC license. */

#include <skalibs/djbunix.h>

int fd_sanitize (void)
{
  if (!fd_ensure_open(2, 1)) return 0 ;
  if (!fd_ensure_open(1, 1)) return 0 ;
  if (!fd_ensure_open(0, 0)) return 0 ;
  return 1 ;
}
