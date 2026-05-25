/* ISC license. */

#include <errno.h>

#include <skalibs/posixplz.h>
#include <skalibs/strerr.h>
#include <skalibs/exec.h>

void xexecvep_loose (char const *file, char const *const *argv, char const *const *envp, char const *path)
{
  execvep_loose(file, argv, envp, path) ;
  strerr_dieexec(errno == ENOENT ? 127 : 126, file) ;
}
