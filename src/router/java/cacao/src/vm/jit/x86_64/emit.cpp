/* src/vm/jit/x86_64/emit.cpp - x86_64 code emitter functions

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
#include "vm/os.hpp"

#include "md-abi.hpp"

#include "vm/jit/x86_64/codegen.hpp"
#include "vm/jit/x86_64/emit.hpp"

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
	codegendata  *cd;
	s4            disp;
	s4            reg;

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(src->flags)) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		switch (src->type) {
		case TYPE_INT:
			M_ILD(tempreg, REG_SP, disp);
			break;
		case TYPE_LNG:
		case TYPE_ADR:
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


/* emit_store ******************************************************************

   This function generates the code to store the result of an
   operation back into a spilled pseudo-variable.  If the
   pseudo-variable has not been spilled in the first place, this
   function will generate nothing.
    
*******************************************************************************/

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;
	s4            disp;

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(dst->flags)) {
		COUNT_SPILLS;

		disp = dst->vv.regoff;

		switch (dst->type) {
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
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
			switch (src->type) {
			case TYPE_INT:
			case TYPE_LNG:
			case TYPE_ADR:
				M_MOV(s1, d);
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


void emit_cmovxx(codegendata *cd, instruction *iptr, s4 s, s4 d)
{
#if 0
	switch (iptr->flags.fields.condition) {
	case ICMD_IFEQ:
		M_CMOVEQ(s, d);
		break;
	case ICMD_IFNE:
		M_CMOVNE(s, d);
		break;
	case ICMD_IFLT:
		M_CMOVLT(s, d);
		break;
	case ICMD_IFGE:
		M_CMOVGE(s, d);
		break;
	case ICMD_IFGT:
		M_CMOVGT(s, d);
		break;
	case ICMD_IFLE:
		M_CMOVLE(s, d);
		break;
	}
#endif
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	M_ICMP_IMM(value, reg);
}


/* emit_branch *****************************************************************

   Emits the code for conditional and unconditional branchs.

*******************************************************************************/

void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 options)
{
	s4 branchdisp;

	/* NOTE: A displacement overflow cannot happen. */

	/* check which branch to generate */

	if (condition == BRANCH_UNCONDITIONAL) {

		/* calculate the different displacements */

		branchdisp = disp - BRANCH_UNCONDITIONAL_SIZE;

		M_JMP_IMM(branchdisp);
	}
	else {
		/* calculate the different displacements */

		branchdisp = disp - BRANCH_CONDITIONAL_SIZE;

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
		case BRANCH_ULT:
			M_BULT(branchdisp);
			break;
		case BRANCH_ULE:
			M_BULE(branchdisp);
			break;
		case BRANCH_UGE:
			M_BUGE(branchdisp);
			break;
		case BRANCH_UGT:
			M_BUGT(branchdisp);
			break;
		default:
			vm_abort("emit_branch: unknown condition %d", condition);
			break;
		}
	}
}


/* emit_arithmetic_check *******************************************************

   Emit an ArithmeticException check.

*******************************************************************************/

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(reg);
		M_BNE(8);
		M_ALD_MEM(reg, TRAP_ArithmeticException);
	}
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (checkbounds && INSTRUCTION_MUST_CHECK(iptr)) {
        M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
        M_ICMP(REG_ITMP3, s2);
		M_BULT(8);
		M_ALD_MEM(s2, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(REG_RESULT);
		M_BNE(8);
		M_ALD_MEM(REG_RESULT, TRAP_ArrayStoreException);
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
			M_BGT(8);
			break;
		case BRANCH_GE:
			M_BLT(8);
			break;
		case BRANCH_EQ:
			M_BNE(8);
			break;
		case BRANCH_NE:
			M_BEQ(8);
			break;
		case BRANCH_UGT:
			M_BULE(8);
			break;
		default:
			vm_abort("emit_classcast_check: unknown condition %d", condition);
			break;
		}
		M_ALD_MEM(s1, TRAP_ClassCastException);
	}
}


/* emit_nullpointer_check ******************************************************

   Emit a NullPointerException check.

*******************************************************************************/

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(reg);
		M_BNE(8);
		M_ALD_MEM(reg, TRAP_NullPointerException);
	}
}


/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(REG_RESULT);
		M_BNE(8);
		M_ALD_MEM(REG_RESULT, TRAP_CHECK_EXCEPTION);
	}
}


/* emit_trap_compiler **********************************************************

   Emit a trap instruction which calls the JIT compiler.

*******************************************************************************/

void emit_trap_compiler(codegendata *cd)
{
	M_ALD_MEM(REG_METHODPTR, TRAP_COMPILER);
}


/* emit_patcher_alignment ******************************************************

   Emit NOP to ensure placement at an even address.

*******************************************************************************/

void emit_patcher_alignment(codegendata *cd)
{
	if ((uintptr_t) cd->mcodeptr & 1)
		M_NOP;
}


/* emit_trap *******************************************************************

   Emit a trap instruction and return the original machine code.

*******************************************************************************/

uint32_t emit_trap(codegendata *cd)
{
	uint16_t mcode;

	/* Get machine code which is patched back in later. The trap is 2
	   bytes long. */

	mcode = *((uint16_t *) cd->mcodeptr);

	/* XXX This needs to be change to INT3 when the debugging problems
	   with gdb are resolved. */

	M_UD2;

	return mcode;
}


/**
 * Generates fast-path code for the below builtin.
 *   Function:  LOCK_monitor_enter
 *   Signature: (Ljava/lang/Object;)V
 *   Slow-path: bool lock_monitor_enter(java_handle_t*);
 */
void emit_fastpath_monitor_enter(jitdata* jd, instruction* iptr, int d)
{
	// Get required compiler data.
	codegendata* cd = jd->cd;

	// XXX Currently the fast-path always fails. Implement me!
	M_CLR(d);
}


/**
 * Generates fast-path code for the below builtin.
 *   Function:  LOCK_monitor_exit
 *   Signature: (Ljava/lang/Object;)V
 *   Slow-path: bool lock_monitor_exit(java_handle_t*);
 */
void emit_fastpath_monitor_exit(jitdata* jd, instruction* iptr, int d)
{
	// Get required compiler data.
	codegendata* cd = jd->cd;

	// XXX Currently the fast-path always fails. Implement me!
	M_CLR(d);
}


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

#ifndef NDEBUG
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		M_LSUB_IMM((INT_ARG_CNT + FLT_ARG_CNT) * 8, REG_SP);

		for (int32_t p = 0; p < INT_ARG_CNT; p++)
			M_LST(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (int32_t p = 0; p < FLT_ARG_CNT; p++)
			M_DST(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		syncslot_offset += (INT_ARG_CNT + FLT_ARG_CNT) * 8;
	}
#endif

	/* decide which monitor enter function to call */

	if (m->flags & ACC_STATIC) {
		M_MOV_IMM(&m->clazz->object.header, REG_A0);
	}
	else {
		M_TEST(REG_A0);
		M_BNE(8);
		M_ALD_MEM(REG_A0, TRAP_NullPointerException);
	}

	M_AST(REG_A0, REG_SP, syncslot_offset);
	M_MOV_IMM(LOCK_monitor_enter, REG_ITMP1);
	M_CALL(REG_ITMP1);

#ifndef NDEBUG
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {

		for (int32_t p = 0; p < INT_ARG_CNT; p++)
			M_LLD(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (int32_t p = 0; p < FLT_ARG_CNT; p++)
			M_DLD(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		M_LADD_IMM((INT_ARG_CNT + FLT_ARG_CNT) * 8, REG_SP);
	}
#endif
}


/**
 * Generates synchronization code to leave a monitor.
 */
void emit_monitor_exit(jitdata* jd, int32_t syncslot_offset)
{
	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

	M_ALD(REG_A0, REG_SP, syncslot_offset);

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
	case TYPE_LNG:
		M_LST(REG_RESULT, REG_SP, syncslot_offset);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, syncslot_offset);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	M_MOV_IMM(LOCK_monitor_exit, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* and now restore the proper return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
	case TYPE_LNG:
		M_LLD(REG_RESULT, REG_SP, syncslot_offset);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, syncslot_offset);
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
	M_MOV_IMM(code, REG_ITMP3);
	M_IINC_MEMBASE(REG_ITMP3, OFFSET(codeinfo, frequency));
}
#endif


/**
 * Emit profiling code for basicblock frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr)
{
	M_MOV_IMM(code->bbfrequency, REG_ITMP3);
	M_IINC_MEMBASE(REG_ITMP3, bptr->nr * 4);
}
#endif


/**
 * Emit profiling code to start CPU cycle counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_cycle_start(codegendata* cd, codeinfo* code)
{
	M_PUSH(RAX);
	M_PUSH(RDX);

	M_MOV_IMM(code, REG_ITMP3);
	M_RDTSC;
	M_ISUB_MEMBASE(RAX, REG_ITMP3, OFFSET(codeinfo, cycles));
	M_ISBB_MEMBASE(RDX, REG_ITMP3, OFFSET(codeinfo, cycles) + 4);

	M_POP(RDX);
	M_POP(RAX);
}
#endif


/**
 * Emit profiling code to stop CPU cycle counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_cycle_stop(codegendata* cd, codeinfo* code)
{
	M_PUSH(RAX);
	M_PUSH(RDX);

	M_MOV_IMM(code, REG_ITMP3);
	M_RDTSC;
	M_IADD_MEMBASE(RAX, REG_ITMP3, OFFSET(codeinfo, cycles));
	M_IADC_MEMBASE(RDX, REG_ITMP3, OFFSET(codeinfo, cycles) + 4);

	M_POP(RDX);
	M_POP(RAX);
}
#endif


/* emit_verbosecall_enter ******************************************************

   Generates the code for the call trace.

*******************************************************************************/

#if !defined(NDEBUG)
void emit_verbosecall_enter(jitdata *jd)
{
	methodinfo   *m;
	codeinfo     *code;
	codegendata  *cd;
	registerdata *rd;
	methoddesc   *md;
	s4            stackframesize;
	s4            i, s;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;
	rd   = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* keep 16-byte stack alignment */

	stackframesize = md->paramcount + ARG_CNT + TMP_CNT;
	ALIGN_2(stackframesize);

	M_LSUB_IMM(stackframesize * 8, REG_SP);

	/* save argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LST(s, REG_SP, i * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DST(s, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* save all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_ARG_CNT; i++)
			M_LST(abi_registers_integer_argument[i], REG_SP, (md->paramcount + i) * 8);

		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DST(abi_registers_float_argument[i], REG_SP, (md->paramcount + INT_ARG_CNT + i) * 8);

		for (i = 0; i < INT_TMP_CNT; i++)
			M_LST(rd->tmpintregs[i], REG_SP, (md->paramcount + ARG_CNT + i) * 8);

		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DST(rd->tmpfltregs[i], REG_SP, (md->paramcount + ARG_CNT + INT_TMP_CNT + i) * 8);
	}

	M_MOV_IMM(m, REG_A0);
	M_MOV(REG_SP, REG_A1);
	M_MOV(REG_SP, REG_A2);
	M_AADD_IMM((stackframesize + cd->stackframesize + 1) * 8, REG_A2);
	M_MOV_IMM(trace_java_call_enter, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* restore argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LLD(s, REG_SP, i * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DLD(s, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}


	/* restore all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_ARG_CNT; i++)
			M_LLD(abi_registers_integer_argument[i], REG_SP, (md->paramcount + i) * 8);

		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DLD(abi_registers_float_argument[i], REG_SP, (md->paramcount + INT_ARG_CNT + i) * 8);

		for (i = 0; i < INT_TMP_CNT; i++)
			M_LLD(rd->tmpintregs[i], REG_SP, (md->paramcount + ARG_CNT + i) * 8);

		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DLD(rd->tmpfltregs[i], REG_SP, (md->paramcount + ARG_CNT + INT_TMP_CNT + i) * 8);
	}

	M_LADD_IMM(stackframesize * 8, REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


/* emit_verbosecall_exit *******************************************************

   Generates the code for the call trace.

*******************************************************************************/

#if !defined(NDEBUG)
void emit_verbosecall_exit(jitdata *jd)
{
	methodinfo   *m;
	codegendata  *cd;
	//registerdata *rd;
	methoddesc   *md;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	//rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* keep 16-byte stack alignment */

	M_ASUB_IMM(2 * 8, REG_SP);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LST(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 0 * 8);
		break;
	default:
		assert(false);
		break;
	}

	M_MOV_IMM(m, REG_A0);
	M_MOV(REG_SP, REG_A1);

	M_MOV_IMM(trace_java_call_exit, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LLD(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 0 * 8);
		break;
	default:
		assert(false);
		break;
	}

	M_AADD_IMM(2 * 8, REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


/* code generation functions **************************************************/

static void emit_membase(codegendata *cd, s4 basereg, s4 disp, s4 dreg)
{
	if ((basereg == REG_SP) || (basereg == R12)) {
		if (disp == 0) {
			emit_address_byte(0, dreg, REG_SP);
			emit_address_byte(0, REG_SP, REG_SP);

		} else if (IS_IMM8(disp)) {
			emit_address_byte(1, dreg, REG_SP);
			emit_address_byte(0, REG_SP, REG_SP);
			emit_imm8(disp);

		} else {
			emit_address_byte(2, dreg, REG_SP);
			emit_address_byte(0, REG_SP, REG_SP);
			emit_imm32(disp);
		}

	} else if ((disp) == 0 && (basereg) != RBP && (basereg) != R13) {
		emit_address_byte(0,(dreg),(basereg));

	} else if ((basereg) == RIP) {
		emit_address_byte(0, dreg, RBP);
		emit_imm32(disp);

	} else {
		if (IS_IMM8(disp)) {
			emit_address_byte(1, dreg, basereg);
			emit_imm8(disp);

		} else {
			emit_address_byte(2, dreg, basereg);
			emit_imm32(disp);
		}
	}
}


static void emit_membase32(codegendata *cd, s4 basereg, s4 disp, s4 dreg)
{
	if ((basereg == REG_SP) || (basereg == R12)) {
		emit_address_byte(2, dreg, REG_SP);
		emit_address_byte(0, REG_SP, REG_SP);
		emit_imm32(disp);
	}
	else {
		emit_address_byte(2, dreg, basereg);
		emit_imm32(disp);
	}
}


static void emit_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	if (basereg == -1) {
		emit_address_byte(0, reg, 4);
		emit_address_byte(scale, indexreg, 5);
		emit_imm32(disp);
	}
	else if ((disp == 0) && (basereg != RBP) && (basereg != R13)) {
		emit_address_byte(0, reg, 4);
		emit_address_byte(scale, indexreg, basereg);
	}
	else if (IS_IMM8(disp)) {
		emit_address_byte(1, reg, 4);
		emit_address_byte(scale, indexreg, basereg);
		emit_imm8(disp);
	}
	else {
		emit_address_byte(2, reg, 4);
		emit_address_byte(scale, indexreg, basereg);
		emit_imm32(disp);
	}
}


void emit_ishift(jitdata *jd, s4 shift_op, instruction *iptr)
{
	s4 s1, s2, d, d_old;
	varinfo *v_s1,*v_s2,*v_dst;
	codegendata *cd;

	/* get required compiler data */

	cd = jd->cd;

	v_s1  = VAROP(iptr->s1);
	v_s2  = VAROP(iptr->sx.s23.s2);
	v_dst = VAROP(iptr->dst);

	s1 = v_s1->vv.regoff;
	s2 = v_s2->vv.regoff;
	d  = v_dst->vv.regoff;

	M_INTMOVE(RCX, REG_ITMP1);                                    /* save RCX */

	if (IS_INMEMORY(v_dst->flags)) {
		if (IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			if (s1 == d) {
				M_ILD(RCX, REG_SP, s2);
				emit_shiftl_membase(cd, shift_op, REG_SP, d);

			} else {
				M_ILD(RCX, REG_SP, s2);
				M_ILD(REG_ITMP2, REG_SP, s1);
				emit_shiftl_reg(cd, shift_op, REG_ITMP2);
				M_IST(REG_ITMP2, REG_SP, d);
			}

		} else if (IS_INMEMORY(v_s2->flags) && !IS_INMEMORY(v_s1->flags)) {
			/* s1 may be equal to RCX */
			if (s1 == RCX) {
				if (s2 == d) {
					M_ILD(REG_ITMP1, REG_SP, s2);
					M_IST(s1, REG_SP, d);
					M_INTMOVE(REG_ITMP1, RCX);

				} else {
					M_IST(s1, REG_SP, d);
					M_ILD(RCX, REG_SP, s2);
				}

			} else {
				M_ILD(RCX, REG_SP, s2);
				M_IST(s1, REG_SP, d);
			}

			emit_shiftl_membase(cd, shift_op, REG_SP, d);

		} else if (!IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			if (s1 == d) {
				M_INTMOVE(s2, RCX);
				emit_shiftl_membase(cd, shift_op, REG_SP, d);

			} else {
				M_INTMOVE(s2, RCX);
				M_ILD(REG_ITMP2, REG_SP, s1);
				emit_shiftl_reg(cd, shift_op, REG_ITMP2);
				M_IST(REG_ITMP2, REG_SP, d);
			}

		} else {
			/* s1 may be equal to RCX */
			M_IST(s1, REG_SP, d);
			M_INTMOVE(s2, RCX);
			emit_shiftl_membase(cd, shift_op, REG_SP, d);
		}

		M_INTMOVE(REG_ITMP1, RCX);                             /* restore RCX */

	} else {
		d_old = d;
		if (d == RCX) {
			d = REG_ITMP3;
		}
					
		if (IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			M_ILD(RCX, REG_SP, s2);
			M_ILD(d, REG_SP, s1);
			emit_shiftl_reg(cd, shift_op, d);

		} else if (IS_INMEMORY(v_s2->flags) && !IS_INMEMORY(v_s1->flags)) {
			/* s1 may be equal to RCX */
			M_INTMOVE(s1, d);
			M_ILD(RCX, REG_SP, s2);
			emit_shiftl_reg(cd, shift_op, d);

		} else if (!IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			M_INTMOVE(s2, RCX);
			M_ILD(d, REG_SP, s1);
			emit_shiftl_reg(cd, shift_op, d);

		} else {
			/* s1 may be equal to RCX */
			if (s1 == RCX) {
				if (s2 == d) {
					/* d cannot be used to backup s1 since this would
					   overwrite s2. */
					M_INTMOVE(s1, REG_ITMP3);
					M_INTMOVE(s2, RCX);
					M_INTMOVE(REG_ITMP3, d);

				} else {
					M_INTMOVE(s1, d);
					M_INTMOVE(s2, RCX);
				}

			} else {
				/* d may be equal to s2 */
				M_INTMOVE(s2, RCX);
				M_INTMOVE(s1, d);
			}
			emit_shiftl_reg(cd, shift_op, d);
		}

		if (d_old == RCX)
			M_INTMOVE(REG_ITMP3, RCX);
		else
			M_INTMOVE(REG_ITMP1, RCX);                         /* restore RCX */
	}
}


void emit_lshift(jitdata *jd, s4 shift_op, instruction *iptr)
{
	s4 s1, s2, d, d_old;
	varinfo *v_s1,*v_s2,*v_dst;
	codegendata *cd;

	/* get required compiler data */

	cd = jd->cd;

	v_s1  = VAROP(iptr->s1);
	v_s2  = VAROP(iptr->sx.s23.s2);
	v_dst = VAROP(iptr->dst);

	s1 = v_s1->vv.regoff;
	s2 = v_s2->vv.regoff;
	d  = v_dst->vv.regoff;
	
	M_INTMOVE(RCX, REG_ITMP1);                                    /* save RCX */

	if (IS_INMEMORY(v_dst->flags)) {
		if (IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			if (s1 == d) {
				M_ILD(RCX, REG_SP, s2);
				emit_shift_membase(cd, shift_op, REG_SP, d);

			} else {
				M_ILD(RCX, REG_SP, s2);
				M_LLD(REG_ITMP2, REG_SP, s1);
				emit_shift_reg(cd, shift_op, REG_ITMP2);
				M_LST(REG_ITMP2, REG_SP, d);
			}

		} else if (IS_INMEMORY(v_s2->flags) && !IS_INMEMORY(v_s1->flags)) {
			/* s1 may be equal to RCX */
			if (s1 == RCX) {
				if (s2 == d) {
					M_ILD(REG_ITMP1, REG_SP, s2);
					M_LST(s1, REG_SP, d);
					M_INTMOVE(REG_ITMP1, RCX);

				} else {
					M_LST(s1, REG_SP, d);
					M_ILD(RCX, REG_SP, s2);
				}

			} else {
				M_ILD(RCX, REG_SP, s2);
				M_LST(s1, REG_SP, d);
			}

			emit_shift_membase(cd, shift_op, REG_SP, d);

		} else if (!IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			if (s1 == d) {
				M_INTMOVE(s2, RCX);
				emit_shift_membase(cd, shift_op, REG_SP, d);

			} else {
				M_INTMOVE(s2, RCX);
				M_LLD(REG_ITMP2, REG_SP, s1);
				emit_shift_reg(cd, shift_op, REG_ITMP2);
				M_LST(REG_ITMP2, REG_SP, d);
			}

		} else {
			/* s1 may be equal to RCX */
			M_LST(s1, REG_SP, d);
			M_INTMOVE(s2, RCX);
			emit_shift_membase(cd, shift_op, REG_SP, d);
		}

		M_INTMOVE(REG_ITMP1, RCX);                             /* restore RCX */

	} else {
		d_old = d;
		if (d == RCX) {
			d = REG_ITMP3;
		}

		if (IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			M_ILD(RCX, REG_SP, s2);
			M_LLD(d, REG_SP, s1);
			emit_shift_reg(cd, shift_op, d);

		} else if (IS_INMEMORY(v_s2->flags) && !IS_INMEMORY(v_s1->flags)) {
			/* s1 may be equal to RCX */
			M_INTMOVE(s1, d);
			M_ILD(RCX, REG_SP, s2);
			emit_shift_reg(cd, shift_op, d);

		} else if (!IS_INMEMORY(v_s2->flags) && IS_INMEMORY(v_s1->flags)) {
			M_INTMOVE(s2, RCX);
			M_LLD(d, REG_SP, s1);
			emit_shift_reg(cd, shift_op, d);

		} else {
			/* s1 may be equal to RCX */
			if (s1 == RCX) {
				if (s2 == d) {
					/* d cannot be used to backup s1 since this would
					   overwrite s2. */
					M_INTMOVE(s1, REG_ITMP3);
					M_INTMOVE(s2, RCX);
					M_INTMOVE(REG_ITMP3, d);

				} else {
					M_INTMOVE(s1, d);
					M_INTMOVE(s2, RCX);
				}

			} else {
				/* d may be equal to s2 */
				M_INTMOVE(s2, RCX);
				M_INTMOVE(s1, d);
			}
			emit_shift_reg(cd, shift_op, d);
		}

		if (d_old == RCX)
			M_INTMOVE(REG_ITMP3, RCX);
		else
			M_INTMOVE(REG_ITMP1, RCX);                         /* restore RCX */
	}
}


/* low-level code emitter functions *******************************************/

void emit_nop(codegendata *cd, int length)
{
    assert(length >= 1 && length <= 9);
    switch (length) {
    case 1:
        *(cd->mcodeptr++) = 0x90;
        break;
    case 2:
        *(cd->mcodeptr++) = 0x66;
        *(cd->mcodeptr++) = 0x90;
        break;
    case 3:
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 4:
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x40;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 5:
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x44;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 6:
        *(cd->mcodeptr++) = 0x66;
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x44;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 7:
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x80;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 8:
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x84;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        break;
    case 9:
        *(cd->mcodeptr++) = 0x66;
        *(cd->mcodeptr++) = 0x0f;
        *(cd->mcodeptr++) = 0x1f;
        *(cd->mcodeptr++) = 0x84;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        *(cd->mcodeptr++) = 0x00;
        break;
    }
}
 
void emit_arbitrary_nop(codegendata *cd, int disp)
{
	while (disp) {
		int x = disp < 9 ? disp : 9;
		emit_nop(cd, x);
		disp -= x;
	}
}

void emit_mov_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x89;
	emit_reg((reg),(dreg));
}


void emit_mov_imm_reg(codegendata *cd, s8 imm, s8 reg)
{
	emit_rex(1,0,0,(reg));
	*(cd->mcodeptr++) = 0xb8 + ((reg) & 0x07);
	emit_imm64((imm));
}


void emit_movl_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(0,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x89;
	emit_reg((reg),(dreg));
}


void emit_movl_imm_reg(codegendata *cd, s8 imm, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xb8 + ((reg) & 0x07);
	emit_imm32((imm));
}


void emit_mov_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg) {
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_membase(cd, (basereg),(disp),(reg));
}


/*
 * this one is for INVOKEVIRTUAL/INVOKEINTERFACE to have a
 * constant membase immediate length of 32bit
 */
void emit_mov_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg) {
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_movl_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg)
{
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_membase(cd, (basereg),(disp),(reg));
}


/* ATTENTION: Always emit a REX byte, because the instruction size can
   be smaller when all register indexes are smaller than 7. */
void emit_movl_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg)
{
	emit_byte_rex((reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_mov_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_mov_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_movl_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_membase(cd, (basereg),(disp),(reg));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movl_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	emit_byte_rex((reg),0,(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_mov_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg) {
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movl_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg) {
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x8b;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movl_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movw_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x89;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movb_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	emit_byte_rex((reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x88;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_imm_membase(codegendata *cd, s8 imm, s8 basereg, s8 disp) {
	emit_rex(1,0,0,(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_membase(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


void emit_mov_imm_membase32(codegendata *cd, s8 imm, s8 basereg, s8 disp) {
	emit_rex(1,0,0,(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_membase32(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


void emit_movl_imm_membase(codegendata *cd, s8 imm, s8 basereg, s8 disp) {
	emit_rex(0,0,0,(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_membase(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movl_imm_membase32(codegendata *cd, s8 imm, s8 basereg, s8 disp) {
	emit_byte_rex(0,0,(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_membase32(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


void emit_movsbq_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbe;
	/* XXX: why do reg and dreg have to be exchanged */
	emit_reg((dreg),(reg));
}


void emit_movswq_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbf;
	/* XXX: why do reg and dreg have to be exchanged */
	emit_reg((dreg),(reg));
}


void emit_movslq_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x63;
	/* XXX: why do reg and dreg have to be exchanged */
	emit_reg((dreg),(reg));
}


void emit_movzbq_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb6;
	/* XXX: why do reg and dreg have to be exchanged */
	emit_reg((dreg),(reg));
}


void emit_movzwq_reg_reg(codegendata *cd, s8 reg, s8 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb7;
	/* XXX: why do reg and dreg have to be exchanged */
	emit_reg((dreg),(reg));
}


void emit_movswq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg) {
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbf;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movsbq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg) {
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbe;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movzwq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg) {
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb7;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	emit_rex(1,0,(indexreg),(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm32((imm));
}


void emit_movl_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	emit_rex(0,0,(indexreg),(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm32((imm));
}


void emit_movw_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,0,(indexreg),(basereg));
	*(cd->mcodeptr++) = 0xc7;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm16((imm));
}


void emit_movb_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	emit_rex(0,0,(indexreg),(basereg));
	*(cd->mcodeptr++) = 0xc6;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm8((imm));
}


void emit_mov_mem_reg(codegendata *cd, s4 disp, s4 dreg)
{
	emit_rex(1, dreg, 0, 0);
	*(cd->mcodeptr++) = 0x8b;
	emit_address_byte(0, dreg, 4);
	emit_mem(4, disp);
}


/*
 * alu operations
 */
void emit_alu_reg_reg(codegendata *cd, s8 opc, s8 reg, s8 dreg)
{
	emit_rex(1,(reg),0,(dreg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 1;
	emit_reg((reg),(dreg));
}


void emit_alul_reg_reg(codegendata *cd, s8 opc, s8 reg, s8 dreg)
{
	emit_rex(0,(reg),0,(dreg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 1;
	emit_reg((reg),(dreg));
}


void emit_alu_reg_membase(codegendata *cd, s8 opc, s8 reg, s8 basereg, s8 disp)
{
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 1;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alul_reg_membase(codegendata *cd, s8 opc, s8 reg, s8 basereg, s8 disp)
{
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 1;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alu_membase_reg(codegendata *cd, s8 opc, s8 basereg, s8 disp, s8 reg)
{
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 3;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alul_membase_reg(codegendata *cd, s8 opc, s8 basereg, s8 disp, s8 reg)
{
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 3;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alu_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg) {
	if (IS_IMM8(imm)) {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0x83;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	} else {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0x81;
		emit_reg((opc),(dreg));
		emit_imm32((imm));
	}
}


void emit_alu_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg)
{
	emit_rex(1,0,0,(dreg));
	*(cd->mcodeptr++) = 0x81;
	emit_reg((opc),(dreg));
	emit_imm32((imm));
}


void emit_alul_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg)
{
	emit_rex(0,0,0,(dreg));
	*(cd->mcodeptr++) = 0x81;
	emit_reg((opc),(dreg));
	emit_imm32((imm));
}


void emit_alul_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg) {
	if (IS_IMM8(imm)) {
		emit_rex(0,0,0,(dreg));
		*(cd->mcodeptr++) = 0x83;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	} else {
		emit_rex(0,0,0,(dreg));
		*(cd->mcodeptr++) = 0x81;
		emit_reg((opc),(dreg));
		emit_imm32((imm));
	}
}


void emit_alu_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp) {
	if (IS_IMM8(imm)) {
		emit_rex(1,0,0,(basereg));
		*(cd->mcodeptr++) = 0x83;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm8((imm));
	} else {
		emit_rex(1,0,0,(basereg));
		*(cd->mcodeptr++) = 0x81;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm32((imm));
	}
}


void emit_alul_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp) {
	if (IS_IMM8(imm)) {
		emit_rex(0,0,0,(basereg));
		*(cd->mcodeptr++) = 0x83;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm8((imm));
	} else {
		emit_rex(0,0,0,(basereg));
		*(cd->mcodeptr++) = 0x81;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm32((imm));
	}
}

void emit_alu_memindex_reg(codegendata *cd, s8 opc, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg)
{
	emit_rex(1,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 3;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}

void emit_alul_memindex_reg(codegendata *cd, s8 opc, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg)
{
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = (((opc)) << 3) + 3;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}

void emit_test_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(1,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x85;
	emit_reg((reg),(dreg));
}


void emit_testl_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(0,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x85;
	emit_reg((reg),(dreg));
}


void emit_test_imm_reg(codegendata *cd, s8 imm, s8 reg) {
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(0,(reg));
	emit_imm32((imm));
}


void emit_testw_imm_reg(codegendata *cd, s8 imm, s8 reg) {
	*(cd->mcodeptr++) = 0x66;
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(0,(reg));
	emit_imm16((imm));
}


void emit_testb_imm_reg(codegendata *cd, s8 imm, s8 reg) {
	*(cd->mcodeptr++) = 0xf6;
	emit_reg(0,(reg));
	emit_imm8((imm));
}


void emit_lea_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg) {
	emit_rex(1,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8d;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_leal_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg) {
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x8d;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_incl_reg(codegendata *cd, s8 reg)
{
	*(cd->mcodeptr++) = 0xff;
	emit_reg(0,(reg));
}

void emit_incq_reg(codegendata *cd, s8 reg)
{
	emit_rex(1,0,0,(reg));
	*(cd->mcodeptr++) = 0xff;
	emit_reg(0,(reg));
}

void emit_incl_membase(codegendata *cd, s8 basereg, s8 disp)
{
	emit_rex(0,0,0,(basereg));
	*(cd->mcodeptr++) = 0xff;
	emit_membase(cd, (basereg),(disp),0);
}

void emit_incq_membase(codegendata *cd, s8 basereg, s8 disp)
{
	emit_rex(1,0,0,(basereg));
	*(cd->mcodeptr++) = 0xff;
	emit_membase(cd, (basereg),(disp),0);
}



void emit_cltd(codegendata *cd) {
    *(cd->mcodeptr++) = 0x99;
}


void emit_cqto(codegendata *cd) {
	emit_rex(1,0,0,0);
	*(cd->mcodeptr++) = 0x99;
}



void emit_imul_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_reg((dreg),(reg));
}


void emit_imull_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_reg((dreg),(reg));
}


void emit_imul_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	emit_rex(1,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_imull_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_imul_imm_reg(codegendata *cd, s8 imm, s8 dreg) {
	if (IS_IMM8((imm))) {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0x6b;
		emit_reg(0,(dreg));
		emit_imm8((imm));
	} else {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0x69;
		emit_reg(0,(dreg));
		emit_imm32((imm));
	}
}


void emit_imul_imm_reg_reg(codegendata *cd, s8 imm, s8 reg, s8 dreg) {
	if (IS_IMM8((imm))) {
		emit_rex(1,(dreg),0,(reg));
		*(cd->mcodeptr++) = 0x6b;
		emit_reg((dreg),(reg));
		emit_imm8((imm));
	} else {
		emit_rex(1,(dreg),0,(reg));
		*(cd->mcodeptr++) = 0x69;
		emit_reg((dreg),(reg));
		emit_imm32((imm));
	}
}


void emit_imull_imm_reg_reg(codegendata *cd, s8 imm, s8 reg, s8 dreg) {
	if (IS_IMM8((imm))) {
		emit_rex(0,(dreg),0,(reg));
		*(cd->mcodeptr++) = 0x6b;
		emit_reg((dreg),(reg));
		emit_imm8((imm));
	} else {
		emit_rex(0,(dreg),0,(reg));
		*(cd->mcodeptr++) = 0x69;
		emit_reg((dreg),(reg));
		emit_imm32((imm));
	}
}


void emit_imul_imm_membase_reg(codegendata *cd, s8 imm, s8 basereg, s8 disp, s8 dreg) {
	if (IS_IMM8((imm))) {
		emit_rex(1,(dreg),0,(basereg));
		*(cd->mcodeptr++) = 0x6b;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm8((imm));
	} else {
		emit_rex(1,(dreg),0,(basereg));
		*(cd->mcodeptr++) = 0x69;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm32((imm));
	}
}


void emit_imull_imm_membase_reg(codegendata *cd, s8 imm, s8 basereg, s8 disp, s8 dreg) {
	if (IS_IMM8((imm))) {
		emit_rex(0,(dreg),0,(basereg));
		*(cd->mcodeptr++) = 0x6b;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm8((imm));
	} else {
		emit_rex(0,(dreg),0,(basereg));
		*(cd->mcodeptr++) = 0x69;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm32((imm));
	}
}


void emit_idiv_reg(codegendata *cd, s8 reg) {
	emit_rex(1,0,0,(reg));
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(7,(reg));
}


void emit_idivl_reg(codegendata *cd, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(7,(reg));
}



/*
 * shift ops
 */
void emit_shift_reg(codegendata *cd, s8 opc, s8 reg) {
	emit_rex(1,0,0,(reg));
	*(cd->mcodeptr++) = 0xd3;
	emit_reg((opc),(reg));
}


void emit_shiftl_reg(codegendata *cd, s8 opc, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xd3;
	emit_reg((opc),(reg));
}


void emit_shift_membase(codegendata *cd, s8 opc, s8 basereg, s8 disp) {
	emit_rex(1,0,0,(basereg));
	*(cd->mcodeptr++) = 0xd3;
	emit_membase(cd, (basereg),(disp),(opc));
}


void emit_shiftl_membase(codegendata *cd, s8 opc, s8 basereg, s8 disp) {
	emit_rex(0,0,0,(basereg));
	*(cd->mcodeptr++) = 0xd3;
	emit_membase(cd, (basereg),(disp),(opc));
}


void emit_shift_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg) {
	if ((imm) == 1) {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0xd1;
		emit_reg((opc),(dreg));
	} else {
		emit_rex(1,0,0,(dreg));
		*(cd->mcodeptr++) = 0xc1;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	}
}


void emit_shiftl_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg) {
	if ((imm) == 1) {
		emit_rex(0,0,0,(dreg));
		*(cd->mcodeptr++) = 0xd1;
		emit_reg((opc),(dreg));
	} else {
		emit_rex(0,0,0,(dreg));
		*(cd->mcodeptr++) = 0xc1;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	}
}


void emit_shift_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp) {
	if ((imm) == 1) {
		emit_rex(1,0,0,(basereg));
		*(cd->mcodeptr++) = 0xd1;
		emit_membase(cd, (basereg),(disp),(opc));
	} else {
		emit_rex(1,0,0,(basereg));
		*(cd->mcodeptr++) = 0xc1;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm8((imm));
	}
}


void emit_shiftl_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp) {
	if ((imm) == 1) {
		emit_rex(0,0,0,(basereg));
		*(cd->mcodeptr++) = 0xd1;
		emit_membase(cd, (basereg),(disp),(opc));
	} else {
		emit_rex(0,0,0,(basereg));
		*(cd->mcodeptr++) = 0xc1;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm8((imm));
	}
}



/*
 * jump operations
 */
void emit_jmp_imm(codegendata *cd, s8 imm) {
	*(cd->mcodeptr++) = 0xe9;
	emit_imm32((imm));
}

/* like emit_jmp_imm but allows 8 bit optimization */
void emit_jmp_imm2(codegendata *cd, s8 imm) {
	if (IS_IMM8(imm)) {
		*(cd->mcodeptr++) = 0xeb;
		emit_imm8((imm));
	}
	else {
		*(cd->mcodeptr++) = 0xe9;
		emit_imm32((imm));
	}
}


void emit_jmp_reg(codegendata *cd, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xff;
	emit_reg(4,(reg));
}


void emit_jcc(codegendata *cd, s8 opc, s8 imm) {
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = (0x80 + (opc));
	emit_imm32((imm));
}



/*
 * conditional set and move operations
 */

/* we need the rex byte to get all low bytes */
void emit_setcc_reg(codegendata *cd, s4 opc, s4 reg)
{
	*(cd->mcodeptr++) = (0x40 | (((reg) >> 3) & 0x01));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = (0x90 + (opc));
	emit_reg(0,(reg));
}


/* we need the rex byte to get all low bytes */
void emit_setcc_membase(codegendata *cd, s4 opc, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = (0x40 | (((basereg) >> 3) & 0x01));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = (0x90 + (opc));
	emit_membase(cd, (basereg),(disp),0);
}


void emit_cmovcc_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg)
{
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = (0x40 + (opc));
	emit_reg((dreg),(reg));
}


void emit_cmovccl_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg)
{
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = (0x40 + (opc));
	emit_reg((dreg),(reg));
}


void emit_neg_reg(codegendata *cd, s8 reg)
{
	emit_rex(1,0,0,(reg));
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(3,(reg));
}


void emit_negl_reg(codegendata *cd, s8 reg)
{
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(3,(reg));
}


void emit_push_reg(codegendata *cd, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0x50 + (0x07 & (reg));
}


void emit_push_imm(codegendata *cd, s8 imm) {
	*(cd->mcodeptr++) = 0x68;
	emit_imm32((imm));
}


void emit_pop_reg(codegendata *cd, s8 reg) {
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0x58 + (0x07 & (reg));
}


void emit_xchg_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(1,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x87;
	emit_reg((reg),(dreg));
}



/*
 * call instructions
 */
void emit_call_reg(codegendata *cd, s8 reg)
{
	emit_rex(0,0,0,(reg));
	*(cd->mcodeptr++) = 0xff;
	emit_reg(2,(reg));
}


void emit_call_imm(codegendata *cd, s8 imm)
{
	*(cd->mcodeptr++) = 0xe8;
	emit_imm32((imm));
}


void emit_call_mem(codegendata *cd, ptrint mem)
{
	*(cd->mcodeptr++) = 0xff;
	emit_mem(2,(mem));
}



/*
 * floating point instructions (SSE2)
 */
void emit_addsd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x58;
	emit_reg((dreg),(reg));
}


void emit_addss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x58;
	emit_reg((dreg),(reg));
}


void emit_cvtsi2ssq_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2a;
	emit_reg((dreg),(reg));
}


void emit_cvtsi2ss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2a;
	emit_reg((dreg),(reg));
}


void emit_cvtsi2sdq_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2a;
	emit_reg((dreg),(reg));
}


void emit_cvtsi2sd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2a;
	emit_reg((dreg),(reg));
}


void emit_cvtss2sd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5a;
	emit_reg((dreg),(reg));
}


void emit_cvtsd2ss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5a;
	emit_reg((dreg),(reg));
}


void emit_cvttss2siq_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2c;
	emit_reg((dreg),(reg));
}


void emit_cvttss2si_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2c;
	emit_reg((dreg),(reg));
}


void emit_cvttsd2siq_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(1,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2c;
	emit_reg((dreg),(reg));
}


void emit_cvttsd2si_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2c;
	emit_reg((dreg),(reg));
}


void emit_divss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5e;
	emit_reg((dreg),(reg));
}


void emit_divsd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5e;
	emit_reg((dreg),(reg));
}


void emit_movd_reg_freg(codegendata *cd, s8 reg, s8 freg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(1,(freg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x6e;
	emit_reg((freg),(reg));
}


void emit_movd_freg_reg(codegendata *cd, s8 freg, s8 reg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(1,(freg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x7e;
	emit_reg((freg),(reg));
}


void emit_movd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x7e;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_movd_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x7e;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(1,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x6e;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_movdl_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x6e;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_movd_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x6e;
	emit_memindex(cd, (dreg),(disp),(basereg),(indexreg),(scale));
}


void emit_movq_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x7e;
	emit_reg((dreg),(reg));
}


void emit_movq_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xd6;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_movq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x7e;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_movss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_reg((reg),(dreg));
}


void emit_movsd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(reg),0,(dreg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_reg((reg),(dreg));
}


void emit_movss_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_membase(cd, (basereg),(disp),(reg));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movss_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0xf3;
	emit_byte_rex((reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_movsd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_membase(cd, (basereg),(disp),(reg));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movsd_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp) {
	*(cd->mcodeptr++) = 0xf2;
	emit_byte_rex((reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_movss_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_membase(cd, (basereg),(disp),(dreg));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movss_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_byte_rex((dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_membase32(cd, (basereg),(disp),(dreg));
}


void emit_movlps_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg)
{
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x12;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_movlps_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp)
{
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x13;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_movsd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_membase(cd, (basereg),(disp),(dreg));
}


/* Always emit a REX byte, because the instruction size can be smaller when   */
/* all register indexes are smaller than 7.                                   */
void emit_movsd_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_byte_rex((dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_membase32(cd, (basereg),(disp),(dreg));
}


void emit_movlpd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg)
{
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x12;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_movlpd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp)
{
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(reg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x13;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_movss_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movsd_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(reg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x11;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movss_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_memindex(cd, (dreg),(disp),(basereg),(indexreg),(scale));
}


void emit_movsd_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),(indexreg),(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x10;
	emit_memindex(cd, (dreg),(disp),(basereg),(indexreg),(scale));
}


void emit_mulss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x59;
	emit_reg((dreg),(reg));
}


void emit_mulsd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x59;
	emit_reg((dreg),(reg));
}


void emit_subss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf3;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5c;
	emit_reg((dreg),(reg));
}


void emit_subsd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0xf2;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x5c;
	emit_reg((dreg),(reg));
}


void emit_ucomiss_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2e;
	emit_reg((dreg),(reg));
}


void emit_ucomisd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x2e;
	emit_reg((dreg),(reg));
}


void emit_xorps_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x57;
	emit_reg((dreg),(reg));
}


void emit_xorps_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x57;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_xorpd_reg_reg(codegendata *cd, s8 reg, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),0,(reg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x57;
	emit_reg((dreg),(reg));
}


void emit_xorpd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg) {
	*(cd->mcodeptr++) = 0x66;
	emit_rex(0,(dreg),0,(basereg));
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x57;
	emit_membase(cd, (basereg),(disp),(dreg));
}


/* system instructions ********************************************************/

void emit_rdtsc(codegendata *cd)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x31;
}

void emit_mfence(codegendata *cd)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xae;
	*(cd->mcodeptr++) = 0xf0;
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
