/* ISC license. */

#include <skalibs/nonposix.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int openc_create (char const *fn)
{
  return open3(fn, O_WRONLY | O_NONBLOCK | O_CREAT | O_CLOEXEC, 0666) ;
}
