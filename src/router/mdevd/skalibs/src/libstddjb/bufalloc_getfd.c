/* ISC license. */

#include <skalibs/bufalloc.h>

int bufalloc_getfd (bufalloc const *ba)
{
  return bufalloc_fd(ba) ;
}
