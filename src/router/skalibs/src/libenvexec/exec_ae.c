/* ISC license. */

#include <stdlib.h>

#include <skalibs/config.h>
#include <skalibs/posixplz.h>
#include <skalibs/exec.h>

void exec_ae (char const *file, char const *const *argv, char const *const *envp)
{
  char const *path = getenv("PATH") ;
  if (!path) path = SKALIBS_DEFAULTPATH ;
  execvep(file, argv, envp, path) ;
}
