/* ISC license. */

#include <unistd.h>

#include <skalibs/djbunix.h>
#include <skalibs/cspawn.h>

pid_t child_spawn2 (char const *prog, char const *const *argv, char const *const *envp, int *fds)
{
  pid_t pid ;
  int p[2][2] ;
  if (pipe(p[0]) == -1) return 0 ;
  if (ndelay_on(p[0][0]) == -1 || coe(p[0][0]) == -1 || pipe(p[1]) == -1) goto errp ;
  if (ndelay_on(p[1][1]) == -1 || coe(p[1][1]) == -1) goto err ;

  {
    cspawn_fileaction fa[2] =
    {
      [0] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = fds[0], [1] = p[1][0] } } },
      [1] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = fds[1], [1] = p[0][1] } } }
    } ;
    pid = cspawn(prog, argv, envp, CSPAWN_FLAGS_SIGBLOCKNONE, fa, 2) ;
    if (!pid) goto err ;
  }

  fd_close(p[0][1]) ;
  fd_close(p[1][0]) ;
  fds[0] = p[0][0] ;
  fds[1] = p[1][1] ;
  return pid ;

 err:
  fd_close(p[1][1]) ;
  fd_close(p[1][0]) ;
 errp:
  fd_close(p[0][1]) ;
  fd_close(p[0][0]) ;
  return 0 ;
}
