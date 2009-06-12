/* Define the __set_errno macro as nothing so that INLINE_SYSCALL
 * won't set errno, which is important since we make system calls
 * before the errno symbol is dynamicly linked. */

#define __set_errno(X) {(void)(X);}

/* Prepare for the case that `__builtin_expect' is not available.  */
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
# define likely(x)	__builtin_expect((!!(x)),1)
#endif
#ifndef unlikely
# define unlikely(x)	__builtin_expect((!!(x)),0)
#endif

#include "sys/syscall.h"

