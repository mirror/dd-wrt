/* src/vm/jit/stack.cpp - stack analysis

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

#include "vm/jit/stack.hpp"

#include "config.h"                     // for ENABLE_VERIFIER, etc

#include <stdint.h>                     // for int32_t
#include <cassert>                      // for assert
#include <climits>
#include <cstdio>                       // for NULL
#include <cstring>

#include "arch.hpp"
#include "md-abi.hpp"

#include "native/native.hpp"

#include "mm/dumpmemory.hpp"

#include "toolbox/logging.hpp"

#include "vm/descriptor.hpp"            // for methoddesc, typedesc, etc
#include "vm/exceptions.hpp"
#include "vm/global.hpp"                // for Type::TYPE_INT, etc
#include "vm/options.hpp"               // for opt_verify
#include "vm/references.hpp"            // for classref_or_classinfo, etc
#include "vm/resolve.hpp"               // for unresolved_field
#include "vm/statistics.hpp"            // for StatVar, StatDist, etc
#include "vm/types.hpp"                 // for s4, ptrint

#include "vm/jit/abi.hpp"               // for md_return_alloc
#include "vm/jit/builtin.hpp"           // for builtintable_get_internal, etc
#include "vm/jit/cfg.hpp"
#include "vm/jit/code.hpp"              // for code_unflag_leafmethod
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/jit.hpp"               // for basicblock, jitdata, etc
#include "vm/jit/parse.hpp"
#include "vm/jit/reg.hpp"               // for varinfo, etc
#include "vm/jit/show.hpp"
#include "vm/jit/verify/typeinfo.hpp"   // for typeinfo_t

#include "vm/jit/ir/icmd.hpp"           // for ::ICMD_ACONST
#include "vm/jit/ir/instruction.hpp"    // for instruction, etc

#if defined(ENABLE_DISASSEMBLER)
# include "vm/jit/disass.hpp"
#endif

#if 0
#if defined(ENABLE_SSA)
# include "vm/jit/optimizing/lsra.hpp"
# include "vm/jit/optimizing/ssa.hpp"
#elif defined(ENABLE_LSRA)
# include "vm/jit/allocator/lsra.hpp"
#endif
#endif

using namespace cacao;


//#define STACK_VERBOSE


/* For returnAddresses we use a field of the typeinfo to store from which  */
/* subroutine the returnAddress will return, if used.                      */
/* XXX It would be nicer to use typeinfo.typeclass, but the verifier seems */
/* to need it initialised to NULL. This should be investigated.            */

#if defined(ENABLE_VERIFIER)
#define SBRSTART  typeinfo.elementclass.any
#endif

STAT_REGISTER_VAR(int,count_basic_blocks,0,"basic blocks","Number of compiled basic blocks")
STAT_REGISTER_VAR(int,count_max_basic_blocks,0,"max basic blocks","Number of max. basic blocks per method")
STAT_REGISTER_VAR(int,count_javainstr,0,"java inst","Number of compiled JavaVM instructions")
STAT_REGISTER_VAR(int,count_max_javainstr,0,"max java inst","Number of max. JavaVM instructions per method")


//STAT_REGISTER_VAR(int,count_pcmd_activ,0,"count_pcmd_activ","Number of Activ    Pseudocommands")
//STAT_REGISTER_VAR(int,count_pcmd_drop,0,"count_pcmd_drop","Number of Drop     Pseudocommands")
//STAT_REGISTER_VAR(int,count_pcmd_move,0,"count_pcmd_move","Number of Move     Pseudocommands")
STAT_REGISTER_VAR(int,count_load_instruction,0,"load instruction","Number of Load Pseudocommands")
STAT_REGISTER_VAR(int,count_pcmd_op,0,"pcmd op","Number of OP Pseudocommands")
STAT_REGISTER_VAR(int,count_dup_instruction,0,"dup instruction","Number of DUP Pseudocommands")
STAT_REGISTER_VAR(int,count_pcmd_mem,0,"pcmd mem","Number of Mem Pseudocommands")
STAT_REGISTER_VAR(int,count_pcmd_met,0,"pcmd met","Number of Method Pseudocommands")
STAT_REGISTER_VAR(int,count_pcmd_table,0,"pcmd table","Number of Table Pseudocommands")
STAT_REGISTER_VAR(int,count_check_null,0,"check null","Number of Null Pointer Checks")
STAT_REGISTER_VAR(int,count_check_bound,0,"check bound","Number of Array Bound Checks")
STAT_REGISTER_VAR(int,count_max_new_stack,0,"max new stack","Maximal count of stack elements")
STAT_REGISTER_VAR(int,count_upper_bound_new_stack,0,"upper bound new stack","Upper bound of max stack elements")

STAT_REGISTER_GROUP(const_pcmd_stat,"const pcmd","Number of Const Pseudocommands")
STAT_REGISTER_GROUP_VAR(int,count_pcmd_load,0,"pcmd load","Number of Const Pseudocommands (load)",const_pcmd_stat)
STAT_REGISTER_GROUP_VAR(int,count_pcmd_zero,0,"pcmd zero","Number of Const Pseudocommands (zero)",const_pcmd_stat)

#if 0
// not used
STAT_REGISTER_GROUP(constalu_pcmd_stat,"const alu pcmd","Number of ConstAlu Pseudocommands")
STAT_REGISTER_GROUP_VAR(int,count_pcmd_const_alu,0,"count_pcmd_const_alu","Number of ConstAlu Pseudocommands",constalu_pcmd_stat)
STAT_REGISTER_GROUP_VAR(int,count_pcmd_const_bra,0,"count_pcmd_const_bra","Number of ConstAlu Pseudocommands (cmp)",constalu_pcmd_stat)
STAT_REGISTER_GROUP_VAR(int,count_pcmd_const_store,0,"count_pcmd_const_store","Number of ConstAlu Pseudocommands (store)",constalu_pcmd_stat)
#endif

STAT_REGISTER_VAR(int,count_pcmd_store,0,"count_pcmd_store","Number of Store    Pseudocommands")
// count_pcmd_store_comb?

STAT_REGISTER_GROUP(branch_pcmd_stat,"branch pcmd","Number of Branch Pseudocommands")
STAT_REGISTER_GROUP_VAR(int,count_pcmd_bra,0,"count_pcmd_bra","Number of Branch   Pseudocommands",branch_pcmd_stat)
STAT_REGISTER_GROUP_VAR(int,count_pcmd_return,0,"count_pcmd_return","Number of Branch   Pseudocommands (rets)",branch_pcmd_stat)
STAT_REGISTER_GROUP_VAR(int,count_pcmd_returnx,0,"count_pcmd_returnx","Number of Branch   Pseudocommands (Xrets)",branch_pcmd_stat)

STAT_REGISTER_DIST(unsigned int,unsigned int,count_block_stack,0,9,1,0,"stack size dist","Distribution of stack sizes at block boundary")
STAT_REGISTER_DIST(unsigned int,unsigned int,count_store_depth,0,9,1,0,"store stack depth dist","Distribution of store stack depth")
STAT_REGISTER_DIST(unsigned int,unsigned int,count_store_length,0,19,1,0,"store creator chains","Distribution of store creator chains")
STAT_REGISTER_DIST(unsigned int,unsigned int,count_analyse_iterations,1,4,1,0,"analysis iter.","Distribution of analysis iterations")

#if defined(ENABLE_STATISTICS)
static const unsigned int count_method_bb_distribution_range[] = {5,10,15,20,30,40,50,75};
static const unsigned int count_block_size_distribution_range[] = {0,1,2,3,4,5,6,7,8,9,12,14,16,18,20,25,30};
#endif

STAT_REGISTER_DIST_RANGE(unsigned int,unsigned int,count_method_bb_distribution,count_method_bb_distribution_range,8,0,"method bb dist.","Distribution of basic blocks per method")
STAT_REGISTER_DIST_RANGE(unsigned int,unsigned int,count_block_size_distribution,count_block_size_distribution_range,17,0,"bb size dist.","Distribution of basic block sizes")


/* stackdata_t *****************************************************************

   This struct holds internal data during stack analysis.

*******************************************************************************/

struct stackdata_t {
	bool analyse(jitdata*);

	basicblock *bptr;             /* the current basic block being analysed   */
	stackelement_t *new_elem;     /* next free stackelement                   */
	s4 vartop;                    /* next free variable index                 */
	s4 localcount;                /* number of locals (at the start of var)   */
	s4 varcount;                  /* maximum number of variables expected     */
	s4 varsallocated;             /* total number of variables allocated      */
	s4 maxlocals;                 /* max. number of Java locals               */
	varinfo *var;                 /* variable array (same as jd->var)         */
	s4 *javalocals;               /* map from Java locals to jd->var indices  */
	methodinfo *m;                /* the method being analysed                */
	jitdata *jd;                  /* current jitdata                          */
	basicblock *last_real_block;  /* the last block before the empty one      */
	bool repeat;                  /* if true, iterate the analysis again      */
	exception_entry **handlers;   /* exception handlers for the current block */
	exception_entry *extableend;  /* points to the last exception entry       */
	stackelement_t exstack;       /* instack for exception handlers           */
private:
	bool throw_stack_underflow() {
		exceptions_throw_verifyerror(m, "Unable to pop operand off an empty stack");
		return false;
	}
	bool throw_stack_overflow() {
		exceptions_throw_verifyerror(m, "Stack size too large");
		return false;
	}
	bool throw_stack_type_error(Type expectedtype) {
		exceptions_throw_verifyerror_for_stack(m, expectedtype);
		return false;
	}
	bool throw_stack_category_error() {
		exceptions_throw_verifyerror(m, "Attempt to split long or double on the stack");
		return false;
	}
};


/* macros used internally by analyse_stack ************************************/

/*--------------------------------------------------*/
/* BASIC TYPE CHECKING                              */
/*--------------------------------------------------*/

/* XXX would be nice if we did not have to pass the expected type */

#if defined(ENABLE_VERIFIER)
#	define CHECK_BASIC_TYPE(expected,actual)                             \
		do {                                                             \
			if ((actual) != (expected)) {                                \
				return throw_stack_type_error((Type) (expected));        \
			}                                                            \
		} while (0)
#else
#	define CHECK_BASIC_TYPE(expected,actual)
#endif

/*--------------------------------------------------*/
/* STACK UNDERFLOW/OVERFLOW CHECKS                  */
/*--------------------------------------------------*/

/* underflow checks */

#if defined(ENABLE_VERIFIER)
#	define REQUIRE(num)                                                  \
		do {                                                             \
			if (stackdepth < (num))                                      \
				return throw_stack_underflow();                          \
		} while (0)
#else
#define REQUIRE(num)
#endif


/* overflow check */
/* We allow ACONST instructions inserted as arguments to builtin
 * functions to exceed the maximum stack depth.  Maybe we should check
 * against maximum stack depth only at block boundaries?
 */

/* XXX we should find a way to remove the opc/op1 check */
#if defined(ENABLE_VERIFIER)
#	define CHECKOVERFLOW                                             \
		do {                                                         \
			if (stackdepth > m->maxstack)                            \
				if ((iptr->opc != ICMD_ACONST) || INSTRUCTION_MUST_CHECK(iptr)) \
					return throw_stack_overflow();                   \
		} while(0)
#else
#	define CHECKOVERFLOW
#endif


/* macros for allocating/releasing variable indices *****************/

#define GET_NEW_INDEX(sd, new_varindex)                              \
    do {                                                             \
        assert((sd).vartop < (sd).varcount);                         \
        (new_varindex) = ((sd).vartop)++;                            \
    } while (0)

/* Not implemented now - could be used to reuse varindices.         */
/* Pay attention to not release a localvar once implementing it!    */
#define RELEASE_INDEX(sd, varindex)

#define GET_NEW_VAR(sd, newvarindex, newtype)                        \
    do {                                                             \
        GET_NEW_INDEX((sd), (newvarindex));                          \
        (sd).var[newvarindex].type = (newtype);                      \
    } while (0)


/* macros for querying variable properties **************************/

#define IS_INOUT(sp)                                                 \
    (sd.var[(sp)->varnum].flags & INOUT)

#define IS_PREALLOC(sp)                                              \
    (sd.var[(sp)->varnum].flags & PREALLOC)

#define IS_TEMPVAR(sp)												 \
    ( ((sp)->varnum >= sd.localcount)								 \
      && !(sd.var[(sp)->varnum].flags & (INOUT | PREALLOC)) )


#define IS_LOCALVAR_SD(sd, sp)                                       \
         ((sp)->varnum < (sd).localcount)

#define IS_LOCALVAR(sp)                                              \
    IS_LOCALVAR_SD(sd, (sp))


/* macros for setting variable properties ****************************/

#define SET_TEMPVAR(sp)                                              \
    do {                                                             \
        if (IS_LOCALVAR((sp))) {                                     \
            stack_change_to_tempvar(&sd, (sp), iptr);                \
        }                                                            \
        sd.var[(sp)->varnum].flags &= ~(INOUT | PREALLOC);           \
    } while (0);

#define SET_PREALLOC(sp)                                             \
    do {                                                             \
        assert(!IS_LOCALVAR((sp)));                                  \
        sd.var[(sp)->varnum].flags |= PREALLOC;                      \
    } while (0);


/* macros for source operands ***************************************/

#define CLR_S1                                                       \
    (iptr->s1.varindex = -1)

#define USE_S1(type1)                                                \
    do {                                                             \
        REQUIRE(1);                                                  \
        CHECK_BASIC_TYPE(type1, curstack->type);                     \
        iptr->s1.varindex = curstack->varnum;                        \
    } while (0)

#define USE_S1_ANY                                                   \
    do {                                                             \
        REQUIRE(1);                                                  \
        iptr->s1.varindex = curstack->varnum;                        \
    } while (0)

#define USE_S1_S2(type1, type2)                                      \
    do {                                                             \
        REQUIRE(2);                                                  \
        CHECK_BASIC_TYPE(type1, curstack->prev->type);               \
        CHECK_BASIC_TYPE(type2, curstack->type);                     \
        iptr->sx.s23.s2.varindex = curstack->varnum;                 \
        iptr->s1.varindex = curstack->prev->varnum;                  \
    } while (0)

#define USE_S1_S2_ANY_ANY                                            \
    do {                                                             \
        REQUIRE(2);                                                  \
        iptr->sx.s23.s2.varindex = curstack->varnum;                 \
        iptr->s1.varindex = curstack->prev->varnum;                  \
    } while (0)

#define USE_S1_S2_S3(type1, type2, type3)                            \
    do {                                                             \
        REQUIRE(3);                                                  \
        CHECK_BASIC_TYPE(type1, curstack->prev->prev->type);         \
        CHECK_BASIC_TYPE(type2, curstack->prev->type);               \
        CHECK_BASIC_TYPE(type3, curstack->type);                     \
        iptr->sx.s23.s3.varindex = curstack->varnum;                 \
        iptr->sx.s23.s2.varindex = curstack->prev->varnum;           \
        iptr->s1.varindex = curstack->prev->prev->varnum;            \
    } while (0)

/* The POPANY macro does NOT check stackdepth, or set stackdepth!   */
#define POPANY                                                       \
    do {                                                             \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        curstack = curstack->prev;                                   \
    } while (0)

#define POP_S1(type1)                                                \
    do {                                                             \
        USE_S1(type1);                                               \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        curstack = curstack->prev;                                   \
    } while (0)

#define POP_S1_ANY                                                   \
    do {                                                             \
        USE_S1_ANY;                                                  \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        curstack = curstack->prev;                                   \
    } while (0)

#define POP_S1_S2(type1, type2)                                      \
    do {                                                             \
        USE_S1_S2(type1, type2);                                     \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        if (curstack->prev->varkind == UNDEFVAR)                     \
            curstack->prev->varkind = TEMPVAR;                       \
        curstack = curstack->prev->prev;                             \
    } while (0)

#define POP_S1_S2_ANY_ANY                                            \
    do {                                                             \
        USE_S1_S2_ANY_ANY;                                           \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        if (curstack->prev->varkind == UNDEFVAR)                     \
            curstack->prev->varkind = TEMPVAR;                       \
        curstack = curstack->prev->prev;                             \
    } while (0)

#define POP_S1_S2_S3(type1, type2, type3)                            \
    do {                                                             \
        USE_S1_S2_S3(type1, type2, type3);                           \
        if (curstack->varkind == UNDEFVAR)                           \
            curstack->varkind = TEMPVAR;                             \
        if (curstack->prev->varkind == UNDEFVAR)                     \
            curstack->prev->varkind = TEMPVAR;                       \
        if (curstack->prev->prev->varkind == UNDEFVAR)               \
            curstack->prev->prev->varkind = TEMPVAR;                 \
        curstack = curstack->prev->prev->prev;                       \
    } while (0)

#define CLR_SX                                                       \
    (iptr->sx.val.l = 0)


/* macros for setting the destination operand ***********************/

#define CLR_DST                                                      \
    (iptr->dst.varindex = -1)

#define DST(typed, index)                                            \
    do {                                                             \
        NEWSTACKn((typed),(index));                                  \
        curstack->creator = iptr;                                    \
        iptr->dst.varindex = (index);                                \
    } while (0)

#define DST_LOCALVAR(typed, index)                                   \
    do {                                                             \
        NEWSTACK((typed), LOCALVAR, (index));                        \
        curstack->creator = iptr;                                    \
        iptr->dst.varindex = (index);                                \
    } while (0)


/* macro for propagating constant values ****************************/

#if defined(ENABLE_VERIFIER)
#define COPY_VAL_AND_TYPE_VAR(sv, dv)                                \
    do {                                                             \
        if (((dv)->type = (sv)->type) == TYPE_RET) {                 \
            (dv)->vv  = (sv)->vv;                                    \
            (dv)->SBRSTART = (sv)->SBRSTART;                         \
        }                                                            \
    } while (0)
#else
#define COPY_VAL_AND_TYPE_VAR(sv, dv)                                \
    do {                                                             \
        (dv)->type = (sv)->type;                                     \
        if (((dv)->type = (sv)->type) == TYPE_RET) {                 \
            (dv)->vv  = (sv)->vv;                                    \
        }                                                            \
    } while (0)
#endif

#define COPY_VAL_AND_TYPE(sd, sindex, dindex)                        \
	COPY_VAL_AND_TYPE_VAR((sd).var + (sindex), (sd).var + (dindex))


/* stack modelling macros *******************************************/

#define OP0_1(typed)                                                 \
    do {                                                             \
        CLR_S1;                                                      \
        GET_NEW_VAR(sd, new_index, (typed));                         \
        DST((typed), new_index);									 \
        stackdepth++;                                                \
    } while (0)

#define OP1_0_ANY                                                    \
    do {                                                             \
        POP_S1_ANY;                                                  \
        CLR_DST;                                                     \
        stackdepth--;                                                \
    } while (0)

#define OP1_BRANCH(type1)                                            \
    do {                                                             \
        POP_S1(type1);                                               \
        stackdepth--;                                                \
    } while (0)

#define OP1_1(type1, typed)                                          \
    do {                                                             \
        POP_S1(type1);                                               \
        GET_NEW_VAR(sd, new_index, (typed));                         \
        DST(typed, new_index);                                       \
    } while (0)

#define OP2_1(type1, type2, typed)                                   \
    do {                                                             \
        POP_S1_S2(type1, type2);                                     \
        GET_NEW_VAR(sd, new_index, ((Type) (typed)));                \
        DST(((Type) (typed)), new_index);                            \
        stackdepth--;                                                \
    } while (0)

#define OP0_0                                                        \
    do {                                                             \
        CLR_S1;                                                      \
        CLR_DST;                                                     \
    } while (0)

#define OP0_BRANCH                                                   \
    do {                                                             \
        CLR_S1;                                                      \
    } while (0)

#define OP1_0(type1)                                                 \
    do {                                                             \
        POP_S1(type1);                                               \
        CLR_DST;                                                     \
        stackdepth--;                                                \
    } while (0)

#define OP2_0(type1, type2)                                          \
    do {                                                             \
        POP_S1_S2(type1, type2);                                     \
        CLR_DST;                                                     \
        stackdepth -= 2;                                             \
    } while (0)

#define OP2_BRANCH(type1, type2)                                     \
    do {                                                             \
        POP_S1_S2(type1, type2);                                     \
        stackdepth -= 2;                                             \
    } while (0)

#define OP2_0_ANY_ANY                                                \
    do {                                                             \
        POP_S1_S2_ANY_ANY;                                           \
        CLR_DST;                                                     \
        stackdepth -= 2;                                             \
    } while (0)

#define OP3_0(type1, type2, type3)                                   \
    do {                                                             \
        POP_S1_S2_S3(type1, type2, type3);                           \
        CLR_DST;                                                     \
        stackdepth -= 3;                                             \
    } while (0)

#define LOAD(type1, index)                                           \
    do {                                                             \
        DST_LOCALVAR(type1, index);                                  \
        stackdepth++;                                                \
    } while (0)

#define STORE(type1, index)                                          \
    do {                                                             \
        POP_S1(type1);                                               \
        stackdepth--;                                                \
    } while (0)


/* macros for DUP elimination ***************************************/

/* XXX replace NEW_VAR with NEW_INDEX */
#define DUP_SLOT(sp)                                                 \
    do {                                                             \
        GET_NEW_VAR(sd, new_index, (sp)->type);                      \
        COPY_VAL_AND_TYPE(sd, (sp)->varnum, new_index);              \
        NEWSTACK((sp)->type, TEMPVAR, new_index);                    \
    } while(0)

/* does not check input stackdepth */
#define MOVE_UP(sp)                                                  \
    do {                                                             \
        iptr->opc = ICMD_MOVE;                                       \
        iptr->s1.varindex = (sp)->varnum;                            \
        DUP_SLOT(sp);                                                \
        curstack->creator = iptr;                                    \
        iptr->dst.varindex = curstack->varnum;                       \
        stackdepth++;                                                \
    } while (0)

/* does not check input stackdepth */
#define COPY_UP(sp)                                                  \
    do {                                                             \
        SET_TEMPVAR((sp));                                           \
        iptr->opc = ICMD_COPY;                                       \
        iptr->s1.varindex = (sp)->varnum;                            \
        DUP_SLOT(sp);                                                \
        curstack->creator = iptr;                                    \
        iptr->dst.varindex = curstack->varnum;                       \
        stackdepth++;                                                \
    } while (0)

#define COPY_DOWN(s, d)                                              \
    do {                                                             \
        SET_TEMPVAR((s));                                            \
        iptr->opc = ICMD_COPY;                                       \
        iptr->s1.varindex = (s)->varnum;                             \
        iptr->dst.varindex = (d)->varnum;                            \
        (d)->creator = iptr;                                         \
    } while (0)

#define MOVE_TO_TEMP(sp)                                             \
    do {                                                             \
        GET_NEW_INDEX(sd, new_index);                                \
        iptr->opc = ICMD_MOVE;                                       \
        iptr->s1.varindex = (sp)->varnum;                            \
        iptr->dst.varindex = new_index;                              \
        COPY_VAL_AND_TYPE(sd, (sp)->varnum, new_index);              \
        (sp)->varnum = new_index;                                    \
		(sp)->varkind = TEMPVAR;                                     \
    } while (0)

/* macros for branching / reaching basic blocks *********************/

#define BRANCH_TARGET(bt, tempbptr)                                  \
    do {                                                             \
        tempbptr = (bt).block;                                       \
        tempbptr = stack_mark_reached(&sd, tempbptr, curstack,       \
                                      stackdepth);                   \
        if (tempbptr == NULL)                                        \
            return false;                                            \
        (bt).block = tempbptr;                                       \
    } while (0)

#define BRANCH(tempbptr)                                             \
    BRANCH_TARGET(iptr->dst, tempbptr)


/*--------------------------------------------------*/
/* ALLOCATING STACK SLOTS                           */
/*--------------------------------------------------*/

#define NEWSTACK(s,v,n)                                              \
    do {                                                             \
        sd.new_elem->prev = curstack;                                \
        sd.new_elem->type = (s);                                     \
        sd.new_elem->flags = 0;                                      \
        sd.new_elem->varkind = (v);                                  \
        sd.new_elem->varnum = (n);                                   \
        curstack = sd.new_elem;                                      \
        sd.var[(n)].type = (s);                                      \
        sd.var[(n)].flags = 0;                                       \
        sd.new_elem++;                                               \
    } while (0)

/* Initialize regoff, so -sia can show regnames even before reg.inc */
/* regs[rd->intregargnum] has to be set for this                    */
/* new_elem->regoff = (IS_FLT_DBL_TYPE(s))?-1:rd->intreg_argnum; }  */

#define NEWSTACKn(s,n)  NEWSTACK(s,UNDEFVAR,n)
#define NEWSTACK0(s)    NEWSTACK(s,UNDEFVAR,0)

/* forward declarations *******************************************************/

static void stack_create_invars(stackdata_t *sd, basicblock *b,
								stackelement_t * curstack, int stackdepth);
static void stack_create_invars_from_outvars(stackdata_t *sd, basicblock *b);

#if defined(STACK_VERBOSE)
static void stack_verbose_show_varinfo(stackdata_t *sd, varinfo *v);
static void stack_verbose_show_variable(stackdata_t *sd, s4 index);
static void stack_verbose_show_block(stackdata_t *sd, basicblock *bptr);
static void stack_verbose_block_enter(stackdata_t *sd, bool reanalyse);
static void stack_verbose_block_exit(stackdata_t *sd, bool superblockend);
static void stack_verbose_show_state(stackdata_t *sd, instruction *iptr,
									 stackelement_t * curstack);
#endif


/* stack_init ******************************************************************

   Initialized the stack analysis subsystem (called by jit_init).

*******************************************************************************/

bool stack_init(void)
{
	return true;
}


/* stack_grow_variable_array ***************************************************

   Grow the variable array so the given number of additional variables fits in.
   The number is added to `varcount`, which is the maximum number of variables
   we expect to need at this point. The actual number of variables
   (`varsallocated`) may be larger than that, in order to avoid too many
   reallocations.

   IN:
      sd...........stack analysis data
	  num..........number of additional variables

*******************************************************************************/

static void stack_grow_variable_array(stackdata_t *sd, s4 num)
{
	s4 newsize;

	assert(num >= 0);

	if (sd->varcount + num > sd->varsallocated) {
		newsize = 2*sd->varsallocated + num;

		sd->var = DMREALLOC(sd->var, varinfo, sd->varsallocated, newsize);
		MZERO(sd->var + sd->varsallocated, varinfo, (newsize - sd->varsallocated));
		sd->varsallocated = newsize;
		sd->jd->var = sd->var;
	}

	sd->varcount += num;
	sd->jd->varcount += num;

	assert(sd->varcount <= sd->varsallocated);
}


/* stack_append_block **********************************************************

   Append the given block after the last real block of the method (before
   the pseudo-block at the end).

   IN:
      sd...........stack analysis data
	  b............the block to append

*******************************************************************************/

static void stack_append_block(stackdata_t *sd, basicblock *b)
{
#if defined(STACK_VERBOSE)
	printf("APPENDING BLOCK L%0d\n", b->nr);
#endif

	b->next = sd->last_real_block->next;
	sd->last_real_block->next = b;
	sd->last_real_block = b;
	b->nr = sd->jd->basicblockcount++;
	b->next->nr = b->nr + 1;
}


/* stack_clone_block ***********************************************************

   Create a copy of the given block and insert it at the end of the method.

   CAUTION: This function does not copy the any variables or the instruction
   list. It _does_, however, reserve space for the block's invars in the
   variable array.

   IN:
      sd...........stack analysis data
	  b............the block to clone

   RETURN VALUE:
      a pointer to the copy

*******************************************************************************/

static basicblock * stack_clone_block(stackdata_t *sd, basicblock *b)
{
	basicblock *clone;

	clone = DNEW(basicblock);
	*clone  = *b;

	clone->iinstr     = NULL;
	clone->inlocals   = NULL;
	clone->javalocals = NULL;
	clone->invars     = NULL;

	clone->original            = (b->original) ? b->original : b;
	clone->copied_to           = clone->original->copied_to;
	clone->original->copied_to = clone;
	clone->next                = NULL;
	clone->state               = basicblock::REACHED;

	stack_append_block(sd, clone);

	/* reserve space for the invars of the clone */

	stack_grow_variable_array(sd, b->indepth);

#if defined(STACK_VERBOSE)
	printf("cloning block L%03d ------> L%03d\n", b->nr, clone->nr);
#endif

	return clone;
}


/* stack_create_locals *********************************************************

   Create the local variables for the start of the given basic block.

   IN:
      sd...........stack analysis data
	  b............block to create the locals for

*******************************************************************************/

static void stack_create_locals(stackdata_t *sd, basicblock *b)
{
	s4       i;
	s4      *jl;
	varinfo *dv;

	/* copy the current state of the local variables */
	/* (one extra local is needed by the verifier)   */

	dv = DMNEW(varinfo, sd->localcount + VERIFIER_EXTRA_LOCALS);
	b->inlocals = dv;
	for (i=0; i<sd->localcount; ++i)
		*dv++ = sd->var[i];

	/* the current map from java locals to cacao variables */

	jl = DMNEW(s4, sd->maxlocals);
	b->javalocals = jl;
	MCOPY(jl, sd->javalocals, s4, sd->maxlocals);
}


/* stack_merge_locals **********************************************************

   Merge local variables at the beginning of the given basic block.

   IN:
      sd...........stack analysis data
	  b............the block that is reached

*******************************************************************************/

static void stack_merge_locals(stackdata_t *sd, basicblock *b)
{
	s4 i;
#if defined(ENABLE_VERIFIER)
	varinfo *dv;
	varinfo *sv;
#endif

	/* If a javalocal is mapped to different cacao locals along the */
	/* incoming control-flow edges, it becomes undefined.           */

	for (i=0; i<sd->maxlocals; ++i) {
		if (b->javalocals[i] != jitdata::UNUSED && b->javalocals[i] != sd->javalocals[i]) {
			b->javalocals[i] = jitdata::UNUSED;
			if (b->state >= basicblock::FINISHED)
				b->state = basicblock::TYPECHECK_REACHED;
			if (b->nr <= sd->bptr->nr)
				sd->repeat = true;
		}
	}

#if defined(ENABLE_VERIFIER)
	if (b->inlocals) {
		for (i=0; i<sd->localcount; ++i) {
			dv = b->inlocals + i;
			sv = sd->var + i;
			if ((sv->type == TYPE_RET && dv->type == TYPE_RET)
					&& (sv->SBRSTART != dv->SBRSTART))
			{
#if defined(STACK_VERBOSE)
				printf("JSR MISMATCH: setting variable %d to VOID\n", i);
#endif
				dv->type = TYPE_VOID;
				if (b->state >= basicblock::FINISHED)
					b->state = basicblock::TYPECHECK_REACHED;
				sd->repeat = true; /* This is very rare, so just repeat */
			}
		}
	}
#endif /* defined(ENABLE_VERIFIER) */
}


/* stack_create_invars *********************************************************

   Create the invars for the given basic block. Also make a copy of the locals.

   IN:
      sd...........stack analysis data
	  b............block to create the invars for
	  curstack.....current stack top
	  stackdepth...current stack depth

   This function creates STACKDEPTH invars and sets their types to the
   types to the types of the corresponding slot in the current stack.

*******************************************************************************/

static void stack_create_invars(stackdata_t *sd, basicblock *b,
								stackelement_t * curstack, int stackdepth)
{
	stackelement_t * sp;
	int i;
	int index;
	varinfo *dv;
	varinfo *sv;

	assert(sd->vartop + stackdepth <= sd->varcount);

	b->indepth = stackdepth;
	b->invars = DMNEW(s4, stackdepth);

	/* allocate the variable indices */
	index = (sd->vartop += stackdepth);

	i = stackdepth;
	for (sp = curstack; i--; sp = sp->prev) {
		b->invars[i] = --index;
		dv = sd->var + index;
		sv = sd->var + sp->varnum;
		dv->flags = INOUT;
		COPY_VAL_AND_TYPE_VAR(sv, dv);
	}

	stack_create_locals(sd, b);
}


/* stack_create_invars_from_outvars ********************************************

   Create the invars for the given basic block. Also make a copy of the locals.
   Types are propagated from the outvars of the current block.

   IN:
      sd...........stack analysis data
	  b............block to create the invars for

*******************************************************************************/

static void stack_create_invars_from_outvars(stackdata_t *sd, basicblock *b)
{
	int i;
	int n;
	varinfo *sv, *dv;

	n = sd->bptr->outdepth;
	assert(sd->vartop + n <= sd->varcount);

	b->indepth = n;
	b->invars = DMNEW(s4, n);

	if (n) {
		dv = sd->var + sd->vartop;

		/* allocate the invars */

		for (i=0; i<n; ++i, ++dv) {
			sv = sd->var + sd->bptr->outvars[i];
			b->invars[i] = sd->vartop++;
			dv->flags = INOUT;
			COPY_VAL_AND_TYPE_VAR(sv, dv);
		}
	}

	stack_create_locals(sd, b);
}


/* stack_check_invars **********************************************************

   Check the current stack against the invars of the given basic block.
   Depth and types must match.

   IN:
      sd...........stack analysis data
	  b............block which invars to check against
	  curstack.....current stack top
	  stackdepth...current stack depth

   RETURN VALUE:
      the destinaton block
	  NULL.........a VerifyError has been thrown

*******************************************************************************/

static basicblock * stack_check_invars(stackdata_t *sd, basicblock *b,
							  		   stackelement_t * curstack, int stackdepth)
{
	int i;
	stackelement_t * sp;
	basicblock *orig;
	bool separable;
	varinfo *sv;
	varinfo *dv;

#if defined(STACK_VERBOSE)
	printf("stack_check_invars(L%03d)\n", b->nr);
#endif

	/* find original of b */
	if (b->original)
		b = b->original;
	orig = b;

#if defined(STACK_VERBOSE)
	printf("original is L%03d\n", orig->nr);
#endif

	i = orig->indepth;

#if defined(ENABLE_VERIFIER)
	if (i != stackdepth) {
		exceptions_throw_verifyerror(sd->m, "Stack depth mismatch");
		return NULL;
	}
#endif

	do {
		separable = false;

#if defined(STACK_VERBOSE)
		printf("checking against ");
		stack_verbose_show_block(sd, b); printf("\n");
#endif

		sp = curstack;
		for (i = orig->indepth; i--; sp = sp->prev) {
			dv = sd->var + b->invars[i];
			sv = sd->var + sp->varnum;

#if defined(ENABLE_VERIFIER)
			if (dv->type != sp->type) {
				exceptions_throw_verifyerror_for_stack(sd->m, dv->type);
				return NULL;
			}
#endif

			if (sp->type == TYPE_RET) {
#if defined(ENABLE_VERIFIER)
				if (dv->SBRSTART != sv->SBRSTART) {
					exceptions_throw_verifyerror(sd->m, "Mismatched stack types");
					return NULL;
				}
#endif
				if (dv->vv.retaddr != sv->vv.retaddr) {
					separable = true;
					/* don't break! have to check the remaining stackslots */
				}
			}
		}

		if (b->inlocals) {
			for (i=0; i<sd->localcount; ++i) {
				dv = b->inlocals + i;
				sv = sd->var + i;
				if (sv->type == TYPE_RET && dv->type == TYPE_RET) {
					if (
#if defined(ENABLE_VERIFIER)
							(sv->SBRSTART == dv->SBRSTART) &&
#endif
							(sv->vv.retaddr != dv->vv.retaddr))
					{
						separable = true;
						break;
					}
				}
			}
		}

		if (!separable) {
			/* XXX cascading collapse? */

			stack_merge_locals(sd, b);

#if defined(STACK_VERBOSE)
			printf("------> using L%03d\n", b->nr);
#endif
			return b;
		}
	} while ((b = b->copied_to) != NULL);

	b = stack_clone_block(sd, orig);
	if (!b)
		return NULL;

	stack_create_invars(sd, b, curstack, stackdepth);
	return b;
}


/* stack_check_invars_from_outvars *********************************************

   Check the outvars of the current block against the invars of the given block.
   Depth and types must match.

   IN:
      sd...........stack analysis data
	  b............block which invars to check against

   RETURN VALUE:
      the destinaton block
	  NULL.........a VerifyError has been thrown

*******************************************************************************/

static basicblock * stack_check_invars_from_outvars(stackdata_t *sd, basicblock *b)
{
	int i;
	int n;
	varinfo *sv, *dv;
	basicblock *orig;
	bool separable;

#if defined(STACK_VERBOSE)
	printf("stack_check_invars_from_outvars(L%03d)\n", b->nr);
#endif

	/* find original of b */
	if (b->original)
		b = b->original;
	orig = b;

#if defined(STACK_VERBOSE)
	printf("original is L%03d\n", orig->nr);
#endif

	i = orig->indepth;
	n = sd->bptr->outdepth;

#if defined(ENABLE_VERIFIER)
	if (i != n) {
		exceptions_throw_verifyerror(sd->m, "Stack depth mismatch");
		return NULL;
	}
#endif

	do {
		separable = false;

#if defined(STACK_VERBOSE)
		printf("checking against ");
		stack_verbose_show_block(sd, b); printf("\n");
#endif

		if (n) {
			dv = sd->var + b->invars[0];

			for (i=0; i<n; ++i, ++dv) {
				sv = sd->var + sd->bptr->outvars[i];

#if defined(ENABLE_VERIFIER)
				if (sv->type != dv->type) {
					exceptions_throw_verifyerror_for_stack(sd->m, dv->type);
					return NULL;
				}
#endif

				if (dv->type == TYPE_RET) {
#if defined(ENABLE_VERIFIER)
					if (sv->SBRSTART != dv->SBRSTART) {
						exceptions_throw_verifyerror(sd->m, "Mismatched stack types");
						return NULL;
					}
#endif
					if (sv->vv.retaddr != dv->vv.retaddr) {
						separable = true;
						/* don't break! have to check the remaining stackslots */
					}
				}
			}
		}

		if (b->inlocals) {
			for (i=0; i<sd->localcount; ++i) {
				dv = b->inlocals + i;
				sv = sd->var + i;
				if (
#if defined(ENABLE_VERIFIER)
						(sv->SBRSTART == dv->SBRSTART) &&
#endif
						(sv->type == TYPE_RET && dv->type == TYPE_RET))
				{
					if (sv->vv.retaddr != dv->vv.retaddr) {
						separable = true;
						break;
					}
				}
			}
		}

		if (!separable) {
			/* XXX cascading collapse? */

			stack_merge_locals(sd, b);

#if defined(STACK_VERBOSE)
			printf("------> using L%03d\n", b->nr);
#endif
			return b;
		}
	} while ((b = b->copied_to) != NULL);

	b = stack_clone_block(sd, orig);
	if (!b)
		return NULL;

	stack_create_invars_from_outvars(sd, b);
	return b;
}


/* stack_create_instack ********************************************************

   Create the instack of the current basic block.

   IN:
      sd...........stack analysis data

   RETURN VALUE:
      the current stack top at the start of the basic block.

*******************************************************************************/

static stackelement_t * stack_create_instack(stackdata_t *sd)
{
    stackelement_t * sp;
	int depth;
	int index;

	if ((depth = sd->bptr->indepth) == 0)
		return NULL;

    sp = (sd->new_elem += depth);

	while (depth--) {
		sp--;
		index = sd->bptr->invars[depth];
		sp->varnum = index;
		sp->type = sd->var[index].type;
		sp->prev = sp - 1;
		sp->creator = NULL;
		sp->flags = 0;
		sp->varkind = STACKVAR;
	}
	sp->prev = NULL;

	/* return the top of the created stack */
	return sd->new_elem - 1;
}


/* stack_mark_reached **********************************************************

   Mark the given block reached and propagate the current stack and locals to
   it. This function specializes the target block, if necessary, and returns
   a pointer to the specialized target.

   IN:
      sd...........stack analysis data
	  b............the block to reach
	  curstack.....the current stack top
	  stackdepth...the current stack depth

   RETURN VALUE:
      a pointer to (a specialized version of) the target
	  NULL.........a VerifyError has been thrown

*******************************************************************************/

static basicblock *stack_mark_reached(stackdata_t *sd, basicblock *b, stackelement_t * curstack, int stackdepth)
{
	assert(b != NULL);

#if defined(STACK_VERBOSE)
	printf("stack_mark_reached(L%03d from L%03d)\n", b->nr, sd->bptr->nr);
#endif

	/* mark targets of backward branches */

	if (b->nr <= sd->bptr->nr)
		b->bitflags |= BBFLAG_REPLACEMENT;

	if (b->state < basicblock::REACHED) {
		/* b is reached for the first time. Create its invars. */

#if defined(STACK_VERBOSE)
		printf("reached L%03d for the first time\n", b->nr);
#endif

		stack_create_invars(sd, b, curstack, stackdepth);

		b->state = basicblock::REACHED;

		return b;
	}
	else {
		/* b has been reached before. Check that its invars match. */

		return stack_check_invars(sd, b, curstack, stackdepth);
	}
}


/* stack_mark_reached_from_outvars *********************************************

   Mark the given block reached and propagate the outvars of the current block
   and the current locals to it. This function specializes the target block,
   if necessary, and returns a pointer to the specialized target.

   IN:
      sd...........stack analysis data
	  b............the block to reach

   RETURN VALUE:
      a pointer to (a specialized version of) the target
	  NULL.........a VerifyError has been thrown

*******************************************************************************/

static basicblock *stack_mark_reached_from_outvars(stackdata_t *sd, basicblock *b)
{
	assert(b != NULL);

#if defined(STACK_VERBOSE)
	printf("stack_mark_reached_from_outvars(L%03d from L%03d)\n", b->nr, sd->bptr->nr);
#endif

	/* mark targets of backward branches */

	if (b->nr <= sd->bptr->nr)
		b->bitflags |= BBFLAG_REPLACEMENT;

	if (b->state < basicblock::REACHED) {
		/* b is reached for the first time. Create its invars. */

#if defined(STACK_VERBOSE)
		printf("reached L%03d for the first time\n", b->nr);
#endif

		stack_create_invars_from_outvars(sd, b);

		b->state = basicblock::REACHED;

		return b;
	}
	else {
		/* b has been reached before. Check that its invars match. */

		return stack_check_invars_from_outvars(sd, b);
	}
}


/* stack_reach_next_block ******************************************************

   Mark the following block reached and propagate the outvars of the
   current block and the current locals to it.  This function
   specializes the target block, if necessary, and returns a pointer
   to the specialized target.

   IN:
      sd...........stack analysis data

   RETURN VALUE:
      a pointer to (a specialized version of) the following block
	  NULL.........a VerifyError has been thrown

*******************************************************************************/

static bool stack_reach_next_block(stackdata_t *sd)
{
	basicblock *tbptr;
	instruction *iptr;

	tbptr = (sd->bptr->original) ? sd->bptr->original : sd->bptr;
	tbptr = stack_mark_reached_from_outvars(sd, tbptr->next);

	if (tbptr == NULL)
		return false;

	if (tbptr != sd->bptr->next) {
#if defined(STACK_VERBOSE)
		printf("NEXT IS NON-CONSEQUITIVE L%03d\n", tbptr->nr);
#endif
		iptr = sd->bptr->iinstr + sd->bptr->icount - 1;
		assert(iptr->opc == ICMD_NOP);
		iptr->opc = ICMD_GOTO;
		iptr->dst.block = tbptr;
#if defined(STACK_VERBOSE)
		if (iptr->line == 0) printf("goto with line 0 in L%03d\n", sd->bptr->nr);
#endif

		if (tbptr->state < basicblock::FINISHED)
			sd->repeat = true; /* XXX check if we really need to repeat */
	}

	return true;
}


/* stack_reach_handlers ********************************************************

   Reach the exception handlers for the current block.

   IN:
      sd...........stack analysis data

   RETURN VALUE:
     true.........everything ok
	 false........a VerifyError has been thrown

*******************************************************************************/

static bool stack_reach_handlers(stackdata_t *sd)
{
	s4 i;
	basicblock *tbptr;

#if defined(STACK_VERBOSE)
	printf("reaching exception handlers...\n");
#endif

	for (i=0; sd->handlers[i]; ++i) {
		tbptr = sd->handlers[i]->handler;

		tbptr->type             = basicblock::TYPE_EXH;
		tbptr->predecessorcount = CFG_UNKNOWN_PREDECESSORS;

		/* reach (and specialize) the handler block */

		tbptr = stack_mark_reached(sd, tbptr, &(sd->exstack), 1);

		if (tbptr == NULL)
			return false;

		sd->handlers[i]->handler = tbptr;
	}

	return true;
}


/* stack_reanalyse_block  ******************************************************

   Re-analyse the current block. This is called if either the block itself
   has already been analysed before, or the current block is a clone of an
   already analysed block, and this clone is reached for the first time.
   In the latter case, this function does all that is necessary for fully
   cloning the block (cloning the instruction list and variables, etc.).

   IN:
      sd...........stack analysis data

   RETURN VALUE:
     true.........everything ok
	 false........a VerifyError has been thrown

*******************************************************************************/

#define RELOCATE(index)                                              \
    do {                                                             \
        if ((index) >= blockvarstart)                                \
            (index) += blockvarshift;                                \
        else if ((index) >= invarstart)                              \
            (index) += invarshift;                                   \
    } while (0)

bool stack_reanalyse_block(stackdata_t *sd)
{
	instruction *iptr;
	basicblock *b;
	basicblock *orig;
	s4 len;
	s4 invarstart;
	s4 blockvarstart;
	s4 invarshift;
	s4 blockvarshift;
	s4 i, varindex;
	s4 *argp;
	branch_target_t *table;
	lookup_target_t *lookup;
	bool superblockend;
	bool cloneinstructions;
	exception_entry *ex;

#if defined(STACK_VERBOSE)
	stack_verbose_block_enter(sd, true);
#endif

	b = sd->bptr;

	if (!b->iinstr) {
		orig = b->original;
		assert(orig != NULL);

		/* clone the instruction list */

		cloneinstructions = true;

		assert(orig->iinstr);
		len = orig->icount;
		iptr = DMNEW(instruction, len + 1);

		MCOPY(iptr, orig->iinstr, instruction, len);
		iptr[len].opc = ICMD_NOP;
		iptr[len].line = 0;
		iptr[len].flags.bits = 0;
		b->iinstr = iptr;
		b->icount = ++len;

		/* reserve space for the clone's block variables */

		stack_grow_variable_array(sd, orig->varcount);

		/* we already have the invars set */

		assert(b->indepth == orig->indepth);

		/* calculate relocation shifts for invars and block variables */

		if (orig->indepth) {
			invarstart = orig->invars[0];
			invarshift = b->invars[0] - invarstart;
		}
		else {
			invarstart = INT_MAX;
			invarshift = 0;
		}
		blockvarstart = orig->varstart;
		blockvarshift = sd->vartop - blockvarstart;

		/* copy block variables */

		b->varstart = sd->vartop;
		b->varcount = orig->varcount;
		sd->vartop += b->varcount;
		MCOPY(sd->var + b->varstart, sd->var + orig->varstart, varinfo, b->varcount);

		/* copy outvars */

		b->outdepth = orig->outdepth;
		b->outvars = DMNEW(s4, orig->outdepth);
		MCOPY(b->outvars, orig->outvars, s4, orig->outdepth);

		/* clone exception handlers */

		for (i=0; sd->handlers[i]; ++i) {
			ex = DNEW(exception_entry);
			ex->handler = sd->handlers[i]->handler;
			ex->start = b;
			ex->end = b; /* XXX hack, see end of stack_analyse */
			ex->catchtype = sd->handlers[i]->catchtype;
			ex->down = NULL;

			assert(sd->extableend->down == NULL);
			sd->extableend->down = ex;
			sd->extableend = ex;
			sd->jd->exceptiontablelength++;

			sd->handlers[i] = ex;
		}
	}
	else {
		cloneinstructions = false;
		invarshift = 0;
		blockvarshift = 0;
		invarstart = sd->vartop;
		blockvarstart = sd->vartop;
		iptr = b->iinstr;
	}

	if (b->original) {
		/* find exception handlers for the cloned block */
		len = 0;
		ex = sd->jd->exceptiontable;
		for (; ex != NULL; ex = ex->down) {
			/* XXX the cloned exception handlers have identical */
			/* start end end blocks.                            */
			if ((ex->start == b) && (ex->end == b)) {
				sd->handlers[len++] = ex;
			}
		}
		sd->handlers[len] = NULL;
	}

#if defined(STACK_VERBOSE)
	printf("invarstart = %d, blockvarstart = %d\n", invarstart, blockvarstart);
	printf("invarshift = %d, blockvarshift = %d\n", invarshift, blockvarshift);
#endif

	/* mark block as finished */

	b->state = basicblock::FINISHED;

	/* initialize locals at the start of this block */

	if (b->inlocals)
		MCOPY(sd->var, b->inlocals, varinfo, sd->localcount);

	MCOPY(sd->javalocals, b->javalocals, s4, sd->maxlocals);

	/* reach exception handlers for this block */

	if (!stack_reach_handlers(sd))
		return false;

	superblockend = false;

	for (len = b->icount; len--; iptr++) {
#if defined(STACK_VERBOSE)
		show_icmd(sd->jd, iptr, false, SHOW_STACK);
		printf("\n");
#endif

		switch (iptr->opc) {
			case ICMD_RET:
				varindex = iptr->s1.varindex;

#if defined(ENABLE_VERIFIER)
				if (sd->var[varindex].type != TYPE_RET) {
					exceptions_throw_verifyerror(sd->m, "RET with non-returnAddress value");
					return false;
				}
#endif

				iptr->dst.block = stack_mark_reached_from_outvars(sd, sd->var[varindex].vv.retaddr);
				superblockend = true;
				break;

			case ICMD_JSR:
				iptr->sx.s23.s3.jsrtarget.block = stack_mark_reached_from_outvars(sd, iptr->sx.s23.s3.jsrtarget.block);
				RELOCATE(iptr->dst.varindex);
				superblockend = true;
				break;

			case ICMD_RETURN:
				superblockend = true;
				break;

			case ICMD_CHECKNULL:
			case ICMD_PUTSTATICCONST:
				break;

			case ICMD_NOP:
			case ICMD_IINC:
				break;

			case ICMD_GOTO:
				iptr->dst.block = stack_mark_reached_from_outvars(sd, iptr->dst.block);
				superblockend = true;
				break;

				/* pop 0 push 1 const */

			case ICMD_ACONST:
			case ICMD_ICONST:
			case ICMD_LCONST:
			case ICMD_FCONST:
			case ICMD_DCONST:

				/* pop 0 push 1 load */

			case ICMD_ILOAD:
			case ICMD_LLOAD:
			case ICMD_FLOAD:
			case ICMD_DLOAD:
			case ICMD_ALOAD:
				RELOCATE(iptr->dst.varindex);
				break;

				/* pop 2 push 1 */

			case ICMD_IALOAD:
			case ICMD_LALOAD:
			case ICMD_FALOAD:
			case ICMD_DALOAD:
			case ICMD_AALOAD:
			case ICMD_BALOAD:
			case ICMD_CALOAD:
			case ICMD_SALOAD:
				RELOCATE(iptr->sx.s23.s2.varindex);
				RELOCATE(iptr->s1.varindex);
				RELOCATE(iptr->dst.varindex);
				break;

				/* pop 3 push 0 */

			case ICMD_IASTORE:
			case ICMD_LASTORE:
			case ICMD_FASTORE:
			case ICMD_DASTORE:
			case ICMD_AASTORE:
			case ICMD_BASTORE:
			case ICMD_CASTORE:
			case ICMD_SASTORE:
				RELOCATE(iptr->sx.s23.s3.varindex);
				RELOCATE(iptr->sx.s23.s2.varindex);
				RELOCATE(iptr->s1.varindex);
				break;

				/* pop 1 push 0 store */

			case ICMD_ISTORE:
			case ICMD_LSTORE:
			case ICMD_FSTORE:
			case ICMD_DSTORE:
			case ICMD_ASTORE:
				RELOCATE(iptr->s1.varindex);

				varindex = iptr->dst.varindex;
				COPY_VAL_AND_TYPE(*sd, iptr->s1.varindex, varindex);
				i = iptr->sx.s23.s3.javaindex;
				if (iptr->flags.bits & INS_FLAG_RETADDR) {
					iptr->sx.s23.s2.retaddrnr =
						JAVALOCAL_FROM_RETADDR(sd->var[varindex].vv.retaddr->nr);
					sd->javalocals[i] = iptr->sx.s23.s2.retaddrnr;
				}
				else
					sd->javalocals[i] = varindex;
				if (iptr->flags.bits & INS_FLAG_KILL_PREV)
					sd->javalocals[i-1] = jitdata::UNUSED;
				if (iptr->flags.bits & INS_FLAG_KILL_NEXT)
					sd->javalocals[i+1] = jitdata::UNUSED;
				break;

				/* pop 1 push 0 */

			case ICMD_ARETURN:
			case ICMD_ATHROW:
			case ICMD_IRETURN:
			case ICMD_LRETURN:
			case ICMD_FRETURN:
			case ICMD_DRETURN:
				RELOCATE(iptr->s1.varindex);
				superblockend = true;
				break;

			case ICMD_PUTSTATIC:
			case ICMD_PUTFIELDCONST:
			case ICMD_POP:
				RELOCATE(iptr->s1.varindex);
				break;

				/* pop 1 push 0 branch */

			case ICMD_IFNULL:
			case ICMD_IFNONNULL:

			case ICMD_IFEQ:
			case ICMD_IFNE:
			case ICMD_IFLT:
			case ICMD_IFGE:
			case ICMD_IFGT:
			case ICMD_IFLE:

			case ICMD_IF_LEQ:
			case ICMD_IF_LNE:
			case ICMD_IF_LLT:
			case ICMD_IF_LGE:
			case ICMD_IF_LGT:
			case ICMD_IF_LLE:
				RELOCATE(iptr->s1.varindex);
				iptr->dst.block = stack_mark_reached_from_outvars(sd, iptr->dst.block);
				break;

				/* pop 1 push 0 table branch */

			case ICMD_TABLESWITCH:
				i = iptr->sx.s23.s3.tablehigh - iptr->sx.s23.s2.tablelow + 1 + 1;

				if (cloneinstructions) {
					table = DMNEW(branch_target_t, i);
					MCOPY(table, iptr->dst.table, branch_target_t, i);
					iptr->dst.table = table;
				}
				else {
					table = iptr->dst.table;
				}

				RELOCATE(iptr->s1.varindex);
				while (i--) {
					table->block = stack_mark_reached_from_outvars(sd, table->block);
					table++;
				}
				superblockend = true;
				break;

			case ICMD_LOOKUPSWITCH:
				i = iptr->sx.s23.s2.lookupcount;
				if (cloneinstructions) {
					lookup = DMNEW(lookup_target_t, i);
					MCOPY(lookup, iptr->dst.lookup, lookup_target_t, i);
					iptr->dst.lookup = lookup;
				}
				else {
					lookup = iptr->dst.lookup;
				}
				RELOCATE(iptr->s1.varindex);
				while (i--) {
					lookup->target.block = stack_mark_reached_from_outvars(sd, lookup->target.block);
					lookup++;
				}
				iptr->sx.s23.s3.lookupdefault.block = stack_mark_reached_from_outvars(sd, iptr->sx.s23.s3.lookupdefault.block);
				superblockend = true;
				break;

			case ICMD_MONITORENTER:
			case ICMD_MONITOREXIT:
				RELOCATE(iptr->s1.varindex);
				break;

				/* pop 2 push 0 branch */

			case ICMD_IF_ICMPEQ:
			case ICMD_IF_ICMPNE:
			case ICMD_IF_ICMPLT:
			case ICMD_IF_ICMPGE:
			case ICMD_IF_ICMPGT:
			case ICMD_IF_ICMPLE:

			case ICMD_IF_LCMPEQ:
			case ICMD_IF_LCMPNE:
			case ICMD_IF_LCMPLT:
			case ICMD_IF_LCMPGE:
			case ICMD_IF_LCMPGT:
			case ICMD_IF_LCMPLE:

			case ICMD_IF_ACMPEQ:
			case ICMD_IF_ACMPNE:
				RELOCATE(iptr->sx.s23.s2.varindex);
				RELOCATE(iptr->s1.varindex);
				iptr->dst.block = stack_mark_reached_from_outvars(sd, iptr->dst.block);
				break;

				/* pop 2 push 0 */

			case ICMD_PUTFIELD:
			case ICMD_IASTORECONST:
			case ICMD_LASTORECONST:
			case ICMD_AASTORECONST:
			case ICMD_BASTORECONST:
			case ICMD_CASTORECONST:
			case ICMD_SASTORECONST:
			case ICMD_POP2:
				RELOCATE(iptr->sx.s23.s2.varindex);
				RELOCATE(iptr->s1.varindex);
				break;

				/* pop 0 push 1 copy */

			case ICMD_COPY:
			case ICMD_MOVE:
				RELOCATE(iptr->dst.varindex);
				RELOCATE(iptr->s1.varindex);
				COPY_VAL_AND_TYPE(*sd, iptr->s1.varindex, iptr->dst.varindex);
				break;

				/* pop 2 push 1 */

			case ICMD_IDIV:
			case ICMD_IREM:
			case ICMD_LDIV:
			case ICMD_LREM:
			case ICMD_IADD:
			case ICMD_ISUB:
			case ICMD_IMUL:
			case ICMD_ISHL:
			case ICMD_ISHR:
			case ICMD_IUSHR:
			case ICMD_IAND:
			case ICMD_IOR:
			case ICMD_IXOR:
			case ICMD_LADD:
			case ICMD_LSUB:
			case ICMD_LMUL:
			case ICMD_LOR:
			case ICMD_LAND:
			case ICMD_LXOR:
			case ICMD_LSHL:
			case ICMD_LSHR:
			case ICMD_LUSHR:
			case ICMD_FADD:
			case ICMD_FSUB:
			case ICMD_FMUL:
			case ICMD_FDIV:
			case ICMD_FREM:
			case ICMD_DADD:
			case ICMD_DSUB:
			case ICMD_DMUL:
			case ICMD_DDIV:
			case ICMD_DREM:
			case ICMD_LCMP:
			case ICMD_FCMPL:
			case ICMD_FCMPG:
			case ICMD_DCMPL:
			case ICMD_DCMPG:
				RELOCATE(iptr->sx.s23.s2.varindex);
				RELOCATE(iptr->s1.varindex);
				RELOCATE(iptr->dst.varindex);
				break;

				/* pop 1 push 1 */

			case ICMD_CHECKCAST:
			case ICMD_ARRAYLENGTH:
			case ICMD_INSTANCEOF:
			case ICMD_NEWARRAY:
			case ICMD_ANEWARRAY:
			case ICMD_GETFIELD:
			case ICMD_IADDCONST:
			case ICMD_ISUBCONST:
			case ICMD_IMULCONST:
			case ICMD_IMULPOW2:
			case ICMD_IDIVPOW2:
			case ICMD_IREMPOW2:
			case ICMD_IANDCONST:
			case ICMD_IORCONST:
			case ICMD_IXORCONST:
			case ICMD_ISHLCONST:
			case ICMD_ISHRCONST:
			case ICMD_IUSHRCONST:
			case ICMD_LADDCONST:
			case ICMD_LSUBCONST:
			case ICMD_LMULCONST:
			case ICMD_LMULPOW2:
			case ICMD_LDIVPOW2:
			case ICMD_LREMPOW2:
			case ICMD_LANDCONST:
			case ICMD_LORCONST:
			case ICMD_LXORCONST:
			case ICMD_LSHLCONST:
			case ICMD_LSHRCONST:
			case ICMD_LUSHRCONST:
			case ICMD_INEG:
			case ICMD_INT2BYTE:
			case ICMD_INT2CHAR:
			case ICMD_INT2SHORT:
			case ICMD_LNEG:
			case ICMD_FNEG:
			case ICMD_DNEG:
			case ICMD_I2L:
			case ICMD_I2F:
			case ICMD_I2D:
			case ICMD_L2I:
			case ICMD_L2F:
			case ICMD_L2D:
			case ICMD_F2I:
			case ICMD_F2L:
			case ICMD_F2D:
			case ICMD_D2I:
			case ICMD_D2L:
			case ICMD_D2F:
				RELOCATE(iptr->s1.varindex);
				RELOCATE(iptr->dst.varindex);
				break;

				/* pop 0 push 1 */

			case ICMD_GETSTATIC:
			case ICMD_NEW:
				RELOCATE(iptr->dst.varindex);
				break;

				/* pop many push any */

			case ICMD_INVOKESTATIC:
			case ICMD_INVOKESPECIAL:
			case ICMD_INVOKEVIRTUAL:
			case ICMD_INVOKEINTERFACE:
			case ICMD_BUILTIN:
			case ICMD_MULTIANEWARRAY:
				i = iptr->s1.argcount;
				if (cloneinstructions) {
					argp = DMNEW(s4, i);
					MCOPY(argp, iptr->sx.s23.s2.args, s4, i);
					iptr->sx.s23.s2.args = argp;
				}
				else {
					argp = iptr->sx.s23.s2.args;
				}

				while (--i >= 0) {
					RELOCATE(*argp);
					argp++;
				}
				RELOCATE(iptr->dst.varindex);
				break;

			default:
				exceptions_throw_internalerror("Unknown ICMD %d during stack re-analysis",
											   iptr->opc);
				return false;
		} /* switch */

#if defined(STACK_VERBOSE)
		show_icmd(sd->jd, iptr, false, SHOW_STACK);
		printf("\n");
#endif
	}

	/* relocate outvars */

	for (i=0; i<b->outdepth; ++i) {
		RELOCATE(b->outvars[i]);
	}

#if defined(STACK_VERBOSE)
	stack_verbose_block_exit(sd, superblockend);
#endif

	/* propagate to the next block */

	if (!superblockend)
		if (!stack_reach_next_block(sd))
			return false;

	return true;
}


/* stack_change_to_tempvar *****************************************************

   Change the given stackslot to a TEMPVAR. This includes creating a new
   temporary variable and changing the dst.varindex of the creator of the
   stacklot to the new variable index. If this stackslot has been passed
   through ICMDs between the point of its creation and the current point,
   then the variable index is also changed in these ICMDs.

   IN:
      sd...........stack analysis data
	  sp...........stackslot to change
	  ilimit.......instruction up to which to look for ICMDs passing-through
	               the stackslot (exclusive). This may point exactly after the
				   last instruction, in which case the search is done to the
				   basic block end.

*******************************************************************************/

static void stack_change_to_tempvar(stackdata_t *sd, stackelement_t * sp,
									instruction *ilimit)
{
	s4 newindex;
	s4 oldindex;
	instruction *iptr;
	s4 depth;
	s4 i;

	oldindex = sp->varnum;

	/* create a new temporary variable */

	GET_NEW_VAR(*sd, newindex, sp->type);

	sd->var[newindex].flags = sp->flags;

	/* change the stackslot */

	sp->varnum = newindex;
	sp->varkind = TEMPVAR;

	/* change the dst.varindex of the stackslot's creator */

	if (sp->creator)
		sp->creator->dst.varindex = newindex;

	/* handle ICMDs this stackslot passed through, if any */

	if (sp->flags & PASSTHROUGH) {
		iptr = (sp->creator) ? (sp->creator + 1) : sd->bptr->iinstr;

		/* assert that the limit points to an ICMD, or after the last one */

		assert(ilimit >= sd->bptr->iinstr);
	   	assert(ilimit <= sd->bptr->iinstr + sd->bptr->icount);

		/* find the stackdepth under sp plus one */
		/* Note: This number is usually known when this function is called, */
		/* but calculating it here is less error-prone and should not be    */
		/* a performance problem.                                           */

		for (depth = 0; sp != NULL; sp = sp->prev)
			depth++;

		/* iterate over all instructions in the range and replace */

		for (; iptr < ilimit; ++iptr) {
			switch (iptr->opc) {
				case ICMD_INVOKESTATIC:
				case ICMD_INVOKESPECIAL:
				case ICMD_INVOKEVIRTUAL:
				case ICMD_INVOKEINTERFACE:
				case ICMD_BUILTIN:
					i = iptr->s1.argcount - depth;
					if (iptr->sx.s23.s2.args[i] == oldindex) {
						iptr->sx.s23.s2.args[i] = newindex;
					}
					break;
				/* IMPORTANT: If any ICMD sets the PASSTHROUGH flag of a */
				/* stackslot, it must be added in this switch!           */
                default:
                    break;
			}
		}
	}
}


/* stack_init_javalocals *******************************************************

   Initialize the mapping from Java locals to cacao variables at method entry.

   IN:
      sd...........stack analysis data

*******************************************************************************/

static void stack_init_javalocals(stackdata_t *sd)
{
	jitdata *jd = sd->jd;
	s4      *jl = DMNEW(s4, sd->maxlocals);

	jd->basicblocks[0].javalocals = jl;

	for (int i=0; i<sd->maxlocals; ++i)
		jl[i] = jitdata::UNUSED;

	methoddesc *md = jd->m->parseddesc;
	for (int i = 0, j = 0; i<md->paramcount; ++i) {
		Type type = md->paramtypes[i].type;
		jl[j] = jd->local_map[5*j + type];

		j += IS_2_WORD_TYPE(type) ? 2 : 1;
	}
}


/* stack_analyse ***************************************************************

   Analyse_stack uses the intermediate code created by parse.c to
   build a model of the JVM operand stack for the current method.

   The following checks are performed:
     - check for operand stack underflow (before each instruction)
     - check for operand stack overflow (after[1] each instruction)
     - check for matching stack depth at merging points
     - check for matching basic types[2] at merging points
     - check basic types for instruction input (except for BUILTIN*
           opcodes, INVOKE* opcodes and MULTIANEWARRAY)

   [1]) Checking this after the instruction should be ok. parse.c
   counts the number of required stack slots in such a way that it is
   only vital that we don't exceed `maxstack` at basic block
   boundaries.

   [2]) 'basic types' means the distinction between INT, LONG, FLOAT,
   DOUBLE and ADDRESS types. Subtypes of INT and different ADDRESS
   types are not discerned.

*******************************************************************************/

bool stack_analyse(jitdata *jd) {
	stackdata_t sd;

	return sd.analyse(jd);
}

bool stackdata_t::analyse(jitdata *jd)
{
	stackdata_t& sd = *this;

	methodinfo   *m;              /* method being analyzed                    */
	codeinfo     *code;
	registerdata *rd;
	int           stackdepth;
	stackelement_t *curstack;       /* current stack top                        */
	stackelement_t *copy;
	ICMD          opcode;         /* opcode of current instruction            */
	int           varindex;
	int           javaindex;
	Type          type;           /* operand type                             */
	int           len;            /* # of instructions after the current one  */
	bool          superblockend;  /* if true, no fallthrough to next block    */
	bool          deadcode;       /* true if no live code has been reached    */
	instruction  *iptr;           /* the current instruction                  */
	basicblock   *tbptr;
	basicblock   *original;

	stackelement_t **last_store_boundary;
	stackelement_t *coalescing_boundary;

	stackelement_t *src1, *src2, *src3, *src4, *dst1, *dst2;

	branch_target_t *table;
	lookup_target_t *lookup;
	builtintable_entry *bte;
	methoddesc         *md;
#if defined(ENABLE_STATISTICS)
	int           iteration_count;  /* number of iterations of analysis       */
#endif
	int           new_index; /* used to get a new var index with GET_NEW_INDEX*/

#if defined(STACK_VERBOSE)
	show_method(jd, SHOW_PARSE);
#endif

	/* get required compiler data - initialization */

	m    = jd->m;
	code = jd->code;
	rd   = jd->rd;

	/* initialize the stackdata_t struct */

	sd.m = m;
	sd.jd = jd;
	sd.varcount = jd->varcount;
	sd.vartop =  jd->vartop;
	sd.localcount = jd->localcount;
	sd.var = jd->var;
	sd.varsallocated = sd.varcount;
	sd.maxlocals = m->maxlocals;
	sd.javalocals = DMNEW(s4, sd.maxlocals);
	sd.handlers = DMNEW(exception_entry *, jd->exceptiontablelength + 1);

	/* prepare the variable for exception handler stacks               */
	/* (has been reserved by STACK_EXTRA_VARS, or VERIFIER_EXTRA_VARS) */

	sd.exstack.type = TYPE_ADR;
	sd.exstack.prev = NULL;
	sd.exstack.varnum = sd.localcount;
	sd.var[sd.exstack.varnum].type = TYPE_ADR;

#if defined(ENABLE_STATISTICS)
	iteration_count = 0;
#endif

	/* find the last real basic block */

	sd.last_real_block = NULL;
	tbptr = jd->basicblocks;
	while (tbptr->next) {
		sd.last_real_block = tbptr;
		tbptr = tbptr->next;
	}
	assert(sd.last_real_block);

	/* find the last exception handler */

	if (jd->exceptiontablelength)
		sd.extableend = jd->exceptiontable + jd->exceptiontablelength - 1;
	else
		sd.extableend = NULL;

	/* init jd->interface_map */

	jd->maxinterfaces = m->maxstack;
	jd->interface_map = DMNEW(interface_info, m->maxstack * 5);
	for (int i = 0; i < m->maxstack * 5; i++)
		jd->interface_map[i].flags = jitdata::UNUSED;

	last_store_boundary = DMNEW(stackelement_t *, m->maxlocals);

	/* initialize state and invars (none) of first block */

	jd->basicblocks[0].state    = basicblock::REACHED;
	jd->basicblocks[0].invars   = NULL;
	jd->basicblocks[0].indepth  = 0;
	jd->basicblocks[0].inlocals =
		DMNEW(varinfo, jd->localcount + VERIFIER_EXTRA_LOCALS);
	MCOPY(jd->basicblocks[0].inlocals, jd->var, varinfo,
			jd->localcount + VERIFIER_EXTRA_LOCALS);

	/* initialize java local mapping of first block */

	stack_init_javalocals(&sd);

	/* stack analysis loop (until fixpoint reached) **************************/

	do {
#if defined(ENABLE_STATISTICS)
		iteration_count++;
#endif

		/* initialize loop over basic blocks */

		sd.bptr       = jd->basicblocks;
		superblockend = true;
		sd.repeat     = false;
		curstack      = NULL;
		stackdepth    = 0;
		deadcode      = true;

		/* iterate over basic blocks *****************************************/

		for (; sd.bptr; sd.bptr = sd.bptr->next) {
			if (sd.bptr->state == basicblock::DELETED) {
				/* This block has been deleted - do nothing. */

				continue;
			}

			if (sd.bptr->state == basicblock::TYPECHECK_REACHED) {
				/* re-analyse a block because its input changed */

				deadcode = false;

				if (!stack_reanalyse_block(&sd))
					return false;

				superblockend = true; /* XXX */
				continue;
			}

			if (superblockend && (sd.bptr->state < basicblock::REACHED)) {
				/* This block has not been reached so far, and we
				   don't fall into it, so we'll have to iterate
				   again. */

				sd.repeat = true;
				continue;
			}

			if (sd.bptr->state > basicblock::REACHED) {
				/* This block is already finished. */

				superblockend = true;
				continue;
			}

			if (sd.bptr->original && sd.bptr->original->state < basicblock::FINISHED) {
				/* This block is a clone and the original has not been
				   analysed, yet. Analyse it on the next
				   iteration. */

				sd.repeat = true;
				/* XXX superblockend? */
				continue;
			}

			/* This block has to be analysed now. */

			deadcode = false;

			/* XXX The rest of this block is still indented one level too */
			/* much in order to avoid a giant diff by changing that.      */

				/* We know that sd.bptr->state == BBREACHED. */
				/* This block has been reached before.    */

				assert(sd.bptr->state == basicblock::REACHED);
				stackdepth = sd.bptr->indepth;

				/* find exception handlers for this block */

				/* determine the active exception handlers for this block */
				/* XXX could use a faster algorithm with sorted lists or  */
				/* something?                                             */

				original = (sd.bptr->original) ? sd.bptr->original : sd.bptr;

				len = 0;
				for (exception_entry *ex = jd->exceptiontable; ex != NULL; ex = ex->down) {
					if ((ex->start <= original) && (ex->end > original)) {
						sd.handlers[len++] = ex;
					}
				}
				sd.handlers[len] = NULL;


				/* reanalyse cloned block */

				if (sd.bptr->original) {
					if (!stack_reanalyse_block(&sd))
						return false;
					continue;
				}

				/* reset the new pointer for allocating stackslots */

				sd.new_elem = jd->stack;

				/* create the instack of this block */

				curstack = stack_create_instack(&sd);

				/* initialize locals at the start of this block */

				if (sd.bptr->inlocals)
					MCOPY(sd.var, sd.bptr->inlocals, varinfo, sd.localcount);

				MCOPY(sd.javalocals, sd.bptr->javalocals, s4, sd.maxlocals);

				/* set up local variables for analyzing this block */

				superblockend = false;
				len = sd.bptr->icount;
				iptr = sd.bptr->iinstr;

				/* mark the block as analysed */

				sd.bptr->state = basicblock::FINISHED;

				/* reset variables for dependency checking */

				coalescing_boundary = sd.new_elem;
				for(int i = 0; i < m->maxlocals; i++)
					last_store_boundary[i] = sd.new_elem;

 				/* remember the start of this block's variables */

 				sd.bptr->varstart = sd.vartop;

#if defined(STACK_VERBOSE)
				stack_verbose_block_enter(&sd, false);
#endif

				/* reach exception handlers for this block */

				if (!stack_reach_handlers(&sd))
					return false;

				/* iterate over ICMDs ****************************************/

				while (--len >= 0)  {

#if defined(STACK_VERBOSE)
					stack_verbose_show_state(&sd, iptr, curstack);
#endif

					/* fetch the current opcode */

					opcode = iptr->opc;

					/* automatically replace some ICMDs with builtins */

					bte = builtintable_get_automatic(opcode);

					if ((bte != NULL) && (bte->opcode == opcode)) {
						iptr->opc            = ICMD_BUILTIN;
						iptr->flags.bits    &= INS_FLAG_ID_MASK;
						iptr->sx.s23.s3.bte  = bte;
						/* iptr->line is already set */
						code_unflag_leafmethod(code);
						goto icmd_BUILTIN;
					}

					/* main opcode switch *************************************/

					switch (opcode) {

						/* pop 0 push 0 */

					case ICMD_NOP:
icmd_NOP:
						CLR_SX;
						OP0_0;
						break;

					case ICMD_CHECKNULL:
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_check_null++);
						USE_S1(TYPE_ADR);
						CLR_SX;
						iptr->dst.varindex = iptr->s1.varindex;
						break;

					case ICMD_RET:
						varindex = iptr->s1.varindex =
							jd->local_map[iptr->s1.varindex * 5 + TYPE_ADR];

#if defined(ENABLE_VERIFIER)
						if (sd.var[varindex].type != TYPE_RET) {
							exceptions_throw_verifyerror(m, "RET with non-returnAddress value");
							return false;
						}
#endif

						CLR_SX;

						iptr->dst.block = stack_mark_reached(&sd, sd.var[varindex].vv.retaddr, curstack, stackdepth);
						superblockend = true;
						break;

					case ICMD_RETURN:
						STATISTICS(count_pcmd_return++);
						CLR_SX;
						OP0_0;
						superblockend = true;
						sd.jd->returncount++;
						sd.jd->returnblock = sd.bptr;
						break;

					case ICMD_BREAKPOINT:
						OP0_0;
						break;


						/* pop 0 push 1 const */

	/************************** ICONST OPTIMIZATIONS **************************/

					case ICMD_ICONST:
						STATISTICS(count_pcmd_load++);
						if (len == 0)
							goto normal_ICONST;

						switch (iptr[1].opc) {
							case ICMD_IADD:
								iptr->opc = ICMD_IADDCONST;
								/* FALLTHROUGH */

							icmd_iconst_tail:
								iptr[1].opc = ICMD_NOP;
								OP1_1(TYPE_INT, TYPE_INT);
								STATISTICS(count_pcmd_op++);
								break;

							case ICMD_ISUB:
								iptr->opc = ICMD_ISUBCONST;
								goto icmd_iconst_tail;
#if SUPPORT_CONST_MUL
							case ICMD_IMUL:
								iptr->opc = ICMD_IMULCONST;
								goto icmd_iconst_tail;
#else /* SUPPORT_CONST_MUL */
							case ICMD_IMUL:
								if (iptr->sx.val.i == 0x00000002)
									iptr->sx.val.i = 1;
								else if (iptr->sx.val.i == 0x00000004)
									iptr->sx.val.i = 2;
								else if (iptr->sx.val.i == 0x00000008)
									iptr->sx.val.i = 3;
								else if (iptr->sx.val.i == 0x00000010)
									iptr->sx.val.i = 4;
								else if (iptr->sx.val.i == 0x00000020)
									iptr->sx.val.i = 5;
								else if (iptr->sx.val.i == 0x00000040)
									iptr->sx.val.i = 6;
								else if (iptr->sx.val.i == 0x00000080)
									iptr->sx.val.i = 7;
								else if (iptr->sx.val.i == 0x00000100)
									iptr->sx.val.i = 8;
								else if (iptr->sx.val.i == 0x00000200)
									iptr->sx.val.i = 9;
								else if (iptr->sx.val.i == 0x00000400)
									iptr->sx.val.i = 10;
								else if (iptr->sx.val.i == 0x00000800)
									iptr->sx.val.i = 11;
								else if (iptr->sx.val.i == 0x00001000)
									iptr->sx.val.i = 12;
								else if (iptr->sx.val.i == 0x00002000)
									iptr->sx.val.i = 13;
								else if (iptr->sx.val.i == 0x00004000)
									iptr->sx.val.i = 14;
								else if (iptr->sx.val.i == 0x00008000)
									iptr->sx.val.i = 15;
								else if (iptr->sx.val.i == 0x00010000)
									iptr->sx.val.i = 16;
								else if (iptr->sx.val.i == 0x00020000)
									iptr->sx.val.i = 17;
								else if (iptr->sx.val.i == 0x00040000)
									iptr->sx.val.i = 18;
								else if (iptr->sx.val.i == 0x00080000)
									iptr->sx.val.i = 19;
								else if (iptr->sx.val.i == 0x00100000)
									iptr->sx.val.i = 20;
								else if (iptr->sx.val.i == 0x00200000)
									iptr->sx.val.i = 21;
								else if (iptr->sx.val.i == 0x00400000)
									iptr->sx.val.i = 22;
								else if (iptr->sx.val.i == 0x00800000)
									iptr->sx.val.i = 23;
								else if (iptr->sx.val.i == 0x01000000)
									iptr->sx.val.i = 24;
								else if (iptr->sx.val.i == 0x02000000)
									iptr->sx.val.i = 25;
								else if (iptr->sx.val.i == 0x04000000)
									iptr->sx.val.i = 26;
								else if (iptr->sx.val.i == 0x08000000)
									iptr->sx.val.i = 27;
								else if (iptr->sx.val.i == 0x10000000)
									iptr->sx.val.i = 28;
								else if (iptr->sx.val.i == 0x20000000)
									iptr->sx.val.i = 29;
								else if (iptr->sx.val.i == 0x40000000)
									iptr->sx.val.i = 30;
								else if (iptr->sx.val.u == 0x80000000)
									iptr->sx.val.i = 31;
								else
									goto normal_ICONST;

								iptr->opc = ICMD_IMULPOW2;
								goto icmd_iconst_tail;
#endif /* SUPPORT_CONST_MUL */
							case ICMD_IDIV:
								if (iptr->sx.val.i == 0x00000002)
									iptr->sx.val.i = 1;
								else if (iptr->sx.val.i == 0x00000004)
									iptr->sx.val.i = 2;
								else if (iptr->sx.val.i == 0x00000008)
									iptr->sx.val.i = 3;
								else if (iptr->sx.val.i == 0x00000010)
									iptr->sx.val.i = 4;
								else if (iptr->sx.val.i == 0x00000020)
									iptr->sx.val.i = 5;
								else if (iptr->sx.val.i == 0x00000040)
									iptr->sx.val.i = 6;
								else if (iptr->sx.val.i == 0x00000080)
									iptr->sx.val.i = 7;
								else if (iptr->sx.val.i == 0x00000100)
									iptr->sx.val.i = 8;
								else if (iptr->sx.val.i == 0x00000200)
									iptr->sx.val.i = 9;
								else if (iptr->sx.val.i == 0x00000400)
									iptr->sx.val.i = 10;
								else if (iptr->sx.val.i == 0x00000800)
									iptr->sx.val.i = 11;
								else if (iptr->sx.val.i == 0x00001000)
									iptr->sx.val.i = 12;
								else if (iptr->sx.val.i == 0x00002000)
									iptr->sx.val.i = 13;
								else if (iptr->sx.val.i == 0x00004000)
									iptr->sx.val.i = 14;
								else if (iptr->sx.val.i == 0x00008000)
									iptr->sx.val.i = 15;
								else if (iptr->sx.val.i == 0x00010000)
									iptr->sx.val.i = 16;
								else if (iptr->sx.val.i == 0x00020000)
									iptr->sx.val.i = 17;
								else if (iptr->sx.val.i == 0x00040000)
									iptr->sx.val.i = 18;
								else if (iptr->sx.val.i == 0x00080000)
									iptr->sx.val.i = 19;
								else if (iptr->sx.val.i == 0x00100000)
									iptr->sx.val.i = 20;
								else if (iptr->sx.val.i == 0x00200000)
									iptr->sx.val.i = 21;
								else if (iptr->sx.val.i == 0x00400000)
									iptr->sx.val.i = 22;
								else if (iptr->sx.val.i == 0x00800000)
									iptr->sx.val.i = 23;
								else if (iptr->sx.val.i == 0x01000000)
									iptr->sx.val.i = 24;
								else if (iptr->sx.val.i == 0x02000000)
									iptr->sx.val.i = 25;
								else if (iptr->sx.val.i == 0x04000000)
									iptr->sx.val.i = 26;
								else if (iptr->sx.val.i == 0x08000000)
									iptr->sx.val.i = 27;
								else if (iptr->sx.val.i == 0x10000000)
									iptr->sx.val.i = 28;
								else if (iptr->sx.val.i == 0x20000000)
									iptr->sx.val.i = 29;
								else if (iptr->sx.val.i == 0x40000000)
									iptr->sx.val.i = 30;
								else if (iptr->sx.val.u == 0x80000000)
									iptr->sx.val.i = 31;
								else
									goto normal_ICONST;

								iptr->opc = ICMD_IDIVPOW2;
								goto icmd_iconst_tail;

							case ICMD_IREM:
								/*log_text("stack.c: ICMD_ICONST/ICMD_IREM");*/
								if ((iptr->sx.val.i == 0x00000002) ||
									(iptr->sx.val.i == 0x00000004) ||
									(iptr->sx.val.i == 0x00000008) ||
									(iptr->sx.val.i == 0x00000010) ||
									(iptr->sx.val.i == 0x00000020) ||
									(iptr->sx.val.i == 0x00000040) ||
									(iptr->sx.val.i == 0x00000080) ||
									(iptr->sx.val.i == 0x00000100) ||
									(iptr->sx.val.i == 0x00000200) ||
									(iptr->sx.val.i == 0x00000400) ||
									(iptr->sx.val.i == 0x00000800) ||
									(iptr->sx.val.i == 0x00001000) ||
									(iptr->sx.val.i == 0x00002000) ||
									(iptr->sx.val.i == 0x00004000) ||
									(iptr->sx.val.i == 0x00008000) ||
									(iptr->sx.val.i == 0x00010000) ||
									(iptr->sx.val.i == 0x00020000) ||
									(iptr->sx.val.i == 0x00040000) ||
									(iptr->sx.val.i == 0x00080000) ||
									(iptr->sx.val.i == 0x00100000) ||
									(iptr->sx.val.i == 0x00200000) ||
									(iptr->sx.val.i == 0x00400000) ||
									(iptr->sx.val.i == 0x00800000) ||
									(iptr->sx.val.i == 0x01000000) ||
									(iptr->sx.val.i == 0x02000000) ||
									(iptr->sx.val.i == 0x04000000) ||
									(iptr->sx.val.i == 0x08000000) ||
									(iptr->sx.val.i == 0x10000000) ||
									(iptr->sx.val.i == 0x20000000) ||
									(iptr->sx.val.i == 0x40000000) ||
									(iptr->sx.val.u == 0x80000000))
								{
									iptr->opc = ICMD_IREMPOW2;
									iptr->sx.val.i -= 1;
									goto icmd_iconst_tail;
								}
								goto normal_ICONST;
#if SUPPORT_CONST_LOGICAL
							case ICMD_IAND:
								iptr->opc = ICMD_IANDCONST;
								goto icmd_iconst_tail;

							case ICMD_IOR:
								iptr->opc = ICMD_IORCONST;
								goto icmd_iconst_tail;

							case ICMD_IXOR:
								iptr->opc = ICMD_IXORCONST;
								goto icmd_iconst_tail;

#endif /* SUPPORT_CONST_LOGICAL */
							case ICMD_ISHL:
								iptr->opc = ICMD_ISHLCONST;
								goto icmd_iconst_tail;

							case ICMD_ISHR:
								iptr->opc = ICMD_ISHRCONST;
								goto icmd_iconst_tail;

							case ICMD_IUSHR:
								iptr->opc = ICMD_IUSHRCONST;
								goto icmd_iconst_tail;
#if SUPPORT_LONG_SHIFT
							case ICMD_LSHL:
								iptr->opc = ICMD_LSHLCONST;
								goto icmd_lconst_tail;

							case ICMD_LSHR:
								iptr->opc = ICMD_LSHRCONST;
								goto icmd_lconst_tail;

							case ICMD_LUSHR:
								iptr->opc = ICMD_LUSHRCONST;
								goto icmd_lconst_tail;
#endif /* SUPPORT_LONG_SHIFT */
							case ICMD_IF_ICMPEQ:
								iptr[1].opc = ICMD_IFEQ;
								/* FALLTHROUGH */

							icmd_if_icmp_tail:
								/* set the constant for the following icmd */
								iptr[1].sx.val.i = iptr->sx.val.i;

								/* this instruction becomes a nop */
								iptr->opc = ICMD_NOP;
								goto icmd_NOP;

							case ICMD_IF_ICMPLT:
								iptr[1].opc = ICMD_IFLT;
								goto icmd_if_icmp_tail;

							case ICMD_IF_ICMPLE:
								iptr[1].opc = ICMD_IFLE;
								goto icmd_if_icmp_tail;

							case ICMD_IF_ICMPNE:
								iptr[1].opc = ICMD_IFNE;
								goto icmd_if_icmp_tail;

							case ICMD_IF_ICMPGT:
								iptr[1].opc = ICMD_IFGT;
								goto icmd_if_icmp_tail;

							case ICMD_IF_ICMPGE:
								iptr[1].opc = ICMD_IFGE;
								goto icmd_if_icmp_tail;

#if SUPPORT_CONST_STORE
							case ICMD_IASTORE:
							case ICMD_BASTORE:
							case ICMD_CASTORE:
							case ICMD_SASTORE:
# if SUPPORT_CONST_STORE_ZERO_ONLY
								if (iptr->sx.val.i != 0)
									goto normal_ICONST;
# endif
								switch (iptr[1].opc) {
									case ICMD_IASTORE:
										iptr->opc = ICMD_IASTORECONST;
										iptr->flags.bits |= INS_FLAG_CHECK;
										break;
									case ICMD_BASTORE:
										iptr->opc = ICMD_BASTORECONST;
										iptr->flags.bits |= INS_FLAG_CHECK;
										break;
									case ICMD_CASTORE:
										iptr->opc = ICMD_CASTORECONST;
										iptr->flags.bits |= INS_FLAG_CHECK;
										break;
									case ICMD_SASTORE:
										iptr->opc = ICMD_SASTORECONST;
										iptr->flags.bits |= INS_FLAG_CHECK;
										break;
                                    default:
                                        break;
								}

								iptr[1].opc = ICMD_NOP;

								/* copy the constant to s3 */
								/* XXX constval -> astoreconstval? */
								iptr->sx.s23.s3.constval = iptr->sx.val.i;
								OP2_0(TYPE_ADR, TYPE_INT);
								STATISTICS(count_pcmd_op++);
								break;

							case ICMD_PUTSTATIC:
							case ICMD_PUTFIELD:
# if SUPPORT_CONST_STORE_ZERO_ONLY
								if (iptr->sx.val.i != 0)
									goto normal_ICONST;
# endif
								/* XXX check field type? */

								/* copy the constant to s2 */
								/* XXX constval -> fieldconstval? */
								iptr->sx.s23.s2.constval = iptr->sx.val.i;
								// fallthrough!

							putconst_tail: {
								constant_FMIref *fmiref;

								/* set the field reference (s3) */
								if (iptr[1].flags.bits & INS_FLAG_UNRESOLVED) {
									iptr->sx.s23.s3.uf = iptr[1].sx.s23.s3.uf;
									iptr->flags.bits |= INS_FLAG_UNRESOLVED;
									fmiref = iptr->sx.s23.s3.uf->fieldref;
								}
								else {
									fmiref = iptr[1].sx.s23.s3.fmiref;
									iptr->sx.s23.s3.fmiref = fmiref;
								}

#if defined(ENABLE_VERIFIER)
								Type expectedtype = fmiref->parseddesc.fd->type;
								switch (iptr[0].opc) {
									case ICMD_ICONST:
										if (expectedtype != TYPE_INT)
											return throw_stack_type_error(expectedtype);
										break;
									case ICMD_LCONST:
										if (expectedtype != TYPE_LNG)
											return throw_stack_type_error(expectedtype);
										break;
									case ICMD_ACONST:
										if (expectedtype != TYPE_ADR)
											return throw_stack_type_error(expectedtype);
										break;
									default:
										assert(false);
										break;
								}
#endif /* defined(ENABLE_VERIFIER) */

								switch (iptr[1].opc) {
									case ICMD_PUTSTATIC:
										iptr->opc = ICMD_PUTSTATICCONST;
										OP0_0;
										break;
									case ICMD_PUTFIELD:
										iptr->opc = ICMD_PUTFIELDCONST;
										OP1_0(TYPE_ADR);
										break;
									default:
										break;
								}

								iptr[1].opc = ICMD_NOP;
								STATISTICS(count_pcmd_op++);
								break;
							}
#endif /* SUPPORT_CONST_STORE */

							default:
								goto normal_ICONST;
						}

						/* if we get here, the ICONST has been optimized */
						break;

normal_ICONST:
						/* normal case of an unoptimized ICONST */
						OP0_1(TYPE_INT);
						break;

	/************************** LCONST OPTIMIZATIONS **************************/

					case ICMD_LCONST:
						STATISTICS(count_pcmd_load++);
						if (len == 0)
							goto normal_LCONST;

						/* switch depending on the following instruction */

						switch (iptr[1].opc) {
#if SUPPORT_LONG_ADD
							case ICMD_LADD:
								iptr->opc = ICMD_LADDCONST;
								/* FALLTHROUGH */

							icmd_lconst_tail:
								/* instruction of type LONG -> LONG */
								iptr[1].opc = ICMD_NOP;
								OP1_1(TYPE_LNG, TYPE_LNG);
								STATISTICS(count_pcmd_op++);
								break;

							case ICMD_LSUB:
								iptr->opc = ICMD_LSUBCONST;
								goto icmd_lconst_tail;

#endif /* SUPPORT_LONG_ADD */
#if SUPPORT_LONG_MUL && SUPPORT_CONST_MUL
							case ICMD_LMUL:
								iptr->opc = ICMD_LMULCONST;
								goto icmd_lconst_tail;
#else /* SUPPORT_LONG_MUL && SUPPORT_CONST_MUL */
# if SUPPORT_LONG_SHIFT
							case ICMD_LMUL:
								if (iptr->sx.val.l == 0x00000002)
									iptr->sx.val.i = 1;
								else if (iptr->sx.val.l == 0x00000004)
									iptr->sx.val.i = 2;
								else if (iptr->sx.val.l == 0x00000008)
									iptr->sx.val.i = 3;
								else if (iptr->sx.val.l == 0x00000010)
									iptr->sx.val.i = 4;
								else if (iptr->sx.val.l == 0x00000020)
									iptr->sx.val.i = 5;
								else if (iptr->sx.val.l == 0x00000040)
									iptr->sx.val.i = 6;
								else if (iptr->sx.val.l == 0x00000080)
									iptr->sx.val.i = 7;
								else if (iptr->sx.val.l == 0x00000100)
									iptr->sx.val.i = 8;
								else if (iptr->sx.val.l == 0x00000200)
									iptr->sx.val.i = 9;
								else if (iptr->sx.val.l == 0x00000400)
									iptr->sx.val.i = 10;
								else if (iptr->sx.val.l == 0x00000800)
									iptr->sx.val.i = 11;
								else if (iptr->sx.val.l == 0x00001000)
									iptr->sx.val.i = 12;
								else if (iptr->sx.val.l == 0x00002000)
									iptr->sx.val.i = 13;
								else if (iptr->sx.val.l == 0x00004000)
									iptr->sx.val.i = 14;
								else if (iptr->sx.val.l == 0x00008000)
									iptr->sx.val.i = 15;
								else if (iptr->sx.val.l == 0x00010000)
									iptr->sx.val.i = 16;
								else if (iptr->sx.val.l == 0x00020000)
									iptr->sx.val.i = 17;
								else if (iptr->sx.val.l == 0x00040000)
									iptr->sx.val.i = 18;
								else if (iptr->sx.val.l == 0x00080000)
									iptr->sx.val.i = 19;
								else if (iptr->sx.val.l == 0x00100000)
									iptr->sx.val.i = 20;
								else if (iptr->sx.val.l == 0x00200000)
									iptr->sx.val.i = 21;
								else if (iptr->sx.val.l == 0x00400000)
									iptr->sx.val.i = 22;
								else if (iptr->sx.val.l == 0x00800000)
									iptr->sx.val.i = 23;
								else if (iptr->sx.val.l == 0x01000000)
									iptr->sx.val.i = 24;
								else if (iptr->sx.val.l == 0x02000000)
									iptr->sx.val.i = 25;
								else if (iptr->sx.val.l == 0x04000000)
									iptr->sx.val.i = 26;
								else if (iptr->sx.val.l == 0x08000000)
									iptr->sx.val.i = 27;
								else if (iptr->sx.val.l == 0x10000000)
									iptr->sx.val.i = 28;
								else if (iptr->sx.val.l == 0x20000000)
									iptr->sx.val.i = 29;
								else if (iptr->sx.val.l == 0x40000000)
									iptr->sx.val.i = 30;
								else if (iptr->sx.val.l == 0x80000000)
									iptr->sx.val.i = 31;
								else {
									goto normal_LCONST;
								}
								iptr->opc = ICMD_LMULPOW2;
								goto icmd_lconst_tail;
# endif /* SUPPORT_LONG_SHIFT */
#endif /* SUPPORT_LONG_MUL && SUPPORT_CONST_MUL */
#if SUPPORT_LONG_DIV_POW2
							case ICMD_LDIV:
								if (iptr->sx.val.l == 0x00000002)
									iptr->sx.val.i = 1;
								else if (iptr->sx.val.l == 0x00000004)
									iptr->sx.val.i = 2;
								else if (iptr->sx.val.l == 0x00000008)
									iptr->sx.val.i = 3;
								else if (iptr->sx.val.l == 0x00000010)
									iptr->sx.val.i = 4;
								else if (iptr->sx.val.l == 0x00000020)
									iptr->sx.val.i = 5;
								else if (iptr->sx.val.l == 0x00000040)
									iptr->sx.val.i = 6;
								else if (iptr->sx.val.l == 0x00000080)
									iptr->sx.val.i = 7;
								else if (iptr->sx.val.l == 0x00000100)
									iptr->sx.val.i = 8;
								else if (iptr->sx.val.l == 0x00000200)
									iptr->sx.val.i = 9;
								else if (iptr->sx.val.l == 0x00000400)
									iptr->sx.val.i = 10;
								else if (iptr->sx.val.l == 0x00000800)
									iptr->sx.val.i = 11;
								else if (iptr->sx.val.l == 0x00001000)
									iptr->sx.val.i = 12;
								else if (iptr->sx.val.l == 0x00002000)
									iptr->sx.val.i = 13;
								else if (iptr->sx.val.l == 0x00004000)
									iptr->sx.val.i = 14;
								else if (iptr->sx.val.l == 0x00008000)
									iptr->sx.val.i = 15;
								else if (iptr->sx.val.l == 0x00010000)
									iptr->sx.val.i = 16;
								else if (iptr->sx.val.l == 0x00020000)
									iptr->sx.val.i = 17;
								else if (iptr->sx.val.l == 0x00040000)
									iptr->sx.val.i = 18;
								else if (iptr->sx.val.l == 0x00080000)
									iptr->sx.val.i = 19;
								else if (iptr->sx.val.l == 0x00100000)
									iptr->sx.val.i = 20;
								else if (iptr->sx.val.l == 0x00200000)
									iptr->sx.val.i = 21;
								else if (iptr->sx.val.l == 0x00400000)
									iptr->sx.val.i = 22;
								else if (iptr->sx.val.l == 0x00800000)
									iptr->sx.val.i = 23;
								else if (iptr->sx.val.l == 0x01000000)
									iptr->sx.val.i = 24;
								else if (iptr->sx.val.l == 0x02000000)
									iptr->sx.val.i = 25;
								else if (iptr->sx.val.l == 0x04000000)
									iptr->sx.val.i = 26;
								else if (iptr->sx.val.l == 0x08000000)
									iptr->sx.val.i = 27;
								else if (iptr->sx.val.l == 0x10000000)
									iptr->sx.val.i = 28;
								else if (iptr->sx.val.l == 0x20000000)
									iptr->sx.val.i = 29;
								else if (iptr->sx.val.l == 0x40000000)
									iptr->sx.val.i = 30;
								else if (iptr->sx.val.l == 0x80000000)
									iptr->sx.val.i = 31;
								else {
									goto normal_LCONST;
								}
								iptr->opc = ICMD_LDIVPOW2;
								goto icmd_lconst_tail;
#endif /* SUPPORT_LONG_DIV_POW2 */

#if SUPPORT_LONG_REM_POW2
							case ICMD_LREM:
								if ((iptr->sx.val.l == 0x00000002) ||
									(iptr->sx.val.l == 0x00000004) ||
									(iptr->sx.val.l == 0x00000008) ||
									(iptr->sx.val.l == 0x00000010) ||
									(iptr->sx.val.l == 0x00000020) ||
									(iptr->sx.val.l == 0x00000040) ||
									(iptr->sx.val.l == 0x00000080) ||
									(iptr->sx.val.l == 0x00000100) ||
									(iptr->sx.val.l == 0x00000200) ||
									(iptr->sx.val.l == 0x00000400) ||
									(iptr->sx.val.l == 0x00000800) ||
									(iptr->sx.val.l == 0x00001000) ||
									(iptr->sx.val.l == 0x00002000) ||
									(iptr->sx.val.l == 0x00004000) ||
									(iptr->sx.val.l == 0x00008000) ||
									(iptr->sx.val.l == 0x00010000) ||
									(iptr->sx.val.l == 0x00020000) ||
									(iptr->sx.val.l == 0x00040000) ||
									(iptr->sx.val.l == 0x00080000) ||
									(iptr->sx.val.l == 0x00100000) ||
									(iptr->sx.val.l == 0x00200000) ||
									(iptr->sx.val.l == 0x00400000) ||
									(iptr->sx.val.l == 0x00800000) ||
									(iptr->sx.val.l == 0x01000000) ||
									(iptr->sx.val.l == 0x02000000) ||
									(iptr->sx.val.l == 0x04000000) ||
									(iptr->sx.val.l == 0x08000000) ||
									(iptr->sx.val.l == 0x10000000) ||
									(iptr->sx.val.l == 0x20000000) ||
									(iptr->sx.val.l == 0x40000000) ||
									(iptr->sx.val.l == 0x80000000))
								{
									iptr->opc = ICMD_LREMPOW2;
									iptr->sx.val.l -= 1;
									goto icmd_lconst_tail;
								}
								goto normal_LCONST;
#endif /* SUPPORT_LONG_REM_POW2 */

#if SUPPORT_CONST_LOGICAL

							case ICMD_LAND:
								iptr->opc = ICMD_LANDCONST;
								goto icmd_lconst_tail;

							case ICMD_LOR:
								iptr->opc = ICMD_LORCONST;
								goto icmd_lconst_tail;

							case ICMD_LXOR:
								iptr->opc = ICMD_LXORCONST;
								goto icmd_lconst_tail;
#endif

							case ICMD_LCMP:
								if ((len <= 1) || (iptr[2].sx.val.i != 0))
									goto normal_LCONST;

								/* switch on the instruction after LCONST - LCMP */

								switch (iptr[2].opc) {
									case ICMD_IFEQ:
										iptr->opc = ICMD_IF_LEQ;
										/* FALLTHROUGH */

									icmd_lconst_lcmp_tail:
										/* convert LCONST, LCMP, IFXX to IF_LXX */
										iptr->dst.block = iptr[2].dst.block;
										iptr[1].opc = ICMD_NOP;
										iptr[2].opc = ICMD_NOP;

										OP1_BRANCH(TYPE_LNG);
										BRANCH(tbptr);
										STATISTICS(count_pcmd_bra++);
										STATISTICS(count_pcmd_op++);
										break;

									case ICMD_IFNE:
										iptr->opc = ICMD_IF_LNE;
										goto icmd_lconst_lcmp_tail;

									case ICMD_IFLT:
										iptr->opc = ICMD_IF_LLT;
										goto icmd_lconst_lcmp_tail;

									case ICMD_IFGT:
										iptr->opc = ICMD_IF_LGT;
										goto icmd_lconst_lcmp_tail;

									case ICMD_IFLE:
										iptr->opc = ICMD_IF_LLE;
										goto icmd_lconst_lcmp_tail;

									case ICMD_IFGE:
										iptr->opc = ICMD_IF_LGE;
										goto icmd_lconst_lcmp_tail;

									default:
										goto normal_LCONST;
								} /* end switch on opcode after LCONST - LCMP */
								break;

#if SUPPORT_CONST_STORE
							case ICMD_LASTORE:
# if SUPPORT_CONST_STORE_ZERO_ONLY
								if (iptr->sx.val.l != 0)
									goto normal_LCONST;
# endif
#if SIZEOF_VOID_P == 4
								/* the constant must fit into a ptrint */
								if (iptr->sx.val.l < -0x80000000L || iptr->sx.val.l >= 0x80000000L)
									goto normal_LCONST;
#endif
								/* move the constant to s3 */
								iptr->sx.s23.s3.constval = iptr->sx.val.l;

								iptr->opc = ICMD_LASTORECONST;
								iptr->flags.bits |= INS_FLAG_CHECK;
								OP2_0(TYPE_ADR, TYPE_INT);

								iptr[1].opc = ICMD_NOP;
								STATISTICS(count_pcmd_op++);
								break;

							case ICMD_PUTSTATIC:
							case ICMD_PUTFIELD:
# if SUPPORT_CONST_STORE_ZERO_ONLY
								if (iptr->sx.val.l != 0)
									goto normal_LCONST;
# endif
#if SIZEOF_VOID_P == 4
								/* the constant must fit into a ptrint */
								if (iptr->sx.val.l < -0x80000000L || iptr->sx.val.l >= 0x80000000L)
									goto normal_LCONST;
#endif
								/* XXX check field type? */

								/* copy the constant to s2 */
								/* XXX constval -> fieldconstval? */
								iptr->sx.s23.s2.constval = iptr->sx.val.l;

								goto putconst_tail;

#endif /* SUPPORT_CONST_STORE */

							default:
								goto normal_LCONST;
						} /* end switch opcode after LCONST */

						/* if we get here, the LCONST has been optimized */
						break;

normal_LCONST:
						/* the normal case of an unoptimized LCONST */
						OP0_1(TYPE_LNG);
						break;

	/************************ END OF LCONST OPTIMIZATIONS *********************/

					case ICMD_FCONST:
						STATISTICS(count_pcmd_load++);
						OP0_1(TYPE_FLT);
						break;

					case ICMD_DCONST:
						STATISTICS(count_pcmd_load++);
						OP0_1(TYPE_DBL);
						break;

	/************************** ACONST OPTIMIZATIONS **************************/

					case ICMD_ACONST:
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_pcmd_load++);
#if SUPPORT_CONST_STORE
						/* We can only optimize if the ACONST is resolved
						 * and there is an instruction after it. */

						if ((len == 0) || (iptr->flags.bits & INS_FLAG_UNRESOLVED))
							goto normal_ACONST;

						switch (iptr[1].opc) {
							case ICMD_AASTORE:
								/* We can only optimize for NULL values
								 * here because otherwise a checkcast is
								 * required. */
								if (iptr->sx.val.anyptr != NULL)
									goto normal_ACONST;

								/* copy the constant (NULL) to s3 */
								iptr->sx.s23.s3.constval = 0;
								iptr->opc = ICMD_AASTORECONST;
								iptr->flags.bits |= INS_FLAG_CHECK;
								OP2_0(TYPE_ADR, TYPE_INT);

								iptr[1].opc = ICMD_NOP;
								STATISTICS(count_pcmd_op++);
								break;

							case ICMD_PUTSTATIC:
							case ICMD_PUTFIELD:
# if SUPPORT_CONST_STORE_ZERO_ONLY
								if (iptr->sx.val.anyptr != NULL)
									goto normal_ACONST;
# endif
								/* XXX check field type? */
								/* copy the constant to s2 */
								/* XXX constval -> fieldconstval? */
								iptr->sx.s23.s2.constval = (ptrint) iptr->sx.val.anyptr;

								goto putconst_tail;

							default:
								goto normal_ACONST;
						}

						/* if we get here the ACONST has been optimized */
						break;

normal_ACONST:
#endif /* SUPPORT_CONST_STORE */
						OP0_1(TYPE_ADR);
						break;


						/* pop 0 push 1 load */

					case ICMD_ILOAD:
					case ICMD_LLOAD:
					case ICMD_FLOAD:
					case ICMD_DLOAD:
					case ICMD_ALOAD:
						STATISTICS(count_load_instruction++);
						type = (Type) (opcode - ICMD_ILOAD);

						varindex = iptr->s1.varindex =
							jd->local_map[iptr->s1.varindex * 5 + type];

#if defined(ENABLE_VERIFIER)
						if (sd.var[varindex].type == TYPE_RET) {
							exceptions_throw_verifyerror(m, "forbidden load of returnAddress");
							return false;
						}
#endif
						LOAD(type, varindex);
						break;

						/* pop 2 push 1 */

					case ICMD_LALOAD:
					case ICMD_FALOAD:
					case ICMD_DALOAD:
					case ICMD_AALOAD:
						coalescing_boundary = sd.new_elem;
						iptr->flags.bits |= INS_FLAG_CHECK;
						STATISTICS(count_check_null++);
						STATISTICS(count_check_bound++);
						STATISTICS(count_pcmd_mem++);
						OP2_1(TYPE_ADR, TYPE_INT, opcode - ICMD_IALOAD);
						break;

					case ICMD_IALOAD:
					case ICMD_BALOAD:
					case ICMD_CALOAD:
					case ICMD_SALOAD:
						coalescing_boundary = sd.new_elem;
						iptr->flags.bits |= INS_FLAG_CHECK;
						STATISTICS(count_check_null++);
						STATISTICS(count_check_bound++);
						STATISTICS(count_pcmd_mem++);
						OP2_1(TYPE_ADR, TYPE_INT, TYPE_INT);
						break;

						/* pop 0 push 0 iinc */

					case ICMD_IINC: {
						STATISTICS(count_store_depth[stackdepth]++);
						javaindex = iptr->s1.varindex;
						last_store_boundary[javaindex] = sd.new_elem;

						iptr->s1.varindex =
							jd->local_map[javaindex * 5 + TYPE_INT];

						copy = curstack;
						for (int i = stackdepth - 1; copy; i--, copy = copy->prev) {
							if ((copy->varkind == LOCALVAR) &&
								(jd->reverselocalmap[copy->varnum] == javaindex))
							{
								assert(IS_LOCALVAR(copy));
								SET_TEMPVAR(copy);
							}
						}

						iptr->dst.varindex = iptr->s1.varindex;
						break;
					}

						/* pop 1 push 0 store */

					case ICMD_ISTORE:
					case ICMD_LSTORE:
					case ICMD_FSTORE:
					case ICMD_DSTORE:
					case ICMD_ASTORE:
						REQUIRE(1);

						type = (Type) (opcode - ICMD_ISTORE);
						javaindex = iptr->dst.varindex;
						varindex = iptr->dst.varindex =
							jd->local_map[javaindex * 5 + type];

						COPY_VAL_AND_TYPE(sd, curstack->varnum, varindex);

						iptr->sx.s23.s3.javaindex = javaindex;

						if (curstack->type == TYPE_RET) {
							iptr->flags.bits |= INS_FLAG_RETADDR;
							iptr->sx.s23.s2.retaddrnr =
								JAVALOCAL_FROM_RETADDR(sd.var[varindex].vv.retaddr->nr);
							sd.javalocals[javaindex] = iptr->sx.s23.s2.retaddrnr;
						}
						else
							sd.javalocals[javaindex] = varindex;

						/* invalidate the following javalocal for 2-word types */

						if (IS_2_WORD_TYPE(type)) {
							sd.javalocals[javaindex+1] = jitdata::UNUSED;
							iptr->flags.bits |= INS_FLAG_KILL_NEXT;
						}

						/* invalidate 2-word types if second half was overwritten */

						if (javaindex > 0) {
							int i = sd.javalocals[javaindex-1];

							if (i >= 0 && IS_2_WORD_TYPE(sd.var[i].type)) {
								sd.javalocals[javaindex-1] = jitdata::UNUSED;
								iptr->flags.bits |= INS_FLAG_KILL_PREV;
							}
						}

						STATISTICS(count_pcmd_store++);
						STATISTICS(count_store_depth[stackdepth-1]++);
						STATISTICS(count_store_length[sd.new_elem - curstack]++);

						/* check for conflicts as described in Figure 5.2 */

						copy = curstack->prev;
						for (int i = stackdepth - 2; copy; i--, copy = copy->prev) {
							if ((copy->varkind == LOCALVAR) &&
								(jd->reverselocalmap[copy->varnum] == javaindex))
							{
								assert(IS_LOCALVAR(copy));
								SET_TEMPVAR(copy);
							}							
						}

						/* if the variable is already coalesced, don't bother */

						/* We do not need to check against INOUT, as invars */
						/* are always before the coalescing boundary.        */

						if (curstack->varkind == LOCALVAR)
							goto store_tail;

						/* there is no STORE Lj while curstack is live */

						if (curstack < last_store_boundary[javaindex])
							goto assume_conflict;

						/* curstack must be after the coalescing boundary */

						if (curstack < coalescing_boundary)
							goto assume_conflict;

						/* there is no DEF LOCALVAR(varindex) while curstack is live */

						copy = sd.new_elem; /* most recent stackslot created + 1 */
						while (--copy > curstack) {
							if (copy->varkind == LOCALVAR && jd->reverselocalmap[copy->varnum] == javaindex)
								goto assume_conflict;
						}

						/* coalesce the temporary variable with Lj */
						assert((curstack->varkind == TEMPVAR)
									|| (curstack->varkind == UNDEFVAR));
						assert(!IS_LOCALVAR(curstack)); /* XXX correct? */
						assert(!IS_INOUT(curstack));
						assert(!IS_PREALLOC(curstack));

						assert(curstack->creator);
						assert(curstack->creator->dst.varindex == curstack->varnum);
						assert(!(curstack->flags & PASSTHROUGH));
						RELEASE_INDEX(sd, curstack);
						curstack->varkind = LOCALVAR;
						curstack->varnum = varindex;
						curstack->creator->dst.varindex = varindex;
						goto store_tail;

						/* revert the coalescing, if it has been done earlier */
assume_conflict:
						if ((curstack->varkind == LOCALVAR)
							&& (jd->reverselocalmap[curstack->varnum] == javaindex))
						{
							assert(IS_LOCALVAR(curstack));
							SET_TEMPVAR(curstack);
						}

						/* remember the stack boundary at this store */
store_tail:
						last_store_boundary[javaindex] = sd.new_elem;

						if (opcode == ICMD_ASTORE && curstack->type == TYPE_RET)
							STORE(TYPE_RET, varindex);
						else
							STORE(opcode - ICMD_ISTORE, varindex);
						break;

					/* pop 3 push 0 */

					case ICMD_AASTORE:
						coalescing_boundary = sd.new_elem;
						iptr->flags.bits |= INS_FLAG_CHECK;
						STATISTICS(count_check_null++);
						STATISTICS(count_check_bound++);
						STATISTICS(count_pcmd_mem++);

						bte = builtintable_get_internal(BUILTIN_FAST_canstore);
						md = bte->md;

						if (md->memuse > rd->memuse)
							rd->memuse = md->memuse;
						if (md->argintreguse > rd->argintreguse)
							rd->argintreguse = md->argintreguse;
						/* XXX non-leaf method? */

						/* make all stack variables saved */

						copy = curstack;
						while (copy) {
							sd.var[copy->varnum].flags |= SAVEDVAR;
							/* in case copy->varnum is/will be a LOCALVAR */
							/* once and set back to a non LOCALVAR        */
							/* the correct SAVEDVAR flag has to be        */
							/* remembered in copy->flags, too             */
							copy->flags |= SAVEDVAR;
							copy = copy->prev;
						}

						OP3_0(TYPE_ADR, TYPE_INT, TYPE_ADR);
						break;


					case ICMD_LASTORE:
					case ICMD_FASTORE:
					case ICMD_DASTORE:
						coalescing_boundary = sd.new_elem;
						iptr->flags.bits |= INS_FLAG_CHECK;
						STATISTICS(count_check_null++);
						STATISTICS(count_check_bound++);
						STATISTICS(count_pcmd_mem++);
						OP3_0(TYPE_ADR, TYPE_INT, opcode - ICMD_IASTORE);
						break;

					case ICMD_IASTORE:
					case ICMD_BASTORE:
					case ICMD_CASTORE:
					case ICMD_SASTORE:
						coalescing_boundary = sd.new_elem;
						iptr->flags.bits |= INS_FLAG_CHECK;
						STATISTICS(count_check_null++);
						STATISTICS(count_check_bound++);
						STATISTICS(count_pcmd_mem++);
						OP3_0(TYPE_ADR, TYPE_INT, TYPE_INT);
						break;

						/* pop 1 push 0 */

					case ICMD_POP:
#ifdef ENABLE_VERIFIER
						if (opt_verify) {
							REQUIRE(1);
							if (IS_2_WORD_TYPE(curstack->type))
								return throw_stack_category_error();
						}
#endif
						OP1_0_ANY;
						break;

					case ICMD_IRETURN:
					case ICMD_LRETURN:
					case ICMD_FRETURN:
					case ICMD_DRETURN:
					case ICMD_ARETURN:
						coalescing_boundary = sd.new_elem;
						/* Assert here that no LOCAL or INOUTS get */
						/* preallocated, since tha macros are not   */
						/* available in md-abi.c! */
						if (IS_TEMPVAR(curstack))
							md_return_alloc(jd, curstack);
						STATISTICS(count_pcmd_return++);
						OP1_0(opcode - ICMD_IRETURN);
						superblockend = true;
						sd.jd->returncount++;
						sd.jd->returnblock = sd.bptr;
						break;

					case ICMD_ATHROW:
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_check_null++);
						OP1_0(TYPE_ADR);
						curstack = NULL; stackdepth = 0;
						superblockend = true;
						break;

					case ICMD_PUTSTATIC: {
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_pcmd_mem++);
						constant_FMIref *fmiref;
						INSTRUCTION_GET_FIELDREF(iptr, fmiref);
						OP1_0(fmiref->parseddesc.fd->type);
						break;
					}

						/* pop 1 push 0 branch */

					case ICMD_IFNULL:
					case ICMD_IFNONNULL:
						STATISTICS(count_pcmd_bra++);
						OP1_BRANCH(TYPE_ADR);
						BRANCH(tbptr);
						break;

					case ICMD_IFEQ:
					case ICMD_IFNE:
					case ICMD_IFLT:
					case ICMD_IFGE:
					case ICMD_IFGT:
					case ICMD_IFLE:
						STATISTICS(count_pcmd_bra++);
						/* iptr->sx.val.i is set implicitly in parse by
						   clearing the memory or from IF_ICMPxx
						   optimization. */

						OP1_BRANCH(TYPE_INT);
/* 						iptr->sx.val.i = 0; */
						BRANCH(tbptr);
						break;

						/* pop 0 push 0 branch */

					case ICMD_GOTO:
						STATISTICS(count_pcmd_bra++);
						OP0_BRANCH;
						BRANCH(tbptr);
						superblockend = true;
						break;

						/* pop 1 push 0 table branch */

					case ICMD_TABLESWITCH: {
						STATISTICS(count_pcmd_table++);
						OP1_BRANCH(TYPE_INT);

						table = iptr->dst.table;
						BRANCH_TARGET(*table, tbptr);
						table++;

						int i = iptr->sx.s23.s3.tablehigh
						      - iptr->sx.s23.s2.tablelow + 1;

						while (--i >= 0) {
							BRANCH_TARGET(*table, tbptr);
							table++;
						}
						superblockend = true;
						break;
					}

						/* pop 1 push 0 table branch */

					case ICMD_LOOKUPSWITCH: {
						STATISTICS(count_pcmd_table++);
						OP1_BRANCH(TYPE_INT);

						BRANCH_TARGET(iptr->sx.s23.s3.lookupdefault, tbptr);

						lookup = iptr->dst.lookup;

						int i = iptr->sx.s23.s2.lookupcount;

						while (--i >= 0) {
							BRANCH_TARGET(lookup->target, tbptr);
							lookup++;
						}
						superblockend = true;
						break;
					}

					case ICMD_MONITORENTER:
					case ICMD_MONITOREXIT:
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_check_null++);
						OP1_0(TYPE_ADR);
						break;

						/* pop 2 push 0 branch */

					case ICMD_IF_ICMPEQ:
					case ICMD_IF_ICMPNE:
					case ICMD_IF_ICMPLT:
					case ICMD_IF_ICMPGE:
					case ICMD_IF_ICMPGT:
					case ICMD_IF_ICMPLE:
						STATISTICS(count_pcmd_bra++);
						OP2_BRANCH(TYPE_INT, TYPE_INT);
						BRANCH(tbptr);
						break;

					case ICMD_IF_ACMPEQ:
					case ICMD_IF_ACMPNE:
						STATISTICS(count_pcmd_bra++);
						OP2_BRANCH(TYPE_ADR, TYPE_ADR);
						BRANCH(tbptr);
						break;

						/* pop 2 push 0 */

					case ICMD_PUTFIELD: {
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_check_null++);
						STATISTICS(count_pcmd_mem++);
						constant_FMIref *fmiref;
						INSTRUCTION_GET_FIELDREF(iptr, fmiref);
						OP2_0(TYPE_ADR, fmiref->parseddesc.fd->type);
						break;
					}

					case ICMD_POP2:
						REQUIRE(1);
						if (!IS_2_WORD_TYPE(curstack->type)) {
							/* ..., cat1 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								REQUIRE(2);
								if (IS_2_WORD_TYPE(curstack->prev->type))
									return throw_stack_category_error();
							}
#endif
							OP2_0_ANY_ANY; /* pop two slots */
						}
						else {
							iptr->opc = ICMD_POP;
							OP1_0_ANY; /* pop one (two-word) slot */
						}
						break;

						/* pop 0 push 1 dup */

					case ICMD_DUP:
#ifdef ENABLE_VERIFIER
						if (opt_verify) {
							REQUIRE(1);
							if (IS_2_WORD_TYPE(curstack->type))
								return throw_stack_category_error();
						}
#endif
						STATISTICS(count_dup_instruction++);

icmd_DUP:
						src1 = curstack;

						COPY_UP(src1);
						coalescing_boundary = sd.new_elem - 1;
						break;

					case ICMD_DUP2:
						REQUIRE(1);
						if (IS_2_WORD_TYPE(curstack->type)) {
							/* ..., cat2 */
							iptr->opc = ICMD_DUP;
							goto icmd_DUP;
						}
						else {
							REQUIRE(2);
							/* ..., ????, cat1 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								if (IS_2_WORD_TYPE(curstack->prev->type))
									return throw_stack_category_error();
							}
#endif
							src1 = curstack->prev;
							src2 = curstack;

							COPY_UP(src1); iptr++; len--;
							COPY_UP(src2);

							coalescing_boundary = sd.new_elem;
						}
						break;

						/* pop 2 push 3 dup */

					case ICMD_DUP_X1:
#ifdef ENABLE_VERIFIER
						if (opt_verify) {
							REQUIRE(2);
							if (IS_2_WORD_TYPE(curstack->type) ||
								IS_2_WORD_TYPE(curstack->prev->type))
									return throw_stack_category_error();
						}
#endif

icmd_DUP_X1:
						src1 = curstack->prev;
						src2 = curstack;
						POPANY; POPANY;
						stackdepth -= 2;

						/* move non-temporary sources out of the way */
						if (!IS_TEMPVAR(src2)) {
							MOVE_TO_TEMP(src2); iptr++; len--;
						}

						DUP_SLOT(src2); dst1 = curstack; stackdepth++;

						MOVE_UP(src1); iptr++; len--;
						MOVE_UP(src2); iptr++; len--;

						COPY_DOWN(curstack, dst1);

						coalescing_boundary = sd.new_elem;
						break;

					case ICMD_DUP2_X1:
						REQUIRE(2);
						if (IS_2_WORD_TYPE(curstack->type)) {
							/* ..., ????, cat2 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								if (IS_2_WORD_TYPE(curstack->prev->type))
									return throw_stack_category_error();
							}
#endif
							iptr->opc = ICMD_DUP_X1;
							goto icmd_DUP_X1;
						}
						else {
							/* ..., ????, cat1 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								REQUIRE(3);
								if (IS_2_WORD_TYPE(curstack->prev->type)
									|| IS_2_WORD_TYPE(curstack->prev->prev->type))
										return throw_stack_category_error();
							}
#endif

icmd_DUP2_X1:
							src1 = curstack->prev->prev;
							src2 = curstack->prev;
							src3 = curstack;
							POPANY; POPANY; POPANY;
							stackdepth -= 3;

							/* move non-temporary sources out of the way */
							if (!IS_TEMPVAR(src2)) {
								MOVE_TO_TEMP(src2); iptr++; len--;
							}
							if (!IS_TEMPVAR(src3)) {
								MOVE_TO_TEMP(src3); iptr++; len--;
							}

							DUP_SLOT(src2); dst1 = curstack; stackdepth++;
							DUP_SLOT(src3); dst2 = curstack; stackdepth++;

							MOVE_UP(src1); iptr++; len--;
							MOVE_UP(src2); iptr++; len--;
							MOVE_UP(src3); iptr++; len--;

							COPY_DOWN(curstack, dst2); iptr++; len--;
							COPY_DOWN(curstack->prev, dst1);

							coalescing_boundary = sd.new_elem;
						}
						break;

						/* pop 3 push 4 dup */

					case ICMD_DUP_X2:
						REQUIRE(2);
						if (IS_2_WORD_TYPE(curstack->prev->type)) {
							/* ..., cat2, ???? */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								if (IS_2_WORD_TYPE(curstack->type))
									return throw_stack_category_error();
							}
#endif
							iptr->opc = ICMD_DUP_X1;
							goto icmd_DUP_X1;
						}
						else {
							/* ..., cat1, ???? */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								REQUIRE(3);
								if (IS_2_WORD_TYPE(curstack->type)
									|| IS_2_WORD_TYPE(curstack->prev->prev->type))
											return throw_stack_category_error();
							}
#endif
icmd_DUP_X2:
							src1 = curstack->prev->prev;
							src2 = curstack->prev;
							src3 = curstack;
							POPANY; POPANY; POPANY;
							stackdepth -= 3;

							/* move non-temporary sources out of the way */
							if (!IS_TEMPVAR(src2)) {
								MOVE_TO_TEMP(src2); iptr++; len--;
							}
							if (!IS_TEMPVAR(src3)) {
								MOVE_TO_TEMP(src3); iptr++; len--;
							}

							DUP_SLOT(src3); dst1 = curstack; stackdepth++;

							MOVE_UP(src1); iptr++; len--;
							MOVE_UP(src2); iptr++; len--;
							MOVE_UP(src3); iptr++; len--;

							COPY_DOWN(curstack, dst1);

							coalescing_boundary = sd.new_elem;
						}
						break;

					case ICMD_DUP2_X2:
						REQUIRE(2);
						if (IS_2_WORD_TYPE(curstack->type)) {
							/* ..., ????, cat2 */
							if (IS_2_WORD_TYPE(curstack->prev->type)) {
								/* ..., cat2, cat2 */
								iptr->opc = ICMD_DUP_X1;
								goto icmd_DUP_X1;
							}
							else {
								/* ..., cat1, cat2 */
#ifdef ENABLE_VERIFIER
								if (opt_verify) {
									REQUIRE(3);
									if (IS_2_WORD_TYPE(curstack->prev->prev->type))
										return throw_stack_category_error();
								}
#endif
								iptr->opc = ICMD_DUP_X2;
								goto icmd_DUP_X2;
							}
						}

						REQUIRE(3);
						/* ..., ????, ????, cat1 */

						if (IS_2_WORD_TYPE(curstack->prev->prev->type)) {
							/* ..., cat2, ????, cat1 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								if (IS_2_WORD_TYPE(curstack->prev->type))
									return throw_stack_category_error();
							}
#endif
							iptr->opc = ICMD_DUP2_X1;
							goto icmd_DUP2_X1;
						}
						else {
							/* ..., cat1, ????, cat1 */
#ifdef ENABLE_VERIFIER
							if (opt_verify) {
								REQUIRE(4);
								if (IS_2_WORD_TYPE(curstack->prev->type)
									|| IS_2_WORD_TYPE(curstack->prev->prev->prev->type))
									return throw_stack_category_error();
							}
#endif

							src1 = curstack->prev->prev->prev;
							src2 = curstack->prev->prev;
							src3 = curstack->prev;
							src4 = curstack;
							POPANY; POPANY; POPANY; POPANY;
							stackdepth -= 4;

							/* move non-temporary sources out of the way */
							if (!IS_TEMPVAR(src2)) {
								MOVE_TO_TEMP(src2); iptr++; len--;
							}
							if (!IS_TEMPVAR(src3)) {
								MOVE_TO_TEMP(src3); iptr++; len--;
							}
							if (!IS_TEMPVAR(src4)) {
								MOVE_TO_TEMP(src4); iptr++; len--;
							}

							DUP_SLOT(src3); dst1 = curstack; stackdepth++;
							DUP_SLOT(src4); dst2 = curstack; stackdepth++;

							MOVE_UP(src1); iptr++; len--;
							MOVE_UP(src2); iptr++; len--;
							MOVE_UP(src3); iptr++; len--;
							MOVE_UP(src4); iptr++; len--;

							COPY_DOWN(curstack, dst2); iptr++; len--;
							COPY_DOWN(curstack->prev, dst1);

							coalescing_boundary = sd.new_elem;
						}
						break;

						/* pop 2 push 2 swap */

					case ICMD_SWAP:
#ifdef ENABLE_VERIFIER
						if (opt_verify) {
							REQUIRE(2);
							if (IS_2_WORD_TYPE(curstack->type)
								|| IS_2_WORD_TYPE(curstack->prev->type))
								return throw_stack_category_error();
						}
#endif

						src1 = curstack->prev;
						src2 = curstack;
						POPANY; POPANY;
						stackdepth -= 2;

						/* move non-temporary sources out of the way */
						if (!IS_TEMPVAR(src1)) {
							MOVE_TO_TEMP(src1); iptr++; len--;
						}

						MOVE_UP(src2); iptr++; len--;
						MOVE_UP(src1);

						coalescing_boundary = sd.new_elem;
						break;

						/* pop 2 push 1 */

					case ICMD_IDIV:
					case ICMD_IREM:
						coalescing_boundary = sd.new_elem;
#if !SUPPORT_DIVISION
						bte = iptr->sx.s23.s3.bte;
						md = bte->md;

						if (md->memuse > rd->memuse)
							rd->memuse = md->memuse;
						if (md->argintreguse > rd->argintreguse)
							rd->argintreguse = md->argintreguse;

						/* make all stack variables saved */

						copy = curstack;
						while (copy) {
							sd.var[copy->varnum].flags |= SAVEDVAR;
							copy->flags |= SAVEDVAR;
							copy = copy->prev;
						}
						/* FALLTHROUGH */

#endif /* !SUPPORT_DIVISION */

					case ICMD_ISHL:
					case ICMD_ISHR:
					case ICMD_IUSHR:
					case ICMD_IADD:
					case ICMD_ISUB:
					case ICMD_IMUL:
					case ICMD_IAND:
					case ICMD_IOR:
					case ICMD_IXOR:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_INT, TYPE_INT, TYPE_INT);
						break;

					case ICMD_LDIV:
					case ICMD_LREM:
						coalescing_boundary = sd.new_elem;
#if !(SUPPORT_DIVISION && SUPPORT_LONG_DIV)
						bte = iptr->sx.s23.s3.bte;
						md = bte->md;

						if (md->memuse > rd->memuse)
							rd->memuse = md->memuse;
						if (md->argintreguse > rd->argintreguse)
							rd->argintreguse = md->argintreguse;
						/* XXX non-leaf method? */

						/* make all stack variables saved */

						copy = curstack;
						while (copy) {
							sd.var[copy->varnum].flags |= SAVEDVAR;
							copy->flags |= SAVEDVAR;
							copy = copy->prev;
						}
						/* FALLTHROUGH */

#endif

					case ICMD_LMUL:
					case ICMD_LADD:
					case ICMD_LSUB:
					case ICMD_LAND:
					case ICMD_LOR:
					case ICMD_LXOR:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_LNG, TYPE_LNG, TYPE_LNG);
						break;

					case ICMD_LSHL:
					case ICMD_LSHR:
					case ICMD_LUSHR:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_LNG, TYPE_INT, TYPE_LNG);
						break;

					case ICMD_FADD:
					case ICMD_FSUB:
					case ICMD_FMUL:
					case ICMD_FDIV:
					case ICMD_FREM:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_FLT, TYPE_FLT, TYPE_FLT);
						break;

					case ICMD_DADD:
					case ICMD_DSUB:
					case ICMD_DMUL:
					case ICMD_DDIV:
					case ICMD_DREM:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_DBL, TYPE_DBL, TYPE_DBL);
						break;

					case ICMD_LCMP:
						STATISTICS(count_pcmd_op++);
						if ((len == 0) || (iptr[1].sx.val.i != 0))
							goto normal_LCMP;

						switch (iptr[1].opc) {
						case ICMD_IFEQ:
							iptr->opc = ICMD_IF_LCMPEQ;
						icmd_lcmp_if_tail:
							iptr->dst.block = iptr[1].dst.block;
							iptr[1].opc = ICMD_NOP;

							OP2_BRANCH(TYPE_LNG, TYPE_LNG);
							BRANCH(tbptr);

							STATISTICS(count_pcmd_bra++);
							break;
						case ICMD_IFNE:
							iptr->opc = ICMD_IF_LCMPNE;
							goto icmd_lcmp_if_tail;
						case ICMD_IFLT:
							iptr->opc = ICMD_IF_LCMPLT;
							goto icmd_lcmp_if_tail;
						case ICMD_IFGT:
							iptr->opc = ICMD_IF_LCMPGT;
							goto icmd_lcmp_if_tail;
						case ICMD_IFLE:
							iptr->opc = ICMD_IF_LCMPLE;
							goto icmd_lcmp_if_tail;
						case ICMD_IFGE:
							iptr->opc = ICMD_IF_LCMPGE;
							goto icmd_lcmp_if_tail;
						default:
							goto normal_LCMP;
						}
						break;
normal_LCMP:
						bte = builtintable_get_internal(BUILTIN_lcmp);

						iptr->opc            = ICMD_BUILTIN;
						iptr->flags.bits    &= INS_FLAG_ID_MASK;
						iptr->sx.s23.s3.bte  = bte;
						/* iptr->line is already set */
						code_unflag_leafmethod(code);
						goto icmd_BUILTIN;

						break;

					case ICMD_FCMPL:
					case ICMD_FCMPG:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_FLT, TYPE_FLT, TYPE_INT);
						break;

					case ICMD_DCMPL:
					case ICMD_DCMPG:
						STATISTICS(count_pcmd_op++);
						OP2_1(TYPE_DBL, TYPE_DBL, TYPE_INT);
						break;

						/* pop 1 push 1 */

					case ICMD_INEG:
					case ICMD_INT2BYTE:
					case ICMD_INT2CHAR:
					case ICMD_INT2SHORT:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_INT, TYPE_INT);
						break;
					case ICMD_LNEG:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_LNG, TYPE_LNG);
						break;
					case ICMD_FNEG:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_FLT, TYPE_FLT);
						break;
					case ICMD_DNEG:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_DBL, TYPE_DBL);
						break;

					case ICMD_I2L:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_INT, TYPE_LNG);
						break;
					case ICMD_I2F:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_INT, TYPE_FLT);
						break;
					case ICMD_I2D:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_INT, TYPE_DBL);
						break;
					case ICMD_L2I:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_LNG, TYPE_INT);
						break;
					case ICMD_L2F:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_LNG, TYPE_FLT);
						break;
					case ICMD_L2D:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_LNG, TYPE_DBL);
						break;
					case ICMD_F2I:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_FLT, TYPE_INT);
						break;
					case ICMD_F2L:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_FLT, TYPE_LNG);
						break;
					case ICMD_F2D:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_FLT, TYPE_DBL);
						break;
					case ICMD_D2I:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_DBL, TYPE_INT);
						break;
					case ICMD_D2L:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_DBL, TYPE_LNG);
						break;
					case ICMD_D2F:
						STATISTICS(count_pcmd_op++);
						OP1_1(TYPE_DBL, TYPE_FLT);
						break;

					case ICMD_CHECKCAST:
						coalescing_boundary = sd.new_elem;
						if (iptr->flags.bits & INS_FLAG_ARRAY) {
							/* array type cast-check */

							bte = builtintable_get_internal(BUILTIN_arraycheckcast);
							md = bte->md;

							if (md->memuse > rd->memuse)
								rd->memuse = md->memuse;
							if (md->argintreguse > rd->argintreguse)
								rd->argintreguse = md->argintreguse;

							/* make all stack variables saved */

							copy = curstack;
							while (copy) {
								sd.var[copy->varnum].flags |= SAVEDVAR;
								copy->flags |= SAVEDVAR;
								copy = copy->prev;
							}
						}
						OP1_1(TYPE_ADR, TYPE_ADR);
						break;

					case ICMD_INSTANCEOF:
					case ICMD_ARRAYLENGTH:
						coalescing_boundary = sd.new_elem;
						OP1_1(TYPE_ADR, TYPE_INT);
						break;

					case ICMD_NEWARRAY:
					case ICMD_ANEWARRAY:
						coalescing_boundary = sd.new_elem;
						OP1_1(TYPE_INT, TYPE_ADR);
						break;

					case ICMD_GETFIELD: {
						coalescing_boundary = sd.new_elem;
						STATISTICS(count_check_null++);
						STATISTICS(count_pcmd_mem++);
						constant_FMIref *fmiref;
						INSTRUCTION_GET_FIELDREF(iptr, fmiref);
						OP1_1(TYPE_ADR, fmiref->parseddesc.fd->type);
						break;
					}

						/* pop 0 push 1 */

					case ICMD_GETSTATIC: {
 						coalescing_boundary = sd.new_elem;
						STATISTICS(count_pcmd_mem++);
						constant_FMIref *fmiref;
						INSTRUCTION_GET_FIELDREF(iptr, fmiref);
						OP0_1(fmiref->parseddesc.fd->type);
						break;
					}

					case ICMD_NEW:
 						coalescing_boundary = sd.new_elem;
						OP0_1(TYPE_ADR);
						break;

					case ICMD_JSR:
						OP0_1(TYPE_RET);

						tbptr = iptr->sx.s23.s3.jsrtarget.block;
						tbptr->type = basicblock::TYPE_SBR;

						assert(sd.bptr->next);  /* XXX exception */
						sd.var[curstack->varnum].vv.retaddr = sd.bptr->next;
#if defined(ENABLE_VERIFIER)
						sd.var[curstack->varnum].SBRSTART = (void*) tbptr;
#endif

						tbptr = stack_mark_reached(&sd, tbptr, curstack, stackdepth);
						if (tbptr == NULL)
							return false;

						iptr->sx.s23.s3.jsrtarget.block = tbptr;

						/* We need to check for overflow right here because
						 * the pushed value is poped afterwards */
						CHECKOVERFLOW;

						superblockend = true;
						/* XXX should not be marked as interface, as it does not need to be */
						/* allocated. Same for the invar of the target. */
						break;

					/* pop many push any */

					case ICMD_BUILTIN:
					icmd_BUILTIN: {
						builtintable_entry *bte = iptr->sx.s23.s3.bte;
						md = bte->md;
						goto _callhandling;
					}

					case ICMD_INVOKESTATIC:
					case ICMD_INVOKESPECIAL:
					case ICMD_INVOKEVIRTUAL:
					case ICMD_INVOKEINTERFACE:
						STATISTICS(count_pcmd_met++);

						/* Check for functions to replace with builtin
						 * functions. */

						if (builtintable_replace_function(iptr))
							goto icmd_BUILTIN;

						INSTRUCTION_GET_METHODDESC(iptr, md);
						/* XXX resurrect this COUNT? */
/*                          if (lm->flags & ACC_STATIC) */
/*                              {COUNT(count_check_null);} */

					_callhandling: {

						coalescing_boundary = sd.new_elem;

						const int paramcount = md->paramcount;

						if (md->memuse > rd->memuse)
							rd->memuse = md->memuse;
						if (md->argintreguse > rd->argintreguse)
							rd->argintreguse = md->argintreguse;
						if (md->argfltreguse > rd->argfltreguse)
							rd->argfltreguse = md->argfltreguse;

						REQUIRE(paramcount);

						iptr->s1.argcount = stackdepth;
						iptr->sx.s23.s2.args = DMNEW(s4, stackdepth);

						copy = curstack;
						for (int i = paramcount - 1; i >= 0; i--) {
							iptr->sx.s23.s2.args[i] = copy->varnum;

							/* do not change STACKVARs or LOCALVARS to ARGVAR*/
							/* ->  won't help anyway */
							if (!(IS_INOUT(copy) || IS_LOCALVAR(copy))) {

#if defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
			/* If we pass float arguments in integer argument registers, we
			 * are not allowed to precolor them here. Floats have to be moved
			 * to this regs explicitly in codegen().
			 * Only arguments that are passed by stack anyway can be precolored
			 * (michi 2005/07/24) */
							if (!(sd.var[copy->varnum].flags & SAVEDVAR) &&
							   (!IS_FLT_DBL_TYPE(copy->type)
								|| md->params[i].inmemory)) {
#else
							if (!(sd.var[copy->varnum].flags & SAVEDVAR)) {
#endif

								SET_PREALLOC(copy);

								if (md->params[i].inmemory) {
									sd.var[copy->varnum].vv.regoff =
										md->params[i].regoff;
									sd.var[copy->varnum].flags |=
										INMEMORY;
								}
								else {
									if (IS_FLT_DBL_TYPE(copy->type)) {
#if defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
										assert(0); /* XXX is this assert ok? */
#else
										sd.var[copy->varnum].vv.regoff =
											md->params[i].regoff;
#endif /* SUPPORT_PASS_FLOATARGS_IN_INTREGS */
									}
									else {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
										if (IS_2_WORD_TYPE(copy->type))
											sd.var[copy->varnum].vv.regoff =
			PACK_REGS(GET_LOW_REG(md->params[i].regoff),
					  GET_HIGH_REG(md->params[i].regoff));

										else
#endif /* SUPPORT_COMBINE_INTEGER_REGISTERS */
											sd.var[copy->varnum].vv.regoff =
												md->params[i].regoff;
									}
								}
							}
							}
							copy = copy->prev;
						}

						/* deal with live-through stack slots "under" the */
						/* arguments */

						for (int i = paramcount; copy; i++) {
							iptr->sx.s23.s2.args[i]     = copy->varnum;
							sd.var[copy->varnum].flags |= SAVEDVAR;
							copy->flags                |= SAVEDVAR | PASSTHROUGH;
							copy                        = copy->prev;
						}

						/* pop the arguments */

						int i = paramcount;

						stackdepth -= i;
						while (--i >= 0) {
							POPANY;
						}

						/* push the return value */

						if (md->returntype.type != TYPE_VOID) {
							GET_NEW_VAR(sd, new_index, md->returntype.type);
							DST(md->returntype.type, new_index);
							stackdepth++;
						}
						break;
					}

					case ICMD_MULTIANEWARRAY: {
						coalescing_boundary = sd.new_elem;
						if (rd->argintreguse < MIN(3, INT_ARG_CNT))
							rd->argintreguse = MIN(3, INT_ARG_CNT);

						int i = iptr->s1.argcount;

						REQUIRE(i);

						iptr->sx.s23.s2.args = DMNEW(s4, i);

#if defined(SPECIALMEMUSE)
# if defined(__DARWIN__)
						if (rd->memuse < (i + INT_ARG_CNT +LA_SIZE_IN_POINTERS))
							rd->memuse = i + LA_SIZE_IN_POINTERS + INT_ARG_CNT;
# else
						if (rd->memuse < (i + LA_SIZE_IN_POINTERS + 3))
							rd->memuse = i + LA_SIZE_IN_POINTERS + 3;
# endif
#else
# if defined(__I386__)
						if (rd->memuse < i + 3)
							rd->memuse = i + 3; /* n integer args spilled on stack */
# elif defined(__MIPS__) && SIZEOF_VOID_P == 4
						if (rd->memuse < i + 2)
							rd->memuse = i + 2; /* 4*4 bytes callee save space */
# else
						if (rd->memuse < i)
							rd->memuse = i; /* n integer args spilled on stack */
# endif /* defined(__I386__) */
#endif
						copy = curstack;
						while (--i >= 0) {
					/* check INT type here? Currently typecheck does this. */
							iptr->sx.s23.s2.args[i] = copy->varnum;
							if (!(sd.var[copy->varnum].flags & SAVEDVAR)
								&& (!IS_INOUT(copy))
								&& (!IS_LOCALVAR(copy)) ) {
								copy->varkind = ARGVAR;
								sd.var[copy->varnum].flags |=
									INMEMORY & PREALLOC;
#if defined(SPECIALMEMUSE)
# if defined(__DARWIN__)
								sd.var[copy->varnum].vv.regoff = i +
									LA_SIZE_IN_POINTERS + INT_ARG_CNT;
# else
								sd.var[copy->varnum].vv.regoff = i +
									LA_SIZE_IN_POINTERS + 3;
# endif
#else
# if defined(__I386__)
								sd.var[copy->varnum].vv.regoff = i + 3;
# elif defined(__MIPS__) && SIZEOF_VOID_P == 4
								sd.var[copy->varnum].vv.regoff = i + 2;
# else
								sd.var[copy->varnum].vv.regoff = i;
# endif /* defined(__I386__) */
#endif /* defined(SPECIALMEMUSE) */
							}
							copy = copy->prev;
						}
						while (copy) {
							sd.var[copy->varnum].flags |= SAVEDVAR;
							copy->flags |= SAVEDVAR;
							copy = copy->prev;
						}

						i = iptr->s1.argcount;
						stackdepth -= i;
						while (--i >= 0) {
							POPANY;
						}
						GET_NEW_VAR(sd, new_index, TYPE_ADR);
						DST(TYPE_ADR, new_index);
						stackdepth++;
						break;
					}

					default:
						exceptions_throw_internalerror("Unknown ICMD %d during stack analysis",
													   opcode);
						return false;
					} /* switch */

					CHECKOVERFLOW;
					iptr++;
				} /* while instructions */

				/* show state after last instruction */

#if defined(STACK_VERBOSE)
				stack_verbose_show_state(&sd, NULL, curstack);
#endif

				/* stack slots at basic block end become interfaces */

				sd.bptr->outdepth = stackdepth;
				sd.bptr->outvars = DMNEW(s4, stackdepth);

				int i = stackdepth - 1;
				for (copy = curstack; copy; i--, copy = copy->prev) {
					varinfo *v;

					/* with the new vars rd->interfaces will be removed */
					/* and all in and outvars have to be STACKVARS!     */
					/* in the moment i.e. SWAP with in and out vars can */
					/* create an unresolvable conflict */

					SET_TEMPVAR(copy);
					type = copy->type;

					v = sd.var + copy->varnum;
					v->flags |= INOUT;

					/* do not allocate variables for returnAddresses */

					if (type != TYPE_RET) {
						if (jd->interface_map[i*5 + type].flags == jitdata::UNUSED) {
							/* no interface var until now for this depth and */
							/* type */
							jd->interface_map[i*5 + type].flags = v->flags;
						}
						else {
							jd->interface_map[i*5 + type].flags |= v->flags;
						}
					}

					sd.bptr->outvars[i] = copy->varnum;
				}

				/* check if interface slots at basic block begin must be saved */

				for (int i=0; i<sd.bptr->indepth; ++i) {
					varinfo *v = sd.var + sd.bptr->invars[i];

					type = v->type;

					if (type != TYPE_RET) {
						if (jd->interface_map[i*5 + type].flags == jitdata::UNUSED) {
							/* no interface var until now for this depth and */
							/* type */
							jd->interface_map[i*5 + type].flags = v->flags;
						}
						else {
							jd->interface_map[i*5 + type].flags |= v->flags;
						}
					}
				}

				/* store the number of this block's variables */

				sd.bptr->varcount = sd.vartop - sd.bptr->varstart;

#if defined(STACK_VERBOSE)
				stack_verbose_block_exit(&sd, superblockend);
#endif

				/* reach the following block, if any */

				if (!superblockend)
					if (!stack_reach_next_block(&sd))
						return false;

		} /* for blocks */

	} while (sd.repeat && !deadcode);

    /* reset locals of TYPE_RET|VOID to TYPE_ADR */

    /* A local variable may be used as both a returnAddress and a reference */
    /* type variable, as we do not split variables between these types when */
    /* renaming locals. While returnAddresses have been eliminated now, we  */
    /* must assume that the variable is still used as TYPE_ADR.             */
    /* The only way that a local can be TYPE_VOID at this point, is that it */
    /* was a TYPE_RET variable for which incompatible returnAddresses were  */
    /* merged. Thus we must treat TYPE_VOID in the same way as TYPE_RET     */
    /* here.                                                                */
    /* XXX: It would be nice to remove otherwise unused returnAddress       */
    /*      variables from the local variable array, so they are not        */
    /*      allocated by simplereg. (For LSRA this is not needed).          */

	for (int i=0; i<sd.localcount; ++i) {
		if (sd.var[i].type == TYPE_RET || sd.var[i].type == TYPE_VOID)
			sd.var[i].type = TYPE_ADR;
	}

	/* mark temporaries of TYPE_RET as PREALLOC to avoid allocation */

	for (int i=sd.localcount; i<sd.vartop; ++i) {
		if (sd.var[i].type == TYPE_RET)
			sd.var[i].flags |= PREALLOC;
	}

	/* XXX hack to fix up the ranges of the cloned single-block handlers */

	for (exception_entry *ex = jd->exceptiontable; ex != NULL; ex = ex->down) {
		if (ex->start == ex->end) {
			assert(ex->end->next);
			ex->end = ex->end->next;
		}
	}

	/* store number of created variables */

	jd->vartop = sd.vartop;

	/* gather statistics *****************************************************/

	STATISTICS(count_max_basic_blocks.max(jd->basicblockcount));
	STATISTICS(count_basic_blocks += jd->basicblockcount);
	STATISTICS(count_max_javainstr.max(jd->instructioncount));
	STATISTICS(count_javainstr += jd->instructioncount);
	STATISTICS(count_upper_bound_new_stack.max(jd->stackcount));
	STATISTICS(count_max_new_stack.max(sd.new_elem - jd->stack));

	STATISTICS(count_analyse_iterations[iteration_count]++);
	STATISTICS(count_method_bb_distribution[jd->basicblockcount]++);
#if defined(ENABLE_STATISTICS)
	sd.bptr = jd->basicblocks;
	for (; sd.bptr; sd.bptr = sd.bptr->next) {
		if (sd.bptr->state > basicblock::REACHED) {
			STATISTICS(count_block_stack[sd.bptr->indepth]++);
			STATISTICS(count_block_size_distribution[sd.bptr->icount]++);
		}
	}
#endif /* defined(ENABLE_STATISTICS) */

	/* everything's ok *******************************************************/

	return true;
}


/* stack_javalocals_store ******************************************************

   Model the effect of a ?STORE instruction upon the given javalocals array.

   IN:
       iptr.............the ?STORE instruction
	   javalocals.......the javalocals array to modify

*******************************************************************************/

void stack_javalocals_store(instruction *iptr, s4 *javalocals)
{
	s4 varindex  = iptr->dst.varindex;         // index into the jd->var array
	s4 javaindex = iptr->sx.s23.s3.javaindex;  // java local index

	if (javaindex != jitdata::UNUSED) {
		assert(javaindex >= 0);
		if (iptr->flags.bits & INS_FLAG_RETADDR)
			javalocals[javaindex] = iptr->sx.s23.s2.retaddrnr;
		else
			javalocals[javaindex] = varindex;

		if (iptr->flags.bits & INS_FLAG_KILL_PREV)
			javalocals[javaindex-1] = jitdata::UNUSED;

		if (iptr->flags.bits & INS_FLAG_KILL_NEXT)
			javalocals[javaindex+1] = jitdata::UNUSED;
	}
}


/* functions for verbose stack analysis output ********************************/

#if defined(STACK_VERBOSE)
static void stack_verbose_show_varinfo(stackdata_t *sd, varinfo *v)
{
	printf("%c", show_jit_type_letters[v->type]);
	if (v->type == TYPE_RET) {
		printf("{L%03d}", v->vv.retaddr->nr);
#if defined(ENABLE_VERIFIER)
		printf("{start=L%03d}", ((basicblock *)v->SBRSTART)->nr);
#endif
	}
}


static void stack_verbose_show_variable(stackdata_t *sd, s4 index)
{
	assert(index >= 0 && index < sd->vartop);
	stack_verbose_show_varinfo(sd, sd->var + index);
}


static void stack_verbose_show_block(stackdata_t *sd, basicblock *bptr)
{
	s4 i;

	printf("L%03d type:%d in:%d [", bptr->nr, bptr->type, bptr->indepth);
	if (bptr->invars) {
		for (i=0; i<bptr->indepth; ++i) {
			if (i)
				putchar(' ');
			stack_verbose_show_variable(sd, bptr->invars[i]);
		}
	}
	else
		putchar('-');
	printf("] javalocals ");
	show_javalocals_array(sd->jd, sd->javalocals, sd->maxlocals, SHOW_STACK);
	printf(" inlocals [");
	if (bptr->inlocals) {
		for (i=0; i<sd->localcount; ++i) {
			if (i)
				putchar(' ');
			stack_verbose_show_varinfo(sd, bptr->inlocals + i);
		}
	}
	else
		putchar('-');
	printf("] out:%d [", bptr->outdepth);
	if (bptr->outvars) {
		for (i=0; i<bptr->outdepth; ++i) {
			if (i)
				putchar(' ');
			stack_verbose_show_variable(sd, bptr->outvars[i]);
		}
	}
	else
		putchar('-');
	printf("]");

	if (bptr->original)
		printf(" (clone of L%03d)", bptr->original->nr);
	else {
		basicblock *b = bptr->copied_to;
		if (b) {
			printf(" (copied to ");
			for (; b; b = b->copied_to)
				printf("L%03d ", b->nr);
			printf(")");
		}
	}
}


static void stack_verbose_block_enter(stackdata_t *sd, bool reanalyse)
{
	int i;

	printf("======================================== STACK %sANALYSE BLOCK ",
			(reanalyse) ? ((sd->bptr->iinstr == NULL) ? "CLONE-" : "RE-") : "");
	stack_verbose_show_block(sd, sd->bptr);
	printf("\n");

	if (sd->handlers[0]) {
		printf("HANDLERS: ");
		for (i=0; sd->handlers[i]; ++i) {
			printf("L%03d ", sd->handlers[i]->handler->nr);
		}
		printf("\n");
	}
	printf("\n");
}


static void stack_verbose_block_exit(stackdata_t *sd, bool superblockend)
{
	printf("%s ", (superblockend) ? "SUPERBLOCKEND" : "END");
	stack_verbose_show_block(sd, sd->bptr);
	printf("\n");
}

static void stack_verbose_show_state(stackdata_t *sd, instruction *iptr, stackelement_t *curstack)
{
	stackelement_t *sp;
	s4       i;
	s4       depth;
	varinfo *v;
	stackelement_t **stack;

	printf("    javalocals ");
	show_javalocals_array(sd->jd, sd->javalocals, sd->maxlocals, SHOW_STACK);
	printf(" stack [");

	for(i = 0, sp = curstack; sp; sp = sp->prev)
		i++;
	depth = i;

	stack = MNEW(stackelement_t *, depth);
	for(sp = curstack; sp; sp = sp->prev)
		stack[--i] = sp;

	for(i=0; i<depth; ++i) {
		if (i)
			putchar(' ');
		sp = stack[i];
		v = &(sd->var[sp->varnum]);

		if (v->flags & INOUT)
			putchar('I');
		if (v->flags & PREALLOC)
			putchar('A');
		printf("%d:%c", sp->varnum, show_jit_type_letters[sp->type]);
		if (v->type == TYPE_RET) {
			printf("(L%03d)", v->vv.retaddr->nr);
		}
	}
	printf("] ... ");
	if (iptr)
		show_icmd(sd->jd, iptr, false, SHOW_PARSE);
	printf("\n");
}
#endif


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
