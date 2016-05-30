/* src/vm/jit/ir/icmd.cpp - Intermediate Commands

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

#include "vm/jit/ir/icmd.hpp"


/* the ICMD table ************************************************************/

#if !defined(NDEBUG)
#define N(name)  name,
#else
#define N(name)
#endif

/* abbreviations for flags */

#define PEI     ICMDTABLE_PEI
#define CALLS   ICMDTABLE_CALLS

/* some machine dependent values */

#if SUPPORT_DIVISION
#define IDIV_CALLS  0
#else
#define IDIV_CALLS  ICMDTABLE_CALLS
#endif

#if (SUPPORT_DIVISION && SUPPORT_LONG_DIV)
#define LDIV_CALLS  0
#else
#define LDIV_CALLS  ICMDTABLE_CALLS
#endif

/* include the actual table */

icmdtable_entry_t icmd_table[256] = {
#include "vm/jit/ir/icmdtable.inc"
};


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
 * vim:noexpandtab:sw=4:ts=4:
 */
