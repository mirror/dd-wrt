/* ISC license. */

/* MT-unsafe */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>

static char buf[BUFFER_OUTSIZE_SMALL] ;
buffer buffer_1small_ = BUFFER_INIT(&fd_writev, 1, buf, BUFFER_OUTSIZE_SMALL) ;
