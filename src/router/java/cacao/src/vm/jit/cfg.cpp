/* src/vm/jit/cfg.c - build a control-flow graph

   Copyright (C) 2006, 2007 R. Grafl, A. Krall, C. Kruegel, C. Oates,
   R. Obermaisser, M. Platter, M. Probst, S. Ring, E. Steiner,
   C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich, J. Wenninger,
   J. Wenninger, Institut f. Computersprachen - TU Wien

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

#include "vm/jit/cfg.hpp"
#include <assert.h>                     // for assert
#include "config.h"
#include "mm/dumpmemory.hpp"            // for DMNEW
#include "vm/jit/ir/icmd.hpp"           // for ::ICMD_NOP, etc
#include "vm/jit/ir/instruction.hpp"    // for instruction, etc
#include "vm/jit/jit.hpp"               // for basicblock, jitdata, etc
#include "vm/jit/stack.hpp"


/* cfg_allocate_predecessors ***************************************************

   Allocates the predecessor array, if there is none, and resets the
   predecessor count.

*******************************************************************************/

static void cfg_allocate_predecessors(basicblock *bptr)
{
	if (bptr->predecessors == NULL) {
		bptr->predecessors = DMNEW(basicblock*, bptr->predecessorcount);

		bptr->predecessorcount = 0;
	}
}


/* cfg_allocate_successors *****************************************************

   Allocates the succecessor array, if there is none, and resets the
   predecessor count.

*******************************************************************************/

static void cfg_allocate_successors(basicblock *bptr)
{
	if (bptr->successors == NULL) {
		bptr->successors = DMNEW(basicblock*, bptr->successorcount);

		bptr->successorcount = 0;
	}
}


/* cfg_insert_predecessor ******************************************************

   Inserts a predecessor into the array, but checks for duplicate
   entries.  This is used for TABLESWITCH and LOOKUPSWITCH.

*******************************************************************************/

static void cfg_insert_predecessors(basicblock *bptr, basicblock *pbptr)
{
	basicblock **tbptr;
	int          i;

	tbptr = bptr->predecessors;

	/* check if the predecessors is already stored in the array */

	for (i = 0; i < bptr->predecessorcount; i++, tbptr++)
		if (*tbptr == pbptr)
			return;

	/* not found, insert it */

	bptr->predecessors[bptr->predecessorcount] = pbptr;
	bptr->predecessorcount++;
}

static void cfg_insert_successors(basicblock *bptr, basicblock *pbptr)
{
	basicblock **tbptr;
	int          i;

	tbptr = bptr->successors;

	/* check if the successor is already stored in the array */

	for (i = 0; i < bptr->successorcount; i++, tbptr++)
		if (*tbptr == pbptr)
			return;

	/* not found, insert it */

	bptr->successors[bptr->successorcount] = pbptr;
	bptr->successorcount++;
}


/* cfg_build *******************************************************************

   Build a control-flow graph in finding all predecessors and
   successors for the basic blocks.

*******************************************************************************/

bool cfg_build(jitdata *jd)
{
	basicblock      *bptr;
	basicblock      *tbptr;
	basicblock      *ntbptr;
	instruction     *iptr;
	branch_target_t *table;
	lookup_target_t *lookup;
	int              i;
	bool             has_fallthrough;

	/* process all basic blocks to find the predecessor/successor counts */

	bptr = jd->basicblocks;

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {

		if (bptr->type == basicblock::TYPE_EXH) {
			/* predecessorcount for exception handlers is initialized to -1,
			   so we need to fix it to 0. */
			bptr->predecessorcount += 1;
		}

		if ((bptr->icount == 0) || (bptr->state == basicblock::UNDEF))
			continue;

		iptr = bptr->iinstr + bptr->icount - 1;

		/* skip NOPs at the end of the block */

		while (iptr->opc == ICMD_NOP) {
			if (iptr == bptr->iinstr)
				break;
			iptr--;
		}

		if (iptr->opc == ICMD_GOTO) {

			/*
			 This is needed for the following special case caused by
			 stack_reach_next_block:
			 I.e. there might be instructions causing control flow before
			 a GOTO:

			 ....
			 129: 192:  IFEQ            Ti102 0 (0x00000000) --> L052
			 131: 193:  NOP
			   0:   0:  GOTO            --> L060
			*/

			bptr->successorcount++;

			tbptr = iptr->dst.block;
			tbptr->predecessorcount++;

			if (iptr == bptr->iinstr) {
				continue;
			}

			iptr--;

			while (iptr->opc == ICMD_NOP) {
				if (iptr == bptr->iinstr) {
					break;
				}
				iptr--;
			}

			has_fallthrough = false;
		} else {
			has_fallthrough = true;
		}

		switch (icmd_table[iptr->opc].controlflow) {

		case CF_END:
			break;

		case CF_IF:

			bptr->successorcount += 2;

			tbptr  = iptr->dst.block;
			tbptr->predecessorcount++;

			if (has_fallthrough) {
				ntbptr = bptr->next;
				ntbptr->predecessorcount++;
			}
			break;

		case CF_JSR:
			bptr->successorcount++;

			tbptr = iptr->sx.s23.s3.jsrtarget.block;
			tbptr->predecessorcount++;
			break;

		case CF_GOTO:
		case CF_RET:
			bptr->successorcount++;

			tbptr = iptr->dst.block;
			tbptr->predecessorcount++;
			break;

		case CF_TABLE:
			table = iptr->dst.table;

			bptr->successorcount++;

			tbptr = table->block;
			tbptr->predecessorcount++;
			table++;

			i = iptr->sx.s23.s3.tablehigh - iptr->sx.s23.s2.tablelow + 1;

			while (--i >= 0) {
				bptr->successorcount++;

				tbptr = table->block;
				tbptr->predecessorcount++;
				table++;
			}
			break;

		case CF_LOOKUP:
			lookup = iptr->dst.lookup;

			bptr->successorcount++;

			tbptr = iptr->sx.s23.s3.lookupdefault.block;
			tbptr->predecessorcount++;

			i = iptr->sx.s23.s2.lookupcount;

			while (--i >= 0) {
				bptr->successorcount++;

				tbptr = lookup->target.block;
				tbptr->predecessorcount++;
				lookup++;
			}
			break;

		default:
			if (has_fallthrough) {
				bptr->successorcount++;

				tbptr = bptr->next;

				/* An exception handler has no predecessors. */

				if (tbptr->type != basicblock::TYPE_EXH)
					tbptr->predecessorcount++;
			}
			break;
		}
	}

	/* Second iteration to allocate the arrays and insert the basic
	   block pointers. */

	bptr = jd->basicblocks;

	for (bptr = jd->basicblocks; bptr != NULL; bptr = bptr->next) {
		if ((bptr->icount == 0) || (bptr->state == basicblock::UNDEF))
			continue;

		iptr = bptr->iinstr + bptr->icount - 1;

		/* skip NOPs at the end of the block */

		while (iptr->opc == ICMD_NOP) {
			if (iptr == bptr->iinstr)
				break;
			iptr--;
		}

		if (iptr->opc == ICMD_GOTO) {
			tbptr = iptr->dst.block;

			cfg_allocate_successors(bptr);

			cfg_insert_successors(bptr, tbptr);

			cfg_allocate_predecessors(tbptr);

			cfg_insert_predecessors(tbptr, bptr);

			if (iptr == bptr->iinstr) {
				continue;
			}

			iptr--;

			while (iptr->opc == ICMD_NOP) {
				if (iptr == bptr->iinstr) {
					break;
				}
				iptr--;
			}

			has_fallthrough = false;

		} else {
			has_fallthrough = true;
		}

		switch (icmd_table[iptr->opc].controlflow) {

		case CF_END:
			break;

		case CF_IF:

			tbptr  = iptr->dst.block;
			ntbptr = bptr->next;

			cfg_allocate_successors(bptr);

			cfg_insert_successors(bptr, tbptr);
			if (has_fallthrough) {
				cfg_insert_successors(bptr, ntbptr);
			}

			cfg_allocate_predecessors(tbptr);
			if (has_fallthrough) {
				cfg_allocate_predecessors(ntbptr);
			}

			cfg_insert_predecessors(tbptr, bptr);
			if (has_fallthrough) {
				cfg_insert_predecessors(ntbptr, bptr);
			}
			break;

		case CF_JSR:
			tbptr = iptr->sx.s23.s3.jsrtarget.block;
			goto goto_tail;

		case CF_GOTO:
		case CF_RET:

			tbptr = iptr->dst.block;
goto_tail:
			cfg_allocate_successors(bptr);

			cfg_insert_successors(bptr, tbptr);

			cfg_allocate_predecessors(tbptr);

			cfg_insert_predecessors(tbptr, bptr);
			break;

		case CF_TABLE:
			table = iptr->dst.table;

			tbptr = table->block;
			table++;

			cfg_allocate_successors(bptr);

			cfg_insert_successors(bptr, tbptr);

			cfg_allocate_predecessors(tbptr);

			cfg_insert_predecessors(tbptr, bptr);

			i = iptr->sx.s23.s3.tablehigh - iptr->sx.s23.s2.tablelow + 1;

			while (--i >= 0) {
				tbptr = table->block;
				table++;

				cfg_insert_successors(bptr, tbptr);

				cfg_allocate_predecessors(tbptr);
				cfg_insert_predecessors(tbptr, bptr);
			}
			break;

		case CF_LOOKUP:
			lookup = iptr->dst.lookup;

			tbptr = iptr->sx.s23.s3.lookupdefault.block;

			cfg_allocate_successors(bptr);

			cfg_insert_successors(bptr, tbptr);

			cfg_allocate_predecessors(tbptr);

			cfg_insert_predecessors(tbptr, bptr);

			i = iptr->sx.s23.s2.lookupcount;

			while (--i >= 0) {
				tbptr = lookup->target.block;
				lookup++;

				cfg_insert_successors(bptr, tbptr);

				cfg_allocate_predecessors(tbptr);
				cfg_insert_predecessors(tbptr, bptr);
			}
			break;

		default:
			if (has_fallthrough) {
				tbptr = bptr->next;

				cfg_allocate_successors(bptr);

				bptr->successors[0] = tbptr;
				bptr->successorcount++;

				/* An exception handler has no predecessors. */

				if (tbptr->type != basicblock::TYPE_EXH) {
					cfg_allocate_predecessors(tbptr);

					tbptr->predecessors[tbptr->predecessorcount] = bptr;
					tbptr->predecessorcount++;
				}
			}
			break;
		}
	}

	/* everything's ok */

	return true;
}

/* cfg_add_root ****************************************************************

   Adds an empty root basicblock.
   The numbers of all other basicblocks are set off by one.
   Needed for some analyses that require the root basicblock to have no
   predecessors and to perform special initializations.

*******************************************************************************/

void cfg_add_root(jitdata *jd) {
	basicblock *root, *zero, *it;

	zero = jd->basicblocks;

	root = DNEW(basicblock);
	MZERO(root, basicblock, 1);

	root->successorcount = 1;
	root->successors     = DMNEW(basicblock *, 1);
	root->successors[0]  = zero;
	root->next           = zero;
	root->nr             = 0;
	root->type           = basicblock::TYPE_STD;
	root->method         = jd->m;

	if (zero->predecessorcount == 0) {
		zero->predecessors = DNEW(basicblock *);
	} else {
		zero->predecessors = DMREALLOC(zero->predecessors, basicblock *, zero->predecessorcount, zero->predecessorcount + 1);
	}
	zero->predecessors[zero->predecessorcount] = root;
	zero->predecessorcount += 1;

	jd->basicblocks = root;
	jd->basicblockcount += 1;

	for (it = zero; it; it = it->next) {
		it->nr += 1;
	}
}

void cfg_remove_root(jitdata *jd) {
	basicblock *root, *zero, *it;

	root = jd->basicblocks;
	zero = root->next;

	zero->predecessorcount -= 1;

	jd->basicblocks = zero;

	for (it = zero; it; it = it->next) {
		it->nr -= 1;
	}
}

void cfg_clear(jitdata *jd)
{

	for (basicblock *b = jd->basicblocks; b != NULL; b = b->next)
	{
		b->predecessorcount = (b->type == basicblock::TYPE_EXH) ? -1 : 0;
		b->successorcount   = 0;
		b->predecessors     = NULL;
		b->successors       = NULL;
	}
}

#if defined(ENABLE_SSA)

static void cfg_eliminate_edges_to_unreachable(jitdata *jd);

/* cfg_add_exceptional_edges ***************************************************

   Edges from basicblocks to their exception handlers and from exception
   handlers to the blocks they handle exceptions for are added. Further
   the number of potentially throwing instructions in the basicblocks are
   counted.

   We don't consider nor do we determine the types of exceptions thrown. Edges
   are added from every block to every potential handler.

*******************************************************************************/

void cfg_add_exceptional_edges(jitdata *jd) {
	basicblock *bptr;
	instruction *iptr;
	exception_entry *ee;
	bool unreachable_exh = false;

	/* Count the number of exceptional exits for every block.
	 * Every PEI is an exceptional out.
	 */

	FOR_EACH_BASICBLOCK(jd, bptr) {

		/* Prepare for reachability calculation. */
		bptr->vp = NULL;

		if (bptr->state == basicblock::UNDEF) {
			continue;
		}

		FOR_EACH_INSTRUCTION(bptr, iptr) {
			if (icmd_table[iptr->opc].flags & ICMDTABLE_PEI) {
				bptr->exouts += 1;
			}
		}
	}

	/* Count the number of exception handlers for every block. */

	for (ee = jd->exceptiontable; ee; ee = ee->down) {
		for (bptr = ee->start; bptr != ee->end; bptr = bptr->next) {
			/* Linking a block with a handler, even if there are no exceptional exits
			   breaks stuff in other passes. */
			if (bptr->exouts > 0) {
				bptr->exhandlercount += 1;
				ee->handler->expredecessorcount += 1;
			}
		}
	}

	/* Allocate and fill exception handler arrays. */

	for (ee = jd->exceptiontable; ee; ee = ee->down) {

		if (ee->handler->expredecessorcount == 0) {
			/* An exception handler that is unreachable.
			   This is inconsistent with the semantics of the CFG,
			   we need to recalculate reachability. */
			unreachable_exh = true;
		}

		for (bptr = ee->start; bptr != ee->end; bptr = bptr->next) {
			if (bptr->exouts > 0) {

				if (bptr->exhandlers == NULL) {
					bptr->exhandlers = DMNEW(basicblock *, bptr->exhandlercount);
					/* Move pointer past the end of the array,
					 * It will be filled in the reverse order.
					 */
					bptr->exhandlers += bptr->exhandlercount;
				}

				bptr->exhandlers -= 1;
				*(bptr->exhandlers) = ee->handler;

				if (ee->handler->expredecessors == NULL) {
					ee->handler->expredecessors = DMNEW(basicblock *, ee->handler->expredecessorcount);
					ee->handler->expredecessors += ee->handler->expredecessorcount;
				}

				ee->handler->expredecessors -= 1;
				*(ee->handler->expredecessors) = bptr;
			}
		}
	}

	if (unreachable_exh) {

		/* This is rare in ``normal'' compiler generated code.

		   The dead block [EXH] is a predecessor of [BB1],
		   but the edge [EXH] -> [BB1] will never be traversed.

		   [BB1] --[next]--> [BB2, no peis] ==[exhandler]==> [EXH] --[next]--+
             ^                                                               |
		     +---------------------------------------------------------------+
		*/

		/*
		fprintf(stderr, "Found unreachable exh, adjusting %s %s",
			UTF_TEXT(jd->m->klazz->name), UTF_TEXT(jd->m->name));
		fprintf(stderr, "<before>\n");
		show_method(jd, 3);
		fprintf(stderr, "</before>\n");
		*/

		cfg_eliminate_edges_to_unreachable(jd);

		/*
		fprintf(stderr, "<after>\n");
		show_method(jd, 3);
		fprintf(stderr, "</after>\n");
		*/
	}
}

static void cfg_calculate_reachability(basicblock *bptr) {
	basicblock **itsucc;

	/* Block not marked. */
	assert(bptr->vp == NULL);

	bptr->vp = bptr; /* Mark block */

	FOR_EACH_SUCCESSOR(bptr, itsucc) {
		if ((*itsucc)->vp == NULL) {
			cfg_calculate_reachability(*itsucc);
		}
	}

	if (bptr->exouts > 0) {
		FOR_EACH_EXHANDLER(bptr, itsucc) {
			if ((*itsucc)->vp == NULL) {
				cfg_calculate_reachability(*itsucc);
			}
		}
	}
}

static void cfg_remove_predecessors(basicblock *bptr, basicblock *pbptr) {
	s4 i;

	for (i = 0; i < bptr->predecessorcount; ++i) {
		/* Search item. */
		if (bptr->predecessors[i] == pbptr) {
			if (i != (bptr->predecessorcount - 1)) {
				/* If not last element, replace element with last element. */
				bptr->predecessors[i] = bptr->predecessors[bptr->predecessorcount - 1];
			}

			/* Decrease element count. */
			bptr->predecessorcount -= 1;

			return;
		}
	}
}

static void cfg_eliminate_edges_to_unreachable(jitdata *jd) {
	basicblock *it;
	basicblock **itsucc;

	cfg_calculate_reachability(jd->basicblocks);

	FOR_EACH_BASICBLOCK(jd, it) {
		if (it->vp == NULL) {

			/* Mark as unreachable. */

			it->state = basicblock::UNDEF;

			/* As this block got unreachable, it is no more a predecessor
			   of its successors. */

			FOR_EACH_SUCCESSOR(it, itsucc) {
				cfg_remove_predecessors(*itsucc, it);
			}

			/* Eliminiate all CFG edges of this block. */

			it->predecessorcount = 0;
			it->successorcount = 0;
			it->expredecessorcount = 0;
		}
	}
}

#endif

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
