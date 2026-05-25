/* ISC license. */

#ifndef SKALIBS_BYTESTR_H
#define SKALIBS_BYTESTR_H


/* for Alphas and other archs where char != 8bit */
#define T8(x)   ((x) & 0xffU)

#include <string.h>
#include <strings.h>

#include <skalibs/gccattributes.h>

#define byte_copy(to, n, from) memmove(to, (from), n)
#define byte_copyr(to, n, from) memmove(to, (from), n)
#define byte_diff(a, n, b) memcmp(a, (b), n)
#define byte_zero(p, n) memset(p, 0, n)
#define str_len strlen
#define str_nlen strnlen

#define str_diff strcmp
#define str_diffn strncmp
#define str_copy(to, from) strlen(strcpy(to, from))
#define case_diffs strcasecmp
#define case_diffn strncasecmp

extern size_t byte_chr (char const *, size_t, int) gccattr_pure ;
extern size_t byte_rchr (char const *, size_t, int) gccattr_pure ;
extern size_t byte_in (char const *, size_t, char const *, size_t) gccattr_pure ;
#define byte_equal(s, n, t) (!memcmp(s, (t), n))
extern size_t byte_count (char const *, size_t, char) gccattr_pure ;
extern size_t byte_search (char const *, size_t, char const *, size_t) ;
extern void byte_zzero (char *, size_t) ;

#define str_diffb(a, n, b) strncmp(a, (b), n)
extern size_t str_chr (char const *, int) gccattr_pure ;
extern size_t str_rchr (char const *, int) gccattr_pure ;
extern int str_start (char const *, char const *) gccattr_pure ;
#define str_equal(s, t) (!strcmp(s, t))
extern size_t str_strn (char const *, size_t, char const *, size_t) gccattr_pure ;

extern void case_lowers (char *) ;
extern void case_lowerb (char *, size_t) ;
extern void case_uppers (char *) ;
extern void case_upperb (char *, size_t) ;
#define case_diffb(a, n, b) case_diffn(a, (b), n)
#define case_equals(a, b) (!strcasecmp(a, b))
#define case_equalb(a, n, b) (!strncasecmp(a, (b), n))
#define case_starts(s, t) case_startb(s, strlen(s), t)
extern int case_startb (char const *, size_t, char const *) gccattr_pure ;

extern int str_cmp (void const *, void const *) gccattr_pure ;
extern int str_bcmp (void const *, void const *) gccattr_pure ;
extern int str_casecmp (void const *, void const *) gccattr_pure ;
extern int str_bcasecmp (void const *, void const *) gccattr_pure ;
extern int stringkey_cmp (void const *, void const *) gccattr_pure ;
extern int stringkey_bcmp (void const *, void const *) gccattr_pure ;
extern int stringkey_casecmp (void const *, void const *) gccattr_pure ;
extern int stringkey_bcasecmp (void const *, void const *) gccattr_pure ;

#endif
