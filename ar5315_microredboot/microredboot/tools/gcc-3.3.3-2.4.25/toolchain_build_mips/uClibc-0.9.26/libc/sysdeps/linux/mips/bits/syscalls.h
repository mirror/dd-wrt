#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

#ifndef __set_errno
# define __set_errno(val) (*__errno_location ()) = (val)
#endif
#ifndef SYS_ify
# define SYS_ify(syscall_name)  (__NR_##syscall_name)
#endif

#ifndef __ASSEMBLER__

/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */
#define _syscall0(type,name) \
type name(void) \
{ \
long __res, __err; \
__asm__ volatile ("li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name) \
                  : "$2","$7","$8","$9","$10","$11","$12","$13","$14","$15", \
		    "$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

/*
 * DANGER: This macro isn't usable for the pipe(2) call
 * which has a unusual return convention.
 */
#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7" \
		  : "=r" (__res), "=r" (__err) \
		  : "i" (__NR_##name),"r" ((long)(a)) \
		  : "$2","$4","$7","$8","$9","$10","$11","$12","$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)) \
                  : "$2","$4","$5","$7","$8","$9","$10","$11","$12","$13", \
		    "$14","$15", "$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name (atype a, btype b, ctype c) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
                  "move\t$6,%5\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)), \
                                      "r" ((long)(c)) \
                  : "$2","$4","$5","$6","$7","$8","$9","$10","$11","$12", \
		    "$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
                  "move\t$6,%5\n\t" \
                  "move\t$7,%6\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)), \
                                      "r" ((long)(c)), \
                                      "r" ((long)(d)) \
                  : "$2","$4","$5","$6","$7","$8","$9","$10","$11","$12", \
		    "$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name (atype a,btype b,ctype c,dtype d,etype e) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
                  "move\t$6,%5\n\t" \
		  "lw\t$2,%7\n\t" \
                  "move\t$7,%6\n\t" \
		  "subu\t$29,24\n\t" \
		  "sw\t$2,16($29)\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7\n\t" \
		  "addiu\t$29,24" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)), \
                                      "r" ((long)(c)), \
                                      "r" ((long)(d)), \
                                      "m" ((long)(e)) \
                  : "$2","$4","$5","$6","$7","$8","$9","$10","$11","$12", \
                    "$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall6(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e,ftype,f) \
type name (atype a,btype b,ctype c,dtype d,etype e,ftype f) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
                  "move\t$6,%5\n\t" \
		  "lw\t$2,%7\n\t" \
		  "lw\t$3,%8\n\t" \
                  "move\t$7,%6\n\t" \
		  "subu\t$29,24\n\t" \
		  "sw\t$2,16($29)\n\t" \
		  "sw\t$3,20($29)\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7\n\t" \
		  "addiu\t$29,24" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)), \
                                      "r" ((long)(c)), \
                                      "r" ((long)(d)), \
                                      "m" ((long)(e)), \
                                      "m" ((long)(f)) \
                  : "$2","$3","$4","$5","$6","$7","$8","$9","$10","$11", \
                    "$12","$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#define _syscall7(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e,ftype,f,gtype,g) \
type name (atype a,btype b,ctype c,dtype d,etype e,ftype f,gtype g) \
{ \
long __res, __err; \
__asm__ volatile ("move\t$4,%3\n\t" \
                  "move\t$5,%4\n\t" \
                  "move\t$6,%5\n\t" \
		  "lw\t$2,%7\n\t" \
		  "lw\t$3,%8\n\t" \
                  "move\t$7,%6\n\t" \
		  "subu\t$29,32\n\t" \
		  "sw\t$2,16($29)\n\t" \
		  "lw\t$2,%9\n\t" \
		  "sw\t$3,20($29)\n\t" \
		  "sw\t$2,24($29)\n\t" \
		  "li\t$2,%2\n\t" \
		  "syscall\n\t" \
		  "move\t%0, $2\n\t" \
		  "move\t%1, $7\n\t" \
		  "addiu\t$29,32" \
                  : "=r" (__res), "=r" (__err) \
                  : "i" (__NR_##name),"r" ((long)(a)), \
                                      "r" ((long)(b)), \
                                      "r" ((long)(c)), \
                                      "r" ((long)(d)), \
                                      "m" ((long)(e)), \
                                      "m" ((long)(f)), \
                                      "m" ((long)(g)) \
                  : "$2","$3","$4","$5","$6","$7","$8","$9","$10","$11", \
                    "$12","$13","$14","$15","$24"); \
if (__err == 0) \
	return (type) __res; \
__set_errno(__res); \
return (type)-1; \
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
