/* ISC license. */

#include <errno.h>
#include <signal.h>

#include <skalibs/cspawn.h>
#include <skalibs/djbunix.h>
#include <skalibs/kolbak.h>
#include <skalibs/skaclient.h>
#include "skaclient-internal.h"

int skaclient_startf_async (
  skaclient *a,
  char *bufss,
  size_t bufsn,
  char *auxbufss,
  size_t auxbufsn,
  char *bufas,
  size_t bufan,
  char *auxbufas,
  size_t auxbufan,
  kolbak_closure *q,
  size_t qlen,
  char const *prog,
  char const *const *argv,
  char const *const *envp,
  uint32_t options,
  char const *before,
  size_t beforelen,
  char const *after,
  size_t afterlen,
  skaclient_cbdata *blah)
{
  int fd ;
  pid_t pid = child_spawn1_socket(prog, argv, envp, &fd) ;
  if (!pid) return 0 ;
  if (ndelay_on(fd) < 0
   || coe(fd) < 0
   || !skaclient_init(a, fd, bufss, bufsn, auxbufss, auxbufsn, bufas, bufan, auxbufas, auxbufan, q, qlen, before, beforelen))
  {
    fd_close(fd) ;
    if (options & SKACLIENT_OPTION_WAITPID)
    {
      int e = errno ;
      int wstat ;
      waitpid_nointr(a->pid, &wstat, 0) ;
      errno = e ;
    }
    return 0 ;
  }
  a->pid = pid ;
  a->options = options ;
  if (!kolbak_enqueue(&a->kq, (unixmessage_handler_func_ref)&skaclient_start_cb, blah))
  {
    skaclient_end(a) ;
    return 0 ;
  }
  blah->a = a ;
  blah->after = after ;
  blah->afterlen = afterlen ;
  return 1 ;
}
