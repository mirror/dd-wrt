/* ISC license. */

#include <errno.h>
#include <skalibs/genset.h>

void genset_init (genset *x, void *storage, uint32_t *freelist, uint32_t esize, uint32_t max)
{
  uint32_t i = max ;
  x->storage = (char *)storage ;
  x->freelist = freelist ;
  x->esize = esize ;
  x->max = max ;
  x->sp = max ;
  while (i--) freelist[i] = max - 1 - i ;
}

uint32_t genset_new (genset *x)
{
  return x->sp ? x->freelist[--x->sp] : (errno = ENOSPC, x->max) ;
}

int genset_delete (genset *x, uint32_t i)
{
  if ((i >= x->max) || (x->sp >= x->max)) return (errno = EINVAL, 0) ;
  x->freelist[x->sp++] = i ;
  return 1 ;
}
