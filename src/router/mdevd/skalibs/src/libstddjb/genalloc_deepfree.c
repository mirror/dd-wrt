/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>

void genalloc_deepfree_size (genalloc *g, free_func_ref f, size_t size)
{
  size_t len = g->len / size ;
  size_t i = 0 ;
  for (; i < len ; i++) (*f)(g->s + i * size) ;
  stralloc_free(g) ;
}
