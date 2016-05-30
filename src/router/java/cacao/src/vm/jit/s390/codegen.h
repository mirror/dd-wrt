/* src/vm/jit/s390/codegen.h - code generation macros for s390

   Copyright (C) 1996-2005, 2006, 2007, 2008, 2010
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


#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "config.h"

#include <ucontext.h>

#include "vm/types.hpp"

#include "vm/jit/jit.hpp"


/* MCODECHECK(icnt) */

#define MCODECHECK(icnt) \
    do { \
        if ((cd->mcodeptr + (icnt)) > cd->mcodeend) \
            codegen_increase(cd); \
    } while (0)

#define ALIGNCODENOP \
    do { \
        while (((ptrint) cd->mcodeptr) & 2) { \
            M_NOP2; \
        } \
        while (((ptrint) cd->mcodeptr) & 4) { \
            M_NOP; \
        } \
    } while (0)

/* some patcher defines *******************************************************/

#define PATCHER_CALL_SIZE    2          /* size in bytes of a patcher call    */
#define PATCHER_NOPS M_NOP3

/* branch defines ************************************************************/

#define BRANCH_NOPS \
	do { \
		if (CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd)) { \
			M_NOP2; M_NOP2; /* brc */ \
			M_NOP2; M_NOP2; M_NOP2; M_NOP2; M_NOP2; M_NOP2; M_NOP2; M_NOP2; /* ild */ \
			M_NOP2; /* ar, bcr */ \
		} else { \
			M_NOP; /* brc */ \
		} \
	} while (0) 


/* *** BIG TODO ***
 * Make all this inline functions !!!!!!!!!!
 */

/* macros to create code ******************************************************/

/* Conventions:
 * N_foo:   defines the instrucition foo as in `ESA/390 Principles of operations'
 * SZ_foo:  defines the size of the instruction N_foo
 * DD_foo:  defines a condition code as used by s390 GCC
 * M_foo:   defines the alpha like instruction used in cacao
 *          the instruction is defined by an equivalent N_ instruction
 * CC_foo:  defines a condition code as used
 *          the instruction is defined as an equivalent DD_ condition code
 */

/* S390 specific code */

/* Argument checks for debug mode */

/* Some instructions with register arguments treat %r0 as "value not given".
 * To prevent bugs, in debug mode we use a special value RN (reg none) with 
 * the meaning "value not given".
 * In debug mode, the instructions assert that %r0 was not given as argument.
 */

#if !defined(NDEBUG)

#	include <stdlib.h>

	/* register none */
#	define RN 16

	static inline int _OR_IMPL(const char *file, int line, int r) {
		if(!(
			((0 < r) && (r < 16)) ||
			(r == RN)
		)) {
			fprintf(stdout, "%d is not a valid register at %s:%d.\n", r, file, line);
			abort();
		}
		return ((r == RN) ? 0 : r);
	}
#	define _OR(r) _OR_IMPL(__FILE__, __LINE__, r)

#	define _SMIN(b) (-(1 << (bits - 1)))
#	define _SMAX(b) ((1 << (b - 1)) - 1)
#	define _UMIN(b) 0
#	define _UMAX(b) ((1 << b) - 1)

	static inline int _UBITS_IMPL(const char *file, int line, int i, int bits) {
		if (!((_UMIN(bits) <= i) && (i <= _UMAX(bits)))) {
			fprintf(stdout, "%d (0x%X) is not an unsigned %d bit integer at %s:%d.\n", i, i, bits, file, line);
			abort();
		}
		return i;
	}

#	define _UBITS(i, bits) _UBITS_IMPL(__FILE__, __LINE__, i, bits)

	static inline int _SBITS_IMPL(const char *file, int line, int i, int bits) {
		if(!((_SMIN(bits) <= i) && (i <= _SMAX(bits)))) {
			fprintf(stdout, "%d (0x%X) is not an signed %d bit integer at %s:%d.\n", i, i, bits, file, line);
			abort();
		}
		return i;
	}

#	define _SBITS(i, bits) _SBITS_IMPL(__FILE__, __LINE__, i, bits)

	static inline int _BITS_IMPL(const char *file, int line, int i, int bits) {
		if (!(
			((_UMIN(bits) <= i) && (i <= _UMAX(bits))) ||
			((_SMIN(bits) <= i) && (i <= _SMAX(bits)))
		)) {
			fprintf(stdout, "%d (0x%X) is not an %d bit integer at %s:%d.\n", i, i, bits, file, line);
			abort();
		}
		return i;
	}

#	define _BITS(i, bits) _BITS_IMPL(__FILE__, __LINE__, i, bits)

#else
#	define RN 0
#	define _OR(x) (x)
#	define _BITS(x, b) (x)
#	define _UBITS(x, b) (x)
#	define _SBITS(x, b) (x)
#endif

/* Register */
#define _R(x) _UBITS((x), 4)
/* Displacement */
#define _D(x) _UBITS((x), 12)
/* 4 bit Immediate */
#define _I4(x) _BITS((x), 4)
#define _UI4(x) _UBITS((x), 4)
#define _SI4(x) _SBITS((x), 4)
/* 8 bit Immediate */
#define _I8(x) _BITS((x), 8)
#define _UI8(x) _UBITS((x), 8)
#define _SI8(x) _SBITS((x), 8)
/* 12 bit Immediate */
#define _I12(x) _BITS((x), 12)
#define _UI12(x) _UBITS((x), 12)
#define _SI12(x) _SBITS((x), 12)
/* 16 bit Immediate */
#define _I16(x) _BITS((x), 16)
#define _UI16(x) _UBITS((x), 16)
#define _SI16(x) _SBITS((x), 16)
/* Opcode */
#define _OP(x) _UBITS((x), 8)
/* Second part of opcode */
#define _OP4(x) _UBITS((x), 4)
/* Extended opcode */
#define _OP16(x) _UBITS((x), 16)

/* Instruction formats */

#define _CODE(t, code) \
	do { \
		*((t *) cd->mcodeptr) = (code); \
		cd->mcodeptr += sizeof(t); \
	} while (0)

#define _CODE2(code) _CODE(u2, code)
#define _CODE4(code) _CODE(u4, code)

#define _IF(cond, t, f) \
	do { if (cond) { t ; } else { f ; } } while (0)

#define _IFNEG(val, neg, pos) _IF((val) < 0, neg, pos)

#define N_RR(op, r1, r2) \
	_CODE2( (_OP(op) << 8) | (_R(r1) << 4) | _R(r2) )

#define SZ_RR 2

static inline uint8_t N_RR_GET_OPC(uint8_t *instrp) {
	return instrp[0];
}

static inline uint8_t N_RR_GET_REG1(uint8_t *instrp) {
	return (instrp[1] >> 4) & 0xF;
}

static inline uint8_t N_RR_GET_REG2(uint8_t *instrp) {
	return (instrp[1] & 0xF);
}

#define N_RR2(op, i) \
	_CODE2( (_OP(op) << 8) | _I8(i) )

#define N_RX(op, r1, d2, x2, b2) \
	_CODE4( (_OP(op) << 24) | (_R(r1) << 20) | (_OR(x2) << 16) | (_OR(b2) << 12) | (_D(d2) << 0) )

#define SZ_RX 4

static inline uint8_t N_RX_GET_OPC(uint8_t *instrp) {
	return instrp[0];
}

static inline uint8_t N_RX_GET_REG(uint8_t *instrp) {
	return (instrp[1] >> 4) & 0xF;	
}

static inline uint8_t N_RX_GET_INDEX(uint8_t *instrp) {
	return (instrp[1] & 0xF);	
}

static inline uint8_t N_RX_GET_BASE(uint8_t *instrp) {
	return (instrp[2] >> 4) & 0xF;
}

static inline uint16_t N_RX_GET_DISP(uint8_t *instrp) {
	return *(uint16_t *)(instrp + 2) & 0xFFF;
}

static inline void N_RX_SET_DISP(uint8_t *instrp, uint16_t disp) {
	*(uint16_t *)(instrp + 2) |= (disp & 0xFFF);
}

#define N_RI(op1, op2, r1, i2) \
	_CODE4( (_OP(op1) << 24) | (_R(r1) << 20) | (_OP4(op2) << 16) | (u2)_SI16(i2) )

static inline int16_t N_RI_GET_IMM(uint8_t *instrp) {
	return *(int16_t *)(instrp + 2);
}

static inline void N_RI_SET_IMM(uint8_t *instrp, int16_t imm) {
	*(int16_t *)(instrp + 2) = imm;
}

#define N_RI2(op1, op2, r1, i2) \
	_CODE4( (_OP(op1) << 24) | (_R(r1) << 20) | (_OP4(op2) << 16) | (u2)_UI16(i2) )

#define SZ_RI 4

#define N_SI(op, d1, b1, i2) \
	_CODE4( (_OP(op) << 24) | (_OR(i2) << 16) | (_OR(b1) << 12) | _D(d1) )

#define SZ_SI 4

#define N_SS(op, d1, l, b1, d2, b2) \
	do { \
		_CODE4( (_OP(op) << 24) | (_I8(l) << 16) | (_OR(b1) << 12) | _D(d1) ); \
		_CODE2( (_OR(b2) << 12) | _D(d2) ); \
	} while (0)

#define SZ_SS 6

#define N_SS2(op, d1, l1, b1, d2, l2, b2) \
	N_SS(op, d1, (_I4(l1) << 4) | _I4(l2), b1, d2, l2)

#define N_RS(op, r1, r3, d2, b2) \
	_CODE4( (_OP(op) << 24) | (_R(r1) << 20) | (_R(r3) << 16) | (_OR(b2) << 12) | _D(d2) )

#define SZ_RS 4

#define N_RSI(op, r1, r2, i2) \
	_CODE4( ((op) << 24) | (_R(r1) << 20) | (_R(r3) << 16) | (u2)_16(i2) )

#define SZ_RSI 4

#define N_RRE(op, r1, r2) \
	_CODE4( (_OP16(op) << 16) | (_R(r1) << 4) | _R(r2) )

#define SZ_RRE 4

#define N_S2(d2, b2) \
	_CODE4( (_OP16(op) << 16) | (_OR(b2) << 12) | _D(d2)  )

#define SZ_S2 4

#define N_E(op) \
	_CODE2( _OP16(op) )

#define SZ_E 2

#define N_RXE(op, r1, d2, x2, b2) \
	do { \
		_CODE4( ((_OP16(op) >> 8) << 24) | (_R(r1) << 20) | \
			(_R(x2) << 16) | (_R(b2) << 12) | _UI12(d2) ); \
		_CODE2( _OP16(op) & 0xFF ); \
	} while (0) 

#define S_RXE 6

#define N_RRF(op, r1, m3, r2) \
	_CODE4( (_OP16(op) << 16) | (_R(m3) << 12) | (_R(r1) << 4) | _R(r2) )

#define S_RRF 4

#define N_IMM_MIN -32768
#define N_IMM_MAX 32767
#define N_VALID_IMM(x) ((N_IMM_MIN <= (x)) && ((x) <= N_IMM_MAX))
#define ASSERT_VALID_IMM(x) assert(N_VALID_IMM(x))

#define N_DISP_MIN 0
#define N_DISP_MAX 0xFFF
#define N_VALID_DISP(x) ((N_DISP_MIN <= (x)) && ((x) <= N_DISP_MAX))
#define ASSERT_VALID_DISP(x) assert(N_VALID_DISP(x))

#define N_PV_OFFSET (-0xFFC)
#define N_DSEG_DISP(x) ((x) - N_PV_OFFSET)
#define N_VALID_DSEG_DISP(x) N_VALID_DISP(N_DSEG_DISP(x))

#define N_BRANCH_MIN (-32768 * 2)
#define N_BRANCH_MAX (32767 * 2)
#define N_VALID_BRANCH(x) ((N_BRANCH_MIN <= (x)) && ((x) <= N_BRANCH_MAX))
#define ASSERT_VALID_BRANCH(x) assert(N_VALID_BRANCH(x))

#define N_IS_EVEN_ODD(x) \
	(((GET_HIGH_REG(x) % 2) == 0) && (GET_LOW_REG(x) == (GET_HIGH_REG(x) + 1)))

/* Condition codes */

#define DD_O 1
#define DD_H 2
#define DD_NLE 3
#define DD_L 4
#define DD_NHE 5
#define DD_LH 6
#define DD_NE 7
#define DD_E 8
#define DD_NLH 9
#define DD_HE 10
#define DD_NL 11
#define DD_LE 12
#define DD_NH 13
#define DD_NO 14
#define DD_ANY 15

#define DD_0 8
#define DD_1 4
#define DD_2 2
#define DD_3 1

/* Misc */

/* Trap instruction.
 * If most significant bits of first opcode byte are 00, then
 * format is RR (1 byte opcode) or E (2 bytes opcode). 
 * There seems to be no opcode 0x02 or 0x02**, so we'll define
 * our trap instruction as:
 * +--------+--------+
 * |  0x02  |  data  |
 * +--------+--------+
 * 0                 15
 */
#define N_ILL(data) _CODE2(0x0200 | _UBITS(data, 8))
#	define OPC_ILL 0x02
#	define SZ_ILL 2

static inline uint8_t N_ILL_GET_REG(uint8_t *instrp) {
	return (instrp[1] >> 4) & 0xF;
}

static inline uint8_t N_ILL_GET_TYPE(uint8_t *instrp) {
	return (instrp[1] & 0xF);
}

#define N_LONG(l) _CODE4(l)
#define SZ_LONG 4

/* Chapter 7. General instructions */

#define N_AR(r1, r2) N_RR(0x1A, r1, r2)
#define N_A(r1, d2, x2, b2) N_RX(0x5A, r1, d2, x2, b2)
#define N_AH(r1, d2, x2, b2) N_RX(0x4A, r1, d2, x2, b2)
#define N_AHI(r1, i2) N_RI(0xA7, 0xA, r1, i2)
#	define SZ_AHI SZ_RI
#define N_ALR(r1, r2) N_RR(0x1E, r1, r2)
#define N_AL(r1, d2, x2, b2) N_RX(0x5E, r1, d2, x2, b2)
#define N_NR(r1, r2) N_RR(0x14, r1, r2)
#	define SZ_NR SZ_RR
#define N_N(r1, d2, x2, b2) N_RX(0x54, r1, d2, x2, b2)
#define N_NI(d1, b1, i2) N_SI(0x94, d1, b1, i2)
#define N_NC(d1, l, b1, d2, b2) N_SS(0xD4, (l - 1), b1, d1, b2, d2)
#define N_BALR(r1, r2) N_RR(0x05, r1, _OR(r2))
#define N_BAL(r1, d2, x2, b2) N_RX(0x45, r1, d2, x2, b2)
#define N_BASR(r1, r2) N_RR(0x0D, r1, _OR(r2))
#	define SZ_BASR SZ_RR
#define N_BAS(r1, d2, x2, b2) N_RX(0x4D, r1, d2, x2, b2)
#define N_BASSM(r1, r2) N_RR(0x0C, r1, _OR(r2))
#define N_BSM(r1, r2) N_RR(0x0B, r1, _OR(r2))
#define N_BCR(m1, r2) N_RR(0x07, m1, _OR(r2))
#	define SZ_BCR SZ_RR
#	define N_BR(r2) N_BCR(DD_ANY, r2)
#define N_BC(m1, d2, x2, b2) N_RX(0x47, m1, d2, x2, b2)
#	define SZ_BC SZ_RS
#define N_BCTR(r1, r2) N_RR(0x06, r1, _OR(r2))
#define N_BCT(r1, d2, x2, b2) N_RX(0x46, r1, d2, x2, b2)
#define N_BHX(r1, r2, d2, b2) N_RS(0xB6, r1, r3, d2, b2)
#define N_BXLE(r1, r3, d2, b2) N_RS(0xB7, r1, r3, d2, b2)
#define N_BRAS(r1, i2) N_RI(0xA7, 0x5, r1, (i2) / 2)
#	define SZ_BRAS SZ_RI
#define N_BRC(m1, i2) N_RI(0xA7, 0x4, m1, (i2) / 2)
#	define N_J(i2) N_BRC(DD_ANY, i2)
#	define SZ_BRC SZ_RI
#	define SZ_J SZ_RI
#	define N_BRC_BACK_PATCH(brc_pos) \
		do { \
			*(u4 *)(brc_pos) |= (u4)(cd->mcodeptr - (brc_pos)) / 2; \
		} while (0)
#define N_BRCT(r1, i2) N_RI(0xA7, 0x6, r1, (i2) / 2)
#define N_BRXH(r1, r3, i2) N_RSI(0x84, r1, r3, (i2) / 2)
#define N_BRXLE(r1, r3, i2) N_RSI(0x85, r1, r2, (i2) / 2)
#define N_CKSM(r1, r2) N_RRE(0xB241, r1, r2)
#define N_CR(r1, r2) N_RR(0x19, r1, r2)
#	define SZ_CR SZ_RR
#define N_C(r1, d2, x2, b2) N_RX(0x59, r1, d2, x2, b2)
#define N_CFC(d2, b2) N_S2(0xB21A, d2, b2)
#define N_CS(r1, r3, d2, b2) N_RS(0xBA, r1, r3, d2, b2)
#define N_CDS(r1, r3, d2, b2) N_RS(0xBB, r1, r3, d2, b2)
#define N_CH(r1, d2, x2, b2) N_CH(0x49, r1, d2, x2, b2)
#define N_CHI(r1, i2) N_RI(0xA7, 0xE, r1, i2)
#define N_CLR(r1, r2) N_RR(0x15, r1, r2)
#define N_CL(r1, d2, x2, b2) N_RX(0x55, r1, d2, x2, b2)
#	define OPC_CL 0x55
#define N_CLI(d1, b1, i2) N_SI(0x95, d1, b1, i2)
#define N_CLC(d1, l, b1, d2, b2) N_SS(0xD5, d1, (l - 1), b1, d2, b2)
#define N_CLM(r1, m3, d2, b2) N_RS(0xBD, r1, m3, d2, b2)
#define N_CLCL(r1, r2) N_RR(0x0F, r1, r2)
#define N_CLCLE(r1, r3, d2, b2) N_RS(0xA9, r1, r3, d2, b2)
#define N_CLST(r1, r2) N_RRE(0xB25D, r1, r2)
#define N_CUSE(r1, r2) N_RRE(0xB257, r1, r2)
#define N_CVB(r1, d2, x2, b2) N_RX(0x4F, r1, r2, x2, b2)
#define N_CVD(r1, d2, x2, b2) N_RX(0x4E, r1, d2, x2, b2)
#define N_CUUTF(r1, r2) N_RRE(0xB2A6, r1, r2)
#define N_CUTFU(r1, r2) N_RRE(0xB2A7, r1, r2)
#define N_CPYA(r1, r2) N_RRE(0xB240, r1, r2)
#define N_DR(r1, r2) N_RR(0x1D, r1, r2)
#	define OPC_DR 0x1D
#define N_D(r1, d2, x2, b2) N_RX(0x5D, r1, d2, x2, b2)
#define N_XR(r1, r2) N_RR(0x17, r1, r2)
#define N_X(r1, d2, x2, b2) N_RX(0x57, r1, d2, x2, b2)
#define N_XI(d1, b1, i2) N_SI(0x97, d1, b1, i2)
#define N_XC(d1, l, b1, d2, b2) N_SS(0xD7, d1, (l - 1), b1, d2, b2)
#define N_EX(r1, d2, x2, b2) N_RX(0x44, r1, d2, x2, b2)
#define N_EAR(r1, r2) N_RRE(0xB24F, r1, r2)
#define N_IC(r1, d2, x2, b2) N_RX(0x43, r1, d2, x2, b2)
#define N_ICM(r1, m3, d2, b2) N_RS(0xBF, r1, m3, d2, b2)
#define N_IPM(r1) N_RRE(0xB222, r1, 0)
#define N_LR(r1, r2) N_RR(0x18, r1, r2)
#define N_L(r1, d2, x2, b2) N_RX(0x58, r1, d2, x2, b2)
#	define SZ_L SZ_RX
#	define OPC_L 0x58
#define N_LAM(r1, r3, d2, b2) N_RS(0x9A, r1, r3, d2, b2)
#define N_LA(r1, d2, x2, b2) N_RX(0x41, r1, d2, x2, b2)
#define N_LAE(r1, d2, x2, b2) N_RX(0x51, r1, d2, x2, b2)
#define N_LTR(r1, r2) N_RR(0x12, r1, r2)
#define N_LCR(r1, r2) N_RR(0x13, r1, r2)
#	define SZ_LCR SZ_RR
#define N_LH(r1, d2, x2, b2) N_RX(0x48, r1, d2, x2, b2)
#define N_LHI(r1, i2) N_RI(0xA7, 0x8, r1, i2)
#	define SZ_LHI SZ_RI
#define N_LM(r1, r3, d2, b2) N_RS(0x98, r1, r3, d2, b2)
#define N_LNR(r1, r2) N_RR(0x11, r1, r2)
#define N_LPR(r1, r2) N_RR(0x10, r1, r2)
#define N_MC(d1, b1, i2) N_SI(0xAF, d1, b1, i2)
#define N_MVI(d1, b1, i2) N_SI(0x92, d1, b1, i2)
#define N_MVC(d1, l, b1, d2, b2) N_SS(0xD2, d1, (l - 1), b1, d2, b2)
#define N_MVCIN(d1, l, b1, d2, b2) N_SS(0xEB, d1, (l - 1), b1, d2, b2)
#define N_MVCL(r1, r2) N_RR(0x0E, r1, r2)
#define N_MVCLE(r1, r3, d2, b2)  N_RS(0xAB, r1, r3, d2, b2)
#define N_MVN(d1, l, b1, d2, b2) N_SS(0xD1, d1, (l - 1), b1, d2, b2)
#define N_MVPG(r1, r2) N_RRE(0xB254, r1, r2)
#define N_MVST(r1, r2) N_RRE(0xB255, r1, r2)
#define N_MVO(d1, l1, b1, d2, l2, b2) N_SS2(0xF1, d1, (l1 - 1), b1, d2, (l2 - 1), b2)
#define N_MVZ(d1, l, b1, d2, b2) N_SS(0xD3, d1, (l - 1), b1, d2, b2)
#define N_MR(r1, r2) N_RR(0x1C, r1, r2)
#define N_M(r1, d2, x2, b2) N_RX(0x5C, r1, d2, x2, b2)
#define N_MH(r1, d2, x2, b2) N_RX(0x4C, r1, d2, x2, b2)
#define N_MHI(r1, i2) N_RI(0xA7, 0xC, r1, i2)
#define N_MSR(r1, r2) N_RRE(0xB252, r1, r2)
#define N_MS(r1, d2, x2, b2) N_RX(0x71, r1, d2, x2, b2)
#define N_OR(r1, r2) N_RR(0x16, r1, r2)
#define N_O(r1, d2, x2, b2) N_RX(0x56, r1, d2, x2, b2)
#define N_OI(d1, b1, i2) N_SI(0x96, d1, b1, i2)
#define N_OC(d1, l, b1, d2, b2) N_SS(0xD6, d1, (l - 1), b1, d2, b2)
#define N_PACK(d1, l1, b1, d2, l2, b2) N_SS2(0xF2, d1, (l1 - 1), b1, d2, (l2 - 1), b2)
#define N_PLO(r1, d2, b2, r3, d4, b4) N_SS2(0xEE, d2, r1, b2, d4, r3, b4)
#define N_SRST(r1, r2) N_RRE(0xB25E, r1, r2)
#define N_SAR(r1, r2) N_RRE(0xB24E, r1, r2)
#define N_SPM(r1) N_RR(0x04, r1, 0x00)
#define N_SLDA(r1, d2, b2) N_RS(0x8F, r1, 0x00, d2, b2)
#define N_SLDL(r1, d2, b2) N_RS(0x8D, r1, 0x00, d2, b2)
#define N_SLA(r1, d2, b2) N_RS(0x8B, r1, 0x00, d2, b2)
#define N_SLL(r1, d2, b2) N_RS(0x89, r1, 0x00, d2, b2)
#define N_SRDA(r1, d2, b2) N_RS(0x8E, r1, 0x00, d2, b2)
#define N_SRDL(r1, d2, b2) N_RS(0x8C, r1, 0x00, d2, b2)
#define N_SRA(r1, d2, b2) N_RS(0x8A, r1, 0x00, d2, b2)
#define N_SRL(r1, d2, b2) N_RS(0x88, r1, 0x00, d2, b2)
#define N_ST(r1, d2, x2, b2) N_RX(0x50, r1, d2, x2, b2)
#	define OPC_ST 0x50
#define N_STAM(r1, r3, d2, b2) N_RS(0x9B, r1, r3, d2, b2)
#define N_STC(r1, d2, x2, b2) N_RX(0x42, r1, d2, x2, b2)
#define N_STCM(r1, m3, d2, b2) N_RS(0xBE, r1, m3, d2, b2)
#define N_STCK(d2, b2) N_S2(0xB205, d2, b2)
#define N_STCKE(d2, b2) N_S2(0xB278, d2, b2)
#define N_STH(r1, d2, x2, b2) N_RX(0x40, r1, d2, x2, b2)
#define N_STM(r1, r3, d2, b2) N_RS(0x90, r1, r3, d2, b2)
#define N_SR(r1, r2) N_RR(0x1B, r1, r2)
#define N_S(r1, d2, x2, b2) N_RX(0x5B, r1, d2, x2, b2)
#define N_SH(r1, d2, x2, b2) N_RX(0x4B, r1, d2, x2, b2)
#define N_SLR(r1, r2) N_RR(0x1F, r1, r2)
#define N_SL(r1, d2, x2, b2) N_RX(0x5F, r1, d2, x2, b2)
#define N_SVC(i) N_RR2(0x0A, i)
#define N_TS(d2, b2) N_S2(0x93, d2, b2)
#define N_TM(d1, b1, i2) N_SI(0x91, d1, b1, i2)
#define N_TMH(r1, i2) N_RI2(0xA7, 0x00, r1, i2)
#define N_TML(r1, i2) N_RI2(0xA7, 0x01, r1, i2)
#define N_TR(d1, l, b1, d2, b2) N_SS(0xDC, d1, (l - 1), b1, d2, b2)
#define N_TRT(d1, l, b1, d2, b2) N_SS(0xDD, d1, (l - 1), b1, d2, b2)
#define N_TRE(r1, r2) N_RRE(0xB2A5, r1, r2)
#define N_UNPK(d1, l1, b1, d2, l2, b2) N_SS2(0xF3, d1, (l1 - 1), b1, d2, (l2 - 2), b2)
#define N_UPT() N_E(0x0102)

/* Chapter 9. Floating point instructions */

#define N_LER(r1, r2) N_RR(0x38, r1, r2)
#define N_LDR(r1, r2) N_RR(0x28, r1, r2)
#define N_LXR(r1, r2) N_RRE(0xB365, r1, r2)
#define N_LE(r1, d2, x2, b2) N_RX(0x78, r1, d2, x2, b2)
#define N_LD(r1, d2, x2, b2) N_RX(0x68, r1, d2, x2, b2)
#define N_LZER(r1) N_RRE(0xB374, r1, 0x0)
#define N_LZDR(r1) N_RRE(0xB375, r1, 0x0)
#define N_LZXR(r1) N_RRE(0xB376, r1, 0x0)
#define N_STE(r1, d2, x2, b2) N_RX(0x70, r1, d2, x2, b2)
#define N_STD(r1, d2, x2, b2) N_RX(0x60, r1, d2, x2, b2)

/* chapter 19. Binary floating point instructions */

#define N_AEBR(r1, r2) N_RRE(0xB30A, r1, r2)
#define N_ADBR(r1, r2) N_RRE(0xB31A, r1, r2)
#define N_AXBR(r1, r2) N_RRE(0xB34A, r1, r2)
#define N_AEB(r1, d2, x2, b2) N_RXE(0xED0A, r1, d2, x2, b2)
#define N_ADB(r1, d2, x2, b2) N_RXE(0xED1A, r1, d2, x2, b2)

#define N_CEBR(r1, r2) N_RRE(0xB309, r1, r2)
#define N_CDBR(r1, r2) N_RRE(0xB319, r1, r2)
#define N_CXBR(r1, r2) N_RRE(0xB349, r1, r2)
#define N_CEB(r1, d2, x2, b2) N_RXE(0xED09, r1, d2, x2, b2)
#define N_CDB(r1, d2, x2, b2) N_RXE(0xED19, r1, d2, x2, b2)

#define N_CEFBR(r1, r2) N_RRE(0xB394, r1, r2)
#define N_CDFBR(r1, r2) N_RRE(0xB395, r1, r2)
#define N_CXFBR(r1, r2) N_RRE(0xB396, r1, r2)

#define N_CFEBR(r1, m3, r2) N_RRF(0xB398, r1, m3, r2)
#define N_CFDBR(r1, m3, r2) N_RRF(0xB399, r1, m3, r2)
#define N_CFXBR(r1, m3, r2) N_RRF(0xB39A, r1, m3, r2)

#define N_DEBR(r1, r2) N_RRE(0xB30D, r1, r2)
#define N_DDBR(r1, r2) N_RRE(0xB31D, r1, r2)
#define N_DXBR(r1, r2) N_RRE(0xB34D, r1, r2)
#define N_DEB(r1, d2, x2, b2) N_RXE(0xED0D, r1, d2, x2, b2)
#define N_DDB(r1, d2, x2, b2) N_RXE(0xED1D, r1, d2, x2, b2)

#define N_LCEBR(r1, r2) N_RRE(0xB303, r1, r2)
#define N_LCDBR(r1, r2) N_RRE(0xB313, r1, r2)
#define N_LCXBR(r1, r2) N_RRE(0xB343, r1, r2)

#define N_LDEBR(r1, r2) N_RRE(0xB304, r1, r2)
#	define SZ_LDEBR SZ_RRE
#define N_LXDBR(r1, r2) N_RRE(0xB305, r1, r2)
#define N_LXEBR(r1, r2) N_RRE(0xB306, r1, r2)

#define N_LEDBR(r1, r2) N_RRE(0xB344, r1, r2)
#define N_LDXBR(r1, r2) N_RRE(0xB345, r1, r2)
#define N_LEXBR(r1, r2) N_RRE(0xB346, r1, r2)

#define N_LTEBR(r1, r2) N_RRE(0xB302, r1, r2)
#define N_LTDBR(r1, r2) N_RRE(0xB312, r1, r2)
#define N_LTXBR(r1, r2) N_RRE(0xB342, r1, r2)

#define N_MEEBR(r1, r2) N_RRE(0xB317, r1, r2)
#define N_MDBR(r1, r2) N_RRE(0xB31C, r1, r2)
#define N_MXBR(r1, r2) N_RRE(0xB34C, r1, r2)
#define N_MDEBR(r1, r2) N_RRE(0xB30C, r1, r2)
#define N_MXDBR(r1, r2) N_RRE(0xB307, r1, r2)

#define N_SEBR(r1, r2) N_RRE(0xB30B, r1, r2)
#define N_SDBR(r1, r2) N_RRE(0xB31B, r1, r2)
#define N_SXBR(r1, r2) N_RRE(0xB34B, r1, r2)
#define N_SEB(r1, d2, x2, b2) N_RXE(0xED0B, r1, d2, x2, b2)
#define N_SDB(r1, d2, x2, b2) N_RXE(0xED1B, r1, d2, x2, b2)

/* Alpha like instructions */

#define M_CALL(r2) N_BASR(R14, r2)
#define M_ILL(data) N_ILL(data)
#define M_ILL2(data1, data2) N_ILL((_UBITS(data1, 4) << 4) | _UBITS(data2, 4))
#define M_LONG(l) N_LONG(l)

#define M_ILD(r, b, d) \
	do { \
		if (N_VALID_DISP(d)) { \
			N_L(r, d, RN, b); \
		} else if ((r == R0) && N_VALID_IMM(d)) { \
			N_LR(R0, R1); \
			N_LHI(R1, d); \
			N_L(R1, 0, R1, b); \
			N_XR(R1, R0); \
			N_XR(R0, R1); \
			N_XR(R1, R0); \
		} else if ((r != R0) && N_VALID_IMM(d)) { \
			N_LHI(r, d); N_L(r, 0, r, b); \
		} else { \
			N_BRAS(r, SZ_BRAS + SZ_LONG); \
			N_LONG(d); \
			N_L(r, 0, RN, r); \
			N_L(r, 0, r, b); \
		} \
	} while (0)

#define M_ILD_DSEG(r, d) M_ILD(r, REG_PV, N_DSEG_DISP(d))

#define M_ALD(r, b, d) M_ILD(r, b, d)
#define M_ALD_DSEG(r, d) M_ALD(r, REG_PV, N_DSEG_DISP(d))

#define M_LDA(r, b, d) \
	do { \
		if (N_VALID_DISP(d)) { \
			N_LA(r, d, RN, b); \
		} else if (N_VALID_IMM(d)) { \
			N_LHI(r, d); \
			N_LA(r, 0, r, b); \
		} else { \
			N_BRAS(r, SZ_BRAS + SZ_LONG); \
			N_LONG(d); \
			N_L(r, 0, RN, r); \
			N_LA(r, 0, r, b); \
		} \
	} while (0)
#define M_LDA_DSEG(r, d) M_LDA(r, REG_PV, N_DSEG_DISP(d))

#define M_FLD(r, b, d) N_LE(r, d, RN, b)
#define M_FLDN(r, b, d, t) _IFNEG( \
	d, \
	N_LHI(t, d); N_LE(r, 0, t, b), \
	N_LE(r, d, RN, b) \
)
#define M_FLD_DSEG(r, d, t) M_FLDN(r, REG_PV, N_DSEG_DISP(d), t)

#define M_DLD(r, b, d) N_LD(r, d, RN, b)
#define M_DLDN(r, b, d, t) _IFNEG( \
	d, \
	N_LHI(t, d); N_LD(r, 0, t, b), \
	N_LD(r, d, RN, b) \
)
#define M_DLD_DSEG(r, d, t) M_DLDN(r, REG_PV, N_DSEG_DISP(d), t)

#define M_LLD(r, b, d) _IFNEG( \
	d, \
	N_LHI(GET_LOW_REG(r), d); \
		N_L(GET_HIGH_REG(r), 0, GET_LOW_REG(r), b); \
		N_L(GET_LOW_REG(r), 4, GET_LOW_REG(r), b), \
	N_L(GET_HIGH_REG(r), (d) + 0, RN, b); N_L(GET_LOW_REG(r), (d) + 4, RN, b) \
)
#define M_LLD_DSEG(r, d) M_LLD(r, REG_PV, N_DSEG_DISP(d)

/* MOV(a, b) -> mov from A to B */

#define M_MOV(a, b) N_LR(b, a)
#define M_FMOV(a, b) N_LDR(b, a)
#define M_DMOV(a, b) M_FMOV((a), (b))
#define M_DST(r, b, d) _IFNEG(d, assert(0), N_STD(r, d, RN, b))
#define M_FST(r, b, d) _IFNEG(d, assert(0), N_STE(r, d, RN, b))
#define M_IST(r, b, d) _IFNEG( \
	d, \
	assert(0), \
	N_ST(r, d, RN, b) \
)
#define M_AST(r, b, d) M_IST(r, b, d)
#define M_LST(r, b, d) _IFNEG( \
	d, \
	assert(0), \
	N_ST(GET_HIGH_REG(r), (d) + 0, RN, b); N_ST(GET_LOW_REG(r), (d) + 4, RN, b) \
)
#define M_TEST(r) N_LTR(r, r)
#define M_BEQ(off) N_BRC(DD_E, off)
#define M_BNE(off) N_BRC(DD_NE, off)
#define M_BLE(off) N_BRC(DD_LE, off)
#define M_BGT(off) N_BRC(DD_H, off)
#define M_BLT(off) N_BRC(DD_L, off)
#define M_BGE(off) N_BRC(DD_HE, off)
#define M_BO(off) N_BRC(DD_O, off)

#define M_CMP(r1, r2) N_CR(r1, r2)
#define M_CMPU(r1, r2) N_CLR(r1, r2)
#define M_CLR(r) N_LHI(r, 0)
#define M_AADD_IMM(val, reg) N_AHI(reg, val)
#define M_IADD_IMM(val, reg) N_AHI(reg, val)
#define M_ISUB_IMM(val, reg) N_AHI(reg, -(val))
#define M_ASUB_IMM(val, reg) N_AHI(reg, -(val))
#define M_RET N_BCR(DD_ANY, R14)
#define M_BSR(ret_reg, disp) N_BRAS(ret_reg, disp)
#define M_BR(disp) N_BRC(DD_ANY, disp)
#define M_JMP(rs, rd) _IF(rs == RN, N_BCR(DD_ANY, rd), N_BASR(rs, rd))
#define M_NOP N_BC(0, 0, RN, RN)
#define M_NOP2 N_BCR(0, RN)
#define M_NOP3 N_BCR(0, 1)
#define M_JSR(reg_ret, reg_addr) N_BASR(reg_ret, reg_addr)
#define M_ICMP(a, b) N_CR(a, b)
#define M_ICMPU(a, b) N_CLR(a, b)
#define M_ICMP_IMM(a, b) N_CHI(a, b)
#define M_ACMP(a, b) N_CR(a, b)
#define M_CVTIF(src, dst) N_CEFBR(dst, src)
#define M_CVTID(src, dst) N_CDFBR(dst, src)
#define M_FMUL(a, dest) N_MEEBR(dest, a)
#define M_FSUB(a, dest) N_SEBR(dest, a)
#define M_FADD(a, dest) N_AEBR(dest, a)
#define M_FDIV(a, dest) N_DEBR(dest, a)
#define M_DMUL(a, dest) N_MDBR(dest, a)
#define M_DSUB(a, dest) N_SDBR(dest, a)
#define M_DADD(a, dest) N_ADBR(dest, a)
#define M_DDIV(a, dest) N_DDBR(dest, a)
#define M_CVTFI(src, dst) N_CFEBR(dst, 5, src)
#define M_CVTDI(src, dst) N_CFDBR(dst, 5, src)
#define M_IADD(a, dest) N_AR(dest, a)
#define M_AADD(a, dest) N_AR(dest, a)
#define M_ISUB(a, dest) N_SR(dest, a)
#define M_ASUB(a, dest) N_SR(dest, a)
#define M_IAND(a, dest) N_NR(dest, a)
#define M_IOR(a, dest) N_OR(dest, a)
#define M_IXOR(a, dest) N_XR(dest, a)
#define M_CVTFD(src,dst) N_LDEBR(dst, src)
#define M_CVTDF(src,dst) N_LEDBR(dst, src)

#define M_SLL_IMM(imm, reg) N_SLL(reg, imm, RN) 
#define M_SLA_IMM(imm, reg) N_SLA(reg, imm, RN) 

#define M_SLDL_IMM(imm, reg) N_SLDL(reg, imm, RN) 
#define M_SLDA_IMM(imm, reg) N_SLDA(reg, imm, RN) 

#define M_SRL_IMM(imm, reg) N_SRL(reg, imm, RN)
#define M_SRA_IMM(imm, reg) N_SRA(reg, imm, RN)

#define M_SRDL_IMM(imm, reg) N_SRDL(reg, imm, RN)
#define M_SRDA_IMM(imm, reg) N_SRDA(reg, imm, RN)

#define M_SLL(op, dst) N_SLL(dst, 0, op)
#define M_SLA(op, dst) N_SLA(dst, 0, op)

#define M_SLDL(op, dst) N_SLDL(dst, 0, op)
#define M_SLDA(op, dst) N_SLDA(dst, 0, op)

#define M_SRL(op, dst) N_SRL(dst, 0, op)
#define M_SRA(op, dst) N_SRA(dst, 0, op)

#define M_SRDL(op, dst) N_SRDL(dst, 0, op)
#define M_SRDA(op, dst) N_SRDA(dst, 0, op)

#define M_IMUL_IMM(val, reg) N_MHI(reg, val)
#define M_IMUL(a, dest) N_MSR(dest, a)

#define M_INEG(a, dest) N_LCR(dest, a)

#define M_FCMP(a, b) N_CEBR(a, b)
#define M_DCMP(a, b) N_CDBR(a, b)

#define M_FMOVN(r, dst) N_LCEBR(dst, r)
#define M_DMOVN(r, dst) N_LCDBR(dst, r)

#define ICONST(reg, i) \
	do { \
		if (N_VALID_IMM(i)) { \
			N_LHI(reg, i); \
		} else { \
			disp = dseg_add_s4(cd, (i)); \
			M_ILD_DSEG(reg, disp); \
		} \
	} while (0) 

#define LCONST(reg,c) \
	do { \
	    ICONST(GET_HIGH_REG((reg)), (s4) ((s8) (c) >> 32));	\
	    ICONST(GET_LOW_REG((reg)), (s4) ((s8) (c))); \
	} while (0)

#define M_ISUB_IMM32(imm, tmpreg, reg) \
	do { \
		if (N_VALID_IMM(imm)) { \
			M_ISUB_IMM(imm, reg); \
		} else { \
			ICONST(tmpreg, imm); \
			M_ISUB(tmpreg, reg); \
		} \
	} while (0)

#define M_ASUB_IMM32(imm, tmpreg, reg) M_ISUB_IMM32(imm, tmpreg, reg)

#endif /* _CODEGEN_H */

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
