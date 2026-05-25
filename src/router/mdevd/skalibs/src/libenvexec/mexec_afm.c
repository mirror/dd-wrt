/* ISC license. */

#include <skalibs/bytestr.h>
#include <skalibs/exec.h>

void mexec_afm (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen)
{
  mexec_afn(file, argv, envp, envlen, modif, modiflen, byte_count(modif, modiflen, '\0')) ;
}
