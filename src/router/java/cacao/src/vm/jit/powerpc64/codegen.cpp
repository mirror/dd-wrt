/* src/vm/jit/powerpc64/codegen.cpp - machine code generator for 64-bit PowerPC

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
#include <stdint.h>
#include <signal.h>

#include "vm/types.hpp"

#include "md-abi.hpp"

#include "vm/jit/powerpc64/arch.hpp"
#include "vm/jit/powerpc64/codegen.hpp"

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
#include "vm/jit/abi-asm.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/asmpart.hpp"
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

	if (!code_is_leafmethod(code)) {
		M_MFLR(REG_ZERO);
		M_AST(REG_ZERO, REG_SP, LA_LR_OFFSET);
	}

	if (cd->stackframesize)
		M_STDU(REG_SP, REG_SP, -cd->stackframesize * 8);

	/* save return address and used callee saved registers */

	p = cd->stackframesize;
	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_LST(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p --; M_DST(rd->savfltregs[i], REG_SP, p * 8);
	}

	/* take arguments out of register or stack frame */

	md = m->parseddesc;

 	for (p = 0, l = 0; p < md->paramcount; p++) {
 		t = md->paramtypes[p].type;
 		varindex = jd->local_map[l*5 + t];
 		l++;
 		if (IS_2_WORD_TYPE(t))    /* increment local counter for 2 word types */
 			l++;
 		if (varindex == jitdata::UNUSED)
 			continue;

		var = VAR(varindex);
		s1  = md->params[p].regoff;

		if (IS_INT_LNG_TYPE(t)) {
 			if (!md->params[p].inmemory) {
				if (!IS_INMEMORY(var->flags))
					M_INTMOVE(s1, var->vv.regoff);
				else
					M_LST(s1, REG_SP, var->vv.regoff);
			}
			else {
 				if (!IS_INMEMORY(var->flags))
					M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1);
				else
					var->vv.regoff = cd->stackframesize * 8 + s1;
			}
		}
		else {
 			if (!md->params[p].inmemory) {
 				if (!IS_INMEMORY(var->flags))
 					emit_fmove(cd, s1, var->vv.regoff);
				else
					if (IS_2_WORD_TYPE(t))
						M_DST(s1, REG_SP, var->vv.regoff);
					else
						M_FST(s1, REG_SP, var->vv.regoff);
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
		/* ATTENTION: Don't use REG_ZERO (r0) here, as M_ALD
		   may have a displacement overflow. */

		M_ALD(REG_ITMP1, REG_SP, p * 8 + LA_LR_OFFSET);
		M_MTLR(REG_ITMP1);
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

	M_RET;
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
	unresolved_field*   uf = NULL;      // prevent warning
	int32_t             fieldtype;
	int32_t             s1, s2, s3, d;
	int32_t             disp;

	// Get required compiler data.
	codeinfo*     code = jd->code;
	codegendata*  cd   = jd->cd;

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
				disp = dseg_add_unique_address(cd, iptr->sx.val.c.ref);
				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									iptr->sx.val.c.ref,
								    disp);
			} else	{
				disp = dseg_add_address(cd, iptr->sx.val.anyptr);
			}
			M_ALD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;


		/* integer operations *************************************************/

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_NEG(s1, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LNEG:    
			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_NEG(s1, d);
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
			M_ISEXT(s1, d);	
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_BSEXT(s1, d);
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
			M_IADD(s1, s2, d);
			M_EXTSW(d,d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IINC:
		case ICMD_IADDCONST:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			if ((iptr->sx.val.i >= -32768) && (iptr->sx.val.i <= 32767)) {
				M_IADD_IMM(s1, iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_IADD(s1, REG_ITMP2, d);
			}
			M_EXTSW(d,d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_LADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* XXX check me */
			if ((iptr->sx.val.l >= -32768) && (iptr->sx.val.l <= 32767)) {
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
			M_SUB(s1, s2, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUBCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -32767) && (iptr->sx.val.i <= 32768)) {
				M_IADD_IMM(s1, -iptr->sx.val.i, d);
			} else {
				ICONST(REG_ITMP2, iptr->sx.val.i);
				M_SUB(s1, REG_ITMP2, d);
			}
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_SUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			/* XXX check me */
			if ((iptr->sx.val.l >= -32767) && (iptr->sx.val.l <= 32767)) {
				M_LADD_IMM(s1, -iptr->sx.val.l, d);
			} else {
				LCONST(REG_ITMP2, iptr->sx.val.l);
				M_SUB(s1, REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, s2);

			M_DIV(s1, s2, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, s2);

			M_DIV(s1, s2, d);
			/* we need to test if divident was 0x8000000000000, bit OV is set in XER in this case */
			/* we only need to check this if we did a LDIV, not for IDIV */
			M_MFXER(REG_ITMP2);
			M_ANDIS(REG_ITMP2, 0x4000, REG_ITMP2);	/* test OV */
			M_BLE(1);
			M_MOV(s1, d);				/* java specs says result == dividend */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREM:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);

			M_DIV(s1, s2,  REG_ITMP3);	
			M_MUL(REG_ITMP3, s2, REG_ITMP2);
			M_SUB(s1, REG_ITMP2,  REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			M_MOV(REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arithmetic_check(cd, iptr, s2);

			M_DIV(s1, s2,  REG_ITMP3);	
			/* we need to test if divident was 0x8000000000000, bit OV is set in XER in this case */
			/* we only need to check this if we did a LDIV, not for IDIV */
			M_MFXER(REG_ITMP2);
			M_ANDIS(REG_ITMP2, 0x4000, REG_ITMP2);	/* test OV */
			M_BLE(2); 
			LCONST(REG_ITMP3, 0);			/* result == 0 in this case */
			M_BR(2 + (s2==REG_ITMP2));
			/* might require a reload */
			emit_load_s2(jd, iptr, REG_ITMP2);
			M_MUL(REG_ITMP3, s2, REG_ITMP2);
			M_SUB(s1, REG_ITMP2,  REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			M_MOV(REG_ITMP3, d);
			emit_store_dst(jd, iptr, d);
			break;

		
		case ICMD_IMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MUL(s1, s2, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMUL:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_MUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= -32768) && (iptr->sx.val.i <= 32767))
				M_MUL_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_MUL(s1, REG_ITMP3, d);
			}
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LMULCONST:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= -32767) && (iptr->sx.val.l <= 32767))
				M_MUL_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP3, iptr->sx.val.l);
				M_MUL(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value << constant       */
		      
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_SRA_IMM(s1, iptr->sx.val.i, d);
			M_ADDZE(d, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x1f, REG_ITMP3);
			M_SLL(s1, REG_ITMP3, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL_IMM(s1, iptr->sx.val.i & 0x1f, d);
			M_EXTSW(d,d);
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
			M_AND_IMM(s2, 0x1f, REG_ITMP2);
			M_MOV(s1, REG_ITMP1);
			M_CLR_HIGH(REG_ITMP1);
			M_SRL(REG_ITMP1, REG_ITMP2, d);
			M_EXTSW(d,d);	/* for the case it was shift 0 bits */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (iptr->sx.val.i & 0x1f) {
				M_MOV(s1, REG_ITMP1);
				M_CLR_HIGH(REG_ITMP1);
				M_SRA_IMM(REG_ITMP1, iptr->sx.val.i & 0x1f, d);
			} else {
				M_INTMOVE(s1, d);
			}
			emit_store_dst(jd, iptr, d);
			break;
	
		case ICMD_LSHLCONST:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SLL_IMM(s1, iptr->sx.val.i & 0x3f, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LSHL:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x3f, REG_ITMP2);
			M_SLL(s1, REG_ITMP2, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LSHRCONST:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRA_IMM(s1, iptr->sx.val.i & 0x3f, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LSHR:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x3f, REG_ITMP2);
			M_SRA(s1, REG_ITMP2, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LUSHRCONST:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_SRL_IMM(s1, iptr->sx.val.i & 0x3f, d);
			emit_store_dst(jd, iptr, d);
			break;
		case ICMD_LUSHR:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_AND_IMM(s2, 0x3f, REG_ITMP2);
			M_SRL(s1, REG_ITMP2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_AND(s1, s2, d);
/*			M_EXTSW(d, d);*/
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LAND:
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_AND(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 65535)) {
				M_AND_IMM(s1, iptr->sx.val.i, d);
				}
			/*
			else if (iptr->sx.val.i == 0xffffff) {
				M_RLWINM(s1, 0, 8, 31, d);
				}
			*/
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_AND(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 65535))
				M_AND_IMM(s1, iptr->sx.val.l, d);
			/*
			else if (iptr->sx.val.l == 0xffffff) {
				M_RLWINM(s1, 0, 8, 31, d);
				}
			*/
			else {
				LCONST(REG_ITMP3, iptr->sx.val.l);
				M_AND(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.i = constant                             */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
#if 0
			/* fast division, result in REG_ITMP3) */
			M_SRA_IMM(s1, iptr->sx.val.i, REG_ITMP3);
			M_ADDZE(REG_ITMP3, REG_ITMP3);

			M_SUB(s1, REG_ITMP3, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;
#else
			
			M_MOV(s1, REG_ITMP2);
			M_CMPI(s1, 0);
			M_BGE(1 + 3*(iptr->sx.val.i >= 32768));
			if (iptr->sx.val.i >= 32768) {
				M_ADDIS(REG_ZERO, iptr->sx.val.i >> 16, REG_ITMP2);
				M_EXTSW(REG_ITMP2, REG_ITMP2);
				M_OR_IMM(REG_ITMP2, iptr->sx.val.i, REG_ITMP2);
				M_IADD(s1, REG_ITMP2, REG_ITMP2);
			} else {
				M_IADD_IMM(s1, iptr->sx.val.i, REG_ITMP2);
			}
			{
				int b=0, m = iptr->sx.val.i;
				while (m >>= 1)
					++b;
				M_RLWINM(REG_ITMP2, 0, 0, 30-b, REG_ITMP2);
			}
			M_SUB(s1, REG_ITMP2, d);
			M_EXTSW(d, d);
			emit_store_dst(jd, iptr, d);
			break;
#endif

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_OR(s1, s2, d);
/*			M_EXTSW(d,d);*/
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LOR:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_OR(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 65535))
				M_OR_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_OR(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 65535))
				M_OR_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP3, iptr->sx.val.l);
				M_OR(s1, REG_ITMP3, d);
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
			if ((iptr->sx.val.i >= 0) && (iptr->sx.val.i <= 65535))
				M_XOR_IMM(s1, iptr->sx.val.i, d);
			else {
				ICONST(REG_ITMP3, iptr->sx.val.i);
				M_XOR(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if ((iptr->sx.val.l >= 0) && (iptr->sx.val.l <= 65535))
				M_XOR_IMM(s1, iptr->sx.val.l, d);
			else {
				LCONST(REG_ITMP3, iptr->sx.val.l);
				M_XOR(s1, REG_ITMP3, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LCMP:       /* ..., val1, val2  ==> ..., val1 cmp val2      */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* XXX implement me!!! */
			vm_abort("codegen: implement ICMD_LCMP!");
			emit_store_dst(jd, iptr, d);
			break;
			break;


		/* floating operations ************************************************/

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FMOVN(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FMOVN(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_DADD(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_DSUB(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_DMUL(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_FDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_DDIV(s1, s2, d);
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */
		case ICMD_D2I:

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_CLR(d);
			disp = dseg_add_float(cd, 0.0);
			M_FLD(REG_FTMP2, REG_PV, disp);
			M_FCMPU(s1, REG_FTMP2);
			M_BNAN(4);
			disp = dseg_add_unique_s4(cd, 0);
			M_CVTDL_C(s1, REG_FTMP1);
			M_LDA(REG_ITMP1, REG_PV, disp);
			M_STFIWX(REG_FTMP1, 0, REG_ITMP1);
			M_ILD(d, REG_PV, disp);
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_F2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			emit_fmove(cd, s1, d);
			emit_store_dst(jd, iptr, d);
			break;
					
		case ICMD_D2F:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			M_CVTDF(s1, d);
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */
		case ICMD_DCMPL:      /* == => 0, < => 1, > => -1                     */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPU(s2, s1);
			M_IADD_IMM(REG_ZERO, -1, d);
			M_BNAN(4);
			M_BGT(3);
			M_IADD_IMM(REG_ZERO, 0, d);
			M_BGE(1);
			M_IADD_IMM(REG_ZERO, 1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */
		case ICMD_DCMPG:      /* == => 0, < => 1, > => -1                     */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_FCMPU(s1, s2);
			M_IADD_IMM(REG_ZERO, 1, d);
			M_BNAN(4);
			M_BGT(3);
			M_IADD_IMM(REG_ZERO, 0, d);
			M_BGE(1);
			M_IADD_IMM(REG_ZERO, -1, d);
			emit_store_dst(jd, iptr, d);
			break;

			
		/* memory operations **************************************************/

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_IADD_IMM(s2, OFFSET(java_bytearray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LBZX(d, s1, REG_ITMP2);
			M_BSEXT(d, d);
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 1, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_chararray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LHZX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 1, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_shortarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LHAX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 2, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_intarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LWAX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, PACK_REGS(REG_ITMP2, REG_ITMP1));
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD(s1, REG_ITMP2, REG_ITMP2);
			/* implicit null-pointer check */
			M_LLD_INTERN(d, REG_ITMP2, OFFSET(java_longarray_t, data[0]));
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 2, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_floatarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LFSX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_doublearray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LFDX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_objectarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_ALDX(d, s1, REG_ITMP2);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_IADD_IMM(s2, OFFSET(java_bytearray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_STBX(s3, s1, REG_ITMP2);
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SLL_IMM(s2, 1, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_chararray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_STHX(s3, s1, REG_ITMP2);
			break;

		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SLL_IMM(s2, 1, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_shortarray_t, data[0]), REG_ITMP2);
			M_STHX(s3, s1, REG_ITMP2);
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SLL_IMM(s2, 2, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_intarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_STWX(s3, s1, REG_ITMP2);
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_longarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_LSTX(s3, s1, REG_ITMP2);
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			M_SLL_IMM(s2, 2, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_floatarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_STFSX(s3, s1, REG_ITMP2);
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_doublearray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_STFDX(s3, s1, REG_ITMP2);
			break;

		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_A0);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_A1);

			disp = dseg_add_functionptr(cd, BUILTIN_FAST_canstore);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_ALD(REG_ITMP3, REG_ITMP3, 0); /* TOC */
			M_MTCTR(REG_ITMP3);

			M_INTMOVE(s1, REG_A0);
			M_INTMOVE(s3, REG_A1);

			M_JSR;
			emit_arraystore_check(cd, iptr);

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			M_SLL_IMM(s2, 3, REG_ITMP2);
			M_IADD_IMM(REG_ITMP2, OFFSET(java_objectarray_t, data[0]), REG_ITMP2);
			/* implicit null-pointer check */
			M_ASTX(s3, s1, REG_ITMP2);
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
			default:
				assert(false);
				break;
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTFIELD:   /* ..., value  ==> ...                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

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

			if (IS_INT_LNG_TYPE(fieldtype)) {
				s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			}
			else
				s2 = emit_load_s2(jd, iptr, REG_FTMP2);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
			}


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
			default:
				assert(false);
				break;
			}
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			disp = dseg_add_functionptr(cd, asm_handle_exception);
			M_ALD(REG_ITMP2, REG_PV, disp);
			M_MTCTR(REG_ITMP2);

			if (code_is_leafmethod(code))
				M_MFLR(REG_ITMP3);                          /* save LR        */

			M_BL(0);                                        /* get current PC */
			M_MFLR(REG_ITMP2_XPC);

			if (code_is_leafmethod(code))
				M_MTLR(REG_ITMP3);                          /* restore LR     */

			M_RTS;                                          /* jump to CTR    */
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_beq(cd, iptr->dst.block);
			break;
		case ICMD_IF_LLT:       /* ..., value ==> ...                         */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_blt(cd, iptr->dst.block);
			break;
		case ICMD_IF_LLE:       /* ..., value ==> ...                         */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_ble(cd, iptr->dst.block);
			break;

		case ICMD_IF_LNE:       /* ..., value ==> ... */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_bne(cd, iptr->dst.block);
			break;
		case ICMD_IF_LGE:       /* ..., value ==> ... */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_bge(cd, iptr->dst.block);
			break;
		case ICMD_IF_LGT:       /* ..., value ==> ...                         */
			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			LCONST(REG_ITMP2, iptr->sx.val.l);
			M_CMP(s1, REG_ITMP2);
			emit_bgt(cd, iptr->dst.block);
			break;
		case ICMD_IF_LCMPEQ:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_beq(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPNE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bne(cd, iptr->dst.block);
			break;


		case ICMD_IF_LCMPLT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_blt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGT:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bgt(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPLE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_ble(cd, iptr->dst.block);
			break;

		case ICMD_IF_LCMPGE:    /* ..., value, value ==> ...                  */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_CMP(s1, s2);
			emit_bge(cd, iptr->dst.block);
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
				M_SUB(s1, REG_ITMP2, REG_ITMP1);
			}

			/* number of targets */
			i = i - l + 1;

			/* range check */

			M_CMPUI(REG_ITMP1, i - 1);
			emit_bgt(cd, table[0].block);

			/* build jump table top down and use address of lowest entry */

			table += i;

			while (--i >= 0) {
				dseg_add_target(cd, table->block); 
				--table;
			}

			/* length of dataseg after last dseg_add_unique_target is used by load */

			M_SLL_IMM(REG_ITMP1, 3, REG_ITMP1);
			M_IADD(REG_ITMP1, REG_PV, REG_ITMP2);
			M_ALD(REG_ITMP2, REG_ITMP2, -(cd->dseglen));
			M_MTCTR(REG_ITMP2);
			M_RTS;
			ALIGNCODENOP;
			}
			break;


		case ICMD_BUILTIN:      /* ..., [arg1, [arg2 ...]] ==> ...            */
			bte = iptr->sx.s23.s3.bte;
			if (bte->stub == NULL) {
				disp = dseg_add_functionptr(cd, bte->fp);
				M_ALD(REG_PV, REG_PV, disp);
				M_ALD(REG_PV, REG_PV, 0);	/* TOC */
			}
			else {
				disp = dseg_add_functionptr(cd, bte->stub);
				M_ALD(REG_PV, REG_PV, disp);
			}

			/* generate the actual call */
			M_MTCTR(REG_PV);
			M_JSR;
			break;

		case ICMD_INVOKESPECIAL:/* ..., objectref, [arg1, [arg2 ...]] ==> ... */
			emit_nullpointer_check(cd, iptr, REG_A0);
			/* fall through */

		case ICMD_INVOKESTATIC: /* ..., [arg1, [arg2 ...]] ==> ...            */
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, um);

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special,
									um, disp);
			} else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_address(cd, lm->stubroutine);
			}
			M_ALD(REG_PV, REG_PV, disp);

			/* generate the actual call */

			M_MTCTR(REG_PV);
			M_JSR;
			break;

		case ICMD_INVOKEVIRTUAL:/* op1 = arg count, val.a = method pointer    */
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				patcher_add_patch_ref(jd, PATCHER_invokevirtual, um, 0);
				s1 = 0;
			} else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, table[0]) +
					sizeof(methodptr) * lm->vftblindex;
			}

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
			M_ALD(REG_PV, REG_METHODPTR, s1);

			/* generate the actual call */

			M_MTCTR(REG_PV);
			M_JSR;
			break;

		case ICMD_INVOKEINTERFACE:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				patcher_add_patch_ref(jd, PATCHER_invokeinterface, um, 0);

				s1 = 0;
				s2 = 0;

			} else {
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

			M_MTCTR(REG_PV);
			M_JSR;
			break;

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
					M_TST(s1);
					emit_label_beq(cd, BRANCH_LABEL_1);
					disp = dseg_add_unique_s4(cd, 0);                     /* super->flags */

					patcher_add_patch_ref(jd,
										PATCHER_resolve_classref_to_flags,
										iptr->sx.s23.s3.c.ref,
										disp);

					M_ILD(REG_ITMP2, REG_PV, disp);
					M_AND_IMM(REG_ITMP2, ACC_INTERFACE, REG_ITMP2);

					emit_label_beq(cd, BRANCH_LABEL_2);
				}

				/* interface checkcast code */

				if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						patcher_add_patch_ref(jd,
											PATCHER_checkcast_interface,
											iptr->sx.s23.s3.c.ref,
											0);
					} else {
						M_TST(s1);
						emit_label_beq(cd, BRANCH_LABEL_3);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, interfacetablelength));
					M_LDATST(REG_ITMP3, REG_ITMP3, -superindex);
					emit_classcast_check(cd, iptr, BRANCH_LE, REG_ITMP3, s1);
					M_ALD(REG_ITMP3, REG_ITMP2,
						  OFFSET(vftbl_t, interfacetable[0]) -
						  superindex * sizeof(methodptr*));
					M_TST(REG_ITMP3);
					emit_classcast_check(cd, iptr, BRANCH_EQ, REG_ITMP3, s1);

					if (super == NULL)	{
						emit_label_br(cd, BRANCH_LABEL_4);
					} else	{
						emit_label(cd, BRANCH_LABEL_3);
					}
				}

				/* class checkcast code */

				if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {
					if (super == NULL) {
						emit_label(cd, BRANCH_LABEL_2);

						disp = dseg_add_unique_address(cd, NULL);
						patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
											iptr->sx.s23.s3.c.ref,
											disp);
					} else {
						disp = dseg_add_address(cd, super->vftbl);
						M_TST(s1);
						emit_label_beq(cd, BRANCH_LABEL_5);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ALD(REG_ITMP3, REG_PV, disp);

					if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_LADD(REG_ITMP1, REG_ITMP2, REG_ITMP1);
						M_ALD(REG_ITMP1, REG_ITMP1, 0);
						M_CMP(REG_ITMP1, REG_ITMP3);
						emit_label_beq(cd, BRANCH_LABEL_6);  /* good */

						if (super == NULL) {
							M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
							M_CMPI(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]));
							emit_label_bne(cd, BRANCH_LABEL_10);  /* throw */
						}

						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
						M_CMP(REG_ITMP1, REG_ITMP3);
						emit_label_bgt(cd, BRANCH_LABEL_9);  /* throw */
						/* reload */
						M_ALD(REG_ITMP3, REG_PV, disp);
						M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));

						M_SLL_IMM(REG_ITMP1, 3, REG_ITMP1);
						M_IADD_IMM(REG_ITMP2, -DISPLAY_SIZE*8, REG_ITMP2);
						M_ALDX(REG_ITMP1, REG_ITMP2, REG_ITMP1);
						M_CMP(REG_ITMP1, REG_ITMP3);
						emit_label_beq(cd, BRANCH_LABEL_7);  /* good */

						emit_label(cd, BRANCH_LABEL_9);
						if (super == NULL)
							emit_label(cd, BRANCH_LABEL_10);

						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
						M_LWZ(s1, REG_ZERO, TRAP_ClassCastException);

						emit_label(cd, BRANCH_LABEL_7);
						emit_label(cd, BRANCH_LABEL_6);
						/* reload s1, might have been destroyed */
						emit_load_s1(jd, iptr, REG_ITMP1);
					}
					else {
						M_ALD(REG_ITMP2, REG_ITMP2, super->vftbl->subtype_offset);
						M_CMP(REG_ITMP2, REG_ITMP3);
						emit_classcast_check(cd, iptr, BRANCH_NE, REG_ITMP2, s1);
					}

					if (super != NULL)
						emit_label(cd, BRANCH_LABEL_5);
				}

				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_1);
					emit_label(cd, BRANCH_LABEL_4);
				}
				d = codegen_reg_of_dst(jd, iptr, s1);

			} else {
				/* array type cast-check */

				s1 = emit_load_s1(jd, iptr, REG_A0);
				M_INTMOVE(s1, REG_A0);


				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					disp = dseg_add_unique_address(cd, NULL);
					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
										iptr->sx.s23.s3.c.ref,
										disp);
				} else {
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
				}

				M_ALD(REG_A1, REG_PV, disp);
				disp = dseg_add_functionptr(cd, BUILTIN_arraycheckcast);
				M_ALD(REG_ITMP2, REG_PV, disp);
				M_ALD(REG_ITMP2, REG_ITMP2, 0);	/* TOC */
				M_MTCTR(REG_ITMP2);
				M_JSR;
				M_TST(REG_RESULT);
				s1 = emit_load_s1(jd, iptr, REG_ITMP1);
				emit_classcast_check(cd, iptr, BRANCH_EQ, REG_RESULT, s1);

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
				M_MOV(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_TST(s1);
				M_CLR(d);
				emit_label_beq(cd, BRANCH_LABEL_1);
				disp = dseg_add_unique_s4(cd, 0);                     /* super->flags */

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
									iptr->sx.s23.s3.c.ref, disp);

				M_ILD(REG_ITMP3, REG_PV, disp);
				M_AND_IMM(REG_ITMP3, ACC_INTERFACE, REG_ITMP3);
				emit_label_beq(cd, BRANCH_LABEL_2);
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

				} else {
					M_TST(s1);
					M_CLR(d);
					emit_label_beq(cd, BRANCH_LABEL_3);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));
				M_ILD(REG_ITMP3, REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));
				M_LDATST(REG_ITMP3, REG_ITMP3, -superindex);
				M_BLE(3);
				M_ALD(REG_ITMP1, REG_ITMP1,
					  OFFSET(vftbl_t, interfacetable[0]) -
					  superindex * sizeof(methodptr*));
				/* This seems to be the canonical sequence to emulate
				 * the Alpha instruction M_CMPULT(zero,x) (check for non-null). */
				M_ADDIC(REG_ITMP1, -1, d);
				M_SUBE(REG_ITMP1, d, d);

				if (super == NULL)	{
					emit_label_br(cd, BRANCH_LABEL_4);
				} else	{
					emit_label(cd, BRANCH_LABEL_3);
				}
			}

			/* class instanceof code */

			if ((super == NULL) || !(super->flags & ACC_INTERFACE)) {

				if (super == NULL) {
					emit_label(cd, BRANCH_LABEL_2);

					disp = dseg_add_unique_address(cd, NULL);
					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_vftbl,
										iptr->sx.s23.s3.c.ref,
										disp);

				} else {
					disp = dseg_add_address(cd, super->vftbl);

					M_TST(s1);
					M_CLR(d);
					emit_label_beq(cd, BRANCH_LABEL_5);
				}

				M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_ALD(REG_ITMP3, REG_PV, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_LADD(REG_ITMP1, REG_ITMP2, REG_ITMP1);
					M_ALD(REG_ITMP1, REG_ITMP1, 0);
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_label_bne(cd, BRANCH_LABEL_8);
					ICONST(d, 1);
					emit_label_br(cd, BRANCH_LABEL_6);  /* true */
					emit_label(cd, BRANCH_LABEL_8);

					if (super == NULL) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_CMPI(REG_ITMP1, OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]));
						emit_label_bne(cd, BRANCH_LABEL_10);  /* false */
					}

					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));

					M_ILD(REG_ITMP3, REG_ITMP2, OFFSET(vftbl_t, subtype_depth));
					M_CMP(REG_ITMP1, REG_ITMP3);
					emit_label_bgt(cd, BRANCH_LABEL_9);  /* false */
					/* reload */
					M_ALD(REG_ITMP3, REG_PV, disp);
					M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));

					M_SLL_IMM(REG_ITMP1, 3, REG_ITMP1);
					M_IADD_IMM(REG_ITMP2, -DISPLAY_SIZE*8, REG_ITMP2);
					M_ALDX(REG_ITMP1, REG_ITMP2, REG_ITMP1);
					/* This seems to be the canonical sequence to emulate
					 * the Alpha instruction M_CMPEQ. */
					M_XOR(REG_ITMP1, REG_ITMP3, d);
					M_CNTLZ(d, d);
					M_RLDICL(d, 58, 6, d);

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
					M_XOR(REG_ITMP2, REG_ITMP3, d);
					M_CNTLZ(d, d);
					M_RLDICL(d, 58, 6, d);
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

			MCODECHECK((iptr->s1.argcount << 2) + 128);

			for (s1 = iptr->s1.argcount; --s1 >= 0; ) {

				var = VAR(iptr->sx.s23.s2.args[s1]);

				/* copy SAVEDVAR sizes to stack */

				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
#if defined(__DARWIN__)
					M_LST(s2, REG_SP, LA_SIZE + (s1 + INT_ARG_CNT) * 8);
#else
					M_LST(s2, REG_SP, LA_SIZE + (s1 + 3) * 8);
#endif
				}
			}

			/* a0 = dimension count */

			ICONST(REG_A0, iptr->s1.argcount);

			/* is patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				disp = dseg_add_unique_address(cd, NULL);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									iptr->sx.s23.s3.c.ref, disp);
			} else {
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
			}

			/* a1 = arraydescriptor */

			M_ALD(REG_A1, REG_PV, disp);

			/* a2 = pointer to dimensions = stack pointer */

#if defined(__DARWIN__)
			M_LDA(REG_A2, REG_SP, LA_SIZE + INT_ARG_CNT * 8);
#else
			M_LDA(REG_A2, REG_SP, LA_SIZE + 3 * 8);
#endif

			disp = dseg_add_functionptr(cd, BUILTIN_multianewarray);
			M_ALD(REG_ITMP3, REG_PV, disp);
			M_ALD(REG_ITMP3, REG_ITMP3, 0); /* TOC */
			M_MTCTR(REG_ITMP3);
			M_JSR;

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

	/* Sanity check. */

	assert(!code_is_leafmethod(code));

	/* set some variables */

	md = m->parseddesc;

	/* calculate stackframe size */

	cd->stackframesize =
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +
		sizeof(localref_table) / SIZEOF_VOID_P +
		4 +                            /* 4 stackframeinfo arguments (darwin)*/
		nmd->paramcount  + 
		nmd->memuse;

/*	cd->stackframesize = (cd->stackframesize + 3) & ~3;*/ /* keep stack 16-byte aligned */

	/* create method header */

	(void) dseg_add_unique_address(cd, code);              /* CodeinfoPointer */
	(void) dseg_add_unique_s4(cd, cd->stackframesize * 8); /* FrameSize       */
	(void) dseg_add_unique_s4(cd, 0);                      /* IsLeaf          */
	(void) dseg_add_unique_s4(cd, 0);                      /* IntSave         */
	(void) dseg_add_unique_s4(cd, 0);                      /* FltSave         */

	/* generate code */

	M_MFLR(REG_ZERO);
	M_AST_INTERN(REG_ZERO, REG_SP, LA_LR_OFFSET);
	M_STDU(REG_SP, REG_SP, -(cd->stackframesize * 8));

	/* save integer and float argument registers */

	for (i = 0; i < md->paramcount; i++) {
		if (!md->params[i].inmemory) {
			s1 = md->params[i].regoff;

			switch (md->paramtypes[i].type) {
			case TYPE_INT:
			case TYPE_LNG:
			case TYPE_ADR:
				M_LST(s1, REG_SP, LA_SIZE + PA_SIZE + 4 * 8 + i * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DST(s1, REG_SP, LA_SIZE + PA_SIZE + 4 * 8 + i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* create native stack info */

	M_MOV(REG_SP, REG_A0);
	M_MOV(REG_PV, REG_A1);
	disp = dseg_add_functionptr(cd, codegen_start_native_call);
	M_ALD(REG_ITMP1, REG_PV, disp);
	M_ALD(REG_ITMP1, REG_ITMP1, 0);		/* TOC */
	M_MTCTR(REG_ITMP1);
	M_JSR;

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
				M_LLD(s1, REG_SP, LA_SIZE + PA_SIZE + 4 * 8 + i * 8);
				break;
			case TYPE_FLT:
			case TYPE_DBL:
				M_DLD(s1, REG_SP, LA_SIZE + PA_SIZE + 4 * 8 + i * 8);
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
			/* We only copy spilled float arguments, as the float
			   argument registers keep unchanged. */

			if (md->params[i].inmemory) {
				s1 = md->params[i].regoff + cd->stackframesize * 8;
				s2 = nmd->params[j].regoff;


				if (IS_2_WORD_TYPE(t)) {
					M_DLD(REG_FTMP1, REG_SP, s1);
					M_DST(REG_FTMP1, REG_SP, s2);
				}
				else {
					M_FLD(REG_FTMP1, REG_SP, s1);
					M_FST(REG_FTMP1, REG_SP, s2 + 4);
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

		disp = dseg_add_unique_address(cd, VM::get_current()->get_jnienv());
		M_ALD(REG_A0, REG_PV, disp);
	}

	/* Call the native function. */
	/* native functions have a different TOC for sure */

	M_AST(REG_TOC, REG_SP, 40);	/* save old TOC */
	disp = dseg_add_functionptr(cd, f);
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_ALD(REG_TOC, REG_ITMP3, 8);	/* load TOC from func. descriptor */
	M_ALD(REG_ITMP3, REG_ITMP3, 0);		
	M_MTCTR(REG_ITMP3);
	M_JSR;
	M_ALD(REG_TOC, REG_SP, 40);	/* restore TOC */

	/* save return value */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_INT_LNG_TYPE(md->returntype.type)) {
			M_LST(REG_RESULT, REG_SP, LA_SIZE + PA_SIZE + 2 * 8);
		}
		else {
			M_DST(REG_FRESULT, REG_SP, LA_SIZE + PA_SIZE + 2 * 8);
		}
	}

	/* remove native stackframe info */

	M_MOV(REG_SP, REG_A0);
	M_MOV(REG_PV, REG_A1);
	disp = dseg_add_functionptr(cd, codegen_finish_native_call);
	M_ALD(REG_ITMP1, REG_PV, disp);
	M_ALD(REG_ITMP1, REG_ITMP1, 0);	/* XXX what about TOC? */
	M_MTCTR(REG_ITMP1);
	M_JSR;
	M_MOV(REG_RESULT, REG_ITMP1_XPTR);

	/* restore return value */

	if (md->returntype.type != TYPE_VOID) {
		if (IS_INT_LNG_TYPE(md->returntype.type)) {
			M_LLD(REG_RESULT, REG_SP, LA_SIZE + PA_SIZE + 2 * 8);
		}
		else {
			M_DLD(REG_FRESULT, REG_SP, LA_SIZE + PA_SIZE + 2 * 8);
		}
	}

	M_ALD(REG_ITMP2_XPC, REG_SP, cd->stackframesize * 8 + LA_LR_OFFSET);
	M_MTLR(REG_ITMP2_XPC);
	M_LDA(REG_SP, REG_SP, cd->stackframesize * 8); /* remove stackframe           */

	/* check for exception */

	M_TST(REG_ITMP1_XPTR);
	M_BNE(1);                           /* if no exception then return        */

	M_RET;

	/* handle exception */

	M_LADD_IMM(REG_ITMP2_XPC, -4, REG_ITMP2_XPC);  /* exception address       */

	disp = dseg_add_functionptr(cd, asm_handle_nat_exception);
	M_ALD(REG_ITMP3, REG_PV, disp);
	M_MTCTR(REG_ITMP3);
	M_RTS;
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
