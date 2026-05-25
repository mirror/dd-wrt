/* ISC license. */

#ifndef SKALIBS_STRALLOC_H
#define SKALIBS_STRALLOC_H

#include <string.h>
#include <sys/uio.h>

typedef struct stralloc_s stralloc, *stralloc_ref ;
struct stralloc_s
{
  char *s ;
  size_t len ;
  size_t a ;
} ;

#define STRALLOC_ZERO { 0, 0, 0 }
extern stralloc const stralloc_zero ;

extern int stralloc_ready_tuned (stralloc *, size_t, size_t, size_t, size_t) ;
extern int stralloc_readyplus_tuned (stralloc *, size_t, size_t, size_t, size_t) ;
#define stralloc_ready(sa, n) stralloc_ready_tuned(sa, (n), 8, 1, 8)
#define stralloc_readyplus(sa, n) stralloc_readyplus_tuned(sa, (n), 8, 1, 8)
extern void stralloc_free (stralloc *) ;
extern int stralloc_shrink (stralloc *) ;
extern int stralloc_copyb (stralloc *, char const *, size_t) ;
extern int stralloc_catb (stralloc *, char const *, size_t) ;
extern int stralloc_catv (stralloc *, struct iovec const *, unsigned int) ;
#define stralloc_copys(sa, s) stralloc_copyb(sa, (s), strlen(s))
#define stralloc_cats(sa, s) stralloc_catb(sa, (s), strlen(s))
#define stralloc_copy(sa1, sa2) stralloc_copyb(sa1, (sa2)->s, (sa2)->len)
#define stralloc_cat(sa1, sa2) stralloc_catb(sa1, (sa2)->s, (sa2)->len)
extern int stralloc_append (stralloc *, char) ;
extern void stralloc_reverse (stralloc *) ;
extern void stralloc_reverse_blocks (stralloc *, size_t) ;
#define stralloc_0(sa) stralloc_catb(sa, "", 1)
extern int stralloc_insertb (stralloc *, size_t, char const *, size_t) ;
#define stralloc_inserts(sa, offset, s) stralloc_insertb(sa, offset, (s), strlen(s))
#define stralloc_insert(sa1, offset, sa2) stralloc_insertb(sa1, offset, (sa2)->s, (sa2)->len)

#endif
