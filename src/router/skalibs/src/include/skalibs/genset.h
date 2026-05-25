/* ISC license. */

#ifndef SKALIBS_GENSET_H
#define SKALIBS_GENSET_H

#include <stdint.h>

#include <skalibs/functypes.h>

typedef struct genset_s genset, *genset_ref ;
struct genset_s
{
  char *storage ;
  uint32_t *freelist ;
  uint32_t esize ;
  uint32_t max ;
  uint32_t sp ;
} ;

#define GENSET_ZERO { .storage = 0, .freelist = 0, .esize = 1, .max = 0, .sp = 0 }
extern void genset_init (genset *, void *, uint32_t *, uint32_t, uint32_t) ;
#define GENSET_init(g, type, storage, fl, size) genset_init(g, storage, fl, sizeof(type), size)

#define genset_p(type, g, i) ((type *)((g)->storage + (i) * (g)->esize))
extern uint32_t genset_new (genset *) ;
extern int genset_delete (genset *, uint32_t) ;
#define genset_n(g) ((g)->max - (g)->sp)
extern uint32_t genset_iter_nocancel (genset *, uint32_t, iter_func_ref, void *) ;
#define genset_iter(g, f, stuff) genset_iter_nocancel(g, (g)->max, f, stuff)
extern int genset_iter_withcancel (genset *, iter_func_ref, iter_func_ref, void *) ;
extern void genset_deepfree (genset *, free_func_ref) ;

#define GENSETB_TYPE(type, size) struct { type storage[size] ; uint32_t freelist[size] ; genset info ; }
#define GENSETB_init(type, g, size) GENSET_init(&(g)->info, type, (g)->storage, (g)->freelist, size)
#define gensetb_p(type, g, i) genset_p(type, &(g)->info, i)
#define gensetb_new(g) genset_new(&(g)->info)
#define gensetb_delete(g, i) genset_delete(&(g)->info, i)
#define gensetb_n(g) genset_n(&(g)->info)
#define gensetb_iter(g, f, p) genset_iter(&(g)->info, f, p)
#define gensetb_deepfree(g, f) genset_deepfree(&(g)->info, f)

#endif
