/* ISC license. */

/* MT-unsafe */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

static char buf[BUFFER_OUTSIZE] ;
buffer buffer_1_ = BUFFER_INIT(&fd_writev, 1, buf, BUFFER_OUTSIZE) ;
