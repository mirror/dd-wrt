/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/env.h>
#include <skalibs/cspawn.h>

static stralloc modifsa = STRALLOC_ZERO ;

int env_mspawn (char const *key, char const *value)
{
  return env_addmodif(&modifsa, key, value) ;
}

pid_t mspawn_af (char const *file, char const *const *argv, char const *const *envp, size_t envlen, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  return mspawn_afm(file, argv, envp, envlen, modifsa.s, modifsa.len, flags, fa, n) ;
}
