/* ISC license. */

#include <unistd.h>

#include <skalibs/exec.h>

void mexec0_afn (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, size_t modifn)
{
  if (!argv[0]) _exit(0) ;
  mexec_afn(file, argv, envp, envlen, modif, modiflen, modifn) ;
}
