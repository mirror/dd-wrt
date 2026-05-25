/* ISC license. */

/* MT-unsafe */

#include <skalibs/unixmessage.h>

static char mainbuf[UNIXMESSAGE_BUFSIZE] ;
static char auxbuf[UNIXMESSAGE_AUXBUFSIZE] ;

unixmessage_receiver unixmessage_receiver_0_ = UNIXMESSAGE_RECEIVER_INIT(0, mainbuf, UNIXMESSAGE_BUFSIZE, auxbuf, UNIXMESSAGE_AUXBUFSIZE) ;
