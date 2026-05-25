/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/unixmessage.h>
#include <skalibs/skaclient.h>
#include <skalibs/posixishard.h>
#include "skaclient-internal.h"

int skaclient_start_cb (unixmessage const *m, skaclient_cbdata *blah)
{
  if (m->len != blah->afterlen
   || memcmp(m->s, blah->after, m->len)
   || m->nfds != 1)
  {
    unixmessage_drop(m) ;
    return (errno = EPROTO, 0) ;
  }
  blah->a->asyncin.fd = m->fds[0] ;
  blah->a->asyncout.fd = m->fds[0] ;
  if (!(blah->a->options & SKACLIENT_OPTION_ASYNC_ACCEPT_FDS))
    unixmessage_receiver_refuse_fds(&blah->a->asyncin) ;
  if (!(blah->a->options & SKACLIENT_OPTION_SYNC_ACCEPT_FDS))
    unixmessage_receiver_refuse_fds(&blah->a->syncin) ;
  return 1 ;
}
