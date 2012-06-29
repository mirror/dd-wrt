#ifndef _PUD_COMPILER_H_
#define _PUD_COMPILER_H_

/** Compiler hint to expect x */
#ifndef likely
# if defined(__GNUC__)
#  define likely(x)       				__builtin_expect((x),1)
# else
#  define likely(x)						(x)
# endif
#endif

/** Compiler hint to not expect x */
#ifndef unlikely
# if defined(__GNUC__)
#  define unlikely(x)     				__builtin_expect((x),0)
# else
#  define unlikely(x)					(x)
# endif
#endif

#endif /* _PUD_COMPILER_H_ */
