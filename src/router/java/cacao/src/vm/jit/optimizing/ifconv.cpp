/* src/vm/jit/optimizing/ifconv.c - if-conversion

   Copyright (C) 1996-2014
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

#include "vm/jit/optimizing/ifconv.hpp"


#include "config.h"

#include <assert.h>

#include "vm/types.hpp"

#include "vm/method.hpp"
#include "vm/vm.hpp"

#include "vm/jit/codegen-common.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/show.hpp"

#include "vm/jit/ir/instruction.hpp"

/* patterns for a total number of 3 instructions ******************************/

#define IFCONV_PATTERN_3_SIZE    sizeof(ifconv_pattern_3) / (sizeof(s4) * 3 * 2)

static s4 ifconv_pattern_3[][2][3] = {
	/* PATTERN 1 */

	{
		{
			ICMD_ICONST,
			ICMD_GOTO,

			ICMD_ICONST,
		},
		{
			ICMD_ICONST,
			ICMD_NOP,

			ICMD_ICONST,
		}
	},
};


/* patterns for a total number of 4 instructions ******************************/

#define IFCONV_PATTERN_4_SIZE    sizeof(ifconv_pattern_4) / (sizeof(s4) * 4 * 2)

static s4 ifconv_pattern_4[][2][4] = {
	/* PATTERN 1 */

	{
		{
			ICMD_ICONST,
			ICMD_IRETURN,

			ICMD_ICONST,
			ICMD_IRETURN
		},
		{
			ICMD_ICONST,
			ICMD_NOP,

			ICMD_ICONST,
			ICMD_IRETURN
		}
	},
};


/* ifconv_condition_complement *************************************************

   Table of conditions and their complement.  Index with:

   (ICMD_IFxx - ICMD_IFEQ)

   ATTENTION: Don't change order!  It depends on the Java bytecode opcode!

*******************************************************************************/
#if 0
static s4 ifconv_condition_complement[6] = {
	/* !ICMD_IFEQ */    ICMD_IFNE,
	/* !ICMD_IFNE */    ICMD_IFEQ,
	/* !ICMD_IFLT */    ICMD_IFGE,
	/* !ICMD_IFGE */    ICMD_IFLT,
	/* !ICMD_IFGT */    ICMD_IFLE,
	/* !ICMD_IFLE */    ICMD_IFGT,
};
#endif


/* ifconv_static ***************************************************************

   Does if-conversion with static data based on pattern matching.

*******************************************************************************/

static void check(jitdata *jd, basicblock *bptr);

bool ifconv_static(jitdata *jd)
{
	basicblock  *bptr;
	instruction *iptr;
	instruction *tiptr;
	s4           bcount;
	s4           icount = 0;
	s4          *pattern = NULL;
	s4           patternsize = 0;
	s4          *p;
	ICMD         opcode;
	u2           condition;
	//u2           complement;
	s4           i = 0;
	s4           j;

	/* get required compiler data */

#if !defined(NDEBUG)
	methodinfo *m  = jd->m;
#endif

	/* iterate over all basic blocks */

	bptr   = jd->basicblocks;
	bcount = jd->basicblockcount;

	for (; bcount >= 0; bcount--, bptr++) {
		/* Deleted basic blocks are just skipped. */

		if (bptr->state == basicblock::DELETED)
			continue;

		/* We need at least 3 basic blocks including the current one. */

		if (bcount < 3)
			continue;

		/* Only look at the last instruction of the current basic
		   block.  All conditional branch instructions are suitable
		   for if-conversion. */

		iptr   = bptr->iinstr + bptr->icount - 1;

		switch (iptr->opc) {
		case ICMD_IFNULL:
		case ICMD_IFNONNULL:

		case ICMD_IF_ICMPEQ:
		case ICMD_IF_ICMPNE:
		case ICMD_IF_ICMPLT:
		case ICMD_IF_ICMPGE:
		case ICMD_IF_ICMPGT:
		case ICMD_IF_ICMPLE:

		case ICMD_IFEQ:
		case ICMD_IFNE:
		case ICMD_IFLT:
		case ICMD_IFGE:
		case ICMD_IFGT:
		case ICMD_IFLE:

		case ICMD_LCMP:

		case ICMD_IF_LEQ:
		case ICMD_IF_LNE:
		case ICMD_IF_LLT:
		case ICMD_IF_LGE:
		case ICMD_IF_LGT:
		case ICMD_IF_LLE:

		case ICMD_IF_LCMPEQ:
		case ICMD_IF_LCMPNE:
		case ICMD_IF_LCMPLT:
		case ICMD_IF_LCMPGE:
		case ICMD_IF_LCMPGT:
		case ICMD_IF_LCMPLE:

		case ICMD_IF_ACMPEQ:
		case ICMD_IF_ACMPNE:
			/* basic blocks can only have 1 predecessor */

			if ((bptr[1].predecessorcount != 1) ||
				(bptr[2].predecessorcount != 1))
				break;

			check(jd, bptr);

			/* only use a fixed size of instructions */

			icount = bptr[1].icount + bptr[2].icount;

			/* we only convert less than or equal 4 instructions */

			if (icount > 4)
				break;

			/* check which pattern to use */

			switch (icount) {
			case 2:
				/* just skip basic blocks with length 1 */

				pattern     = NULL;
				patternsize = 0;
				break;

			case 3:
				pattern     = (s4 *) ifconv_pattern_3;
				patternsize = IFCONV_PATTERN_3_SIZE;
				break;

			case 4:
				pattern     = (s4 *) ifconv_pattern_4;
				patternsize = IFCONV_PATTERN_4_SIZE;
				break;

			default:
				/* keep compiler happy */

				pattern     = NULL;
				patternsize = 0;

				/* that should not happen */

				vm_abort("ifconv_static: invalid instruction count %d", icount);
			}

			/* Iterate over all patterns of the given pattern. */

			for (i = 0; i < patternsize; i++) {
				/* Check the first and the second basic block at the
				   same time.  The instructions _MUST NOT_ be
				   reordered in the array before. */

				tiptr = bptr[1].iinstr;

				for (j = 0; j < icount; j++, tiptr++) {
					/* get the opcode */

					p = pattern + (icount * 2 * i) + (icount * 0) + j;

					opcode = (ICMD) *p;

					if (tiptr->opc != opcode)
						goto nomatch;
				}

				/* found a matching pattern */

#if !defined(NDEBUG)
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[0]);
				show_basicblock(jd, &bptr[1]);
				show_basicblock(jd, &bptr[2]);
				show_basicblock(jd, &bptr[3]);
#endif
#endif

				/* check the condition */

				switch (iptr->opc) {
				case ICMD_IFEQ:
				case ICMD_IF_ICMPEQ:
				case ICMD_IF_LEQ:
				case ICMD_IF_LCMPEQ:
				case ICMD_IF_ACMPEQ:
				case ICMD_IFNULL:
					condition = ICMD_IFEQ;
					break;

				case ICMD_IFNE:
				case ICMD_IF_ICMPNE:
				case ICMD_IF_LNE:
				case ICMD_IF_LCMPNE:
				case ICMD_IF_ACMPNE:
				case ICMD_IFNONNULL:
					condition = ICMD_IFNE;
					break;

				case ICMD_IFLT:
				case ICMD_IF_ICMPLT:
				case ICMD_IF_LLT:
				case ICMD_IF_LCMPLT:
					condition = ICMD_IFLT;
					break;

				case ICMD_IFGE:
				case ICMD_IF_ICMPGE:
				case ICMD_IF_LGE:
				case ICMD_IF_LCMPGE:
					condition = ICMD_IFGE;
					break;

				case ICMD_IFGT:
				case ICMD_IF_ICMPGT:
				case ICMD_IF_LGT:
				case ICMD_IF_LCMPGT:
					condition = ICMD_IFGT;
					break;

				case ICMD_IFLE:
				case ICMD_IF_ICMPLE:
				case ICMD_IF_LLE:
				case ICMD_IF_LCMPLE:
					condition = ICMD_IFLE;
					break;

				case ICMD_LCMP:
					assert(0);

				default:
					/* keep compiler happy */

					condition = 0;

					vm_abort("ifconv_static: invalid opcode: %d", iptr->opc);
				}

				/* get the condition array index */

				//complement = ifconv_condition_complement[condition - ICMD_IFEQ];

				/* Set the new instructions, first basic block 1... */

				tiptr = bptr[1].iinstr;
				j = 0;

				for (; j < bptr[1].icount; j++, tiptr++) {
					/* get the replacing opcode */

					p = pattern + (icount * 2 * i) + (icount * 1) + j;

					opcode = (ICMD) *p;

					/* If we add a NOP, skip the current instruction
					   and set the stack of the next instruction
					   to the previous one. */

					if (opcode == ICMD_NOP) {
						tiptr[1].dst = tiptr[-1].dst;
					}

					/* For the first basic block we have to set the
					   complementary condition. */

/* 					tiptr->opc = opcode | (complement << 8); */
					tiptr->opc = opcode;
				}

				/* ...then basic block 2.  We split this step, as we
				   have to set different conditions in the blocks. */

				for (; j < bptr[1].icount + bptr[2].icount; j++, tiptr++) {
					p = pattern + (icount * 2 * i) + (icount * 1) + j;

					/* For the first basic block we have to set the
					   complementary condition. */

					tiptr->opc = (ICMD) (*p | (condition << 8));

					/* if we add a NOP, set the stacks correctly */

					if (tiptr->opc == ICMD_NOP) {
						assert(0);
					}
				}

				/* tag the conditional branch instruction as conditional */

				iptr->opc = (ICMD) (iptr->opc | (condition << 8));

				/* add the instructions to the current basic block */

				bptr->icount += icount;

#if !defined(NDEBUG)
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[0]);
#endif
#endif

				/* delete the 2 following basic blocks */

				bptr[1].state  = basicblock::DELETED;
				bptr[2].state  = basicblock::DELETED;
				bptr[1].icount = 0;
				bptr[2].icount = 0;

				/* we had a match, exit this iteration */

				break;

			default:
			nomatch:
				;
			}
		}
	}

	/* everything's ok */

	return true;
}

static void check(jitdata *jd, basicblock *bptr)
{
#if !defined(NDEBUG)
	int pattern = 0;

	/* get required compiler data */

	methodinfo *m  = jd->m;

	/*
	  generated by jikes.

	  java.lang.reflect.Modifier.isPublic(I)Z

	  [            ] L000(5 - 0) flags=1:
	  [         i00]     0 (line:   227)  ICONST          0 (0x00000000)
	  [     l00 i00]     1 (line:   227)  ILOAD           0
	  [     r15 i00]     2 (line:   227)  IANDCONST       1 (0x00000001)
	  [     r15 i00]     3 (line:   227)  NOP
	  [         i00]     4 (line:   227)  IFEQ            0 (0x00000000) L002

	  [         i00] L001(2 - 1) flags=1:
	  [            ]     0 (line:   227)  POP
	  [         i00]     1 (line:   227)  ICONST          1 (0x00000001)

	  [         i00] L002(1 - 2) flags=1:
	  [            ]     0 (line:   227)  IRETURN
	*/

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_POP) &&
		(bptr[1].iinstr[1].opc == ICMD_ICONST))
		{
			pattern = 1;
		}

	/*
	  generated by ecj.

	  java.util.Hashtable.isEmpty()Z PUBLIC SYNCHRONIZED

	  [    ] L000(3 - 0) flags=1:
	  [ l00]     0 (line:   292)  ALOAD           0
	  [ rdi]     1 (line:   292)  GETFIELD        36, java.util.Hashtable.size (type I
	  )
	  [    ]     2 (line:   292)  IFNE            0 (0x00000000) L002

	  [    ] L001(2 - 1) flags=1:
	  [ rdi]     0 (line:   292)  ICONST          1 (0x00000001)
	  [    ]     1 (line:   292)  IRETURN

	  [    ] L002(2 - 1) flags=1:
	  [ rdi]     0 (line:   292)  ICONST          0 (0x00000000)
	  [    ]     1 (line:   292)  IRETURN
	*/

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[1].opc == ICMD_IRETURN) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[2].iinstr[1].opc == ICMD_IRETURN))
		{
			pattern = 2;
		}

	/*
	  this seems to be the most common and simplest if, check for all types
	*/

	/* xCONST */

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ICONST))
		{
			pattern = 3;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_LCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LCONST))
		{
			pattern = 4;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ACONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ACONST))
		{
			pattern = 5;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_FCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_FCONST))
		{
			pattern = 6;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_DCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_DCONST))
		{
			pattern = 7;
		}

	/* xLOAD */


	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ILOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ILOAD))
		{
			pattern = 8;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_LLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LLOAD))
		{
			pattern = 9;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD))
		{
			pattern = 10;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_FLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_FLOAD))
		{
			pattern = 11;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_DLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_DLOAD))
		{
			pattern = 12;
		}

	/* xCONST, GOTO - xLOAD */

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ILOAD))
		{
			pattern = 13;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_LCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LLOAD))
		{
			pattern = 14;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ACONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD))
		{
			pattern = 15;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_FCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_FLOAD))
		{
			pattern = 16;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_DCONST) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_DLOAD))
		{
			pattern = 17;
		}

	/* xLOAD, GOTO - xCONST */

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ILOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ICONST))
		{
			pattern = 18;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_LLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LCONST))
		{
			pattern = 19;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ACONST))
		{
			pattern = 20;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_FLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_FCONST))
		{
			pattern = 21;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_DLOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_DCONST))
		{
			pattern = 22;
		}

	/*
	  check for different ISTORE destinations or handle them properly
	*/

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[1].opc == ICMD_ISTORE) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[2].iinstr[1].opc == ICMD_ISTORE))
		{
			pattern = 23;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ILOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_ISTORE) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_ILOAD) &&
		(bptr[2].iinstr[1].opc == ICMD_ISTORE))
		{
			pattern = 24;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[1].opc == ICMD_ISTORE) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_ILOAD) &&
		(bptr[2].iinstr[1].opc == ICMD_ISTORE))
		{
			pattern = 25;
		}


	/* ALOAD, GETFIELD - ALOAD, GETFIELD */

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[2].iinstr[1].opc == ICMD_GETFIELD))
		{
			pattern = 26;
		}


	/* ALOAD, ICONST, PUTFIELD - ALOAD, ICONST, PUTFIELD */

	if ((bptr[1].icount == 4) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_ICONST) &&
		(bptr[1].iinstr[2].opc == ICMD_PUTFIELD) &&
		(bptr[1].iinstr[3].opc == ICMD_GOTO) &&

		(bptr[2].icount == 3) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[2].iinstr[1].opc == ICMD_ICONST) &&
		(bptr[2].iinstr[2].opc == ICMD_PUTFIELD)
		)
		{
			pattern = 27;
		}

	if ((bptr[1].icount == 4) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_ASTORE) &&
		(bptr[1].iinstr[3].opc == ICMD_GOTO) &&

		(bptr[2].icount == 3) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[2].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[2].iinstr[2].opc == ICMD_ASTORE)
		)
		{
			pattern = 28;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_GETSTATIC) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_GETSTATIC))
		{
			pattern = 29;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_GETSTATIC) &&
		(bptr[1].iinstr[1].opc == ICMD_ASTORE) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 2) &&
		(bptr[2].iinstr[0].opc == ICMD_GETSTATIC) &&
		(bptr[2].iinstr[1].opc == ICMD_ASTORE))
		{
			pattern = 30;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_GETSTATIC) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD))
		{
			pattern = 31;
		}

	if ((bptr[1].icount == 2) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_GETSTATIC))
		{
			pattern = 32;
		}


	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ICONST))
		{
			pattern = 33;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LCONST))
		{
			pattern = 34;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ACONST))
		{
			pattern = 35;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ILOAD))
		{
			pattern = 36;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_LLOAD))
		{
			pattern = 37;
		}

	if ((bptr[1].icount == 3) &&
		(bptr[1].iinstr[0].opc == ICMD_ALOAD) &&
		(bptr[1].iinstr[1].opc == ICMD_GETFIELD) &&
		(bptr[1].iinstr[2].opc == ICMD_GOTO) &&

		(bptr[2].icount == 1) &&
		(bptr[2].iinstr[0].opc == ICMD_ALOAD))
		{
			pattern = 38;
		}

	/*

	CHECK 3   : (BB:   0) IFEQ            javax.swing.plaf.basic.BasicInternalFrameTitlePane.paintTitleBackground(Ljava/awt/Graphics;)V PROTECTED
	[ ?   ?   ?   ?   ?  ] L001(instruction count: 4, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:   909)  ALOAD           0
	[ ?   ?   ?   ?   ?  ]     1 (line:   909)  GETFIELD        (NOT RESOLVED), javax.swing.plaf.basic.BasicInternalFrameTitlePane.selectedTitleColor (type Ljava/awt/Color;)
	[ ?   ?   ?   ?   ?  ]     2 (line:   909)  ASTORE          4
	[ ?   ?   ?   ?   ?  ]     3 (line:   909)  GOTO            op1=41
	[ ?   ?   ?   ?   ?  ] L002(instruction count: 3, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:   911)  ALOAD           0
	[ ?   ?   ?   ?   ?  ]     1 (line:   911)  GETFIELD        (NOT RESOLVED), javax.swing.plaf.basic.BasicInternalFrameTitlePane.notSelectedTitleColor (type Ljava/awt/Color;)
	[ ?   ?   ?   ?   ?  ]     2 (line:   911)  ASTORE          4


	CHECK 3   : (BB:   3) IFEQ            javax.swing.plaf.basic.BasicTreeUI$MouseHandler.mousePressed(Ljava/awt/event/MouseEvent;)V PUBLIC
	[ ?   ?   ?   ?   ?  ] L004(instruction count: 4, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:  2244)  ACONST          0x3602d30, String = "Tree.openIcon"
	[ ?   ?   ?   ?   ?  ]     1 (line:  2244)  INVOKESTATIC    (NOT RESOLVED) javax.swing.UIManager.getIcon(Ljava/lang/Object;)Ljavax/swing/Icon;
	[ ?   ?   ?   ?   ?  ]     2 (line:  2244)  ASTORE          8
	[ ?   ?   ?   ?   ?  ]     3 (line:  2244)  GOTO            op1=155
	[ ?   ?   ?   ?   ?  ] L005(instruction count: 3, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:  2246)  ACONST          0x3602e00, String = "Tree.closedIcon"
	[ ?   ?   ?   ?   ?  ]     1 (line:  2246)  INVOKESTATIC    (NOT RESOLVED) javax.swing.UIManager.getIcon(Ljava/lang/Object;)Ljavax/swing/Icon;
	[ ?   ?   ?   ?   ?  ]     2 (line:  2246)  ASTORE          8


	CHECK 3   : (BB:   2) IFEQ            javax.naming.CompoundName.initializeSyntax()V PRIVATE FINAL
	[ ?   ?   ?   ?  ] L003(instruction count: 4, predecessors: 1):
	[ ?   ?   ?   ?  ]     0 (line:   445)  ALOAD           0
	[ ?   ?   ?   ?  ]     1 (line:   445)  ICONST          1 (0x00000001)
	[ ?   ?   ?   ?  ]     2 (line:   445)  PUTFIELD        (NOT RESOLVED), javax.naming.CompoundName.direction (type I)
	[ ?   ?   ?   ?  ]     3 (line:   445)  GOTO            op1=51
	[ ?   ?   ?   ?  ] L004(instruction count: 3, predecessors: 1):
	[ ?   ?   ?   ?  ]     0 (line:   449)  ALOAD           0
	[ ?   ?   ?   ?  ]     1 (line:   449)  ICONST          0 (0x00000000)
	[ ?   ?   ?   ?  ]     2 (line:   449)  PUTFIELD        (NOT RESOLVED), javax.naming.CompoundName.direction (type I)


	CHECK 3   : (BB:  15) IFNE            java.awt.Scrollbar.setValues(IIII)V PUBLIC SYNCHRONIZED
	[ ?   ?   ?   ?   ?  ] L016(instruction count: 4, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:   371)  ALOAD           0
	[ ?   ?   ?   ?   ?  ]     1 (line:   371)  ICONST          1 (0x00000001)
	[ ?   ?   ?   ?   ?  ]     2 (line:   371)  PUTFIELD        (NOT RESOLVED), java.awt.Scrollbar.lineIncrement (type I)
	[ ?   ?   ?   ?   ?  ]     3 (line:   371)  GOTO            op1=152
	[ ?   ?   ?   ?   ?  ] L017(instruction count: 3, predecessors: 1):
	[ ?   ?   ?   ?   ?  ]     0 (line:   373)  ALOAD           0
	[ ?   ?   ?   ?   ?  ]     1 (line:   373)  ILOAD           6
	[ ?   ?   ?   ?   ?  ]     2 (line:   373)  PUTFIELD        (NOT RESOLVED), java.awt.Scrollbar.lineIncrement (type I)


	CHECK 3   : (BB:   1) IFEQ            javax.swing.JInternalFrame.setIcon(Z)V PUBLIC
	[ ?   ?   ?   ?  ] L002(instruction count: 4, predecessors: 1):
	[ ?   ?   ?   ?  ]     0 (line:  1395)  ALOAD           0
	[ ?   ?   ?   ?  ]     1 (line:  1395)  ICONST          25552 (0x000063d0)
	[ ?   ?   ?   ?  ]     2 (line:  1395)  INVOKEVIRTUAL   (NOT RESOLVED) javax.swing.JInternalFrame.fireInternalFrameEvent(I)V
	[ ?   ?   ?   ?  ]     3 (line:  1395)  GOTO            op1=61
	[ ?   ?   ?   ?  ] L003(instruction count: 3, predecessors: 1):
	[ ?   ?   ?   ?  ]     0 (line:  1397)  ALOAD           0
	[ ?   ?   ?   ?  ]     1 (line:  1397)  ICONST          25553 (0x000063d1)
	[ ?   ?   ?   ?  ]     2 (line:  1397)  INVOKEVIRTUAL   (NOT RESOLVED) javax.swing.JInternalFrame.fireInternalFrameEvent(I)V

	*/

	if (pattern != 0) {
		printf("PATTERN %02d: (BB: %3d) ", pattern, jd->basicblockcount - bptr->nr);
		method_println(m);

		/* 								if (pattern == 27) { */
		/* 									show_basicblock(m, cd, &bptr[1]); */
		/* 									show_basicblock(m, cd, &bptr[2]); */
		/* 								} */

		fflush(stdout);

	} else {
		if ((bptr[1].icount == 2) &&
			(bptr[2].icount == 1) &&

			(bptr[1].iinstr[1].opc == ICMD_GOTO))
			{
				printf("CHECK 1   : (BB: %3d) ", jd->basicblockcount - bptr->nr);
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[1]);
				show_basicblock(jd, &bptr[2]);
#endif
				fflush(stdout);
			}

		if ((bptr[1].icount == 3) &&
			(bptr[2].icount == 2) &&

			(bptr[1].iinstr[2].opc == ICMD_GOTO))
			{
				printf("CHECK 2   : (BB: %3d) ", jd->basicblockcount - bptr->nr);
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[1]);
				show_basicblock(jd, &bptr[2]);
#endif
				fflush(stdout);
			}

		if ((bptr[1].icount == 4) &&
			(bptr[2].icount == 3) &&

			(bptr[1].iinstr[3].opc == ICMD_GOTO))
			{
				printf("CHECK 3   : (BB: %3d) ", jd->basicblockcount - bptr->nr);
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[1]);
				show_basicblock(jd, &bptr[2]);
#endif
				fflush(stdout);
			}

		if ((bptr[1].icount == 3) &&
			(bptr[2].icount == 1) &&

			(bptr[1].iinstr[2].opc == ICMD_GOTO))
			{
				printf("CHECK 4   : (BB: %3d) ", jd->basicblockcount - bptr->nr);
				method_println(m);
#if 0
				show_basicblock(jd, &bptr[1]);
				show_basicblock(jd, &bptr[2]);
#endif
				fflush(stdout);
			}
	}
#endif /* !defined(NDEBUG) */
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
 * vim:noexpandtab:sw=4:ts=4:
 */
