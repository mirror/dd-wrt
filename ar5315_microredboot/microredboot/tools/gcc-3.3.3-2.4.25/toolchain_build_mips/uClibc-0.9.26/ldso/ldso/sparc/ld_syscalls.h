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
#define __NR_getgid		 47
#define __NR_geteuid		 49
#define __NR_getegid		 50
#define __NR_readlink            58
#define __NR_mmap		 71
#define __NR_munmap		 73
#define __NR_stat		 38
#define __NR_mprotect		 74

/* Here are the macros which define how this platform makes
 * system calls.  This particular variant does _not_ set 
 * errno (note how it is disabled in __syscall_return) since
 * these will get called before the errno symbol is dynamicly 
 * linked. */

#define _syscall0(type,name) \
type name(void) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res)\
		      : "r" (__g1) \
		      : "o0", "cc"); \
if (__res < -255 || __res >= 0) \
    return (type) __res; \
/*errno = -__res; */\
return -1; \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
register long __o0 __asm__ ("o0") = (long)(arg1); \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res), "=&r" (__o0) \
		      : "1" (__o0), "r" (__g1) \
		      : "cc"); \
if (__res < -255 || __res >= 0) \
	return (type) __res; \
/*errno = -__res;*/ \
return -1; \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
register long __o0 __asm__ ("o0") = (long)(arg1); \
register long __o1 __asm__ ("o1") = (long)(arg2); \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res), "=&r" (__o0) \
		      : "1" (__o0), "r" (__o1), "r" (__g1) \
		      : "cc"); \
if (__res < -255 || __res >= 0) \
	return (type) __res; \
/*errno = -__res;*/ \
return -1; \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
register long __o0 __asm__ ("o0") = (long)(arg1); \
register long __o1 __asm__ ("o1") = (long)(arg2); \
register long __o2 __asm__ ("o2") = (long)(arg3); \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res), "=&r" (__o0) \
		      : "1" (__o0), "r" (__o1), "r" (__o2), "r" (__g1) \
		      : "cc"); \
if (__res < -255 || __res>=0) \
	return (type) __res; \
/*errno = -__res;*/ \
return -1; \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
register long __o0 __asm__ ("o0") = (long)(arg1); \
register long __o1 __asm__ ("o1") = (long)(arg2); \
register long __o2 __asm__ ("o2") = (long)(arg3); \
register long __o3 __asm__ ("o3") = (long)(arg4); \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res), "=&r" (__o0) \
		      : "1" (__o0), "r" (__o1), "r" (__o2), "r" (__o3), "r" (__g1) \
		      : "cc"); \
if (__res < -255 || __res>=0) \
	return (type) __res; \
/*errno = -__res;*/ \
return -1; \
} 

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
long __res; \
register long __g1 __asm__ ("g1") = __NR_##name; \
register long __o0 __asm__ ("o0") = (long)(arg1); \
register long __o1 __asm__ ("o1") = (long)(arg2); \
register long __o2 __asm__ ("o2") = (long)(arg3); \
register long __o3 __asm__ ("o3") = (long)(arg4); \
register long __o4 __asm__ ("o4") = (long)(arg5); \
__asm__ __volatile__ ("t 0x10\n\t" \
		      "bcc 1f\n\t" \
		      "mov %%o0, %0\n\t" \
		      "sub %%g0, %%o0, %0\n\t" \
		      "1:\n\t" \
		      : "=r" (__res), "=&r" (__o0) \
		      : "1" (__o0), "r" (__o1), "r" (__o2), "r" (__o3), "r" (__o4), "r" (__g1) \
		      : "cc"); \
if (__res < -255 || __res>=0) \
	return (type) __res; \
/*errno = -__res; */\
return -1; \
}
