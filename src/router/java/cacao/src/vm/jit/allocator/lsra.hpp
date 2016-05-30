/* src/vm/jit/allocator/lsra.hpp - linear scan register allocator header

   Copyright (C) 2005, 2006, 2008
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


#ifndef LSRA_HPP_
#define LSRA_HPP_ 1

#include "config.h"

#if !defined(NDEBUG)
# include <assert.h>
# define LSRA_DEBUG_CHECK
# define LSRA_DEBUG_VERBOSE
#endif

#ifdef SSA_DEBUG_CHECK
# define _LSRA_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
# define _LSRA_ASSERT(a) assert((a));
#else
# define _LSRA_CHECK_BOUNDS(i,l,h)
# define _LSRA_ASSERT(a)
#endif

/* #define LSRA_DEBUG  */ /* lsra debug messages */
/* #define LSRA_SAVEDVAR */
/* #define LSRA_MEMORY */
#if defined(__I386__) || defined(__X86_64__)
#define JOIN_DEST_STACK           /* The destination stackslot gets the same  */
        /* register as one of the src stackslots. Important for i386 & X86_64 */
        /* since they do not have "3 operand" arithmetic instructions to      */
        /* prevent usage of a reserved register (REG_ITMPX)                   */
#endif
#define JOIN_DUP_STACK         /* join "identical" stackslots created by dup* */

#define USAGE_COUNT        /* influence LSRA with usagecount */
#define USEAGE_COUNT_EXACT /* search all active lifetimes and regard */
                           /* usage_count */
#define USAGE_PER_INSTR    /* divide usagecount by lifetimelength */

#define LSRA_BB_IN 3
#define LSRA_BB_OUT 2
#define LSRA_STORE 1
#define LSRA_LOAD 0
#define LSRA_POP -1

/* join types and flags*/
#define JOIN     0 /* joins that are not in any way dangerous                 */
#define JOIN_BB  1 /* join Stackslots over Basic Block Boundaries             */
#define JOIN_DUP 2 /* join of two possibly concurring lifeteimes through DUP* */
#define JOIN_OP  4 /* join of src operand with dst operand on i386 and x86_64 */
                   /* architecture                                            */
                   /* JOIN_DUP and JOIN_OP is mutually exclusive as JOIN_OP   */
                   /* and JOIN_BB                                             */
#define JOINING  8 /* set while joining for DUP or OP to prevent assignement  */
                   /* to a REG_RES before all involved lifetimes have been    */
                   /* seen completely */

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)<(b)?(b):(a))

struct _list {
	int value;
	struct _list *next;
};

struct _backedge {
	int start;
	int end;
	int nesting;
	struct _backedge *next;
};

struct lifetime {
	int i_start;                /* instruction number of first use */
	int i_end;                  /* instruction number of last use */
	int v_index;           /* local variable index or negative for stackslots */
	int type;                   /* TYPE_* or -1 for unused lifetime */
	long usagecount;            /* number of references*/
	int reg;                    /* regoffset allocated by lsra*/
	int savedvar;
	int flags;
	struct stackslot *local_ss; /* Stackslots for this Lifetime or NULL ( ==  */
                                /* "pure" Local Var) */
	int bb_last_use;
	int bb_first_def;
	int i_last_use;
	int i_first_def;
};

struct l_loop {
	int b_first;
	int b_last;
	int nesting;
};

struct b_loop {
	int loop;
	int instr;
};


struct stackslot {
	stackelement_t* s;
	int bb;
	struct stackslot *next;
};

struct lsra_register {
	int *sav_reg;
	int *tmp_reg;
	int *nregdesc;
	int sav_top;
	int tmp_top;
};

struct lsra_reg {
	int reg_index;
	int use;
};

struct _sbr {
	int header;          /* BB Index of subroutine start (SBR_HEADER) */
	struct _list *ret;   /* List of possible return BB indizes */
	struct _sbr *next;
};

struct lsradata {
#if defined(LSRA_USES_REG_RES)
	int reg_res_free[REG_RES_CNT];
#endif
	struct _list **succ; /* CFG successors*/
	struct _list **pred; /* CFG predecessors */
	int *num_pred;       /* CFG number of predecessors */
	int *sorted;         /* BB sorted in reverse post order */
	int *sorted_rev;     /* BB reverse lookup of sorted */

	struct _backedge **backedge; /* backedge data structure */
	int backedge_count;          /* number of backedges */

	struct _sbr sbr;     /* list of subroutines, sorted by header */

	long *nesting;    /* Nesting level of BB*/

	int maxlifetimes; /* copy from methodinfo to prevent passing methodinfo   */
                      /* as parameter */

	struct lifetime *lifetime; /* array of lifetimes */
	int lifetimecount;         /* number of lifetimes */
	int *lt_used;              /* index to lifetimearray for used lifetimes   */
	int *lt_int;               /* index to lifetimearray for int lifetimes    */
	int lt_int_count;          /* number of int/[lng]/[adr] lifetimes */
	int *lt_flt;               /* index to lifetimearray for float lifetimes  */
	int lt_flt_count;          /* number of float/double lifetimes */
	int *lt_mem;               /* index to lifetimearray for all lifetimes    */
                               /* not to be allocated in registers */
	int lt_mem_count;          /* number of this other lifetimes */

	struct lifetime **active_tmp, **active_sav;
	int active_tmp_top, active_sav_top;

	struct lsra_exceptiontable *ex;
	int v_index;               /* next free index for stack slot lifetimes    */
	                           /* decrements from -1 */
	int *icount_block;
};

struct freemem {
	int off;
	int end;
	struct freemem *next;
};

typedef struct lsradata lsradata;


/* function prototypes ********************************************************/

bool lsra(jitdata *jd);

#endif // LSRA_HPP_


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
