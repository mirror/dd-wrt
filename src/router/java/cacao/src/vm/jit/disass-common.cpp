/* src/vm/jit/disass-common.cpp - common functions for GNU binutils disassembler

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
#include <cstdarg>
#include <cstdio>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "vm/jit/disass.hpp"


/* global variables ***********************************************************/

#if defined(WITH_BINUTILS_DISASSEMBLER)
disassemble_info info;
bool disass_initialized = false;
#endif


/* We need this on i386 and x86_64 since we don't know the byte length
   of currently printed instructions.  512 bytes should be enough. */

#if defined(__I386__) || defined(__X86_64__) || defined(__S390__)
char disass_buf[512];
s4   disass_len;
#endif


/* disassemble *****************************************************************

   Outputs a disassembler listing of some machine code on `stdout'.

   start: pointer to first machine instruction
   end:   pointer after last machine instruction

*******************************************************************************/

#if defined(ENABLE_JIT)
void disassemble(u1 *start, u1 *end)
{
	printf("  --- disassembler listing ---\n");

	for (; start < end; )
		start = disassinstr(start);
}
#endif


/* disass_printf ***************************************************************

   Required by binutils disassembler.  This just prints the
   disassembled instructions to stdout.

*******************************************************************************/

void disass_printf(PTR p, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

#if defined(__I386__) || defined(__X86_64__) || defined(__S390__)
	disass_len += vsprintf(disass_buf + disass_len, fmt, ap);
#else
	vprintf(fmt, ap);
	fflush(stdout);
#endif

	va_end(ap);
}


/* buffer_read_memory **********************************************************

   We need to replace the buffer_read_memory from binutils.

*******************************************************************************/

int disass_buffer_read_memory(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info)
{
	MCOPY(myaddr, (void *) (ptrint) memaddr, u1, length);

	return 0;
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
