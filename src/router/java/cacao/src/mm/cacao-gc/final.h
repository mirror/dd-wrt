/* mm/cacao-gc/final.h - GC header for finalization and weak references

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


#ifndef _FINAL_H
#define _FINAL_H

#include "config.h"
#include "vm/types.hpp"

#include "toolbox/list.hpp"
#include "vm/method.hpp"


/* Global Variables ***********************************************************/

extern list_t *final_list;


/* Structures *****************************************************************/

typedef struct list_final_entry_t list_final_entry_t;

#define FINAL_REACHABLE   1
#define FINAL_RECLAIMABLE 2
#define FINAL_FINALIZING  3
#define FINAL_FINALIZED   4

struct list_final_entry_t {
	listnode_t         linkage;
	u4                 type;
	java_object_t     *o;
	methodinfo        *finalizer;
};


/* Prototypes *****************************************************************/

void final_init();
void final_register(java_object_t *o, methodinfo *finalizer);
void final_invoke();
void final_set_all_reclaimable();


#endif /* _FINAL_H */

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
