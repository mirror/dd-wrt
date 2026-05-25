/* ISC license. */

/* MT-unsafe */

#include <skalibs/buffer.h>

static char buf[BUFFER_INSIZE] ;
buffer buffer_0f1_ = BUFFER_INIT(&buffer_flush1read, 0, buf, BUFFER_INSIZE) ;
