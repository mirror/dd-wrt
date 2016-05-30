/* src/vm/jit/mips/codegen.hpp - code generation macros and definitions for MIPS

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
#include "vm/types.hpp"

#include "vm/jit/jit.hpp"


/* additional functions and macros to generate code ***************************/

/* MCODECHECK(icnt) */

#define MCODECHECK(icnt) \
    do { \
        if ((cd->mcodeptr + (icnt) * 4) > cd->mcodeend) \
            codegen_increase(cd); \
    } while (0)


#define ALIGNCODENOP \
    if ((s4) ((ptrint) cd->mcodeptr & 7)) { \
        M_NOP; \
    }

#define ICONST(r,c)                     emit_iconst(cd, (r), (c))
#define LCONST(r,c)                     emit_lconst(cd, (r), (c))


/* branch defines *************************************************************/

#define BRANCH_NOPS \
    do { \
        if (CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) { \
            M_NOP; \
            M_NOP; \
            M_NOP; \
            M_NOP; \
            M_NOP; \
            M_NOP; \
        } \
        else { \
            M_NOP; \
            M_NOP; \
        } \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_INSTRUCTIONS    1     /* number of instructions          */
#define PATCHER_CALL_SIZE            1 * 4 /* size in bytes of a patcher call */

#define PATCHER_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* macros to create code ******************************************************/

/* code generation macros operands:
      op ..... opcode
      fu ..... function-number
      rs ..... register number source 1
      rt ..... register number or constant integer source 2
      rd ..... register number destination
      imm .... immediate/offset
      sa ..... shift amount
*/


#define M_ITYPE(op,rs,rt,imm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((op) << 26) | ((rs) << 21) | ((rt) << 16) | ((imm) & 0xffff)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_ITYPE_GET_RS(x)               (((x) >> 21) & 0x1f  )
#define M_ITYPE_GET_RT(x)               (((x) >> 16) & 0x1f  )
#define M_ITYPE_GET_IMM(x)              ( (x)        & 0xffff)


#define M_JTYPE(op,imm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((op) << 26) | ((off) & 0x3ffffff)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_RTYPE(op,rs,rt,rd,sa,fu) \
    do { \
        *((u4 *) cd->mcodeptr) = (((op) << 26) | ((rs) << 21) | ((rt) << 16) | ((rd) << 11) | ((sa) << 6) | (fu)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_FP2(fu, fmt, fs, fd)       M_RTYPE(0x11, fmt,  0, fs, fd, fu)
#define M_FP3(fu, fmt, fs, ft, fd)   M_RTYPE(0x11, fmt, ft, fs, fd, fu)

#define FMT_F  16
#define FMT_D  17
#define FMT_I  20
#define FMT_L  21


/* macros for all used commands (see a MIPS-manual for description) ***********/

#define M_RESERVED              M_RTYPE(0x3b, 0, 0, 0, 0, 0)

/* load/store macros use the form OPERATION(source/dest, base, offset)        */

#define M_LDA(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_AADD_IMM(b,lo,a); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD_IMM(REG_ITMP3, lo, REG_ITMP3); \
            M_AADD(b, REG_ITMP3, a); \
        } \
    } while (0)

#define M_BLDS(a,b,disp)        M_ITYPE(0x20,b,a,disp)          /*  8 load    */
#define M_BLDU(a,b,disp)        M_ITYPE(0x24,b,a,disp)          /*  8 load    */
#define M_SLDS(a,b,disp)        M_ITYPE(0x21,b,a,disp)          /* 16 load    */
#define M_SLDU(a,b,disp)        M_ITYPE(0x25,b,a,disp)          /* 16 load    */

#define M_ILD_INTERN(a,b,disp)  M_ITYPE(0x23,b,a,disp)          /* 32 load    */

#define M_ILD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_ILD_INTERN(a,b,lo); \
        } else { \
            M_LUI(a,hi); \
            M_AADD(b,a,a); \
            M_ILD_INTERN(a,a,lo); \
        } \
    } while (0)

#if SIZEOF_VOID_P == 8

#define M_LLD_INTERN(a,b,disp)  M_ITYPE(0x37,b,a,disp)          /* 64 load    */

#else /* SIZEOF_VOID_P == 8 */

# if WORDS_BIGENDIAN == 1

/* ATTENTION: b may be equal to GET_LOW_REG(a) (displacement overflow case ) */

#define M_LLD_INTERN(a,b,disp) \
    do { \
        M_ILD_INTERN(GET_HIGH_REG(a), b, disp); \
        M_ILD_INTERN(GET_LOW_REG(a), b, disp + 4); \
    } while (0)

/* this macro is specially for ICMD_GETFIELD since the base (source)
   register may be one of the destination registers */

#define M_LLD_GETFIELD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            if (GET_HIGH_REG(a) == (b)) { \
                M_ILD_INTERN(GET_LOW_REG(a), b, disp + 4); \
                M_ILD_INTERN(GET_HIGH_REG(a), b, disp); \
            } else { \
                M_ILD_INTERN(GET_HIGH_REG(a), b, disp); \
                M_ILD_INTERN(GET_LOW_REG(a), b, disp + 4); \
            } \
        } else { \
            M_LUI(GET_LOW_REG(a), hi); \
            M_AADD(b, GET_LOW_REG(a), GET_LOW_REG(a)); \
            M_LLD_INTERN(a, GET_LOW_REG(a), lo); \
        } \
    } while (0)

# else /* if WORDS_BIGENDIAN == 1 */

/* ATTENTION: b may be equal to GET_LOW_REG(a) (displacement overflow case ) */

#define M_LLD_INTERN(a,b,disp) \
    do { \
        M_ILD_INTERN(GET_HIGH_REG(a), b, disp + 4); \
        M_ILD_INTERN(GET_LOW_REG(a), b, disp); \
    } while (0)

/* this macro is specially for ICMD_GETFIELD since the base (source)
   register may be one of the destination registers */

#define M_LLD_GETFIELD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            if (GET_HIGH_REG(a) == (b)) { \
                M_ILD_INTERN(GET_LOW_REG(a), b, disp); \
                M_ILD_INTERN(GET_HIGH_REG(a), b, disp + 4); \
            } else { \
                M_ILD_INTERN(GET_HIGH_REG(a), b, disp + 4); \
                M_ILD_INTERN(GET_LOW_REG(a), b, disp); \
            } \
        } else { \
            M_LUI(GET_LOW_REG(a), hi); \
            M_AADD(b, GET_LOW_REG(a), GET_LOW_REG(a)); \
            M_LLD_INTERN(a, GET_LOW_REG(a), lo); \
        } \
    } while (0)

# endif /* if WORDS_BIGENDIAN == 1 */

#endif /* SIZEOF_VOID_P == 8 */

#define M_LLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LLD_INTERN(a, b, lo); \
        } else { \
            M_LUI(GET_LOW_REG(a), hi); \
            M_AADD(b, GET_LOW_REG(a), GET_LOW_REG(a)); \
            M_LLD_INTERN(a, GET_LOW_REG(a), lo); \
        } \
    } while (0)

#define M_BST(a,b,disp)         M_ITYPE(0x28,b,a,disp)          /*  8 store   */
#define M_SST(a,b,disp)         M_ITYPE(0x29,b,a,disp)          /* 16 store   */

#define M_IST_INTERN(a,b,disp)  M_ITYPE(0x2b,b,a,disp)          /* 32 store   */

#define M_IST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_IST_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_IST_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#if SIZEOF_VOID_P == 8

#define M_LST_INTERN(a,b,disp)  M_ITYPE(0x3f,b,a,disp)          /* 64 store   */

#else /* SIZEOF_VOID_P == 8 */

#if WORDS_BIGENDIAN == 1

#define M_LST_INTERN(a,b,disp) \
    do { \
        M_IST_INTERN(GET_HIGH_REG(a), b, disp); \
        M_IST_INTERN(GET_LOW_REG(a), b, disp + 4); \
    } while (0)

#else /* if WORDS_BIGENDIAN == 1 */

#define M_LST_INTERN(a,b,disp) \
    do { \
        M_IST_INTERN(GET_HIGH_REG(a), b, disp + 4); \
        M_IST_INTERN(GET_LOW_REG(a), b, disp); \
    } while (0)

#endif /* if WORDS_BIGENDIAN == 1 */

#endif /* SIZEOF_VOID_P == 8 */

#define M_LST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LST_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_LST_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#define M_FLD_INTERN(a,b,disp)  M_ITYPE(0x31,b,a,disp)          /* load flt   */
#define M_DLD_INTERN(a,b,disp)  M_ITYPE(0x35,b,a,disp)          /* load dbl   */

#define M_FLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FLD_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_FLD_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#define M_DLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_DLD_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_DLD_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#define M_FST_INTERN(a,b,disp)  M_ITYPE(0x39,b,a,disp)          /* store flt  */
#define M_DST_INTERN(a,b,disp)  M_ITYPE(0x3d,b,a,disp)          /* store dbl  */

#define M_FST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FST_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_FST_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#define M_DST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_DST_INTERN(a,b,lo); \
        } else { \
            M_LUI(REG_ITMP3, hi); \
            M_AADD(b, REG_ITMP3, REG_ITMP3); \
            M_DST_INTERN(a, REG_ITMP3, lo); \
        } \
    } while (0)

#define M_BEQ(a,b,disp)         M_ITYPE(0x04,a,b,disp)          /* br a == b  */
#define M_BNE(a,b,disp)         M_ITYPE(0x05,a,b,disp)          /* br a != b  */
#define M_BEQZ(a,disp)          M_ITYPE(0x04,a,0,disp)          /* br a == 0  */
#define M_BLTZ(a,disp)          M_ITYPE(0x01,a,0,disp)          /* br a <  0  */
#define M_BLEZ(a,disp)          M_ITYPE(0x06,a,0,disp)          /* br a <= 0  */
#define M_BNEZ(a,disp)          M_ITYPE(0x05,a,0,disp)          /* br a != 0  */
#define M_BGEZ(a,disp)          M_ITYPE(0x01,a,1,disp)          /* br a >= 0  */
#define M_BGTZ(a,disp)          M_ITYPE(0x07,a,0,disp)          /* br a >  0  */

#if 0
#define M_BEQL(a,b,disp)        M_ITYPE(0x14,a,b,disp)          /* br a == b  */
#define M_BNEL(a,b,disp)        M_ITYPE(0x15,a,b,disp)          /* br a != b  */
#define M_BEQZL(a,disp)         M_ITYPE(0x14,a,0,disp)          /* br a == 0  */
#define M_BLTZL(a,disp)         M_ITYPE(0x01,a,2,disp)          /* br a <  0  */
#define M_BLEZL(a,disp)         M_ITYPE(0x16,a,0,disp)          /* br a <= 0  */
#define M_BNEZL(a,disp)         M_ITYPE(0x15,a,0,disp)          /* br a != 0  */
#define M_BGEZL(a,disp)         M_ITYPE(0x01,a,3,disp)          /* br a >= 0  */
#define M_BGTZL(a,disp)         M_ITYPE(0x17,a,0,disp)          /* br a >  0  */
#endif

#define M_BR(disp)              M_ITYPE(0x04,0,0,disp)          /* branch     */
#define M_BRS(disp)             M_ITYPE(0x01,0,17,disp)         /* branch sbr */

#define M_JMP(a)                M_RTYPE(0,a,0,0,0,0x08)         /* jump       */
#define M_JSR(r,a)              M_RTYPE(0,a,0,r,0,0x09)         /* call       */
#define M_RET(a)                M_RTYPE(0,a,0,0,0,0x08)         /* return     */

#define M_TGE(a,b,code)         M_RTYPE(0,a,b,0,code&3ff,0x30)  /* trp a >= b */
#define M_TGEU(a,b,code)        M_RTYPE(0,a,b,0,code&3ff,0x31)  /* trp a >= b */
#define M_TLT(a,b,code)         M_RTYPE(0,a,b,0,code&3ff,0x32)  /* trp a <  b */
#define M_TLTU(a,b,code)        M_RTYPE(0,a,b,0,code&3ff,0x33)  /* trp a <  b */
#define M_TEQ(a,b,code)         M_RTYPE(0,a,b,0,code&3ff,0x34)  /* trp a == b */
#define M_TNE(a,b,code)         M_RTYPE(0,a,b,0,code&3ff,0x36)  /* trp a != b */
#define M_TLE(a,b,code)         M_RTYPE(0,b,a,0,code&3ff,0x30)  /* trp a <= b */
#define M_TLEU(a,b,code)        M_RTYPE(0,b,a,0,code&3ff,0x31)  /* trp a <= b */
#define M_TGT(a,b,code)         M_RTYPE(0,b,a,0,code&3ff,0x32)  /* trp a >  b */
#define M_TGTU(a,b,code)        M_RTYPE(0,b,a,0,code&3ff,0x33)  /* trp a >  b */

#define M_TGE_IMM(a,b)          M_ITYPE(1,a,0x08,b)             /* trp a >= b */
#define M_TGEU_IMM(a,b)         M_ITYPE(1,a,0x09,b)             /* trp a >= b */
#define M_TLT_IMM(a,b)          M_ITYPE(1,a,0x0a,b)             /* trp a <  b */
#define M_TLTU_IMM(a,b)         M_ITYPE(1,a,0x0b,b)             /* trp a <  b */
#define M_TEQ_IMM(a,b)          M_ITYPE(1,a,0x0c,b)             /* trp a == b */
#define M_TNE_IMM(a,b)          M_ITYPE(1,a,0x0e,b)             /* trp a != b */
#if 0
#define M_TGT_IMM(a,b)          M_ITYPE(1,a,0x08,b+1)           /* trp a >  b */
#define M_TGTU_IMM(a,b)         M_ITYPE(1,a,0x09,b+1)           /* trp a >  b */
#define M_TLE_IMM(a,b)          M_ITYPE(1,a,0x0a,b+1)           /* trp a <= b */
#define M_TLEU_IMM(a,b)         M_ITYPE(1,a,0x0b,b+1)           /* trp a <= b */
#endif

/* arithmetic macros use the form OPERATION(source, source/immediate, dest)   */

#define M_IADD(a,b,c)           M_RTYPE(0,a,b,c,0,0x21)         /* 32 add     */
#define M_LADD(a,b,c)           M_RTYPE(0,a,b,c,0,0x2d)         /* 64 add     */
#define M_ISUB(a,b,c)           M_RTYPE(0,a,b,c,0,0x23)         /* 32 sub     */
#define M_LSUB(a,b,c)           M_RTYPE(0,a,b,c,0,0x2f)         /* 64 sub     */
#define M_IMUL(a,b)             M_ITYPE(0,a,b,0x18)             /* 32 mul     */
#define M_IMULU(a,b)            M_ITYPE(0,a,b,0x19)             /* 32 mul     */
#define M_LMUL(a,b)             M_ITYPE(0,a,b,0x1c)             /* 64 mul     */
#define M_IDIV(a,b)             M_ITYPE(0,a,b,0x1a)             /* 32 div     */
#define M_LDIV(a,b)             M_ITYPE(0,a,b,0x1e)             /* 64 div     */

#define M_MFLO(a)              	M_RTYPE(0,0,0,a,0,0x12)         /* quotient   */
#define M_MFHI(a)              	M_RTYPE(0,0,0,a,0,0x10)         /* remainder  */

#define M_IADD_IMM(a,b,c)       M_ITYPE(0x09,a,c,b)             /* 32 add     */
#define M_LADD_IMM(a,b,c)       M_ITYPE(0x19,a,c,b)             /* 64 add     */
#define M_ISUB_IMM(a,b,c)       M_IADD_IMM(a,-(b),c)            /* 32 sub     */
#define M_LSUB_IMM(a,b,c)       M_LADD_IMM(a,-(b),c)            /* 64 sub     */

#define M_LUI(a,imm)            M_ITYPE(0x0f,0,a,imm)           /* a = imm<<16*/

#define M_CMPLT(a,b,c)          M_RTYPE(0,a,b,c,0,0x2a)         /* c = a <  b */
#define M_CMPGT(a,b,c)          M_RTYPE(0,b,a,c,0,0x2a)         /* c = a >  b */

#define M_CMPULT(a,b,c)         M_RTYPE(0,a,b,c,0,0x2b)         /* c = a <  b */
#define M_CMPUGT(a,b,c)         M_RTYPE(0,b,a,c,0,0x2b)         /* c = a >  b */

#define M_CMPLT_IMM(a,b,c)      M_ITYPE(0x0a,a,c,b)             /* c = a <  b */
#define M_CMPULT_IMM(a,b,c)     M_ITYPE(0x0b,a,c,b)             /* c = a <  b */

#define M_AND(a,b,c)            M_RTYPE(0,a,b,c,0,0x24)         /* c = a &  b */
#define M_OR( a,b,c)            M_RTYPE(0,a,b,c,0,0x25)         /* c = a |  b */
#define M_XOR(a,b,c)            M_RTYPE(0,a,b,c,0,0x26)         /* c = a ^  b */

#define M_AND_IMM(a,b,c)        M_ITYPE(0x0c,a,c,b)             /* c = a &  b */
#define M_OR_IMM( a,b,c)        M_ITYPE(0x0d,a,c,b)             /* c = a |  b */
#define M_XOR_IMM(a,b,c)        M_ITYPE(0x0e,a,c,b)             /* c = a ^  b */

#define M_ISLL(a,b,c)           M_RTYPE(0,b,a,c,0,0x04)         /* c = a << b */
#define M_ISRL(a,b,c)           M_RTYPE(0,b,a,c,0,0x06)         /* c = a >>>b */
#define M_ISRA(a,b,c)           M_RTYPE(0,b,a,c,0,0x07)         /* c = a >> b */
#define M_LSLL(a,b,c)           M_RTYPE(0,b,a,c,0,0x14)         /* c = a << b */
#define M_LSRL(a,b,c)           M_RTYPE(0,b,a,c,0,0x16)         /* c = a >>>b */
#define M_LSRA(a,b,c)           M_RTYPE(0,b,a,c,0,0x17)         /* c = a >> b */

#define M_ISLL_IMM(a,b,c)       M_RTYPE(0,0,a,c,(b)&31,0x00)    /* c = a << b */
#define M_ISRL_IMM(a,b,c)       M_RTYPE(0,0,a,c,(b)&31,0x02)    /* c = a >>>b */
#define M_ISRA_IMM(a,b,c)       M_RTYPE(0,0,a,c,(b)&31,0x03)    /* c = a >> b */
#define M_LSLL_IMM(a,b,c) M_RTYPE(0,0,a,c,(b)&31,0x38+((b)>>3&4)) /*c = a << b*/
#define M_LSRL_IMM(a,b,c) M_RTYPE(0,0,a,c,(b)&31,0x3a+((b)>>3&4)) /*c = a >>>b*/
#define M_LSRA_IMM(a,b,c) M_RTYPE(0,0,a,c,(b)&31,0x3b+((b)>>3&4)) /*c = a >> b*/

#define M_MOV(a,c)              M_OR(a,0,c)                     /* c = a      */
#define M_CLR(c)                M_OR(0,0,c)                     /* c = 0      */
#define M_NOP                   M_ISLL_IMM(0,0,0)               /* ;          */

/* floating point macros use the form OPERATION(source, source, dest)         */

#define M_FADD(a,b,c)           M_FP3(0x00,FMT_F,a,b,c)         /* flt add    */
#define M_DADD(a,b,c)           M_FP3(0x00,FMT_D,a,b,c)         /* dbl add    */
#define M_FSUB(a,b,c)           M_FP3(0x01,FMT_F,a,b,c)         /* flt sub    */
#define M_DSUB(a,b,c)           M_FP3(0x01,FMT_D,a,b,c)         /* dbl sub    */
#define M_FMUL(a,b,c)           M_FP3(0x02,FMT_F,a,b,c)         /* flt mul    */
#define M_DMUL(a,b,c)           M_FP3(0x02,FMT_D,a,b,c)         /* dbl mul    */
#define M_FDIV(a,b,c)           M_FP3(0x03,FMT_F,a,b,c)         /* flt div    */
#define M_DDIV(a,b,c)           M_FP3(0x03,FMT_D,a,b,c)         /* dbl div    */

#define M_FSQRT(a,c)            M_FP2(0x04,FMT_F,a,c)           /* flt sqrt   */
#define M_DSQRT(a,c)            M_FP2(0x04,FMT_D,a,c)           /* dbl sqrt   */
#define M_FABS(a,c)             M_FP2(0x05,FMT_F,a,c)           /* flt abs    */
#define M_DABS(a,c)             M_FP2(0x05,FMT_D,a,c)           /* dbl abs    */
#define M_FMOV(a,c)             M_FP2(0x06,FMT_F,a,c)           /* flt mov    */
#define M_DMOV(a,c)             M_FP2(0x06,FMT_D,a,c)           /* dbl mov    */
#define M_FNEG(a,c)             M_FP2(0x07,FMT_F,a,c)           /* flt neg    */
#define M_DNEG(a,c)             M_FP2(0x07,FMT_D,a,c)           /* dbl neg    */

#define M_ROUNDFI(a,c)          M_FP2(0x0c,FMT_F,a,c)           /* flt roundi */
#define M_ROUNDDI(a,c)          M_FP2(0x0c,FMT_D,a,c)           /* dbl roundi */
#define M_TRUNCFI(a,c)          M_FP2(0x0d,FMT_F,a,c)           /* flt trunci */
#define M_TRUNCDI(a,c)          M_FP2(0x0d,FMT_D,a,c)           /* dbl trunci */
#define M_CEILFI(a,c)           M_FP2(0x0e,FMT_F,a,c)           /* flt ceili  */
#define M_CEILDI(a,c)           M_FP2(0x0e,FMT_D,a,c)           /* dbl ceili  */
#define M_FLOORFI(a,c)          M_FP2(0x0f,FMT_F,a,c)           /* flt trunci */
#define M_FLOORDI(a,c)          M_FP2(0x0f,FMT_D,a,c)           /* dbl trunci */

#define M_ROUNDFL(a,c)          M_FP2(0x08,FMT_F,a,c)           /* flt roundl */
#define M_ROUNDDL(a,c)          M_FP2(0x08,FMT_D,a,c)           /* dbl roundl */
#define M_TRUNCFL(a,c)          M_FP2(0x09,FMT_F,a,c)           /* flt truncl */
#define M_TRUNCDL(a,c)          M_FP2(0x09,FMT_D,a,c)           /* dbl truncl */
#define M_CEILFL(a,c)           M_FP2(0x0a,FMT_F,a,c)           /* flt ceill  */
#define M_CEILDL(a,c)           M_FP2(0x0a,FMT_D,a,c)           /* dbl ceill  */
#define M_FLOORFL(a,c)          M_FP2(0x0b,FMT_F,a,c)           /* flt truncl */
#define M_FLOORDL(a,c)          M_FP2(0x0b,FMT_D,a,c)           /* dbl truncl */

#define M_CVTDF(a,c)            M_FP2(0x20,FMT_D,a,c)           /* dbl2flt    */
#define M_CVTIF(a,c)            M_FP2(0x20,FMT_I,a,c)           /* int2flt    */
#define M_CVTLF(a,c)            M_FP2(0x20,FMT_L,a,c)           /* long2flt   */
#define M_CVTFD(a,c)            M_FP2(0x21,FMT_F,a,c)           /* flt2dbl    */
#define M_CVTID(a,c)            M_FP2(0x21,FMT_I,a,c)           /* int2dbl    */
#define M_CVTLD(a,c)            M_FP2(0x21,FMT_L,a,c)           /* long2dbl   */
#define M_CVTFI(a,c)            M_FP2(0x24,FMT_F,a,c)           /* flt2int    */
#define M_CVTDI(a,c)            M_FP2(0x24,FMT_D,a,c)           /* dbl2int    */
#define M_CVTFL(a,c)            M_FP2(0x25,FMT_F,a,c)           /* flt2long   */
#define M_CVTDL(a,c)            M_FP2(0x25,FMT_D,a,c)           /* dbl2long   */

#define M_MOVDI(d,i)            M_FP3(0,0,d,i,0)                /* i = d      */
#define M_MOVDL(d,l)            M_FP3(0,1,d,l,0)                /* l = d      */
#define M_MOVID(i,d)            M_FP3(0,4,d,i,0)                /* d = i      */
#define M_MOVLD(l,d)            M_FP3(0,5,d,l,0)                /* d = l      */

#define M_MFC1(l,f)             M_FP3(0,0,f,l,0)
#define M_MTC1(l,f)             M_FP3(0,4,f,l,0)

#define M_DMFC1(l,f)            M_FP3(0,1,f,l,0)
#define M_DMTC1(l,f)            M_FP3(0,5,f,l,0)

#define M_FCMPFF(a,b)           M_FP3(0x30,FMT_F,a,b,0)         /* c = a == b */
#define M_FCMPFD(a,b)           M_FP3(0x30,FMT_D,a,b,0)         /* c = a == b */
#define M_FCMPUNF(a,b)          M_FP3(0x31,FMT_F,a,b,0)         /* c = a == b */
#define M_FCMPUND(a,b)          M_FP3(0x31,FMT_D,a,b,0)         /* c = a == b */
#define M_FCMPEQF(a,b)          M_FP3(0x32,FMT_F,a,b,0)         /* c = a == b */
#define M_FCMPEQD(a,b)          M_FP3(0x32,FMT_D,a,b,0)         /* c = a == b */
#define M_FCMPUEQF(a,b)         M_FP3(0x33,FMT_F,a,b,0)         /* c = a == b */
#define M_FCMPUEQD(a,b)         M_FP3(0x33,FMT_D,a,b,0)         /* c = a == b */
#define M_FCMPOLTF(a,b)         M_FP3(0x34,FMT_F,a,b,0)         /* c = a <  b */
#define M_FCMPOLTD(a,b)         M_FP3(0x34,FMT_D,a,b,0)         /* c = a <  b */
#define M_FCMPULTF(a,b)         M_FP3(0x35,FMT_F,a,b,0)         /* c = a <  b */
#define M_FCMPULTD(a,b)         M_FP3(0x35,FMT_D,a,b,0)         /* c = a <  b */
#define M_FCMPOLEF(a,b)         M_FP3(0x36,FMT_F,a,b,0)         /* c = a <= b */
#define M_FCMPOLED(a,b)         M_FP3(0x36,FMT_D,a,b,0)         /* c = a <= b */
#define M_FCMPULEF(a,b)         M_FP3(0x37,FMT_F,a,b,0)         /* c = a <= b */
#define M_FCMPULED(a,b)         M_FP3(0x37,FMT_D,a,b,0)         /* c = a <= b */

#define M_FBF(disp)             M_ITYPE(0x11,8,0,disp)          /* br false   */
#define M_FBT(disp)             M_ITYPE(0x11,8,1,disp)          /* br true    */
#define M_FBFL(disp)            M_ITYPE(0x11,8,2,disp)          /* br false   */
#define M_FBTL(disp)            M_ITYPE(0x11,8,3,disp)          /* br true    */

#define M_CMOVF(a,b)			M_RTYPE(0x00,a,0,b,0,1)
#define M_CMOVT(a,b)			M_RTYPE(0x00,a,1,b,0,1)


/*
 * Load Address pseudo instruction:
 * -n32 addressing mode -> 32 bit addrs, -64 addressing mode -> 64 bit addrs
 */
#if SIZEOF_VOID_P == 8

#define POINTERSHIFT 3

#define M_ALD_INTERN(a,b,disp)  M_LLD_INTERN(a,b,disp)
#define M_ALD(a,b,disp)         M_LLD(a,b,disp)
#define M_ALD_DSEG(a,disp)      M_LLD(a,REG_PV,disp)
#define M_AST_INTERN(a,b,disp)  M_LST_INTERN(a,b,disp)
#define M_AST(a,b,disp)         M_LST(a,b,disp)
#define M_AADD(a,b,c)           M_LADD(a,b,c)
#define M_AADD_IMM(a,b,c)       M_LADD_IMM(a,b,c)
#define M_ASUB_IMM(a,b,c)       M_LSUB_IMM(a,b,c)
#define M_ASLL_IMM(a,b,c)       M_LSLL_IMM(a,b,c)

#else /* SIZEOF_VOID_P == 8 */

#define POINTERSHIFT 2

#define M_ALD_INTERN(a,b,disp)  M_ILD_INTERN(a,b,disp)
#define M_ALD(a,b,disp)         M_ILD(a,b,disp)
#define M_ALD_DSEG(a,disp)      M_ILD(a,REG_PV,disp)
#define M_AST_INTERN(a,b,disp)  M_IST_INTERN(a,b,disp)
#define M_AST(a,b,disp)         M_IST(a,b,disp)
#define M_AADD(a,b,c)           M_IADD(a,b,c)
#define M_AADD_IMM(a,b,c)       M_IADD_IMM(a,b,c)
#define M_ASUB_IMM(a,b,c)       M_ISUB_IMM(a,b,c)
#define M_ASLL_IMM(a,b,c)       M_ISLL_IMM(a,b,c)

#endif /* SIZEOF_VOID_P == 8 */

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
