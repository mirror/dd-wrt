/* ISC license. */

#ifndef SKALIBS_GENSETDYN_H
#define SKALIBS_GENSETDYN_H

#include <stdint.h>

#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/functypes.h>

typedef struct gensetdyn_s gensetdyn, *gensetdyn_ref ;
struct gensetdyn_s
{
  stralloc storage ;
  genalloc freelist ; /* array of uint32_t */
  uint32_t esize ;
  uint32_t base ;
  uint32_t fracnum ;
  uint32_t fracden ;
} ;

#define GENSETDYN_ZERO { .storage = STRALLOC_ZERO, .freelist = GENALLOC_ZERO, .esize = 1, .base = 0, .fracnum = 0, .fracden = 1 }
extern gensetdyn const gensetdyn_zero ;

#define GENSETDYN_INIT(type, b, num, den) { .storage = STRALLOC_ZERO, .freelist = GENALLOC_ZERO, .esize = sizeof(type), .base = (b), .fracnum = (num), .fracden = (den) }
extern void gensetdyn_init (gensetdyn *, uint32_t, uint32_t, uint32_t, uint32_t) ;

#define gensetdyn_n(g) ((uint32_t)((g)->storage.len - genalloc_len(uint32_t, &(g)->freelist)))
extern int gensetdyn_ready (gensetdyn *, uint32_t) ;
#define gensetdyn_readyplus(x, n) gensetdyn_ready(x, gensetdyn_n(x) + (n))
extern void gensetdyn_free (gensetdyn *) ;
extern void gensetdyn_deepfree (gensetdyn *, free_func_ref) ;

extern int gensetdyn_new (gensetdyn *, uint32_t *) ;
extern int gensetdyn_delete (gensetdyn *, uint32_t) ;

#define gensetdyn_p(g, i) ((g)->storage.s + (i) * (g)->esize)
#define GENSETDYN_P(type, g, i) ((type *)gensetdyn_p(g, i))

extern uint32_t gensetdyn_iter_nocancel (gensetdyn *, uint32_t, iter_func_ref, void *) ;
#define gensetdyn_iter(g, f, stuff) gensetdyn_iter_nocancel(g, gensetdyn_n(g), f, stuff)
extern int gensetdyn_iter_withcancel (gensetdyn *, iter_func_ref, iter_func_ref, void *) ;

#endif
