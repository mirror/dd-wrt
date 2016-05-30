/* src/vm/jit/powerpc/emit.cpp - PowerPC code emitter functions

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
#include <stdint.h>

#include "vm/types.hpp"

#include "md-abi.hpp"

#include "vm/jit/powerpc/codegen.hpp"

#include "mm/memory.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/options.hpp"

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
	codegendata *cd;
	s4           disp;
	s4           reg;

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(src->flags)) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		switch (src->type) {
		case TYPE_INT:
		case TYPE_ADR:
			M_ILD(tempreg, REG_SP, disp);
			break;
		case TYPE_LNG:
			M_LLD(tempreg, REG_SP, disp);
			break;
		case TYPE_FLT:
			M_FLD(tempreg, REG_SP, disp);
			break;
		case TYPE_DBL:
			M_DLD(tempreg, REG_SP, disp);
			break;
		default:
			vm_abort("emit_load: unknown type %d", src->type);
			break;
		}

		reg = tempreg;
	}
	else
		reg = src->vv.regoff;

	return reg;
}


/* emit_load_low ***************************************************************

   Emits a possible load of the low 32-bits of an operand.

*******************************************************************************/

s4 emit_load_low(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	assert(src->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(src->flags)) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		M_ILD(tempreg, REG_SP, disp + 4);

		reg = tempreg;
	}
	else
		reg = GET_LOW_REG(src->vv.regoff);

	return reg;
}


/* emit_load_high **************************************************************

   Emits a possible load of the high 32-bits of an operand.

*******************************************************************************/

s4 emit_load_high(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	assert(src->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(src->flags)) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		M_ILD(tempreg, REG_SP, disp);

		reg = tempreg;
	}
	else
		reg = GET_HIGH_REG(src->vv.regoff);

	return reg;
}


/* emit_store ******************************************************************

   Emit a possible store for the given variable.

*******************************************************************************/

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata *cd;
	s4           disp;

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(dst->flags)) {
		COUNT_SPILLS;

		disp = dst->vv.regoff;

		switch (dst->type) {
		case TYPE_INT:
		case TYPE_ADR:
			M_IST(d, REG_SP, disp);
			break;
		case TYPE_LNG:
			M_LST(d, REG_SP, disp);
			break;
		case TYPE_FLT:
			M_FST(d, REG_SP, disp);
			break;
		case TYPE_DBL:
			M_DST(d, REG_SP, disp);
			break;
		default:
			vm_abort("emit_store: unknown type %d", dst->type);
			break;
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
		(IS_INMEMORY(src->flags ^ dst->flags))) {

		if ((src->type == TYPE_RET) || (dst->type == TYPE_RET)) {
			/* emit nothing, as the value won't be used anyway */
			return;
		}

		/* If one of the variables resides in memory, we can eliminate
		   the register move from/to the temporary register with the
		   order of getting the destination register and the load. */

		if (IS_INMEMORY(src->flags)) {
			if (IS_LNG_TYPE(src->type))
				d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP12_PACKED);
			else
				d = codegen_reg_of_var(iptr->opc, dst, REG_IFTMP);

			s1 = emit_load(jd, iptr, src, d);
		}
		else {
			if (IS_LNG_TYPE(src->type))
				s1 = emit_load(jd, iptr, src, REG_ITMP12_PACKED);
			else
				s1 = emit_load(jd, iptr, src, REG_IFTMP);

			d = codegen_reg_of_var(iptr->opc, dst, s1);
		}

		if (s1 != d) {
			switch (src->type) {
			case TYPE_INT:
			case TYPE_ADR:
				M_MOV(s1, d);
				break;
			case TYPE_LNG:
				M_MOV(GET_LOW_REG(s1), GET_LOW_REG(d));
				M_MOV(GET_HIGH_REG(s1), GET_HIGH_REG(d));
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_FMOV(s1, d);
				break;
			default:
				vm_abort("emit_copy: unknown type %d", src->type);
				break;
			}
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

	if ((value >= -32768) && (value <= 32767))
		M_LDA_INTERN(d, REG_ZERO, value);
	else {
		disp = dseg_add_s4(cd, value);
		M_ILD(d, REG_PV, disp);
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
	}
	else {
		assert(reg != REG_ITMP2);
		disp = dseg_add_s4(cd, value);
		M_ILD(REG_ITMP2, REG_PV, disp);
		M_CMP(reg, REG_ITMP2);
	}
}


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


/* emit_arithmetic_check *******************************************************

   Emit an ArithmeticException check.

*******************************************************************************/

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(reg);
		M_BNE(1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_ArithmeticException);
	}
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
		M_TRAPGEU(s2, REG_ITMP3);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(REG_RESULT);
		M_BNE(1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_ArrayStoreException);
	}
}


/* emit_classcast_check ********************************************************

   Emit a ClassCastException check.

*******************************************************************************/

void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		switch (condition) {
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
		M_ALD_INTERN(s1, REG_ZERO, TRAP_ClassCastException);
	}
}


/* emit_nullpointer_check ******************************************************

   Emit a NullPointerException check.

*******************************************************************************/

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(reg);
		M_BNE(1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}
}


/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(REG_RESULT);
		M_BNE(1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_CHECK_EXCEPTION);
	}
}


/* emit_trap_compiler **********************************************************

   Emit a trap instruction which calls the JIT compiler.

*******************************************************************************/

void emit_trap_compiler(codegendata *cd)
{
	M_ALD_INTERN(REG_METHODPTR, REG_ZERO, TRAP_COMPILER);
}


/* emit_trap *******************************************************************

   Emit a trap instruction and return the original machine code.

*******************************************************************************/

uint32_t emit_trap(codegendata *cd)
{
	// Get machine code which is patched back in later. The rap is 1
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


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int32_t p;
	int32_t disp;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		M_AADD_IMM(REG_SP, -((LA_SIZE_IN_POINTERS + ARG_CNT) * 8), REG_SP);

		for (p = 0; p < INT_ARG_CNT; p++)
			M_IST(abi_registers_integer_argument[p], REG_SP, LA_SIZE + p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DST(abi_registers_float_argument[p], REG_SP, LA_SIZE + (INT_ARG_CNT + p) * 8);

		syncslot_offset += (LA_SIZE_IN_POINTERS + ARG_CNT) * 8;
	}
# endif

	disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_MTCTR(REG_ITMP3);

	/* get or test the lock object */

	if (m->flags & ACC_STATIC) {
		disp = dseg_add_address(cd, &m->clazz->object.header);
		M_ALD(REG_A0, REG_PV, disp);
	}
	else {
		M_TST(REG_A0);
		M_BNE(1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}

	M_AST(REG_A0, REG_SP, syncslot_offset);
	M_JSR;

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		for (p = 0; p < INT_ARG_CNT; p++)
			M_ILD(abi_registers_integer_argument[p], REG_SP, LA_SIZE + p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DLD(abi_registers_float_argument[p], REG_SP, LA_SIZE + (INT_ARG_CNT + p) * 8);

		M_AADD_IMM(REG_SP, (LA_SIZE_IN_POINTERS + ARG_CNT) * 8, REG_SP);
	}
# endif
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
	M_MTCTR(REG_ITMP3);

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_LNG:
		M_IST(REG_RESULT2, REG_SP, syncslot_offset + 8);
		/* fall through */
	case TYPE_INT:
	case TYPE_ADR:
		M_IST(REG_RESULT , REG_SP, syncslot_offset + 4);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, syncslot_offset + 4);
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
		M_ILD(REG_RESULT2, REG_SP, syncslot_offset + 8);
		/* fall through */
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT , REG_SP, syncslot_offset + 4);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, syncslot_offset + 4);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}
}


/**
 * Emit profiling code for method frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_method(codegendata* cd, codeinfo* code)
{
	M_ALD(REG_ITMP1, REG_PV, CodeinfoPointer);
	M_ALD(REG_ITMP2, REG_ITMP1, OFFSET(codeinfo, frequency));
	M_IADD_IMM(REG_ITMP2, 1, REG_ITMP2);
	M_AST(REG_ITMP2, REG_ITMP1, OFFSET(codeinfo, frequency));
}
#endif


/**
 * Emit profiling code for basicblock frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr)
{
	int32_t disp = dseg_add_address(cd, code->bbfrequency);

	M_ALD(REG_ITMP2, REG_PV, disp);
	M_ALD(REG_ITMP3, REG_ITMP2, bptr->nr * 4);
	M_IADD_IMM(REG_ITMP3, 1, REG_ITMP3);
	M_AST(REG_ITMP3, REG_ITMP2, bptr->nr * 4);
}
#endif


/**
 * Emit profiling code to start CPU cycle counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_cycle_start(codegendata* cd, codeinfo* code)
{
	// XXX Not implemented yet!
}
#endif


/**
 * Emit profiling code to stop CPU cycle counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_cycle_stop(codegendata* cd, codeinfo* code)
{
	// XXX Not implemented yet!
}
#endif


/* emit_verbosecall_enter ******************************************************

   Generates the code for the call trace.

*******************************************************************************/

void emit_verbosecall_enter(jitdata *jd)
{
#if !defined(NDEBUG)
	methodinfo   *m;
	codegendata  *cd;
	registerdata *rd;
	methoddesc   *md;
	int32_t       disp;
	int32_t       i;
	int32_t       s, d;

	if (!JITDATA_HAS_FLAG_VERBOSECALL(jd))
		return;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* On Darwin we need to allocate an additional 3*4 bytes of stack
	   for the arguments to trace_java_call_enter, we make it 2*8. */

	M_MFLR(REG_ZERO);
	M_AST(REG_ZERO, REG_SP, LA_LR_OFFSET);
	M_STWU(REG_SP, REG_SP, -(LA_SIZE + (2 + ARG_CNT + TMP_CNT) * 8));

	/* save argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;
			d = LA_SIZE + (i + 2) * 8;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_ADR:
				M_IST(s, REG_SP, d);
				break;
			case TYPE_LNG:
				M_LST(s, REG_SP, d);
				break;
			case TYPE_FLT:
				M_FST(s, REG_SP, d);
				break;
			case TYPE_DBL:
				M_DST(s, REG_SP, d);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* pass methodinfo and pointers to the tracer function */

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, LA_SIZE + 2 * 8, REG_A1);
	M_AADD_IMM(REG_SP, LA_SIZE + (2 + ARG_CNT + TMP_CNT + cd->stackframesize) * 8, REG_A2);
	
	disp = dseg_add_functionptr(cd, trace_java_call_enter);
	M_ALD(REG_ITMP2, REG_PV, disp);
	M_MTCTR(REG_ITMP2);
	M_JSR;

	/* restore argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = LA_SIZE + (i + 2) * 8;
			d = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_ADR:
				M_ILD(d, REG_SP, s);
				break;
			case TYPE_LNG:
				M_LLD(d, REG_SP, s);
				break;
			case TYPE_FLT:
				M_FLD(d, REG_SP, s);
				break;
			case TYPE_DBL:
				M_DLD(d, REG_SP, s);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	M_ALD(REG_ZERO, REG_SP, LA_SIZE + (2 + ARG_CNT + TMP_CNT) * 8 + LA_LR_OFFSET);
	M_MTLR(REG_ZERO);
	M_LDA(REG_SP, REG_SP, LA_SIZE + (2 + ARG_CNT + TMP_CNT) * 8);

	/* mark trace code */

	M_NOP;
#endif /* !defined(NDEBUG) */
}


/* emit_verbosecall_exit *******************************************************

   Generates the code for the call trace.

*******************************************************************************/

void emit_verbosecall_exit(jitdata *jd)
{
#if !defined(NDEBUG)
	methodinfo   *m;
	codegendata  *cd;
	registerdata *rd;
	methoddesc   *md;
	s4            disp;

	if (!JITDATA_HAS_FLAG_VERBOSECALL(jd))
		return;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;
	
	/* mark trace code */

	M_NOP;

	/* On Darwin we need to allocate an additional 2*4 bytes of stack
	   for the arguments to trace_java_call_exit, we make it 1*8. */

	M_MFLR(REG_ZERO);
	M_AST(REG_ZERO, REG_SP, LA_LR_OFFSET);
	M_STWU(REG_SP, REG_SP, -(LA_SIZE + (1 + 1) * 8));

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		M_IST(REG_RESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_LNG:
		M_LST(REG_RESULT_PACKED, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_FLT:
		M_FST(REG_FRESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, LA_SIZE + 1 * 8, REG_A1);

	disp = dseg_add_functionptr(cd, trace_java_call_exit);
	M_ALD(REG_ITMP2, REG_PV, disp);
	M_MTCTR(REG_ITMP2);
	M_JSR;

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_LNG:
		M_LLD(REG_RESULT_PACKED, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_FLT:
		M_FLD(REG_FRESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, LA_SIZE + 1 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	M_ALD(REG_ZERO, REG_SP, LA_SIZE + (1 + 1) * 8 + LA_LR_OFFSET);
	M_MTLR(REG_ZERO);
	M_LDA(REG_SP, REG_SP, LA_SIZE + (1 + 1) * 8);

	/* mark trace code */

	M_NOP;
#endif /* !defined(NDEBUG) */
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
