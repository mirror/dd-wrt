/* ISC license. */

#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>

static ssize_t get (buffer *b)
{
  return sanitize_read(buffer_fill(b)) ;
}

ssize_t buffer_timed_fill (buffer *b, tain const *deadline, tain *stamp)
{
  return timed_get(b, (init_func_ref)&buffer_getfd, (get_func_ref)&get, deadline, stamp) ;
}
