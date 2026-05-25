/* ISC license. */

#include <errno.h>

#include <skalibs/skaclient.h>
#include <skalibs/unixmessage.h>
#include <skalibs/posixishard.h>

int skaclient_default_cb (unixmessage const *m, void *p)
{
  unsigned char *err = p ;
  if (m->len != 1 || m->nfds) return (errno = EPROTO, 0) ;
  *err = m->s[0] ;
  return 1 ;
}
