/* src/vm/optimizing/ssa2.cpp

   Copyright (C) 2008-2013
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

   Reimplementation of code in ssa.c.
   Uses the new dominator tree and the new CFG.
*/

#include "config.h"

#include "mm/dumpmemory.hpp"

#include "toolbox/bitvector.hpp"
#include "toolbox/set.hpp"
#include "toolbox/worklist.hpp"

#include "vm/global.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/show.hpp"

#if 1
#define printf(...) do { if (getenv("VERB")) printf(__VA_ARGS__); } while (0)
#define show_method(...) do { if (getenv("VERB")) show_method(__VA_ARGS__); } while (0)
#endif

typedef struct phi_function {
	s4 dst;
	s4 *args;
} phi_function;

typedef struct basicblock_info {
	bitvector defines;
	bitvector phi;
	unsigned phi_count;
	phi_function *phi_functions;
} basicblock_info;

typedef struct var_info {
	set *work;
	unsigned num_defs;

	unsigned offset;

	unsigned count;
	unsigned *stack;
	unsigned *stack_top;

} var_info;

typedef struct ssa_info {
	jitdata *jd;
	var_info *vars;
	unsigned vars_count;
	unsigned total_local_count;
} ssa_info;

static inline basicblock_info *bb_info(basicblock *bb) {
	return (basicblock_info *)(bb->vp);
}

static ssa_info *ssa_init(jitdata *jd) {
	unsigned i;
	ssa_info *ssa;

	ssa = DNEW(ssa_info);
	ssa->jd = jd;
	ssa->vars_count = jd->localcount;

	ssa->vars = DMNEW(var_info, ssa->vars_count);
	MZERO(ssa->vars, var_info, ssa->vars_count);
	for (i = 0; i < ssa->vars_count; ++i) {
		ssa->vars[i].work = set_new(jd->basicblockcount);
	}

	return ssa;
}

static void ssa_place_phi_functions(ssa_info *ssa) {

	basicblock *bptr, *Y, *n, **itdf;
	basicblock_info *bbi;
	instruction *iptr;
	s4 a;
	set *work;

	for (bptr = ssa->jd->basicblocks; bptr; bptr = bptr->next) {

		bbi = DNEW(basicblock_info);
		bbi->defines = bv_new(ssa->vars_count);
		bbi->phi = bv_new(ssa->vars_count);
		bbi->phi_count = 0;

		bptr->vp = bbi;

		for (iptr = bptr->iinstr; iptr != bptr->iinstr + bptr->icount; ++iptr) {
			if (instruction_has_dst(iptr)) {
				if (
					var_is_local(ssa->jd, iptr->dst.varindex)
				) {
					/* A_orig */
					bv_set_bit(bbi->defines, iptr->dst.varindex);
					/* defsites */
					set_insert(ssa->vars[iptr->dst.varindex].work, bptr);
					/* Accout definition */
					ssa->vars[iptr->dst.varindex].num_defs += 1;
				}
			}
		}
	}

	bptr = ssa->jd->basicblocks;
	bbi = bb_info(bptr);
	for (a = 0;  a < ssa->vars_count; ++a) {
		bv_set_bit(bbi->defines, a);
		set_insert(ssa->vars[a].work, bptr);
		ssa->vars[a].num_defs += 1;
	}

	for (a = 0; a < ssa->vars_count; ++a) {
		work = ssa->vars[a].work;
		while (! set_empty(work)) {
			n = (basicblock *)set_pop(work);
			for (
				itdf = n->domfrontier;
				itdf != n->domfrontier + n->domfrontiercount;
				++itdf
			) {
				Y = *itdf;
				if (! bv_get_bit(bb_info(Y)->phi, a)) {
					bv_set_bit(bb_info(Y)->phi, a);
					printf(" *** BB %d: phi for var %d\n", Y->nr, a);
					bb_info(Y)->phi_count += 1;
					ssa->vars[a].num_defs += 1;
					if (! bv_get_bit(bb_info(Y)->defines, a)) {
						set_insert(work, Y);
					}
				}
			}
		}
	}
}

static void ssa_create_phi_functions(ssa_info *ssa) {
	unsigned i, j;
	basicblock_info *bbi;
	basicblock *bptr;
	phi_function *itph;

	for (bptr = ssa->jd->basicblocks; bptr; bptr = bptr->next) {

		bbi = bb_info(bptr);
		bbi->phi_functions = DMNEW(phi_function, bbi->phi_count);
		itph = bbi->phi_functions;

		for (i = 0; i < ssa->vars_count; ++i) {
			if (bv_get_bit(bbi->phi, i)) {
				itph->dst = i;
				itph->args = DMNEW(s4, bptr->predecessorcount);
				for (j = 0; j < bptr->predecessorcount; ++j) {
					itph->args[j] = i;
				}
				itph += 1;
			}
		}
	}
}

static void ssa_calculate_offsets(ssa_info *ssa) {
	int i;
	unsigned cur_offset = ssa->jd->localcount;

	ssa->total_local_count = 0;

	for (i = 0; i < ssa->vars_count; ++i) {

		ssa->vars[i].offset = cur_offset;

		ssa->total_local_count += ssa->vars[i].num_defs;

		if (ssa->vars[i].num_defs > 1) {
			cur_offset += (ssa->vars[i].num_defs - 1);
		}
	}
}


static s4 ssa_rename_var(ssa_info *ssa, s4 var, unsigned index) {
	s4 ret;
#define return ret=
	if (var_is_local(ssa->jd, var)) {
		assert(0 < index && index <= ssa->vars[var].num_defs);
		if (index == 1) {
			return var;
		} else {
			return ssa->vars[var].offset + (index - 2);
		}
		assert(ret < ssa->total_local_count);
	} else {
		return ssa->total_local_count + (var - ssa->vars_count);
	}
#undef return
	printf(" *** rename %c %d vers %d => %d\n", var_is_local(ssa->jd, var) ? 'L' : 'O',  var, index, ret);
	return ret;
}

static void ssa_rename_uses(ssa_info *ssa, s4 *uses, unsigned uses_count) {
	while (uses_count > 0) {
		if (var_is_local(ssa->jd, *uses)) {
			*uses = ssa_rename_var(ssa, *uses, *(ssa->vars[*uses].stack_top));
		} else {
			*uses = ssa_rename_var(ssa, *uses, 0);
		}
		uses_count -= 1;
		uses += 1;
	}
}

static void ssa_rename_definition(ssa_info *ssa, s4 *pdef) {
	s4 def = *pdef;
	unsigned i = 0;

	if (var_is_local(ssa->jd, def)) {
		ssa->vars[def].count += 1;
		i = ssa->vars[def].count;
		ssa->vars[def].stack_top += 1;
		*(ssa->vars[def].stack_top) = i;
	}

	*pdef = ssa_rename_var(ssa, def, i);
}

static void ssa_rename_block(ssa_info *ssa, basicblock *bptr) {

	basicblock_info *bbi = bb_info(bptr);
	s4 s[3];
	s4 *uses;
	unsigned uses_count;
	instruction *iptr;
	basicblock **itsucc, **itpred, **itdsucc, *Y;
	phi_function *itph;
	unsigned j;
	s4 i, tmp;
	s4 a;
	u4 **orig_stack_top;

	/* XXX */
	orig_stack_top = DMNEW(u4 *, ssa->vars_count);
	for (a = 0; a < ssa->vars_count; ++a) orig_stack_top[a] = ssa->vars[a].stack_top;

		int jj;

printf(" *** === %d ===========\n", bptr->nr);

	ssa_rename_uses(ssa, bptr->invars, bptr->indepth);

	/* Phi functions are the first instructions in the block */
printf(" *** --- phis ---------\n");
	for (
		itph = bbi->phi_functions;
		itph != bbi->phi_functions + bbi->phi_count;
		++itph
	) {
		ssa_rename_definition(ssa, &(itph->dst));
	}

printf(" *** --- vars ---------\n");

	if (bptr == ssa->jd->basicblocks) {
		for (i = 0; i < ssa->jd->localcount; ++i) {
			tmp = i;
			ssa_rename_definition(ssa, &tmp);
		}
	}

	for (iptr = bptr->iinstr; iptr != bptr->iinstr + bptr->icount; ++iptr) {

		/* Determine uses */

		uses_count = 0;

		switch (icmd_table[iptr->opc].dataflow) {
			case DF_3_TO_0:
			case DF_3_TO_1:
				s[2] = iptr->sx.s23.s3.varindex;
				uses_count += 1;

			case DF_2_TO_0:
			case DF_2_TO_1:
				s[1] = iptr->sx.s23.s2.varindex;
				uses_count += 1;

			case DF_1_TO_0:
			case DF_1_TO_1:
			case DF_COPY:
			case DF_MOVE:
				s[0] = iptr->s1.varindex;
				uses_count += 1;

				uses = s;
				break;

			case DF_N_TO_1:
			case DF_INVOKE:
			case DF_BUILTIN:

				uses = iptr->sx.s23.s2.args;
				uses_count = iptr->s1.argcount;
				break;

		}

		printf(" *** %s uses ", icmd_table[iptr->opc].name);
		for (jj = 0; jj < uses_count; ++jj) printf("%d ",uses[jj]);
		printf("\n");

		if (uses_count > 0) {
			/* Update uses, if there are any */

			ssa_rename_uses(ssa, uses, uses_count);

			/* If uses were s, then we need to update the instruction */

			if (uses == s) {
				switch (uses_count) {
					case 3:
						iptr->sx.s23.s3.varindex = s[2];
					case 2:
						iptr->sx.s23.s2.varindex = s[1];
					case 1:
						iptr->s1.varindex = s[0];
				}
			}
		}

		/* Rename definitions */

		if (instruction_has_dst(iptr)) {
			printf(" *** %s defines %d\n", icmd_table[iptr->opc].name, iptr->dst.varindex);
			ssa_rename_definition(ssa, &(iptr->dst.varindex));
		}

	}

	for (i = 0; i < bptr->outdepth; ++i) {
		ssa_rename_definition(ssa, bptr->outvars + i);
	}

	/* Successors */

	printf(" *** succs %d\n", bptr->successorcount);

	for (
		itsucc = bptr->successors;
		itsucc != bptr->successors + bptr->successorcount;
		++itsucc
	) {
		Y = *itsucc;

		for (
			itpred = Y->predecessors, j = 0;
			itpred != Y->predecessors + Y->predecessorcount;
			++itpred, ++j
		) {
			if (*itpred == bptr) break;
		}

		assert(j != Y->predecessorcount);

		for (
			itph = bb_info(Y)->phi_functions;
			itph != bb_info(Y)->phi_functions + bb_info(Y)->phi_count;
			++itph
		) {
			ssa_rename_uses(ssa, itph->args + j, 1);
		}
	}

	/* Recurse */

	for (
		itdsucc = bptr->domsuccessors;
		itdsucc != bptr->domsuccessors + bptr->domsuccessorcount;
		++itdsucc
	) {
		ssa_rename_block(ssa, *itdsucc);
	}

	/* For each definition of some variable a in the original S, pop stack */

	/* XXX */
	for (a = 0; a < ssa->vars_count; ++a)  ssa->vars[a].stack_top = orig_stack_top[a];
}

static void ssa_rename(ssa_info *ssa) {
	unsigned i;

	for (i = 0; i < ssa->vars_count; ++i) {
		ssa->vars[i].stack = DMNEW(unsigned, ssa->vars[i].num_defs + 1);
		ssa->vars[i].stack[0] = 0;
		ssa->vars[i].stack_top = ssa->vars[i].stack;
	}

	ssa_rename_block(ssa, ssa->jd->basicblocks);
}

static void ssa_export(ssa_info *ssa) {
	unsigned i, j;
	jitdata *jd = ssa->jd;
	methoddesc *md = jd->m->parseddesc;
	varinfo *vars, *it;
	s4 vartop, varindex;

	vartop = ssa->total_local_count + jd->vartop - jd->localcount;
	vars = DMNEW(varinfo, vartop);

	printf(" *** vartop(%d) = ssa->total_local_count(%d) + jd->vartop(%d) - jd->localcount(%d)\n",
		vartop , ssa->total_local_count , jd->vartop , jd->localcount);

	it = vars;

	/* Version 1 of each local */

	for (i = 0; i < jd->localcount; ++i) {
		*(it++) = jd->var[i];
	}

	/* Other versions of each local */

	for (i = 0; i < jd->localcount; ++i) {
		for (j = 1; j < ssa->vars[i].num_defs; ++j) {
			*(it++) = jd->var[i];
		}
	}

	/* Other vars */

	for (i = jd->localcount; i < jd->vartop; ++i) {
		*(it++) = jd->var[i];
	}

	jd->var = vars;
	jd->vartop = jd->varcount = vartop;

	jd->local_map = DMREALLOC(jd->local_map, s4, 5 * jd->maxlocals, 5 * (jd->maxlocals + ssa->total_local_count - jd->localcount));

	for (i = 0; i < ssa->total_local_count - jd->localcount; ++i) {
		for (j = 0; j < 5; ++j) {
			varindex = jd->localcount + i;
			if (jd->var[varindex].type != j) {
				varindex = jitdata::UNUSED;
			}
			jd->local_map[((jd->maxlocals + i) * 5) + j] = varindex;
		}
	}

	jd->maxlocals += (ssa->total_local_count - jd->localcount);
	jd->localcount = ssa->total_local_count;

	printf(" *** jd->localcount %d, jd->maxlocals %d\n", jd->localcount , jd->maxlocals);
}

static unsigned get_predecessor_index(basicblock *from, basicblock *to) {
	basicblock **itpred;
	unsigned j = 0;

	for (itpred = to->predecessors; itpred != to->predecessors + to->predecessorcount; ++itpred) {
		if (*itpred == from) break;
		j++;
	}

	if (j == to->predecessorcount) {
		printf(" *** %d => %d\n", from->nr, to->nr);
		assert(j != to->predecessorcount);
	}

	return j;
}

static basicblock *create_block(ssa_info *ssa, basicblock *from, basicblock *to) {
	basicblock *mid;
	basicblock_info *toi;
	instruction *iptr;
	phi_function *itph;
	unsigned j = get_predecessor_index(from, to);

	mid = DNEW(basicblock);
	MZERO(mid, basicblock, 1);

	toi = bb_info(to);
	assert(toi);

	mid->nr = ssa->jd->basicblockcount;
	ssa->jd->basicblockcount += 1;
	mid->mpc = -1;
	mid->type = (basicblock::Type) 666;
	mid->icount = toi->phi_count + 1;
	iptr = mid->iinstr = DMNEW(instruction, mid->icount);
	MZERO(mid->iinstr, instruction, mid->icount);

	for (itph = toi->phi_functions; itph != toi->phi_functions + toi->phi_count; ++itph) {
		iptr->opc = ICMD_COPY;
		iptr->dst.varindex = itph->dst;
		iptr->s1.varindex =  itph->args[j];
		assert(itph->dst < ssa->total_local_count);
		assert(itph->args[j] < ssa->total_local_count);
		iptr++;
	}

	iptr->opc = ICMD_GOTO;
	iptr->dst.block = to;

	while (from->next) {
		from = from->next;
	}

	from->next = mid;

	return mid;
}

static void crate_fallthrough(ssa_info *ssa, basicblock *bptr) {
	unsigned j;
	basicblock_info *toi;
	instruction *iptr;
	phi_function *itph;

	if (bptr->next == NULL) return;

	j = get_predecessor_index(bptr, bptr->next);

	toi = bb_info(bptr->next);
	assert(toi);

	bptr->iinstr = DMREALLOC(bptr->iinstr, instruction, bptr->icount, bptr->icount + toi->phi_count);
	iptr = bptr->iinstr + bptr->icount;
	bptr->icount += toi->phi_count;

	for (itph = toi->phi_functions; itph != toi->phi_functions + toi->phi_count; ++itph) {
		iptr->opc = ICMD_COPY;
		iptr->dst.varindex = itph->dst;
		iptr->s1.varindex =  itph->args[j];
		assert(itph->dst < ssa->total_local_count);
		assert(itph->args[j] < ssa->total_local_count);
		iptr++;
	}

}

static void ssa_create_phi_moves(ssa_info *ssa) {
	basicblock *bptr;
	instruction *iptr;

	s4 i, l;
	branch_target_t *table;
	lookup_target_t *lookup;
	bool gt;

	for (bptr = ssa->jd->basicblocks; bptr; bptr = bptr->next) {
		if (bptr->type == basicblock::Type(666)) {
			bptr->type = basicblock::TYPE_STD;
			continue;
		}
		if (! bptr->vp) continue;
		if (! (bptr->state >= basicblock::REACHED)) continue;
		gt = false;
		for (iptr = bptr->iinstr; iptr != bptr->iinstr + bptr->icount; ++iptr) {
			switch (icmd_table[iptr->opc].controlflow) {
				case CF_IF:
				case CF_RET:
				case CF_GOTO:
					iptr->dst.block = create_block(ssa, bptr, iptr->dst.block);
					break;
				case CF_TABLE:
					table = iptr->dst.table;
					l = iptr->sx.s23.s2.tablelow;
					i = iptr->sx.s23.s3.tablehigh;
					i = i - l + 1;
					i += 1; /* default */
					while (--i >= 0) {
						table->block = create_block(ssa, bptr, table->block);
						++table;
					}
					break;
				case CF_LOOKUP:
					lookup = iptr->dst.lookup;
					i = iptr->sx.s23.s2.lookupcount;
					while (--i >= 0) {
						lookup->target.block = create_block(ssa, bptr, lookup->target.block);
						lookup++;
					}
					iptr->sx.s23.s3.lookupdefault.block = create_block(ssa, bptr, iptr->sx.s23.s3.lookupdefault.block);
					break;
				case CF_JSR:
					iptr->sx.s23.s3.jsrtarget.block = create_block(ssa, bptr, iptr->sx.s23.s3.jsrtarget.block);
					break;
			}
			if ((iptr->opc == ICMD_GOTO) || icmd_table[iptr->opc].controlflow == CF_END)
				gt = true;
			else if (iptr->opc != ICMD_NOP)
				gt = false;
		}
		if (! bptr->next) continue;
		if (! (bptr->next->state >= basicblock::REACHED)) continue;
		if (bptr->next->type == basicblock::Type(666)) continue;
		if (!gt) crate_fallthrough(ssa, bptr);
	}
}

void xssa(jitdata *jd) {
	ssa_info *ssa = ssa_init(jd);

	printf("=============== [ before %s ] =========================\n", jd->m->name.begin());
	show_method(jd, 3);
	printf("=============== [ /before ] =========================\n");

	ssa_place_phi_functions(ssa);
	ssa_create_phi_functions(ssa);
	ssa_calculate_offsets(ssa);
	ssa_rename(ssa);
	ssa_export(ssa);
	ssa_create_phi_moves(ssa);

	printf("=============== [ after ] =========================\n");
	show_method(jd, 3);
	printf("=============== [ /after ] =========================\n");
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
