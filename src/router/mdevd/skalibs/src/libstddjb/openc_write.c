/* ISC license. */

#include <skalibs/nonposix.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int openc_write (char const *fn)
{
  return open2(fn, O_WRONLY | O_NONBLOCK | O_CLOEXEC) ;
}
