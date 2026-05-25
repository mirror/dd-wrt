/* ISC license. */

#include <unistd.h>

#include <skalibs/exec.h>

void mexec0_af (char const *file, char const *const *argv, char const *const *envp, size_t envlen)
{
  if (!argv[0]) _exit(0) ;
  mexec_af(file, argv, envp, envlen) ;
}
