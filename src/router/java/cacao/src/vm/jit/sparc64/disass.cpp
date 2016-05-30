/* src/vm/jit/i386/disass.cpp - wrapper functions for GNU binutils disassembler

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

   Authors: Andreas  Krall
            Reinhard Grafl
            Christian Thalinger
            Alexander Jordan

   Changes:      

*/


#include "config.h"

#include <cassert>
#include <cstdarg>
#include <dis-asm.h>

#include "vm/types.hpp"

#include "vm/global.hpp"
#include "vm/jit/disass.hpp"

char *regs[] = {
	"g0",
	"g1",
	"g2",
	"g3",
	"g4",
	"g5",
	"g6",
	"g7",
	"o0",
	"o1",
	"o2",
	"o3",
	"o4",
	"o5",
	"o6",
	"o7",
	"l0",
	"l1",
	"l2",
	"l3",
	"l4",
	"l5",
	"l6",
	"l7",
	"l0",
	"i1",
	"i2",
	"i3",
	"i4",
	"i5",
	"i6",
	"i7"
};

/* disassinstr *****************************************************************

   Outputs a disassembler listing of one machine code instruction on
   'stdout'.

   code: instructions machine code

*******************************************************************************/

u1 *disassinstr(u1 *code)
{
	if (!disass_initialized) {
		INIT_DISASSEMBLE_INFO(info, NULL, disass_printf);

		/* setting the struct members must be done after
		   INIT_DISASSEMBLE_INFO */

		info.mach             = bfd_mach_sparc_v9;
		info.endian           = BFD_ENDIAN_BIG;
		info.read_memory_func = &disass_buffer_read_memory;

		disass_initialized = 1;
	}

	printf("0x%016lx:   %08x    ", (s8) code, *((u4 *) code));

	print_insn_sparc((bfd_vma) code, &info);

	printf("\n");

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
