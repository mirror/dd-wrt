/* src/vm/jit/x86_64/codegen.cpp - machine code generator for x86_64

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

#include "vm/types.hpp"
#include "vm/os.hpp"

#include "md-abi.hpp"

#include "vm/jit/x86_64/codegen.hpp"
#include "vm/jit/x86_64/emit.hpp"

#include "mm/memory.hpp"

#include "native/localref.hpp"
#include "native/native.hpp"

#include "threads/lock.hpp"

#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/options.hpp"
#include "vm/primitive.hpp"
#include "vm/statistics.hpp"
#include "vm/vm.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
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
	methodinfo*   m  = jd->m;
	codegendata*  cd = jd->cd;
	registerdata* rd = jd->rd;

	/* create stack frame (if necessary) */

	if (cd->stackframesize)
		M_ASUB_IMM(cd->stackframesize * 8, REG_SP);

	/* save used callee saved registers */

  	p = cd->stackframesize;
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
			else {                                 /* stack arguments       */
 				if (!IS_INMEMORY(var->flags))
					/* + 8 for return address */
 					M_LLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1 + 8);
				else
					var->vv.regoff = cd->stackframesize * 8 + s1 + 8;
			}
		}
		else {                                     /* floating args         */
			// With SSE, the fst/dld instructions don't actually mean "store a
			// float/double to memory", but rather "store the lower 32/64 bits
			// of the register to memory". Therefore we get by with treating
			// all floating point values the same.
 			if (!md->params[p].inmemory) {           /* register arguments    */
 				if (!IS_INMEMORY(var->flags))
					emit_fmove(cd, s1, var->vv.regoff);
				else
					M_DST(s1, REG_SP, var->vv.regoff);
			}
			else {                                 /* stack arguments       */
 				if (!IS_INMEMORY(var->flags))
					M_DLD(var->vv.regoff, REG_SP, cd->stackframesize * 8 + s1 + 8);
				else
					var->vv.regoff = cd->stackframesize * 8 + s1 + 8;
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
	codegendata*  cd = jd->cd;
	registerdata* rd = jd->rd;

	p = cd->stackframesize;

	/* restore saved registers */

	for (i = INT_SAV_CNT - 1; i >= rd->savintreguse; i--) {
		p--; M_LLD(rd->savintregs[i], REG_SP, p * 8);
	}
	for (i = FLT_SAV_CNT - 1; i >= rd->savfltreguse; i--) {
		p--; M_DLD(rd->savfltregs[i], REG_SP, p * 8);
	}

	/* deallocate stack */

	if (cd->stackframesize)
		M_AADD_IMM(cd->stackframesize * 8, REG_SP);

	M_RET;
}

/**
 * Generates a memory barrier to be used after volatile writes. It can be
 * patched out later if the field turns out not to be volatile.
 */
void codegen_emit_patchable_barrier(instruction *iptr, codegendata *cd, patchref_t *pr, fieldinfo *fi)
{
	if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
		/* Align on word boundary */
		if ((((intptr_t) cd->mcodeptr) & 3) >= 2)
			emit_nop(cd, 4 - (((intptr_t) cd->mcodeptr) & 3));
		/* Displacement for patching out MFENCE */
		pr->disp_mb = (cd->mcodeptr - cd->mcodebase - pr->mpc);
	}
	if (INSTRUCTION_IS_UNRESOLVED(iptr) || fi->flags & ACC_VOLATILE)
		M_MFENCE;
}

/**
 * Ensures that the patched location (an int32_t) is aligned.
 */
static void codegen_fixup_alignment(codegendata *cd, patchref_t *pr, u1 *mcodeptr_save)
{
	u1 *codeptr = cd->mcodeptr;
	int disp = PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 0, sizeof(int32_t));
	memmove(mcodeptr_save + disp, mcodeptr_save, cd->mcodeptr - mcodeptr_save);
	pr->patch_align += cd->mcodeptr - mcodeptr_save + disp;

	cd->mcodeptr = mcodeptr_save;
	emit_arbitrary_nop(cd, disp);
	cd->mcodeptr = codeptr + disp;
}

/**
 * Generates machine code for one ICMD.
 */
void codegen_emit_instruction(jitdata* jd, instruction* iptr)
{
	varinfo*            var;
	varinfo*            dst;
	builtintable_entry* bte;
	methodinfo*         lm;             // Local methodinfo for ICMD_INVOKE*.
	unresolved_method*  um;
	fieldinfo*          fi;
	unresolved_field*   uf;
	patchref_t*         pr;
	int32_t             fieldtype;
	int32_t             s1, s2, s3, d;
	int32_t             disp;
	u1*                 mcodeptr_save = NULL;

	// Get required compiler data.
	codegendata*  cd = jd->cd;

	switch (iptr->opc) {

		/* constant operations ************************************************/

		case ICMD_FCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			disp = dseg_add_float(cd, iptr->sx.val.f);
			emit_movdl_membase_reg(cd, RIP, -((cd->mcodeptr + ((d > 7) ? 9 : 8)) - cd->mcodebase) + disp, d);
			emit_store_dst(jd, iptr, d);
			break;
		
		case ICMD_DCONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			disp = dseg_add_double(cd, iptr->sx.val.d);
			emit_movd_membase_reg(cd, RIP, -((cd->mcodeptr + 9) - cd->mcodebase) + disp, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ACONST:     /* ...  ==> ..., constant                       */

			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				constant_classref *cr = iptr->sx.val.c.ref;
				disp = dseg_add_unique_address(cd, cr);

/* 				PROFILE_CYCLE_STOP; */

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									  cr, disp);

/* 				PROFILE_CYCLE_START; */

				M_ALD(d, RIP, disp);
			}
			else {
				if (iptr->sx.val.anyptr == 0) {
					M_CLR(d);
				}
				else {
					disp = dseg_add_address(cd, iptr->sx.val.anyptr);
					M_ALD(d, RIP, disp);
				}
			}
			emit_store_dst(jd, iptr, d);
			break;


		/* integer operations *************************************************/

		case ICMD_INEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_INEG(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1); 
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_LNEG(d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2L:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_ISEXT(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2I:        /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_IMOV(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2BYTE:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_BSEXT(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2CHAR:   /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_CZEXT(s1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_INT2SHORT:  /* ..., value  ==> ..., value                   */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
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
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);

			/* Using inc and dec is not faster than add (tested with
			   sieve). */

			M_INTMOVE(s1, d);
			M_IADD_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_LADD(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_LADD(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LADDCONST:  /* ..., value  ==> ..., value + constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			if (IS_IMM32(iptr->sx.val.l))
				M_LADD_IMM(iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LADD(REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d) {
				M_INTMOVE(s1, REG_ITMP1);
				M_ISUB(s2, REG_ITMP1);
				M_INTMOVE(REG_ITMP1, d);
			} else {
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

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d) {
				M_INTMOVE(s1, REG_ITMP1);
				M_LSUB(s2, REG_ITMP1);
				M_INTMOVE(REG_ITMP1, d);
			} else {
				M_INTMOVE(s1, d);
				M_LSUB(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSUBCONST:  /* ..., value  ==> ..., value - constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			if (IS_IMM32(iptr->sx.val.l))
				M_LSUB_IMM(iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LSUB(REG_ITMP2, d);
			}
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
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			if (iptr->sx.val.i == 2) {
				M_INTMOVE(s1, d);
				M_ISLL_IMM(1, d);
			} else
				M_IMUL_IMM(s1, iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_LMUL(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_LMUL(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LMULCONST:  /* ..., value  ==> ..., value * constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			if (IS_IMM32(iptr->sx.val.l))
				M_LMUL_IMM(s1, iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_INTMOVE(s1, d);
				M_LMUL(REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, RAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, RAX);

			M_INTMOVE(s1, RAX);
			M_INTMOVE(s2, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_MOV(RDX, REG_ITMP2);    /* save RDX (it's an argument register) */

			M_ICMP_IMM(0x80000000, RAX);    /* check as described in jvm spec */
			M_BNE(4 + 6);
			M_ICMP_IMM(-1, REG_ITMP3);                             /* 4 bytes */
			M_BEQ(1 + 3);                                          /* 6 bytes */

  			emit_cltd(cd);                                         /* 1 byte  */
			emit_idivl_reg(cd, REG_ITMP3);                         /* 3 bytes */

			M_INTMOVE(RAX, d);
			emit_store_dst(jd, iptr, d);
			dst = VAROP(iptr->dst);
			if (IS_INMEMORY(dst->flags) || (dst->vv.regoff != RDX))
				M_MOV(REG_ITMP2, RDX);                         /* restore RDX */
			break;

		case ICMD_IREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, RAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, RDX);

			M_INTMOVE(s1, RAX);
			M_INTMOVE(s2, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_MOV(RDX, REG_ITMP2);    /* save RDX (it's an argument register) */

			M_ICMP_IMM(0x80000000, RAX);    /* check as described in jvm spec */
			M_BNE(3 + 4 + 6);
			M_CLR(RDX);                                            /* 3 bytes */
			M_ICMP_IMM(-1, REG_ITMP3);                             /* 4 bytes */
			M_BEQ(1 + 3);                                          /* 6 bytes */

  			emit_cltd(cd);                                         /* 1 byte  */
			emit_idivl_reg(cd, REG_ITMP3);                         /* 3 byte  */

			M_INTMOVE(RDX, d);
			emit_store_dst(jd, iptr, d);
			dst = VAROP(iptr->dst);
			if (IS_INMEMORY(dst->flags) || (dst->vv.regoff != RDX))
				M_MOV(REG_ITMP2, RDX);                         /* restore RDX */
			break;

		case ICMD_IDIVPOW2:   /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP1);
			emit_alul_imm_reg(cd, ALU_CMP, -1, REG_ITMP1);
			emit_leal_membase_reg(cd, REG_ITMP1, (1 << iptr->sx.val.i) - 1, REG_ITMP2);
			emit_cmovccl_reg_reg(cd, CC_LE, REG_ITMP2, REG_ITMP1);
			emit_shiftl_imm_reg(cd, SHIFT_SAR, iptr->sx.val.i, REG_ITMP1);
			emit_mov_reg_reg(cd, REG_ITMP1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP1);
			emit_alul_imm_reg(cd, ALU_CMP, -1, REG_ITMP1);
			emit_leal_membase_reg(cd, REG_ITMP1, iptr->sx.val.i, REG_ITMP2);
			emit_cmovccl_reg_reg(cd, CC_G, REG_ITMP1, REG_ITMP2);
			emit_alul_imm_reg(cd, ALU_AND, -1 - (iptr->sx.val.i), REG_ITMP2);
			emit_alul_reg_reg(cd, ALU_SUB, REG_ITMP2, REG_ITMP1);
			emit_mov_reg_reg(cd, REG_ITMP1, d);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_LDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, RAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, RAX);

			M_INTMOVE(s1, RAX);
			M_INTMOVE(s2, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_MOV(RDX, REG_ITMP2);    /* save RDX (it's an argument register) */

			/* check as described in jvm spec */
			disp = dseg_add_s8(cd, 0x8000000000000000LL);
  			M_LCMP_MEMBASE(RIP, -((cd->mcodeptr + 7) - cd->mcodebase) + disp, RAX);
			M_BNE(4 + 6);
			M_LCMP_IMM(-1, REG_ITMP3);                             /* 4 bytes */
			M_BEQ(2 + 3);                                          /* 6 bytes */

  			emit_cqto(cd);                                         /* 2 bytes */
			emit_idiv_reg(cd, REG_ITMP3);                          /* 3 bytes */

			M_INTMOVE(RAX, d);
			emit_store_dst(jd, iptr, d);
			dst = VAROP(iptr->dst);
			if (IS_INMEMORY(dst->flags) || (dst->vv.regoff != RDX))
				M_MOV(REG_ITMP2, RDX);                         /* restore RDX */
			break;

		case ICMD_LREM:       /* ..., val1, val2  ==> ..., val1 % val2        */

			s1 = emit_load_s1(jd, iptr, RAX);
			s2 = emit_load_s2(jd, iptr, REG_ITMP3);
			d = codegen_reg_of_dst(jd, iptr, RDX);

			M_INTMOVE(s1, RAX);
			M_INTMOVE(s2, REG_ITMP3);
			emit_arithmetic_check(cd, iptr, REG_ITMP3);

			M_MOV(RDX, REG_ITMP2);    /* save RDX (it's an argument register) */

			/* check as described in jvm spec */
			disp = dseg_add_s8(cd, 0x8000000000000000LL);
  			M_LCMP_MEMBASE(RIP, -((cd->mcodeptr + 7) - cd->mcodebase) + disp, REG_ITMP1);
			M_BNE(3 + 4 + 6);
			M_LXOR(RDX, RDX);                                      /* 3 bytes */
			M_LCMP_IMM(-1, REG_ITMP3);                             /* 4 bytes */
			M_BEQ(2 + 3);                                          /* 6 bytes */

  			emit_cqto(cd);                                         /* 2 bytes */
			emit_idiv_reg(cd, REG_ITMP3);                          /* 3 bytes */

			M_INTMOVE(RDX, d);
			emit_store_dst(jd, iptr, d);
			dst = VAROP(iptr->dst);
			if (IS_INMEMORY(dst->flags) || (dst->vv.regoff != RDX))
				M_MOV(REG_ITMP2, RDX);                         /* restore RDX */
			break;

		case ICMD_LDIVPOW2:   /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP1);
			emit_alu_imm_reg(cd, ALU_CMP, -1, REG_ITMP1);
			emit_lea_membase_reg(cd, REG_ITMP1, (1 << iptr->sx.val.i) - 1, REG_ITMP2);
			emit_cmovcc_reg_reg(cd, CC_LE, REG_ITMP2, REG_ITMP1);
			emit_shift_imm_reg(cd, SHIFT_SAR, iptr->sx.val.i, REG_ITMP1);
			emit_mov_reg_reg(cd, REG_ITMP1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LREMPOW2:   /* ..., value  ==> ..., value % constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_INTMOVE(s1, REG_ITMP1);
			emit_alu_imm_reg(cd, ALU_CMP, -1, REG_ITMP1);
			emit_lea_membase_reg(cd, REG_ITMP1, iptr->sx.val.i, REG_ITMP2);
			emit_cmovcc_reg_reg(cd, CC_G, REG_ITMP1, REG_ITMP2);
			emit_alu_imm_reg(cd, ALU_AND, -1 - (iptr->sx.val.i), REG_ITMP2);
			emit_alu_reg_reg(cd, ALU_SUB, REG_ITMP2, REG_ITMP1);
			emit_mov_reg_reg(cd, REG_ITMP1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_ishift(jd, SHIFT_SHL, iptr);
			break;

		case ICMD_ISHLCONST:  /* ..., value  ==> ..., value << constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_ISLL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_ISHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_ishift(jd, SHIFT_SAR, iptr);
			break;

		case ICMD_ISHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			M_ISRA_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_ishift(jd, SHIFT_SHR, iptr);
			break;

		case ICMD_IUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			M_ISRL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHL:       /* ..., val1, val2  ==> ..., val1 << val2       */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_lshift(jd, SHIFT_SHL, iptr);
			break;

        case ICMD_LSHLCONST:  /* ..., value  ==> ..., value << constant       */
 			                  /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_LSLL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LSHR:       /* ..., val1, val2  ==> ..., val1 >> val2       */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_lshift(jd, SHIFT_SAR, iptr);
			break;

		case ICMD_LSHRCONST:  /* ..., value  ==> ..., value >> constant       */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			M_LSRA_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LUSHR:      /* ..., val1, val2  ==> ..., val1 >>> val2      */

			d = codegen_reg_of_dst(jd, iptr, REG_NULL);
			emit_lshift(jd, SHIFT_SHR, iptr);
			break;

  		case ICMD_LUSHRCONST: /* ..., value  ==> ..., value >>> constant      */
  		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			M_INTMOVE(s1, d);
			M_LSRL_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_IAND(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_IAND(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_IAND_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LAND:       /* ..., val1, val2  ==> ..., val1 & val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_LAND(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_LAND(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LANDCONST:  /* ..., value  ==> ..., value & constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			if (IS_IMM32(iptr->sx.val.l))
				M_LAND_IMM(iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LAND(REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_IOR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_IOR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_IOR_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LOR:        /* ..., val1, val2  ==> ..., val1 | val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_LOR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_LOR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LORCONST:   /* ..., value  ==> ..., value | constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			if (IS_IMM32(iptr->sx.val.l))
				M_LOR_IMM(iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LOR(REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_IXOR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_IXOR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.i = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			M_IXOR_IMM(iptr->sx.val.i, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXOR:       /* ..., val1, val2  ==> ..., val1 ^ val2        */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
			if (s2 == d)
				M_LXOR(s1, d);
			else {
				M_INTMOVE(s1, d);
				M_LXOR(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LXORCONST:  /* ..., value  ==> ..., value ^ constant        */
		                      /* sx.val.l = constant                             */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_INTMOVE(s1, d);
			if (IS_IMM32(iptr->sx.val.l))
				M_LXOR_IMM(iptr->sx.val.l, d);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LXOR(REG_ITMP2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;


		/* floating operations ************************************************/

		case ICMD_FNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_s4(cd, 0x80000000);
			emit_fmove(cd, s1, d);
			emit_movss_membase_reg(cd, RIP, -((cd->mcodeptr + 9) - cd->mcodebase) + disp, REG_FTMP2);
			emit_xorps_reg_reg(cd, REG_FTMP2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DNEG:       /* ..., value  ==> ..., - value                 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			disp = dseg_add_s8(cd, 0x8000000000000000);
			emit_fmove(cd, s1, d);
			emit_movd_membase_reg(cd, RIP, -((cd->mcodeptr + 9) - cd->mcodebase) + disp, REG_FTMP2);
			emit_xorpd_reg_reg(cd, REG_FTMP2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d)
				M_FADD(s1, d);
			else {
				emit_fmove(cd, s1, d);
				M_FADD(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DADD:       /* ..., val1, val2  ==> ..., val1 + val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d)
				M_DADD(s1, d);
			else {
				emit_fmove(cd, s1, d);
				M_DADD(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d) {
				emit_fmove(cd, s2, REG_FTMP2);
				s2 = REG_FTMP2;
			}
			emit_fmove(cd, s1, d);
			M_FSUB(s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DSUB:       /* ..., val1, val2  ==> ..., val1 - val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d) {
				emit_fmove(cd, s2, REG_FTMP2);
				s2 = REG_FTMP2;
			}
			emit_fmove(cd, s1, d);
			M_DSUB(s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d)
				M_FMUL(s1, d);
			else {
				emit_fmove(cd, s1, d);
				M_FMUL(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DMUL:       /* ..., val1, val2  ==> ..., val1 * val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d)
				M_DMUL(s1, d);
			else {
				emit_fmove(cd, s1, d);
				M_DMUL(s2, d);
			}
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d) {
				emit_fmove(cd, s2, REG_FTMP2);
				s2 = REG_FTMP2;
			}
			emit_fmove(cd, s1, d);
			M_FDIV(s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DDIV:       /* ..., val1, val2  ==> ..., val1 / val2        */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			if (s2 == d) {
				emit_fmove(cd, s2, REG_FTMP2);
				s2 = REG_FTMP2;
			}
			emit_fmove(cd, s1, d);
			M_DDIV(s2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_CVTIF(s1, d);
  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_I2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_CVTID(s1, d);
  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_L2F:       /* ..., value  ==> ..., (float) value            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_CVTLF(s1, d);
  			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_L2D:       /* ..., value  ==> ..., (double) value           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP1);
			M_CVTLD(s1, d);
  			emit_store_dst(jd, iptr, d);
			break;
			
		case ICMD_F2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_CVTFI(s1, d);
			M_ICMP_IMM(0x80000000, d);                        /* corner cases */
			disp = ((s1 == REG_FTMP1) ? 0 : 5) + 10 + 3 +
				((REG_RESULT == d) ? 0 : 3);
			M_BNE(disp);
			emit_fmove(cd, s1, REG_FTMP1);
			M_MOV_IMM(asm_builtin_f2i, REG_ITMP2);
			M_CALL(REG_ITMP2);
			M_INTMOVE(REG_RESULT, d);
  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2I:       /* ..., value  ==> ..., (int) value              */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_CVTDI(s1, d);
			M_ICMP_IMM(0x80000000, d);                        /* corner cases */
			disp = ((s1 == REG_FTMP1) ? 0 : 5) + 10 + 3 +
				((REG_RESULT == d) ? 0 : 3);
			M_BNE(disp);
			emit_fmove(cd, s1, REG_FTMP1);
			M_MOV_IMM(asm_builtin_d2i, REG_ITMP2);
			M_CALL(REG_ITMP2);
			M_INTMOVE(REG_RESULT, d);
  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_F2L:       /* ..., value  ==> ..., (long) value             */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_CVTFL(s1, d);
			M_MOV_IMM(0x8000000000000000, REG_ITMP2);
			M_LCMP(REG_ITMP2, d);                             /* corner cases */
			disp = ((s1 == REG_FTMP1) ? 0 : 5) + 10 + 3 +
				((REG_RESULT == d) ? 0 : 3);
			M_BNE(disp);
			emit_fmove(cd, s1, REG_FTMP1);
			M_MOV_IMM(asm_builtin_f2l, REG_ITMP2);
			M_CALL(REG_ITMP2);
			M_INTMOVE(REG_RESULT, d);
  			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_D2L:       /* ..., value  ==> ..., (long) value             */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
			M_CVTDL(s1, d);
			M_MOV_IMM(0x8000000000000000, REG_ITMP2);
			M_LCMP(REG_ITMP2, d);                             /* corner cases */
			disp = ((s1 == REG_FTMP1) ? 0 : 5) + 10 + 3 +
				((REG_RESULT == d) ? 0 : 3);
			M_BNE(disp);
			emit_fmove(cd, s1, REG_FTMP1);
			M_MOV_IMM(asm_builtin_d2l, REG_ITMP2);
			M_CALL(REG_ITMP2);
			M_INTMOVE(REG_RESULT, d);
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

		case ICMD_FCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */
 			                  /* == => 0, < => 1, > => -1 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_CLR(d);
			M_MOV_IMM(1, REG_ITMP1);
			M_MOV_IMM(-1, REG_ITMP2);
			emit_ucomiss_reg_reg(cd, s1, s2);
			M_CMOVULT(REG_ITMP1, d);
			M_CMOVUGT(REG_ITMP2, d);
			M_CMOVP(REG_ITMP2, d);                   /* treat unordered as GT */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */
 			                  /* == => 0, < => 1, > => -1 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_CLR(d);
			M_MOV_IMM(1, REG_ITMP1);
			M_MOV_IMM(-1, REG_ITMP2);
			emit_ucomiss_reg_reg(cd, s1, s2);
			M_CMOVULT(REG_ITMP1, d);
			M_CMOVUGT(REG_ITMP2, d);
			M_CMOVP(REG_ITMP1, d);                   /* treat unordered as LT */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DCMPL:      /* ..., val1, val2  ==> ..., val1 fcmpl val2    */
 			                  /* == => 0, < => 1, > => -1 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_CLR(d);
			M_MOV_IMM(1, REG_ITMP1);
			M_MOV_IMM(-1, REG_ITMP2);
			emit_ucomisd_reg_reg(cd, s1, s2);
			M_CMOVULT(REG_ITMP1, d);
			M_CMOVUGT(REG_ITMP2, d);
			M_CMOVP(REG_ITMP2, d);                   /* treat unordered as GT */
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DCMPG:      /* ..., val1, val2  ==> ..., val1 fcmpg val2    */
 			                  /* == => 0, < => 1, > => -1 */

			s1 = emit_load_s1(jd, iptr, REG_FTMP1);
			s2 = emit_load_s2(jd, iptr, REG_FTMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			M_CLR(d);
			M_MOV_IMM(1, REG_ITMP1);
			M_MOV_IMM(-1, REG_ITMP2);
			emit_ucomisd_reg_reg(cd, s1, s2);
			M_CMOVULT(REG_ITMP1, d);
			M_CMOVUGT(REG_ITMP2, d);
			M_CMOVP(REG_ITMP1, d);                   /* treat unordered as LT */
			emit_store_dst(jd, iptr, d);
			break;


		/* memory operations **************************************************/

		case ICMD_BALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
   			emit_movsbq_memindex_reg(cd, OFFSET(java_bytearray_t, data[0]), s1, s2, 0, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_CALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movzwq_memindex_reg(cd, OFFSET(java_chararray_t, data[0]), s1, s2, 1, d);
			emit_store_dst(jd, iptr, d);
			break;			

		case ICMD_SALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movswq_memindex_reg(cd, OFFSET(java_shortarray_t, data[0]), s1, s2, 1, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_IALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movl_memindex_reg(cd, OFFSET(java_intarray_t, data[0]), s1, s2, 2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_LALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_memindex_reg(cd, OFFSET(java_longarray_t, data[0]), s1, s2, 3, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_FALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movss_memindex_reg(cd, OFFSET(java_floatarray_t, data[0]), s1, s2, 2, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_DALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_FTMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movsd_memindex_reg(cd, OFFSET(java_doublearray_t, data[0]), s1, s2, 3, d);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_AALOAD:     /* ..., arrayref, index  ==> ..., value         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			d = codegen_reg_of_dst(jd, iptr, REG_ITMP3);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_memindex_reg(cd, OFFSET(java_objectarray_t, data[0]), s1, s2, 3, d);
			emit_store_dst(jd, iptr, d);
			break;


		case ICMD_BASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movb_reg_memindex(cd, s3, OFFSET(java_bytearray_t, data[0]), s1, s2, 0);
			break;

		case ICMD_CASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movw_reg_memindex(cd, s3, OFFSET(java_chararray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_SASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movw_reg_memindex(cd, s3, OFFSET(java_shortarray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_IASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_movl_reg_memindex(cd, s3, OFFSET(java_intarray_t, data[0]), s1, s2, 2);
			break;

		case ICMD_LASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_mov_reg_memindex(cd, s3, OFFSET(java_longarray_t, data[0]), s1, s2, 3);
			break;

		case ICMD_FASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			emit_movss_reg_memindex(cd, s3, OFFSET(java_floatarray_t, data[0]), s1, s2, 2);
			break;

		case ICMD_DASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_FTMP3);
			emit_movsd_reg_memindex(cd, s3, OFFSET(java_doublearray_t, data[0]), s1, s2, 3);
			break;

		case ICMD_AASTORE:    /* ..., arrayref, index, value  ==> ...         */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);

			M_MOV(s1, REG_A0);
			M_MOV(s3, REG_A1);
			M_MOV_IMM(BUILTIN_FAST_canstore, REG_ITMP1);
			M_CALL(REG_ITMP1);
			emit_arraystore_check(cd, iptr);

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			s3 = emit_load_s3(jd, iptr, REG_ITMP3);
			emit_mov_reg_memindex(cd, s3, OFFSET(java_objectarray_t, data[0]), s1, s2, 3);
			break;


		case ICMD_BASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movb_imm_memindex(cd, iptr->sx.s23.s3.constval, OFFSET(java_bytearray_t, data[0]), s1, s2, 0);
			break;

		case ICMD_CASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movw_imm_memindex(cd, iptr->sx.s23.s3.constval, OFFSET(java_chararray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_SASTORECONST:   /* ..., arrayref, index  ==> ...            */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movw_imm_memindex(cd, iptr->sx.s23.s3.constval, OFFSET(java_shortarray_t, data[0]), s1, s2, 1);
			break;

		case ICMD_IASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_movl_imm_memindex(cd, iptr->sx.s23.s3.constval, OFFSET(java_intarray_t, data[0]), s1, s2, 2);
			break;

		case ICMD_LASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);

			if (IS_IMM32(iptr->sx.s23.s3.constval)) {
				emit_mov_imm_memindex(cd, (u4) (iptr->sx.s23.s3.constval & 0x00000000ffffffff), OFFSET(java_longarray_t, data[0]), s1, s2, 3);
			}
			else {
				emit_movl_imm_memindex(cd, (u4) (iptr->sx.s23.s3.constval & 0x00000000ffffffff), OFFSET(java_longarray_t, data[0]), s1, s2, 3);
				emit_movl_imm_memindex(cd, (u4) (iptr->sx.s23.s3.constval >> 32), OFFSET(java_longarray_t, data[0]) + 4, s1, s2, 3);
			}
			break;

		case ICMD_AASTORECONST: /* ..., arrayref, index  ==> ...              */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			/* implicit null-pointer check */
			emit_arrayindexoutofbounds_check(cd, iptr, s1, s2);
			emit_mov_imm_memindex(cd, 0, OFFSET(java_objectarray_t, data[0]), s1, s2, 3);
			break;

		case ICMD_PUTSTATICCONST: /* ...  ==> ...                             */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = dseg_add_unique_address(cd, uf);

/* 				PROFILE_CYCLE_STOP; */

				pr = patcher_add_patch_ref(jd, PATCHER_get_putstatic, uf, disp);

/* 				PROFILE_CYCLE_START; */

				fi = NULL;		/* Silence compiler warning */
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = dseg_add_address(cd, fi->value);

				if (!class_is_or_almost_initialized(fi->clazz)) {
					//PROFILE_CYCLE_STOP;

					patcher_add_patch_ref(jd, PATCHER_initialize_class,
										  fi->clazz, 0);

					//PROFILE_CYCLE_START;
				}

				pr = NULL;		/* Silence compiler warning */
  			}

			/* This approach is much faster than moving the field
			   address inline into a register. */

  			M_ALD(REG_ITMP1, RIP, disp);

			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_FLT:
				M_IST_IMM(iptr->sx.s23.s2.constval, REG_ITMP1, 0);
				break;
			case TYPE_LNG:
			case TYPE_ADR:
			case TYPE_DBL:
				if (IS_IMM32(iptr->sx.s23.s2.constval))
					M_LST_IMM32(iptr->sx.s23.s2.constval, REG_ITMP1, 0);
				else {
					M_MOV_IMM(iptr->sx.s23.s2.constval, REG_ITMP2);
					M_LST(REG_ITMP2, REG_ITMP1, 0);
				}
				break;
			default:
				assert(false);
				break;
			}
			codegen_emit_patchable_barrier(iptr, cd, pr, fi);
			break;

		case ICMD_GETFIELD:   /* ...  ==> ..., value                          */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

/* 				PROFILE_CYCLE_STOP; */

				pr = patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
				mcodeptr_save = cd->mcodeptr;

/* 				PROFILE_CYCLE_START; */

				fi = NULL;		/* Silence compiler warning */
			}
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;

				pr = 0;
			}

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
				M_ILD32(d, s1, disp);
				break;
			case TYPE_LNG:
			case TYPE_ADR:
				d = codegen_reg_of_dst(jd, iptr, REG_ITMP1);
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
				// Silence compiler warning.
				d = 0;
			}
			if (pr)
				codegen_fixup_alignment(cd, pr, mcodeptr_save);
			emit_store_dst(jd, iptr, d);
			break;

		case ICMD_PUTFIELD:   /* ..., objectref, value  ==> ...               */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_IFTMP); /* REG_IFTMP == REG_ITMP2 */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

/* 				PROFILE_CYCLE_STOP; */

				pr = patcher_add_patch_ref(jd, PATCHER_get_putfield, uf, 0);
				mcodeptr_save = cd->mcodeptr;

/* 				PROFILE_CYCLE_START; */

				fi = NULL;		/* Silence compiler warning */
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;

				pr = NULL;		/* Silence compiler warning */
			}

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
				M_IST32(s2, s1, disp);
				break;
			case TYPE_LNG:
			case TYPE_ADR:
				M_LST32(s2, s1, disp);
				break;
			case TYPE_FLT:
				M_FST32(s2, s1, disp);
				break;
			case TYPE_DBL:
				M_DST32(s2, s1, disp);
				break;
			}
			if (pr)
				codegen_fixup_alignment(cd, pr, mcodeptr_save);
			codegen_emit_patchable_barrier(iptr, cd, pr, fi);
			break;

		case ICMD_PUTFIELDCONST:  /* ..., objectref, value  ==> ...           */
		                          /* val = value (in current instruction)     */
		                          /* following NOP)                           */

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				uf        = iptr->sx.s23.s3.uf;
				fieldtype = uf->fieldref->parseddesc.fd->type;
				disp      = 0;

/* 				PROFILE_CYCLE_STOP; */

				pr = patcher_add_patch_ref(jd, PATCHER_putfieldconst, uf, 0);
				mcodeptr_save = cd->mcodeptr;

/* 				PROFILE_CYCLE_START; */

				fi = NULL;		/* Silence compiler warning */
			} 
			else {
				fi        = iptr->sx.s23.s3.fmiref->p.field;
				fieldtype = fi->type;
				disp      = fi->offset;

				pr = NULL;		/* Silence compiler warning */
			}

			/* implicit null-pointer check */
			switch (fieldtype) {
			case TYPE_INT:
			case TYPE_FLT:
				M_IST32_IMM(iptr->sx.s23.s2.constval, s1, disp);
				break;
			case TYPE_LNG:
			case TYPE_ADR:
			case TYPE_DBL:
				/* XXX why no check for IS_IMM32? -- probably because of the patcher */
				M_MOV_IMM(iptr->sx.s23.s2.constval, REG_ITMP2);
				if (disp)  /* resolved, disp can never be 0 */
					M_LST(REG_ITMP2, s1, disp);
				else {     /* unresolved */
					M_LST32(REG_ITMP2, s1, disp);
					pr->patch_align = 4;
				}
				break;
			}
			if (pr)
				codegen_fixup_alignment(cd, pr, mcodeptr_save);
			codegen_emit_patchable_barrier(iptr, cd, pr, fi);
			break;


		/* branch operations **************************************************/

		case ICMD_ATHROW:       /* ..., objectref ==> ... (, objectref)       */

			M_CALL_IMM(0);                            /* passing exception pc */
			M_POP(REG_ITMP2_XPC);

  			M_MOV_IMM(asm_handle_exception, REG_ITMP3);
  			M_JMP(REG_ITMP3);
			break;

		case ICMD_IF_LEQ:       /* ..., value ==> ...                         */
		case ICMD_IF_LNE:
		case ICMD_IF_LLT:
		case ICMD_IF_LGE:
		case ICMD_IF_LGT:
		case ICMD_IF_LLE:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			if (IS_IMM32(iptr->sx.val.l))
				M_LCMP_IMM(iptr->sx.val.l, s1);
			else {
				M_MOV_IMM(iptr->sx.val.l, REG_ITMP2);
				M_LCMP(REG_ITMP2, s1);
			}
			emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IF_LEQ, BRANCH_OPT_NONE);
			break;

		case ICMD_IF_LCMPEQ:    /* ..., value, value ==> ...                  */
		case ICMD_IF_LCMPNE:
		case ICMD_IF_LCMPLT:
		case ICMD_IF_LCMPGE:
		case ICMD_IF_LCMPGT:
		case ICMD_IF_LCMPLE:

			s1 = emit_load_s1(jd, iptr, REG_ITMP1);
			s2 = emit_load_s2(jd, iptr, REG_ITMP2);
			M_LCMP(s2, s1);
			emit_bcc(cd, iptr->dst.block, iptr->opc - ICMD_IF_LCMPEQ, BRANCH_OPT_NONE);
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

				/* number of targets */
				i = i - l + 1;

                /* range check */

				M_ICMP_IMM(i - 1, REG_ITMP1);
				emit_bugt(cd, table[0].block);

				/* build jump table top down and use address of lowest entry */

				table += i;

				while (--i >= 0) {
					dseg_add_target(cd, table->block);
					--table;
				}

				/* length of dataseg after last dseg_add_target is used
				   by load */

				M_MOV_IMM(0, REG_ITMP2);
				dseg_adddata(cd);
				emit_mov_memindex_reg(cd, -(cd->dseglen), REG_ITMP2, REG_ITMP1, 3, REG_ITMP1);
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
			break;

		case ICMD_INVOKESPECIAL:
			emit_nullpointer_check(cd, iptr, REG_A0);
			/* fall through */

		case ICMD_INVOKESTATIC:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				disp = dseg_add_unique_address(cd, um);

				patcher_add_patch_ref(jd, PATCHER_invokestatic_special,
									  um, disp);
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				disp = dseg_add_functionptr(cd, lm->stubroutine);
			}

			M_ALD(REG_ITMP2, RIP, disp);
			M_CALL(REG_ITMP2);
			break;

		case ICMD_INVOKEVIRTUAL:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				patcher_add_patch_ref(jd, PATCHER_invokevirtual, um, 0);
				emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 6, sizeof(int32_t)));

				s1 = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, table[0]) +
					sizeof(methodptr) * lm->vftblindex;
			}

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
			M_ALD32(REG_ITMP3, REG_METHODPTR, s1);
			M_CALL(REG_ITMP3);
			break;

		case ICMD_INVOKEINTERFACE:
			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				um = iptr->sx.s23.s3.um;
				patcher_add_patch_ref(jd, PATCHER_invokeinterface, um, 0);
				emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 6, sizeof(int32_t)));

				s1 = 0;
				s2 = 0;
			}
			else {
				lm = iptr->sx.s23.s3.fmiref->p.method;
				s1 = OFFSET(vftbl_t, interfacetable[0]) -
					sizeof(methodptr) * lm->clazz->index;

				s2 = sizeof(methodptr) * (lm - lm->clazz->methods);
			}

			/* implicit null-pointer check */
			M_ALD(REG_METHODPTR, REG_A0, OFFSET(java_object_t, vftbl));
			M_ALD32(REG_METHODPTR, REG_METHODPTR, s1);
			if (INSTRUCTION_IS_UNRESOLVED(iptr))
				emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 3, sizeof(int32_t)));
			M_ALD32(REG_ITMP3, REG_METHODPTR, s2);
			M_CALL(REG_ITMP3);
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
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_1);

					patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
										  iptr->sx.s23.s3.c.ref, 0);

					emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 2, sizeof(int32_t)));

					M_IMOV_IMM(0, REG_ITMP2);                 /* super->flags */
					M_IAND_IMM(ACC_INTERFACE, REG_ITMP2);
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
						emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 10, sizeof(int32_t)));
					}

					M_ILD32(REG_ITMP3,
							REG_ITMP2, OFFSET(vftbl_t, interfacetablelength));
					M_ICMP_IMM32(superindex, REG_ITMP3);
					emit_classcast_check(cd, iptr, BRANCH_LE, REG_ITMP3, s1);

					if (super == NULL)
						emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 3, sizeof(int32_t)));
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

						constant_classref *cr = iptr->sx.s23.s3.c.ref;
						disp = dseg_add_unique_address(cd, cr);

						patcher_add_patch_ref(jd,
											  PATCHER_resolve_classref_to_vftbl,
											  cr, disp);
					}
					else {
						M_TEST(s1);
						emit_label_beq(cd, BRANCH_LABEL_5);

						disp = dseg_add_address(cd, super->vftbl);
					}

					M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
					M_ALD(REG_ITMP3, RIP, disp);

					if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
						M_LCMP_MEMINDEX(REG_ITMP2, 0, REG_ITMP1, 0, REG_ITMP3);
						emit_label_beq(cd, BRANCH_LABEL_6);  /* good */

						if (super == NULL) {
							M_LCMP_IMM(OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
							emit_label_bne(cd, BRANCH_LABEL_10);  /* throw */
						}

						M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
						M_ICMP_MEMBASE(REG_ITMP2, OFFSET(vftbl_t, subtype_depth), REG_ITMP1);
						emit_label_bgt(cd, BRANCH_LABEL_9);  /* throw */

						M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
						M_LCMP_MEMINDEX(REG_ITMP2, -8*DISPLAY_SIZE, REG_ITMP1, 3, REG_ITMP3);
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
						M_LCMP_MEMBASE(REG_ITMP2, super->vftbl->subtype_offset, REG_ITMP3);
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
				M_INTMOVE(s1, REG_A0);

				if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
					constant_classref *cr = iptr->sx.s23.s3.c.ref;
					disp = dseg_add_unique_address(cd, cr);

					patcher_add_patch_ref(jd,
										  PATCHER_resolve_classref_to_classinfo,
										  cr, disp);
				}
				else {
					disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
				}

				M_ALD(REG_A1, RIP, disp);
				M_MOV_IMM(BUILTIN_arraycheckcast, REG_ITMP1);
				M_CALL(REG_ITMP1);

				/* s1 may have been destroyed over the function call */
				s1 = emit_load_s1(jd, iptr, REG_ITMP2);
				M_TEST(REG_RESULT);
				emit_classcast_check(cd, iptr, BRANCH_EQ, REG_RESULT, s1);

				d = codegen_reg_of_dst(jd, iptr, REG_ITMP2);
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
				M_INTMOVE(s1, REG_ITMP1);
				s1 = REG_ITMP1;
			}

			M_CLR(d);

			/* if class is not resolved, check which code to call */

			if (super == NULL) {
				M_TEST(s1);
				emit_label_beq(cd, BRANCH_LABEL_1);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_flags,
									  iptr->sx.s23.s3.c.ref, 0);

				emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 2, sizeof(int32_t)));

				M_IMOV_IMM(0, REG_ITMP3);                     /* super->flags */
				M_IAND_IMM(ACC_INTERFACE, REG_ITMP3);
				emit_label_beq(cd, BRANCH_LABEL_2);
			}

			/* interface instanceof code */

			if ((super == NULL) || (super->flags & ACC_INTERFACE)) {
				int nops;
				if (super != NULL) {
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_3);
				}

				M_ALD(REG_ITMP1, s1, OFFSET(java_object_t, vftbl));

				if (super == NULL) {
					patcher_add_patch_ref(jd, PATCHER_instanceof_interface,
										  iptr->sx.s23.s3.c.ref, 0);
					emit_arbitrary_nop(cd, PATCH_ALIGNMENT((uintptr_t) cd->mcodeptr, 10, sizeof(int32_t)));
				}

				M_ILD32(REG_ITMP3,
						REG_ITMP1, OFFSET(vftbl_t, interfacetablelength));
				M_ICMP_IMM32(superindex, REG_ITMP3);

				nops = super == NULL ? PATCH_ALIGNMENT((uintptr_t) (cd->mcodeptr + 6), 3, sizeof(int32_t)) : 0;

				int a = 3 + 4 /* mov_membase32_reg */ + 3 /* test */ + 4 /* setcc */ + nops;

				M_BLE(a);
				emit_arbitrary_nop(cd, nops);
				M_ALD32(REG_ITMP1, REG_ITMP1,
						OFFSET(vftbl_t, interfacetable[0]) -
						superindex * sizeof(methodptr*));
				M_TEST(REG_ITMP1);
				M_SETNE(d);

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
					disp = dseg_add_unique_address(cd, cr);

					patcher_add_patch_ref(jd,
										  PATCHER_resolve_classref_to_vftbl,
										  cr, disp);
				}
				else {
					M_TEST(s1);
					emit_label_beq(cd, BRANCH_LABEL_5);

					disp = dseg_add_address(cd, super->vftbl);
				}

				M_ALD(REG_ITMP2, s1, OFFSET(java_object_t, vftbl));
				M_ALD(REG_ITMP3, RIP, disp);

				if (super == NULL || super->vftbl->subtype_depth >= DISPLAY_SIZE) {
					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_offset));
					M_LCMP_MEMINDEX(REG_ITMP2, 0, REG_ITMP1, 0, REG_ITMP3);
					emit_label_bne(cd, BRANCH_LABEL_8); /* jump over INC/SETE */
					if (d == REG_ITMP2) {
						M_SETE(d);
						M_BSEXT(d, d);
					} else
						M_LINC(d);
					emit_label_br(cd, BRANCH_LABEL_6);  /* true */
					emit_label(cd, BRANCH_LABEL_8);

					if (super == NULL) {
						M_LCMP_IMM(OFFSET(vftbl_t, subtype_display[DISPLAY_SIZE]), REG_ITMP1);
						emit_label_bne(cd, BRANCH_LABEL_10);  /* false */
					}

					M_ILD(REG_ITMP1, REG_ITMP3, OFFSET(vftbl_t, subtype_depth));
					M_ICMP_MEMBASE(REG_ITMP2, OFFSET(vftbl_t, subtype_depth), REG_ITMP1);
					emit_label_bgt(cd, BRANCH_LABEL_9);  /* false */

					M_ALD(REG_ITMP2, REG_ITMP2, OFFSET(vftbl_t, subtype_overflow));
					M_LCMP_MEMINDEX(REG_ITMP2, -8*DISPLAY_SIZE, REG_ITMP1, 3, REG_ITMP3);
					M_SETE(d);
					if (d == REG_ITMP2) {
						M_BSEXT(d, d);

						emit_label_br(cd, BRANCH_LABEL_7); /* jump over M_CLR */
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
					M_LCMP_MEMBASE(REG_ITMP2, super->vftbl->subtype_offset, REG_ITMP3);
					M_SETE(d);
					if (d == REG_ITMP2)
						M_BSEXT(d, d);
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

  			MCODECHECK((10 * 4 * iptr->s1.argcount) + 5 + 10 * 8);

			for (s1 = iptr->s1.argcount; --s1 >= 0; ) {

				/* copy SAVEDVAR sizes to stack */
				var = VAR(iptr->sx.s23.s2.args[s1]);

				/* Already Preallocated? */
				if (!(var->flags & PREALLOC)) {
					s2 = emit_load(jd, iptr, var, REG_ITMP1);
					M_LST(s2, REG_SP, s1 * 8);
				}
			}

			/* a0 = dimension count */

			M_MOV_IMM(iptr->s1.argcount, REG_A0);

			/* is a patcher function set? */

			if (INSTRUCTION_IS_UNRESOLVED(iptr)) {
				constant_classref *cr = iptr->sx.s23.s3.c.ref;
				disp = dseg_add_unique_address(cd, cr);

				patcher_add_patch_ref(jd, PATCHER_resolve_classref_to_classinfo,
									  cr, disp);
			}
			else {
				disp = dseg_add_address(cd, iptr->sx.s23.s3.c.cls);
			}

			/* a1 = classinfo */

			M_ALD(REG_A1, RIP, disp);

			/* a2 = pointer to dimensions = stack pointer */

			M_MOV(REG_SP, REG_A2);

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
	int          i, j;
	int          s1, s2;
	int          disp;

	/* Sanity check. */

	assert(f != NULL);

	/* Get required compiler data. */

	m    = jd->m;
	code = jd->code;
	cd   = jd->cd;

	/* initialize variables */

	md = m->parseddesc;

	/* calculate stack frame size */

	cd->stackframesize =
		sizeof(stackframeinfo_t) / SIZEOF_VOID_P +
		sizeof(localref_table) / SIZEOF_VOID_P +
		md->paramcount +
		(md->returntype.type == TYPE_VOID ? 0 : 1) +
		nmd->memuse;

	ALIGN_ODD(cd->stackframesize);              /* keep stack 16-byte aligned */

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

		M_MOV_IMM(code, REG_ITMP3);
		M_IINC_MEMBASE(REG_ITMP3, OFFSET(codeinfo, frequency));
	}
#endif

	/* generate stub code */

	M_ASUB_IMM(cd->stackframesize * 8, REG_SP);

#if defined(ENABLE_GC_CACAO)
	/* Save callee saved integer registers in stackframeinfo (GC may
	   need to recover them during a collection). */

	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
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
			case TYPE_DBL:
				M_DST(s1, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* create dynamic stack info */

	M_MOV(REG_SP, REG_A0);
	emit_lea_membase_reg(cd, RIP, -((cd->mcodeptr + 7) - cd->mcodebase), REG_A1);
	M_MOV_IMM(codegen_start_native_call, REG_ITMP1);
	M_CALL(REG_ITMP1);

	/* remember class argument */

	if (m->flags & ACC_STATIC)
		M_MOV(REG_RESULT, REG_ITMP2);

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
			case TYPE_DBL:
				M_DLD(s1, REG_SP, i * 8);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	/* Copy or spill arguments to new locations. */

	for (i = md->paramcount - 1, j = i + skipparams; i >= 0; i--, j--) {
		s2 = nmd->params[j].regoff;

		switch (md->paramtypes[i].type) {
		case TYPE_INT:
		case TYPE_LNG:
		case TYPE_ADR:
			if (!md->params[i].inmemory) {
				s1 = md->params[i].regoff;

				if (!nmd->params[j].inmemory)
					M_INTMOVE(s1, s2);
				else
					M_LST(s1, REG_SP, s2);
			}
			else {
				s1 = md->params[i].regoff + cd->stackframesize * 8 + 8;/* +1 (RA) */
				M_LLD(REG_ITMP1, REG_SP, s1);
				M_LST(REG_ITMP1, REG_SP, s2);
			}
			break;
		case TYPE_FLT:
			/* We only copy spilled float arguments, as the float
			   argument registers keep unchanged. */

			if (md->params[i].inmemory) {
				s1 = md->params[i].regoff + cd->stackframesize * 8 + 8;/* +1 (RA) */

				M_FLD(REG_FTMP1, REG_SP, s1);
				M_FST(REG_FTMP1, REG_SP, s2);
			}
			break;
		case TYPE_DBL:
			if (md->params[i].inmemory) {
				s1 = md->params[i].regoff + cd->stackframesize * 8 + 8;/* +1 (RA) */
				M_DLD(REG_FTMP1, REG_SP, s1);
				M_DST(REG_FTMP1, REG_SP, s2);
			}
			break;
		default:
			assert(false);
			break;
		}
	}

	/* Handle native Java methods. */

	if (m->flags & ACC_NATIVE) {
		/* put class into second argument register */

		if (m->flags & ACC_STATIC)
			M_MOV(REG_ITMP2, REG_A1);

		/* put env into first argument register */

		M_MOV_IMM(VM::get_current()->get_jnienv(), REG_A0);
	}

	/* Call the native function. */

	disp = dseg_add_functionptr(cd, f);
	M_ALD(REG_ITMP1, RIP, disp);
	M_CALL(REG_ITMP1);

	/* save return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
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
		M_LST(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
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
	emit_lea_membase_reg(cd, RIP, -((cd->mcodeptr + 7) - cd->mcodebase), REG_A1);
	M_MOV_IMM(codegen_finish_native_call, REG_ITMP1);
	M_CALL(REG_ITMP1);
	M_MOV(REG_RESULT, REG_ITMP3);

	/* restore return value */

	switch (md->returntype.type) {
	case TYPE_INT:
	case TYPE_LNG:
	case TYPE_ADR:
		M_LLD(REG_RESULT, REG_SP, 0 * 8);
		break;
	case TYPE_FLT:
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
        
	disp = cd->stackframesize * 8 - sizeof(stackframeinfo_t) +
		OFFSET(stackframeinfo_t, intregs);

	for (i = 0; i < INT_SAV_CNT; i++)
		M_ALD(abi_registers_integer_saved[i], REG_SP, disp + i * 8);
#endif

	/* remove stackframe */

	M_AADD_IMM(cd->stackframesize * 8, REG_SP);

	/* test for exception */

	M_TEST(REG_ITMP3);
	M_BNE(1);
	M_RET;

	/* handle exception */

	M_MOV(REG_ITMP3, REG_ITMP1_XPTR);
	M_ALD(REG_ITMP2_XPC, REG_SP, 0 * 8);     /* get return address from stack */
	M_ASUB_IMM(3, REG_ITMP2_XPC);                                    /* callq */

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
