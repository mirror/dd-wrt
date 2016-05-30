/* src/vm/jit/optimizing/ssa3.cpp

   Copyright (C) 1996-2013
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   SSA transformation PROTOTYPE based on:

   Moessenboeck, H.,
   Adding Static Single Assignment Form and a Graph Coloring Register
   Allocator to the Java Hotspot Client Compiler, 2000
   (http://www.ssw.uni-linz.ac.at/Research/Reports/Report15.html)

   TODO

   * Adapt for exception handling [done]
   * Eliminiation of redundand PHI functions. [in progress]
   * Handle also inout variables. [done]
   * Create PHI functions lazyly, when they are used for the first time
     (I suspect that currently PHIs are created that are never used).
   * REWRITE. The code was never designed for producion. [done]
   * Unify access to phi args.
*/

#include "vm/jit/optimizing/ssa.hpp"

#include "config.h"

#include "vm/jit/jit.hpp"
#include "vm/jit/optimizing/escape.hpp"
#include "vm/global.hpp"
#include "mm/memory.hpp"
#include "mm/dumpmemory.hpp"
#include "toolbox/list.hpp"

#include <limits.h>
#include <stdio.h>

/*#define ELIMINATE_NOP_LOAD_STORE*/
#define FIXME(x) x
#define SSA_VERIFY
/*#define SSA_VERBOSE */

/*
__attribute__((always_inline))
*/

/*** phi ********************************************************************/

typedef enum {
	PHI_FLAG_USED,
	PHI_FLAG_REDUNDANT_ALL,
	PHI_FLAG_REDUNDANT_ONE
} phi_flags_t;

static inline void phi_set_flag(instruction *iptr, phi_flags_t flag) {
	iptr->flags.bits |= (1 << flag);
}

static inline void phi_clear_flag(instruction *iptr, phi_flags_t flag) {
	iptr->flags.bits &= ~(1 << flag);
}

static inline bool phi_has_flag(const instruction *iptr, phi_flags_t flag) {
	return (iptr->flags.bits & (1 << flag)) != 0;
}

static inline instruction *phi_get_subst(instruction *iptr) {
	return iptr->sx.s23.s2.iargs[-1];
}

static inline bool phi_has_subst(const instruction *iptr) {
	return (iptr->sx.s23.s2.iargs[-1] != NULL);
}


static inline void phi_init(instruction *iptr, unsigned argcount, s4 index) {
	unsigned i;

	iptr->opc = ICMD_PHI;
	iptr->flags.bits = 0;
	iptr->dst.varindex = 0;
	iptr->s1.argcount = argcount;
	iptr->sx.s23.s2.iargs = DMNEW(instruction *, argcount + 1);
	iptr->sx.s23.s2.iargs += 1;
	iptr->sx.s23.s3.javaindex = index;

	/* subst field - If non-null, the phi function shall be replaced by the
	   value produced by the subst instruction. */
	iptr->sx.s23.s2.iargs[-1] = NULL;

#if !defined(NDEBUG)
	for (i = 0; i < argcount; ++i) {
		iptr->sx.s23.s2.iargs[i] = (instruction *)0x7fffffff;
	}
#endif
}

#define phi_assert_opc(iptr) assert(iptr->opc == ICMD_PHI)

#define phi_assert_arg(iptr, arg) assert(arg < iptr->s1.argcount)

static inline s4 phi_arg_count(const instruction *iptr) {
	phi_assert_opc(iptr);
	return iptr->s1.argcount;
}

static inline instruction *phi_get_arg(const instruction *iptr, unsigned arg) {
	phi_assert_opc(iptr);
	phi_assert_arg(iptr, arg);
	return iptr->sx.s23.s2.iargs[arg];
}

static inline s4 phi_get_arg_var(const instruction *iptr, unsigned arg) {
	phi_assert_opc(iptr);
	phi_assert_arg(iptr, arg);
	return iptr->sx.s23.s2.iargs[arg]->dst.varindex;
}

static inline void phi_set_all_args(instruction *iptr, instruction *value) {
	unsigned i;
	phi_assert_opc(iptr);
	for (i = 0; i < iptr->s1.argcount; ++i) {
		iptr->sx.s23.s2.iargs[i] = value;
	}
}

static inline s4 phi_get_index(const instruction *iptr) {
	phi_assert_opc(iptr);
	return iptr->sx.s23.s3.javaindex;
}

static inline s4 phi_get_dst(const instruction *iptr) {
	phi_assert_opc(iptr);
	return iptr->dst.varindex;
}

static inline void phi_set_dst(instruction *iptr, s4 dst) {
	phi_assert_opc(iptr);
	iptr->dst.varindex = dst;
}

static inline bool phi_get_used(const instruction *iptr) {
	phi_assert_opc(iptr);
	return phi_has_flag(iptr, PHI_FLAG_USED);
}

static void phi_set_used(instruction *iptr) {
	phi_assert_opc(iptr);
	if (! phi_has_flag(iptr, PHI_FLAG_USED)) {
		phi_set_flag(iptr, PHI_FLAG_USED);
		/* TODO recurse into arguments */
	}
}

static instruction *phi_resolve_use(instruction *use) {
	if (use != NULL) {
		while (use->opc == ICMD_PHI) {
			if (phi_has_subst(use)) {
				use = phi_get_subst(use);
			} else {
				break;
			}
		}
	}
	return use;
}

static inline void phi_set_arg(instruction *iptr, unsigned arg, instruction *value) {
	phi_assert_opc(iptr);
	phi_assert_arg(iptr, arg);
	assert(value != NULL);
	iptr->sx.s23.s2.iargs[arg] = value;
}

static inline bool phi_is_redundant(const instruction *iptr) {
	return (
		phi_has_flag(iptr, PHI_FLAG_REDUNDANT_ONE) ||
		phi_has_flag(iptr, PHI_FLAG_REDUNDANT_ALL)
	);
}

static inline void phi_create_copy(instruction *iptr, unsigned arg, instruction *copy) {
	phi_assert_opc(iptr);
	phi_assert_arg(iptr, arg);
	copy->dst.varindex = phi_get_dst(iptr);
	copy->s1.varindex = phi_resolve_use(phi_get_arg(iptr, arg))->dst.varindex;
	if (copy->dst.varindex == copy->s1.varindex || phi_is_redundant(iptr)) {
		copy->opc = ICMD_NOP;
	} else {
		copy->opc = ICMD_COPY;
	}
}

#if !defined(NDEBUG)
static void phi_print(const instruction *iptr) {
	unsigned i;
	instruction *use;
	printf("%d = phi(", iptr->dst.varindex);
	for (i = 0; i < iptr->s1.argcount; ++i) {
		use = phi_resolve_use(iptr->sx.s23.s2.iargs[i]);
		if (use) {
			printf("%d, ", use->dst.varindex);
		} else {
			printf("null, ");
		}
	}
	printf(")\n");
}
#endif

#define FOR_EACH_PHI_USE_CAST(iptr, it, cast) \
	for ( \
		(it) = cast (iptr)->sx.s23.s2.iargs; \
		(it) != cast (iptr)->sx.s23.s2.iargs + (iptr)->s1.argcount; \
		++(it) \
	)

#define FOR_EACH_PHI_USE(iptr, it) \
	FOR_EACH_PHI_USE_CAST(iptr, it, )

#define FOR_EACH_PHI_USE_CONST(iptr, it) \
	FOR_EACH_PHI_USE_CAST(iptr, it, (const instruction *))

static void phi_calculate_redundancy(instruction *iptr) {

	s4 dst = iptr->dst.varindex;
	s4 use;
	instruction *iuse;
	instruction **ituse;
	unsigned num_different = 0;
	instruction *different;

	if (phi_is_redundant(iptr)) return; /* XXX */

	/* x = phi(x, y, x, x) ... PHI_FLAG_REDUNDANT_ONE
	   x = phi(x, x, x, x) ... PHI_FLAG_REDUNDANT_ALL */

	FOR_EACH_PHI_USE(iptr, ituse) {
		iuse = phi_resolve_use(*ituse);
		assert(iuse);

		use = iuse->dst.varindex;

		if (use != dst) {
			different = *ituse;
			num_different += 1;
			if (num_different >= 2) {
				phi_clear_flag(iptr, PHI_FLAG_REDUNDANT_ONE);
				phi_clear_flag(iptr, PHI_FLAG_REDUNDANT_ALL);
			}
		}
	}

	if (num_different == 1) {
		/* Set the subst field of the instruction.
		   I.e. the instruction will be replaced by the value produced by y. */
		iptr->sx.s23.s2.iargs[-1] = different;

		phi_set_flag(iptr, PHI_FLAG_REDUNDANT_ONE);
		phi_clear_flag(iptr, PHI_FLAG_REDUNDANT_ALL);
	} else if (num_different == 0) {
		assert(0);
		/*iptr->sx.s23.s2.iargs[-1] = iptr->sx.s23.s2.iargs[0];*/

		phi_clear_flag(iptr, PHI_FLAG_REDUNDANT_ONE);
		phi_clear_flag(iptr, PHI_FLAG_REDUNDANT_ALL);
		/*assert(0);*/
	}
}


/*** goto *******************************************************************/

static inline void goto_init(instruction *iptr, basicblock *dst) {
	iptr->opc = ICMD_GOTO;
	iptr->dst.block = dst;
}

/*** instruction ***********************************************************/

static void instruction_get_uses(const instruction *iptr, s4 *buf, s4 **puses, unsigned *puses_count) {
	unsigned uses_count = 0;

	switch (icmd_table[iptr->opc].dataflow) {
		case DF_3_TO_0:
		case DF_3_TO_1:
			buf[2] = iptr->sx.s23.s3.varindex;
			uses_count += 1;

		case DF_2_TO_0:
		case DF_2_TO_1:
			buf[1] = iptr->sx.s23.s2.varindex;
			uses_count += 1;

		case DF_1_TO_0:
		case DF_1_TO_1:
		case DF_COPY:
		case DF_MOVE:
			buf[0] = iptr->s1.varindex;
			uses_count += 1;

			*puses_count = uses_count;
			*puses = buf;
			break;

		case DF_N_TO_1:
		case DF_INVOKE:
		case DF_BUILTIN:

			*puses = iptr->sx.s23.s2.args;
			*puses_count = iptr->s1.argcount;
			break;

		default:

			*puses_count = 0;
			break;
	}
}

static inline void instruction_set_uses(instruction *iptr, s4 *buf, s4 *uses, unsigned uses_count) {
	if (uses == buf) {
		switch (uses_count) {
			case 3:
				iptr->sx.s23.s3.varindex = buf[2];
			case 2:
				iptr->sx.s23.s2.varindex = buf[1];
			case 1:
				iptr->s1.varindex = buf[0];
		}
	}
}

/*** vars *******************************************************************/

#define VARS_CATEGORY_SHIFT 28
#define VARS_INDEX_MASK 0x0FFFFFFF

#define VARS_CATEGORY_LOCAL 0
#define VARS_CATEGORY_STACK 1
#define VARS_CATEGORY_OTHERS 2

#define VAR_TYPE_SUBSTITUED ((Type) 666)

#define OLD_INDEX_UNUSED -2

typedef struct {
	varinfo v;
	s4 old_index;
} vars_item_t;

typedef struct {
	vars_item_t items[9000]; /* XXX hardcoded max */
	unsigned max;
	unsigned count;
	unsigned category;
} vars_t;

static inline unsigned vars_add_item(vars_t *vs, const varinfo *item) {
	unsigned i = vs->count;
	assert(i < vs->max);
	vs->count += 1;
	vs->items[i].v = *item;
	vs->items[i].old_index = OLD_INDEX_UNUSED;
	return (vs->category << VARS_CATEGORY_SHIFT) | i;
}

static inline unsigned vars_add(vars_t *vs) {
	unsigned i = vs->count;
	assert(i < vs->max);
	vs->count += 1;
	return (vs->category << VARS_CATEGORY_SHIFT) | i;
}

static inline varinfo *vars_back(vars_t *vs) {
	assert(vs->count > 0);
	return &(vs->items[vs->count - 1].v);
}

static inline void vars_init(vars_t *vs, unsigned category) {
	vs->max = sizeof(vs->items) / sizeof(vs->items[0]);
	vs->count = 0;
	assert((category & 0x3) == category);
	vs->category = category;
}

static inline unsigned vars_get_category(unsigned varindex) {
	return (varindex >> VARS_CATEGORY_SHIFT);
}

static inline unsigned vars_get_index(unsigned varindex) {
	return (varindex & VARS_INDEX_MASK);
}

static void vars_subst(vars_t *vs, unsigned varindex, unsigned replacementindex) {
	varindex = vars_get_index(varindex);
	replacementindex = vars_get_index(replacementindex);

	vs->items[varindex].v.type = VAR_TYPE_SUBSTITUED;
	vs->items[varindex].v.vv.ii[1] = replacementindex;
}

static unsigned vars_resolve_subst(const vars_t *vs, unsigned varindex) {
#if !defined(NDEBUG)
	unsigned loop_ctr = 0;
#endif
	varindex = vars_get_index(varindex);

	if (vs->items[varindex].v.type == VAR_TYPE_SUBSTITUED) /*fprintf(stderr, "*")*/;

	while (vs->items[varindex].v.type == VAR_TYPE_SUBSTITUED) {
		assert(loop_ctr++ != vs->count);
		varindex = vs->items[varindex].v.vv.ii[1];
	}

	return (vs->category << VARS_CATEGORY_SHIFT) | varindex ;
}

static void vars_copy_to_final(vars_t *vs, varinfo *dst) {
	const vars_item_t *it;
	unsigned subst;

	for (it = vs->items; it != vs->items + vs->count; ++it, ++dst) {

		/* Copy variable. */

		*dst = it->v;

		/* Eliminate VAR_TYPE_SUBSTITUED as it leads to problems. */

		if (dst->type == VAR_TYPE_SUBSTITUED) {
			subst = vars_get_index(vars_resolve_subst(vs, it - vs->items));
			dst->type = vs->items[subst].v.type;

		}
	}
}

static void vars_import(vars_t *vs, varinfo *v, unsigned count, s4 old_index) {
	vars_item_t *it;

	assert((vs->count + count) <= vs->max);

	it = vs->items + vs->count;
	vs->count += count;

	while (count-- > 0) {
		it->v = *v;
		it->old_index = old_index;
		it += 1;
		v += 1;
		old_index += 1;
	}
}

static inline void vars_record_old_index(vars_t *vs, unsigned varindex, s4 old_index) {
	vars_item_t *item;
	varindex = vars_get_index(varindex);

	assert(varindex < vs->count);

	item = vs->items + varindex;

	assert(
		item->old_index == OLD_INDEX_UNUSED ||
		item->old_index == old_index
	);

	item->old_index = old_index;
}

static inline s4 vars_get_old_index(vars_t *vs, unsigned varindex) {
	s4 old;

	varindex = vars_get_index(varindex);

	assert(varindex < vs->count);
	old = vs->items[varindex].old_index;
	assert(old != OLD_INDEX_UNUSED);

	return old;
}

/*** phis *******************************************************************/

typedef struct {
	instruction *items;
	unsigned max;
	unsigned count;
} phis_t;

static inline void phis_init(phis_t *ps, unsigned max) {
	ps->max = max;
	ps->count = 0;
	ps->items = DMNEW(instruction, max);
}

static inline instruction *phis_add(phis_t *ps) {
	unsigned i = ps->count;
	assert(i < ps->max);
	ps->count += 1;
	return ps->items + i;
}

static inline instruction *phis_get(const phis_t *ps, unsigned i) {
	assert(i < ps->count);
	return ps->items + i;
}

static inline bool phis_contains(const phis_t *ps, const instruction *phi) {
	return (ps->items <= phi) && (phi < (ps->items + ps->max));
}

#define FOR_EACH_PHI_FUNCTION_(ps, it) \
	for ((it) = (ps)->items; (it) != (ps)->items + (ps)->count; ++(it)) \

#define FOR_EACH_PHI_FUNCTION(ps, it) \
		FOR_EACH_PHI_FUNCTION_(ps, it) if (!phi_is_redundant((it)))

#if !defined(NDEBUG)
FIXME() inline void phis_print(const phis_t *ps) {
	const instruction *iptr;
	FOR_EACH_PHI_FUNCTION(ps, iptr) {
		phi_print(iptr);
	}
}
#endif

static inline unsigned phis_copy_to(const phis_t *ps, instruction *dst) {
	instruction *it;
	instruction *out = dst;

	FOR_EACH_PHI_FUNCTION(ps, it) {
		*(out++) = *it;
	}

	return (out - dst);
}

/*** state_array ************************************************************/

typedef struct {
	instruction **items;
	unsigned count;
} state_array_t;

static inline void state_array_init(state_array_t *sa, unsigned count) {
	sa->items = NULL;
	sa->count = count;
}

static inline bool state_array_has_items(const state_array_t *sa) {
	return (sa->items != NULL);
}

static inline s4 state_array_get_var(const state_array_t *sa, unsigned index) {
	assert(index < sa->count);
	assert(sa->items[index]);
	return sa->items[index]->dst.varindex;
}

static inline instruction *state_array_get(const state_array_t *sa, unsigned index) {
	assert(index < sa->count);
	return sa->items[index];
}

static inline void state_array_set(const state_array_t *sa, unsigned index, instruction *value) {
	assert(index < sa->count);
	sa->items[index] = value;
}

static inline void state_array_copy(state_array_t *sa, state_array_t *other) {
	assert(sa->count == other->count);
	MCOPY(sa->items, other->items, instruction *, sa->count);
}

#define state_array_assert_items(sa) assert(state_array_has_items(sa) || (sa->count == 0))
#define state_array_assert_no_items(sa) assert(! state_array_has_items(sa))

static inline void state_array_allocate_items(state_array_t *sa) {
	sa->items = DMNEW(instruction *, sa->count);
	MZERO(sa->items, instruction *, sa->count);
}

/*** basicblock_chain *******************************************************/

typedef struct {
	basicblock *first;
	basicblock *last;
} basicblock_chain_t;

static void basicblock_chain_init(basicblock_chain_t *bbc) {
	bbc->first = NULL;
	bbc->last = NULL;
}

#define basicblock_chain_clear basicblock_chain_init

static void basicblock_chain_add(basicblock_chain_t *bbc, basicblock *bb) {
	if (bbc->first == NULL) {
		assert(bbc->last == NULL);
		assert(bb->next == NULL);
		bbc->first = bb;
		bbc->last = bb;
	} else {
		assert(bbc->last->next == NULL);
		bbc->last->next = bb;
		bbc->last = bb;
	}
}

static inline basicblock *basicblock_chain_front(basicblock_chain_t *bbc) {
	assert(bbc->first);
	return bbc->first;
}

static inline basicblock *basicblock_chain_back(basicblock_chain_t *bbc) {
	assert(bbc->last);
	return bbc->last;
}

static inline bool basicblock_chain_empty(const basicblock_chain_t *bbc) {
	return bbc->first == NULL;
}

/*** exception_entry_chain ***************************************************/

typedef struct {
	exception_entry *first;
	exception_entry *last;
} exception_entry_chain_t;

static void exception_entry_chain_init(exception_entry_chain_t *eec) {
	eec->first = NULL;
	eec->last = NULL;
}

#define exception_entry_chain_clear exception_entry_chain_init

static void exception_entry_chain_add(exception_entry_chain_t *eec, exception_entry *ee) {
	if (eec->first == NULL) {
		eec->first = ee;
		eec->last = ee;
	} else {
		eec->last->next = ee;
		eec->last->down = ee;
		eec->last = ee;
	}
}

static inline bool exception_entry_chain_empty(const exception_entry_chain_t *eec) {
	return eec->first == NULL;
}

static inline exception_entry *exception_entry_chain_back(exception_entry_chain_t *eec) {
	assert(eec->last);
	return eec->last;
}

static inline exception_entry *exception_entry_chain_front(exception_entry_chain_t *eec) {
	assert(eec->first);
	return eec->first;
}

/*** traversal **************************************************************/

typedef struct {
	unsigned (*var_num_to_index)(void *vp, s4 var);
	varinfo *(*index_to_initial_var)(void *vp, unsigned index);
	varinfo *(*var_num_to_varinfo)(void *vp, s4 var);
	unsigned (*variables_count)(void *vp);
} traversal_ops_t;

typedef struct {
	phis_t *phis;
	state_array_t *state_array;
	void *ops_vp;
	traversal_ops_t *ops;
} traversal_t;

/*** basicblock_info ********************************************************/

typedef struct basicblock_info {
	bool visited;
	bool active;
	bool traversed;
	unsigned backward_branches;

	traversal_t *locals;
	traversal_t *stack;

	basicblock_chain_t *subbasicblocks;

	unsigned complete_predecessors;

#if defined(SSA_VERIFY)
	unsigned num_phi_elimination;
#endif

} basicblock_info_t;

/*** traversal **************************************************************/

void traversal_init(traversal_t *t, unsigned count, void *ops_vp, traversal_ops_t *ops) {
	t->phis = DNEW(phis_t);
	phis_init(t->phis, count);

	t->state_array = DNEW(state_array_t);
	state_array_init(t->state_array, count);

	t->ops_vp = ops_vp;
	t->ops = ops;
}

instruction *traversal_create_phi(traversal_t *t, vars_t *v, unsigned argcount, s4 index) {
	instruction *phi = phis_add(t->phis);
	s4 dst;

	phi_init(phi, argcount, index);
	dst = vars_add_item(v, t->ops->index_to_initial_var(t->ops_vp, index));
	phi_set_dst(phi, dst);

	state_array_set(t->state_array, index, phi);

	vars_record_old_index(v, phi->dst.varindex, index);

	return phi;
}

static void traversal_rename_def(traversal_t *t, vars_t *vars, instruction *iptr) {
	const varinfo *v;
	unsigned index;
	s4 old;

	state_array_assert_items(t->state_array);

	v = t->ops->var_num_to_varinfo(t->ops_vp, iptr->dst.varindex);
	index = t->ops->var_num_to_index(t->ops_vp, iptr->dst.varindex);

	old = iptr->dst.varindex;
	iptr->dst.varindex = vars_add_item(vars, v);
	state_array_set(t->state_array, index, iptr);

	vars_record_old_index(vars, iptr->dst.varindex, index);
}

static void traversal_rename_use(traversal_t *t, vars_t *vars, s4 *puse) {
	unsigned index;
	s4 old;

	state_array_assert_items(t->state_array);

	index = t->ops->var_num_to_index(t->ops_vp, *puse);

	assert(state_array_get(t->state_array, index));
	old = *puse;
	*puse = state_array_get_var(t->state_array, index);

	vars_record_old_index(vars, *puse, index);
}

static inline unsigned traversal_variables_count(traversal_t *t) {
	return t->ops->variables_count(t->ops_vp);
}

unsigned local_var_num_to_index(void *vp, s4 var) {
	return (unsigned)var;
}

varinfo *local_index_to_initial_var(void *vp, unsigned index) {
	jitdata *jd = (jitdata *)vp;
	return jd->var + index;
}

varinfo *local_var_num_to_varinfo(void *vp, s4 var) {
	jitdata *jd = (jitdata *)vp;
	return jd->var + var;
}

unsigned local_variables_count(void *vp) {
	jitdata *jd = (jitdata *)vp;
	return jd->localcount;
}

traversal_ops_t traversal_local_ops = {
	local_var_num_to_index,
	local_index_to_initial_var,
	local_var_num_to_varinfo,
	local_variables_count
};

unsigned inout_var_num_to_index(void *vp, s4 var) {
	basicblock *bb = (basicblock *)vp;
	unsigned i;

	for (i = 0; i < bb->indepth; ++i) {
		if (bb->invars[i] == var) {
			return i;
		}
	}

	for (i = 0; i < bb->outdepth; ++i) {
		if (bb->outvars[i] == var) {
			return i;
		}
	}

	assert(0);
}

varinfo *inout_index_to_initial_var(void *vp, unsigned index) {
	basicblock *bb = (basicblock *)vp;
	jitdata *jd = (jitdata *)(((basicblock_info_t *)bb->vp)->locals->ops_vp); /* evil hack */
	assert(index < bb->indepth);
	return jd->var + bb->invars[index];
}

varinfo *inout_var_num_to_varinfo(void *vp, s4 var) {
	basicblock *bb = (basicblock *)vp;
	jitdata *jd = (jitdata *)(((basicblock_info_t *)bb->vp)->locals->ops_vp); /* evil hack */
	return jd->var + var;
}

unsigned inout_variables_count(void *vp) {
	basicblock *bb = (basicblock *)vp;
	return bb->indepth;
}

traversal_ops_t traversal_inout_ops = {
	inout_var_num_to_index,
	inout_index_to_initial_var,
	inout_var_num_to_varinfo,
	inout_variables_count
};

/*** basicblock_info ********************************************************/

void basicblock_info_init(basicblock_info_t *bbi, basicblock *bb, jitdata *jd) {
	MZERO(bbi, basicblock_info_t, 1);

	bbi->locals = DNEW(traversal_t);
	traversal_init(bbi->locals, jd->localcount, jd, &traversal_local_ops);

	bbi->stack = DNEW(traversal_t);
	traversal_init(bbi->stack, jd->maxinterfaces, bb, &traversal_inout_ops);

	bbi->subbasicblocks = DNEW(basicblock_chain_t);
	basicblock_chain_init(bbi->subbasicblocks);
}

/*** basicblock *************************************************************/

static inline basicblock_info_t *basicblock_info(basicblock *bb) {
	return (basicblock_info_t *)(bb->vp);
}

#define bb_info basicblock_info

static unsigned basicblock_get_predecessor_count(basicblock *bb) {
	unsigned ret;
	basicblock **itpred;

	ret = bb->predecessorcount;

	FOR_EACH_EXPREDECESSOR(bb, itpred) {
		ret += (*itpred)->exouts;
	}

	return ret;
}

static unsigned basicblock_get_predecessor_index(basicblock *from, basicblock *to) {
	basicblock **itpred;
	unsigned j = 0;

	FOR_EACH_PREDECESSOR(to, itpred) {
		if (*itpred == from) break;
		j++;
	}

	assert(j != to->predecessorcount);

	return j;
}

static unsigned basicblock_get_ex_predecessor_index(basicblock *from, unsigned pei, basicblock *to) {
	unsigned j;
	basicblock **itpred;

	j = to->predecessorcount;

	FOR_EACH_EXPREDECESSOR(to, itpred) {
		if ((*itpred)->nr == from->nr) {
			j += pei;
			return j;
		} else {
			j += (*itpred)->exouts;
		}
	}

	assert(0);
}

/*** ssa_info ***************************************************************/

typedef struct ssa_info {
	jitdata *jd;

	vars_t *locals;
	vars_t *stack;
	vars_t *others;

	s4 s_buf[3];

	basicblock_chain_t *new_blocks;

	struct {
		s4 maxlocals;
		s4 maxinterfaces;
		s4 *local_map;
		varinfo *var;
		s4 vartop;
		s4 varcount;
		s4 localcount;
	} original;

	unsigned keep_in_out:1;

} ssa_info, ssa_info_t;

void ssa_info_init(ssa_info_t *ssa, jitdata *jd) {
	ssa->jd = jd;

	ssa->locals = DNEW(vars_t);
	vars_init(ssa->locals, VARS_CATEGORY_LOCAL);

	/* Initialize first version of locals, that is always available. */
	vars_import(ssa->locals, jd->var, jd->localcount, 0);

	ssa->stack = DNEW(vars_t);
	vars_init(ssa->stack, VARS_CATEGORY_STACK);

	ssa->others = DNEW(vars_t);
	vars_init(ssa->others, VARS_CATEGORY_OTHERS);

	ssa->new_blocks = DNEW(basicblock_chain_t);
	basicblock_chain_init(ssa->new_blocks);

	ssa->original.maxlocals = jd->maxlocals;
	ssa->original.maxinterfaces = jd->maxinterfaces;
	ssa->original.local_map = jd->local_map;
	ssa->original.var = jd->var;
	ssa->original.vartop = jd->vartop;
	ssa->original.varcount = jd->varcount;
	ssa->original.localcount = jd->localcount;

	ssa->keep_in_out = false;
}

/*** others_mapping *********************************************************/

static inline void others_mapping_set(ssa_info *ssa, s4 var, s4 new_var) {
	ssa->jd->var[var].vv.ii[1] = new_var;
}

static inline s4 others_mapping_get(const ssa_info *ssa, s4 var) {
	return ssa->jd->var[var].vv.ii[1];
}

/*** code *******************************************************************/

void fix_exception_handlers(jitdata *jd) {
	basicblock *bptr;
	basicblock *exh = NULL;
	instruction *iptr;
	exception_entry *ee;
	size_t add_vars;
	size_t avail_vars;
	s4 v;
	basicblock_chain_t chain;
	basicblock *last = NULL;
	basicblock *marker = NULL;
	s4 vartop;
	unsigned i;

	if (jd->exceptiontablelength == 0) {
		return;
	}

	basicblock_chain_init(&chain);

	/* Remember, where we started adding IO variables. */

	vartop = jd->vartop;

	/* For each exception handler block, create one block with a prologue block */

	FOR_EACH_BASICBLOCK(jd, bptr) {
		if (bptr->type == basicblock::TYPE_EXH) {

			/*

            +---- EXH (exh)-------+
            |  in0 in1 in2 exc    |
			|  .....              |
            +---------------------+

            === TRANSFORMED TO ===>

            +---- PROL (exh) -------+
            |  in0 in1 in2          |
            |  GETEXECEPTION => OU3 |
            |  GOTO REAL_EXH        |
            |  in0 in1 in2 OU3      |
            +-----------------------+

            +---- REAL_EXH (std) -+
            |  in0 in1 in2 exc    |
			|  ......             |
            +---------------------+

			*/

			bptr->type = basicblock::TYPE_STD;
			bptr->predecessorcount = 0; /* legacy */

			/* Create basicblock with 2 instructions */

			exh = DNEW(basicblock);
			MZERO(exh, basicblock, 1);

			iptr = DMNEW(instruction, 2);
			MZERO(iptr, instruction, 2);

			/* Create outvars */

			exh->outdepth = bptr->indepth;

			if (exh->outdepth > 0) {
				exh->outvars = DMNEW(s4, exh->outdepth);
				for (i = 0; i < exh->outdepth; ++i) {
					exh->outvars[i] = vartop++;
				}
			}

			/* Create invars */

			exh->indepth = exh->outdepth - 1;
			exh->invars = exh->outvars;

#if 0
			/* Create new outvar */

			assert(jd->vartop < jd->varcount);
			v = jd->vartop;
			jd->vartop += 1;
			jd->var[v].type = TYPE_ADR;
			jd->var[v].flags = INOUT;
#endif

			exh->nr = jd->basicblockcount;
			jd->basicblockcount += 1;
			exh->mpc = -1;
			exh->type = basicblock::TYPE_EXH;
			exh->icount = 2;
			exh->iinstr = iptr;
/*
			exh->outdepth = 1;
			exh->outvars = DNEW(s4);
			exh->outvars[0] = v;
*/
			exh->predecessorcount = -1; /* legacy */
			exh->state  = basicblock::FINISHED;
			exh->method = jd->m;

			basicblock_chain_add(&chain, exh);

			/* Get exception */

			iptr->opc = ICMD_GETEXCEPTION;
			iptr->dst.varindex = exh->outvars[exh->outdepth - 1];
			iptr += 1;

			/* Goto real exception handler */

			goto_init(iptr, bptr);

			bptr->vp = exh;
		} else {
			bptr->vp = NULL;
		}

		if (bptr->next == NULL) {
			marker = bptr;
		} else {
			last = bptr;
		}
	}

	/* We need to allocate the new iovars in the var array */

	avail_vars = (jd->varcount - jd->vartop);
	add_vars = (vartop - jd->vartop);

	if (add_vars > avail_vars) {
		add_vars -= avail_vars;
		jd->var = DMREALLOC(jd->var, varinfo, jd->varcount, jd->varcount + add_vars);
		jd->varcount += add_vars;
	}

	jd->vartop = vartop;

	/* Put the chain of exception handlers between just before the last
	   basic block (end marker). */

	if (! basicblock_chain_empty(&chain)) {
		marker->nr = jd->basicblockcount;
		basicblock_chain_back(&chain)->next = marker;
		last->next = basicblock_chain_front(&chain);

		assert(last->nr + 1 == basicblock_chain_front(&chain)->nr);
		assert(marker->nr == jd->basicblockcount);
	}

	/* Replace all exception handlers in exception table with their respective
	   prologue blocks. */

	for (ee = jd->exceptiontable; ee; ee = ee->down) {
		assert(ee->handler->vp);

		bptr = ee->handler;
		exh = (basicblock *)ee->handler->vp;

		ee->handler = exh;

		/* Set up IO variables in newly craeted exception handlers. */

		for (i = 0; i < exh->outdepth; ++i) {
			v = exh->outvars[i];

			jd->var[v].flags = INOUT;
			jd->var[v].type = jd->var[ bptr->invars[i] ].type;
		}
	}

}

void unfix_exception_handlers(jitdata *jd) {
	basicblock *bptr, *exh;
	unsigned i;
	exception_entry *ee;
#if !defined(NDEBUG)
	bool found = false;
#endif

	FOR_EACH_BASICBLOCK(jd, bptr) {
		if (bptr->type == basicblock::TYPE_EXH) {
			assert(bptr->iinstr[1].opc == ICMD_GOTO);
			exh = bptr->iinstr[1].dst.block;

			bptr->state    = basicblock::DELETED;
			bptr->icount   = 0;
			bptr->indepth  = 0;
			bptr->outdepth = 0;
			exh->type      = basicblock::TYPE_EXH;
			bptr->vp       = exh;

			/* bptr is no more a predecessor of exh */

			for (i = 0; i < exh->predecessorcount; ++i) {
				if (exh->predecessors[i] == bptr) {
					exh->predecessors[i] = exh->predecessors[exh->predecessorcount - 1];
					exh->predecessorcount -= 1;
#if !defined(NDEBUG)
					found = true;
#endif
					break;
				}
			}

			assert(found);

		} else {
			bptr->vp = NULL;
		}
	}

	for (ee = jd->exceptiontable; ee; ee = ee->down) {
		assert(ee->handler->vp);
		ee->handler = (basicblock*) ee->handler->vp;
	}
}

/*** ssa_enter ***************************************************************/

static void ssa_enter_mark_loops_intern(basicblock *bb, unsigned num_branches) {
	basicblock_info_t *bbi = bb_info(bb);
	basicblock **itsucc;

	if (! bbi->visited) {
		bbi->visited = true;
		bbi->active = true;
		FOR_EACH_SUCCESSOR(bb, itsucc) {
			/* There is a single branch from bb into the successor. */
			ssa_enter_mark_loops_intern(*itsucc, 1);
		}
		FOR_EACH_EXHANDLER(bb, itsucc) {
			/* For exception handler type successors,
			   we count a branch into the exception handler from every PEI. */
			ssa_enter_mark_loops_intern(*itsucc, bb->exouts);
		}
		bbi->active = false;
	} else if (bbi->active) {
		bbi->backward_branches += num_branches;
	}
}

static inline void ssa_enter_mark_loops(basicblock *bb) {
	ssa_enter_mark_loops_intern(bb, 1);
}

static void ssa_enter_merge(
	traversal_t *src,
	traversal_t *dst,
	basicblock *bdst,
	unsigned predecessor_index,
	vars_t *vdst
) {

	basicblock_info_t *dsti = bb_info(bdst);
	unsigned predecessor_count = basicblock_get_predecessor_count(bdst);
	instruction *phi;
	instruction *old;
	s4 i;

	/* We are merging for the first time into this block. */

	if (! state_array_has_items(dst->state_array)) {

		state_array_allocate_items(dst->state_array);

		if (dsti->backward_branches > 0) {
			/* Loop header, create phi functions for all variables. */
			for (i = 0; i < traversal_variables_count(dst); ++i) {
				phi = traversal_create_phi(dst, vdst, predecessor_count, i);
				/* No need to init, they will only be filled in later. */
			}
		} else {
			state_array_copy(dst->state_array, src->state_array);
			return;
		}
	}

	/* We are merging another block. */

	/* Process every variable. */

	for (i = 0; i < traversal_variables_count(dst); ++i) {
		if (dsti->traversed) {

			/* Back edge, all phi functions are already created.
			   We only need to set their arguments. */

			phi_set_arg(
				phis_get(dst->phis, i),
				predecessor_index,
				state_array_get(src->state_array, i)
			);

		} else if (state_array_get(dst->state_array, i) != state_array_get(src->state_array, i)) {

			/* A different definition of this var reaches the block.
			   We need to create a phi function. */

			if (phis_contains(dst->phis, state_array_get(dst->state_array, i))) {
				/* There is already a phi function created for this var.
				   No need to create one. */
			} else {
				/* Create a new phi function.
				   Set all arguments to old value in state array. */
				old = state_array_get(dst->state_array, i);
				phi = traversal_create_phi(dst, vdst, predecessor_count, i);
				phi_set_all_args(phi, old);
			}

			/* Set argument of phi function. */

			phi_set_arg(
				state_array_get(dst->state_array, i),
				predecessor_index,
				state_array_get(src->state_array, i)
			);
		}
	}
}

static void ssa_enter_process_block(ssa_info *ssa, basicblock *bb);
static bool ssa_enter_eliminate_redundant_phis(traversal_t *t, vars_t *vs, basicblock_info_t *bbi);

#if defined(SSA_VERIFY)
static void ssa_enter_verify_no_redundant_phis(ssa_info_t *ssa) {
	basicblock *bptr;
	basicblock_info_t *bbi;
	instruction *itph;

	/* XXX */
	return;

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {
		if (basicblock_reached(bptr)) {
			bbi = bb_info(bptr);
			FOR_EACH_PHI_FUNCTION(bbi->locals->phis, itph) {
				if (! phi_is_redundant(itph)) {
					phi_calculate_redundancy(itph);
					assert(! phi_is_redundant(itph));
				}
			}
			FOR_EACH_PHI_FUNCTION(bbi->stack->phis, itph) {
				if (! phi_is_redundant(itph)) {
					phi_calculate_redundancy(itph);
					assert(! phi_is_redundant(itph));
				}
			}
		}
	}
}
#endif

static void ssa_enter_traverse(ssa_info_t *ssa, basicblock *bb) {
	basicblock **itsucc;
	basicblock_info_t *succi;
	basicblock_info_t *bbi = bb_info(bb);
	unsigned predecessor_count;

	/* Process block */

	ssa_enter_process_block(ssa, bb);

	/* Recurse */

	FOR_EACH_SUCCESSOR(bb, itsucc) {

		succi = bb_info(*itsucc);

		ssa_enter_merge(
			bbi->locals,
			succi->locals,
			*itsucc,
			basicblock_get_predecessor_index(bb, *itsucc),
			ssa->locals
		);

		ssa_enter_merge(
			bbi->stack,
			succi->stack,
			*itsucc,
			basicblock_get_predecessor_index(bb, *itsucc),
			ssa->stack
		);

		succi->complete_predecessors += 1;

		predecessor_count = basicblock_get_predecessor_count(*itsucc);

		if (
			succi->complete_predecessors ==
			(predecessor_count - succi->backward_branches)
		) {
			ssa_enter_traverse(ssa, *itsucc);
		}

		if (
			(succi->complete_predecessors == predecessor_count) &&
			(succi->backward_branches > 0)
		) {
#if defined(SSA_VERIFY)
			assert(succi->num_phi_elimination < 2);
			succi->num_phi_elimination += 1;
#endif
			ssa_enter_eliminate_redundant_phis(succi->locals, ssa->locals, succi);
			ssa_enter_eliminate_redundant_phis(succi->stack, ssa->stack, succi);
		}
	}

	FOR_EACH_EXHANDLER(bb, itsucc) {

		succi = bb_info(*itsucc);

		succi->complete_predecessors += bb->exouts; /* XXX this might be 0 */

		predecessor_count = basicblock_get_predecessor_count(*itsucc);

		if (
			succi->complete_predecessors ==
			(predecessor_count - succi->backward_branches)
		) {
			ssa_enter_traverse(ssa, *itsucc);
		}

		if (
			(succi->complete_predecessors == predecessor_count) &&
			(succi->backward_branches > 0)
		) {
#if defined(SSA_VERIFY)
			assert(succi->num_phi_elimination < 2);
			succi->num_phi_elimination += 1;
#endif
			ssa_enter_eliminate_redundant_phis(succi->locals, ssa->locals, succi);
			ssa_enter_eliminate_redundant_phis(succi->stack, ssa->stack, succi);
		}

	}

}

static void ssa_enter_process_pei(ssa_info *ssa, basicblock *bb, unsigned pei) {
	basicblock_info_t *bbi = bb_info(bb);
	basicblock_info_t *succi;
	basicblock **itsucc;

	FOR_EACH_EXHANDLER(bb, itsucc) {
		succi = bb_info(*itsucc);

		ssa_enter_merge(
			bbi->locals,
			succi->locals,
			*itsucc,
			basicblock_get_ex_predecessor_index(bb, pei, *itsucc),
			ssa->locals
		);

		ssa_enter_merge(
			bbi->stack,
			succi->stack,
			*itsucc,
			basicblock_get_ex_predecessor_index(bb, pei, *itsucc),
			ssa->stack
		);
	}
}

static FIXME(bool) ssa_enter_eliminate_redundant_phis(traversal_t *t, vars_t *vs, basicblock_info_t *FIXME(bbi)) {

	instruction *itph;
	bool ret = false;

	/* XXX */
	assert(false);
	return false;

	FOR_EACH_PHI_FUNCTION(t->phis, itph) {

		phi_calculate_redundancy(itph);

		/* If the phi function is redundant,
		   make the variable it defines point to the value defined by the substituing
		   instruction. */

		if (phi_is_redundant(itph)) {
			vars_subst(vs, itph->dst.varindex, phi_get_subst(itph)->dst.varindex);
			assert(bbi->backward_branches > 0);
			ret = true;
		}
	}

	return ret;
}

#if 0
static void ssa_enter_post_eliminate_redundand_phi(
	ssa_info_t *ssa,
	instruction *phi,
	state_array *sa,
	basicblock *bptr
) {
	phi_calculate_redundancy(phi);
	phi_set_flag(PHI_FLAG_REDUNDANCY_CHECKED);

	/* if redundancy changed and phi function escapes block */

	/* for each successor */
}

static void ssa_enter_post_eliminate_redundant_phis(ssa_info_t *ssa) {
	basicblock *bptr;
	basicblock_info_t *bbi;
	instruction *itph;

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {
		bbi = bb_info(bptr);

		if (bbi->backward_branches > 0) {
			/* Redundant phi functions have the left hand side as operand.
			   This can happen by definition only in loop headers. */

			FOR_EACH_PHI_FUNCTION(bbi->locals, itph) {
				if (! phi_has_flag(PHI_FLAG_REDUNDANCY_CHECKED)) {
					/* Calculate redundancy? */
					/* Changed? */
					/* If yes recurse? */
				}
			}

			FOR_EACH_PHI_FUNCTION(bbi->stack, itph) {
			}
		}
	}
}
#endif

static void ssa_enter_init_locals(state_array_t *sa) {
	unsigned i;
	instruction *iptr;

	for (i = 0; i < sa->count; ++i) {
		iptr = DNEW(instruction);
		iptr->opc = ICMD_NOP;
		iptr->dst.varindex = i;
		state_array_set(sa, i, iptr);
	}
}

static void ssa_enter_process_block(ssa_info *ssa, basicblock *bb) {
	basicblock_info_t *bbi = bb_info(bb);
	instruction *iptr;
	unsigned pei = 0;
	s4 *ituse;
	s4 *uses;
	unsigned uses_count;
	s4 old;

	/* Every basic block can be traversed only once. */

	assert(! bbi->traversed);
	bbi->traversed = true;

	/* The root basicblock needs special treatment. */

	if (bb->predecessorcount == 0 && bb->type != basicblock::TYPE_EXH) {
		state_array_assert_no_items(bbi->locals->state_array);
		state_array_allocate_items(bbi->locals->state_array);
		ssa_enter_init_locals(bbi->locals->state_array);

		state_array_assert_no_items(bbi->stack->state_array);
		state_array_allocate_items(bbi->stack->state_array);
	}

#if 0
	/* Exception handlers have a clean stack. */

	/* Not true with inlining. */

	if (bb->type == basicblock::TYPE_EXH) {
		state_array_assert_no_items(bbi->stack->state_array);
		state_array_allocate_items(bbi->stack->state_array);
	}
#endif

	/* Some in/out vars get marked as INOUT in simplereg,
	   and are not marked at this point.
	   Mark them manually. */

	for (ituse = bb->invars; ituse != bb->invars + bb->indepth; ++ituse) {
		if (ssa->keep_in_out && ssa->jd->var[*ituse].type == TYPE_RET) {
			continue;
		}
		ssa->jd->var[*ituse].flags |= INOUT;
		ssa->jd->var[*ituse].flags &= ~PREALLOC;
	}

	for (ituse = bb->outvars; ituse != bb->outvars + bb->outdepth; ++ituse) {
		if (ssa->keep_in_out && ssa->jd->var[*ituse].type == TYPE_RET) {
			continue;
		}
		ssa->jd->var[*ituse].flags |= INOUT;
		ssa->jd->var[*ituse].flags &= ~PREALLOC;
	}

	/* Process instructions */

	state_array_assert_items(bbi->locals->state_array);

	FOR_EACH_INSTRUCTION(bb, iptr) {

#if defined(ELIMINATE_NOP_LOAD_STORE)

		/* Kill NOP instructions of the form:
		   LOAD foo => foo
		   STORE foo => foo
		   As they create a lot of unnecessary definitions.
		   For performance, definitely eliminate them. However, keeping them is a
		   good stress test.
		*/

		if (
			(icmd_table[iptr->opc].dataflow == DF_LOAD) ||
			(icmd_table[iptr->opc].dataflow == DF_STORE)
		) {
			if (iptr->dst.varindex == iptr->s1.varindex) {
				iptr->opc = ICMD_NOP;
				continue;
			}
		}
#endif

		if (icmd_table[iptr->opc].flags & ICMDTABLE_PEI) {
			ssa_enter_process_pei(ssa, bb, pei++);
		}

		instruction_get_uses(iptr, ssa->s_buf, &uses, &uses_count);

		for (ituse = uses; ituse != uses + uses_count; ++ituse) {
			if (var_is_local(ssa->jd, *ituse)) {
				traversal_rename_use(
					bbi->locals,
					ssa->locals,
					ituse
				);
			} else if (var_is_inout(ssa->jd, *ituse)) {
				traversal_rename_use(
					bbi->stack,
					ssa->stack,
					ituse
				);
			} else {
				*ituse = others_mapping_get(ssa, *ituse);
			}
		}

		instruction_set_uses(iptr, ssa->s_buf, uses, uses_count);

		if (instruction_has_dst(iptr)) {
			if (var_is_local(ssa->jd, iptr->dst.varindex)) {
				traversal_rename_def(
					bbi->locals,
					ssa->locals,
					iptr
				);
			} else if (var_is_inout(ssa->jd, iptr->dst.varindex)) {
				traversal_rename_def(
					bbi->stack,
					ssa->stack,
					iptr
				);
			} else {
				old = iptr->dst.varindex;
				iptr->dst.varindex = vars_add_item(
					ssa->others,
					ssa->jd->var + iptr->dst.varindex
				);
				vars_record_old_index(ssa->others, iptr->dst.varindex, old);
				others_mapping_set(ssa, old, iptr->dst.varindex);
			}
		}
	}
}

/*

 [locals.........................][interaces][others]
 [original locals][version > 1   ]
 <--------------- new locals --------------->
*/

static void ssa_enter_export_variables(ssa_info *ssa) {
	s4 vartop;
	varinfo *vars;
	s4 *it;
	unsigned i, j;
	jitdata *jd = ssa->jd;
	s4 *local_map;

	vartop = ssa->locals->count + ssa->stack->count + ssa->others->count;
	vars = DMNEW(varinfo, vartop);

	vars_copy_to_final(ssa->locals, vars);
	vars_copy_to_final(ssa->stack, vars + ssa->locals->count);
	vars_copy_to_final(ssa->others, vars + ssa->locals->count + ssa->stack->count);

	jd->var = vars;
	jd->vartop = jd->varcount = vartop;

	/* Grow local map to accomodate all new locals and iovars.
	   But keep the local map for version 1 of locals, that contains the holes. */

	local_map = DMNEW(
		s4,
		5 * (jd->maxlocals + ssa->locals->count + ssa->stack->count - jd->localcount)
	);

	MCOPY(local_map, jd->local_map, s4, 5 * jd->maxlocals);

	jd->local_map = local_map;

	it = jd->local_map + (jd->maxlocals * 5); /* start adding entries here */

	/* Add version > 1 of all locals */

	for (i = jd->localcount; i < ssa->locals->count; ++i) {
		for (j = 0; j < 5; ++j) {
			if (jd->var[i].type != j) {
				*it = jitdata::UNUSED;
			} else {
				*it = i;
			}
			it += 1;
		}
	}

	/* Add all io vars. */

	for (i = ssa->locals->count; i < ssa->locals->count + ssa->stack->count; ++i) {
		for (j = 0; j < 5; ++j) {
			if (jd->var[i].type != j) {
				*it = jitdata::UNUSED;
			} else {
				*it = i;
			}
			it += 1;
		}
	}

	/* Add locals. */

	jd->maxlocals += (ssa->locals->count + ssa->stack->count - jd->localcount);
	jd->localcount = ssa->locals->count + ssa->stack->count;

	/* Eliminate interfaces. */

	jd->maxinterfaces = 0;

}

static void ssa_enter_export_phis(ssa_info_t *ssa) {
	basicblock *bptr;
	basicblock_info_t *bbi;
	instruction *dst;

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {
		bbi = bb_info(bptr);
		if (bbi != NULL) {
			bptr->phis = DMNEW(instruction, bbi->locals->phis->count + bbi->stack->phis->count);

			dst = bptr->phis;

			dst += phis_copy_to(bbi->locals->phis, dst);

			dst += phis_copy_to(bbi->stack->phis, dst);

			bptr->phicount = dst - bptr->phis;
		}
	}
}

/* TODO rename */
static inline void ssa_enter_eliminate_category(ssa_info_t *ssa, s4 *pvar) {
	switch (vars_get_category(*pvar)) {
		case VARS_CATEGORY_LOCAL:
			*pvar = vars_get_index(vars_resolve_subst(ssa->locals, *pvar));
			break;
		case VARS_CATEGORY_STACK:
			*pvar = vars_get_index(vars_resolve_subst(ssa->stack, *pvar)) + ssa->locals->count;
			break;
		case VARS_CATEGORY_OTHERS:
			*pvar = vars_get_index(*pvar) + ssa->locals->count + ssa->stack->count;
			break;
	}
}

/* TODO rename */
void ssa_enter_eliminate_categories(ssa_info_t *ssa) {
	basicblock *bb;
	instruction *iptr;
	s4 *ituse, *uses;
	unsigned uses_count;
	basicblock_info_t *bbi;
	instruction *itph;

	FOR_EACH_BASICBLOCK(ssa->jd, bb) {

		bbi = bb_info(bb);

		if (! ssa->keep_in_out) {
			bb->indepth = 0;
			bb->outdepth = 0;
		}

		if (bbi != NULL) {
			FOR_EACH_PHI_FUNCTION(bbi->locals->phis, itph) {
				ssa_enter_eliminate_category(ssa, &(itph->dst.varindex));
			}

			FOR_EACH_PHI_FUNCTION(bbi->stack->phis, itph) {
				ssa_enter_eliminate_category(ssa, &(itph->dst.varindex));
			}
		}

		FOR_EACH_INSTRUCTION(bb, iptr) {
			if (instruction_has_dst(iptr)) {
				ssa_enter_eliminate_category(ssa, &(iptr->dst.varindex));
			}
			instruction_get_uses(iptr, ssa->s_buf, &uses, &uses_count);
			for (ituse = uses; ituse != uses + uses_count; ++ituse) {
				ssa_enter_eliminate_category(ssa, ituse);
			}
			instruction_set_uses(iptr, ssa->s_buf, uses, uses_count);
		}
	}
}

static void ssa_enter_create_phi_graph(ssa_info *ssa) {
	basicblock *bptr;
	basicblock_info_t *bbi;
	instruction *itph;
	instruction **ituse;
	unsigned i;
	char path[PATH_MAX], *ppath;
	FILE *f;

	snprintf(path, PATH_MAX, "|tmp|graphs|%s.%s.dot", ssa->jd->m->clazz->name.begin(), ssa->jd->m->name.begin());
	for (ppath = path; *ppath; ++ppath) {
		if (*ppath == '|') *ppath = '/';
		else if (*ppath == '/') *ppath = '.';
	}

	f = fopen(path, "w");

	if (f == NULL) return;

	fprintf(f, "digraph G {\n");

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {
		bbi = bb_info(bptr);
		if (bbi != NULL) {
			FOR_EACH_PHI_FUNCTION(bbi->locals->phis, itph) {
				i = 0;
				FOR_EACH_PHI_USE(itph, ituse) {
					if ((*ituse)->opc == ICMD_PHI) {
						fprintf(f, "%d -> %d;\n", (*ituse)->dst.varindex, itph->dst.varindex);
					}
					i += 1;
				}
			}
		}
	}

	fprintf(f, "};\n");

	fclose(f);
}

static basicblock *ssa_leave_create_transition_block_intern(
	ssa_info *ssa,
	basicblock *from,
	basicblock *to,
	unsigned predecessor_index,
	unsigned reserved_insns
) {
	basicblock *bb;
	instruction *iptr;
	instruction *itph;
	basicblock_info_t *toi;

	toi = bb_info(to);

	/* Create basicblock and instruction array. */

	bb = DNEW(basicblock);
	MZERO(bb, basicblock, 1);

	bb->nr = ssa->jd->basicblockcount;
	ssa->jd->basicblockcount += 1;
	bb->mpc    = -1;
	bb->method = ssa->jd->m;
	bb->type   = basicblock::TYPE_STD;
	bb->icount = reserved_insns
	           + toi->locals->phis->count
	           + toi->stack->phis->count
	           + 1;
	bb->iinstr = DMNEW(instruction, bb->icount);
	MZERO(bb->iinstr, instruction, bb->icount);

	/* Populate instruction array. */

	iptr = bb->iinstr + reserved_insns;

	/* Add phi moves. */

	FOR_EACH_PHI_FUNCTION(toi->locals->phis, itph) {
		phi_create_copy(itph, predecessor_index, iptr++);
	}

	FOR_EACH_PHI_FUNCTION(toi->stack->phis, itph) {
		phi_create_copy(itph, predecessor_index, iptr++);
	}

	/* Add goto to real block. */

	goto_init(iptr, to);

	/* Add basicblock to chain of newly created basicblocks. */

	basicblock_chain_add(ssa->new_blocks, bb);

	return bb;
}

static inline basicblock *ssa_leave_create_transition_block(
	ssa_info *ssa,
	basicblock *from,
	basicblock *to
) {
	return ssa_leave_create_transition_block_intern(
		ssa,
		from,
		to,
		basicblock_get_predecessor_index(from, to),
		0
	);
}

static void ssa_leave_create_fallthrough(ssa_info *ssa, basicblock *bptr) {
	unsigned predecessor_index;
	basicblock_info_t *toi;
	instruction *iptr;
	instruction *itph;
	unsigned icount;

	if (bptr->next == NULL) {
		/* No fallthrough. */
		return;
	}

	predecessor_index = basicblock_get_predecessor_index(bptr, bptr->next);
	toi = bb_info(bptr->next);

	assert(toi);

	/* Resize instruction array to accomodate all phi moves. */

	icount = bptr->icount + toi->locals->phis->count + toi->stack->phis->count;

	bptr->iinstr = DMREALLOC(
		bptr->iinstr,
		instruction,
		bptr->icount,
		icount
	);

	iptr = bptr->iinstr + bptr->icount;
	bptr->icount = icount;

	/* Create phi moves. */

	FOR_EACH_PHI_FUNCTION(toi->locals->phis, itph) {
		phi_create_copy(itph, predecessor_index, iptr++);
	}

	FOR_EACH_PHI_FUNCTION(toi->stack->phis, itph) {
		phi_create_copy(itph, predecessor_index, iptr++);
	}
}

static void ssa_leave_create_phi_moves(ssa_info *ssa) {
	basicblock *bptr;
	instruction *iptr;
	basicblock *last = NULL;

	s4 i, l;
	branch_target_t *table;
	lookup_target_t *lookup;
	bool has_fallthrough;

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {

		if (bptr->next == NULL) {
			last = bptr;
		}

		if (bptr->vp == NULL) {
			continue;
		}

		if (! basicblock_reached(bptr)) {
			continue;
		}

		has_fallthrough = true;

		for (iptr = bptr->iinstr; iptr != bptr->iinstr + bptr->icount; ++iptr) {
			switch (icmd_table[iptr->opc].controlflow) {
				case CF_IF:
				case CF_RET:
				case CF_GOTO:
					iptr->dst.block =
						ssa_leave_create_transition_block(ssa, bptr, iptr->dst.block);
					break;
				case CF_TABLE:
					table = iptr->dst.table;
					l = iptr->sx.s23.s2.tablelow;
					i = iptr->sx.s23.s3.tablehigh;
					i = i - l + 1;
					i += 1; /* default */
					while (--i >= 0) {
						table->block =
							ssa_leave_create_transition_block(ssa, bptr, table->block);
						++table;
					}
					break;
				case CF_LOOKUP:
					lookup = iptr->dst.lookup;
					i = iptr->sx.s23.s2.lookupcount;
					while (--i >= 0) {
						lookup->target.block =
							ssa_leave_create_transition_block(ssa, bptr, lookup->target.block);
						lookup++;
					}
					iptr->sx.s23.s3.lookupdefault.block =
						ssa_leave_create_transition_block(ssa, bptr, iptr->sx.s23.s3.lookupdefault.block);
					break;
				case CF_JSR:
					iptr->sx.s23.s3.jsrtarget.block =
						ssa_leave_create_transition_block(ssa, bptr, iptr->sx.s23.s3.jsrtarget.block);
					break;
			}

			if (
				(iptr->opc == ICMD_GOTO) ||
				(iptr->opc == ICMD_JSR) ||
				(iptr->opc == ICMD_RET) ||
				icmd_table[iptr->opc].controlflow == CF_END ||
				(iptr->opc == ICMD_TABLESWITCH) ||
				(iptr->opc == ICMD_LOOKUPSWITCH)
			) {
				has_fallthrough = false;
			} else if (iptr->opc != ICMD_NOP) {
				has_fallthrough = true;
			}

		}

		if (bptr->next == NULL) {
			continue;
		}

		if (! basicblock_reached(bptr->next)) {
			continue;
		}

		if (has_fallthrough) {
			ssa_leave_create_fallthrough(ssa, bptr);
		}
	}

	/* Add chain of new basic blocks */

	if (last != NULL && ! basicblock_chain_empty(ssa->new_blocks)) {
		last->next = basicblock_chain_front(ssa->new_blocks);
	}

}

static basicblock *ssa_leave_split_basicblock_at(ssa_info *ssa, basicblock *bptr, instruction *iptr) {

	basicblock_info_t *bbi = bb_info(bptr);
	unsigned iidx = iptr - bptr->iinstr;
	basicblock *newblock;
	basicblock *tosplit;
	unsigned ileft;
	unsigned pos;

	assert(iidx < bptr->icount);
	assert(bbi);

	/* If there are no subbasicblocks yet, we initialize the first one to be a
	   copy of the original basicblock. */

	if (basicblock_chain_empty(bbi->subbasicblocks)) {
		newblock = DNEW(basicblock);
		*newblock = *bptr;
		newblock->next = NULL;
		newblock->vp = NULL;
		basicblock_chain_add(bbi->subbasicblocks, newblock);
	}

	/* Find the subbasicblock that will be split:
	   the one that cointains iptr. */

	tosplit = basicblock_chain_front(bbi->subbasicblocks);
	pos = 0;

	while (tosplit->next && (iidx >= (pos + tosplit->icount))) {
		assert(bptr->nr == tosplit->nr);
		pos += tosplit->icount;
		tosplit = tosplit->next;
	}

	assert(bptr->nr == tosplit->nr);

	/* Calculate number of instructions left in block to split. */

	ileft = iptr - tosplit->iinstr + 1;
	assert(ileft <= tosplit->icount);

	/* If there are any instructions left in the block to split, split */

	if (ileft < tosplit->icount) {
		newblock = DNEW(basicblock);
		*newblock = *tosplit;

		tosplit->next = newblock;
		tosplit->icount = ileft;

		newblock->icount -= ileft;
		newblock->iinstr += ileft;

		assert(tosplit->nr == bptr->nr);
		assert(newblock->nr == bptr->nr);
		assert(newblock->next == NULL);

		if (newblock->next == NULL) {
			bbi->subbasicblocks->last = newblock;
		}
	}

	/* We won't break pointers/references to bptr.
	   So return bptr instread of the first fragment.
	   Later, we will put the first fragment into the memory used by bptr.
	*/

	if (tosplit == basicblock_chain_front(bbi->subbasicblocks)) {
		tosplit = bptr;
	}

	return tosplit;
}

static basicblock *ssa_leave_create_transition_exception_handler(
	ssa_info *ssa,
	basicblock *from,
	unsigned pei,
	basicblock *to
) {
	basicblock *exh;

	/* From is a try block, to is an exception handler prologue. */

	/* Remove old prologue. */

	to->state = basicblock::DELETED;

	/* Create new exception handler. */

	exh = ssa_leave_create_transition_block_intern(
		ssa,
		from,
		to,
		basicblock_get_ex_predecessor_index(from, pei, to),
		1
	);
	exh->type = basicblock::TYPE_EXH;

	/* Copy goto to real exception handler at the end of the exception handler
	   prologue. */

	assert(to->iinstr[to->icount - 1].opc == ICMD_GOTO);
	assert(exh->iinstr[exh->icount - 1].opc == ICMD_GOTO);
	exh->iinstr[exh->icount - 1] = to->iinstr[to->icount - 1];

	/* Copy getexception from the old prologue. */

	assert(to->iinstr[0].opc == ICMD_GETEXCEPTION);
	exh->iinstr[0] = to->iinstr[0];

	return exh;
}

static exception_entry *ssa_leave_create_transition_exception_entry(
	ssa_info_t *ssa,
	basicblock *from,
	basicblock *handler,
	classref_or_classinfo catchtype
) {

	exception_entry *ee = DNEW(exception_entry);
	basicblock_info_t *fromi = bb_info(from);

	ee->start = from;

	/* If the try block has subbasicblocks, the next block is the next fragment,
	   not the successor block. */

	if (fromi != NULL) {
		ee->end = basicblock_chain_front(fromi->subbasicblocks)->next;
	} else {
		ee->end = from->next;
	}
	ee->handler = handler;
	ee->catchtype = catchtype;
	ee->next = NULL;
	ee->down = NULL;

	return ee;
}

static void ssa_leave_create_exceptional_phi_moves(ssa_info *ssa) {
	basicblock *bptr;
	instruction *iptr;
	exception_entry *ite;
	exception_entry_chain_t chain;
	classref_or_classinfo catchtype;
	basicblock *ittry;
	unsigned pei;
	basicblock *try_block;
	basicblock *exh;
	exception_entry *ee;
	basicblock *last = NULL;

	if (! basicblock_chain_empty(ssa->new_blocks)) {
		last = basicblock_chain_back(ssa->new_blocks);
	}

	basicblock_chain_clear(ssa->new_blocks);

	for (ite = ssa->jd->exceptiontable; ite; ite = ite->down) {
		bptr = ite->handler;
		catchtype = ite->catchtype;
		exception_entry_chain_init(&chain);
		for (ittry = ite->start; ittry != ite->end; ittry = ittry->next) {
			if (basicblock_reached(ittry)) {
				/* Dead code does not have a basicblock_info_t associated. */
				pei = 0;
				FOR_EACH_INSTRUCTION(ittry, iptr) {
					if (icmd_table[iptr->opc].flags & ICMDTABLE_PEI) {
						/* try is basicblock fragment till (including) the pei */
						try_block = ssa_leave_split_basicblock_at(ssa, ittry, iptr);
						/* ee is handler for try */
						exh = ssa_leave_create_transition_exception_handler(
							ssa, try_block, pei, bptr
						);
						ee = ssa_leave_create_transition_exception_entry(
							ssa, try_block, exh, catchtype
						);
						exception_entry_chain_add(&chain, ee);
						pei += 1;
						ssa->jd->exceptiontablelength += 1;
					}
				}
			}
		}
		if (! exception_entry_chain_empty(&chain)) {
			exception_entry_chain_back(&chain)->down = ite->down;
			exception_entry_chain_back(&chain)->next = ite->next;
			/* Replace original exception entry by first new one. */
			*ite = *exception_entry_chain_front(&chain);
			/* Set current iteration position to last newly created one. */
			ite = exception_entry_chain_back(&chain);
		}
	}

	if (last == NULL) {
		for (last = ssa->jd->basicblocks; last->next != NULL; last = last->next);
	}

	if (last != NULL && ! basicblock_chain_empty(ssa->new_blocks)) {
		last->next = basicblock_chain_front(ssa->new_blocks);
	}
}

void ssa_simple_leave_restore(ssa_info_t *ssa, basicblock *bptr, s4 *pvar) {
	s4 var = *pvar;
	s4 index;
	basicblock_info_t *bbi;

	if (var < ssa->locals->count) {
		*pvar = vars_get_old_index(ssa->locals, var);
	} else if (var < ssa->locals->count + ssa->stack->count) {

		index = vars_get_old_index(
			ssa->stack,
			var - ssa->locals->count
		);

		bbi = bb_info(bptr);

		/* We have to determine whether to take an invar or an outvar for
		   the stack depth ``index''.
		   The state array contains the last definition of the stack element
		   at the given depth.
		*/

		if (state_array_get_var(bbi->stack->state_array, index) == var) {
			/* The last definition of a stack depth inside the basicblock.
			   This is the outvar at the given depth.
			   If there is no outvar at the given depth, it must be an invar.
			*/
			if (index < bptr->outdepth) {
				*pvar = bptr->outvars[index];
			} else if (index < bptr->indepth) {
				*pvar = bptr->invars[index];
			} else {
				assert(0);
			}
		} else {
			/* A different than the last definition of a stack depth.
			   This must be an invar.
			*/
			assert(index < bptr->indepth);
			*pvar = bptr->invars[index];
		}
	} else {
		*pvar = vars_get_old_index(
			ssa->others,
			var - ssa->locals->count - ssa->stack->count
		);
	}
}

void ssa_simple_leave(ssa_info_t *ssa) {
	basicblock *bptr;
	instruction *iptr;
	s4 *ituse, *uses;
	unsigned uses_count;

	FOR_EACH_BASICBLOCK(ssa->jd, bptr) {
		if (bptr->type == basicblock::TYPE_EXH) {
			/* (Aritifical) exception handler blocks will be eliminated. */
			continue;
		}
		/* In reverse order. We need to rename the definition after any use! */
		FOR_EACH_INSTRUCTION_REV(bptr, iptr) {
			if (instruction_has_dst(iptr)) {
				ssa_simple_leave_restore(ssa, bptr, &(iptr->dst.varindex));
			}
			instruction_get_uses(iptr, ssa->s_buf, &uses, &uses_count);
			for (ituse = uses; ituse != uses + uses_count; ++ituse) {
				ssa_simple_leave_restore(ssa, bptr, ituse);
			}
			instruction_set_uses(iptr, ssa->s_buf, uses, uses_count);
		}
		bptr->phicount = 0;
	}

	unfix_exception_handlers(ssa->jd);

	ssa->jd->maxlocals = ssa->original.maxlocals;
	ssa->jd->maxinterfaces = ssa->original.maxinterfaces;
	ssa->jd->local_map =ssa->original.local_map;
	ssa->jd->var = ssa->original.var;
	ssa->jd->vartop = ssa->original.vartop;
	ssa->jd->varcount = ssa->original.varcount;
	ssa->jd->localcount = ssa->original.localcount;
}

#include "vm/rt-timing.hpp"

void yssa(jitdata *jd) {
	basicblock *it;
	basicblock_info_t *iti;
	ssa_info *ssa;

	struct timespec bs, es, be, ee;

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(bs);
#endif

#ifdef SSA_VERBOSE
	bool verb = true;
	if (verb) {
		printf("=============== [ before %s ] =========================\n", jd->m->name.begin());
		show_method(jd, 3);
		printf("=============== [ /before ] =========================\n");
	}
#endif

	ssa = DNEW(ssa_info);

	ssa_info_init(ssa, jd);
	ssa->keep_in_out = true;

	FOR_EACH_BASICBLOCK(jd, it) {
		if (basicblock_reached(it)) {
			iti = DNEW(basicblock_info_t);
			basicblock_info_init(iti, it, jd);
			it->vp = iti;
		} else {
			it->vp = NULL;
		}
	}

	ssa_enter_mark_loops(jd->basicblocks);

	ssa_enter_traverse(ssa, jd->basicblocks);

	ssa_enter_eliminate_categories(ssa);

	ssa_enter_export_variables(ssa);

	ssa_enter_export_phis(ssa);

	ssa_enter_verify_no_redundant_phis(ssa);

	/*ssa_enter_create_phi_graph(ssa);*/

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(be);
#endif
	escape_analysis_perform(ssa->jd);
/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(ee);
#endif

	/*
	ssa_leave_create_phi_moves(ssa);

	ssa_leave_create_exceptional_phi_moves(ssa);
	*/

#ifdef SSA_VERBOSE
	if (verb) {
		printf("=============== [ mid ] =========================\n");
		show_method(jd, 3);
		printf("=============== [ /mid ] =========================\n");
	}
#endif

	ssa_simple_leave(ssa);

#ifdef SSA_VERBOSE
	if (verb) {
		printf("=============== [ after ] =========================\n");
		show_method(jd, 3);
		printf("=============== [ /after ] =========================\n");
	}
#endif

/* TODO port to new rt-timing */
#if 0
	RT_TIMING_GET_TIME(es);

	RT_TIMING_TIME_DIFF(bs, es, RT_TIMING_1);
	RT_TIMING_TIME_DIFF(be, ee, RT_TIMING_2);
#endif
}

void eliminate_subbasicblocks(jitdata *jd) {
	basicblock *bptr, *next;
	basicblock_info_t *bbi;

	FOR_EACH_BASICBLOCK(jd, bptr) {
		bbi = bb_info(bptr);
		if (bbi != NULL) {
			if (! basicblock_chain_empty(bbi->subbasicblocks)) {
				next = bptr->next;
				/* Copy first subblock, to keep pointers intact. */
				*bptr = *basicblock_chain_front(bbi->subbasicblocks);
				bptr = basicblock_chain_back(bbi->subbasicblocks);
				bptr->next = next;
			}
		}
	}

#ifdef SSA_VERBOSE
	printf("=============== [ elim ] =========================\n");
	show_method(jd, 3);
	printf("=============== [ /elim ] =========================\n");
#endif
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
