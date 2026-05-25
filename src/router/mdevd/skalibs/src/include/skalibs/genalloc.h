/* ISC license. */

#ifndef SKALIBS_GENALLOC_H
#define SKALIBS_GENALLOC_H

#include <skalibs/functypes.h>
#include <skalibs/stralloc.h>

typedef stralloc genalloc, *genalloc_ref ;

#define GENALLOC_ZERO STRALLOC_ZERO
#define genalloc_zero stralloc_zero

#define genalloc_s(type, g) ((type *)((g)->s))
#define genalloc_len(type, g) ((g)->len/sizeof(type))
#define genalloc_setlen(type, g, n) ((g)->len = (n)*sizeof(type))

#define genalloc_ready(type, g, n) stralloc_ready((g), (n)*sizeof(type))
#define genalloc_ready_tuned(type, g, n, base, fracnum, fracden) stralloc_ready_tuned((g), (n)*sizeof(type), base, fracnum, fracden)
#define genalloc_readyplus(type, g, n) stralloc_readyplus((g), (n)*sizeof(type))
#define genalloc_free(type, g) stralloc_free(g)
#define genalloc_shrink(type, g) stralloc_shrink(g)
#define genalloc_catb(type, g, s, n) stralloc_catb((g), (char const *)(s), (n)*sizeof(type))
#define genalloc_copyb(type, g, s, n) stralloc_copyb((g), (char const *)(s), (n)*sizeof(type))
#define genalloc_copy(type, g1, g2) stralloc_copy((g1), g2)
#define genalloc_cat(type, g1, g2) stralloc_cat((g1), g2)
#define genalloc_append(type, g, p) stralloc_catb((g), (char const *)(p), sizeof(type))
#define genalloc_reverse(type, g) stralloc_reverse_blocks((g), sizeof(type))
#define genalloc_insertb(type, g, offset, s, n) stralloc_insertb((g), (offset)*sizeof(type), (char const *)(s), (n)*sizeof(type))
#define genalloc_insert(type, g1, offset, g2) stralloc_insert((g1), (offset)*sizeof(type), (g2))

#define genalloc_deepfree(type, g, f) genalloc_deepfree_size(g, (free_func_ref)(f), sizeof(type))
extern void genalloc_deepfree_size (genalloc *, free_func_ref, size_t) ;

#endif
