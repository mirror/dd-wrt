/* ISC license. */

/* MT-unsafe */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

static char buf[BUFFER_ERRSIZE] ;
buffer buffer_2_ = BUFFER_INIT(&fd_writev, 2, buf, BUFFER_ERRSIZE) ;
