/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/cspawn.h>
#include "cspawn-internal.h"

pid_t child_spawn1_internal (char const *prog, char const *const *argv, char const *const *envp, int *p, int to)
{
  pid_t pid ;
  cspawn_fileaction fa[3] =
  {
    [0] = { .type = CSPAWN_FA_CLOSE, .x = { .fd = p[!(to & 1)]} },
    [1] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = to & 1, [1] = p[to & 1] } } },
    [2] = { .type = CSPAWN_FA_COPY, .x = { .fd2 = { [0] = !(to & 1), [1] = to & 1 } } }
  } ;

  pid = cspawn(prog, argv, envp, CSPAWN_FLAGS_SIGBLOCKNONE, fa, 2 + !!(to & 2)) ;
  if (!pid) goto err ;

  fd_close(p[to & 1]) ;
  return pid ;

 err:
  fd_close(p[1]) ;
  fd_close(p[0]) ;
  return 0 ;
}
