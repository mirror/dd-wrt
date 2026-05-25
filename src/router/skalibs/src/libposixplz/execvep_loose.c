/* ISC license. */

#include <unistd.h>
#include <skalibs/posixplz.h>
#include "posixplz-internal.h"

void execvep_loose (char const *file, char const *const *argv, char const *const *envp, char const *path)
{
  if (file[0] == '/')
    execve(file, (char *const *)argv, (char *const *)envp) ;
  else
    execvep_internal(file, argv, envp, path) ;
}
