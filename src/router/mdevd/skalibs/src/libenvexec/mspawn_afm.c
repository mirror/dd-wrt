/* ISC license. */

#include <skalibs/bytestr.h>
#include <skalibs/cspawn.h>

pid_t mspawn_afm (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  return mspawn_afn(file, argv, envp, envlen, modif, modiflen, byte_count(modif, modiflen, '\0'), flags, fa, n) ;
}
