/* src/vm/jit/alpha/codegen.cpp - machine code generator for Alpha

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

#include "md.hpp"
#include "md-abi.hpp"

#include "vm/jit/alpha/arch.hpp"
#include "vm/jit/alpha/codegen.hpp"

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
#include "vm/jit/parse.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trap.hpp"


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

	/* create stack frame (if necessary) */

	if (cd->stackframesize)
		M_LDA(REG_SP, REG_SP, -(cd->stackframesize * 8));

	/* save return address and used callee saved registers */

	p = cd->stackframesize;
	if (!code_is_leafmethod(code)) {
		p--; M_AST(REG_RA, REG_SP, p * 8);
	}
	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_LST(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DST(rd->savfltregs[i], REG_SP, p * 8);
	}

	/* take arguments out of register or stack frame */

	md = m->parseddesc;

 	for (p = 0, l = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;

  		varindex = jd->local_map[l * 5 + t];

 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter for 2 word types */
 			l++;

		if (varindex == jitdata::UNUSED)
			continue;

 		var = VAR(varindex);

		s1 = md->params[p].regoff;

		if (IS_INT_LNG_TYPE(t)) {                    /* integer args          */
 			if (!md->params[p].inmemory) {           /* register arguments    */
 				if (!IS_INMEMORY(var->flags))
 					M_INTMOVE(s1, var->vv.regoff);
				else
 					M_LST(s1, REG_SP, var->vv.regoff);
			}
			else {                                   /* stack arguments       */
 				if (!IS_INMEMORY(var->flags))
 					M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				else
					var->vv.regoff = cd->stackframesize * 8 + s1;
			}
		}
		else {                                       /* floating args         */
 			if (!md->params[p].inmemory) {           /* register arguments    */
 				if (!IS_INMEMORY(var->flags))
 					emit_fmove(cd, s1, var->vv.regoff);
 				else
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, var->vv.regoff);
					else
						M_FST(s1, REG_SP, var->vv.regoff);
			}
			else {                                   /* stack arguments       */
 				if (!(var->flags & INMEMORY))
					if (IS_2_WORD_TYPE(t))
						M_DLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
					else
						M_FLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				else
					var->vv.regoff = cd->stackframesize * 8 + s1;
			}
		}
	}
}


/**
 * Generates machine code for the method epilog.
 */
void codegen_emit_epilog(jitdata* jd)
{
	int32_t p;
	int i;

	// Get required compiler data.
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;
	registerdata* rd   = jd->rd;

	p = cd->stackframesize;

	/* restore return address */

	if (!code_is_leafmethod(code)) {
		p--; M_LLD(REG_RA, REG_SP, p * 8);
	}

	/* restore saved registers */

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_LLD(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DLD(rd->savfltregs[i], REG_SP, p * 8);
	}

	/* deallocate stack */

	if (cd->stackframesize)
		M_LDA(REG_SP, REG_SP, cd->stackframesize * 8);

	M_RET(REG_ZERO, REG_RA);
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
	int32_t             s1, s2, s3, d = 0;
	int32_t             disp;

	// Get required compiler data.
	codegendata* cd = jd->cd;

	switch (iptr->opc) {

		/* constant operations ************************************************/

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

				/* XXX Only add the patcher, if this position needs to
				   be patched.  If there was a previous position which
				   resolved the same class, the returned displacement
				   of dseg_add_address is ok to use. */

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									  cr, disp);

				M_ALD(d, REG_PV, disp);
			}
			else {
				if (iptr->sx.val.anyptr == NULL)
					M_INTMOVE(REG_ZERO, d);
				else {
					disp = dseg_add_address(cd, iptr->sx.val.anyptr);
					M_ALD(d, REG_PV, disp);
				}
			}
			emit_store_dst(jd, iptr, d);
			break;


		/* integer operations *************************************************/

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISUB(REG_ZERO, s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSUB(REG_ZERO, s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_IADD(s1, REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (has_ext_instr_set) {
				M_BSEXT(s1, d);
			} else {
				M_SLL_IMM(s1, 56, d);
				M_SRA_IMM( d, 56, d);
			}
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
			if (has_ext_instr_set) {
				M_SSEXT(s1, d);
			} else {
				M_SLL_IMM(s1, 48, d);
				M_SRA_IMM( d, 48, d);
			}
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_IADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_IADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IINC:
		case ICMD_IADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_IADD_IMM(s1, iptr->sx.val.i, d);
			} else if ((iptr->sx.val.i > -256) && (iptr->sx.val.i < 0)) {
				M_ISUB_IMM(s1, (-iptr->sx.val.i), d);
			} else {
				/* XXX maybe use M_LDA? */
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_IADD(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
				M_LADD_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_LADD(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_ISUB_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_ISUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
				M_LSUB_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_LSUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_IMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_IMUL_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_IMUL(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
				M_LMUL_IMM(s1, iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_LMUL(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_A1);
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			emit_arithmetic_check(cd, iptr, s2);

			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s2, REG_A1);
			bte = iptr->sx.s23.s3.bte;
			disp = dseg_add_functionptr(cd, bte->fp);
			M_ALD(REG_PV, REG_PV, disp);
			M_JSR(REG_RA, REG_PV);
			disp = (s4) (cd->mcodeptr - cd->mcodebase);
			M_LDA(REG_PV, REG_RA, -disp);

			M_IADD(REG_RESULT, REG_ZERO, d); /* sign extend (bugfix for gcc -O2) */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_A1);
			d = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			emit_arithmetic_check(cd, iptr, s2);

			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s2, REG_A1);
			bte = iptr->sx.s23.s3.bte;
			disp = dseg_add_functionptr(cd, bte->fp);
			M_ALD(REG_PV, REG_PV, disp);
			M_JSR(REG_RA, REG_PV);
			disp = (s4) (cd->mcodeptr - cd->mcodebase);
			M_LDA(REG_PV, REG_RA, -disp);

			M_INTMOVE(REG_RESULT, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value << constant       */
		case ICMD_LDIVPOW2:   /* val.i = constant                             */
		                      
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.i <= 15) {
				M_LDA(REG_ITMP2, s1, (1 << iptr->sx.val.i) -1);
				M_CMOVGE(s1, s1, REG_ITMP2);
			} else {
				M_SRA_IMM(s1, 63, REG_ITMP2);
				M_SRL_IMM(REG_ITMP2, 64 - iptr->sx.val.i, REG_ITMP2);
				M_LADD(s1, REG_ITMP2, REG_ITMP2);
			}
			M_SRA_IMM(REG_ITMP2, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x1f, REG_ITMP3);
			M_SLL(s1, REG_ITMP3, d);
			M_IADD(d, REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL_IMM(s1, iptr->sx.val.i & 0x1f, d);
			M_IADD(d, REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x1f, REG_ITMP3);
			M_SRA(s1, REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA_IMM(s1, iptr->sx.val.i & 0x1f, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x1f, REG_ITMP3);
            M_IZEXT(s1, d);
			M_SRL(d, REG_ITMP3, d);
			M_IADD(d, REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
            M_IZEXT(s1, d);
			M_SRL_IMM(d, iptr->sx.val.i & 0x1f, d);
			M_IADD(d, REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL_IMM(s1, iptr->sx.val.i & 0x3f, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA_IMM(s1, iptr->sx.val.i & 0x3f, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRL_IMM(s1, iptr->sx.val.i & 0x3f, d);
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
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
			} else if (iptr->sx.val.i == 0xffff) {
				M_CZEXT(s1, d);
			} else if (iptr->sx.val.i == 0xffffff) {
				M_ZAPNOT_IMM(s1, 0x07, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_AND(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
				M_BGEZ(s1, 3);
				M_ISUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.i, d);
			} else if (iptr->sx.val.i == 0xffff) {
				M_CZEXT(s1, d);
				M_BGEZ(s1, 3);
				M_ISUB(REG_ZERO, s1, d);
				M_CZEXT(d, d);
			} else if (iptr->sx.val.i == 0xffffff) {
				M_ZAPNOT_IMM(s1, 0x07, d);
				M_BGEZ(s1, 3);
				M_ISUB(REG_ZERO, s1, d);
				M_ZAPNOT_IMM(d, 0x07, d);
			} else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_AND(s1, REG_ITMP3, d);
				M_BGEZ(s1, 3);
				M_ISUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP3, d);
			}
			M_ISUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
				M_AND_IMM(s1, iptr->sx.val.l, d);
			} else if (iptr->sx.val.l == 0xffffL) {
				M_CZEXT(s1, d);
			} else if (iptr->sx.val.l == 0xffffffL) {
				M_ZAPNOT_IMM(s1, 0x07, d);
			} else if (iptr->sx.val.l == 0xffffffffL) {
				M_IZEXT(s1, d);
			} else if (iptr->sx.val.l == 0xffffffffffL) {
				M_ZAPNOT_IMM(s1, 0x1f, d);
			} else if (iptr->sx.val.l == 0xffffffffffffL) {
				M_ZAPNOT_IMM(s1, 0x3f, d);
			} else if (iptr->sx.val.l == 0xffffffffffffffL) {
				M_ZAPNOT_IMM(s1, 0x7f, d);
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
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
				M_AND_IMM(s1, iptr->sx.val.l, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.l, d);
			} else if (iptr->sx.val.l == 0xffffL) {
				M_CZEXT(s1, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_CZEXT(d, d);
			} else if (iptr->sx.val.l == 0xffffffL) {
				M_ZAPNOT_IMM(s1, 0x07, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_ZAPNOT_IMM(d, 0x07, d);
			} else if (iptr->sx.val.l == 0xffffffffL) {
				M_IZEXT(s1, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_IZEXT(d, d);
			} else if (iptr->sx.val.l == 0xffffffffffL) {
 				M_ZAPNOT_IMM(s1, 0x1f, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_ZAPNOT_IMM(d, 0x1f, d);
			} else if (iptr->sx.val.l == 0xffffffffffffL) {
				M_ZAPNOT_IMM(s1, 0x3f, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_ZAPNOT_IMM(d, 0x3f, d);
			} else if (iptr->sx.val.l == 0xffffffffffffffL) {
				M_ZAPNOT_IMM(s1, 0x7f, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_ZAPNOT_IMM(d, 0x7f, d);
			} else {
				LCONST(REG_ITMP3, iptr->sx.val.l);
				M_AND(s1, REG_ITMP3, d);
				M_BGEZ(s1, 3);
				M_LSUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP3, d);
			}
			M_LSUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */
		case ICMD_LOR:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_OR( s1,s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_OR_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_OR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
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
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 255)) {
				M_XOR_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_XOR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 255)) {
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
			M_CMPLT(s1, s2, REG_ITMP3);
			M_CMPLT(s2, s1, REG_ITMP1);
			M_LSUB(REG_ITMP1, REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;


		/* floating operations ************************************************/

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FMOVN(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_FMOVN(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_FADDS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_FADDS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_DADDS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_DADDS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_FSUBS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_FSUBS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_DSUBS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_DSUBS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_FMULS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_FMULS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 *** val2      */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_DMULS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_DMULS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_FDIVS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_FDIVS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (d == s1 || d == s2) {
				M_DDIVS(s1, s2, REG_FTMP3);
				M_TRAPB;
				M_FMOV(REG_FTMP3, d);
			} else {
				M_DDIVS(s1, s2, d);
				M_TRAPB;
			}
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_I2F:       /* ..., value  ==> ..., (float) value            */
		case ICMD_L2F:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_double(cd, 0.0); /* FIXME Not thread safe! */
			M_LST(s1, REG_PV, disp);
			M_DLD(d, REG_PV, disp);
			M_CVTLF(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2D:       /* ..., value  ==> ..., (double) value           */
		case ICMD_L2D:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_unique_double(cd, 0.0); /* FIXME Not thread safe! */
			M_LST(s1, REG_PV, disp);
			M_DLD(d, REG_PV, disp);
			M_CVTLD(d, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */
		case ICMD_D2I:
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_double(cd, 0.0); /* FIXME Not thread safe! */
			M_CVTDL_C(s1, REG_FTMP2);
			M_CVTLI(REG_FTMP2, REG_FTMP3);
			M_DST(REG_FTMP3, REG_PV, disp);
			M_ILD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_F2L:       /* ..., value  ==> ..., (long) value             */
		case ICMD_D2L:
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			disp = dseg_add_unique_double(cd, 0.0); /* FIXME Not thread safe! */
			M_CVTDL_C(s1, REG_FTMP2);
			M_DST(REG_FTMP2, REG_PV, disp);
			M_LLD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_CVTFDS(s1, d);
			M_TRAPB;
			emit_store_dst(jd, iptr, d);
			break;
					
		case ICMD_D2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_CVTDFS(s1, d);
			M_TRAPB;
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */
		case ICMD_DCMPL:
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_LSUB_IMM(REG_ZERO, 1, d);
			M_FCMPEQS(s1, s2, REG_FTMP3);
			M_TRAPB;
			M_FBEQZ (REG_FTMP3, 1);        /* jump over next instructions */
			M_CLR   (d);
			M_FCMPLTS(s2, s1, REG_FTMP3);
			M_TRAPB;
			M_FBEQZ (REG_FTMP3, 1);        /* jump over next instruction  */
			M_LADD_IMM(REG_ZERO, 1, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */
		case ICMD_DCMPG:
			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_LADD_IMM(REG_ZERO, 1, d);
			M_FCMPEQS(s1, s2, REG_FTMP3);
			M_TRAPB;
			M_FBEQZ (REG_FTMP3, 1);        /* jump over next instruction  */
			M_CLR   (d);
			M_FCMPLTS(s1, s2, REG_FTMP3);
			M_TRAPB;
			M_FBEQZ (REG_FTMP3, 1);        /* jump over next instruction  */
			M_LSUB_IMM(REG_ZERO, 1, d);
			emit_store_dst(jd, iptr, d);
			break;


		/* memory operations **************************************************/

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_BLDU(d, REG_ITMP1, OFFSET (java_bytearray_t, data[0]));
				M_BSEXT(d, d);
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_bytearray_t, data[0])+1);
				M_EXTQH(REG_ITMP2, REG_ITMP1, d);
				M_SRA_IMM(d, 56, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SLDU(d, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			}
			else {
				M_LADD (s2, s1, REG_ITMP1);
				M_LADD (s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_LDA  (REG_ITMP1, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_EXTWL(REG_ITMP2, REG_ITMP1, d);
			}
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SLDU( d, REG_ITMP1, OFFSET (java_shortarray_t, data[0]));
				M_SSEXT(d, d);
			} else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_shortarray_t, data[0])+2);
				M_EXTQH(REG_ITMP2, REG_ITMP1, d);
				M_SRA_IMM(d, 48, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S4ADDQ(s2, s1, REG_ITMP1);
			M_ILD(d, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S8ADDQ(s2, s1, REG_ITMP1);
			M_LLD(d, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S4ADDQ(s2, s1, REG_ITMP1);
			M_FLD(d, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S8ADDQ(s2, s1, REG_ITMP1);
			M_DLD(d, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SAADDQ(s2, s1, REG_ITMP1);
			M_ALD(d, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_BST(s3, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
				M_INSBL(s3, REG_ITMP1, REG_ITMP3);
				M_MSKBL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SST(s3, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_INSWL(s3, REG_ITMP1, REG_ITMP3);
				M_MSKWL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SST(s3, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
				M_INSWL(s3, REG_ITMP1, REG_ITMP3);
				M_MSKWL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_S4ADDQ(s2, s1, REG_ITMP1);
			M_IST(s3, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_S8ADDQ(s2, s1, REG_ITMP1);
			M_LST(s3, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			M_S4ADDQ(s2, s1, REG_ITMP1);
			M_FST(s3, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			M_S8ADDQ(s2, s1, REG_ITMP1);
			M_DST(s3, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
			break;

		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_A1);

			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s3, REG_A1);

			disp = dseg_add_functionptr(cd, BUILTIN_FAST_canstore);
			M_ALD(REG_PV, REG_PV, disp);
			M_JSR(REG_RA, REG_PV);
			disp = (s4) (cd->mcodeptr - cd->mcodebase);
			M_LDA(REG_PV, REG_RA, -disp);
			emit_arraystore_check(cd, iptr);

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SAADDQ(s2, s1, REG_ITMP1);
			M_AST(s3, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;


		case ICMD_BASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_BST(REG_ZERO, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
				M_INSBL(REG_ZERO, REG_ITMP1, REG_ITMP3);
				M_MSKBL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SST(REG_ZERO, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
				M_INSWL(REG_ZERO, REG_ITMP1, REG_ITMP3);
				M_MSKWL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			if (has_ext_instr_set) {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_SST(REG_ZERO, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
			}
			else {
				M_LADD(s2, s1, REG_ITMP1);
				M_LADD(s2, REG_ITMP1, REG_ITMP1);
				M_LLD_U(REG_ITMP2, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
				M_LDA(REG_ITMP1, REG_ITMP1, OFFSET(java_shortarray_t, data[0]));
				M_INSWL(REG_ZERO, REG_ITMP1, REG_ITMP3);
				M_MSKWL(REG_ITMP2, REG_ITMP1, REG_ITMP2);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP2);
				M_LST_U(REG_ITMP2, REG_ITMP1, 0);
			}
			break;

		case ICMD_IASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S4ADDQ(s2, s1, REG_ITMP1);
			M_IST(REG_ZERO, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_S8ADDQ(s2, s1, REG_ITMP1);
			M_LST(REG_ZERO, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_AASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SAADDQ(s2, s1, REG_ITMP1);
			M_AST(REG_ZERO, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;

		case ICMD_PUTSTATICCONST: /* ...  ==> ...                             */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = dseg_add_unique_address(cd, uf);

				patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz))
					patcher_add_patch_ref(jd, PATCHER_initialize_class, fi->clazz,
										  0);
  			}
			
			M_ALD(REG_ITMP1, REG_PV, disp);
			switch (fieldtype) {
			case TYPE_INT:
				M_IST(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				M_LST(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_ADR:
				M_AST(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_FLT:
				M_FST(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_DBL:
				M_DST(REG_ZERO, REG_ITMP1, 0);
				break;
			}
			break;


		case ICMD_GETFIELD:   /* ...  ==> ..., value                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD(d, s1, disp);
				break;
			case TYPE_LNG:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_LLD(d, s1, disp);
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
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTFIELD:   /* ..., objectref, value  ==> ...               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
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
				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
				M_IST(s2, s1, disp);
				break;
			case TYPE_LNG:
				M_LST(s2, s1, disp);
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
			}
			break;

		case ICMD_PUTFIELDCONST:  /* ..., objectref  ==> ...                  */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
				M_IST(REG_ZERO, s1, disp);
				break;
			case TYPE_LNG:
				M_LST(REG_ZERO, s1, disp);
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
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			disp = dseg_add_functionptr(cd, asm_handle_exception);
			M_ALD(REG_ITMP2, REG_PV, disp);
			M_JMP(REG_ITMP2_XPC, REG_ITMP2);
			M_NOP;              /* nop ensures that XPC is less than the end */
			                    /* of basic block                            */
			break;

		case ICMD_IFEQ:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPEQ_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPEQ(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFLT:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPLT_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFLE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPLE_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLE(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFNE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPEQ_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPEQ(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFGT:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPLE_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLE(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFGE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i > 0) && (iptr->sx.val.i <= 255))
				M_CMPLT_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_beqz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPEQ_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPEQ(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_LLT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bltz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPLT_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_LLE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_blez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPLE_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLE(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_LNE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bnez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPEQ_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPEQ(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_LGT:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgtz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPLE_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLE(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_LGE:       /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l > 0) && (iptr->sx.val.l <= 255))
					M_CMPLT_IMM(s1, iptr->sx.val.l, REG_ITMP1);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
				}
				emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IF_ICMPEQ:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPEQ:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPEQ(s1, s2, REG_ITMP1);
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_ICMPNE:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPNE:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPEQ(s1, s2, REG_ITMP1);
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_ICMPLT:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPLT:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP1);
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_ICMPGT:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPGT:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLE(s1, s2, REG_ITMP1);
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_ICMPLE:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPLE:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLE(s1, s2, REG_ITMP1);
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_ICMPGE:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPGE:    /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP1);
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
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
			} else if (l <= 32768) {
				M_LDA(REG_ITMP1, s1, -l);
			} else {
				ICONST(REG_ITMP2, l);
				M_ISUB(s1, REG_ITMP2, REG_ITMP1);
			}

			/* number of targets */
			i = i - l + 1;

			/* range check */

			if (i <= 256)
				M_CMPULE_IMM(REG_ITMP1, i - 1, REG_ITMP2);
			else {
				M_LDA(REG_ITMP2, REG_ZERO, i - 1);
				M_CMPULE(REG_ITMP1, REG_ITMP2, REG_ITMP2);
			}
			emit_beqz(cd, table[0].block, REG_ITMP2);

			/* build jump table top down and use address of lowest entry */

			table += i;

			while (--i >= 0) {
				dseg_add_target(cd, table->block); 
				--table;
			}
			}

			/* length of dataseg after last dseg_add_target is used by load */

			M_SAADDQ(REG_ITMP1, REG_PV, REG_ITMP2);
			M_ALD(REG_ITMP2, REG_ITMP2, -(cd->dseglen));
			M_JMP(REG_ZERO, REG_ITMP2);
			ALIGNCODENOP;
			break;

		case ICMD_BUILTIN:      /* ..., arg1, arg2, arg3 ==> ...              */
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL)
				disp = dseg_add_functionptr(cd, bte->fp);
			else
				disp = dseg_add_functionptr(cd, bte->stub);

			M_ALD(REG_PV, REG_PV, disp);  /* Pointer to built-in-function */

			/* generate the actual call */

			M_JSR(REG_RA, REG_PV);
			break;

		case ICMD_INVOKESPECIAL:
			emit_nullpointer_check(cd, iptr, REG_A0);
			/* fall-through */

		case ICMD_INVOKESTATIC: /* ..., [arg1, [arg2 ...]] ==> ...            */
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, um);

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special,
									  um, disp);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_address(cd, lm->stubroutine);
			}

			M_ALD(REG_PV, REG_PV, disp);         /* method pointer in r27 */

			/* generate the actual call */

			M_JSR(REG_RA, REG_PV);
			break;

		case ICMD_INVOKEVIRTUAL:/* op1 = arg count, val.a = method pointer    */
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

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
			M_ALD(REG_PV, REG_METHODPTR, s1);

			/* generate the actual call */

			M_JSR(REG_RA, REG_PV);
			break;

		case ICMD_INVOKEINTERFACE:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				patcher_add_patch_ref(jd, PATCHER_invokeinterface, um, 0);

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
			M_ALD(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
			M_ALD(REG_METHODPTR, REG_METHODPTR, s1);
			M_ALD(REG_PV, REG_METHODPTR, s2);

			/* generate the actual call */

			M_JSR(REG_RA, REG_PV);
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
					emit_label_beqz(cd, BRANCH_LABEL_1, s1);

					disp = dseg_add_unique_s4(cd, 0);         /* super->flags */

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
										  iptr->sx.s23.s3.c.ref,
										  disp);

					M_ILD(REG_ITMP2, REG_PV, disp);
					disp = dseg_add_s4(cd, ACC_INTERFACE);
					M_ILD(REG_ITMP3, REG_PV, disp);
					M_AND(REG_ITMP2, REG_ITMP3, REG_ITMP2);
					emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP2);
				}

				/* interface checkcast code */

				if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						patcher_add_patch_ref(jd,
											  PATCHER_checkcast_interface,
											  iptr->sx.s23.s3.c.ref,
											  0);
					}
					else
						emit_label_beqz(cd, BRANCH_LABEL_3, s1);

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ILD(REG_ITMP3, REG_ITMP2,
						  OFFSET(vftbl_t, interfacetablelength));
					M_LDA(REG_ITMP3, REG_ITMP3, -superindex);
					emit_classcast_check(cd, iptr, BRANCH_LE, REG_ITMP3, s1);

					M_ALD(REG_ITMP3, REG_ITMP2,
						  (s4) (OFFSET(vftbl_t, interfacetable[0]) -
								superindex * sizeof(methodptr*)));
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

						patcher_add_patch_ref(jd,
											  PATCHER_resolve_classref_to_vftbl,
											  iptr->sx.s23.s3.c.ref,
											  disp);
					}
					else {
						disp = dseg_add_address(cd, super->vftbl);

						emit_label_beqz(cd, BRANCH_LABEL_5, s1);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ALD(REG_ITMP3, REG_PV, disp);

					if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_LADD(REG_ITMP1, REG_ITMP2, REG_ITMP1);
						M_ALD(REG_ITMP1, REG_ITMP1, 0);
						M_CMPEQ(REG_ITMP1, REG_ITMP3, REG_ITMP1);
						emit_label_bnez(cd, BRANCH_LABEL_6, REG_ITMP1);  /* good */

						if (super == NULL) {
							M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
							M_CMPEQ_IMM(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
							emit_label_beqz(cd, BRANCH_LABEL_10, REG_ITMP1);  /* throw */
						}

						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
						M_CMPLE(REG_ITMP1, REG_ITMP3, REG_ITMP3);
						emit_label_beqz(cd, BRANCH_LABEL_9, REG_ITMP3);  /* throw */
						/* reload */
						M_ALD(REG_ITMP3, REG_PV, disp);
						M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
						M_S8ADDQ(REG_ITMP1, REG_ITMP2, REG_ITMP2);
						M_ALD(REG_ITMP1, REG_ITMP2, -DISPLAY_SIZE*8);
						M_CMPEQ(REG_ITMP1, REG_ITMP3, REG_ITMP1);
						emit_label_bnez(cd, BRANCH_LABEL_7, REG_ITMP1);  /* good */

						emit_label(cd, BRANCH_LABEL_9);
						if (super == NULL)
							emit_label(cd, BRANCH_LABEL_10);

						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
						M_ALD_INTERN(s1, REG_ZERO, TRAP_ClassCastException);

						emit_label(cd, BRANCH_LABEL_7);
						emit_label(cd, BRANCH_LABEL_6);
						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
					}
					else {
						M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
						M_CMPEQ(REG_ITMP2, REG_ITMP3, REG_ITMP2);
						emit_classcast_check(cd, iptr, BRANCH_EQ, REG_ITMP2, s1);
					}

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

				s1 = emit_load_s1(jd, iptr, REG_A0);
				M_INTMOVE(s1, REG_A0);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd,
										  PATCHER_resolve_classref_to_classinfo,
										  iptr->sx.s23.s3.c.ref,
										  disp);
				}
				else
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

				M_ALD(REG_A1, REG_PV, disp);
				disp = dseg_add_functionptr(cd, BUILTIN_arraycheckcast);
				M_ALD(REG_PV, REG_PV, disp);
				M_JSR(REG_RA, REG_PV);
				disp = (s4) (cd->mcodeptr - cd->mcodebase);
				M_LDA(REG_PV, REG_RA, -disp);

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
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
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_CLR(d);
				emit_label_beqz(cd, BRANCH_LABEL_1, s1);

				disp = dseg_add_unique_s4(cd, 0);             /* super->flags */

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
									  iptr->sx.s23.s3.c.ref, disp);

				M_ILD(REG_ITMP3, REG_PV, disp);

				disp = dseg_add_s4(cd, ACC_INTERFACE);
				M_ILD(REG_ITMP2, REG_PV, disp);
				M_AND(REG_ITMP3, REG_ITMP2, REG_ITMP3);
				emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP3);
			}

			/* interface instanceof code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					/* If d == REG_ITMP2, then it's destroyed in check
					   code above. */
					if (d == REG_ITMP2)
						M_CLR(d);

					patcher_add_patch_ref(jd,
										  PATCHER_instanceof_interface,
										  iptr->sx.s23.s3.c.ref, 0);
				}
				else {
					M_CLR(d);
					emit_label_beqz(cd, BRANCH_LABEL_3, s1);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ILD(REG_ITMP3, REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));
				M_LDA(REG_ITMP3, REG_ITMP3, -superindex);
				M_BLEZ(REG_ITMP3, 2);
				M_ALD(REG_ITMP1, REG_ITMP1,
					  (s4) (OFFSET(vftbl_t, interfacetable[0]) -
							superindex * sizeof(methodptr*)));
				M_CMPULT(REG_ZERO, REG_ITMP1, d);      /* REG_ITMP1 != 0  */

				if (super == NULL)
					emit_label_br(cd, BRANCH_LABEL_4);
				else
					emit_label(cd, BRANCH_LABEL_3);
			}

			/* class instanceof code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);

					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
										  iptr->sx.s23.s3.c.ref,
										  disp);
				}
				else {
					disp = dseg_add_address(cd, supervftbl);

					M_CLR(d);
					emit_label_beqz(cd, BRANCH_LABEL_5, s1);
				}

				M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_ALD(REG_ITMP3, REG_PV, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_LADD(REG_ITMP1, REG_ITMP2, REG_ITMP1);
					M_ALD(REG_ITMP1, REG_ITMP1, 0);
					M_CMPEQ(REG_ITMP1, REG_ITMP3, REG_ITMP1);
					emit_label_beqz(cd, BRANCH_LABEL_8, REG_ITMP1);
					ICONST(d, 1);
					emit_label_br(cd, BRANCH_LABEL_6);  /* true */
					emit_label(cd, BRANCH_LABEL_8);

					if (super == NULL) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_CMPEQ_IMM(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
						emit_label_beqz(cd, BRANCH_LABEL_10, REG_ITMP1);  /* false */
					}

					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));

					M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
					M_CMPLE(REG_ITMP1, REG_ITMP3, REG_ITMP3);
					emit_label_beqz(cd, BRANCH_LABEL_9, REG_ITMP3);  /* false */
					/* reload */
					M_ALD(REG_ITMP3, REG_PV, disp);
					M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
					M_S8ADDQ(REG_ITMP1, REG_ITMP2, REG_ITMP2);
					M_ALD(REG_ITMP1, REG_ITMP2, -DISPLAY_SIZE*8);
					M_CMPEQ(REG_ITMP1, REG_ITMP3, d);

					if (d == REG_ITMP2)
						emit_label_br(cd, BRANCH_LABEL_7);
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
					M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
					M_CMPEQ(REG_ITMP2, REG_ITMP3, d);
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

				var = VAR(iptr->sx.s23.s2.args[s1]);
	
				/* copy SAVEDVAR sizes to stack */

				/* Already Preallocated? */

				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
					M_LST(s2, REG_SP, s1 * 8);
				}
			}

			/* a0 = dimension count */

			ICONST(REG_A0, iptr->s1.argcount);

			/* is patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				disp = dseg_add_unique_address(cd, 0);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									  iptr->sx.s23.s3.c.ref,
									  disp);
			}
			else
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);

			/* a1 = arraydescriptor */

			M_ALD(REG_A1, REG_PV, disp);

			/* a2 = pointer to dimensions = stack pointer */

			M_INTMOVE(REG_SP, REG_A2);

			disp = dseg_add_functionptr(cd, BUILTIN_multianewarray);
			M_ALD(REG_PV, REG_PV, disp);
			M_JSR(REG_RA, REG_PV);
			disp = (s4) (cd->mcodeptr - cd->mcodebase);
			M_LDA(REG_PV, REG_RA, -disp);

			/* check for exception before result assignment */

			emit_exception_check(cd, iptr);

			d = codegen_reg_of_dst(jd, iptr, REG_RESULT);
			M_INTMOVE(REG_RESULT, d);
			emit_store_dst(jd, iptr, d);
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
	int          i, j;
	int          t;
	int          s1, s2;
	int          disp;

	/* get required compiler data */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	/* initialize variables */

	md = m->parseddesc;

	/* calculate stack frame size */

	cd->stackframesize =
		1 +                             /* return address                     */
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +
		sizeof(localref_table) / SIZEOF_VOID_P +
		1 +                             /* methodinfo for call trace          */
		md->paramcount +
		nmd->memuse;

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate stub code */

	M_LDA(REG_SP, REG_SP, -(cd->stackframesize * 8));
	M_AST(REG_RA, REG_SP, cd->stackframesize * 8 - SIZEOF_VOID_P);

#if defined(ENABLE_GC_CACAO)
	/* Save callee saved integer registers in stackframeinfo (GC may
	   need to recover them during a collection). */

	disp = cd->stackframesize * 8 - SIZEOF_VOID_P - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_AST(abi_registers_integer_saved[i], REG_SP, disp + i * 8);
#endif

	/* save integer and float argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s1 = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_LNG:
			case TYPE_ADR:
				M_LST(s1, REG_SP, i * 8);
				break;
			case TYPE_FLT:
				M_FST(s1, REG_SP, i * 8);
				break;
			case TYPE_DBL:
				M_DST(s1, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* prepare data structures for native function call */

	M_MOV(REG_SP, REG_A0);
	M_MOV(REG_PV, REG_A1);
	disp = dseg_add_functionptr(cd, codegen_start_native_call);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	disp = (s4) (cd->mcodeptr - cd->mcodebase);
	M_LDA(REG_PV, REG_RA, -disp);

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_RESULT, REG_ITMP3);

	/* restore integer and float argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s1 = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_LNG:
			case TYPE_ADR:
				M_LLD(s1, REG_SP, i * 8);
				break;
			case TYPE_FLT:
				M_FLD(s1, REG_SP, i * 8);
				break;
			case TYPE_DBL:
				M_DLD(s1, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* copy or spill arguments to new locations */

	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		t = md->paramtypes[i].type;

		if (IS_INT_LNG_TYPE(t)) {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;
				s2 = nmd->params[j].regoff;

				if (!nmd->params[j].inmemory)
					M_INTMOVE(s1, s2);
				else
					M_LST(s1, REG_SP, s2);
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = nmd->params[j].regoff;
				M_LLD(REG_ITMP1, REG_SP, s1);
				M_LST(REG_ITMP1, REG_SP, s2);
			}
		}
		else {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;
				s2 = nmd->params[j].regoff;

				if (!nmd->params[j].inmemory)
					emit_fmove(cd, s1, s2);
				else {
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, s2);
					else
						M_FST(s1, REG_SP, s2);
				}
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = nmd->params[j].regoff;
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

	/* Handle native Java methods. */

	if (m->flags & ACC_NATIVE) {
		/* put class into second argument register */

		if (m->flags & ACC_STATIC)
			M_MOV(REG_ITMP3, REG_A1);

		/* put env into first argument register */

		disp = dseg_add_address(cd, VM::get_current()->get_jnienv());
		M_ALD(REG_A0, REG_PV, disp);
	}

	/* Call the native function. */

	disp = dseg_add_functionptr(cd, f);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);              /* call native method                 */
	disp = (s4) (cd->mcodeptr - cd->mcodebase);
	M_LDA(REG_PV, REG_RA, -disp);       /* recompute pv from ra               */

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
		M_LST(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
		M_FST(REG_FRESULT, REG_SP, 0 * 8);
		break;
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 0 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	/* remove native stackframe info */

	M_MOV(REG_SP, REG_A0);
	M_MOV(REG_PV, REG_A1);
	disp = dseg_add_functionptr(cd, codegen_finish_native_call);
	M_ALD(REG_PV, REG_PV, disp);
	M_JSR(REG_RA, REG_PV);
	disp = (s4) (cd->mcodeptr - cd->mcodebase);
	M_LDA(REG_PV, REG_RA, -disp);
	M_MOV(REG_RESULT, REG_ITMP1_XPTR);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
		M_LLD(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
		M_FLD(REG_FRESULT, REG_SP, 0 * 8);
		break;
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 0 * 8);
		break;
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

#if defined(ENABLE_GC_CACAO)
	/* Restore callee saved integer registers from stackframeinfo (GC
	   might have modified them during a collection). */
  	 
	disp = cd->stackframesize * 8 - SIZEOF_VOID_P - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_ALD(abi_registers_integer_saved[i], REG_SP, disp + i * 8);
#endif

	M_ALD(REG_RA, REG_SP, (cd->stackframesize - 1) * 8); /* get RA            */
	M_LDA(REG_SP, REG_SP, cd->stackframesize * 8);

	/* check for exception */

	M_BNEZ(REG_ITMP1_XPTR, 1);          /* if no exception then return        */
	M_RET(REG_ZERO, REG_RA);            /* return to caller                   */

	/* handle exception */

	M_ASUB_IMM(REG_RA, 4, REG_ITMP2_XPC); /* get exception address            */

	disp = dseg_add_functionptr(cd, asm_handle_nat_exception);
	M_ALD(REG_ITMP3, REG_PV, disp);     /* load asm exception handler address */
	M_JMP(REG_ZERO, REG_ITMP3);         /* jump to asm exception handler      */
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
