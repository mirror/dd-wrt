/* src/vm/jit/x86_64/md-emit.h - machine dependent emit function prototypes

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


#ifndef _MD_EMIT_H
#define _MD_EMIT_H

#include "vm/types.hpp"


/* macros to create code ******************************************************/

s4 emit_load_s1_but(jitdata *jd, instruction *iptr, s4 tempreg, s4 notreg);
s4 emit_load_s2_but(jitdata *jd, instruction *iptr, s4 tempreg, s4 notreg);

/* If the destination operand is in a register, different than
 * dtmpreg, a register copy is emitted.
 */
void emit_copy_dst(jitdata *jd, instruction *iptr, s4 dtmpreg);

/* Emits code to recalculate the PV (procedure vector) after a subroutine call. */
void emit_restore_pv(codegendata *cd);

#endif /* _MD_EMIT_H */


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
