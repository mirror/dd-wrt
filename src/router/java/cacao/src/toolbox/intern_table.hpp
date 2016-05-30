/* src/toolbox/intern_table.hpp - Intern table header

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

#ifndef INTERN_TABLE_HPP_
#define INTERN_TABLE_HPP_ 1

#include "config.h"

#include "threads/mutex.hpp"

#include "toolbox/assert.hpp"
#include "toolbox/hashtable.hpp"
#include "toolbox/util.hpp"

/***
 *
 *	A specialized hash map that allows for parallel access by multiple threads.
 *	This is achieved by splitting up the table into multiple segments. Each
 *	segment is a separate hash table protected by its own lock.
 *	We need no global lock for accessing segments because there is a
 *	fixed number of segments.
 *
 *	InternTable has the same requirements on elements you want to insert into
 *	the table as HashTable.
 *
 *	InternTable is meant to be used as a global, so its constructor and desctructor
 *	do no real work. You have to call initialize() and destroy() manually.
 *
 *	@tparam _Entry              The type of element stored in the table.
 *		                        Must fulfill the same requirements as a
 *		                        HashTable entry.
 *	@tparam concurrency_factor  The maximum number of threads that can access
 *		                        the table concurrently. Must be a power of two.
 *
 *	@note
 *		If we don't wan't any concurrency we can use an intern table with a
 *		concurrency factor of 1. This means the table has exactly one global
 *		lock without any overhead.
 */
template<class _Entry, size_t concurrency_factor=4>
struct InternTable {
	typedef _Entry Entry;

	static const size_t DEFAULT_INITIAL_CAPACITY = 256;
	static const size_t DEFAULT_LOAD_FACTOR      = 85;

	InternTable() : segments(0) {}

	void initialize(size_t initial_capacity = DEFAULT_INITIAL_CAPACITY, size_t load_factor = DEFAULT_LOAD_FACTOR) {
		assert(!is_initialized());
		assert(load_factor      > 0);
		assert(load_factor      < 100);
		assert(initial_capacity > 0);

		segments = new Segment[concurrency_factor];

		// evenly divide capacity among segments
		size_t cap = divide_rounding_up(initial_capacity, concurrency_factor);

		for (size_t i = 0; i < concurrency_factor; ++i) {
			segments[i].initialize(cap, load_factor);
		}
	}

	void destroy() {
		delete [] segments;
		segments = 0;
	}

	bool is_initialized() const { return segments != 0; }

	template<typename Thunk>
	const Entry& intern(const Thunk& t) {
		EXPENSIVE_ASSERT(is_initialized());

		size_t hash = t.hash();

		return segments[hash % concurrency_factor].intern(t);
	}
private:
	InternTable(const InternTable&);            // non-copyable
	InternTable& operator=(const InternTable&); // non-assignable

	struct Segment {
		void initialize(size_t initial_capacity, size_t load_factor) {
			ht.set_load_factor(load_factor);
			ht.reserve(initial_capacity);
		}

		void destroy() {
			ht.clear();
		}

		template<typename Thunk>
		const Entry& intern(const Thunk& thunk) {
			MutexLocker lock(mutex);

			return ht.insert(thunk);
		}

		HashTable<Entry>  ht;
		Mutex             mutex;  // for locking this segment
	};

	Segment *segments; // the sub-hashtables
};

#endif // INTERN_TABLE_HPP_

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
