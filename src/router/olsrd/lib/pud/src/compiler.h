#ifndef _PUD_COMPILER_H_
#define _PUD_COMPILER_H_

/** Compiler hint to expect x */
#ifndef likely
# if defined(__GNUC__)
#  define likely(x)       				__builtin_expect((x),1)
# else /* defined(__GNUC__) */
#  define likely(x)						(x)
# endif /* defined(__GNUC__) */
#endif /* likely */

/** Compiler hint to not expect x */
#ifndef unlikely
# if defined(__GNUC__)
#  define unlikely(x)     				__builtin_expect((x),0)
# else /* defined(__GNUC__) */
#  define unlikely(x)					(x)
# endif /* defined(__GNUC__) */
#endif /* unlikely */

#endif /* _PUD_COMPILER_H_ */
