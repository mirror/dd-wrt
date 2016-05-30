/* src/toolbox/set.c - Set implementation.

   Copyright (C) 2008
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

   This file implements a set of pointers.

   The current implementation is naive and should be improved in the future,
   so that the O(size) operations take O(log(size)) instead.

   The representation of the set is an contingous unordered array of the
   elements (pointers).
*/

#include "toolbox/set.hpp"
#include <assert.h>                     // for assert
#include <stddef.h>                     // for NULL
#include "mm/dumpmemory.hpp"            // for DumpMemory
#include "mm/memory.hpp"                // for MZERO

/* struct set ******************************************************************

   Represents the set.

*******************************************************************************/

struct set {
	void **elements;   /* An array of elements */
	unsigned capacity; /* Size of elements */
	unsigned size;     /* Current number of elements */
};

/* set_new ********************************************************************

   Creates an instance of a set on the dump area.

   IN:
       capacity: Maximal number of elements of elements the set can hold.

*******************************************************************************/

set *set_new(unsigned capacity) {
	set *s = (set*) DumpMemory::allocate(sizeof(set));

	s->elements = (void**) DumpMemory::allocate(sizeof(void*) * capacity);
	MZERO(s->elements, void *, capacity);
	s->capacity = capacity;
	s->size = 0;

	return s;
}

/* set_insert ******************************************************************

   Inserts element e into set s

   The current implementation takes O(size).

*******************************************************************************/

void set_insert(set *s, void *element) {
	unsigned i;

	for (i = 0; i < s->size; ++i) {
		if (s->elements[i] == element) {
			return;
		}
	}

	assert(i < s->capacity);

	s->size += 1;
	s->elements[i] = element;
}

/* set_remove ******************************************************************

   Removes element e into set s

   The current implementation takes O(size).

*******************************************************************************/

void set_remove(set *s, void *element) {
	unsigned i;
	for (i = 0; i < s->size; ++i) {
		if (s->elements[i] == element) {
			/* Do not creaet a "hole".
			 * Overwrite this element with the last element.
			 */
			if (i == (s->size - 1)) { /* The last one */
				s->elements[i] = NULL;
			} else {
				s->elements[i] = s->elements[s->size - 1];
				s->elements[s->size - 1] = NULL;
			}
			s->size -= 1;
		}
	}
}

/* set_size ********************************************************************

   Returns the number of elements in the set s.
   The complexity of the operation is O(1).

*******************************************************************************/

unsigned set_size(const set *s) {
	return s->size;
}

/* set_empty *******************************************************************

   Returns true, iif the set s is empty.
   The complexity of the operation is O(1).

*******************************************************************************/

bool set_empty(const set *s) {
	return s->size == 0;
}

/* set_contains ****************************************************************

   Returns true, iif the set s contains element element.

   The current implementation takes O(size).

*******************************************************************************/

bool set_contains(const set *s, void *element) {
	unsigned i;
	for (i = 0; i < s->size; ++i) {
		if (s->elements[i] == element) {
			return true;
		}
	}
	return false;
}

/* set_pop *********************************************************************

   Pics and removes some element from the set s and returns it.
   Returns NULL if the set s is empty.
   The complexity of the operation is O(1).

*******************************************************************************/

void *set_pop(set *s) {
	void *ret = NULL;

	if (s->size > 0) {
		ret = s->elements[s->size - 1];
		s->elements[s->size - 1] = NULL;
		s->size -= 1;
	}

	return ret;
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
