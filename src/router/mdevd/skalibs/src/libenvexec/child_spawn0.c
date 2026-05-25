/* ISC license. */

#include <skalibs/cspawn.h>

pid_t child_spawn0 (char const *prog, char const *const *argv, char const *const *envp)
{
  return cspawn(prog, argv, envp, CSPAWN_FLAGS_SIGBLOCKNONE, 0, 0) ;
}
