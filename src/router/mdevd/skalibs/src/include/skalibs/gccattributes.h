/* ISC license. */

#ifndef SKALIBS_GCCATTRIBUTES_H
#define SKALIBS_GCCATTRIBUTES_H

#ifdef __GNUC__

#define gccattr_noreturn __attribute__((noreturn))
#define gccattr_returns_twice __attribute__((returns_twice))
#define gccattr_noinline __attribute__((noinline))
#define gccattr_inline __attribute__((always_inline))
#define gccattr_const __attribute__((const))
#define gccattr_unused __attribute__((unused))
#define gccattr_used __attribute__((used))
#define gccattr_weak __attribute__((weak))
#define gccattr_aligned __attribute__((aligned (__BIGGEST_ALIGNMENT__)))
#define gccattr_nonstring __attribute__((nonstring))

# if (__GNUC__ >= 3) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96))
#define gccattr_malloc __attribute__((malloc))
#define gccattr_pure __attribute__((pure))
# else
#define gccattr_malloc
#define gccattr_pure
# endif

# if (__GNUC__ >= 3)
#define gccattr_deprecated __attribute__((deprecated))
# else
#define gccattr_deprecated
# endif

#else

#define gccattr_noreturn
#define gccattr_returns_twice
#define gccattr_noinline
#define gccattr_inline
#define gccattr_const
#define gccattr_unused
#define gccattr_used
#define gccattr_weak
#define gccattr_aligned
#define gccattr_nonstring
#define gccattr_malloc
#define gccattr_pure
#define gccattr_deprecated

#endif

#endif
