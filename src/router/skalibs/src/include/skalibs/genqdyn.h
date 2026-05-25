/* ISC license. */

#ifndef SKALIBS_GENQDYN_H
#define SKALIBS_GENQDYN_H

#include <sys/types.h>

#include <skalibs/stralloc.h>

typedef struct genqdyn_s genqdyn, *genqdyn_ref ;
struct genqdyn_s
{
  stralloc queue ;
  size_t esize ;
  size_t head ;
  unsigned int num ;
  unsigned int den ;
} ;

#define GENQDYN_ZERO { .queue = STRALLOC_ZERO, .esize = 1, .head = 0, .num = 0, .den = 1 }
extern genqdyn const genqdyn_zero ;

#define GENQDYN_INIT(type, n, d) { .queue = STRALLOC_ZERO, .esize = sizeof(type), .head = 0, .num = n, .den = d }
extern void genqdyn_init (genqdyn *, size_t, unsigned int, unsigned int) ;

#define genqdyn_n(g) (((g)->queue.len - (g)->head) / (g)->esize)

extern void genqdyn_free (genqdyn *) ;
extern int genqdyn_push (genqdyn *, void const *) ;
extern int genqdyn_unpush (genqdyn *) ;
#define GENQDYN_PEEK(type, g) ((type *)((g)->queue.s + (g)->head))
#define genqdyn_peek(g) GENQDYN_PEEK(void, (g))
extern int genqdyn_pop (genqdyn *) ;

#endif
