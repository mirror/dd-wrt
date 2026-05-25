/* ISC license. */

#ifndef SKALIBS_BITARRAY_H
#define SKALIBS_BITARRAY_H

#include <string.h>

#include <skalibs/gccattributes.h>

#define bitarray_div8(n) ((n) ? 1U + (((n) - 1) >> 3) : 0U)

extern void bitarray_clearsetn (unsigned char *, size_t, size_t, int) ;
#define bitarray_clearn(s, start, len) bitarray_clearsetn(s, start, (len), 0)
#define bitarray_setn(s, start, len)   bitarray_clearsetn(s, start, (len), 1)

#define bitarray_peek(s, n) (((s)[(n)>>3] & (1U<<((n)&7))) ? 1 : 0)
#define bitarray_isset(b, n) bitarray_peek(b, n)
#define bitarray_clear(s, n) ((s)[(n)>>3] &= ~(1U << ((n) & 7)))
#define bitarray_set(s, n) ((s)[(n)>>3] |= 1U << ((n) & 7))
#define bitarray_poke(s, n, h) ((h) ? bitarray_set(s, n) : bitarray_clear(s, n))

extern int bitarray_testandpoke (unsigned char *, size_t, int) ;
#define bitarray_testandclear(b, n) bitarray_testandpoke(b, n, 0)
#define bitarray_testandset(b, n)   bitarray_testandpoke(b, n, 1)

extern size_t bitarray_firstclear (unsigned char const *, size_t) gccattr_pure ;
extern size_t bitarray_firstset (unsigned char const *, size_t) gccattr_pure ;
#define bitarray_first(s, n, h) ((h) ? bitarray_firstset(s, n) : bitarray_firstclear(s, n))

extern size_t bitarray_firstclear_skip (unsigned char const *, size_t, size_t) gccattr_pure ;
extern size_t bitarray_firstset_skip (unsigned char const *, size_t, size_t) gccattr_pure ;
#define bitarray_first_skip(s, n, k, h) ((h) ? bitarray_firstset_skip(s, n, k) : bitarray_firstclear_skip(s, n, k))

extern size_t bitarray_countones (unsigned char const *, size_t) gccattr_pure ;

extern void bitarray_not (unsigned char *, size_t, size_t) ;
extern void bitarray_and (unsigned char *, unsigned char const *, unsigned char const *, size_t) ;
extern void bitarray_or (unsigned char *, unsigned char const *, unsigned char const *, size_t) ;
extern void bitarray_xor (unsigned char *, unsigned char const *, unsigned char const *, size_t) ;

#endif
