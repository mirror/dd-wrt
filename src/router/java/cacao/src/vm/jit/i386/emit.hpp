/* src/vm/jit/i386/emit.hpp - machine dependent emit function prototypes

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


#define REG_AL       0
#define REG_CL       1
#define REG_DL       2
#define REG_BL       3
#define REG_AH       4
#define REG_CH       5
#define REG_DH       6
#define REG_BH       7


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
        *(cd->mcodeptr++) = ((((mod) & 0x03) << 6) | (((reg) & 0x07) << 3) | (((rm) & 0x07))); \
    } while (0)


#define emit_imm8(imm) \
    do { \
        *(cd->mcodeptr++) = (u1) ((imm) & 0xff); \
    } while (0)


#define emit_imm16(imm) \
    do { \
        imm_union imb; \
        imb.i = (int) (imm); \
        *(cd->mcodeptr++) = imb.b[0]; \
        *(cd->mcodeptr++) = imb.b[1]; \
    } while (0)


#define emit_imm32(imm) \
    do { \
        imm_union imb; \
        imb.i = (int) (imm); \
        *(cd->mcodeptr++) = imb.b[0]; \
        *(cd->mcodeptr++) = imb.b[1]; \
        *(cd->mcodeptr++) = imb.b[2]; \
        *(cd->mcodeptr++) = imb.b[3]; \
    } while (0)


#define emit_mem(r,mem) \
    do { \
        emit_address_byte(0,(r),5); \
        emit_imm32((mem)); \
    } while (0)


/* convenience macros *********************************************************/

#define emit_reg(reg,rm)                emit_address_byte(3,(reg),(rm))

/* integer instructions */

void emit_mov_reg_reg(codegendata *cd, s4 reg, s4 dreg);
void emit_mov_imm_reg(codegendata *cd, s4 imm, s4 dreg);
void emit_mov_imm2_reg(codegendata *cd, s4 imm, s4 dreg);
void emit_movb_imm_reg(codegendata *cd, s4 imm, s4 dreg);
void emit_mov_membase_reg(codegendata *cd, s4 basereg, s4 disp, s4 reg);
void emit_mov_membase32_reg(codegendata *cd, s4 basereg, s4 disp, s4 reg);
void emit_mov_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp);
void emit_mov_reg_membase32(codegendata *cd, s4 reg, s4 basereg, s4 disp);
void emit_mov_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg);
void emit_mov_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movw_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movb_reg_memindex(codegendata *cd, s4 reg, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_mov_reg_mem(codegendata *cd, s4 reg, s4 mem);
void emit_mov_mem_reg(codegendata *cd, s4 mem, s4 dreg);
void emit_mov_imm_mem(codegendata *cd, s4 imm, s4 mem);
void emit_mov_imm_membase(codegendata *cd, s4 imm, s4 basereg, s4 disp);
void emit_mov_imm_membase32(codegendata *cd, s4 imm, s4 basereg, s4 disp);
void emit_movb_imm_membase(codegendata *cd, s4 imm, s4 basereg, s4 disp);
void emit_movsbl_reg_reg(codegendata *cd, s4 a, s4 b);
void emit_movsbl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg);
void emit_movswl_reg_reg(codegendata *cd, s4 a, s4 b);
void emit_movswl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg);
void emit_movzbl_reg_reg(codegendata *cd, s4 a, s4 b);
void emit_movzwl_reg_reg(codegendata *cd, s4 a, s4 b);
void emit_movzwl_memindex_reg(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg);
void emit_mov_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movw_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_movb_imm_memindex(codegendata *cd, s4 imm, s4 disp, s4 basereg, s4 indexreg, s4 scale);

void emit_alu_reg_reg(codegendata *cd, s4 opc, s4 reg, s4 dreg);
void emit_alu_reg_membase(codegendata *cd, s4 opc, s4 reg, s4 basereg, s4 disp);
void emit_alu_membase_reg(codegendata *cd, s4 opc, s4 basereg, s4 disp, s4 reg);
void emit_alu_imm_reg(codegendata *cd, s4 opc, s4 imm, s4 reg);
void emit_alu_imm32_reg(codegendata *cd, s4 opc, s4 imm, s4 reg);
void emit_alu_imm_membase(codegendata *cd, s4 opc, s4 imm, s4 basereg, s4 disp);
void emit_alu_imm_memabs(codegendata *cd, s4 opc, s4 imm, s4 disp);
void emit_alu_memindex_reg(codegendata *cd, s4 opc, s4 disp, s4 basereg, s4 indexreg, s4 scale, s4 reg);
void emit_inc_reg(codegendata *cd, s4 reg);
void emit_test_reg_reg(codegendata *cd, s4 reg, s4 dreg);
void emit_test_imm_reg(codegendata *cd, s4 imm, s4 dreg);
void emit_dec_mem(codegendata *cd, s4 mem);
void emit_imul_reg_reg(codegendata *cd, s4 reg, s4 dreg);
void emit_imul_membase_reg(codegendata *cd, s4 basereg, s4 disp, s4 dreg);
void emit_imul_imm_reg(codegendata *cd, s4 imm, s4 reg);
void emit_imul_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg);
void emit_imul_imm_membase_reg(codegendata *cd, s4 imm, s4 basereg, s4 disp, s4 dreg);
void emit_mul_reg(codegendata *cd, s4 reg);
void emit_idiv_reg(codegendata *cd, s4 reg);
void emit_shift_reg(codegendata *cd, s4 opc, s4 reg);
void emit_shift_imm_reg(codegendata *cd, s4 opc, s4 imm, s4 reg);
void emit_shld_reg_reg(codegendata *cd, s4 reg, s4 dreg);
void emit_shld_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg);
void emit_shld_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp);
void emit_shrd_reg_reg(codegendata *cd, s4 reg, s4 dreg);
void emit_shrd_imm_reg_reg(codegendata *cd, s4 imm, s4 reg, s4 dreg);
void emit_shrd_reg_membase(codegendata *cd, s4 reg, s4 basereg, s4 disp);
void emit_jmp_imm(codegendata *cd, s4 imm);
void emit_jmp_reg(codegendata *cd, s4 reg);
void emit_jcc(codegendata *cd, s4 opc, s4 imm);
void emit_setcc_reg(codegendata *cd, s4 opc, s4 reg);
void emit_setcc_membase(codegendata *cd, s4 opc, s4 basereg, s4 disp);
void emit_xadd_reg_mem(codegendata *cd, s4 reg, s4 mem);
void emit_neg_reg(codegendata *cd, s4 reg);
void emit_push_imm(codegendata *cd, s4 imm);
void emit_pop_reg(codegendata *cd, s4 reg);
void emit_push_reg(codegendata *cd, s4 reg);
void emit_lock(codegendata *cd);
void emit_call_reg(codegendata *cd, s4 reg);
void emit_call_imm(codegendata *cd, s4 imm);
void emit_call_mem(codegendata *cd, s4 mem);


/* floating point instructions */

void emit_fld1(codegendata *cd);
void emit_fldz(codegendata *cd);
void emit_fld_reg(codegendata *cd, s4 reg);
void emit_flds_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_flds_membase32(codegendata *cd, s4 basereg, s4 disp);
void emit_fldl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fldl_membase32(codegendata *cd, s4 basereg, s4 disp);
void emit_fldt_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_flds_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_fldl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_flds_mem(codegendata *cd, s4 mem);
void emit_fldl_mem(codegendata *cd, s4 mem);
void emit_fildl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fildll_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fst_reg(codegendata *cd, s4 reg);
void emit_fsts_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fstl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fsts_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_fstl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_fstp_reg(codegendata *cd, s4 reg);
void emit_fstps_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fstps_membase32(codegendata *cd, s4 basereg, s4 disp);
void emit_fstpl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fstpl_membase32(codegendata *cd, s4 basereg, s4 disp);
void emit_fstpt_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fstps_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_fstpl_memindex(codegendata *cd, s4 disp, s4 basereg, s4 indexreg, s4 scale);
void emit_fstps_mem(codegendata *cd, s4 mem);
void emit_fstpl_mem(codegendata *cd, s4 mem);
void emit_fistl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fistpl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fistpll_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fchs(codegendata *cd);
void emit_faddp(codegendata *cd);
void emit_fadd_reg_st(codegendata *cd, s4 reg);
void emit_fadd_st_reg(codegendata *cd, s4 reg);
void emit_faddp_st_reg(codegendata *cd, s4 reg);
void emit_fadds_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_faddl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fsub_reg_st(codegendata *cd, s4 reg);
void emit_fsub_st_reg(codegendata *cd, s4 reg);
void emit_fsubp_st_reg(codegendata *cd, s4 reg);
void emit_fsubp(codegendata *cd);
void emit_fsubs_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fsubl_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fmul_reg_st(codegendata *cd, s4 reg);
void emit_fmul_st_reg(codegendata *cd, s4 reg);
void emit_fmulp(codegendata *cd);
void emit_fmulp_st_reg(codegendata *cd, s4 reg);
void emit_fmuls_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fmull_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_fdiv_reg_st(codegendata *cd, s4 reg);
void emit_fdiv_st_reg(codegendata *cd, s4 reg);
void emit_fdivp(codegendata *cd);
void emit_fdivp_st_reg(codegendata *cd, s4 reg);
void emit_fxch(codegendata *cd);
void emit_fxch_reg(codegendata *cd, s4 reg);
void emit_fprem(codegendata *cd);
void emit_fprem1(codegendata *cd);
void emit_fucom(codegendata *cd);
void emit_fucom_reg(codegendata *cd, s4 reg);
void emit_fucomp_reg(codegendata *cd, s4 reg);
void emit_fucompp(codegendata *cd);
void emit_fnstsw(codegendata *cd);
void emit_sahf(codegendata *cd);
void emit_finit(codegendata *cd);
void emit_fldcw_mem(codegendata *cd, s4 mem);
void emit_fldcw_membase(codegendata *cd, s4 basereg, s4 disp);
void emit_wait(codegendata *cd);
void emit_ffree_reg(codegendata *cd, s4 reg);
void emit_fdecstp(codegendata *cd);
void emit_fincstp(codegendata *cd);

#if defined(ENABLE_ESCAPE_CHECK)
void emit_escape_check(codegendata *cd, s4 reg);
void emit_escape_annotate_object(codegendata *cd, methodinfo *m);
#endif


/**
 * Emit code to recompute the procedure vector. This is a nop,
 * because we do not use a procedure vector.
 */
static inline void emit_recompute_pv(codegendata* cd) {}

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
 */
