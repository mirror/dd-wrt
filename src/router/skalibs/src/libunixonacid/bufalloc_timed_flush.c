/* ISC license. */

#include <skalibs/bufalloc.h>
#include <skalibs/functypes.h>
#include <skalibs/unix-timed.h>

static int bufalloc_isnonempty (bufalloc *ba)
{
  return !!bufalloc_len(ba) ;
}

int bufalloc_timed_flush (bufalloc *ba, tain const *deadline, tain *stamp)
{
  return timed_flush(ba, (init_func_ref)&bufalloc_getfd, (init_func_ref)&bufalloc_isnonempty, (init_func_ref)&bufalloc_flush, deadline, stamp) ;
}
