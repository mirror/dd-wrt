/* ISC license. */

#include <skalibs/djbunix.h>
#include <skalibs/unixmessage.h>

void unixmessage_drop (unixmessage const *m)
{
  unsigned int i = m->nfds ;
  while (i--) fd_close(m->fds[i]) ;
}
