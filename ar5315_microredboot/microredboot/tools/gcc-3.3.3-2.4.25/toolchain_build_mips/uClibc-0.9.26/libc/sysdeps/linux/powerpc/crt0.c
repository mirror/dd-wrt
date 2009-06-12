/*
 * uC-libc/sysdeps/linux/powerpc/crt0.c
 * process init code for powerpc
 *
 * Copyright (C) 2001 by Lineo, Inc.
 * Author: David A. Schleef <ds@schleef.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <errno.h>

asm(
	"\t.global _start\n"
	"\t_start:\n"
	//"\tmr 9,1\n"
	//"\trlwinm 1,1,0,0,27\n"
	"\tli 0,0\n"
	"\tstwu	1,-32(1)\n"
	"\tmtlr 0\n"
	//"\tstw 0,0(1)\n"
/* XXX fixme.  We want this to be an internally resolved jump */
//	"\tb _start2@xxx\n"
	);


/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
extern void main(int argc,void *argv,void *envp);
void (*mainp)(int argc,void *argv,void *envp) = main;

void __uClibc_main(int argc,void *argv,void *envp);

void _start2(void)
{
	void **p;
	int argc;

	p=__builtin_frame_address(0)+0x30;

	argc=*(int *)p;

	/* gross hack for dynamic linker */
	if(argc==0){
		p=((void *)p)+0x10;
		argc=*(int *)p;
	}

	__uClibc_main(argc,p+1,p+2+argc);
}
