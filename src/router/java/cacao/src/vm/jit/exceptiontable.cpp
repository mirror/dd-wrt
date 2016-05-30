/* src/vm/jit/exceptiontable.c - method exception table

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

#include "vm/jit/exceptiontable.hpp"
#include "config.h"
#include <assert.h>                     // for assert
#include <stdint.h>                     // for uint8_t
#include "mm/memory.hpp"                // for NEW
#include "toolbox/logging.hpp"          // for log_print, log_finish, etc
#include "vm/class.hpp"                 // for class_classref_print, etc
#include "vm/jit/code.hpp"              // for codeinfo
#include "vm/jit/jit.hpp"               // for exception_entry, jitdata, etc
#include "vm/method.hpp"                // for method_print

/* exceptiontable_create *******************************************************

   Builds the exception table for the currently compiled method.

   IN:
       jd ... JIT data of the currently compiled method

*******************************************************************************/

void exceptiontable_create(jitdata *jd)
{
	codeinfo               *code;
	exceptiontable_t       *et;
	exceptiontable_entry_t *ete;
	exception_entry        *ex;
	uint8_t                *pv;

	/* Get required compiler data. */

	code = jd->code;

	/* Don't allocate an exception table if we don't need one. */

	if (jd->exceptiontablelength == 0)
		return;

	/* Allocate the exception table and the entries array. */

	et  = NEW(exceptiontable_t);
	ete = MNEW(exceptiontable_entry_t, jd->exceptiontablelength);

	/* Fill the exception table. */

	et->length  = jd->exceptiontablelength;
	et->entries = ete;

	/* Fill the exception table entries. */

	pv = code->entrypoint;

	for (ex = jd->exceptiontable; ex != NULL; ex = ex->down, ete++) {
		/* Resolve basicblock relative start PCs to absolute
		   addresses. */

		ete->startpc       = pv + ex->start->mpc;
		ete->endpc         = pv + ex->end->mpc;
		ete->handlerpc     = pv + ex->handler->mpc;

		/* Store the catch type. */

		ete->catchtype.any = ex->catchtype.any;
	}

	/* Store the exception table in the codeinfo. */

	code->exceptiontable = et;

#if 0
	exceptiontable_print(code);
#endif
}


/* exceptiontable_free *********************************************************

   Frees the memory allocated by the exception table stored in the
   codeinfo.

   IN:
       code ... codeinfo of a method realization

*******************************************************************************/

void exceptiontable_free(codeinfo *code)
{
	exceptiontable_t       *et;
	exceptiontable_entry_t *ete;

	/* Sanity check. */

	assert(code != NULL);

	/* Free the exception table memory. */

	et = code->exceptiontable;

	if (et != NULL) {
		ete = et->entries;

		if (ete != NULL) {
			MFREE(ete, exceptiontable_entry_t, et->length);

			/* Clear the pointer. */

			et->entries = NULL;
		}

		FREE(et, exceptiontable_t);

		/* Clear the pointer. */

		code->exceptiontable = NULL;
	}
}


/* exceptiontable_print ********************************************************

   Print the exception table.

   IN:
       code ... codeinfo of a method realization

*******************************************************************************/

#if !defined(NDEBUG)
void exceptiontable_print(codeinfo *code)
{
	exceptiontable_t       *et;
	exceptiontable_entry_t *ete;
	int                     i;

	et = code->exceptiontable;

	/* Print the exception table. */

	log_start();
	log_print("[exceptiontable: m=%p, code=%p, exceptiontable=%p, length=%d, method=",
			  code->m, code, et, (et != NULL) ? et->length : 0);
	method_print(code->m);
	log_print("]");
	log_finish();

	if (et == NULL)
		return;

	/* Iterate over all entries. */

	for (i = 0, ete = et->entries; i < et->length; i++, ete++) {
		log_start();
		log_print("[exceptiontable entry %3d: startpc=%p, endpc=%p, handlerpc=%p, catchtype=%p (",
				  i, ete->startpc, ete->endpc, ete->handlerpc, ete->catchtype.any);

		if (ete->catchtype.any != NULL)
			if (ete->catchtype.is_classref())
				class_classref_print(ete->catchtype.ref);
			else
				class_print(ete->catchtype.cls);
		else
			log_print("ANY");

		log_print(")]");
		log_finish();
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
