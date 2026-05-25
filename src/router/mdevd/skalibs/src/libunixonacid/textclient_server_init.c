/* ISC license. */

#include <stdlib.h>

#include <skalibs/cspawn.h>
#include <skalibs/textclient.h>

int textclient_server_init (textmessage_receiver *in, textmessage_sender *syncout, textmessage_sender *asyncout, char const *before, size_t beforelen, char const *after, size_t afterlen, tain const *deadline, tain *stamp)
{
  return getenv(SKALIBS_CHILD_SPAWN_FDS_ENVVAR) ?
    textclient_server_init_frompipe(in, syncout, asyncout, before, beforelen, after, afterlen, deadline, stamp) :
    textclient_server_init_fromsocket(in, syncout, asyncout, before, beforelen, after, afterlen, deadline, stamp) ;
}
