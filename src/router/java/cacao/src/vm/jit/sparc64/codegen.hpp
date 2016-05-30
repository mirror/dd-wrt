/* src/vm/jit/sparc64/codegen.hpp - code generation macros and
                                    definitions for SPARC64

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

#include "md-abi.hpp" /* for INT_NATARG_CNT */

#include <cassert>

/* debug defines **************************************************************/
#ifndef NDEBUG
# define PASS13BIT(imm) ((((s4)(imm)&0x1fff)<<19)>>19)
#else
# define PASS13BIT(imm) imm
#endif


/* from md-abi.c */
s4 nat_argintregs[INT_NATARG_CNT];

/* branch defines *************************************************************/

#define BRANCH_NOPS \
    do { \
        M_NOP; \
        M_NOP; \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_INSTRUCTIONS    2     /* number of instructions          */
#define PATCHER_CALL_SIZE            2 * 4 /* size in bytes of a patcher call */

#define EXCEPTION_CHECK_INSTRUCTIONS 3     /* number of instructions          */
#define EXCEPTION_CHECK_SIZE         3 * 4 /* byte size of an exception check */

#define PATCHER_NOPS \
    do { \
        M_NOP; \
        M_NOP; \
    } while (0)


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

#define ALIGN_STACK_SLOTS(slots) \
	if (slots & 1) \
		slots++;


#define M_COPY(s,d)                     emit_copy(jd, iptr, (s), (d))
#define ICONST(d,c)                     emit_iconst(cd, (d), (c))
#define LCONST(d,c)                     emit_lconst(cd, (d), (c))




/********************** instruction formats ***********************************/

#define REG     0
#define IMM     1

/* 3-address-operations: M_OP3
 *       op  ..... opcode
 *       op3 ..... operation
 *       rs1 ..... register number source 1
 *       rs2 ..... register number or constant integer source 2
 *       rd  ..... register number destination
 *       imm ..... switch to use rs2 as constant 13bit integer 
 *                  (REG means: use b as register number)
 *                  (IMM means: use b as signed immediate value)
 */
#define M_OP3(op,op3,rd,rs1,rs2,imm) \
	do { \
		assert(check_13bit_imm(rs2)); \
		*((u4 *) cd->mcodeptr) =  ((((s4) (op)) << 30) | ((rd) << 25) | ((op3) << 19) | ((rs1) << 14) | ((imm)<<13) | (imm?((rs2)&0x1fff):(rs2)) ); \
		cd->mcodeptr += 4; \
	} while (0)


#define M_OP3_GET_RD(x)               (((x) >> 25) & 0x1f)
#define M_OP3_GET_RS(x)               (((x) >> 14) & 0x1f)
#define M_OP3_GET_IMM(x)              ( (x)        & 0x1fff)

/* 3-address-operations: M_OP3C
 *       rcond ... condition opcode
 *       rs2 ..... register number or 10bit signed immediate
 *
 */
#define M_OP3C(op,op3,rcond,rd,rs1,rs2,imm) \
	do { \
		*((u4 *) cd->mcodeptr) = ((((s4) (op)) << 30) | ((rd) << 25) | ((op3) << 19) | ((rs1) << 14) | ((imm)<<13) | \
			((rcond) << 10) | (imm?((rs2)&0x3ff):(rs2)) ); \
		cd->mcodeptr += 4; \
	} while (0)


/* shift Format 3
 *    op ..... opcode
 *    op3..... op3 code
 *    rs1 .... source reg 1
 *    rs2 .... source reg 2 or immediate shift count (5 or 6 bits long depending whether 32 or 64 bit shift)
 *    rd ..... dest reg
 *    imm .... switch for constant
 *    x ...... 0 => 32, 1 => 64 bit shift 
 */
#define M_SHFT(op,op3,rs1,rs2,rd,imm,x) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op)) << 30) | ((op3) << 19) | ((rd) << 25) | ((rs1) << 14) | ((imm) << 13) | \
			((x) << 12) | (((imm) && (x))?((rs2) & 0x3f):((rs2) & 0x1f)) ); \
		cd->mcodeptr += 4; \
	} while (0)


/* Format 4
 *    op ..... opcode
 *    op3..... op3 code
 *    cond ... condition opcode
 *    rs2 .... source 2 or signed 11-bit constant
 *    rd ..... dest reg
 *    imm .... switch for constant
 *    cc{0-2}  32-bit 64-bit or fp condition
 */
 #define M_FMT4(op,op3,rd,rs2,cond,cc2,cc1,cc0,imm) \
	do { \
 		*((u4 *) cd->mcodeptr) =  ( (((s4)(op)) << 30) | ((op3) << 19) | ((rd) << 25) | ((cc2) << 18) |  ((cond) << 14) | \
 			((imm) << 13) | ((cc1) << 12) | ((cc0) << 11) | ((rs2) & 0x7ff) ); \
		cd->mcodeptr += 4; \
	} while (0)


#define FR_X(r) (((r)<<1) + 1) /* transpose macro for floats which reside in upper half of double word */
#define DR_X(r) ((((r)*2)|(((r)*2)>>5)) & 0x1f) /* transpose & pack double, see SPARC spec.            */

/* 3-address-floating-point-operation
 *   op .... opcode
 *   op3,opf .... function-number
 *   rd .... dest reg
 *   rs1 ... source reg (-1 signals unused)
 *   rs2 ... source reg
 *   
 *
 */ 
#define M_FOP3(op,op3,opf,rd,rs1,rs2) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op))<<30) | ((rd)<<25) | ((op3)<<19) | ((((rs1)==-1)?0:(rs1)) << 14) | \
		((opf)<<5) | (rs2) ); \
		cd->mcodeptr += 4; \
	} while (0)
/* float addressing */
#define M_FOP3_FX(op,op3,opf,rd,rs1,rs2) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op))<<30) | (FR_X(rd)<<25) | ((op3)<<19) | ((((rs1)==-1)?0:FR_X(rs1)) << 14) | \
		((opf)<<5) | FR_X(rs2) ); \
		cd->mcodeptr += 4; \
	} while (0)
/* double addressing */
#define M_FOP3_DX(op,op3,opf,rd,rs1,rs2) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op))<<30) | (DR_X(rd)<<25) | ((op3)<<19) | ((((rs1)==-1)?0:DR_X(rs1)) << 14) | \
		((opf)<<5) | DR_X(rs2) ); \
		cd->mcodeptr += 4; \
	} while (0)

/* 3-address-floating-point-compare-operation
 *   op .... opcode
 *   op3,opf .... function-number
 *   fcc ... floating point condition code (fcc0 - fcc3)
 *   rs1 ... source reg
 *   rs2 ... source reg
 *   
 *
 */ 
#define M_FCMP_DX(op,op3,opf,fcc,rs1,rs2) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op))<<30) | ((fcc)<<25) | ((op3)<<19) | (DR_X(rs1) << 14) | \
		((opf)<<5) | DR_X(rs2) ); \
		cd->mcodeptr += 4; \
	} while (0)
	
#define M_FCMP_FX(op,op3,opf,fcc,rs1,rs2) \
	do { \
		*((u4 *) cd->mcodeptr) =  ( (((s4)(op))<<30) | ((fcc)<<25) | ((op3)<<19) | (FR_X(rs1) << 14) | \
		((opf)<<5) | FR_X(rs2) ); \
		cd->mcodeptr += 4; \
	} while (0)

/**** format 2 operations ********/

/* branch on integer reg instruction 
      op ..... opcode
      rcond ...... condition to be tested
      disp16 ... 16-bit relative address to be jumped to (divided by 4)
      rs1 ..... register to be tested
      p ..... prediction bit
      anul .... annullment bit
*/
#define M_BRAREG(op,rcond,rs1,disp16,p,anul) \
	do { \
		*((u4 *) cd->mcodeptr) = ( (((s4)(op))<<30) | ((anul)<<29) | (0<<28) | ((rcond)<<25) | (3<<22) | \
			( ((disp16)& 0xC000) << 6 ) | (p << 19) | ((rs1) << 14) | ((disp16)&0x3fff) ); \
		cd->mcodeptr += 4; \
	} while (0)

		
/* branch on integer reg instruction 
      op,op2 .... opcodes
      cond ...... condition to be tested
      disp19 ... 19-bit relative address to be jumped to (divided by 4)
      ccx ..... 32(0) or 64(2) bit test
      p ..... prediction bit
      anul .... annullment bit
*/		
#define M_BRACC(op,op2,cond,disp19,ccx,p,anul) \
	do { \
		*((u4 *) cd->mcodeptr) = ( (((s4)(op))<<30) | ((anul)<<29) | ((cond)<<25) | (op2<<22) | (ccx<<20) | \
			(p << 19 ) | ((disp19) & 0x007ffff)  ); \
		cd->mcodeptr += 4; \
	} while (0)

        
/************** end-user instructions (see a SPARC asm manual) ***************/

#define M_SETHI(imm22, rd) \
	do { \
		*((u4 *) cd->mcodeptr) = ((((s4)(0x00)) << 30) | ((rd) << 25) | ((0x04)<<22) | ((imm22)&0x3FFFFF) ); \
		cd->mcodeptr += 4; \
	} while (0)

#define M_NOP M_SETHI(0,0)      /* nop */

#define M_AND(rs1,rs2,rd)       M_OP3(0x02,0x01,rd,rs1,rs2,REG)     /* 64b c = a &  b */
#define M_AND_IMM(rs1,rs2,rd)	M_OP3(0x02,0x01,rd,rs1,rs2,IMM)
#define M_ANDCC(rs1,rs2,rd)		M_OP3(0x02,0x11,rd,rs1,rs2,REG)
#define M_ANDCC_IMM(rs1,rs2,rd)	M_OP3(0x02,0x11,rd,rs1,rs2,IMM)

#define M_OR(rs1,rs2,rd)        M_OP3(0x02,0x02,rd,rs1,rs2,REG)     /* rd = rs1 | rs2     */
#define M_OR_IMM(rs1,rs2,rd)    M_OP3(0x02,0x02,rd,rs1,rs2,IMM)
#define M_XOR(rs1,rs2,rd)       M_OP3(0x02,0x03,rd,rs1,rs2,REG)     /* rd = rs1 ^  rs2    */
#define M_XOR_IMM(rs1,rs2,rd)   M_OP3(0x02,0x03,rd,rs1,rs2,IMM)

#define M_MOV(rs,rd)            M_OR(REG_ZERO, rs, rd)              /* rd = rs            */
#define M_CLR(rd)               M_OR(REG_ZERO,REG_ZERO,rd)          /* rd = 0             */



#define M_SLLX(rs1,rs2,rd)		M_SHFT(0x02,0x25,rs1,rs2,rd,REG,1)	/* 64b rd = rs << rs2 */
#define M_SLLX_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x25,rs1,rs2,rd,IMM,1)
#define M_SLL(rs1,rs2,rd)		M_SHFT(0x02,0x25,rs1,rs2,rd,REG,0)	/* 32b rd = rs << rs2 */
#define M_SLL_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x25,rs1,rs2,rd,IMM,0)
#define M_SRLX(rs1,rs2,rd)		M_SHFT(0x02,0x26,rs1,rs2,rd,REG,1)	/* 64b rd = rs >>>rs2 */
#define M_SRLX_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x26,rs1,rs2,rd,IMM,1)
#define M_SRL(rs1,rs2,rd)		M_SHFT(0x02,0x26,rs1,rs2,rd,REG,0)	/* 32b rd = rs >>>rs2 */
#define M_SRL_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x26,rs1,rs2,rd,IMM,0)
#define M_SRAX(rs1,rs2,rd)		M_SHFT(0x02,0x27,rs1,rs2,rd,REG,1)	/* 64b rd = rs >> rs2 */
#define M_SRAX_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x27,rs1,rs2,rd,IMM,1)
#define M_SRA(rs1,rs2,rd)       M_SHFT(0x02,0x27,rs1,rs2,rd,REG,0)  /* 32b rd = rs >> rs2 */
#define M_SRA_IMM(rs1,rs2,rd)	M_SHFT(0x02,0x27,rs1,rs2,rd,IMM,0)

#define M_ISEXT(rs,rd) 			M_SRA(rs,REG_ZERO,rd)                 /* sign extend 32 bits*/


#define M_ADD(rs1,rs2,rd)   	M_OP3(0x02,0x00,rd,rs1,rs2,REG)  	/* 64b rd = rs1 + rs2 */
#define M_ADD_IMM(rs1,rs2,rd)   M_OP3(0x02,0x00,rd,rs1,rs2,IMM)
#define M_SUB(rs1,rs2,rd)       M_OP3(0x02,0x04,rd,rs1,rs2,REG) 	/* 64b rd = rs1 - rs2 */
#define M_SUB_IMM(rs1,rs2,rd)   M_OP3(0x02,0x04,rd,rs1,rs2,IMM)
#define M_MULX(rs1,rs2,rd)      M_OP3(0x02,0x09,rd,rs1,rs2,REG)  	/* 64b rd = rs1 * rs2 */
#define M_MULX_IMM(rs1,rs2,rd)  M_OP3(0x02,0x09,rd,rs1,rs2,IMM)
#define M_DIVX(rs1,rs2,rd)      M_OP3(0x02,0x2d,rd,rs1,rs2,REG)  	/* 64b rd = rs1 / rs2 */

#define M_SUBcc(rs1,rs2,rd)     M_OP3(0x02,0x14,rd,rs1,rs2,REG)     /* sets xcc and icc   */
#define M_SUBcc_IMM(rs1,rs2,rd) M_OP3(0x02,0x14,rd,rs1,rs2,IMM)     /* sets xcc and icc   */



/**** compare and conditional ALU operations ***********/

#define M_CMP(rs1,rs2)          M_SUBcc(rs1,rs2,REG_ZERO)             
#define M_CMP_IMM(rs1,rs2)      M_SUBcc_IMM(rs1,rs2,REG_ZERO)


/* move integer register on (64-bit) condition */

#define M_XCMOVEQ(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0x1,1,1,0,REG)   /* a==b ? rd=rs    */
#define M_XCMOVNE(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0x9,1,1,0,REG)   /* a!=b ? rd=rs    */
#define M_XCMOVLT(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0x3,1,1,0,REG)   /* a<b  ? rd=rs    */
#define M_XCMOVGE(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0xb,1,1,0,REG)   /* a>=b ? rd=rs    */
#define M_XCMOVLE(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0x2,1,1,0,REG)   /* a<=b ? rd=rs    */
#define M_XCMOVGT(rs,rd)        M_FMT4(0x2,0x2c,rd,rs,0xa,1,1,0,REG)   /* a>b  ? rd=rs    */
#define M_XCMOVULE(rs,rd)       M_FMT4(0x2,0x2c,rd,rs,0x4,1,1,0,REG)   /* a<=b ? rd=rs (u)*/

#define M_XCMOVEQ_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x1,1,1,0,IMM)   /* a==b ? rd=rs    */
#define M_XCMOVNE_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x9,1,1,0,IMM)   /* a!=b ? rd=rs    */
#define M_XCMOVLT_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x3,1,1,0,IMM)   /* a<b  ? rd=rs    */
#define M_XCMOVGE_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0xb,1,1,0,IMM)   /* a>=b ? rd=rs    */
#define M_XCMOVLE_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x2,1,1,0,IMM)   /* a<=b ? rd=rs    */
#define M_XCMOVGT_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0xa,1,1,0,IMM)   /* a>b  ? rd=rs    */
#define M_XCMOVULE_IMM(rs,rd)   M_FMT4(0x2,0x2c,rd,rs,0x4,1,1,0,IMM)   /* a<=b ? rd=rs (u)*/

/* move integer register on (fcc0) floating point condition */

#define M_CMOVFGT_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x6,0,0,0,IMM)   /* fa>fb  ? rd=rs  */
#define M_CMOVFLT_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x4,0,0,0,IMM)   /* fa<fb  ? rd=rs  */
#define M_CMOVFEQ_IMM(rs,rd)    M_FMT4(0x2,0x2c,rd,rs,0x9,0,0,0,IMM)   /* fa==fb ? rd=rs  */

/* move integer register on (32-bit) condition */



/* move integer register on register condition */

#define M_CMOVREQ(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x1,rd,rs1,rs2,REG)      /* rs1==0 ? rd=rs2 */
#define M_CMOVRNE(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x5,rd,rs1,rs2,REG)      /* rs1!=0 ? rd=rs2 */
#define M_CMOVRLE(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x2,rd,rs1,rs2,REG)      /* rs1<=0 ? rd=rs2 */
#define M_CMOVRLT(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x3,rd,rs1,rs2,REG)      /* rs1<0  ? rd=rs2 */
#define M_CMOVRGT(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x6,rd,rs1,rs2,REG)      /* rs1>0  ? rd=rs2 */
#define M_CMOVRGE(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x7,rd,rs1,rs2,REG)      /* rs1>=0 ? rd=rs2 */

#define M_CMOVREQ_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x1,rd,rs1,rs2,IMM)  /* rs1==0 ? rd=rs2 */
#define M_CMOVRNE_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x5,rd,rs1,rs2,IMM)  /* rs1!=0 ? rd=rs2 */
#define M_CMOVRLE_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x2,rd,rs1,rs2,IMM)  /* rs1<=0 ? rd=rs2 */
#define M_CMOVRLT_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x3,rd,rs1,rs2,IMM)  /* rs1<0  ? rd=rs2 */
#define M_CMOVRGT_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x6,rd,rs1,rs2,IMM)  /* rs1>0  ? rd=rs2 */
#define M_CMOVRGE_IMM(rs1,rs2,rd) M_OP3C(0x2,0x2f,0x7,rd,rs1,rs2,IMM)  /* rs1>=0 ? rd=rs2 */


/**** big constant helpers *********/

/* #define FITS_13BIT_IMM(x)       ((x >= -4096) && (x <= 4095)) */

bool fits_13(s4 disp);
s4   get_lopart_disp(s4 disp);

#define abs(x) ((x) < 0 ? (-(x)) : (x))

#define sethi_part(x) ((x)>>10)
#define setlo_part(x) ((x) & 0x3ff)

#define DO_SETHI_REG(c,rd) \
	do { \
		if (c > 0) { \
			M_SETHI(sethi_part(c), rd); \
			if (setlo_part(c)) { \
				M_OR_IMM(rd, setlo_part(c), rd); \
			} \
		} \
		else { \
			M_SETHI(sethi_part(~c), rd); \
			M_XOR_IMM(rd, PASS13BIT(setlo_part(c) | 0xffffffffffff1c00), rd); \
		} \
	} while (0)
	
#define DO_SETHI_PART(c,rs,rd) \
	do { \
		if (c > 0) { \
			M_SETHI(sethi_part(c), rd); \
			M_ADD(rs,rd,rd); \
		} \
		else { \
			M_SETHI(sethi_part(-c), rd); \
			M_SUB(rs,rd,rd); \
			assert(sethi_part(c) != 0xf); \
		} \
	} while (0)
	

		

#define M_LDA(rd,rs,disp) \
    do { \
        if (fits_13(disp)) { \
            M_AADD_IMM(rs,disp,rd); \
        } \
        else { \
            DO_SETHI_REG(disp,rd); \
            M_AADD(rd,rs,rd); \
        } \
    } while (0)

/**** load/store operations ********/

#define M_SLDU(rd,rs,disp)      M_OP3(0x03,0x02,rd,rs,disp,IMM)        /* 16-bit load, uns*/
#define M_SLDS(rd,rs,disp)      M_OP3(0x03,0x0a,rd,rs,disp,IMM)        /* 16-bit load, sig*/
#define M_BLDS(rd,rs,disp)      M_OP3(0x03,0x09,rd,rs,disp,IMM)        /* 8-bit load, sig */


#define M_LDX_INTERN(rd,rs,disp) M_OP3(0x03,0x0b,rd,rs,disp,IMM)       /* 64-bit load, sig*/
#define M_LDX(rd,rs,disp) \
	do { \
        if (fits_13(disp)) { \
            M_LDX_INTERN(rd,rs,disp); \
        } \
        else { \
        	DO_SETHI_PART(disp,rs,rd); \
        	M_LDX_INTERN(rd,rd,PASS13BIT(get_lopart_disp(disp))); \
        } \
    } while (0)

#define M_ILD_INTERN(rd,rs,disp) M_OP3(0x03,0x08,rd,rs,disp,IMM)       /* 32-bit load, sig */
#define M_ILD(rd,rs,disp) \
	do { \
        if (fits_13(disp)) { \
            M_ILD_INTERN(rd,rs,disp); \
       } \
        else { \
            DO_SETHI_PART(disp,rs,rd); \
            M_ILD_INTERN(rd,rd,PASS13BIT(get_lopart_disp(disp))); \
        } \
    } while (0)



#define M_SST(rd,rs,disp)       M_OP3(0x03,0x06,rd,rs,disp,IMM)        /* 16-bit store     */
#define M_BST(rd,rs,disp)       M_OP3(0x03,0x05,rd,rs,disp,IMM)        /*  8-bit store     */

/* Stores with displacement overflow should only happen with PUTFIELD or on   */
/* the stack. The PUTFIELD instruction does not use REG_ITMP3 and a           */
/* reg_of_var call should not use REG_ITMP3!!!                                */

#define M_STX_INTERN(rd,rs,disp) M_OP3(0x03,0x0e,rd,rs,disp,IMM)       /* 64-bit store    */
#define M_STX(rd,rs,disp) \
	do { \
        if (fits_13(disp)) { \
            M_STX_INTERN(rd,rs,disp); \
        } \
        else { \
            DO_SETHI_PART(disp,rs,REG_ITMP3); \
            M_STX_INTERN(rd,REG_ITMP3,PASS13BIT(get_lopart_disp(disp))); \
        } \
    } while (0)


#define M_IST_INTERN(rd,rs,disp) M_OP3(0x03,0x04,rd,rs,disp,IMM)       /* 32-bit store    */
#define M_IST(rd,rs,disp) \
    do { \
        if (fits_13(disp)) { \
            M_IST_INTERN(rd,rs,disp); \
       } \
        else { \
            DO_SETHI_PART(disp,rs,REG_ITMP3); \
            M_IST_INTERN(rd,REG_ITMP3,PASS13BIT(get_lopart_disp(disp))); \
        } \
    } while (0)


/**** branch operations ********/
/* XXX prediction and annul bits currently set to defaults, but could be used for optimizations */

/* branch on integer register */

#define M_BEQZ(r,disp)          M_BRAREG(0x0,0x1,r,disp,1,0)          /* br r == 0   */
#define M_BLEZ(r,disp)          M_BRAREG(0x0,0x2,r,disp,1,0)          /* br r <= 0   */
#define M_BLTZ(r,disp)          M_BRAREG(0x0,0x3,r,disp,1,0)          /* br r < 0    */
#define M_BNEZ(r,disp)          M_BRAREG(0x0,0x5,r,disp,1,0)          /* br r != 0   */
#define M_BGTZ(r,disp)          M_BRAREG(0x0,0x6,r,disp,1,0)          /* br r > 0    */
#define M_BGEZ(r,disp)          M_BRAREG(0x0,0x7,r,disp,1,0)          /* br r >= 0   */


/* branch on (64-bit) integer condition codes */

#define M_XBEQ(disp)            M_BRACC(0x00,0x1,0x1,disp,2,1,0)      /* branch a==b */
#define M_XBNE(disp)            M_BRACC(0x00,0x1,0x9,disp,2,1,0)      /* branch a!=b */
#define M_XBGT(disp)            M_BRACC(0x00,0x1,0xa,disp,2,1,0)      /* branch a>b  */
#define M_XBLT(disp)            M_BRACC(0x00,0x1,0x3,disp,2,1,0)      /* branch a<b  */
#define M_XBGE(disp)            M_BRACC(0x00,0x1,0xb,disp,2,1,0)      /* branch a>=b */
#define M_XBLE(disp)            M_BRACC(0x00,0x1,0x2,disp,2,1,0)      /* branch a<=b */
#define M_XBUGE(disp)           M_BRACC(0x00,0x1,0xd,disp,2,1,0)      /* br uns a>=b */
#define M_XBUGT(disp)           M_BRACC(0x00,0x1,0xc,disp,2,1,0)      /* br uns a>b  */
#define M_XBULT(disp)           M_BRACC(0x00,0x1,0x5,disp,2,1,0)      /* br uns a<b  */

/* branch on (32-bit) integer condition codes */

#define M_BR(disp)              M_BRACC(0x00,0x1,0x8,disp,0,1,0)      /* branch      */
#define M_BEQ(disp)             M_BRACC(0x00,0x1,0x1,disp,0,1,0)      /* branch a==b */
#define M_BNE(disp)             M_BRACC(0x00,0x1,0x9,disp,0,1,0)      /* branch a!=b */
#define M_BGT(disp)             M_BRACC(0x00,0x1,0xa,disp,0,1,0)      /* branch a>b  */
#define M_BLT(disp)             M_BRACC(0x00,0x1,0x3,disp,0,1,0)      /* branch a<b  */
#define M_BGE(disp)             M_BRACC(0x00,0x1,0xb,disp,0,1,0)      /* branch a>=b */
#define M_BLE(disp)             M_BRACC(0x00,0x1,0x2,disp,0,1,0)      /* branch a<=b */
#define M_BULE(disp)            M_BRACC(0x00,0x1,0x4,disp,0,1,0)      /* br uns a<=b */
#define M_BUGT(disp)            M_BRACC(0x00,0x1,0xc,disp,0,1,0)      /* br uns a>b  */
#define M_BULT(disp)            M_BRACC(0x00,0x1,0x5,disp,0,1,0)      /* br uns a<b  */

/* branch on (fcc0) floating point condition codes */

#define M_FBR(disp)             M_BRACC(0x00,0x5,0x8,disp,0,1,0)      /* branch      */
#define M_FBU(disp)             M_BRACC(0x00,0x5,0x7,disp,0,1,0)      /* unordered   */
#define M_FBG(disp)             M_BRACC(0x00,0x5,0x6,disp,0,1,0)      /* branch a>b  */
#define M_FBL(disp)             M_BRACC(0x00,0x5,0x4,disp,0,1,0)      /* branch a<b  */
#define M_FBO(disp)             M_BRACC(0x00,0x5,0xf,disp,0,1,0)      /* br ordered  */



#define M_SAVE(rs1,rs2,rd)      M_OP3(0x02,0x3c,rd,rs1,rs2,IMM)
#define M_SAVE_REG(rs1,rs2,rd)  M_OP3(0x02,0x3c,rd,rs1,rs2,REG)
#define M_RESTORE(rs1,rs2,rd)   M_OP3(0x02,0x3d,rd,rs1,rs2,IMM)



#define M_JMP(rd,rs1,rs2)       M_OP3(0x02,0x38,rd, rs1,rs2,REG)  /* jump to rs1+rs2, adr of instr. saved to rd */
#define M_JMP_IMM(rd,rs1,rs2)   M_OP3(0x02,0x38,rd, rs1,rs2,IMM)
#define M_RET(rs1,imm)          M_OP3(0x02,0x38,REG_ZERO,rs1,imm,IMM) /* a jump which discards the current pc */

#define M_RETURN(rs1,imm)       M_OP3(0x02,0x39,0,rs1,imm,IMM) /* like ret, but does window restore */
 
/**** floating point operations **/


#define M_DMOV(rs,rd)           M_FOP3_DX(0x02,0x34,0x02,rd,-1,rs)      /* rd = rs */
#define M_FMOV(rs,rd)           M_FOP3_FX(0x02,0x34,0x01,rd,-1,rs)      /* rd = rs */

#define M_FMOV_INTERN(rs,rd)    M_FOP3(0x02,0x34,0x01,rd,-1,rs)         /* rd = rs */

#define M_FNEG(rs,rd)          	M_FOP3_FX(0x02,0x34,0x05,rd,-1,rs)	 	/* rd = -rs     */
#define M_DNEG(rs,rd)          	M_FOP3_DX(0x02,0x34,0x06,rd,-1,rs)  	/* rd = -rs     */

#define M_FADD(rs1,rs2,rd)      M_FOP3_FX(0x02,0x34,0x41,rd,rs1,rs2)  /* float add    */
#define M_DADD(rs1,rs2,rd)      M_FOP3_DX(0x02,0x34,0x42,rd,rs1,rs2)  /* double add   */
#define M_FSUB(rs1,rs2,rd)      M_FOP3_FX(0x02,0x34,0x045,rd,rs1,rs2)	/* float sub    */
#define M_DSUB(rs1,rs2,rd)      M_FOP3_DX(0x02,0x34,0x046,rd,rs1,rs2) /* double sub   */
#define M_FMUL(rs1,rs2,rd)      M_FOP3_FX(0x02,0x34,0x049,rd,rs1,rs2) /* float mul    */
#define M_DMUL(rs1,rs2,rd)      M_FOP3_DX(0x02,0x34,0x04a,rd,rs1,rs2) /* double mul   */
#define M_FDIV(rs1,rs2,rd)      M_FOP3_FX(0x02,0x34,0x04d,rd,rs1,rs2) /* float div    */
#define M_DDIV(rs1,rs2,rd)      M_FOP3_DX(0x02,0x34,0x04e,rd,rs1,rs2) /* double div   */


/**** compare and conditional FPU operations ***********/

/* rd field 0 ==> fcc target unit is fcc0 */
#define M_FCMP(rs1,rs2)		    M_FCMP_FX(0x02,0x35,0x051,0,rs1,rs2)     /* compare flt  */
#define M_DCMP(rs1,rs2)		    M_FCMP_DX(0x02,0x35,0x052,0,rs1,rs2)     /* compare dbl  */

/* conversion functions */

#define M_CVTIF(rs,rd)          M_FOP3_FX(0x02,0x34,0x0c4,rd,-1,rs)/* int2flt      */
#define M_CVTID(rs,rd)          M_FOP3(0x02,0x34,0x0c8,DR_X(rd),-1,FR_X(rs))  /* int2dbl      */
#define M_CVTLF(rs,rd)          M_FOP3(0x02,0x34,0x084,FR_X(rd),-1,DR_X(rs))  /* long2flt     */
#define M_CVTLD(rs,rd)          M_FOP3_DX(0x02,0x34,0x088,rd,-1,rs)    /* long2dbl     */

#define M_CVTFI(rs,rd)          M_FOP3_FX(0x02,0x34,0x0d1,rd,-1,rs)   /* flt2int   */
#define M_CVTDI(rs,rd)          M_FOP3(0x02,0x34,0x0d2,FR_X(rd),-1,DR_X(rs))     /* dbl2int   */
#define M_CVTFL(rs,rd)          M_FOP3(0x02,0x34,0x081,DR_X(rd),-1,FR_X(rs))     /* flt2long  */
#define M_CVTDL(rs,rd)          M_FOP3_DX(0x02,0x34,0x082,rd,-1,rs)       /* dbl2long  */

#define M_CVTFD(rs,rd)          M_FOP3(0x02,0x34,0x0c9,DR_X(rd),-1,FR_X(rs))     /* flt2dbl   */
#define M_CVTDF(rs,rd)          M_FOP3(0x02,0x34,0x0c6,FR_X(rd),-1,DR_X(rs))     /* dbl2float */



#define M_DLD_INTERN(rd,rs1,disp) M_OP3(0x03,0x23,DR_X(rd),rs1,disp,IMM)    /* double (64-bit) load */
#define M_DLD(rd,rs,disp) \
	do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 13); \
        if (hi == 0) { \
            M_DLD_INTERN(rd,rs,lo); \
        } else { \
            M_SETHI(hi&0x3ffff8,rd); \
            M_AADD(rs,rd,rd); \
            M_DLD_INTERN(rd,rd,PASS13BIT(lo)); \
        } \
    } while (0)
/* Note for SETHI: sethi has a 22bit imm, only set upper 19 bits */ 

#define M_FLD_INTERN(rd,rs1,disp) M_OP3(0x03,0x20,FR_X(rd),rs1,disp,IMM)    /* float (32-bit) load */
#define M_FLD(rd,rs,disp) \
	do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 13); \
        if (hi == 0) { \
            M_FLD_INTERN(rd,rs,lo); \
        } else { \
            M_SETHI(hi&0x3ffff8,rd); \
            M_AADD(rs,rd,rd); \
            M_FLD_INTERN(rd,rd,PASS13BIT(lo)); \
        } \
    } while (0)


#define M_FST_INTERN(rd,rs,disp) M_OP3(0x03,0x24,FR_X(rd),rs,disp,IMM)    /* float (32-bit) store  */
#define M_FST(rd,rs,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 13); \
        if (hi == 0) { \
            M_FST_INTERN(rd,rs,lo); \
        } else { \
            M_SETHI(hi&0x3ffff8,REG_ITMP3); \
            M_AADD(rs,REG_ITMP3,REG_ITMP3); \
            M_FST_INTERN(rd,REG_ITMP3,PASS13BIT(lo)); \
        } \
    } while (0)
    

#define M_DST_INTERN(rd,rs1,disp) M_OP3(0x03,0x27,DR_X(rd),rs1,disp,IMM)    /* double (64-bit) store */
#define M_DST(rd,rs,disp) \
    do { \
        s4 lo = (short) (disp); \
        s4 hi = (short) (((disp) - lo) >> 13); \
        if (hi == 0) { \
            M_DST_INTERN(rd,rs,lo); \
        } else { \
            M_SETHI(hi&0x3ffff8,REG_ITMP3); \
            M_AADD(rs,REG_ITMP3,REG_ITMP3); \
            M_DST_INTERN(rd,REG_ITMP3,PASS13BIT(lo)); \
        } \
    } while (0)
    
    
    
/*
 * Address pseudo instruction
 */

#define POINTERSHIFT 3 /* x8 */


#define M_ALD_INTERN(a,b,disp)  M_LDX_INTERN(a,b,disp)
#define M_ALD(rd,rs,disp)       M_LDX(rd,rs,disp)
#define M_AST_INTERN(a,b,disp)  M_STX_INTERN(a,b,disp)
#define M_AST(a,b,disp)         M_STX(a,b,disp)
#define M_AADD(a,b,c)           M_ADD(a,b,c)
#define M_AADD_IMM(a,b,c)       M_ADD_IMM(a,b,c)
#define M_ASUB_IMM(a,b,c)       M_SUB_IMM(a,b,c)
#define M_ASLL_IMM(a,b,c)       M_SLLX_IMM(a,b,c)

#define M_ACMP(a,b)             M_CMP(a,b)
#define M_ICMP(a,b)             M_CMP(a,b)

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