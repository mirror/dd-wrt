/* ISC license. */

#ifndef SKALIBS_AVLTREEN_H
#define SKALIBS_AVLTREEN_H

#include <stdint.h>

#include <skalibs/functypes.h>
#include <skalibs/genset.h>
#include <skalibs/avlnode.h>

 /*
    avltreen is the structure managing the AVL tree.
    It needs pre-declared arrays: "storage", an array of avlnode,
    and "freelist", an array of uint32_t, given as arguments to
    avltreen_init(). Pointers to those arrays are then stored in
    the genset.
 */

typedef struct avltreen_s avltreen, *avltreen_ref ;
struct avltreen_s
{
  genset x ;
  uint32_t root ;
  dtok_func_ref dtok ;
  cmp_func_ref kcmp ;
  void *external ;
} ;

#define AVLTREEN_ZERO { .x = GENSET_ZERO, .root = UINT32_MAX, .dtok = 0, .kcmp = 0, .external = 0 }
#define avltreen_totalsize(t) ((t)->x.max)
#define avltreen_len(t) genset_n(&(t)->x)
#define avltreen_nodes(t) ((avlnode *)(t)->x.storage)
#define avltreen_data(t, i) (avltreen_nodes(t)[i].data)
#define avltreen_root(t) ((t)->root)
#define avltreen_setroot(t, r) ((t)->root = (r))
extern void avltreen_init (avltreen *, avlnode *, uint32_t *, uint32_t, dtok_func_ref, cmp_func_ref, void *) ;

#define AVLTREEN_DECLARE_AND_INIT(name, size, dtk, cmp, p) \
avlnode name##_storage[size] ; \
uint32_t name##_freelist[size] ; \
avltreen name ; \
avltreen_init(&name, name##_storage, name##_freelist, size, dtk, cmp, p)

#define avltreen_searchnode(t, k) avlnode_searchnode(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), (k), (t)->dtok, (t)->kcmp, (t)->external)
#define avltreen_search(t, k, data) avlnode_search(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), k, (data), (t)->dtok, (t)->kcmp, (t)->external)
#define avltreen_height(t) avlnode_height(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t))

#define avltreen_extremenode(t, h) avlnode_extremenode(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), h)
#define avltreen_minnode(t) avltreen_extremenode((t), 0)
#define avltreen_maxnode(t) avltreen_extremenode((t), 1)

#define avltreen_extreme(t, h, data) avlnode_extreme(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), (h), data)
#define avltreen_min(t, data) avltreen_extreme((t), 0, data)
#define avltreen_max(t, data) avltreen_extreme((t), 1, data)

extern uint32_t avltreen_newnode (avltreen *, uint32_t) ;
#define avltreen_insertnode(t, i) avltreen_setroot(t, avlnode_insertnode(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), i, (t)->dtok, (t)->kcmp, (t)->external))
extern int avltreen_insert (avltreen *, uint32_t) ;

extern int avltreen_delete (avltreen *, void const *) ;

#define avltreen_iter(t, f, p) avlnode_iter(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), f, p)
#define avltreen_iter_nocancel(t, cut, f, p) avlnode_iter_nocancel(avltreen_nodes(t), avltreen_totalsize(t), cut, avltreen_root(t), f, p)
#define avltreen_iter_withcancel(t, f, cancelf, p) avlnode_iter_withcancel(avltreen_nodes(t), avltreen_totalsize(t), avltreen_root(t), f, cancelf, p)


#define AVLTREEB_SPACE(n) (sizeof(avltreen) + (n)*sizeof(avlnode) + (n)*sizeof(uint32_t))
#define avltreeb_totalsize(t) avltreen_totalsize((avltreen const *)(t))
#define avltreeb_len(t) avltreen_len((avltreen const *)(t))
#define avltreeb_nodes(t) avltreen_nodes((avltreen const *)(t))
#define avltreeb_data(t, d) avltreen_data((avltreen const *)(t), d)
#define avltreeb_root(t) avltreen_root((avltreen const *)(t))
#define avltreeb_setroot(t, r) avltreen_setroot((avltreen *)(t), r)

extern void avltreeb_init (void *, uint32_t, dtok_func_ref, cmp_func_ref, void *) ;
#define avltreeb_searchnode(t, k) avltreen_searchnode((avltreen const *)(t), k)
#define avltreeb_search(t, k, data) avltreen_search((avltreen const *)(t), k, data)
#define avltreeb_height(t) avltreen_height((avltreen const *)(t))
#define avltreeb_extremenode(t, h) avltreen_extremenode((avltreen const *)(t), h)
#define avltreeb_minnode(t) avltreen_minnode((avltreen const *)(t))
#define avltreeb_maxnode(t) avltreen_maxnode((avltreen const *)(t))
#define avltreeb_extreme(t, h, data) avltreen_extreme((avltreen const *)(t), h, data)
#define avltreeb_min(t, data) avltreen_min((avltreen const *)(t), data)
#define avltreeb_max(t, data) avltreen_max((avltreen const *)(t), data)

#define avltreeb_newnode(t, i) avltreen_newnode((avltreen *)(t), d)
#define avltreeb_insertnode(t, i) avltreen_insertnode((avltreen *)(t), i)
#define avltreeb_insert(t, d) avltreen_insert((avltreen *)(t), d)
#define avltreeb_delete(t, k) avltreen_delete((avltreen *)(t), k)

#define avltreeb_iter(t, f, p) avltreen_iter((avltreen *)(t), f, p)
#define avltreeb_iter_nocancel(t, cut, f, p) avltreen_iter_nocancel((avltreen *)(t), cut, f, p)
#define avltreeb_iter_withcancel(t, f, cancelf, p) avltreen_iter_withcancel((avltreen *)(t), f, cancelf, p)


#endif
