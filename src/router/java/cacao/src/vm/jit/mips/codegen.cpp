/* src/vm/jit/mips/codegen.cpp - machine code generator for MIPS

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
#include <stdio.h>

#include "vm/types.hpp"

#include "md-abi.hpp"

#include "vm/jit/mips/arch.hpp"
#include "vm/jit/mips/codegen.hpp"

#include "mm/memory.hpp"

#include "native/localref.hpp"
#include "native/native.hpp"

#include "threads/lock.hpp"

#include "vm/class.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
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
		M_LDA(REG_SP, REG_SP, -cd->stackframesize * 8);

	/* save return address and used callee saved registers */

	p = cd->stackframesize;
	if (!code_is_leafmethod(code)) {
		p--; M_AST(REG_RA, REG_SP, p * 8);
	}
	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_AST(rd->savintregs[i], REG_SP, p * 8);
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
		s1  = md->params[p].regoff;

		if (IS_INT_LNG_TYPE(t)) {                    /* integer args          */
 			if (!md->params[p].inmemory) {           /* register arguments    */
#if SIZEOF_VOID_P == 8
 				if (!(var->flags & INMEMORY))
 					M_INTMOVE(s1, var->vv.regoff);
 				else
 					M_LST(s1, REG_SP, var->vv.regoff);
#else
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
#endif
 			}
			else {                                   /* stack arguments       */
 				if (!(var->flags & INMEMORY)) {
#if SIZEOF_VOID_P == 8
 					M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
#else
					if (IS_2_WORD_TYPE(t))
						M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
					else
						M_ILD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
#endif
 				}
				else
					var->vv.regoff = cd->stackframesize * 8 + s1;
			}
 		}
		else {                                       /* floating args         */
 			if (!md->params[p].inmemory) {
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
 			}
			else {
 				if (!(var->flags & INMEMORY)) {
					if (IS_2_WORD_TYPE(t))
						M_DLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
					else
						M_FLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				}
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
		p--; M_ALD(REG_RA, REG_SP, p * 8);
	}

	/* restore saved registers */

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_ALD(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DLD(rd->savfltregs[i], REG_SP, p * 8);
	}

	/* deallocate stack and return */

	if (cd->stackframesize) {
		int32_t lo, hi, disp;

		disp = cd->stackframesize * 8;
		lo = (short) (disp);
		hi = (short) (((disp) - lo) >> 16);

		if (hi == 0) {
			M_RET(REG_RA);
			M_AADD_IMM(REG_SP, lo, REG_SP);             /* delay slot */
		} else {
			M_LUI(REG_ITMP3,hi);
			M_AADD_IMM(REG_ITMP3,lo,REG_ITMP3);
			M_RET(REG_RA);
			M_AADD(REG_ITMP3,REG_SP,REG_SP);            /* delay slot */
		}

	} else {
		M_RET(REG_RA);
		M_NOP;
	}
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
	unresolved_field*   uf = NULL;
	int32_t             fieldtype;
	int32_t             s1, s2, s3, d = 0;
	int32_t             disp;

	// Get required compiler data.
	codegendata*  cd   = jd->cd;

	switch (iptr->opc) {

		/* constant operations ************************************************/

		case ICMD_LCONST:     /* ...  ==> ..., constant                       */

#if SIZEOF_VOID_P == 8
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
#endif
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

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSUB(REG_ZERO, s1, d);
#else
			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_ISUB(REG_ZERO, GET_LOW_REG(s1), GET_LOW_REG(d));
			M_ISUB(REG_ZERO, GET_HIGH_REG(s1), GET_HIGH_REG(d));
			M_CMPULT(REG_ZERO, GET_LOW_REG(d), REG_ITMP3);
			M_ISUB(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
#if SIZEOF_VOID_P == 8
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_INTMOVE(s1, GET_LOW_REG(d));
			M_ISRA_IMM(GET_LOW_REG(d), 31, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISLL_IMM(s1, 0, d);
#else
			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(GET_LOW_REG(s1), d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#if SIZEOF_VOID_P == 8
			M_LSLL_IMM(s1, 56, d);
			M_LSRA_IMM( d, 56, d);
#else
			M_ISLL_IMM(s1, 24, d);
			M_ISRA_IMM( d, 24, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2CHAR:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s1, 0xffff, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2SHORT:  /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#if SIZEOF_VOID_P == 8
			M_LSLL_IMM(s1, 48, d);
			M_LSRA_IMM( d, 48, d);
#else
			M_ISLL_IMM(s1, 16, d);
			M_ISRA_IMM( d, 16, d);
#endif
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
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -32768) && (iptr->sx.val.i <= 32767))
				M_IADD_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_IADD(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LADD(s1, s2, d);
#else
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_IADD(s1, s2, GET_HIGH_REG(d));
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, GET_LOW_REG(REG_ITMP12_PACKED));
			if (s1 == GET_LOW_REG(d)) {
				M_MOV(s1, REG_ITMP3);
				s1 = REG_ITMP3;
			}
			M_IADD(s1, s2, GET_LOW_REG(d));
			M_CMPULT(GET_LOW_REG(d), s1, REG_ITMP3);
			M_IADD(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                          */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -32768) && (iptr->sx.val.l <= 32767))
				M_LADD_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_LADD(s1, REG_ITMP2, d);
			}
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 32767)) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_IADD_IMM(GET_LOW_REG(s1), iptr->sx.val.l, GET_LOW_REG(d));
				M_CMPULT_IMM(GET_LOW_REG(d), iptr->sx.val.l, REG_ITMP3);
				M_IADD(GET_HIGH_REG(s1), REG_ITMP3, GET_HIGH_REG(d));
			}
			else if ((iptr->sx.val.l >= (-32768 + 1)) && (iptr->sx.val.l < 0)) {
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
				M_ISUB_IMM(s1, -(iptr->sx.val.l), GET_LOW_REG(d));
				M_CMPULT_IMM(GET_LOW_REG(d), iptr->sx.val.l, REG_ITMP3);
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				M_ISUB_IMM(s1, 1, GET_HIGH_REG(d));
				M_IADD(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
			}
			else {
				ICONST(REG_ITMP1, iptr->sx.val.l & 0xffffffff);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				M_IADD(s1, REG_ITMP1, GET_LOW_REG(d));
				M_CMPULT(GET_LOW_REG(d), s1, REG_ITMP3);
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				M_IADD(s1, REG_ITMP3, GET_HIGH_REG(d));
				ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_IADD(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
			}
#endif
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
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -32767) && (iptr->sx.val.i <= 32768))
				M_IADD_IMM(s1, -iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_ISUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSUB(s1, s2, d);
#else
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_ISUB(s1, s2, GET_HIGH_REG(d));
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPULT(s1, s2, REG_ITMP3);
			M_ISUB(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
			/* if s1 is equal to REG_ITMP3 we have to reload it, since
			   the CMPULT instruction destroyed it */
			if (s1 == REG_ITMP3)
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
			M_ISUB(s1, s2, GET_LOW_REG(d));

#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                          */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -32767) && (iptr->sx.val.l <= 32768))
				M_LADD_IMM(s1, -iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_LSUB(s1, REG_ITMP2, d);
			}
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 32768)) {
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
				M_ISUB_IMM(s1, iptr->sx.val.l, GET_LOW_REG(d));
				M_CMPULT_IMM(GET_LOW_REG(d), -(iptr->sx.val.l), REG_ITMP3);
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				M_ISUB_IMM(s1, 1, GET_HIGH_REG(d));
				M_IADD(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
			}
			else if ((iptr->sx.val.l >= -32767) && (iptr->sx.val.l < 0)) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_IADD_IMM(GET_LOW_REG(s1), -(iptr->sx.val.l), GET_LOW_REG(d));
				M_CMPULT_IMM(GET_LOW_REG(d), -(iptr->sx.val.l), REG_ITMP3);
				M_IADD(GET_HIGH_REG(s1), REG_ITMP3, GET_HIGH_REG(d));
			}
			else {
				ICONST(REG_ITMP1, iptr->sx.val.l & 0xffffffff);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				M_ISUB(s1, REG_ITMP1, GET_LOW_REG(d));
				M_CMPULT(s1, REG_ITMP1, REG_ITMP3);
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				M_ISUB(s1, REG_ITMP3, GET_HIGH_REG(d));
				ICONST(REG_ITMP3, iptr->sx.val.l >> 32);
				M_ISUB(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));
			}
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_IMUL(s1, s2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			ICONST(REG_ITMP2, iptr->sx.val.i);
			M_IMUL(s1, REG_ITMP2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LMUL(s1, s2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
#else
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_IMUL(s2, s1);
			M_MFLO(REG_ITMP3);
			M_NOP;
			M_NOP;
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_IMULU(s2, s1);
			M_MFHI(GET_HIGH_REG(d));
			M_MFLO(GET_LOW_REG(d));
			M_NOP;
			M_NOP;
			M_IADD(GET_HIGH_REG(d), REG_ITMP3, REG_ITMP3);

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_IMUL(s1, s2);
			M_MFLO(s2);
			/* XXX do we need nops here? */
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_LMUL(s1, REG_ITMP2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_IDIV(s1, s2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_IDIV(s1, s2);
			M_MFHI(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

#if SIZEOF_VOID_P == 8

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_LDIV(s1, s2);
			M_MFLO(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);
			M_LDIV(s1, s2);
			M_MFHI(d);
			M_NOP;
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

#else /* SIZEOF_VOID_P == 8 */

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */
		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_A0_A1_PACKED);
			s2 = emit_load_s2(jd, iptr, REG_A2_A3_PACKED);

			/* XXX TODO: only do this if arithmetic check is really done! */
			M_OR(GET_HIGH_REG(s2), GET_LOW_REG(s2), REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_LNGMOVE(s1, REG_A0_A1_PACKED);
			M_LNGMOVE(s2, REG_A2_A3_PACKED);

			bte = iptr->sx.s23.s3.bte;
			disp = dseg_add_functionptr(cd, bte->fp);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JSR(REG_RA, REG_ITMP3);
			M_NOP;

			d = codegen_reg_of_dst(jd, iptr, REG_RESULT_PACKED);
			M_LNGMOVE(REG_RESULT_PACKED, d);
			emit_store_dst(jd, iptr, d);
			break;

#endif /* SIZEOF_VOID_P == 8 */

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value << constant       */
		                      /* val.i = constant                             */
		                      
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#if SIZEOF_VOID_P == 8
			M_LSRA_IMM(s1, 63, REG_ITMP2);
			M_LSRL_IMM(REG_ITMP2, 64 - iptr->sx.val.i, REG_ITMP2);
			M_LADD(s1, REG_ITMP2, REG_ITMP2);
			M_LSRA_IMM(REG_ITMP2, iptr->sx.val.i, d);
#else
			M_ISRA_IMM(s1, 31, REG_ITMP2);
			M_ISRL_IMM(REG_ITMP2, 32 - iptr->sx.val.i, REG_ITMP2);
			M_IADD(s1, REG_ITMP2, REG_ITMP2);
			M_ISRA_IMM(REG_ITMP2, iptr->sx.val.i, d);
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 0xffff)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_ISUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.i, d);
			}
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_AND(s1, REG_ITMP3, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_ISUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP3, d);
			}
			M_ISUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

#if SIZEOF_VOID_P == 8

		case ICMD_LDIVPOW2:   /* ..., value  ==> ..., value << constant       */
		                      /* val.i = constant                             */
		                      
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSRA_IMM(s1, 63, REG_ITMP2);
			M_LSRL_IMM(REG_ITMP2, 64 - iptr->sx.val.i, REG_ITMP2);
			M_LADD(s1, REG_ITMP2, REG_ITMP2);
			M_LSRA_IMM(REG_ITMP2, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s1 == d) {
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff)) {
				M_AND_IMM(s1, iptr->sx.val.l, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_LSUB(REG_ZERO, s1, d);
				M_AND_IMM(d, iptr->sx.val.l, d);
			}
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_AND(s1, REG_ITMP2, d);
				M_BGEZ(s1, 4);
				M_NOP;
				M_LSUB(REG_ZERO, s1, d);
				M_AND(d, REG_ITMP2, d);
			}
			M_LSUB(REG_ZERO, d, d);
			emit_store_dst(jd, iptr, d);
			break;

#endif /* SIZEOF_VOID_P == 8 */

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISLL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISLL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISRA(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISRA_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISRL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_ISRL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSLL(s1, s2, d);
#else
			s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
			s2 = emit_load_s2(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);

			M_ISLL(s2, 26, REG_ITMP1);
			M_BGEZ(REG_ITMP1, 3);
			M_NOP;

			M_ISLL(GET_LOW_REG(s1), s2, GET_HIGH_REG(d));
			M_BR(7);
			M_MOV(REG_ZERO, GET_LOW_REG(d));                    /* delay slot */

#if 1
			M_ISLL(GET_LOW_REG(s1), s2, GET_LOW_REG(d));
#endif
			M_BEQZ(REG_ITMP1, 4);
			M_ISLL(GET_HIGH_REG(s1), s2, GET_HIGH_REG(d));      /* delay slot */

			M_ISUB(s2, REG_ZERO, REG_ITMP3);
			M_ISRL(GET_LOW_REG(s1), REG_ITMP3, REG_ITMP3);
			M_OR(GET_HIGH_REG(d), REG_ITMP3, GET_HIGH_REG(d));

#if 0
			M_ISLL(GET_LOW_REG(s1), s2, GET_LOW_REG(d));
#endif
#endif
			emit_store_dst(jd, iptr, d);
			break;

#if SIZEOF_VOID_P == 8

		case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSLL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSRA(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSRA_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSRL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_LSRL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

#endif /* SIZEOF_VOID_P == 8 */

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 0xffff))
				M_AND_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_AND(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_AND(s1, s2, d);
#else
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_AND(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_AND(s1, s2, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                          */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff))
				M_AND_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_AND(s1, REG_ITMP2, d);
			}
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff)) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_AND_IMM(GET_LOW_REG(s1), iptr->sx.val.l, GET_LOW_REG(d));
				M_AND_IMM(GET_HIGH_REG(s1), 0, GET_HIGH_REG(d));
			}
			else {
				LCONST(REG_ITMP12_PACKED, iptr->sx.val.l);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				M_AND(s1, GET_LOW_REG(REG_ITMP12_PACKED), GET_LOW_REG(d));
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
				M_AND(s1, GET_HIGH_REG(REG_ITMP12_PACKED), GET_HIGH_REG(d));
			}
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_OR(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 0xffff))
				M_OR_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_OR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_OR(s1, s2, d);
#else
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_OR(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_OR(s1, s2, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                          */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff))
				M_OR_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_OR(s1, REG_ITMP2, d);
			}
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff)) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_OR_IMM(GET_LOW_REG(s1), iptr->sx.val.l, GET_LOW_REG(d));
				M_OR_IMM(GET_HIGH_REG(s1), 0, GET_HIGH_REG(d));
			}
			else {
				LCONST(REG_ITMP12_PACKED, iptr->sx.val.l);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				M_OR(s1, GET_LOW_REG(REG_ITMP12_PACKED), GET_LOW_REG(d));
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
				M_OR(s1, GET_HIGH_REG(REG_ITMP12_PACKED), GET_HIGH_REG(d));
			}
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

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
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 0xffff))
				M_XOR_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_XOR(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_XOR(s1, s2, d);
#else
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP2);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			M_XOR(s1, s2, GET_LOW_REG(d));
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP3);
			M_XOR(s1, s2, GET_HIGH_REG(d));
#endif
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                          */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff))
				M_XOR_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_XOR(s1, REG_ITMP2, d);
			}
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 0xffff)) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_XOR_IMM(GET_LOW_REG(s1), iptr->sx.val.l, GET_LOW_REG(d));
				M_XOR_IMM(GET_HIGH_REG(s1), 0, GET_HIGH_REG(d));
			}
			else {
				LCONST(REG_ITMP12_PACKED, iptr->sx.val.l);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				M_XOR(s1, GET_LOW_REG(REG_ITMP12_PACKED), GET_LOW_REG(d));
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP3);
				M_XOR(s1, GET_HIGH_REG(REG_ITMP12_PACKED), GET_HIGH_REG(d));
			}
#endif
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_LCMP:       /* ..., val1, val2  ==> ..., val1 cmp val2      */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			M_CMPLT(s2, s1, REG_ITMP1);
			M_LSUB(REG_ITMP1, REG_ITMP3, d);
#else
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			M_CMPLT(s2, s1, REG_ITMP1);
			M_ISUB(REG_ITMP1, REG_ITMP3, d);
			emit_label_bnez(cd, BRANCH_LABEL_1, d);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPULT(s1, s2, REG_ITMP3);
			M_CMPULT(s2, s1, REG_ITMP1);
			M_ISUB(REG_ITMP1, REG_ITMP3, d);
			emit_label(cd, BRANCH_LABEL_1);
#endif
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

#if 0		
		case ICMD_FREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FDIV(s1,s2, REG_FTMP3);
			M_FLOORFL(REG_FTMP3, REG_FTMP3);
			M_CVTLF(REG_FTMP3, REG_FTMP3);
			M_FMUL(REG_FTMP3, s2, REG_FTMP3);
			M_FSUB(s1, REG_FTMP3, d);
			emit_store_dst(jd, iptr, d);
		    break;

		case ICMD_DREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_DDIV(s1,s2, REG_FTMP3);
			M_FLOORDL(REG_FTMP3, REG_FTMP3);
			M_CVTLD(REG_FTMP3, REG_FTMP3);
			M_DMUL(REG_FTMP3, s2, REG_FTMP3);
			M_DSUB(s1, REG_FTMP3, d);
			emit_store_dst(jd, iptr, d);
		    break;
#endif

		case ICMD_I2F:       /* ..., value  ==> ..., (float) value            */
		case ICMD_L2F:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_MOVLD(s1, d);
			M_CVTLF(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2D:       /* ..., value  ==> ..., (double) value           */
		case ICMD_L2D:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_MOVLD(s1, d);
			M_CVTLD(d, d);
			emit_store_dst(jd, iptr, d);
			break;

#if 0
		/* XXX these do not work correctly */

		case ICMD_F2I:       /* ..., (float) value  ==> ..., (int) value      */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_TRUNCFI(s1, REG_FTMP1);
			M_MOVDI(REG_FTMP1, d);
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_D2I:       /* ..., (double) value  ==> ..., (int) value     */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_TRUNCDI(s1, REG_FTMP1);
			M_MOVDI(REG_FTMP1, d);
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_F2L:       /* ..., (float) value  ==> ..., (long) value     */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_TRUNCFL(s1, REG_FTMP1);
			M_MOVDL(REG_FTMP1, d);
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2L:       /* ..., (double) value  ==> ..., (long) value    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_TRUNCDL(s1, REG_FTMP1);
			M_MOVDL(REG_FTMP1, d);
			M_NOP;
			emit_store_dst(jd, iptr, d);
			break;
#endif

		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_CVTFD(s1, d);
			emit_store_dst(jd, iptr, d);
			break;
					
		case ICMD_D2F:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP2);
			M_CVTDF(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

#if SUPPORT_FLOAT_CMP
		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPULEF(s1, s2);
			M_FBT(3);
			M_AADD_IMM(REG_ZERO, 1, d);
			M_BR(4);
			M_NOP;
			M_FCMPEQF(s1, s2);
			M_ASUB_IMM(REG_ZERO, 1, d);
			M_CMOVT(REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPOLTF(s1, s2);
			M_FBF(3);
			M_ASUB_IMM(REG_ZERO, 1, d);
			M_BR(4);
			M_NOP;
			M_FCMPEQF(s1, s2);
			M_AADD_IMM(REG_ZERO, 1, d);
			M_CMOVT(REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;
#endif

#if SUPPORT_DOUBLE_CMP
		case ICMD_DCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPULED(s1, s2);
			M_FBT(3);
			M_AADD_IMM(REG_ZERO, 1, d);
			M_BR(4);
			M_NOP;
			M_FCMPEQD(s1, s2);
			M_ASUB_IMM(REG_ZERO, 1, d);
			M_CMOVT(REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_DCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPOLTD(s1, s2);
			M_FBF(3);
			M_ASUB_IMM(REG_ZERO, 1, d);
			M_BR(4);
			M_NOP;
			M_FCMPEQD(s1, s2);
			M_AADD_IMM(REG_ZERO, 1, d);
			M_CMOVT(REG_ZERO, d);
			emit_store_dst(jd, iptr, d);
			break;
#endif


		/* memory operations **************************************************/

		case ICMD_ARRAYLENGTH: /* ..., arrayref  ==> ..., length              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			M_ILD(d, s1, OFFSET(java_array_t, size));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_BLDS(d, REG_ITMP3, OFFSET(java_bytearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			M_AADD(s2, REG_ITMP3, REG_ITMP3);
			/* implicit null-pointer check */
			M_SLDU(d, REG_ITMP3, OFFSET(java_chararray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP3);
			M_AADD(s2, REG_ITMP3, REG_ITMP3);
			/* implicit null-pointer check */
			M_SLDS(d, REG_ITMP3, OFFSET(java_shortarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_ILD_INTERN(d, REG_ITMP3, OFFSET(java_intarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
#if SIZEOF_VOID_P == 8
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#else
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP12_PACKED);
#endif
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_LLD_INTERN(d, REG_ITMP3, OFFSET(java_longarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_FLD_INTERN(d, REG_ITMP3, OFFSET(java_floatarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_DLD_INTERN(d, REG_ITMP3, OFFSET(java_doublearray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, POINTERSHIFT, REG_ITMP3);
			M_AADD(REG_ITMP3, s1, REG_ITMP3);
			/* implicit null-pointer check */
			M_ALD_INTERN(d, REG_ITMP3, OFFSET(java_objectarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			M_BST(s3, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */
		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			M_AADD(s2, REG_ITMP1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			M_SST(s3, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			M_IST_INTERN(s3, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
#if SIZEOF_VOID_P == 8
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
#else
			s3 = emit_load_s3(jd, iptr, REG_ITMP23_PACKED);
#endif
			/* implicit null-pointer check */
			M_LST_INTERN(s3, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			/* implicit null-pointer check */
			M_FST_INTERN(s3, REG_ITMP1, OFFSET(java_floatarray_t, data[0]));
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			s3 = emit_load_s3(jd, iptr, REG_FTMP1);
			/* implicit null-pointer check */
			M_DST_INTERN(s3, REG_ITMP1, OFFSET(java_doublearray_t, data[0]));
			break;


		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);

			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s3, REG_A1);
			disp = dseg_add_functionptr(cd, BUILTIN_FAST_canstore);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JSR(REG_RA, REG_ITMP3);
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
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			/* implicit null-pointer check */
			M_BST(REG_ZERO, REG_ITMP1, OFFSET(java_bytearray_t, data[0]));
			break;

		case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...            */
		case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_AADD(s2, s1, REG_ITMP1);
			M_AADD(s2, REG_ITMP1, REG_ITMP1);
			/* implicit null-pointer check */
			M_SST(REG_ZERO, REG_ITMP1, OFFSET(java_chararray_t, data[0]));
			break;

		case ICMD_IASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 2, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			/* implicit null-pointer check */
			M_IST_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_intarray_t, data[0]));
			break;

		case ICMD_LASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, 3, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			/* implicit null-pointer check */
#if SIZEOF_VOID_P == 8
			M_LST_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_longarray_t, data[0]));
#else
			M_LST_INTERN(PACK_REGS(REG_ZERO, REG_ZERO), REG_ITMP1, OFFSET(java_longarray_t, data[0]));
#endif
			break;

		case ICMD_AASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_ASLL_IMM(s2, POINTERSHIFT, REG_ITMP2);
			M_AADD(REG_ITMP2, s1, REG_ITMP1);
			/* implicit null-pointer check */
			M_AST_INTERN(REG_ZERO, REG_ITMP1, OFFSET(java_objectarray_t, data[0]));
			break;


		case ICMD_GETSTATIC:  /* ...  ==> ..., value                          */

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
					patcher_add_patch_ref(jd, PATCHER_initialize_class,
										  fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD_INTERN(d, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
#if SIZEOF_VOID_P == 8
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#else
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP23_PACKED);
#endif
				M_LLD_INTERN(d, REG_ITMP1, 0);
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

				patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz))
					patcher_add_patch_ref(jd, PATCHER_initialize_class,
										  fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_IST_INTERN(s1, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
#if SIZEOF_VOID_P == 8
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
#else
				s1 = emit_load_s1(jd, iptr, REG_ITMP23_PACKED);
#endif
				M_LST_INTERN(s1, REG_ITMP1, 0);
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
					patcher_add_patch_ref(jd, PATCHER_initialize_class,
										  fi->clazz, disp);
  			}

			M_ALD(REG_ITMP1, REG_PV, disp);

			switch (fieldtype) {
			case TYPE_INT:
				M_IST_INTERN(REG_ZERO, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
				M_LST_INTERN(REG_ZERO, REG_ITMP1, 0);
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

			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_ILD(d, s1, disp);
				break;
			case TYPE_LNG:
#if SIZEOF_VOID_P == 8
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
				M_LLD(d, s1, disp);
#else
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP23_PACKED);
				M_LLD_GETFIELD(d, s1, disp);
#endif
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
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;
			}

#if SIZEOF_VOID_P == 8
			if (IS_INT_LNG_TYPE(fieldtype))
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			else
				s2 = emit_load_s2(jd, iptr, REG_FTMP1);
#else
			if (IS_INT_LNG_TYPE(fieldtype)) {
				if (IS_2_WORD_TYPE(fieldtype))
					s2 = emit_load_s2(jd, iptr, REG_ITMP23_PACKED);
				else
					s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			}
			else
				s2 = emit_load_s2(jd, iptr, REG_FTMP2);
#endif

			if (INSTRUCTION_IS_UNRESOLVED(iptr))
				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);

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
			default:
				assert(false);
				break;
			}
			break;

		case ICMD_PUTFIELDCONST:  /* ..., objectref  ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			emit_nullpointer_check(cd, iptr, s1);

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
			default:
				assert(false);
				break;
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			// Some processor implementations seem to have a problem when using
			// the JALR instruction with (reg_dest == reg_src), so avoid that.
			disp = dseg_add_functionptr(cd, asm_handle_exception);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JSR(REG_ITMP2_XPC, REG_ITMP3);
			M_NOP;
			M_NOP;              /* nop ensures that XPC is less than the end */
			                    /* of basic block                            */
			break;

		case ICMD_IFEQ:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			ICONST(REG_ITMP2, iptr->sx.val.i);
			emit_beq(cd, iptr->dst.block, s1, REG_ITMP2);
			break;

		case ICMD_IFLT:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i >= -32768) && (iptr->sx.val.i <= 32767))
				M_CMPLT_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IFLE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i >= -32769) && (iptr->sx.val.i <= 32766)) {
				M_CMPLT_IMM(s1, iptr->sx.val.i + 1, REG_ITMP1);
				emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			}
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPGT(s1, REG_ITMP2, REG_ITMP1);
				emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IFNE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			ICONST(REG_ITMP2, iptr->sx.val.i);
			emit_bne(cd, iptr->dst.block, s1, REG_ITMP2);
			break;

		case ICMD_IFGT:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i >= -32769) && (iptr->sx.val.i <= 32766)) {
				M_CMPLT_IMM(s1, iptr->sx.val.i + 1, REG_ITMP1);
				emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			}
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPGT(s1, REG_ITMP2, REG_ITMP1);
				emit_bnez(cd, iptr->dst.block, REG_ITMP1);
			}
			break;

		case ICMD_IFGE:         /* ..., value ==> ...                         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i >= -32768) && (iptr->sx.val.i <= 32767))
				M_CMPLT_IMM(s1, iptr->sx.val.i, REG_ITMP1);
			else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP1);
			}
			emit_beqz(cd, iptr->dst.block, REG_ITMP1);
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_beqz(cd, iptr->dst.block, s1);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				emit_beq(cd, iptr->dst.block, s1, REG_ITMP2);
			}
#else
			if (iptr->sx.val.l == 0) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_OR(GET_LOW_REG(s1), GET_HIGH_REG(s1), REG_ITMP3);
				emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_XOR(s1, REG_ITMP2, REG_ITMP2);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_XOR(s1, REG_ITMP3, REG_ITMP3);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP3);
				emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			}
#endif
			break;

		case ICMD_IF_LLT:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bltz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -32768) && (iptr->sx.val.l <= 32767))
					M_CMPLT_IMM(s1, iptr->sx.val.l, REG_ITMP3);
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLT(s1, REG_ITMP2, REG_ITMP3);
				}
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			}
#else
			if (iptr->sx.val.l == 0) {
				/* if high word is less than zero, the whole long is too */
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				emit_bltz(cd, iptr->dst.block, s1);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label_bne(cd, BRANCH_LABEL_1, s1, REG_ITMP2);
				s2 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				ICONST(REG_ITMP2, iptr->sx.val.l & 0xffffffff);
				M_CMPULT(s2, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label(cd, BRANCH_LABEL_1);
			}
#endif
			break;

		case ICMD_IF_LLE:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_blez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -32769) && (iptr->sx.val.l <= 32766)) {
					M_CMPLT_IMM(s1, iptr->sx.val.l + 1, REG_ITMP2);
					emit_bnez(cd, iptr->dst.block, REG_ITMP2);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPGT(s1, REG_ITMP2, REG_ITMP3);
					emit_beqz(cd, iptr->dst.block, REG_ITMP3);
				}
			}
#else
			if (iptr->sx.val.l == 0) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				emit_label_bgtz(cd, BRANCH_LABEL_1, GET_HIGH_REG(s1));
				emit_bltz(cd, iptr->dst.block, GET_HIGH_REG(s1));
				emit_beqz(cd, iptr->dst.block, GET_LOW_REG(s1));	
				emit_label(cd, BRANCH_LABEL_1);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_CMPLT(s1, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label_bne(cd, BRANCH_LABEL_1, s1, REG_ITMP2);
				s2 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				ICONST(REG_ITMP2, iptr->sx.val.l & 0xffffffff);
				M_CMPUGT(s2, REG_ITMP2, REG_ITMP3);
				emit_beqz(cd, iptr->dst.block, REG_ITMP3);
				emit_label(cd, BRANCH_LABEL_1);
			}
#endif
			break;

		case ICMD_IF_LNE:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bnez(cd, iptr->dst.block, s1);
			else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				emit_bne(cd, iptr->dst.block, s1, REG_ITMP2);
			}
#else
			if (iptr->sx.val.l == 0) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				M_OR(GET_LOW_REG(s1), GET_HIGH_REG(s1), REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_XOR(s1, REG_ITMP2, REG_ITMP2);
				s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP3, iptr->sx.val.l & 0xffffffff);
				M_XOR(s1, REG_ITMP3, REG_ITMP3);
				M_OR(REG_ITMP2, REG_ITMP3, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			}
#endif
			break;

		case ICMD_IF_LGT:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgtz(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -32769) && (iptr->sx.val.l <= 32766)) {
					M_CMPLT_IMM(s1, iptr->sx.val.l + 1, REG_ITMP2);
					emit_beqz(cd, iptr->dst.block, REG_ITMP2);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPGT(s1, REG_ITMP2, REG_ITMP3);
					emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				}
			}
#else
			if (iptr->sx.val.l == 0) {
				s1 = emit_load_s1(jd, iptr, REG_ITMP12_PACKED);
				emit_bgtz(cd, iptr->dst.block, GET_HIGH_REG(s1));
				emit_label_bltz(cd, BRANCH_LABEL_1, GET_HIGH_REG(s1));
				emit_bnez(cd, iptr->dst.block, GET_LOW_REG(s1));
				emit_label(cd, BRANCH_LABEL_1);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_CMPGT(s1, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label_bne(cd, BRANCH_LABEL_1, s1, REG_ITMP2);
				s2 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				ICONST(REG_ITMP2, iptr->sx.val.l & 0xffffffff);
				M_CMPUGT(s2, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label(cd, BRANCH_LABEL_1);
			}
#endif
			break;

		case ICMD_IF_LGE:       /* ..., value ==> ...                         */

#if SIZEOF_VOID_P == 8
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.l == 0)
				emit_bgez(cd, iptr->dst.block, s1);
			else {
				if ((iptr->sx.val.l >= -32768) && (iptr->sx.val.l <= 32767)) {
					M_CMPLT_IMM(s1, iptr->sx.val.l, REG_ITMP3);
				}
				else {
					LCONST(REG_ITMP2, iptr->sx.val.l);
					M_CMPLT(s1, REG_ITMP2, REG_ITMP3);
				}
				emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			}
#else
			if (iptr->sx.val.l == 0) {
				/* if high word is greater equal zero, the whole long is too */
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				emit_bgez(cd, iptr->dst.block, s1);
			}
			else {
				s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
				ICONST(REG_ITMP2, iptr->sx.val.l >> 32);
				M_CMPGT(s1, REG_ITMP2, REG_ITMP3);
				emit_bnez(cd, iptr->dst.block, REG_ITMP3);
				emit_label_bne(cd, BRANCH_LABEL_1, s1, REG_ITMP2);
				s2 = emit_load_s1_low(jd, iptr, REG_ITMP3);
				ICONST(REG_ITMP2, iptr->sx.val.l & 0xffffffff);
				M_CMPULT(s2, REG_ITMP2, REG_ITMP3);
				emit_beqz(cd, iptr->dst.block, REG_ITMP3);
				emit_label(cd, BRANCH_LABEL_1);
			}
#endif
			break;

#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPEQ:
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_beq(cd, iptr->dst.block, s1, s2);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPEQ:    /* ..., value, value ==> ...                  */
		                        /* op1 = target JavaVM pc                     */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			emit_label_bne(cd, BRANCH_LABEL_1, s1, s2);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			emit_beq(cd, iptr->dst.block, s1, s2);
			emit_label(cd, BRANCH_LABEL_1);
			break;
#endif

#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPNE:
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_bne(cd, iptr->dst.block, s1, s2);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPNE:    /* ..., value, value ==> ...                  */

			/* TODO: could be optimized (XOR or SUB) */
			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			emit_bne(cd, iptr->dst.block, s1, s2);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			emit_bne(cd, iptr->dst.block, s1, s2);
			break;
#endif

		case ICMD_IF_ICMPLT:    /* ..., value, value ==> ...                  */
#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPLT:    /* op1 = target JavaVM pc                     */
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPLT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_label_bnez(cd, BRANCH_LABEL_1, REG_ITMP3);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPULT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			emit_label(cd, BRANCH_LABEL_1);
			break;
#endif

		case ICMD_IF_ICMPGT:    /* ..., value, value ==> ...                  */
#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPGT:    /* op1 = target JavaVM pc                     */
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPGT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_label_bnez(cd, BRANCH_LABEL_1, REG_ITMP3);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPUGT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			emit_label(cd, BRANCH_LABEL_1);
			break;
#endif

		case ICMD_IF_ICMPLE:    /* ..., value, value ==> ...                  */
#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPLE:    /* op1 = target JavaVM pc                     */
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPLE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_label_bnez(cd, BRANCH_LABEL_1, REG_ITMP3);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPUGT(s1, s2, REG_ITMP3);
			emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			emit_label(cd, BRANCH_LABEL_1);
			break;
#endif

		case ICMD_IF_ICMPGE:    /* ..., value, value ==> ...                  */
#if SIZEOF_VOID_P == 8
		case ICMD_IF_LCMPGE:    /* op1 = target JavaVM pc                     */
#endif

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			break;

#if SIZEOF_VOID_P == 4
		case ICMD_IF_LCMPGE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1_high(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_high(jd, iptr, REG_ITMP2);
			M_CMPGT(s1, s2, REG_ITMP3);
			emit_bnez(cd, iptr->dst.block, REG_ITMP3);
			M_CMPLT(s1, s2, REG_ITMP3);
			emit_label_bnez(cd, BRANCH_LABEL_1, REG_ITMP3);
			s1 = emit_load_s1_low(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2_low(jd, iptr, REG_ITMP2);
			M_CMPULT(s1, s2, REG_ITMP3);
			emit_beqz(cd, iptr->dst.block, REG_ITMP3);
			emit_label(cd, BRANCH_LABEL_1);
			break;
#endif

		case ICMD_TABLESWITCH:  /* ..., index ==> ...                         */
			{
			s4 i, l;
			branch_target_t *table;

			table = iptr->dst.table;

			l = iptr->sx.s23.s2.tablelow;
			i = iptr->sx.s23.s3.tablehigh;

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (l == 0)
				{M_INTMOVE(s1, REG_ITMP1);}
			else if (l <= 32768) {
				M_IADD_IMM(s1, -l, REG_ITMP1);
				}
			else {
				ICONST(REG_ITMP2, l);
				M_ISUB(s1, REG_ITMP2, REG_ITMP1);
				}

			/* number of targets */
			i = i - l + 1;

			/* range check */

			M_CMPULT_IMM(REG_ITMP1, i, REG_ITMP2);
			emit_beqz(cd, table[0].block, REG_ITMP2);

			/* build jump table top down and use address of lowest entry */

			table += i;

			while (--i >= 0) {
				dseg_add_target(cd, table->block); 
				--table;
			}
			}

			/* length of dataseg after last dseg_add_target is used by load */

			M_ASLL_IMM(REG_ITMP1, POINTERSHIFT, REG_ITMP1);
			M_AADD(REG_ITMP1, REG_PV, REG_ITMP2);
			M_ALD(REG_ITMP2, REG_ITMP2, -(cd->dseglen));
			M_JMP(REG_ITMP2);
			M_NOP;
			ALIGNCODENOP;
			break;

		case ICMD_BUILTIN:
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL) {
				disp = dseg_add_functionptr(cd, bte->fp);
				M_ALD(REG_ITMP3, REG_PV, disp);  /* built-in-function pointer */

				/* generate the actual call */

				/* TWISTI: i actually don't know the reason for using
				   REG_ITMP3 here instead of REG_PV. */

				M_JSR(REG_RA, REG_ITMP3);
				M_NOP;
			}
			else {
				disp = dseg_add_functionptr(cd, bte->stub);
				M_ALD(REG_PV, REG_PV, disp);          /* method pointer in pv */

				/* generate the actual call */

				M_JSR(REG_RA, REG_PV);
				M_NOP;
			}
			break;

		case ICMD_INVOKESPECIAL:
			emit_nullpointer_check(cd, iptr, REG_A0);
			/* fall through */

		case ICMD_INVOKESTATIC:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, um);

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special, um,
									  disp);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_address(cd, lm->stubroutine);
			}

			M_ALD(REG_PV, REG_PV, disp);          /* method pointer in pv */

			/* generate the actual call */

			M_JSR(REG_RA, REG_PV);
			M_NOP;
			break;

		case ICMD_INVOKEVIRTUAL:
			emit_nullpointer_check(cd, iptr, REG_A0);

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
			M_NOP;
			break;

		case ICMD_INVOKEINTERFACE:
			emit_nullpointer_check(cd, iptr, REG_A0);

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
			M_NOP;
			break;



		case ICMD_CHECKCAST:  /* ..., objectref ==> ..., objectref            */

			if (!(iptr->flags.bits & INS_FLAG_ARRAY)) {
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

					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_s4(cd, 0);         /* super->flags */

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
										  cr, disp);

					M_ILD(REG_ITMP2, REG_PV, disp);
					M_AND_IMM(REG_ITMP2, ACC_INTERFACE, REG_ITMP2);
					emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP2);
				}

				/* interface checkcast code */

				if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						constant_classref *cr = iptr->sx.s23.s3.c.ref;

						patcher_add_patch_ref(jd, PATCHER_checkcast_interface,
											  cr, 0);
					}
					else {
						emit_label_beqz(cd, BRANCH_LABEL_3, s1);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ILD(REG_ITMP3, REG_ITMP2,
						  OFFSET(vftbl_t, interfacetablelength));
					M_IADD_IMM(REG_ITMP3, -superindex, REG_ITMP3);
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

						patcher_add_patch_ref(jd,
											  PATCHER_resolve_classref_to_vftbl,
											  cr, disp);
					}
					else {
						disp = dseg_add_address(cd, super->vftbl);

						emit_label_beqz(cd, BRANCH_LABEL_5, s1);
					}

					// The following code checks whether object s is a subtype of class t.
					// Represents the following semantic:
					//    if (!fast_subtype_check(s->vftbl, t->vftbl)) throw;

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ALD(REG_ITMP3, REG_PV, disp);

					if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						// Represents the following semantic:
						//    if (*(s->vftbl + t->vftbl->subtype_offset) == t->vftbl) good;
						// Preconditions:
						//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_AADD(REG_ITMP1, REG_ITMP2, REG_ITMP1);
						M_ALD(REG_ITMP1, REG_ITMP1, 0);
						emit_label_beq(cd, BRANCH_LABEL_6, REG_ITMP1, REG_ITMP3);  /* good */

						// Represents the following semantic:
						//    if (t->vftbl->subtype_offset != OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE])) throw;
						// Preconditions:
						//    REG_ITMP3==t->vftbl;
						if (super == NULL) {
							M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
							M_ISUB_IMM(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
							emit_label_bnez(cd, BRANCH_LABEL_9, REG_ITMP1);  /* throw */
						}

						// Represents the following semantic:
						//    if (s->vftbl->subtype_depth < t->vftbl->subtype_depth) throw;
						// Preconditions:
						//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
						M_ILD(REG_ITMP1, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
						M_ILD(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_CMPULT(REG_ITMP1, REG_ITMP3, REG_ITMP1);
						emit_label_bnez(cd, BRANCH_LABEL_8, REG_ITMP1);  /* throw */

						// Represents the following semantic:
						//    if (s->vftbl->subtype_overflow[t->vftbl->subtype_depth - DISPLAY_SIZE] != t->vftbl) throw;
						// Preconditions:
						//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl->subtype_depth;
						M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
						M_ASLL_IMM(REG_ITMP3, POINTERSHIFT, REG_ITMP3);
						M_AADD(REG_ITMP2, REG_ITMP3, REG_ITMP2);
						M_ALD(REG_ITMP2, REG_ITMP2, -DISPLAY_SIZE * SIZEOF_VOID_P);
						M_ALD(REG_ITMP3, REG_PV, disp);  /* reload REG_ITMP3, was destroyed */
						emit_label_beq(cd, BRANCH_LABEL_7, REG_ITMP2, REG_ITMP3);  /* good */

						// Throw case
						emit_label(cd, BRANCH_LABEL_8);
						if (super == NULL)
							emit_label(cd, BRANCH_LABEL_9);
						emit_load_s1(jd, iptr, REG_ITMP1);  /* reload s1, might have been destroyed */
						M_ALD_INTERN(s1, REG_ZERO, TRAP_ClassCastException);

						// Good case
						emit_label(cd, BRANCH_LABEL_7);
						emit_label(cd, BRANCH_LABEL_6);
						emit_load_s1(jd, iptr, REG_ITMP1);  /* reload s1, might have been destroyed */
					}
					else {
						// Represents the following semantic:
						//    if (*(s->vftbl + t->vftbl->subtype_offset) != t->vftbl) throw;
						// Preconditions:
						//    REG_ITMP2==s->vftbl; REG_ITMP3==t->vftbl;
						M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
						M_BEQ(REG_ITMP2, REG_ITMP3, 2);
						M_NOP;  /* delay slot */
						M_ALD_INTERN(s1, REG_ZERO, TRAP_ClassCastException);
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
				s1 = emit_load_s1(jd, iptr, REG_A0);
				M_INTMOVE(s1, REG_A0);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd,
										  PATCHER_resolve_classref_to_classinfo,
										  cr, disp);
				}
				else {
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
				}

				M_ALD(REG_A1, REG_PV, disp);
				disp = dseg_add_functionptr(cd, BUILTIN_arraycheckcast);
				M_ALD(REG_ITMP3, REG_PV, disp);
				M_JSR(REG_RA, REG_ITMP3);
				M_NOP;

				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				emit_classcast_check(cd, iptr, ICMD_IFEQ, REG_RESULT, s1);

				d = codegen_reg_of_dst(jd, iptr, s1);
			}

			M_INTMOVE(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INSTANCEOF: /* ..., objectref ==> ..., intresult            */

			{
			classinfo *super;
			s4         superindex;

			super = iptr->sx.s23.s3.c.cls;

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
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			M_CLR(d);

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				emit_label_beqz(cd, BRANCH_LABEL_1, s1);

				constant_classref *cr = iptr->sx.s23.s3.c.ref;
				disp = dseg_add_unique_s4(cd, 0);             /* super->flags */

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
									  cr, disp);

				M_ILD(REG_ITMP3, REG_PV, disp);
				M_AND_IMM(REG_ITMP3, ACC_INTERFACE, REG_ITMP3);
				emit_label_beqz(cd, BRANCH_LABEL_2, REG_ITMP3);
			}

			/* interface instanceof code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				if (super == NULL) {
					patcher_add_patch_ref(jd, PATCHER_instanceof_interface,
										  iptr->sx.s23.s3.c.ref, 0);
				}
				else {
					emit_label_beqz(cd, BRANCH_LABEL_3, s1);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ILD(REG_ITMP3, REG_ITMP1,
					  OFFSET(vftbl_t, interfacetablelength));
				M_IADD_IMM(REG_ITMP3, -superindex, REG_ITMP3);
				M_BLEZ(REG_ITMP3, 3);
				M_NOP;
				M_ALD(REG_ITMP1, REG_ITMP1,
					  OFFSET(vftbl_t, interfacetable[0]) -
					  superindex * sizeof(methodptr*));
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

					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_address(cd, NULL);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
										  cr, disp);
				}
				else {
					disp = dseg_add_address(cd, super->vftbl);

					emit_label_beqz(cd, BRANCH_LABEL_5, s1);
				}

				// The following code checks whether object s is a subtype of class t.
				// Represents the following semantic:
				//    fast_subtype_check(s->vftbl, t->vftbl));

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ALD(REG_ITMP3, REG_PV, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						// Represents the following semantic:
						//    if (*(s->vftbl + t->vftbl->subtype_offset) == t->vftbl) true;
						// Preconditions:
						//    REG_ITMP1==s->vftbl; REG_ITMP3==t->vftbl;
						M_ILD(REG_ITMP2, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_AADD(REG_ITMP2, REG_ITMP1, REG_ITMP2);
						M_ALD(REG_ITMP2, REG_ITMP2, 0);
						emit_label_beq(cd, BRANCH_LABEL_6, REG_ITMP2, REG_ITMP3);  /* true */

						// Represents the following semantic:
						//    if (t->vftbl->subtype_offset != OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE])) false;
						// Preconditions:
						//    REG_ITMP3==t->vftbl;
						if (super == NULL) {
							M_ILD(REG_ITMP2, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
							M_ISUB_IMM(REG_ITMP2, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP2);
							emit_label_bnez(cd, BRANCH_LABEL_9, REG_ITMP2);  /* false */
						}

						// Represents the following semantic:
						//    if (s->vftbl->subtype_depth < t->vftbl->subtype_depth) false;
						// Preconditions:
						//    REG_ITMP1==s->vftbl; REG_ITMP3==t->vftbl;
						M_ILD(REG_ITMP2, REG_ITMP1, OFFSET(vftbl_t, subtype_depth));
						M_ILD(REG_ITMP3, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_CMPULT(REG_ITMP2, REG_ITMP3, REG_ITMP2);
						emit_label_bnez(cd, BRANCH_LABEL_8, REG_ITMP2);  /* false */

						// Represents the following semantic:
						//    if (s->vftbl->subtype_overflow[t->vftbl->subtype_depth - DISPLAY_SIZE] != t->vftbl) false;
						// Preconditions:
						//    REG_ITMP1==s->vftbl; REG_ITMP3==t->vftbl->subtype_depth;
						M_ALD(REG_ITMP1, REG_ITMP1, OFFSET(vftbl_t, subtype_overflow));
						M_ASLL_IMM(REG_ITMP3, POINTERSHIFT, REG_ITMP3);
						M_AADD(REG_ITMP1, REG_ITMP3, REG_ITMP1);
						M_ALD(REG_ITMP1, REG_ITMP1, -DISPLAY_SIZE * SIZEOF_VOID_P);
						M_ALD(REG_ITMP3, REG_PV, disp);  /* reload REG_ITMP3, was destroyed */
						emit_label_bne(cd, BRANCH_LABEL_7, REG_ITMP1, REG_ITMP3);  /* false */

						// True case
						emit_label(cd, BRANCH_LABEL_6);
						M_MOV(1, d);
						if (d == REG_ITMP2) {
							M_BR(2);  /* branch over M_CLR */
							M_NOP;  /* delay slot */
						}

						// False (fall-through) case
						emit_label(cd, BRANCH_LABEL_7);
						emit_label(cd, BRANCH_LABEL_8);
						if (super == NULL)
							emit_label(cd, BRANCH_LABEL_9);
						if (d == REG_ITMP2)
							M_CLR(d);  /* if d == REG_ITMP2, it was destroyed */
				}
				else {
					// Represents the following semantic:
					//    *(s->vftbl + t->vftbl->subtype_offset) == t->vftbl;
					// Preconditions:
					//    REG_ITMP1==s->vftbl; REG_ITMP3==t->vftbl;
					M_ALD(REG_ITMP1, REG_ITMP1, super->vftbl->subtype_offset);
					M_XOR(REG_ITMP1, REG_ITMP3, d);
					M_CMPULT_IMM(d, 1, d);
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

				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
#if SIZEOF_VOID_P == 8
					M_LST(s2, REG_SP, s1 * 8);
#else
					M_IST(s2, REG_SP, (s1 + 2) * 8);
#endif
				}
			}

			/* a0 = dimension count */

			ICONST(REG_A0, iptr->s1.argcount);

			/* is patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				constant_classref *cr = iptr->sx.s23.s3.c.ref;
				disp = dseg_add_unique_address(cd, NULL);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									  cr, disp);
			}
			else {
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
			}

			/* a1 = arraydescriptor */

			M_ALD(REG_A1, REG_PV, disp);

			/* a2 = pointer to dimensions = stack pointer */

#if SIZEOF_VOID_P == 8
			M_MOV(REG_SP, REG_A2);
#else
			M_AADD_IMM(REG_SP, 4*4, REG_A2);
#endif

			disp = dseg_add_functionptr(cd, BUILTIN_multianewarray);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_JSR(REG_RA, REG_ITMP3);
			M_NOP;

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
	s4           i, j;
	s4           t;
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
		md->paramcount +                /* for saving arguments over calls    */
#if SIZEOF_VOID_P == 4
		5 +                             /* additional save space (MIPS32)     */
#endif
		1 +                             /* for saving return address          */
		nmd->memuse;

	/* adjust stackframe size for 16-byte alignment */

	if (cd->stackframesize & 1)
		cd->stackframesize++;

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate stub code */

	M_LDA(REG_SP, REG_SP, -cd->stackframesize * 8); /* build up stackframe    */
	M_AST(REG_RA, REG_SP, (cd->stackframesize - 1) * 8); /* store RA          */

	/* save integer and float argument registers */

#if SIZEOF_VOID_P == 8
	for (i = 0, j = 0; i < md->paramcount && i < INT_ARG_CNT; i++) {
		if (IS_INT_LNG_TYPE(md->params[i].type)) {
			s1 = md->params[i].regoff;
			M_AST(s1, REG_SP, j * 8);
			j++;
		}
	}
#else
	for (i = 0, j = 5; i < md->paramcount && i < INT_ARG_CNT; i++) {
		if (IS_INT_LNG_TYPE(md->params[i].type)) {
			if (!md->params[i].inmemory) {
 				s1 = md->params[i].regoff;

				if (IS_2_WORD_TYPE(md->params[i].type))
					M_LST(s1, REG_SP, j * 8);
				else
					M_IST(s1, REG_SP, j * 8);

				j++;
			}
		}
	}
#endif

	for (i = 0; i < md->paramcount && i < FLT_ARG_CNT; i++) {
		if (IS_FLT_DBL_TYPE(md->params[i].type)) {
			s1 = md->params[i].regoff;

			if (IS_2_WORD_TYPE(md->params[i].type))
				M_DST(s1, REG_SP, j * 8);
			else
				M_FST(s1, REG_SP, j * 8);

			j++;
		}
	}

	/* prepare data structures for native function call */

	M_MOV(REG_SP, REG_A0);
	M_MOV(REG_PV, REG_A1);
	disp = dseg_add_functionptr(cd, codegen_start_native_call);
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_JSR(REG_RA, REG_ITMP3);
	M_NOP; /* XXX fill me! */

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_RESULT, REG_ITMP3);

	/* restore integer and float argument registers */

#if SIZEOF_VOID_P == 8
	for (i = 0, j = 0; i < md->paramcount && i < INT_ARG_CNT; i++) {
		if (IS_INT_LNG_TYPE(md->params[i].type)) {
			s1 = md->params[i].regoff;
			M_LLD(s1, REG_SP, j * 8);
			j++;
		}
	}
#else
	for (i = 0, j = 5; i < md->paramcount && i < INT_ARG_CNT; i++) {
		if (IS_INT_LNG_TYPE(md->params[i].type)) {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;

				if (IS_2_WORD_TYPE(md->params[i].type))
					M_LLD(s1, REG_SP, j * 8);
				else
					M_ILD(s1, REG_SP, j * 8);

				j++;
			}
		}
	}
#endif

	for (i = 0; i < md->paramcount && i < FLT_ARG_CNT; i++) {
		if (IS_FLT_DBL_TYPE(md->params[i].type)) {
			s1 = md->params[i].regoff;

			if (IS_2_WORD_TYPE(md->params[i].type))
				M_DLD(s1, REG_SP, j * 8);
			else
				M_FLD(s1, REG_SP, j * 8);

			j++;
		}
	}

	/* copy or spill arguments to new locations */

	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		t = md->params[i].type;

		if (IS_INT_LNG_TYPE(t)) {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;
				s2 = nmd->params[j].regoff;

				if (!nmd->params[j].inmemory) {
#if SIZEOF_VOID_P == 8
					M_INTMOVE(s1, s2);
#else
					if (IS_2_WORD_TYPE(t))
						M_LNGMOVE(s1, s2);
					else
						M_INTMOVE(s1, s2);
#endif
				}
				else {
#if SIZEOF_VOID_P == 8
					M_LST(s1, REG_SP, s2);
#else
					if (IS_2_WORD_TYPE(t))
						M_LST(s1, REG_SP, s2);
					else
						M_IST(s1, REG_SP, s2);
#endif
				}
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = nmd->params[j].regoff;

#if SIZEOF_VOID_P == 8
				M_LLD(REG_ITMP1, REG_SP, s1);
				M_LST(REG_ITMP1, REG_SP, s2);
#else
				if (IS_2_WORD_TYPE(t)) {
					M_LLD(REG_ITMP12_PACKED, REG_SP, s1);
					M_LST(REG_ITMP12_PACKED, REG_SP, s2);
				}
				else {
					M_ILD(REG_ITMP1, REG_SP, s1);
					M_IST(REG_ITMP1, REG_SP, s2);
				}
#endif
			}
		}
		else {
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;
				s2 = nmd->params[j].regoff;

				if (!nmd->params[j].inmemory) {
#if SIZEOF_VOID_P == 8
					if (IS_2_WORD_TYPE(t))
						M_DMOV(s1, s2);
					else
						M_FMOV(s1, s2);
#else
					/* On MIPS32 float arguments for native functions
					   can never be in float argument registers, since
					   the first argument is _always_ an integer
					   argument (JNIEnv) */

					if (IS_2_WORD_TYPE(t)) {
						/* double high/low order is endian
						   independent: even numbered holds low
						   32-bits, odd numbered high 32-bits */

						M_MFC1(GET_LOW_REG(s2), s1);           /* low 32-bits */
						M_MFC1(GET_HIGH_REG(s2), s1 + 1);     /* high 32-bits */
					}
					else
						M_MFC1(s2, s1);
#endif
				}
				else {
#if SIZEOF_VOID_P == 8
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, s2);
					else
						M_FST(s1, REG_SP, s2);
#else
					/* s1 may have been originally in 2 int registers,
					   but was moved out by the native function
					   argument(s), just get low register */

					if (IS_2_WORD_TYPE(t))
						M_DST(GET_LOW_REG(s1), REG_SP, s2);
					else
						M_FST(GET_LOW_REG(s1), REG_SP, s2);
#endif
				}
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = nmd->params[j].regoff;

#if SIZEOF_VOID_P == 8
				if (IS_2_WORD_TYPE(t)) {
					M_DLD(REG_FTMP1, REG_SP, s1);
					M_DST(REG_FTMP1, REG_SP, s2);
				}
				else {
					M_FLD(REG_FTMP1, REG_SP, s1);
					M_FST(REG_FTMP1, REG_SP, s2);
				}
#else
				if (IS_2_WORD_TYPE(t)) {
					M_DLD(REG_FTMP1, REG_SP, s1);
					M_DST(REG_FTMP1, REG_SP, s2);
				}
				else {
					M_FLD(REG_FTMP1, REG_SP, s1);
					M_FST(REG_FTMP1, REG_SP, s2);
				}
#endif
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
	M_ALD(REG_ITMP3, REG_PV, disp);     /* load adress of native method       */
	M_JSR(REG_RA, REG_ITMP3);           /* call native method                 */
	M_NOP;                              /* delay slot                         */

	/* save return value */

	switch (md->returntype.type) {
#if SIZEOF_VOID_P == 8
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
		M_LST(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 0 * 8);
		break;
#else
	case TYPE_INT:
	case TYPE_ADR:
		M_IST(REG_RESULT, REG_SP, 2*4 + 0 * 8);
		break;
	case TYPE_LNG:
		M_LST(REG_RESULT_PACKED, REG_SP, 2*4 + 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DST(REG_FRESULT, REG_SP, 2*4 + 0 * 8);
		break;
#endif
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
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_JSR(REG_RA, REG_ITMP3);
	M_NOP; /* XXX fill me! */
	M_MOV(REG_RESULT, REG_ITMP1_XPTR);

	/* restore return value */

	switch (md->returntype.type) {
#if SIZEOF_VOID_P == 8
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
		M_LLD(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 0 * 8);
		break;
#else
	case TYPE_INT:
	case TYPE_ADR:
		M_ILD(REG_RESULT, REG_SP, 2*4 + 0 * 8);
		break;
	case TYPE_LNG:
		M_LLD(REG_RESULT_PACKED, REG_SP, 2*4 + 0 * 8);
		break;
	case TYPE_FLT:
	case TYPE_DBL:
		M_DLD(REG_FRESULT, REG_SP, 2*4 + 0 * 8);
		break;
#endif
	case TYPE_VOID:
		break;
	default:
		assert(false);
		break;
	}

	M_ALD(REG_RA, REG_SP, (cd->stackframesize - 1) * 8); /* load RA           */

	/* check for exception */

	M_BNEZ(REG_ITMP1_XPTR, 2);          /* if no exception then return        */
	M_LDA(REG_SP, REG_SP, cd->stackframesize * 8); /* DELAY SLOT              */

	M_RET(REG_RA);                      /* return to caller                   */
	M_NOP;                              /* DELAY SLOT                         */

	/* handle exception */
	
	disp = dseg_add_functionptr(cd, asm_handle_nat_exception);
	M_ALD(REG_ITMP3, REG_PV, disp);     /* load asm exception handler address */
	M_JMP(REG_ITMP3);                   /* jump to asm exception handler      */
	M_ASUB_IMM(REG_RA, 4, REG_ITMP2_XPC); /* get exception address (DELAY)    */

	/* Generate patcher traps. */

	emit_patcher_traps(jd);
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
