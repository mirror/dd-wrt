/* brk system call for Linux/ARM.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

/* This must be initialized data because commons can't have aliases.  */
void *___brk_addr = 0;

int brk (void *addr)
{
    void *newbrk;

    asm ("mov a1, %1\n"	/* save the argment in r0 */
	    "swi %2\n"	/* do the system call */
	    "mov %0, a1;"	/* keep the return value */
	    : "=r"(newbrk) 
	    : "r"(addr), "i" (__NR_brk)
	    : "a1");

    ___brk_addr = newbrk;

    if (newbrk < addr)
    {
	__set_errno (ENOMEM);
	return -1;
    }

    return 0;
}
