/* src/vm/jit/x86_64/codegen.hpp - code generation macros for x86_64

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


#ifndef CODEGEN_HPP_
#define CODEGEN_HPP_ 1

#include "config.h"

#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/x86_64/emit.hpp"

/* additional functions and macros to generate code ***************************/

/* MCODECHECK(icnt) */

#define MCODECHECK(icnt) \
    do { \
        if ((cd->mcodeptr + (icnt)) > cd->mcodeend) \
            codegen_increase(cd); \
    } while (0)


#define ALIGNCODENOP \
    do { \
        int len = (-(ptrint) cd->mcodeptr) & 7; \
        if (len) \
            emit_nop(cd, len); \
    } while (0)


#define PATCH_ALIGNMENT(addr, offset, size) \
	((((addr)+(offset)+(size)-1) & ~((size)-1)) - ((addr)+(offset)))


#define ICONST(r,c) \
    do { \
        if ((c) == 0) \
            M_CLR((d)); \
        else \
            M_IMOV_IMM((c), (d)); \
    } while (0)
/*     do { \ */
/*        M_IMOV_IMM((c), (d)); \ */
/*     } while (0) */


#define LCONST(r,c) \
    do { \
        if ((c) == 0) \
            M_CLR((d)); \
        else \
            M_MOV_IMM((c), (d)); \
    } while (0)


/* branch defines *************************************************************/

#define BRANCH_UNCONDITIONAL_SIZE    5  /* size in bytes of a branch          */
#define BRANCH_CONDITIONAL_SIZE      6  /* size in bytes of a branch          */

/* These NOPs are never executed; they are only used as placeholders during
 * code generation.
 */
#define BRANCH_NOPS \
    do { \
        M_NOP; \
        M_NOP; \
        M_NOP; \
        M_NOP; \
        M_NOP; \
        M_NOP; \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_SIZE    2          /* size in bytes of a patcher call    */

#define PATCHER_NOPS \
    do { \
        emit_nop(cd, 2); \
    } while (0)


/* macros to create code ******************************************************/

#define M_BYTE1(a) \
    do { \
        *(cd->mcodeptr) = (a); \
        cd->mcodeptr++; \
    } while (0)


#define M_BYTE2(a, b) \
    do { \
        M_BYTE1(a); \
        M_BYTE1(b); \
    } while (0)


#define M_MOV(a,b)              emit_mov_reg_reg(cd, (a), (b))
#define M_MOV_IMM(a,b)          emit_mov_imm_reg(cd, (u8) (a), (b))

#define M_IMOV(a,b)             emit_movl_reg_reg(cd, (a), (b))
#define M_IMOV_IMM(a,b)         emit_movl_imm_reg(cd, (u4) (a), (b))

#define M_FMOV(a,b)             emit_movq_reg_reg(cd, (a), (b))
#define M_DMOV(a,b)             M_FMOV(a,b)

#define M_ILD(a,b,disp)         emit_movl_membase_reg(cd, (b), (disp), (a))
#define M_LLD(a,b,disp)         emit_mov_membase_reg(cd, (b), (disp), (a))

#define M_ILD32(a,b,disp)       emit_movl_membase32_reg(cd, (b), (disp), (a))
#define M_LLD32(a,b,disp)       emit_mov_membase32_reg(cd, (b), (disp), (a))

#define M_IST(a,b,disp)         emit_movl_reg_membase(cd, (a), (b), (disp))
#define M_LST(a,b,disp)         emit_mov_reg_membase(cd, (a), (b), (disp))

#define M_IST_IMM(a,b,disp)     emit_movl_imm_membase(cd, (a), (b), (disp))
#define M_LST_IMM32(a,b,disp)   emit_mov_imm_membase(cd, (a), (b), (disp))

#define M_IST32(a,b,disp)       emit_movl_reg_membase32(cd, (a), (b), (disp))
#define M_LST32(a,b,disp)       emit_mov_reg_membase32(cd, (a), (b), (disp))

#define M_IST32_IMM(a,b,disp)   emit_movl_imm_membase32(cd, (a), (b), (disp))
#define M_LST32_IMM32(a,b,disp) emit_mov_imm_membase32(cd, (a), (b), (disp))

#define M_IADD(a,b)             emit_alul_reg_reg(cd, ALU_ADD, (a), (b))
#define M_ISUB(a,b)             emit_alul_reg_reg(cd, ALU_SUB, (a), (b))
#define M_IMUL(a,b)             emit_imull_reg_reg(cd, (a), (b))

#define M_IADD_IMM(a,b)         emit_alul_imm_reg(cd, ALU_ADD, (a), (b))
#define M_ISUB_IMM(a,b)         emit_alul_imm_reg(cd, ALU_SUB, (a), (b))
#define M_IMUL_IMM(a,b,c)       emit_imull_imm_reg_reg(cd, (b), (a), (c))

#define M_LADD(a,b)             emit_alu_reg_reg(cd, ALU_ADD, (a), (b))
#define M_LSUB(a,b)             emit_alu_reg_reg(cd, ALU_SUB, (a), (b))
#define M_LMUL(a,b)             emit_imul_reg_reg(cd, (a), (b))

#define M_LADD_IMM(a,b)         emit_alu_imm_reg(cd, ALU_ADD, (a), (b))
#define M_LSUB_IMM(a,b)         emit_alu_imm_reg(cd, ALU_SUB, (a), (b))
#define M_LMUL_IMM(a,b,c)       emit_imul_imm_reg_reg(cd, (b), (a), (c))

#define M_IINC(a)               emit_incl_reg(cd, (a))
#define M_LINC(a)               emit_incq_reg(cd, (a))
#define M_IDEC(a)               emit_decl_reg(cd, (a))

#define M_ALD(a,b,disp) \
    do { \
        if (b == RIP) \
            M_LLD(a, b, disp + -((cd->mcodeptr + 7) - cd->mcodebase)); \
        else \
            M_LLD(a, b, disp); \
    } while (0)

#define M_ALD32(a,b,disp)       M_LLD32(a,b,disp)
#define M_ALD_DSEG(a,disp)      M_ALD(a,RIP,disp)

#define M_ALD_MEM(a,disp)       emit_mov_mem_reg(cd, (disp), (a))

#define M_ALD_MEM_GET_OPC(p)     (  *(        (p) + 1))
#define M_ALD_MEM_GET_MOD(p)     (((*(        (p) + 2)) >> 6) & 0x03)
#define M_ALD_MEM_GET_REG(p)    ((((*(        (p) + 2)) >> 3) & 0x07) + (((*(p) >> 2) & 0x01) << 3))
#define M_ALD_MEM_GET_RM(p)      (((*(        (p) + 2))     ) & 0x07)
#define M_ALD_MEM_GET_DISP(p)    (  *((u4 *) ((p) + 4)))

#define M_AST(a,b,c)            M_LST(a,b,c)
#define M_AST_IMM32(a,b,c)      M_LST_IMM32(a,b,c)

#define M_AADD(a,b)             M_LADD(a,b)
#define M_AADD_IMM(a,b)         M_LADD_IMM(a,b)
#define M_ASUB_IMM(a,b)         M_LSUB_IMM(a,b)

#define M_ISUB_IMM32(a,b)       emit_alul_imm32_reg(cd, ALU_SUB, (a), (b))

#define M_LADD_IMM32(a,b)       emit_alu_imm32_reg(cd, ALU_ADD, (a), (b))
#define M_LSUB_IMM32(a,b)       emit_alu_imm32_reg(cd, ALU_SUB, (a), (b))

#define M_AADD_IMM32(a,b)       M_LADD_IMM32(a,b)

#define M_ILEA(a,b,c)           emit_leal_membase_reg(cd, (a), (b), (c))
#define M_LLEA(a,b,c)           emit_lea_membase_reg(cd, (a), (b), (c))
#define M_ALEA(a,b,c)           M_LLEA(a,b,c)

#define M_INEG(a)               emit_negl_reg(cd, (a))
#define M_LNEG(a)               emit_neg_reg(cd, (a))

#define M_IAND(a,b)             emit_alul_reg_reg(cd, ALU_AND, (a), (b))
#define M_IOR(a,b)              emit_alul_reg_reg(cd, ALU_OR, (a), (b))
#define M_IXOR(a,b)             emit_alul_reg_reg(cd, ALU_XOR, (a), (b))

#define M_IAND_IMM(a,b)         emit_alul_imm_reg(cd, ALU_AND, (a), (b))
#define M_IOR_IMM(a,b)          emit_alul_imm_reg(cd, ALU_OR, (a), (b))
#define M_IXOR_IMM(a,b)         emit_alul_imm_reg(cd, ALU_XOR, (a), (b))

#define M_LAND(a,b)             emit_alu_reg_reg(cd, ALU_AND, (a), (b))
#define M_LOR(a,b)              emit_alu_reg_reg(cd, ALU_OR, (a), (b))
#define M_LXOR(a,b)             emit_alu_reg_reg(cd, ALU_XOR, (a), (b))

#define M_LAND_IMM(a,b)         emit_alu_imm_reg(cd, ALU_AND, (a), (b))
#define M_LOR_IMM(a,b)          emit_alu_imm_reg(cd, ALU_OR, (a), (b))
#define M_LXOR_IMM(a,b)         emit_alu_imm_reg(cd, ALU_XOR, (a), (b))

#define M_BSEXT(a,b)            emit_movsbq_reg_reg(cd, (a), (b))
#define M_SSEXT(a,b)            emit_movswq_reg_reg(cd, (a), (b))
#define M_ISEXT(a,b)            emit_movslq_reg_reg(cd, (a), (b))

#define M_BZEXT(a,b)            emit_movzbq_reg_reg(cd, (a), (b))
#define M_CZEXT(a,b)            emit_movzwq_reg_reg(cd, (a), (b))

#define M_ISLL_IMM(a,b)         emit_shiftl_imm_reg(cd, SHIFT_SHL, (a), (b))
#define M_ISRA_IMM(a,b)         emit_shiftl_imm_reg(cd, SHIFT_SAR, (a), (b))
#define M_ISRL_IMM(a,b)         emit_shiftl_imm_reg(cd, SHIFT_SHR, (a), (b))

#define M_LSLL_IMM(a,b)         emit_shift_imm_reg(cd, SHIFT_SHL, (a), (b))
#define M_LSRA_IMM(a,b)         emit_shift_imm_reg(cd, SHIFT_SAR, (a), (b))
#define M_LSRL_IMM(a,b)         emit_shift_imm_reg(cd, SHIFT_SHR, (a), (b))

#define M_TEST(a)               emit_test_reg_reg(cd, (a), (a))
#define M_ITEST(a)              emit_testl_reg_reg(cd, (a), (a))

#define M_LCMP(a,b)             emit_alu_reg_reg(cd, ALU_CMP, (a), (b))
#define M_LCMP_IMM(a,b)         emit_alu_imm_reg(cd, ALU_CMP, (a), (b))
#define M_LCMP_IMM_MEMBASE(a,b,c) emit_alu_imm_membase(cd, ALU_CMP, (a), (b), (c))
#define M_LCMP_MEMBASE(a,b,c)   emit_alu_membase_reg(cd, ALU_CMP, (a), (b), (c))
#define M_LCMP_MEMINDEX(a,b,c,d,e) emit_alul_memindex_reg(cd, ALU_CMP, (b), (a), (c), (d), (e))

#define M_ICMP(a,b)             emit_alul_reg_reg(cd, ALU_CMP, (a), (b))
#define M_ICMP_IMM(a,b)         emit_alul_imm_reg(cd, ALU_CMP, (a), (b))
#define M_ICMP_IMM32(a,b)       emit_alul_imm32_reg(cd, ALU_CMP, (a), (b))
#define M_ICMP_IMM_MEMBASE(a,b,c) emit_alul_imm_membase(cd, ALU_CMP, (a), (b), (c))
#define M_ICMP_MEMBASE(a,b,c)   emit_alul_membase_reg(cd, ALU_CMP, (a), (b), (c))
#define M_ICMP_MEMINDEX(a,b,c,d,e) emit_alu_memindex_reg(cd, ALU_CMP, (b), (a), (c), (d), (e))

#define M_ACMP(a,b)             M_LCMP(a,b)

#define M_BEQ(disp)             emit_jcc(cd, CC_E, (disp))
#define M_BNE(disp)             emit_jcc(cd, CC_NE, (disp))
#define M_BLT(disp)             emit_jcc(cd, CC_L, (disp))
#define M_BLE(disp)             emit_jcc(cd, CC_LE, (disp))
#define M_BGE(disp)             emit_jcc(cd, CC_GE, (disp))
#define M_BGT(disp)             emit_jcc(cd, CC_G, (disp))

#define M_BULT(disp)            emit_jcc(cd, CC_B, (disp))
#define M_BULE(disp)            emit_jcc(cd, CC_BE, (disp))
#define M_BUGE(disp)            emit_jcc(cd, CC_AE, (disp))
#define M_BUGT(disp)            emit_jcc(cd, CC_A, (disp))

#define M_SETE(a)               emit_setcc_reg(cd, CC_E, (a))
#define M_SETNE(a)              emit_setcc_reg(cd, CC_NE, (a))
#define M_SETULE(a)             emit_setcc_reg(cd, CC_BE, (a))

#define M_CMOVEQ(a,b)           emit_cmovcc_reg_reg(cd, CC_E, (a), (b))
#define M_CMOVNE(a,b)           emit_cmovcc_reg_reg(cd, CC_NE, (a), (b))
#define M_CMOVLT(a,b)           emit_cmovcc_reg_reg(cd, CC_L, (a), (b))
#define M_CMOVLE(a,b)           emit_cmovcc_reg_reg(cd, CC_LE, (a), (b))
#define M_CMOVGE(a,b)           emit_cmovcc_reg_reg(cd, CC_GE, (a), (b))
#define M_CMOVGT(a,b)           emit_cmovcc_reg_reg(cd, CC_G, (a), (b))

#define M_CMOVULT(a,b)          emit_cmovcc_reg_reg(cd, CC_B, (a), (b))
#define M_CMOVUGT(a,b)          emit_cmovcc_reg_reg(cd, CC_A, (a), (b))
#define M_CMOVP(a,b)            emit_cmovcc_reg_reg(cd, CC_P, (a), (b))

#define M_PUSH(a)               emit_push_reg(cd, (a))
#define M_PUSH_IMM(a)           emit_push_imm(cd, (a))
#define M_POP(a)                emit_pop_reg(cd, (a))

#define M_JMP(a)                emit_jmp_reg(cd, (a))
#define M_JMP_IMM(a)            emit_jmp_imm(cd, (a))
#define M_JMP_IMM2(a)           emit_jmp_imm2(cd, (a))
#define M_CALL(a)               emit_call_reg(cd, (a))
#define M_CALL_IMM(a)           emit_call_imm(cd, (a))
#define M_RET                   M_BYTE1(0xc3)

#define M_NOP                   M_BYTE1(0x90)
#define M_UD2                   M_BYTE2(0x0f, 0x0b)

#define M_CLR(a)                M_LXOR(a,a)


#define M_FLD(a,b,disp)         emit_movss_membase_reg(cd, (b), (disp), (a))
#define M_DLD(a,b,disp)         emit_movsd_membase_reg(cd, (b), (disp), (a))

#define M_FLD32(a,b,disp)       emit_movss_membase32_reg(cd, (b), (disp), (a))
#define M_DLD32(a,b,disp)       emit_movsd_membase32_reg(cd, (b), (disp), (a))

#define M_FST(a,b,disp)         emit_movss_reg_membase(cd, (a), (b), (disp))
#define M_DST(a,b,disp)         emit_movsd_reg_membase(cd, (a), (b), (disp))

#define M_FST32(a,b,disp)       emit_movss_reg_membase32(cd, (a), (b), (disp))
#define M_DST32(a,b,disp)       emit_movsd_reg_membase32(cd, (a), (b), (disp))

#define M_FADD(a,b)             emit_addss_reg_reg(cd, (a), (b))
#define M_DADD(a,b)             emit_addsd_reg_reg(cd, (a), (b))
#define M_FSUB(a,b)             emit_subss_reg_reg(cd, (a), (b))
#define M_DSUB(a,b)             emit_subsd_reg_reg(cd, (a), (b))
#define M_FMUL(a,b)             emit_mulss_reg_reg(cd, (a), (b))
#define M_DMUL(a,b)             emit_mulsd_reg_reg(cd, (a), (b))
#define M_FDIV(a,b)             emit_divss_reg_reg(cd, (a), (b))
#define M_DDIV(a,b)             emit_divsd_reg_reg(cd, (a), (b))

#define M_CVTIF(a,b)            emit_cvtsi2ss_reg_reg(cd, (a), (b))
#define M_CVTID(a,b)            emit_cvtsi2sd_reg_reg(cd, (a), (b))
#define M_CVTLF(a,b)            emit_cvtsi2ssq_reg_reg(cd, (a), (b))
#define M_CVTLD(a,b)            emit_cvtsi2sdq_reg_reg(cd, (a), (b))
#define M_CVTFI(a,b)            emit_cvttss2si_reg_reg(cd, (a), (b))
#define M_CVTDI(a,b)            emit_cvttsd2si_reg_reg(cd, (a), (b))
#define M_CVTFL(a,b)            emit_cvttss2siq_reg_reg(cd, (a), (b))
#define M_CVTDL(a,b)            emit_cvttsd2siq_reg_reg(cd, (a), (b))

#define M_CVTFD(a,b)            emit_cvtss2sd_reg_reg(cd, (a), (b))
#define M_CVTDF(a,b)            emit_cvtsd2ss_reg_reg(cd, (a), (b))


/* system instructions ********************************************************/

#define M_MFENCE                emit_mfence(cd)
#define M_RDTSC                 emit_rdtsc(cd)

#define M_IINC_MEMBASE(a,b)     emit_incl_membase(cd, (a), (b))
#define M_LINC_MEMBASE(a,b)     emit_incq_membase(cd, (a), (b))

#define M_IADD_MEMBASE(a,b,c)   emit_alul_reg_membase(cd, ALU_ADD, (a), (b), (c))
#define M_IADC_MEMBASE(a,b,c)   emit_alul_reg_membase(cd, ALU_ADC, (a), (b), (c))
#define M_ISUB_MEMBASE(a,b,c)   emit_alul_reg_membase(cd, ALU_SUB, (a), (b), (c))
#define M_ISBB_MEMBASE(a,b,c)   emit_alul_reg_membase(cd, ALU_SBB, (a), (b), (c))


#endif // CODEGEN_HPP_

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
