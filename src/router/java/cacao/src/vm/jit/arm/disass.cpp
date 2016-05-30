/* src/vm/jit/arm/disass.cpp - wrapper functions for GNU binutils disassembler

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

*/


#include "config.h"

#include <dis-asm.h>
#include <stdio.h>

#include "vm/types.hpp"

#include "vm/global.hpp"

#include "vm/jit/disass.hpp"


/* disass_pseudo_instr *********************************************************

   Outputs the disassembler listing of one pseudo instruction on
   'stdout'. Trap instructions fall under this category.

   Returns true if the instruction really was a pseudo instruction

   code: pointer to machine code

*******************************************************************************/

static bool disass_pseudo_instr(u1 *code)
{
	s4 mcode;

	mcode = *((s4 *) code);

	if ((mcode & 0x0ff000f0) == 0x07f000f0) {
		printf("ill\t#%d, #%d (condition:0x%x) (pseudo)", ((mcode >> 8) & 0x0fff), ((mcode >> 0) & 0x0f), ((mcode >> 28) & 0x0f));
		return true;
	}
	else
		return false;
}


/* disassinstr *****************************************************************

   Outputs a disassembler listing of one machine code instruction on
   'stdout'.

   code: pointer to instructions machine code

*******************************************************************************/

u1 *disassinstr(u1 *code)
{
	if (!disass_initialized) {
		INIT_DISASSEMBLE_INFO(info, stdout, disass_printf);

		/* setting the struct members must be done after
		   INIT_DISASSEMBLE_INFO */

		info.read_memory_func = &disass_buffer_read_memory;

		disass_initialized = true;
	}

	printf("0x%08x:   %08x    ", (u4) code, *((s4 *) code));

	if (!disass_pseudo_instr(code))
#if defined(__ARMEL__)
		print_insn_little_arm((bfd_vma) code, &info);
#else
		print_insn_big_arm((bfd_vma) code, &info);
#endif

	printf("\n");

	/* 1 instruction is 4-bytes long */
	return code + 4;
}


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
