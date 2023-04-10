/* Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* vfork() is just a special case of clone().  */
#include <extra/asm.h>
#include <extra/regdef.h>


LOCALSZ= 1
FRAMESZ= (((NARGSAVE+LOCALSZ)*SZREG)+ALSZ)&ALMASK
GPOFF= FRAMESZ-(1*SZREG)
NESTED(vfork,FRAMESZ,sp)
#ifdef __PIC__
	SETUP_GP
#endif
	PTR_SUBU sp, FRAMESZ
	SETUP_GP64 (a5, vfork)
#ifdef __PIC__
	SAVE_GP (GPOFF)
#endif

	PTR_ADDU	sp, FRAMESZ

	li		a0, 0x4112	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	move		a1, sp

	/* Do the system call */
	li		v0, 4120
	syscall

	move	a0, v0
#ifdef __PIC__
	PTR_LA		t9, __syscall_ret
	RESTORE_GP64
	jr		t9
#else
	RESTORE_GP64
	j		__syscall_ret
#endif
