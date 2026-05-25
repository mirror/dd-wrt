/* ISC license. */

#include <string.h>
#include <unistd.h>

#include <skalibs/types.h>
#include <skalibs/djbunix.h>
#include <skalibs/env.h>
#include <skalibs/cspawn.h>

pid_t child_spawn (char const *prog, char const *const *argv, char const *const *envp, int *fds, size_t n)
{
  pid_t pid ;
  cspawn_fileaction fa[2] =
  {
    [0] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = 1 } } },
    [1] = { .type = CSPAWN_FA_MOVE, .x = { .fd2 = { [0] = 0 } } }
  } ;
  size_t m = sizeof(SKALIBS_CHILD_SPAWN_FDS_ENVVAR) ;
  size_t envlen = env_len(envp) ;
  size_t i = 0 ;
  int p[n ? n : 1][2] ;
  char const *newenv[envlen + 2] ;
  char modifs[m + 1 + n * UINT_FMT] ;

  for (; i < n ; i++)
  {
    if (pipe(p[i]) == -1) goto errpi ;
    if ((ndelay_on(p[i][i & 1]) == -1) || (coe(p[i][i & 1]) == -1)) goto errpip ;
  }

  memcpy(modifs, SKALIBS_CHILD_SPAWN_FDS_ENVVAR "=", sizeof(SKALIBS_CHILD_SPAWN_FDS_ENVVAR)) ;
  for (i = 2 ; i < n ; i++)
  {
    m += uint_fmt(modifs + m, p[i][!(i & 1)]) ;
    if (i+1 < n) modifs[m++] = ',' ;
  }
  i = n ;
  modifs[m++] = 0 ;
  env_mergen(newenv, envlen + 2, envp, envlen, modifs, m, 1) ;
  if (n) fa[0].x.fd2[1] = p[0][1] ;
  if (n >= 2) fa[1].x.fd2[1] = p[1][0] ;
  pid = cspawn(prog, argv, newenv, CSPAWN_FLAGS_SIGBLOCKNONE, fa, n < 2 ? n : 2) ;
  if (!pid) goto errpi ;

  for (i = 0 ; i < n ; i++)
  {
    fd_close(p[i][!(i & 1)]) ;
    fds[i] = p[i][i & 1] ;
  }
  return pid ;

 errpip:
  i++ ;
 errpi:
  while (i--)
  {
    fd_close(p[i][1]) ;
    fd_close(p[i][0]) ;
  }
  return 0 ;
}
