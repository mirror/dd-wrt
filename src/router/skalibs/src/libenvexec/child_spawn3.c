/* ISC license. */

#include <unistd.h>

#include <skalibs/types.h>
#include <skalibs/env.h>
#include <skalibs/djbunix.h>
#include <skalibs/cspawn.h>

pid_t child_spawn3 (char const *prog, char const *const *argv, char const *const *envp, int *fds)
{
  pid_t pid ;
  int p[3][2] ;

  if (pipe(p[0]) == -1) return 0 ;
  if (ndelay_on(p[0][0]) == -1 || coe(p[0][0]) == -1 || pipe(p[1]) == -1) goto errp0 ;
  if (ndelay_on(p[1][1]) == -1 || coe(p[1][1]) == -1 || pipe(p[2]) == -1) goto errp1 ;
  if (ndelay_on(p[2][0]) == -1 || coe(p[2][0]) == -1) goto err ;

  {
    cspawn_fileaction fa[2] =
    {
      [0] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = fds[0], [1] = p[1][0] } } },
      [1] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = fds[1], [1] = p[0][1] } } }
    } ;
    size_t envlen = env_len(envp) ;
    size_t m = sizeof(SKALIBS_CHILD_SPAWN_FDS_ENVVAR) ;
    char modifs[sizeof(SKALIBS_CHILD_SPAWN_FDS_ENVVAR) + UINT_FMT] = SKALIBS_CHILD_SPAWN_FDS_ENVVAR "=" ;
    char const *newenv[envlen + 2] ;
    m += uint_fmt(modifs + sizeof(SKALIBS_CHILD_SPAWN_FDS_ENVVAR), p[2][1]) ;
    modifs[m++] = 0 ;
    env_mergen(newenv, envlen + 2, envp, envlen, modifs, m, 1) ;
    pid = cspawn(prog, argv, newenv, CSPAWN_FLAGS_SIGBLOCKNONE, fa, 2) ;
    if (!pid) goto err ;
  }

  fd_close(p[2][1]) ;
  fd_close(p[1][0]) ;
  fd_close(p[0][1]) ;
  fds[0] = p[0][0] ;
  fds[1] = p[1][1] ;
  fds[2] = p[2][0] ;
  return pid ;

 err:
  fd_close(p[2][1]) ;
  fd_close(p[2][0]) ;
 errp1:
  fd_close(p[1][1]) ;
  fd_close(p[1][0]) ;
 errp0:
  fd_close(p[0][1]) ;
  fd_close(p[0][0]) ;
  return 0 ;
}
