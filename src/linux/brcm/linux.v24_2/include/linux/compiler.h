#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

/* Somewhere in the middle of the GCC 2.96 development cycle, we implemented
   a mechanism by which the user can annotate likely branch directions and
   expect the blocks to be reordered appropriately.  Define __builtin_expect
   to nothing for earlier compilers.  */

#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

#if __GNUC__ > 3
#define __attribute_used__	__attribute__((__used__))
#elif __GNUC__ == 3
#if  __GNUC_MINOR__ >= 3
# define __attribute_used__	__attribute__((__used__))
#else
# define __attribute_used__	__attribute__((__unused__))
#endif /* __GNUC_MINOR__ >= 3 */
#elif __GNUC__ == 2
#define __attribute_used__	__attribute__((__unused__))
#else
#define __attribute_used__	/* not implemented */
#endif /* __GNUC__ */

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define __attribute_const__	__attribute__((__const__))
#else
#define __attribute_const__	/* unimplemented */
#endif

#ifdef __GNUC_STDC_INLINE__
# define __gnu_inline	__attribute__((gnu_inline))
#else
# define __gnu_inline
#endif

#if defined(CC_USING_HOTPATCH) && !defined(__CHECKER__)
#define notrace __attribute__((hotpatch(0,0)))
#else
#define notrace __attribute__((no_instrument_function))
#endif



#if !defined(CONFIG_OPTIMIZE_INLINING) || (__GNUC__ < 4)
#define inline \
	inline __attribute__((always_inline, unused)) notrace __gnu_inline
#else
/* A lot of inline functions can cause havoc with function tracing */
#define inline inline		__attribute__((unused)) notrace __gnu_inline
#endif

#define __inline__ inline
#define __inline inline


#ifdef __KERNEL__
/*#if __GNUC__ > 5 || __GNUC__ == 5 && __GNUC_MINOR__ >= 2
#error "GCC >= 4.2 miscompiles kernel 2.4, do not use it!"
#error "While the resulting kernel may boot, you will encounter random bugs"
#error "at runtime. Only versions 2.95.3 to 4.1 are known to work reliably."
#error "To build with another version, for instance 3.3, please do"
#error "   make bzImage CC=gcc-3.3 "
#endif

*/
/*#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 2
#error "GCC >= 4.2 miscompiles kernel 2.4, do not use it!"
#error "While the resulting kernel may boot, you will encounter random bugs"
#error "at runtime. Only versions 2.95.3 to 4.1 are known to work reliably."
#error "To build with another version, for instance 3.3, please do"
#error "   make bzImage CC=gcc-3.3 "
#endif
*/
#endif

/* no checker support, so we unconditionally define this as (null) */
#define __user
#define __iomem

#endif /* __LINUX_COMPILER_H */
