/* src/vm/jit/alpha/emit.cpp - Alpha code emitter functions

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

#include <cassert>
#include <stdint.h>

#include "md-abi.hpp"

#include "vm/jit/alpha/codegen.hpp"

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

   Emit a possible store for the given variable.

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


/* emit_lconst *****************************************************************

   XXX

*******************************************************************************/

void emit_lconst(codegendata *cd, s4 d, s8 value)
{
	s4 disp;

	if ((value >= -32768) && (value <= 32767))
		M_LDA_INTERN(d, REG_ZERO, value);
	else {
		disp = dseg_add_s8(cd, value);
		M_LLD(d, REG_PV, disp);
	}
}


/**
 * Emits code comparing one integer register to an immediate value.
 */
void emit_icmpeq_imm(codegendata* cd, int reg, int32_t value, int d)
{
	int32_t disp;

	if ((value >= 0) && (value <= 255)) {
		M_CMPEQ_IMM(reg, value, d);
	} else {
		assert(reg != REG_ITMP2);
		if ((value >= -32768) && (value <= 32767)) {
			M_LDA(REG_ITMP2, REG_ZERO, value);
		} else {
			disp = dseg_add_s4(cd, value);
			M_ILD(REG_ITMP2, REG_PV, disp);
		}
		M_CMPEQ(reg, REG_ITMP2, d);
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

	checkdisp  = (disp - 4);
	branchdisp = (disp - 4) >> 2;

	/* check which branch to generate */

	if (condition == BRANCH_UNCONDITIONAL) {
		/* check displacement for overflow */

		if ((checkdisp < (s4) 0xffe00000) || (checkdisp > (s4) 0x001fffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				log_println("setting error");
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

		if ((checkdisp < (s4) 0xffe00000) || (checkdisp > (s4) 0x001fffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				log_println("setting error");
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit conditional long-branch code");
		}
		else {
			switch (condition) {
			case BRANCH_EQ:
				M_BEQZ(reg, branchdisp);
				break;
			case BRANCH_NE:
				M_BNEZ(reg, branchdisp);
				break;
			case BRANCH_LT:
				M_BLTZ(reg, branchdisp);
				break;
			case BRANCH_GE:
				M_BGEZ(reg, branchdisp);
				break;
			case BRANCH_GT:
				M_BGTZ(reg, branchdisp);
				break;
			case BRANCH_LE:
				M_BLEZ(reg, branchdisp);
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
		M_BNEZ(reg, 1);
		/* Destination register must not be REG_ZERO, because then no
		   SIGSEGV is thrown. */
		M_ALD_INTERN(reg, REG_ZERO, TRAP_ArithmeticException);
	}
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit an ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
		M_CMPULT(s2, REG_ITMP3, REG_ITMP3);
		M_BNEZ(REG_ITMP3, 1);
		M_ALD_INTERN(s2, REG_ZERO, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(REG_RESULT, 1);
		/* Destination register must not be REG_ZERO, because then no
		   SIGSEGV is thrown. */
		M_ALD_INTERN(REG_RESULT, REG_ZERO, TRAP_ArrayStoreException);
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
			M_BNEZ(reg, 1);
			break;
		case BRANCH_LE:
			M_BGTZ(reg, 1);
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
		M_BNEZ(reg, 1);
		/* Destination register must not be REG_ZERO, because then no
		   SIGSEGV is thrown. */
		M_ALD_INTERN(reg, REG_ZERO, TRAP_NullPointerException);
	}
}


/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(REG_RESULT, 1);
		/* Destination register must not be REG_ZERO, because then no
		   SIGSEGV is thrown. */
		M_ALD_INTERN(REG_RESULT, REG_ZERO, TRAP_CHECK_EXCEPTION);
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
	uint32_t mcode;

	/* Get machine code which is patched back in later. The
	   trap is 1 instruction word long. */

	mcode = *((uint32_t*) cd->mcodeptr);

	// Generate a SIGILL.
	M_UNDEFINED;

	return mcode;
}


/**
 * Emit code to recompute the procedure vector.
 */
void emit_recompute_pv(codegendata *cd)
{
	int32_t disp = (int32_t) (cd->mcodeptr - cd->mcodebase);

	M_LDA(REG_PV, REG_RA, -disp);
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

#if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		M_LDA(REG_SP, REG_SP, -(INT_ARG_CNT + FLT_ARG_CNT) * 8);

		for (p = 0; p < INT_ARG_CNT; p++)
			M_LST(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DST(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		syncslot_offset += (INT_ARG_CNT + FLT_ARG_CNT) * 8;
	}
#endif /* !defined(NDEBUG) */

	/* decide which monitor enter function to call */

	if (m->flags & ACC_STATIC) {
		disp = dseg_add_address(cd, &m->clazz->object.header);
		M_ALD(REG_A0, REG_PV, disp);
	}
	else {
		M_BNEZ(REG_A0, 1);
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}

	M_AST(REG_A0, REG_SP, syncslot_offset);
	disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	emit_recompute_pv(cd);

#if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		for (p = 0; p < INT_ARG_CNT; p++)
			M_LLD(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DLD(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		M_LDA(REG_SP, REG_SP, (INT_ARG_CNT + FLT_ARG_CNT) * 8);
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

	M_ALD(REG_A0, REG_SP, syncslot_offset);

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
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

	disp = dseg_add_functionptr(cd, LOCK_monitor_exit);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	emit_recompute_pv(cd);

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
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
	int32_t       stackframesize;
	s4            disp;
	s4            i, j, s;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;
	rd   = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	stackframesize = ARG_CNT + TMP_CNT + md->paramcount + 1;

	M_LDA(REG_SP, REG_SP, -(stackframesize * 8));
	M_AST(REG_RA, REG_SP, 0 * 8);

	/* save all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		j = 1 + md->paramcount;

		for (i = 0; i < INT_ARG_CNT; i++, j++)
			M_LST(abi_registers_integer_argument[i], REG_SP, j * 8);

		for (i = 0; i < FLT_ARG_CNT; i++, j++)
			M_DST(abi_registers_float_argument[i], REG_SP, j * 8);

		for (i = 0; i < INT_TMP_CNT; i++, j++)
			M_LST(rd->tmpintregs[i], REG_SP, j * 8);

		for (i = 0; i < FLT_TMP_CNT; i++, j++)
			M_DST(rd->tmpfltregs[i], REG_SP, j * 8);
	}

	/* save argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LST(s, REG_SP, (1 + i) * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DST(s, REG_SP, (1 + i) * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, 1 * 8, REG_A1);
	M_LDA(REG_A2, REG_SP, stackframesize * 8 + cd->stackframesize * 8);

	disp = dseg_add_functionptr(cd, trace_java_call_enter);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	disp = (s4) (cd->mcodeptr - cd->mcodebase);
	M_LDA(REG_PV, REG_RA, -disp);
	M_ALD(REG_RA, REG_SP, 0 * 8);

	/* restore argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_ADR:
			case TYPE_INT:
			case TYPE_LNG:
				M_LLD(s, REG_SP, (1 + i) * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DLD(s, REG_SP, (1 + i) * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* restore all argument and temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {
		j = 1 + md->paramcount;

		for (i = 0; i < INT_ARG_CNT; i++, j++)
			M_LLD(abi_registers_integer_argument[i], REG_SP, j * 8);

		for (i = 0; i < FLT_ARG_CNT; i++, j++)
			M_DLD(abi_registers_float_argument[i], REG_SP, j * 8);

		for (i = 0; i < INT_TMP_CNT; i++, j++)
			M_LLD(rd->tmpintregs[i], REG_SP, j * 8);

		for (i = 0; i < FLT_TMP_CNT; i++, j++)
			M_DLD(rd->tmpfltregs[i], REG_SP, j * 8);
	}

	M_LDA(REG_SP, REG_SP, stackframesize * 8);

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
	registerdata *rd;
	methoddesc   *md;
	s4            disp;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	M_ASUB_IMM(REG_SP, 2 * 8, REG_SP);
	M_AST(REG_RA, REG_SP, 0 * 8);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LST(REG_RESULT, REG_SP, 1 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 1 * 8);
		break;
	default:
		assert(false);
		break;
	}

	disp = dseg_add_address(cd, m);
	M_ALD(REG_A0, REG_PV, disp);
	M_AADD_IMM(REG_SP, 1 * 8, REG_A1);

	disp = dseg_add_functionptr(cd, trace_java_call_exit);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	disp = (cd->mcodeptr - cd->mcodebase);
	M_LDA(REG_PV, REG_RA, -disp);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_ADR:
	case TYPE_INT:
	case TYPE_LNG:
		M_LLD(REG_RESULT, REG_SP, 1 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 1 * 8);
		break;
	default:
		assert(false);
		break;
	}

	M_ALD(REG_RA, REG_SP, 0 * 8);
	M_AADD_IMM(REG_SP, 2 * 8, REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


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
