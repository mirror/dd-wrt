/* src/vm/jit/s390/emit.c - s390 code emitter functions

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

#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "vm/jit/s390/codegen.h"
#include "vm/jit/s390/emit.h"
#include "vm/jit/s390/md-abi.h"

#include "mm/memory.hpp"

#include "threads/lock.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/global.hpp"
#include "vm/types.hpp"
#include "vm/options.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/abi-asm.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/replace.hpp"
#include "vm/jit/trace.hpp"
#include "vm/jit/trap.hpp"


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

		if (IS_FLT_DBL_TYPE(src->type)) {
			if (IS_2_WORD_TYPE(src->type))
				M_DLD(tempreg, REG_SP, disp);
			else
				M_FLD(tempreg, REG_SP, disp);
		}
		else {
			if (IS_2_WORD_TYPE(src->type))
				M_LLD(tempreg, REG_SP, disp);
			else
				M_ILD(tempreg, REG_SP, disp);
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
	codegendata *cd;

	/* get required compiler data */

	cd = jd->cd;

	if (IS_INMEMORY(dst->flags)) {
		COUNT_SPILLS;

		if (IS_FLT_DBL_TYPE(dst->type)) {
			if (IS_2_WORD_TYPE(dst->type))
				M_DST(d, REG_SP, dst->vv.regoff);
			else
				M_FST(d, REG_SP, dst->vv.regoff);
		}
		else {
			if (IS_2_WORD_TYPE(dst->type))
				M_LST(d, REG_SP, dst->vv.regoff);
			else
				M_IST(d, REG_SP, dst->vv.regoff);
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

		if (IS_INMEMORY(src->flags) && IS_INMEMORY(dst->flags)) {
			if (IS_2_WORD_TYPE(src->type)) {
				N_MVC(dst->vv.regoff, 8, REG_SP, src->vv.regoff, REG_SP);
			} else {
				N_MVC(dst->vv.regoff, 4, REG_SP, src->vv.regoff, REG_SP);
			}
		} else {

			/* If one of the variables resides in memory, we can eliminate
			   the register move from/to the temporary register with the
			   order of getting the destination register and the load. */

			if (IS_INMEMORY(src->flags)) {
				if (IS_FLT_DBL_TYPE(dst->type)) {
					d = codegen_reg_of_var(iptr->opc, dst, REG_FTMP1);
				} else {
					if (IS_2_WORD_TYPE(dst->type)) {
						d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP12_PACKED);
					} else {
						d = codegen_reg_of_var(iptr->opc, dst, REG_ITMP1);
					}
				}
				s1 = emit_load(jd, iptr, src, d);
			}
			else {
				if (IS_FLT_DBL_TYPE(src->type)) {
					s1 = emit_load(jd, iptr, src, REG_FTMP1);
				} else {
					if (IS_2_WORD_TYPE(src->type)) {
						s1 = emit_load(jd, iptr, src, REG_ITMP12_PACKED);
					} else {
						s1 = emit_load(jd, iptr, src, REG_ITMP1);
					}
				}
				d = codegen_reg_of_var(iptr->opc, dst, s1);
			}

			if (s1 != d) {
				if (IS_FLT_DBL_TYPE(src->type)) {
					M_FMOV(s1, d);
				} else {
					if (IS_2_WORD_TYPE(src->type)) {
						M_LNGMOVE(s1, d);
					} else {
						M_MOV(s1, d);
					}
				}
			}

			emit_store(jd, iptr, dst, d);
		}
	}
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	int32_t disp;

	if (N_VALID_IMM(value)) {
		M_ICMP_IMM(reg, value);
	} else {
		disp = dseg_add_s4(cd, value);
		if (N_VALID_DSEG_DISP(disp)) {
			N_C(reg, N_DSEG_DISP(disp), RN, REG_PV);
		} else {
			assert(reg != REG_ITMP2);
			ICONST(REG_ITMP2, disp);
			N_C(reg, -N_PV_OFFSET, REG_ITMP2, REG_PV);
		}
	}
}


/* emit_trap *******************************************************************

   Emit a trap instruction and return the original machine code.

*******************************************************************************/

uint32_t emit_trap(codegendata *cd)
{
	uint32_t mcode;

	/* Get machine code which is patched back in later. The
	   trap is 2 bytes long. */

	mcode = *((u2 *) cd->mcodeptr);

	M_ILL(TRAP_PATCHER);

	return mcode;
}


/**
 * Generates synchronization code to enter a monitor.
 */
#if defined(ENABLE_THREADS)
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int32_t p;
	int32_t disp;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

#if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		M_ASUB_IMM((INT_ARG_CNT + FLT_ARG_CNT) * 8, REG_SP);

		for (p = 0; p < INT_ARG_CNT; p++)
			M_IST(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DST(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		syncslot_offset += (INT_ARG_CNT + FLT_ARG_CNT) * 8;
	}
#endif

	/* decide which monitor enter function to call */

	if (m->flags & ACC_STATIC) {
		disp = dseg_add_address(cd, &m->clazz->object.header);
		M_ALD_DSEG(REG_A0, disp);
	}
	else {
		M_TEST(REG_A0);
		M_BNE(SZ_BRC + SZ_ILL);
		M_ILL(TRAP_NullPointerException);
	}

	disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
	M_ALD_DSEG(REG_ITMP2, disp);

	M_AST(REG_A0, REG_SP, syncslot_offset);

	M_ASUB_IMM(96, REG_SP);	
	M_CALL(REG_ITMP2);
	M_AADD_IMM(96, REG_SP);	

#if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		for (p = 0; p < INT_ARG_CNT; p++)
			M_ILD(abi_registers_integer_argument[p], REG_SP, p * 8);

		for (p = 0; p < FLT_ARG_CNT; p++)
			M_DLD(abi_registers_float_argument[p], REG_SP, (INT_ARG_CNT + p) * 8);

		M_AADD_IMM((INT_ARG_CNT + FLT_ARG_CNT) * 8, REG_SP);
	}
#endif
}
#endif


/**
 * Generates synchronization code to leave a monitor.
 */
#if defined(ENABLE_THREADS)
void emit_monitor_exit(jitdata* jd, int32_t syncslot_offset)
{
	int32_t disp;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

	/* we need to save the proper return value */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_LNG:
		M_IST(REG_RESULT2, REG_SP, syncslot_offset + 8 + 4);
		/* fall through */
	case TYPE_INT:
	case TYPE_ADR:
		M_IST(REG_RESULT , REG_SP, syncslot_offset + 8);
		break;
	case TYPE_FLT:
		M_FST(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	}

	M_ALD(REG_A0, REG_SP, syncslot_offset);

	disp = dseg_add_functionptr(cd, LOCK_monitor_exit);
	M_ALD_DSEG(REG_ITMP2, disp);

	M_ASUB_IMM(96, REG_SP);
	M_CALL(REG_ITMP2);
	M_AADD_IMM(96, REG_SP);

	/* and now restore the proper return value */

	switch (md->returntype.type) {
	case TYPE_LNG:
		M_ILD(REG_RESULT2, REG_SP, syncslot_offset + 8 + 4);
		/* fall through */
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT , REG_SP, syncslot_offset + 8);
		break;
	case TYPE_FLT:
		M_FLD(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, syncslot_offset + 8);
		break;
	}
}
#endif


/**
 * Emit profiling code for method frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_method(codegendata* cd, codeinfo* code)
{
	M_ALD_DSEG(REG_ITMP1, CodeinfoPointer);
	ICONST(REG_ITMP2, 1);
	N_AL(REG_ITMP2, OFFSET(codeinfo, frequency), RN, REG_ITMP1);
	M_IST(REG_ITMP2, REG_ITMP1, OFFSET(codeinfo, frequency));
}
#endif


/**
 * Emit profiling code for basicblock frequency counting.
 */
#if defined(ENABLE_PROFILING)
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr)
{
	M_ALD_DSEG(REG_ITMP1, CodeinfoPointer);
	M_ALD(REG_ITMP1, REG_ITMP1, OFFSET(codeinfo, bbfrequency));
	ICONST(REG_ITMP2, 1);
	N_AL(REG_ITMP2, bptr->nr * 4, RN, REG_ITMP1);
	M_IST(REG_ITMP2, REG_ITMP1, bptr->nr * 4);
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
	methoddesc   *md;
	s4            stackframesize;
	s4            i, off, disp, s;

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	md   = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* allocate stack frame */

	stackframesize = 96 + (md->paramcount * 8);

	/* for leaf methods we need to store unused argument and temporary registers */

	if (code_is_leafmethod(code)) {
		stackframesize += (ARG_CNT + TMP_CNT) * 8;
	}

	/* allocate stack frame */

	M_ASUB_IMM(stackframesize, REG_SP);

	/* store argument registers in array */

	off = 96;

	for (i = 0; i < md->paramcount; i++) {
		if (! md->params[i].inmemory) {
			s = md->params[i].regoff;
			switch (md->paramtypes[i].type) {
				case TYPE_INT:
				case TYPE_ADR:
					M_IST(s, REG_SP, off);
					break;
				case TYPE_LNG:
					M_LST(s, REG_SP, off);
					break;
				case TYPE_FLT:
					M_FST(s, REG_SP, off);
					break;
				case TYPE_DBL:
					M_DST(s, REG_SP, off);
					break;
			}
		}
		off += 8;
	}

	/* save unused (currently all) argument registers for leaf methods */
	/* save temporary registers for leaf methods */

	if (code_is_leafmethod(code)) {

		for (i = 0; i < INT_ARG_CNT; ++i, off += 8) {
			M_IST(abi_registers_integer_argument[i], REG_SP, off);
		}

		for (i = 0; i < FLT_ARG_CNT; ++i, off += 8) {
			M_DST(abi_registers_float_argument[i], REG_SP, off);
		}

		for (i = 0; i < INT_TMP_CNT; ++i, off += 8) {
			M_IST(abi_registers_integer_temporary[i], REG_SP, off);
		}

		for (i = 0; i < FLT_TMP_CNT; ++i, off += 8) {
			M_DST(abi_registers_float_temporary[i], REG_SP, off);
		}
	}

	/* load arguments for trace_java_call_enter */

	/* methodinfo */

	disp = dseg_add_address(cd, m);
	M_ALD_DSEG(REG_A0, disp);	
	/* pointer to argument registers array */
	M_LDA(REG_A1, REG_SP, 96);
	/* pointer to on stack arguments */
	M_LDA(REG_A2, REG_SP, stackframesize + (cd->stackframesize * 8));

	/* call trace_java_call_enter */

	disp = dseg_add_functionptr(cd, trace_java_call_enter);
	M_ALD_DSEG(REG_ITMP2, disp);
	M_CALL(REG_ITMP2);

	/* restore used argument registers */
	/* for leaf methods restore all argument and temporary registers */

	if (code_is_leafmethod(code)) {
		off = 96 + (8 * md->paramcount);

		for (i = 0; i < INT_ARG_CNT; ++i, off += 8) {
			M_ILD(abi_registers_integer_argument[i], REG_SP, off);
		}

		for (i = 0; i < FLT_ARG_CNT; ++i, off += 8) {
			M_DLD(abi_registers_float_argument[i], REG_SP, off);
		}

		for (i = 0; i < INT_TMP_CNT; ++i, off += 8) {
			M_ILD(abi_registers_integer_temporary[i], REG_SP, off);
		}

		for (i = 0; i < FLT_TMP_CNT; ++i, off += 8) {
			M_DLD(abi_registers_float_temporary[i], REG_SP, off);
		}
	} else {
		off = 96;

		for (i = 0; i < md->paramcount; i++) {
			if (! md->params[i].inmemory) {
				s = md->params[i].regoff;
				switch (md->paramtypes[i].type) {
					case TYPE_INT:
					case TYPE_ADR:
						M_ILD(s, REG_SP, off);
						break;
					case TYPE_LNG:
						M_LLD(s, REG_SP, off);
						break;
					case TYPE_FLT:
						M_FLD(s, REG_SP, off);
						break;
					case TYPE_DBL:
						M_DLD(s, REG_SP, off);
						break;
				}
			}
			off += 8;
		}
	}

	/* remove stack frame */

	M_AADD_IMM(stackframesize, REG_SP);

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
	s4            disp;
	s4            stackframesize;
	s4            off;
	s4            t;

	m  = jd->m;
	cd = jd->cd;
	t = m->parseddesc->returntype.type;

	/* mark trace code */

	M_NOP;

	/* allocate stackframe */

	stackframesize = 96 + (1 * 8);
	M_ASUB_IMM(stackframesize, REG_SP);

	off = 96;

	/* store return values in array */

	if (IS_INT_LNG_TYPE(t)) {
		if (IS_2_WORD_TYPE(t)) {
			M_LST(REG_RESULT_PACKED, REG_SP, off);
		} else {
			M_IST(REG_RESULT, REG_SP, off);
		}
	} else {
		M_DST(REG_FRESULT, REG_SP, off);
	}

	/* call trace_java_call_exit */

	disp = dseg_add_address(cd, m);
	M_ALD_DSEG(REG_A0, disp);
	M_LDA(REG_A1, REG_SP, off);
	disp = dseg_add_functionptr(cd, trace_java_call_exit);
	M_ALD_DSEG(REG_ITMP2, disp);
	M_CALL(REG_ITMP2);

	/* restore return value */

	if (IS_INT_LNG_TYPE(t)) {
		if (IS_2_WORD_TYPE(t)) {
			M_LLD(REG_RESULT_PACKED, REG_SP, off);
		} else {
			M_ILD(REG_RESULT, REG_SP, off);
		}
	} else {
		M_DLD(REG_FRESULT, REG_SP, off);
	}

	/* remove stackframe */

	M_AADD_IMM(stackframesize, REG_SP);

	/* mark trace code */

	M_NOP;
}
#endif /* !defined(NDEBUG) */


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

s4 emit_load_s1_but(jitdata *jd, instruction *iptr, s4 tempreg, s4 notreg) {
	codegendata *cd = jd->cd;
	s4 reg = emit_load_s1(jd, iptr, tempreg);
	if (reg == notreg) {
		if (IS_FLT_DBL_TYPE(VAROP(iptr->s1)->type)) {
			M_FMOV(reg, tempreg);
		} else {
			M_MOV(reg, tempreg);
		}
		return tempreg;
	} else {
		return reg;
	}
}

s4 emit_load_s2_but(jitdata *jd, instruction *iptr, s4 tempreg, s4 notreg) {
	codegendata *cd = jd->cd;
	s4 reg = emit_load_s2(jd, iptr, tempreg);
	if (reg == notreg) {
		if (IS_FLT_DBL_TYPE(VAROP(iptr->sx.s23.s2)->type)) {
			M_FMOV(reg, tempreg);
		} else {
			M_MOV(reg, tempreg);
		}
		return tempreg;
	} else {
		return reg;
	}
}

void emit_copy_dst(jitdata *jd, instruction *iptr, s4 dtmpreg) {
	codegendata *cd;
	varinfo *dst;
	cd = jd->cd;
	dst = VAROP(iptr->dst);
	if (! IS_INMEMORY(dst->flags)) {
		if (dst->vv.regoff != dtmpreg) {
			if (IS_FLT_DBL_TYPE(dst->type)) {
				M_FLTMOVE(dtmpreg, dst->vv.regoff);
			} else if (IS_2_WORD_TYPE(dst->type)) {
				M_LNGMOVE(dtmpreg, dst->vv.regoff);
			} else {
				M_INTMOVE(dtmpreg, dst->vv.regoff);
			}
		}
	}
}

void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 opt) {

	s4 branchdisp = disp;
	s4 branchmpc;
	u1 *ref;

	if (N_VALID_BRANCH(branchdisp)) {

		/* valid displacement */

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
			case BRANCH_UNCONDITIONAL:
				M_BR(branchdisp);
				break;
			default:
				vm_abort("emit_branch: unknown condition %d", condition);
		}
	} else {

		/* If LONGBRANCHES is not set, the flag and the error flag */

		if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
			cd->flags |= (CODEGENDATA_FLAG_ERROR |
				CODEGENDATA_FLAG_LONGBRANCHES);
		}

		/* If error flag is set, do nothing. The method has to be recompiled. */

		if (CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd) && CODEGENDATA_HAS_FLAG_ERROR(cd)) {
			return;
		}

		/* Patch the displacement to branch over the actual branch manually
		 * to not get yet more nops.
		 */

		branchmpc = cd->mcodeptr - cd->mcodebase;
		ref = cd->mcodeptr;

		switch (condition) {
			case BRANCH_EQ:
				M_BNE(0);
				break;
			case BRANCH_NE:
				M_BEQ(0);
				break;
			case BRANCH_LT:
				M_BGE(0);
				break;
			case BRANCH_GE:
				M_BLT(0);
				break;
			case BRANCH_GT:
				M_BLE(0);
				break;
			case BRANCH_LE:
				M_BGT(0);
				break;
			case BRANCH_UNCONDITIONAL:
				/* fall through, no displacement to patch */
				ref = NULL;
				break;
			default:
				vm_abort("emit_branch: unknown condition %d", condition);
		}

		/* The actual long branch */

		disp = dseg_add_s4(cd, branchmpc + disp - N_PV_OFFSET);
		M_ILD_DSEG(REG_ITMP2, disp);
		M_AADD(REG_PV, REG_ITMP2);
		M_JMP(RN, REG_ITMP2);

		/* Patch back the displacement */

		N_BRC_BACK_PATCH(ref);
	}
}

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(reg);
		M_BNE(SZ_BRC + SZ_ILL);
		M_ILL(TRAP_ArithmeticException);
	}
}

/* emit_arrayindexoutofbounds_check ********************************************

   Emit a ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		/* Size is s4, >= 0
		 * Do unsigned comparison to catch negative indexes.
		 */
		N_CL(s2, OFFSET(java_array_t, size), RN, s1);
        M_BLT(SZ_BRC + SZ_ILL);
		M_ILL2(s2, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(REG_RESULT);
		M_BNE(SZ_BRC + SZ_ILL);
		M_ILL(TRAP_ArrayStoreException);
	}
}


void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1) {
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		if (reg != RN) {
			M_TEST(reg);
		}
		switch (condition) {
			case BRANCH_LE:
				M_BGT(SZ_BRC + SZ_ILL);
				break;
			case BRANCH_EQ:
				M_BNE(SZ_BRC + SZ_ILL);
				break;
			case BRANCH_GT:
				M_BLE(SZ_BRC + SZ_ILL);
				break;
			default:
				vm_abort("emit_classcast_check: unknown condition %d", condition);
		}
		M_ILL2(s1, TRAP_ClassCastException);
	}
}

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(reg);
		M_BNE(SZ_BRC + SZ_ILL);
		M_ILL(TRAP_NullPointerException);
	}
}

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_TEST(REG_RESULT);
		M_BNE(SZ_BRC + SZ_ILL);
		M_ILL(TRAP_CHECK_EXCEPTION);
	}
}

void emit_recompute_pv(codegendata *cd) {
	s4 offset, offset_imm;

	/*
	N_BASR(REG_PV, RN);
	disp = (s4) (cd->mcodeptr - cd->mcodebase);
	M_ASUB_IMM32(disp, REG_ITMP1, REG_PV);
	*/

	/* If the offset from the method start does not fit into an immediate
	 * value, we can't put it into the data segment!
	 */

	/* Displacement from start of method to here */

	offset = (s4) (cd->mcodeptr - cd->mcodebase);
	offset_imm = -offset - SZ_BASR + N_PV_OFFSET;

	if (N_VALID_IMM(offset_imm)) {
		/* Get program counter */
		N_BASR(REG_PV, RN);
		/* Substract displacement */
		M_AADD_IMM(offset_imm, REG_PV);
	} else {
		/* Save program counter and jump over displacement in instruction flow */
		N_BRAS(REG_PV, SZ_BRAS + SZ_LONG);
		/* Place displacement here */
		/* REG_PV points now exactly to this position */
		N_LONG(-offset - SZ_BRAS + N_PV_OFFSET);
		/* Substract *(REG_PV) from REG_PV */
		N_A(REG_PV, 0, RN, REG_PV);
	}
}

/* emit_trap_compiler **********************************************************

   Emit a trap instruction which calls the JIT compiler.

*******************************************************************************/

void emit_trap_compiler(codegendata *cd)
{
	M_ILL2(REG_METHODPTR, TRAP_COMPILER);
}

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
