/* src/vm/jit/verify/typecheck-fields.inc - type checking for field ICMDs

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


{
	unresolved_field *uf;
	constant_FMIref *fieldref;
#if !defined(TYPECHECK_TYPEINFERER)
	typeinfo_t *instanceti;
	typeinfo_t *valueti;
	resolve_result_t result;
#endif

	TYPECHECK_COUNT(stat_ins_field);

#if !defined(TYPECHECK_TYPEINFERER)
	instanceti = (instance) ? &(instance->typeinfo) : NULL;
	valueti    = (value && value->type == TYPE_ADR) ? &(value->typeinfo) : NULL;
#endif

	/* get the field reference from the instruction */

	if (INSTRUCTION_IS_UNRESOLVED(state->iptr)) {
		uf = state->iptr->sx.s23.s3.uf;
		fieldref = uf->fieldref;
	}
	else {
		uf = NULL;
		fieldref = state->iptr->sx.s23.s3.fmiref;
	}

#if !defined(TYPECHECK_TYPEINFERER)
	/* check the basic value type for PUT instructions */

	if (value && value->type != fieldref->parseddesc.fd->type)
		VERIFY_ERROR("Field type mismatch");

	/* try to resolve the field reference lazily */

	result = resolve_field_lazy(state->m, fieldref);

	if (result == resolveSucceeded) {
		fieldinfo *fi;

		/* perform verification checks now */

		fi  = fieldref->p.field;

		result = resolve_field_verifier_checks(
				state->m, fieldref, fi->clazz, fi,
				instanceti, valueti,
				(instance == NULL),
				(value != NULL));
	}

	if (result == resolveFailed)
		EXCEPTION;

	/* if not resolved, yet, create an unresolved field */

	if (result != resolveSucceeded) {
		if (!uf) {
			uf = resolve_create_unresolved_field(state->m->clazz, 
					state->m, state->iptr);
			if (!uf)
				EXCEPTION;

			state->iptr->sx.s23.s3.uf = uf;
			state->iptr->flags.bits |= INS_FLAG_UNRESOLVED;
		}

		/* record the subtype constraints for this field access */

		if (!resolve_constrain_unresolved_field(
					uf, state->m->clazz, state->m,
					instanceti, valueti))
			EXCEPTION; /* XXX maybe wrap exception? */

		TYPECHECK_COUNTIF(INSTRUCTION_IS_UNRESOLVED(state->iptr),stat_ins_field_unresolved);
		TYPECHECK_COUNTIF(INSTRUCTION_IS_RESOLVED(state->iptr) && 
				!state->iptr->sx.s23.s3.fmiref->p.field->clazz->initialized,
				stat_ins_field_uninitialized);
	}
#endif /* !defined(TYPECHECK_TYPEINFERER) */
		
	/* write the result type */

	if (value == NULL) {
#if defined(TYPECHECK_STACKBASED)
		typedescriptor_t *dv;

		if (IS_2_WORD_TYPE(fieldref->parseddesc.fd->type)) {
			CHECK_STACK_SPACE(2);
			stack += 2;
			dv = &(stack[-1]);
			stack[0].type = TYPE_VOID;
		}
		else {
			CHECK_STACK_SPACE(1);
			stack += 1;
			dv = stack;
		}
#define DST dv
#else
#define DST VAROP(state->iptr->dst)
#endif
		DST->type = fieldref->parseddesc.fd->type;
		if (DST->type == TYPE_ADR) {
			if (!DST->typeinfo.init_from_typedesc(fieldref->parseddesc.fd, NULL))
				EXCEPTION;
		}
#undef DST
	}
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
