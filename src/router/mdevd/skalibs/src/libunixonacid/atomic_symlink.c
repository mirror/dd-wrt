/* ISC license. */

#include <skalibs/unix-transactional.h>

int atomic_symlink (char const *target, char const *name, char const *suffix)
{
  (void)suffix ;
  return atomic_symlink4(target, name, 0, 0) ;
}
