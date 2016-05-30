/* src/vm/jit/verify/typecheck-builtins.inc - type checking for ICMD_BUILTIN

   Copyright (C) 1996-2014
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


#define ISBUILTIN(v)   (bte->fp == (functionptr) (v))

{
	builtintable_entry *bte;

	bte = state->iptr->sx.s23.s3.bte;

	/* XXX this is an ugly if-chain but twisti did not want a function */
	/* pointer in builtintable_entry for this, so here you go.. ;)     */

	if (ISBUILTIN(BUILTIN_new)) {
		dv->type = TYPE_ADR;
#if defined(TYPECHECK_TYPEINFERER)
		assert(state->iptr[-1].opc == ICMD_ACONST);
		dv->typeinfo.init_class(state->iptr[-1].sx.val.c);
#else
		if (state->iptr[-1].opc != ICMD_ACONST)
			TYPECHECK_VERIFYERROR_bool("illegal instruction: builtin_new without class");
		dv->typeinfo.init_newobject(state->iptr);
#endif
	}
	else if (ISBUILTIN(BUILTIN_newarray_boolean)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_BOOLEAN);
	}
	else if (ISBUILTIN(BUILTIN_newarray_char)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_CHAR);
	}
	else if (ISBUILTIN(BUILTIN_newarray_float)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_FLOAT);
	}
	else if (ISBUILTIN(BUILTIN_newarray_double)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_DOUBLE);
	}
	else if (ISBUILTIN(BUILTIN_newarray_byte)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_BYTE);
	}
	else if (ISBUILTIN(BUILTIN_newarray_short)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_SHORT);
	}
	else if (ISBUILTIN(BUILTIN_newarray_int)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_INT);
	}
	else if (ISBUILTIN(BUILTIN_newarray_long)) {
		TYPECHECK_INT(OP1);
		dv->type = TYPE_ADR;
		dv->typeinfo.init_primitive_array(ARRAYTYPE_LONG);
	}
	else if (ISBUILTIN(BUILTIN_newarray))
	{
		TYPECHECK_INT(OP1);
		if (state->iptr[-1].opc != ICMD_ACONST)
			TYPECHECK_VERIFYERROR_bool("illegal instruction: builtin_newarray without class");
		/* XXX check that it is an array class(ref) */
		dv->type = TYPE_ADR;
		dv->typeinfo.init_class(state->iptr[-1].sx.val.c);
	}
	else if (ISBUILTIN(BUILTIN_arrayinstanceof))
	{
		TYPECHECK_ADR(OP1);
		if (state->iptr[-1].opc != ICMD_ACONST)
			TYPECHECK_VERIFYERROR_bool("illegal instruction: builtin_arrayinstanceof without class");
		dv->type = TYPE_INT;
		/* XXX check that it is an array class(ref) */
	}
	else {
		methoddesc *md;
		Type        rtype;
#if !defined(TYPECHECK_TYPEINFERER)
		s4 i;
#endif
#if defined(TYPECHECK_STACKBASED)
		typedescriptor_t *av;
#endif

		/* verify a generic builtin call */

		TYPECHECK_COUNT(stat_ins_builtin_gen);

		md = bte->md;
#if !defined(TYPECHECK_TYPEINFERER)
		i = md->paramcount;

		/* check the types of the arguments on the stack */

#if defined(TYPECHECK_STACKBASED)
		av = stack - (md->paramslots - 1);
#endif

		for (i--; i >= 0; i--) {
#if defined(TYPECHECK_VARIABLESBASED)
			varinfo *av = VAR(state->iptr->sx.s23.s2.args[i]);
#endif

			if (av->type != md->paramtypes[i].type) {
				TYPECHECK_VERIFYERROR_bool("parameter type mismatch for builtin method");
			}

#ifdef TYPECHECK_DEBUG
			/* generic builtins may only take primitive types and java.lang.Object references */
			if (av->type == TYPE_ADR && md->paramtypes[i].classref->name != utf8::java_lang_Object) {
				exceptions_throw_internalerror("generic builtin method with non-generic reference parameter");
				return false;
			}
#endif

#if defined(TYPECHECK_STACKBASED)
			av += (IS_2_WORD_TYPE(av->type)) ? 2 : 1;
#endif
		}
#endif /* !defined(TYPECHECK_TYPEINFERER) */

		/* set the return type */

		rtype = md->returntype.type;
		if (rtype != TYPE_VOID) {
			dv->type = rtype;
			if (!dv->typeinfo.init_from_typedesc(&md->returntype, NULL))
				return false;
		}
	}
}

#undef ISBUILTIN

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
