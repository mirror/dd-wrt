/* ISC license. */

#include <skalibs/env.h>
#include <skalibs/cspawn.h>

pid_t gmspawn_afn (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, size_t modifn, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  char const *newenvp[envlen + modifn + 1] ;
  env_merge(newenvp, envlen + modifn + 1, envp, envlen, modif, modiflen) ;
  return gcspawn(file, argv, newenvp, flags, fa, n) ;
}
