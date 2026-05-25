/* ISC license. */

#include <skalibs/env.h>

int env_dump (char const *dir, mode_t mode, char const *const *envp)
{
  return env_dump4(dir, mode, envp, 1) ;
}
