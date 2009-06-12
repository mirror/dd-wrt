/* Copyright (C) 2001 Hewlett-Packard

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

 Derived in part from the Linux-8086 C library, the GNU C Library, and several
 other sundry sources.  Files within this library are copyright by their
 respective copyright holders.
*/

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#ifdef HIOS

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5, type6,arg6) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
register long __sc3 __asm__ ("r3") = __NR_##name; \
register long __sc4 __asm__ ("r4") = (long) arg1; \
register long __sc5 __asm__ ("r5") = (long) arg2; \
register long __sc6 __asm__ ("r6") = (long) arg3; \
register long __sc7 __asm__ ("r7") = (long) arg4; \
register long __sc0 __asm__ ("r0") = (long) arg5; \
register long __sc1 __asm__ ("r1") = (long) arg6; \
__asm__ __volatile__ ("trapa	#0x2E" \
	: "=z" (__sc0) \
	: "0" (__sc0), "r" (__sc4), "r" (__sc5), "r" (__sc6), "r" (__sc7),  \
	  "r" (__sc3), "r" (__sc1) \
	: "memory" ); \
__syscall_return(type,__sc0); \
}

#else // HIOS

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5, type6,arg6) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
register long __sc3 __asm__ ("r3") = __NR_##name; \
register long __sc4 __asm__ ("r4") = (long) arg1; \
register long __sc5 __asm__ ("r5") = (long) arg2; \
register long __sc6 __asm__ ("r6") = (long) arg3; \
register long __sc7 __asm__ ("r7") = (long) arg4; \
register long __sc0 __asm__ ("r0") = (long) arg5; \
register long __sc1 __asm__ ("r1") = (long) arg6; \
__asm__ __volatile__ ("trapa	#0x15" \
	: "=z" (__sc0) \
	: "0" (__sc0), "r" (__sc4), "r" (__sc5), "r" (__sc6), "r" (__sc7),  \
	  "r" (__sc3), "r" (__sc1) \
	: "memory" ); \
__syscall_return(type,__sc0); \
}

#endif // HIOS

_syscall6(__ptr_t, mmap, __ptr_t, addr, size_t, len, int, prot, int, flags, int, fd, __off_t, offset);


