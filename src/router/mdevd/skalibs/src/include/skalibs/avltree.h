/* ISC license. */

#ifndef SKALIBS_AVLTREE_H
#define SKALIBS_AVLTREE_H

#include <stdint.h>

#include <skalibs/functypes.h>
#include <skalibs/gensetdyn.h>
#include <skalibs/avlnode.h>

typedef struct avltree_s avltree, *avltree_ref ;
struct avltree_s
{
  gensetdyn x ;
  uint32_t root ;
  dtok_func_ref dtok ;
  cmp_func_ref kcmp ;
  void *external ;
} ;

#define AVLTREE_ZERO { .x = GENSETDYN_ZERO, .root = UINT32_MAX, .dtok = 0, .kcmp = 0, .external = 0 }
extern avltree const avltree_zero ;
#define avltree_len(t) gensetdyn_n(&(t)->x)
#define avltree_totalsize(t) ((uint32_t)(t)->x.storage.len)
#define avltree_nodes(t) ((avlnode *)(t)->x.storage.s)
#define avltree_data(t, i) ((uint32_t)avltree_nodes(t)[i].data)
#define avltree_root(t) ((t)->root)
#define avltree_setroot(t, r) ((t)->root = (r))

extern void avltree_free (avltree *) ;
extern void avltree_init (avltree *, uint32_t, uint32_t, uint32_t, dtok_func_ref, cmp_func_ref, void *) ;
#define AVLTREE_INIT(b, num, den, dtk, f, p) { .x = GENSETDYN_INIT(avlnode, (b), num, den), .root = UINT32_MAX, .dtok = (dtk), .kcmp = (f), .external = (p) }

#define avltree_searchnode(t, k) avlnode_searchnode(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), k, (t)->dtok, (t)->kcmp, (t)->external)
#define avltree_search(t, k, data) avlnode_search(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), k, (data), (t)->dtok,  (t)->kcmp, (t)->external)

#define avltree_height(t) avlnode_height(avltree_nodes(t), avltree_totalsize(t), avltree_root(t))

#define avltree_extremenode(t, h) avlnode_extremenode(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), h)
#define avltree_minnode(t) avltree_extremenode((t), 0)
#define avltree_maxnode(t) avltree_extremenode((t), 1)

#define avltree_extreme(t, h, data) avlnode_extreme(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), (h), data)
#define avltree_min(t, data) avltree_extreme((t), 0, data)
#define avltree_max(t, data) avltree_extreme((t), 1, data)

extern int avltree_newnode (avltree *, uint32_t, uint32_t *) ;
#define avltree_insertnode(t, i) avltree_setroot(t, avlnode_insertnode(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), i, (t)->dtok, (t)->kcmp, (t)->external))
extern int avltree_insert (avltree *, uint32_t) ;

extern int avltree_delete (avltree *, void const *) ;

#define avltree_iter_nocancel(t, cut, f, p) avlnode_iter_nocancel(avltree_nodes(t), avltree_totalsize(t), cut, avltree_root(t), f, p)
#define avltree_iter(t, f, p) avlnode_iter(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), f, p)
#define avltree_iter_withcancel(t, f, cancelf, p) avlnode_iter_withcancel(avltree_nodes(t), avltree_totalsize(t), avltree_root(t), f, cancelf, p)

#endif
