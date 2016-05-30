/* mm/cacao-gc/final.c - GC module for finalization and weak references

   Copyright (C) 2006, 2008
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


#include "config.h"
#include "vm/types.hpp"

#include "gc.h"
#include "final.h"
#include "heap.h"
#include "mm/memory.hpp"
#include "vm/finalizer.hpp"


/* Global Variables ***********************************************************/

list_t *final_list;



void final_init()
{
	final_list = list_create(OFFSET(list_final_entry_t, linkage));
}

void final_register(java_object_t *o, methodinfo *finalizer)
{
	list_final_entry_t *fe;

	fe = NEW(list_final_entry_t);
	fe->type      = FINAL_REACHABLE;
	fe->o         = o;
	fe->finalizer = finalizer;

	list_add_first(final_list, fe);

	GC_LOG2( printf("Finalizer registered for: %p\n", (void *) o); );
}

void final_invoke()
{
	list_final_entry_t *fe;
	list_final_entry_t *fe_next;

	fe = list_first(final_list);
	fe_next = NULL;
	while (fe) {
		fe_next = list_next(final_list, fe);

		if (fe->type == FINAL_RECLAIMABLE) {

			GC_LOG( printf("Finalizer starting for: ");
					heap_print_object(fe->o); printf("\n"); );

			GC_ASSERT(fe->finalizer == fe->o->vftbl->class->finalizer);

			fe->type = FINAL_FINALIZING;

			finalizer_run(fe->o, NULL);

			fe->type = FINAL_FINALIZED;

			list_remove(final_list, fe);
			FREE(fe, list_final_entry_t);
		}

		fe = fe_next;
	}
}

void final_set_all_reclaimable()
{
	list_final_entry_t *fe;

	fe = list_first(final_list);
	while (fe) {

		if (fe->type == FINAL_REACHABLE)
			fe->type = FINAL_RECLAIMABLE;

		fe = list_next(final_list, fe);
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
