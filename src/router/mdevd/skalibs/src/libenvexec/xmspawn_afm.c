/* ISC license. */

#include <errno.h>

#include <skalibs/strerr.h>
#include <skalibs/cspawn.h>

pid_t xmspawn_afm (char const *file, char const *const *argv, char const *const *envp, size_t envlen, char const *modif, size_t modiflen, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  pid_t pid = mspawn_afm(file, argv, envp, envlen, modif, modiflen, flags, fa, n) ;
  if (!pid) strerr_diefu2sys(errno == ENOENT ? 127 : 126, "spawn ", file) ;
  return pid ;
}
