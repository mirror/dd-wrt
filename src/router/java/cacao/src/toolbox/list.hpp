/* src/toolbox/list.hpp - linked list

   Copyright (C) 1996-2012
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


#ifndef _LIST_HPP
#define _LIST_HPP

#include "config.h"

#include <stdint.h>
#include <list>
#include "mm/dumpmemory.hpp"
#include "threads/mutex.hpp"

/**
 * List implementation.
 */
template<class T> class List : protected std::list<T> {
public:
	// make iterator of std::list visible
	using typename std::list<T>::iterator;
	using typename std::list<T>::reverse_iterator;

	// make functions of std::list visible
	using std::list<T>::back;
	using std::list<T>::begin;
	using std::list<T>::clear;
	using std::list<T>::empty;
	using std::list<T>::end;
	using std::list<T>::front;
	using std::list<T>::pop_front;
	using std::list<T>::push_back;
	using std::list<T>::push_front;
	using std::list<T>::rbegin;
	using std::list<T>::remove;
	using std::list<T>::rend;
	using std::list<T>::size;
};


/**
 * List implementation with a Mutex.
 */
template<class T> class LockedList : public List<T> {
private:
	Mutex _mutex;

public:
	virtual ~LockedList() {}

	void lock  () { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
};


/**
 * List implementation with dump memory.
 */
template<class T> class DumpList :
		public DumpClass,
		protected std::list<T, DumpMemoryAllocator<T> > {
public:
	virtual ~DumpList() {}

	// make iterator of std::list visible
	using typename std::list<T, DumpMemoryAllocator<T> >::iterator;
	using typename std::list<T, DumpMemoryAllocator<T> >::reverse_iterator;

	// make functions of std::list visible
	using std::list<T, DumpMemoryAllocator<T> >::back;
	using std::list<T, DumpMemoryAllocator<T> >::begin;
	using std::list<T, DumpMemoryAllocator<T> >::clear;
	using std::list<T, DumpMemoryAllocator<T> >::empty;
	using std::list<T, DumpMemoryAllocator<T> >::end;
	using std::list<T, DumpMemoryAllocator<T> >::front;
	using std::list<T, DumpMemoryAllocator<T> >::push_back;
	using std::list<T, DumpMemoryAllocator<T> >::push_front;
	using std::list<T, DumpMemoryAllocator<T> >::rbegin;
	using std::list<T, DumpMemoryAllocator<T> >::remove;
	using std::list<T, DumpMemoryAllocator<T> >::rend;
	using std::list<T, DumpMemoryAllocator<T> >::size;
	using std::list<T, DumpMemoryAllocator<T> >::sort;
};

#endif // _LIST_HPP


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
