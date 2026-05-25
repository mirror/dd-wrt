/* ISC license. */

#include <errno.h>

#include <skalibs/exec.h>
#include <skalibs/strerr.h>

void xexec_ae (char const *file, char const *const *argv, char const *const *envp)
{
  exec_ae(file, argv, envp) ;
  strerr_dieexec(errno == ENOENT ? 127 : 126, file) ;
}
