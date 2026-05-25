/* ISC license. */

#ifndef SKALIBS_AVLNODE_H
#define SKALIBS_AVLNODE_H

#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/functypes.h>


#define AVLNODE_MAXDEPTH 49  /* enough for 2^32 nodes in the worst case */

typedef int avliter_func (uint32_t, unsigned int, void *) ;
typedef avliter_func *avliter_func_ref ;

typedef struct avlnode_s avlnode, *avlnode_ref ;
struct avlnode_s
{
  uint32_t data ;
  uint32_t child[2] ;
  signed char balance : 2 ;
} ;

#define AVLNODE_ZERO { .data = 0, .child = { UINT32_MAX, UINT32_MAX }, .balance = 0 }
extern avlnode const avlnode_zero ;

extern uint32_t avlnode_searchnode (avlnode const *, uint32_t, uint32_t, void const *, dtok_func_ref, cmp_func_ref, void *) ;
extern int avlnode_search (avlnode const *, uint32_t, uint32_t, void const *, uint32_t *, dtok_func_ref, cmp_func_ref, void *) ;
extern unsigned int avlnode_height (avlnode const *, uint32_t, uint32_t) gccattr_pure ;

extern uint32_t avlnode_extremenode (avlnode const *, uint32_t, uint32_t, int) gccattr_pure ;
#define avlnode_minnode(s, max, r) avlnode_extremenode(s, max, (r), 0)
#define avlnode_maxnode(s, max, r) avlnode_extremenode(s, max, (r), 1)

extern int avlnode_extreme (avlnode const *, uint32_t, uint32_t, int, uint32_t *) ;
#define avlnode_min(s, max, r, data) avlnode_extreme(s, max, (r), 0, data)
#define avlnode_max(s, max, r, data) avlnode_extreme(s, max, (r), 1, data)

extern uint32_t avlnode_insertnode (avlnode *, uint32_t, uint32_t, uint32_t, dtok_func_ref, cmp_func_ref, void *) ;
extern uint32_t avlnode_delete (avlnode *, uint32_t, uint32_t *, void const *, dtok_func_ref, cmp_func_ref, void *) ;

extern uint32_t avlnode_iter_nocancel (avlnode *, uint32_t, uint32_t, uint32_t, avliter_func_ref, void *) ;
#define avlnode_iter(tree, max, root, f, stuff) (avlnode_iter_nocancel(tree, max, max, root, f, stuff) == (max))
extern int avlnode_iter_withcancel (avlnode *, uint32_t, uint32_t, avliter_func_ref, avliter_func_ref, void *) ;

#endif
