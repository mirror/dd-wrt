/* ISC license. */

#include <unistd.h>

#include <skalibs/exec.h>

void mexec0_afm (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen)
{
  if (!argv[0]) _exit(0) ;
  mexec_afm(file, argv, envp, envlen, modif, modiflen) ;
}
