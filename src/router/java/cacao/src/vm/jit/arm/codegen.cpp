/* src/vm/jit/arm/codegen.cpp - machine code generator for Arm

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

#include <cassert>
#include <cstdio>

#include "vm/types.hpp"

#include "md-abi.hpp"

#include "vm/jit/arm/arch.hpp"
#include "vm/jit/arm/codegen.hpp"
#include "vm/jit/arm/emit.hpp"

#include "mm/memory.hpp"

#include "native/localref.hpp"
#include "native/native.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"
#include "vm/vm.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/stacktrace.hpp"


/**
 * Generates machine code for the method prolog.
 */
void codegen_emit_prolog(jitdata* jd)
{
	varinfo*    var;
	methoddesc* md;
	int32_t     s1;
	int32_t     t, len;
	int32_t     varindex;
	int         i;

	// Get required compiler data.
	methodinfo*   m    = jd->m;
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;

	int32_t savedregs_num = 0;
	uint32_t savedregs_bitmask = 0;

	if (!code_is_leafmethod(code)) {
		savedregs_num++;
		savedregs_bitmask = (1<<REG_LR);
	}

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		savedregs_num++;
		savedregs_bitmask |= (1<<(rd->savintregs[i]));
	}

#if !defined(NDEBUG) && !defined(__ARMHF__)
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		vm_abort("codegen_emit_prolog: Floating-point callee saved registers are not saved to stack");
	}
#endif

	/* save return address and used callee saved registers */

	if (savedregs_bitmask != 0)
		M_STMFD(savedregs_bitmask, REG_SP);

	/* create additional stack frame for spilled variables (if necessary) */

	int32_t additional_bytes = (cd->stackframesize * 8 - savedregs_num * 4);

	if (additional_bytes > 0)
		M_SUB_IMM_EXT_MUL4(REG_SP, REG_SP, additional_bytes / 4);

#if defined(__ARMHF__)
	int32_t p = cd->stackframesize - savedregs_num / 2;
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DST(rd->savfltregs[i], REG_SP, p * 8 - (savedregs_num & 1) * 4);
	}
#endif

	/* take arguments out of register or stack frame */

	md = m->parseddesc;
	for (i = 0, len = 0; i < md->paramcount; i++) {
		t = md->paramtypes[i].type;

		varindex = jd->local_map[len * 5 + t];

		len += (IS_2_WORD_TYPE(t)) ? 2 : 1;          /* 2 word type arguments */

		if (varindex == jitdata::UNUSED)
			continue;

		var = VAR(varindex);
		s1 = md->params[i].regoff;

		/* ATTENTION: we use interger registers for all arguments (even float) */
#if !defined(ENABLE_SOFTFLOAT)
		if (IS_INT_LNG_TYPE(t)) {
#endif
			if (!md->params[i].inmemory) {
				if (!(var->flags & INMEMORY)) {
					if (IS_2_WORD_TYPE(t))
						M_LNGMOVE(s1, var->vv.regoff);
					else
						M_INTMOVE(s1, var->vv.regoff);
				}
				else {
					if (IS_2_WORD_TYPE(t))
						M_LST(s1, REG_SP, var->vv.regoff);
					else
						M_IST(s1, REG_SP, var->vv.regoff);
				}
			}
			else {                                   /* stack arguments       */
				if (!(var->flags & INMEMORY)) {      /* stack arg -> register */
					if (IS_2_WORD_TYPE(t))
						M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
					else
						M_ILD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				}
				else {                               /* stack arg -> spilled  */
					/* Reuse Memory Position on Caller Stack */
					var->vv.regoff = cd->stackframesize * 8 + s1;
				}
			}
#if !defined(ENABLE_SOFTFLOAT)
		}
		else {
			if (!md->params[i].inmemory) {
#if defined(__ARMHF__)
				if (!(var->flags & INMEMORY)) {
					if (IS_2_WORD_TYPE(t))
						emit_dmove(cd, s1, var->vv.regoff);
					else
						emit_fmove(cd, s1, var->vv.regoff);
				}
				else {
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, var->vv.regoff);
					else
						M_FST(s1, REG_SP, var->vv.regoff);
				}
#else
				if (!(var->flags & INMEMORY)) {
					if (IS_2_WORD_TYPE(t))
						M_CAST_L2D(s1, var->vv.regoff);
					else
						M_CAST_I2F(s1, var->vv.regoff);
				}
				else {
					if (IS_2_WORD_TYPE(t))
						M_LST(s1, REG_SP, var->vv.regoff);
					else
						M_IST(s1, REG_SP, var->vv.regoff);
				}
#endif
			}
			else {
				if (!(var->flags & INMEMORY)) {
					if (IS_2_WORD_TYPE(t))
						M_DLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
					else
						M_FLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				}
				else {
					/* Reuse Memory Position on Caller Stack */
					var->vv.regoff = cd->stackframesize * 8 + s1;
				}
			}
		}
#endif /* !defined(ENABLE_SOFTFLOAT) */
	}
}


/**
 * Generates machine code for the method epilog.
 */
void codegen_emit_epilog(jitdata* jd)
{
	int i;

	// Get required compiler data.
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;

	int32_t savedregs_num = 0;
	uint32_t savedregs_bitmask = 0;

	if (!code_is_leafmethod(code)) {
		savedregs_num++;
		savedregs_bitmask = (1<<REG_LR);
	}

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		savedregs_num++;
		savedregs_bitmask |= (1<<(rd->savintregs[i]));
	}

#if defined(__ARMHF__)
	int32_t p = cd->stackframesize - savedregs_num / 2;
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DLD(rd->savfltregs[i], REG_SP, p * 8 - (savedregs_num & 1) * 4);
	}
#endif

	/* deallocate stackframe for spilled variables */

	int32_t additional_bytes = (cd->stackframesize * 8 - savedregs_num * 4);

	if (additional_bytes > 0)
		M_ADD_IMM_EXT_MUL4(REG_SP, REG_SP, additional_bytes / 4);

	/* restore callee saved registers + do return */

	if (savedregs_bitmask) {
		if (!code_is_leafmethod(code)) {
			savedregs_bitmask &= ~(1<<REG_LR);
			savedregs_bitmask |= (1<<REG_PC);
		}
		M_LDMFD(savedregs_bitmask, REG_SP);
	}

	/* if LR was not on stack, we need to return manually */

	if (code_is_leafmethod(code))
		M_MOV(REG_PC, REG_LR);
}


/**
 * Generates machine code for one ICMD.
 */
void codegen_emit_instruction(jitdata* jd, instruction* iptr)
{
	varinfo*            var;
	builtintable_entry* bte;
	methodinfo*         lm;             // Local methodinfo for ICMD_INVOKE*.
	unresolved_method*  um;
	fieldinfo*          fi;
	unresolved_field*   uf;
	int32_t             fieldtype;
	int32_t             s1, s2, s3, d;
	int32_t             disp = 0;  // prevent uninitialized warning

	// Get required compiler data.
	codegendata*  cd   = jd->cd;

	/* the big switch */
	switch (iptr->opc) {

		/* constant operations ************************************************/

		case ICMD_ACONST:     /* ... ==> ..., constant                        */

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				disp = dseg_add_unique_address(cd, NULL);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
				                    iptr->sx.val.c.ref, disp);

				M_DSEG_LOAD(d, disp);
			}
			else {
				ICONST(d, (u4) iptr->sx.val.anyptr);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCONST:     /* ...  ==> ..., constant                       */

#if defined(ENABLE_SOFTFLOAT)
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			ICONST(d, iptr->sx.val.i);
			emit_store_dst(jd, iptr, d);
#else
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			FCONST(d, iptr->sx.val.f);
			emit_store_dst(jd, iptr, d);
#endif
			break;

		case ICMD_DCONST:     /* ...  ==> ..., constant                       */

#if defined(ENABLE_SOFTFLOAT)
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			LCONST(d, iptr->sx.val.l);
			emit_store_dst(jd, iptr, d);
#else
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			DCONST(d, iptr->sx.val.d);
			emit_store_dst(jd, iptr, d);
#endif
			break;


		/* integer operations *************************************************/

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV(d, REG_LSL(s1, 24));
			M_MOV(d, REG_ASR(d, 24));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2CHAR:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV(d, REG_LSL(s1, 16));
			M_MOV(d, REG_LSR(d, 16)); /* ATTENTION: char is unsigned */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2SHORT:  /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV(d, REG_LSL(s1, 16));
			M_MOV(d, REG_ASR(d, 16));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_INTMOVE(s1, GET_LOW_REG(d));
			M_MOV(GET_HIGH_REG(d), REG_ASR(s1, 31));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_RSB_IMM(d, s1, 0);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_RSB_IMMS(GET_LOW_REG(d), GET_LOW_REG(s1), 0);
			M_RSC_IMM(GET_HIGH_REG(d), GET_HIGH_REG(s1), 0);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_ADD(d, s1, s2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_ADD_S(GET_LOW_REG(d), s1, s2);
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_ADC(GET_HIGH_REG(d), s1, s2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IADDCONST:
		case ICMD_IINC:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			if (IS_IMM(iptr->sx.val.i)) {
				M_ADD_IMM(d, s1, iptr->sx.val.i);
			} else if (IS_IMM(-iptr->sx.val.i)) {
				M_SUB_IMM(d, s1, (-iptr->sx.val.i));
			} else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_ADD(d, s1, REG_ITMP3);
			}

			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                          */

			s3 = iptr->sx.val.l & 0xffffffff;
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (IS_IMM(s3))
				M_ADD_IMMS(GET_LOW_REG(d), s1, s3);
			else {
				ICONST(REG_ITMP3, s3);
				M_ADD_S(GET_LOW_REG(d), s1, REG_ITMP3);
			}
			s3 = iptr->sx.val.l >> 32;
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			if (IS_IMM(s3))
				M_ADC_IMM(GET_HIGH_REG(d), s1, s3);
			else {
				ICONST(REG_ITMP3, s3);
				M_ADC(GET_HIGH_REG(d), s1, REG_ITMP3);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_SUB(d, s1, s2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_SUB_S(GET_LOW_REG(d), s1, s2);
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_SBC(GET_HIGH_REG(d), s1, s2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (IS_IMM(iptr->sx.val.i))
				M_SUB_IMM(d, s1, iptr->sx.val.i);
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_SUB(d, s1, REG_ITMP3);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                          */

			s3 = iptr->sx.val.l & 0xffffffff;
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (IS_IMM(s3))
				M_SUB_IMMS(GET_LOW_REG(d), s1, s3);
			else {
				ICONST(REG_ITMP3, s3);
				M_SUB_S(GET_LOW_REG(d), s1, REG_ITMP3);
			}
			s3 = iptr->sx.val.l >> 32;
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			if (IS_IMM(s3))
				M_SBC_IMM(GET_HIGH_REG(d), s1, s3);
			else {
				ICONST(REG_ITMP3, s3);
				M_SBC(GET_HIGH_REG(d), s1, REG_ITMP3);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_MUL(d, s1, s2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_A1);
			emit_arithmetic_check(cd, iptr, s2);

			/* move arguments into argument registers */
			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s2, REG_A1);

			/* call builtin function */
			bte = iptr->sx.s23.s3.bte;
			disp = dseg_add_functionptr(cd, bte->fp);
			M_DSEG_BRANCH(disp);

			/* recompute pv */
			emit_recompute_pv(cd);

			/* move result into destination register */
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			M_INTMOVE(REG_RESULT, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			/* move arguments into argument registers */

			s1 = emit_load_s1(jd, iptr, REG_A0_A1_PACKED);
			s2 = emit_load_s2(jd, iptr, REG_A2_A3_PACKED);
			/* XXX TODO: only do this if arithmetic check is really done! */
			M_ORR(GET_HIGH_REG(s2), GET_LOW_REG(s2), REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_LNGMOVE(s1, REG_A0_A1_PACKED);
			M_LNGMOVE(s2, REG_A2_A3_PACKED);

			/* call builtin function */
			bte = iptr->sx.s23.s3.bte;
			disp = dseg_add_functionptr(cd, bte->fp);
			M_DSEG_BRANCH(disp);

			/* recompute pv */
			emit_recompute_pv(cd);

			/* move result into destination register */
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_PACKED);
			M_LNGMOVE(REG_RESULT_PACKED, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULPOW2:   /* ..., value  ==> ..., value * (2 ^ constant)  */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV(d, REG_LSL(s1, iptr->sx.val.i));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value / (2 ^ constant)  */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* this rounds towards 0 as java likes it */
			M_MOV(REG_ITMP3, REG_ASR(s1, 31));
			M_ADD(REG_ITMP3, s1, REG_LSR(REG_ITMP3, 32 - iptr->sx.val.i));
			M_MOV(d, REG_ASR(REG_ITMP3, iptr->sx.val.i));
			/* this rounds towards nearest, not java style */
			/*M_MOV_S(d, REG_ASR(s1, iptr->sx.val.i));
			M_ADCMI_IMM(d, d, 0);*/
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.i = constant [ (2 ^ x) - 1 ]          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV_S(REG_ITMP1, s1);
			M_RSBMI_IMM(REG_ITMP1, REG_ITMP1, 0);
			if (IS_IMM(iptr->sx.val.i))
				M_AND_IMM(REG_ITMP1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_AND(REG_ITMP1, REG_ITMP3, d);
			}
			M_RSBMI_IMM(d, d, 0);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_AND_IMM(s2, 0x1f, REG_ITMP2);
			M_MOV(d, REG_LSL_REG(s1, REG_ITMP2));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_AND_IMM(s2, 0x1f, REG_ITMP2);
			M_MOV(d, REG_ASR_REG(s1, REG_ITMP2));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_AND_IMM(s2, 0x1f, REG_ITMP2);
			M_MOV(d, REG_LSR_REG(s1, REG_ITMP2));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MOV(d, REG_LSL(s1, iptr->sx.val.i & 0x1f));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* we need to check for zero here because arm interprets it as SHR by 32 */
			if ((iptr->sx.val.i & 0x1f) == 0) {
				M_INTMOVE(s1, d);
			} else {
				M_MOV(d, REG_ASR(s1, iptr->sx.val.i & 0x1f));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* we need to check for zero here because arm interprets it as SHR by 32 */
			if ((iptr->sx.val.i & 0x1f) == 0)
				M_INTMOVE(s1, d);
			else
				M_MOV(d, REG_LSR(s1, iptr->sx.val.i & 0x1f));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_AND(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_AND(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_AND(s1, s2, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_ORR(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LOR:       /* ..., val1, val2  ==> ..., val1 | val2        */	

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_ORR(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_ORR(s1, s2, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_EOR(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_EOR(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_EOR(s1, s2, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;


	/* floating operations ************************************************/

#if !defined(ENABLE_SOFTFLOAT)

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FNEG(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		/* ATTENTION: Jave does not want IEEE behaviour in FREM, do
		   not use this */

#if 0
		case ICMD_FREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_RMFS(d, s1, s2);
			emit_store_dst(jd, iptr, d);
			break;
#endif

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DNEG(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		/* ATTENTION: Jave does not want IEEE behaviour in DREM, do
		   not use this */

#if 0
		case ICMD_DREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_RMFD(d, s1, s2);
			emit_store_dst(jd, iptr, d);
			break;
#endif

		case ICMD_I2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
#if defined(__VFP_FP__)
			M_FMSR(s1, REG_FTMP1);
			M_CVTIF(REG_FTMP1, d);
#else
			M_CVTIF(s1, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
#if defined(__VFP_FP__)
			M_FMSR(s1, d);
			M_CVTID(d, d);
#else
			M_CVTID(s1, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
#if defined(__VFP_FP__)
			M_CVTFI(s1, REG_FTMP2);
			M_FMRS(REG_FTMP2, d);
#else
			/* this uses round towards zero, as Java likes it */
			M_CVTFI(s1, d);
			/* this checks for NaN; to return zero as Java likes it */
			M_FCMP(s1, 0x8);
			M_MOVVS_IMM(0, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
#if defined(__VFP_FP__)
			M_CVTDI(s1, REG_FTMP2);
			M_FMRS(REG_FTMP2, d);
#else
			/* this uses round towards zero, as Java likes it */
			M_CVTDI(s1, d);
			/* this checks for NaN; to return zero as Java likes it */
			M_DCMP(s1, 0x8);
			M_MOVVS_IMM(0, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_CVTDF(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_CVTFD(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMP(s2, s1);
			M_MOV_IMM(d, 0);
#if defined(__VFP_FP__)
			M_FMSTAT; /* on VFP we need to transfer the flags */
#endif
			M_SUBGT_IMM(d, d, 1);
			M_ADDLT_IMM(d, d, 1);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DCMPG:      /* ..., val1, val2  ==> ..., val1 dcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_DCMP(s2, s1);
			M_MOV_IMM(d, 0);
#if defined(__VFP_FP__)
			M_FMSTAT; /* on VFP we need to transfer the flags */
#endif
			M_SUBGT_IMM(d, d, 1);
			M_ADDLT_IMM(d, d, 1);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMP(s1, s2);
			M_MOV_IMM(d, 0);
#if defined(__VFP_FP__)
			M_FMSTAT; /* on VFP we need to transfer the flags */
#endif
			M_SUBLT_IMM(d, d, 1);
			M_ADDGT_IMM(d, d, 1);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DCMPL:      /* ..., val1, val2  ==> ..., val1 dcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_DCMP(s1, s2);
			M_MOV_IMM(d, 0);
#if defined(__VFP_FP__)
			M_FMSTAT; /* on VFP we need to transfer the flags */
#endif
			M_SUBLT_IMM(d, d, 1);
			M_ADDGT_IMM(d, d, 1);
			emit_store_dst(jd, iptr, d);
			break;

#endif /* !defined(ENABLE_SOFTFLOAT) */


		/* memory operations **************************************************/

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, s2); /* REG_ITMP1 = s1 + 1 * s2 */
			M_LDRSB(d, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 1)); /* REG_ITMP1 = s1 + 2 * s2 */
			M_LDRH(d, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 1)); /* REG_ITMP1 = s1 + 2 * s2 */
			M_LDRSH(d, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
			M_ILD_INTERN(d, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP3, s1, REG_LSL(s2, 3)); /* REG_ITMP3 = s1 + 8 * s2 */
			M_LLD_INTERN(d, REG_ITMP3, OFFSET(java_longarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
#if !defined(ENABLE_SOFTFLOAT)
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_FLD_INTERN(d, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_ILD_INTERN(d, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP3, s1, REG_LSL(s2, 3)); /* REG_ITMP3 = s1 + 8 * s2 */
#if !defined(ENABLE_SOFTFLOAT)
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_DLD_INTERN(d, REG_ITMP3, OFFSET(java_doublearray_t, data[0]));
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LLD_INTERN(d, REG_ITMP3, OFFSET(java_doublearray_t, data[0]));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
			M_LDR_INTERN(d, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_ADD(REG_ITMP1, s1, s2); /* REG_ITMP1 = s1 + 1 * s2 */
			M_STRB(s3, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 1)); /* REG_ITMP1 = s1 + 2 * s2 */
			M_STRH(s3, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			break;

		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 1)); /* REG_ITMP1 = s1 + 2 * s2 */
			M_STRH(s3, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
			M_IST_INTERN(s3, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP3, s1, REG_LSL(s2, 3)); /* REG_ITMP3 = s1 + 8 * s2 */
			s3 = emit_load_s3(jd, iptr, REG_ITMP12_PACKED);
			M_LST_INTERN(s3, REG_ITMP3, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
#if !defined(ENABLE_SOFTFLOAT)
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			M_FST_INTERN(s3, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
#else
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_IST_INTERN(s3, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
#endif
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 3)); /* REG_ITMP1 = s1 + 8 * s2 */
#if !defined(ENABLE_SOFTFLOAT)
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			M_DST_INTERN(s3, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
#else
			s3 = emit_load_s3(jd, iptr, REG_ITMP23_PACKED);
			M_LST_INTERN(s3, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
#endif
			break;

		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_A1);

			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);

			/* move arguments to argument registers */
			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s3, REG_A1);

			/* call builtin function */
			disp = dseg_add_functionptr(cd, BUILTIN_FAST_canstore);
			M_DSEG_BRANCH(disp);

			/* recompute pv */
			emit_recompute_pv(cd);

			/* check resturn value of builtin */
			emit_arraystore_check(cd, iptr);

			/* finally store address into array */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_ADD(REG_ITMP1, s1, REG_LSL(s2, 2)); /* REG_ITMP1 = s1 + 4 * s2 */
			M_STR_INTERN(s3, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;

		case ICMD_GETFIELD:   /* ..., objectref, value  ==> ...               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP3);
			emit_nullpointer_check(cd, iptr, s1);


			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

#if !defined(ENABLE_SOFTFLOAT)
			/* HACK: softnull checks on floats */
			if (!INSTRUCTION_MUST_CHECK(iptr) && IS_FLT_DBL_TYPE(fieldtype))
				emit_nullpointer_check_force(cd, iptr, s1);
#endif

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				/* XXX REMOVE ME */
				uf = iptr->sx.s23.s3.uf;

				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
			}

			switch (fieldtype) {
			case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
#endif
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
				M_ILD(d, s1, disp);
				break;
			case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_DBL:
#endif
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
				M_LLD(d, s1, disp);
				break;
#if !defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_FLD(d, s1, disp);
				break;
			case TYPE_DBL:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_DLD(d, s1, disp);
				break;
#endif
			default:
				assert(false);
				break;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTFIELD:   /* ..., objectref, value  ==> ...               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP3);
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

#if !defined(ENABLE_SOFTFLOAT)
			/* HACK: softnull checks on floats */
			if (!INSTRUCTION_MUST_CHECK(iptr) && IS_FLT_DBL_TYPE(fieldtype))
				emit_nullpointer_check_force(cd, iptr, s1);
#endif

			switch (fieldtype) {
			case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
#endif
			case TYPE_ADR:
				s2 = emit_load_s2(jd, iptr, REG_ITMP1);
				break;
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_DBL: /* fall through */
#endif
			case TYPE_LNG:
				s2 = emit_load_s2(jd, iptr, REG_ITMP12_PACKED);
				break;
#if !defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
			case TYPE_DBL:
				s2 = emit_load_s2(jd, iptr, REG_FTMP1);
				break;
#endif
			default:
				assert(false);
				break;
			}

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				/* XXX REMOVE ME */
				uf = iptr->sx.s23.s3.uf;

				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
			}

			switch (fieldtype) {
			case TYPE_INT:
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
#endif
			case TYPE_ADR:
				M_IST(s2, s1, disp);
				break;
			case TYPE_LNG:
#if defined(ENABLE_SOFTFLOAT)
			case TYPE_DBL:
#endif
				M_LST(s2, s1, disp);
				break;
#if !defined(ENABLE_SOFTFLOAT)
			case TYPE_FLT:
				M_FST(s2, s1, disp);
				break;
			case TYPE_DBL:
				M_DST(s2, s1, disp);
				break;
#endif
			default:
				assert(false);
				break;
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			disp = dseg_add_functionptr(cd, asm_handle_exception);
			M_DSEG_LOAD(REG_ITMP3, disp);
			M_MOV(REG_ITMP2_XPC, REG_PC);
			M_MOV(REG_PC, REG_ITMP3);
			M_NOP;              /* nop ensures that XPC is less than the end  */
			                    /* of basic block                             */
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.l == 0) {
				M_ORR_S(s1, s2, REG_ITMP3);
			}
			else {
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMPEQ(s2, REG_ITMP3);
			}
			emit_beq(cd, iptr->dst.block);
			break;

		case ICMD_IF_LLT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.l == 0) {
				/* if high word is less than zero, the whole long is too */
				M_CMP_IMM(s1, 0);
				emit_blt(cd, iptr->dst.block);
			}
			else {
				/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				M_EOR(REG_ITMP1, REG_ITMP1, REG_ITMP1);
				M_MOVGT_IMM(2, REG_ITMP1);
				M_MOVEQ_IMM(1, REG_ITMP1);

				/* low compare: x=x-1(ifLO) */
				emit_icmp_imm(cd, s2, (iptr->sx.val.l & 0xffffffff));
  				/*ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMP(s2, REG_ITMP3);*/
				M_SUBLO_IMM(REG_ITMP1, REG_ITMP1, 1);

				/* branch if (x LT 1) */
				M_CMP_IMM(REG_ITMP1, 1);
				emit_blt(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LLE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.l == 0) {
				/* if high word is less than zero, the whole long is too  */
				M_CMP_IMM(s1, 0);
				emit_blt(cd, iptr->dst.block);

				/* ... otherwise the low word has to be zero (tricky!) */
				M_CMPEQ_IMM(s2, 0);
				emit_beq(cd, iptr->dst.block);
			}
			else {
				/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				M_EOR(REG_ITMP1, REG_ITMP1, REG_ITMP1);
				M_MOVGT_IMM(2, REG_ITMP1);
				M_MOVEQ_IMM(1, REG_ITMP1);

				/* low compare: x=x+1(ifHI) */
				emit_icmp_imm(cd, s2, (iptr->sx.val.l & 0xffffffff));
  				/*ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMP(s2, REG_ITMP3);*/
				M_ADDHI_IMM(REG_ITMP1, REG_ITMP1, 1);

				/* branch if (x LE 1) */
				M_CMP_IMM(REG_ITMP1, 1);
				emit_ble(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LGE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.l == 0) {
				/* if high word is greater or equal zero, the whole long is too */
				M_CMP_IMM(s1, 0);
				emit_bge(cd, iptr->dst.block);
			}
			else {
				/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				M_EOR(REG_ITMP1, REG_ITMP1, REG_ITMP1);
				M_MOVGT_IMM(2, REG_ITMP1);
				M_MOVEQ_IMM(1, REG_ITMP1);

				/* low compare: x=x-1(ifLO) */
				emit_icmp_imm(cd, s2, (iptr->sx.val.l & 0xffffffff));
  				/*ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMP(s2, REG_ITMP3);*/
				M_SUBLO_IMM(REG_ITMP1, REG_ITMP1, 1);

				/* branch if (x GE 1) */
				M_CMP_IMM(REG_ITMP1, 1);
				emit_bge(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LGT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
#if 0
			if (iptr->sx.val.l == 0) {
				/* if high word is greater than zero, the whole long is too */
				M_CMP_IMM(s1, 0);
				M_BGT(0);
				codegen_add_branch_ref(cd, iptr->dst.block);

				/* ... or high was zero and low is non zero (tricky!) */
				M_EOR(REG_ITMP3, REG_ITMP3, REG_ITMP3);
				M_MOVLT_IMM(1, REG_ITMP3);
				M_ORR_S(REG_ITMP3, s2, REG_ITMP3);
				M_BNE(0);
				codegen_add_branch_ref(cd, iptr->dst.block);
			}
			else {
#endif
				/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				M_EOR(REG_ITMP1, REG_ITMP1, REG_ITMP1);
				M_MOVGT_IMM(2, REG_ITMP1);
				M_MOVEQ_IMM(1, REG_ITMP1);

				/* low compare: x=x+1(ifHI) */
				emit_icmp_imm(cd, s2, (iptr->sx.val.l & 0xffffffff));
  				/*ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMP(s2, REG_ITMP3);*/
				M_ADDHI_IMM(REG_ITMP1, REG_ITMP1, 1);

				/* branch if (x GT 1) */
				M_CMP_IMM(REG_ITMP1, 1);
				emit_bgt(cd, iptr->dst.block);
#if 0
			}
#endif
			break;

		case ICMD_IF_LNE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.l == 0) {
				M_ORR_S(s1, s2, REG_ITMP3);
			}
			else {
				emit_icmp_imm(cd, s1, (iptr->sx.val.l >> 32));
				/*ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_CMP(s1, REG_ITMP3);*/
				ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_CMPEQ(s2, REG_ITMP3);
			}
			emit_bne(cd, iptr->dst.block);
			break;
			
		case ICMD_IF_LCMPEQ:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPEQ(s1, s2);

			emit_beq(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPNE:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPEQ(s1, s2);

			emit_bne(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLT:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_EOR(REG_ITMP3, REG_ITMP3, REG_ITMP3);
			M_MOVGT_IMM(2, REG_ITMP3);
			M_MOVEQ_IMM(1, REG_ITMP3);

			/* low compare: x=x-1(ifLO) */
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_SUBLO_IMM(REG_ITMP3, REG_ITMP3, 1);

			/* branch if (x LT 1) */
			M_CMP_IMM(REG_ITMP3, 1);
			emit_blt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLE:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_EOR(REG_ITMP3, REG_ITMP3, REG_ITMP3);
			M_MOVGT_IMM(2, REG_ITMP3);
			M_MOVEQ_IMM(1, REG_ITMP3);

			/* low compare: x=x-1(ifLO) */
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_ADDHI_IMM(REG_ITMP3, REG_ITMP3, 1);

			/* branch if (x LE 1) */
			M_CMP_IMM(REG_ITMP3, 1);
			emit_ble(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGT:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_EOR(REG_ITMP3, REG_ITMP3, REG_ITMP3);
			M_MOVGT_IMM(2, REG_ITMP3);
			M_MOVEQ_IMM(1, REG_ITMP3);

			/* low compare: x=x-1(ifLO) */
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_ADDHI_IMM(REG_ITMP3, REG_ITMP3, 1);

			/* branch if (x GT 1) */
			M_CMP_IMM(REG_ITMP3, 1);
			emit_bgt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGE:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			/* high compare: x=0(ifLT) ; x=1(ifEQ) ; x=2(ifGT) */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_EOR(REG_ITMP3, REG_ITMP3, REG_ITMP3);
			M_MOVGT_IMM(2, REG_ITMP3);
			M_MOVEQ_IMM(1, REG_ITMP3);

			/* low compare: x=x-1(ifLO) */
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_SUBLO_IMM(REG_ITMP3, REG_ITMP3, 1);

			/* branch if (x GE 1) */
			M_CMP_IMM(REG_ITMP3, 1);
			emit_bge(cd, iptr->dst.block);
			break;

		case ICMD_TABLESWITCH:  /* ..., index ==> ...                         */
			{
			s4 i, l;
			branch_target_t *table;

			table = iptr->dst.table;

			l = iptr->sx.s23.s2.tablelow;
			i = iptr->sx.s23.s3.tablehigh;

			/* calculate new index (index - low) */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (l  == 0) {
				M_INTMOVE(s1, REG_ITMP1);
			} else if (IS_IMM(l)) {
				M_SUB_IMM(REG_ITMP1, s1, l);
			} else {
				ICONST(REG_ITMP2, l);
				M_SUB(REG_ITMP1, s1, REG_ITMP2);
			}

			/* range check (index <= high-low) */
			i = i - l + 1;
			emit_icmp_imm(cd, REG_ITMP1, i-1);
			emit_bugt(cd, table[0].block);

			/* build jump table top down and use address of lowest entry */

			table += i;

			while (--i >= 0) {
				dseg_add_target(cd, table->block);
				--table;
			}
			}

			/* length of dataseg after last dseg_add_target is used by load */
			/* TODO: this loads from data-segment */
			M_ADD(REG_ITMP2, REG_PV, REG_LSL(REG_ITMP1, 2));
			M_LDR(REG_PC, REG_ITMP2, -(cd->dseglen));
			break;

		case ICMD_BUILTIN:
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL) {
				disp = dseg_add_functionptr(cd, bte->fp);
			} else {
				disp = dseg_add_functionptr(cd, bte->stub);
			}

			M_DSEG_LOAD(REG_PV, disp); /* pointer to built-in-function */

			/* generate the actual call */

#if defined(ARM_NO_THUMB_IW)
			M_MOV(REG_LR, REG_PC);
			M_MOV(REG_PC, REG_PV);
#else
			M_BLX(REG_PV);
#endif

			break;

		case ICMD_INVOKESPECIAL:
			emit_nullpointer_check(cd, iptr, REG_A0);
			/* fall through */

		case ICMD_INVOKESTATIC:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, NULL);

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special,
									um, disp);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_address(cd, lm->stubroutine);
			}

			M_DSEG_LOAD(REG_PV, disp);            /* Pointer to method */

			/* generate the actual call */

			M_MOV(REG_LR, REG_PC);
			M_MOV(REG_PC, REG_PV);
			break;

		case ICMD_INVOKEVIRTUAL:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				int32_t disp = dseg_add_unique_s4(cd, 0);
				patcher_add_patch_ref(jd, PATCHER_invokevirtual, um, disp);

				// The following instruction MUST NOT change a0 because of the implicit NPE check.
				M_LDR_INTERN(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));

				// Sanity check.
				assert(REG_ITMP1 != REG_METHODPTR);
				assert(REG_ITMP2 == REG_METHODPTR);

				M_DSEG_LOAD(REG_ITMP1, disp);
				M_ADD(REG_METHODPTR, REG_METHODPTR, REG_ITMP1);

				// This must be a load with displacement,
				// otherwise the JIT method address patching does
				// not work anymore (see md_jit_method_patch_address).
				M_LDR_INTERN(REG_PV, REG_METHODPTR, 0);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, table[0]) + sizeof(methodptr) * lm->vftblindex;

				// The following instruction MUST NOT change a0 because of the implicit NPE check.
				M_LDR_INTERN(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
				M_LDR(REG_PV, REG_METHODPTR, s1);
			}

			// Generate the actual call.
			M_MOV(REG_LR, REG_PC);
			M_MOV(REG_PC, REG_PV);
			break;

		case ICMD_INVOKEINTERFACE:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				int32_t disp  = dseg_add_unique_s4(cd, 0);
				int32_t disp2 = dseg_add_unique_s4(cd, 0);

				// XXX We need two displacements.
				assert(disp2 == disp - 4);
				patcher_add_patch_ref(jd, PATCHER_invokeinterface, um, disp);

				// The following instruction MUST NOT change a0 because of the implicit NPE check.
				M_LDR_INTERN(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));

				// Sanity check.
				assert(REG_ITMP1 != REG_METHODPTR);
				assert(REG_ITMP2 == REG_METHODPTR);
				assert(REG_ITMP3 != REG_METHODPTR);

				M_DSEG_LOAD(REG_ITMP1, disp);
				M_LDR_REG(REG_METHODPTR, REG_METHODPTR, REG_ITMP1);

				M_DSEG_LOAD(REG_ITMP3, disp2);
				M_ADD(REG_METHODPTR, REG_METHODPTR, REG_ITMP3);

				// This must be a load with displacement,
				// otherwise the JIT method address patching does
				// not work anymore (see md_jit_method_patch_address).
				M_LDR_INTERN(REG_PV, REG_METHODPTR, 0);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, interfacetable[0]) - sizeof(methodptr*) * lm->clazz->index;
				s2 = sizeof(methodptr) * (lm - lm->clazz->methods);

				// The following instruction MUST NOT change a0 because of the implicit NPE check.
				M_LDR_INTERN(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
				M_LDR(REG_METHODPTR, REG_METHODPTR, s1);
				M_LDR(REG_PV, REG_METHODPTR, s2);
			}

			// Generate the actual call.
			M_MOV(REG_LR, REG_PC);
			M_MOV(REG_PC, REG_PV);
			break;

		case ICMD_CHECKCAST:  /* ..., objectref ==> ..., objectref            */

			if (!(iptr->flags.bits & INS_FLAG_ARRAY)) {
				/* object type cast-check */

			classinfo *super;
			s4         superindex;

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				super      = NULL;
				superindex = 0;
			}
			else {
				super      = iptr->sx.s23.s3.c.cls;
				superindex = super->index;
			}

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_TST(s1, s1);
				emit_label_beq(cd, BRANCH_LABEL_1);

				disp = dseg_add_unique_s4(cd, 0); /* super->flags */
				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
				                    iptr->sx.s23.s3.c.ref, disp);

				M_DSEG_LOAD(REG_ITMP2, disp);
				disp = dseg_add_s4(cd, ACC_INTERFACE);
				M_DSEG_LOAD(REG_ITMP3, disp);
				M_TST(REG_ITMP2, REG_ITMP3);
				emit_label_beq(cd, BRANCH_LABEL_2);
			}

			/* interface checkcast code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if ((super == NULL) || !IS_IMM(superindex)) {
					disp = dseg_add_unique_s4(cd, superindex);
				}
				if (super == NULL) {
					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_index,
					                    iptr->sx.s23.s3.c.ref, disp);
				}
				else {
					M_TST(s1, s1);
					emit_label_beq(cd, BRANCH_LABEL_3);
				}

				M_LDR_INTERN(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_LDR_INTERN(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, interfacetablelength));

				/* we put unresolved or non-immediate superindices onto dseg */
				if ((super == NULL) || !IS_IMM(superindex)) {
					/* disp was computed before we added the patcher */
					M_DSEG_LOAD(REG_ITMP2, disp);
					M_CMP(REG_ITMP3, REG_ITMP2);
				} else {
					assert(IS_IMM(superindex));
					M_CMP_IMM(REG_ITMP3, superindex);
				}

				emit_classcast_check(cd, iptr, BRANCH_LE, REG_ITMP3, s1);

				/* if we loaded the superindex out of the dseg above, we do
				   things differently here! */
				if ((super == NULL) || !IS_IMM(superindex)) {

					M_LDR_INTERN(REG_ITMP3, s1, OFFSET(java_object_t, vftbl));

					/* this assumes something */
					assert(OFFSET(vftbl_t, interfacetable[0]) == 0);

					/* this does: REG_ITMP3 - superindex * sizeof(methodptr*) */
					assert(sizeof(methodptr*) == 4);
					M_SUB(REG_ITMP2, REG_ITMP3, REG_LSL(REG_ITMP2, 2));

					s2 = 0;

				} else {

					s2 = OFFSET(vftbl_t, interfacetable[0]) -
								superindex * sizeof(methodptr*);

				}

				M_LDR_INTERN(REG_ITMP3, REG_ITMP2, s2);
				M_TST(REG_ITMP3, REG_ITMP3);
				emit_classcast_check(cd, iptr, BRANCH_EQ, REG_ITMP3, s1);

				if (super == NULL)
					emit_label_br(cd, BRANCH_LABEL_4);
				else
					emit_label(cd, BRANCH_LABEL_3);
			}

			/* class checkcast code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);

					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
					                    iptr->sx.s23.s3.c.ref,
										disp);
				}
				else {
					disp = dseg_add_address(cd, super->vftbl);

					M_TST(s1, s1);
					emit_label_beq(cd, BRANCH_LABEL_5);
				}

				// The following code checks whether object s is a subtype of class t.
				// Represents the following semantic:
				//    if (!fast_subtype_check(s->vftbl, t->vftbl)) throw;

				M_LDR_INTERN(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_DSEG_LOAD(REG_ITMP3, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					// Represents the following semantic:
					//    if (*(s->vftbl + t->vftbl->subtype_offset) == t->vftbl) good;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_LDR_INTERN(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_LDR_REG(REG_ITMP1, REG_ITMP2, REG_ITMP1);
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_load_s1(jd, iptr, REG_ITMP1);  /* reload s1, might have been destroyed */
					emit_label_beq(cd, BRANCH_LABEL_6);  /* good */

					// Represents the following semantic:
					//    if (t->vftbl->subtype_offset != OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE])) throw;
					// Preconditions:
					//    REG_ITMP3==t->vftbl;
					if (super == NULL) {
						M_LDR_INTERN(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_CMP_IMM(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]));
						emit_load_s1(jd, iptr, REG_ITMP1);  /* reload s1, might have been destroyed */
						emit_classcast_check(cd, iptr, BRANCH_NE, 0, s1);  /* throw */
					}

					// Represents the following semantic:
					//    if (s->vftbl->subtype_depth < t->vftbl->subtype_depth) throw;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_LDR_INTERN(REG_ITMP1, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
					M_LDR_INTERN(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_load_s1(jd, iptr, REG_ITMP1);  /* reload s1, might have been destroyed */
					emit_classcast_check(cd, iptr, BRANCH_LT, 0, s1);  /* throw */

					// Represents the following semantic:
					//    if (s->vftbl->subtype_overflow[t->vftbl->subtype_depth - DISPLAY_SIZE] != t->vftbl) throw;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl->subtype_depth;
					M_LDR_INTERN(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
					M_ADD(REG_ITMP2, REG_ITMP2, REG_LSL(REG_ITMP3, 2));  /* REG_ITMP2 = REG_ITMP2 + 4 * REG_ITMP3 */
					M_LDR_INTERN(REG_ITMP2, REG_ITMP2, -DISPLAY_SIZE * SIZEOF_VOID_P);
					M_DSEG_LOAD(REG_ITMP3, disp);  /* reload REG_ITMP3, was destroyed */
					M_CMP(REG_ITMP2, REG_ITMP3);
					emit_classcast_check(cd, iptr, BRANCH_NE, 0, s1);  /* throw */

					emit_label(cd, BRANCH_LABEL_6);
				}
				else {
					// Represents the following semantic:
					//    if (*(s->vftbl + t->vftbl->subtype_offset) != t->vftbl) throw;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
					M_CMP(REG_ITMP2, REG_ITMP3);
					emit_classcast_check(cd, iptr, BRANCH_NE, 0, s1);
				}

				if (super != NULL)
					emit_label(cd, BRANCH_LABEL_5);
			}

			if (super == NULL) {
				emit_label(cd, BRANCH_LABEL_1);
				emit_label(cd, BRANCH_LABEL_4);
			}

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			}
			else {
				/* array type cast-check */

				s1 = emit_load_s1(jd, iptr, REG_A0);
				M_INTMOVE(s1, REG_A0);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
										iptr->sx.s23.s3.c.ref,
										disp);
				}
				else
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

				M_DSEG_LOAD(REG_A1, disp);
				disp = dseg_add_functionptr(cd, BUILTIN_arraycheckcast);
				M_DSEG_BRANCH(disp);

				emit_recompute_pv(cd);

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				M_TST(REG_RESULT, REG_RESULT);
				emit_classcast_check(cd, iptr, BRANCH_EQ, REG_RESULT, s1);

				d = codegen_reg_of_dst(jd, iptr, s1);
			}

			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INSTANCEOF: /* ..., objectref ==> ..., intresult            */

			{
			classinfo *super;
			s4         superindex;

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				super      = NULL;
				superindex = 0;
			}
			else {
				super      = iptr->sx.s23.s3.c.cls;
				superindex = super->index;
			}

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);

			if (s1 == d) {
				M_MOV(REG_ITMP1, s1);
				s1 = REG_ITMP1;
			}

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_EOR(d, d, d);

				M_TST(s1, s1);
				emit_label_beq(cd, BRANCH_LABEL_1);

				disp = dseg_add_unique_s4(cd, 0); /* super->flags */
				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
				                    iptr->sx.s23.s3.c.ref, disp);

				M_DSEG_LOAD(REG_ITMP2, disp);
				disp = dseg_add_s4(cd, ACC_INTERFACE);
				M_DSEG_LOAD(REG_ITMP3, disp);
				M_TST(REG_ITMP2, REG_ITMP3);
				emit_label_beq(cd, BRANCH_LABEL_2);
			}

			/* interface checkcast code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if ((super == NULL) || !IS_IMM(superindex)) {
					disp = dseg_add_unique_s4(cd, superindex);
				}
				if (super == NULL) {
					/* If d == REG_ITMP2, then it's destroyed in check
					   code above.  */
					if (d == REG_ITMP2)
						M_EOR(d, d, d);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_index,
					                    iptr->sx.s23.s3.c.ref, disp);
				}
				else {
					M_EOR(d, d, d);
					M_TST(s1, s1);
					emit_label_beq(cd, BRANCH_LABEL_3);
				}

				M_LDR_INTERN(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_LDR_INTERN(REG_ITMP3,
							 REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));

				/* we put unresolved or non-immediate superindices onto dseg
				   and do things slightly different */
				if ((super == NULL) || !IS_IMM(superindex)) {
					/* disp was computed before we added the patcher */
					M_DSEG_LOAD(REG_ITMP2, disp);
					M_CMP(REG_ITMP3, REG_ITMP2);

					if (d == REG_ITMP2) {
						M_EORLE(d, d, d);
						M_BLE(4);
					} else {
						M_BLE(3);
					}

					/* this assumes something */
					assert(OFFSET(vftbl_t, interfacetable[0]) == 0);

					/* this does: REG_ITMP3 - superindex * sizeof(methodptr*) */
					assert(sizeof(methodptr*) == 4);
					M_SUB(REG_ITMP1, REG_ITMP1, REG_LSL(REG_ITMP2, 2));

					if (d == REG_ITMP2) {
						M_EOR(d, d, d);
					}

					s2 = 0;

				} else {
					assert(IS_IMM(superindex));
					M_CMP_IMM(REG_ITMP3, superindex);

					M_BLE(2);

					s2 = OFFSET(vftbl_t, interfacetable[0]) -
						superindex * sizeof(methodptr*);

				}

				M_LDR_INTERN(REG_ITMP3, REG_ITMP1, s2);
				M_TST(REG_ITMP3, REG_ITMP3);
				M_MOVNE_IMM(1, d);

				if (super == NULL)
					emit_label_br(cd, BRANCH_LABEL_4);
				else
					emit_label(cd, BRANCH_LABEL_3);
			}

			/* class checkcast code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);

					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
					                    iptr->sx.s23.s3.c.ref, disp);
				}
				else {
					disp = dseg_add_address(cd, super->vftbl);

					M_EOR(d, d, d);
					M_TST(s1, s1);
					emit_label_beq(cd, BRANCH_LABEL_5);
				}

				// The following code checks whether object s is a subtype of class t.
				// Represents the following semantic:
				//    fast_subtype_check(s->vftbl, t->vftbl));

				M_LDR_INTERN(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_DSEG_LOAD(REG_ITMP3, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					// Represents the following semantic:
					//    if (*(s->vftbl + t->vftbl->subtype_offset) == t->vftbl) true;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_LDR_INTERN(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_LDR_REG(REG_ITMP1, REG_ITMP2, REG_ITMP1);
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_label_beq(cd, BRANCH_LABEL_6);  /* true */

					// Represents the following semantic:
					//    if (t->vftbl->subtype_offset != OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE])) false;
					// Preconditions:
					//    REG_ITMP3==t->vftbl;
					if (super == NULL) {
						M_LDR_INTERN(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_CMP_IMM(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]));
						emit_label_bne(cd, BRANCH_LABEL_7);  /* false */
					}

					// Represents the following semantic:
					//    if (s->vftbl->subtype_depth < t->vftbl->subtype_depth) false;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_LDR_INTERN(REG_ITMP1, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
					M_LDR_INTERN(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_label_blt(cd, BRANCH_LABEL_8);  /* false */

					// Represents the following semantic:
					//    if (s->vftbl->subtype_overflow[t->vftbl->subtype_depth - DISPLAY_SIZE] != t->vftbl) false;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl->subtype_depth;
					M_LDR_INTERN(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
					M_ADD(REG_ITMP2, REG_ITMP2, REG_LSL(REG_ITMP3, 2));  /* REG_ITMP2 = REG_ITMP2 + 4 * REG_ITMP3 */
					M_LDR_INTERN(REG_ITMP2, REG_ITMP2, -DISPLAY_SIZE * SIZEOF_VOID_P);
					M_DSEG_LOAD(REG_ITMP3, disp);  /* reload REG_ITMP3, was destroyed */
					M_CMP(REG_ITMP2, REG_ITMP3);

					emit_label(cd, BRANCH_LABEL_6);
					if (super == NULL)
						emit_label(cd, BRANCH_LABEL_7);
					emit_label(cd, BRANCH_LABEL_8);

					/* If d == REG_ITMP2, then it's destroyed */
					if (d == REG_ITMP2)
						M_EOR(d, d, d);
					M_MOVEQ_IMM(1, d);
				}
				else {
					// Represents the following semantic:
					//    *(s->vftbl + t->vftbl->subtype_offset) == t->vftbl;
					// Preconditions:
					//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
					M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
					M_CMP(REG_ITMP2, REG_ITMP3);
					/* If d == REG_ITMP2, then it's destroyed */
					if (d == REG_ITMP2)
						M_EOR(d, d, d);
					M_MOVEQ_IMM(1, d);
				}

				if (super != NULL)
					emit_label(cd, BRANCH_LABEL_5);
			}

			if (super == NULL) {
				emit_label(cd, BRANCH_LABEL_1);
				emit_label(cd, BRANCH_LABEL_4);
			}

			}

			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_MULTIANEWARRAY:/* ..., cnt1, [cnt2, ...] ==> ..., arrayref  */

			/* copy sizes to stack if necessary  */

			MCODECHECK((iptr->s1.argcount << 1) + 64);

			for (s1 = iptr->s1.argcount; --s1 >= 0; ) {

				var = VAR(iptr->sx.s23.s2.args[s1]);
	
				/* copy SAVEDVAR sizes to stack */

				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
					M_STR(s2, REG_SP, s1 * 4);
				}
			}

			/* a0 = dimension count */

			assert(IS_IMM(iptr->s1.argcount));
			M_MOV_IMM(REG_A0, iptr->s1.argcount);

			/* is patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				disp = dseg_add_unique_address(cd, NULL);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									iptr->sx.s23.s3.c.ref, disp);
			}
			else
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

			/* a1 = arraydescriptor */

			M_DSEG_LOAD(REG_A1, disp);

			/* a2 = pointer to dimensions = stack pointer */

			M_INTMOVE(REG_SP, REG_A2);

			/* call builtin_multianewarray here */

			disp = dseg_add_functionptr(cd, BUILTIN_multianewarray);
			M_DSEG_BRANCH(disp);

			/* recompute pv */

			emit_recompute_pv(cd);

			/* check for exception before result assignment */

			emit_exception_check(cd, iptr);

			/* get arrayref */

			d = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			M_INTMOVE(REG_RESULT, d);
			emit_store_dst(jd, iptr, d);
			break;

		default:
			vm_abort("Unknown ICMD %d during code generation", iptr->opc);
	} /* the big switch */
}


/* codegen_emit_stub_native ****************************************************

   Emits a stub routine which calls a native method.

*******************************************************************************/

void codegen_emit_stub_native(jitdata *jd, methoddesc *nmd, functionptr f, int skipparams)
{
	methodinfo  *m;
	codeinfo    *code;
	codegendata *cd;
	methoddesc  *md;
	s4           i, j;
	s4           t;
	int          s1, s2, tmpfreg;
	int          disp;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	/* initialize variables */

	md = m->parseddesc;

	/* calculate stackframe size */

	/* XXX There seems to be confusion about the unit of this number. Sometimes
	 * it's interpreted as the number of 4-byte words, while at other times, it
	 * seems to be the number of 8-byte entities. */
	cd->stackframesize =
		1 +                                                /* return address  */
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +         /* stackframeinfo  */
		sizeof(localref_table) / SIZEOF_VOID_P +           /* localref_table  */
		nmd->memuse +                                      /* stack arguments */
#if defined(__ARMHF__)
		9;                                                 /* float arguments (8 regs + 1 temporary) */
#else
		0;
#endif

	/* align stack to 8-byte */

	cd->stackframesize = (cd->stackframesize + 1) & ~1;

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize);     /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate stub code */

	M_STMFD(1<<REG_LR, REG_SP);
	M_SUB_IMM_EXT_MUL4(REG_SP, REG_SP, cd->stackframesize * 2 - 1);

#if defined(ENABLE_GC_CACAO)
	/* Save callee saved integer registers in stackframeinfo (GC may
	   need to recover them during a collection). */

	disp = cd->stackframesize - SIZEOF_VOID_P - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_STR_INTERN(abi_registers_integer_saved[i], REG_SP, disp + i * 4);
#endif

	/* Save integer and float argument registers (these are 4
	   registers, stack is 8-byte aligned). */

	M_STMFD(BITMASK_ARGS, REG_SP);

#if defined(__ARMHF__)
	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s1 = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_FLT:
			case TYPE_DBL:
				assert(s1 < 8);
				M_DST(s1, REG_SP, (cd->stackframesize - 9 + s1) * 8);
				break;
			default:
				break;
			}
		}
	}

	tmpfreg = REG_FTMP2;
	/* If REG_FTMP2 is taken up by the arguments, we have to resort to one of
	 * the saved registers. */
	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		t = md->paramtypes[i].type;
		if (IS_FLT_DBL_TYPE(t)) {
			if (nmd->params[j].inmemory)
				continue;
			s2 = nmd->params[j].regoff;

			if (s2 < REG_FTMP2)
				break;
			if (s2 == REG_FTMP2) {
				tmpfreg = abi_registers_float_saved[0];
				break;
			}
		}
	}

	if (tmpfreg != REG_FTMP2)
		M_DST(tmpfreg, REG_SP, (cd->stackframesize - 1) * 8);
#endif

	/* create native stackframe info */

	M_ADD_IMM(REG_A0, REG_SP, 4*4);
	M_MOV(REG_A1, REG_PV);
	disp = dseg_add_functionptr(cd, codegen_start_native_call);
	M_DSEG_BRANCH(disp);

	/* recompute pv */

	emit_recompute_pv(cd);

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_ITMP3, REG_RESULT);

	/* Restore integer and float argument registers (these are 4
	   registers, stack is 8-byte aligned). */

	M_LDMFD(BITMASK_ARGS, REG_SP);

	/* copy or spill arguments to new locations */

#if defined(__ARMHF__)
	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		t = md->paramtypes[i].type;
		s2 = nmd->params[j].regoff;

		if (IS_INT_LNG_TYPE(t)) {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;

				if (!nmd->params[j].inmemory) {
					if (IS_2_WORD_TYPE(t))
						M_LNGMOVE(s1, s2);
					else
						M_INTMOVE(s1, s2);
				}
				else {
					if (IS_2_WORD_TYPE(t))
						M_LST(s1, REG_SP, s2);
					else
						M_IST(s1, REG_SP, s2);
				}
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;

				if (IS_2_WORD_TYPE(t)) {
					M_LLD(REG_ITMP12_PACKED, REG_SP, s1);
					M_LST(REG_ITMP12_PACKED, REG_SP, s2);
				}
				else {
					M_ILD(REG_ITMP1, REG_SP, s1);
					M_IST(REG_ITMP1, REG_SP, s2);
				}
			}
		}
		else {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;

				M_DLD(tmpfreg, REG_SP, (cd->stackframesize - 11 + s1) * 8);
				if (!nmd->params[j].inmemory) {

					if (IS_2_WORD_TYPE(t))
						M_DMOV(tmpfreg, s2);
					else
						M_FMOV(tmpfreg, s2);

				}
				else {
					if (IS_2_WORD_TYPE(t))
						M_DST(REG_FTMP2, REG_SP, s2);
					else
						M_FST(REG_FTMP2, REG_SP, s2);
				}
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;

				if (!nmd->params[j].inmemory) {
					if (IS_2_WORD_TYPE(t))
						M_DLD(s2, REG_SP, s1);
					else
						M_FLD(s2, REG_SP, s1);
				}
				else {
					if (IS_2_WORD_TYPE(t)) {
						M_DLD(REG_FTMP1, REG_SP, s1);
						M_DST(REG_FTMP1, REG_SP, s2);
					}
					else {
						M_FLD(REG_FTMP1, REG_SP, s1);
						M_FST(REG_FTMP1, REG_SP, s2);
					}
				}
			}
		}
	}
#else
	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		t = md->paramtypes[i].type;
		s2 = nmd->params[j].regoff;

		if (!md->params[i].inmemory) {
			s1 = md->params[i].regoff;

			if (!nmd->params[j].inmemory) {
#if !defined(__ARM_EABI__)
				SPLIT_OPEN(t, s2, REG_ITMP1);
#endif

				if (IS_2_WORD_TYPE(t))
					M_LNGMOVE(s1, s2);
				else
					M_INTMOVE(s1, s2);

#if !defined(__ARM_EABI__)
				SPLIT_STORE_AND_CLOSE(t, s2, 0);
#endif
			}
			else {
				if (IS_2_WORD_TYPE(t))
					M_LST(s1, REG_SP, s2);
				else
					M_IST(s1, REG_SP, s2);
			}
		}
		else {
			s1 = md->params[i].regoff + cd->stackframesize * 8;

			if (IS_2_WORD_TYPE(t)) {
				M_LLD(REG_ITMP12_PACKED, REG_SP, s1);
				M_LST(REG_ITMP12_PACKED, REG_SP, s2);
			}
			else {
				M_ILD(REG_ITMP1, REG_SP, s1);
				M_IST(REG_ITMP1, REG_SP, s2);
			}
		}
	}
#endif

	/* Handle native Java methods. */

	if (m->flags & ACC_NATIVE) {
		/* put class into second argument register */

		if (m->flags & ACC_STATIC)
			M_MOV(REG_A1, REG_ITMP3);

		/* put env into first argument register */

		disp = dseg_add_address(cd, VM::get_current()->get_jnienv());
		M_DSEG_LOAD(REG_A0, disp);
	}

	/* Call the native function. */

	disp = dseg_add_functionptr(cd, f);
	M_DSEG_BRANCH(disp);

	/* recompute pv */
	/* TODO: this is only needed because of the tracer ... do we
	   really need it? */

	emit_recompute_pv(cd);

#if defined(__ARMHF__)
	/* restore borrowed float register */
	if (tmpfreg != REG_FTMP2)
		M_DLD(tmpfreg, REG_SP, (cd->stackframesize - 1) * 8);
#endif

	/* remove native stackframe info */
	/* TODO: improve this store/load */

	M_STMFD(BITMASK_RESULT, REG_SP);

	M_ADD_IMM(REG_A0, REG_SP, 2*4);
	M_MOV(REG_A1, REG_PV);
	disp = dseg_add_functionptr(cd, codegen_finish_native_call);
	M_DSEG_BRANCH(disp);
	emit_recompute_pv(cd);

	M_MOV(REG_ITMP1_XPTR, REG_RESULT);
	M_LDMFD(BITMASK_RESULT, REG_SP);

#if defined(ENABLE_GC_CACAO)
	/* restore callee saved int registers from stackframeinfo (GC might have  */
	/* modified them during a collection).                                    */

	disp = cd->stackframesize - SIZEOF_VOID_P - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_LDR_INTERN(abi_registers_integer_saved[i], REG_SP, disp + i * 4);
#endif

	/* finish stub code, but do not yet return to caller */

	M_ADD_IMM_EXT_MUL4(REG_SP, REG_SP, cd->stackframesize * 2 - 1);
	M_LDMFD(1<<REG_LR, REG_SP);

	/* check for exception */

	M_TST(REG_ITMP1_XPTR, REG_ITMP1_XPTR);
	M_MOVEQ(REG_LR, REG_PC);            /* if no exception, return to caller  */

	/* handle exception here */

	M_SUB_IMM(REG_ITMP2_XPC, REG_LR, 4);/* move fault address into xpc        */

	disp = dseg_add_functionptr(cd, asm_handle_nat_exception);
	M_DSEG_LOAD(REG_ITMP3, disp);       /* load asm exception handler address */
	M_MOV(REG_PC, REG_ITMP3);           /* jump to asm exception handler      */
}


/* asm_debug *******************************************************************

   Lazy debugger!

*******************************************************************************/

void asm_debug(int a1, int a2, int a3, int a4)
{
	printf("===> i am going to exit after this debugging message!\n");
	printf("got asm_debug(%p, %p, %p, %p)\n",(void*)a1,(void*)a2,(void*)a3,(void*)a4);
	vm_abort("leave you now");
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
