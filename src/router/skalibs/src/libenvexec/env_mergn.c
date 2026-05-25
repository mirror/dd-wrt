/* ISC license. */

#include <skalibs/env.h>

size_t env_mergn (char const **v, size_t vmax, char const *const *envp, char const *modifs, size_t modiflen, size_t modifn)
{
  return env_mergen(v, vmax, envp, env_len(envp), modifs, modiflen, modifn) ;
}
