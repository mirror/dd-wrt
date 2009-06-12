/* Code to enable profiling at program startup.
   Copyright (C) 1995,1996,1997,2000,2001,2002 Free Software Foundation, Inc.
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

#include <features.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/gmon.h>

#ifdef __UCLIBC_PROFILING__

/* Beginning and end of our code segment. We cannot declare them
   as the external functions since we want the addresses of those
   labels. Taking the address of a function may have different
   meanings on different platforms. */

extern void _start;
extern void etext;


void __gmon_start__ (void)
{
#ifdef __UCLIBC_CTOR_DTOR__
    /* Protect from being called more than once.  Since crti.o is linked
       into every shared library, each of their init functions will call us.  */
    static int called;

    if (called)
	return;

    called = 1;
#endif

    /* Start keeping profiling records.  */
    monstartup ((u_long) &_start, (u_long) &etext);

    /* Call _mcleanup before exiting; it will write out gmon.out from the
       collected data.  */
    atexit (&_mcleanup);
}
#endif

