/* src/vm/jit/arm/codegen.hpp - code generation macros and definitions for ARM

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


/******************************************************************************/
/* register splitting stuff (ugly) ********************************************/
/******************************************************************************/

#if defined(__ARMEL__)

# define SPLIT_OPEN(type, reg, tmpreg) \
	if (IS_2_WORD_TYPE(type) && GET_HIGH_REG(reg)==REG_SPLIT) { \
		/*dolog("SPLIT_OPEN({R%d;SPL} > {R%d;R%d})", GET_LOW_REG(reg), GET_LOW_REG(reg), tmpreg);*/ \
		/*assert(GET_LOW_REG(reg) == 3);*/ \
		(reg) = PACK_REGS(GET_LOW_REG(reg), tmpreg); \
	}

# define SPLIT_STORE_AND_CLOSE(type, reg, offset) \
	if (IS_2_WORD_TYPE(type) && GET_LOW_REG(reg)==3) { \
		/*dolog("SPLIT_STORE({R%d;R%d} to [%x])", GET_LOW_REG(reg), GET_HIGH_REG(reg), offset);*/ \
		M_STR(GET_HIGH_REG(reg), REG_SP, 4 * (offset)); \
		(reg) = PACK_REGS(GET_LOW_REG(reg), REG_SPLIT); \
	}

#else /* defined(__ARMEB__) */

# define SPLIT_OPEN(type, reg, tmpreg) \
	if (IS_2_WORD_TYPE(type) && GET_LOW_REG(reg)==REG_SPLIT) { \
		/*dolog("SPLIT_OPEN({SPL;R%d} > {R%d;R%d})", GET_HIGH_REG(reg), tmpreg, GET_HIGH_REG(reg));*/ \
		/*assert(GET_HIGH_REG(reg) == 3);*/ \
		(reg) = PACK_REGS(tmpreg, GET_HIGH_REG(reg)); \
	}

# define SPLIT_STORE_AND_CLOSE(type, reg, offset) \
	if (IS_2_WORD_TYPE(type) && GET_HIGH_REG(reg)==3) { \
		/*dolog("SPLIT_STORE({R%d;R%d} to [%x])", GET_LOW_REG(reg), GET_HIGH_REG(reg), offset);*/ \
		M_STR(GET_LOW_REG(reg), REG_SP, 4 * (offset)); \
		(reg) = PACK_REGS(REG_SPLIT, GET_HIGH_REG(reg)); \
	}

#endif


/******************************************************************************/
/* checking macros ************************************************************/
/******************************************************************************/

#define MCODECHECK(icnt) \
    do { \
        if ((cd->mcodeptr + (icnt) * 4) > cd->mcodeend) \
            codegen_increase(cd); \
    } while (0)

#define ALIGNCODENOP /* empty */

/* TODO: correct this! */
#define IS_IMM(val) ( ((val) >= 0) && ((val) <= 255) )
#define IS_OFFSET(off,max) ((s4)(off) <= (max) && (s4)(off) >= -(max))

#define CHECK_INT_REG(r)              assert((r)>=0 && (r)<=15)
#define CHECK_FLT_REG(r)              assert((r)>=0 && (r)<=15)
#define CHECK_OFFSET(off,max)         assert(IS_OFFSET(off,max))


/* branch defines *************************************************************/

#define BRANCH_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* patcher defines ************************************************************/

#define PATCHER_CALL_SIZE    1 * 4      /* an instruction is 4-bytes long     */

#define PATCHER_NOPS \
    do { \
        M_NOP; \
    } while (0)


/* lazy debugger **************************************************************/

#if !defined(NDEBUG)
void asm_debug(int a1, int a2, int a3, int a4);
void asm_debug_intern(int a1, int a2, int a3, int a4);

/* if called with this macros, it can be placed nearly anywhere */
/* almost all registers are saved and restored afterwards       */
/* it uses a long branch to call the asm_debug_intern (no exit) */
#define ASM_DEBUG_PREPARE \
	M_STMFD(0x7fff, REG_SP)
#define ASM_DEBUG_EXECUTE \
	M_LONGBRANCH(asm_debug_intern); \
	M_LDMFD(0x7fff, REG_SP)
#endif


/******************************************************************************/
/* macros to create code ******************************************************/
/******************************************************************************/

/* the condition field */
#define COND_EQ 0x0  /* Equal        Z set   */
#define COND_NE 0x1  /* Not equal    Z clear */
#define COND_CS 0x2  /* Carry set    C set   */
#define COND_CC 0x3  /* Carry clear  C clear */
#define COND_MI 0x4  /* Negative     N set   */
#define COND_PL 0x5  /* Positive     N clear */
#define COND_VS 0x6  /* Overflow     V set   */
#define COND_VC 0x7  /* No overflow  V clear */
#define COND_HI 0x8  /* Unsigned higher      */
#define COND_LS 0x9  /* Unsigned lower, same */
#define COND_GE 0xA  /* Sig. greater, equal  */
#define COND_LT 0xB  /* Sig. less than       */
#define COND_GT 0xC  /* Sig. greater than    */
#define COND_LE 0xD  /* Sig. less, equal     */
#define COND_AL 0xE  /* Always               */
#define CONDNV  0xF  /* Special (see A3-5)   */
#define UNCOND COND_AL

/* data processing operation: M_DAT
   cond ... conditional execution
   op ..... opcode
   d ...... destination reg
   n ...... source reg
   S ...... update condition codes
   I ...... switch to immediate mode
   shift .. shifter operand
*/

#define M_DAT(cond,op,d,n,S,I,shift) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | ((op) << 21) | ((d) << 12) | ((n) << 16) | ((I) << 25) | ((S) << 20) | ((shift) & 0x00000fff)); \
        cd->mcodeptr += 4; \
    } while (0)


/* load and store instruction: M_MEM
   cond ... conditional execution
   L ...... load (L=1) or store (L=0)
   B ...... unsigned byte (B=1) or word (B=0)
   d ...... destination reg
   n ...... base reg for addressing
   adr .... addressing mode specific
*/

#define M_MEM(cond,L,B,d,n,adr,I,P,U,W) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (1 << 26) | ((L) << 20) | ((B) << 22) | ((d) << 12) | ((n) << 16) | ((adr) & 0x0fff) | ((I) << 25) | ((P) << 24) | ((U) << 23) | ((W) << 21)); \
        cd->mcodeptr += 4; \
    } while (0)

#define M_MEM_GET_Rd(mcode)    (((mcode) >> 12) & 0x0f)
#define M_MEM_GET_Rbase(mcode) (((mcode) >> 16) & 0x0f)


/* load and store instruction: M_MEM2
   cond ... conditional execution
   L ...... load (L=1) or store (L=0)
   H ...... halfword (H=1) or signed byte (H=0)
   S ...... signed (S=1) or unsigned (S=0) halfword
   d ...... destination reg
   n ...... base reg for addressing
   adr .... addressing mode specific
*/

#define M_MEM2(cond,L,H,S,d,n,adr,I,P,U,W) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (1 << 22) | (0x9 << 4) | ((L) << 20) | ((H) << 5) | ((S) << 6) | ((d) << 12) | ((n) << 16) | ((adr) & 0x0f) | (((adr) & 0xf0) << (8-4)) | ((I) << 22) | ((P) << 24) | ((U) << 23) | ((W) << 21)); \
        cd->mcodeptr += 4; \
    } while (0)


/* load and store multiple instruction: M_MEM_MULTI
   cond ... conditional execution
   L ...... load (L=1) or store (L=0)
   S ...... special (see "The ARM ARM A3-21")
   regs ... register list
   n ...... base reg for addressing
*/

#define M_MEM_MULTI(cond,L,S,regs,n,P,U,W) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (1 << 27) | ((L) << 20) | ((S) << 22) | ((n) << 16) | ((regs) & 0xffff) | ((P) << 24) | ((U) << 23) | ((W) << 21)); \
        cd->mcodeptr += 4; \
    } while (0)


/* branch and branch with link X (instruction set exchange)
   cond ... conditional execution
   L ...... branch with link (L=1)
   reg .... register
*/

#define M_BRAX(cond,L,reg) \
    do { \
		*((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x12 << 20) | (0xfff << 8) | (L << 5) | (1 << 4) | ((reg) & 0xf)); \
		cd->mcodeptr += 4; \
    } while (0)


/* branch and branch with link: M_BRA
   cond ... conditional execution
   L ...... branch with link (L=1)
   offset . 24bit offset
*/

#define M_BRA(cond,L,offset) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x5 << 25) | ((L) << 24) | ((offset) & 0x00ffffff)); \
        cd->mcodeptr += 4; \
    } while (0)


/* multiplies: M_MULT
   cond ... conditional execution
   d ...... destination register
   n, m ... source registers
   S ...... update conditional codes
   A ...... accumulate flag (enables third source)
   s ...... third source register
*/

#define M_MULT(cond,d,n,m,S,A,s) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | ((d) << 16) | ((n) << 8) | (m) | (0x09 << 4) | ((S) << 20) | ((A) << 21) | ((s) << 12)); \
        cd->mcodeptr += 4; \
    } while (0)


/* no operation (mov r0,r0): M_NOP */

#define M_NOP \
    do { \
        *((u4 *) cd->mcodeptr) = (0xe1a00000); \
        cd->mcodeptr += 4; \
    } while (0)


/* software breakpoint (only v5 and above): M_BREAKPOINT */

#define M_BREAKPOINT(imm) \
    do { \
        *((u4 *) cd->mcodeptr) = (0x0e12 << 20) | (0x07 << 4) | (((imm) & 0xfff0) << (8-4)) | ((imm) & 0x0f); \
        cd->mcodeptr += 4; \
    } while (0)


/* undefined instruction used for hardware exceptions */

#define M_UNDEFINED(cond,imm,n) \
	do { \
		*((u4 *) cd->mcodeptr) = ((cond) << 28) | (0x7f << 20) | (((imm) & 0x0fff) << 8) | (0x0f << 4) | (n); \
		cd->mcodeptr += 4; \
	} while (0)


#if !defined(ENABLE_SOFTFLOAT)

/* M_CPDO **********************************************************************

   Floating-Point Coprocessor Data Operations

   cond ... conditional execution
   op ..... opcode
   D ...... dyadic (D=0) or monadic (D=1) instruction
   Fd ..... destination float-register
   Fn ..... source float-register
   Fm ..... source float-register or immediate

*******************************************************************************/

#define M_CPDOS(cond,op,D,Fd,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | ((op) << 20) | ((D) << 15) | ((Fd) << 12) | ((Fn) << 16) | ((Fm) & 0x0f)); \
        cd->mcodeptr += 4; \
    } while (0)


#define M_CPDOD(cond,op,D,Fd,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | ((op) << 20) | ((D) << 15) | ((Fd) << 12) | ((Fn) << 16) | ((Fm) & 0x0f) | (1 << 7)); \
        cd->mcodeptr += 4; \
    } while (0)


#define M_CPDP(cond,p,q,r,s,cp_num,D,N,M,Fd,Fn,Fm) \
	do { \
		*((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | ((p) << 23) | ((q) << 21) | ((r) << 20) | ((s) << 6) | ((cp_num) << 8) | ((D) << 22) | ((N) << 7) | ((M) << 5) | ((Fd) << 12) | ((Fn) << 16) | ((Fm) & 0x0f)); \
		cd->mcodeptr += 4; \
	} while (0)

#define M_CPDPF(cond,p,q,r,s,cp_num,Fd,Fn,Fm) \
	M_CPDP(cond,p,q,r,s,cp_num,(Fd)>>4,(Fn)>>4,(Fm)>>4,(Fd)&0xf,(Fn)&0xf,Fm)

/* M_CPDT **********************************************************************

   Floating-Point Coprocessor Data Transfer

   cond ... conditional execution
   L ...... load (L=1) or store (L=0)
   Fd ..... destination float-register
   n ...... base reg for addressing

*******************************************************************************/

#define M_CPDT(cond,L,T1,T0,Fd,n,off,P,U,W) \
	do { \
		*((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0c << 24) | (1 << 8) | ((L) << 20) | ((T1) << 22) | ((T0) << 15) | ((Fd) << 12) | ((n) << 16) | ((off) & 0xff) | ((P) << 24) | ((U) << 23) | ((W) << 21)); \
		cd->mcodeptr += 4; \
	} while (0)

#define M_CPLS(cond,L,P,U,W,cp_num,D,Fd,n,off) \
	do { \
		*((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0c << 24) | ((P) << 24) | ((U) << 23) | ((W) << 21) | ((L) << 20) | ((cp_num) << 8) | ((D) << 22) | ((Fd) << 12) | ((n) << 16) | ((off) & 0xff)); \
		cd->mcodeptr += 4; \
	} while (0)


/* M_CPRT **********************************************************************

   Floating-Point Coprocessor Register Transfer

   XXX

*******************************************************************************/

#define M_CPRT(cond,op,L,cp_num,N,Fn,n) \
	do { \
		*((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 4) | ((op) << 21) | ((L) << 20) | ((cp_num) << 8) | ((N) << 7) | ((Fn) << 16) | ((n) << 12)); \
		cd->mcodeptr += 4; \
	} while (0)

#define M_CPRTS(cond,L,d,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | (1 << 4) | ((L) << 20) | ((d) << 12) | ((Fn) << 16) | (Fm)); \
        cd->mcodeptr += 4; \
    } while (0)


#define M_CPRTD(cond,L,d,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | (1 << 4) | ((L) << 20) | ((d) << 12) | ((Fn) << 16) | (Fm) | (1 << 7)); \
        cd->mcodeptr += 4; \
    } while (0)


#define M_CPRTI(cond,L,d,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | (1 << 4) | ((L) << 20) | ((d) << 12) | ((Fn) << 16) | (Fm) | (3 << 5)); \
        cd->mcodeptr += 4; \
    } while (0)


/* XXX TWISTI: replace X by something useful */

#define M_CPRTX(cond,L,d,Fn,Fm) \
    do { \
        *((u4 *) cd->mcodeptr) = (((cond) << 28) | (0x0e << 24) | (1 << 8) | (1 << 4) | ((L) << 20) | ((d) << 12) | ((Fn) << 16) | (Fm) | (1 << 23)); \
        cd->mcodeptr += 4; \
    } while (0)

#endif /* !defined(ENABLE_SOFTFLOAT) */


/* used to store values! */
#define DCD(val) \
    do { \
        *((u4 *) cd->mcodeptr) = val; \
        cd->mcodeptr += 4; \
    } while (0)


/* used to directly access shifter; insert this as shifter operand! */
#define REG_LSL(reg, shift) ( (((shift) & 0x1f) << 7) | ((reg) & 0x0f) )
#define REG_LSR(reg, shift) ( (((shift) & 0x1f) << 7) | ((reg) & 0x0f) | (1 << 5) )
#define REG_ASR(reg, shift) ( (((shift) & 0x1f) << 7) | ((reg) & 0x0f) | (1 << 6) )
#define REG_LSL_REG(reg, s) ( (((s) & 0x0f) << 8) | ((reg) & 0x0f) | (1 << 4) )
#define REG_LSR_REG(reg, s) ( (((s) & 0x0f) << 8) | ((reg) & 0x0f) | (1 << 4) | (1 << 5) )
#define REG_ASR_REG(reg, s) ( (((s) & 0x0f) << 8) | ((reg) & 0x0f) | (1 << 4) | (1 << 6) )

/* used to directly rotate immediate values; insert this as immediate! */
/* ATTENTION: this rotates the immediate right by (2 * rot) bits */
#define IMM_ROTR(imm, rot) ( ((imm) & 0xff) | (((rot) & 0x0f) << 8) )
#define IMM_ROTL(imm, rot) IMM_ROTR(imm, 16-(rot))


/******************************************************************************/
/* macros for all basic arm instructions **************************************/
/******************************************************************************/

#define M_ADD(d,a,b)       M_DAT(UNCOND,0x04,d,a,0,0,b)         /* d = a +  b */
#define M_ADC(d,a,b)       M_DAT(UNCOND,0x05,d,a,0,0,b)         /* d = a +  b (with Carry) */
#define M_SUB(d,a,b)       M_DAT(UNCOND,0x02,d,a,0,0,b)         /* d = a -  b */
#define M_SBC(d,a,b)       M_DAT(UNCOND,0x06,d,a,0,0,b)         /* d = a -  b (with Carry) */
#define M_AND(a,b,d)       M_DAT(UNCOND,0x00,d,a,0,0,b)         /* d = a &  b */
#define M_ORR(a,b,d)       M_DAT(UNCOND,0x0c,d,a,0,0,b)         /* d = a |  b */
#define M_EOR(a,b,d)       M_DAT(UNCOND,0x01,d,a,0,0,b)         /* d = a ^  b */
#define M_TST(a,b)         M_DAT(UNCOND,0x08,0,a,1,0,b)         /* TST a &  b */
#define M_TEQ(a,b)         M_DAT(UNCOND,0x09,0,a,1,0,b)         /* TST a ^  b */
#define M_CMP(a,b)         M_DAT(UNCOND,0x0a,0,a,1,0,b)         /* TST a -  b */
#define M_MOV(d,b)         M_DAT(UNCOND,0x0d,d,0,0,0,b)         /* d =      b */
#define M_ADD_S(d,a,b)     M_DAT(UNCOND,0x04,d,a,1,0,b)         /* d = a +  b (update Flags) */
#define M_SUB_S(d,a,b)     M_DAT(UNCOND,0x02,d,a,1,0,b)         /* d = a -  b (update Flags) */
#define M_ORR_S(a,b,d)     M_DAT(UNCOND,0x0c,d,a,1,0,b)         /* d = a |  b (update flags) */
#define M_MOV_S(d,b)       M_DAT(UNCOND,0x0d,d,0,1,0,b)         /* d =      b (update Flags) */

#define M_ADD_IMM(d,a,i)   M_DAT(UNCOND,0x04,d,a,0,1,i)         /* d = a +  i */
#define M_ADC_IMM(d,a,i)   M_DAT(UNCOND,0x05,d,a,0,1,i)         /* d = a +  i (with Carry) */
#define M_SUB_IMM(d,a,i)   M_DAT(UNCOND,0x02,d,a,0,1,i)         /* d = a -  i */
#define M_SBC_IMM(d,a,i)   M_DAT(UNCOND,0x06,d,a,0,1,i)         /* d = a -  i (with Carry) */
#define M_RSB_IMM(d,a,i)   M_DAT(UNCOND,0x03,d,a,0,1,i)         /* d = -a + i */
#define M_RSC_IMM(d,a,i)   M_DAT(UNCOND,0x07,d,a,0,1,i)         /* d = -a + i (with Carry) */
#define M_AND_IMM(a,i,d)   M_DAT(UNCOND,0x00,d,a,0,1,i)         /* d = a &  i */
#define M_TST_IMM(a,i)     M_DAT(UNCOND,0x08,0,a,1,1,i)         /* TST a &  i */
#define M_TEQ_IMM(a,i)     M_DAT(UNCOND,0x09,0,a,1,1,i)         /* TST a ^  i */
#define M_CMP_IMM(a,i)     M_DAT(UNCOND,0x0a,0,a,1,1,i)         /* TST a -  i */
#define M_CMN_IMM(a,i)     M_DAT(UNCOND,0x0b,0,a,1,1,i)         /* TST a +  i */
#define M_MOV_IMM(d,i)     M_DAT(UNCOND,0x0d,d,0,0,1,i)         /* d =      i */
#define M_ADD_IMMS(d,a,i)  M_DAT(UNCOND,0x04,d,a,1,1,i)         /* d = a +  i (update Flags) */
#define M_SUB_IMMS(d,a,i)  M_DAT(UNCOND,0x02,d,a,1,1,i)         /* d = a -  i (update Flags) */
#define M_RSB_IMMS(d,a,i)  M_DAT(UNCOND,0x03,d,a,1,1,i)         /* d = -a + i (update Flags) */

#define M_ADDSUB_IMM(d,a,i) if((i)>=0) M_ADD_IMM(d,a,i); else M_SUB_IMM(d,a,-(i))
#define M_MOVEQ(a,d)       M_DAT(COND_EQ,0x0d,d,0,0,0,a)
#define M_EORLE(d,a,b)     M_DAT(COND_LE,0x01,d,a,0,0,b)

#define M_MOVVS_IMM(i,d)   M_DAT(COND_VS,0x0d,d,0,0,1,i)
#define M_MOVEQ_IMM(i,d)   M_DAT(COND_EQ,0x0d,d,0,0,1,i)
#define M_MOVNE_IMM(i,d)   M_DAT(COND_NE,0x0d,d,0,0,1,i)
#define M_MOVLT_IMM(i,d)   M_DAT(COND_LT,0x0d,d,0,0,1,i)
#define M_MOVGT_IMM(i,d)   M_DAT(COND_GT,0x0d,d,0,0,1,i)
#define M_MOVLS_IMM(i,d)   M_DAT(COND_LS,0x0d,d,0,0,1,i)

#define M_ADDHI_IMM(d,a,i) M_DAT(COND_HI,0x04,d,a,0,1,i)
#define M_ADDLT_IMM(d,a,i) M_DAT(COND_LT,0x04,d,a,0,1,i)
#define M_ADDGT_IMM(d,a,i) M_DAT(COND_GT,0x04,d,a,0,1,i)
#define M_SUBLO_IMM(d,a,i) M_DAT(COND_CC,0x02,d,a,0,1,i)
#define M_SUBLT_IMM(d,a,i) M_DAT(COND_LT,0x02,d,a,0,1,i)
#define M_SUBGT_IMM(d,a,i) M_DAT(COND_GT,0x02,d,a,0,1,i)
#define M_RSBMI_IMM(d,a,i) M_DAT(COND_MI,0x03,d,a,0,1,i)
#define M_ADCMI_IMM(d,a,i) M_DAT(COND_MI,0x05,d,a,0,1,i)

#define M_CMPEQ(a,b)       M_DAT(COND_EQ,0x0a,0,a,1,0,b)        /* TST a -  b */
#define M_CMPLE(a,b)       M_DAT(COND_LE,0x0a,0,a,1,0,b)        /* TST a -  b */

#define M_CMPEQ_IMM(a,i)   M_DAT(COND_EQ,0x0a,0,a,1,1,i)

#define M_MUL(d,a,b)       M_MULT(UNCOND,d,a,b,0,0,0x0)         /* d = a *  b */

#define M_B(off)           M_BRA(UNCOND,0,off)    /* unconditional branch */
#define M_BL(off)          M_BRA(UNCOND,1,off)    /* branch and link      */
#define M_BEQ(off)         M_BRA(COND_EQ,0,off)   /* conditional branches */
#define M_BNE(off)         M_BRA(COND_NE,0,off)
#define M_BGE(off)         M_BRA(COND_GE,0,off)
#define M_BGT(off)         M_BRA(COND_GT,0,off)
#define M_BLT(off)         M_BRA(COND_LT,0,off)
#define M_BLE(off)         M_BRA(COND_LE,0,off)
#define M_BHI(off)         M_BRA(COND_HI,0,off)   /* unsigned conditional */
#define M_BHS(off)         M_BRA(COND_CS,0,off)
#define M_BLO(off)         M_BRA(COND_CC,0,off)
#define M_BLS(off)         M_BRA(COND_LS,0,off)

#define M_BX(a)            M_BRAX(COND_AL,0,a)
#define M_BLX(a)           M_BRAX(COND_AL,1,a)


/******************************************************************************/
/* macros for load and store instructions *************************************/
/******************************************************************************/

#define M_LDMFD(regs,base) M_MEM_MULTI(UNCOND,1,0,regs,base,0,1,1)
#define M_STMFD(regs,base) M_MEM_MULTI(UNCOND,0,0,regs,base,1,0,1)

#define M_LDR_REG(d,base,offreg) M_MEM(UNCOND,1,0,d,base,offreg,1,1,1,0)
#define M_STR_REG(d,base,offreg) M_MEM(UNCOND,0,0,d,base,offreg,1,1,1,0)

#define M_LDR_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x0fff); \
        M_MEM(UNCOND,1,0,d,base,(((off) < 0) ? -(off) : off),0,1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_STR_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x0fff); \
        M_MEM(UNCOND,0,0,d,base,(((off) < 0) ? -(off) : off),0,1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_LDR_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x0fff); \
        M_MEM(UNCOND,1,0,d,base,(((off) < 0) ? -(off) : off),0,0,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_STR_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off,0x0fff); \
        M_MEM(UNCOND,0,0,d,base,(((off) < 0) ? -(off) : off),0,1,(((off) < 0) ? 0 : 1),1); \
    } while (0)


#define M_LDRH(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x00ff); \
        assert(off >= 0); \
        M_MEM2(UNCOND,1,1,0,d,base,off,1,1,1,0); \
    } while (0)

#define M_LDRSH(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x00ff); \
        assert(off >= 0); \
        M_MEM2(UNCOND,1,1,1,d,base,off,1,1,1,0); \
    } while (0)

#define M_LDRSB(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x00ff); \
        assert(off >= 0); \
        M_MEM2(UNCOND,1,0,1,d,base,off,1,1,1,0); \
    } while (0)

#define M_STRH(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x00ff); \
        assert(off >= 0); \
        M_MEM2(UNCOND,0,1,0,d,base,off,1,1,1,0); \
    } while (0)

#define M_STRB(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x0fff); \
        assert(off >= 0); \
        M_MEM(UNCOND,0,1,d,base,off,0,1,1,0); \
    } while (0)


#define M_TRAP(a,i)        M_UNDEFINED(UNCOND,i,a);
#define M_TRAPEQ(a,i)      M_UNDEFINED(COND_EQ,i,a);
#define M_TRAPNE(a,i)      M_UNDEFINED(COND_NE,i,a);
#define M_TRAPLT(a,i)      M_UNDEFINED(COND_LT,i,a);
#define M_TRAPLE(a,i)      M_UNDEFINED(COND_LE,i,a);
#define M_TRAPHI(a,i)      M_UNDEFINED(COND_HI,i,a);
#define M_TRAPHS(a,i)      M_UNDEFINED(COND_CS,i,a);


/* if we do not have double-word load/store command, we can fake them */
/* ATTENTION: the original LDRD/STRD of ARMv5e would always use (Rd/Rd+1),
   so these faked versions are more "powerful" */

#if defined(__ARMEL__)

#define M_LDRD_INTERN(d,base,off) \
    do { \
        M_LDR_INTERN(GET_LOW_REG(d), base, off); \
        M_LDR_INTERN(GET_HIGH_REG(d), base, (off) + 4); \
    } while (0)

#define M_STRD_INTERN(d,base,off) \
    do { \
        M_STR_INTERN(GET_LOW_REG(d), base, off); \
        M_STR_INTERN(GET_HIGH_REG(d), base, (off) + 4); \
    } while (0)

#define M_LDRD_ALTERN(d,base,off) \
    do { \
        M_LDR_INTERN(GET_HIGH_REG(d), base, (off) + 4); \
        M_LDR_INTERN(GET_LOW_REG(d), base, off); \
    } while (0)

#define M_LDRD_UPDATE(d,base,off) \
    do { \
        assert((off) == +8); \
        M_LDR_UPDATE(GET_LOW_REG(d), base, 4); \
        M_LDR_UPDATE(GET_HIGH_REG(d), base, 4); \
    } while (0)

#define M_STRD_UPDATE(d,base,off) \
    do { \
        assert((off) == -8); \
        M_STR_UPDATE(GET_HIGH_REG(d), base, -4); \
        M_STR_UPDATE(GET_LOW_REG(d), base, -4); \
    } while (0)

#define GET_FIRST_REG(d)  GET_LOW_REG(d)
#define GET_SECOND_REG(d) GET_HIGH_REG(d)

#else /* defined(__ARMEB__) */

#define M_LDRD_INTERN(d,base,off) \
    do { \
        M_LDR_INTERN(GET_HIGH_REG(d), base, off); \
        M_LDR_INTERN(GET_LOW_REG(d), base, (off) + 4); \
    } while (0)

#define M_STRD_INTERN(d,base,off) \
    do { \
        M_STR_INTERN(GET_HIGH_REG(d), base, off); \
        M_STR_INTERN(GET_LOW_REG(d), base, (off) + 4); \
    } while (0)

#define M_LDRD_ALTERN(d,base,off) \
    do { \
        M_LDR_INTERN(GET_LOW_REG(d), base, (off) + 4); \
        M_LDR_INTERN(GET_HIGH_REG(d), base, off); \
    } while (0)

#define M_LDRD_UPDATE(d,base,off) \
    do { \
        assert((off) == +8); \
        M_LDR_UPDATE(GET_HIGH_REG(d), base, 4); \
        M_LDR_UPDATE(GET_LOW_REG(d), base, 4); \
    } while (0)

#define M_STRD_UPDATE(d,base,off) \
    do { \
        assert((off) == -8); \
        M_STR_UPDATE(GET_LOW_REG(d), base, -4); \
        M_STR_UPDATE(GET_HIGH_REG(d) ,base, -4); \
    } while (0)

#define GET_FIRST_REG(d)  GET_HIGH_REG(d)
#define GET_SECOND_REG(d) GET_LOW_REG(d)

#endif /* defined(__ARMEB__) */


/******************************************************************************/
/* macros for all floating point instructions *********************************/
/******************************************************************************/

#if !defined(ENABLE_SOFTFLOAT)

#if defined(__VFP_FP__)

#define M_FADD(a,b,d)      M_CPDPF(UNCOND,0,1,1,0,10,d,a,b)/* d = a +  b */
#define M_FSUB(a,b,d)      M_CPDPF(UNCOND,0,1,1,1,10,d,a,b)/* d = a -  b */
#define M_FMUL(a,b,d)      M_CPDPF(UNCOND,0,1,0,0,10,d,a,b)/* d = a *  b */
#define M_FDIV(a,b,d)      M_CPDPF(UNCOND,1,0,0,0,10,d,a,b)/* d = a /  b */
#define M_DADD(a,b,d)      M_CPDP(UNCOND,0,1,1,0,11,0,0,0,d,a,b)/* d = a +  b */
#define M_DSUB(a,b,d)      M_CPDP(UNCOND,0,1,1,1,11,0,0,0,d,a,b)/* d = a -  b */
#define M_DMUL(a,b,d)      M_CPDP(UNCOND,0,1,0,0,11,0,0,0,d,a,b)/* d = a *  b */
#define M_DDIV(a,b,d)      M_CPDP(UNCOND,1,0,0,0,11,0,0,0,d,a,b)/* d = a /  b */

#define M_FMOV(a,d)        M_CPDPF(UNCOND,1,1,1,1,10,d,0x0,a)
#define M_DMOV(a,d)        M_CPDP(UNCOND,1,1,1,1,11,0,0,0,d,0x0,a)
#define M_FNEG(a,d)        M_CPDPF(UNCOND,1,1,1,1,10,d,0x1,a)
#define M_DNEG(a,d)        M_CPDP(UNCOND,1,1,1,1,11,0,0,0,d,0x1,a)

#define M_FCMP(a,b)        M_CPDPF(UNCOND,1,1,1,1,10,a,0x4,b)
#define M_DCMP(a,b)        M_CPDP(UNCOND,1,1,1,1,11,0,0,0,a,0x4,b)

#define M_CVTDF(a,d)       M_CPDP(UNCOND,1,1,1,1,11,(d)>>4,1,0,(d)&0xf,0x7,a)
#define M_CVTFD(a,d)       M_CPDP(UNCOND,1,1,1,1,10,0,1,0,d,0x7,a)
#define M_CVTIF(a,d)       M_CPDP(UNCOND,1,1,1,1,10,(d)>>4,1,0,(d)&0xf,0x8,a)
#define M_CVTID(a,d)       M_CPDP(UNCOND,1,1,1,1,11,0,1,0,d,0x8,a)
#define M_CVTFI(a,d)       M_CPDP(UNCOND,1,1,1,1,10,0,1,(a)>>4,d,0xd,a) // ftosis
#define M_CVTDI(a,d)       M_CPDP(UNCOND,1,1,1,1,11,0,1,0,d,0xd,a) // ftosid

#define M_FMSTAT           M_CPRT(UNCOND,0x07,1,10,0,0x1,0xf)

#define M_FMSR(a,Fb)       M_CPRT(UNCOND,0x00,0,10,0,Fb,a)
#define M_FMRS(Fa,b)       M_CPRT(UNCOND,0x00,1,10,0,Fa,b)
#define M_FMDLR(a,Fb)      M_CPRT(UNCOND,0x00,0,11,0,Fb,a)
#define M_FMRDL(Fa,b)      M_CPRT(UNCOND,0x00,1,11,0,Fa,b)
#define M_FMDHR(a,Fb)      M_CPRT(UNCOND,0x01,0,11,0,Fb,a)
#define M_FMRDH(Fa,b)      M_CPRT(UNCOND,0x01,1,11,0,Fa,b)

#else

#define M_FADD(a,b,d)      M_CPDOS(UNCOND,0x00,0,d,a,b)         /* d = a +  b */
#define M_FSUB(a,b,d)      M_CPDOS(UNCOND,0x02,0,d,a,b)         /* d = a -  b */
#define M_FMUL(a,b,d)      M_CPDOS(UNCOND,0x01,0,d,a,b)         /* d = a *  b */
#define M_FDIV(a,b,d)      M_CPDOS(UNCOND,0x04,0,d,a,b)         /* d = a /  b */
#define M_RMFS(d,a,b)      M_CPDOS(UNCOND,0x08,0,d,a,b)         /* d = a %  b */
#define M_DADD(a,b,d)      M_CPDOD(UNCOND,0x00,0,d,a,b)         /* d = a +  b */
#define M_DSUB(a,b,d)      M_CPDOD(UNCOND,0x02,0,d,a,b)         /* d = a -  b */
#define M_DMUL(a,b,d)      M_CPDOD(UNCOND,0x01,0,d,a,b)         /* d = a *  b */
#define M_DDIV(a,b,d)      M_CPDOD(UNCOND,0x04,0,d,a,b)         /* d = a /  b */
#define M_RMFD(d,a,b)      M_CPDOD(UNCOND,0x08,0,d,a,b)         /* d = a %  b */

#define M_FMOV(a,d)        M_CPDOS(UNCOND,0x00,1,d,0,a)         /* d =      a */
#define M_DMOV(a,d)        M_CPDOD(UNCOND,0x00,1,d,0,a)         /* d =      a */
#define M_FNEG(a,d)        M_CPDOS(UNCOND,0x01,1,d,0,a)         /* d =    - a */
#define M_DNEG(a,d)        M_CPDOD(UNCOND,0x01,1,d,0,a)         /* d =    - a */

#define M_FCMP(a,b)        M_CPRTX(UNCOND,1,0x0f,a,b)           /* COMPARE a;  b */
#define M_DCMP(a,b)        M_CPRTX(UNCOND,1,0x0f,a,b)           /* COMPARE a;  b */

#define M_CVTDF(a,b)       M_FMOV(a,b)
#define M_CVTFD(a,b)       M_DMOV(a,b)
#define M_CVTIF(a,d)       M_CPRTS(UNCOND,0,a,d,0)              /* d = (float) a */
#define M_CVTID(a,d)       M_CPRTD(UNCOND,0,a,d,0)              /* d = (float) a */
#define M_CVTFI(a,d)       M_CPRTI(UNCOND,1,d,0,a)              /* d = (int)   a */
#define M_CVTDI(a,d)       M_CPRTI(UNCOND,1,d,0,a)              /* d = (int)   a */

#endif


/* M_CAST_x2x:
   loads the value of the integer-register a (argument or result) into
   float-register Fb. (and vice versa)
*/

#if defined(__VFP_FP__)

#define M_CAST_I2F(a,Fb) M_FMSR(a,Fb)

#define M_CAST_F2I(Fa,b) M_FMRS(Fa,b)

#define M_CAST_L2D(a,Fb) \
	do { \
		M_FMDLR(GET_LOW_REG(a), Fb); \
		M_FMDHR(GET_HIGH_REG(a), Fb); \
	} while (0)

#define M_CAST_D2L(Fa,b) \
	do { \
		M_FMRDL(Fa, GET_LOW_REG(b)); \
		M_FMRDH(Fa, GET_HIGH_REG(b)); \
	} while (0)

#else

#define M_CAST_I2F(a,Fb) \
	do { \
		CHECK_FLT_REG(Fb); \
		CHECK_INT_REG(a); \
		M_STR_UPDATE(a, REG_SP, -4); \
		M_FLD_UPDATE(Fb, REG_SP, 4); \
	} while (0)

#define M_CAST_L2D(a,Fb) \
	do { \
		CHECK_FLT_REG(Fb); \
		CHECK_INT_REG(GET_LOW_REG(a)); \
		CHECK_INT_REG(GET_HIGH_REG(a)); \
		M_STRD_UPDATE(a, REG_SP, -8); \
		M_DLD_UPDATE(Fb, REG_SP, 8); \
	} while (0)

#define M_CAST_F2I(Fa,b) \
	do { \
		CHECK_FLT_REG(Fa); \
		CHECK_INT_REG(b); \
		M_FST_UPDATE(Fa, REG_SP, -4); \
		M_LDR_UPDATE(b, REG_SP, 4); \
	} while (0)

#define M_CAST_D2L(Fa,b) \
	do { \
		CHECK_INT_REG(GET_LOW_REG(b)); \
		CHECK_INT_REG(GET_HIGH_REG(b)); \
		M_DST_UPDATE(Fa, REG_SP, -8); \
		M_LDRD_UPDATE(b, REG_SP, 8); \
	} while (0)

#endif

/* M_xLD_xx & M_xST_xx:
   XXX document me!
*/

#if defined(__VFP_FP__)

#define M_FLD_INTERN(d,base,off) \
	do {						   \
		CHECK_OFFSET(off, 0x03ff);										\
		M_CPLS(UNCOND,1,1,(((off) < 0) ? 0 : 1),0,10,(d)>>4,(d)&0xf,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2)); \
	} while (0)

#define M_DLD_INTERN(d,base,off) \
	do {						   \
		CHECK_OFFSET(off, 0x03ff);										\
		M_CPLS(UNCOND,1,1,(((off) < 0) ? 0 : 1),0,11,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2)); \
	} while (0)

#define M_FST_INTERN(d,base,off) \
	do {						   \
		CHECK_OFFSET(off, 0x03ff);										\
		M_CPLS(UNCOND,0,1,(((off) < 0) ? 0 : 1),0,10,(d)>>4,(d)&0xf,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2)); \
	} while (0)

#define M_DST_INTERN(d,base,off) \
	do {						   \
		CHECK_OFFSET(off, 0x03ff);										\
		M_CPLS(UNCOND,0,1,(((off) < 0) ? 0 : 1),0,11,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2)); \
	} while (0)

#else

#define M_FLD_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,1,0,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_DLD_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,1,0,1,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_FST_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,0,0,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_DST_INTERN(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,0,0,1,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),0); \
    } while (0)

#define M_FLD_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,1,0,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),0,(((off) < 0) ? 0 : 1),1); \
    } while (0)

#define M_DLD_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,1,0,1,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),0,(((off) < 0) ? 0 : 1),1); \
    } while (0)

#define M_FST_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,0,0,0,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),1); \
    } while (0)

#define M_DST_UPDATE(d,base,off) \
    do { \
        CHECK_OFFSET(off, 0x03ff); \
        M_CPDT(UNCOND,0,0,1,d,base,(((off) < 0) ? -(off) >> 2 : (off) >> 2),1,(((off) < 0) ? 0 : 1),1); \
    } while (0)

#endif

#endif /* !defined(ENABLE_SOFTFLOAT) */


/******************************************************************************/
/* wrapper macros for load and store instructions *****************************/
/******************************************************************************/

/* M_LDR/M_STR:
   these are replacements for the original LDR/STR instructions, which can
   handle longer offsets (up to 20bits). the original functions are now
   called M_xxx_INTERN.
*/
/* ATTENTION: We use ITMP3 here, take into account that it gets destroyed.
   This means that only ITMP1 and ITMP2 can be used in reg_of_var()!!!
*/
/* ATTENTION2: It is possible to use ITMP3 as base reg. Remember that when
   changing these macros!!!
*/

#define M_LDR(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x0fffff); \
	if (IS_OFFSET(offset, 0x000fff)) { \
		M_LDR_INTERN(d, base, offset); \
	} else { \
		/* we cannot handle REG_PC here */ \
		assert((d) != REG_PC); \
		if ((offset) > 0) { \
			M_ADD_IMM(d, base, IMM_ROTL((offset) >> 12, 6)); \
			M_LDR_INTERN(d, d, (offset) & 0x000fff); \
		} else { \
			M_SUB_IMM(d, base, IMM_ROTL((-(offset)) >> 12, 6)); \
			M_LDR_INTERN(d, d, -(-(offset) & 0x000fff)); \
		} \
	} \
} while (0)

#define M_LDR_NEGATIVE(d, base, offset) { \
	/*assert((offset) <= 0);*/ \
	if (IS_OFFSET(offset, 0x000fff)) { \
		M_LDR_INTERN(d, base, offset); \
	} else { \
		/* we cannot handle REG_PC here */ \
		assert((d) != REG_PC); \
		M_SUB_IMM(d, base, IMM_ROTL((-(offset)) >> 12, 6)); \
		M_LDR_INTERN(d, d, -(-(offset) & 0x000fff)); \
	} \
}

#define M_LDRD(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x0fffff - 4); \
	if (IS_OFFSET(offset, 0x000fff - 4)) { \
		if (GET_FIRST_REG(d) != (base)) { \
			M_LDRD_INTERN(d, base, offset); \
		} else { \
			M_LDRD_ALTERN(d, base, offset); \
		} \
	} else if (IS_OFFSET(offset, 0x000fff)) { \
		dolog("M_LDRD: this offset seems to be complicated (%d)", offset); \
		assert(0); \
	} else { \
		if ((offset) > 0) { \
			M_ADD_IMM(GET_SECOND_REG(d), base, IMM_ROTL((offset) >> 12, 6)); \
			M_LDRD_INTERN(d, GET_SECOND_REG(d), (offset) & 0x000fff); \
		} else { \
			M_SUB_IMM(GET_SECOND_REG(d), base, IMM_ROTL((-(offset)) >> 12, 6)); \
			M_LDRD_INTERN(d, GET_SECOND_REG(d), -(-(offset) & 0x000fff)); \
		} \
	} \
} while (0)

#if !defined(ENABLE_SOFTFLOAT)

#define M_LDFS(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x03ffff); \
	if (IS_OFFSET(offset, 0x03ff)) { \
		M_FLD_INTERN(d, base, offset); \
	} else { \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 10, 5)); \
			M_FLD_INTERN(d, REG_ITMP3, (offset) & 0x03ff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 10, 5)); \
			M_FLD_INTERN(d, REG_ITMP3, -(-(offset) & 0x03ff)); \
		} \
	} \
} while (0)

#define M_LDFD(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x03ffff); \
	if (IS_OFFSET(offset, 0x03ff)) { \
		M_DLD_INTERN(d, base, offset); \
	} else { \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 10, 5)); \
			M_DLD_INTERN(d, REG_ITMP3, (offset) & 0x03ff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 10, 5)); \
			M_DLD_INTERN(d, REG_ITMP3, -(-(offset) & 0x03ff)); \
		} \
	} \
} while (0)

#endif /* !defined(ENABLE_SOFTFLOAT) */

#define M_STR(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x0fffff); \
	if (IS_OFFSET(offset, 0x000fff)) { \
		M_STR_INTERN(d, base, offset); \
	} else { \
		assert((d) != REG_ITMP3); \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 12, 6)); \
			M_STR_INTERN(d, REG_ITMP3, (offset) & 0x000fff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 12, 6)); \
			M_STR_INTERN(d, REG_ITMP3, -(-(offset) & 0x000fff)); \
		} \
	} \
} while (0)

#define M_STRD(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x0fffff - 4); \
	if (IS_OFFSET(offset, 0x000fff - 4)) { \
		M_STRD_INTERN(d,base,offset); \
	} else if (IS_OFFSET(offset, 0x000fff)) { \
		dolog("M_STRD: this offset seems to be complicated (%d)", offset); \
		assert(0); \
	} else { \
		assert(GET_LOW_REG(d) != REG_ITMP3); \
		assert(GET_HIGH_REG(d) != REG_ITMP3); \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 12, 6)); \
			M_STRD_INTERN(d, REG_ITMP3, (offset) & 0x000fff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 12, 6)); \
			M_STRD_INTERN(d, REG_ITMP3, -(-(offset) & 0x000fff)); \
		} \
	} \
} while (0)

#if !defined(ENABLE_SOFTFLOAT)

#define M_STFS(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x03ffff); \
	if (IS_OFFSET(offset, 0x03ff)) { \
		M_FST_INTERN(d, base, offset); \
	} else { \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 10, 5)); \
			M_FST_INTERN(d, REG_ITMP3, (offset) & 0x03ff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 10, 5)); \
			M_FST_INTERN(d, REG_ITMP3, -(-(offset) & 0x03ff)); \
		} \
	} \
} while (0)

#define M_STFD(d, base, offset) \
do { \
	CHECK_OFFSET(offset, 0x03ffff); \
	if (IS_OFFSET(offset, 0x03ff)) { \
		M_DST_INTERN(d, base, offset); \
	} else { \
		if ((offset) > 0) { \
			M_ADD_IMM(REG_ITMP3, base, IMM_ROTL((offset) >> 10, 5)); \
			M_DST_INTERN(d, REG_ITMP3, (offset) & 0x03ff); \
		} else { \
			M_SUB_IMM(REG_ITMP3, base, IMM_ROTL((-(offset)) >> 10, 5)); \
			M_DST_INTERN(d, REG_ITMP3, -(-(offset) & 0x03ff)); \
		} \
	} \
} while (0)

#endif /* !defined(ENABLE_SOFTFLOAT) */


/******************************************************************************/
/* additional helper macros ***************************************************/
/******************************************************************************/

/* M_???_IMM_EXT_MUL4:
   extended immediate operations, to handle immediates larger than 8bit.
   ATTENTION: the immediate is rotated left by 2 (multiplied by 4)!!!
*/

#define M_ADD_IMM_EXT_MUL4(d,n,imm) \
    do { \
        assert(d != REG_PC); \
        assert((imm) >= 0 && (imm) <= 0x00ffffff); \
        M_ADD_IMM(d, n, IMM_ROTL(imm, 1)); \
        if ((imm) > 0x000000ff) M_ADD_IMM(d, d, IMM_ROTL((imm) >>  8, 5)); \
        if ((imm) > 0x0000ffff) M_ADD_IMM(d, d, IMM_ROTL((imm) >> 16, 9)); \
    } while (0)

#define M_SUB_IMM_EXT_MUL4(d,n,imm) \
    do { \
        assert(d != REG_PC); \
        assert((imm) >= 0 && (imm) <= 0x00ffffff); \
        M_SUB_IMM(d, n, IMM_ROTL(imm, 1)); \
        if ((imm) > 0x000000ff) M_SUB_IMM(d, d, IMM_ROTL((imm) >>  8, 5)); \
        if ((imm) > 0x0000ffff) M_SUB_IMM(d, d, IMM_ROTL((imm) >> 16, 9)); \
    } while (0)


/* ICONST/LCONST:
   loads the integer/long value const into register d.
*/

#define ICONST(d,c)                     emit_iconst(cd, (d), (c))

#define LCONST(d,c) \
	if (IS_IMM((c) >> 32)) { \
		M_MOV_IMM(GET_HIGH_REG(d), (s4) ((s8) (c) >> 32)); \
		ICONST(GET_LOW_REG(d), (s4) ((s8) (c) & 0xffffffff)); \
	} else if (IS_IMM((c) & 0xffffffff)) { \
		M_MOV_IMM(GET_LOW_REG(d), (s4) ((s8) (c) & 0xffffffff)); \
		ICONST(GET_HIGH_REG(d), (s4) ((s8) (c) >> 32)); \
	} else { \
		disp = dseg_add_s8(cd, (c)); \
		M_LDRD(d, REG_PV, disp); \
	}


#if !defined(ENABLE_SOFTFLOAT)

#define FCONST(d,c) \
    do { \
        disp = dseg_add_float(cd, (c)); \
        M_LDFS(d, REG_PV, disp); \
    } while (0)

#define DCONST(d,c) \
    do { \
        disp = dseg_add_double(cd, (c)); \
        M_LDFD(d, REG_PV, disp); \
    } while (0)

#endif /* !defined(ENABLE_SOFTFLOAT) */


/* M_LONGBRANCH:
   performs a long branch to an absolute address with return address in LR
   takes up 3 bytes of code space; address is hard-coded into code
*/
#define M_LONGBRANCH(adr) \
	M_ADD_IMM(REG_LR, REG_PC, 4); \
	M_LDR_INTERN(REG_PC, REG_PC, -4); \
	DCD((s4) adr);

/* M_DSEG_LOAD/BRANCH:
   TODO: document me
   ATTENTION: if you change this, you have to look at the asm_call_jit_compiler!
   ATTENTION: we use M_LDR, so the same restrictions apply to us!
*/
#define M_DSEG_LOAD(reg, offset) \
	M_LDR_NEGATIVE(reg, REG_PV, offset)

#define M_DSEG_BRANCH(offset) \
	if (IS_OFFSET(offset, 0x0fff)) { \
		M_MOV(REG_LR, REG_PC); \
		M_LDR_INTERN(REG_PC, REG_PV, offset); \
	} else { \
		/*assert((offset) <= 0);*/ \
		CHECK_OFFSET(offset,0x0fffff); \
		M_SUB_IMM(REG_ITMP3, REG_PV, ((-(offset) >>  12) & 0xff) | (((10) & 0x0f) << 8)); /*TODO: more to go*/ \
		M_MOV(REG_LR, REG_PC); \
		M_LDR_INTERN(REG_PC, REG_ITMP3, -(-(offset) & 0x0fff)); /*TODO: this looks ugly*/ \
	}


#define M_ILD(a,b,c)                    M_LDR(a,b,c)
#define M_LLD(a,b,c)                    M_LDRD(a,b,c)

#define M_ILD_INTERN(a,b,c)             M_LDR_INTERN(a,b,c)
#define M_LLD_INTERN(a,b,c)             M_LDRD_INTERN(a,b,c)

#define M_ALD(a,b,c)                    M_ILD(a,b,c)
#define M_ALD_INTERN(a,b,c)             M_ILD_INTERN(a,b,c)
#define M_ALD_DSEG(a,c)                 M_DSEG_LOAD(a,c)


#define M_IST(a,b,c)                    M_STR(a,b,c)
#define M_LST(a,b,c)                    M_STRD(a,b,c)

#define M_IST_INTERN(a,b,c)             M_STR_INTERN(a,b,c)
#define M_LST_INTERN(a,b,c)             M_STRD_INTERN(a,b,c)

#define M_AST(a,b,c)                    M_IST(a,b,c)
#define M_AST_INTERN(a,b,c)             M_IST_INTERN(a,b,c)


#define M_ACMP(a,b)                     M_CMP(a,b)
#define M_ICMP(a,b)                     M_CMP(a,b)


#define M_TEST(a)                       M_TEQ_IMM(a, 0);


#if !defined(ENABLE_SOFTFLOAT)

#define M_FLD(a,b,c)                    M_LDFS(a,b,c)
#define M_DLD(a,b,c)                    M_LDFD(a,b,c)

#define M_FST(a,b,c)                    M_STFS(a,b,c)
#define M_DST(a,b,c)                    M_STFD(a,b,c)

#else /* !defined(ENABLE_SOFTFLOAT) */

#define M_FMOV(s,d)                     M_MOV((d), (s))
#define M_DMOV(s,d) \
	{ \
		M_MOV(GET_LOW_REG(d), GET_LOW_REG(s)); \
		M_MOV(GET_HIGH_REG(d), GET_HIGH_REG(s)); \
	}

#endif /* !defined(ENABLE_SOFTFLOAT) */


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
