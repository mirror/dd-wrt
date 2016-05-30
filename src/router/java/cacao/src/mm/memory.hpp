/* src/mm/memory.hpp - macros for memory management

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


#ifndef MEMORY_HPP_
#define MEMORY_HPP_ 1

#include "config.h"

#include <stdint.h>
#include <cstddef>                     // for size_t
#include <cstring>                     // for memset

// Align the size of memory allocations to this size.
#define ALIGNSIZE 8
#define MEMORY_ALIGN(pos,size) ((((pos) + (size) - 1) / (size)) * (size))


// Constants for ENABLE_MEMCHECK.

#if defined(ENABLE_MEMCHECK)
#define MEMORY_CANARY_SIZE          16
#define MEMORY_CANARY_FIRST_BYTE    0xca
#define MEMORY_CLEAR_BYTE           0xa5
#endif


// Includes.
//#include "mm/dumpmemory.hpp"

/* 
---------------------------- Interface description -----------------------

There are two possible choices for allocating memory:

	1.   explicit allocating / deallocating

			mem_alloc ..... allocate a memory block 
			mem_free ...... free a memory block
			mem_realloc ... change size of a memory block (position may change)
			mem_usage ..... amount of allocated memory

There are some useful macros:

	NEW (type) ....... allocate memory for an element of type `type`
	FREE (ptr,type) .. free memory
	
	MNEW (type,num) .. allocate memory for an array
	MFREE (ptr,type,num) .. free memory
	
	MREALLOC (ptr,type,num1,num2) .. enlarge the array to size num2
	                                 
-------------------------------------------------------------------------------

Some more macros:

	MEMORY_ALIGN (pos, size) ... make pos divisible by size. always returns an
                                 address >= pos.
	                      
	
	OFFSET (s,el) ....... returns the offset of 'el' in structure 's' in bytes.
	                      
	MCOPY (dest,src,type,num) ... copy 'num' elements of type 'type'.
	

*/

#define PADDING(pos,size)     (MEMORY_ALIGN((pos),(size)) - (pos))
#define OFFSET(s,el)          ((int32_t) ((ptrint) &(((s*) 0)->el)))


#define NEW(type)             ((type *) mem_alloc(sizeof(type)))
#define FREE(ptr,type)        mem_free((ptr), sizeof(type))

#define MNEW(type,num)        ((type *) mem_alloc(sizeof(type) * (num)))
#define MFREE(ptr,type,num)   mem_free((ptr), sizeof(type) * (num))

#define MREALLOC(ptr,type,num1,num2) mem_realloc((ptr), sizeof(type) * (num1), \
                                                        sizeof(type) * (num2))


#define MCOPY(dest,src,type,num) std::memcpy((dest), (src), sizeof(type) * (num))
#define MSET(ptr,byte,type,num)  std::memset((ptr), (byte), sizeof(type) * (num))
#define MZERO(ptr,type,num)      MSET(ptr,0,type,num)
#define MMOVE(dest,src,type,num) std::memmove((dest), (src), sizeof(type) * (num))


/* GC macros (boehm only) *****************************************************/

#if defined(ENABLE_GC_BOEHM)

/* Uncollectable memory which can contain references */

#define GCNEW_UNCOLLECTABLE(type,num) ((type *) heap_alloc_uncollectable(sizeof(type) * (num)))

#define GCNEW(type)           heap_alloc(sizeof(type), true, NULL, true)
#define GCMNEW(type,num)      heap_alloc(sizeof(type) * (num), true, NULL, true)

#define GCFREE(ptr)           heap_free((ptr))

#endif


/* function prototypes ********************************************************/

bool  memory_init(void);

void  memory_mprotect(void *addr, size_t len, int prot);

void *memory_checked_alloc(size_t size);

void *memory_cnew(int32_t size);
void  memory_cfree(void *p, int32_t size);

void *mem_alloc(int32_t size);
void  mem_free(void *m, int32_t size);
void *mem_realloc(void *src, int32_t len1, int32_t len2);

bool  memory_start_thread(void);

// **** a stl style memory allocator
template<class T> 
class MemoryAllocator {
public:
	// Type definitions.
	typedef T              value_type;
	typedef T*             pointer;
	typedef const T*       const_pointer;
	typedef T&             reference;
	typedef const T&       const_reference;
	typedef std::size_t    size_type;
	typedef std::ptrdiff_t difference_type;

	// Constructors and destructor, nothing to do because the
	// allocator has no state.
	MemoryAllocator() throw() {}
	MemoryAllocator(const MemoryAllocator&) throw() {}
	template <class U> MemoryAllocator(const MemoryAllocator<U>&) throw() {}

	~MemoryAllocator() throw() {}

	// ** Return address
	inline pointer       address ( reference x )       const { return &x; }
	inline const_pointer address ( const_reference x ) const { return &x; }

	// ** Allocate block of storage
	inline pointer allocate(size_type n, const_pointer hint=0)
	{
		return static_cast<pointer>(mem_alloc(n * sizeof(T)));
	}

	// ** Reallocate block of storage (non-standard!)
	inline pointer reallocate(pointer p, size_type old_sz, size_type new_sz)
	{
		return static_cast<pointer>(mem_realloc(p, old_sz * sizeof(T), new_sz * sizeof(T)));
	}
	
	// ** Release block of storage
	inline void deallocate(pointer p, size_type n)
	{
		mem_free(p,n);
	}

	// ** Maximum size possible to allocate
	// TODO

	// ** Construct an object
	void construct(pointer p, const T& value) {
		new ((void*) p) T(value);
	}

	// ** Destroy an object
	void destroy(pointer p) {
		p->~T();
	}
};

/// Allow operator new to allocate with mem_alloc
enum MemAllocPlacement { MemAlloc };

inline void *operator new(size_t size, MemAllocPlacement) {
	return mem_alloc(size);
}

#endif // MEMORY_HPP_


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
