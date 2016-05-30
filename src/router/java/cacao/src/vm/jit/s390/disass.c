/* src/vm/jit/x86_64/disass.c - wrapper functions for GNU binutils disassembler

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

   Authors: Andreas  Krall
            Reinhard Grafl

   Changes: Christian Thalinger

*/


#include "config.h"

#include <dis-asm.h>
#include <stdio.h>
#include <stdint.h>

#include "vm/types.hpp"

#include "vm/global.hpp"
#include "vm/jit/disass.hpp"


/* global variables ***********************************************************/

/* name table for 16 integer registers */

char *regs[] = {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15"
};


/* disass_pseudo_instr *********************************************************

   Outputs a disassembler listing of one pseudo instruction instruction on
   `stdout'.
	
   Returns number of bytes consumed or 0 if code does not contain a pseudo 
   instruction.

   code: pointer to machine code

*******************************************************************************/

static s4 disass_pseudo_instr(u1 *code) {
	switch (code[0]) {
		/* Trap */
		case 0x02:
			snprintf(disass_buf, 512, "ill\t0x%02x (pseudo)", (int)code[1]);
			return 2;
		/* Not recognized */
		default:
			return 0;
	}
}

/* disassinstr *****************************************************************

   Outputs a disassembler listing of one machine code instruction on
   `stdout'.

   code: pointer to machine code

*******************************************************************************/

u1 *disassinstr(u1 *code)
{
	s4 seqlen;
	s4 i;

	if (!disass_initialized) {
		INIT_DISASSEMBLE_INFO(info, NULL, disass_printf);

		/* setting the struct members must be done after
		   INIT_DISASSEMBLE_INFO */

		info.mach             = bfd_mach_s390_31;
		info.read_memory_func = &disass_buffer_read_memory;

		disass_initialized = true;
	}

	printf("0x%08x:   ", (s4) code);

	disass_len = 0;

	seqlen = disass_pseudo_instr(code);

	if (seqlen == 0) {
		seqlen = print_insn_s390((bfd_vma)(intptr_t)code, &info);
	}

	for (i = 0; i < seqlen; i++, code++) {
		printf("%02x ", *code);
	}
	
	for (; i < 10; i++) {
		printf("   ");
	}

	printf("   %s\n", disass_buf);

	return code;
}


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
