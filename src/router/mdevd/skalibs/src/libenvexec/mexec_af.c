/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>

static stralloc modifsa = STRALLOC_ZERO ;

int env_mexec (char const *key, char const *value)
{
  return env_addmodif(&modifsa, key, value) ;
}

void mexec_af (char const *file, char const *const *argv, char const *const *envp, size_t envlen)
{
  mexec_afm(file, argv, envp, envlen, modifsa.s, modifsa.len) ;
}
