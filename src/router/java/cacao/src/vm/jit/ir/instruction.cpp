/* src/vm/jit/ir/instruction.cpp - IR instruction

   Copyright (C) 2008
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

#include "vm/jit/ir/instruction.hpp"
#include "vm/jit/builtin.hpp"           // for builtintable_entry
#include "vm/descriptor.hpp"            // for typedesc
#include "vm/method.hpp"                // for methodinfo
#include "vm/resolve.hpp"               // for unresolved_method

methoddesc* instruction_call_site(const instruction* iptr)
{
	if (iptr->opc == ICMD_BUILTIN) {
		return iptr->sx.s23.s3.bte->md;
	}
	else if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
		return iptr->sx.s23.s3.um->methodref->parseddesc.md;
	}
	else {
		return iptr->sx.s23.s3.fmiref->p.method->parseddesc;
	}
}

Type instruction_call_site_return_type(const instruction* iptr) {
   return (Type) instruction_call_site(iptr)->returntype.type;
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
 * vim:noexpandtab:sw=4:ts=4:
 */
