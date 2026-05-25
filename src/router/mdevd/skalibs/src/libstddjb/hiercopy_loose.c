/* ISC license. */

#include <skalibs/skamisc.h>
#include <skalibs/djbunix.h>

int hiercopy_loose (char const *src, char const *dst)
{
  return hiercopy_internal(src, dst, &satmp, 1) ;
}
