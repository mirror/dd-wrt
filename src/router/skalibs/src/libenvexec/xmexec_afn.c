/* ISC license. */

#include <errno.h>

#include <skalibs/exec.h>
#include <skalibs/strerr.h>

void xmexec_afn (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, size_t modifn)
{
  mexec_afn(file, argv, envp, envlen, modif, modiflen, modifn) ;
  strerr_dieexec(errno == ENOENT ? 127 : 126, file) ;
}
