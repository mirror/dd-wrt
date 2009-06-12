/* Machine-dependent definitions for profiling support.  SH version.
 *
 * Copyright (C) 2003 Stefan Allius <allius@atecom.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 */

#define mcount_internal __mcount_internal

#define _MCOUNT_DECL(frompc, selfpc) \
static void __attribute__((unused)) mcount_internal (u_long frompc, u_long selfpc)

/*
 * This mcount implementation expect the 'frompc' return address on
 * the stack and the 'selfpc' return address in register pr.
 *
 * Your compiler should include some stuff like this at each function
 * entry:
 *
 *	mov.l	1f,r1
 *	sts.l	pr,@-r15
 *	mova	2f,r0
 *	jmp	@r1
 *	 lds	r0,pr
 *	.align	2
 * 1:	.long	mcount
 * 2:	lds.l	@r15+,pr
 *
 * or for PIC:
 *
 *	mov.l	3f,r1
 *	mova	3f,r0
 *	add	r1,r0
 *	mov.l	1f,r1
 *	mov.l	@(r0,r1),r1
 *	sts.l	pr,@-r15
 *	mova	2f,r0
 *	jmp	@r1
 *	 lds	r0,pr
 *	.align	2
 * 1:	.long	mcount@GOT
 * 3:	.long	_GLOBAL_OFFSET_TABLE_
 * 2:	lds.l	@r15+,pr
 *
 *
 * This ABI will be supported by GCC version 3.3 or newer!
 */
#define MCOUNT asm(\
	".align 4\n\t" \
	".globl _mcount\n\t" \
	".type _mcount,@function\n" \
        "_mcount:\n\t" \
	"mov.l r4,  @-r15\n\t" \
	"mov.l r5,  @-r15\n\t" \
	"mov.l r6,  @-r15\n\t" \
	"mov.l r7,  @-r15\n\t" \
	"sts.l pr,  @-r15\n\t" \
	"sts   pr, r5\n\t" \
	"bsr __mcount_internal\n\t" \
	" mov.l @(5*4,r15), r4\n\t" \
	"lds.l @r15+, pr\n\t" \
	"mov.l @r15+, r7\n\t" \
	"mov.l @r15+, r6\n\t" \
	"mov.l @r15+, r5\n\t" \
	"rts\n\t" \
	" mov.l @r15+, r4\n\t" \
	".size _mcount,.-_mcount;\n\t" \
	".weak mcount;\n\t" \
	" mcount = _mcount;");

