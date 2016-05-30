/* src/vm/jit/intrp/disass.c - disassembler wrapper for interpreter

   Copyright (C) 1996-2005, 2006, 2008
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

#include <stdio.h>

#include "vm/types.hpp"

#include "vm/jit/intrp/intrp.h"


/* function disassinstr ********************************************************

	outputs a disassembler listing of one machine code instruction on 'stdout'
	c:   instructions machine code

*******************************************************************************/

u1 *intrp_disassinstr(u1 *code)
{
	FILE *savedout;
	u1   *r;

	savedout = vm_out;
	vm_out = stdout;
	r = (u1 *) vm_disassemble_inst((Inst *) code, vm_prim);
	vm_out = savedout;

	return r;
}


/* function disassemble ********************************************************

	outputs a disassembler listing of some machine code on 'stdout'
	code: pointer to first instruction
	len:  code size (number of instructions * 4)

*******************************************************************************/

void intrp_disassemble(u1 *start, u1 *end)
{
	FILE *savedout;

	printf("  --- disassembler listing ---\n");
	savedout = vm_out;
	vm_out = stdout;
	vm_disassemble((Inst *) start, (Inst *) end, vm_prim);
	vm_out = savedout;
}


void printarg_ui      (u4                 ui      )
{
	fprintf(vm_out, "%ud", ui);
}

void printarg_v       (Cell               v       )
{
	fprintf(vm_out, "%lld", (long long)v);
}

void printarg_Cell    (Cell               x       )
{
	fprintf(vm_out, "%lld", (long long)x);
}


void printarg_b       (s4                 b       )
{
	fprintf(vm_out, "%d", b);
}

void printarg_s       (s4                 s       )
{
	fprintf(vm_out, "%d", s);
}

void printarg_i       (s4                 i       )
{
	fprintf(vm_out, "%d", i);
}

void printarg_l       (s8                 l       )
{
	fprintf(vm_out, "%lld", (long long)l);
}

void printarg_f       (float              f       )
{
	fprintf(vm_out, "%f", (double)f);
}

void printarg_d       (double             d       )
{
	fprintf(vm_out, "%f", d);
}

void printarg_aRef    (java_objectheader *aRef    )
{
	fprintf(vm_out, "obj: %p", (void *)aRef);
}

void printarg_aArray  (java_arrayheader * aArray  )
{
	fprintf(vm_out, "array %p", (void *)aArray);
}

void printarg_aaTarget(Inst **            aaTarget)
{
	if (aaTarget) {
		methodinfo *m=((methodinfo **)aaTarget)[3];
		printarg_am(m);
	} else
		fprintf(vm_out, "NULL");
}

void printarg_aClass  (classinfo *        aClass  )
{
	if (aClass)
		utf_fprint_printable_ascii_classname(vm_out, aClass->name);
	else
		fprintf(vm_out, "NULL");
}

void printarg_acr     (constant_classref *acr     )
{
	fprintf(vm_out, "cr: %p", (void *)acr);
}

void printarg_addr    (u1 *               addr    )
{
	fprintf(vm_out, "%p", (void *)addr);
}

void printarg_af      (functionptr        af      )
{
	fprintf(vm_out, "f: %p", (void *)af);
}

void printarg_afi     (fieldinfo *        afi      )
{
	if (afi) {
		utf_fprint_printable_ascii_classname(vm_out, afi->clazz->name);
		fprintf(vm_out, ".");
		utf_fprint_printable_ascii(vm_out, afi->name);
		utf_fprint_printable_ascii(vm_out, afi->descriptor);
	} else
		fprintf(vm_out, "fi=NULL");
}

void printarg_am      (methodinfo *       am      )
{
	if (am) {
		utf_fprint_printable_ascii_classname(vm_out, am->clazz->name);
		fprintf(vm_out, ".");
		utf_fprint_printable_ascii(vm_out, am->name);
		utf_fprint_printable_ascii(vm_out, am->descriptor);
	} else
		fprintf(vm_out, "m=NULL");
}

void printarg_acell   (Cell *             acell   )
{
	fprintf(vm_out, "%p", (void *)acell);
}

void printarg_ainst   (Inst *             ainst   )
{
	fprintf(vm_out, "%p", (void *)ainst);
}

void printarg_auf     (unresolved_field * auf     )
{
	if (auf) {
		utf_fprint_printable_ascii(vm_out, auf->fieldref->name);
		fprintf(vm_out, " (type ");
		utf_fprint_printable_ascii(vm_out, auf->fieldref->descriptor);
		fprintf(vm_out, ")");
	} else
		fprintf(vm_out, "NULL");
}

void printarg_aum     (unresolved_method *aum     )
{
	if (aum) {
		utf_fprint_printable_ascii_classname(vm_out, METHODREF_CLASSNAME(aum->methodref));
		fprintf(vm_out, ".");
		utf_fprint_printable_ascii(vm_out, aum->methodref->name);
		utf_fprint_printable_ascii(vm_out, aum->methodref->descriptor);
	} else
		fprintf(vm_out, "NULL");
}

void printarg_avftbl  (vftbl_t *          avftbl  )
{
	if (avftbl) {
		fprintf(vm_out, "vftbl: ");
		utf_fprint_printable_ascii_classname(vm_out, avftbl->class->name);
	} else
		fprintf(vm_out, "NULL");
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
