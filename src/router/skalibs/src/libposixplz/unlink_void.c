/* ISC license. */

#include <unistd.h>
#include <errno.h>
#include <skalibs/posixplz.h>

void unlink_void (char const *file)
{
  int e = errno ;
  unlink(file) ;
  errno = e ;
}
