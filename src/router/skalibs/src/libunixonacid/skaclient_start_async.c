/* ISC license. */

#include <errno.h>

#include <skalibs/error.h>
#include <skalibs/kolbak.h>
#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>
#include "skaclient-internal.h"

int skaclient_start_async (
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
  char const *path,
  uint32_t options,
  char const *before,
  size_t beforelen,
  char const *after,
  size_t afterlen,
  skaclient_cbdata *blah)
{
  int fd = ipc_stream_nbcoe() ;
  if (fd < 0) return 0 ;
  if ((!ipc_connect(fd, path) && !error_isalready(errno))
   || !skaclient_init(a, fd, bufss, bufsn, auxbufss, auxbufsn, bufas, bufan, auxbufas, auxbufan, q, qlen, before, beforelen))
  {
    fd_close(fd) ;
    return 0 ;
  }
  a->pid = 0 ;
  a->options = options & ~SKACLIENT_OPTION_WAITPID ;
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
