/* ISC license. */

#include <skalibs/env.h>

size_t env_merg (char const **v, size_t vmax, char const *const *envp, char const *modifs, size_t modiflen)
{
  return env_merge(v, vmax, envp, env_len(envp), modifs, modiflen) ;
}
