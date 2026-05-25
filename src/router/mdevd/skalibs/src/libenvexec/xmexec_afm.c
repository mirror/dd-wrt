/* ISC license. */

#include <errno.h>

#include <skalibs/exec.h>
#include <skalibs/strerr.h>

void xmexec_afm (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen)
{
  mexec_afm(file, argv, envp, envlen, modif, modiflen) ;
  strerr_dieexec(errno == ENOENT ? 127 : 126, file) ;
}
