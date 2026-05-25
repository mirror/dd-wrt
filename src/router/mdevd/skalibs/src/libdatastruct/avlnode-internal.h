/* ISC license. */

#ifndef AVLNODE_INTERNAL_H
#define AVLNODE_INTERNAL_H

#include <stdint.h>
#include <skalibs/avlnode.h>

#define avlnode_ufroms(c) ((c) > 0)
#define avlnode_sfromu(h) ((h) ? 1 : -1)

extern uint32_t avlnode_rotate (avlnode *, uint32_t, uint32_t, int) ;
extern uint32_t avlnode_doublerotate (avlnode *, uint32_t, uint32_t, int) ;
#define avlnode_rotate_maydouble(s, max, r, h, isdouble) ((isdouble) ? avlnode_doublerotate(s, max, r, h) : avlnode_rotate(s, max, r, h))

#endif
