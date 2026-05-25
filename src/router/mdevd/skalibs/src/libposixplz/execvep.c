/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <skalibs/posixplz.h>
#include "posixplz-internal.h"

void execvep (char const *file, char const *const *argv, char const *const *envp, char const *path)
{
  if (strchr(file, '/'))
    execve(file, (char *const *)argv, (char *const *)envp) ;
  else
    execvep_internal(file, argv, envp, path) ;
}
