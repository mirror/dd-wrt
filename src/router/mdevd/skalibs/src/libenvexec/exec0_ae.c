/* ISC license. */

#include <unistd.h>

#include <skalibs/exec.h>

void exec0_ae (char const *file, char const *const *argv, char const *const *envp)
{
  if (!argv[0]) _exit(0) ;
  exec_ae(file, argv, envp) ;
}
