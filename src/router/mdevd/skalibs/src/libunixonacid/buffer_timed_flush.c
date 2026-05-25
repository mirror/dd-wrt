/* ISC license. */

#include <skalibs/buffer.h>
#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>

static int buffer_isnonempty (buffer *b)
{
  return !buffer_isempty(b) ;
}

int buffer_timed_flush (buffer *b, tain const *deadline, tain *stamp)
{
  return timed_flush(b, (init_func_ref)&buffer_getfd, (init_func_ref)&buffer_isnonempty, (init_func_ref)&buffer_flush, deadline, stamp) ;
}
