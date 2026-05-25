/* ISC license. */

#include <skalibs/functypes.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/bufalloc.h>

int bufalloc_flush (bufalloc *ba)
{
  ba->p += allreadwrite((io_func_ref)ba->op, ba->fd, ba->x.s + ba->p, ba->x.len - ba->p) ;
  bufalloc_clean(ba) ;
  return !ba->x.len ;
}
