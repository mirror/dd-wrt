/* src/vm/jit/verify/typecheck-common.cpp - shared verifier code

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
#include "vm/types.hpp"
#include "vm/global.hpp"

#include <assert.h>

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/method.hpp"

#include "vm/jit/show.hpp"

#include "typecheck-common.hpp"


/****************************************************************************/
/* DEBUG HELPERS                                                            */
/****************************************************************************/

#ifdef TYPECHECK_VERBOSE_OPT
bool opt_typecheckverbose = false;
#endif

#if defined(TYPECHECK_VERBOSE) || defined(TYPECHECK_VERBOSE_IMPORTANT)

void typecheck_print_var(FILE *file, jitdata *jd, s4 index)
{
	varinfo *var;

	assert(index >= 0 && index < jd->varcount);
	var = VAR(index);
	typeinfo_print_type(file, var->type, &(var->typeinfo));
}

void typecheck_print_vararray(FILE *file, jitdata *jd, s4 *vars, int len)
{
	s4 i;

	for (i=0; i<len; ++i) {
		if (i)
			fputc(' ', file);
		typecheck_print_var(file, jd, *vars++);
	}
}

#endif /* defined(TYPECHECK_VERBOSE) || defined(TYPECHECK_VERBOSE_IMPORTANT) */


/****************************************************************************/
/* STATISTICS                                                               */
/****************************************************************************/

#if defined(TYPECHECK_STATISTICS)
#warning port to new statistics framework
int stat_typechecked = 0;
int stat_methods_with_handlers = 0;
int stat_methods_maythrow = 0;
int stat_iterations[STAT_ITERATIONS+1] = { 0 };
int stat_reached = 0;
int stat_copied = 0;
int stat_merged = 0;
int stat_merging_changed = 0;
int stat_blocks[STAT_BLOCKS+1] = { 0 };
int stat_locals[STAT_LOCALS+1] = { 0 };
int stat_ins = 0;
int stat_ins_maythrow = 0;
int stat_ins_stack = 0;
int stat_ins_field = 0;
int stat_ins_field_unresolved = 0;
int stat_ins_field_uninitialized = 0;
int stat_ins_invoke = 0;
int stat_ins_invoke_unresolved = 0;
int stat_ins_primload = 0;
int stat_ins_aload = 0;
int stat_ins_builtin = 0;
int stat_ins_builtin_gen = 0;
int stat_ins_branch = 0;
int stat_ins_switch = 0;
int stat_ins_primitive_return = 0;
int stat_ins_areturn = 0;
int stat_ins_areturn_unresolved = 0;
int stat_ins_athrow = 0;
int stat_ins_athrow_unresolved = 0;
int stat_ins_unchecked = 0;
int stat_handlers_reached = 0;
int stat_savedstack = 0;

static void print_freq(FILE *file,int *array,int limit)
{
	int i;
	for (i=0; i<limit; ++i)
		fprintf(file,"      %3d: %8d\n",i,array[i]);
	fprintf(file,"    >=%3d: %8d\n",limit,array[limit]);
}

void typecheck_print_statistics(FILE *file) {
	fprintf(file,"typechecked methods: %8d\n",stat_typechecked);
	fprintf(file,"    with handler(s): %8d\n",stat_methods_with_handlers);
	fprintf(file,"    with throw(s)  : %8d\n",stat_methods_maythrow);
	fprintf(file,"reached blocks     : %8d\n",stat_reached);
	fprintf(file,"copied states      : %8d\n",stat_copied);
	fprintf(file,"merged states      : %8d\n",stat_merged);
	fprintf(file,"merging changed    : %8d\n",stat_merging_changed);
	fprintf(file,"handlers reached   : %8d\n",stat_handlers_reached);
	fprintf(file,"saved stack (times): %8d\n",stat_savedstack);
	fprintf(file,"instructions       : %8d\n",stat_ins);
	fprintf(file,"    stack          : %8d\n",stat_ins_stack);
	fprintf(file,"    field access   : %8d\n",stat_ins_field);
	fprintf(file,"      (unresolved) : %8d\n",stat_ins_field_unresolved);
	fprintf(file,"      (uninit.)    : %8d\n",stat_ins_field_uninitialized);
	fprintf(file,"    invocations    : %8d\n",stat_ins_invoke);
	fprintf(file,"      (unresolved) : %8d\n",stat_ins_invoke_unresolved);
	fprintf(file,"    load primitive : (currently not counted) %8d\n",stat_ins_primload);
	fprintf(file,"    load address   : %8d\n",stat_ins_aload);
	fprintf(file,"    builtins       : %8d\n",stat_ins_builtin);
	fprintf(file,"        generic    : %8d\n",stat_ins_builtin_gen);
	fprintf(file,"    branches       : %8d\n",stat_ins_branch);
	fprintf(file,"    switches       : %8d\n",stat_ins_switch);
	fprintf(file,"    prim. return   : %8d\n",stat_ins_primitive_return);
	fprintf(file,"    areturn        : %8d\n",stat_ins_areturn);
	fprintf(file,"      (unresolved) : %8d\n",stat_ins_areturn_unresolved);
	fprintf(file,"    athrow         : %8d\n",stat_ins_athrow);
	fprintf(file,"      (unresolved) : %8d\n",stat_ins_athrow_unresolved);
	fprintf(file,"    unchecked      : %8d\n",stat_ins_unchecked);
	fprintf(file,"    maythrow       : %8d\n",stat_ins_maythrow);
	fprintf(file,"iterations used:\n");
	print_freq(file,stat_iterations,STAT_ITERATIONS);
	fprintf(file,"basic blocks per method / 10:\n");
	print_freq(file,stat_blocks,STAT_BLOCKS);
	fprintf(file,"locals:\n");
	print_freq(file,stat_locals,STAT_LOCALS);
}
#endif /* defined(TYPECHECK_STATISTICS) */


/* typecheck_init_state ********************************************************

   Initialize the basic block state for the following CFG traversal.

   IN:
       state............the current state of the verifier
       minflags.........minimum state value of blocks that should be
                        considered

*******************************************************************************/

void typecheck_init_state(verifier_state *state, basicblock::State minstate)
{
	basicblock *block;

    	// set all FINISHED blocks to TYPECHECK_UNDEF

	for (block = state->basicblocks; block; block = block->next) {

#ifdef TYPECHECK_DEBUG
		// check for invalid state
		if (block->state != basicblock::FINISHED &&
		    block->state != basicblock::DELETED  &&
		    block->state != basicblock::UNDEF)
		{
			OLD_LOGSTR1("block state: %d\n",block->state); OLD_LOGFLUSH;
			TYPECHECK_ASSERT(false);
		}
#endif

		if (block->state >= minstate) {
			block->state = basicblock::TYPECHECK_UNDEF;
		}
	}

	// the first block is always reached

	if (state->basicblockcount && state->basicblocks[0].state == basicblock::TYPECHECK_UNDEF)
		state->basicblocks[0].state = basicblock::TYPECHECK_REACHED;
}


/* typecheck_reset_state *******************************************************

   Reset the state of basic blocks we have not reached.

   IN:
       state............the current state of the verifier

*******************************************************************************/

void typecheck_reset_state(verifier_state *state)
{
	// check for invalid state at exit

#ifdef TYPECHECK_DEBUG
	for (basicblock *block = state->basicblocks; block; block = block->next) {
		if (block->state != basicblock::DELETED  &&
		    block->state != basicblock::UNDEF    &&
		    block->state != basicblock::FINISHED &&
		    block->state != basicblock::TYPECHECK_UNDEF) // typecheck may never reach
		                                                 // some exception handlers,
		                                                 // that's ok.
		{
			OLD_LOG2("block L%03d has invalid state after typecheck: %d",
				 block->nr,block->state);
			TYPECHECK_ASSERT(false);
		}
	}
#endif

	// Delete blocks we never reached

	for (basicblock *block = state->basicblocks; block; block = block->next) {
		if (block->state == basicblock::TYPECHECK_UNDEF)
			block->state = basicblock::DELETED;
	}
}


/****************************************************************************/
/* TYPESTACK MACROS AND FUNCTIONS                                           */
/*                                                                          */
/* These macros and functions act on the 'type stack', which is a shorthand */
/* for the types of the stackslots of the current stack. The type of a      */
/* stack slot is usually described by a TYPE_* constant and -- for TYPE_ADR */
/* -- by the typeinfo of the slot. The only thing that makes the type stack */
/* more complicated are returnAddresses of local subroutines, because a     */
/* single stack slot may contain a set of more than one possible return     */
/* address. This is handled by 'return address sets'. A return address set  */
/* is kept as a linked list dangling off the typeinfo of the stack slot.    */
/****************************************************************************/

/* typecheck_copy_types ********************************************************

   Copy the types of the source variables to the destination variables.

   IN:
	   state............current verifier state
	   srcvars..........array of variable indices to copy
	   dstvars..........array of the destination variables
	   n................number of variables to copy

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

bool typecheck_copy_types(verifier_state *state, s4 *srcvars, s4 *dstvars, s4 n)
{
	s4 i;
	varinfo *sv;
	varinfo *dv;
	jitdata *jd = state->jd;

	for (i=0; i < n; ++i, ++srcvars, ++dstvars) {
		sv = VAR(*srcvars);
		dv = VAR(*dstvars);

		dv->type = sv->type;
		if (dv->type == TYPE_ADR) {
			dv->typeinfo = sv->typeinfo;
		}
	}
	return true;
}


/* typecheck_merge_types *******************************************************

   Merge the types of the source variables into the destination variables.

   IN:
       state............current state of the verifier
	   srcvars..........source variable indices
	   dstvars..........destination variable indices
	   n................number of variables

   RETURN VALUE:
       typecheck_TRUE...the destination variables have been modified
	   typecheck_FALSE..the destination variables are unchanged
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

typecheck_result typecheck_merge_types(verifier_state *state,
									   s4 *srcvars,
									   s4 *dstvars,
									   s4 n)
{
	s4 i;
	varinfo *sv;
	varinfo *dv;
	jitdata *jd = state->jd;
	typecheck_result r;
	bool changed = false;

	for (i=0; i < n; ++i, ++srcvars, ++dstvars) {
		sv = VAR(*srcvars);
		dv = VAR(*dstvars);

		if (dv->type != sv->type) {
			exceptions_throw_verifyerror(state->m,"Stack type mismatch");
			return typecheck_FAIL;
		}
		if (dv->type == TYPE_ADR) {
			if (dv->typeinfo.is_primitive()) {
				/* dv has returnAddress type */
				if (!sv->typeinfo.is_primitive()) {
					exceptions_throw_verifyerror(state->m,"Merging returnAddress with reference");
					return typecheck_FAIL;
				}
			}
			else {
				/* dv has reference type */
				if (sv->typeinfo.is_primitive()) {
					exceptions_throw_verifyerror(state->m,"Merging reference with returnAddress");
					return typecheck_FAIL;
				}
				r = dv->typeinfo.merge(state->m, &(sv->typeinfo));
				if (r == typecheck_FAIL)
					return r;
				changed |= r;
			}
		}
	}
	return (typecheck_result) changed;
}


/* typestate_merge *************************************************************

   Merge the types of one state into the destination state.

   IN:
       state............current state of the verifier
	   dstvars..........indices of the destinations invars
	   dstlocals........the destinations inlocals
	   srcvars..........indices of the source's outvars
	   srclocals........the source locals
	   n................number of invars (== number of outvars)

   RETURN VALUE:
       typecheck_TRUE...destination state has been modified
	   typecheck_FALSE..destination state has not been modified
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

typecheck_result typestate_merge(verifier_state *state,
				                 s4 *srcvars, varinfo *srclocals,
				                 s4 *dstvars, varinfo *dstlocals,
				                 s4 n)
{
	bool changed = false;
	typecheck_result r;

	/* The stack is always merged. If there are returnAddresses on
	 * the stack they are ignored in this step. */

	r = typecheck_merge_types(state, srcvars, dstvars, n);
	if (r == typecheck_FAIL)
		return r;
	changed |= r;

	/* merge the locals */

	r = typevector_merge(state->m, dstlocals, srclocals, state->numlocals);
	if (r == typecheck_FAIL)
		return r;
	return (typecheck_result) (changed | r);
}


/* typestate_reach *************************************************************

   Reach a destination block and propagate stack and local variable types

   IN:
       state............current state of the verifier
	   destblock........destination basic block
	   srcvars..........variable indices of the outvars to propagate
	   srclocals........local variables to propagate
	   n................number of srcvars

   OUT:
       state->repeat....set to true if the verifier must iterate again
	                    over the basic blocks

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

bool typestate_reach(verifier_state *state,
					 basicblock *destblock,
					 s4 *srcvars, varinfo *srclocals, s4 n)
{
	varinfo *destloc;
	bool changed = false;
	typecheck_result r;

	OLD_LOG1("reaching block L%03d",destblock->nr);
	TYPECHECK_COUNT(stat_reached);

	destloc = destblock->inlocals;

	if (destblock->state == basicblock::TYPECHECK_UNDEF) {
		/* The destblock has never been reached before */

		TYPECHECK_COUNT(stat_copied);
		OLD_LOG1("block L%03d reached first time",destblock->nr);

		if (!typecheck_copy_types(state, srcvars, destblock->invars, n))
			return false;
		typevector_copy_inplace(srclocals, destloc, state->numlocals);
		changed = true;
	}
	else {
		/* The destblock has already been reached before */

		TYPECHECK_COUNT(stat_merged);
		OLD_LOG1("block L%03d reached before", destblock->nr);

		r = typestate_merge(state, srcvars, srclocals,
				destblock->invars, destblock->inlocals, n);
		if (r == typecheck_FAIL)
			return false;
		changed = r;
		TYPECHECK_COUNTIF(changed,stat_merging_changed);
	}

	if (changed) {
		OLD_LOG("changed!");
		destblock->state = basicblock::TYPECHECK_REACHED;
		if (destblock->nr <= state->bptr->nr) {
			OLD_LOG("REPEAT!");
			state->repeat = true;
		}
	}
	return true;
}


/* typecheck_init_locals *******************************************************

   Initialize the local variables in the verifier state.

   IN:
       state............the current state of the verifier
	   newthis..........if true, mark the instance in <init> methods as
	                    uninitialized object.

   RETURN VALUE:
       true.............success,
	   false............an exception has been thrown.

*******************************************************************************/

bool typecheck_init_locals(verifier_state *state, bool newthis)
{
	int i;
	int varindex;
	varinfo *locals;
	varinfo *v;
	jitdata *jd = state->jd;
	int skip = 0;

	locals = state->basicblocks[0].inlocals;

	/* allocate parameter descriptors if necessary */

	state->m->parseddesc->params_from_paramtypes(state->m->flags);

	/* pre-initialize variables as TYPE_VOID */

	i = state->numlocals;
	v = locals;
	while (i--) {
		v->type = TYPE_VOID;
		v++;
	}

    /* if this is an instance method initialize the "this" ref type */

    if (!(state->m->flags & ACC_STATIC)) {
		varindex = jd->local_map[5*0 + TYPE_ADR];
		if (varindex != jitdata::UNUSED) {
			if (state->validlocals < 1)
				TYPECHECK_VERIFYERROR_bool("Not enough local variables for method arguments");
			v = locals + varindex;
			v->type = TYPE_ADR;
			if (state->initmethod && newthis)
				v->typeinfo.init_newobject(NULL);
			else
				v->typeinfo.init_class(state->m->clazz);
		}

		skip = 1;
    }

    OLD_LOG("'this' argument set.\n");

    /* the rest of the arguments and the return type */

    if (!typeinfo_init_varinfos_from_methoddesc(locals, state->m->parseddesc,
											  state->validlocals,
											  skip, /* skip 'this' pointer */
											  jd->local_map,
											  &state->returntype))
		return false;

    OLD_LOG("Arguments set.\n");
	return true;
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
