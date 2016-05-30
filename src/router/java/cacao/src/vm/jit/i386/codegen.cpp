/* src/vm/jit/i386/codegen.cpp - machine code generator for i386

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
#include <stdio.h>

#include "vm/types.hpp"
#include "vm/os.hpp"

#include "vm/jit/i386/md-abi.hpp"

#include "vm/jit/i386/codegen.hpp"
#include "vm/jit/i386/emit.hpp"

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
#include "vm/primitive.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/codegen-common.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/emit-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/linenumbertable.hpp"
#include "vm/jit/parse.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trap.hpp"

using namespace cacao;


/**
 * Generates machine code for the method prolog.
 */
void codegen_emit_prolog(jitdata* jd)
{
	varinfo*    var;
	methoddesc* md;
	int32_t     s1, d;
	int32_t     p, t, l;
	int32_t     varindex;
	int         i;
	int         align_off;

	// Get required compiler data.
	methodinfo*   m  = jd->m;
	codegendata*  cd = jd->cd;
	registerdata* rd = jd->rd;

	/* create stack frame (if necessary) */

	align_off = cd->stackframesize ? 4 : 0;

	if (cd->stackframesize) {
		assert(align_off == 4);
		M_ASUB_IMM(cd->stackframesize * 8 + 4, REG_SP);
	}

	/* save return address and used callee saved registers */

  	p = cd->stackframesize;
	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
 		p--; M_AST(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; emit_fld_reg(cd, rd->savfltregs[i]); emit_fstpl_membase(cd, REG_SP, p * 8);
	}

	/* take arguments out of register or stack frame */

	md = m->parseddesc;

	for (p = 0, l = 0; p < md->paramcount; p++) {
		t = md->paramtypes[p].type;

		varindex = jd->local_map[l * 5 + t];
#if defined(ENABLE_SSA)
		if ( ls != NULL ) {
			if (varindex != jitdata::UNUSED)
				varindex = ls->var_0[varindex];
			if ((varindex != jitdata::UNUSED) && (ls->lifetime[varindex].type == jitdata::UNUSED))
				varindex = jitdata::UNUSED;
		}
#endif
 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter for 2 word types */
 			l++;

		if (varindex == jitdata::UNUSED)
			continue;

		var = VAR(varindex);
		s1  = md->params[p].regoff;
		d   = var->vv.regoff;

		if (IS_INT_LNG_TYPE(t)) {                    /* integer args          */
			if (!md->params[p].inmemory) {           /* register arguments    */
				log_text("integer register argument");
				assert(0);
				if (!(var->flags & INMEMORY)) {      /* reg arg -> register   */
					/* rd->argintregs[md->params[p].regoff -> var->vv.regoff     */
				} 
				else {                               /* reg arg -> spilled    */
					/* rd->argintregs[md->params[p].regoff -> var->vv.regoff * 4 */
				}
			} 
			else {
				if (!(var->flags & INMEMORY)) {
					M_ILD(d, REG_SP,
						  cd->stackframesize * 8 + 4 + align_off + s1);
				} 
				else {
					if (!IS_2_WORD_TYPE(t)) {
#if defined(ENABLE_SSA)
						/* no copy avoiding by now possible with SSA */
						if (ls != NULL) {
							emit_mov_membase_reg(   /* + 4 for return address */
								cd, REG_SP,
								cd->stackframesize * 8 + s1 + 4 + align_off,
								REG_ITMP1);    
							emit_mov_reg_membase(
								cd, REG_ITMP1, REG_SP, var->vv.regoff);
						}
						else 
#endif /*defined(ENABLE_SSA)*/
							/* reuse stackslot */
							var->vv.regoff = cd->stackframesize * 8 + 4 +
								align_off + s1;

					} 
					else {
#if defined(ENABLE_SSA)
						/* no copy avoiding by now possible with SSA */
						if (ls != NULL) {
							emit_mov_membase_reg(  /* + 4 for return address */
								cd, REG_SP,
								cd->stackframesize * 8 + s1 + 4 + align_off,
								REG_ITMP1);
							emit_mov_reg_membase(
								cd, REG_ITMP1, REG_SP, var->vv.regoff);
							emit_mov_membase_reg(   /* + 4 for return address */
								cd, REG_SP,
  								cd->stackframesize * 8 + s1 + 4 + 4 + align_off,
  								REG_ITMP1);             
							emit_mov_reg_membase(
								cd, REG_ITMP1, REG_SP, var->vv.regoff + 4);
						}
						else
#endif /*defined(ENABLE_SSA)*/
							/* reuse stackslot */
							var->vv.regoff = cd->stackframesize * 8 + 8 + s1;
					}
				}
			}
		}
		else {                                       /* floating args         */
			if (!md->params[p].inmemory) {           /* register arguments    */
				log_text("There are no float argument registers!");
				assert(0);
				if (!(var->flags & INMEMORY)) {  /* reg arg -> register   */
					/* rd->argfltregs[md->params[p].regoff -> var->vv.regoff     */
				} else {			             /* reg arg -> spilled    */
					/* rd->argfltregs[md->params[p].regoff -> var->vv.regoff * 8 */
				}

			} 
			else {                                   /* stack arguments       */
				if (!(var->flags & INMEMORY)) {      /* stack-arg -> register */
					if (t == TYPE_FLT) {
						emit_flds_membase(
                            cd, REG_SP,
							cd->stackframesize * 8 + s1 + 4 + align_off);
						assert(0);
/* 						emit_fstp_reg(cd, var->vv.regoff + fpu_st_offset); */

					} 
					else {
						emit_fldl_membase(
                            cd, REG_SP,
							cd->stackframesize * 8 + s1 + 4 + align_off);
						assert(0);
/* 						emit_fstp_reg(cd, var->vv.regoff + fpu_st_offset); */
					}

				} else {                             /* stack-arg -> spilled  */
#if defined(ENABLE_SSA)
					/* no copy avoiding by now possible with SSA */
					if (ls != NULL) {
						emit_mov_membase_reg(
							cd, REG_SP,
							cd->stackframesize * 8 + s1 + 4 + align_off,
							REG_ITMP1);
						emit_mov_reg_membase(
							cd, REG_ITMP1, REG_SP, var->vv.regoff);
						if (t == TYPE_FLT) {
							emit_flds_membase(
								cd, REG_SP,
								cd->stackframesize * 8 + s1 + 4 + align_off);
							emit_fstps_membase(cd, REG_SP, var->vv.regoff);
						} 
						else {
							emit_fldl_membase(
								cd, REG_SP,
								cd->stackframesize * 8 + s1 + 4 + align_off);
							emit_fstpl_membase(cd, REG_SP, var->vv.regoff);
						}
					}
					else
#endif /*defined(ENABLE_SSA)*/
						/* reuse stackslot */
						var->vv.regoff = cd->stackframesize * 8 + 4 +
							align_off + s1;
				}
			}
		}
	}
}


/**
 * Generates machine code for the method epilog.
 */
void codegen_emit_epilog(jitdata* jd)
{
	methoddesc* md;
	int32_t p;
	int i;

	// Get required compiler data.
	methodinfo*   m  = jd->m;
	codegendata*  cd = jd->cd;
	registerdata* rd = jd->rd;

	p = cd->stackframesize;
	md = m->parseddesc;

	/* restore saved registers */

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_ALD(rd->savintregs[i], REG_SP, p * 8);
	}

	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--;
		emit_fldl_membase(cd, REG_SP, p * 8);
		if (md->returntype.type == TYPE_FLT || md->returntype.type == TYPE_DBL) {
			assert(0);
/* 			emit_fstp_reg(cd, rd->savfltregs[i] + fpu_st_offset + 1); */
		} else {
			assert(0);
/* 			emit_fstp_reg(cd, rd->savfltregs[i] + fpu_st_offset); */
		}
	}

	/* deallocate stack */

	if (cd->stackframesize)
		M_AADD_IMM(cd->stackframesize * 8 + 4, REG_SP);

	M_RET;
}


/**
 * Generates machine code for one ICMD.
 */
void codegen_emit_instruction(jitdata* jd, instruction* iptr)
{
	varinfo*            var;
	varinfo*            var1;
	builtintable_entry* bte;
	methodinfo*         lm;             // Local methodinfo for ICMD_INVOKE*.
	unresolved_method*  um;
	fieldinfo*          fi;
	unresolved_field*   uf;
	int32_t             fieldtype;
	int32_t             s1 = 0;         // silence compiler warning
	int32_t             s2, s3;
	int32_t             d = 0;          // silence compiler warning
	int32_t             disp;

	// Get required compiler data.
	codegendata* cd = jd->cd;

	switch (iptr->opc) {

		/* constant operations ************************************************/

		case ICMD_FCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			if (iptr->sx.val.f == 0.0) {
				emit_fldz(cd);

				/* -0.0 */
				if ((uint32_t)iptr->sx.val.i == 0x80000000) {
					emit_fchs(cd);
				}

			} else if (iptr->sx.val.f == 1.0) {
				emit_fld1(cd);

			} else if (iptr->sx.val.f == 2.0) {
				emit_fld1(cd);
				emit_fld1(cd);
				emit_faddp(cd);

			} else {
  				disp = dseg_add_float(cd, iptr->sx.val.f);
				emit_mov_imm_reg(cd, 0, REG_ITMP1);
				dseg_adddata(cd);
				emit_flds_membase(cd, REG_ITMP1, disp);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			if (iptr->sx.val.d == 0.0) {
				emit_fldz(cd);

				/* -0.0 */
				if ((uint64_t)iptr->sx.val.l == 0x8000000000000000LL) {
					emit_fchs(cd);
				}

			} else if (iptr->sx.val.d == 1.0) {
				emit_fld1(cd);

			} else if (iptr->sx.val.d == 2.0) {
				emit_fld1(cd);
				emit_fld1(cd);
				emit_faddp(cd);

			} else {
				disp = dseg_add_double(cd, iptr->sx.val.d);
				emit_mov_imm_reg(cd, 0, REG_ITMP1);
				dseg_adddata(cd);
				emit_fldl_membase(cd, REG_ITMP1, disp);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ACONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				patcher_add_patch_ref(jd, PATCHER_aconst,
									iptr->sx.val.c.ref, 0);

				M_MOV_IMM2(NULL, d);

			} else {
				if (iptr->sx.val.anyptr == NULL)
					M_CLR(d);
				else
					M_MOV_IMM(iptr->sx.val.anyptr, d);
			}
			emit_store_dst(jd, iptr, d);
			break;


		/* integer operations *************************************************/

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_NEG(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_NEG(GET_LOW_REG(d));
			M_IADDC_IMM(0, GET_HIGH_REG(d));
			M_NEG(GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, EAX);
			d = codegen_reg_of_dst(jd, iptr, EAX_EDX_PACKED);
			M_INTMOVE(s1, EAX);
			M_CLTD;
			M_LNGMOVE(EAX_EDX_PACKED, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_SLL_IMM(24, d);
			M_SRA_IMM(24, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2CHAR:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_CZEXT(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2SHORT:  /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SSEXT(s1, d);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_IADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_IADD(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_IADD(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IINC:
		case ICMD_IADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
				
			/* `inc reg' is slower on p4's (regarding to ia32
			   optimization reference manual and benchmarks) and as
			   fast on athlon's. */

			M_INTMOVE(s1, d);
			M_IADD_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_INTMOVE(s1, GET_LOW_REG(d));
			M_IADD(s2, GET_LOW_REG(d));
			/* don't use REG_ITMP1 */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, GET_HIGH_REG(d));
			M_IADDC(s2, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_IADD_IMM(iptr->sx.val.l, GET_LOW_REG(d));
			M_IADDC_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			if (s2 == d) {
				M_INTMOVE(s1, REG_ITMP1);
				M_ISUB(s2, REG_ITMP1);
				M_INTMOVE(REG_ITMP1, d);
			}
			else {
				M_INTMOVE(s1, d);
				M_ISUB(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_ISUB_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (s2 == GET_LOW_REG(d)) {
				M_INTMOVE(s1, REG_ITMP1);
				M_ISUB(s2, REG_ITMP1);
				M_INTMOVE(REG_ITMP1, GET_LOW_REG(d));
			}
			else {
				M_INTMOVE(s1, GET_LOW_REG(d));
				M_ISUB(s2, GET_LOW_REG(d));
			}
			/* don't use REG_ITMP1 */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			if ((uint32_t)s2 == GET_HIGH_REG(d)) {
				M_INTMOVE(s1, REG_ITMP2);
				M_ISUBB(s2, REG_ITMP2);
				M_INTMOVE(REG_ITMP2, GET_HIGH_REG(d));
			}
			else {
				M_INTMOVE(s1, GET_HIGH_REG(d));
				M_ISUBB(s2, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_ISUB_IMM(iptr->sx.val.l, GET_LOW_REG(d));
			M_ISUBB_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_IMUL(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_IMUL(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_IMUL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_low(jd, iptr, EDX);
			d = codegen_reg_of_dst(jd, iptr, EAX_EDX_PACKED);

			M_INTMOVE(s1, REG_ITMP2);
			M_IMUL(s2, REG_ITMP2);

			s1 = emit_load_s1_low(jd, iptr, EAX);
			s2 = emit_load_s2_high(jd, iptr, EDX);
			M_INTMOVE(s2, EDX);
			M_IMUL(s1, EDX);
			M_IADD(EDX, REG_ITMP2);

			s1 = emit_load_s1_low(jd, iptr, EAX);
			s2 = emit_load_s2_low(jd, iptr, EDX);
			M_INTMOVE(s1, EAX);
			M_MUL(s2);
			M_INTMOVE(EAX, GET_LOW_REG(d));
			M_IADD(REG_ITMP2, GET_HIGH_REG(d));

			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, EAX_EDX_PACKED);
			ICONST(EAX, iptr->sx.val.l);
			M_MUL(s1);
			M_IMUL_IMM(s1, iptr->sx.val.l >> 32, REG_ITMP2);
			M_IADD(REG_ITMP2, EDX);
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			M_IMUL_IMM(s1, iptr->sx.val.l, REG_ITMP2);
			M_IADD(REG_ITMP2, EDX);
			M_LNGMOVE(EAX_EDX_PACKED, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, EAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, EAX);
			emit_arithmetic_check(cd, iptr, s2);

			M_INTMOVE(s1, EAX);           /* we need the first operand in EAX */

			/* check as described in jvm spec */

			M_CMP_IMM(0x80000000, EAX);
			M_BNE(3 + 6);
			M_CMP_IMM(-1, s2);
			M_BEQ(1 + 2);
  			M_CLTD;
			M_IDIV(s2);

			M_INTMOVE(EAX, d);           /* if INMEMORY then d is already EAX */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, EAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, EDX);
			emit_arithmetic_check(cd, iptr, s2);

			M_INTMOVE(s1, EAX);           /* we need the first operand in EAX */

			/* check as described in jvm spec */

			M_CMP_IMM(0x80000000, EAX);
			M_BNE(2 + 3 + 6);
			M_CLR(EDX);
			M_CMP_IMM(-1, s2);
			M_BEQ(1 + 2);
  			M_CLTD;
			M_IDIV(s2);

			M_INTMOVE(EDX, d);           /* if INMEMORY then d is already EDX */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                          */

			/* TODO: optimize for `/ 2' */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_TEST(d);
			M_BNS(6);
			M_IADD_IMM32((1 << iptr->sx.val.i) - 1, d);/* 32-bit for jump off */
			M_SRA_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			} 
			M_INTMOVE(s1, d);
			M_AND_IMM(iptr->sx.val.i, d);
			M_TEST(s1);
			M_BGE(2 + 2 + 6 + 2);
			M_MOV(s1, d);  /* don't use M_INTMOVE, so we know the jump offset */
			M_NEG(d);
			M_AND_IMM32(iptr->sx.val.i, d);     /* use 32-bit for jump offset */
			M_NEG(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s2 = emit_load_s2(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_PACKED);

			M_INTMOVE(GET_LOW_REG(s2), REG_ITMP3);
			M_OR(GET_HIGH_REG(s2), REG_ITMP3);
			/* XXX could be optimized */
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			bte = iptr->sx.s23.s3.bte;

			M_LST(s2, REG_SP, 2 * 4);

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			M_LST(s1, REG_SP, 0 * 4);

			M_MOV_IMM(bte->fp, REG_ITMP3);
			M_CALL(REG_ITMP3);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIVPOW2:   /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_PACKED);
			M_LNGMOVE(s1, d);
			M_TEST(GET_HIGH_REG(d));
			M_BNS(6 + 3);
			M_IADD_IMM32((1 << iptr->sx.val.i) - 1, GET_LOW_REG(d));
			M_IADDC_IMM(0, GET_HIGH_REG(d));
			M_SRLD_IMM(iptr->sx.val.i, GET_HIGH_REG(d), GET_LOW_REG(d));
			M_SRA_IMM(iptr->sx.val.i, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

#if 0
		case ICMD_LREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.l = constant                          */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			if (iptr->dst.var->flags & INMEMORY) {
				if (iptr->s1.var->flags & INMEMORY) {
					/* Alpha algorithm */
					disp = 3;
					CALCOFFSETBYTES(disp, REG_SP, iptr->s1.var->vv.regoff * 8);
					disp += 3;
					CALCOFFSETBYTES(disp, REG_SP, iptr->s1.var->vv.regoff * 8 + 4);

					disp += 2;
					disp += 3;
					disp += 2;

					/* TODO: hmm, don't know if this is always correct */
					disp += 2;
					CALCIMMEDIATEBYTES(disp, iptr->sx.val.l & 0x00000000ffffffff);
					disp += 2;
					CALCIMMEDIATEBYTES(disp, iptr->sx.val.l >> 32);

					disp += 2;
					disp += 3;
					disp += 2;

					emit_mov_membase_reg(cd, REG_SP, iptr->s1.var->vv.regoff * 8, REG_ITMP1);
					emit_mov_membase_reg(cd, REG_SP, iptr->s1.var->vv.regoff * 8 + 4, REG_ITMP2);
					
					emit_alu_imm_reg(cd, ALU_AND, iptr->sx.val.l, REG_ITMP1);
					emit_alu_imm_reg(cd, ALU_AND, iptr->sx.val.l >> 32, REG_ITMP2);
					emit_alu_imm_membase(cd, ALU_CMP, 0, REG_SP, iptr->s1.var->vv.regoff * 8 + 4);
					emit_jcc(cd, CC_GE, disp);

					emit_mov_membase_reg(cd, REG_SP, iptr->s1.var->vv.regoff * 8, REG_ITMP1);
					emit_mov_membase_reg(cd, REG_SP, iptr->s1.var->vv.regoff * 8 + 4, REG_ITMP2);
					
					emit_neg_reg(cd, REG_ITMP1);
					emit_alu_imm_reg(cd, ALU_ADC, 0, REG_ITMP2);
					emit_neg_reg(cd, REG_ITMP2);
					
					emit_alu_imm_reg(cd, ALU_AND, iptr->sx.val.l, REG_ITMP1);
					emit_alu_imm_reg(cd, ALU_AND, iptr->sx.val.l >> 32, REG_ITMP2);
					
					emit_neg_reg(cd, REG_ITMP1);
					emit_alu_imm_reg(cd, ALU_ADC, 0, REG_ITMP2);
					emit_neg_reg(cd, REG_ITMP2);

					emit_mov_reg_membase(cd, REG_ITMP1, REG_SP, iptr->dst.var->vv.regoff * 8);
					emit_mov_reg_membase(cd, REG_ITMP2, REG_SP, iptr->dst.var->vv.regoff * 8 + 4);
				}
			}

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_PACKED);
			M_LNGMOVE(s1, d);
			M_AND_IMM(iptr->sx.val.l, GET_LOW_REG(d));	
			M_AND_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			M_TEST(GET_LOW_REG(s1));
			M_BGE(0);
			M_LNGMOVE(s1, d);
		break;
#endif

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s2, ECX);                       /* s2 may be equal to d */
			M_INTMOVE(s1, d);
			M_SLL(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_SLL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s2, ECX);                       /* s2 may be equal to d */
			M_INTMOVE(s1, d);
			M_SRA(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_SRA_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s2, ECX);                       /* s2 may be equal to d */
			M_INTMOVE(s1, d);
			M_SRL(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_SRL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP13_PACKED);
			s2 = emit_load_s2(jd, iptr, ECX);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP13_PACKED);
			M_LNGMOVE(s1, d);
			M_INTMOVE(s2, ECX);
			M_TEST_IMM(32, ECX);
			M_BEQ(2 + 2);
			M_MOV(GET_LOW_REG(d), GET_HIGH_REG(d));
			M_CLR(GET_LOW_REG(d));
			M_SLLD(GET_LOW_REG(d), GET_HIGH_REG(d));
			M_SLL(GET_LOW_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

        case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant       */
 			                  /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			if (iptr->sx.val.i & 0x20) {
				M_MOV(GET_LOW_REG(d), GET_HIGH_REG(d));
				M_CLR(GET_LOW_REG(d));
				M_SLLD_IMM(iptr->sx.val.i & 0x3f, GET_LOW_REG(d), 
						   GET_HIGH_REG(d));
			}
			else {
				M_SLLD_IMM(iptr->sx.val.i & 0x3f, GET_LOW_REG(d),
						   GET_HIGH_REG(d));
				M_SLL_IMM(iptr->sx.val.i & 0x3f, GET_LOW_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP13_PACKED);
			s2 = emit_load_s2(jd, iptr, ECX);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP13_PACKED);
			M_LNGMOVE(s1, d);
			M_INTMOVE(s2, ECX);
			M_TEST_IMM(32, ECX);
			M_BEQ(2 + 3);
			M_MOV(GET_HIGH_REG(d), GET_LOW_REG(d));
			M_SRA_IMM(31, GET_HIGH_REG(d));
			M_SRLD(GET_HIGH_REG(d), GET_LOW_REG(d));
			M_SRA(GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			if (iptr->sx.val.i & 0x20) {
				M_MOV(GET_HIGH_REG(d), GET_LOW_REG(d));
				M_SRA_IMM(31, GET_HIGH_REG(d));
				M_SRLD_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d), 
						   GET_LOW_REG(d));
			}
			else {
				M_SRLD_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d), 
						   GET_LOW_REG(d));
				M_SRA_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP13_PACKED);
			s2 = emit_load_s2(jd, iptr, ECX);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP13_PACKED);
			M_LNGMOVE(s1, d);
			M_INTMOVE(s2, ECX);
			M_TEST_IMM(32, ECX);
			M_BEQ(2 + 2);
			M_MOV(GET_HIGH_REG(d), GET_LOW_REG(d));
			M_CLR(GET_HIGH_REG(d));
			M_SRLD(GET_HIGH_REG(d), GET_LOW_REG(d));
			M_SRL(GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

  		case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
  		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			if (iptr->sx.val.i & 0x20) {
				M_MOV(GET_HIGH_REG(d), GET_LOW_REG(d));
				M_CLR(GET_HIGH_REG(d));
				M_SRLD_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d), 
						   GET_LOW_REG(d));
			}
			else {
				M_SRLD_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d), 
						   GET_LOW_REG(d));
				M_SRL_IMM(iptr->sx.val.i & 0x3f, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
  			break;

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_AND(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_AND(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_AND_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (s2 == GET_LOW_REG(d))
				M_AND(s1, GET_LOW_REG(d));
			else {
				M_INTMOVE(s1, GET_LOW_REG(d));
				M_AND(s2, GET_LOW_REG(d));
			}
			/* REG_ITMP1 probably contains low 32-bit of destination */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			if ((uint32_t)s2 == GET_HIGH_REG(d))
				M_AND(s1, GET_HIGH_REG(d));
			else {
				M_INTMOVE(s1, GET_HIGH_REG(d));
				M_AND(s2, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_AND_IMM(iptr->sx.val.l, GET_LOW_REG(d));
			M_AND_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_OR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_OR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_OR_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (s2 == GET_LOW_REG(d))
				M_OR(s1, GET_LOW_REG(d));
			else {
				M_INTMOVE(s1, GET_LOW_REG(d));
				M_OR(s2, GET_LOW_REG(d));
			}
			/* REG_ITMP1 probably contains low 32-bit of destination */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			if ((uint32_t)s2 == GET_HIGH_REG(d))
				M_OR(s1, GET_HIGH_REG(d));
			else {
				M_INTMOVE(s1, GET_HIGH_REG(d));
				M_OR(s2, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_OR_IMM(iptr->sx.val.l, GET_LOW_REG(d));
			M_OR_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_XOR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_XOR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_XOR_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if (s2 == GET_LOW_REG(d))
				M_XOR(s1, GET_LOW_REG(d));
			else {
				M_INTMOVE(s1, GET_LOW_REG(d));
				M_XOR(s2, GET_LOW_REG(d));
			}
			/* REG_ITMP1 probably contains low 32-bit of destination */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			if ((uint32_t)s2 == GET_HIGH_REG(d))
				M_XOR(s1, GET_HIGH_REG(d));
			else {
				M_INTMOVE(s1, GET_HIGH_REG(d));
				M_XOR(s2, GET_HIGH_REG(d));
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_LNGMOVE(s1, d);
			M_XOR_IMM(iptr->sx.val.l, GET_LOW_REG(d));
			M_XOR_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(d));
			emit_store_dst(jd, iptr, d);
			break;


		/* floating operations ************************************************/

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			emit_fchs(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			emit_fchs(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_faddp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_faddp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fsubp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fsubp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fmulp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fmulp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fdivp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			emit_fdivp(cd);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			/* exchanged to skip fxch */
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
/*  			emit_fxch(cd); */
			emit_fprem(cd);
			emit_wait(cd);
			emit_fnstsw(cd);
			emit_sahf(cd);
			emit_jcc(cd, CC_P, -(2 + 1 + 2 + 1 + 6));
			emit_store_dst(jd, iptr, d);
			emit_ffree_reg(cd, 0);
			emit_fincstp(cd);
			break;

		case ICMD_DREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			/* exchanged to skip fxch */
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
/*  			emit_fxch(cd); */
			emit_fprem(cd);
			emit_wait(cd);
			emit_fnstsw(cd);
			emit_sahf(cd);
			emit_jcc(cd, CC_P, -(2 + 1 + 2 + 1 + 6));
			emit_store_dst(jd, iptr, d);
			emit_ffree_reg(cd, 0);
			emit_fincstp(cd);
			break;

		case ICMD_I2F:       /* ..., value  ==> ..., (float) value            */
		case ICMD_I2D:       /* ..., value  ==> ..., (double) value           */

			var = VAROP(iptr->s1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);

			if (var->flags & INMEMORY) {
				emit_fildl_membase(cd, REG_SP, var->vv.regoff);
			} else {
				/* XXX not thread safe! */
				disp = dseg_add_unique_s4(cd, 0);
				emit_mov_imm_reg(cd, 0, REG_ITMP1);
				dseg_adddata(cd);
				emit_mov_reg_membase(cd, var->vv.regoff, REG_ITMP1, disp);
				emit_fildl_membase(cd, REG_ITMP1, disp);
			}

  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2F:       /* ..., value  ==> ..., (float) value            */
		case ICMD_L2D:       /* ..., value  ==> ..., (double) value           */

			var = VAROP(iptr->s1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			if (var->flags & INMEMORY) {
				emit_fildll_membase(cd, REG_SP, var->vv.regoff);

			} else {
				log_text("L2F: longs have to be in memory");
				assert(0);
			}
  			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_NULL);

			emit_mov_imm_reg(cd, 0, REG_ITMP1);
			dseg_adddata(cd);

			/* Round to zero, 53-bit mode, exception masked */
			disp = dseg_add_s4(cd, 0x0e7f);
			emit_fldcw_membase(cd, REG_ITMP1, disp);

			var = VAROP(iptr->dst);
			var1 = VAROP(iptr->s1);

			if (var->flags & INMEMORY) {
				emit_fistpl_membase(cd, REG_SP, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

				emit_alu_imm_membase(cd, ALU_CMP, 0x80000000, 
									 REG_SP, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);

			} else {
				/* XXX not thread safe! */
				disp = dseg_add_unique_s4(cd, 0);
				emit_fistpl_membase(cd, REG_ITMP1, disp);
				emit_mov_membase_reg(cd, REG_ITMP1, disp, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

				emit_alu_imm_reg(cd, ALU_CMP, 0x80000000, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + ((REG_RESULT == var->vv.regoff) ? 0 : 2);
			}

			emit_jcc(cd, CC_NE, disp);

			/* XXX: change this when we use registers */
			emit_flds_membase(cd, REG_SP, var1->vv.regoff);
			emit_mov_imm_reg(cd, (ptrint) asm_builtin_f2i, REG_ITMP1);
			emit_call_reg(cd, REG_ITMP1);

			if (var->flags & INMEMORY) {
				emit_mov_reg_membase(cd, REG_RESULT, REG_SP, var->vv.regoff);

			} else {
				M_INTMOVE(REG_RESULT, var->vv.regoff);
			}
			break;

		case ICMD_D2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_NULL);

			emit_mov_imm_reg(cd, 0, REG_ITMP1);
			dseg_adddata(cd);

			/* Round to zero, 53-bit mode, exception masked */
			disp = dseg_add_s4(cd, 0x0e7f);
			emit_fldcw_membase(cd, REG_ITMP1, disp);

			var  = VAROP(iptr->dst);
			var1 = VAROP(iptr->s1);

			if (var->flags & INMEMORY) {
				emit_fistpl_membase(cd, REG_SP, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

  				emit_alu_imm_membase(cd, ALU_CMP, 0x80000000, 
									 REG_SP, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);

			} else {
				/* XXX not thread safe! */
				disp = dseg_add_unique_s4(cd, 0);
				emit_fistpl_membase(cd, REG_ITMP1, disp);
				emit_mov_membase_reg(cd, REG_ITMP1, disp, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

				emit_alu_imm_reg(cd, ALU_CMP, 0x80000000, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + ((REG_RESULT == var->vv.regoff) ? 0 : 2);
			}

			emit_jcc(cd, CC_NE, disp);

			/* XXX: change this when we use registers */
			emit_fldl_membase(cd, REG_SP, var1->vv.regoff);
			emit_mov_imm_reg(cd, (ptrint) asm_builtin_d2i, REG_ITMP1);
			emit_call_reg(cd, REG_ITMP1);

			if (var->flags & INMEMORY) {
				emit_mov_reg_membase(cd, REG_RESULT, REG_SP, var->vv.regoff);
			} else {
				M_INTMOVE(REG_RESULT, var->vv.regoff);
			}
			break;

		case ICMD_F2L:       /* ..., value  ==> ..., (long) value             */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_NULL);

			emit_mov_imm_reg(cd, 0, REG_ITMP1);
			dseg_adddata(cd);

			/* Round to zero, 53-bit mode, exception masked */
			disp = dseg_add_s4(cd, 0x0e7f);
			emit_fldcw_membase(cd, REG_ITMP1, disp);

			var  = VAROP(iptr->dst);
			var1 = VAROP(iptr->s1);

			if (var->flags & INMEMORY) {
				emit_fistpll_membase(cd, REG_SP, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

  				emit_alu_imm_membase(cd, ALU_CMP, 0x80000000, 
									 REG_SP, var->vv.regoff + 4);

				disp = 6 + 4;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2;
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff + 4);

				emit_jcc(cd, CC_NE, disp);

  				emit_alu_imm_membase(cd, ALU_CMP, 0, 
									 REG_SP, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);

				emit_jcc(cd, CC_NE, disp);

				/* XXX: change this when we use registers */
				emit_flds_membase(cd, REG_SP, var1->vv.regoff);
				emit_mov_imm_reg(cd, (ptrint) asm_builtin_f2l, REG_ITMP1);
				emit_call_reg(cd, REG_ITMP1);
				emit_mov_reg_membase(cd, REG_RESULT, REG_SP, var->vv.regoff);
				emit_mov_reg_membase(cd, REG_RESULT2, 
									 REG_SP, var->vv.regoff + 4);

			} else {
				log_text("F2L: longs have to be in memory");
				assert(0);
			}
			break;

		case ICMD_D2L:       /* ..., value  ==> ..., (long) value             */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_NULL);

			emit_mov_imm_reg(cd, 0, REG_ITMP1);
			dseg_adddata(cd);

			/* Round to zero, 53-bit mode, exception masked */
			disp = dseg_add_s4(cd, 0x0e7f);
			emit_fldcw_membase(cd, REG_ITMP1, disp);

			var  = VAROP(iptr->dst);
			var1 = VAROP(iptr->s1);

			if (var->flags & INMEMORY) {
				emit_fistpll_membase(cd, REG_SP, var->vv.regoff);

				/* Round to nearest, 53-bit mode, exceptions masked */
				disp = dseg_add_s4(cd, 0x027f);
				emit_fldcw_membase(cd, REG_ITMP1, disp);

  				emit_alu_imm_membase(cd, ALU_CMP, 0x80000000, 
									 REG_SP, var->vv.regoff + 4);

				disp = 6 + 4;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2;
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);
				disp += 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff + 4);

				emit_jcc(cd, CC_NE, disp);

  				emit_alu_imm_membase(cd, ALU_CMP, 0, REG_SP, var->vv.regoff);

				disp = 3;
				CALCOFFSETBYTES(disp, REG_SP, var1->vv.regoff);
				disp += 5 + 2 + 3;
				CALCOFFSETBYTES(disp, REG_SP, var->vv.regoff);

				emit_jcc(cd, CC_NE, disp);

				/* XXX: change this when we use registers */
				emit_fldl_membase(cd, REG_SP, var1->vv.regoff);
				emit_mov_imm_reg(cd, (ptrint) asm_builtin_d2l, REG_ITMP1);
				emit_call_reg(cd, REG_ITMP1);
				emit_mov_reg_membase(cd, REG_RESULT, REG_SP, var->vv.regoff);
				emit_mov_reg_membase(cd, REG_RESULT2, 
									 REG_SP, var->vv.regoff + 4);

			} else {
				log_text("D2L: longs have to be in memory");
				assert(0);
			}
			break;

		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			/* nothing to do */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			/* nothing to do */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */
		case ICMD_DCMPL:

			/* exchanged to skip fxch */
			s2 = emit_load_s1(jd, iptr, REG_FTMP1);
			s1 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
/*    			emit_fxch(cd); */
			emit_fucompp(cd);
			emit_fnstsw(cd);
			emit_test_imm_reg(cd, 0x400, EAX);    /* unordered treat as GT */
			emit_jcc(cd, CC_E, 6);
			emit_alu_imm_reg(cd, ALU_AND, 0x000000ff, EAX);
 			emit_sahf(cd);
			emit_mov_imm_reg(cd, 0, d);    /* does not affect flags */
  			emit_jcc(cd, CC_E, 6 + 3 + 5 + 3);
  			emit_jcc(cd, CC_B, 3 + 5);
			emit_alu_imm_reg(cd, ALU_SUB, 1, d);
			emit_jmp_imm(cd, 3);
			emit_alu_imm_reg(cd, ALU_ADD, 1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */
		case ICMD_DCMPG:

			/* exchanged to skip fxch */
			s2 = emit_load_s1(jd, iptr, REG_FTMP1);
			s1 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
/*    			emit_fxch(cd); */
			emit_fucompp(cd);
			emit_fnstsw(cd);
			emit_test_imm_reg(cd, 0x400, EAX);    /* unordered treat as LT */
			emit_jcc(cd, CC_E, 3);
			emit_movb_imm_reg(cd, 1, REG_AH);
 			emit_sahf(cd);
			emit_mov_imm_reg(cd, 0, d);    /* does not affect flags */
  			emit_jcc(cd, CC_E, 6 + 3 + 5 + 3);
  			emit_jcc(cd, CC_B, 3 + 5);
			emit_alu_imm_reg(cd, ALU_SUB, 1, d);
			emit_jmp_imm(cd, 3);
			emit_alu_imm_reg(cd, ALU_ADD, 1, d);
			emit_store_dst(jd, iptr, d);
			break;


		/* memory operations **************************************************/

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
   			emit_movsbl_memindex_reg(cd, OFFSET(java_bytearray_t, data[0]), 
									 s1, s2, 0, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movzwl_memindex_reg(cd, OFFSET(java_chararray_t, data[0]), 
									 s1, s2, 1, d);
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movswl_memindex_reg(cd, OFFSET(java_shortarray_t, data[0]), 
									 s1, s2, 1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_memindex_reg(cd, OFFSET(java_intarray_t, data[0]), 
								  s1, s2, 2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);

			var = VAROP(iptr->dst);

			assert(var->flags & INMEMORY);
			emit_mov_memindex_reg(cd, OFFSET(java_longarray_t, data[0]), 
								  s1, s2, 3, REG_ITMP3);
			emit_mov_reg_membase(cd, REG_ITMP3, REG_SP, var->vv.regoff);
			emit_mov_memindex_reg(cd, OFFSET(java_longarray_t, data[0]) + 4, 
								  s1, s2, 3, REG_ITMP3);
			emit_mov_reg_membase(cd, REG_ITMP3, REG_SP, var->vv.regoff + 4);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_flds_memindex(cd, OFFSET(java_floatarray_t, data[0]), s1, s2, 2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_fldl_memindex(cd, OFFSET(java_doublearray_t, data[0]), s1, s2,3);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_memindex_reg(cd, OFFSET(java_objectarray_t, data[0]),
								  s1, s2, 2, d);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			if (s3 >= EBP) { 
				/* because EBP, ESI, EDI have no xH and xL nibbles */
				M_INTMOVE(s3, REG_ITMP3);
				s3 = REG_ITMP3;
			}
			emit_movb_reg_memindex(cd, s3, OFFSET(java_bytearray_t, data[0]),
								   s1, s2, 0);
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movw_reg_memindex(cd, s3, OFFSET(java_chararray_t, data[0]),
								   s1, s2, 1);
			break;

		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movw_reg_memindex(cd, s3, OFFSET(java_shortarray_t, data[0]),
								   s1, s2, 1);
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_mov_reg_memindex(cd, s3, OFFSET(java_intarray_t, data[0]),
								  s1, s2, 2);
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);

			var = VAROP(iptr->sx.s23.s3);

			assert(var->flags & INMEMORY);
			emit_mov_membase_reg(cd, REG_SP, var->vv.regoff, REG_ITMP3);
			emit_mov_reg_memindex(cd, REG_ITMP3, OFFSET(java_longarray_t, data[0])
								  , s1, s2, 3);
			emit_mov_membase_reg(cd, REG_SP, var->vv.regoff + 4, REG_ITMP3);
			emit_mov_reg_memindex(cd, REG_ITMP3,
							    OFFSET(java_longarray_t, data[0]) + 4, s1, s2, 3);
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			emit_fstps_memindex(cd, OFFSET(java_floatarray_t, data[0]), s1, s2,2);
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			emit_fstpl_memindex(cd, OFFSET(java_doublearray_t, data[0]),
								s1, s2, 3);
			break;

		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);

			M_AST(s1, REG_SP, 0 * 4);
			M_AST(s3, REG_SP, 1 * 4);
			M_MOV_IMM(BUILTIN_FAST_canstore, REG_ITMP1);
			M_CALL(REG_ITMP1);
			emit_arraystore_check(cd, iptr);

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_mov_reg_memindex(cd, s3, OFFSET(java_objectarray_t, data[0]),
								  s1, s2, 2);
			break;

		case ICMD_BASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movb_imm_memindex(cd, iptr->sx.s23.s3.constval,
								   OFFSET(java_bytearray_t, data[0]), s1, s2, 0);
			break;

		case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movw_imm_memindex(cd, iptr->sx.s23.s3.constval,
								   OFFSET(java_chararray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movw_imm_memindex(cd, iptr->sx.s23.s3.constval,
								   OFFSET(java_shortarray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_IASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_imm_memindex(cd, iptr->sx.s23.s3.constval,
								  OFFSET(java_intarray_t, data[0]), s1, s2, 2);
			break;

		case ICMD_LASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_imm_memindex(cd, 
						   (u4) (iptr->sx.s23.s3.constval & 0x00000000ffffffff),
						   OFFSET(java_longarray_t, data[0]), s1, s2, 3);
			emit_mov_imm_memindex(cd, 
						        ((s4)iptr->sx.s23.s3.constval) >> 31, 
						        OFFSET(java_longarray_t, data[0]) + 4, s1, s2, 3);
			break;

		case ICMD_AASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_imm_memindex(cd, 0, 
								  OFFSET(java_objectarray_t, data[0]), s1, s2, 2);
			break;


		case ICMD_GETSTATIC:  /* ...  ==> ..., value                          */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, 0);

			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = (intptr_t) fi->value;

				if (!class_is_or_almost_initialized(fi->clazz))
					patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz, 0);
  			}

			M_MOV_IMM2(disp, REG_ITMP1);
			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD(d, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP23_PACKED);
				M_LLD(d, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_FLD(d, REG_ITMP1, 0);
				break;
			case TYPE_DBL:				
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_DLD(d, REG_ITMP1, 0);
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
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = (intptr_t) fi->value;

				if (!class_is_or_almost_initialized(fi->clazz))
					patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz, 0);
  			}

			M_MOV_IMM2(disp, REG_ITMP1);
			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_IST(s1, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				s1 = emit_load_s1(jd, iptr, REG_ITMP23_PACKED);
				M_LST(s1, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				s1 = emit_load_s1(jd, iptr, REG_FTMP1);
				emit_fstps_membase(cd, REG_ITMP1, 0);
				break;
			case TYPE_DBL:
				s1 = emit_load_s1(jd, iptr, REG_FTMP1);
				emit_fstpl_membase(cd, REG_ITMP1, 0);
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
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = (intptr_t) fi->value;

				if (!class_is_or_almost_initialized(fi->clazz))
					patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz, 0);
  			}

			M_MOV_IMM2(disp, REG_ITMP1);
			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				M_IST_IMM(iptr->sx.s23.s2.constval, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				M_IST_IMM(iptr->sx.s23.s2.constval & 0xffffffff, REG_ITMP1, 0);
				M_IST_IMM(((s4)iptr->sx.s23.s2.constval) >> 31, REG_ITMP1, 4);
				break;
			default:
				assert(false);
				break;
			}
			break;

		case ICMD_GETFIELD:   /* .., objectref.  ==> ..., value               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			emit_nullpointer_check(cd, iptr, s1);

#if defined(ENABLE_ESCAPE_CHECK)
			/*emit_escape_check(cd, s1);*/
#endif

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_getfield,
									iptr->sx.s23.s3.uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD32(d, s1, disp);
				break;
			case TYPE_LNG:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP23_PACKED);
				M_LLD32(d, s1, disp);
				break;
			case TYPE_FLT:
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_FLD32(d, s1, disp);
				break;
			case TYPE_DBL:				
				d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
				M_DLD32(d, s1, disp);
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

			/* must be done here because of code patching */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
			}

			if (!IS_FLT_DBL_TYPE(fieldtype)) {
				if (IS_2_WORD_TYPE(fieldtype))
					s2 = emit_load_s2(jd, iptr, REG_ITMP23_PACKED);
				else
					s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			}
			else
				s2 = emit_load_s2(jd, iptr, REG_FTMP2);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				/* XXX */
				uf   = iptr->sx.s23.s3.uf;
				disp = 0;

				patcher_add_patch_ref(jd, PATCHER_putfield, uf, 0);
			}
			else {
				/* XXX */
				fi   = iptr->sx.s23.s3.fmiref->p.field;
				disp = fi->offset;
			}

			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				M_IST32(s2, s1, disp);
				break;
			case TYPE_LNG:
				M_LST32(s2, s1, disp);
				break;
			case TYPE_FLT:
				emit_fstps_membase32(cd, s1, disp);
				break;
			case TYPE_DBL:
				emit_fstpl_membase32(cd, s1, disp);
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
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_putfieldconst,
									uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_ADR:
				M_IST32_IMM(iptr->sx.s23.s2.constval, s1, disp);
				break;
			case TYPE_LNG:
				M_IST32_IMM(iptr->sx.s23.s2.constval & 0xffffffff, s1, disp);
				M_IST32_IMM(((s4)iptr->sx.s23.s2.constval) >> 31, s1, disp + 4);
				break;
			default:
				assert(false);
				break;
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			M_CALL_IMM(0);                            /* passing exception pc */
			M_POP(REG_ITMP2_XPC);

			M_MOV_IMM(asm_handle_exception, REG_ITMP3);
			M_JMP(REG_ITMP3);
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			if (iptr->sx.val.l == 0) {
				M_INTMOVE(GET_LOW_REG(s1), REG_ITMP1);
				M_OR(GET_HIGH_REG(s1), REG_ITMP1);
			}
			else {
				M_LNGMOVE(s1, REG_ITMP12_PACKED);
				M_XOR_IMM(iptr->sx.val.l, REG_ITMP1);
				M_XOR_IMM(iptr->sx.val.l >> 32, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP1);
			}
			emit_beq(cd, iptr->dst.block);
			break;

		case ICMD_IF_LLT:       /* ..., value ==> ...                         */

			if (iptr->sx.val.l == 0) {
				/* If high 32-bit are less than zero, then the 64-bits
				   are too. */
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
				M_CMP_IMM(0, s1);
				emit_blt(cd, iptr->dst.block);
			}
			else {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_CMP_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(s1));
				emit_blt(cd, iptr->dst.block);
				M_BGT(6 + 6);
				M_CMP_IMM32(iptr->sx.val.l, GET_LOW_REG(s1));
				emit_bult(cd, iptr->dst.block);
			}			
			break;

		case ICMD_IF_LLE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			M_CMP_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(s1));
			emit_blt(cd, iptr->dst.block);
			M_BGT(6 + 6);
			M_CMP_IMM32(iptr->sx.val.l, GET_LOW_REG(s1));
			emit_bule(cd, iptr->dst.block);
			break;

		case ICMD_IF_LNE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			if (iptr->sx.val.l == 0) {
				M_INTMOVE(GET_LOW_REG(s1), REG_ITMP1);
				M_OR(GET_HIGH_REG(s1), REG_ITMP1);
			}
			else {
				M_LNGMOVE(s1, REG_ITMP12_PACKED);
				M_XOR_IMM(iptr->sx.val.l, REG_ITMP1);
				M_XOR_IMM(iptr->sx.val.l >> 32, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP1);
			}
			emit_bne(cd, iptr->dst.block);
			break;

		case ICMD_IF_LGT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			M_CMP_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(s1));
			emit_bgt(cd, iptr->dst.block);
			M_BLT(6 + 6);
			M_CMP_IMM32(iptr->sx.val.l, GET_LOW_REG(s1));
			emit_bugt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LGE:       /* ..., value ==> ...                         */

			if (iptr->sx.val.l == 0) {
				/* If high 32-bit are greater equal zero, then the
				   64-bits are too. */
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
				M_CMP_IMM(0, s1);
				emit_bge(cd, iptr->dst.block);
			}
			else {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_CMP_IMM(iptr->sx.val.l >> 32, GET_HIGH_REG(s1));
				emit_bgt(cd, iptr->dst.block);
				M_BLT(6 + 6);
				M_CMP_IMM32(iptr->sx.val.l, GET_LOW_REG(s1));
				emit_buge(cd, iptr->dst.block);
			}
			break;

		case ICMD_IF_LCMPEQ:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, REG_ITMP1);
			M_XOR(s2, REG_ITMP1);
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP2);
			M_XOR(s2, REG_ITMP2);
			M_OR(REG_ITMP1, REG_ITMP2);
			emit_beq(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPNE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, REG_ITMP1);
			M_XOR(s2, REG_ITMP1);
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP2);
			M_XOR(s2, REG_ITMP2);
			M_OR(REG_ITMP1, REG_ITMP2);
			emit_bne(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s2, s1);
			emit_blt(cd, iptr->dst.block);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_BGT(2 + 6);
			M_CMP(s2, s1);
			emit_bult(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s2, s1);
			emit_bgt(cd, iptr->dst.block);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_BLT(2 + 6);
			M_CMP(s2, s1);
			emit_bugt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s2, s1);
			emit_blt(cd, iptr->dst.block);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_BGT(2 + 6);
			M_CMP(s2, s1);
			emit_bule(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMP(s2, s1);
			emit_bgt(cd, iptr->dst.block);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_BLT(2 + 6);
			M_CMP(s2, s1);
			emit_buge(cd, iptr->dst.block);
			break;

		case ICMD_TABLESWITCH:  /* ..., index ==> ...                         */
			{
				s4 i, l;
				branch_target_t *table;

				table = iptr->dst.table;

				l = iptr->sx.s23.s2.tablelow;
				i = iptr->sx.s23.s3.tablehigh;

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				M_INTMOVE(s1, REG_ITMP1);

				if (l != 0)
					M_ISUB_IMM(l, REG_ITMP1);

				i = i - l + 1;

                /* range check */

				M_CMP_IMM(i - 1, REG_ITMP1);
				emit_bugt(cd, table[0].block);

				/* build jump table top down and use address of lowest entry */

				table += i;

				while (--i >= 0) {
					dseg_add_target(cd, table->block); 
					--table;
				}

				/* length of dataseg after last dseg_addtarget is used
				   by load */

				M_MOV_IMM(0, REG_ITMP2);
				dseg_adddata(cd);
				emit_mov_memindex_reg(cd, -(cd->dseglen), REG_ITMP2, REG_ITMP1, 2, REG_ITMP1);
				M_JMP(REG_ITMP1);
			}
			break;

		case ICMD_BUILTIN:
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL) {
				M_MOV_IMM(bte->fp, REG_ITMP1);
			}
			else {
				M_MOV_IMM(bte->stub, REG_ITMP1);
			}
			M_CALL(REG_ITMP1);

#if defined(ENABLE_ESCAPE_CHECK)
			if (bte->opcode == ICMD_NEW || bte->opcode == ICMD_NEWARRAY) {
				/*emit_escape_annotate_object(cd, m);*/
			}
#endif
			break;

		case ICMD_INVOKESPECIAL:
			M_ALD(REG_ITMP1, REG_SP, 0 * 8);
			emit_nullpointer_check(cd, iptr, REG_ITMP1);
			/* fall through */

		case ICMD_INVOKESTATIC:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special,
									um, 0);

				disp = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = (ptrint) lm->stubroutine;
			}

			M_MOV_IMM2(disp, REG_ITMP2);
			M_CALL(REG_ITMP2);
			break;

		case ICMD_INVOKEVIRTUAL:
			M_ALD(REG_ITMP1, REG_SP, 0 * 8);
			// XXX s1 may be uninitialized?
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;

				patcher_add_patch_ref(jd, PATCHER_invokevirtual, um, 0);

				s1 = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, table[0]) +
					sizeof(methodptr) * lm->vftblindex;
			}

			M_ALD(REG_METHODPTR, REG_ITMP1,
				  OFFSET(java_object_t, vftbl));
			M_ALD32(REG_ITMP3, REG_METHODPTR, s1);
			M_CALL(REG_ITMP3);
			break;

		case ICMD_INVOKEINTERFACE:
			M_ALD(REG_ITMP1, REG_SP, 0 * 8);
			emit_nullpointer_check(cd, iptr, s1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;

				patcher_add_patch_ref(jd, PATCHER_invokeinterface, um, 0);

				s1 = 0;
				s2 = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, interfacetable[0]) -
					sizeof(methodptr) * lm->clazz->index;

				s2 = sizeof(methodptr) * (lm - lm->clazz->methods);
			}

			M_ALD(REG_METHODPTR, REG_ITMP1,
				  OFFSET(java_object_t, vftbl));
			M_ALD32(REG_METHODPTR, REG_METHODPTR, s1);
			M_ALD32(REG_ITMP3, REG_METHODPTR, s2);
			M_CALL(REG_ITMP3);
			break;

		case ICMD_CHECKCAST:  /* ..., objectref ==> ..., objectref            */

			if (!(iptr->flags.bits & INS_FLAG_ARRAY)) {
				/* object type cast-check */

				classinfo *super;
				vftbl_t   *supervftbl;
				s4         superindex;

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					super = NULL;
					superindex = 0;
					supervftbl = NULL;
				}
				else {
					super = iptr->sx.s23.s3.c.cls;
					superindex = super->index;
					supervftbl = super->vftbl;
				}
			
				s1 = emit_load_s1(jd, iptr, REG_ITMP1);

				/* if class is not resolved, check which code to call */

				if (super == NULL) {
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_1);

					patcher_add_patch_ref(jd, PATCHER_checkcast_instanceof_flags,
										iptr->sx.s23.s3.c.ref, 0);

					M_MOV_IMM2(0, REG_ITMP2);                 /* super->flags */
					M_AND_IMM32(ACC_INTERFACE, REG_ITMP2);
					emit_label_beq(cd, BRANCH_LABEL_2);
				}

				/* interface checkcast code */

				if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
					if (super != NULL) {
						M_TEST(s1);
						emit_label_beq(cd, BRANCH_LABEL_3);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));

					if (super == NULL) {
						patcher_add_patch_ref(jd, PATCHER_checkcast_interface,
											iptr->sx.s23.s3.c.ref,
											0);
					}

					M_ILD32(REG_ITMP3,
							REG_ITMP2, OFFSET(vftbl_t, interfacetablelength));
					M_ISUB_IMM32(superindex, REG_ITMP3);
					/* XXX do we need this one? */
					M_TEST(REG_ITMP3);
					emit_classcast_check(cd, iptr, BRANCH_LE, REG_ITMP3, s1);

					M_ALD32(REG_ITMP3, REG_ITMP2,
							OFFSET(vftbl_t, interfacetable[0]) -
							superindex * sizeof(methodptr*));
					M_TEST(REG_ITMP3);
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
					}
					else {
						M_TEST(s1);
						emit_label_beq(cd, BRANCH_LABEL_5);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					if (super == NULL) {
						patcher_add_patch_ref(jd, PATCHER_checkcast_class,
											iptr->sx.s23.s3.c.ref,
											0);
					}
					M_MOV_IMM2(supervftbl, REG_ITMP3);

					if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_CMP_MEMINDEX(REG_ITMP2, 0, REG_ITMP1, 0, REG_ITMP3);
						emit_label_beq(cd, BRANCH_LABEL_6);  /* good */

						if (super == NULL) {
							M_ICMP_IMM(OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
							emit_label_bne(cd, BRANCH_LABEL_10);  /* throw */
						}

						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_CMP_MEMBASE(REG_ITMP2, OFFSET(vftbl_t, subtype_depth), REG_ITMP1);
						emit_label_bgt(cd, BRANCH_LABEL_9);  /* throw */

						M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
						M_CMP_MEMINDEX(REG_ITMP2, -4*DISPLAY_SIZE, REG_ITMP1, 2, REG_ITMP3);
						emit_label_beq(cd, BRANCH_LABEL_7);  /* good */

						emit_label(cd, BRANCH_LABEL_9);
						if (super == NULL)
							emit_label(cd, BRANCH_LABEL_10);

						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
						M_ALD_MEM(s1, TRAP_ClassCastException);

						emit_label(cd, BRANCH_LABEL_7);
						emit_label(cd, BRANCH_LABEL_6);
						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
					}
					else {
						M_CMP_MEMBASE(REG_ITMP2, super->vftbl->subtype_offset, REG_ITMP3);

						emit_classcast_check(cd, iptr, BRANCH_NE, REG_ITMP3, s1);
					}

					if (super != NULL)
						emit_label(cd, BRANCH_LABEL_5);
				}

				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_1);
					emit_label(cd, BRANCH_LABEL_4);
				}

				d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			}
			else {
				/* array type cast-check */

				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_AST(s1, REG_SP, 0 * 4);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					patcher_add_patch_ref(jd, PATCHER_builtin_arraycheckcast,
										iptr->sx.s23.s3.c.ref, 0);
				}

				M_AST_IMM(iptr->sx.s23.s3.c.cls, REG_SP, 1 * 4);
				M_MOV_IMM(BUILTIN_arraycheckcast, REG_ITMP3);
				M_CALL(REG_ITMP3);

				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_TEST(REG_RESULT);
				emit_classcast_check(cd, iptr, BRANCH_EQ, REG_RESULT, s1);

				d = codegen_reg_of_dst(jd, iptr, s1);
			}

			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INSTANCEOF: /* ..., objectref ==> ..., intresult            */

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
				M_INTMOVE(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			M_CLR(d);

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_TEST(s1);
				emit_label_beq(cd, BRANCH_LABEL_1);

				patcher_add_patch_ref(jd, PATCHER_checkcast_instanceof_flags,
									iptr->sx.s23.s3.c.ref, 0);

				M_MOV_IMM2(0, REG_ITMP3);                     /* super->flags */
				M_AND_IMM32(ACC_INTERFACE, REG_ITMP3);
				emit_label_beq(cd, BRANCH_LABEL_2);
			}

			/* interface instanceof code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if (super != NULL) {
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_3);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));

				if (super == NULL) {
					patcher_add_patch_ref(jd, PATCHER_instanceof_interface,
										iptr->sx.s23.s3.c.ref, 0);
				}

				M_ILD32(REG_ITMP3,
						REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));
				M_ISUB_IMM32(superindex, REG_ITMP3);
				M_TEST(REG_ITMP3);

				disp = (2 + 4 /* mov_membase32_reg */ + 2 /* test */ +
						6 /* jcc */ + 5 /* mov_imm_reg */);

				M_BLE(disp);
				M_ALD32(REG_ITMP1, REG_ITMP1,
						OFFSET(vftbl_t, interfacetable[0]) -
						superindex * sizeof(methodptr*));
				M_TEST(REG_ITMP1);
/*  					emit_setcc_reg(cd, CC_A, d); */
/*  					emit_jcc(cd, CC_BE, 5); */
				M_BEQ(5);
				M_MOV_IMM(1, d);

				if (super == NULL)
					emit_label_br(cd, BRANCH_LABEL_4);
				else
					emit_label(cd, BRANCH_LABEL_3);
			}

			/* class instanceof code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);
				}
				else {
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_5);
				}

				M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				if (super == NULL) {
					patcher_add_patch_ref(jd, PATCHER_instanceof_class,
										iptr->sx.s23.s3.c.ref, 0);
				}
				M_MOV_IMM2(supervftbl, REG_ITMP3);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_CMP_MEMINDEX(REG_ITMP2, 0, REG_ITMP1, 0, REG_ITMP3);
					emit_label_bne(cd, BRANCH_LABEL_8); /* jump over INC/SETE */
					if (d == REG_ITMP2) {
						M_SETE(d);
						M_BSEXT(d, d);
					} else
						M_IINC(d);
					emit_label_br(cd, BRANCH_LABEL_6);  /* true */
					emit_label(cd, BRANCH_LABEL_8);

					if (super == NULL) {
						M_ICMP_IMM(OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
						emit_label_bne(cd, BRANCH_LABEL_10);  /* false */
					}

					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
					M_CMP_MEMBASE(REG_ITMP2, OFFSET(vftbl_t, subtype_depth), REG_ITMP1);
					emit_label_bgt(cd, BRANCH_LABEL_9);  /* false */

					M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
					M_CMP_MEMINDEX(REG_ITMP2, -4*DISPLAY_SIZE, REG_ITMP1, 2, REG_ITMP3);
					if (d >= 4) {
						M_SETE(REG_ITMP1);
						M_BSEXT(REG_ITMP1, d);
					}
					else {
						M_SETE(d);
						if (d == REG_ITMP2) {
							M_BSEXT(d, d);

							emit_label_br(cd, BRANCH_LABEL_7); /* jump over M_CLR */
						}
					}

					emit_label(cd, BRANCH_LABEL_9);
					if (super == NULL)
						emit_label(cd, BRANCH_LABEL_10);
					if (d == REG_ITMP2) {
						M_CLR(d);

						emit_label(cd, BRANCH_LABEL_7);
					}
					emit_label(cd, BRANCH_LABEL_6);
				}
				else {
					M_CMP_MEMBASE(REG_ITMP2, super->vftbl->subtype_offset, REG_ITMP3);

					if (d >= 4) {
						M_SETE(REG_ITMP1);
						M_BSEXT(REG_ITMP1, d);
					}
					else {
						M_SETE(d);
						if (d == REG_ITMP2)
							M_BSEXT(d, d);
					}
				}

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
				/* copy SAVEDVAR sizes to stack */
				var = VAR(iptr->sx.s23.s2.args[s1]);

				/* Already Preallocated? */
				if (!(var->flags & PREALLOC)) {
					if (var->flags & INMEMORY) {
						M_ILD(REG_ITMP1, REG_SP, var->vv.regoff);
						M_IST(REG_ITMP1, REG_SP, (s1 + 3) * 4);
					}
					else
						M_IST(var->vv.regoff, REG_SP, (s1 + 3) * 4);
				}
			}

			/* is a patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				patcher_add_patch_ref(jd, PATCHER_builtin_multianewarray,
									iptr->sx.s23.s3.c.ref, 0);

				disp = 0;

			}
			else
				disp = (ptrint) iptr->sx.s23.s3.c.cls;

			/* a0 = dimension count */

			M_IST_IMM(iptr->s1.argcount, REG_SP, 0 * 4);

			/* a1 = arraydescriptor */

			M_IST_IMM(disp, REG_SP, 1 * 4);

			/* a2 = pointer to dimensions = stack pointer */

			M_MOV(REG_SP, REG_ITMP1);
			M_AADD_IMM(3 * 4, REG_ITMP1);
			M_AST(REG_ITMP1, REG_SP, 2 * 4);

			M_MOV_IMM(BUILTIN_multianewarray, REG_ITMP1);
			M_CALL(REG_ITMP1);

			/* check for exception before result assignment */

			emit_exception_check(cd, iptr);

			s1 = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			M_INTMOVE(REG_RESULT, s1);
			emit_store_dst(jd, iptr, s1);
			break;

		default:
			vm_abort("Unknown ICMD %d during code generation", iptr->opc);
	} /* switch */
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
	int          i, j;                 /* count variables                    */
	int          s1, s2;
	int          disp;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	/* set some variables */

	md = m->parseddesc;

	/* calculate stackframe size */

	cd->stackframesize =
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +
		sizeof(localref_table) / SIZEOF_VOID_P +
		4 +                             /* 4 arguments (start_native_call)    */
		nmd->memuse;

    /* keep stack 16-byte aligned */

	ALIGN_ODD(cd->stackframesize);

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

#if defined(ENABLE_PROFILING)
	/* generate native method profiling code */

	if (JITDATA_HAS_FLAG_INSTRUMENT(jd)) {
		/* count frequency */

		M_MOV_IMM(code, REG_ITMP1);
		M_IADD_IMM_MEMBASE(1, REG_ITMP1, OFFSET(codeinfo, frequency));
	}
#endif

	/* calculate stackframe size for native function */

	M_ASUB_IMM(cd->stackframesize * 8 + 4, REG_SP);

	/* Mark the whole fpu stack as free for native functions (only for saved  */
	/* register count == 0).                                                  */

	emit_ffree_reg(cd, 0);
	emit_ffree_reg(cd, 1);
	emit_ffree_reg(cd, 2);
	emit_ffree_reg(cd, 3);
	emit_ffree_reg(cd, 4);
	emit_ffree_reg(cd, 5);
	emit_ffree_reg(cd, 6);
	emit_ffree_reg(cd, 7);

#if defined(ENABLE_GC_CACAO)
	/* remember callee saved int registers in stackframeinfo (GC may need to  */
	/* recover them during a collection).                                     */

	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_AST(abi_registers_integer_saved[i], REG_SP, disp + i * 4);
#endif

	/* prepare data structures for native function call */

	M_MOV(REG_SP, REG_ITMP1);
	M_AST(REG_ITMP1, REG_SP, 0 * 4);
	M_IST_IMM(0, REG_SP, 1 * 4);
	dseg_adddata(cd);

	M_MOV_IMM(codegen_start_native_call, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_RESULT, REG_ITMP3);

	/* Copy or spill arguments to new locations. */

	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		if (!md->params[i].inmemory)
			assert(0);

		s1 = md->params[i].regoff + cd->stackframesize * 8 + 8;
		s2 = nmd->params[j].regoff;

		/* float/double in memory can be copied like int/longs */

		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_FLT:
		case TYPE_ADR:
			M_ILD(REG_ITMP1, REG_SP, s1);
			M_IST(REG_ITMP1, REG_SP, s2);
			break;
		case TYPE_LNG:
		case TYPE_DBL:
			M_LLD(REG_ITMP12_PACKED, REG_SP, s1);
			M_LST(REG_ITMP12_PACKED, REG_SP, s2);
			break;
		default:
			assert(false);
			break;
		}
	}

	/* Handle native Java methods. */

	if (m->flags & ACC_NATIVE) {
		/* if function is static, put class into second argument */

		if (m->flags & ACC_STATIC)
			M_AST(REG_ITMP3, REG_SP, 1 * 4);

		/* put env into first argument */

		M_AST_IMM(VM::get_current()->get_jnienv(), REG_SP, 0 * 4);
	}

	/* Call the native function. */

	disp = dseg_add_functionptr(cd, f);
	emit_mov_imm_reg(cd, 0, REG_ITMP3);
	dseg_adddata(cd);
	M_ALD(REG_ITMP1, REG_ITMP3, disp);
	M_CALL(REG_ITMP1);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		switch (md->returntype.primitivetype) {
		case PRIMITIVETYPE_BOOLEAN:
			M_BZEXT(REG_RESULT, REG_RESULT);
			break;
		case PRIMITIVETYPE_BYTE:
			M_BSEXT(REG_RESULT, REG_RESULT);
			break;
		case PRIMITIVETYPE_CHAR:
			M_CZEXT(REG_RESULT, REG_RESULT);
			break;
		case PRIMITIVETYPE_SHORT:
			M_SSEXT(REG_RESULT, REG_RESULT);
			break;
		default:
			break;
		}
		M_IST(REG_RESULT, REG_SP, 1 * 8);
		break;
	case TYPE_LNG:
		M_LST(REG_RESULT_PACKED, REG_SP, 1 * 8);
		break;
	case TYPE_FLT:
		emit_fsts_membase(cd, REG_SP, 1 * 8);
		break;
	case TYPE_DBL:
		emit_fstl_membase(cd, REG_SP, 1 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	/* remove native stackframe info */

	M_MOV(REG_SP, REG_ITMP1);
	M_AST(REG_ITMP1, REG_SP, 0 * 4);
	M_IST_IMM(0, REG_SP, 1 * 4);
	dseg_adddata(cd);

	M_MOV_IMM(codegen_finish_native_call, REG_ITMP1);
	M_CALL(REG_ITMP1);
	M_MOV(REG_RESULT, REG_ITMP2);                 /* REG_ITMP3 == REG_RESULT2 */

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT, REG_SP, 1 * 8);
		break;
	case TYPE_LNG:
		M_LLD(REG_RESULT_PACKED, REG_SP, 1 * 8);
		break;
	case TYPE_FLT:
		emit_flds_membase(cd, REG_SP, 1 * 8);
		break;
	case TYPE_DBL:
		emit_fldl_membase(cd, REG_SP, 1 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

#if defined(ENABLE_GC_CACAO)
	/* restore callee saved int registers from stackframeinfo (GC might have  */
	/* modified them during a collection).                                    */

	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_ALD(abi_registers_integer_saved[i], REG_SP, disp + i * 4);
#endif

	M_AADD_IMM(cd->stackframesize * 8 + 4, REG_SP);

	/* check for exception */

	M_TEST(REG_ITMP2);
	M_BNE(1);

	M_RET;

	/* handle exception */

	M_MOV(REG_ITMP2, REG_ITMP1_XPTR);
	M_ALD(REG_ITMP2_XPC, REG_SP, 0);
	M_ASUB_IMM(2, REG_ITMP2_XPC);

	M_MOV_IMM(asm_handle_nat_exception, REG_ITMP3);
	M_JMP(REG_ITMP3);
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
