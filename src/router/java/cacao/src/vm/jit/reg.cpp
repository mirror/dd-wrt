/* src/vm/jit/reg.cpp - register allocator setup

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

#include "vm/jit/reg.hpp"
#include <cassert>                      // for assert
#include "config.h"
#include "arch.hpp"
#include "md-abi.hpp"
#include "mm/dumpmemory.hpp"            // for DNEW, DMNEW
#include "vm/jit/abi.hpp"               // for nregdescfloat, nregdescint
#include "vm/jit/jit.hpp"               // for jitdata
#include "vm/method.hpp"                // for methodinfo
#include "vm/types.hpp"                 // for s4

/* reg_setup *******************************************************************

   TODO

*******************************************************************************/

void reg_setup(jitdata *jd)
{
	methodinfo   *m;
	registerdata *rd;
	s4            i;

	/* get required compiler data */

	m  = jd->m;
	rd = jd->rd;

	/* setup the integer register table */

	rd->tmpintregs = DMNEW(s4, INT_TMP_CNT);
	rd->savintregs = DMNEW(s4, INT_SAV_CNT);
	rd->freeargintregs = DMNEW(s4, INT_ARG_CNT);
	rd->freetmpintregs = DMNEW(s4, INT_TMP_CNT);
	rd->freesavintregs = DMNEW(s4, INT_SAV_CNT);

	rd->argintreguse = 0;
	rd->tmpintreguse = 0;
	rd->savintreguse = 0;

	for (i = 0; i < INT_REG_CNT; i++) {
		switch (nregdescint[i]) {
		case REG_RET:
			rd->intreg_ret = i;
			break;
		case REG_SAV:
			rd->savintregs[rd->savintreguse++] = i;
			break;
		case REG_TMP:
  			rd->tmpintregs[rd->tmpintreguse++] = i;
			break;
		}
	}
	assert(rd->savintreguse == INT_SAV_CNT);
	assert(rd->tmpintreguse == INT_TMP_CNT);

	/* setup the float register table */

	rd->tmpfltregs = DMNEW(s4, FLT_TMP_CNT);
	rd->savfltregs = DMNEW(s4, FLT_SAV_CNT);
	rd->freeargfltregs = DMNEW(s4, FLT_ARG_CNT);
	rd->freetmpfltregs = DMNEW(s4, FLT_TMP_CNT);
	rd->freesavfltregs = DMNEW(s4, FLT_SAV_CNT);

	rd->argfltreguse = 0;
	rd->tmpfltreguse = 0;
	rd->savfltreguse = 0;

	for (i = 0; i < FLT_REG_CNT; i++) {
		switch (nregdescfloat[i]) {
		case REG_RET:
			rd->fltreg_ret = i;
			break;
		case REG_SAV:
			rd->savfltregs[rd->savfltreguse++] = i;
			break;
		case REG_TMP:
			rd->tmpfltregs[rd->tmpfltreguse++] = i;
			break;
		}
	}
	assert(rd->savfltreguse == FLT_SAV_CNT);
	assert(rd->tmpfltreguse == FLT_TMP_CNT);

	rd->freemem    = DMNEW(s4, m->maxstack);

#if defined(SPECIALMEMUSE)
# if defined(__DARWIN__)
	/* 6*4=24 byte linkage area + 8*4=32 byte minimum parameter Area */
	rd->memuse = LA_SIZE_IN_POINTERS + INT_ARG_CNT;
# else
	rd->memuse = LA_SIZE_IN_POINTERS;
# endif
#else
	rd->memuse = 0; /* init to zero -> analyse_stack will set it to a higher  */
	                /* value, if appropriate */
#endif

	/* Set rd->arg*reguse to *_ARG_CNBT to not use unused argument            */
	/* registers as temp registers  */
	rd->argintreguse = 0;
	rd->argfltreguse = 0;
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
