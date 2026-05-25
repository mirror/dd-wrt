/* ISC license. */

#include <errno.h>

#include <skalibs/exec.h>
#include <skalibs/strerr.h>

void xmexec_af (char const *file, char const *const *argv, char const *const *envp, size_t envlen)
{
  mexec_af(file, argv, envp, envlen) ;
  strerr_dieexec(errno == ENOENT ? 127 : 126, file) ;
}
