/* ISC license. */

#include <skalibs/env.h>
#include <skalibs/exec.h>

void mexec_afn (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, size_t modifn)
{
  char const *newenvp[envlen + modifn + 1] ;
  env_mergen(newenvp, envlen + modifn + 1, envp, envlen, modif, modiflen, modifn) ;
  exec_ae(file, argv, newenvp) ;
}
