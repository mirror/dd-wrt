// C++ IA64 / g++ v3 demangler  -*- C++ -*-

// Copyright (C) 2003 Free Software Foundation, Inc.
// Written by Carlo Wood <carlo@alinoe.com>
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "libiberty.h"

namespace __gnu_cxx
{
  namespace demangler
  {
    namespace
    {
      class alloc
      {
      public:
	static void* allocate(size_t n) { return malloc(n); }
	static void deallocate(void* p, size_t) { free(p); }
      };

      template<typename _Tp>
      class allocator
      {
	typedef alloc _Alloc;          // The underlying allocator.
      public:
	typedef size_t     size_type;
	typedef ptrdiff_t  difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;

	template<typename _Tp1>
	struct rebind
	{ typedef allocator<_Tp1> other; };

	allocator() throw() {}
	allocator(const allocator&) throw() {}
	template<typename _Tp1>
	allocator(const allocator<_Tp1>&) throw() {}
	~allocator() throw() {}

	pointer
	address(reference __x) const { return &__x; }

	const_pointer
	address(const_reference __x) const { return &__x; }

	_Tp*
	allocate(size_type __n, const void* = 0)
	{
	  _Tp* __ret = 0;
	  if (__n)
	    {
	      if (__n <= this->max_size())
		__ret = static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp)));
	      else
		xexit (1);
	    }
	  return __ret;
	}

	// __p is not permitted to be a null pointer.
	void
	deallocate(pointer __p, size_type __n)
	{ _Alloc::deallocate(__p, __n * sizeof(_Tp)); }

	size_type
	max_size() const throw() { return size_t(-1) / sizeof(_Tp); }

	void construct(pointer __p, const _Tp& __val)
	{ new(__p) _Tp(__val); }
	void destroy(pointer __p) { __p->~_Tp(); }
      };
    }
  }
}
