/* src/vm/jit/optimizing/lsra.inc - linear scan register allocator

   Copyright (C) 2005-2013
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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "arch.hpp"
#include "md-abi.hpp"

#include "mm/dumpmemory.hpp"

#include "toolbox/bitvector.hpp"

#include "vm/statistics.hpp"
#include "vm/options.hpp"
#include "vm/method.hpp"
#include "vm/descriptor.hpp"

#include "vm/jit/abi.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/reg.hpp"
#include "vm/jit/jit.hpp"

#include "vm/jit/optimizing/graph.hpp"
#include "vm/jit/optimizing/lifetimes.hpp"
#include "vm/jit/optimizing/ssa.hpp"

#include "vm/jit/optimizing/lsra.hpp"

#include "toolbox/logging.hpp"

STAT_DECLARE_VAR(int,count_locals_conflicts,0)

extern const char *string_java_lang_InternalError;
/* function prototypes */
void lsra_setup(jitdata *);
void lsra_main(jitdata *);
#ifdef LSRA_DEBUG_VERBOSE
void lsra_dump_stack(stackelement_t*);
void print_lifetimes(jitdata *, int *, int);
void print_all_lifetimes(jitdata *);
#endif
void lsra_reg_setup(jitdata *,struct lsra_register *,
					struct lsra_register *);

void lsra_calc_lifetime_length(jitdata *);

void _lsra_main( jitdata *, int *, int, struct lsra_register *,
				 int *);
void lsra_expire_old_intervalls(jitdata *, struct lifetime *,
								struct lsra_register *);
void spill_at_intervall(jitdata *, struct lifetime *);
void lsra_add_active(struct lifetime *, struct lifetime **, int *);
void _lsra_expire_old_intervalls(jitdata *, struct lifetime *,
								 struct lsra_register *,
								 struct lifetime **, int *);
void _spill_at_intervall(struct lifetime *, struct lifetime **, int *);
void lsra_alloc(jitdata *, int *, int,
				int *);
int lsra_getmem(struct lifetime *, struct freemem *, int *);
struct freemem *lsra_getnewmem(int *);

void lsra(jitdata *jd) {
	methodinfo *m;
	codegendata *cd;
	registerdata *rd;
	lsradata *ls;
#if defined(ENABLE_STATISTICS)
	int locals_start;
	int i,j;
#endif
#if defined(LSRA_DEBUG_CHECK)
#if 0
	int b_index;
	stackelement_t* in,out;
	int      ind, outd;
#endif
#endif

	m = jd->m;
	cd = jd->cd;
	rd = jd->rd;
	ls = jd->ls;

#if defined(LSRA_DEBUG_CHECK)
#if 0
	b_index = 0;
	while (b_index < jd->basicblockcount ) {

		if (jd->basicblocks[b_index].flags >= basicblock::REACHED) {

			in=m->basicblocks[b_index].instack;
			ind=m->basicblocks[b_index].indepth;
			for (;ind != 0;in=in->prev, ind--) {
				/* ARGVAR or LOCALVAR in instack is ok*/
#if defined(LSRA_DEBUG_VERBOSE)
				if (compileverbose) {
					if (in->varkind == ARGVAR) printf("ARGVAR in instack: \n");
					if (in->varkind == LOCALVAR)
						printf("LOCALVAR in instack\n");
				}
#endif
			}
			out=m->basicblocks[b_index].outstack;
			outd=m->basicblocks[b_index].outdepth;
			for (;outd != 0;out=out->prev, outd--) {
				if (out->varkind == ARGVAR)
					{ log_text("ARGVAR in outstack\n"); assert(0); }
				if (out->varkind == LOCALVAR)
					{ log_text("LOCALVAR in outstack\n"); assert(0); }
			}
		}
			b_index++;
	}
#endif
#endif

#if defined(LSRA_DEBUG_CHECK) || defined(LSRA_DEBUG_VERBOSE)
#if defined(LSRA_DEBUG_VERBOSE)
	if (compileverbose) {
		printf("%s %s ", m->clazz->name.begin(), m->name.begin());
		if (code_is_leafmethod(jd->code))
			printf("**Leafmethod**");
		printf("\n");
	}
#endif
	if (strcmp(m->clazz->name.begin(),"java/lang/String")==0)
		if (strcmp(m->name.begin(),"toLowerCase")==0)
#if defined(LSRA_DEBUG_VERBOSE)
			if (compileverbose)
				printf("12-------------------12\n");
#else
	        { int dummy=1; dummy++; }
#endif
#endif

	lsra_setup(jd);

#if defined(ENABLE_STATISTICS)
	/* find conflicts between locals for statistics */
	if (opt_stat) {
		/* local Variable Lifetimes are at the end of the lifetime array and */
		/* have v_index >= 0 */
		for (locals_start = ls->lifetimecount-1; (locals_start >=0) &&
			(ls->lifetime[ls->lt_used[locals_start]].v_index >= 0);
			 locals_start--);
		for (i=locals_start + 1; i < ls->lifetimecount; i++)
			for (j=i+1; j < ls->lifetimecount; j++)
				if ( !((ls->lifetime[ls->lt_used[i]].i_end
					   < ls->lifetime[ls->lt_used[j]].i_start)
					|| (ls->lifetime[ls->lt_used[j]].i_end <
					   ls->lifetime[ls->lt_used[i]].i_start)) )
					count_locals_conflicts += 2;
	 }
#endif
	/* Run LSRA */
	lsra_main(jd);

	fflush(stdout);
}


void lsra_setup(jitdata *jd)
{
	methodinfo *m;
	codegendata *cd;
	registerdata *rd;
	lsradata *ls;

#if defined(ENABLE_LOOPS)
	/* Loop optimization "destroys" the basicblock array */
	/* TODO: work with the basicblock list               */
	if (opt_loops) {
		log_text("lsra not possible with loop optimization\n");
		exit(1);
	}
#endif

	m = jd->m;
	cd = jd->cd;
	rd = jd->rd;
	ls = jd->ls;

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Lifetimes after LifenessAnalyse: \n");
		print_all_lifetimes(jd);
	}
#endif

	lsra_calc_lifetime_length(jd);

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Basicblockcount: %4i\n",ls->basicblockcount);
	}
#endif
}

void lsra_reg_setup(jitdata *jd,
					struct lsra_register *int_reg,
					struct lsra_register *flt_reg ) {
	int i, j, iarg, farg;
	int int_sav_top;
	int flt_sav_top;
	bool *fltarg_used, *intarg_used;
	methoddesc *md;
	methodinfo *m;
	registerdata *rd;

	m = jd->m;
	rd = jd->rd;
	md = m->parseddesc;

	int_reg->nregdesc = nregdescint;
	flt_reg->nregdesc = nregdescfloat;
	if (code_is_leafmethod(jd->code)) {
		/* Temp and Argumentregister can be used as saved registers */

		int_reg->sav_top = INT_ARG_CNT + INT_TMP_CNT + INT_SAV_CNT;
		int_reg->sav_reg = DMNEW(int, int_reg->sav_top);
		int_reg->tmp_reg = NULL;
		int_reg->tmp_top = -1;
		flt_reg->sav_top = FLT_ARG_CNT + FLT_TMP_CNT + FLT_SAV_CNT;
		flt_reg->sav_reg = DMNEW(int, flt_reg->sav_top);
		flt_reg->tmp_reg = NULL;
		flt_reg->tmp_top = -1;

		/* additionaly precolour registers for Local Variables acting as */
		/* Parameters */

		farg = FLT_ARG_CNT;
		iarg = INT_ARG_CNT;

		intarg_used = DMNEW(bool, INT_ARG_CNT);
		for (i=0; i < INT_ARG_CNT; i++)
			intarg_used[i]=false;

		fltarg_used = DMNEW(bool, FLT_ARG_CNT);
		for (i=0; i < FLT_ARG_CNT; i++)
			fltarg_used[i]=false;

		int_sav_top=int_reg->sav_top;
		flt_sav_top=flt_reg->sav_top;

		for (i=0; (i < md->paramcount); i++) {
			if (!md->params[i].inmemory) {
				if (IS_INT_LNG_TYPE(md->paramtypes[i].type)) {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
					if (IS_2_WORD_TYPE(md->paramtypes[i].type)) {
						int_reg->sav_reg[--int_sav_top] =
							GET_HIGH_REG(md->params[i].regoff);
						intarg_used[GET_HIGH_REG(md->params[i].regoff)]=true;
						/*used -> don't copy later on */
						int_reg->sav_reg[--int_sav_top] =
							GET_LOW_REG(md->params[i].regoff);
						intarg_used[GET_LOW_REG(md->params[i].regoff)]=true;
						/*used -> don't copy later on */
					} else
#endif
					{ /* !IS_2_WORD_TYPE(md->paramtypes[i].type */
						int_reg->sav_reg[--int_sav_top] =
							md->params[i].regoff;
						intarg_used[md->params[i].regoff]=true;
						/*used -> don't copy later on */
					}
				}
#if !defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
				/* do not precolour float arguments if they are passed in     */
				/* integer registers. But these integer argument registers    */
				/* still be used in the method! */
				else { /* IS_FLT_DBL_TYPE(md->paramtypes[i].type */
						flt_reg->sav_reg[--flt_sav_top] =
							md->params[i].regoff;
						fltarg_used[md->params[i].regoff]=true;
				}
#endif

			}
		}

		/* copy rest of argument registers to flt_reg->sav_reg and */
		/* int_reg->sav_reg; */
		for (i=0; i < INT_ARG_CNT; i++)
			if (!intarg_used[i])
				int_reg->sav_reg[--int_sav_top]=i;
		for (i=0; i < FLT_ARG_CNT; i++)
			if (!fltarg_used[i])
				flt_reg->sav_reg[--flt_sav_top]=i;

		/* copy temp registers to flt_reg->sav_reg and int_reg->sav_reg */
		for (i=0; i < INT_TMP_CNT; i++)
			int_reg->sav_reg[--int_sav_top]=rd->tmpintregs[i];
		for (i=0; i < FLT_TMP_CNT; i++)
			flt_reg->sav_reg[--flt_sav_top]=rd->tmpfltregs[i];

	} else {
		/* non leaf method -> use Argument Registers [arg[int|flt]reguse */
		/* ... [INT|FLT]_ARG_CNT[ as temp reg */
		/* divide temp and saved registers */

		int argintreguse, argfltreguse;

		/* with Locals as non SAVEDVAR, the used arg[int|flt] as in params */
		/* of the method itself have to be regarded, or mismatch before    */
		/* block 0 with parameter copy could happen! */

		argintreguse = MAX(rd->argintreguse, md->argintreguse);
		argfltreguse = MAX(rd->argfltreguse, md->argfltreguse);

		int_sav_top = int_reg->sav_top = INT_SAV_CNT;
		int_reg->sav_reg = DMNEW(int, int_reg->sav_top);
		int_reg->tmp_top = INT_TMP_CNT +
			MAX(0, (INT_ARG_CNT - argintreguse));
		int_reg->tmp_reg = DMNEW(int, int_reg->tmp_top);

		flt_sav_top =flt_reg->sav_top = FLT_SAV_CNT;
		flt_reg->sav_reg = DMNEW(int, flt_reg->sav_top);
		flt_reg->tmp_top = FLT_TMP_CNT +
			MAX(0 , (FLT_ARG_CNT - argfltreguse));
		flt_reg->tmp_reg = DMNEW(int, flt_reg->tmp_top);

		/* copy temp and unused argument registers to flt_reg->tmp_reg and */
		/* int_reg->tmp_reg */

		for (i=0; i < INT_TMP_CNT; i++)
			int_reg->tmp_reg[i]=rd->tmpintregs[i];

		/* quick and dirty patch for the drop of rd->argxxxreg[] - but will */
		/* work anyhow on i386, !! has to be made "real" for other archs    */

		for (j = argintreguse; j < INT_ARG_CNT; j++, i++)
			int_reg->tmp_reg[i]=j;
		for (i=0; i < FLT_TMP_CNT; i++)
			flt_reg->tmp_reg[i]=rd->tmpfltregs[i];

		/* quick and dirty patch for the drop of rd->argxxxreg[] - but will */
		/* work anyhow on i386, !! has to be made "real" for other archs    */

		for (j = argfltreguse; j < FLT_ARG_CNT; j++, i++)
			flt_reg->tmp_reg[i]=j;
	}

	/* now copy saved registers to flt_reg->sav_reg and int_reg->sav_reg */
	for (i = INT_SAV_CNT-1; i >= 0; i--)
		int_reg->sav_reg[--int_sav_top]=rd->savintregs[i];
	for (i = FLT_SAV_CNT-1; i >= 0; i--)
		flt_reg->sav_reg[--flt_sav_top]=rd->savfltregs[i];
	/* done */
}

void lsra_insertion( struct lsradata *ls, int *a, int lo, int hi) {
	int i,j,t,tmp;

	for (i=lo+1; i<=hi; i++) {
		j=i;
		t=ls->lifetime[a[j]].i_start;
		tmp = a[j];
		while ((j>lo) && (ls->lifetime[a[j-1]].i_start > t)) {
			a[j]=a[j-1];
			j--;
		}
		a[j]=tmp;
	}
}

void lsra_qsort( struct lsradata *ls, int *a, int lo, int hi) {
	int i,j,x,tmp;
	if (lo < hi) {
		if ( (lo+5)<hi) {
			i = lo;
			j = hi;
			x = ls->lifetime[a[(lo+hi)/2]].i_start;

			while (i <= j) {
				while (ls->lifetime[a[i]].i_start < x) i++;
				while (ls->lifetime[a[j]].i_start > x) j--;
				if (i <= j) {
					/* exchange a[i], a[j] */
					tmp = a[i];
					a[i] = a[j];
					a[j] = tmp;

					i++;
					j--;
				}
			}

			if (lo < j) lsra_qsort( ls, a, lo, j);
			if (i < hi) lsra_qsort( ls, a, i, hi);
		} else
			lsra_insertion(ls, a, lo, hi);
	}
}

void lsra_param_sort(struct lsradata *ls, int *lifetime, int lifetime_count) {

	int param_count;
	int i,j,tmp;

	/* count number of parameters ( .i_start == 0) */
	for (param_count=0; (param_count < lifetime_count) &&
		 (ls->lifetime[lifetime[param_count]].i_start == 0); param_count++);

	if (param_count > 0) {
		/* now sort the parameters by v_index */
		for (i=0; i < param_count -1; i++)
			for (j=i+1; j < param_count; j++)
				if ( ls->lifetime[lifetime[i]].v_index >
					 ls->lifetime[lifetime[j]].v_index) {
					/* swap */
					tmp = lifetime[i];
					lifetime[i]=lifetime[j];
					lifetime[j]=tmp;
				}
	}
}

void lsra_main(jitdata *jd)
{
#ifdef LSRA_DEBUG_VERBOSE
	int i;
#endif
	int lsra_mem_use;
	int lsra_reg_use;
	struct lsra_register flt_reg, int_reg;
	methodinfo *m;
	registerdata *rd;
	lsradata *ls;

	ls = jd->ls;
	m = jd->m;
	rd = jd->rd;

/* sort lifetimes by increasing start */
	lsra_qsort( ls, ls->lt_mem, 0, ls->lt_mem_count - 1);
	lsra_qsort( ls, ls->lt_int, 0, ls->lt_int_count - 1);
	lsra_qsort( ls, ls->lt_flt, 0, ls->lt_flt_count - 1);
/* sort local vars used as parameter */
	lsra_param_sort( ls, ls->lt_int, ls->lt_int_count);
	lsra_param_sort( ls, ls->lt_flt, ls->lt_flt_count);
	lsra_reg_setup(jd, &int_reg, &flt_reg);

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("INTSAV REG: ");
		for (i=0; i<int_reg.sav_top; i++)
			printf("%2i ",int_reg.sav_reg[i]);
		printf("\nINTTMP REG: ");
		for (i=0; i<int_reg.tmp_top; i++)
			printf("%2i ",int_reg.tmp_reg[i]);
		printf("\nFLTSAV REG: ");
		for (i=0; i<flt_reg.sav_top; i++)
			printf("%2i ",flt_reg.sav_reg[i]);
		printf("\nFLTTMP REG: ");
		for (i=0; i<flt_reg.tmp_top; i++)
			printf("%2i ",flt_reg.tmp_reg[i]);
		printf("\n");
	}
#endif

	ls->active_tmp = DMNEW( struct lifetime *, MAX(INT_REG_CNT, FLT_REG_CNT));
	ls->active_sav = DMNEW( struct lifetime *, MAX(INT_REG_CNT, FLT_REG_CNT));

	lsra_reg_use=INT_SAV_CNT; /* init to no saved reg used... */
	_lsra_main(jd, ls->lt_int, ls->lt_int_count, &int_reg,
				   &lsra_reg_use);
	if (lsra_reg_use > INT_SAV_CNT) lsra_reg_use=INT_SAV_CNT;
	rd->savintreguse = lsra_reg_use;

	lsra_reg_use = FLT_SAV_CNT; /* no saved reg used... */

	_lsra_main(jd, ls->lt_flt, ls->lt_flt_count, &flt_reg, &lsra_reg_use);
	if (lsra_reg_use > FLT_SAV_CNT) lsra_reg_use=FLT_SAV_CNT;

	rd->savfltreguse=lsra_reg_use;

	/* rd->memuse was already set in stack.c to allocate stack space for */
	/* passing arguments to called methods */
#if defined(__I386__)
	if (checksync && (m->flags & ACC_SYNCHRONIZED)) {
		/* reserve 0(%esp) for Monitorenter/exit Argument on i386 */
		if (rd->memuse < 1)
			rd->memuse = 1;
	}
#endif

	lsra_mem_use = rd->memuse; /* Init with memuse from stack.c */

	lsra_alloc(jd, ls->lt_mem, ls->lt_mem_count, &lsra_mem_use);
	lsra_alloc(jd, ls->lt_int, ls->lt_int_count, &lsra_mem_use);
	lsra_alloc(jd, ls->lt_flt, ls->lt_flt_count, &lsra_mem_use);

	rd->memuse=lsra_mem_use;

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("Int RA complete \n");
		printf("Lifetimes after splitting int: \n");
		print_lifetimes(jd, ls->lt_int, ls->lt_int_count);

		printf("Flt RA complete \n");
		printf("Lifetimes after splitting flt:\n");
		print_lifetimes(jd, ls->lt_flt, ls->lt_flt_count);

		printf("Rest RA complete \n");
		printf("Lifetimes after leftt:\n");
		print_lifetimes(jd, ls->lt_mem, ls->lt_mem_count);

		printf("jd->varcount: %i jd->vartop %i\n", jd->varcount, jd->vartop);
	}
#endif
}

void lsra_alloc(jitdata *jd, int *lifet, int lifetimecount, int *mem_use)
{
	int flags,regoff;
	struct lifetime *lt;
	struct freemem *fmem;
	int lt_index;
	methodinfo *m;
	registerdata *rd;
	lsradata *ls;

	m = jd->m;
	rd = jd->rd;
	ls = jd->ls;

	fmem=DNEW(struct freemem);
	fmem->off=-1;
	fmem->next=NULL;

	for (lt_index = 0; lt_index < lifetimecount; lt_index ++) {
		lt = ls->lifetime + lifet[lt_index];
#ifdef LSRA_MEMORY
		lt->reg=-1;
#endif
		if (lt->regoff == -1) {
			flags = INMEMORY;
			regoff = lsra_getmem(lt, fmem, mem_use);
		} else {
			flags = lt->savedvar;
			regoff = lt->regoff;
		}

		lt->regoff = regoff;
		VAR(lt->v_index)->vv.regoff = regoff;
		VAR(lt->v_index)->flags  = flags;
	}
}

int lsra_getmem(struct lifetime *lt, struct freemem *fmem, int *mem_use)
{
	struct freemem *fm, *p;

	/* no memmory allocated till now, or all other are still live */
	if ((fmem->next == NULL) || (fmem->next->end > lt->i_start)) {
/* 	if (1) { */
		fm=lsra_getnewmem(mem_use);
	} else {
		/* Speicherstelle frei */
		fm=fmem->next;
		fmem->next=fm->next;
		fm->next=NULL;
	}
	fm->end=lt->i_end;
	for (p=fmem; (p->next!=NULL) && (p->next->end < fm->end); p=p->next);
	fm->next=p->next;
	p->next=fm;
	/* HACK: stackslots are 8 bytes on all architectures for now, I hope.
	 * -- pm
	 */
	return fm->off * 8;
}

struct freemem *lsra_getnewmem(int *mem_use)
{
	struct freemem *fm;

	fm=DNEW(struct freemem);
	fm->next=NULL;
	fm->off=*mem_use;
	(*mem_use)++;
	return fm;
}

void _lsra_main( jitdata *jd, int *lifet, int lifetimecount,
				 struct lsra_register *reg, int *reg_use)
{
	struct lifetime *lt;
	int lt_index;
	int reg_index;
	int regsneeded;
	bool temp; /* reg from temp registers (true) or saved registers (false) */
	methodinfo *m;
	lsradata *ls;

	m = jd->m;
	ls = jd->ls;

#if !defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
	regsneeded = 0;
#endif
	if ((reg->tmp_top+reg->sav_top) == 0) {

		/* no registers available */
		for (lt_index = 0; lt_index < lifetimecount; lt_index++)
			ls->lifetime[lifet[lt_index]].regoff = -1;
		return;
	}

	ls->active_tmp_top = 0;
	ls->active_sav_top = 0;

	for (lt_index = 0; lt_index < lifetimecount; lt_index++) {
		lt = &(ls->lifetime[lifet[lt_index]]);

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
	regsneeded = (lt->type == TYPE_LNG)?1:0;
#endif

		lsra_expire_old_intervalls(jd, lt, reg);
		reg_index = -1;
		temp = false;
#ifdef LSRA_SAVEDVAR
		lt->savedvar = SAVEDVAR;
#endif
		if (lt->savedvar || code_is_leafmethod(jd->code)) {
			/* use Saved Reg (in case of leafmethod all regs are saved regs) */
			if (reg->sav_top > regsneeded) {
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				if (regsneeded)
					reg_index = PACK_REGS(reg->sav_reg[--reg->sav_top],
										  reg->sav_reg[--reg->sav_top]);
				else
#endif

					reg_index = reg->sav_reg[--reg->sav_top];
			}
		} else { /* use Temp Reg or if none is free a Saved Reg */
			if (reg->tmp_top > regsneeded) {
				temp = true;
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (regsneeded)
				reg_index = PACK_REGS(reg->tmp_reg[--reg->tmp_top],
									  reg->tmp_reg[--reg->tmp_top]);
			else
#endif
				reg_index = reg->tmp_reg[--reg->tmp_top];
			}
			else
				if (reg->sav_top > regsneeded) {

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
				if (regsneeded)
					reg_index = PACK_REGS(reg->sav_reg[--reg->sav_top],
										  reg->sav_reg[--reg->sav_top]);
				else
#endif
					reg_index = reg->sav_reg[--reg->sav_top];
				}
		}
		if (reg_index == -1) /* no reg is available anymore... -> spill */
			spill_at_intervall(jd, lt);
		else {
			lt->regoff = reg_index;
			if (temp)
				lsra_add_active(lt, ls->active_tmp, &(ls->active_tmp_top));
			else {
				if (reg->sav_top<*reg_use) *reg_use=reg->sav_top;
				lsra_add_active(lt, ls->active_sav, &(ls->active_sav_top));
			}
		}
	}
}

void lsra_add_active(struct lifetime *lt, struct lifetime **active,
					 int *active_top)
{
	int i, j;

	for(i = 0; (i < *active_top) && (active[i]->i_end < lt->i_end); i++);

	for(j = *active_top; j > i; j--) active[j] = active[j-1];

	(*active_top)++;

	active[i] = lt;

}

void lsra_expire_old_intervalls(jitdata *jd,
								struct lifetime *lt, struct lsra_register *reg)
{
	_lsra_expire_old_intervalls(jd, lt, reg, jd->ls->active_tmp,
								&(jd->ls->active_tmp_top));
	_lsra_expire_old_intervalls(jd, lt, reg, jd->ls->active_sav,
								&(jd->ls->active_sav_top));
}

void _lsra_expire_old_intervalls(jitdata *jd, struct lifetime *lt,
								 struct lsra_register *reg,
								 struct lifetime **active, int *active_top)
{
	int i, j, k;

	for(i = 0; i < *active_top; i++) {
		if (active[i]->i_end > lt->i_start) break;

		/* make active[i]->reg available again */
		if (code_is_leafmethod(jd->code)) {
			/* leafmethod -> don't care about type -> put all again into */
			/* reg->sav_reg */
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (active[i]->type == TYPE_LNG) {
				reg->sav_reg[reg->sav_top++] = GET_LOW_REG(active[i]->reg);
				reg->sav_reg[reg->sav_top++] = GET_HIGH_REG(active[i]->reg);
			} else
#endif
				reg->sav_reg[reg->sav_top++] = active[i]->regoff;
		} else {
			/* no leafmethod -> distinguish between temp and saved register */
#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			if (active[i]->type == TYPE_LNG) {
				/* no temp and saved regs are packed together, so looking at */
				/* LOW_REG is sufficient */
				if ( reg->nregdesc[ GET_LOW_REG(active[i]->reg)] == REG_SAV) {
					reg->sav_reg[reg->sav_top++] = GET_LOW_REG(active[i]->reg);
					reg->sav_reg[reg->sav_top++] = GET_HIGH_REG(active[i]->reg);
				} else {
					reg->tmp_reg[reg->tmp_top++] = GET_LOW_REG(active[i]->reg);
					reg->tmp_reg[reg->tmp_top++] = GET_HIGH_REG(active[i]->reg);
				}
			} else
#endif
			if ( reg->nregdesc[active[i]->regoff] == REG_SAV) {
					reg->sav_reg[reg->sav_top++] = active[i]->regoff;
			} else {
					reg->tmp_reg[reg->tmp_top++] = active[i]->regoff;
			}
		}
	}

	/* active[0..i[ is to be removed */
	/* -> move [i..*active_top[ to [0..*active_top-i[ */
	for(k = 0, j = i; (j < *active_top); k++,j++)
		active[k] = active[j];

	(*active_top) -= i;

}

void spill_at_intervall(jitdata *jd, struct lifetime *lt )
{
	lsradata *ls;

	ls = jd->ls;

	if (lt->savedvar || code_is_leafmethod(jd->code)) {
		_spill_at_intervall(lt, ls->active_sav, &(ls->active_sav_top));
	} else {
		_spill_at_intervall(lt, ls->active_tmp, &(ls->active_tmp_top));
		if (lt->regoff == -1) { /* kein tmp mehr frei gewesen */
			_spill_at_intervall(lt, ls->active_sav, &(ls->active_sav_top));
		}
	}
}

void _spill_at_intervall(struct lifetime *lt, struct lifetime **active,
						 int *active_top)
{
	int i, j;
#if 0
#ifdef USAGE_COUNT
	int u_min, i_min;
#endif /* USAGE_COUNT */
#endif /* 0 */

	if (*active_top == 0) {
		lt->regoff = -1;
		return;
	}

	i = *active_top - 1;
#ifdef USAGE_COUNT
#if 0
    /* find intervall which ends later or equal than than lt and has the */
	/* lowest usagecount lower than lt */
	i_min = -1;
	u_min = lt->usagecount;
	for (; (i >= 0) && (active[i]->i_end >= lt->i_end); i--) {
		if (active[i]->usagecount < u_min) {
			u_min = active[i]->usagecount;
			i_min = i;
		}
	}

	if (i_min != -1) {
		i = i_min;
#endif /* 0 */
	if ((active[i]->i_end >= lt->i_end) &&
		(active[i]->usagecount < lt->usagecount)) {
#else /* USAGE_COUNT */
	/* get last intervall from active */
	if (active[i]->i_end > lt->i_end) {
#endif /* USAGE_COUNT */

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
		/* Don't spill between one and two word int types */
		if ((active[i]->type == TYPE_LNG) != (lt->type == TYPE_LNG))
			return;
#endif

		lt->regoff = active[i]->regoff;
		active[i]->regoff = -1;

		(*active_top)--;
		for (j = i; j < *active_top; j++)
			active[j] = active[j + 1];

		lsra_add_active(lt, active, active_top);
	} else {
		lt->regoff = -1;
	}
}

void lsra_calc_lifetime_length(jitdata *jd)
{
	struct lifetime *lt;
	int i, lt_index;
	int lifetimecount;

	int *icount_block, icount;
	int flags; /* 0 INMEMORY -> ls->lt_mem */
	           /* 1 INTREG   -> ls->lt_int  */
	           /* 2 FLTREG   -> ls->lt_flt  */

	lsradata *ls;

	ls = jd->ls;

	icount_block = DMNEW(int, ls->basicblockcount);
	icount_block[0] = icount = 0 /* + ls->MAX_vars_with_indices + 1 */;
	for (i=1; i < ls->basicblockcount; i++) {
		if (ls->sorted[i-1] != -1)
			icount += ls->basicblocks[ls->sorted[i-1]]->icount + 1 +
				ls->varcount_with_indices;
		if (ls->sorted[i] != -1)
			icount_block[i] = icount;
	}

#ifdef LSRA_DEBUG_VERBOSE
	if (compileverbose) {
		printf("icount_block: ");
		for (i=0; i < ls->basicblockcount; i++)
			printf("(%3i-%3i) ",i, icount_block[i]);
		printf("\n");
	}
#endif

	lifetimecount = 0;
	for(lt_index = 0 ;lt_index < ls->lifetimecount; lt_index++) {
		if ( ls->lifetime[lt_index].type != -1) { /* used lifetime */
			/* remember lt_index in lt_sorted */
			ls->lt_used[lifetimecount ++] = lt_index;
			lt = &(ls->lifetime[lt_index]);

			/* compute lt->bb_first_def, i_first_def, bb_last_use and */
			/* i_last_use */

			_LSRA_ASSERT(lt->def != NULL);
			/*			_LSRA_ASSERT(lt->use != NULL);*/
			if (lt->use == NULL)
				lt->use = lt->def;

#if defined(SUPPORT_COMBINE_INTEGER_REGISTERS)
			/* prevent conflicts between lifetimes of type long by increasing */
			/* the lifetime by one instruction */
			/* i.e.(ri/rj)  ...       */
			/*     (rk/rl)  ICMD_LNEG */
			/* with i==l and/or j==k  */
			/* to resolve this during codegeneration a temporary register     */
			/* would be needed */
			if (lt->type == TYPE_LNG)
				lt->i_last_use++;
#endif

/* distribute lifetimes to lt_int, lt_flt and lt_mem */

			lt->regoff = -1;

			switch (lt->type) {
			case TYPE_LNG:
#if defined (__I386__)
				flags = 0;
#else
				flags = 1;
#endif
				break;

			case TYPE_INT:
			case TYPE_ADR:
				flags=1;
				break;
			case TYPE_DBL:
			case TYPE_FLT:
#if defined(__I386__)
				/*
				 * for i386 put all floats in memory
				 */
				flags=0;
				break;
#endif
				flags=2;
				break;
			default:
				{ log_text("Unknown Type\n"); exit(1); }
			}

			switch (flags) {
			case 0: /* lt_used[lt_used_index] -> lt_rest */
				ls->lt_mem[ ls->lt_mem_count++ ] = lt_index;
				break;
			case 1: /* l->lifetimes -> lt_int */
				ls->lt_int[ ls->lt_int_count++ ] = lt_index;
				break;
			case 2: /* l->lifetimes -> lt_flt */
				ls->lt_flt[ ls->lt_flt_count++ ] = lt_index;
				break;
			}


			if (lt->bb_first_def < -1) {
				printf("--------- Warning: variable not defined!------------------vi: %i start: %i end: %i\n", lt->v_index, lt->i_start, lt->i_end);
				lt->bb_first_def = 0;
				lt->i_first_def = 0;
			}

  			lt->i_start = icount_block[lt->bb_first_def] + lt->i_first_def;

			if (lt->bb_last_use == -1) {
				/* unused Vars are not regarded by lifeness_analysis! */
				_LSRA_ASSERT(lt->def != NULL)
				_LSRA_ASSERT(lt->def->next == NULL)

				if (compileverbose) {
					printf("--------- Warning: variable not used! ---------");
					printf("vi: %i start: %i end: %i\n", lt->v_index,
						   lt->i_start, lt->i_end);
				}
				lt->bb_last_use = lt->bb_first_def;
				lt->i_last_use = lt->i_first_def;
			}

			lt->i_end = icount_block[lt->bb_last_use] + lt->i_last_use;

			if (lt->i_start < 0)
				printf("------- Error: Lt %3i Vi %4i invalid!\n",lt_index, lt->v_index);

			if (lt->i_start > lt->i_end)
				printf("--------- Warning: last use before first def! vi: %i start: %i end: %i\n", lt->v_index, lt->i_start, lt->i_end);

#ifdef USAGE_PER_INSTR
			lt->usagecount = lt->usagecount / ( lt->i_end - lt->i_start + 1);
#endif
		}
	}
	ls->lifetimecount = lifetimecount;
}

#ifdef LSRA_DEBUG_VERBOSE
void print_lifetimes(jitdata *jd, int *lt, int lifetimecount)
{
	struct lifetime *n;
	int lt_index;
	lsradata *ls;
	registerdata *rd;

	rd = jd->rd;
	ls = jd->ls;

	for (lt_index = 0; lt_index < lifetimecount; lt_index++) {
		n = ls->lifetime + lt[lt_index];
		printf("i_Start: %3i(%3i,%3i) i_stop: %3i(%3i,%3i) reg: %3i VI: %3i type: %3i flags: %3i usage: %3li ltflags: %xi \n",n->i_start, ls->sorted[n->bb_first_def], n->i_first_def,n->i_end, ls->sorted[n->bb_last_use], n->i_last_use,n->regoff,n->v_index,n->type,n->flags, n->usagecount, n->flags);
	}
	printf( "%3i Lifetimes printed \n",lt_index);
}

void print_all_lifetimes(jitdata *jd)
{
	struct lifetime *n;
	int lt_index;
	lsradata *ls;
	registerdata *rd;

	rd = jd->rd;
	ls = jd->ls;

	for (lt_index = 0; lt_index < ls->lifetimecount; lt_index++) {
		n = &(ls->lifetime[lt_index]);
		if (n->type != -1) {
			printf("i_Start: %3i(%3i,%3i) i_stop: %3i(%3i,%3i) VI: %3i type: %3i flags: %3i usage: %3li ltflags: %xi \n",n->i_start, ls->sorted[n->bb_first_def], n->i_first_def,n->i_end, ls->sorted[n->bb_last_use], n->i_last_use,n->v_index,n->type,n->flags, n->usagecount, n->flags);
		}
	}
	printf( "%3i Lifetimes printed \n",lt_index);
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
 */
