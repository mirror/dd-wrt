/* src/vm/jit/x86_64/emit.hpp - machine dependent emit function prototypes

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


#ifndef MD_EMIT_HPP_
#define MD_EMIT_HPP_ 1

#include "config.h"
#include "vm/types.hpp"

#include "vm/jit/codegen-common.hpp"

/* macros to create code ******************************************************/

/* immediate data union */

typedef union {
    s4 i;
    s8 l;
    float f;
    double d;
    void *a;
    u1 b[8];
} imm_buf;


/* opcodes for alu instructions */

#define ALU_ADD      0
#define ALU_OR       1
#define ALU_ADC      2
#define ALU_SBB      3
#define ALU_AND      4
#define ALU_SUB      5
#define ALU_XOR      6
#define ALU_CMP      7


#define SHIFT_ROL    0
#define SHIFT_ROR    1
#define SHIFT_RCL    2
#define SHIFT_RCR    3
#define SHIFT_SHL    4
#define SHIFT_SHR    5
#define SHIFT_SAR    7


#define CC_O         0
#define CC_NO        1
#define CC_B         2
#define CC_C         2
#define CC_NAE       2
#define CC_AE        3
#define CC_NB        3
#define CC_NC        3
#define CC_E         4
#define CC_Z         4
#define CC_NE        5
#define CC_NZ        5
#define CC_BE        6
#define CC_NA        6
#define CC_A         7
#define CC_NBE       7
#define CC_S         8
#define CC_LZ        8
#define CC_NS        9
#define CC_GEZ       9
#define CC_P         0x0a
#define CC_PE        0x0a
#define CC_NP        0x0b
#define CC_PO        0x0b
#define CC_L         0x0c
#define CC_NGE       0x0c
#define CC_GE        0x0d
#define CC_NL        0x0d
#define CC_LE        0x0e
#define CC_NG        0x0e
#define CC_G         0x0f
#define CC_NLE       0x0f


/* modrm and stuff */

#define emit_address_byte(mod,reg,rm) \
    do { \
        *(cd->mcodeptr++) = ((((mod) & 0x03) << 6) | (((reg) & 0x07) << 3) | ((rm) & 0x07)); \
    } while (0);


#define emit_rex(size,reg,index,rm) \
    do { \
        if (((size) == 1) || ((reg) > 7) || ((index) > 7) || ((rm) > 7)) \
            *(cd->mcodeptr++) = (0x40 | (((size) & 0x01) << 3) | ((((reg) >> 3) & 0x01) << 2) | ((((index) >> 3) & 0x01) << 1) | (((rm) >> 3) & 0x01)); \
    } while (0)


#define emit_byte_rex(reg,index,rm) \
    do { \
        *(cd->mcodeptr++) = (0x40 | ((((reg) >> 3) & 0x01) << 2) | ((((index) >> 3) & 0x01) << 1) | (((rm) >> 3) & 0x01)); \
    } while (0)


#define emit_mem(r,disp) \
    do { \
        emit_address_byte(0,(r),5); \
        emit_imm32((disp)); \
    } while (0)


#define emit_imm8(imm) \
    do { \
        *(cd->mcodeptr++) = (u1) ((imm) & 0xff); \
    } while (0)


#define emit_imm16(imm) \
    do { \
        imm_buf imb; \
        imb.i = (s4) (imm); \
        *(cd->mcodeptr++) = imb.b[0]; \
        *(cd->mcodeptr++) = imb.b[1]; \
    } while (0)


#define emit_imm32(imm) \
    do { \
        imm_buf imb; \
        imb.i = (s4) (imm); \
        *(cd->mcodeptr++) = imb.b[0]; \
        *(cd->mcodeptr++) = imb.b[1]; \
        *(cd->mcodeptr++) = imb.b[2]; \
        *(cd->mcodeptr++) = imb.b[3]; \
    } while (0)


#define emit_imm64(imm) \
    do { \
        imm_buf imb; \
        imb.l = (s8) (imm); \
        *(cd->mcodeptr++) = imb.b[0]; \
        *(cd->mcodeptr++) = imb.b[1]; \
        *(cd->mcodeptr++) = imb.b[2]; \
        *(cd->mcodeptr++) = imb.b[3]; \
        *(cd->mcodeptr++) = imb.b[4]; \
        *(cd->mcodeptr++) = imb.b[5]; \
        *(cd->mcodeptr++) = imb.b[6]; \
        *(cd->mcodeptr++) = imb.b[7]; \
    } while (0)


/* convenience macros *********************************************************/

#define emit_reg(reg,rm)                emit_address_byte(3,(reg),(rm))


/* function prototypes ********************************************************/

void emit_cmovxx(codegendata *cd, instruction *iptr, s4 s, s4 d);


/* code generation prototypes */

void emit_ishift(jitdata *jd, s4 shift_op, instruction *iptr);
void emit_lshift(jitdata *jd, s4 shift_op, instruction *iptr);


/* integer instructions */

void emit_nop(codegendata *cd, int length);
void emit_mov_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_mov_imm_reg(codegendata *cd, s8 imm, s8 reg);
void emit_movl_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movl_imm_reg(codegendata *cd, s8 imm, s8 reg);
void emit_mov_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);
void emit_mov_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);
void emit_movl_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);
void emit_movl_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);
void emit_mov_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_mov_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movl_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movl_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_mov_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_movl_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_mov_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movl_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movw_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movb_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_mov_imm_membase(codegendata *cd, s8 imm, s8 basereg, s8 disp);
void emit_mov_imm_membase32(codegendata *cd, s8 imm, s8 basereg, s8 disp);
void emit_movl_imm_membase(codegendata *cd, s8 imm, s8 basereg, s8 disp);
void emit_movl_imm_membase32(codegendata *cd, s8 imm, s8 basereg, s8 disp);
void emit_movsbq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movsbq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movswq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movswq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movslq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movslq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movzbq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movzwq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movzwq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movswq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_movsbq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_movzwq_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_mov_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movl_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movw_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movb_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);

void emit_mov_mem_reg(codegendata *cd, s4 disp, s4 dreg);

void emit_alu_reg_reg(codegendata *cd, s8 opc, s8 reg, s8 dreg);
void emit_alul_reg_reg(codegendata *cd, s8 opc, s8 reg, s8 dreg);
void emit_alu_reg_membase(codegendata *cd, s8 opc, s8 reg, s8 basereg, s8 disp);
void emit_alul_reg_membase(codegendata *cd, s8 opc, s8 reg, s8 basereg, s8 disp);
void emit_alu_membase_reg(codegendata *cd, s8 opc, s8 basereg, s8 disp, s8 reg);
void emit_alul_membase_reg(codegendata *cd, s8 opc, s8 basereg, s8 disp, s8 reg);
void emit_alu_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg);
void emit_alu_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg);
void emit_alul_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 dreg);
void emit_alul_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg);
void emit_alu_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp);
void emit_alul_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp);
void emit_alu_memindex_reg(codegendata *cd, s8 opc, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_alul_memindex_reg(codegendata *cd, s8 opc, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 reg);
void emit_test_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_testl_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_test_imm_reg(codegendata *cd, s8 imm, s8 reg);
void emit_testw_imm_reg(codegendata *cd, s8 imm, s8 reg);
void emit_testb_imm_reg(codegendata *cd, s8 imm, s8 reg);
void emit_lea_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);
void emit_leal_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 reg);

void emit_incl_reg(codegendata *cd, s8 reg);
void emit_incq_reg(codegendata *cd, s8 reg);
void emit_incl_membase(codegendata *cd, s8 basereg, s8 disp);
void emit_incq_membase(codegendata *cd, s8 basereg, s8 disp);

void emit_cltd(codegendata *cd);
void emit_cqto(codegendata *cd);
void emit_imul_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_imull_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_imul_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_imull_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_imul_imm_reg(codegendata *cd, s8 imm, s8 dreg);
void emit_imul_imm_reg_reg(codegendata *cd, s8 imm,s8 reg, s8 dreg);
void emit_imull_imm_reg_reg(codegendata *cd, s8 imm, s8 reg, s8 dreg);
void emit_imul_imm_membase_reg(codegendata *cd, s8 imm, s8 basereg, s8 disp, s8 dreg);
void emit_imull_imm_membase_reg(codegendata *cd, s8 imm, s8 basereg, s8 disp, s8 dreg);
void emit_idiv_reg(codegendata *cd, s8 reg);
void emit_idivl_reg(codegendata *cd, s8 reg);
void emit_shift_reg(codegendata *cd, s8 opc, s8 reg);
void emit_shiftl_reg(codegendata *cd, s8 opc, s8 reg);
void emit_shift_membase(codegendata *cd, s8 opc, s8 basereg, s8 disp);
void emit_shiftl_membase(codegendata *cd, s8 opc, s8 basereg, s8 disp);
void emit_shift_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg);
void emit_shiftl_imm_reg(codegendata *cd, s8 opc, s8 imm, s8 dreg);
void emit_shift_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp);
void emit_shiftl_imm_membase(codegendata *cd, s8 opc, s8 imm, s8 basereg, s8 disp);
void emit_jmp_imm(codegendata *cd, s8 imm);
void emit_jmp_reg(codegendata *cd, s8 reg);
void emit_jcc(codegendata *cd, s8 opc, s8 imm);

void emit_setcc_reg(codegendata *cd, s4 opc, s4 reg);
void emit_setcc_membase(codegendata *cd, s4 opc, s4 basereg, s4 disp);

void emit_cmovcc_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg);
void emit_cmovccl_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg);

void emit_neg_reg(codegendata *cd, s8 reg);
void emit_negl_reg(codegendata *cd, s8 reg);
void emit_neg_membase(codegendata *cd, s8 basereg, s8 disp);
void emit_negl_membase(codegendata *cd, s8 basereg, s8 disp);
void emit_push_reg(codegendata *cd, s8 reg);
void emit_push_imm(codegendata *cd, s8 imm);
void emit_pop_reg(codegendata *cd, s8 reg);
void emit_xchg_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_call_reg(codegendata *cd, s8 reg);
void emit_call_imm(codegendata *cd, s8 imm);
void emit_call_mem(codegendata *cd, ptrint mem);


/* floating point instructions (SSE2) */

void emit_addsd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_addss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtsi2ssq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtsi2ss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtsi2sdq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtsi2sd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtss2sd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvtsd2ss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvttss2siq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvttss2si_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvttsd2siq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_cvttsd2si_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_divss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_divsd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movd_reg_freg(codegendata *cd, s8 reg, s8 freg);
void emit_movd_freg_reg(codegendata *cd, s8 freg, s8 reg);
void emit_movd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movd_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movdl_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movd_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg);
void emit_movq_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movq_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movq_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movsd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_movss_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movss_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movsd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movsd_reg_membase32(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movss_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movss_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movlps_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movlps_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movsd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movsd_membase32_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movlpd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_movlpd_reg_membase(codegendata *cd, s8 reg, s8 basereg, s8 disp);
void emit_movss_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movsd_reg_memindex(codegendata *cd, s8 reg, s8 disp, s8 basereg, s8 indexreg, s8 scale);
void emit_movss_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg);
void emit_movsd_memindex_reg(codegendata *cd, s8 disp, s8 basereg, s8 indexreg, s8 scale, s8 dreg);
void emit_mulss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_mulsd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_subss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_subsd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_ucomiss_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_ucomisd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_xorps_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_xorps_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);
void emit_xorpd_reg_reg(codegendata *cd, s8 reg, s8 dreg);
void emit_xorpd_membase_reg(codegendata *cd, s8 basereg, s8 disp, s8 dreg);


/* system instructions ********************************************************/

void emit_rdtsc(codegendata *cd);
void emit_mfence(codegendata *cd);


/**
 * Emit code to recompute the procedure vector. This is a nop,
 * because we do not use a procedure vector.
 */
static inline void emit_recompute_pv(codegendata* cd) {}

void emit_arbitrary_nop(codegendata *cd, int disp);

#endif // MD_EMIT_HPP_


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
