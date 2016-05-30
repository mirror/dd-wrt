/* src/vm/jit/verify/typecheck-invoke.inc - type checking for invocations

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


{
	unresolved_method *um;      /* struct describing the called method */
	constant_FMIref *mref;           /* reference to the called method */
	methoddesc *md;                 /* descriptor of the called method */
	Utf8String mname;                                   /* method name */
    Type rtype;                        /* return type of called method */
#if !defined(TYPECHECK_TYPEINFERER)
	methodinfo *mi;                        /* resolved method (if any) */
	Utf8String mclassname;               /* name of the method's class */
	bool specialmethod;            /* true if a <...> method is called */
	int opcode;                                   /* invocation opcode */
	bool callinginit;                      /* true if <init> is called */
	instruction *ins;
	classref_or_classinfo initclass;
	initclass.any = NULL;                     // silence compiler warning
	typedesc *td;
#if defined(TYPECHECK_VARIABLESBASED)
	s4 argindex;                            /* argument variable index */
	varinfo *av;                                  /* argument variable */
#else
	typedescriptor_t *av;                       /* argument stack slot */
#endif
	int i;                                                  /* counter */
	resolve_result_t result;
	bool invokestatic;
	bool invokespecial;
#endif /* !defined(TYPECHECK_TYPEINFERER) */

	/* get the FMIref and the unresolved_method struct (if any) */
	/* from the instruction                                     */

	if (INSTRUCTION_IS_UNRESOLVED(state->iptr)) {
		/* unresolved method */
		um = state->iptr->sx.s23.s3.um;
		mref = um->methodref;
	}
	else {
		/* resolved method */
		um = NULL;
		mref = state->iptr->sx.s23.s3.fmiref;
	}

	/* get method descriptor and name */

	md = mref->parseddesc.md;
	mname = mref->name;

#if !defined(TYPECHECK_TYPEINFERER)
	/* get method info (if resolved) and classname */

	if (mref->is_resolved()) {
		mi = mref->p.method;
		mclassname = mi->clazz->name;
	}
	else {
		mi = NULL;
		mclassname = mref->p.classref->name;
	}

	/* gather some info about this instruction */

	opcode = state->iptr->opc;
	invokestatic = (opcode == ICMD_INVOKESTATIC);
	invokespecial = (opcode == ICMD_INVOKESPECIAL);
	specialmethod = (mname[0] == '<');

	/* prevent compiler warnings */

	ins = NULL;

	/* check whether we are calling <init> */
	
	callinginit = (invokespecial && mname == utf8::init);
	if (specialmethod && !callinginit)
		TYPECHECK_VERIFYERROR_bool("Invalid invocation of special method");

	/* allocate parameters if necessary */
	
	md->params_from_paramtypes(invokestatic ? ACC_STATIC : ACC_NONE);

	/* check parameter types */

#if defined(TYPECHECK_STACKBASED)
	av = stack - (md->paramslots - 1);
	for (i=0; i<md->paramcount; ++i) {
#else
	i = md->paramcount; /* number of parameters including 'this'*/
	while (--i >= 0) {
		argindex = state->iptr->sx.s23.s2.args[i];
		av = VAR(argindex);
#endif
		OLD_LOG1("\t\tparam %d",i);
		td = md->paramtypes + i;

		if (av->type != td->type)
			TYPECHECK_VERIFYERROR_bool("Parameter type mismatch in method invocation");

		if (av->type == TYPE_ADR) {
			OLD_LOGSTR("\t\t"); OLD_LOGINFO(&(av->typeinfo));
			if (i==0 && callinginit)
			{
				/* first argument to <init> method */
				if (!av->typeinfo.is_newobject())
					TYPECHECK_VERIFYERROR_bool("Calling <init> on initialized object");

				/* get the address of the NEW instruction */
				OLD_LOGSTR("\t\t"); OLD_LOGINFO(&(av->typeinfo));
				ins = av->typeinfo.newobject_instruction();
				if (ins)
					initclass = ins[-1].sx.val.c;
				else
					initclass.cls = state->m->clazz;
				OLD_LOGSTR("\t\tclass: "); OLD_LOGNAME(initclass); OLD_LOGNL;
			}
		}
		else {
			/* non-adress argument. if this is the first argument and we are */
			/* invoking an instance method, this is an error.                */
			if (i==0 && !invokestatic) {
				TYPECHECK_VERIFYERROR_bool("Parameter type mismatch for 'this' argument");
			}
		}
		OLD_LOG("\t\tok");

#if defined(TYPECHECK_STACKBASED)
		av += (IS_2_WORD_TYPE(av->type)) ? 2 : 1;
#endif
	}

	if (callinginit) {
		OLD_LOG("\treplacing uninitialized object");
		/* replace uninitialized object type on stack */

		/* for all live-in and live-through variables */ 
#if defined(TYPECHECK_VARIABLESBASED)
		for (i=0; i<state->iptr->s1.argcount; ++i) {
			argindex = state->iptr->sx.s23.s2.args[i];
			av = VAR(argindex);
#else
		for (av=stackfloor; av <= stack; ++av) {
#endif
			if (av->type == TYPE_ADR
					&& av->typeinfo.is_newobject()
					&& av->typeinfo.newobject_instruction() == ins)
			{
				OLD_LOG("\treplacing uninitialized type");

#if defined(TYPECHECK_VARIABLESBASED)
				/* If this stackslot is in the instack of
				 * this basic block we must save the type(s)
				 * we are going to replace.
				 */
				/* XXX this needs a new check */
				if (state->bptr->invars
						&& argindex >= state->bptr->invars[0]
						&& argindex < state->bptr->varstart 
						&& !state->savedinvars)
				{
					typestate_save_invars(state);
				}
#endif /* defined(TYPECHECK_VARIABLESBASED) */

				if (!av->typeinfo.init_class(initclass))
					return false;
			}
		}

		/* replace uninitialized object type in locals */
#if defined(TYPECHECK_VARIABLESBASED)
		if (!typevector_init_object(state->jd->var, ins, initclass,
					state->numlocals))
			return false;
#else
		/* XXX should reuse typevector code */
		for (i=0; i<state->numlocals; ++i) {
			if (state->locals[i].type == TYPE_ADR
				&& state->locals[i].typeinfo.is_newobject()
				&& state->locals[i].typeinfo.newobject_instruction() == ins)
			{
				if (!state->locals[i].typeinfo.init_class(initclass))
					return false;
			}
		}
#endif

		/* initializing the 'this' reference? */
		if (!ins) {
			classinfo *cls;
			TYPECHECK_ASSERT(state->initmethod);
			/* { we are initializing the 'this' reference }                           */
			/* must be <init> of current class or direct superclass                   */
			/* the current class is linked, so must be its superclass. thus we can be */
			/* sure that resolving will be trivial.                                   */
			if (mi) {
				cls = mi->clazz;
			}
			else {
				if (!resolve_classref(state->m,mref->p.classref,resolveLazy,false,true,&cls))
					return false; /* exception */
			}

			/* if lazy resolving did not succeed, it's not one of the allowed classes */
			/* otherwise we check it directly                                         */
			if (cls == NULL || (cls != state->m->clazz && cls != state->m->clazz->super)) {
				TYPECHECK_VERIFYERROR_bool("<init> calling <init> of the wrong class");
			}

			/* set our marker variable to type int */
			OLD_LOG("\tsetting <init> marker");
#if defined(TYPECHECK_VARIABLESBASED)
			typevector_store(jd->var, state->numlocals-1, TYPE_INT, NULL);
#else
			state->locals[state->numlocals-1].type = TYPE_INT;
#endif
		}
		else {
			/* { we are initializing an instance created with NEW } */
			if (CLASSREF_OR_CLASSINFO_NAME(initclass) != mclassname) {
				TYPECHECK_VERIFYERROR_bool("wrong <init> called for uninitialized reference");
			}
		}
	} /* end if (callinginit) */

	/* try to resolve the method lazily */

	result = resolve_method_lazy(state->m, mref, invokespecial);

	/* perform verification checks */

	if (result == resolveSucceeded) {

		assert(mref->is_resolved());

		mi = mref->p.method;

		result = resolve_method_verifier_checks(state->m, 
												mref,
												mi,
												invokestatic);
	}

	/* check types of parameters */

	if (result == resolveSucceeded && !invokestatic)
		result = resolve_method_instance_type_checks(
				state->m, mi,
				&(OP1->typeinfo),
				invokespecial);

#if defined(TYPECHECK_VARIABLESBASED)
	if (result == resolveSucceeded)
		result = resolve_method_param_type_checks(
				jd, state->m, state->iptr,
				mi, invokestatic);
#else
	if (result == resolveSucceeded)
		result = resolve_method_param_type_checks_stackbased(
				state->m, mi, invokestatic, stack);
#endif

	/* impose loading constraints */

	if (result == resolveSucceeded) {
		/* XXX state->m->clazz may have to be wrong when inlining */
		if (!resolve_method_loading_constraints(state->m->clazz, mi))
			return false;
	}

	if (result == resolveFailed)
		return false;

	if (result == resolveSucceeded) {
		/* if this call is monomorphic, turn it into an INVOKESPECIAL */

		if ((state->iptr->opc == ICMD_INVOKEVIRTUAL)
			&& (mi->flags & (ACC_FINAL | ACC_PRIVATE)))
		{
			state->iptr->opc         = ICMD_INVOKESPECIAL;
			state->iptr->flags.bits |= INS_FLAG_CHECK;
		}
	}
	else {
		/* resolution must be deferred */

		if (!um) {
			um = resolve_create_unresolved_method(state->m->clazz, state->m,
					mref, 
					invokestatic,
					invokespecial);

			if (!um)
				return false;
		}

		/* record subtype constraints for parameters */

		if (!invokestatic && !resolve_constrain_unresolved_method_instance(
					um, state->m, 
					&(OP1->typeinfo),
					invokespecial))
			return false; /* XXX maybe wrap exception */

#if defined(TYPECHECK_VARIABLESBASED)
		if (!resolve_constrain_unresolved_method_params(
					jd, um, state->m, state->iptr))
			return false; /* XXX maybe wrap exception */
#else
		if (!resolve_constrain_unresolved_method_params_stackbased(
					um, state->m, stack))
			return false; /* XXX maybe wrap exception */
#endif

		/* store the unresolved_method pointer */

		state->iptr->sx.s23.s3.um = um;
		state->iptr->flags.bits |= INS_FLAG_UNRESOLVED;
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
