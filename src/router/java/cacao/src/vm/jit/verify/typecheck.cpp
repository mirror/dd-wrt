/* src/vm/jit/verify/typecheck.cpp - typechecking (part of bytecode verification)

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

/**
 * @defgroup verify Bytecode Verification
 * Bytecode Verification
 */

/**
@file
@ingroup verify

Typechecker

What's the purpose of the `typechecker`?
----------------------------------------

The typechecker analyses (the intermediate repr. of) the bytecode of
each method and ensures that for each instruction the values on the
stack and in local variables are of the correct type whenever the
instruction is executed.

type checking is a mandatory part of bytecode verification.


How does the typechecker work?
------------------------------

The JVM stack and the local variables are not statically typed, so the
typechecker has to *infer* the static types of stack slots and local
variables at each point of the method. The JVM spec imposes a lot of
restrictions on the bytecode in order to guarantee that this is always
possible.

Basically the typechecker solves the data flow equations of the method.
This is done in the usual way for a forward data flow analysis: Starting
from the entry point of the method the typechecker follows the CFG and
records the type of each stack slot and local variable at each point @fn{1}.
When two or more control flow paths merge at a point, the union of the
types for each slot/variable is taken. The algorithm continues to follow
all possible paths @fn{2} until the recorded types do not change anymore (ie.
the equations have been solved).

If the solution has been reached and the resulting types are valid for
all instructions, then type checking terminates with success, otherwise
an exception is thrown.


Why is this code so damn complicated?
-------------------------------------

Short answer: The devil's in the details.

While the basic operation of the typechecker is no big deal, there are
many properties of Java bytecode which make type checking hard. Some of
them are not even addressed in the JVM spec. Some problems and their
solutions:

- Finding a good representation of the union of two reference types is
difficult because of multiple inheritance of interfaces. 

	Solution: The typeinfo system can represent such "merged" types by a
	list of proper subclasses of a class. Example:

		typeclass=java.lang.Object merged={ InterfaceA, InterfaceB }
	
	represents the result of merging two interface types "InterfaceA"
	and "InterfaceB".

- When the code of a method is verified, there may still be unresolved
references to classes/methods/fields in the code, which we may not force
to be resolved eagerly. (A similar problem arises because of the special
checks for protected members.)@fn{3}

	Solution: The typeinfo system knows how to deal with unresolved
	class references. Whenever a check has to be performed for an
	unresolved type, the type is annotated with constraints representing
	the check. Later, when the type is resolved, the constraints are
	checked. (See the constrain_unresolved_... and the resolve_...
	methods.)

- Checks for uninitialized object instances are hard because after the
invocation of <init> on an uninitialized object *all* slots/variables
referring to this object (and exactly those slots/variables) must be
marked as initialized.

	Solution: The JVM spec describes a solution, which has been
	implemented in this typechecker.

Note that some checks mentioned in the JVM spec are unnecessary @fn{4} and
not performed by either the reference implementation, or this implementation.


--- Footnotes

@footnote{1} Actually only the types of slots/variables at the start of each
basic block are remembered. Within a basic block the algorithm only keeps
the types of the slots/variables for the "current" instruction which is
being analysed. 

@footnote{2} Actually the algorithm iterates through the basic block list until
there are no more changes. Theoretically it would be wise to sort the
basic blocks topologically beforehand, but the number of average/max
iterations observed is so low, that this was not deemed necessary.

@footnote{3} This is similar to a method proposed by: Alessandro Coglio et al.,
"A Formal Specification of Java Class Loading" @cite Coglio2000.
An important difference is that Coglio's subtype constraints are checked
after loading, while our constraints are checked when the field/method
is accessed for the first time, so we can guarantee lexically correct
error reporting.

@footnote{4} Alessandro Coglio "Improving the official specification of Java
bytecode verification" @cite Coglio2003.
*/


#include "config.h"

#include <assert.h>
#include <string.h>

#include "vm/types.hpp"

#ifdef ENABLE_VERIFIER

#include "mm/memory.hpp"
#include "mm/dumpmemory.hpp"

#include "native/native.hpp"

#include "toolbox/logging.hpp"

#include "vm/access.hpp"
#include "vm/array.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/options.hpp"
#include "vm/primitive.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/show.hpp"

#include "vm/jit/ir/instruction.hpp"

#include "vm/jit/verify/typecheck-common.hpp"


/****************************************************************************/
/* MACROS FOR VARIABLE TYPE CHECKING                                        */
/****************************************************************************/

#define TYPECHECK_CHECK_TYPE(i,tp,msg)                               \
    do {                                                             \
        if (VAR(i)->type != (tp)) {                                  \
            exceptions_throw_verifyerror(state->m, (msg));           \
            return false;                                            \
        }                                                            \
    } while (0)

#define TYPECHECK_INT(i)                                             \
    TYPECHECK_CHECK_TYPE(i,TYPE_INT,"Expected to find integer value")
#define TYPECHECK_LNG(i)                                             \
    TYPECHECK_CHECK_TYPE(i,TYPE_LNG,"Expected to find long value")
#define TYPECHECK_FLT(i)                                             \
    TYPECHECK_CHECK_TYPE(i,TYPE_FLT,"Expected to find float value")
#define TYPECHECK_DBL(i)                                             \
    TYPECHECK_CHECK_TYPE(i,TYPE_DBL,"Expected to find double value")
#define TYPECHECK_ADR(i)                                             \
    TYPECHECK_CHECK_TYPE(i,TYPE_ADR,"Expected to find object value")

#define TYPECHECK_INT_OP(o)  TYPECHECK_INT((o).varindex)
#define TYPECHECK_LNG_OP(o)  TYPECHECK_LNG((o).varindex)
#define TYPECHECK_FLT_OP(o)  TYPECHECK_FLT((o).varindex)
#define TYPECHECK_DBL_OP(o)  TYPECHECK_DBL((o).varindex)
#define TYPECHECK_ADR_OP(o)  TYPECHECK_ADR((o).varindex)


/* typestate_save_invars *******************************************************
 
   Save the invars of the current basic block in the space reserved by
   parse.

   This function must be called before an instruction modifies a variable
   that is an invar of the current block. In such cases the invars of the
   block must be saved, and restored at the end of the analysis of this
   basic block, so that the invars again reflect the *input* to this basic
   block (and do not randomly contain types that appear within the block).

   IN:
       state............current state of the verifier

*******************************************************************************/

static void
typestate_save_invars(verifier_state *state)
{
	s4 i, index;
	s4 *pindex;
	
	OLD_LOG("saving invars");

	if (!state->savedindices) {
		OLD_LOG("allocating savedindices buffer");
		pindex = (s4*) DumpMemory::allocate(sizeof(s4) * state->m->maxstack);
		state->savedindices = pindex;
		index = state->numlocals + VERIFIER_EXTRA_VARS;
		for (i=0; i<state->m->maxstack; ++i)
			*pindex++ = index++;
	}

	/* save types */

	typecheck_copy_types(state, state->bptr->invars, state->savedindices, 
			state->bptr->indepth);

	/* set the invars of the block to the saved variables */
	/* and remember the original invars                   */

	state->savedinvars = state->bptr->invars;
	state->bptr->invars = state->savedindices;
}


/* typestate_restore_invars  ***************************************************
 
   Restore the invars of the current basic block that have been previously
   saved by `typestate_save_invars`.

   IN:
       state............current state of the verifier

*******************************************************************************/

static void
typestate_restore_invars(verifier_state *state)
{
	TYPECHECK_COUNT(stat_savedstack);
	OLD_LOG("restoring saved invars");

	/* restore the invars pointer */

	state->bptr->invars = state->savedinvars;

	/* copy the types back */

	typecheck_copy_types(state, state->savedindices, state->bptr->invars,
			state->bptr->indepth);

	/* mark that there are no saved invars currently */

	state->savedinvars = NULL;
}


/* handle_fieldaccess **********************************************************
 
   Verify an ICMD_{GET,PUT}{STATIC,FIELD}(CONST)?
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_fieldaccess(verifier_state *state,
				   varinfo *instance,
				   varinfo *value)
{
	jitdata *jd;

	jd = state->jd;

#define TYPECHECK_VARIABLESBASED
#define EXCEPTION  do { return false; } while (0)
#define VERIFY_ERROR(msg)  TYPECHECK_VERIFYERROR_bool(msg)
#include <typecheck-fields.inc>
#undef  EXCEPTION
#undef  VERIFY_ERROR
#undef  TYPECHECK_VARIABLESBASED

	return true;
}


/* handle_invocation ***********************************************************
 
   Verify an ICMD_INVOKE* instruction.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_invocation(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_VARIABLESBASED
#define OP1   VAR(state->iptr->sx.s23.s2.args[0])
#include <typecheck-invoke.inc>
#undef  OP1
#undef  TYPECHECK_VARIABLESBASED

	return true;
}


/* handle_builtin **************************************************************
 
   Verify the call of a builtin method.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_builtin(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_VARIABLESBASED
#define OP1   state->iptr->sx.s23.s2.args[0]
#include <typecheck-builtins.inc>
#undef  OP1
#undef  TYPECHECK_VARIABLESBASED

	return true;
}

/* handle_multianewarray *******************************************************
 
   Verify a MULTIANEWARRAY instruction.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_multianewarray(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_VARIABLESBASED
#define VERIFY_ERROR(msg)  TYPECHECK_VERIFYERROR_bool(msg)
#include <typecheck-multianewarray.inc>
#undef VERIFY_ERROR
#undef  TYPECHECK_VARIABLESBASED

	return true;
}

/* typecheck_invalidate_locals *************************************************
 
   Invalidate locals that are overwritten by writing to the given local.
  
   IN:
       state............the current state of the verifier
	   index............the index of the local that is written
	   twoword..........true, if a two-word type is written

*******************************************************************************/

static void typecheck_invalidate_locals(verifier_state *state, s4 index, bool twoword)
{
	s4 javaindex;
	s4 t;
	s4 varindex;
	jitdata *jd = state->jd;
	s4 *localmap = jd->local_map;
	varinfo *vars = jd->var;

	javaindex = jd->reverselocalmap[index];

	/* invalidate locals of two-word type at index javaindex-1 */

	if (javaindex > 0) {
		localmap += 5 * (javaindex-1);
		for (t=0; t<5; ++t) {
			varindex = *localmap++;
			if (varindex >= 0 && IS_2_WORD_TYPE(vars[varindex].type)) {
				OLD_LOG1("invalidate local %d", varindex);
				vars[varindex].type = TYPE_VOID;
			}
		}
	}
	else {
		localmap += 5 * javaindex;
	}

	/* invalidate locals at index javaindex */

	for (t=0; t<5; ++t) {
		varindex = *localmap++;
		if (varindex >= 0) {
			OLD_LOG1("invalidate local %d", varindex);
			vars[varindex].type = TYPE_VOID;
		}
	}

	/* if a two-word type is written, invalidate locals at index javaindex+1 */

	if (twoword) {
		for (t=0; t<5; ++t) {
			varindex = *localmap++;
			if (varindex >= 0) {
				OLD_LOG1("invalidate local %d", varindex);
				vars[varindex].type = TYPE_VOID;
			}
		}
	}
}


/* macros used by the generated code ******************************************/

#define EXCEPTION          do { return false; } while (0)
#define VERIFY_ERROR(msg)  TYPECHECK_VERIFYERROR_bool(msg)

#define CHECK_LOCAL_TYPE(index, t)                                   \
    do {                                                             \
        if (!typevector_checktype(jd->var, (index), (t)))            \
             VERIFY_ERROR("Local variable type mismatch");           \
    } while (0)

#define STORE_LOCAL(t, index)                                        \
    do {                                                             \
         Type temp_t = (t);                                          \
         typecheck_invalidate_locals(state, (index), false);         \
         typevector_store(jd->var, (index), (temp_t), NULL);         \
    } while (0)

#define STORE_LOCAL_2_WORD(t, index)                                 \
    do {                                                             \
         Type temp_t = (t);                                          \
         typecheck_invalidate_locals(state, (index), true);          \
         typevector_store(jd->var, (index), (temp_t), NULL);         \
    } while (0)

#define REACH_BLOCK(target)                                          \
    do {                                                             \
        if (!typestate_reach(state, (target),                        \
                             state->bptr->outvars, jd->var,          \
                             state->bptr->outdepth))                 \
                return false;                                        \
    } while (0)

#define REACH(target)   REACH_BLOCK((target).block)


/* handle_basic_block **********************************************************
 
   Perform bytecode verification of a basic block.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_basic_block(verifier_state *state)
{
    int opcode;                                      /* current opcode */
    int len;                        /* for counting instructions, etc. */
    bool superblockend;        /* true if no fallthrough to next block */
	instruction *iptr;                      /* the current instruction */
    basicblock *tbptr;                   /* temporary for target block */
    bool maythrow;               /* true if this instruction may throw */
	s4 i;
	typecheck_result r;
	branch_target_t *table;
	lookup_target_t *lookup;
	jitdata *jd = state->jd;
	exception_entry *ex;
	varinfo constvalue;                               /* for PUT*CONST */
	constant_FMIref *fieldref;

	OLD_LOGSTR1("\n---- BLOCK %04d ------------------------------------------------\n",state->bptr->nr);
	OLD_LOGFLUSH;

	superblockend      = false;
	state->bptr->state = basicblock::FINISHED;

	/* prevent compiler warnings */


	/* determine the active exception handlers for this block */
	/* XXX could use a faster algorithm with sorted lists or  */
	/* something?                                             */
	len = 0;
	for (ex = state->jd->exceptiontable; ex ; ex = ex->down) {
		if ((ex->start->nr <= state->bptr->nr) && (ex->end->nr > state->bptr->nr)) {
			OLD_LOG1("active handler L%03d", ex->handler->nr);
			state->handlers[len++] = ex;
		}
	}
	state->handlers[len] = NULL;

	/* init variable types at the start of this block */
	typevector_copy_inplace(state->bptr->inlocals, jd->var, state->numlocals);

	DOLOG(show_basicblock(jd, state->bptr, SHOW_STACK));
	DOLOG(typecheck_print_vararray(stdout, jd, state->bptr->invars, 
				state->bptr->indepth));
	DOLOG(typevector_print(stdout, jd->var, state->numlocals));
	OLD_LOGNL; OLD_LOGFLUSH;

	/* loop over the instructions */
	len = state->bptr->icount;
	state->iptr = state->bptr->iinstr;
	while (--len >= 0)  {
		TYPECHECK_COUNT(stat_ins);

		iptr = state->iptr;

		DOLOG(typevector_print(stdout, jd->var, state->numlocals));
		OLD_LOGNL; OLD_LOGFLUSH;
		DOLOG(show_icmd(jd, state->iptr, false, SHOW_STACK)); OLD_LOGNL; OLD_LOGFLUSH;

		opcode = iptr->opc;
		maythrow = false;

		switch (opcode) {

			/* include generated code for ICMDs verification */

#define TYPECHECK_VARIABLESBASED
#define STATE  state
#define METHOD (state->m)
#define IPTR   iptr
#define BPTR   (state->bptr)
#include <typecheck-variablesbased-gen.inc>
#undef  STATE
#undef  METHOD
#undef  IPTR
#undef  BPTR
#undef  TYPECHECK_VARIABLESBASED

			default:
				OLD_LOG1("ICMD %d\n", opcode);
				TYPECHECK_VERIFYERROR_bool("Missing ICMD code during typecheck");
		}

		/* reach exception handlers for this instruction */

		if (maythrow) {
			TYPECHECK_COUNT(stat_ins_maythrow);
			TYPECHECK_MARK(state->stat_maythrow);
			OLD_LOG("reaching exception handlers");
			i = 0;
			while (state->handlers[i]) {
				TYPECHECK_COUNT(stat_handlers_reached);
				if (state->handlers[i]->catchtype.any)
					VAR(state->exinvars)->typeinfo.typeclass = state->handlers[i]->catchtype;
				else
					VAR(state->exinvars)->typeinfo.typeclass.cls = class_java_lang_Throwable;
				if (!typestate_reach(state,
						state->handlers[i]->handler,
						&(state->exinvars), jd->var, 1))
					return false;
				i++;
			}
		}

		OLD_LOG("\t\tnext instruction");
		state->iptr++;
	} /* while instructions */

	OLD_LOG("instructions done");
	OLD_LOGSTR("RESULT=> ");
	DOLOG(typecheck_print_vararray(stdout, jd, state->bptr->outvars,
				state->bptr->outdepth));
	DOLOG(typevector_print(stdout, jd->var, state->numlocals));
	OLD_LOGNL; OLD_LOGFLUSH;

	/* propagate stack and variables to the following block */
	if (!superblockend) {
		OLD_LOG("reaching following block");
		tbptr = state->bptr->next;
		while (tbptr->state == basicblock::DELETED) {
			tbptr = tbptr->next;
#ifdef TYPECHECK_DEBUG
			/* this must be checked in parse.c */
			if ((tbptr->nr) >= state->basicblockcount)
				TYPECHECK_VERIFYERROR_bool("Control flow falls off the last block");
#endif
		}
		if (!typestate_reach(state,tbptr,state->bptr->outvars, jd->var,
					state->bptr->outdepth))
			return false;
	}

	/* We may have to restore the types of the instack slots. They
	 * have been saved if an <init> call inside the block has
	 * modified the instack types. (see INVOKESPECIAL) */

	if (state->savedinvars)
		typestate_restore_invars(state);

	return true;
}


/****************************************************************************/
/* typecheck()                                                              */
/* This is the main function of the bytecode verifier. It is called         */
/* directly after analyse_stack.                                            */
/*                                                                          */
/* IN:                                                                      */
/*    meth.............the method to verify                                 */
/*    cdata............codegendata for the method                           */
/*    rdata............registerdata for the method                          */
/*                                                                          */
/* RETURN VALUE:                                                            */
/*     true.............successful verification                             */
/*     false............an exception has been thrown                        */
/*                                                                          */
/****************************************************************************/

#define MAXPARAMS 255

bool typecheck(jitdata *jd)
{
	methodinfo     *meth;
	codegendata    *cd;
	varinfo        *savedlocals;
	verifier_state  state;             /* current state of the verifier */

	/* collect statistics */

#ifdef TYPECHECK_STATISTICS
	int count_iterations = 0;
	TYPECHECK_COUNT(stat_typechecked);
	TYPECHECK_COUNT_FREQ(stat_locals,jd->maxlocals,STAT_LOCALS);
	TYPECHECK_COUNT_FREQ(stat_blocks,cdata->method->basicblockcount/10,STAT_BLOCKS);
	TYPECHECK_COUNTIF(cdata->method->exceptiontablelength != 0,stat_methods_with_handlers);
	state.stat_maythrow = false;
#endif

	/* get required compiler data */

	meth = jd->m;
	cd   = jd->cd;

	/* some logging on entry */


    OLD_LOGSTR("\n==============================================================================\n");
    DOLOG( show_method(jd, SHOW_STACK) );
    OLD_LOGSTR("\n==============================================================================\n");
    OLD_LOGMETHOD("Entering typecheck: ",cd->method);

	/* initialize the verifier state */

	state.m = meth;
	state.jd = jd;
	state.cd = cd;
	state.basicblockcount = jd->basicblockcount;
	state.basicblocks = jd->basicblocks;
	state.savedindices = NULL;
	state.savedinvars = NULL;

	/* check that the basicblock numbers are valid */

#if !defined(NDEBUG)
	jit_check_basicblock_numbers(jd);
#endif

	/* check if this method is an instance initializer method */

    state.initmethod = (state.m->name == utf8::init);

	/* initialize the basic block flags for the following CFG traversal */

	typecheck_init_state(&state, basicblock::FINISHED);

    /* number of local variables */

    /* In <init> methods we use an extra local variable to indicate whether */
    /* the 'this' reference has been initialized.                           */
	/*         TYPE_VOID...means 'this' has not been initialized,           */
	/*         TYPE_INT....means 'this' has been initialized.               */

    state.numlocals = state.jd->localcount;
	state.validlocals = state.numlocals;
    if (state.initmethod)
		state.numlocals++; /* VERIFIER_EXTRA_LOCALS */

	DOLOG(
		s4 i;
		s4 t;
		OLD_LOG("reverselocalmap:");
		for (i=0; i<state.validlocals; ++i) {
			OLD_LOG2("    %i => javaindex %i", i, jd->reverselocalmap[i]);
		});

    /* allocate the buffer of active exception handlers */

    state.handlers = (exception_entry**) DumpMemory::allocate(sizeof(exception_entry*) * (state.jd->exceptiontablelength + 1));

	/* save local variables */

	savedlocals = (varinfo*) DumpMemory::allocate(sizeof(varinfo) * state.numlocals);
	MCOPY(savedlocals, jd->var, varinfo, state.numlocals);

	/* initialized local variables of first block */

	if (!typecheck_init_locals(&state, true))
		return false;

    /* initialize invars of exception handlers */

	state.exinvars = state.numlocals;
	VAR(state.exinvars)->type = TYPE_ADR;
	VAR(state.exinvars)->typeinfo.init_class(class_java_lang_Throwable); /* changed later */

	OLD_LOG("Exception handler stacks set.\n");

	// loop while there are still blocks to be checked
	do {
		TYPECHECK_COUNT(count_iterations);

		state.repeat = false;
		state.bptr   = state.basicblocks;

		for (; state.bptr; state.bptr = state.bptr->next) {
			OLD_LOGSTR1("---- BLOCK %04d, ",state.bptr->nr);
			OLD_LOGSTR1("blockflags: %d\n",state.bptr->state);
			OLD_LOGFLUSH;

			// verify reached block
			if (state.bptr->state == basicblock::TYPECHECK_REACHED) {
				if (!handle_basic_block(&state))
					return false;
			}
		} // for blocks

		OLD_LOGIF(state.repeat,"state.repeat == true");
	} while (state.repeat);

	/* statistics */

#ifdef TYPECHECK_STATISTICS
	OLD_LOG1("Typechecker did %4d iterations",count_iterations);
	TYPECHECK_COUNT_FREQ(stat_iterations,count_iterations,STAT_ITERATIONS);
	TYPECHECK_COUNTIF(state.jsrencountered,stat_typechecked_jsr);
	TYPECHECK_COUNTIF(state.stat_maythrow,stat_methods_maythrow);
#endif

	/* reset the flags of blocks we haven't reached */

	typecheck_reset_state(&state);

	/* restore locals */

	MCOPY(jd->var, savedlocals, varinfo, state.numlocals);

	/* everything's ok */

    OLD_LOGimp("exiting typecheck");
	return true;
}

#endif /* ENABLE_VERIFIER */

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
