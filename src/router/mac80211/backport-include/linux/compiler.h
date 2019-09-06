#ifndef __BACKPORT_LINUX_COMPILER_H
#define __BACKPORT_LINUX_COMPILER_H
#include_next <linux/compiler.h>

#ifndef __rcu
#define __rcu
#endif

#ifndef __always_unused
#ifdef __GNUC__
#define __always_unused			__attribute__((unused))
#else
#define __always_unused			/* unimplemented */
#endif
#endif

#ifndef __PASTE
/* Indirect macros required for expanded argument pasting, eg. __LINE__. */
#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)
#endif

/* Not-quite-unique ID. */
#ifndef __UNIQUE_ID
# define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __LINE__)
#endif

#ifndef barrier_data
#ifdef __GNUC__
#define barrier_data(ptr) __asm__ __volatile__("": :"r"(ptr) :"memory")
#else /* __GNUC__ */
# define barrier_data(ptr) barrier()
#endif /* __GNUC__ */
#endif

#endif /* __BACKPORT_LINUX_COMPILER_H */
