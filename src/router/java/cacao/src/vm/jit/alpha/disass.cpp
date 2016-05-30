/* src/vm/jit/alpha/disass.cpp - primitive disassembler for Alpha machine code

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

#include <cstdio>

#include "vm/types.hpp"

#include "vm/global.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/disass.hpp"


/*  The disassembler uses two tables for decoding the instructions. The first
	table (ops) is used to classify the instructions based on the op code and
	contains the instruction names for instructions which don't used the
	function codes. This table is indexed by the op code (6 bit, 64 entries).
	The second table (op3s) contains instructions which contain both an op
	code and a function code. This table is an unsorted list of instructions
	which is terminated by op code and function code zero. This list is
	searched linearly for a matching pair of opcode and function code.
*/

#define ITYPE_UNDEF 0           /* undefined instructions (illegal opcode)    */
#define ITYPE_JMP   1           /* jump instructions                          */
#define ITYPE_MEM   2           /* memory instructions                        */
#define ITYPE_FMEM  3           /* floating point memory instructions         */
#define ITYPE_BRA   4           /* branch instructions                        */
#define ITYPE_OP    5           /* integer instructions                       */
#define ITYPE_FOP   6           /* floating point instructions                */


/* instruction decode table for 6 bit op codes                                */

static struct {char *name; int itype;} ops[] = {
	/* 0x00 */  {"",        ITYPE_UNDEF},
	/* 0x01 */  {"",        ITYPE_UNDEF},
	/* 0x02 */  {"",        ITYPE_UNDEF},
	/* 0x03 */  {"",        ITYPE_UNDEF},
	/* 0x04 */  {"",        ITYPE_UNDEF},
	/* 0x05 */  {"",        ITYPE_UNDEF},
	/* 0x06 */  {"",        ITYPE_UNDEF},
	/* 0x07 */  {"",        ITYPE_UNDEF},
	/* 0x08 */  {"lda    ",   ITYPE_MEM},
	/* 0x09 */  {"ldah   ",   ITYPE_MEM},
	/* 0x0a */  {"ldb    ",   ITYPE_MEM},
	/* 0x0b */  {"ldq_u  ",   ITYPE_MEM},
	/* 0x0c */  {"ldw    ",   ITYPE_MEM},
	/* 0x0d */  {"stw    ",   ITYPE_MEM},
	/* 0x0e */  {"stb    ",   ITYPE_MEM},
	/* 0x0f */  {"stq_u  ",   ITYPE_MEM},
	/* 0x10 */  {"op     ",    ITYPE_OP},
	/* 0x11 */  {"op     ",    ITYPE_OP},
	/* 0x12 */  {"op     ",    ITYPE_OP},
	/* 0x13 */  {"op     ",    ITYPE_OP},
	/* 0x14 */  {"",        ITYPE_UNDEF},
	/* 0x15 */  {"",        ITYPE_UNDEF},
	/* 0x16 */  {"fop    ",   ITYPE_FOP},
	/* 0x17 */  {"fop    ",   ITYPE_FOP},
	/* 0x18 */  {"memfmt ",   ITYPE_MEM},
	/* 0x19 */  {"",        ITYPE_UNDEF},
	/* 0x1a */  {"jmp    ",   ITYPE_JMP},
	/* 0x1b */  {"",        ITYPE_UNDEF},
	/* 0x1c */  {"op     ",    ITYPE_OP},
	/* 0x1d */  {"",        ITYPE_UNDEF},
	/* 0x1e */  {"",        ITYPE_UNDEF},
	/* 0x1f */  {"",        ITYPE_UNDEF},
	/* 0x20 */  {"ldf    ",  ITYPE_FMEM},
	/* 0x21 */  {"ldg    ",  ITYPE_FMEM},
	/* 0x22 */  {"lds    ",  ITYPE_FMEM},
	/* 0x23 */  {"ldt    ",  ITYPE_FMEM},
	/* 0x24 */  {"stf    ",  ITYPE_FMEM},
	/* 0x25 */  {"stg    ",  ITYPE_FMEM},
	/* 0x26 */  {"sts    ",  ITYPE_FMEM},
	/* 0x27 */  {"stt    ",  ITYPE_FMEM},
	/* 0x28 */  {"ldl    ",   ITYPE_MEM},
	/* 0x29 */  {"ldq    ",   ITYPE_MEM},
	/* 0x2a */  {"ldl_l  ",   ITYPE_MEM},
	/* 0x2b */  {"ldq_l  ",   ITYPE_MEM},
	/* 0x2c */  {"stl    ",   ITYPE_MEM},
	/* 0x2d */  {"stq    ",   ITYPE_MEM},
	/* 0x2e */  {"stl_c  ",   ITYPE_MEM},
	/* 0x2f */  {"stq_c  ",   ITYPE_MEM},
	/* 0x30 */  {"br     ",   ITYPE_BRA},
	/* 0x31 */  {"fbeq   ",   ITYPE_BRA},
	/* 0x32 */  {"fblt   ",   ITYPE_BRA},
	/* 0x33 */  {"fble   ",   ITYPE_BRA},
	/* 0x34 */  {"bsr    ",   ITYPE_BRA},
	/* 0x35 */  {"fbne   ",   ITYPE_BRA},
	/* 0x36 */  {"fbge   ",   ITYPE_BRA},
	/* 0x37 */  {"fbgt   ",   ITYPE_BRA},
	/* 0x38 */  {"blbc   ",   ITYPE_BRA},
	/* 0x39 */  {"beq    ",   ITYPE_BRA},
	/* 0x3a */  {"blt    ",   ITYPE_BRA},
	/* 0x3b */  {"ble    ",   ITYPE_BRA},
	/* 0x3c */  {"blbs   ",   ITYPE_BRA},
	/* 0x3d */  {"bne    ",   ITYPE_BRA},
	/* 0x3e */  {"bge    ",   ITYPE_BRA},
	/* 0x3f */  {"bgt    ",   ITYPE_BRA}
};


/* instruction decode list for 6 bit op codes and 9 bit function codes        */
 
static struct { u2 op, fun; char *name; }  op3s[] = {
	{ 0x10, 0x00,  "addl   " },
	{ 0x10, 0x40,  "addl/v " },
	{ 0x10, 0x20,  "addq   " },
	{ 0x10, 0x60,  "addq/v " },
	{ 0x10, 0x09,  "subl   " },
	{ 0x10, 0x49,  "subl/v " },
	{ 0x10, 0x29,  "subq   " },
	{ 0x10, 0x69,  "subq/v " },
	{ 0x10, 0x2D,  "cmpeq  " },
	{ 0x10, 0x4D,  "cmplt  " },
	{ 0x10, 0x6D,  "cmple  " },
	{ 0x10, 0x1D,  "cmpult " },
	{ 0x10, 0x3D,  "cmpule " },
	{ 0x10, 0x0F,  "cmpbge " },
	{ 0x10, 0x02,  "s4addl " },
	{ 0x10, 0x0b,  "s4subl " },
	{ 0x10, 0x22,  "s4addq " },
	{ 0x10, 0x2b,  "s4subq " },
	{ 0x10, 0x12,  "s8addl " },
	{ 0x10, 0x1b,  "s8subl " },
	{ 0x10, 0x32,  "s8addq " },
	{ 0x10, 0x3b,  "s8subq " },
	{ 0x11, 0x00,  "and    " },
	{ 0x11, 0x20,  "or     " },
	{ 0x11, 0x40,  "xor    " },
	{ 0x11, 0x08,  "andnot " },
	{ 0x11, 0x28,  "ornot  " },
	{ 0x11, 0x48,  "xornot " },
	{ 0x11, 0x24,  "cmoveq " },
	{ 0x11, 0x44,  "cmovlt " },
	{ 0x11, 0x64,  "cmovle " },
	{ 0x11, 0x26,  "cmovne " },
	{ 0x11, 0x46,  "cmovge " },
	{ 0x11, 0x66,  "cmovgt " },
	{ 0x11, 0x14,  "cmovlbs" },
	{ 0x11, 0x16,  "cmovlbc" },
	{ 0x12, 0x39,  "sll    " },
	{ 0x12, 0x3C,  "sra    " },
	{ 0x12, 0x34,  "srl    " },
	{ 0x12, 0x30,  "zap    " },
	{ 0x12, 0x31,  "zapnot " },
	{ 0x12, 0x06,  "extbl  " },
	{ 0x12, 0x16,  "extwl  " },
	{ 0x12, 0x26,  "extll  " },
	{ 0x12, 0x36,  "extql  " },
	{ 0x12, 0x5a,  "extwh  " },
	{ 0x12, 0x6a,  "extlh  " },
	{ 0x12, 0x7a,  "extqh  " },
	{ 0x12, 0x0b,  "insbl  " },
	{ 0x12, 0x1b,  "inswl  " },
	{ 0x12, 0x2b,  "insll  " },
	{ 0x12, 0x3b,  "insql  " },
	{ 0x12, 0x57,  "inswh  " },
	{ 0x12, 0x67,  "inslh  " },
	{ 0x12, 0x77,  "insqh  " },
	{ 0x12, 0x02,  "mskbl  " },
	{ 0x12, 0x12,  "mskwl  " },
	{ 0x12, 0x22,  "mskll  " },
	{ 0x12, 0x32,  "mskql  " },
	{ 0x12, 0x52,  "mskwh  " },
	{ 0x12, 0x62,  "msklh  " },
	{ 0x12, 0x72,  "mskqh  " },
	{ 0x13, 0x00,  "mull   " },
	{ 0x13, 0x20,  "mulq   " },
	{ 0x13, 0x40,  "mull/v " },
	{ 0x13, 0x60,  "mulq/v " },
	{ 0x13, 0x30,  "umulh  " },
	{ 0x16, 0x080, "fadd   " },
	{ 0x16, 0x0a0, "dadd   " },
	{ 0x16, 0x081, "fsub   " },
	{ 0x16, 0x0a1, "dsub   " },
	{ 0x16, 0x082, "fmul   " },
	{ 0x16, 0x0a2, "dmul   " },
	{ 0x16, 0x083, "fdiv   " },
	{ 0x16, 0x0a3, "ddiv   " },
	{ 0x16, 0x580, "fadds  " },
	{ 0x16, 0x5a0, "dadds  " },
	{ 0x16, 0x581, "fsubs  " },
	{ 0x16, 0x5a1, "dsubs  " },
	{ 0x16, 0x582, "fmuls  " },
	{ 0x16, 0x5a2, "dmuls  " },
	{ 0x16, 0x583, "fdivs  " },
	{ 0x16, 0x5a3, "ddivs  " },
	{ 0x16, 0x0ac, "cvtdf  " },
	{ 0x16, 0x0bc, "cvtlf  " },
	{ 0x16, 0x0be, "cvtld  " },
	{ 0x16, 0x0af, "cvtdl  " },
	{ 0x16, 0x02f, "cvtdlc " },
	{ 0x17, 0x030, "cvtli  " },
	{ 0x16, 0x1af, "cvtdlv " },
	{ 0x16, 0x12f, "cvtdlcv" },
	{ 0x17, 0x130, "cvtliv " },
	{ 0x16, 0x5ac, "cvtdfs " },
	{ 0x16, 0x5af, "cvtdls " },
	{ 0x16, 0x52f, "cvtdlcs" },
	{ 0x16, 0x0a4, "fcmpun " },
	{ 0x16, 0x0a5, "fcmpeq " },
	{ 0x16, 0x0a6, "fcmplt " },
	{ 0x16, 0x0a7, "fcmple " },
	{ 0x16, 0x5a4, "fcmpuns" },
	{ 0x16, 0x5a5, "fcmpeqs" },
	{ 0x16, 0x5a6, "fcmplts" },
	{ 0x16, 0x5a7, "fcmples" },
	{ 0x17, 0x020, "fmov   " },
	{ 0x17, 0x021, "fmovn  " },
	{ 0x1c, 0x0,   "bsext  " },
	{ 0x1c, 0x1,   "wsext  " },
	
	{ 0x00, 0x00,  NULL }
};


/* disassinstr *****************************************************************

   Outputs a disassembler listing of one machine code instruction on
   'stdout'.

   code: pointer to instructions machine code

*******************************************************************************/

u1 *disassinstr(u1 *code)
{
	s4 op;                      /* 6 bit op code                              */
	s4 opfun;                   /* 7 bit function code                        */
	s4 ra, rb, rc;              /* 6 bit register specifiers                  */
	s4 lit;                     /* 8 bit unsigned literal                     */
	s4 i;                       /* loop counter                               */
	s4 c;

	c = *((s4 *) code);

	op    = (c >> 26) & 0x3f;   /* 6 bit op code                              */
	opfun = (c >> 5)  & 0x7f;   /* 7 bit function code                        */
	ra    = (c >> 21) & 0x1f;   /* 6 bit source register specifier            */
	rb    = (c >> 16) & 0x1f;   /* 6 bit source register specifier            */
	rc    = (c >> 0)  & 0x1f;   /* 6 bit destination register specifiers      */
	lit   = (c >> 13) & 0xff;   /* 8 bit unsigned literal                     */

	printf("0x%016lx:   %08x    ", (u8) code, c);
	
	switch (ops[op].itype) {
	case ITYPE_JMP:
		switch ((c >> 14) & 3) {  /* branch hint */
		case 0:
			if (ra == 31) {
				printf("jmp     (%s)\n", abi_registers_integer_name[rb]); 
				goto _return;
			}
			printf("jmp     "); 
			break;
		case 1:
			if (ra == 26) {
				printf("jsr     (%s)\n", abi_registers_integer_name[rb]); 
				goto _return;
			}
			printf("jsr     "); 
			break;
		case 2:
			if (ra == 31 && rb == 26) {
				printf("ret\n"); 
				goto _return;
			}
			if (ra == 31) {
				printf("ret     (%s)\n", abi_registers_integer_name[rb]); 
				goto _return;
			}
			printf("ret     ");
			break;
		case 3:
			printf("jsr_co  "); 
			break;
		}
		printf("%s,(%s)\n", abi_registers_integer_name[ra],
			   abi_registers_integer_name[rb]); 
		break;

	case ITYPE_MEM: {
		s4 disp = (c << 16) >> 16;              /* 16 bit signed displacement */

		if (op == 0x18 && ra == 0 && ra == 0 && disp == 0)
			printf("trapb\n"); 
		else
			printf("%s %s,%d(%s)\n", ops[op].name,
				   abi_registers_integer_name[ra], disp,
				   abi_registers_integer_name[rb]); 
		break;
	}

	case ITYPE_FMEM:
		printf("%s $f%d,%d(%s)\n", ops[op].name, ra, (c << 16) >> 16,
			   abi_registers_integer_name[rb]); 
		break;

	case ITYPE_BRA:                            /* 21 bit signed branch offset */
		if (op == 0x30 && ra == 31)
			printf("br      0x%016lx\n", (u8) code + 4 + ((c << 11) >> 9));
		else if (op == 0x34 && ra == 26)
			printf("brs     0x%016lx\n", (u8) code + 4 + ((c << 11) >> 9));
		else
			printf("%s %s,0x%016lx\n", ops[op].name,
				   abi_registers_integer_name[ra],
				   (u8) code + 4 + ((c << 11) >> 9));
		break;
			
	case ITYPE_FOP: {
		s4 fopfun = (c >> 5) & 0x7ff;              /* 11 bit fp function code */

		if (op == 0x17 && fopfun == 0x020 && ra == rb) {
			if (ra == 31 && rc == 31)
				printf("fnop\n");
			else
				printf("fmov    $f%d,$f%d\n", ra, rc);
			goto _return;
		}
		for (i = 0; op3s[i].name; i++)
			if (op3s[i].op == op && op3s[i].fun == fopfun) {
				printf("%s $f%d,$f%d,$f%d\n", op3s[i].name, ra, rb,  rc);
				goto _return;
			}
		printf("%s%x $f%d,$f%d,$f%d\n", ops[op].name, fopfun, ra, rb, rc);
		break;
	}

	case ITYPE_OP:
		if (op == 0x11 && opfun == 0x20 && ra == rb && ~(c&0x1000)) {
			if (ra == 31 && rc == 31)
				printf("nop\n");
			else if (ra == 31)
				printf("clr     %s\n", abi_registers_integer_name[rc]);
			else
				printf("mov     %s,%s\n", abi_registers_integer_name[ra],
					   abi_registers_integer_name[rc]);
			goto _return;
		}
		for (i = 0; op3s[i].name; i++) {
			if (op3s[i].op == op && op3s[i].fun == opfun) {
				if (c & 0x1000)                      /* immediate instruction */
					printf("%s %s,%d,%s\n", op3s[i].name,
						   abi_registers_integer_name[ra], lit,
						   abi_registers_integer_name[rc]);
				else
					printf("%s %s,%s,%s\n", op3s[i].name,
						   abi_registers_integer_name[ra],
						   abi_registers_integer_name[rb],
						   abi_registers_integer_name[rc]);
				goto _return;
			}
		}
		/* fall through */
	default:
		if (c & 0x1000)                              /* immediate instruction */
			printf("UNDEF  %x(%x) $%d,%d,$%d\n", op, opfun, ra, lit, rc);
		else
			printf("UNDEF  %x(%x) $%d,$%d,$%d\n", op, opfun, ra, rb,  rc);		
	}

	/* 1 instruction is 4-bytes long */

 _return:
	return code + 4;
}


/* disassemble *****************************************************************

   Outputs a disassembler listing of some machine code on 'stdout'.

   start: pointer to first instruction
   end:   pointer to last instruction

*******************************************************************************/

void disassemble(u1 *start, u1 *end)
{
	printf("  --- disassembler listing ---\n");
	for (; start < end; )
		start = disassinstr(start);
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
 */
