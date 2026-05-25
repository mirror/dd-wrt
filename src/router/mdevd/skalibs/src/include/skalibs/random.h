/* ISC license. */

#ifndef SKALIBS_RANDOM_H
#define SKALIBS_RANDOM_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/functypes.h>
#include <skalibs/stralloc.h>

extern void random_devurandom (char *, size_t) ;

extern void random_buf (char *, size_t) ;
extern void random_buf_early (char *, size_t) ;

extern uint32_t random_uint32_from (uint32_t, randomgen_func_ref) ;
#define random_uint32(n) random_uint32_from((n), &random_buf)
#define random_uint32_early(n) random_uint32_from((n), &random_buf_early)

extern void random_name_from (char *, size_t, randomgen_func_ref) ;
#define random_name(s, n) random_name_from(s, (n), &random_buf)
#define random_name_early(s, n) random_name_from(s, (n), &random_buf_early)

extern void random_unsort_from (char *, size_t, size_t, randomgen_func_ref) ;
#define random_unsort(s, n, c) random_unsort_from(s, n, (c), &random_buf)
#define random_unsort_early(s, n, c) random_unsort_from(s, n, (c), &random_buf_early)

extern int random_sauniquename_from (stralloc *, size_t, randomgen_func_ref) ;
#define random_sauniquename(sa, n) random_sauniquename_from(sa, (n), &random_buf)
#define random_sauniquename_early(sa, n) random_sauniquename_from(sa, (n), &random_buf_early)

#endif
