/* ISC license. */

/* MT-unsafe */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

static char buf[BUFFER_INSIZE] ;
buffer buffer_0_ = BUFFER_INIT(&fd_readv, 0, buf, BUFFER_INSIZE) ;
