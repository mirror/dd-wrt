/* src/vm/jit/codegen-common.cpp - architecture independent code generator stuff

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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

   All functions assume the following code area / data area layout:

   +-----------+
   |           |
   | code area | code area grows to higher addresses
   |           |
   +-----------+ <-- start of procedure
   |           |
   | data area | data area grows to lower addresses
   |           |
   +-----------+

   The functions first write into a temporary code/data area allocated by
   "codegen_init". "codegen_finish" copies the code and data area into permanent
   memory. All functions writing values into the data area return the offset
   relative the begin of the code area (start of procedure).	

*/


#include "config.h"

#include <cassert>
#include <cstring>

#include "vm/types.hpp"

#include "codegen.hpp"
#include "md.hpp"
#include "md-abi.hpp"

#include "mm/codememory.hpp"
#include "mm/memory.hpp"

#include "toolbox/avl.hpp"
#include "toolbox/list.hpp"
#include "toolbox/logging.hpp"

#include "native/llni.hpp"
#include "native/localref.hpp"
#include "native/native.hpp"

#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/options.hpp"
#include "vm/statistics.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/disass.hpp"
#include "vm/jit/exceptiontable.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/methodtree.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/replace.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trace.hpp"

#include "vm/jit/optimizing/profile.hpp"

#if defined(ENABLE_SSA)
# include "vm/jit/optimizing/lsra.hpp"
# include "vm/jit/optimizing/ssa.hpp"
#elif defined(ENABLE_LSRA)
# include "vm/jit/allocator/lsra.hpp"
#endif

#if defined(ENABLE_INTRP)
#include "vm/jit/intrp/intrp.h"
#endif


STAT_REGISTER_VAR(int,count_branches_unresolved,0,"unresolved branches","unresolved branches")
STAT_DECLARE_GROUP(function_call_stat)
STAT_REGISTER_GROUP_VAR(u8,count_calls_java_to_native,0,"calls java to native","java-to-native calls",function_call_stat)

STAT_REGISTER_GROUP(memory_stat,"mem. stat.","Memory usage")
STAT_REGISTER_SUM_SUBGROUP(code_data_stat,"code data","Code and data usage",memory_stat)
STAT_REGISTER_GROUP_VAR(int,count_code_len,0,"code len","code length",code_data_stat)
STAT_REGISTER_GROUP_VAR(int,count_data_len,0,"data len","data length",code_data_stat)

struct methodinfo;

using namespace cacao;


/* codegen_init ****************************************************************

   TODO

*******************************************************************************/

void codegen_init(void)
{
}


/* codegen_setup ***************************************************************

   Allocates and initialises code area, data area and references.

*******************************************************************************/

void codegen_setup(jitdata *jd)
{
	//methodinfo  *m;
	codegendata *cd;

	/* get required compiler data */

	//m  = jd->m;
	cd = jd->cd;

	/* initialize members */

	// Set flags as requested.
	if (opt_AlwaysEmitLongBranches) {
		cd->flags = CODEGENDATA_FLAG_LONGBRANCHES;
	}
	else {
		cd->flags = 0;
	}

	cd->mcodebase    = (u1*) DumpMemory::allocate(MCODEINITSIZE);
	cd->mcodeend     = cd->mcodebase + MCODEINITSIZE;
	cd->mcodesize    = MCODEINITSIZE;

	/* initialize mcode variables */

	cd->mcodeptr     = cd->mcodebase;
	cd->lastmcodeptr = cd->mcodebase;

#if defined(ENABLE_INTRP)
	/* native dynamic superinstructions variables */

	if (opt_intrp) {
		cd->ncodebase = (u1*) DumpMemory::allocate(NCODEINITSIZE);
		cd->ncodesize = NCODEINITSIZE;

		/* initialize ncode variables */
	
		cd->ncodeptr = cd->ncodebase;

		cd->lastinstwithoutdispatch = ~0; /* no inst without dispatch */
		cd->superstarts = NULL;
	}
#endif

	cd->dseg           = NULL;
	cd->dseglen        = 0;

	cd->jumpreferences = NULL;

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
	cd->datareferences = NULL;
#endif

	cd->brancheslabel  = new DumpList<branch_label_ref_t*>();
	cd->linenumbers    = new DumpList<Linenumber>();
}


/* codegen_reset ***************************************************************

   Resets the codegen data structure so we can recompile the method.

*******************************************************************************/

static void codegen_reset(jitdata *jd)
{
	codeinfo    *code;
	codegendata *cd;
	basicblock  *bptr;

	/* get required compiler data */

	code = jd->code;
	cd   = jd->cd;

	/* reset error flag */

	cd->flags          &= ~CODEGENDATA_FLAG_ERROR;

	/* reset some members, we reuse the code memory already allocated
	   as this should have almost the correct size */

	cd->mcodeptr        = cd->mcodebase;
	cd->lastmcodeptr    = cd->mcodebase;

	cd->dseg            = NULL;
	cd->dseglen         = 0;

	cd->jumpreferences  = NULL;

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
	cd->datareferences  = NULL;
#endif

	cd->brancheslabel   = new DumpList<branch_label_ref_t*>();
	cd->linenumbers     = new DumpList<Linenumber>();
	
	/* We need to clear the mpc and the branch references from all
	   basic blocks as they will definitely change. */

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {
		bptr->mpc        = -1;
		bptr->branchrefs = NULL;
	}

	/* We need to clear all the patcher references from the codeinfo
	   since they all will be regenerated */

	patcher_list_reset(code);

#if defined(ENABLE_REPLACEMENT)
	code->rplpoints     = NULL;
	code->rplpointcount = 0;
	code->regalloc      = NULL;
	code->regalloccount = 0;
	code->globalcount   = 0;
#endif
}


/* codegen_generate ************************************************************

   Generates the code for the currently compiled method.

*******************************************************************************/

bool codegen_generate(jitdata *jd)
{
	codegendata *cd;

	/* get required compiler data */

	cd = jd->cd;

	/* call the machine-dependent code generation function */

	if (!codegen_emit(jd))
		return false;

	/* check for an error */

	if (CODEGENDATA_HAS_FLAG_ERROR(cd)) {
		/* check for long-branches flag, if it is set we recompile the
		   method */

#if !defined(NDEBUG)
        if (compileverbose)
            log_message_method("Re-generating code: ", jd->m);
#endif

		/* XXX maybe we should tag long-branches-methods for recompilation */

		if (CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
			/* we have to reset the codegendata structure first */

			codegen_reset(jd);

			/* and restart the compiler run */

			if (!codegen_emit(jd))
				return false;
		}
		else {
			os::abort("codegen_generate: unknown error occurred during codegen_emit: flags=%x\n", cd->flags);
		}

#if !defined(NDEBUG)
        if (compileverbose)
            log_message_method("Re-generating code done: ", jd->m);
#endif
	}

	/* reallocate the memory and finish the code generation */

	codegen_finish(jd);

	/* everything's ok */

	return true;
}


/* codegen_close ***************************************************************

   TODO

*******************************************************************************/

void codegen_close(void)
{
	/* TODO: release avl tree on i386 and x86_64 */
}


/* codegen_increase ************************************************************

   Doubles code area.

*******************************************************************************/

void codegen_increase(codegendata *cd)
{
	u1 *oldmcodebase;

	/* save old mcodebase pointer */

	oldmcodebase = cd->mcodebase;

	/* reallocate to new, doubled memory */

	cd->mcodebase = (u1*) DumpMemory::reallocate(cd->mcodebase,
												 cd->mcodesize,
												 cd->mcodesize * 2);
	cd->mcodesize *= 2;
	cd->mcodeend   = cd->mcodebase + cd->mcodesize;

	/* set new mcodeptr */

	cd->mcodeptr = cd->mcodebase + (cd->mcodeptr - oldmcodebase);

#if defined(__I386__) || defined(__MIPS__) || defined(__X86_64__) || defined(ENABLE_INTRP) \
 || defined(__SPARC_64__)
	/* adjust the pointer to the last patcher position */

	if (cd->lastmcodeptr != NULL)
		cd->lastmcodeptr = cd->mcodebase + (cd->lastmcodeptr - oldmcodebase);
#endif
}


/* codegen_ncode_increase ******************************************************

   Doubles code area.

*******************************************************************************/

#if defined(ENABLE_INTRP)
u1 *codegen_ncode_increase(codegendata *cd, u1 *ncodeptr)
{
	u1 *oldncodebase;

	/* save old ncodebase pointer */

	oldncodebase = cd->ncodebase;

	/* reallocate to new, doubled memory */

	cd->ncodebase = DMREALLOC(cd->ncodebase,
							  u1,
							  cd->ncodesize,
							  cd->ncodesize * 2);
	cd->ncodesize *= 2;

	/* return the new ncodeptr */

	return (cd->ncodebase + (ncodeptr - oldncodebase));
}
#endif


/* codegen_add_branch_ref ******************************************************

   Prepends an branch to the list.

*******************************************************************************/

void codegen_add_branch_ref(codegendata *cd, basicblock *target, s4 condition, s4 reg, u4 options)
{
	branchref *br;
	s4         branchmpc;

	STATISTICS(count_branches_unresolved++);

	/* calculate the mpc of the branch instruction */

	branchmpc = cd->mcodeptr - cd->mcodebase;

	br = (branchref*) DumpMemory::allocate(sizeof(branchref));

	br->branchmpc = branchmpc;
	br->condition = condition;
	br->reg       = reg;
	br->options   = options;
	br->next      = target->branchrefs;

	target->branchrefs = br;
}


/* codegen_resolve_branchrefs **************************************************

   Resolves and patches the branch references of a given basic block.

*******************************************************************************/

void codegen_resolve_branchrefs(codegendata *cd, basicblock *bptr)
{
	branchref *br;
	u1        *mcodeptr;

	/* Save the mcodeptr because in the branch emitting functions
	   we generate code somewhere inside already generated code,
	   but we're still in the actual code generation phase. */

	mcodeptr = cd->mcodeptr;

	/* just to make sure */

	assert(bptr->mpc >= 0);

	for (br = bptr->branchrefs; br != NULL; br = br->next) {
		/* temporary set the mcodeptr */

		cd->mcodeptr = cd->mcodebase + br->branchmpc;

		/* emit_bccz and emit_branch emit the correct code, even if we
		   pass condition == BRANCH_UNCONDITIONAL or reg == -1. */

		emit_bccz(cd, bptr, br->condition, br->reg, br->options);
	}

	/* restore mcodeptr */

	cd->mcodeptr = mcodeptr;
}


/* codegen_branch_label_add ****************************************************

   Append an branch to the label-branch list.

*******************************************************************************/

void codegen_branch_label_add(codegendata *cd, s4 label, s4 condition, s4 reg, u4 options)
{
	// Calculate the current mpc.
	int32_t mpc = cd->mcodeptr - cd->mcodebase;

	branch_label_ref_t* br = (branch_label_ref_t*) DumpMemory::allocate(sizeof(branch_label_ref_t));

	br->mpc       = mpc;
	br->label     = label;
	br->condition = condition;
	br->reg       = reg;
	br->options   = options;

	// Add the branch to the list.
	cd->brancheslabel->push_back(br);
}


/* codegen_set_replacement_point_notrap ****************************************

   Record the position of a non-trappable replacement point.

*******************************************************************************/

#if defined(ENABLE_REPLACEMENT)
#if !defined(NDEBUG)
void codegen_set_replacement_point_notrap(codegendata *cd, s4 type)
#else
void codegen_set_replacement_point_notrap(codegendata *cd)
#endif
{
	assert(cd->replacementpoint);
	assert(cd->replacementpoint->type == type);
	assert(cd->replacementpoint->flags & rplpoint::FLAG_NOTRAP);

	cd->replacementpoint->pc = (u1*) (ptrint) (cd->mcodeptr - cd->mcodebase);

	cd->replacementpoint++;
}
#endif /* defined(ENABLE_REPLACEMENT) */


/* codegen_set_replacement_point ***********************************************

   Record the position of a trappable replacement point.

*******************************************************************************/

#if defined(ENABLE_REPLACEMENT)
#if !defined(NDEBUG)
void codegen_set_replacement_point(codegendata *cd, s4 type)
#else
void codegen_set_replacement_point(codegendata *cd)
#endif
{
	assert(cd->replacementpoint);
	assert(cd->replacementpoint->type == type);
	assert(!(cd->replacementpoint->flags & rplpoint::FLAG_NOTRAP));

	cd->replacementpoint->pc = (u1*) (ptrint) (cd->mcodeptr - cd->mcodebase);

	cd->replacementpoint++;

#if !defined(NDEBUG)
	/* XXX actually we should use an own REPLACEMENT_NOPS here! */
	if (opt_TestReplacement)
		PATCHER_NOPS;
#endif

	/* XXX assert(cd->lastmcodeptr <= cd->mcodeptr); */

	cd->lastmcodeptr = cd->mcodeptr + PATCHER_CALL_SIZE;
}
#endif /* defined(ENABLE_REPLACEMENT) */


/* codegen_finish **************************************************************

   Finishes the code generation. A new memory, large enough for both
   data and code, is allocated and data and code are copied together
   to their final layout, unresolved jumps are resolved, ...

*******************************************************************************/

void codegen_finish(jitdata *jd)
{
	s4       mcodelen;
#if defined(ENABLE_INTRP)
	s4       ncodelen;
#endif
	s4       alignedmcodelen;
	jumpref *jr;
	u1      *epoint;
	s4       alignedlen;

	/* Get required compiler data. */

	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;

	/* prevent compiler warning */

#if defined(ENABLE_INTRP)
	ncodelen = 0;
#endif

	/* calculate the code length */

	mcodelen = (s4) (cd->mcodeptr - cd->mcodebase);

	STATISTICS(count_code_len += mcodelen);
	STATISTICS(count_data_len += cd->dseglen);

	alignedmcodelen = MEMORY_ALIGN(mcodelen, MAX_ALIGN);

#if defined(ENABLE_INTRP)
	if (opt_intrp)
		ncodelen = cd->ncodeptr - cd->ncodebase;
	else {
		ncodelen = 0; /* avoid compiler warning */
	}
#endif

	cd->dseglen = MEMORY_ALIGN(cd->dseglen, MAX_ALIGN);
	alignedlen = alignedmcodelen + cd->dseglen;

#if defined(ENABLE_INTRP)
	if (opt_intrp) {
		alignedlen += ncodelen;
	}
#endif

	/* allocate new memory */

	code->mcodelength = mcodelen + cd->dseglen;
	code->mcode       = CNEW(u1, alignedlen);

	/* set the entrypoint of the method */
	
	assert(code->entrypoint == NULL);
	code->entrypoint = epoint = (code->mcode + cd->dseglen);

	/* fill the data segment (code->entrypoint must already be set!) */

	dseg_finish(jd);

	/* copy code to the new location */

	MCOPY((void *) code->entrypoint, cd->mcodebase, u1, mcodelen);

#if defined(ENABLE_INTRP)
	/* relocate native dynamic superinstruction code (if any) */

	if (opt_intrp) {
		cd->mcodebase = code->entrypoint;

		if (ncodelen > 0) {
			u1 *ncodebase = code->mcode + cd->dseglen + alignedmcodelen;

			MCOPY((void *) ncodebase, cd->ncodebase, u1, ncodelen);

			/* flush the instruction and data caches */

			md_cacheflush(ncodebase, ncodelen);

			/* set some cd variables for dynamic_super_rerwite */

			cd->ncodebase = ncodebase;

		} else {
			cd->ncodebase = NULL;
		}

		dynamic_super_rewrite(cd);
	}
#endif

	/* Fill runtime information about generated code. */

	code->stackframesize     = cd->stackframesize;
	code->synchronizedoffset = rd->memuse * 8;
	code->savedintcount      = INT_SAV_CNT - rd->savintreguse;
	code->savedfltcount      = FLT_SAV_CNT - rd->savfltreguse;

	/* Create the exception table. */

	exceptiontable_create(jd);

	/* Create the linenumber table. */

	code->linenumbertable = new LinenumberTable(jd);

	/* jump table resolving */

	for (jr = cd->jumpreferences; jr != NULL; jr = jr->next)
		*((functionptr *) ((ptrint) epoint + jr->tablepos)) =
			(functionptr) ((ptrint) epoint + (ptrint) jr->target->mpc);

	/* patcher resolving */

	patcher_resolve(jd);

#if defined(ENABLE_REPLACEMENT)
	/* replacement point resolving */
	{
		int i;
		rplpoint *rp;

		rp = code->rplpoints;
		for (i=0; i<code->rplpointcount; ++i, ++rp) {
			rp->pc = (u1*) ((ptrint) epoint + (ptrint) rp->pc);
		}
	}
#endif /* defined(ENABLE_REPLACEMENT) */

	/* Insert method into methodtree to find the entrypoint. */

	methodtree_insert(code->entrypoint, code->entrypoint + mcodelen);

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
	/* resolve data segment references */

	dseg_resolve_datareferences(jd);
#endif

	/* flush the instruction and data caches */

	md_cacheflush(code->mcode, code->mcodelength);
}

namespace {
/**
 * Outsource stack adjustment logic to reduce in-code `#if defined`s.
 *
 * @note should be moved to a backend code unit.
 */
#if defined(__ALPHA__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return  sp + framesize - SIZEOF_VOID_P; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const { return (uint64_t *) sp; }
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const { return (uint64_t *) sp; }
};
#elif defined(__ARM__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return  sp + framesize - SIZEOF_VOID_P; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const { return (uint64_t *) sp; }
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const { return (uint64_t *) sp; }
};
#elif defined(__I386__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return sp + framesize; }
	uint8_t  *get_javasp()    const { return sp + framesize + SIZEOF_VOID_P; }
	uint64_t *get_arg_regs()  const { return (uint64_t *) sp; }
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const { return (uint64_t *) (sp + 2 * SIZEOF_VOID_P); }
};
#elif defined(__MIPS__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	/* MIPS always uses 8 bytes to store the RA */
	uint8_t  *get_datasp()    const { return  sp + framesize - 8; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const {
# if SIZEOF_VOID_P == 8
		return (uint64_t *) sp;
# else
		return (uint64_t *) (sp + 5 * 8);
# endif
	}
	uint64_t *get_ret_regs()  const {
# if SIZEOF_VOID_P == 8
		return (uint64_t *) sp;
# else
		return (uint64_t *) (sp + 1 * 8);
# endif
	}
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
};
#elif defined(__S390__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return sp + framesize - 8; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const { return (uint64_t *) (sp + 96); }
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const { return (uint64_t *) (sp + 96); }
};
#elif defined(__POWERPC__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return  sp + framesize; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const {
		return (uint64_t *) (sp + LA_SIZE + 4 * SIZEOF_VOID_P);
	}
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const {
		return (uint64_t *) (sp + LA_SIZE + 2 * SIZEOF_VOID_P);
	}
};
#elif defined(__POWERPC64__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return  sp + framesize; }
	uint8_t  *get_javasp()    const { return  sp + framesize; }
	uint64_t *get_arg_regs()  const {
		return (uint64_t *) (sp + PA_SIZE + LA_SIZE + 4 * SIZEOF_VOID_P);
	}
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const {
		return (uint64_t *) (sp + PA_SIZE + LA_SIZE + 2 * SIZEOF_VOID_P);
	}
};
#elif defined(__X86_64__)
struct FrameInfo {
	u1 *sp;
	int32_t framesize;
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {}
	uint8_t  *get_datasp()    const { return sp + framesize; }
	uint8_t  *get_javasp()    const { return sp + framesize + SIZEOF_VOID_P; }
	uint64_t *get_arg_regs()  const { return (uint64_t *) sp; }
	uint64_t *get_arg_stack() const { return (uint64_t *) get_javasp(); }
	uint64_t *get_ret_regs()  const { return (uint64_t *) sp; }
};
#else
// dummy
struct FrameInfo {
	FrameInfo(u1 *sp, int32_t framesize) : sp(sp), framesize(framesize) {
		/* XXX is was unable to do this port for SPARC64, sorry. (-michi) */
		/* XXX maybe we need to pass the RA as argument there */
		os::abort("codegen_start_native_call: unsupported architecture");
	}
	uint8_t  *get_datasp()    const { return NULL; }
	uint8_t  *get_javasp()    const { return NULL; }
	uint64_t *get_arg_regs()  const { return NULL; }
	uint64_t *get_arg_stack() const { return NULL; }
	uint64_t *get_ret_regs()  const { return NULL; }
};
#endif

} // end anonymous namespace

/* codegen_start_native_call ***************************************************

   Prepares the stuff required for a native (JNI) function call:

   - adds a stackframe info structure to the chain, for stacktraces
   - prepares the local references table on the stack

   The layout of the native stub stackframe should look like this:

   +---------------------------+ <- java SP (of parent Java function)
   | return address            |
   +---------------------------+ <- data SP
   |                           |
   | stackframe info structure |
   |                           |
   +---------------------------+
   |                           |
   | local references table    |
   |                           |
   +---------------------------+
   |                           |
   | saved registers (if any)  |
   |                           |
   +---------------------------+
   |                           |
   | arguments (if any)        |
   |                           |
   +---------------------------+ <- current SP (native stub)

*******************************************************************************/

java_handle_t *codegen_start_native_call(u1 *sp, u1 *pv)
{
	stackframeinfo_t *sfi;
	localref_table   *lrt;
	codeinfo         *code;
	methodinfo       *m;
	int32_t           framesize;

	STATISTICS(count_calls_java_to_native++);

	// Get information from method header.
	code = code_get_codeinfo_for_pv(pv);
	assert(code != NULL);

	framesize = md_stacktrace_get_framesize(code);
	assert(framesize >= (int32_t) (sizeof(stackframeinfo_t) + sizeof(localref_table)));

	// Get the methodinfo.
	m = code_get_methodinfo_for_pv(pv);
	assert(m);

	/* calculate needed values */

	FrameInfo FI(sp,framesize);

	uint8_t  *datasp = FI.get_datasp();
	//uint8_t  *javasp = FI.get_javasp();
#if defined(ENABLE_HANDLES) || ( !defined(NDEBUG) && !defined(__ARM__) )
	uint64_t *arg_regs = FI.get_arg_regs();
	uint64_t *arg_stack = FI.get_arg_stack();
#endif

	/* get data structures from stack */

	sfi = (stackframeinfo_t *) (datasp - sizeof(stackframeinfo_t));
	lrt = (localref_table *)   (datasp - sizeof(stackframeinfo_t) -
								sizeof(localref_table));

#if defined(ENABLE_JNI)
	/* add current JNI local references table to this thread */

	localref_table_add(lrt);
#endif

#if !defined(NDEBUG)
# if defined(__ALPHA__) || defined(__I386__) || defined(__MIPS__) || defined(__POWERPC__) || defined(__POWERPC64__) || defined(__S390__) || defined(__X86_64__)
	/* print the call-trace if necesarry */
	/* BEFORE: filling the local reference table */

	if (opt_TraceJavaCalls || opt_TraceBuiltinCalls)
		trace_java_call_enter(m, arg_regs, arg_stack);
# endif
#endif

#if defined(ENABLE_HANDLES)
	/* place all references into the local reference table */
	/* BEFORE: creating stackframeinfo */

	localref_native_enter(m, arg_regs, arg_stack);
#endif

	/* Add a stackframeinfo for this native method.  We don't have RA
	   and XPC here.  These are determined in
	   stacktrace_stackframeinfo_add. */

	stacktrace_stackframeinfo_add(sfi, pv, sp, NULL, NULL);

	/* Return a wrapped classinfo for static methods. */

	if (m->flags & ACC_STATIC)
		return (java_handle_t *) LLNI_classinfo_wrap(m->clazz);
	else
		return NULL;
}


/* codegen_finish_native_call **************************************************

   Removes the stuff required for a native (JNI) function call.
   Additionally it checks for an exceptions and in case, get the
   exception object and clear the pointer.

*******************************************************************************/

java_object_t *codegen_finish_native_call(u1 *sp, u1 *pv)
{
	stackframeinfo_t *sfi;
	java_handle_t    *e;
	java_object_t    *o;
	codeinfo         *code;
	int32_t           framesize;


	// Get information from method header.
	code = code_get_codeinfo_for_pv(pv);
	assert(code != NULL);

	framesize = md_stacktrace_get_framesize(code);

	// Get the methodinfo.
#if defined(ENABLE_HANDLES) || !defined(NDEBUG)
	methodinfo *m = code->m;
	assert(m != NULL);
#endif

	/* calculate needed values */

	FrameInfo FI(sp,framesize);

	uint8_t  *datasp = FI.get_datasp();
#if defined(ENABLE_HANDLES) || ( !defined(NDEBUG) && !defined(__ARM__) )
	uint64_t *ret_regs = FI.get_ret_regs();
#endif

	/* get data structures from stack */

	sfi = (stackframeinfo_t *) (datasp - sizeof(stackframeinfo_t));

	/* Remove current stackframeinfo from chain. */

	stacktrace_stackframeinfo_remove(sfi);

#if defined(ENABLE_HANDLES)
	/* unwrap the return value from the local reference table */
	/* AFTER: removing the stackframeinfo */
	/* BEFORE: releasing the local reference table */

	localref_native_exit(m, ret_regs);
#endif

	/* get and unwrap the exception */
	/* AFTER: removing the stackframe info */
	/* BEFORE: releasing the local reference table */

	e = exceptions_get_and_clear_exception();
	o = LLNI_UNWRAP(e);

#if defined(ENABLE_JNI)
	/* release JNI local references table for this thread */

	localref_frame_pop_all();
	localref_table_remove();
#endif

#if !defined(NDEBUG)
# if defined(__ALPHA__) || defined(__I386__) || defined(__MIPS__) || defined(__POWERPC__) || defined(__POWERPC64__) || defined(__S390__) || defined(__X86_64__)
	/* print the call-trace if necesarry */
	/* AFTER: unwrapping the return value */

	if (opt_TraceJavaCalls || opt_TraceBuiltinCalls)
		trace_java_call_exit(m, ret_regs);
# endif
#endif

	return o;
}


/* codegen_reg_of_var **********************************************************

   This function determines a register, to which the result of an
   operation should go, when it is ultimatively intended to store the
   result in pseudoregister v.  If v is assigned to an actual
   register, this register will be returned.  Otherwise (when v is
   spilled) this function returns tempregnum.  If not already done,
   regoff and flags are set in the stack location.

*******************************************************************************/

s4 codegen_reg_of_var(u2 opcode, varinfo *v, s4 tempregnum)
{
	if (!(v->flags & INMEMORY))
		return v->vv.regoff;

	return tempregnum;
}


/* codegen_reg_of_dst **********************************************************

   This function determines a register, to which the result of an
   operation should go, when it is ultimatively intended to store the
   result in iptr->dst.var.  If dst.var is assigned to an actual
   register, this register will be returned.  Otherwise (when it is
   spilled) this function returns tempregnum.  If not already done,
   regoff and flags are set in the stack location.

*******************************************************************************/

s4 codegen_reg_of_dst(jitdata *jd, instruction *iptr, s4 tempregnum)
{
	return codegen_reg_of_var(iptr->opc, VAROP(iptr->dst), tempregnum);
}

/**
 * Fix up register locations in the case where control is transferred to an
 * exception handler block via normal control flow (no exception).
 */
static void fixup_exc_handler_interface(jitdata *jd, basicblock *bptr)
{
	// Exception handlers have exactly 1 in-slot
	assert(bptr->indepth == 1);
	varinfo *var = VAR(bptr->invars[0]);
	int32_t d = codegen_reg_of_var(0, var, REG_ITMP1_XPTR);
	emit_load(jd, NULL, var, d);
	// Copy the interface variable to ITMP1 (XPTR) because that's where
	// the handler expects it.
	emit_imove(jd->cd, d, REG_ITMP1_XPTR);
}

/**
 * Generates machine code.
 */
bool codegen_emit(jitdata *jd)
{
	varinfo*            var;
	builtintable_entry* bte = 0;
	methoddesc*         md;
	int32_t             s1, s2, /*s3,*/ d;
#if !defined(__I386__)
	int32_t             fieldtype;
	int32_t             disp;
#endif
	int                 i;

	// Get required compiler data.
	//methodinfo*   m    = jd->m;
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;
#if defined(ENABLE_SSA)
	lsradata*     ls   = jd->ls;
	bool last_cmd_was_goto = false;
#endif

	// Space to save used callee saved registers.
	int32_t savedregs_num = 0;
	savedregs_num += (INT_SAV_CNT - rd->savintreguse);
	savedregs_num += (FLT_SAV_CNT - rd->savfltreguse);

	// Calculate size of stackframe.
	cd->stackframesize = rd->memuse + savedregs_num;

	// Space to save the return address.
#if STACKFRAME_RA_TOP_OF_FRAME
# if STACKFRAME_LEAFMETHODS_RA_REGISTER
	if (!code_is_leafmethod(code))
# endif
		cd->stackframesize += 1;
#endif

	// Space to save argument of monitor_enter.
	if (checksync && code_is_synchronized(code))
#if STACKFRAME_SYNC_NEEDS_TWO_SLOTS
		/* On some architectures the stack position for the argument can
		   not be shared with place to save the return register values to
		   survive monitor_exit since both values reside in the same register. */
		cd->stackframesize += 2;
#else
		cd->stackframesize += 1;
#endif

	// Keep stack of non-leaf functions 16-byte aligned for calls into
	// native code.
	if (!code_is_leafmethod(code) || JITDATA_HAS_FLAG_VERBOSECALL(jd))
#if STACKFRMAE_RA_BETWEEN_FRAMES
		ALIGN_ODD(cd->stackframesize);
#else
		ALIGN_EVEN(cd->stackframesize);
#endif

#if defined(SPECIALMEMUSE)
	// On architectures having a linkage area, we can get rid of the whole
	// stackframe in leaf functions without saved registers.
	if (code_is_leafmethod(code) && (cd->stackframesize == LA_SIZE_IN_POINTERS))
		cd->stackframesize = 0;
#endif

	/*
	 * SECTION 1: Method header generation.
	 */

	// The method header was reduced to the bare minimum of one pointer
	// to the codeinfo structure, which in turn contains all runtime
	// information. However this section together with the methodheader.h
	// file will be kept alive for historical reasons. It might come in
	// handy at some point.

	(void) dseg_add_unique_address(cd, code);   ///< CodeinfoPointer

	// XXX, REMOVEME: We still need it for exception handling in assembler.
	// XXX ARM: (void) dseg_add_unique_s4(cd, cd->stackframesize);
#if defined(__I386__)
	int align_off = (cd->stackframesize != 0) ? 4 : 0;
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8 + align_off); /* FrameSize       */
#else
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
#endif
	(void) dseg_add_unique_s4(cd, code_is_leafmethod(code) ? 1 : 0);
	(void) dseg_add_unique_s4(cd, INT_SAV_CNT - rd->savintreguse); /* IntSave */
	(void) dseg_add_unique_s4(cd, FLT_SAV_CNT - rd->savfltreguse); /* FltSave */

	/*
	 * SECTION 2: Method prolog generation.
	 */

#if defined(ENABLE_PROFILING)
	// Generate method profiling code.
	if (JITDATA_HAS_FLAG_INSTRUMENT(jd)) {

		// Count method frequency.
		emit_profile_method(cd, code);

		// Start CPU cycle counting.
		emit_profile_cycle_start(cd, code);
	}
#endif

	// Emit code for the method prolog.
	codegen_emit_prolog(jd);

	// Emit code to call monitorenter function.
	if (checksync && code_is_synchronized(code))
		emit_monitor_enter(jd, rd->memuse * 8);

#if !defined(NDEBUG)
	// Call trace function.
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd))
		emit_verbosecall_enter(jd);
#endif

#if defined(ENABLE_SSA)
	// With SSA the header is basicblock 0, insert phi moves if necessary.
	if (ls != NULL)
		codegen_emit_phi_moves(jd, ls->basicblocks[0]);
#endif

	// Create replacement points.
	REPLACEMENT_POINTS_INIT(cd, jd);

	/*
	 * SECTION 3: ICMD code generation.
	 */

	// Walk through all basic blocks.
	for (basicblock* bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {

		bptr->mpc = (s4) (cd->mcodeptr - cd->mcodebase);

		// Is this basic block reached?
		if (bptr->state < basicblock::REACHED)
			continue;

		// Branch resolving.
		codegen_resolve_branchrefs(cd, bptr);

		// Handle replacement points.
		REPLACEMENT_POINT_BLOCK_START(cd, bptr);

#if defined(ENABLE_REPLACEMENT) && defined(__I386__)
		// Generate countdown trap code.
		methodinfo* m = jd->m;
		if (bptr->bitflags & BBFLAG_REPLACEMENT) {
			if (cd->replacementpoint[-1].flags & rplpoint::FLAG_COUNTDOWN) {
				MCODECHECK(32);
				emit_trap_countdown(cd, &(m->hitcountdown));
			}
		}
#endif

#if defined(ENABLE_PROFILING)
		// Generate basicblock profiling code.
		if (JITDATA_HAS_FLAG_INSTRUMENT(jd)) {

			// Count basicblock frequency.
			emit_profile_basicblock(cd, code, bptr);

			// If this is an exception handler, start profiling again.
			if (bptr->type == basicblock::TYPE_EXH)
				emit_profile_cycle_start(cd, code);
		}
#endif

		// Copy interface registers to their destination.
		int32_t indepth = bptr->indepth;
		// XXX Check if this is true for all archs.
		MCODECHECK(64+indepth);   // All
		MCODECHECK(128+indepth);  // PPC64
		MCODECHECK(512);          // I386, X86_64, S390
#if defined(ENABLE_SSA)
		// XXX Check if this is correct and add a propper comment!
		if (ls != NULL) {
			last_cmd_was_goto = false;
		} else {
#elif defined(ENABLE_LSRA)
		if (opt_lsra) {
			while (indepth > 0) {
				indepth--;
				var = VAR(bptr->invars[indepth]);
				if ((indepth == bptr->indepth-1) && (bptr->type == basicblock::TYPE_EXH)) {
					if (!IS_INMEMORY(src->flags))
						d = var->vv.regoff;
					else
						d = REG_ITMP1_XPTR;
					// XXX Sparc64: Here we use REG_ITMP2_XPTR, fix this!
					// XXX S390: Here we use REG_ITMP3_XPTR, fix this!
					emit_imove(cd, REG_ITMP1_XPTR, d);
					emit_store(jd, NULL, var, d);
				}
			}
		} else {
#endif
			while (indepth > 0) {
				indepth--;
				var = VAR(bptr->invars[indepth]);
				if ((indepth == bptr->indepth-1) && (bptr->type == basicblock::TYPE_EXH)) {
					d = codegen_reg_of_var(0, var, REG_ITMP1_XPTR);
					// XXX Sparc64: Here we use REG_ITMP2_XPTR, fix this!
					// XXX S390: Here we use REG_ITMP3_XPTR, fix this!
					emit_imove(cd, REG_ITMP1_XPTR, d);
					emit_store(jd, NULL, var, d);
				}
				else {
					assert((var->flags & INOUT));
				}
			}
#if defined(ENABLE_SSA) || defined(ENABLE_LSRA)
		}
#endif

		// Walk through all instructions.
		int32_t len = bptr->icount;
		uint16_t currentline = 0;
		for (instruction* iptr = bptr->iinstr; len > 0; len--, iptr++) {

			// Add line number.
			if (iptr->line != currentline) {
				linenumbertable_list_entry_add(cd, iptr->line);
				currentline = iptr->line;
			}

			// An instruction usually needs < 64 words.
			// XXX Check if this is true for all archs.
			MCODECHECK(64);    // All
			MCODECHECK(128);   // PPC64
			MCODECHECK(1024);  // I386, X86_64, S390      /* 1kB should be enough */

			// The big switch.
			switch (iptr->opc) {

			case ICMD_NOP:        /* ...  ==> ...                             */
			case ICMD_POP:        /* ..., value  ==> ...                      */
			case ICMD_POP2:       /* ..., value, value  ==> ...               */
				break;

			case ICMD_CHECKNULL:  /* ..., objectref  ==> ..., objectref       */

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				emit_nullpointer_check(cd, iptr, s1);
				break;

			case ICMD_BREAKPOINT: /* ...  ==> ...                             */
			                      /* sx.val.anyptr = Breakpoint               */

				patcher_add_patch_ref(jd, PATCHER_breakpoint, iptr->sx.val.anyptr, 0);
				PATCHER_NOPS;
	 			break;

#if defined(ENABLE_SSA)
			case ICMD_GETEXCEPTION:

				d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
				emit_imove(cd, REG_ITMP1, d);
				emit_store_dst(jd, iptr, d);
				break;
#endif

			/* inline operations **********************************************/

			case ICMD_INLINE_START:

				REPLACEMENT_POINT_INLINE_START(cd, iptr);
				break;

			case ICMD_INLINE_BODY:

				REPLACEMENT_POINT_INLINE_BODY(cd, iptr);
				linenumbertable_list_entry_add_inline_start(cd, iptr);
				linenumbertable_list_entry_add(cd, iptr->line);
				break;

			case ICMD_INLINE_END:

				linenumbertable_list_entry_add_inline_end(cd, iptr);
				linenumbertable_list_entry_add(cd, iptr->line);
				break;


			/* constant operations ********************************************/

			case ICMD_ICONST:     /* ...  ==> ..., constant                   */

				d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
				ICONST(d, iptr->sx.val.i);
				emit_store_dst(jd, iptr, d);
				break;

			case ICMD_LCONST:     /* ...  ==> ..., constant                   */

				d = codegen_reg_of_dst(jd, iptr, REG_LTMP12);
				LCONST(d, iptr->sx.val.l);
				emit_store_dst(jd, iptr, d);
				break;


			/* load/store/copy/move operations ********************************/

			case ICMD_COPY:
			case ICMD_MOVE:
			case ICMD_ILOAD:      /* ...  ==> ..., content of local variable  */
			case ICMD_LLOAD:      /* s1 = local variable                      */
			case ICMD_FLOAD:
			case ICMD_DLOAD:
			case ICMD_ALOAD:
			case ICMD_ISTORE:     /* ..., value  ==> ...                      */
			case ICMD_LSTORE:
			case ICMD_FSTORE:
			case ICMD_DSTORE:

				emit_copy(jd, iptr);
				break;

			case ICMD_ASTORE:

				if (!(iptr->flags.bits & INS_FLAG_RETADDR))
					emit_copy(jd, iptr);
				break;


			/* integer operations *********************************************/

			case ICMD_FCONST:     /* ...  ==> ..., constant                   */
			case ICMD_DCONST:     /* ...  ==> ..., constant                   */
			case ICMD_ACONST:     /* ...  ==> ..., constant                   */
			case ICMD_INEG:       /* ..., value  ==> ..., - value             */
			case ICMD_LNEG:       /* ..., value  ==> ..., - value             */
			case ICMD_I2L:        /* ..., value  ==> ..., value               */
			case ICMD_L2I:        /* ..., value  ==> ..., value               */
			case ICMD_INT2BYTE:   /* ..., value  ==> ..., value               */
			case ICMD_INT2CHAR:   /* ..., value  ==> ..., value               */
			case ICMD_INT2SHORT:  /* ..., value  ==> ..., value               */
			case ICMD_IADD:       /* ..., val1, val2  ==> ..., val1 + val2    */
			case ICMD_IINC:
			case ICMD_IADDCONST:  /* ..., value  ==> ..., value + constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2    */
			case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2    */
			case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2    */
			case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2    */
			case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_IMULPOW2:   /* ..., value  ==> ..., value * (2 ^ constant) */
			                      /* sx.val.i = constant                      */
			case ICMD_LMUL:       /* ..., val1, val2  ==> ..., val1 * val2    */
			case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_LMULPOW2:   /* ..., value  ==> ..., value * (2 ^ constant) */
			                      /* sx.val.l = constant                      */
			case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2    */
			case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2    */
			case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value >> constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2    */
			case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2    */
			case ICMD_LDIVPOW2:   /* ..., value  ==> ..., value >> constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_LREMPOW2:   /* ..., value  ==> ..., value % constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2   */
			case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2   */
			case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2  */
			case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant  */
			                      /* sx.val.i = constant                      */
			case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2   */
			case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2   */
			case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant   */
			                      /* sx.val.i = constant                      */
			case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2  */
			case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant  */
			                      /* sx.val.l = constant                      */
			case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2    */
			case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LAND:       /* ..., val1, val2  ==> ..., val1 & val2    */
			case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2    */
			case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LOR:        /* ..., val1, val2  ==> ..., val1 | val2    */
			case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant    */
			                      /* sx.val.l = constant                      */
			case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2    */
			case ICMD_IXORCONST:  /* ..., value  ==> ..., value ^ constant    */
			                      /* sx.val.i = constant                      */
			case ICMD_LXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2    */
			case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant    */
			                      /* sx.val.l = constant                      */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;


			/* floating operations ********************************************/

#if !defined(ENABLE_SOFTFLOAT)
			case ICMD_FNEG:       /* ..., value  ==> ..., - value             */
			case ICMD_DNEG:
			case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2    */
			case ICMD_DADD:
			case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2    */
			case ICMD_DSUB:
			case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2    */
			case ICMD_DMUL:
			case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2    */
			case ICMD_DDIV:
			case ICMD_FREM:       /* ..., val1, val2  ==> ..., val1 % val2        */
			case ICMD_DREM:
			case ICMD_I2F:        /* ..., value  ==> ..., (float) value       */
			case ICMD_I2D:        /* ..., value  ==> ..., (double) value      */
			case ICMD_L2F:        /* ..., value  ==> ..., (float) value       */
			case ICMD_L2D:        /* ..., value  ==> ..., (double) value      */
			case ICMD_F2I:        /* ..., value  ==> ..., (int) value         */
			case ICMD_D2I:
			case ICMD_F2L:        /* ..., value  ==> ..., (long) value        */
			case ICMD_D2L:
			case ICMD_F2D:        /* ..., value  ==> ..., (double) value      */
			case ICMD_D2F:        /* ..., value  ==> ..., (float) value       */
			case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpg val2 */
			case ICMD_DCMPL:      /* == => 0, < => 1, > => -1                 */
			case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpl val2 */
			case ICMD_DCMPG:      /* == => 0, < => 1, > => -1                 */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;
#endif /* !defined(ENABLE_SOFTFLOAT) */


			/* memory operations **********************************************/

			case ICMD_ARRAYLENGTH:/* ..., arrayref  ==> ..., length           */

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				/* implicit null-pointer check */
				// XXX PPC64: Here we had an explicit null-pointer check
				//     which I think was obsolete, please confirm. Otherwise:
				// emit_nullpointer_check(cd, iptr, s1);
				M_ILD(d, s1, OFFSET(java_array_t, size));
				emit_store_dst(jd, iptr, d);
				break;

			case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value     */
			case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...     */
			case ICMD_BASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_IASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_LASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_FASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_DASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_AASTORECONST:   /* ..., arrayref, index  ==> ...        */
			case ICMD_GETFIELD:   /* ...  ==> ..., value                      */
			case ICMD_PUTFIELD:   /* ..., value  ==> ...                      */
			case ICMD_PUTFIELDCONST:  /* ..., objectref  ==> ...              */
			                          /* val = value (in current instruction) */
			case ICMD_PUTSTATICCONST: /* ...  ==> ...                         */
			                          /* val = value (in current instruction) */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;

			case ICMD_GETSTATIC:  /* ...  ==> ..., value                      */

#if defined(__I386__)
				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;
#else
			{
				fieldinfo* fi;
				//patchref_t* pr;
				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					unresolved_field* uf = iptr->sx.s23.s3.uf;
					fieldtype = uf->fieldref->parseddesc.fd->type;
					disp      = dseg_add_unique_address(cd, 0);

					//pr = patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);
					patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);

					fi = NULL;		/* Silence compiler warning */
				}
				else {
					fi        = iptr->sx.s23.s3.fmiref->p.field;
					fieldtype = fi->type;
					disp      = dseg_add_address(cd, fi->value);

					if (!class_is_or_almost_initialized(fi->clazz)) {
						PROFILE_CYCLE_STOP;
						patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz, 0);
						PROFILE_CYCLE_START;
					}

					//pr = NULL;		/* Silence compiler warning */
				}

				// XXX X86_64: Here We had this:
				/* This approach is much faster than moving the field
				   address inline into a register. */

				M_ALD_DSEG(REG_ITMP1, disp);

				switch (fieldtype) {
				case TYPE_ADR:
					d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
					M_ALD(d, REG_ITMP1, 0);
					break;
				case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
#endif
					d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
					M_ILD(d, REG_ITMP1, 0);
					break;
				case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_DBL:
#endif
					d = codegen_reg_of_dst(jd, iptr, REG_LTMP23);
					M_LLD(d, REG_ITMP1, 0);
					break;
#if !defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
					d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
					M_FLD(d, REG_ITMP1, 0);
					break;
				case TYPE_DBL:
					d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
					M_DLD(d, REG_ITMP1, 0);
					break;
#endif
				default:
					// Silence compiler warning.
					d = 0;
				}
				emit_store_dst(jd, iptr, d);
				break;
			}
#endif

			case ICMD_PUTSTATIC:  /* ..., value  ==> ...                      */

#if defined(__I386__)
				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;
#else
			{
				fieldinfo* fi;
#if defined(USES_PATCHABLE_MEMORY_BARRIER)
				patchref_t* pr = NULL;
#endif

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					unresolved_field* uf = iptr->sx.s23.s3.uf;
					fieldtype = uf->fieldref->parseddesc.fd->type;
					disp      = dseg_add_unique_address(cd, 0);

#if defined(USES_PATCHABLE_MEMORY_BARRIER)
					pr =
#endif
					patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);

					fi = NULL;		/* Silence compiler warning */
				}
				else {
					fi = iptr->sx.s23.s3.fmiref->p.field;
					fieldtype = fi->type;
					disp      = dseg_add_address(cd, fi->value);

					if (!class_is_or_almost_initialized(fi->clazz)) {
						PROFILE_CYCLE_STOP;
						patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz, 0);
						PROFILE_CYCLE_START;
					}
				}

				// XXX X86_64: Here We had this:
				/* This approach is much faster than moving the field
				   address inline into a register. */

				M_ALD_DSEG(REG_ITMP1, disp);

				switch (fieldtype) {
				case TYPE_ADR:
					s1 = emit_load_s1(jd, iptr, REG_ITMP2);
					M_AST(s1, REG_ITMP1, 0);
					break;
				case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
#endif
					s1 = emit_load_s1(jd, iptr, REG_ITMP2);
					M_IST(s1, REG_ITMP1, 0);
					break;
				case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_DBL:
#endif
					s1 = emit_load_s1(jd, iptr, REG_LTMP23);
					M_LST(s1, REG_ITMP1, 0);
					break;
#if !defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
					s1 = emit_load_s1(jd, iptr, REG_FTMP2);
					M_FST(s1, REG_ITMP1, 0);
					break;
				case TYPE_DBL:
					s1 = emit_load_s1(jd, iptr, REG_FTMP2);
					M_DST(s1, REG_ITMP1, 0);
					break;
#endif
				}
#if defined(USES_PATCHABLE_MEMORY_BARRIER)
				codegen_emit_patchable_barrier(iptr, cd, pr, fi);
#endif
				break;
			}
#endif

			/* branch operations **********************************************/

			case ICMD_ATHROW:     /* ..., objectref ==> ... (, objectref)     */

				// We might leave this method, stop profiling.
				PROFILE_CYCLE_STOP;

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				// XXX Sparc64: We use REG_ITMP2_XPTR here, fix me!
				emit_imove(cd, s1, REG_ITMP1_XPTR);

#ifdef ENABLE_VERIFIER
				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					unresolved_class *uc = iptr->sx.s23.s2.uc;
					patcher_add_patch_ref(jd, PATCHER_resolve_class, uc, 0);
				}
#endif /* ENABLE_VERIFIER */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				ALIGNCODENOP;
				break;

			case ICMD_GOTO:       /* ... ==> ...                              */
			case ICMD_RET:        /* ... ==> ...                              */

#if defined(ENABLE_SSA)
				// In case of a goto, phimoves have to be inserted
				// before the jump.
				if (ls != NULL) {
					last_cmd_was_goto = true;
					codegen_emit_phi_moves(jd, bptr);
				}
#endif
				if (iptr->dst.block->type == basicblock::TYPE_EXH)
					fixup_exc_handler_interface(jd, iptr->dst.block);
				emit_br(cd, iptr->dst.block);
				ALIGNCODENOP;
				break;

			case ICMD_JSR:        /* ... ==> ...                              */

				assert(iptr->sx.s23.s3.jsrtarget.block->type != basicblock::TYPE_EXH);
				emit_br(cd, iptr->sx.s23.s3.jsrtarget.block);
				ALIGNCODENOP;
				break;

			case ICMD_IFNULL:     /* ..., value ==> ...                       */
			case ICMD_IFNONNULL:

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);
				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
				emit_bccz(cd, iptr->dst.block, iptr->opc - ICMD_IFNULL, s1, BRANCH_OPT_NONE);
#elif SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
				M_TEST(s1);
				emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IFNULL, BRANCH_OPT_NONE);
#else
# error Unable to generate code for this configuration!
#endif
				break;

			case ICMD_IFEQ:       /* ..., value ==> ...                       */
			case ICMD_IFNE:
			case ICMD_IFLT:
			case ICMD_IFLE:
			case ICMD_IFGT:
			case ICMD_IFGE:

				// XXX Sparc64: int compares must not branch on the
				// register directly. Reason is, that register content is
				// not 32-bit clean. Fix this!

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
				if (iptr->sx.val.i == 0) {
					s1 = emit_load_s1(jd, iptr, REG_ITMP1);
					emit_bccz(cd, iptr->dst.block, iptr->opc - ICMD_IFEQ, s1, BRANCH_OPT_NONE);
				} else {
					// Generate architecture specific instructions.
					codegen_emit_instruction(jd, iptr);
				}
#elif SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				emit_icmp_imm(cd, s1, iptr->sx.val.i);
				emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IFEQ, BRANCH_OPT_NONE);
#else
# error Unable to generate code for this configuration!
#endif
				break;

			case ICMD_IF_LEQ:     /* ..., value ==> ...                       */
			case ICMD_IF_LNE:
			case ICMD_IF_LLT:
			case ICMD_IF_LGE:
			case ICMD_IF_LGT:
			case ICMD_IF_LLE:

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;

			case ICMD_IF_ACMPEQ:  /* ..., value, value ==> ...                */
			case ICMD_IF_ACMPNE:  /* op1 = target JavaVM pc                   */

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
#if SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS
				switch (iptr->opc) {
					case ICMD_IF_ACMPEQ:
						emit_beq(cd, iptr->dst.block, s1, s2);
						break;
					case ICMD_IF_ACMPNE:
						emit_bne(cd, iptr->dst.block, s1, s2);
						break;
					default:
						break;
				}
#elif SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
				M_ACMP(s1, s2);
				emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IF_ACMPEQ, BRANCH_OPT_NONE);
#elif SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
				M_CMPEQ(s1, s2, REG_ITMP1);
				switch (iptr->opc) {
					case ICMD_IF_ACMPEQ:
						emit_bnez(cd, iptr->dst.block, REG_ITMP1);
						break;
					case ICMD_IF_ACMPNE:
						emit_beqz(cd, iptr->dst.block, REG_ITMP1);
						break;
					default:
						break;
				}
#else
# error Unable to generate code for this configuration!
#endif
				break;

			case ICMD_IF_ICMPEQ:  /* ..., value, value ==> ...                */
			case ICMD_IF_ICMPNE:  /* op1 = target JavaVM pc                   */

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

#if SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS
				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
				switch (iptr->opc) {
					case ICMD_IF_ICMPEQ:
						emit_beq(cd, iptr->dst.block, s1, s2);
						break;
					case ICMD_IF_ICMPNE:
						emit_bne(cd, iptr->dst.block, s1, s2);
						break;
				}
				break;
#else
				/* fall-through */
#endif

			case ICMD_IF_ICMPLT:  /* ..., value, value ==> ...                */
			case ICMD_IF_ICMPGT:  /* op1 = target JavaVM pc                   */
			case ICMD_IF_ICMPLE:
			case ICMD_IF_ICMPGE:

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
#if SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
# if defined(__I386__) || defined(__X86_64__)
				// XXX Fix this soon!!!
				M_ICMP(s2, s1);
# else
				M_ICMP(s1, s2);
# endif
				emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IF_ICMPEQ, BRANCH_OPT_NONE);
#elif SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
#else
# error Unable to generate code for this configuration!
#endif
				break;

			case ICMD_IF_LCMPEQ:  /* ..., value, value ==> ...                */
			case ICMD_IF_LCMPNE:  /* op1 = target JavaVM pc                   */
			case ICMD_IF_LCMPLT:
			case ICMD_IF_LCMPGT:
			case ICMD_IF_LCMPLE:
			case ICMD_IF_LCMPGE:

				assert(iptr->dst.block->type != basicblock::TYPE_EXH);

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;

			case ICMD_RETURN:     /* ...  ==> ...                             */

				REPLACEMENT_POINT_RETURN(cd, iptr);
				goto nowperformreturn;

			case ICMD_ARETURN:    /* ..., retvalue ==> ...                    */

				REPLACEMENT_POINT_RETURN(cd, iptr);
				s1 = emit_load_s1(jd, iptr, REG_RESULT);
				// XXX Sparc64: Here this should be REG_RESULT_CALLEE!
				emit_imove(cd, s1, REG_RESULT);

#ifdef ENABLE_VERIFIER
				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					PROFILE_CYCLE_STOP;
					unresolved_class *uc = iptr->sx.s23.s2.uc;
					patcher_add_patch_ref(jd, PATCHER_resolve_class, uc, 0);
					PROFILE_CYCLE_START;
				}
#endif /* ENABLE_VERIFIER */
				goto nowperformreturn;

			case ICMD_IRETURN:    /* ..., retvalue ==> ...                    */
#if defined(ENABLE_SOFTFLOAT)
			case ICMD_FRETURN:
#endif

				REPLACEMENT_POINT_RETURN(cd, iptr);
				s1 = emit_load_s1(jd, iptr, REG_RESULT);
				// XXX Sparc64: Here this should be REG_RESULT_CALLEE!
				emit_imove(cd, s1, REG_RESULT);
				goto nowperformreturn;

			case ICMD_LRETURN:    /* ..., retvalue ==> ...                    */
#if defined(ENABLE_SOFTFLOAT)
			case ICMD_DRETURN:
#endif

				REPLACEMENT_POINT_RETURN(cd, iptr);
				s1 = emit_load_s1(jd, iptr, REG_LRESULT);
				// XXX Sparc64: Here this should be REG_RESULT_CALLEE!
				emit_lmove(cd, s1, REG_LRESULT);
				goto nowperformreturn;

#if !defined(ENABLE_SOFTFLOAT)
			case ICMD_FRETURN:    /* ..., retvalue ==> ...                    */

				REPLACEMENT_POINT_RETURN(cd, iptr);
				s1 = emit_load_s1(jd, iptr, REG_FRESULT);
#if defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
				M_CAST_F2I(s1, REG_RESULT);
#else
				emit_fmove(cd, s1, REG_FRESULT);
#endif
				goto nowperformreturn;

			case ICMD_DRETURN:    /* ..., retvalue ==> ...                    */

				REPLACEMENT_POINT_RETURN(cd, iptr);
				s1 = emit_load_s1(jd, iptr, REG_FRESULT);
#if defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
				M_CAST_D2L(s1, REG_LRESULT);
#else
				emit_dmove(cd, s1, REG_FRESULT);
#endif
				goto nowperformreturn;
#endif

nowperformreturn:
#if !defined(NDEBUG)
				// Call trace function.
				if (JITDATA_HAS_FLAG_VERBOSECALL(jd))
					emit_verbosecall_exit(jd);
#endif

				// Emit code to call monitorexit function.
				if (checksync && code_is_synchronized(code)) {
					emit_monitor_exit(jd, rd->memuse * 8);
				}

				// Generate method profiling code.
				PROFILE_CYCLE_STOP;

				// Emit code for the method epilog.
				codegen_emit_epilog(jd);
				ALIGNCODENOP;
				break;

			case ICMD_BUILTIN:      /* ..., [arg1, [arg2 ...]] ==> ...        */

				REPLACEMENT_POINT_FORGC_BUILTIN(cd, iptr);

				bte = iptr->sx.s23.s3.bte;
				md  = bte->md;

#if defined(ENABLE_ESCAPE_REASON) && defined(__I386__)
				if (bte->fp == BUILTIN_escape_reason_new) {
					void set_escape_reasons(void *);
					M_ASUB_IMM(8, REG_SP);
					M_MOV_IMM(iptr->escape_reasons, REG_ITMP1);
					M_AST(EDX, REG_SP, 4);
					M_AST(REG_ITMP1, REG_SP, 0);
					M_MOV_IMM(set_escape_reasons, REG_ITMP1);
					M_CALL(REG_ITMP1);
					M_ALD(EDX, REG_SP, 4);
					M_AADD_IMM(8, REG_SP);
				}
#endif

				// Emit the fast-path if available.
				if (bte->emit_fastpath != NULL) {
					void (*emit_fastpath)(jitdata* jd, instruction* iptr, int d);
					emit_fastpath = (void (*)(jitdata* jd, instruction* iptr, int d)) bte->emit_fastpath;

					assert(md->returntype.type == TYPE_VOID);
					d = REG_ITMP1;

					// Actually call the fast-path emitter.
					emit_fastpath(jd, iptr, d);

					// If fast-path succeeded, jump to the end of the builtin
					// invocation.
					// XXX Actually the slow-path block below should be moved
					// out of the instruction stream and the jump below should be
					// inverted.
#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
					os::abort("codegen_emit: Implement jump over slow-path for this configuration.");
#elif SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
					M_TEST(d);
					emit_label_bne(cd, BRANCH_LABEL_10);
#else
# error Unable to generate code for this configuration!
#endif
				}

				goto gen_method;

			case ICMD_INVOKESTATIC: /* ..., [arg1, [arg2 ...]] ==> ...        */
			case ICMD_INVOKESPECIAL:/* ..., objectref, [arg1, [arg2 ...]] ==> ... */
			case ICMD_INVOKEVIRTUAL:/* op1 = arg count, val.a = method pointer    */
			case ICMD_INVOKEINTERFACE:

				REPLACEMENT_POINT_INVOKE(cd, iptr);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					unresolved_method* um = iptr->sx.s23.s3.um;
					md = um->methodref->parseddesc.md;
				}
				else {
					methodinfo* lm = iptr->sx.s23.s3.fmiref->p.method;
					md = lm->parseddesc;
				}

gen_method:
				i = md->paramcount;

				// XXX Check this again!
				MCODECHECK((i << 1) + 64);   // PPC

				// Copy arguments to registers or stack location.
				for (i = i - 1; i >= 0; i--) {
					var = VAR(iptr->sx.s23.s2.args[i]);
					d   = md->params[i].regoff;

					// Already pre-allocated?
					if (var->flags & PREALLOC)
						continue;

					if (!md->params[i].inmemory) {
						switch (var->type) {
						case TYPE_ADR:
						case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
						case TYPE_FLT:
#endif
							s1 = emit_load(jd, iptr, var, d);
							emit_imove(cd, s1, d);
							break;

						case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
						case TYPE_DBL:
#endif
							s1 = emit_load(jd, iptr, var, d);
							emit_lmove(cd, s1, d);
							break;

#if !defined(ENABLE_SOFTFLOAT)
						case TYPE_FLT:
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
							s1 = emit_load(jd, iptr, var, d);
							emit_fmove(cd, s1, d);
#else
							s1 = emit_load(jd, iptr, var, REG_FTMP1);
							M_CAST_F2I(s1, d);
#endif
							break;

						case TYPE_DBL:
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
							s1 = emit_load(jd, iptr, var, d);
							emit_dmove(cd, s1, d);
#else
							s1 = emit_load(jd, iptr, var, REG_FTMP1);
							M_CAST_D2L(s1, d);
#endif
							break;
#endif
						default:
							assert(false);
							break;
						}
					}
					else {
						switch (var->type) {
						case TYPE_ADR:
							s1 = emit_load(jd, iptr, var, REG_ITMP1);
							// XXX Sparc64: Here this actually was:
							//     M_STX(s1, REG_SP, JITSTACK + d);
							M_AST(s1, REG_SP, d);
							break;

						case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
						case TYPE_FLT:
#endif
#if SIZEOF_VOID_P == 4
							s1 = emit_load(jd, iptr, var, REG_ITMP1);
							M_IST(s1, REG_SP, d);
							break;
#else
							/* fall-through */
#endif

						case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
						case TYPE_DBL:
#endif
							s1 = emit_load(jd, iptr, var, REG_LTMP12);
							// XXX Sparc64: Here this actually was:
							//     M_STX(s1, REG_SP, JITSTACK + d);
							M_LST(s1, REG_SP, d);
							break;

#if !defined(ENABLE_SOFTFLOAT)
						case TYPE_FLT:
							s1 = emit_load(jd, iptr, var, REG_FTMP1);
							M_FST(s1, REG_SP, d);
							break;

						case TYPE_DBL:
							s1 = emit_load(jd, iptr, var, REG_FTMP1);
							// XXX Sparc64: Here this actually was:
							//     M_DST(s1, REG_SP, JITSTACK + d);
							M_DST(s1, REG_SP, d);
							break;
#endif
						default:
							assert(false);
							break;
						}
					}
				}

				// Generate method profiling code.
				PROFILE_CYCLE_STOP;

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);

				// Generate method profiling code.
				PROFILE_CYCLE_START;

				// Store size of call code in replacement point.
				REPLACEMENT_POINT_INVOKE_RETURN(cd, iptr);
				REPLACEMENT_POINT_FORGC_BUILTIN_RETURN(cd, iptr);

				// Recompute the procedure vector (PV).
				emit_recompute_pv(cd);

				// Store return value.
#if defined(ENABLE_SSA)
				if ((ls == NULL) /* || (!IS_TEMPVAR_INDEX(iptr->dst.varindex)) */ ||
					(ls->lifetime[iptr->dst.varindex].type != jitdata::UNUSED))
					/* a "living" stackslot */
#endif
				switch (md->returntype.type) {
				case TYPE_INT:
				case TYPE_ADR:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
#endif
					s1 = codegen_reg_of_dst(jd, iptr, REG_RESULT);
					// XXX Sparc64: This should actually be REG_RESULT_CALLER, fix this!
					emit_imove(cd, REG_RESULT, s1);
					emit_store_dst(jd, iptr, s1);
					break;

				case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
				case TYPE_DBL:
#endif
					s1 = codegen_reg_of_dst(jd, iptr, REG_LRESULT);
					// XXX Sparc64: This should actually be REG_RESULT_CALLER, fix this!
					emit_lmove(cd, REG_LRESULT, s1);
					emit_store_dst(jd, iptr, s1);
					break;

#if !defined(ENABLE_SOFTFLOAT)
				case TYPE_FLT:
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
					s1 = codegen_reg_of_dst(jd, iptr, REG_FRESULT);
					emit_fmove(cd, REG_FRESULT, s1);
#else
					s1 = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
					M_CAST_I2F(REG_RESULT, s1);
#endif
					emit_store_dst(jd, iptr, s1);
					break;

				case TYPE_DBL:
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
					s1 = codegen_reg_of_dst(jd, iptr, REG_FRESULT);
					emit_dmove(cd, REG_FRESULT, s1);
#else
					s1 = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
					M_CAST_L2D(REG_LRESULT, s1);
#endif
					emit_store_dst(jd, iptr, s1);
					break;
#endif

				case TYPE_VOID:
					break;
				default:
					assert(false);
					break;
				}

				// If we are emitting a fast-path block, this is the label for
				// successful fast-path execution.
				if ((iptr->opc == ICMD_BUILTIN) && (bte->emit_fastpath != NULL)) {
					emit_label(cd, BRANCH_LABEL_10);
				}

				break;

			case ICMD_TABLESWITCH:  /* ..., index ==> ...                     */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;

			case ICMD_LOOKUPSWITCH: /* ..., key ==> ...                       */

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				i = iptr->sx.s23.s2.lookupcount;

				// XXX Again we need to check this
				MCODECHECK((i<<2)+8);   // Alpha, ARM, i386, MIPS, Sparc64
				MCODECHECK((i<<3)+8);   // PPC64
				MCODECHECK(8 + ((7 + 6) * i) + 5);   // X86_64, S390

				// Compare keys.
				for (lookup_target_t* lookup = iptr->dst.lookup; i > 0; ++lookup, --i) {
#if SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
					emit_icmp_imm(cd, s1, lookup->value);
					emit_beq(cd, lookup->target.block);
#elif SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS
					ICONST(REG_ITMP2, lookup->value);
					emit_beq(cd, lookup->target.block, s1, REG_ITMP2);
#elif SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
					emit_icmpeq_imm(cd, s1, lookup->value, REG_ITMP2);
					emit_bnez(cd, lookup->target.block, REG_ITMP2);
#else
# error Unable to generate code for this configuration!
#endif
				}

				// Default branch.
				emit_br(cd, iptr->sx.s23.s3.lookupdefault.block);
				ALIGNCODENOP;
				break;

			case ICMD_CHECKCAST:  /* ..., objectref ==> ..., objectref        */
			case ICMD_INSTANCEOF: /* ..., objectref ==> ..., intresult        */
			case ICMD_MULTIANEWARRAY:/* ..., cnt1, [cnt2, ...] ==> ..., arrayref  */

				// Generate architecture specific instructions.
				codegen_emit_instruction(jd, iptr);
				break;

			default:
				exceptions_throw_internalerror("Unknown ICMD %d during code generation",
											   iptr->opc);
				return false;

			} // the big switch

		} // for all instructions

#if defined(ENABLE_SSA)
		// By edge splitting, in blocks with phi moves there can only
		// be a goto as last command, no other jump/branch command.
		if (ls != NULL) {
			if (!last_cmd_was_goto)
				codegen_emit_phi_moves(jd, bptr);
		}
#endif

#if defined(__I386__) || defined(__MIPS__) || defined(__S390__) || defined(__SPARC_64__) || defined(__X86_64__)
		// XXX Again!!!
		/* XXX require a lower number? */
		MCODECHECK(64);  // I386, MIPS, Sparc64
		MCODECHECK(512); // S390, X86_64

		/* XXX We can remove that when we don't use UD2 anymore on i386
		   and x86_64. */

		/* At the end of a basic block we may have to append some nops,
		   because the patcher stub calling code might be longer than the
		   actual instruction. So codepatching does not change the
		   following block unintentionally. */

		if (cd->mcodeptr < cd->lastmcodeptr) {
			while (cd->mcodeptr < cd->lastmcodeptr) {
				M_NOP;
			}
		}
#endif

		if (bptr->next && bptr->next->type == basicblock::TYPE_EXH)
			fixup_exc_handler_interface(jd, bptr->next);

	} // for all basic blocks

	// Generate traps.
	emit_patcher_traps(jd);

	// Everything's ok.
	return true;
}


/* codegen_emit_phi_moves ****************************************************

   Emits phi moves at the end of the basicblock.

*******************************************************************************/

#if defined(ENABLE_SSA)
void codegen_emit_phi_moves(jitdata *jd, basicblock *bptr)
{
	int lt_d,lt_s,i;
	lsradata *ls;
	codegendata *cd;
	varinfo *s, *d;
	instruction tmp_i;

	cd = jd->cd;
	ls = jd->ls;

	MCODECHECK(512);

	/* Moves from phi functions with highest indices have to be */
	/* inserted first, since this is the order as is used for   */
	/* conflict resolution */

	for(i = ls->num_phi_moves[bptr->nr] - 1; i >= 0 ; i--) {
		lt_d = ls->phi_moves[bptr->nr][i][0];
		lt_s = ls->phi_moves[bptr->nr][i][1];
#if defined(SSA_DEBUG_VERBOSE)
		if (compileverbose)
			printf("BB %3i Move %3i <- %3i ", bptr->nr, lt_d, lt_s);
#endif
		if (lt_s == jitdata::UNUSED) {
#if defined(SSA_DEBUG_VERBOSE)
		if (compileverbose)
			printf(" ... not processed \n");
#endif
			continue;
		}
			
		d = VAR(ls->lifetime[lt_d].v_index);
		s = VAR(ls->lifetime[lt_s].v_index);
		

		if (d->type == Type(-1)) {
#if defined(SSA_DEBUG_VERBOSE)
			if (compileverbose)
				printf("...returning - phi lifetimes where joined\n");
#endif
			continue;
		}

		if (s->type == Type(-1)) {
#if defined(SSA_DEBUG_VERBOSE)
			if (compileverbose)
				printf("...returning - phi lifetimes where joined\n");
#endif
			continue;
		}

		tmp_i.opc          = ICMD_NOP;
		tmp_i.s1.varindex  = ls->lifetime[lt_s].v_index;
		tmp_i.dst.varindex = ls->lifetime[lt_d].v_index;
		emit_copy(jd, &tmp_i);

#if defined(SSA_DEBUG_VERBOSE)
		if (compileverbose) {
			if (IS_INMEMORY(d->flags) && IS_INMEMORY(s->flags)) {
				/* mem -> mem */
				printf("M%3i <- M%3i",d->vv.regoff,s->vv.regoff);
			}
			else if (IS_INMEMORY(s->flags)) {
				/* mem -> reg */
				printf("R%3i <- M%3i",d->vv.regoff,s->vv.regoff);
			}
			else if (IS_INMEMORY(d->flags)) {
				/* reg -> mem */
				printf("M%3i <- R%3i",d->vv.regoff,s->vv.regoff);
			}
			else {
				/* reg -> reg */
				printf("R%3i <- R%3i",d->vv.regoff,s->vv.regoff);
			}
			printf("\n");
		}
#endif /* defined(SSA_DEBUG_VERBOSE) */
	}
}
#endif /* defined(ENABLE_SSA) */


/* REMOVEME When we have exception handling in C. */

void *md_asm_codegen_get_pv_from_pc(void *ra)
{
	return md_codegen_get_pv_from_pc(ra);
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
