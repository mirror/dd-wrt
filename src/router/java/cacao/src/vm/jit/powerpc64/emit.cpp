/* src/vm/jit/powerpc64/emit.cpp - PowerPC64 code emitter functions

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

#include <cassert>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "md-abi.hpp"
#include "vm/jit/powerpc64/codegen.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/options.hpp"
#include "vm/vm.hpp"
   
#include "vm/jit/abi.hpp"
#include "vm/jit/abi-asm.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/replace.hpp"
#include "vm/jit/trace.hpp"
#include "vm/jit/trap.hpp"

#include "vm/jit/ir/instruction.hpp"


/* emit_load *******************************************************************

   Emits a possible load of an operand.

*******************************************************************************/

s4 emit_load(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	/* get required compiler data */

	cd = jd->cd;

	if (src->flags & INMEMORY) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		if (IS_FLT_DBL_TYPE(src->type)) {
			if (IS_2_WORD_TYPE(src->type))
				M_DLD(tempreg, REG_SP, disp);
			else
				M_FLD(tempreg, REG_SP, disp);
		}
		else {
			M_LLD(tempreg, REG_SP, disp);
		}

		reg = tempreg;
	}
	else
		reg = src->vv.regoff;

	return reg;
}


/* emit_store ******************************************************************

   Emits a possible store to a variable.

*******************************************************************************/

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;

	/* get required compiler data */

	cd = jd->cd;

	if (dst->flags & INMEMORY) {
		COUNT_SPILLS;

		if (IS_FLT_DBL_TYPE(dst->type)) {
			if (IS_2_WORD_TYPE(dst->type))
				M_DST(d, REG_SP, dst->vv.regoff);
			else
				M_FST(d, REG_SP, dst->vv.regoff);
		}
		else {
			M_LST(d, REG_SP, dst->vv.regoff);
		}
	}
}


/* emit_copy *******************************************************************

   Generates a register/memory to register/memory copy.

*******************************************************************************/

void emit_copy(jitdata *jd, instruction *iptr)
{
	codegendata *cd;
	varinfo     *src;
	varinfo     *dst;
	s4           s1, d;

	/* get required compiler data */

	cd = jd->cd;

	/* get source and destination variables */

	src = VAROP(iptr->s1);
	dst = VAROP(iptr->dst);

	if ((src->vv.regoff != dst->vv.regoff) ||
		((src->flags ^ dst->flags) & INMEMORY)) {

		if ((src->type == TYPE_RET) || (dst->type == TYPE_RET)) {
			/* emit nothing, as the value won't be used anyway */
			return;
		}

		/* If one of the variables resides in memory, we can eliminate
		   the register move from/to the temporary register with the
		   order of getting the destination register and the load. */

		if (IS_INMEMORY(src->flags)) {
			d  = codegen_reg_of_var(iptr->opc, dst, REG_IFTMP);
			s1 = emit_load(jd, iptr, src, d);
		}
		else {
			s1 = emit_load(jd, iptr, src, REG_IFTMP);
			d  = codegen_reg_of_var(iptr->opc, dst, s1);
		}

		if (s1 != d) {
			if (IS_FLT_DBL_TYPE(src->type))
				M_FMOV(s1, d);
			else
				M_MOV(s1, d);
		}

		emit_store(jd, iptr, dst, d);
	}
}


/* emit_iconst *****************************************************************

   XXX

*******************************************************************************/

void emit_iconst(codegendata *cd, s4 d, s4 value)
{
	s4 disp;

	if ((value >= -32768) && (value <= 32767)) {
		M_LDA_INTERN(d, REG_ZERO, value);
	} else {
		disp = dseg_add_s4(cd, value);
		M_ILD(d, REG_PV, disp);
	}
}

void emit_lconst(codegendata *cd, s4 d, s8 value)
{
	s4 disp;
	if ((value >= -32768) && (value <= 32767)) {
		M_LDA_INTERN(d, REG_ZERO, value);
	} else {
		disp = dseg_add_s8(cd, value);
		M_LLD(d, REG_PV, disp);
	}
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	int32_t disp;

	if ((value >= -32768) && (value <= 32767)) {
		M_CMPI(reg, value);
	} else {
		assert(reg != REG_ITMP2);
		disp = dseg_add_s4(cd, value);
		M_ILD(REG_ITMP2, REG_PV, disp);
		M_CMP(reg, REG_ITMP2);
	}
}


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int32_t p;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

#if !defined (NDEBUG)
		if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
			M_AADD_IMM(REG_SP, -((LA_SIZE_IN_POINTERS + PA_SIZE_IN_POINTERS + ARG_CNT) * 8), REG_SP);

			for (p = 0; p < INT_ARG_CNT; p++)
				M_LST(abi_registers_integer_argument[p], REG_SP, LA_SIZE + PA_SIZE + p * 8);

			for (p = 0; p < FLT_ARG_CNT; p++)
				M_DST(abi_registers_float_argument[p], REG_SP, LA_SIZE + PA_SIZE + (INT_ARG_CNT + p) * 8);

			/* used for LOCK_monitor_exit, adopt size because we created another stackframe */
			syncslot_offset += (LA_SIZE_IN_POINTERS + PA_SIZE_IN_POINTERS + ARG_CNT) * 8;
		}
#endif

		p = dseg_add_functionptr(cd, LOCK_monitor_enter);
		M_ALD(REG_ITMP3, REG_PV, p);
		M_ALD(REG_ITMP3, REG_ITMP3, 0); /* TOC */
		M_MTCTR(REG_ITMP3);

		/* get or test the lock object */

		if (m->flags & ACC_STATIC) {
			p = dseg_add_address(cd, &m->clazz->object.header);
			M_ALD(REG_A0, REG_PV, p);
		}
		else {
			M_TST(REG_A0);
			M_BNE(1);
			M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
		}

		M_AST(REG_A0, REG_SP, syncslot_offset);
		M_JSR;

#if !defined(NDEBUG)
		if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
			for (p = 0; p < INT_ARG_CNT; p++)
				M_LLD(abi_registers_integer_argument[p], REG_SP, LA_SIZE + PA_SIZE + p * 8);

			for (p = 0; p < FLT_ARG_CNT; p++)
				M_DLD(abi_registers_float_argument[p], REG_SP, LA_SIZE + PA_SIZE + (INT_ARG_CNT + p) * 8);

			M_AADD_IMM(REG_SP, (LA_SIZE_IN_POINTERS + PA_SIZE_IN_POINTERS + ARG_CNT) * 8, REG_SP);
		}
#endif
}


/**
 * Generates synchronization code to leave a monitor.
 */
void emit_monitor_exit(jitdata* jd, int32_t syncslot_offset)
{
	int32_t disp;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

	disp = dseg_add_functionptr(cd, LOCK_monitor_exit);
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_ALD(REG_ITMP3, REG_ITMP3, 0); /* TOC */
	M_MTCTR(REG_ITMP3);

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_LNG:
	case TYPE_INT:
	case TYPE_ADR:
		/* fall through */
		M_LST(REG_RESULT , REG_SP, syncslot_offset + 8);
		break;
	case TYPE_FLT:
		M_FST(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	M_ALD(REG_A0, REG_SP, syncslot_offset);
	M_JSR;

	/* and now restore the proper return value */

	switch (md->returntype.type) {
	case TYPE_LNG:
	case TYPE_INT:
	case TYPE_ADR:
		/* fall through */
		M_LLD(REG_RESULT , REG_SP, syncslot_offset + 8);
		break;
	case TYPE_FLT:
		M_FLD(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}
}


/* emit_verbosecall_enter ******************************************************

   Generates the code for the call trace.

*******************************************************************************/

#if !defined(NDEBUG)
void emit_verbosecall_enter(jitdata *jd)
{
	methodinfo   *m;
	codegendata  *cd;
	methoddesc   *md;
	int32_t       paramcount;
	int32_t       stackframesize;
	s4            disp;
	s4            i, s;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;

	md = m->parseddesc;
	
	/* mark trace code */

	M_NOP;

	/* align stack to 16-bytes */

	paramcount = md->paramcount;
	ALIGN_2(paramcount);
	stackframesize = LA_SIZE + PA_SIZE + md->paramcount * 8;

	M_MFLR(REG_ZERO);
	M_AST(REG_ZERO, REG_SP, LA_LR_OFFSET);
	M_STDU(REG_SP, REG_SP, -stackframesize);

#if defined(__DARWIN__)
	#warning "emit_verbosecall_enter not implemented"
#else
	/* save argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LST(s, REG_SP, LA_SIZE+PA_SIZE+i*8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DST(s, REG_SP, LA_SIZE+PA_SIZE+i*8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}
#endif

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, LA_SIZE+PA_SIZE, REG_A1);
	M_AADD_IMM(REG_SP, stackframesize + cd->stackframesize * 8, REG_A2);
	/* call via function descriptor, XXX: what about TOC? */
	disp = dseg_add_functionptr(cd, trace_java_call_enter);
	M_ALD(REG_ITMP2, REG_PV, disp);
	M_ALD(REG_ITMP1, REG_ITMP2, 0);
	M_MTCTR(REG_ITMP1);
	M_JSR;

#if defined(__DARWIN__)
	#warning "emit_verbosecall_enter not implemented"
#else
	/* restore argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LLD(s, REG_SP, LA_SIZE+PA_SIZE+i*8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DLD(s, REG_SP, LA_SIZE+PA_SIZE+i*8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}
#endif

	M_ALD(REG_ZERO, REG_SP, stackframesize + LA_LR_OFFSET);
	M_MTLR(REG_ZERO);
	M_LDA(REG_SP, REG_SP, stackframesize);

	/* mark trace code */

	M_NOP;
}
#endif


/* emit_verbosecall_exit ******************************************************

   Generates the code for the call trace.

*******************************************************************************/

#if !defined(NDEBUG)
void emit_verbosecall_exit(jitdata *jd)
{
	methodinfo   *m;
	codegendata  *cd;
	methoddesc   *md;
	s4            disp;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	M_MFLR(REG_ZERO);
	M_LDA(REG_SP, REG_SP, -(LA_SIZE+PA_SIZE+10*8));
	M_AST(REG_ZERO, REG_SP, LA_SIZE+PA_SIZE+1*8);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LST(REG_RESULT, REG_SP, LA_SIZE+PA_SIZE+0*8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, LA_SIZE+PA_SIZE+0*8);
		break;
	default:
		assert(false);
		break;
	}

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, LA_SIZE+PA_SIZE, REG_A1);

	disp = dseg_add_functionptr(cd, trace_java_call_exit);
	/* call via function descriptor, XXX: what about TOC ? */
	M_ALD(REG_ITMP2, REG_PV, disp);
	M_ALD(REG_ITMP2, REG_ITMP2, 0);
	M_MTCTR(REG_ITMP2);
	M_JSR;

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LLD(REG_RESULT, REG_SP, LA_SIZE+PA_SIZE+0*8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, LA_SIZE+PA_SIZE+0*8);
		break;
	default:
		assert(false);
		break;
	}

	M_ALD(REG_ZERO, REG_SP, LA_SIZE+PA_SIZE+1*8);
	M_LDA(REG_SP, REG_SP, LA_SIZE+PA_SIZE+10*8);
	M_MTLR(REG_ZERO);

	/* mark trace code */

	M_NOP;
}
#endif


/* emit_branch *****************************************************************

   Emits the code for conditional and unconditional branchs.

*******************************************************************************/

void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 opt)
{
	s4 checkdisp;
	s4 branchdisp;

	/* calculate the different displacements */

	checkdisp  =  disp + 4;
	branchdisp = (disp - 4) >> 2;

	/* check which branch to generate */

	if (condition == BRANCH_UNCONDITIONAL) {
		/* check displacement for overflow */

		if ((checkdisp < (s4) 0xfe000000) || (checkdisp > (s4) 0x01fffffc)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit unconditional long-branch code");
		}
		else {
			M_BR(branchdisp);
		}
	}
	else {
		/* and displacement for overflow */

		if ((checkdisp < (s4) 0xffff8000) || (checkdisp > (s4) 0x00007fff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			// Subtract 1 instruction from the displacement as the
			// actual branch is the second instruction.
			checkdisp  = checkdisp - 4;
			branchdisp = branchdisp - 1;

			if ((checkdisp < (int32_t) 0xfe000000) || (checkdisp > (int32_t) 0x01fffffc)) {
				vm_abort("emit_branch: emit conditional long-branch code");
			}
			else {
				switch (condition) {
				case BRANCH_EQ:
					M_BNE(1);
					M_BR(branchdisp);
					break;
				case BRANCH_NE:
					M_BEQ(1);
					M_BR(branchdisp);
					break;
				case BRANCH_LT:
					M_BGE(1);
					M_BR(branchdisp);
					break;
				case BRANCH_GE:
					M_BLT(1);
					M_BR(branchdisp);
					break;
				case BRANCH_GT:
					M_BLE(1);
					M_BR(branchdisp);
					break;
				case BRANCH_LE:
					M_BGT(1);
					M_BR(branchdisp);
					break;
				case BRANCH_NAN:
					vm_abort("emit_branch: long BRANCH_NAN");
					break;
				default:
					vm_abort("emit_branch: unknown condition %d", condition);
					break;
				}
			}
		}
		else {
			switch (condition) {
			case BRANCH_EQ:
				M_BEQ(branchdisp);
				break;
			case BRANCH_NE:
				M_BNE(branchdisp);
				break;
			case BRANCH_LT:
				M_BLT(branchdisp);
				break;
			case BRANCH_GE:
				M_BGE(branchdisp);
				break;
			case BRANCH_GT:
				M_BGT(branchdisp);
				break;
			case BRANCH_LE:
				M_BLE(branchdisp);
				break;
			case BRANCH_NAN:
				M_BNAN(branchdisp);
				break;
			default:
				vm_abort("emit_branch: unknown condition %d", condition);
				break;
			}
		}
	}
}

/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (checkbounds) {
		M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
		M_CMPU(s2, REG_ITMP3);
		M_BLT(1);
		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(s2, REG_ZERO, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr))	{
		M_TST(REG_RESULT);
		M_BNE(1);
		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(REG_ZERO, REG_ZERO, TRAP_ArrayStoreException);
	}
}


/* emit_arithmetic_check *******************************************************

   Emit an ArithmeticException check.

*******************************************************************************/

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr))	{
		M_TST(reg);
		M_BNE(1);
		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(REG_ZERO, REG_ZERO, TRAP_ArithmeticException);
	}
}


/* emit_classcast_check ********************************************************

   Emit a ClassCastException check.

*******************************************************************************/

void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1)
{
	if (INSTRUCTION_MUST_CHECK(iptr))	{
		switch(condition)	{
		case BRANCH_LE:
			M_BGT(1);
			break;
		case BRANCH_EQ:
			M_BNE(1);
			break;
		case BRANCH_NE:
			M_BEQ(1);
			break;
		case BRANCH_GT:
			M_BLE(1);
			break;
		default:
			vm_abort("emit_classcast_check: unknown condition %d", condition);
			break;
		}

		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(s1, REG_ZERO, TRAP_ClassCastException);
	}
}


/* emit_nullpointer_check ******************************************************

   Emit a NullPointerException check.

*******************************************************************************/

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr))	{
		M_TST(reg);
		M_BNE(1);
		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}
}

/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr))	{
		M_TST(REG_RESULT);
		M_BNE(1);
		/* ALD is 4 byte aligned, ILD 2, onyl LWZ is byte aligned */
		M_LWZ(REG_ZERO, REG_ZERO, TRAP_CHECK_EXCEPTION);
	}
}


/* emit_trap_compiler **********************************************************

   Emit a trap instruction which calls the JIT compiler.

*******************************************************************************/

void emit_trap_compiler(codegendata *cd)
{
	M_LWZ(REG_METHODPTR, REG_ZERO, TRAP_COMPILER);
}


/* emit_trap *******************************************************************

   Emit a trap instruction and return the original machine code.

*******************************************************************************/

uint32_t emit_trap(codegendata *cd)
{
	// Get machine code which is patched back in later. The trap is 1
	// instruction word long.
	uint32_t mcode = *((uint32_t*) cd->mcodeptr);

	M_ILLEGAL;

	return mcode;
}


/**
 * Emit code to recompute the procedure vector.
 */
void emit_recompute_pv(codegendata *cd)
{
	int32_t disp = (int32_t) (cd->mcodeptr - cd->mcodebase);

	M_MFLR(REG_ITMP1);
	M_LDA(REG_PV, REG_ITMP1, -disp);
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
