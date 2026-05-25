/* ISC license. */

#include <errno.h>

#include <skalibs/direntry.h>

void dir_close (DIR *dir)
{
  int e = errno ;
  for (;;)
  {
    if (closedir(dir) == 0) break ;
    if (errno != EINTR) break ;
  }
  errno = e ;
}
