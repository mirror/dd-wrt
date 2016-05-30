/* src/vm/jit/abi-asm.hpp - common assembler ABI defines

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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


#ifndef ABI_ASM_HPP_
#define ABI_ASM_HPP_ 1

/* ATTENTION: Don't add C stuff in here, as this file is included in
   asmpart.S */

/* some count defines for asmpart.S *******************************************/

#define SAV_CNT    (INT_SAV_CNT + FLT_SAV_CNT)
#define ARG_CNT    (INT_ARG_CNT + FLT_ARG_CNT)
#define TMP_CNT    (INT_TMP_CNT + FLT_TMP_CNT)

#endif // ABI_ASM_HP_

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
