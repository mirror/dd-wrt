/* src/vm/jit/schedule/schedule.c - architecture independent instruction
                                    scheduler

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

   Contact: cacao@cacaojvm.org

   Authors: Christian Thalinger

   Changes:

*/


#include "config.h"

#include <stdio.h>

#include "vm/types.hpp"

#include "disass.hpp"

#include "mm/memory.hpp"
#include "vm/options.hpp"
#include "vm/statistics.hpp"
#include "vm/jit/schedule/schedule.h"

#error port to C++ first!
STAT_REGISTER_GROUP(sched_stat,"inst. sched.","Instruction scheduler statistics:")
STAT_REGISTER_GROUP_VAR(s4,count_schedule_basic_blocks,0,"num basicblocs","Number of basic blocks")
STAT_REGISTER_GROUP_VAR(s4,count_schedule_nodes,0,"num nodes","Number of nodes")
STAT_REGISTER_GROUP_VAR(s4,count_schedule_leaders,0,"num leader nodes","Number of leaders nodes")
STAT_REGISTER_GROUP_VAR(s4,count_schedule_critical_path,0,"critical path","Length of critical path")
STAT_REGISTER_GROUP_VAR(s4,count_schedule_max_leaders,"max leader nodes","Number of max. leaders nodes")

/* XXX quick hack */
s4 stackrange;

scheduledata *schedule_init(methodinfo *m, registerdata *rd)
{
	scheduledata *sd;

	sd = DNEW(scheduledata);

	stackrange = 
		(rd->savintregcnt - rd->maxsavintreguse) +
		(rd->savfltregcnt - rd->maxsavfltreguse) +
		rd->maxmemuse +
		m->parseddesc->paramcount +
		1;                           /* index 0 are all other memory accesses */

	/* XXX quick hack: just use a fixed number of instructions */
	sd->mi = DMNEW(minstruction, 20000);

	sd->intregs_define_dep = DMNEW(edgenode*, rd->intregsnum);
	sd->fltregs_define_dep = DMNEW(edgenode*, rd->fltregsnum);

	sd->intregs_use_dep = DMNEW(edgenode*, rd->intregsnum);
	sd->fltregs_use_dep = DMNEW(edgenode*, rd->fltregsnum);

  	sd->memory_define_dep = DMNEW(edgenode*, stackrange);
  	sd->memory_use_dep = DMNEW(edgenode*, stackrange);


	/* open graphviz file */

	if (opt_verbose) {
		FILE *file;

		file = fopen("cacao.dot", "w+");
		fprintf(file, "digraph G {\n");

		sd->file = file;
	}

	return sd;
}


void schedule_reset(scheduledata *sd, registerdata *rd)
{
	sd->micount = 0;
	sd->leaders = NULL;

	/* set define array to -1, because 0 is a valid node number */

	MSET(sd->intregs_define_dep, 0, edgenode*, rd->intregsnum);
	MSET(sd->fltregs_define_dep, 0, edgenode*, rd->fltregsnum);

	/* clear all use pointers */

	MSET(sd->intregs_use_dep, 0, edgenode*, rd->intregsnum);
	MSET(sd->fltregs_use_dep, 0, edgenode*, rd->fltregsnum);

  	MSET(sd->memory_define_dep, 0, edgenode*, stackrange);
  	MSET(sd->memory_use_dep, 0, edgenode*, stackrange);
}


void schedule_close(scheduledata *sd)
{
	if (opt_verbose) {
		FILE *file;

		file = sd->file;

		fprintf(file, "}\n");
		fclose(file);
	}
}



/* schedule_add_define_dep *****************************************************

   XXX

*******************************************************************************/

void schedule_add_define_dep(scheduledata *sd, s1 opnum, edgenode **define_dep, edgenode **use_dep)
{
	minstruction *mi;
	minstruction *defmi;
	minstruction *usemi;
	edgenode     *en;
	edgenode     *useen;
	edgenode     *defen;
	edgenode     *tmpen;
	s4            minum;

	/* get current machine instruction */

	minum = sd->micount - 1;
	mi = &sd->mi[minum];

	/* get current use dependency nodes, if non-null use them... */

	if ((useen = *use_dep)) {
		while (useen) {
			/* Save next pointer so we can link the current node to the       */
			/* machine instructions' dependency list.                         */

			tmpen = useen->next;

			/* don't add the current machine instruction (e.g. add A0,A0,A0) */

			if (useen->minum != minum) {
				/* some edges to current machine instruction -> no leader */

				mi->flags &= ~SCHEDULE_LEADER;

				/* get use machine instruction */

				usemi = &sd->mi[useen->minum];

				/* link current use node into dependency list */

				useen->minum = minum;
				useen->opnum2 = opnum;

				/* calculate latency, for define add 1 cycle */
				useen->latency = (usemi->op[useen->opnum].lastcycle -
								  mi->op[opnum].firstcycle) + 1;

				useen->next = usemi->deps;
				usemi->deps = useen;
			}

			useen = tmpen;
		}

		/* ...otherwise use last define dependency, if non-null */
	} else if ((defen = *define_dep)) {
		/* some edges to current machine instruction -> no leader */

		mi->flags &= ~SCHEDULE_LEADER;

		/* get last define machine instruction */

		defmi = &sd->mi[defen->minum];

		/* link current define node into dependency list */

		defen->minum = minum;
		defen->opnum2 = opnum;

		/* calculate latency, for define add 1 cycle */
		defen->latency = (defmi->op[defen->opnum].lastcycle -
						  mi->op[opnum].firstcycle) + 1;

		defen->next = defmi->deps;
		defmi->deps = defen;
	}

	/* Set current instruction as new define dependency and clear use         */
	/* dependencies.                                                          */

	en = DNEW(edgenode);
	en->minum = minum;
	en->opnum = opnum;
	
	*define_dep = en;
	*use_dep = NULL;
}


/* schedule_add_use_dep ********************************************************

   XXX

*******************************************************************************/

void schedule_add_use_dep(scheduledata *sd, s1 opnum, edgenode **define_dep, edgenode **use_dep)
{
	minstruction *mi;
	minstruction *defmi;
	edgenode     *en;
	edgenode     *defen;
	s4            minum;

	/* get current machine instruction */

	minum = sd->micount - 1;
	mi = &sd->mi[minum];

	/* get current define dependency instruction */

	if ((defen = *define_dep)) {
		/* some edges to current machine instruction -> no leader */

		mi->flags &= ~SCHEDULE_LEADER;

		/* add node to dependency list of current define node */

		defmi = &sd->mi[defen->minum];

		en = DNEW(edgenode);
		en->minum = minum;
		en->opnum = defen->opnum;
		en->opnum2 = opnum;

		/* calculate latency */
		en->latency = (defmi->op[defen->opnum].lastcycle -
					   mi->op[opnum].firstcycle);

		en->next = defmi->deps;
		defmi->deps = en;
	}

	/* add node to list of current use nodes */

	en = DNEW(edgenode);
	en->minum = minum;
	en->opnum = opnum;
	en->next = *use_dep;
	*use_dep = en;
}


/* schedule_calc_priorities ****************************************************

   Calculates the current node's priority by getting highest priority
   of the dependency nodes, adding this nodes latency plus 1 (for the
   instruction itself).

*******************************************************************************/

void schedule_calc_priorities(scheduledata *sd)
{
	minstruction *mi;
	minstruction *lastmi;
	edgenode     *en;
	s4            lastnode;
	s4            i;
	s4            j;
	s4            criticalpath;
	s4            currentpath;
	s1            lastcycle;


	/* last node MUST always be the last instruction scheduled */

	lastnode = sd->micount - 1;

	/* if last instruction is the first instruction, skip everything */

	if (lastnode > 0) {
		lastmi = &sd->mi[lastnode];

		/* last instruction is no leader */

		lastmi->flags &= ~SCHEDULE_LEADER;

		/* last instruction has no dependencies, use virtual sink node */

		lastcycle = 0;

		for (j = 0; j < 4; j++) {
			if (lastmi->op[j].lastcycle > lastcycle)
				lastcycle = lastmi->op[j].lastcycle;
		}

#define EARLIEST_USE_CYCLE 3
		lastmi->priority = (lastcycle - EARLIEST_USE_CYCLE) + 1;


		/* walk through all remaining machine instructions backwards */

		for (i = lastnode - 1, mi = &sd->mi[lastnode - 1]; i >= 0; i--, mi--) {
			en = mi->deps;

			if (en) {
				s4 priority;

				criticalpath = 0;

				/* walk through depedencies and calculate highest latency */

				while (en) {
					priority = sd->mi[en->minum].priority;

					currentpath = en->latency + priority;

					if (currentpath > criticalpath)
						criticalpath = currentpath;

					en = en->next;
				}

				/* set priority to critical path */

				mi->priority = criticalpath;

			} else {
				/* no dependencies, use virtual sink node */

				lastcycle = 0;

				for (j = 0; j < 4; j++) {
					if (mi->op[j].lastcycle > lastcycle)
						lastcycle = mi->op[j].lastcycle;
				}

				mi->priority = (lastcycle - EARLIEST_USE_CYCLE);
			}

			/* add current machine instruction to leader list, if one */

			if (mi->flags & SCHEDULE_LEADER) {
				en = DNEW(edgenode);
				en->minum = i;
				en->next = sd->leaders;
				sd->leaders = en;
			}
		}

	} else {
		/* last node is first instruction, add to leader list */

		en = DNEW(edgenode);
		en->minum = lastnode;
		en->next = sd->leaders;
		sd->leaders = en;
	}
}


static void schedule_create_graph(scheduledata *sd, s4 criticalpath)
{
	minstruction *mi;
	edgenode     *en;
	s4            i;

	FILE *file;
	static int bb = 1;

	file = sd->file;

	fprintf(file, "subgraph cluster_%d {\n", bb);
	fprintf(file, "label = \"BB%d (nodes: %d, critical path: %d)\"\n", bb, sd->micount, criticalpath);

	for (i = 0, mi = sd->mi; i < sd->micount; i++, mi++) {

		en = mi->deps;

		if (en) {
			while (en) {
				fprintf(file, "\"#%d.%d ", bb, i);
				disassinstr(file, &mi->instr);
				fprintf(file, "\\np=%d\"", mi->priority);

				fprintf(file, " -> ");

				fprintf(file, "\"#%d.%d ", bb, en->minum);
				disassinstr(file, &sd->mi[en->minum].instr);
				fprintf(file, "\\np=%d\"", sd->mi[en->minum].priority);

				fprintf(file, " [ label = \"%d\" ]\n", en->latency);
				en = en->next;
			}

		} else {
			fprintf(file, "\"#%d.%d ", bb, i);
			disassinstr(file, &mi->instr);
			fprintf(file, "\\np=%d\"", mi->priority);

			fprintf(file, " -> ");
			
			fprintf(file, "\"end %d\"", bb);

			fprintf(file, " [ label = \"%d\" ]\n", mi->priority);
	
			fprintf(file, "\n");
		}
	}

	fprintf(file, "}\n");

	bb++;
}


/* schedule_add_deps_to_leaders ************************************************

   Walk through all dependencies, change the starttime and add the
   node to the leaders list.

*******************************************************************************/

static void schedule_add_deps_to_leaders(scheduledata *sd, edgenode *deps, s4 time)
{
	edgenode *depen;
	edgenode *preven;

	if ((depen = deps)) {
		while (depen) {
			/* set starttime of instruction */

			sd->mi[depen->minum].starttime = time + depen->latency;

			/* save last entry */

			preven = depen;

			depen = depen->next;
		}

		/* add dependencies to front of the list */

		preven->next = sd->leaders;
		sd->leaders = deps;
	}
}


/* schedule_do_schedule ********************************************************

   XXX

*******************************************************************************/

void schedule_do_schedule(scheduledata *sd)
{
	minstruction *mi;
	edgenode     *en;
	s4            i;
	s4            j;
	s4            leaders;
	s4            criticalpath;
	s4            time;
	s4            schedulecount;

	minstruction *alumi;
	minstruction *memmi;
	minstruction *brmi;

	edgenode     *preven;
	edgenode     *depen;

	edgenode     *aluen;
	edgenode     *memen;
	edgenode     *bren;

	/* It may be possible that no instructions are in the current basic block */
	/* because after an branch instruction the instructions are scheduled.    */

	if (sd->micount > 0) {
		/* calculate priorities and critical path */

		schedule_calc_priorities(sd);

		if (opt_verbose) {
			printf("bb start ---\n");
			printf("nodes: %d\n", sd->micount);
			printf("leaders: ");
		}

		leaders = 0;
		criticalpath = 0;

		en = sd->leaders;
		while (en) {
			if (opt_verbose) {
				printf("#%d ", en->minum);
			}

			leaders++;
			if (sd->mi[en->minum].priority > criticalpath)
				criticalpath = sd->mi[en->minum].priority;
			en = en->next;
		}

		/* check last node for critical path (e.g. ret) */

		if (sd->mi[sd->micount - 1].priority > criticalpath)
			criticalpath = sd->mi[sd->micount - 1].priority;
		
		if (opt_verbose) {
			printf("\n");
			printf("critical path: %d\n", criticalpath);

			for (i = 0, mi = sd->mi; i < sd->micount; i++, mi++) {
				disassinstr(stdout, &mi->instr);

				printf("\t--> #%d, prio=%d", i, mi->priority);

				printf(", mem=%d:%d", mi->op[0].firstcycle, mi->op[0].lastcycle);

				for (j = 1; j <= 3; j++) {
					printf(", op%d=%d:%d", j, mi->op[j].firstcycle, mi->op[j].lastcycle);
				}

				printf(", deps= ");
				en = mi->deps;
				while (en) {
					printf("#%d (op%d->op%d: %d) ", en->minum, en->opnum, en->opnum2, en->latency);
					en = en->next;
				}
				printf("\n");
			}
			printf("bb end ---\n\n");

			schedule_create_graph(sd, criticalpath);
		}


		/* set start time to zero */

		printf("\n\nschedule start ---\n");
		time = 0;
		schedulecount = 0;

		while (sd->leaders) {
			/* XXX argh! how to make this portable? */
			for (j = 0; j < 2; j++ ) {
				en = sd->leaders;
				preven = NULL;

				alumi = NULL;
				memmi = NULL;
				brmi = NULL;

				aluen = NULL;
				memen = NULL;
				bren = NULL;

				printf("\n\nleaders: ");
				while (en) {
					/* get current machine instruction from leader list */

					mi = &sd->mi[en->minum];
					disassinstr(stdout, &mi->instr);
					printf(", ");

					/* starttime reached */

					if (mi->starttime <= time) {

						/* check for a suitable ALU instruction */

						if ((mi->flags & SCHEDULE_UNIT_ALU) &&
							(!alumi || (mi->priority > alumi->priority))) {
							/* remove/replace current node from leaders list */

							if (preven)
								if (alumi) {
									preven->next = aluen;
									aluen->next = en->next;
								} else {
									preven->next = en->next;
								}
							else
								if (alumi) {
									sd->leaders = aluen;
									aluen->next = en->next;
								} else {
									sd->leaders = en->next;
								}

							/* set current ALU instruction and node */

							alumi = mi;
							aluen = en;

						/* check for a suitable MEM instruction */

						} else if ((mi->flags & SCHEDULE_UNIT_MEM) &&
								   (!memmi || (mi->priority > memmi->priority))) {
							if (preven)
								if (memmi) {
									preven->next = memen;
									memen->next = en->next;
								} else {
									preven->next = en->next;
								}
							else
								if (memmi) {
									sd->leaders = memen;
									memen->next = en->next;
								} else {
									sd->leaders = en->next;
								}

							memmi = mi;
							memen = en;

						/* check for a suitable BRANCH instruction */

						} else if ((mi->flags & SCHEDULE_UNIT_BRANCH) &&
								   (!brmi || (mi->priority > brmi->priority))) {
							if (preven)
								preven->next = en->next;
							else
								sd->leaders = en->next;

							memmi = mi;
							memen = en;

						} else
							preven = en;

					} else {
						/* not leader removed, save next previous node */

						preven = en;
					}

					en = en->next;
				}
				printf("\n");

				/* schedule ALU instruction, if one was found */

				if (aluen) {
					mi = &sd->mi[aluen->minum];

					disassinstr(stdout, &mi->instr);
					printf(" || ");

					schedulecount++;
					schedule_add_deps_to_leaders(sd, mi->deps, time);
				}

				/* schedule MEM instruction, if one was found */

				if (memen) {
					mi = &sd->mi[memen->minum];

					disassinstr(stdout, &mi->instr);
					printf(" || ");

					schedulecount++;
					schedule_add_deps_to_leaders(sd, mi->deps, time);
				}

				/* schedule BRANCH instruction, if one was found */

				if (bren) {
					mi = &sd->mi[bren->minum];

					disassinstr(stdout, &brmi->instr);
					printf(" || ");

					schedulecount++;
					schedule_add_deps_to_leaders(sd, mi->deps, time);
				}

				if (!aluen && !memen && !bren) {
					printf("nop");
					printf(" || ");
				}
			}
			printf("\n");

			/* bundle finished, increase execution time */

			time++;
		}
		printf("schedule end ---\n\n");

		STATISTICS(count_schedule_basic_blocks++);
		STATISTICS(count_schedule_nodes += sd->micount);
		STATISTICS(count_schedule_leaders += leaders);
		STATISTICS(count_schedule_max_leaders.max(leaders));
		STATISTICS(count_schedule_critical_path += criticalpath);
	}
}


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
 * vim:noexpandtab:sw=4:ts=4:
 */
