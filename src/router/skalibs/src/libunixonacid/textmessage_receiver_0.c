/* ISC license. */

/* MT-unsafe */

#include <skalibs/buffer.h>
#include <skalibs/textmessage.h>

static char buf[BUFFER_INSIZE] ;
textmessage_receiver textmessage_receiver_0_ = TEXTMESSAGE_RECEIVER_INIT(0, buf, BUFFER_INSIZE, TEXTMESSAGE_MAXLEN) ;
