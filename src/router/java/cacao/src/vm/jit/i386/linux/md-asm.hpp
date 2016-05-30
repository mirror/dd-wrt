/* src/vm/jit/i386/md-asm.h - assembler defines for i386 ABI

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Contact: cacao@cacaojvm.org

   Authors: Christian Thalinger

   Changes:

*/


#ifndef _MD_ASM_H
#define _MD_ASM_H

/* register defines ***********************************************************/

#define v0       %eax
#define itmp1    v0

#define itmp2    %ecx
#define itmp3    %edx

#define t0       %ebx

#define sp       %esp
#define s0       %ebp
#define s1       %esi
#define s2       %edi

#define bp       s0

#define itmp1b   %al

#define xptr     itmp1
#define xpc      itmp2
#define mptr     itmp2


/* save and restore macros ****************************************************/

#define SAVE_ARGUMENT_REGISTERS(off) \
    /* no argument registers */

#define SAVE_TEMPORARY_REGISTERS(off) \
	mov     t0,(0+(off))*4(sp) ;


#define RESTORE_ARGUMENT_REGISTERS(off) \
    /* no argument registers */

#define RESTORE_TEMPORARY_REGISTERS(off) \
	mov     (0+(off))*4(sp),t0 ;

#endif /* _MD_ASM_H */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
