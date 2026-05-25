/* ISC license. */

#include <skalibs/djbunix.h>

int hiercopy_loose_tmp (char const *src, char const *dst, stralloc *tmp)
{
  return hiercopy_internal(src, dst, tmp, 1) ;
}
