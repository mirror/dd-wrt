/* src/vm/jit/verify/typecheck-stackbased.c - stack-based verifier

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


#include "config.h"

#include <assert.h>

#include "vm/method.hpp"
#include "vm/types.hpp"

#include "mm/memory.hpp"
#include "mm/dumpmemory.hpp"

#include "vm/array.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/primitive.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/stack.hpp"
#include "vm/jit/ir/instruction.hpp"
#include "vm/jit/verify/typecheck-common.hpp"


/* this #if runs over the whole file: */
#if defined(ENABLE_VERIFIER)

typedef typedescriptor_t verifier_slot_t;

#if defined(TYPECHECK_VERBOSE)
static void typecheck_stackbased_show_state(verifier_state *state,
											typedescriptor *stack,
											typedescriptor *stackfloor,
											bool showins);
#endif


#define CHECK_STACK_DEPTH(d)                                         \
    if (((u1*)stack - (u1*)stackfloor)                               \
            < (((d)-1) * (int)sizeof(verifier_slot_t)))              \
        goto throw_stack_underflow;

/* XXX don't need to check against ACONST for every ICMD */
#define CHECK_STACK_SPACE(d)                                         \
    if (((u1*)STATE->stackceiling - (u1*)stack)                      \
            < (((d)+1) * (int)sizeof(verifier_slot_t)))              \
        if (STATE->iptr->opc != ICMD_ACONST                          \
                || INSTRUCTION_MUST_CHECK(STATE->iptr))              \
            goto throw_stack_overflow;

#define CHECK_STACK_TYPE(s, t)                                       \
    if ((s).type != (t))                                             \
        goto throw_stack_type_error;

/* XXX inefficient */
#define CHECK_LOCAL_TYPE(index, t)                                   \
    do {                                                             \
        if (state.locals[(index)].type != (t))                       \
            goto throw_local_type_error;                             \
        if (STATE->topjsr)                                           \
            STATE->topjsr->usedlocals[(index)] = 1;                  \
        if (STATE->topjsr && IS_2_WORD_TYPE(t))                      \
            STATE->topjsr->usedlocals[(index) + 1] = 1;              \
    } while(0)

/* XXX inefficient */
#define STORE_LOCAL(t, index)                                        \
    do {                                                             \
        state.locals[(index)].type = (t);                            \
        if ((index) && IS_2_WORD_TYPE(state.locals[(index)-1].type)) \
            state.locals[(index-1)].type = TYPE_VOID;                \
        if (STATE->topjsr)                                           \
            STATE->topjsr->usedlocals[(index)] = 1;                  \
    } while (0)

/* XXX inefficient */
#define STORE_LOCAL_2_WORD(t, index)                                 \
    do {                                                             \
        STORE_LOCAL(t, index);                                       \
        state.locals[(index)+1].type = TYPE_VOID;                    \
        if (STATE->topjsr)                                           \
            STATE->topjsr->usedlocals[(index)] = 1;                  \
    } while (0)

#define VERIFY_ERROR_ret(msg,ret)                                    \
    do {                                                             \
        OLD_LOG1("VerifyError: %s", msg);                                \
        exceptions_throw_verifyerror(STATE->m, msg);                 \
        return ret;                                                  \
    } while (0)

#define VERIFY_ERROR(msg) VERIFY_ERROR_ret(msg,false)

#define IS_CAT1(slot)                                                \
    ((slot).type != TYPE_VOID && !IS_2_WORD_TYPE((slot).type))

#define IS_CAT2(slot)                                                \
    ((slot).type != TYPE_VOID && IS_2_WORD_TYPE((slot).type))

#define CHECK_CAT1(slot)                                             \
    do {                                                             \
        if (!IS_CAT1(slot))                                          \
            goto throw_stack_category_error;                         \
    } while (0)

#define CHECK_CAT2(slot)                                             \
    do {                                                             \
        if (!IS_CAT2(slot))                                          \
            goto throw_stack_category_error;                         \
    } while (0)

#define COPY_SLOT(s, d)                                              \
    do { (d) = (s); } while (0)

#define REACH_BLOCK(target)                                          \
    do {                                                             \
        if (!typecheck_stackbased_reach(STATE, (target), stack,      \
                    (stack - stackfloor) + 1))                       \
            return false;                                            \
    } while (0)

#define REACH(target)                                                \
    do {                                                             \
        REACH_BLOCK((target).block);                                 \
    } while (0)

#undef TYPECHECK_INT
#undef TYPECHECK_LNG
#undef TYPECHECK_FLT
#undef TYPECHECK_DBL
#undef TYPECHECK_ADR

/* XXX should reuse typevector code */
static typecheck_result typecheck_stackbased_merge_locals(methodinfo *m,
														  typedescriptor_t *dst,
														  typedescriptor_t *y,
														  int size)
{
	bool changed = false;
	typecheck_result r;

	typedescriptor_t *a = dst;
	typedescriptor_t *b = y;
	while (size--) {
		if (a->type != TYPE_VOID && a->type != b->type) {
			a->type = TYPE_VOID;
			changed = true;
		}
		else if (a->type == TYPE_ADR) {
			if (a->typeinfo.is_primitive()) {
				/* 'a' is a returnAddress */
				if (!b->typeinfo.is_primitive() || (a->typeinfo.returnaddress() != b->typeinfo.returnaddress()))
				{
					a->type = TYPE_VOID;
					changed = true;
				}
			}
			else {
				/* 'a' is a reference */
				if (b->typeinfo.is_primitive()) {
					a->type = TYPE_VOID;
					changed = true;
				}
				else {
					/* two reference types are merged. There cannot be */
					/* a merge error. In the worst case we get j.l.O.  */
					r = a->typeinfo.merge(m, &(b->typeinfo));
					if (r == typecheck_FAIL)
						return r;
					changed |= r;
				}
			}
		}
		a++;
		b++;
	}
	return (typecheck_result) changed;
}

static typecheck_result typecheck_stackbased_merge(verifier_state *state,
												   basicblock *destblock,
												   typedescriptor_t *stack,
												   s4 stackdepth)
{
	s4 i;
	s4 destidx;
	typedescriptor_t *stackfloor;
	typedescriptor_t *sp;
	typedescriptor_t *dp;
	typecheck_result r;
	bool changed = false;

	destidx = destblock->nr;

	if (stackdepth != state->indepth[destidx]) {
		exceptions_throw_verifyerror(state->m, "Stack depth mismatch");
		return typecheck_FAIL;
	}

	stackfloor = stack - (stackdepth - 1);

	sp = stackfloor;
	dp = state->startstack + (destidx * state->m->maxstack);

	for (i=0; i<stackdepth; ++i, ++sp, ++dp) {
		if (sp->type != dp->type) {
			exceptions_throw_verifyerror(state->m, "Mismatched stack types");
			return typecheck_FAIL;
		}
		if (dp->type == TYPE_ADR) {
			if (dp->typeinfo.is_primitive()) {
				/* dp has returnAddress type */
				if (sp->typeinfo.is_primitive()) {
					if (dp->typeinfo.returnaddress() != sp->typeinfo.returnaddress()) {
						exceptions_throw_verifyerror(state->m, "Mismatched stack types");
						return typecheck_FAIL;
					}
				}
				else {
					exceptions_throw_verifyerror(state->m,"Merging returnAddress with reference");
					return typecheck_FAIL;
				}
			}
			else {
				/* dp has reference type */
				if (sp->typeinfo.is_primitive()) {
					exceptions_throw_verifyerror(state->m,"Merging reference with returnAddress");
					return typecheck_FAIL;
				}
				r = dp->typeinfo.merge(state->m, &(sp->typeinfo));
				if (r == typecheck_FAIL)
					return r;
				changed |= r;
			}
		}
	}

	dp = state->startlocals + (destidx * state->numlocals);
	r = typecheck_stackbased_merge_locals(state->m, dp, state->locals, state->numlocals);
	if (r == typecheck_FAIL)
		return r;
	changed |= r;

	return (typecheck_result) changed;
}

static bool typecheck_stackbased_reach(verifier_state *state,
									   basicblock *destblock,
									   typedescriptor_t *stack,
									   s4 stackdepth)
{
	bool changed = false;
	typecheck_result r;

	assert(destblock);

	if (destblock->state == basicblock::TYPECHECK_UNDEF) {
		/* The destblock has never been reached before */

		TYPECHECK_COUNT(stat_copied);
		OLD_LOG1("block L%03d reached first time",destblock->nr); OLD_LOGSTR("\t");
		DOLOG( typecheck_stackbased_show_state(state, stack, stack - (stackdepth - 1), false); );

		state->indepth[destblock->nr] = stackdepth;

		MCOPY(state->startstack + (destblock->nr * state->m->maxstack),
			  stack - (stackdepth - 1),
			  typedescriptor_t,
			  stackdepth);

		MCOPY(state->startlocals + (destblock->nr * state->numlocals),
			  state->locals,
			  typedescriptor_t,
			  state->numlocals);

		changed = true;
	}
	else {
		/* The destblock has already been reached before */

		TYPECHECK_COUNT(stat_merged);
		OLD_LOG1("block L%03d reached before", destblock->nr); OLD_LOGSTR("\t");
		DOLOG( typecheck_stackbased_show_state(state, stack, stack - (stackdepth - 1), false); );

		r = typecheck_stackbased_merge(state, destblock, stack, stackdepth);
		if (r == typecheck_FAIL)
			return false;
		changed = r;

		TYPECHECK_COUNTIF(changed,stat_merging_changed);
	}

	if (changed) {
		OLD_LOG("\tchanged!");
		destblock->state = basicblock::TYPECHECK_REACHED;
		/* XXX is this check ok? */
		if (destblock->nr <= state->bptr->nr) {
			OLD_LOG("\tREPEAT!");
			state->repeat = true;
		}
	}
	return true;
}


/* typecheck_stackbased_verify_fieldaccess *************************************

   Verify an ICMD_{GET,PUT}{STATIC,FIELD}

   IN:
       state............the current state of the verifier
	   instance.........the instance slot, or NULL
	   value............the value slot, or NULL
	   stack............stack after popping the arguments

   RETURN VALUE:
       stack pointer....successful verification,
	   NULL.............an exception has been thrown.

*******************************************************************************/

static typedescriptor_t *typecheck_stackbased_verify_fieldaccess(
		verifier_state *state,
		typedescriptor_t *instance,
		typedescriptor_t *value,
		typedescriptor_t *stack)
{
	//jitdata *jd;

	//jd = state->jd;

#define TYPECHECK_STACKBASED
#define EXCEPTION  do { return NULL; } while (0)
#define STATE  state
#undef  VERIFY_ERROR
#define VERIFY_ERROR(msg) VERIFY_ERROR_ret(msg,NULL)
#include <typecheck-fields.inc>
#undef  EXCEPTION
#undef  STATE
#undef  TYPECHECK_STACKBASED
#undef  VERIFY_ERROR
#define VERIFY_ERROR(msg) VERIFY_ERROR_ret(msg,false)

	return stack;

throw_stack_overflow:
	OLD_LOG("STACK OVERFLOW!");
	exceptions_throw_verifyerror(state->m, "Stack size too large");
	return NULL;
}

static bool typecheck_stackbased_verify_invocation(verifier_state *state,
												   typedescriptor_t *stack,
												   typedescriptor_t *stackfloor)
{
	s4 paramslots;
	methoddesc *md;
	typedescriptor_t *dv;

	/* check stack depth */

	/* XXX parse params */

	INSTRUCTION_GET_METHODDESC(state->iptr, md);

	paramslots = md->paramslots;

	if ((stack - stackfloor) + 1 < paramslots) {
		exceptions_throw_verifyerror(state->m, 
				"Trying to pop operand of an empty stack");
		return false;
	}

	dv = stack - (paramslots - 1);

#define TYPECHECK_STACKBASED
#define OP1   dv
#include <typecheck-invoke.inc>
#undef  OP1
#undef  TYPECHECK_STACKBASED

	return true;
}

static bool typecheck_stackbased_verify_builtin(verifier_state *state,
												typedescriptor_t *stack,
												typedescriptor_t *stackfloor)
{
	s4 paramslots;
	typedescriptor_t *dv;

	/* check stack depth */

	/* XXX parse params */

	paramslots = state->iptr->sx.s23.s3.bte->md->paramslots;

	if ((stack - stackfloor) + 1 < paramslots) {
		exceptions_throw_verifyerror(state->m, 
				"Trying to pop operand of an empty stack");
		return false;
	}

	dv = stack - (paramslots - 1);

#define TYPECHECK_STACKBASED
#define OP1   dv
#define TYPECHECK_INT(s)  CHECK_STACK_TYPE(*(s), TYPE_INT)
#define TYPECHECK_ADR(s)  CHECK_STACK_TYPE(*(s), TYPE_ADR)
#define TYPECHECK_LNG(s)  CHECK_STACK_TYPE(*(s), TYPE_LNG)
#define TYPECHECK_FLT(s)  CHECK_STACK_TYPE(*(s), TYPE_FLT)
#define TYPECHECK_DBL(s)  CHECK_STACK_TYPE(*(s), TYPE_DBL)
#include <typecheck-builtins.inc>
#undef  OP1
#undef  TYPECHECK_STACKBASED

	return true;

throw_stack_type_error:
	exceptions_throw_verifyerror(state->m, "Wrong type on stack"); /* XXX */
	return false;
}

static bool typecheck_stackbased_multianewarray(verifier_state *state,
												typedescriptor_t *stack,
												typedescriptor_t *stackfloor)
{
	/* XXX recombine with verify_multianewarray */

	classinfo *arrayclass;
	arraydescriptor *desc;
	s4 i;
	typedescriptor_t *sp;
	typedescriptor_t *dst;

	/* destination slot */

	i = state->iptr->s1.argcount;

	dst = stack - (i-1);

	/* check the array lengths on the stack */

	if ((stack - stackfloor) + 1 < i) {
		exceptions_throw_verifyerror(state->m, 
				"Trying to pop operand of an empty stack");
		return false;
	}

	if (i < 1)
		TYPECHECK_VERIFYERROR_bool("Illegal dimension argument");

	for (sp = dst; sp <= stack; ++sp) {
		if (sp->type != TYPE_INT) {
			exceptions_throw_verifyerror_for_stack(state->m, TYPE_INT);
			return false;
		}
	}

	/* check array descriptor */

	if (INSTRUCTION_IS_RESOLVED(state->iptr)) {
		/* the array class reference has already been resolved */
		arrayclass = state->iptr->sx.s23.s3.c.cls;
		if (!arrayclass)
			TYPECHECK_VERIFYERROR_bool("MULTIANEWARRAY with unlinked class");
		if ((desc = arrayclass->vftbl->arraydesc) == NULL)
			TYPECHECK_VERIFYERROR_bool("MULTIANEWARRAY with non-array class");
		if (desc->dimension < state->iptr->s1.argcount)
			TYPECHECK_VERIFYERROR_bool("MULTIANEWARRAY dimension to high");

		/* set the array type of the result */
		dst->typeinfo.init_class(arrayclass);
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
		if (i < 1)
			TYPECHECK_VERIFYERROR_bool("MULTIANEWARRAY with non-array class");
		if (i < state->iptr->s1.argcount)
			TYPECHECK_VERIFYERROR_bool("MULTIANEWARRAY dimension to high");

		/* set the array type of the result */
		if (!dst->typeinfo.init_class(cr))
			return false;
	}

	/* everything ok */
	return true;

}

static void typecheck_stackbased_add_jsr_caller(typecheck_jsr_t *jsr,
												basicblock *bptr)
{
	typecheck_jsr_caller_t *jc;

	for (jc = jsr->callers; jc; jc = jc->next)
		if (jc->callblock == bptr)
			return;

	jc = (typecheck_jsr_caller_t*) DumpMemory::allocate(sizeof(typecheck_jsr_caller_t));
	jc->next = jsr->callers;
	jc->callblock = bptr;
	jsr->callers = jc;
}

static typedescriptor_t *typecheck_stackbased_jsr(verifier_state *state,
												typedescriptor_t *stack,
												typedescriptor_t *stackfloor)
{
	typecheck_jsr_t *jsr;
	basicblock *tbptr;
	//jitdata *jd;
	s4 i;

	//jd = state->jd;

	tbptr = state->iptr->sx.s23.s3.jsrtarget.block;
	jsr = state->jsrinfos[tbptr->nr];

	if (jsr && tbptr->state == basicblock::FINISHED) {

		OLD_LOG1("another JSR to analysed subroutine L%03d", tbptr->nr);
		if (jsr->active) {
			exceptions_throw_verifyerror(state->m, "Recursive JSR");
			return NULL;
		}

		assert(jsr->callers);
		assert(jsr->callers->callblock);

		/* copy the stack of the RET edge */

		MCOPY(stackfloor, jsr->retstack, typedescriptor_t, jsr->retdepth);
		stack = stackfloor + (jsr->retdepth - 1);

		/* copy variables that were used in the subroutine from the RET edge */

		for (i=0; i<state->numlocals; ++i)
			if (jsr->usedlocals[i])
				state->locals[i] = jsr->retlocals[i];

		/* reach the following block */

		if (!typecheck_stackbased_reach(state, state->bptr->next, stack, 
					(stack - stackfloor) + 1))
			return NULL;
	}
	else {
		if (!jsr) {
			OLD_LOG1("first JSR to block L%03d", tbptr->nr);

			jsr = (typecheck_jsr_t*) DumpMemory::allocate(sizeof(typecheck_jsr_t));
			state->jsrinfos[tbptr->nr] = jsr;
			jsr->callers = NULL;
			jsr->blockflags = (char*) DumpMemory::allocate(sizeof(char) * state->basicblockcount);
			jsr->retblock = NULL;
			jsr->start = tbptr;
			jsr->usedlocals = (char*) DumpMemory::allocate(sizeof(char) * state->numlocals);
			MZERO(jsr->usedlocals, char, state->numlocals);
			jsr->retlocals = (typedescriptor_t*) DumpMemory::allocate(sizeof(typedescriptor_t) * state->numlocals);
			jsr->retstack = (typedescriptor_t*) DumpMemory::allocate(sizeof(typedescriptor_t) * state->m->maxstack);
			jsr->retdepth = 0;
		}
		else {
			OLD_LOG1("re-analysing JSR to block L%03d", tbptr->nr);
		}

		jsr->active = true;
		jsr->next = state->topjsr;
		state->topjsr = jsr;

		assert(state->iptr->sx.s23.s3.jsrtarget.block->state == basicblock::TYPECHECK_REACHED);

		tbptr->state = basicblock::FINISHED;

		for (tbptr = state->basicblocks; tbptr != NULL; tbptr = tbptr->next) {
			jsr->blockflags[tbptr->nr] = tbptr->state;

			if (tbptr->state == basicblock::TYPECHECK_REACHED)
				tbptr->state = basicblock::FINISHED;
		}

		state->iptr->sx.s23.s3.jsrtarget.block->state = basicblock::TYPECHECK_REACHED;
	}

	/* register this block as a caller, if not already done */

	typecheck_stackbased_add_jsr_caller(jsr, state->bptr);

	return stack;
}

static bool typecheck_stackbased_ret(verifier_state *state,
									 typedescriptor_t *stack,
									 typedescriptor_t *stackfloor)
{
	basicblock *tbptr;
	typecheck_jsr_caller_t *jsrcaller;
	typecheck_jsr_t *jsr;
	s4 i;

	/* get the subroutine we are RETurning from */

	tbptr = (basicblock*) state->locals[state->iptr->s1.varindex].typeinfo.returnaddress();
	if (tbptr == NULL) {
		exceptions_throw_verifyerror(state->m, "Illegal RET");
		return false;
	}

	OLD_LOG1("RET from subroutine L%03d", tbptr->nr);
	jsr = state->jsrinfos[tbptr->nr];
	assert(jsr);

	/* check against recursion */

	if (!jsr->active) {
		exceptions_throw_verifyerror(state->m, "RET outside of local subroutine");
		return false;
	}

	/* check against multiple RETs for one subroutine */

	if (jsr->retblock && jsr->retblock != state->bptr) {
		exceptions_throw_verifyerror(state->m, "Multiple RETs from local subroutine");
		return false;
	}

	/* store data-flow of the RET edge */

	jsr->retblock = state->bptr;
	jsr->retdepth = (stack - stackfloor) + 1;
	MCOPY(jsr->retstack, stackfloor, typedescriptor_t, jsr->retdepth);
	MCOPY(jsr->retlocals, state->locals, typedescriptor_t, state->numlocals);

	/* invalidate the returnAddress used by this RET */
	/* XXX should we also invalidate the returnAddresses of JSRs that are skipped by this RET? */

	for (i=0; i<state->numlocals; ++i) {
		typedescriptor_t *lc = &(jsr->retlocals[i]);
		if (lc->is_returnaddress())
			if (lc->typeinfo.returnaddress() == tbptr) {
				OLD_LOG1("invalidating returnAddress in local %d", i);
				lc->typeinfo.init_returnaddress(NULL);
			}
	}

	/* touch all callers of the subroutine, so they are analysed again */

	for (jsrcaller = jsr->callers; jsrcaller != NULL; jsrcaller = jsrcaller->next) {
		tbptr = jsrcaller->callblock;
		OLD_LOG1("touching caller L%03d from RET", tbptr->nr);
		assert(jsr->blockflags[tbptr->nr] >= basicblock::FINISHED);
		jsr->blockflags[tbptr->nr] = basicblock::TYPECHECK_REACHED; /* XXX repeat? */
	}

	return true;
}

bool typecheck_stackbased(jitdata *jd)
{
	register verifier_slot_t *stack;
	verifier_slot_t *stackfloor;
	s4 len;
	methoddesc *md;
	bool maythrow;
	bool superblockend;
	verifier_slot_t temp;
	branch_target_t *table;
	lookup_target_t *lookup;
	s4 i;
	typecheck_result r;
	verifier_slot_t *dst;
	verifier_state state;
	basicblock *tbptr;
	exception_entry *ex;
	typedescriptor_t exstack;
	s4 skip = 0;

	DOLOG( show_method(jd, SHOW_PARSE); );

	/* initialize verifier state */

	state.jd = jd;
	state.m = jd->m;
	state.cd = jd->cd;
	state.basicblocks = jd->basicblocks;
	state.basicblockcount = jd->basicblockcount;
	state.topjsr = NULL;
#   define STATE (&state)

	/* check that the basicblock numbers are valid */

#if !defined(NDEBUG)
	jit_check_basicblock_numbers(jd);
#endif

	/* check if this method is an instance initializer method */

    state.initmethod = (state.m->name == utf8::init);

	/* allocate parameter descriptors if necessary */

	state.m->parseddesc->params_from_paramtypes(state.m->flags);

	/* allocate the stack buffers */

	stackfloor = (verifier_slot_t*) DumpMemory::allocate(sizeof(verifier_slot_t) * (state.m->maxstack + 1));
	state.stackceiling = stackfloor + state.m->maxstack;
	stack = stackfloor - 1;
	state.indepth = (s4*) DumpMemory::allocate(sizeof(s4) * state.basicblockcount);
	state.startstack = (verifier_slot_t*) DumpMemory::allocate(sizeof(verifier_slot_t) * state.m->maxstack * state.basicblockcount);

	/* allocate the local variables buffers */

	state.numlocals = state.m->maxlocals;
	state.validlocals = state.m->maxlocals;
    if (state.initmethod)
		state.numlocals++; /* extra marker variable */

	state.locals = (verifier_slot_t*) DumpMemory::allocate(sizeof(verifier_slot_t) * state.numlocals);
	state.startlocals = (verifier_slot_t*) DumpMemory::allocate(sizeof(verifier_slot_t) * state.numlocals * state.basicblockcount);

    /* allocate the buffer of active exception handlers */

    state.handlers = (exception_entry**) DumpMemory::allocate(sizeof(exception_entry*) * (state.jd->exceptiontablelength + 1));

    /* initialize instack of exception handlers */

	exstack.type = TYPE_ADR;
	exstack.typeinfo.init_class(class_java_lang_Throwable); /* changed later */

    OLD_LOG("Exception handler stacks set.\n");

	/* initialize jsr info buffer */

	state.jsrinfos = (typecheck_jsr_t**) DumpMemory::allocate(sizeof(typecheck_jsr_t*) * state.basicblockcount);
	MZERO(state.jsrinfos, typecheck_jsr_t *, state.basicblockcount);

	/* initialize stack of first block */

	state.indepth[0] = 0;

	/* initialize locals of first block */

    /* if this is an instance method initialize the "this" ref type */

    if (!(state.m->flags & ACC_STATIC)) {
		if (state.validlocals < 1)
			VERIFY_ERROR("Not enough local variables for method arguments");
		dst = state.startlocals;
		dst->type = TYPE_ADR;
		if (state.initmethod)
			dst->typeinfo.init_newobject(NULL);
		else
			dst->typeinfo.init_class(state.m->clazz);

		skip = 1;
    }

    OLD_LOG("'this' argument set.\n");

	len = typedescriptors_init_from_methoddesc(state.startlocals + skip,
			state.m->parseddesc,
			state.validlocals, true, skip, &state.returntype);
	if (len < 0)
		return false;

	/* set remaining locals to void */

	for (i = skip + len; i<state.numlocals; ++i)
		state.startlocals[i].type = TYPE_VOID;

	/* initialize block flags */

	typecheck_init_state(&state, basicblock::UNDEF);

	/* iterate until fixpoint reached */

	do {

		state.repeat = false;

		/* iterate over the basic blocks */

		for (state.bptr = state.basicblocks; state.bptr != NULL; state.bptr = state.bptr->next) {

			if (state.bptr->state != basicblock::TYPECHECK_REACHED)
				continue;

			DOLOG( show_basicblock(jd, state.bptr, SHOW_PARSE); );

			/* mark this block as analysed */

			state.bptr->state = basicblock::FINISHED;

			/* determine the active exception handlers for this block */
			/* XXX could use a faster algorithm with sorted lists or  */
			/* something?                                             */
			/* XXX reuse code from variables based verifer? */
			len = 0;
			for (ex = STATE->jd->exceptiontable; ex ; ex = ex->down) {
				if ((ex->start->nr <= STATE->bptr->nr) && (ex->end->nr > STATE->bptr->nr)) {
					OLD_LOG1("\tactive handler L%03d", ex->handler->nr);
					STATE->handlers[len++] = ex;
				}
			}
			STATE->handlers[len] = NULL;

			/* initialize the locals */

			MCOPY(state.locals,
				  state.startlocals + (state.bptr->nr * state.numlocals),
				  verifier_slot_t, state.numlocals);

			/* initialize the stack */

			len = state.indepth[state.bptr->nr];

			MCOPY(stackfloor,
				  state.startstack + (state.bptr->nr * state.m->maxstack),
				  verifier_slot_t, len);

			stack = stackfloor + (len - 1);

			/* iterate over the instructions in this block */

			state.iptr = state.bptr->iinstr;
			len = state.bptr->icount;

			superblockend = false;

			for (; len--; state.iptr++) {

				maythrow = false;

				OLD_LOGNL;
				DOLOG( typecheck_stackbased_show_state(&state, stack, stackfloor, true); );

				switch (state.iptr->opc) {
#define TYPECHECK_STACKBASED 1
#define STATE (&state)
#define IPTR state.iptr
#define BPTR state.bptr
#define METHOD state.m
#define LOCAL_SLOT(index)  (state.locals + (index))
#define EXCEPTION                                                    \
    do {                                                             \
        OLD_LOG("EXCEPTION THROWN!\n");                                  \
        return false;                                                \
    } while (0)

#include <typecheck-stackbased-gen.inc>
#undef  TYPECHECK_STACKBASED
				}

				/* reach exception handlers for this instruction */

				if (maythrow) {
					TYPECHECK_COUNT(stat_ins_maythrow);
					TYPECHECK_MARK(STATE->stat_maythrow);
					OLD_LOG("\treaching exception handlers");
					i = 0;
					while (STATE->handlers[i]) {
						TYPECHECK_COUNT(stat_handlers_reached);
						if (STATE->handlers[i]->catchtype.any)
							exstack.typeinfo.typeclass = STATE->handlers[i]->catchtype;
						else
							exstack.typeinfo.typeclass.cls = class_java_lang_Throwable;
						if (!typecheck_stackbased_reach(
								STATE,
								STATE->handlers[i]->handler,
								&exstack, 1))
							EXCEPTION;
						i++;
					}
				}
			}

			/* propagate types to the following block */

			if (!superblockend) {
				if (!typecheck_stackbased_reach(&state, state.bptr->next,
												stack, stack - stackfloor + 1))
					EXCEPTION;
			}
		} /* end loop over blocks */

		while (!state.repeat && state.topjsr) {
			OLD_LOG1("done analysing subroutine L%03d", state.topjsr->start->nr);

			/* propagate down used locals */

			if (state.topjsr->next) {
				for (i=0; i<state.numlocals; ++i)
					state.topjsr->next->usedlocals[i] |= state.topjsr->usedlocals[i];
			}

			/* restore REACHED flags */

			for (tbptr = state.basicblocks; tbptr != NULL; tbptr = tbptr->next) {
				assert(tbptr->state != basicblock::TYPECHECK_REACHED);
				if (state.topjsr->blockflags[tbptr->nr] == basicblock::TYPECHECK_REACHED) {
					tbptr->state = basicblock::TYPECHECK_REACHED;
					state.repeat = true;
				}
			}

			/* dactivate the subroutine */

			state.topjsr->active = false;
			state.topjsr = state.topjsr->next;
		}
	} while (state.repeat);

	/* reset block flags */

	typecheck_reset_state(&state);

	OLD_LOG("typecheck_stackbased successful");

	return true;

throw_stack_underflow:
	OLD_LOG("STACK UNDERFLOW!");
	exceptions_throw_verifyerror(state.m, "Unable to pop operand off an empty stack");
	return false;

throw_stack_overflow:
	OLD_LOG("STACK OVERFLOW!");
	exceptions_throw_verifyerror(state.m, "Stack size too large");
	return false;

throw_stack_type_error:
	OLD_LOG("STACK TYPE ERROR!");
	exceptions_throw_verifyerror(state.m, "Mismatched stack types");
	return false;

throw_local_type_error:
	OLD_LOG("LOCAL TYPE ERROR!");
	exceptions_throw_verifyerror(state.m, "Local variable type mismatch");
	return false;

throw_stack_category_error:
	OLD_LOG("STACK CATEGORY ERROR!");
	exceptions_throw_verifyerror(state.m, "Attempt to split long or double on the stack");
	return false;
}


#if defined(TYPECHECK_VERBOSE)
static void typecheck_stackbased_show_state(verifier_state *state,
											typedescriptor_t *stack,
											typedescriptor_t *stackfloor,
											bool showins)
{
	typedescriptor_t *sp;
	s4 i;

	OLD_LOGSTR1("stackdepth %d stack [", (stack - stackfloor) + 1);
	for (sp=stackfloor; sp <= stack; sp++) {
		OLD_LOGSTR(" ");
		DOLOG( typedescriptor_print(stdout, sp); );
	}
	OLD_LOGSTR(" ] locals [");
	for (i=0; i<state->numlocals; ++i) {
		OLD_LOGSTR(" ");
		DOLOG( typedescriptor_print(stdout, state->locals + i); );
	}
	OLD_LOGSTR(" ]");
	OLD_LOGNL;
	if (showins && state->iptr < (state->bptr->iinstr + state->bptr->icount)) {
		OLD_LOGSTR("\t");
		DOLOG( show_icmd(state->jd, state->iptr, false, SHOW_PARSE); );
		OLD_LOGNL;
	}
}
#endif


#endif /* defined(ENABLE_VERIFIER) */


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
