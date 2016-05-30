/* src/threads/atomic.hpp - atomic instructions

   Copyright (C) 2008
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


#ifndef _ATOMIC_HPP
#define _ATOMIC_HPP

#include "config.h"

#include <stdint.h>

namespace Atomic_md {
	// Machine dependent functions.
	uint32_t compare_and_swap(volatile uint32_t *p, uint32_t oldval, uint32_t newval);
	uint64_t compare_and_swap(volatile uint64_t *p, uint64_t oldval, uint64_t newval);

	void     memory_barrier(void);
	void     write_memory_barrier(void);
	void     instruction_barrier(void);
}

namespace Atomic {

	// Generic functions.
	uint32_t generic_compare_and_swap(volatile uint32_t *p, uint32_t oldval, uint32_t newval);
	uint64_t generic_compare_and_swap(volatile uint64_t *p, uint64_t oldval, uint64_t newval);
	void*    generic_compare_and_swap(volatile void** p, void* oldval, void* newval);
	void     generic_memory_barrier(void);

}

// Include machine dependent implementation.
#include "md-atomic.hpp"

namespace Atomic {

	struct CAS_32_functor {
		typedef uint32_t value_type;
		static value_type compare_and_swap(value_type *p, value_type o, value_type n) {
			return Atomic_md::compare_and_swap(p, o, n);
		}
	};

	struct CAS_64_functor {
		typedef uint64_t value_type;
		static value_type compare_and_swap(value_type *p, value_type o, value_type n) {
			return Atomic_md::compare_and_swap(p, o, n);
		}
	};

	template<int N> class CAS_chooser;
	template<> class CAS_chooser<4> {
		public:
			typedef CAS_32_functor the_type;
	};
	template<> class CAS_chooser<8> {
		public:
			typedef CAS_64_functor the_type;
	};

	template<class T> class CAS {
		public:
			typedef typename CAS_chooser<sizeof(T)>::the_type S;
			static T compare_and_swap(T *p, T o, T n) {
				return (T) S::compare_and_swap((typename S::value_type*) p,
						(typename S::value_type) o,
						(typename S::value_type) n);
			}
	};

	template<class T> T compare_and_swap(T *p, T o, T n) {
		return CAS<T>::compare_and_swap(p, o, n);
	}

	inline void     memory_barrier(void)       { Atomic_md::memory_barrier(); }
	inline void     write_memory_barrier(void) { Atomic_md::write_memory_barrier(); }
	inline void     instruction_barrier(void)  { Atomic_md::instruction_barrier(); }
}

#endif // _ATOMIC_HPP


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
