/* src/vm/jit/powerpc64/codegen.hpp - code generation macros and
                                      definitions for PowerPC64

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

#include "md-abi.hpp"

#include "vm/global.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/reg.hpp"


/* additional functions and macros to generate code ***************************/

/* MCODECHECK(icnt) */

#define MCODECHECK(icnt) \
    do { \
        if ((cd->mcodeptr + (icnt) * 4) > cd->mcodeend) \
            codegen_increase(cd); \
    } while (0)


#define ICONST(d,c)                     emit_iconst(cd, (d), (c))
#define LCONST(reg,c) 			emit_lconst(cd, (reg), (c))


#define ALIGNCODENOP \
    if ((s4) ((ptrint) cd->mcodeptr & 7)) { \
        M_NOP; \
    }


/* branch defines *************************************************************/
/* and additional branch is needed when generating long branches */
#define BRANCH_NOPS \
    do { \
    	if (CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) {\
		M_NOP; \
	} \
        M_NOP; \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_SIZE    1 * 4      /* an instruction is 4-bytes long     */

#define PATCHER_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* macros to create code ******************************************************/

#define M_OP3(opcode,y,oe,rc,d,a,b) \
    do { \
        *((u4 *) cd->mcodeptr) = (((opcode) << 26) | ((d) << 21) | ((a) << 16) | ((b) << 11) | ((oe) << 10) | ((y) << 1) | (rc)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_OP4(x,y,rc,d,a,b,c) \
    do { \
        *((u4 *) cd->mcodeptr) = (((x) << 26) | ((d) << 21) | ((a) << 16) | ((b) << 11) | ((c) << 6) | ((y) << 1) | (rc)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_OP2_IMM(x,d,a,i) \
    do { \
        *((u4 *) cd->mcodeptr) = (((x) << 26) | ((d) << 21) | ((a) << 16) | ((i) & 0xffff)); \
        cd->mcodeptr += 4; \
    } while (0)

/* for instruction decodeing */
#define M_INSTR_OP2_IMM_D(x)            (((x) >> 21) & 0x1f  )
#define M_INSTR_OP2_IMM_A(x)            (((x) >> 16) & 0x1f  )
#define M_INSTR_OP2_IMM_I(x)            ( (x)        & 0xffff)

#define M_BCMASK     0x0000fffc                     /* (((1 << 16) - 1) & ~3) */
#define M_BMASK    0x03fffffc                     /* (((1 << 26) - 1) & ~3) */

#define M_B(x,i,a,l) \
    do { \
        *((u4 *) cd->mcodeptr) = (((x) << 26) | ((((i) * 4) + 4) & M_BMASK) | ((a) << 1) | (l)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_BC(x,bo,bi,i,a,l) \
    do { \
        *((u4 *) cd->mcodeptr) = (((x) << 26) | ((bo) << 21) | ((bi) << 16) | (((i) * 4 + 4) & M_BCMASK) | ((a) << 1) | (l)); \
        cd->mcodeptr += 4; \
    } while (0)



#define M_EXTSW(a,b)			M_OP3(31, 986, 0, 0, a, b, 0)


/* instruction macros *********************************************************/

#define M_ILLEGAL                       M_OP3(0, 0, 0, 0, 0, 0, 0)
#define M_IADD(a,b,c)                   M_LADD(a,b,c)
#define M_LADD(a,b,c)                   M_OP3(31, 266, 0, 0, c, a, b) 
#define M_IADD_IMM(a,b,c)               M_OP2_IMM(14, c, a, b)	/* XXX */
#define M_LADD_IMM(a,b,c)               M_OP2_IMM(14, c, a, b)
#define M_ADDC(a,b,c)                   M_OP3(31, 10, 0, 0, c, a, b)
#define M_ADDIC(a,b,c)                  M_OP2_IMM(12, c, a, b)
#define M_ADDICTST(a,b,c)               M_OP2_IMM(13, c, a, b)
#define M_ADDE(a,b,c)                   M_OP3(31, 138, 0, 0, c, a, b)
#define M_ADDZE(a,b)                    M_OP3(31, 202, 0, 0, b, a, 0)
#define M_ADDME(a,b)                    M_OP3(31, 234, 0, 0, b, a, 0)

#define M_SUB(a,b,c)                    M_OP3(31, 40, 0, 0, c, b, a)
#define M_ISUBTST(a,b,c)                M_OP3(31, 40, 0, 1, c, b, a)
#define M_SUBC(a,b,c)                   M_OP3(31, 8, 0, 0, c, b, a)
#define M_SUBIC(a,b,c)                  M_OP2_IMM(8, c, b, a)
#define M_SUBE(a,b,c)                   M_OP3(31, 136, 0, 0, c, b, a)
#define M_SUBZE(a,b)                    M_OP3(31, 200, 0, 0, b, a, 0)
#define M_SUBME(a,b)                    M_OP3(31, 232, 0, 0, b, a, 0)

#define M_AND(a,b,c)                    M_OP3(31, 28, 0, 0, a, c, b)
#define M_AND_IMM(a,b,c)                M_OP2_IMM(28, a, c, b)
#define M_ANDIS(a,b,c)                  M_OP2_IMM(29, a, c, b)
#define M_OR(a,b,c)                     M_OP3(31, 444, 0, 0, a, c, b)
#define M_OR_TST(a,b,c)                 M_OP3(31, 444, 0, 1, a, c, b)
#define M_OR_IMM(a,b,c)                 M_OP2_IMM(24, a, c, b)
#define M_ORIS(a,b,c)                   M_OP2_IMM(25, a, c, b)
#define M_XOR(a,b,c)                    M_OP3(31, 316, 0, 0, a, c, b)
#define M_XOR_IMM(a,b,c)                M_OP2_IMM(26, a, c, b)
#define M_XORIS(a,b,c)                  M_OP2_IMM(27, a, c, b)

/* RLDICR is said to be turing complete, this seems right */
#define M_SLL(a,b,c)                    M_OP3(31, 27, 0, 0, a, c, b)
#define M_SLL_IMM(a,b,c)                M_OP3(30, ((b)&0x20 ? 1:0), 0, ((((63-(b))&0x1f)<<6) | (((63-(b))&0x20 ? 1:0)<<5) | 0x04), a, c, (b)&0x1f);
#define M_SRL(a,b,c)                    M_OP3(31, 539, 0, 0, a, c, b)
#define M_SRL_IMM(a,b,c)                M_OP3(30, ((64-(b))&0x20 ? 1:0), 0, (((((b))&0x1f)<<6) | ((((b))&0x20 ? 1:0)<<5) | 0x00), a, c, (64-(b))&0x1f);
#define M_SRA(a,b,c)                    M_OP3(31, 794, 0, 0, a, c, b)
#define M_SRA_IMM(a,b,c)                M_OP3(31, (826 | ((b)&0x20?1:0)), 0, 0, a, c, ((b)&0x1f))
#define M_RLDICL(a,b,c,d)               M_OP4(30, ((b)&0x20 ? 1:0), 0, a, d, (b)&0x1f, c)
#define M_CNTLZ(a,b)                    M_OP3(31, 58, 0, 0, a, b, 0)

#define M_MUL(a,b,c)                   M_OP3(31, 233, 0, 0, c, a, b)
#define M_MUL_IMM(a,b,c)               M_OP2_IMM(7, c, a, b)
#define M_DIV(a,b,c)                   M_OP3(31, 489, 1, 0, c, a, b)

#define M_NEG(a,b)                      M_OP3(31, 104, 0, 0, b, a, 0)
#define M_NOT(a,b)                      M_OP3(31, 124, 0, 0, a, b, a)

#define M_SUBFIC(a,b,c)                 M_OP2_IMM(8, c, a, b)
#define M_SUBFZE(a,b)                   M_OP3(31, 200, 0, 0, b, a, 0)
#define M_RLWINM(a,b,c,d,e)             M_OP4(21, d, 0, a, e, b, c)
#define M_ADDZE(a,b)                    M_OP3(31, 202, 0, 0, b, a, 0)
#define M_ADDIS(a,b,c)                  M_OP2_IMM(15, c, a, b)
#define M_STFIWX(a,b,c)                 M_OP3(31, 983, 0, 0, a, b, c)

#define M_LWAX(a,b,c)                   M_OP3(31, 341, 0, 0, a, b, c)
#define M_LHZX(a,b,c)                   M_OP3(31, 279, 0, 0, a, b, c)
#define M_LHAX(a,b,c)                   M_OP3(31, 343, 0, 0, a, b, c)
#define M_LHAX(a,b,c)                   M_OP3(31, 343, 0, 0, a, b, c)
#define M_LBZX(a,b,c)                   M_OP3(31, 87, 0, 0, a, b, c)
#define M_LFSX(a,b,c)                   M_OP3(31, 535, 0, 0, a, b, c)
#define M_LFDX(a,b,c)                   M_OP3(31, 599, 0, 0, a, b, c)

#define M_STWX(a,b,c)                   M_OP3(31, 151, 0, 0, a, b, c)
#define M_STHX(a,b,c)                   M_OP3(31, 407, 0, 0, a, b, c)
#define M_STBX(a,b,c)                   M_OP3(31, 215, 0, 0, a, b, c)
#define M_STFSX(a,b,c)                  M_OP3(31, 663, 0, 0, a, b, c)
#define M_STFDX(a,b,c)                  M_OP3(31, 727, 0, 0, a, b, c)

/*
#define M_STWU_INTERN(a,b,disp)         M_OP2_IMM(37,a,b,disp)
#define M_STWU(a,b,disp) \
    do { \
        s4 lo = (disp) & 0x0000ffff; \
        s4 hi = ((disp) >> 16); \
        if (((disp) >= -32678) && ((disp) <= 32767)) { \
            M_STWU_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(REG_ZERO,hi,REG_ITMP3); \
            M_OR_IMM(REG_ITMP3,lo,REG_ITMP3); \
            M_STWUX(REG_SP,REG_SP,REG_ITMP3); \
        } \
    } while (0)

#define M_STWUX(a,b,c)                  M_OP3(31,183,0,0,a,b,c)
*/

#define M_STDU_INTERN(a,b,disp)		M_OP2_IMM(62,a,b,(disp)|0x0001)
#define M_STDU(a,b,disp) \
    do { \
        s4 lo = (disp) & 0x0000ffff; \
        s4 hi = ((disp) >> 16); \
        if (((disp) >= -32678) && ((disp) <= 32767)) { \
            M_STDU_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(REG_ZERO,hi,REG_ITMP3); \
            M_OR_IMM(REG_ITMP3,lo,REG_ITMP3); \
            M_STDUX(REG_SP,REG_SP,REG_ITMP3); \
        } \
    } while (0)
#define M_STDUX(a,b,c)			M_OP3(31,181,0,0,a,b,c)

#define M_LDAH(a,b,c)                   M_ADDIS(b, c, a)
#define M_TRAP                          M_OP3(31, 4, 0, 0, 31, 0, 0)

#define M_NOP                           M_OR_IMM(0, 0, 0)
#define M_MOV(a,b)                      M_OR(a, a, b)
#define M_TST(a)                        M_OP3(31, 444, 0, 1, a, a, a)

#define M_DADD(a,b,c)                   M_OP3(63, 21, 0, 0, c, a, b)
#define M_FADD(a,b,c)                   M_OP3(59, 21, 0, 0, c, a, b)
#define M_DSUB(a,b,c)                   M_OP3(63, 20, 0, 0, c, a, b)
#define M_FSUB(a,b,c)                   M_OP3(59, 20, 0, 0, c, a, b)
#define M_DMUL(a,b,c)                   M_OP4(63, 25, 0, c, a, 0, b)
#define M_FMUL(a,b,c)                   M_OP4(59, 25, 0, c, a, 0, b)
#define M_DDIV(a,b,c)                   M_OP3(63, 18, 0, 0, c, a, b)
#define M_FDIV(a,b,c)                   M_OP3(59, 18, 0, 0, c, a, b)

#define M_FABS(a,b)                     M_OP3(63, 264, 0, 0, b, 0, a)
#define M_CVTDL(a,b)                    M_OP3(63, 14, 0, 0, b, 0, a)
#define M_CVTDL_C(a,b)                  M_OP3(63, 15, 0, 0, b, 0, a)
#define M_CVTDF(a,b)                    M_OP3(63, 12, 0, 0, b, 0, a)
#define M_FMOV(a,b)                     M_OP3(63, 72, 0, 0, b, 0, a)
#define M_FMOVN(a,b)                    M_OP3(63, 40, 0, 0, b, 0, a)
#define M_DSQRT(a,b)                    M_OP3(63, 22, 0, 0, b, 0, a)
#define M_FSQRT(a,b)                    M_OP3(59, 22, 0, 0, b, 0, a)

#define M_FCMPU(a,b)                    M_OP3(63, 0, 0, 0, 0, a, b)
#define M_FCMPO(a,b)                    M_OP3(63, 32, 0, 0, 0, a, b)

#define M_BLDU(a,b,c)                   M_OP2_IMM(34, a, b, c)	/* LBZ */
#define M_SLDU(a,b,c)                   M_OP2_IMM(40, a, b, c)	/* LHZ */

#if 0
#define M_ILD_INTERN(a,b,disp)          M_OP2_IMM(32,a,b,disp)	/* LWZ */
#endif

#define M_LWZ(a,b,disp)			M_OP2_IMM(32,a,b,disp)	/* needed for hardware exceptions */

#define M_ILD_INTERN(a,b,disp)		M_OP2_IMM(58, a, b, (((disp) & 0xfffe) | 0x0002))	/* this is LWA actually */

#define M_ILD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_ILD_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,a); \
            M_ILD_INTERN(a,a,lo); \
        } \
    } while (0)

#define M_LLD_INTERN(a,b,disp) 		M_OP2_IMM(58,a,b,disp)	/* LD */

#define M_LLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LLD_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,a); \
            M_LLD_INTERN(a,GET_LOW_REG(a),lo); \
        } \
    } while (0)

#define M_ALD_INTERN(a,b,disp)          M_LLD_INTERN(a,b,disp)
#define M_ALD(a,b,disp)                 M_LLD(a,b,disp)
#define M_ALD_DSEG(a,disp)              M_LLD(a,REG_PV,disp)
#define M_ALDX(a,b,c)			M_OP3(31, 21, 0, 0, a, b, c)	/* LDX */

#define M_BST(a,b,c)                    M_OP2_IMM(38, a, b, c)	/* STB */
#define M_SST(a,b,c)                    M_OP2_IMM(44, a, b, c)	/* LMW */

#define M_IST_INTERN(a,b,disp)          M_OP2_IMM(36,a,b,disp)	/* STW */

/* Stores with displacement overflow should only happen with PUTFIELD
   or on the stack. The PUTFIELD instruction does not use REG_ITMP3
   and a reg_of_var call should not use REG_ITMP3!!! */

#define M_IST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_IST_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_IST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_LST_INTERN(a,b,disp) M_OP2_IMM(62,a,b,disp)	/* STD */

#define M_LST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LST_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_LST_INTERN(a,REG_ITMP3, lo); \
        } \
    } while (0)

#define M_AST_INTERN(a,b,disp)          M_LST_INTERN(a,b,disp)
#define M_AST(a,b,disp)                 M_LST(a,b,disp)
#define M_ASTX(a,b,c)			M_OP3(31, 149, 0, 0, a, b, c)
#define M_LSTX(a,b,c)			M_ASTX(a,b,c)


#define M_BSEXT(a,b)                    M_OP3(31, 954, 0, 0, a, b, 0)
#define M_CZEXT(a,b)                    M_RLWINM(a,0,16,31,b)
#define M_SSEXT(a,b)                    M_OP3(31, 922, 0, 0, a, b, 0)
#define M_ISEXT(a,b)			M_OP3(31, 986, 0, 0, a, b, 0)

#define M_BR(a)                         M_B(18, a, 0, 0)
#define M_BL(a)                         M_B(18, a, 0, 1)
#define M_RET                           M_OP3(19, 16, 0, 0, 20, 0, 0)
#define M_JSR                           M_OP3(19, 528, 0, 1, 20, 0, 0)
#define M_RTS                           M_OP3(19, 528, 0, 0, 20, 0, 0)

#define M_CMP(a,b)                      M_OP3(31, 0, 0, 0, 1, a, b)
#define M_CMPU(a,b)                     M_OP3(31, 32, 0, 0, 1, a, b)
#define M_CMPI(a,b)                     M_OP2_IMM(11, 1, a, b)	
#define M_CMPUI(a,b)                    M_OP2_IMM(10, 1, a, b)  

#define M_BLT(a)                        M_BC(16, 12, 0, a, 0, 0)
#define M_BLE(a)                        M_BC(16, 4, 1, a, 0, 0)
#define M_BGT(a)                        M_BC(16, 12, 1, a, 0, 0)
#define M_BGE(a)                        M_BC(16, 4, 0, a, 0, 0)
#define M_BEQ(a)                        M_BC(16, 12, 2, a, 0, 0)
#define M_BNE(a)                        M_BC(16, 4, 2, a, 0, 0)
#define M_BNAN(a)                       M_BC(16, 12, 3, a, 0, 0)

#define M_FLD_INTERN(a,b,disp)          M_OP2_IMM(48,a,b,disp)
#define M_DLD_INTERN(a,b,disp)          M_OP2_IMM(50,a,b,disp)

#define M_FLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FLD_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_FLD_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_DLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_DLD_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_DLD_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_FST_INTERN(a,b,disp)          M_OP2_IMM(52,a,b,disp)	/* STFS */
#define M_DST_INTERN(a,b,disp)          M_OP2_IMM(54,a,b,disp)	/* STFD */

#define M_FST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FST_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_FST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_DST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_DST_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,REG_ITMP3); \
            M_DST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_MFLR(a)                       M_OP3(31, 339, 0, 0, a, 8, 0)
#define M_MFXER(a)                      M_OP3(31, 339, 0, 0, a, 1, 0)
#define M_MFCTR(a)                      M_OP3(31, 339, 0, 0, a, 9, 0)
#define M_MTLR(a)                       M_OP3(31, 467, 0, 0, a, 8, 0)
#define M_MTXER(a)                      M_OP3(31, 467, 0, 0, a, 1, 0)
#define M_MTCTR(a)                      M_OP3(31, 467, 0, 0, a, 9, 0)

#define M_LDA_INTERN(a,b,c)             M_LADD_IMM(b, c, a)

#define M_LDA(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LDA_INTERN(a,b,lo); \
        } else { \
            M_ADDIS(b,hi,a); \
            M_LDA_INTERN(a,a,lo); \
        } \
    } while (0)


#define M_LDATST(a,b,c)                 M_ADDICTST(b, c, a)
#define M_CLR(a)                        M_LADD_IMM(0, 0, a)
#define M_CLR_HIGH(a)			M_OP3(30, 0, 0, 0x20, (a), (a), 0);
#define M_AADD_IMM(a,b,c)               M_LADD_IMM(a, b, c)

#define M_DMOV(a,b)                     M_FMOV(a,b)

#define M_ACMP(a,b)                     M_CMP(a,b)
#define M_ICMP(a,b)                     M_CMP(a,b)

#define M_TEST(a)                       M_TST(a)

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
 */
