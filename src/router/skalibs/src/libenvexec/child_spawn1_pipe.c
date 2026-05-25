/* ISC license. */

#include <unistd.h>

#include <skalibs/cspawn.h>
#include "cspawn-internal.h"

pid_t child_spawn1_pipe (char const *prog, char const *const *argv, char const *const *envp, int *fd, int to)
{
  pid_t pid ;
  int p[2] ;
  if (pipe(p) < 0) return 0 ;
  pid = child_spawn1_internal(prog, argv, envp, p, !!to) ;
  if (!pid) return 0 ;
  *fd = p[!to] ;
  return pid ;
}
