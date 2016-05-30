/* src/mm/dumpmemory.hpp - dump memory management

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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


#ifndef DUMPMEMORY_HPP_
#define DUMPMEMORY_HPP_ 1

#include <assert.h>                     // for assert
#include <stddef.h>                     // for size_t, NULL
#include <stdint.h>                     // for uint8_t
#include <cstddef>                      // for ptrdiff_t, size_t
#include <list>                         // for list
#include <new>                          // for operator new
#include <vector>                       // for vector
#include "config.h"                     // for ENABLE_MEMCHECK
#include "mm/memory.hpp"                // for MEMORY_CANARY_SIZE, etc
#include "threads/thread.hpp"           // for thread_get_current, etc
#include "vm/os.hpp"                    // for os

class DumpMemoryAllocation;
class DumpMemoryArea;
class DumpMemoryBlock;

/**
 * All classes intended to be allocated on dump memory should extend this
 * base class to inherit the appropriate allocation operators.
 */
class DumpClass {
public:
	void* operator new(size_t size);
	void operator delete(void* p);
};


/**
 * Thread-local dump memory structure.
 */
class DumpMemory {
private:
	size_t                     _size;  ///< Size of the dump areas in this dump memory.
	size_t                     _used;  ///< Used memory in this dump memory.
	std::list<DumpMemoryArea*> _areas; ///< Pointer to the current dump area.

public:
	DumpMemory();
	~DumpMemory();

	static inline DumpMemory* get_current();
	static inline void*       allocate(size_t size);

	inline void   add_size(size_t size) { _size += size; }

	inline size_t get_size() const { return _size; }
	inline size_t get_used() const { return _used; }

	inline DumpMemoryArea* get_current_area() const;

	static void* reallocate(void* src, size_t len1, size_t len2);

	void  add_area(DumpMemoryArea* dma);
	void  remove_area(DumpMemoryArea* dma);
};


/**
 * Dump memory area.
 */
class DumpMemoryArea {
private:
	size_t                        _size;   ///< Size of the current memory block.
	size_t                        _used;   ///< Used memory in the current memory block.
	std::vector<DumpMemoryBlock*> _blocks; ///< List of memory blocks in this area.
#if defined(ENABLE_MEMCHECK)
	std::vector<DumpMemoryAllocation*> _allocs; ///< List of allocations in this area.
#endif

public:
	DumpMemoryArea(size_t size = 0);
	~DumpMemoryArea();

	inline size_t get_size() const { return _size; }
	inline size_t get_used() const { return _used; }

	// Inline functions.
	inline void*            allocate(size_t size);
	inline DumpMemoryBlock* get_current_block() const;

	DumpMemoryBlock* allocate_new_block(size_t size);

#if defined(ENABLE_MEMCHECK)
private:
	void check_canaries();
#endif
};


/**
 * Dump memory block.
 */
class DumpMemoryBlock {
private:
	static const size_t DEFAULT_SIZE = 2 << 13; // 2 * 8192 bytes

	size_t _size;  ///< Size of the current memory block.
	size_t _used;  ///< Used memory in the current memory block.
	void*  _block; ///< List of memory blocks in this area.

public:
	DumpMemoryBlock(size_t size = 0);
	~DumpMemoryBlock();

	inline size_t get_size() const { return _size; }
	inline size_t get_used() const { return _used; }
	inline size_t get_free() const { return _size - _used; }

	// Inline functions.
	inline void* allocate(size_t size);
};


/**
 * Allocator for the dump memory.
 */
template<class T> class DumpMemoryAllocator {
public:
	// Type definitions.
	typedef T              value_type;
	typedef T*             pointer;
	typedef const T*       const_pointer;
	typedef T&             reference;
	typedef const T&       const_reference;
	typedef std::size_t    size_type;
	typedef std::ptrdiff_t difference_type;

	// Rebind allocator to type U.
	template <class U> struct rebind {
		typedef DumpMemoryAllocator<U> other;
	};

	// Constructors and destructor, nothing to do because the
	// allocator has no state.
	DumpMemoryAllocator() throw() {}
	DumpMemoryAllocator(const DumpMemoryAllocator&) throw() {}
	template <class U> DumpMemoryAllocator(const DumpMemoryAllocator<U>&) throw() {}

	~DumpMemoryAllocator() throw() {}

	pointer allocate(size_type n, void* = 0) {
// 		printf("allocate: n=%d * %d\n", n, sizeof(T));
		return static_cast<pointer>(DumpMemory::allocate(n * sizeof(T)));
	}

	// ** Reallocate block of storage (non-standard!)
	inline pointer reallocate(pointer p, size_type old_sz, size_type new_sz) {
		return static_cast<pointer>(DumpMemory::reallocate(p, old_sz * sizeof(T), new_sz * sizeof(T)));
	}

	// Initialize elements of allocated storage p with value value.
	void construct(pointer p, const T& value) {
// 		printf("construct: p=%p, value=%p\n", (void*) p, (void*) value);
		// Initialize memory with placement new.
		new ((void*) p) T(value);
	}

	// Destroy elements of initialized storage p.
	void destroy(pointer p) {
// 		printf("destroy: p=%p\n", (void*) p);
		// Destroy objects by calling their destructor.
		p->~T();
	}

	void deallocate(pointer p, size_type n) {
// 		printf("deallocate: p=%p, n=%d\n", (void*) p, n);
		// We don't need to deallocate on dump memory.
	}
};


/**
 * Dump memory allocation, used for for ENABLE_MEMCHECK.
 */
#if defined(ENABLE_MEMCHECK)
class DumpMemoryAllocation {
private:
	size_t _size;
	void*  _mem;

public:
	DumpMemoryAllocation() : _size(0), _mem(NULL) {}
	DumpMemoryAllocation(size_t size, void* mem) : _size(size), _mem(mem) {}
	~DumpMemoryAllocation() {};

	inline size_t get_size() const { return _size; }
	inline void*  get_mem()  const { return _mem; }
};
#endif


// Includes.
#include "mm/memory.hpp"

#include "vm/statistics.hpp"

// Inline functions.

inline void* DumpClass::operator new(size_t size)
{
	return DumpMemory::allocate(size);
}

inline void DumpClass::operator delete(void* p)
{
	// We don't need to deallocate on dump memory.
}

inline DumpMemory* DumpMemory::get_current()
{
	// Get the DumpMemory object of the current thread.
	threadobject* t = thread_get_current();
	DumpMemory* dm = t->_dumpmemory;
	return dm;
}

inline DumpMemoryArea* DumpMemory::get_current_area() const
{
	return _areas.back();
}

inline void* DumpMemory::allocate(size_t size)
{
	DumpMemory* dm = get_current();
	DumpMemoryArea* dma = dm->get_current_area();

	size_t alignedsize = size;

#if defined(ENABLE_MEMCHECK)
	alignedsize += 2 * MEMORY_CANARY_SIZE;
#endif

	// Align the allocation size.
	alignedsize = MEMORY_ALIGN(alignedsize, ALIGNSIZE);

	void* p = dma->allocate(alignedsize);

	// Increase the used count of the dump memory.
	dm->_used += alignedsize;

	return p;
}

inline DumpMemoryBlock* DumpMemoryArea::get_current_block() const
{
	return _blocks.empty() ? NULL : _blocks.back();
}

inline void* DumpMemoryArea::allocate(size_t size)
{
	DumpMemoryBlock* dmb = get_current_block();

	// Check if we have a memory block or have enough memory in the
	// current memory block.
	if (dmb == NULL || size > dmb->get_free()) {
		// No, allocate a new one.
		dmb = allocate_new_block(size);

		// Increase the size of the memory area.  We use get_size()
		// here because the default size is very likely to be bigger
		// than size.
		_size += dmb->get_size();
	}

	void* p = dmb->allocate(size);

#if defined(ENABLE_MEMCHECK)
	uint8_t *pm;
	size_t   origsize = size - 2 * MEMORY_CANARY_SIZE;

	// Make p point after the bottom canary.

	p = ((uint8_t *) p) + MEMORY_CANARY_SIZE;

	// Add the allocation to our list of allocations

	DumpMemoryAllocation* dma = new DumpMemoryAllocation(origsize, p);

	_allocs.push_back(dma);

	// Write the canaries.

	pm = ((uint8_t *) p) - MEMORY_CANARY_SIZE;

	for (int i = 0; i < MEMORY_CANARY_SIZE; ++i)
		pm[i] = i + MEMORY_CANARY_FIRST_BYTE;

	pm = ((uint8_t *) p) + dma->get_size();

	for (int i = 0; i < MEMORY_CANARY_SIZE; ++i)
		pm[i] = i + MEMORY_CANARY_FIRST_BYTE;

	// Clear the memory.

	(void) os::memset(p, MEMORY_CLEAR_BYTE, dma->get_size());
#endif /* defined(ENABLE_MEMCHECK) */

	// Increase the used size of the memory area.
	_used += size;

	return p;
}

/**
 * Allocate memory in the current dump memory area.
 *
 * This function is a fast allocator suitable for scratch memory that
 * can be collectively freed when the current activity (eg. compiling)
 * is done.
 *
 * You cannot selectively free dump memory. Before you start
 * allocating it, you remember the current size returned by
 * `dumpmemory_marker`. Later, when you no longer need the memory,
 * call `dumpmemory_release` with the remembered size and all dump
 * memory allocated since the call to `dumpmemory_marker` will be
 * freed.
 *
 * @parm size Size of block to allocate in bytes. May be zero, in which case NULL is returned
 *
 * @return Pointer to allocated memory, or NULL iff size was zero.
 */
void* DumpMemoryBlock::allocate(size_t size)
{
	if (size == 0)
		return NULL;

	// Sanity check.
	assert(size <= (_size - _used));

	// Calculate the memory address of the newly allocated memory.
	void* p = (void*) (((uint8_t*) _block) + _used);

	// Increase used memory block size by the allocated memory size.
	_used += size;

	return p;
}

// Legacy C interface.

#define DNEW(type)                    ((type*) DumpMemory::allocate(sizeof(type)))
#define DMNEW(type,num)               ((type*) DumpMemory::allocate(sizeof(type) * (num)))
#define DMREALLOC(ptr,type,num1,num2) ((type*) DumpMemory::reallocate((ptr), sizeof(type) * (num1), sizeof(type) * (num2)))

#endif // DUMPMEMORY_HPP_


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
