/*
 * This file contains the system call macros and syscall 
 * numbers used by the shared library loader.
 */

#define __NR_exit		  1
#define __NR_read		  3
#define __NR_write		  4
#define __NR_open		  5
#define __NR_close		  6
#define __NR_getuid		 24
#define __NR_geteuid		 49
#define __NR_getgid		 47
#define __NR_getegid		 50
#define __NR_readlink		 85
#define __NR_mmap		 90
#define __NR_munmap		 91
#define __NR_stat		106
#define __NR_mprotect		125


/* Here are the macros which define how this platform makes
 * system calls.  This particular variant does _not_ set 
 * errno (note how it is disabled in __syscall_return) since
 * these will get called before the errno symbol is dynamicly 
 * linked. */


#define __syscall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
	/* avoid using res which is declared to be in register d0; \
	   errno might expand to a function call and clobber it.  */ \
		/* int __err = -(res); \
		errno = __err; */ \
		res = -1; \
	} \
	return (type) (res); \
} while (0)

#define _syscall0(type, name)						\
type name(void)								\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name)				\
			: "cc", "%d0");					\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall1(type, name, atype, a)					\
type name(atype a)							\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%2, %%d1\n\t"				\
  			"movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name),				\
			  "g" ((long)a)					\
			: "cc", "%d0", "%d1");				\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall2(type, name, atype, a, btype, b)			\
type name(atype a, btype b)						\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%3, %%d2\n\t"				\
  			"movel	%2, %%d1\n\t"				\
			"movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name),				\
			  "a" ((long)a),				\
			  "g" ((long)b)					\
			: "cc", "%d0", "%d1", "%d2");			\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c)		\
type name(atype a, btype b, ctype c)					\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%4, %%d3\n\t"				\
			"movel	%3, %%d2\n\t"				\
  			"movel	%2, %%d1\n\t"				\
			"movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name),				\
			  "a" ((long)a),				\
			  "a" ((long)b),				\
			  "g" ((long)c)					\
			: "cc", "%d0", "%d1", "%d2", "%d3");		\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall4(type, name, atype, a, btype, b, ctype, c, dtype, d)	\
type name(atype a, btype b, ctype c, dtype d)				\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%5, %%d4\n\t"				\
			"movel	%4, %%d3\n\t"				\
			"movel	%3, %%d2\n\t"				\
  			"movel	%2, %%d1\n\t"				\
			"movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name),				\
			  "a" ((long)a),				\
			  "a" ((long)b),				\
			  "a" ((long)c),				\
			  "g" ((long)d)					\
			: "cc", "%d0", "%d1", "%d2", "%d3",		\
			  "%d4");					\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall5(type, name, atype, a, btype, b, ctype, c, dtype, d, etype, e)\
type name(atype a, btype b, ctype c, dtype d, etype e)			\
{									\
  long __res;								\
  __asm__ __volatile__ ("movel	%6, %%d5\n\t"				\
			"movel	%5, %%d4\n\t"				\
			"movel	%4, %%d3\n\t"				\
			"movel	%3, %%d2\n\t"				\
  			"movel	%2, %%d1\n\t"				\
			"movel	%1, %%d0\n\t"				\
  			"trap	#0\n\t"					\
  			"movel	%%d0, %0"				\
			: "=g" (__res)					\
			: "i" (__NR_##name),				\
			  "a" ((long)a),				\
			  "a" ((long)b),				\
			  "a" ((long)c),				\
			  "a" ((long)d),				\
			  "g" ((long)e)					\
			: "cc", "%d0", "%d1", "%d2", "%d3",		\
			  "%d4", "%d5");				\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    /* errno = -__res; */							\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

