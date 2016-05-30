/* src/mm/cacao-gc/rootset.h - GC header for root set management

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


#ifndef _ROOTSET_H
#define _ROOTSET_H

typedef struct rootset_t rootset_t;

#include "config.h"
#include "vm/types.hpp"

#include "threads/thread.hpp"

#include "vm/method.hpp"

#include "vm/jit/replace.hpp"


/* Structures *****************************************************************/

#define ROOTSET_DUMMY_THREAD ((threadobject *) (ptrint) -1)

#define ROOTSET_INITIAL_CAPACITY 16

/* rootset is passed as array of pointers, which point to the location of
   the reference */

typedef struct rootset_entry_t {
	java_object_t     **ref;            /* a pointer to the actual reference */
	bool                marks;          /* indicates if a reference marks */
#if !defined(NDEBUG)
	s4                  reftype;
#endif
} rootset_entry_t;


struct rootset_t {
	rootset_t          *next;           /* link to the next chain element */
	threadobject       *thread;         /* thread this rootset belongs to */
	s4                  capacity;       /* the current capacity of this rs */
	s4                  refcount;       /* number of references */
	rootset_entry_t     refs[ROOTSET_INITIAL_CAPACITY]; /* list of references */
};


/* Prototypes *****************************************************************/

/*
rootset_t *rootset_create(void);
void rootset_from_globals(rootset_t *rs);
void rootset_from_thread(threadobject *thread, rootset_t *rs);
*/
rootset_t *rootset_readout();
void rootset_writeback(rootset_t *rs);

#if !defined(NDEBUG)
void rootset_print(rootset_t *rs);
#endif


#endif /* _ROOTSET_H */

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
