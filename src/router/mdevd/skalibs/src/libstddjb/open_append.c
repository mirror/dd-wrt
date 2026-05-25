/* ISC license. */

#include <skalibs/nonposix.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

int open_append (char const *fn)
{
  return open3(fn, O_WRONLY | O_NONBLOCK | O_APPEND | O_CREAT, 0666) ;
}
