/* ISC license. */

#include <errno.h>
#include <skalibs/avlnode.h>

int avlnode_iter_withcancel (avlnode *tree, uint32_t max, uint32_t root, avliter_func_ref f, avliter_func_ref cancelf, void *stuff)
{
  uint32_t cut = avlnode_iter_nocancel(tree, max, max, root, f, stuff) ;
  if (cut != max)
  {
    int e = errno ;
    avlnode_iter_nocancel(tree, max, cut, root, cancelf, stuff) ;
    errno = e ;
    return 0 ;
  }
  return 1 ;
}
