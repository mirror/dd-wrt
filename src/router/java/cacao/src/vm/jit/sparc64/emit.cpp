/* src/vm/jit/sparc64/emit.cpp - SPARC code emitter functions

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

#include "vm/jit/sparc64/codegen.hpp"
#include "vm/jit/sparc64/md-abi.hpp"
#include "vm/jit/sparc64/emit.hpp"

#include "mm/memory.hpp"

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

#include "vm/jit/sparc64/solaris/macro_rename.hpp"


/* how to leaf optimization in the emitted stubs?? */
#define REG_PV REG_PV_CALLEE


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
		COUNT_READ_SPILLS(src)

		disp = JITSTACK + src->vv.regoff;

		switch(src->type)
		{
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
			M_LDX(tempreg, REG_SP, disp);
			break;
		case TYPE_FLT:
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

   Emits a possible store to variable.

*******************************************************************************/

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d)
{
	codegendata  *cd;
	s4            disp;

	/* get required compiler data */

	cd = jd->cd;

	if (dst->flags & INMEMORY) {
		COUNT_WRITE_SPILLS(dst)

		disp = JITSTACK + dst->vv.regoff;
	
		switch(dst->type)
		{
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
			M_STX(d, REG_SP, disp);
			break;
		case TYPE_FLT:
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
			switch(src->type) {
			case TYPE_INT:
			case TYPE_LNG:
			case TYPE_ADR:
				M_MOV(s1, d);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DMOV(s1, d);
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

	if ((value >= -4096) && (value <= 4095)) {
		M_XOR_IMM(REG_ZERO, value, d);
	} else {
		disp = dseg_add_s4(cd, value);
		M_ILD(d, REG_PV_CALLEE, disp);
	}
}


/* emit_lconst *****************************************************************

   XXX

*******************************************************************************/

void emit_lconst(codegendata *cd, s4 d, s8 value)
{
	s4 disp;

	if ((value >= -4096) && (value <= 4095)) {
		M_XOR_IMM(REG_ZERO, value, d);	
	} else {
		disp = dseg_add_s8(cd, value);
		M_LDX(d, REG_PV_CALLEE, disp);
	}
}


/**
 * Emits code updating the condition register by comparing one integer
 * register to an immediate integer value.
 */
void emit_icmp_imm(codegendata* cd, int reg, int32_t value)
{
	if ((value >= -4096) && (value <= 4095)) {
		M_CMP_IMM(reg, value);
	} else {
		assert(reg != REG_ITMP2);
		ICONST(REG_ITMP2, value);
		M_CMP(reg, REG_ITMP2);
	}
}


/* emit_branch *****************************************************************

   Emits the code for conditional and unconditional branchs.

*******************************************************************************/

void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 opt)
{
	s4 branchdisp;

	/* calculate the different displacements */

	branchdisp = disp >> 2;

	/* check which branch to generate */

	if (condition == BRANCH_UNCONDITIONAL) {
		/* check displacement for overflow (19-bit)*/

		if ((branchdisp < (s4) 0xfffc0000) || (branchdisp > (s4) 0x003ffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit unconditional long-branch code");
		}
		else {
			M_BR(branchdisp);
			M_NOP;
		}
	}
	else if (reg == -1) {
		/* branch on condition codes */

		/* check displacement for overflow (19-bit)*/

		if ((branchdisp < (s4) 0xfffc0000) || (branchdisp > (s4) 0x003ffff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				log_println("setting error");
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit long-branch on cc code");
		}
		else {
			/* check whether to branch on 64-bit condition code */
			if (BRANCH_CHECKS_XCC(opt)) {
				switch (condition) {
				case BRANCH_EQ:
					M_XBEQ(branchdisp);
					break;
				case BRANCH_NE:
					M_XBNE(branchdisp);
					break;
				case BRANCH_LT:
					M_XBLT(branchdisp);
					break;
				case BRANCH_GE:
					M_XBGE(branchdisp);
					break;
				case BRANCH_GT:
					M_XBGT(branchdisp);
					break;
				case BRANCH_LE:
					M_XBLE(branchdisp);
					break;
				case BRANCH_UGT:
					M_XBUGT(branchdisp);
					break;
				case BRANCH_ULT:
					M_XBULT(branchdisp);
					break;
				default:
					vm_abort("emit_branch: unknown condition %d", condition);
					break;
				}
				
				/* branch delay */
				M_NOP;
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
					M_BUGT(branchdisp);
					break;
				case BRANCH_ULT:
					M_BULT(branchdisp);
					break;
				default:
					vm_abort("emit_branch: unknown condition %d", condition);
					break;
				}

				/* branch delay */
				M_NOP;
			}
		}
	}
	else {
		/* branch on register */

		/* check displacement for overflow (16-bit) */

		if ((branchdisp < (s4) 0xffff8000) || (branchdisp > (s4) 0x0007fff)) {
			/* if the long-branches flag isn't set yet, do it */

			if (!CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {
				log_println("setting error");
				cd->flags |= (CODEGENDATA_FLAG_ERROR |
							  CODEGENDATA_FLAG_LONGBRANCHES);
			}

			vm_abort("emit_branch: emit long-branch on reg code");
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

			/* branch delay */
			M_NOP;
		}
	}
}


/* emit_bxx_xcc*****************************************************************

   Wrappers for branches on 64-bit condition codes (SPARC specific).

*******************************************************************************/

void emit_beq_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_EQ, BRANCH_OPT_XCC);
}

void emit_bne_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_NE, BRANCH_OPT_XCC);
}

void emit_blt_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_LT, BRANCH_OPT_XCC);
}

void emit_bge_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_GE, BRANCH_OPT_XCC);
}

void emit_bgt_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_GT, BRANCH_OPT_XCC);
}

void emit_ble_xcc(codegendata *cd, basicblock *target)
{
	emit_bcc(cd, target, BRANCH_LE, BRANCH_OPT_XCC);
}





/* emit_arithmetic_check *******************************************************

   Emit an ArithmeticException check.

*******************************************************************************/

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(reg, 3);
		M_NOP;
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_ArithmeticException);
	}
}


/* emit_arrayindexoutofbounds_check ********************************************

   Emit an ArrayIndexOutOfBoundsException check.

*******************************************************************************/

void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_ILD(REG_ITMP3, s1, OFFSET(java_array_t, size));
		M_CMP(s2, REG_ITMP3);
		M_XBULT(3);
		M_NOP;
		M_ALD_INTERN(s2, REG_ZERO, TRAP_ArrayIndexOutOfBoundsException);
	}
}


/* emit_arraystore_check *******************************************************

   Emit an ArrayStoreException check.

*******************************************************************************/

void emit_arraystore_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(REG_RESULT_CALLER, 3);
		M_NOP;
		M_ALD_INTERN(REG_RESULT_CALLER, REG_ZERO, TRAP_ArrayStoreException);
	}
}


/* emit_classcast_check ********************************************************

   Emit a ClassCastException check.

*******************************************************************************/

void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1)
{
/* XXX: use 64-bit or 32-bit compares??? */

	if (INSTRUCTION_MUST_CHECK(iptr)) {
		switch (condition) {
		case ICMD_IFEQ:
			M_BNEZ(reg, 3);
			break;

		case ICMD_IFLE:
			M_BGTZ(reg, 3);
			break;

		case BRANCH_ULT:
			M_XBUGE(3);
			break;

		default:
			vm_abort("emit_classcast_check: unknown condition %d", condition);
			break;
		}

		M_NOP;
		M_ALD_INTERN(s1, REG_ZERO, TRAP_ClassCastException);
	}
}


/* emit_nullpointer_check ******************************************************

   Emit a NullPointerException check.

*******************************************************************************/

void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(reg, 3);
		M_NOP;
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}
}


/* emit_exception_check ********************************************************

   Emit an Exception check.

*******************************************************************************/

void emit_exception_check(codegendata *cd, instruction *iptr)
{
	if (INSTRUCTION_MUST_CHECK(iptr)) {
		M_BNEZ(REG_RESULT_CALLER, 3);
		M_NOP;
		M_ALD_INTERN(REG_RESULT_CALLER, REG_ZERO, TRAP_CHECK_EXCEPTION);
	}
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

	M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_PATCHER);

	return mcode;
}


/**
 * Emit code to recompute the procedure vector.
 */
void emit_recompute_pv(codegendata *cd)
{
	int32_t disp = (int32_t) (cd->mcodeptr - cd->mcodebase);

	/* REG_RA holds the value of the jmp instruction, therefore +8 */
	M_LDA(REG_ZERO, REG_RA_CALLER, -disp + 8); 
}


/**
 * Generates synchronization code to enter a monitor.
 */
void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset)
{
	int32_t i, slots;
	int32_t disp;

	// Get required compiler data.
	methodinfo*  m  = jd->m;
	codegendata* cd = jd->cd;

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		/* save float argument registers */

		/* XXX jit-c-call */
		slots = FLT_ARG_CNT;
		ALIGN_STACK_SLOTS(slots);

		M_LDA(REG_SP, REG_SP, -(slots * 8));
		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DST(abi_registers_float_argument[i], REG_SP, CSTACK +  i * 8);

		syncslot_offset += slots * 8;
	}
# endif

	/* get correct lock object */

	if (m->flags & ACC_STATIC) {
		disp = dseg_add_address(cd, &m->clazz->object.header);
		M_ALD(REG_OUT0, REG_PV, disp);
		disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
		M_ALD(REG_ITMP3, REG_PV, disp);
	}
	else {
		/* copy class pointer: $i0 -> $o0 */
		M_MOV(REG_RESULT_CALLEE, REG_OUT0);
		M_BNEZ(REG_OUT0, 3);
		disp = dseg_add_functionptr(cd, LOCK_monitor_enter);
		M_ALD(REG_ITMP3, REG_PV, disp);                       /* branch delay */
		M_ALD_INTERN(REG_ZERO, REG_ZERO, TRAP_NullPointerException);
	}

	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_AST(REG_OUT0, REG_SP, CSTACK + syncslot_offset);        /* branch delay */

# if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		/* restore float argument registers */

		for (i = 0; i < FLT_ARG_CNT; i++)
			M_DLD(abi_registers_float_argument[i], REG_SP, CSTACK + i * 8);

		M_LDA(REG_SP, REG_SP, slots * 8);
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

	/* XXX jit-c-call */
	disp = dseg_add_functionptr(cd, LOCK_monitor_exit);
	M_ALD(REG_ITMP3, REG_PV, disp);

	/* we need to save fp return value (int saved by window) */

	methoddesc* md = m->parseddesc;

	switch (md->returntype.type) {
	case TYPE_FLT:
	case TYPE_DBL:
		M_ALD(REG_OUT0, REG_SP, CSTACK + syncslot_offset);
		M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
		M_DST(REG_FRESULT, REG_SP, CSTACK + syncslot_offset); /* delay */

		/* restore the fp return value */

		M_DLD(REG_FRESULT, REG_SP, CSTACK + syncslot_offset);
		break;
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_DBL:
	default:
		M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
		M_ALD(REG_OUT0, REG_SP, CSTACK + syncslot_offset); /* delay */
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;	
	}
}


/* emit_patcher_stubs **********************************************************

   Generates the code for the patcher stubs.

*******************************************************************************/

void emit_patcher_stubs(jitdata *jd)
{
	codegendata *cd;
	patchref    *pref;
	u4           mcode[2];
	u1          *savedmcodeptr;
	u1          *tmpmcodeptr;
	s4           targetdisp;
	s4           disp;

	/* get required compiler data */

	cd = jd->cd;

	/* generate code patching stub call code */

	targetdisp = 0;

	for (pref = cd->patchrefs; pref != NULL; pref = pref->next) {
		/* check code segment size */

		MCODECHECK(100);

		/* Get machine code which is patched back in later. The
		   call is 2 instruction words long. */

		tmpmcodeptr = (u1 *) (cd->mcodebase + pref->branchpos);

		/* We use 2 loads here as an unaligned 8-byte read on 64-bit
		   SPARC causes a SIGSEGV */

		mcode[0] = ((u4 *) tmpmcodeptr)[0];
		mcode[1] = ((u4 *) tmpmcodeptr)[1];

		/* Patch in the call to call the following code (done at
		   compile time). */

		savedmcodeptr = cd->mcodeptr;   /* save current mcodeptr          */
		cd->mcodeptr  = tmpmcodeptr;    /* set mcodeptr to patch position */

		disp = ((u4 *) savedmcodeptr) - (((u4 *) tmpmcodeptr) );

		if ((disp < (s4) 0xfffc0000) || (disp > (s4) 0x003ffff)) {
			vm_abort("Jump offset is out of range: %d > +/-%d",
					 disp, 0x003ffff);
			return;
		}

		M_BR(disp);
		M_NOP;

		cd->mcodeptr = savedmcodeptr;   /* restore the current mcodeptr   */

		/* extend stack frame for wrapper data */

		M_ASUB_IMM(REG_SP, 6 * 8, REG_SP);

		/* calculate return address and move it onto the stack */

		M_LDA(REG_ITMP3, REG_PV, pref->branchpos);
		M_AST(REG_ITMP3, REG_SP, JITSTACK + 5 * 8);

		/* move pointer to java_objectheader onto stack */

		/* create a virtual java_objectheader */

		(void) dseg_add_unique_address(cd, NULL);                  /* flcword */
		(void) dseg_add_unique_address(cd, lock_get_initial_lock_word());
		disp = dseg_add_unique_address(cd, NULL);                  /* vftbl   */

		M_LDA(REG_ITMP3, REG_PV, disp);
		M_AST(REG_ITMP3, REG_SP, JITSTACK + 4 * 8);

		/* move machine code onto stack */

		disp = dseg_add_s4(cd, mcode[0]);
		M_ILD(REG_ITMP3, REG_PV, disp);
		M_IST(REG_ITMP3, REG_SP, JITSTACK + 3 * 8);

		disp = dseg_add_s4(cd, mcode[1]);
		M_ILD(REG_ITMP3, REG_PV, disp);
		M_IST(REG_ITMP3, REG_SP, JITSTACK + 3 * 8 + 4);

		/* move class/method/field reference onto stack */

		disp = dseg_add_address(cd, pref->ref);
		M_ALD(REG_ITMP3, REG_PV, disp);
		M_AST(REG_ITMP3, REG_SP, JITSTACK + 2 * 8);

	/* move data segment displacement onto stack */

		disp = dseg_add_s4(cd, pref->disp);
		M_ILD(REG_ITMP3, REG_PV, disp);
		M_IST(REG_ITMP3, REG_SP, JITSTACK + 1 * 8);

		/* move patcher function pointer onto stack */

		disp = dseg_add_functionptr(cd, pref->patcher);
		M_ALD(REG_ITMP3, REG_PV, disp);
		M_AST(REG_ITMP3, REG_SP, JITSTACK + 0 * 8);

		if (targetdisp == 0) {
			targetdisp = ((u4 *) cd->mcodeptr) - ((u4 *) cd->mcodebase);

			disp = dseg_add_functionptr(cd, asm_patcher_wrapper);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JMP(REG_ZERO, REG_ITMP3, REG_ZERO);
			M_NOP;
		}
		else {
			disp = (((u4 *) cd->mcodebase) + targetdisp) -
				(((u4 *) cd->mcodeptr));

			M_BR(disp);
			M_NOP;
		}
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
	registerdata *rd;
	methoddesc   *md;
	s4            disp;
	s4            i, t;
	s4            stackslots;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	rd = jd->rd;

	md = m->parseddesc;

	/* mark trace code */

	M_NOP;

	/* XXX jit-c-call */
	stackslots = 1 + FLT_ARG_CNT;
	ALIGN_STACK_SLOTS(stackslots);

	M_LDA(REG_SP, REG_SP, -(stackslots * 8));

	/* save float argument registers */

	for (i = 0; i < FLT_ARG_CNT; i++)
		M_DST(abi_registers_float_argument[i], REG_SP, JITSTACK + (1 + i) * 8);

	/* save temporary registers for leaf methods */
/* XXX no leaf optimization yet
	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_TMP_CNT; i++)
			M_LST(rd->tmpintregs[i], REG_SP, (2 + ARG_CNT + i) * 8);

		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DST(rd->tmpfltregs[i], REG_SP, (2 + ARG_CNT + INT_TMP_CNT + i) * 8);
	}
*/
	/* load int/float arguments into integer argument registers */

	for (i = 0; i < md->paramcount && i < INT_NATARG_CNT; i++) {
		t = md->paramtypes[i].type;

		/* all available argument registers used, which adds a little complexity */
		
		if (IS_INT_LNG_TYPE(t)) {
			if (i < INT_ARG_CNT) {
				M_INTMOVE(REG_WINDOW_TRANSPOSE(abi_registers_integer_argument[i]), 
					abi_registers_integer_argument[i]);
			}
			else {
				assert(i == 5);
				M_LDX(REG_OUT5, REG_FP, JITSTACK);
			}
		}
		else {
			if (i < FLT_ARG_CNT) {
				
				/* reg -> mem -> reg */
				
				if (IS_2_WORD_TYPE(t)) {
					M_DST(abi_registers_float_argument[i], REG_SP, JITSTACK);
					M_LDX(abi_registers_integer_argument[i], REG_SP, JITSTACK);
				}
				else {
					M_FST(abi_registers_float_argument[i], REG_SP, JITSTACK);
					M_ILD(abi_registers_integer_argument[i], REG_SP, JITSTACK);
				}
			}
			else {
				
				/* mem -> reg */
				
				assert(i == 5);
				if (IS_2_WORD_TYPE(t)) {
					M_LDX(REG_OUT5, REG_FP, JITSTACK);
				}
				else {
					M_ILD(REG_OUT5, REG_FP, JITSTACK);
				}
			}
		}
	}
	
	
	/* method info pointer is passed via stack */
	disp = dseg_add_address(cd, m);
	M_ALD(REG_ITMP1, REG_PV_CALLEE, disp);
	M_AST(REG_ITMP1, REG_SP, CSTACK);
	disp = dseg_add_functionptr(cd, builtin_verbosecall_enter);
	M_ALD(REG_ITMP1, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP1, REG_ZERO);
	M_NOP;

	/* restore float argument registers */

	for (i = 0; i < FLT_ARG_CNT; i++)
		M_DLD(abi_registers_float_argument[i], REG_SP, JITSTACK + (1 + i) * 8);

	/* restore temporary registers for leaf methods */
/* XXX no leaf optimization yet
	if (code_is_leafmethod(code)) {
		for (i = 0; i < INT_TMP_CNT; i++)
			M_LLD(rd->tmpintregs[i], REG_SP, (2 + ARG_CNT + i) * 8);

		for (i = 0; i < FLT_TMP_CNT; i++)
			M_DLD(rd->tmpfltregs[i], REG_SP, (2 + ARG_CNT + INT_TMP_CNT + i) * 8);
	}
*/
	M_LDA(REG_SP, REG_SP, stackslots * 8);

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
	s4            disp;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;
	rd = jd->rd;

	/* mark trace code */

	M_NOP;
	
	/* XXX jit-c-call (keep stack aligned)*/
	M_LDA(REG_SP, REG_SP, -(2 * 8));

	M_DST(REG_FRESULT, REG_SP, JITSTACK);

	M_MOV(REG_RESULT_CALLEE, REG_OUT0);
	M_DMOV(REG_FRESULT, 1); /* logical dreg 1 => f2 */
	M_FMOV(REG_FRESULT, 2); /* logical freg 2 => f5 */

	disp = dseg_add_functionptr(cd, m);
	M_ALD(REG_OUT3, REG_PV_CALLEE, disp);

	disp = dseg_add_functionptr(cd, builtin_verbosecall_exit);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_NOP;

	M_DLD(REG_FRESULT, REG_SP, JITSTACK);

	M_LDA(REG_SP, REG_SP, 2 * 8);

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
