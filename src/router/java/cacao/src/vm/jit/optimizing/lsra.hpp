/* src/vm/jit/optimizing/lsra.hpp - linear scan register allocator header

   Copyright (C) 2005-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   Contact: cacao@complang.tuwien.ac.at

   Authors: Christian Ullrich

   $Id: lsra.h,v 1.17 2005/11/22 14:36:16 christian Exp $

*/


#ifndef LSRA_HPP_
#define LSRA_HPP_ 1

#include "toolbox/bitvector.hpp"

struct basicblock;
struct jitdata;

#if !defined(NDEBUG)
# include <assert.h>
# define LSRA_DEBUG_CHECK
# define LSRA_DEBUG_VERBOSE
#endif

#ifdef LSRA_DEBUG_CHECK
# define _LSRA_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
# define _LSRA_ASSERT(a) assert((a));
#else
# define _LSRA_CHECK_BOUNDS(i,l,h)
# define _LSRA_ASSERT(a)
#endif

/* let LSRA allocate reserved registers (REG_ITMP[1|2|3]) */
#if defined(__I386__)
/* #define LSRA_USES_REG_RES */
/* # include "vm/jit/i386/icmd_uses_reg_res.inc.h" */
#endif

/* #define LSRA_SAVEDVAR */
/* #define LSRA_MEMORY */

#define USAGE_COUNT        /* influence LSRA with usagecount */
/* #define USAGE_PER_INSTR */    /* divide usagecount by lifetimelength */


struct site {
	int b_index;
	int iindex;
	struct site *next;
};

struct lifetime {
	int i_start;                /* instruction number of first use */
	int i_end;                  /* instruction number of last use */
	int v_index;           /* local variable index or negative for stackslots */
	int type;                   /* TYPE_XXX or -1 for unused lifetime */
	long usagecount;            /* number of references*/
	int regoff;                    /* regoffset allocated by lsra*/
	int savedvar;
	int flags;
	/* struct stackslot *local_ss; */ /* Stackslots for this Lifetime or NULL ( ==  */
                                /* "pure" Local Var) */
	int bb_last_use;
	int i_last_use;
	int bb_first_def;
	int i_first_def;

	struct site *def;
	struct site *use;
	struct site *last_use;
};

struct l_loop {
	int b_first;
	int b_last;
	int nesting;
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

struct lsradata {
	int varcount;       /* size of vars array */
	int ssavarcount;    /* ls->vars[0..ssavarcount[ are all locals and iovars */
	                    /* they are regarded for ssa renaming */
	                    /* the rest (ls->vars[ssavarcount..varcount[ are      */
	                    /* TEMP or PREALLOC vars with just on definition and  */
	                    /* use within one basicblock -> not of interest for   */
	                    /* ssa renaming procedures */
	int vartop;         /* next free var */
	int varcount_with_indices;
	int *new_varindex;  /* new_varindex[0..jd->varcount[ points to the new    */
	                    /* unique index of ls->vars(maps jd->vars to ls->vars)*/

	int *var_0;        /* [0..ls->varcount]  */
	                   /* var_0[a] with a in [0..ls->varcount[ holds the */
	                   /* index of La,0 */
	                   /* var_0[ls->varcount] holds the number of vars with */
	                   /*indices */

	int *sorted;         /* BB sorted in reverse post order */
	int *sorted_rev;     /* BB reverse lookup of sorted */

	long *nesting;    /* Nesting level of BB*/

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

	/* SSA fields */
	bitvector *var_def; /* LocalVar Definition Bitvector [0..ls->bbcount]  */
	                    /* Bitvector holds ls->max_vars Bits               */
	bitvector *use_sites; /* LocalVar Use Bitvector[0..ls->maxvars] */
	int **num_var_use; /* count of var_use[bb][var_index] */
	int **var; /* [0..cd->maxlocal+cd->maxstack[[0..4] */
	/* ssa_set_local_def and ssa_set_interface (called from analyse_stack)    */
	/* set var[local_index][local_type] and var[jd->maxlocals+stack_depth]    */
	/* [stack_type] to a unique type independend index [0..ls->max_vars[      */
	/* unused entries are set to -1                                           */
	int max_vars;
	int max_vars_with_indices;
	int *num_defs;    /* counts definitions of variables     */
	                  /* [0..jd->maxlocals*5+cd->maxstack*5[ */
	                  /* valid for [0..ls->max_vars[         */


	int *local_0;     /* [0..ls->max_locals]  */
	                  /* local_0[a] with a in [0..ls->max_locals[ holds the */
	                  /* index of La,0 */
	                  /* local_0[ls->maxlocals] holds the number of local   */
	                  /* vars with indices */
	int *interface_0; /* same here, just with interfaces */

	int *var_to_index; /* var index to interface (<0) or local (>=0) index */
	                   /* [0..jd->maxlocals*5+cd->maxstack*5[              */
	                   /* valid for [0..ls->max_vars[                      */
	                   /* holds var_index or the negative interface index  */
	                   /* in ssa_Rename_init the indices are changed to the */
                       /* index of the corresponding first Var with index;) */
	                   /* (== local_0[] or interface_0[] */
	int max_locals;
	int max_interfaces;

	int uses;
	int basicblockcount;
	basicblock **basicblocks;
	/* [0..ls->basicblockcount[[0..ls->max_locals[[0..ls->num_pre[bb]] */
	/* 3rd index represents the the var involved in the phi function   */
	/* a0 = phi(a1,a2,...,a(ls->num_pre[bb]-1)) */
	int ***phi;  /* [0..ls->basicblockcount[[0..ls->max_vars[[0,1] */
	             /* if no phi function for a Basic Block and var exists */
	             /* phi[bb][a] == NULL */
	int *num_phi_moves;
	int ***phi_moves; /* phi_moves[block_index][0..num_phi_moves[bi][0..1] */
	                  /* [][][0] target */
	                  /* [][][1] source */

	int *count;  /* Helpers for ssa_Rename */
	int **stack;
	int *stack_top;

};


struct freemem {
	int off;
	int end;
	struct freemem *next;
};

typedef struct lsradata lsradata;

/* function prototypes */
void lsra(jitdata *);

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
 */
