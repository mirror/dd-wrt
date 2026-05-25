/* ISC license. */

#include <skalibs/env.h>

size_t env_mergen (char const **v, size_t vmax, char const *const *envp, size_t envlen, char const *modifs, size_t modiflen, size_t modifn)
{
  (void)modifn ;
  return env_merge(v, vmax, envp, envlen, modifs, modiflen) ;
}
