/* ISC license. */

#include <skalibs/nonposix.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int open_read (char const *fn)
{
  return open2(fn, O_RDONLY | O_NONBLOCK) ;
}
