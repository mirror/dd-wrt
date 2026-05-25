/* ISC license. */

#include <skalibs/nonposix.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int open_excl (char const *fn)
{
  return open3(fn, O_WRONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0666) ;
}
