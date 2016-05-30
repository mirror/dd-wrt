/* src/vm/jit/alpha/codegen.hpp - code generation macros and definitions for Alpha

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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

#define gen_bound_check \
    if (checkbounds) { \
        M_ILD(REG_ITMP3, s1, OFFSET(java_arrayheader, size));\
        M_CMPULT(s2, REG_ITMP3, REG_ITMP3);\
        M_BEQZ(REG_ITMP3, 0);\
        codegen_add_arrayindexoutofboundsexception_ref(cd, s2); \
    }


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


#define ICONST(d,c)        emit_iconst(cd, (d), (c))
#define LCONST(d,c)        emit_lconst(cd, (d), (c))


/* branch defines *************************************************************/

#define BRANCH_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_SIZE    1 * 4     /* an instruction is 4-bytes long      */

#define PATCHER_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* macros to create code ******************************************************/

/* M_MEM - memory instruction format *******************************************

    Opcode ........ opcode
    Ra ............ source/target register for memory access
    Rb ............ base register
    Memory_disp ... memory displacement (16 bit signed) to be added to Rb

*******************************************************************************/

#define M_MEM(Opcode,Ra,Rb,Memory_disp) \
    do { \
        *((uint32_t *) cd->mcodeptr) = ((((Opcode)) << 26) | ((Ra) << 21) | ((Rb) << 16) | ((Memory_disp) & 0xffff)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_MEM_GET_Opcode(x)             (          (((x) >> 26) & 0x3f  ))
#define M_MEM_GET_Ra(x)                 (          (((x) >> 21) & 0x1f  ))
#define M_MEM_GET_Rb(x)                 (          (((x) >> 16) & 0x1f  ))
#define M_MEM_GET_Memory_disp(x)        ((int16_t) ( (x)        & 0xffff))


/* M_BRA - branch instruction format *******************************************

    Opcode ........ opcode
    Ra ............ register to be tested
    Branch_disp ... relative address to be jumped to (divided by 4)

*******************************************************************************/

#define M_BRA(Opcode,Ra,Branch_disp) \
    do { \
        *((uint32_t *) cd->mcodeptr) = ((((Opcode)) << 26) | ((Ra) << 21) | ((Branch_disp) & 0x1fffff)); \
        cd->mcodeptr += 4; \
    } while (0)


#define REG   0
#define CONST 1

/* 3-address-operations: M_OP3
      op ..... opcode
      fu ..... function-number
      a  ..... register number source 1
      b  ..... register number or constant integer source 2
      c  ..... register number destination
      const .. switch to use b as constant integer 
                 (REG means: use b as register number)
                 (CONST means: use b as constant 8-bit-integer)
*/      

#define M_OP3(op,fu,a,b,c,const) \
    do { \
        *((u4 *) cd->mcodeptr) = ((((s4) (op)) << 26) | ((a) << 21) | ((b) << (16 - 3 * (const))) | ((const) << 12) | ((fu) << 5) | ((c))); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_OP3_GET_Opcode(x)             (          (((x) >> 26) & 0x3f  ))


/* 3-address-floating-point-operation: M_FOP3 
     op .... opcode
     fu .... function-number
     a,b ... source floating-point registers
     c ..... destination register
*/ 

#define M_FOP3(op,fu,a,b,c) \
    do { \
        *((u4 *) cd->mcodeptr) = ((((s4) (op)) << 26) | ((a) << 21) | ((b) << 16) | ((fu) << 5) | (c)); \
        cd->mcodeptr += 4; \
    } while (0)


/* macros for all used commands (see an Alpha-manual for description) *********/

#define M_LDA_INTERN(a,b,disp)  M_MEM(0x08,a,b,disp)            /* low const  */

#define M_LDA(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LDA_INTERN(a,b,lo); \
        } else { \
            M_LDAH(a,b,hi); \
            M_LDA_INTERN(a,a,lo); \
        } \
    } while (0)

#define M_LDAH(a,b,disp)        M_MEM (0x09,a,b,disp)           /* high const */

#define M_BLDU(a,b,disp)        M_MEM (0x0a,a,b,disp)           /*  8 load    */
#define M_SLDU(a,b,disp)        M_MEM (0x0c,a,b,disp)           /* 16 load    */

#define M_ILD_INTERN(a,b,disp)  M_MEM(0x28,a,b,disp)            /* 32 load    */
#define M_LLD_INTERN(a,b,disp)  M_MEM(0x29,a,b,disp)            /* 64 load    */

#define M_ILD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_ILD_INTERN(a,b,lo); \
        } else { \
            M_LDAH(a,b,hi); \
            M_ILD_INTERN(a,a,lo); \
        } \
    } while (0)

#define M_LLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LLD_INTERN(a,b,lo); \
        } else { \
            M_LDAH(a,b,hi); \
            M_LLD_INTERN(a,a,lo); \
        } \
    } while (0)

#define M_ALD_INTERN(a,b,disp)  M_LLD_INTERN(a,b,disp)
#define M_ALD(a,b,disp)         M_LLD(a,b,disp)                 /* addr load  */
#define M_ALD_DSEG(a,disp)      M_LLD(a,REG_PV,disp)

#define M_BST(a,b,disp)         M_MEM(0x0e,a,b,disp)            /*  8 store   */
#define M_SST(a,b,disp)         M_MEM(0x0d,a,b,disp)            /* 16 store   */

#define M_IST_INTERN(a,b,disp)  M_MEM(0x2c,a,b,disp)            /* 32 store   */
#define M_LST_INTERN(a,b,disp)  M_MEM(0x2d,a,b,disp)            /* 64 store   */

/* Stores with displacement overflow should only happen with PUTFIELD or on   */
/* the stack. The PUTFIELD instruction does not use REG_ITMP3 and a           */
/* reg_of_var call should not use REG_ITMP3!!!                                */

#define M_IST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_IST_INTERN(a,b,lo); \
        } else { \
            M_LDAH(REG_ITMP3,b,hi); \
            M_IST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_LST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_LST_INTERN(a,b,lo); \
        } else { \
            M_LDAH(REG_ITMP3,b,hi); \
            M_LST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_AST(a,b,disp)         M_LST(a,b,disp)                 /* addr store */

#define M_BSEXT(b,c)            M_OP3 (0x1c,0x0,REG_ZERO,b,c,0) /*  8 signext */
#define M_SSEXT(b,c)            M_OP3 (0x1c,0x1,REG_ZERO,b,c,0) /* 16 signext */

#define M_BR(disp)              M_BRA (0x30,REG_ZERO,disp)      /* branch     */
#define M_BSR(ra,disp)          M_BRA (0x34,ra,disp)            /* branch sbr */
#define M_BEQZ(a,disp)          M_BRA (0x39,a,disp)             /* br a == 0  */
#define M_BLTZ(a,disp)          M_BRA (0x3a,a,disp)             /* br a <  0  */
#define M_BLEZ(a,disp)          M_BRA (0x3b,a,disp)             /* br a <= 0  */
#define M_BNEZ(a,disp)          M_BRA (0x3d,a,disp)             /* br a != 0  */
#define M_BGEZ(a,disp)          M_BRA (0x3e,a,disp)             /* br a >= 0  */
#define M_BGTZ(a,disp)          M_BRA (0x3f,a,disp)             /* br a >  0  */

#define M_JMP(a,b)              M_MEM (0x1a,a,b,0x0000)         /* jump       */
#define M_JSR(a,b)              M_MEM (0x1a,a,b,0x4000)         /* call sbr   */
#define M_RET(a,b)              M_MEM (0x1a,a,b,0x8000)         /* return     */

#define M_IADD(a,b,c)           M_OP3 (0x10,0x0,  a,b,c,0)      /* 32 add     */
#define M_LADD(a,b,c)           M_OP3 (0x10,0x20, a,b,c,0)      /* 64 add     */
#define M_ISUB(a,b,c)           M_OP3 (0x10,0x09, a,b,c,0)      /* 32 sub     */
#define M_LSUB(a,b,c)           M_OP3 (0x10,0x29, a,b,c,0)      /* 64 sub     */
#define M_IMUL(a,b,c)           M_OP3 (0x13,0x00, a,b,c,0)      /* 32 mul     */
#define M_LMUL(a,b,c)           M_OP3 (0x13,0x20, a,b,c,0)      /* 64 mul     */

#define M_IADD_IMM(a,b,c)       M_OP3 (0x10,0x0,  a,b,c,1)      /* 32 add     */
#define M_LADD_IMM(a,b,c)       M_OP3 (0x10,0x20, a,b,c,1)      /* 64 add     */
#define M_ISUB_IMM(a,b,c)       M_OP3 (0x10,0x09, a,b,c,1)      /* 32 sub     */
#define M_LSUB_IMM(a,b,c)       M_OP3 (0x10,0x29, a,b,c,1)      /* 64 sub     */
#define M_IMUL_IMM(a,b,c)       M_OP3 (0x13,0x00, a,b,c,1)      /* 32 mul     */
#define M_LMUL_IMM(a,b,c)       M_OP3 (0x13,0x20, a,b,c,1)      /* 64 mul     */

#define M_AADD_IMM(a,b,c)       M_LADD_IMM(a,b,c)
#define M_ASUB_IMM(a,b,c)       M_LSUB_IMM(a,b,c)

#define M_CMPEQ(a,b,c)          M_OP3 (0x10,0x2d, a,b,c,0)      /* c = a == b */
#define M_CMPLT(a,b,c)          M_OP3 (0x10,0x4d, a,b,c,0)      /* c = a <  b */
#define M_CMPLE(a,b,c)          M_OP3 (0x10,0x6d, a,b,c,0)      /* c = a <= b */

#define M_CMPULE(a,b,c)         M_OP3 (0x10,0x3d, a,b,c,0)      /* c = a <= b */
#define M_CMPULT(a,b,c)         M_OP3 (0x10,0x1d, a,b,c,0)      /* c = a <= b */

#define M_CMPEQ_IMM(a,b,c)      M_OP3 (0x10,0x2d, a,b,c,1)      /* c = a == b */
#define M_CMPLT_IMM(a,b,c)      M_OP3 (0x10,0x4d, a,b,c,1)      /* c = a <  b */
#define M_CMPLE_IMM(a,b,c)      M_OP3 (0x10,0x6d, a,b,c,1)      /* c = a <= b */

#define M_CMPULE_IMM(a,b,c)     M_OP3 (0x10,0x3d, a,b,c,1)      /* c = a <= b */
#define M_CMPULT_IMM(a,b,c)     M_OP3 (0x10,0x1d, a,b,c,1)      /* c = a <= b */

#define M_AND(a,b,c)            M_OP3 (0x11,0x00, a,b,c,0)      /* c = a &  b */
#define M_OR( a,b,c)            M_OP3 (0x11,0x20, a,b,c,0)      /* c = a |  b */
#define M_XOR(a,b,c)            M_OP3 (0x11,0x40, a,b,c,0)      /* c = a ^  b */

#define M_AND_IMM(a,b,c)        M_OP3 (0x11,0x00, a,b,c,1)      /* c = a &  b */
#define M_OR_IMM( a,b,c)        M_OP3 (0x11,0x20, a,b,c,1)      /* c = a |  b */
#define M_XOR_IMM(a,b,c)        M_OP3 (0x11,0x40, a,b,c,1)      /* c = a ^  b */

#define M_MOV(a,c)              M_OR (a,a,c)                    /* c = a      */
#define M_CLR(c)                M_OR (31,31,c)                  /* c = 0      */
#define M_NOP                   M_OR (31,31,31)                 /* ;          */

#define M_SLL(a,b,c)            M_OP3 (0x12,0x39, a,b,c,0)      /* c = a << b */
#define M_SRA(a,b,c)            M_OP3 (0x12,0x3c, a,b,c,0)      /* c = a >> b */
#define M_SRL(a,b,c)            M_OP3 (0x12,0x34, a,b,c,0)      /* c = a >>>b */

#define M_SLL_IMM(a,b,c)        M_OP3 (0x12,0x39, a,b,c,1)      /* c = a << b */
#define M_SRA_IMM(a,b,c)        M_OP3 (0x12,0x3c, a,b,c,1)      /* c = a >> b */
#define M_SRL_IMM(a,b,c)        M_OP3 (0x12,0x34, a,b,c,1)      /* c = a >>>b */

#define M_FLD_INTERN(a,b,disp)  M_MEM(0x22,a,b,disp)            /* load flt   */
#define M_DLD_INTERN(a,b,disp)  M_MEM(0x23,a,b,disp)            /* load dbl   */

#define M_FLD(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FLD_INTERN(a,b,lo); \
        } else { \
            M_LDAH(REG_ITMP3,b,hi); \
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
            M_LDAH(REG_ITMP3,b,hi); \
            M_DLD_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)

#define M_FST_INTERN(a,b,disp)  M_MEM(0x26,a,b,disp)            /* store flt  */
#define M_DST_INTERN(a,b,disp)  M_MEM(0x27,a,b,disp)            /* store dbl  */

/* Stores with displacement overflow should only happen with PUTFIELD or on   */
/* the stack. The PUTFIELD instruction does not use REG_ITMP3 and a           */
/* reg_of_var call should not use REG_ITMP3!!!                                */

#define M_FST(a,b,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 16); \
        if (hi == 0) { \
            M_FST_INTERN(a,b,lo); \
        } else { \
            M_LDAH(REG_ITMP3,b,hi); \
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
            M_LDAH(REG_ITMP3,b,hi); \
            M_DST_INTERN(a,REG_ITMP3,lo); \
        } \
    } while (0)


#define M_FADD(a,b,c)           M_FOP3 (0x16, 0x080, a,b,c)     /* flt add    */
#define M_DADD(a,b,c)           M_FOP3 (0x16, 0x0a0, a,b,c)     /* dbl add    */
#define M_FSUB(a,b,c)           M_FOP3 (0x16, 0x081, a,b,c)     /* flt sub    */
#define M_DSUB(a,b,c)           M_FOP3 (0x16, 0x0a1, a,b,c)     /* dbl sub    */
#define M_FMUL(a,b,c)           M_FOP3 (0x16, 0x082, a,b,c)     /* flt mul    */
#define M_DMUL(a,b,c)           M_FOP3 (0x16, 0x0a2, a,b,c)     /* dbl mul    */
#define M_FDIV(a,b,c)           M_FOP3 (0x16, 0x083, a,b,c)     /* flt div    */
#define M_DDIV(a,b,c)           M_FOP3 (0x16, 0x0a3, a,b,c)     /* dbl div    */

#define M_FADDS(a,b,c)          M_FOP3 (0x16, 0x580, a,b,c)     /* flt add    */
#define M_DADDS(a,b,c)          M_FOP3 (0x16, 0x5a0, a,b,c)     /* dbl add    */
#define M_FSUBS(a,b,c)          M_FOP3 (0x16, 0x581, a,b,c)     /* flt sub    */
#define M_DSUBS(a,b,c)          M_FOP3 (0x16, 0x5a1, a,b,c)     /* dbl sub    */
#define M_FMULS(a,b,c)          M_FOP3 (0x16, 0x582, a,b,c)     /* flt mul    */
#define M_DMULS(a,b,c)          M_FOP3 (0x16, 0x5a2, a,b,c)     /* dbl mul    */
#define M_FDIVS(a,b,c)          M_FOP3 (0x16, 0x583, a,b,c)     /* flt div    */
#define M_DDIVS(a,b,c)          M_FOP3 (0x16, 0x5a3, a,b,c)     /* dbl div    */

#define M_CVTDF(b,c)            M_FOP3 (0x16, 0x0ac, 31,b,c)    /* dbl2flt    */
#define M_CVTLF(b,c)            M_FOP3 (0x16, 0x0bc, 31,b,c)    /* long2flt   */
#define M_CVTLD(b,c)            M_FOP3 (0x16, 0x0be, 31,b,c)    /* long2dbl   */
#define M_CVTDL(b,c)            M_FOP3 (0x16, 0x1af, 31,b,c)    /* dbl2long   */
#define M_CVTDL_C(b,c)          M_FOP3 (0x16, 0x12f, 31,b,c)    /* dbl2long   */
#define M_CVTLI(b,c)            M_FOP3 (0x17, 0x130, 31,b,c)    /* long2int   */

#define M_CVTDFS(b,c)           M_FOP3 (0x16, 0x5ac, 31,b,c)    /* dbl2flt    */
#define M_CVTFDS(b,c)           M_FOP3 (0x16, 0x6ac, 31,b,c)    /* flt2dbl    */
#define M_CVTDLS(b,c)           M_FOP3 (0x16, 0x5af, 31,b,c)    /* dbl2long   */
#define M_CVTDL_CS(b,c)         M_FOP3 (0x16, 0x52f, 31,b,c)    /* dbl2long   */
#define M_CVTLIS(b,c)           M_FOP3 (0x17, 0x530, 31,b,c)    /* long2int   */

#define M_FCMPEQ(a,b,c)         M_FOP3 (0x16, 0x0a5, a,b,c)     /* c = a==b   */
#define M_FCMPLT(a,b,c)         M_FOP3 (0x16, 0x0a6, a,b,c)     /* c = a<b    */

#define M_FCMPEQS(a,b,c)        M_FOP3 (0x16, 0x5a5, a,b,c)     /* c = a==b   */
#define M_FCMPLTS(a,b,c)        M_FOP3 (0x16, 0x5a6, a,b,c)     /* c = a<b    */

#define M_FMOV(fa,fb)           M_FOP3 (0x17, 0x020, fa,fa,fb)  /* b = a      */
#define M_DMOV(fa,fb)           M_FMOV (fa,fb)
#define M_FMOVN(fa,fb)          M_FOP3 (0x17, 0x021, fa,fa,fb)  /* b = -a     */

#define M_FNOP                  M_FMOV (31,31)

#define M_FBEQZ(fa,disp)        M_BRA (0x31,fa,disp)            /* br a == 0.0*/

/* macros for special commands (see an Alpha-manual for description) **********/

#define M_TRAPB                 M_MEM (0x18,0,0,0x0000)        /* trap barrier*/

#define M_S4ADDL(a,b,c)         M_OP3 (0x10,0x02, a,b,c,0)     /* c = a*4 + b */
#define M_S4ADDQ(a,b,c)         M_OP3 (0x10,0x22, a,b,c,0)     /* c = a*4 + b */
#define M_S4SUBL(a,b,c)         M_OP3 (0x10,0x0b, a,b,c,0)     /* c = a*4 - b */
#define M_S4SUBQ(a,b,c)         M_OP3 (0x10,0x2b, a,b,c,0)     /* c = a*4 - b */
#define M_S8ADDL(a,b,c)         M_OP3 (0x10,0x12, a,b,c,0)     /* c = a*8 + b */
#define M_S8ADDQ(a,b,c)         M_OP3 (0x10,0x32, a,b,c,0)     /* c = a*8 + b */
#define M_S8SUBL(a,b,c)         M_OP3 (0x10,0x1b, a,b,c,0)     /* c = a*8 - b */
#define M_S8SUBQ(a,b,c)         M_OP3 (0x10,0x3b, a,b,c,0)     /* c = a*8 - b */
#define M_SAADDQ(a,b,c)         M_S8ADDQ(a,b,c)                /* c = a*8 + b */

#define M_S4ADDL_IMM(a,b,c)     M_OP3 (0x10,0x02, a,b,c,1)     /* c = a*4 + b */
#define M_S4ADDQ_IMM(a,b,c)     M_OP3 (0x10,0x22, a,b,c,1)     /* c = a*4 + b */
#define M_S4SUBL_IMM(a,b,c)     M_OP3 (0x10,0x0b, a,b,c,1)     /* c = a*4 - b */
#define M_S4SUBQ_IMM(a,b,c)     M_OP3 (0x10,0x2b, a,b,c,1)     /* c = a*4 - b */
#define M_S8ADDL_IMM(a,b,c)     M_OP3 (0x10,0x12, a,b,c,1)     /* c = a*8 + b */
#define M_S8ADDQ_IMM(a,b,c)     M_OP3 (0x10,0x32, a,b,c,1)     /* c = a*8 + b */
#define M_S8SUBL_IMM(a,b,c)     M_OP3 (0x10,0x1b, a,b,c,1)     /* c = a*8 - b */
#define M_S8SUBQ_IMM(a,b,c)     M_OP3 (0x10,0x3b, a,b,c,1)     /* c = a*8 - b */

#define M_LLD_U(a,b,disp)       M_MEM (0x0b,a,b,disp)          /* unalign ld  */
#define M_LST_U(a,b,disp)       M_MEM (0x0f,a,b,disp)          /* unalign st  */

#define M_ZAP(a,b,c)            M_OP3 (0x12,0x30, a,b,c,0)
#define M_ZAPNOT(a,b,c)         M_OP3 (0x12,0x31, a,b,c,0)

#define M_ZAP_IMM(a,b,c)        M_OP3 (0x12,0x30, a,b,c,1)
#define M_ZAPNOT_IMM(a,b,c)     M_OP3 (0x12,0x31, a,b,c,1)

#define M_BZEXT(a,b)            M_ZAPNOT_IMM(a, 0x01, b)       /*  8 zeroext  */
#define M_CZEXT(a,b)            M_ZAPNOT_IMM(a, 0x03, b)       /* 16 zeroext  */
#define M_IZEXT(a,b)            M_ZAPNOT_IMM(a, 0x0f, b)       /* 32 zeroext  */

#define M_EXTBL(a,b,c)          M_OP3 (0x12,0x06, a,b,c,0)
#define M_EXTWL(a,b,c)          M_OP3 (0x12,0x16, a,b,c,0)
#define M_EXTLL(a,b,c)          M_OP3 (0x12,0x26, a,b,c,0)
#define M_EXTQL(a,b,c)          M_OP3 (0x12,0x36, a,b,c,0)
#define M_EXTWH(a,b,c)          M_OP3 (0x12,0x5a, a,b,c,0)
#define M_EXTLH(a,b,c)          M_OP3 (0x12,0x6a, a,b,c,0)
#define M_EXTQH(a,b,c)          M_OP3 (0x12,0x7a, a,b,c,0)
#define M_INSBL(a,b,c)          M_OP3 (0x12,0x0b, a,b,c,0)
#define M_INSWL(a,b,c)          M_OP3 (0x12,0x1b, a,b,c,0)
#define M_INSLL(a,b,c)          M_OP3 (0x12,0x2b, a,b,c,0)
#define M_INSQL(a,b,c)          M_OP3 (0x12,0x3b, a,b,c,0)
#define M_INSWH(a,b,c)          M_OP3 (0x12,0x57, a,b,c,0)
#define M_INSLH(a,b,c)          M_OP3 (0x12,0x67, a,b,c,0)
#define M_INSQH(a,b,c)          M_OP3 (0x12,0x77, a,b,c,0)
#define M_MSKBL(a,b,c)          M_OP3 (0x12,0x02, a,b,c,0)
#define M_MSKWL(a,b,c)          M_OP3 (0x12,0x12, a,b,c,0)
#define M_MSKLL(a,b,c)          M_OP3 (0x12,0x22, a,b,c,0)
#define M_MSKQL(a,b,c)          M_OP3 (0x12,0x32, a,b,c,0)
#define M_MSKWH(a,b,c)          M_OP3 (0x12,0x52, a,b,c,0)
#define M_MSKLH(a,b,c)          M_OP3 (0x12,0x62, a,b,c,0)
#define M_MSKQH(a,b,c)          M_OP3 (0x12,0x72, a,b,c,0)

#define M_EXTBL_IMM(a,b,c)      M_OP3 (0x12,0x06, a,b,c,1)
#define M_EXTWL_IMM(a,b,c)      M_OP3 (0x12,0x16, a,b,c,1)
#define M_EXTLL_IMM(a,b,c)      M_OP3 (0x12,0x26, a,b,c,1)
#define M_EXTQL_IMM(a,b,c)      M_OP3 (0x12,0x36, a,b,c,1)
#define M_EXTWH_IMM(a,b,c)      M_OP3 (0x12,0x5a, a,b,c,1)
#define M_EXTLH_IMM(a,b,c)      M_OP3 (0x12,0x6a, a,b,c,1)
#define M_EXTQH_IMM(a,b,c)      M_OP3 (0x12,0x7a, a,b,c,1)
#define M_INSBL_IMM(a,b,c)      M_OP3 (0x12,0x0b, a,b,c,1)
#define M_INSWL_IMM(a,b,c)      M_OP3 (0x12,0x1b, a,b,c,1)
#define M_INSLL_IMM(a,b,c)      M_OP3 (0x12,0x2b, a,b,c,1)
#define M_INSQL_IMM(a,b,c)      M_OP3 (0x12,0x3b, a,b,c,1)
#define M_INSWH_IMM(a,b,c)      M_OP3 (0x12,0x57, a,b,c,1)
#define M_INSLH_IMM(a,b,c)      M_OP3 (0x12,0x67, a,b,c,1)
#define M_INSQH_IMM(a,b,c)      M_OP3 (0x12,0x77, a,b,c,1)
#define M_MSKBL_IMM(a,b,c)      M_OP3 (0x12,0x02, a,b,c,1)
#define M_MSKWL_IMM(a,b,c)      M_OP3 (0x12,0x12, a,b,c,1)
#define M_MSKLL_IMM(a,b,c)      M_OP3 (0x12,0x22, a,b,c,1)
#define M_MSKQL_IMM(a,b,c)      M_OP3 (0x12,0x32, a,b,c,1)
#define M_MSKWH_IMM(a,b,c)      M_OP3 (0x12,0x52, a,b,c,1)
#define M_MSKLH_IMM(a,b,c)      M_OP3 (0x12,0x62, a,b,c,1)
#define M_MSKQH_IMM(a,b,c)      M_OP3 (0x12,0x72, a,b,c,1)

#define M_UMULH(a,b,c)          M_OP3 (0x13,0x30, a,b,c,0)     /* 64 umulh    */

#define M_UMULH_IMM(a,b,c)      M_OP3 (0x13,0x30, a,b,c,1)     /* 64 umulh    */

#define M_CMOVEQ(a,b,c)         M_OP3 (0x11,0x24, a,b,c,0)     /* a==0 ? c=b  */
#define M_CMOVNE(a,b,c)         M_OP3 (0x11,0x26, a,b,c,0)     /* a!=0 ? c=b  */
#define M_CMOVLT(a,b,c)         M_OP3 (0x11,0x44, a,b,c,0)     /* a< 0 ? c=b  */
#define M_CMOVGE(a,b,c)         M_OP3 (0x11,0x46, a,b,c,0)     /* a>=0 ? c=b  */
#define M_CMOVLE(a,b,c)         M_OP3 (0x11,0x64, a,b,c,0)     /* a<=0 ? c=b  */
#define M_CMOVGT(a,b,c)         M_OP3 (0x11,0x66, a,b,c,0)     /* a> 0 ? c=b  */

#define M_CMOVEQ_IMM(a,b,c)     M_OP3 (0x11,0x24, a,b,c,1)     /* a==0 ? c=b  */
#define M_CMOVNE_IMM(a,b,c)     M_OP3 (0x11,0x26, a,b,c,1)     /* a!=0 ? c=b  */
#define M_CMOVLT_IMM(a,b,c)     M_OP3 (0x11,0x44, a,b,c,1)     /* a< 0 ? c=b  */
#define M_CMOVGE_IMM(a,b,c)     M_OP3 (0x11,0x46, a,b,c,1)     /* a>=0 ? c=b  */
#define M_CMOVLE_IMM(a,b,c)     M_OP3 (0x11,0x64, a,b,c,1)     /* a<=0 ? c=b  */
#define M_CMOVGT_IMM(a,b,c)     M_OP3 (0x11,0x66, a,b,c,1)     /* a> 0 ? c=b  */

// 0x04 seems to be the first undefined instruction which does not
// call PALcode.
#define M_UNDEFINED             M_OP3(0x04, 0, 0, 0, 0, 0)

/* macros for unused commands (see an Alpha-manual for description) ***********/

#define M_ANDNOT(a,b,c,const)   M_OP3 (0x11,0x08, a,b,c,const) /* c = a &~ b  */
#define M_ORNOT(a,b,c,const)    M_OP3 (0x11,0x28, a,b,c,const) /* c = a |~ b  */
#define M_XORNOT(a,b,c,const)   M_OP3 (0x11,0x48, a,b,c,const) /* c = a ^~ b  */

#define M_CMPBGE(a,b,c,const)   M_OP3 (0x10,0x0f, a,b,c,const)

#define M_FCMPUN(a,b,c)         M_FOP3 (0x16, 0x0a4, a,b,c)    /* unordered   */
#define M_FCMPLE(a,b,c)         M_FOP3 (0x16, 0x0a7, a,b,c)    /* c = a<=b    */

#define M_FCMPUNS(a,b,c)        M_FOP3 (0x16, 0x5a4, a,b,c)    /* unordered   */
#define M_FCMPLES(a,b,c)        M_FOP3 (0x16, 0x5a7, a,b,c)    /* c = a<=b    */

#define M_FBNEZ(fa,disp)        M_BRA (0x35,fa,disp)
#define M_FBLEZ(fa,disp)        M_BRA (0x33,fa,disp)

#define M_JMP_CO(a,b)           M_MEM (0x1a,a,b,0xc000)        /* call cosub  */

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
