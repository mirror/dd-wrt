/* src/vm/jit/arm/emit.cpp - Arm code emitter functions

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

#include "vm/jit/arm/emit.hpp"

#include "config.h"

#include <cassert>
#include <stdint.h>

#include "vm/types.hpp"

#include "md-abi.hpp"

#include "vm/jit/arm/codegen.hpp"

#include "mm/memory.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/global.hpp"

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

#include "toolbox/logging.hpp" /* XXX for debugging only */


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

#if defined(ENABLE_SOFTFLOAT)
		switch (src->type) {
		case TYPE_INT:
		case TYPE_FLT:
		case TYPE_ADR:
			M_ILD(tempreg, REG_SP, disp);
			break;
		case TYPE_LNG:
		case TYPE_DBL:
			M_LLD(tempreg, REG_SP, disp);
			break;
		default:
			vm_abort("emit_load: unknown type %d", src->type);
			break;
		}
#else
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
#endif

		reg = tempreg;
	}
	else
		reg = src->vv.regoff;

	return reg;
}


/* emit_load_low ***************************************************************

   Emits a possible load of the low 32-bits of a long source operand.

*******************************************************************************/

s4 emit_load_low(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	assert(src->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (src->flags & INMEMORY) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

#if defined(__ARMEL__)
		M_ILD(tempreg, REG_SP, disp);
#else
		M_ILD(tempreg, REG_SP, disp + 4);
#endif

		reg = tempreg;
	}
	else
		reg = GET_LOW_REG(src->vv.regoff);

	return reg;
}


/* emit_load_high **************************************************************

   Emits a possible load of the high 32-bits of a long source operand.

*******************************************************************************/

s4 emit_load_high(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg)
{
	codegendata  *cd;
	s4            disp;
	s4            reg;

	assert(src->type == TYPE_LNG);

	/* get required compiler data */

	cd = jd->cd;

	if (src->flags & INMEMORY) {
		COUNT_SPILLS;

		disp = src->vv.regoff;

#if defined(__ARMEL__)
		M_ILD(tempreg, REG_SP, disp + 4);
#else
		M_ILD(tempreg, REG_SP, disp);
#endif

		reg = tempreg;
	}
	else
		reg = GET_HIGH_REG(src->vv.regoff);

	return reg;
}


/* emit_store ******************************************************************

   Emits a possible store to a variable.

*******************************************************************************/

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;
	s4            disp;

	/* get required compiler data */

	cd = jd->cd;

	if (dst->flags & INMEMORY) {
		COUNT_SPILLS;

		disp = dst->vv.regoff;

#if defined(ENABLE_SOFTFLOAT)
		switch (dst->type) {
		case TYPE_INT:
		case TYPE_FLT:
		case TYPE_ADR:
			M_IST(d, REG_SP, disp);
			break;
		case TYPE_LNG:
		case TYPE_DBL:
			M_LST(d, REG_SP, disp);
			break;
		default:
			vm_abort("emit_store: unknown type %d", dst->type);
			break;
		}
#else
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
#endif
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

	/* XXX dummy call, removed me!!! */
	d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP1);

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
#if !defined(ENABLE_SOFTFLOAT)
			if (IS_FLT_DBL_TYPE(src->type))
				d = codegen_reg_of_var(iptr->opc, dst, REG_FTMP1);
			else
#endif
			{
				if (IS_2_WORD_TYPE(src->type))
					d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP12_PACKED);
				else
					d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP1);
			}

			s1 = emit_load(jd, iptr, src, d);
		}
		else {
#if !defined(ENABLE_SOFTFLOAT)
			if (IS_FLT_DBL_TYPE(src->type))
				s1 = emit_load(jd, iptr, src, REG_FTMP1);
			else
#endif
			{
				if (IS_2_WORD_TYPE(src->type))
					s1 = emit_load(jd, iptr, src, REG_ITMP12_PACKED);
				else
					s1 = emit_load(jd, iptr, src, REG_ITMP1);
			}

			d = codegen_reg_of_var(iptr->opc, dst, s1);
		}

		if (s1 != d) {
#if defined(ENABLE_SOFTFLOAT)
			switch (src->type) {
			case TYPE_INT:
			case TYPE_FLT:
			case TYPE_ADR:
				/* XXX grrrr, wrong direction! */
				M_MOV(d, s1);
				break;
			case TYPE_LNG:
			case TYPE_DBL:
				/* XXX grrrr, wrong direction! */
				M_MOV(GET_LOW_REG(d), GET_LOW_REG(s1));
				M_MOV(GET_HIGH_REG(d), GET_HIGH_REG(s1));
				break;
			default:
				vm_abort("emit_copy: unknown type %d", src->type);
				break;
			}
#else
			switch (src->type) {
			case TYPE_INT:
			case TYPE_ADR:
				/* XXX grrrr, wrong direction! */
				M_MOV(d, s1);
				break;
			case TYPE_LNG:
				/* XXX grrrr, wrong direction! */
				M_MOV(GET_LOW_REG(d), GET_LOW_REG(s1));
				M_MOV(GET_HIGH_REG(d), GET_HIGH_REG(s1));
				break;
			case TYPE_FLT:
				M_FMOV(s1, d);
				break;
			case TYPE_DBL:
				M_DMOV(s1, d);
				break;
			default:
				vm_abort("emit_copy: unknown type %d", src->type);
				break;
			}
#endif
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

	if (IS_IMM(value))
		M_MOV_IMM(d, value);
	else {
		disp = dseg_add_s4(cd, value);
		M_DSEG_LOAD(d, disp);
	}
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	int32_t disp;

	if (IS_IMM(value)) {
		M_CMP_IMM(reg, value);
	} else if (IS_IMM(-value)) {
		M_CMN_IMM(reg, -value);
	} else {
		assert(reg != REG_ITMP3);
		disp = dseg_add_s4(cd, value);
		M_DSEG_LOAD(REG_ITMP3, disp);
		M_CMP(reg, REG_ITMP3);
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

	checkdisp  = (disp - 8);
	branchdisp = (disp - 8) >> 2;

	/* check which branch to generate */

	if (condition == BRANCH_UNCONDITIONAL) {
		/* check displacement for overflow */

		if ((checkdisp < (s4) 0xff000000) || (checkdisp > (s4) 0x00ffffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit unconditional long-branch code");
		}
		else {
			M_B(branchdisp);
		}
	}
	else {
		/* and displacement for overflow */

		if ((checkdisp < (s4) 0xff000000) || (checkdisp > (s4) 0x00ffffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit conditional long-branch code");
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
			case BRANCH_UGT:
				M_BHI(branchdisp);
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
		CHECK_INT_REG(reg);
		M_TEQ_IMM(reg, 0);
		M_TRAPEQ(0, TRAP_ArithmeticException);
	}
}


/* emit_nullpointer_check ******************************************************

   Emit a NullPointerException check.

*******************************************************************************/

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(reg, reg);
		M_TRAPEQ(0, TRAP_NullPointerException);
	}
}

void emit_nullpointer_check_force(codegendata *cd, instruction *iptr, s4 reg)
{
	M_TST(reg, reg);
	M_TRAPEQ(0, TRAP_NullPointerException);
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_ILD_INTERN(REG_ITMP3, s1, OFFSET(java_array_t, size));
		M_CMP(s2, REG_ITMP3);
		M_TRAPHS(s2, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(REG_RESULT, REG_RESULT);
		M_TRAPEQ(0, TRAP_ArrayStoreException);
	}
}


/* emit_classcast_check ********************************************************

   Emit a ClassCastException check.

*******************************************************************************/

void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		switch (condition) {
		case BRANCH_EQ:
			M_TRAPEQ(s1, TRAP_ClassCastException);
			break;

		case BRANCH_NE:
			M_TRAPNE(s1, TRAP_ClassCastException);
			break;

		case BRANCH_LT:
			M_TRAPLT(s1, TRAP_ClassCastException);
			break;

		case BRANCH_LE:
			M_TRAPLE(s1, TRAP_ClassCastException);
			break;

		case BRANCH_UGT:
			M_TRAPHI(s1, TRAP_ClassCastException);
			break;

		default:
			vm_abort("emit_classcast_check: unknown condition %d", condition);
			break;
		}
	}
}

/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TST(REG_RESULT, REG_RESULT);
		M_TRAPEQ(0, TRAP_CHECK_EXCEPTION);
	}
}


/* emit_trap_compiler **********************************************************

   Emit a trap instruction which calls the JIT compiler.

*******************************************************************************/

void emit_trap_compiler(codegendata *cd)
{
	M_TRAP(REG_METHODPTR, TRAP_COMPILER);
}


/* emit_trap *******************************************************************

   Emit a trap instruction and return the original machine code.

*******************************************************************************/

uint32_t emit_trap(codegendata *cd)
{
	uint32_t mcode;

	/* Get machine code which is patched back in later. The
	   trap is 1 instruction word long. */

	mcode = *((uint32_t *) cd->mcodeptr);

	M_TRAP(0, TRAP_PATCHER);

	return mcode;
}


/**
 * Emit code to recompute the procedure vector.
 */
void emit_recompute_pv(codegendata *cd)
{
	// This is used to recompute our PV (we use the IP for this) out
	// of the current PC.
	int32_t disp = (int32_t) (cd->mcodeptr - cd->mcodebase);

	// We use PC relative addressing.
	disp += 8;

	// Sanity checks.
	assert((disp & 0x03) == 0);
	assert(disp >= 0 && disp <= 0x03ffffff);

	// ATTENTION: If you change this, you have to look at other functions
	// as well! Following things depend on it: md_codegen_get_pv_from_pc();
	if (disp > 0x0003ffff) {
		M_SUB_IMM(REG_PV, REG_PC, IMM_ROTL(disp >> 18, 9));
		M_SUB_IMM(REG_PV, REG_PV, IMM_ROTL(disp >> 10, 5));
		M_SUB_IMM(REG_PV, REG_PV, IMM_ROTL(disp >> 2, 1));
	} else if (disp > 0x000003ff) {
		M_SUB_IMM(REG_PV, REG_PC, IMM_ROTL(disp >> 10, 5));
		M_SUB_IMM(REG_PV, REG_PV, IMM_ROTL(disp >> 2, 1));
	} else {
		M_SUB_IMM(REG_PV, REG_PC, IMM_ROTL(disp >> 2, 1));
	}
}


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int32_t disp, p;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		M_STMFD(BITMASK_ARGS, REG_SP);
		syncslot_offset += 4 * 4;

#if defined(__ARMHF__)
		M_SUB_IMM(REG_SP, REG_SP, FLT_ARG_CNT * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DST(abi_registers_float_argument[p], REG_SP, p * 8);

		syncslot_offset += FLT_ARG_CNT * 8;
#endif
	}
# endif

	/* get the correct lock object */

	if (m->flags & ACC_STATIC) {
		disp = dseg_add_address(cd, &m->clazz->object.header);
		M_DSEG_LOAD(REG_A0, disp);
	}
	else {
		emit_nullpointer_check_force(cd, NULL, REG_A0);
	}

	M_STR(REG_A0, REG_SP, syncslot_offset);
	disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
	M_DSEG_BRANCH(disp);
	emit_recompute_pv(cd);

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
#if defined(__ARMHF__)
		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DLD(abi_registers_float_argument[p], REG_SP, p * 8);

		M_ADD_IMM(REG_SP, REG_SP, FLT_ARG_CNT * 8);
#endif

		M_LDMFD(BITMASK_ARGS, REG_SP);
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

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
	case TYPE_LNG:
	case TYPE_FLT: /* XXX TWISTI: is that correct? */
	case TYPE_DBL:
		M_STMFD(BITMASK_RESULT, REG_SP);
		syncslot_offset += 2 * 4;
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	M_LDR(REG_A0, REG_SP, syncslot_offset);
	disp = dseg_add_functionptr(cd, LOCK_monitor_exit);
	M_DSEG_BRANCH(disp);

	/* we no longer need PV here, no more loading */
	/*emit_recompute_pv(cd);*/

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
	case TYPE_LNG:
	case TYPE_FLT: /* XXX TWISTI: is that correct? */
	case TYPE_DBL:
		M_LDMFD(BITMASK_RESULT, REG_SP);
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
	codeinfo     *code;
	codegendata  *cd;
	registerdata *rd;
	methoddesc   *md;
	s4            disp;
	s4            i, s;

	/* get required compiler data */

	m  = jd->m;
	code = jd->code;
	cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* Keep stack 8-byte aligned. */

	M_STMFD((1<<REG_LR) | (1<<REG_PV), REG_SP);
	M_SUB_IMM_EXT_MUL4(REG_SP, REG_SP, (md->paramcount + ARG_CNT + TMP_CNT) * 8 >> 2);

	/* save argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_ADR:
#if !defined(__ARMHF__)
			case TYPE_FLT:
#endif
				M_IST(s, REG_SP, i * 8);
				break;
			case TYPE_LNG:
#if !defined(__ARMHF__)
			case TYPE_DBL:
#endif
				M_LST(s, REG_SP, i * 8);
				break;
#if defined(__ARMHF__)
			case TYPE_FLT:
			case TYPE_DBL:
				M_DST(s, REG_SP, i * 8);
				break;
#endif
			default:
				assert(false);
				break;
			}
		}
	}

	/* save all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_ARG_CNT; i++)
			M_IST(abi_registers_integer_argument[i], REG_SP, (md->paramcount + i) * 8);

#if defined(__ARMHF__)
		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DST(abi_registers_float_argument[i], REG_SP, (md->paramcount + INT_ARG_CNT + i) * 8);
#endif

		for (i = 0; i < INT_TMP_CNT; i++)
			M_IST(rd->tmpintregs[i], REG_SP, (md->paramcount + ARG_CNT + i) * 8);

#if defined(__ARMHF__)
		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DST(rd->tmpfltregs[i], REG_SP, (md->paramcount + ARG_CNT + INT_TMP_CNT + i) * 8);
#endif
	}

	disp = dseg_add_address(cd, m);
	M_DSEG_LOAD(REG_A0, disp);
	M_MOV(REG_A1, REG_SP);
	M_ADD_IMM_EXT_MUL4(REG_A2, REG_SP, ((md->paramcount + ARG_CNT + TMP_CNT) * 8 + 2 * 4 + cd->stackframesize * 8) >> 2);
	M_LONGBRANCH(trace_java_call_enter);

	/* restore argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_ADR:
#if !defined(__ARMHF__)
			case TYPE_FLT:
#endif
				M_ILD(s, REG_SP, i * 8);
				break;
			case TYPE_LNG:
#if !defined(__ARMHF__)
			case TYPE_DBL:
#endif
				M_LLD(s, REG_SP, i * 8);
				break;
#if defined(__ARMHF__)
			case TYPE_FLT:
			case TYPE_DBL:
				M_DLD(s, REG_SP, i * 8);
				break;
#endif
			default:
				assert(false);
				break;
			}
		}
	}

	/* restore all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_ARG_CNT; i++)
			M_ILD(abi_registers_integer_argument[i], REG_SP, (md->paramcount + i) * 8);

#if defined(__ARMHF__)
		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DLD(abi_registers_float_argument[i], REG_SP, (md->paramcount + INT_ARG_CNT + i) * 8);
#endif

		for (i = 0; i < INT_TMP_CNT; i++)
			M_ILD(rd->tmpintregs[i], REG_SP, (md->paramcount + ARG_CNT + i) * 8);

#if defined(__ARMHF__)
		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DLD(rd->tmpfltregs[i], REG_SP, (md->paramcount + ARG_CNT + INT_TMP_CNT + i) * 8);
#endif
	}

	/* Keep stack 8-byte aligned. */

	M_ADD_IMM_EXT_MUL4(REG_SP, REG_SP, (md->paramcount + ARG_CNT + TMP_CNT) * 8 >> 2);
	M_LDMFD((1<<REG_LR) | (1<<REG_PV), REG_SP);

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
	methoddesc   *md;
	s4            disp;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* Keep stack 8-byte aligned. */

	M_STMFD((1<<REG_LR) | (1<<REG_PV), REG_SP);
	M_SUB_IMM(REG_SP, REG_SP, 1 * 8);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
#if !defined(__ARMHF__)
	case TYPE_FLT:
#endif
		M_IST(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_LNG:
#if !defined(__ARMHF__)
	case TYPE_DBL:
#endif
		M_LST(REG_RESULT_PACKED, REG_SP, 0 * 8);
		break;
#if defined(__ARMHF__)
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 0 * 8);
		break;
#endif
	default:
		assert(false);
		break;
	}

	disp = dseg_add_address(cd, m);
	M_DSEG_LOAD(REG_A0, disp);
	M_MOV(REG_A1, REG_SP);
	M_LONGBRANCH(trace_java_call_exit);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
#if !defined(__ARMHF__)
	case TYPE_FLT:
#endif
		M_ILD(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_LNG:
#if !defined(__ARMHF__)
	case TYPE_DBL:
#endif
		M_LLD(REG_RESULT_PACKED, REG_SP, 0 * 8);
		break;
#if defined(__ARMHF__)
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 0 * 8);
		break;
#endif
	default:
		assert(false);
		break;
	}

	/* Keep stack 8-byte aligned. */

	M_ADD_IMM(REG_SP, REG_SP, 1 * 8);
	M_LDMFD((1<<REG_LR) | (1<<REG_PV), REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


/**
 * Emit profiling code for method frequency counting.
 * Its slow but working, so be carefull, if you want to use it...
 */
#if defined(ENABLE_PROFILING)
void emit_profile_method(codegendata* cd, codeinfo* code)
{
	ICONST(REG_ITMP3,code);
	M_LDR(REG_ITMP2,REG_ITMP3,OFFSET(codeinfo, frequency));
	M_ADD_IMM(REG_ITMP2, REG_ITMP2, 1);
	M_STR(REG_ITMP2,REG_ITMP3,OFFSET(codeinfo, frequency));
//	M_TRAP(0, TRAP_DEBUG);
}

#endif

/**
 * Emit profiling code for basicblock frequency counting.
 * Its slow but working, so be carefull, if you want to use it...
 */
#if defined(ENABLE_PROFILING)
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr)
{
	ICONST(REG_ITMP3,code);
	M_LDR(REG_ITMP2,REG_ITMP3,OFFSET(codeinfo, bbfrequency));
	M_ADD_IMM(REG_ITMP2, REG_ITMP2, 1);
	M_STR(REG_ITMP2,REG_ITMP3,OFFSET(codeinfo, bbfrequency));
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
