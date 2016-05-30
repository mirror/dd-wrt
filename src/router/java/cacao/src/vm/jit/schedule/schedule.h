/* src/vm/jit/schedule/schedule.h - architecture independent instruction
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


#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "arch.hpp"
#include "types.hpp"
#include "vm/jit/reg.hpp"


typedef struct scheduledata scheduledata;
typedef struct minstruction minstruction;
typedef struct edgenode edgenode;
typedef struct opcycles opcycles;


/* machine instruction flags **************************************************/

#define SCHEDULE_LEADER         0x01
#define SCHEDULE_SINK           0x02

#define SCHEDULE_UNIT_ALU       0x04
#define SCHEDULE_UNIT_MEM       0x08
#define SCHEDULE_UNIT_BRANCH    0x10


#define M_SCHEDULE_SET_EXCEPTION_POINT    /*  if (jd->exceptiontablelength > 0) { schedule_do_schedule(sd); schedule_reset(sd, rd); } */


struct opcycles {
	s1 firstcycle;
	s1 lastcycle;
};


/* scheduledata ****************************************************************

   XXX

*******************************************************************************/

struct scheduledata {
	minstruction  *mi;                  /* machine instruction array          */
	s4             micount;             /* number of machine instructions     */
	edgenode      *leaders;             /* list containing leader nodes       */

	edgenode     **intregs_define_dep;
	edgenode     **fltregs_define_dep;
    edgenode     **memory_define_dep;

	edgenode     **intregs_use_dep;
	edgenode     **fltregs_use_dep;
	edgenode     **memory_use_dep;

	FILE *file;
};


/* minstruction ****************************************************************

   This structure contains all information for one machine instruction
   required to schedule it.

*******************************************************************************/

struct minstruction {
	u4             instr[2];            /* machine instruction word           */
	u1             flags;
#if 1
	s1             startcycle;          /* start pipeline cycle               */
	s1             endcycle;            /* end pipeline cycle                 */
#endif
	opcycles       op[4];
	s4             priority;            /* priority of this instruction node  */
	s4             starttime;
	edgenode      *deps;                /* operand dependencies               */
	minstruction  *next;                /* link to next machine instruction   */
};


/* edgenode ********************************************************************

   XXX

*******************************************************************************/

/* TODO rename to edgenode */
struct edgenode {
	s4        minum;                    /* machine instruction number         */
	s1        opnum;                    /* dependency operand number          */
	s1        opnum2;
	s1        latency;
	edgenode *next;                     /* link to next node                  */
};


/* function prototypes ********************************************************/

scheduledata *schedule_init(methodinfo *m, registerdata *rd);
void schedule_reset(scheduledata *sd, registerdata *rd);
void schedule_close(scheduledata *sd);

void schedule_calc_priority(minstruction *mi);

/*  void schedule_add_define_dep(scheduledata *sd, s1 operand, s4 *define_dep, edgenode **use_dep); */
/*  void schedule_add_use_dep(scheduledata *sd, s1 operand, s4 *define_dep, edgenode **use_dep); */
void schedule_add_define_dep(scheduledata *sd, s1 opnum, edgenode **define_dep, edgenode **use_dep);
void schedule_add_use_dep(scheduledata *sd, s1 opnum, edgenode **define_dep, edgenode **use_dep);

void schedule_do_schedule(scheduledata *sd);

#endif /* _SCHEDULE_H */


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
