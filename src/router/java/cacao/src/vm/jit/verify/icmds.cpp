/* src/vm/jit/verify/icmds.cpp - ICMD-specific type checking code

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

#if 0 /* (needed for code examples in the following comment) */
/******************************************************************************/
/* This file contains ICMD-specific code for type checking and type
 * inference. It is an input file for the verifier generator
 * (src/vm/jit/verify/generate.pl). The verifier generator creates
 * code for three compiler passes:
 *     - stack-based type-infering verification
 *     - vasiables-based type-infering verification
 *     - type inference pass (no verification - used for optimizing compiler)
 *
 * The rest of this file must consist of "case" clauses starting in
 * the first column. Each clause can be marked with tags like this:
 *
 */          case ICMD_CONSTANT: /* {TAG, TAG, ...} */
/*
 * This must be on one line. The following tags are defined:
 *     STACKBASED..........use this clause for the stack-based verifier
 *     VARIABLESBASED......use this clause for the variables-based verifier
 *     TYPEINFERER.........use this clause for the type inference pass
 *     ALL.................use for all passes
 *
 * If no tag is specified, {STACKBASED,VARIABLESBASED} is assumed.
 *
 * There are also tags that can be used inside a clause like this:
 *
 */          /* {TAG} */
/*
 * The following tags are defined within clauses:
 *     RESULTNOW...........generate code for modelling the stack action
 *                         _before_ the user-defined code in the clause
 *                         (Default is to model the stack action afterwards.)
 *
 * The following macros are pre-defined:
 *
 *     TYPECHECK_STACKBASED.......iff compiling the stack-based verifier
 *     TYPECHECK_VARIABLESBASED...iff compiling the variables-based verifier
 *     TYPECHECK_TYPEINFERER......iff compiling the type inference pass
 *
/******************************************************************************/
#endif /* (end #if 0) */


/* this marker is needed by generate.pl: */
/* {START_OF_CODE} */

	/****************************************/
	/* MOVE/COPY                            */

	/* We just need to copy the typeinfo */
	/* for slots containing addresses.   */

	/* (These are only used by the variables based verifier.) */

case ICMD_MOVE: /* {VARIABLESBASED,TYPEINFERER} */
case ICMD_COPY: /* {VARIABLESBASED,TYPEINFERER} */
	TYPECHECK_COUNT(stat_ins_stack);
	COPYTYPE(IPTR->s1, IPTR->dst);
	DST->type = OP1->type;
	break;

	/****************************************/
	/* LOADING ADDRESS FROM VARIABLE        */

case ICMD_ALOAD: /* {ALL} */
	TYPECHECK_COUNT(stat_ins_aload);

#if !defined(TYPECHECK_TYPEINFERER)
	/* loading a returnAddress is not allowed */
	if (!OP1->is_reference()) {
		VERIFY_ERROR("illegal instruction: ALOAD loading non-reference");
	}
#endif
	DST->typeinfo = OP1->typeinfo;
	break;

	/****************************************/
	/* STORING ADDRESS TO VARIABLE          */

case ICMD_ASTORE: /* {ALL} */
	DST->typeinfo = OP1->typeinfo;
	break;

	/****************************************/
	/* LOADING ADDRESS FROM ARRAY           */

case ICMD_AALOAD: /* {ALL} */
#if !defined(TYPECHECK_TYPEINFERER)
	if (!OP1->typeinfo.maybe_array_of_refs())
		VERIFY_ERROR("illegal instruction: AALOAD on non-reference array");
#endif

	if (!DST->typeinfo.init_component(OP1->typeinfo))
		EXCEPTION;
	break;

	/****************************************/
	/* FIELD ACCESS                         */

case ICMD_PUTFIELD: /* {STACKBASED} */
	CHECK_STACK_DEPTH(2);
	if (!IS_CAT1(stack[0])) {
		CHECK_STACK_DEPTH(3);
		stack -= 1;
	}
	CHECK_STACK_TYPE(stack[-1], TYPE_ADR);
	stack = typecheck_stackbased_verify_fieldaccess(STATE, stack-1, stack, stack-2);
	if (stack == NULL)
		EXCEPTION;
	break;

case ICMD_PUTSTATIC: /* {STACKBASED} */
	CHECK_STACK_DEPTH(1);
	if (!IS_CAT1(stack[0])) {
		/* (stack depth >= 2 is guaranteed) */
		stack -= 1;
	}
	stack = typecheck_stackbased_verify_fieldaccess(STATE, NULL, stack, stack-1);
	if (stack == NULL)
		EXCEPTION;
	break;

case ICMD_GETFIELD: /* {STACKBASED} */
	CHECK_STACK_TYPE(stack[0], TYPE_ADR);
	stack = typecheck_stackbased_verify_fieldaccess(STATE, stack, NULL, stack-1);
	if (stack == NULL)
		EXCEPTION;
	break;

case ICMD_GETSTATIC:      /* {STACKBASED} */
	stack = typecheck_stackbased_verify_fieldaccess(STATE, NULL, NULL, stack);
	if (stack == NULL)
		EXCEPTION;
	break;

case ICMD_PUTFIELD:       /* {VARIABLESBASED} */
	if (!handle_fieldaccess(state, VAROP(iptr->s1), VAROP(iptr->sx.s23.s2)))
		return false;
	maythrow = true;
	break;

case ICMD_PUTSTATIC:      /* {VARIABLESBASED} */
	if (!handle_fieldaccess(state, NULL, VAROP(iptr->s1)))
		return false;
	maythrow = true;
	break;

case ICMD_PUTFIELDCONST:  /* {VARIABLESBASED} */
	/* XXX this mess will go away with const operands */
	INSTRUCTION_GET_FIELDREF(state->iptr, fieldref);
	constvalue.type = fieldref->parseddesc.fd->type;
	if (IS_ADR_TYPE(constvalue.type)) {
		if (state->iptr->sx.val.anyptr) {
			classinfo *cc = (state->iptr->flags.bits & INS_FLAG_CLASS)
				? class_java_lang_Class : class_java_lang_String;
			assert(cc);
			assert(cc->state & CLASS_LINKED);
			constvalue.typeinfo.init_class(cc);
		}
		else {
			constvalue.typeinfo.init_nulltype();
		}
	}
	if (!handle_fieldaccess(state, VAROP(iptr->s1), &constvalue))
		return false;
	maythrow = true;
	break;

case ICMD_PUTSTATICCONST: /* {VARIABLESBASED} */
	/* XXX this mess will go away with const operands */
	INSTRUCTION_GET_FIELDREF(state->iptr, fieldref);
	constvalue.type = fieldref->parseddesc.fd->type;
	if (IS_ADR_TYPE(constvalue.type)) {
		if (state->iptr->sx.val.anyptr) {
			classinfo *cc = (state->iptr->flags.bits & INS_FLAG_CLASS)
				? class_java_lang_Class : class_java_lang_String;
			assert(cc);
			assert(cc->state & CLASS_LINKED);
			constvalue.typeinfo.init_class(cc);
		}
		else {
			constvalue.typeinfo.init_nulltype();
		}
	}
	if (!handle_fieldaccess(state, NULL, &constvalue))
		return false;
	maythrow = true;
	break;

case ICMD_GETFIELD:       /* {VARIABLESBASED,TYPEINFERER} */
	if (!handle_fieldaccess(state, VAROP(iptr->s1), NULL))
		return false;
	maythrow = true;
	break;

case ICMD_GETSTATIC:      /* {VARIABLESBASED,TYPEINFERER} */
	if (!handle_fieldaccess(state, NULL, NULL))
		return false;
	maythrow = true;
	break;

	/****************************************/
	/* PRIMITIVE ARRAY ACCESS               */

case ICMD_ARRAYLENGTH:
	if (!OP1->typeinfo.maybe_array() && OP1->typeinfo.typeclass.cls != pseudo_class_Arraystub)
		VERIFY_ERROR("illegal instruction: ARRAYLENGTH on non-array");
	break;

case ICMD_BALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BOOLEAN)
			&& !OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BYTE))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_CALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_CHAR))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_DALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_DOUBLE))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_FALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_FLOAT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_IALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_INT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_SALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_SHORT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_LALOAD:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_LONG))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_BASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BOOLEAN)
			&& !OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BYTE))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_CASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_CHAR))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_DASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_DOUBLE))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_FASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_FLOAT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_IASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_INT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_SASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_SHORT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_LASTORE:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_LONG))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_AASTORE:
	/* we just check the basic input types and that the           */
	/* destination is an array of references. Assignability to    */
	/* the actual array must be checked at runtime, each time the */
	/* instruction is performed. (See builtin_canstore.)          */
	if (!OP1->typeinfo.maybe_array_of_refs())
		VERIFY_ERROR("illegal instruction: AASTORE to non-reference array");
	break;

case ICMD_IASTORECONST:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_INT))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_LASTORECONST:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_LONG))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_BASTORECONST:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BOOLEAN)
			&& !OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_BYTE))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_CASTORECONST:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_CHAR))
		VERIFY_ERROR("Array type mismatch");
	break;

case ICMD_SASTORECONST:
	if (!OP1->typeinfo.maybe_primitive_array(ARRAYTYPE_SHORT))
		VERIFY_ERROR("Array type mismatch");
	break;

	/****************************************/
	/* ADDRESS CONSTANTS                    */

case ICMD_ACONST: /* {ALL} */
	if (IPTR->flags.bits & INS_FLAG_CLASS) {
		/* a java.lang.Class reference */
		DST->typeinfo.init_java_lang_class(IPTR->sx.val.c);
	}
	else {
		if (IPTR->sx.val.anyptr == NULL)
			DST->typeinfo.init_nulltype();
		else {
			/* string constant (or constant for builtin function) */
			DST->typeinfo.init_class(class_java_lang_String);
		}
	}
	break;

	/****************************************/
	/* CHECKCAST AND INSTANCEOF             */

case ICMD_CHECKCAST: /* {ALL} */
#if !defined(TYPECHECK_TYPEINFERER)
	/* returnAddress is not allowed */
	if (!OP1->typeinfo.is_reference())
		VERIFY_ERROR("Illegal instruction: CHECKCAST on non-reference");
#endif

    /* XXX only if narrower */
	if (!DST->typeinfo.init_class(IPTR->sx.s23.s3.c))
		EXCEPTION;
	break;

case ICMD_INSTANCEOF:
	/* returnAddress is not allowed */
	if (!OP1->typeinfo.is_reference())
		VERIFY_ERROR("Illegal instruction: INSTANCEOF on non-reference");

	/* XXX should propagate type information to the following if-branches */
	break;

	/****************************************/
	/* BRANCH INSTRUCTIONS                  */

case ICMD_GOTO:            /* {ALL} */
case ICMD_IFNULL:          /* {ALL} */
case ICMD_IFNONNULL:       /* {ALL} */
case ICMD_IFEQ:            /* {ALL} */
case ICMD_IFNE:            /* {ALL} */
case ICMD_IFLT:            /* {ALL} */
case ICMD_IFGE:            /* {ALL} */
case ICMD_IFGT:            /* {ALL} */
case ICMD_IFLE:            /* {ALL} */
case ICMD_IF_ICMPEQ:       /* {ALL} */
case ICMD_IF_ICMPNE:       /* {ALL} */
case ICMD_IF_ICMPLT:       /* {ALL} */
case ICMD_IF_ICMPGE:       /* {ALL} */
case ICMD_IF_ICMPGT:       /* {ALL} */
case ICMD_IF_ICMPLE:       /* {ALL} */
case ICMD_IF_ACMPEQ:       /* {ALL} */
case ICMD_IF_ACMPNE:       /* {ALL} */

case ICMD_IF_LEQ:          /* {ALL} */
case ICMD_IF_LNE:          /* {ALL} */
case ICMD_IF_LLT:          /* {ALL} */
case ICMD_IF_LGE:          /* {ALL} */
case ICMD_IF_LGT:          /* {ALL} */
case ICMD_IF_LLE:          /* {ALL} */

case ICMD_IF_LCMPEQ:       /* {ALL} */
case ICMD_IF_LCMPNE:       /* {ALL} */
case ICMD_IF_LCMPLT:       /* {ALL} */
case ICMD_IF_LCMPGE:       /* {ALL} */
case ICMD_IF_LCMPGT:       /* {ALL} */
case ICMD_IF_LCMPLE:       /* {ALL} */
	/* {RESULTNOW} */
	TYPECHECK_COUNT(stat_ins_branch);

	/* propagate stack and variables to the target block */
	REACH(IPTR->dst);
	break;

	/****************************************/
	/* SWITCHES                             */

case ICMD_TABLESWITCH:     /* {ALL} */
	/* {RESULTNOW} */
	TYPECHECK_COUNT(stat_ins_switch);

	table = IPTR->dst.table;
	i = IPTR->sx.s23.s3.tablehigh
	- IPTR->sx.s23.s2.tablelow + 1 + 1; /* plus default */

	while (--i >= 0) {
		REACH(*table);
		table++;
	}

	OLD_LOG("switch done");
	break;

case ICMD_LOOKUPSWITCH:    /* {ALL} */
	/* {RESULTNOW} */
	TYPECHECK_COUNT(stat_ins_switch);

	lookup = IPTR->dst.lookup;
	i = IPTR->sx.s23.s2.lookupcount;
	REACH(IPTR->sx.s23.s3.lookupdefault);

	while (--i >= 0) {
		REACH(lookup->target);
		lookup++;
	}

	OLD_LOG("switch done");
	break;


	/****************************************/
	/* ADDRESS RETURNS AND THROW            */

case ICMD_ATHROW:
	TYPECHECK_COUNT(stat_ins_athrow);
	r = OP1->typeinfo.is_assignable_to_class(to_classref_or_classinfo(class_java_lang_Throwable));
	if (r == typecheck_FALSE)
		VERIFY_ERROR("illegal instruction: ATHROW on non-Throwable");
	if (r == typecheck_FAIL)
		EXCEPTION;
	if (r == typecheck_MAYBE) {
		/* the check has to be postponed. we need a patcher */
		TYPECHECK_COUNT(stat_ins_athrow_unresolved);
		IPTR->sx.s23.s2.uc = create_unresolved_class(
				METHOD,
				/* XXX make this more efficient, use class_java_lang_Throwable
				 * directly */
				class_get_classref(METHOD->clazz,utf8::java_lang_Throwable),
				&OP1->typeinfo);
		IPTR->flags.bits |= INS_FLAG_UNRESOLVED;
	}
	break;

case ICMD_ARETURN:
	TYPECHECK_COUNT(stat_ins_areturn);
	if (!OP1->typeinfo.is_reference())
		VERIFY_ERROR("illegal instruction: ARETURN on non-reference");

	if (STATE->returntype.type != TYPE_ADR
			|| (r = OP1->typeinfo.is_assignable_to(&(STATE->returntype.typeinfo)))
			== typecheck_FALSE)
		VERIFY_ERROR("Return type mismatch");
	if (r == typecheck_FAIL)
		EXCEPTION;
	if (r == typecheck_MAYBE) {
		/* the check has to be postponed, we need a patcher */
		TYPECHECK_COUNT(stat_ins_areturn_unresolved);
		IPTR->sx.s23.s2.uc = create_unresolved_class(
				METHOD,
				METHOD->parseddesc->returntype.classref,
				&OP1->typeinfo);
		IPTR->flags.bits |= INS_FLAG_UNRESOLVED;
	}
	goto return_tail;

	/****************************************/
	/* PRIMITIVE RETURNS                    */

case ICMD_IRETURN:
	if (STATE->returntype.type != TYPE_INT)
		VERIFY_ERROR("Return type mismatch");
	goto return_tail;

case ICMD_LRETURN:
	if (STATE->returntype.type != TYPE_LNG)
		VERIFY_ERROR("Return type mismatch");
	goto return_tail;

case ICMD_FRETURN:
	if (STATE->returntype.type != TYPE_FLT)
		VERIFY_ERROR("Return type mismatch");
	goto return_tail;

case ICMD_DRETURN:
	if (STATE->returntype.type != TYPE_DBL)
		VERIFY_ERROR("Return type mismatch");
	goto return_tail;

case ICMD_RETURN:
	if (STATE->returntype.type != TYPE_VOID)
		VERIFY_ERROR("Return type mismatch");

return_tail:
	TYPECHECK_COUNT(stat_ins_primitive_return);

	if (STATE->initmethod && METHOD->clazz != class_java_lang_Object) {
		/* Check if the 'this' instance has been initialized. */
		OLD_LOG("Checking <init> marker");
#if defined(TYPECHECK_VARIABLESBASED)
		if (!typevector_checktype(jd->var,STATE->numlocals-1,TYPE_INT))
#else
		if (STATE->locals[STATE->numlocals-1].type != TYPE_INT)
#endif
			VERIFY_ERROR("<init> method does not initialize 'this'");
	}
	break;

	/****************************************/
	/* SUBROUTINE INSTRUCTIONS              */

case ICMD_JSR: /* {VARIABLESBASED,TYPEINFERER} */
	DST->typeinfo.init_returnaddress(BPTR->next);
	REACH(IPTR->sx.s23.s3.jsrtarget);
	break;

case ICMD_JSR: /* {STACKBASED} */
	/* {RESULTNOW} */
	tbptr = IPTR->sx.s23.s3.jsrtarget.block;

	stack[0].typeinfo.init_returnaddress(tbptr);
	REACH_BLOCK(tbptr);

	stack = typecheck_stackbased_jsr(STATE, stack, stackfloor);
	if (stack == NULL)
		EXCEPTION;
	break;

case ICMD_RET: /* {VARIABLESBASED,TYPEINFERER} */
#if !defined(TYPECHECK_TYPEINFERER)
	/* check returnAddress variable */
	if (!typevector_checkretaddr(jd->var,IPTR->s1.varindex))
		VERIFY_ERROR("illegal instruction: RET using non-returnAddress variable");
#endif
	REACH(IPTR->dst);
	break;

case ICMD_RET: /* {STACKBASED} */
	/* {RESULTNOW} */
	CHECK_LOCAL_TYPE(IPTR->s1.varindex, TYPE_RET);
	if (!STATE->locals[IPTR->s1.varindex].typeinfo.is_primitive())
		VERIFY_ERROR("illegal instruction: RET using non-returnAddress variable");

	if (!typecheck_stackbased_ret(STATE, stack, stackfloor))
		EXCEPTION;
	break;

	/****************************************/
	/* INVOKATIONS                          */

case ICMD_INVOKEVIRTUAL:   /* {VARIABLESBASED,TYPEINFERER} */
case ICMD_INVOKESPECIAL:   /* {VARIABLESBASED,TYPEINFERER} */
case ICMD_INVOKESTATIC:    /* {VARIABLESBASED,TYPEINFERER} */
case ICMD_INVOKEINTERFACE: /* {VARIABLESBASED,TYPEINFERER} */
	TYPECHECK_COUNT(stat_ins_invoke);
	if (!handle_invocation(state))
		EXCEPTION;
	TYPECHECK_COUNTIF(INSTRUCTION_IS_UNRESOLVED(IPTR), stat_ins_invoke_unresolved);
	break;

case ICMD_INVOKEVIRTUAL:   /* {STACKBASED} */
case ICMD_INVOKESPECIAL:   /* {STACKBASED} */
case ICMD_INVOKESTATIC:    /* {STACKBASED} */
case ICMD_INVOKEINTERFACE: /* {STACKBASED} */
	TYPECHECK_COUNT(stat_ins_invoke);

	INSTRUCTION_GET_METHODDESC(IPTR, md);
	CHECK_STACK_DEPTH(md->paramslots);

	if (!typecheck_stackbased_verify_invocation(STATE, stack, stackfloor))
		EXCEPTION;

	stack -= md->paramslots;

	if (md->returntype.type != TYPE_VOID) {
		if (IS_2_WORD_TYPE(md->returntype.type)) {
			CHECK_STACK_SPACE(2);
			stack += 2;
			stack[0].type = TYPE_VOID;
			stack[-1].type = md->returntype.type;
		}
		else {
			CHECK_STACK_SPACE(1);
			stack += 1;
			stack[0].type = md->returntype.type;
		}
	}
	TYPECHECK_COUNTIF(INSTRUCTION_IS_UNRESOLVED(IPTR), stat_ins_invoke_unresolved);
	break;

	/****************************************/
	/* MULTIANEWARRAY                       */

case ICMD_MULTIANEWARRAY: /* {VARIABLESBASED,TYPEINFERER} */
	if (!handle_multianewarray(STATE))
		EXCEPTION;
	break;

case ICMD_MULTIANEWARRAY: /* {STACKBASED} */
	if (!typecheck_stackbased_multianewarray(STATE, stack, stackfloor))
		EXCEPTION;
	stack -= (IPTR->s1.argcount - 1);
	stack[0].type = TYPE_ADR;
	break;

	/****************************************/
	/* BUILTINS                             */

case ICMD_BUILTIN: /* {VARIABLESBASED,TYPEINFERER} */
	TYPECHECK_COUNT(stat_ins_builtin);
	if (!handle_builtin(state))
		EXCEPTION;
	break;

case ICMD_BUILTIN: /* {STACKBASED} */
	TYPECHECK_COUNT(stat_ins_builtin);
	if (!typecheck_stackbased_verify_builtin(STATE, stack, stackfloor))
		EXCEPTION;

	/* pop operands and push return value */
	{
		u1 rtype = IPTR->sx.s23.s3.bte->md->returntype.type;
		stack -=  IPTR->sx.s23.s3.bte->md->paramslots;
		if (rtype != TYPE_VOID) {
			if (IS_2_WORD_TYPE(rtype))
				stack += 2;
			else
				stack += 1;
		}
	}
	break;

/* the following code is only used by the stackbased verifier */

case ICMD_POP: /* {STACKBASED} */
	/* we pop 1 */
	CHECK_CAT1(stack[0]);
	break;

case ICMD_POP2: /* {STACKBASED} */
	/* we pop either 11 or 2 */
	if (IS_CAT1(stack[0]))
		CHECK_CAT1(stack[-1]);
	break;

case ICMD_SWAP: /* {STACKBASED} */
	CHECK_CAT1(stack[0]);
	CHECK_CAT1(stack[-1]);

	COPY_SLOT(stack[ 0], temp     );
	COPY_SLOT(stack[-1], stack[ 0]);
	COPY_SLOT(temp     , stack[-1]);
	break;

case ICMD_DUP: /* {STACKBASED} */
	/* we dup 1 */
	CHECK_CAT1(stack[0]);

	COPY_SLOT(stack[ 0], stack[ 1]);
	break;

case ICMD_DUP_X1: /* {STACKBASED} */
	/* we dup 1 */
	CHECK_CAT1(stack[0]);
	/* we skip 1 */
	CHECK_CAT1(stack[-1]);

	COPY_SLOT(stack[ 0], stack[ 1]);
	COPY_SLOT(stack[-1], stack[ 0]);
	COPY_SLOT(stack[ 1], stack[-1]);
	break;

case ICMD_DUP_X2: /* {STACKBASED} */
	/* we dup 1 */
	CHECK_CAT1(stack[0]);
	/* we skip either 11 or 2 */
	if (IS_CAT1(stack[-1]))
		CHECK_CAT1(stack[-2]);

	COPY_SLOT(stack[ 0], stack[ 1]);
	COPY_SLOT(stack[-1], stack[ 0]);
	COPY_SLOT(stack[-2], stack[-1]);
	COPY_SLOT(stack[ 1], stack[-2]);
	break;

case ICMD_DUP2: /* {STACKBASED} */
	/* we dup either 11 or 2 */
	if (IS_CAT1(stack[0]))
		CHECK_CAT1(stack[-1]);

	COPY_SLOT(stack[ 0], stack[ 2]);
	COPY_SLOT(stack[-1], stack[ 1]);
	break;

case ICMD_DUP2_X1: /* {STACKBASED} */
	/* we dup either 11 or 2 */
	if (IS_CAT1(stack[0]))
		CHECK_CAT1(stack[-1]);
	/* we skip 1 */
	CHECK_CAT1(stack[-2]);

	COPY_SLOT(stack[ 0], stack[ 2]);
	COPY_SLOT(stack[-1], stack[ 1]);
	COPY_SLOT(stack[-2], stack[ 0]);
	COPY_SLOT(stack[ 2], stack[-1]);
	COPY_SLOT(stack[ 1], stack[-2]);
	break;

case ICMD_DUP2_X2: /* {STACKBASED} */
	/* we dup either 11 or 2 */
	if (IS_CAT1(stack[0]))
		CHECK_CAT1(stack[-1]);
	/* we skip either 11 or 2 */
	if (IS_CAT1(stack[-2]))
		CHECK_CAT1(stack[-3]);

	COPY_SLOT(stack[ 0], stack[ 2]);
	COPY_SLOT(stack[-1], stack[ 1]);
	COPY_SLOT(stack[-2], stack[ 0]);
	COPY_SLOT(stack[-3], stack[-1]);
	COPY_SLOT(stack[ 2], stack[-2]);
	COPY_SLOT(stack[ 1], stack[-3]);
	break;


/* this marker is needed by generate.pl: */
/* {END_OF_CODE} */

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
