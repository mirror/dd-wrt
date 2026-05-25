/* ISC license. */

#include <errno.h>

#include <skalibs/kolbak.h>
#include <skalibs/skaclient.h>
#include "skaclient-internal.h"

int skaclient_start (
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
  tain const *deadline,
  tain *stamp)
{
  skaclient_cbdata blah ;
  unixmessage m ;
  int r ;
  if (!skaclient_start_async(a, bufss, bufsn, auxbufss, auxbufsn, bufas, bufan, auxbufas, auxbufan, q, qlen, path, options, before, beforelen, after, afterlen, &blah)) return 0 ;
  if (!skaclient_timed_flush(a, deadline, stamp))
  {
    int e = errno ;
    skaclient_end(a) ;
    errno = e ;
    return 0 ;
  }
  r = unixmessage_timed_receive(&a->syncin, &m, deadline, stamp) ;
  if (r < 1)
  {
    int e = errno ;
    if (!r) e = EPIPE ;
    skaclient_end(a) ;
    errno = e ;
    return 0 ;
  }
  return kolbak_call(&m, &a->kq) ;
}
