/* ISC license. */

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>
#include <skalibs/bigkv.h>

void bigkv_free (bigkv *b)
{
  avltree_free(&b->map) ;
  genalloc_free(bigkv_node, &b->nodes) ;
  stralloc_free(&b->storage) ;
  *b = bigkv_zero ;
}
