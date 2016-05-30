/* src/vm/jit/verify/typecheck-multianewarray.inc - type checking for MULTIANEWARRAY

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

   Authors: Edwin Steiner


*/


{
	classinfo *arrayclass;
	arraydescriptor *desc;
	s4 i;

	/* check the array lengths on the stack */
	i = state->iptr->s1.argcount;

#if !defined(TYPECHECK_TYPEINFERER)
	if (i < 1)
		VERIFY_ERROR("Illegal dimension argument");

	while (i--) {
		TYPECHECK_INT(state->iptr->sx.s23.s2.args[i]);
	}
#endif /* !defined(TYPECHECK_TYPEINFERER) */

	/* check array descriptor */
	if (INSTRUCTION_IS_RESOLVED(state->iptr)) {
		/* the array class reference has already been resolved */
		arrayclass = state->iptr->sx.s23.s3.c.cls;
		if (!arrayclass)
			VERIFY_ERROR("MULTIANEWARRAY with unlinked class");
		if ((desc = arrayclass->vftbl->arraydesc) == NULL)
			VERIFY_ERROR("MULTIANEWARRAY with non-array class");
		if (desc->dimension < state->iptr->s1.argcount)
			VERIFY_ERROR("MULTIANEWARRAY dimension to high");

		/* set the array type of the result */
		dv->typeinfo.init_class(arrayclass);
	}
	else {
		const char *p;
		constant_classref *cr;

		/* the array class reference is still unresolved */
		/* check that the reference indicates an array class of correct dimension */
		cr = state->iptr->sx.s23.s3.c.ref;
		i = 0;
		p = cr->name.begin();
		while (p[i] == '[')
			i++;
		/* { the dimension of the array class == i } */
#if !defined(TYPECHECK_TYPEINFERER)
		if (i < 1)
			VERIFY_ERROR("MULTIANEWARRAY with non-array class");
		if (i < state->iptr->s1.argcount)
			VERIFY_ERROR("MULTIANEWARRAY dimension to high");
#endif /* !defined(TYPECHECK_TYPEINFERER) */

		/* set the array type of the result */
		if (!dv->typeinfo.init_class(cr))
			return false;
	}

	/* set return type */

	dv->type = TYPE_ADR;
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
 * vim:noexpandtab:sw=4:ts=4:filetype=c:
 */
