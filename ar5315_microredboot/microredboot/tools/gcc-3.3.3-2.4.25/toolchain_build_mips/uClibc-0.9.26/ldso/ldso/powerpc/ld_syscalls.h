/*
 * This file contains the system call macros and syscall 
 * numbers used by the shared library loader.
 */

#define __NR_exit		  1
#define __NR_read		  3
#define __NR_write		  4
#define __NR_open		  5
#define __NR_close		  6
#define __NR_getpid		 20
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

#undef __syscall_return
#define __syscall_return(type) \
	return (__sc_err & 0x10000000 ? /*errno = __sc_ret,*/ __sc_ret = -1 : 0), \
	       (type) __sc_ret

#undef __syscall_clobbers
#define __syscall_clobbers \
	"r9", "r10", "r11", "r12"
	//"r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"

#undef _syscall0
#define _syscall0(type,name)						\
type name(void)								\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
									\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0)		\
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12" ); \
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}

#undef _syscall1
#define _syscall1(type,name,type1,arg1)					\
type name(type1 arg1)							\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0)		\
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12" ); \
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}

#undef _syscall2
#define _syscall2(type,name,type1,arg1,type2,arg2)			\
type name(type1 arg1, type2 arg2)					\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
		register unsigned long __sc_4 __asm__ ("r4");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_4 = (unsigned long) (arg2);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0),		\
			  "r"   (__sc_4)				\
			: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12" ); \
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}

#undef _syscall3
#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3)		\
type name(type1 arg1, type2 arg2, type3 arg3)				\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
		register unsigned long __sc_4 __asm__ ("r4");		\
		register unsigned long __sc_5 __asm__ ("r5");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_4 = (unsigned long) (arg2);			\
		__sc_5 = (unsigned long) (arg3);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0),		\
			  "r"   (__sc_4),				\
			  "r"   (__sc_5)				\
			: "r6", "r7", "r8", "r9", "r10", "r11", "r12" ); \
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}

#undef _syscall4
#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4)		\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
		register unsigned long __sc_4 __asm__ ("r4");		\
		register unsigned long __sc_5 __asm__ ("r5");		\
		register unsigned long __sc_6 __asm__ ("r6");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_4 = (unsigned long) (arg2);			\
		__sc_5 = (unsigned long) (arg3);			\
		__sc_6 = (unsigned long) (arg4);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0),		\
			  "r"   (__sc_4),				\
			  "r"   (__sc_5),				\
			  "r"   (__sc_6)				\
			: "r7", "r8", "r9", "r10", "r11", "r12" );	\
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}

#undef _syscall5
#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)	\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
		register unsigned long __sc_4 __asm__ ("r4");		\
		register unsigned long __sc_5 __asm__ ("r5");		\
		register unsigned long __sc_6 __asm__ ("r6");		\
		register unsigned long __sc_7 __asm__ ("r7");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_4 = (unsigned long) (arg2);			\
		__sc_5 = (unsigned long) (arg3);			\
		__sc_6 = (unsigned long) (arg4);			\
		__sc_7 = (unsigned long) (arg5);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0),		\
			  "r"   (__sc_4),				\
			  "r"   (__sc_5),				\
			  "r"   (__sc_6),				\
			  "r"   (__sc_7)				\
			: "r8", "r9", "r10", "r11", "r12" );		\
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}


#undef _syscall6
#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6)	\
{									\
	unsigned long __sc_ret, __sc_err;				\
	{								\
		register unsigned long __sc_0 __asm__ ("r0");		\
		register unsigned long __sc_3 __asm__ ("r3");		\
		register unsigned long __sc_4 __asm__ ("r4");		\
		register unsigned long __sc_5 __asm__ ("r5");		\
		register unsigned long __sc_6 __asm__ ("r6");		\
		register unsigned long __sc_7 __asm__ ("r7");		\
		register unsigned long __sc_8 __asm__ ("r8");		\
									\
		__sc_3 = (unsigned long) (arg1);			\
		__sc_4 = (unsigned long) (arg2);			\
		__sc_5 = (unsigned long) (arg3);			\
		__sc_6 = (unsigned long) (arg4);			\
		__sc_7 = (unsigned long) (arg5);			\
		__sc_8 = (unsigned long) (arg6);			\
		__sc_0 = __NR_##name;					\
		__asm__ __volatile__					\
			("sc           \n\t"				\
			 "mfcr %1      "				\
			: "=&r" (__sc_3), "=&r" (__sc_0)		\
			: "0"   (__sc_3), "1"   (__sc_0),		\
			  "r"   (__sc_4),				\
			  "r"   (__sc_5),				\
			  "r"   (__sc_6),				\
			  "r"   (__sc_7),				\
			  "r"   (__sc_8)				\
			: "r9", "r10", "r11", "r12" );			\
		__sc_ret = __sc_3;					\
		__sc_err = __sc_0;					\
	}								\
	__syscall_return (type);					\
}


