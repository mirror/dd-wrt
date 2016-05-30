/* src/vm/jit/sparc64/codegen.cpp - machine code generator for Sparc

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
#include <stdint.h>
#include <stdio.h>

#include "vm/types.hpp"

#include "md-abi.hpp"

// #include "vm/jit/sparc64/arch.hpp"
#include "vm/jit/sparc64/codegen.hpp"
#include "vm/jit/sparc64/emit.hpp"

#include "mm/memory.hpp"

#include "native/localref.hpp"
#include "native/native.hpp"

#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/patcher.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/stacktrace.hpp"

#include "vm/jit/sparc64/solaris/macro_rename.hpp"

#define BUILTIN_FLOAT_ARGS 1

/* XXX use something like this for window control ? 
 * #define REG_PV (own_window?REG_PV_CALLEE:REG_PV_CALLER)
 */
#define REG_PV REG_PV_CALLEE

bool fits_13(s4 disp)
{
	/*
	if ((disp < -4096) || (disp > 4095))
		printf("disp %d\n", disp);
	*/

	return (disp >= -4096) && (disp <= 4095);
}

s4 get_lopart_disp(disp)
{
	s4 lodisp;
	
	if (disp > 0)
		lodisp = setlo_part(disp);
	else {
		if (setlo_part(disp) == 0)
			lodisp = 0;
		else
			lodisp = setlo_part(disp) | 0x1c00;
	}
		
	return lodisp;
}

#ifndef NDEBUG
bool check_13bit_imm(s8 imm)
{
	s4 sign = (imm >> 12) & 0x1;

	if (sign == 0) {
		if ((imm & ~0xfff) == 0) return true; /* pos imm. */
	}
	else
		if ((imm & ~0xfff) + 0xfff == -1) return true; /* neg imm. */
	
	printf("immediate out-of-bounds: %ld\n", imm);
	return false;
}
#endif
	

/* codegen_emit ****************************************************************

   Generates machine code.

*******************************************************************************/

bool codegen_emit(jitdata *jd)
{

	s4 savedregs_num;
	s4 framesize_disp;

#if 0 /* no leaf optimization yet */
	savedregs_num = (code_is_leafmethod(code)) ? 0 : 1;       /* space to save the RA */
#endif
	savedregs_num = WINSAVE_CNT + ABIPARAMS_CNT; /* register-window save area */ 


#if defined(ENABLE_THREADS)        /* space to save argument of monitor_enter */
	if (checksync && code_is_synchronized(code))
		cd->stackframesize++;
#endif

	/* keep stack 16-byte aligned (ABI requirement) */

	if (cd->stackframesize & 1)
		cd->stackframesize++;











/**
 * Generates machine code for the method prolog.
 */
void codegen_emit_prolog(jitdata* jd)
{
	varinfo*    var;
	methoddesc* md;
	int32_t     s1;
	int32_t     p, t, l;
	int32_t     varindex;
	int         i;

	// Get required compiler data.
	methodinfo*   m    = jd->m;
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;

	/* save register window and create stack frame (if necessary) */

	if (cd->stackframesize) {
		if (cd->stackframesize <= 4095)
			M_SAVE(REG_SP, -cd->stackframesize * 8, REG_SP);
		else {
			M_ILD_INTERN(REG_ITMP3, REG_PV_CALLER, framesize_disp);
			M_SUB(REG_ZERO, REG_ITMP3, REG_ITMP3);
			M_SAVE_REG(REG_SP, REG_ITMP3, REG_SP);
		}
	}

	/* save callee saved float registers (none right now) */
#if 0
	p = cd->stackframesize;
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DST(rd->savfltregs[i], REG_SP, USESTACK + (p * 8));
	}
#endif

	/* take arguments out of register or stack frame */
	
	md = m->parseddesc;

 	for (p = 0, l = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;

  		varindex = jd->local_map[l * 5 + t];

 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter for 2 word types */
 			l++;

		if (varindex == UNUSED)
 			continue;

 		var = VAR(varindex);
 		s1 = md->params[p].regoff;
		
		if (IS_INT_LNG_TYPE(t)) {                    /* integer args          */			

			s2 = var->vv.regoff;
			
 			if (!md->params[p].inmemory) {           /* register arguments    */
				s1 = REG_WINDOW_TRANSPOSE(s1);
				
 				if (!(var->flags & INMEMORY)) {      /* reg arg -> register   */

					/* the register allocator does not know about the window. */
					/* avoid copying the locals from save to save regs by     */
					/* swapping variables.                                    */

					{
					int old_dest = var->vv.regoff;
					int new_dest = p + 24;

					/* run through all variables */

					for (i = 0; i < jd->varcount; i++) {
						varinfo* uvar = VAR(i);

						if (IS_FLT_DBL_TYPE(uvar->type) || IS_INMEMORY(uvar->flags))
							continue;

						s2 = uvar->vv.regoff;

						/* free the in reg by moving all other references */

						if (s2 == new_dest) {
							uvar->vv.regoff = old_dest;
							/*printf("p%d-var[%d]: moved %d -> %d (to free save reg)\n", p, i, s2, old_dest);*/
						}

						/* move all variables to the in reg */

						if (s2 == old_dest) {
							uvar->vv.regoff = new_dest;
							/*printf("p%d-var[%d]: moved %d -> %d (to avoid copy)\n", p, i, s2, new_dest);*/
						}
					}
					}



				} 
				else {                             /* reg arg -> spilled    */
					M_STX(s1, REG_SP, JITSTACK + var->vv.regoff);
 				}

			} else {                                 /* stack arguments       */
 				if (!(var->flags & INMEMORY)) {      /* stack arg -> register */
 					M_LDX(var->vv.regoff, REG_FP, JITSTACK + s1);

 				} else {                             /* stack arg -> spilled  */
					/* add the callers window save registers */
					var->vv.regoff = cd->stackframesize * 8 + s1;
				}
			}
		
		} else {                                     /* floating args         */
 			if (!md->params[p].inmemory) {           /* register arguments    */
 				if (!(var->flags & INMEMORY)) {      /* reg arg -> register   */
 					emit_fmove(cd, s1, var->vv.regoff);

 				} else {			                 /* reg arg -> spilled    */
 					M_DST(s1, REG_SP, JITSTACK + var->vv.regoff);
 				}

			} else {                                 /* stack arguments       */
 				if (!(var->flags & INMEMORY)) {      /* stack-arg -> register */
 					M_DLD(var->vv.regoff, REG_FP, JITSTACK + s1);

 				} else {                             /* stack-arg -> spilled  */
					var->vv.regoff = cd->stackframesize * 8 + s1;
				}
			}
		}
	} /* end for */
}


/**
 * Generates machine code for the method epilog.
 */
void codegen_emit_epilog(jitdata* jd)
{
	M_RETURN(REG_RA_CALLEE, 8); /* implicit window restore */
	M_NOP;
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
	int32_t             disp;

	// Get required compiler data.
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;

	switch (iptr->opc) {

		/* constant operations ************************************************/

		case ICMD_LCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			LCONST(d, iptr->sx.val.l);
			emit_store_dst(jd, iptr, d);
			break;	

		case ICMD_FCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			disp = dseg_add_float(cd, iptr->sx.val.f);
			M_FLD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_DCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			disp = dseg_add_double(cd, iptr->sx.val.d);
			M_DLD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ACONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				constant_classref *cr = iptr->sx.val.c.ref;
				disp = dseg_add_unique_address(cd, cr);

				codegen_add_patch_ref(cd, PATCHER_aconst, cr, disp);

				M_ALD(d, REG_PV, disp);

			} 
			else {
				if (iptr->sx.val.anyptr == NULL) {
					M_INTMOVE(REG_ZERO, d);
				} 
				else {
					disp = dseg_add_address(cd, iptr->sx.val.anyptr);
					M_ALD(d, REG_PV, disp);
				}
			}
			emit_store_dst(jd, iptr, d);
			break;


		/* integer operations *************************************************/

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */
		case ICMD_LNEG:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SUB(REG_ZERO, s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA_IMM(s1, 0, d); /* sign extend upper 32 bits */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLLX_IMM(s1, 56, d);
			M_SRAX_IMM( d, 56, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2CHAR:   /* ..., value  ==> ..., value                   */
		
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLLX_IMM(s1, 48, d);
			M_SRLX_IMM( d, 48, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_INT2SHORT:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLLX_IMM(s1, 48, d);
			M_SRAX_IMM( d, 48, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IADD:       /* ..., val1, val2  ==> ..., val1 + val2        */
		case ICMD_LADD:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IINC:
		case ICMD_IADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_ADD_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_ADD(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_ADD_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_ADD(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2        */
		case ICMD_LSUB: 

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_SUB_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_SUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_SUB_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_SUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */
		case ICMD_LMUL:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MULX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_MULX_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_MULX(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_MULX_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_MULX(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
/* XXX could also clear Y and use 32bit div */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_ISEXT(s1, s1);
			/* XXX trim s2 like s1 ? */
			M_DIVX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_DIVX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, s2);
			M_ISEXT(s1, s1);
			/* XXX trim s2 like s1 ? */
			M_DIVX(s1, s2, REG_ITMP3);
			M_MULX(s2, REG_ITMP3, REG_ITMP3);
			M_SUB(s1, REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, s2);
			M_DIVX(s1, s2, REG_ITMP3);
			M_MULX(s2, REG_ITMP3, REG_ITMP3);
			M_SUB(s1, REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value << constant       */
		case ICMD_LDIVPOW2:   /* val.i = constant                             */
		                      
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRAX_IMM(s1, 63, REG_ITMP2);
			M_SRLX_IMM(REG_ITMP2, 64 - iptr->sx.val.i, REG_ITMP2);
			M_ADD(s1, REG_ITMP2, REG_ITMP2);
			M_SRAX_IMM(REG_ITMP2, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLLX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLLX_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRAX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRAX_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRLX(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRLX_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */
		case ICMD_LAND:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_AND(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant           */
		                      /* sx.val.i = constant                             */
		                      /* constant is actually constant - 1               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}
			M_ISEXT(s1, s1); /* trim for 32-bit compare (BGEZ) */
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 4095)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
				M_BGEZ(s1, 5);
				M_NOP;
				M_SUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_AND(s1, REG_ITMP2, d);
				M_BGEZ(s1, 5);
				M_NOP;
				M_SUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP2, d);
			}
			M_SUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_AND_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_AND(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_AND_IMM(s1, iptr->sx.val.l, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_SUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_AND(s1, REG_ITMP2, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_SUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP2, d);
			}
			M_SUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */
		case ICMD_LOR:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_OR(s1,s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_OR_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_OR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_OR_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_OR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */
		case ICMD_LXOR:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_XOR(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -4096) && (iptr->sx.val.i <= 4095)) {
				M_XOR_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_XOR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
				M_XOR_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_XOR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_LCMP:       /* ..., val1, val2  ==> ..., val1 cmp val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			M_MOV(REG_ZERO, d);
			M_XCMOVLT_IMM(-1, d);
			M_XCMOVGT_IMM(1, d);
			emit_store_dst(jd, iptr, d);
			break;


		/* floating operations ************************************************/

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FNEG(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_DNEG(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_DADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_DSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 *** val2      */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_DMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_DDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;	

		case ICMD_I2F:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_float(cd, 0.0);
			M_IST (s1, REG_PV_CALLEE, disp);
			M_FLD (d, REG_PV_CALLEE, disp);
			M_CVTIF (d, d); /* rd gets translated to double target register */
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_I2D:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_float(cd, 0.0);
			M_IST(s1, REG_PV_CALLEE, disp);
			M_FLD(REG_FTMP2, REG_PV_CALLEE, disp); /* REG_FTMP2 needs to be a double temp */
			M_CVTID (REG_FTMP2, d); /* rd gets translated to double target register */
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_L2F:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_double(cd, 0.0);
			M_STX(s1, REG_PV_CALLEE, disp);
			M_DLD(REG_FTMP3, REG_PV_CALLEE, disp);
			M_CVTLF(REG_FTMP3, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_L2D:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_double(cd, 0.0);
			M_STX(s1, REG_PV_CALLEE, disp);
			M_DLD(d, REG_PV_CALLEE, disp);
			M_CVTLD(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_float(cd, 0.0);
			
			/* check for NaN, SPARC overflow is noncompliant (see V9 spec B.5)  */
			M_FCMP(s1, s1);
			M_FBU(5);
			M_MOV(REG_ZERO, d); /* delay slot */
			
			M_CVTFI(s1, REG_FTMP2);
			M_FST(REG_FTMP2, REG_PV_CALLEE, disp);
			M_ILD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;
			
			       
		case ICMD_D2I:       /* ..., value  ==> ..., (int) value             */
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_float(cd, 0.0);
			
			/* check for NaN, SPARC overflow is noncompliant (see V9 spec B.5)  */
			M_DCMP(s1, s1);
			M_FBU(5);
			M_MOV(REG_ZERO, d); /* delay slot */
			
			M_CVTDI(s1, REG_FTMP2);
			M_FST(REG_FTMP2, REG_PV, disp);
			M_ILD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2L:       /* ..., value  ==> ..., (long) value             */
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_double(cd, 0.0);
			
			/* check for NaN, SPARC overflow is noncompliant (see V9 spec B.5)  */
			M_FCMP(s1, s1);
			M_FBU(5);
			M_MOV(REG_ZERO, d); /* delay slot */
			
			M_CVTFL(s1, REG_FTMP2); /* FTMP2 needs to be double reg */
			M_DST(REG_FTMP2, REG_PV, disp);
			M_LDX(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_D2L:       /* ..., value  ==> ..., (long) value             */
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_double(cd, 0.0);
			
			/* check for NaN, SPARC overflow is noncompliant (see V9 spec B.5)  */
			M_DCMP(s1, s1);
			M_FBU(5);
			M_MOV(REG_ZERO, d); /* delay slot */
			
			M_CVTDL(s1, REG_FTMP2); /* FTMP2 needs to be double reg */
			M_DST(REG_FTMP2, REG_PV, disp);
			M_LDX(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_CVTFD(s1, d);
			emit_store_dst(jd, iptr, d);
			break;
					
		case ICMD_D2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_CVTDF(s1, d);
			emit_store_dst(jd, iptr, d);
			break;
	
	/* XXX merge F/D versions? only compare instr. is different */
		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_FCMP(s1,s2);
			M_OR_IMM(REG_ZERO, -1, d); /* less by default (less or unordered) */
			M_CMOVFEQ_IMM(0, d); /* 0 if equal */
			M_CMOVFGT_IMM(1, d); /* 1 if greater */
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_DCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_DCMP(s1,s2);
			M_OR_IMM(REG_ZERO, -1, d); /* less by default (less or unordered) */
			M_CMOVFEQ_IMM(0, d); /* 0 if equal */
			M_CMOVFGT_IMM(1, d); /* 1 if greater */
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);	
			M_FCMP(s1,s2);
			M_OR_IMM(REG_ZERO, 1, d); /* greater by default (greater or unordered) */
			M_CMOVFEQ_IMM(0, d); /* 0 if equal */
			M_CMOVFLT_IMM(-1, d); /* -1 if less */
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_DCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);	
			M_DCMP(s1,s2);
			M_OR_IMM(REG_ZERO, 1, d); /* greater by default (greater or unordered) */
			M_CMOVFEQ_IMM(0, d); /* 0 if equal */
			M_CMOVFLT_IMM(-1, d); /* -1 if less */
			emit_store_dst(jd, iptr, d);
			break;
			

		/* memory operations **************************************************/

		case ICMD_ARRAYLENGTH: /* ..., arrayref  ==> ..., length              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_nullpointer_check(cd, iptr, s1);
			M_ILD(d, s1, OFFSET(java_array_t, size));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			M_BLDS(d, REG_ITMP3, OFFSET(java_bytearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			M_AADD(s2, REG_ITMP3, REG_ITMP3);
			M_SLDU(d, REG_ITMP3, OFFSET(java_chararray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			M_AADD(s2, REG_ITMP3, REG_ITMP3);
			M_SLDS(d, REG_ITMP3, OFFSET(java_shortarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			M_ILD(d, REG_ITMP3, OFFSET(java_intarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			M_LDX(d, REG_ITMP3, OFFSET(java_longarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			M_FLD(d, REG_ITMP3, OFFSET(java_floatarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			M_DLD(d, REG_ITMP3, OFFSET(java_doublearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, POINTERSHIFT, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			M_ALD(d, REG_ITMP3, OFFSET(java_objectarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

	
		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_BST(s3, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */
		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			M_AADD(s2, REG_ITMP1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SST(s3, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_IST_INTERN(s3, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_STX_INTERN(s3, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			M_FST_INTERN(s3, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			M_DST_INTERN(s3, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
			break;


		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);

			M_MOV(s1, REG_OUT0);
			M_MOV(s3, REG_OUT1);
			disp = dseg_add_functionptr(cd, BUILTIN_FAST_canstore);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
			M_NOP;
			emit_arraystore_check(cd, iptr);

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_ASLL_IMM(s2, POINTERSHIFT, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			M_AST_INTERN(s3, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;


		case ICMD_BASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			M_BST(REG_ZERO, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			break;

		case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...            */
		case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			M_AADD(s2, REG_ITMP1, REG_ITMP1);
			M_SST(REG_ZERO, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			break;

		case ICMD_IASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			M_IST_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			M_STX_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_AASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, POINTERSHIFT, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			M_AST_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;
		

		case ICMD_GETSTATIC:  /* ...  ==> ..., value                          */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = dseg_add_unique_address(cd, uf);

				codegen_add_patch_ref(cd, PATCHER_get_putstatic, uf, disp);
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz))
					codegen_add_patch_ref(cd, PATCHER_clinit, fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD_INTERN(d, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_LDX_INTERN(d, REG_ITMP1, 0);
				break;
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ALD_INTERN(d, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_FLD_INTERN(d, REG_ITMP1, 0);
				break;
			case TYPE_DBL:				
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_DLD_INTERN(d, REG_ITMP1, 0);
				break;
			default:
				assert(false);
				break;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTSTATIC:  /* ..., value  ==> ...                          */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = dseg_add_unique_address(cd, uf);

				codegen_add_patch_ref(cd, PATCHER_get_putstatic, uf, disp);
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz))
					codegen_add_patch_ref(cd, PATCHER_clinit, fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_IST_INTERN(s1, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_STX_INTERN(s1, REG_ITMP1, 0);
				break;
			case TYPE_ADR:
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_AST_INTERN(s1, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				s1 = emit_load_s1(jd, iptr, REG_FTMP2);
				M_FST_INTERN(s1, REG_ITMP1, 0);
				break;
			case TYPE_DBL:
				s1 = emit_load_s1(jd, iptr, REG_FTMP2);
				M_DST_INTERN(s1, REG_ITMP1, 0);
				break;
			default:
				assert(false);
				break;
			}
			break;

		case ICMD_PUTSTATICCONST: /* ...  ==> ...                             */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = dseg_add_unique_address(cd, uf);

				codegen_add_patch_ref(cd, PATCHER_get_putstatic, uf, disp);
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz))
					codegen_add_patch_ref(cd, PATCHER_clinit, fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				M_IST_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				M_STX_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_ADR:
				M_AST_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				M_FST_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_DBL:
				M_DST_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			default:
				assert(false);
				break;
			}
			break;


		case ICMD_GETFIELD:   /* ...  ==> ..., value                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf = iptr->sx.s23.s3.uf;

				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				codegen_add_patch_ref(cd, PATCHER_get_putfield, uf, 0);
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD(d, s1, disp);
				break;
			case TYPE_LNG:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_LDX(d, s1, disp);
				break;
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ALD(d, s1, disp);
				break;
			case TYPE_FLT:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_FLD(d, s1, disp);
				break;
			case TYPE_DBL:				
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_DLD(d, s1, disp);
				break;
			default:
				assert(false);
				break;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTFIELD:   /* ..., objectref, value  ==> ...               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;
			}
			else {
				uf        = NULL;
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
				}

			if (IS_INT_LNG_TYPE(fieldtype))
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			else
				s2 = emit_load_s2(jd, iptr, REG_FTMP2);

			if (INSTRUCTION_IS_UNRESOLVED(iptr))
				codegen_add_patch_ref(cd, PATCHER_get_putfield, uf, 0);

			switch (fieldtype) {
			case TYPE_INT:
				M_IST(s2, s1, disp);
				break;
			case TYPE_LNG:
				M_STX(s2, s1, disp);
				break;
			case TYPE_ADR:
				M_AST(s2, s1, disp);
				break;
			case TYPE_FLT:
				M_FST(s2, s1, disp);
				break;
			case TYPE_DBL:
				M_DST(s2, s1, disp);
				break;
			default:
				assert(false);
				break;
			}
			break;

		case ICMD_PUTFIELDCONST:  /* ..., objectref  ==> ...                  */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				unresolved_field *uf = iptr->sx.s23.s3.uf;

				fieldtype = uf->fieldref->parseddesc.fd->type;

				codegen_addpatchref(cd, PATCHER_get_putfield,
									uf, 0);

				if (opt_showdisassemble) {
					M_NOP; M_NOP;
				}

				disp = 0;

			} else {
			{
				fieldinfo *fi = iptr->sx.s23.s3.fmiref->p.field;

				fieldtype = fi->type;
				disp = fi->offset;
			}

			}

			switch (fieldtype) {
			case TYPE_INT:
				M_IST(REG_ZERO, s1, disp);
				break;
			case TYPE_LNG:
				M_STX(REG_ZERO, s1, disp);
				break;
			case TYPE_ADR:
				M_AST(REG_ZERO, s1, disp);
				break;
			case TYPE_FLT:
				M_FST(REG_ZERO, s1, disp);
				break;
			case TYPE_DBL:
				M_DST(REG_ZERO, s1, disp);
				break;
			default:
				assert(false);
				break;
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			disp = dseg_add_functionptr(cd, asm_handle_exception);
			M_ALD(REG_ITMP1, REG_PV, disp);
			M_JMP(REG_ITMP3_XPC, REG_ITMP1, REG_ZERO);
			M_NOP;
			M_NOP;              /* nop ensures that XPC is less than the end */
			                    /* of basic block                            */
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_beqz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_beq_xcc(cd, iptr->dst.block);
			}
			break;
			
		case ICMD_IF_LLT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bltz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				} 
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_blt_xcc(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LLE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_blez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_ble_xcc(cd, iptr->dst.block);
			}
			break;
			
		case ICMD_IF_LNE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bnez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_bne_xcc(cd, iptr->dst.block);
			}
			break;
						
		case ICMD_IF_LGT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgtz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				} 
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_bgt_xcc(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LGE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -4096) && (iptr->sx.val.l <= 4095)) {
					M_CMP_IMM(s1, iptr->sx.val.l);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMP(s1, REG_ITMP2);
				}
				emit_bge_xcc(cd, iptr->dst.block);
			}
			break;			
			

		case ICMD_IF_ACMPEQ:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPEQ:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_beq_xcc(cd, iptr->dst.block);
			break;

		case ICMD_IF_ACMPNE:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPNE:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bne_xcc(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_blt_xcc(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bgt_xcc(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_ble_xcc(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bge_xcc(cd, iptr->dst.block);
			break;

		case ICMD_TABLESWITCH:  /* ..., index ==> ...                         */
			{
			s4 i, l;
			branch_target_t *table;

			table = iptr->dst.table;

			l = iptr->sx.s23.s2.tablelow;
			i = iptr->sx.s23.s3.tablehigh;
			
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (l == 0) {
				M_INTMOVE(s1, REG_ITMP1);
			}
			else if (-l >= 4096 && -l <= 4095) {
				M_ADD_IMM(s1, -l, REG_ITMP1);
			}
			else {
				ICONST(REG_ITMP2, l);
				M_SUB(s1, REG_ITMP2, REG_ITMP1);
			}

			i = i - l + 1; /* number of targets (>0) */


			/* range check */
					
			if (i <= 4095) {
				M_CMP_IMM(REG_ITMP1, i - 1);
			}
			else {
				ICONST(REG_ITMP2, i - 1);
				M_CMP(REG_ITMP1, REG_ITMP2);
			}		
			emit_bugt(cd, table[0].block); /* default target */

			/* build jump table top down and use address of lowest entry */

			table += i;

			while (--i >= 0) {
				dseg_add_target(cd, table->block); 
				--table;
				}
			}

			/* length of dataseg after last dseg_addtarget is used by load */

			M_ASLL_IMM(REG_ITMP1, POINTERSHIFT, REG_ITMP1);
			M_AADD(REG_ITMP1, REG_PV, REG_ITMP2);
			M_ALD(REG_ITMP2, REG_ITMP2, -(cd->dseglen));
			M_JMP(REG_ZERO, REG_ITMP2, REG_ZERO);
			M_NOP;
			ALIGNCODENOP;
			break;

		// XXX This is the old builtin invocation containing some
		// special argument handling code, port me!
		case ICMD_BUILTIN:      /* ..., arg1, arg2, arg3 ==> ...              */

			bte = iptr->sx.s23.s3.bte;
			md = bte->md;
			
			/* XXX: builtin calling with stack arguments not implemented */
			assert(md->paramcount <= 5 && md->argfltreguse <= 16);
			
			s3 = md->paramcount;

			MCODECHECK((s3 << 1) + 64);

#ifdef BUILTIN_FLOAT_ARGS /* float args for builtins disabled */

			/* copy float arguments according to ABI convention */

			int num_fltregargs = 0;
			int fltregarg_inswap[16];

			for (s3 = s3 - 1; s3 >= 0; s3--) {
				var = VAR(iptr->sx.s23.s2.args[s3]);

				if (IS_FLT_DBL_TYPE(var->type)) {
					if (!md->params[s3].inmemory) {
						s1 = s3; /*native flt args use argument index directly*/
						d = emit_load(jd, iptr, var, REG_FTMP1);
						
						M_DMOV(d, s1 + 16);
						fltregarg_inswap[num_fltregargs] = s1;
						num_fltregargs++;
						/*printf("builtin: flt arg swap to %d\n", s1 + 16);*/
					}
					else {
						assert(0);
					}
				}
			}
			
			int i;
			/* move swapped float args to target regs */
			for (i = 0; i < num_fltregargs; i++) {
				s1 = fltregarg_inswap[i];
				M_DMOV(s1 + 16, s1);
				/*printf("builtin float arg to target reg: %d ==> %d\n", s1+16, s1);*/
			}
			
#else
			assert(md->argfltreguse == 0);
#endif




XXXXXX
				if (IS_INT_LNG_TYPE(var->type)) {
					if (!md->params[s3].inmemory) {
						s1 = emit_load(jd, iptr, var, d);
						M_INTMOVE(s1, d);
					} 
					else {
						s1 = emit_load(jd, iptr, var, REG_ITMP1);
						M_STX(s1, REG_SP, JITSTACK + d);
					}
				}
			}
XXXXXX

		case ICMD_BUILTIN:
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL) {
				disp = dseg_add_functionptr(cd, bte->fp);
			}
			else {
				disp = dseg_add_functionptr(cd, bte->stub);
			}

			M_ALD(REG_PV_CALLER, REG_PV, disp);  /* built-in-function pointer */

			/* XXX jit-c-call */
			/* generate the actual call */
   
		    M_JMP(REG_RA_CALLER, REG_PV_CALLER, REG_ZERO);
		    M_NOP;

			if (md->returntype.type == TYPE_FLT) {
				/* special handling for float return value in %f0 */
				M_FMOV_INTERN(0,1);
			}
			break;

		case ICMD_INVOKESPECIAL:
			emit_nullpointer_check(cd, iptr, REG_OUT0);
			/* fall-through */

		case ICMD_INVOKESTATIC:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, NULL);

				codegen_add_patch_ref(cd, PATCHER_invokestatic_special,
									um, disp);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_address(cd, lm->stubroutine);
			}

			M_ALD(REG_PV_CALLER, REG_PV, disp);          /* method pointer in pv */
			
			/* generate the actual call */
   
		    M_JMP(REG_RA_CALLER, REG_PV_CALLER, REG_ZERO);
		    M_NOP;
			break;

		case ICMD_INVOKEVIRTUAL:
			emit_nullpointer_check(cd, iptr, REG_OUT0);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				codegen_add_patch_ref(cd, PATCHER_invokevirtual, um, 0);

				s1 = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, table[0]) +
					sizeof(methodptr) * lm->vftblindex;
			}

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_OUT0,OFFSET(java_object_t, vftbl));
			M_ALD(REG_PV_CALLER, REG_METHODPTR, s1);
			
			/* generate the actual call */
   
		    M_JMP(REG_RA_CALLER, REG_PV_CALLER, REG_ZERO);
		    M_NOP;
			break;

		case ICMD_INVOKEINTERFACE:
			emit_nullpointer_check(cd, iptr, REG_OUT0);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				codegen_add_patch_ref(cd, PATCHER_invokeinterface, um, 0);

				s1 = 0;
				s2 = 0;
			} 
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, interfacetable[0]) -
					sizeof(methodptr*) * lm->clazz->index;

				s2 = sizeof(methodptr) * (lm - lm->clazz->methods);
			}

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_OUT0, OFFSET(java_object_t, vftbl));
			M_ALD(REG_METHODPTR, REG_METHODPTR, s1);
			M_ALD(REG_PV_CALLER, REG_METHODPTR, s2);

		    /* generate the actual call */
   
		    M_JMP(REG_RA_CALLER, REG_PV_CALLER, REG_ZERO);
		    M_NOP;
			break;

XXXXXX
			/* store return value */
			else {
				s1 = codegen_reg_of_dst(jd, iptr, REG_FRESULT);
				if (IS_2_WORD_TYPE(d)) {
					emit_dmove(cd, REG_FRESULT, s1);
				} else {
					emit_fmove(cd, REG_FRESULT, s1);
				}
			}
XXXXX

		case ICMD_CHECKCAST:  /* ..., objectref ==> ..., objectref            */
		                      /* val.a: (classinfo*) superclass               */

			/*  superclass is an interface:
			 *
			 *  OK if ((sub == NULL) ||
			 *         (sub->vftbl->interfacetablelength > super->index) &&
			 *         (sub->vftbl->interfacetable[-super->index] != NULL));
			 *
			 *  superclass is a class:
			 *
			 *  OK if ((sub == NULL) || (0
			 *         <= (sub->vftbl->baseval - super->vftbl->baseval) <=
			 *         super->vftbl->diffvall));
			 */

			if (!(iptr->flags.bits & INS_FLAG_ARRAY)) {
				classinfo *super;
				s4         superindex;

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					super      = NULL;
					superindex = 0;
				}
				else {
					super = iptr->sx.s23.s3.c.cls;
					superindex = super->index;
				}

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);

				/* if class is not resolved, check which code to call */

				if (super == NULL) {
					emit_label_beqz(cd, BRANCH_LABEL_1, s1);

					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_s4(cd, 0);         /* super->flags */

					codegen_add_patch_ref(cd, PATCHER_checkcast_instanceof_flags,
										  cr, disp);

					M_ILD(REG_ITMP2, REG_PV, disp);
					M_AND_IMM(REG_ITMP2, ACC_INTERFACE, REG_ITMP2);
					emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP2);
				}

				/* interface checkcast code */

				if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						constant_classref *cr = iptr->sx.s23.s3.c.ref;

						codegen_add_patch_ref(cd, PATCHER_checkcast_interface,
											  cr, 0);
					}
					else {
						emit_label_beqz(cd, BRANCH_LABEL_3, s1);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ILD(REG_ITMP3, REG_ITMP2,
 							OFFSET(vftbl_t, interfacetablelength));
					M_ADD_IMM(REG_ITMP3, -superindex, REG_ITMP3);
					emit_classcast_check(cd, iptr, ICMD_IFLE, REG_ITMP3, s1);

					M_ALD(REG_ITMP3, REG_ITMP2,
						  OFFSET(vftbl_t, interfacetable[0]) -
						  superindex * sizeof(methodptr*));
					emit_classcast_check(cd, iptr, ICMD_IFEQ, REG_ITMP3, s1);

					if (super == NULL)
						emit_label_br(cd, BRANCH_LABEL_4);
					else
						emit_label(cd, BRANCH_LABEL_3);
				}

				/* class checkcast code */

				if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						emit_label(cd, BRANCH_LABEL_2);

						constant_classref *cr = iptr->sx.s23.s3.c.ref;
						disp = dseg_add_unique_address(cd, NULL);

						codegen_add_patch_ref(cd,
											PATCHER_checkcast_instanceof_class,
											  cr, disp);
					}
					else {
						disp = dseg_add_address(cd, super->vftbl);

						emit_label_beqz(cd, BRANCH_LABEL_5, s1);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ALD(REG_ITMP3, REG_PV, disp);
					
					M_ILD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, baseval));
					M_ILD(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, baseval));
					M_SUB(REG_ITMP2, REG_ITMP3, REG_ITMP2);
					M_ALD(REG_ITMP3, REG_PV, disp);
					M_ILD(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, diffval));

					/*  				} */
					M_CMP(REG_ITMP3, REG_ITMP2);
					emit_classcast_check(cd, iptr, BRANCH_ULT, REG_ITMP3, s1);

					if (super != NULL)
						emit_label(cd, BRANCH_LABEL_5);
				}

				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_1);
					emit_label(cd, BRANCH_LABEL_4);
				}

				d = codegen_reg_of_dst(jd, iptr, s1);
			}
			else {
				/* array type cast-check */

				s1 = emit_load_s1(jd, iptr, REG_OUT0);
				M_INTMOVE(s1, REG_OUT0);

				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_address(cd, NULL);

					codegen_add_patch_ref(cd, PATCHER_builtin_arraycheckcast,
										  cr, disp);
				}
				else
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

				M_ALD(REG_OUT1, REG_PV, disp);
				disp = dseg_add_functionptr(cd, BUILTIN_arraycheckcast);
				M_ALD(REG_ITMP3, REG_PV, disp);
				/* XXX jit-c-call */
				M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
				M_NOP;

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				emit_classcast_check(cd, iptr, ICMD_IFEQ, REG_RESULT_CALLER, s1);

				d = codegen_reg_of_dst(jd, iptr, s1);
			}

			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INSTANCEOF: /* ..., objectref ==> ..., intresult            */
		                      /* val.a: (classinfo*) superclass               */

			/*  superclass is an interface:
			 *	
			 *  return (sub != NULL) &&
			 *         (sub->vftbl->interfacetablelength > super->index) &&
			 *         (sub->vftbl->interfacetable[-super->index] != NULL);
			 *	
			 *  superclass is a class:
			 *	
			 *  return ((sub != NULL) && (0
			 *          <= (sub->vftbl->baseval - super->vftbl->baseval) <=
			 *          super->vftbl->diffvall));
			 */

			{
			classinfo *super;
			vftbl_t   *supervftbl;
			s4         superindex;

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				super = NULL;
				superindex = 0;
				supervftbl = NULL;

			} else {
				super = iptr->sx.s23.s3.c.cls;
				superindex = super->index;
				supervftbl = super->vftbl;
			}

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			M_CLR(d);

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				emit_label_beqz(cd, BRANCH_LABEL_1, s1);

				constant_classref *cr = iptr->sx.s23.s3.c.ref;
				disp = dseg_add_unique_s4(cd, 0);             /* super->flags */

				codegen_add_patch_ref(cd, PATCHER_checkcast_instanceof_flags,
									  cr, disp);

				M_ILD(REG_ITMP3, REG_PV, disp);
				M_AND_IMM(REG_ITMP3, ACC_INTERFACE, REG_ITMP3);
				emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP3);
			}

			/* interface instanceof code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					constant_classref *cr = iptr->sx.s23.s3.c.ref;

					codegen_add_patch_ref(cd, PATCHER_instanceof_interface,
										  cr, 0);
				}
				else {
					emit_label_beqz(cd, BRANCH_LABEL_3, s1);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ILD(REG_ITMP3, REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));
				M_CMP_IMM(REG_ITMP3, superindex);
				M_BLE(4);
				M_NOP;
				M_ALD(REG_ITMP1, REG_ITMP1,
					  (s4) (OFFSET(vftbl_t, interfacetable[0]) -
							superindex * sizeof(methodptr*)));
				M_CMOVRNE_IMM(REG_ITMP1, 1, d);      /* REG_ITMP1 != 0  */

				if (super == NULL)
					emit_label_br(cd, BRANCH_LABEL_4);
				else
					emit_label(cd, BRANCH_LABEL_3);
			}

			/* class instanceof code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);

					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_address(cd, NULL);

					codegen_add_patch_ref(cd, PATCHER_checkcast_instanceof_class,
										  cr, disp);
				}
				else {
					disp = dseg_add_address(cd, supervftbl);

					emit_label_beqz(cd, BRANCH_LABEL_5, s1);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ALD(REG_ITMP2, REG_PV, disp);

				M_ILD(REG_ITMP1, REG_ITMP1, OFFSET(vftbl_t, baseval));
				M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, baseval));
				M_ILD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, diffval));

				M_SUB(REG_ITMP1, REG_ITMP3, REG_ITMP1);
				M_CMP(REG_ITMP1, REG_ITMP2);
				M_XCMOVULE_IMM(1, d);

				if (super != NULL)
					emit_label(cd, BRANCH_LABEL_5);
			}

			if (super == NULL) {
				emit_label(cd, BRANCH_LABEL_1);
				emit_label(cd, BRANCH_LABEL_4);
			}

			emit_store_dst(jd, iptr, d);
			}
			break;

		case ICMD_MULTIANEWARRAY:/* ..., cnt1, [cnt2, ...] ==> ..., arrayref  */

			/* check for negative sizes and copy sizes to stack if necessary  */

			MCODECHECK((iptr->s1.argcount << 1) + 64);

			for (s1 = iptr->s1.argcount; --s1 >= 0; ) {

				var = VAR(iptr->sx.s23.s2.args[s1]);
	
				/* copy SAVEDVAR sizes to stack */

				/* Already Preallocated? */

				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
					M_STX(s2, REG_SP, CSTACK + (s1 * 8));
				}
			}

			/* arg 0 = dimension count */

			ICONST(REG_OUT0, iptr->s1.argcount);

			/* is patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				disp = dseg_add_unique_address(cd, 0);

				codegen_add_patch_ref(cd, PATCHER_builtin_multianewarray,
									  iptr->sx.s23.s3.c.ref,
									  disp);
			}
			else
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

			/* arg 1 = arraydescriptor */

			M_ALD(REG_OUT1, REG_PV, disp);

			/* arg 2 = pointer to dimensions = stack pointer (absolute) */

			M_ADD_IMM(REG_SP, CSTACK, REG_OUT2);

			/* XXX c abi call */
			disp = dseg_add_functionptr(cd, BUILTIN_multianewarray);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
			M_NOP;

			/* check for exception before result assignment */

			emit_exception_check(cd, iptr);

			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_CALLER);
			M_INTMOVE(REG_RESULT_CALLER, d);
			emit_store_dst(jd, iptr, d);
			break;

		default:
			vm_abort("Unknown ICMD %d during code generation", iptr->opc);
	} /* switch */
}


/* codegen_emit_stub_compiler **************************************************

   Emits a stub routine which calls the compiler.
	
*******************************************************************************/

void codegen_emit_stub_compiler(jitdata *jd)
{
	methodinfo  *m;
	codegendata *cd;

	/* get required compiler data */

	m  = jd->m;
	cd = jd->cd;

	/* code for the stub */

	/* no window save yet, user caller's PV */
	M_ALD_INTERN(REG_ITMP1, REG_PV_CALLER, -2 * SIZEOF_VOID_P);  /* codeinfo pointer */
	M_ALD_INTERN(REG_PV_CALLER, REG_PV_CALLER, -3 * SIZEOF_VOID_P);  /* pointer to compiler */
	M_JMP(REG_ZERO, REG_PV_CALLER, REG_ZERO);  /* jump to the compiler, RA is wasted */
	M_NOP;
}


/* codegen_emit_stub_builtin ***************************************************

   Creates a stub routine which calls a builtin function.

*******************************************************************************/

void codegen_emit_stub_builtin(jitdata *jd, builtintable_entry *bte)
{
	codeinfo    *code;
	codegendata *cd;
	methoddesc  *md;
	s4           i;
	s4           disp;
	s4           s1, s2;

	/* get required compiler data */
	code = jd->code;
	cd   = jd->cd;

	/* set some variables */
	md = bte->md;

	/* calculate stack frame size */
	cd->stackframesize =
		WINSAVE_CNT +
		ABIPARAMS_CNT +
		FLT_ARG_CNT +
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +
		4;                              /* 4 arguments or return value        */


	/* keep stack 16-byte aligned (ABI requirement) */

	if (cd->stackframesize & 1)
		cd->stackframesize++;

	/* create method header */
	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 4); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate stub code */
	M_SAVE(REG_SP, -cd->stackframesize * 8, REG_SP); /* build up stackframe   */

#if defined(ENABLE_GC_CACAO)
	/* Save callee saved integer registers in stackframeinfo (GC may
	   need to recover them during a collection). */

	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs) + BIAS;

	for (i = 0; i < INT_SAV_CNT; i++)
		M_AST(abi_registers_integer_saved[i], REG_SP, disp + i * 8);
#endif

	for (i = 0; i < md->paramcount; i++) {
		s1 = md->params[i].regoff;

		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
			break;
		case TYPE_FLT:
		case TYPE_DBL:
			M_DST(s1, REG_SP, JITSTACK + i * 8);
			break;
		default:
			assert(false);
			break;
		}
	}

	/* create dynamic stack info */

	M_AADD_IMM(REG_SP, BIAS + cd->stackframesize * 8, REG_OUT0); /* data sp*/
	M_MOV(REG_PV_CALLEE, REG_OUT1); /* PV */
	M_MOV(REG_FP, REG_OUT2); /* java sp */
	M_MOV(REG_RA_CALLEE, REG_OUT3); /* ra */

	disp = dseg_add_functionptr(cd, codegen_stub_builtin_enter);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_NOP; /* XXX fill me! */


	/* builtins are allowed to have 5 arguments max */

	assert(md->paramcount <= 5);

	/* copy arguments into position */

	for (i = 0; i < md->paramcount; i++) {
		assert(!md->params[i].inmemory);

		s1 = md->params[i].regoff;

		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
			M_MOV(REG_WINDOW_TRANSPOSE(abi_registers_integer_argument[i]), s1);
			break;
		case TYPE_FLT:
		case TYPE_DBL:
			M_DLD(s1, REG_SP, JITSTACK + i * 8);
			break;
		default:
			assert(false);
			break;
		}

	}

	/* call the builtin function */

	disp = dseg_add_functionptr(cd, bte->fp);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp); /* load adress of builtin */
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO); /* call builtin                */
	M_NOP;                              /* delay slot                         */


	/* save return value */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_INT_LNG_TYPE(md->returntype.type))
			M_MOV(REG_RESULT_CALLER, REG_RESULT_CALLEE);
		else
			M_DST(REG_FRESULT, REG_SP, CSTACK);
	}	


	/* remove native stackframe info */

	M_ADD_IMM(REG_FP, BIAS, REG_OUT0); /* datasp, like above */
	disp = dseg_add_functionptr(cd, codegen_stub_builtin_exit);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_NOP;

	/* restore float return value, int return value already in our return reg */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_FLT_DBL_TYPE(md->returntype.type)) {
			if (IS_2_WORD_TYPE(md->returntype.type))
				M_DLD(REG_FRESULT, REG_SP, CSTACK);
			else
				M_FLD(REG_FRESULT, REG_SP, CSTACK);
		}
	}


#if defined(ENABLE_GC_CACAO)
	/* Restore callee saved integer registers from stackframeinfo (GC
	   might have modified them during a collection). */
        
	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs) + BIAS;

	for (i = 0; i < INT_SAV_CNT; i++)
		M_ALD(abi_registers_integer_saved[i], REG_SP, disp + i * 8);
#endif

	/* return */
	M_RETURN(REG_RA_CALLEE, 8); /* implicit window restore */
	M_NOP;

	/* assert(0); */
}


/* codegen_emit_stub_native ****************************************************

   Emits a stub routine which calls a native method.

*******************************************************************************/

void codegen_emit_stub_native(jitdata *jd, methoddesc *nmd, functionptr f)
{
	methodinfo   *m;
	codeinfo     *code;
	codegendata  *cd;
	methoddesc   *md;
	s4            nativeparams;
	s4            i, j;                 /* count variables                    */
	s4            t;
	s4            s1, s2, disp;
	s4            funcdisp;             /* displacement of the function       */
	s4            fltregarg_offset[FLT_ARG_CNT];

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	/* initialize variables */

	md = m->parseddesc;
	nativeparams = (m->flags & ACC_STATIC) ? 2 : 1;

	/* calculate stack frame size */

	cd->stackframesize =
		sizeof(stackframeinfo) / SIZEOF_VOID_P +
		sizeof(localref_table) / SIZEOF_VOID_P +
		md->paramcount +                /* for saving arguments over calls    */
		nmd->memuse +              /* nmd->memuse includes the (6) abi params */
		WINSAVE_CNT;


	/* keep stack 16-byte aligned (ABI requirement) */

	if (cd->stackframesize & 1)
		cd->stackframesize++;

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate stub code */

	M_SAVE(REG_SP, -cd->stackframesize * 8, REG_SP); /* build up stackframe    */

#if !defined(NDEBUG)
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd))
		emit_verbosecall_enter(jd);
#endif

	/* get function address (this must happen before the stackframeinfo) */

	funcdisp = dseg_add_functionptr(cd, f);

	if (f == NULL)
		codegen_add_patch_ref(cd, PATCHER_resolve_native, m, funcdisp);

	/* save float argument registers */

	assert(ABIPARAMS_CNT >= FLT_ARG_CNT);

	for (i = 0, j = 0; i < md->paramcount && i < FLT_ARG_CNT; i++) {
		if (IS_FLT_DBL_TYPE(md->paramtypes[i].type)) {
			s1 = WINSAVE_CNT + nmd->memuse + j;
			M_DST(abi_registers_float_argument[i], REG_SP, BIAS + (s1*8));
			fltregarg_offset[i] = s1; /* remember stack offset */
			j++;
		}
	}

	/* prepare data structures for native function call */

	M_ADD_IMM(REG_FP, BIAS, REG_OUT0); /* datasp == top of the stack frame (absolute, ie. + BIAS) */
	M_MOV(REG_PV_CALLEE, REG_OUT1);
	M_MOV(REG_FP, REG_OUT2); /* java sp */
	M_MOV(REG_RA_CALLEE, REG_OUT3);
	disp = dseg_add_functionptr(cd, codegen_start_native_call);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_NOP; /* XXX fill me! */

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_RESULT_CALLER, REG_ITMP3);

	/* keep float arguments on stack */
#if 0
	for (i = 0, j = 0; i < md->paramcount && i < FLT_ARG_CNT; i++) {
		if (IS_FLT_DBL_TYPE(md->paramtypes[i].type)) {
			M_DLD(abi_registers_float_argument[i], REG_SP, CSTACK + (j * 8));
			j++;
		}
	}
#endif

	/* copy or spill arguments to new locations */

	for (i = md->paramcount - 1, j = i + nativeparams; i >= 0; i--, j--) {
		t = md->paramtypes[i].type;

		if (IS_INT_LNG_TYPE(t)) {

			/* integral types */

			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;
				/* s1 refers to the old window, transpose */
				s1 = REG_WINDOW_TRANSPOSE(s1);

				if (!nmd->params[j].inmemory) {
					s2 = nmd->params[j].regoff;
					M_INTMOVE(s1, s2);
				} else {
					/* nmd's regoff is relative to the start of the param array */
					s2 = BIAS + WINSAVE_CNT * 8 + nmd->params[j].regoff;
					M_AST(s1, REG_SP, s2);
				}

			} else {
				if (!nmd->params[j].inmemory) {
					/* JIT stack arg -> NAT reg arg */

					/* Due to the Env pointer that is always passed, the 6th JIT arg   */
					/* is the 7th (or 8th w/ class ptr) NAT arg, and goes to the stack */

					assert(false); /* path never taken */
				}

				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = BIAS + WINSAVE_CNT * 8 + nmd->params[j].regoff;
				M_ALD(REG_ITMP1, REG_SP, CSTACK + s1);
				M_AST(REG_ITMP1, REG_SP, s2);
			}

		} else {

			/* floating point types */

			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;

				if (!nmd->params[j].inmemory) {

					/* no mapping to regs needed, native flt args use regoff */
					s2 = nmd->params[j].regoff;

					/* JIT float regs are still on the stack */
					M_DLD(s2, REG_SP, BIAS + (fltregarg_offset[i] * 8));
				} 
				else {
					/* not supposed to happen with 16 NAT flt args */
					assert(false); 
					/*
					s2 = nmd->params[j].regoff;
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, CSTACK + (s2 * 8));
					else
						M_FST(s1, REG_SP, CSTACK + (s2 * 8));
					*/
				}

			} 
			else {
				s1 = md->params[i].regoff;

				if (!nmd->params[j].inmemory) {

					/* JIT stack -> NAT reg */

					s2 = nmd->params[j].regoff;
					M_DLD(s2, REG_FP, JITSTACK + s1);
				}
				else {

					/* JIT stack -> NAT stack */

					s2 = WINSAVE_CNT * 8 + nmd->params[j].regoff;

					/* The FTMP register may already be loaded with args */
					/* we know $f0 is unused because of the env pointer  */
					M_DLD(REG_F0, REG_FP, JITSTACK + s1);
					M_DST(REG_F0, REG_SP, BIAS + s2);
				}
			}
		}
	}
	

	/* put class into second argument register */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_ITMP3, REG_OUT1);

	/* put env into first argument register */

	disp = dseg_add_address(cd, VM::get_current()->get_jnienv());
	M_ALD(REG_OUT0, REG_PV_CALLEE, disp);

	/* do the native function call */

	M_ALD(REG_ITMP3, REG_PV_CALLEE, funcdisp); /* load adress of native method       */
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO); /* call native method                 */
	M_NOP;                              /* delay slot                         */

	/* save return value */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_INT_LNG_TYPE(md->returntype.type))
			M_MOV(REG_RESULT_CALLER, REG_RESULT_CALLEE);
		else
			M_DST(REG_FRESULT, REG_SP, CSTACK);
	}
	
	/* Note: native functions return float values in %f0 (see ABI) */
	/* we handle this by doing M_FLD below. (which will load the lower word into %f1) */

#if !defined(NDEBUG)
	/* But for the trace function we need to put a flt result into %f1 */
	if (JITDATA_HAS_FLAG_VERBOSECALL(jd)) {
		if (!IS_2_WORD_TYPE(md->returntype.type))
			M_FLD(REG_FRESULT, REG_SP, CSTACK);
		emit_verbosecall_exit(jd);
	}
#endif

	/* remove native stackframe info */

	M_ADD_IMM(REG_FP, BIAS, REG_OUT0); /* datasp, like above */
	disp = dseg_add_functionptr(cd, codegen_finish_native_call);
	M_ALD(REG_ITMP3, REG_PV_CALLEE, disp);
	M_JMP(REG_RA_CALLER, REG_ITMP3, REG_ZERO);
	M_NOP; /* XXX fill me! */
	M_MOV(REG_RESULT_CALLER, REG_ITMP2_XPTR);

	/* restore float return value, int return value already in our return reg */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_FLT_DBL_TYPE(md->returntype.type)) {
			if (IS_2_WORD_TYPE(md->returntype.type))
				M_DLD(REG_FRESULT, REG_SP, CSTACK);
			else
				M_FLD(REG_FRESULT, REG_SP, CSTACK);
		}
	}

	/* check for exception */
	M_BNEZ(REG_ITMP2_XPTR, 4);          /* if no exception then return        */
	M_NOP;

	M_RETURN(REG_RA_CALLEE, 8); /* implicit window restore */
	M_NOP;

	/* handle exception */
	
	disp = dseg_add_functionptr(cd, asm_handle_nat_exception);
	M_ALD(REG_ITMP1, REG_PV, disp);     /* load asm exception handler address */
	M_MOV(REG_RA_CALLEE, REG_ITMP3_XPC); /* get exception address             */
	M_JMP(REG_ZERO, REG_ITMP1, REG_ZERO);/* jump to asm exception handler     */
	M_RESTORE(REG_ZERO, 0, REG_ZERO);   /* restore callers window (DELAY)     */
	
	/* generate patcher stubs */

	emit_patcher_stubs(jd);
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
