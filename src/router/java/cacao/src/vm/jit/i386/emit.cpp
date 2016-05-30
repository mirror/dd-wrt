/* src/vm/jit/i386/emit.cpp - i386 code emitter functions

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

#include "vm/types.hpp"
#include "vm/os.hpp"

#include "vm/jit/i386/codegen.hpp"
#include "vm/jit/i386/emit.hpp"
#include "vm/jit/i386/md-abi.hpp"

#include "mm/memory.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/options.hpp"
#include "vm/statistics.hpp"

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


STAT_DECLARE_VAR(int,count_mov_reg_reg,0)
STAT_DECLARE_VAR(int,count_mov_mem_reg,0)
STAT_DECLARE_VAR(int,count_mov_reg_mem,0)
STAT_DECLARE_VAR(int,count_mov_mem_mem,0)

/* emit_load ******************************************************************

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


/* emit_load_low ************************************************************

   Emits a possible load of the low 32-bits of an operand.

*******************************************************************************/

s4 emit_load_low(jitdata *jd, instruction *iptr, varinfo *src,s4 tempreg)
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
		reg = GET_LOW_REG(src->vv.regoff);

	return reg;
}


/* emit_load_high ***********************************************************

   Emits a possible load of the high 32-bits of an operand.

*******************************************************************************/

s4 emit_load_high(jitdata *jd, instruction *iptr,varinfo *src,s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	/* get required compiler data */

	assert(src->type == TYPE_LNG);

	cd = jd->cd;

	if (IS_INMEMORY(src->flags)) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

		M_ILD(tempreg, REG_SP, disp + 4);

		reg = tempreg;
	}
	else
		reg = GET_HIGH_REG(src->vv.regoff);

	return reg;
}


/* emit_store ******************************************************************

   Emits a possible store of the destination operand.

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


/* emit_store_low **************************************************************

   Emits a possible store of the low 32-bits of the destination
   operand.

*******************************************************************************/

void emit_store_low(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;

	assert(dst->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(dst->flags)) {
		COUNT_SPILLS;
		M_IST(GET_LOW_REG(d), REG_SP, dst->vv.regoff);
	}
}


/* emit_store_high *************************************************************

   Emits a possible store of the high 32-bits of the destination
   operand.

*******************************************************************************/

void emit_store_high(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;

	assert(dst->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(dst->flags)) {
		COUNT_SPILLS;
		M_IST(GET_HIGH_REG(d), REG_SP, dst->vv.regoff + 4);
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
			if (IS_LNG_TYPE(src->type))
				d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP12_PACKED);
			else
				d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP1);

			s1 = emit_load(jd, iptr, src, d);
		}
		else {
			if (IS_LNG_TYPE(src->type))
				s1 = emit_load(jd, iptr, src, REG_ITMP12_PACKED);
			else
				s1 = emit_load(jd, iptr, src, REG_ITMP1);

			d = codegen_reg_of_var(iptr->opc, dst, s1);
		}

		if (s1 != d) {
			switch (src->type) {
			case TYPE_INT:
			case TYPE_ADR:
				M_MOV(s1, d);
				break;
			case TYPE_LNG:
				M_LNGMOVE(s1, d);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
/* 				M_FMOV(s1, d); */
				break;
			default:
				vm_abort("emit_copy: unknown type %d", src->type);
				break;
			}
		}

		emit_store(jd, iptr, dst, d);
	}
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	M_CMP_IMM(value, reg);
}


/* emit_branch *****************************************************************

   Emits the code for conditional and unconditional branchs.

*******************************************************************************/

void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 options)
{
	s4 branchdisp;

	/* ATTENTION: a displacement overflow cannot happen */

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
			M_BB(branchdisp);
			break;
		case BRANCH_ULE:
			M_BBE(branchdisp);
			break;
		case BRANCH_UGE:
			M_BAE(branchdisp);
			break;
		case BRANCH_UGT:
			M_BA(branchdisp);
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
		M_BNE(6);
		M_ALD_MEM(reg, TRAP_ArithmeticException);
	}
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
        M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
        M_CMP(REG_ITMP3, s2);
        M_BB(6);
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
		M_BNE(6);
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
			M_BGT(6);
			break;
		case BRANCH_GE:
			M_BLT(6);
			break;
		case BRANCH_EQ:
			M_BNE(6);
			break;
		case BRANCH_NE:
			M_BEQ(6);
			break;
		case BRANCH_ULE:
			M_BBE(6);
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
		M_BNE(6);
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
		M_BNE(6);
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

/* emit_trap_countdown *********************************************************

   Emit a countdown trap.

   counter....absolute address of the counter variable

*******************************************************************************/

void emit_trap_countdown(codegendata *cd, s4 *counter)
{
	M_ISUB_IMM_MEMABS(1, (s4) counter);
	M_BNS(6);
	M_ALD_MEM(REG_METHODPTR, TRAP_COUNTDOWN);
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

	/* Get machine code which is patched back in later. The
	   trap is 2 bytes long. */

	mcode = *((uint16_t *) cd->mcodeptr);

#if 0
	/* XXX this breaks GDB, so we disable it for now */
	*(cd->mcodeptr++) = 0xcc;
	M_INT3;
#else
	M_UD2;
#endif

	return (uint32_t) mcode;
}


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int align_off;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

	align_off = cd->stackframesize ? 4 : 0;

	if (m->flags & ACC_STATIC) {
		M_MOV_IMM(&m->clazz->object.header, REG_ITMP1);
	}
	else {
		M_ALD(REG_ITMP1, REG_SP, cd->stackframesize * 8 + 4 + align_off);
		M_TEST(REG_ITMP1);
		M_BNE(6);
		M_ALD_MEM(REG_ITMP1, TRAP_NullPointerException);
	}

	M_AST(REG_ITMP1, REG_SP, syncslot_offset);
	M_AST(REG_ITMP1, REG_SP, 0 * 4);
	M_MOV_IMM(LOCK_monitor_enter, REG_ITMP3);
	M_CALL(REG_ITMP3);
}


/**
 * Generates synchronization code to leave a monitor.
 */
void emit_monitor_exit(jitdata* jd, int32_t syncslot_offset)
{
	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

	M_ALD(REG_ITMP2, REG_SP, syncslot_offset);

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		M_IST(REG_RESULT, REG_SP, syncslot_offset);
		break;

	case TYPE_LNG:
		M_LST(REG_RESULT_PACKED, REG_SP, syncslot_offset);
		break;

	case TYPE_FLT:
		emit_fstps_membase(cd, REG_SP, syncslot_offset);
		break;

	case TYPE_DBL:
		emit_fstpl_membase(cd, REG_SP, syncslot_offset);
		break;

	case TYPE_VOID:
		break;

	default:
		assert(false);
		break;
	}

	M_AST(REG_ITMP2, REG_SP, 0);
	M_MOV_IMM(LOCK_monitor_exit, REG_ITMP3);
	M_CALL(REG_ITMP3);

	/* and now restore the proper return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT, REG_SP, syncslot_offset);
		break;

	case TYPE_LNG:
		M_LLD(REG_RESULT_PACKED, REG_SP, syncslot_offset);
		break;

	case TYPE_FLT:
		emit_flds_membase(cd, REG_SP, syncslot_offset);
		break;

	case TYPE_DBL:
		emit_fldl_membase(cd, REG_SP, syncslot_offset);
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
	M_IADD_IMM_MEMBASE(1, REG_ITMP3, OFFSET(codeinfo, frequency));
}
#endif


/**
 * Emit profiling code for basicblock frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr)
{
	M_MOV_IMM(code->bbfrequency, REG_ITMP3);
	M_IADD_IMM_MEMBASE(1, REG_ITMP3, bptr->nr * 4);
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

#if !defined(NDEBUG)
void emit_verbosecall_enter(jitdata *jd)
{
	methodinfo   *m;
	codeinfo     *code;
	codegendata  *cd;
	registerdata *rd;
	//methoddesc   *md;
	int32_t       stackframesize;
	int           i;
	int           align_off;             /* offset for alignment compensation */

	if (!JITDATA_HAS_FLAG_VERBOSECALL(jd))
		return;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;
	rd   = jd->rd;

	//md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* keep stack 16-byte aligned */

	stackframesize = 2 + TMP_CNT;
	ALIGN_2(stackframesize);

	M_ASUB_IMM(stackframesize * 8, REG_SP);

	/* save temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_TMP_CNT; i++)
			M_IST(rd->tmpintregs[i], REG_SP, (2 + i) * 8);
	}

	/* no argument registers to save */

	align_off = cd->stackframesize ? 4 : 0;
	M_AST_IMM(m, REG_SP, 0 * 4);
	M_AST_IMM(0, REG_SP, 1 * 4);
	M_AST(REG_SP, REG_SP, 2 * 4);
	M_IADD_IMM_MEMBASE(stackframesize * 8 + cd->stackframesize * 8 + 4 + align_off, REG_SP, 2 * 4);
	M_MOV_IMM(trace_java_call_enter, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* no argument registers to restore */

	/* restore temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_TMP_CNT; i++)
			M_ILD(rd->tmpintregs[i], REG_SP, (2 + i) * 8);
	}

	M_AADD_IMM(stackframesize * 8, REG_SP);

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

	if (!JITDATA_HAS_FLAG_VERBOSECALL(jd))
		return;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	//rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* keep stack 16-byte aligned */

	M_ASUB_IMM(4 + 4 + 8, REG_SP);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
		M_IST(REG_RESULT, REG_SP, 2 * 4);
		break;
	case TYPE_LNG:
		M_LST(REG_RESULT_PACKED, REG_SP, 2 * 4);
		break;
	case TYPE_FLT:
		M_FSTNP(REG_NULL, REG_SP, 2 * 4);
		break;
	case TYPE_DBL:
		M_DSTNP(REG_NULL, REG_SP, 2 * 4);
		break;
	default:
		assert(false);
		break;
	}

	M_AST_IMM(m, REG_SP, 0 * 4);
	M_AST(REG_SP, REG_SP, 1 * 4);
	M_IADD_IMM_MEMBASE(2 * 4, REG_SP, 1 * 4);
	M_MOV_IMM(trace_java_call_exit, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
		M_ILD(REG_RESULT, REG_SP, 2 * 4);
		break;
	case TYPE_LNG:
		M_LLD(REG_RESULT_PACKED, REG_SP, 2 * 4);
		break;
	default:
		assert(false);
		break;
	}

	M_AADD_IMM(4 + 4 + 8, REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


/* code generation functions **************************************************/

static void emit_membase(codegendata *cd, s4 basereg, s4 disp, s4 dreg)
{
	if (basereg == ESP) {
		if (disp == 0) {
			emit_address_byte(0, dreg, ESP);
			emit_address_byte(0, ESP, ESP);
		}
		else if (IS_IMM8(disp)) {
			emit_address_byte(1, dreg, ESP);
			emit_address_byte(0, ESP, ESP);
			emit_imm8(disp);
		}
		else {
			emit_address_byte(2, dreg, ESP);
			emit_address_byte(0, ESP, ESP);
			emit_imm32(disp);
		}
	}
	else if ((disp == 0) && (basereg != EBP)) {
		emit_address_byte(0, dreg, basereg);
	}
	else if (IS_IMM8(disp)) {
		emit_address_byte(1, dreg, basereg);
		emit_imm8(disp);
	}
	else {
		emit_address_byte(2, dreg, basereg);
		emit_imm32(disp);
	}
}


static void emit_membase32(codegendata *cd, s4 basereg, s4 disp, s4 dreg)
{
	if (basereg == ESP) {
		emit_address_byte(2, dreg, ESP);
		emit_address_byte(0, ESP, ESP);
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
	else if ((disp == 0) && (basereg != EBP)) {
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


/* low-level code emitter functions *******************************************/

void emit_mov_reg_reg(codegendata *cd, s4 reg, s4 dreg)
{
	STATISTICS(count_mov_reg_reg++);
	*(cd->mcodeptr++) = 0x89;
	emit_reg((reg),(dreg));
}


void emit_mov_imm_reg(codegendata *cd, s4 imm, s4 reg)
{
	*(cd->mcodeptr++) = 0xb8 + ((reg) & 0x07);
	emit_imm32((imm));
}

/* 2-byte opcode for use with patchers */
void emit_mov_imm2_reg(codegendata *cd, s4 imm, s4 reg)
{
	*(cd->mcodeptr++) = 0xc7;
	emit_address_byte(3, 0, reg);
	emit_imm32((imm));
}



void emit_movb_imm_reg(codegendata *cd, s4 imm, s4 reg)
{
	*(cd->mcodeptr++) = 0xc6;
	emit_reg(0,(reg));
	emit_imm8((imm));
}


void emit_mov_membase_reg(codegendata *cd, s4 basereg, s4 disp, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x8b;
	emit_membase(cd, (basereg),(disp),(reg));
}


/*
 * this one is for INVOKEVIRTUAL/INVOKEINTERFACE to have a
 * constant membase immediate length of 32bit
 */
void emit_mov_membase32_reg(codegendata *cd, s4 basereg, s4 disp, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x8b;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_mov_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x89;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_mov_reg_membase32(codegendata *cd, s4 reg, s4 basereg, s4 disp)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x89;
	emit_membase32(cd, (basereg),(disp),(reg));
}


void emit_mov_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x8b;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x89;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movw_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x66;
	*(cd->mcodeptr++) = 0x89;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movb_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x88;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_reg_mem(codegendata *cd, s4 reg, s4 mem)
{
	STATISTICS(count_mov_reg_mem++);
	*(cd->mcodeptr++) = 0x89;
	emit_mem((reg),(mem));
}


void emit_mov_mem_reg(codegendata *cd, s4 mem, s4 dreg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x8b;
	emit_mem((dreg),(mem));
}


void emit_mov_imm_mem(codegendata *cd, s4 imm, s4 mem)
{
	*(cd->mcodeptr++) = 0xc7;
	emit_mem(0, mem);
	emit_imm32(imm);
}


void emit_mov_imm_membase(codegendata *cd, s4 imm, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xc7;
	emit_membase(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


void emit_mov_imm_membase32(codegendata *cd, s4 imm, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xc7;
	emit_membase32(cd, (basereg),(disp),0);
	emit_imm32((imm));
}


void emit_movb_imm_membase(codegendata *cd, s4 imm, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xc6;
	emit_membase(cd, (basereg),(disp),0);
	emit_imm8((imm));
}


void emit_movsbl_reg_reg(codegendata *cd, s4 a, s4 b)
{
	assert(a < 4);                     /* Can only operate on al, bl, cl, dl. */
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbe;
	emit_reg((b),(a));
}


void emit_movsbl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbe;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movswl_reg_reg(codegendata *cd, s4 a, s4 b)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbf;
	emit_reg((b),(a));
}


void emit_movswl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xbf;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_movzbl_reg_reg(codegendata *cd, s4 a, s4 b)
{
	assert(a < 4);                     /* Can only operate on al, bl, cl, dl. */
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb6;
	emit_reg((b),(a));
}


void emit_movzwl_reg_reg(codegendata *cd, s4 a, s4 b)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb7;
	emit_reg((b),(a));
}


void emit_movzwl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg)
{
	STATISTICS(count_mov_mem_reg++);
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xb7;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}


void emit_mov_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xc7;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm32((imm));
}


void emit_movw_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0x66;
	*(cd->mcodeptr++) = 0xc7;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm16((imm));
}


void emit_movb_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xc6;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
	emit_imm8((imm));
}


/*
 * alu operations
 */
void emit_alu_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = (((u1) (opc)) << 3) + 1;
	emit_reg((reg),(dreg));
}


void emit_alu_reg_membase(codegendata *cd, s4 opc, s4 reg, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = (((u1) (opc)) << 3) + 1;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alu_membase_reg(codegendata *cd, s4 opc, s4 basereg, s4 disp, s4 reg)
{
	*(cd->mcodeptr++) = (((u1) (opc)) << 3) + 3;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_alu_imm_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg)
{
	if (IS_IMM8(imm)) { 
		*(cd->mcodeptr++) = 0x83;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	} else { 
		*(cd->mcodeptr++) = 0x81;
		emit_reg((opc),(dreg));
		emit_imm32((imm));
	} 
}


void emit_alu_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg)
{
	*(cd->mcodeptr++) = 0x81;
	emit_reg((opc),(dreg));
	emit_imm32((imm));
}


void emit_alu_imm_membase(codegendata *cd, s4 opc, s4 imm, s4 basereg, s4 disp)
{
	if (IS_IMM8(imm)) { 
		*(cd->mcodeptr++) = 0x83;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm8((imm));
	} else { 
		*(cd->mcodeptr++) = 0x81;
		emit_membase(cd, (basereg),(disp),(opc));
		emit_imm32((imm));
	} 
}


void emit_alu_imm_memabs(codegendata *cd, s4 opc, s4 imm, s4 disp)
{
	if (IS_IMM8(imm)) { 
		*(cd->mcodeptr++) = 0x83;
		emit_mem(opc, disp);
		emit_imm8((imm));
	} else { 
		*(cd->mcodeptr++) = 0x81;
		emit_mem(opc, disp);
		emit_imm32((imm));
	}
}

void emit_alu_memindex_reg(codegendata *cd, s4 opc, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg)
{
	*(cd->mcodeptr++) = (((u1) (opc)) << 3) + 3;
	emit_memindex(cd, (reg),(disp),(basereg),(indexreg),(scale));
}

void emit_test_reg_reg(codegendata *cd, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x85;
	emit_reg((reg),(dreg));
}


void emit_test_imm_reg(codegendata *cd, s4 imm, s4 reg)
{
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(0,(reg));
	emit_imm32((imm));
}



/*
 * inc, dec operations
 */
void emit_inc_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xff;
	emit_reg(0,(reg));
}

void emit_dec_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xff;
	emit_mem(1,(mem));
}


void emit_imul_reg_reg(codegendata *cd, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_reg((dreg),(reg));
}


void emit_imul_membase_reg(codegendata *cd, s4 basereg, s4 disp, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xaf;
	emit_membase(cd, (basereg),(disp),(dreg));
}


void emit_imul_imm_reg(codegendata *cd, s4 imm, s4 dreg)
{
	if (IS_IMM8((imm))) { 
		*(cd->mcodeptr++) = 0x6b;
		emit_reg(0,(dreg));
		emit_imm8((imm));
	} else { 
		*(cd->mcodeptr++) = 0x69;
		emit_reg(0,(dreg));
		emit_imm32((imm));
	} 
}


void emit_imul_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg)
{
	if (IS_IMM8((imm))) { 
		*(cd->mcodeptr++) = 0x6b;
		emit_reg((dreg),(reg));
		emit_imm8((imm));
	} else { 
		*(cd->mcodeptr++) = 0x69;
		emit_reg((dreg),(reg));
		emit_imm32((imm));
	} 
}


void emit_imul_imm_membase_reg(codegendata *cd, s4 imm, s4 basereg, s4 disp, s4 dreg)
{
	if (IS_IMM8((imm))) {
		*(cd->mcodeptr++) = 0x6b;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm8((imm));
	} else {
		*(cd->mcodeptr++) = 0x69;
		emit_membase(cd, (basereg),(disp),(dreg));
		emit_imm32((imm));
	}
}


void emit_mul_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(4, reg);
}


void emit_mul_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xf7;
	emit_membase(cd, (basereg),(disp),4);
}


void emit_idiv_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(7,(reg));
}



/*
 * shift ops
 */
void emit_shift_reg(codegendata *cd, s4 opc, s4 reg)
{
	*(cd->mcodeptr++) = 0xd3;
	emit_reg((opc),(reg));
}


void emit_shift_imm_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg)
{
	if ((imm) == 1) {
		*(cd->mcodeptr++) = 0xd1;
		emit_reg((opc),(dreg));
	} else {
		*(cd->mcodeptr++) = 0xc1;
		emit_reg((opc),(dreg));
		emit_imm8((imm));
	}
}


void emit_shld_reg_reg(codegendata *cd, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xa5;
	emit_reg((reg),(dreg));
}


void emit_shld_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xa4;
	emit_reg((reg),(dreg));
	emit_imm8((imm));
}


void emit_shld_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xa5;
	emit_membase(cd, (basereg),(disp),(reg));
}


void emit_shrd_reg_reg(codegendata *cd, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xad;
	emit_reg((reg),(dreg));
}


void emit_shrd_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xac;
	emit_reg((reg),(dreg));
	emit_imm8((imm));
}


void emit_shrd_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xad;
	emit_membase(cd, (basereg),(disp),(reg));
}



/*
 * jump operations
 */
void emit_jmp_imm(codegendata *cd, s4 imm)
{
	*(cd->mcodeptr++) = 0xe9;
	emit_imm32((imm));
}


void emit_jmp_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xff;
	emit_reg(4,(reg));
}


void emit_jcc(codegendata *cd, s4 opc, s4 imm)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) =  0x80 + (u1) (opc);
	emit_imm32((imm));
}



/*
 * conditional set operations
 */
void emit_setcc_reg(codegendata *cd, s4 opc, s4 reg)
{
	assert(reg < 4);                     /* Can only operate on al, bl, cl, dl. */
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0x90 + (u1) (opc);
	emit_reg(0,(reg));
}


void emit_setcc_membase(codegendata *cd, s4 opc, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) =  0x90 + (u1) (opc);
	emit_membase(cd, (basereg),(disp),0);
}


void emit_xadd_reg_mem(codegendata *cd, s4 reg, s4 mem)
{
	*(cd->mcodeptr++) = 0x0f;
	*(cd->mcodeptr++) = 0xc1;
	emit_mem((reg),(mem));
}


void emit_neg_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xf7;
	emit_reg(3,(reg));
}



void emit_push_imm(codegendata *cd, s4 imm)
{
	*(cd->mcodeptr++) = 0x68;
	emit_imm32((imm));
}


void emit_pop_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0x58 + (0x07 & (u1) (reg));
}


void emit_push_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0x50 + (0x07 & (u1) (reg));
}


void emit_lock(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xf0;
}


/*
 * call instructions
 */
void emit_call_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xff;
	emit_reg(2,(reg));
}


void emit_call_imm(codegendata *cd, s4 imm)
{
	*(cd->mcodeptr++) = 0xe8;
	emit_imm32((imm));
}



/*
 * floating point instructions
 */
void emit_fld1(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xe8;
}


void emit_fldz(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xee;
}


void emit_fld_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xc0 + (0x07 & (u1) (reg));
}


void emit_flds_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase(cd, (basereg),(disp),0);
}


void emit_flds_membase32(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase32(cd, (basereg),(disp),0);
}


void emit_fldl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_membase(cd, (basereg),(disp),0);
}


void emit_fldl_membase32(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_membase32(cd, (basereg),(disp),0);
}


void emit_fldt_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdb;
	emit_membase(cd, (basereg),(disp),5);
}


void emit_flds_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
}


void emit_fldl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_memindex(cd, 0,(disp),(basereg),(indexreg),(scale));
}


void emit_flds_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_mem(0,(mem));
}


void emit_fldl_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_mem(0,(mem));
}


void emit_fildl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdb;
	emit_membase(cd, (basereg),(disp),0);
}


void emit_fildll_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdf;
	emit_membase(cd, (basereg),(disp),5);
}


void emit_fst_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xd0 + (0x07 & (u1) (reg));
}


void emit_fsts_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase(cd, (basereg),(disp),2);
}


void emit_fstl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_membase(cd, (basereg),(disp),2);
}


void emit_fsts_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_memindex(cd, 2,(disp),(basereg),(indexreg),(scale));
}


void emit_fstl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_memindex(cd, 2,(disp),(basereg),(indexreg),(scale));
}


void emit_fstp_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xd8 + (0x07 & (u1) (reg));
}


void emit_fstps_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase(cd, (basereg),(disp),3);
}


void emit_fstps_membase32(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase32(cd, (basereg),(disp),3);
}


void emit_fstpl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_membase(cd, (basereg),(disp),3);
}


void emit_fstpl_membase32(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_membase32(cd, (basereg),(disp),3);
}


void emit_fstpt_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdb;
	emit_membase(cd, (basereg),(disp),7);
}


void emit_fstps_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_memindex(cd, 3,(disp),(basereg),(indexreg),(scale));
}


void emit_fstpl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_memindex(cd, 3,(disp),(basereg),(indexreg),(scale));
}


void emit_fstps_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_mem(3,(mem));
}


void emit_fstpl_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xdd;
	emit_mem(3,(mem));
}


void emit_fistl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdb;
	emit_membase(cd, (basereg),(disp),2);
}


void emit_fistpl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdb;
	emit_membase(cd, (basereg),(disp),3);
}


void emit_fistpll_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdf;
	emit_membase(cd, (basereg),(disp),7);
}


void emit_fchs(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xe0;
}


void emit_faddp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xc1;
}


void emit_fadd_reg_st(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd8;
	*(cd->mcodeptr++) = 0xc0 + (0x0f & (u1) (reg));
}


void emit_fadd_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdc;
	*(cd->mcodeptr++) = 0xc0 + (0x0f & (u1) (reg));
}


void emit_faddp_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xc0 + (0x0f & (u1) (reg));
}


void emit_fadds_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd8;
	emit_membase(cd, (basereg),(disp),0);
}


void emit_faddl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdc;
	emit_membase(cd, (basereg),(disp),0);
}


void emit_fsub_reg_st(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd8;
	*(cd->mcodeptr++) = 0xe0 + (0x07 & (u1) (reg));
}


void emit_fsub_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdc;
	*(cd->mcodeptr++) = 0xe8 + (0x07 & (u1) (reg));
}


void emit_fsubp_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xe8 + (0x07 & (u1) (reg));
}


void emit_fsubp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xe9;
}


void emit_fsubs_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd8;
	emit_membase(cd, (basereg),(disp),4);
}


void emit_fsubl_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdc;
	emit_membase(cd, (basereg),(disp),4);
}


void emit_fmul_reg_st(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd8;
	*(cd->mcodeptr++) = 0xc8 + (0x07 & (u1) (reg));
}


void emit_fmul_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdc;
	*(cd->mcodeptr++) = 0xc8 + (0x07 & (u1) (reg));
}


void emit_fmulp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xc9;
}


void emit_fmulp_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xc8 + (0x07 & (u1) (reg));
}


void emit_fmuls_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd8;
	emit_membase(cd, (basereg),(disp),1);
}


void emit_fmull_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xdc;
	emit_membase(cd, (basereg),(disp),1);
}


void emit_fdiv_reg_st(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd8;
	*(cd->mcodeptr++) = 0xf0 + (0x07 & (u1) (reg));
}


void emit_fdiv_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdc;
	*(cd->mcodeptr++) = 0xf8 + (0x07 & (u1) (reg));
}


void emit_fdivp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xf9;
}


void emit_fdivp_st_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xde;
	*(cd->mcodeptr++) = 0xf8 + (0x07 & (u1) (reg));
}


void emit_fxch(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xc9;
}


void emit_fxch_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xc8 + (0x07 & (reg));
}


void emit_fprem(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xf8;
}


void emit_fprem1(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xf5;
}


void emit_fucom(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xe1;
}


void emit_fucom_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xe0 + (0x07 & (u1) (reg));
}


void emit_fucomp_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xe8 + (0x07 & (u1) (reg));
}


void emit_fucompp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xda;
	*(cd->mcodeptr++) = 0xe9;
}


void emit_fnstsw(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xdf;
	*(cd->mcodeptr++) = 0xe0;
}


void emit_sahf(codegendata *cd)
{
	*(cd->mcodeptr++) = 0x9e;
}


void emit_finit(codegendata *cd)
{
	*(cd->mcodeptr++) = 0x9b;
	*(cd->mcodeptr++) = 0xdb;
	*(cd->mcodeptr++) = 0xe3;
}


void emit_fldcw_mem(codegendata *cd, s4 mem)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_mem(5,(mem));
}


void emit_fldcw_membase(codegendata *cd, s4 basereg, s4 disp)
{
	*(cd->mcodeptr++) = 0xd9;
	emit_membase(cd, (basereg),(disp),5);
}


void emit_wait(codegendata *cd)
{
	*(cd->mcodeptr++) = 0x9b;
}


void emit_ffree_reg(codegendata *cd, s4 reg)
{
	*(cd->mcodeptr++) = 0xdd;
	*(cd->mcodeptr++) = 0xc0 + (0x07 & (u1) (reg));
}


void emit_fdecstp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xf6;
}


void emit_fincstp(codegendata *cd)
{
	*(cd->mcodeptr++) = 0xd9;
	*(cd->mcodeptr++) = 0xf7;
}

#if defined(ENABLE_ESCAPE_CHECK)
void emit_escape_check(codegendata *cd, s4 reg) {
	M_PUSH(reg);
	M_MOV_IMM(asm_escape_check, REG_ITMP3);
	M_CALL(REG_ITMP3);
	M_IADD_IMM(4, REG_SP);
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
 */
