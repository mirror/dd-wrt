/* src/vm/jit/loop/LoopList.hpp

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

#ifndef _LOOP_LIST_HPP
#define _LOOP_LIST_HPP

#include "LoopContainer.hpp"

#include <algorithm>

namespace
{
	bool depthGreaterThan(const LoopContainer* a, const LoopContainer* b)
	{	
		return a->depth > b->depth;
	}
}

class LoopList
{
	std::vector<LoopContainer*> _loops;

public:

	typedef std::vector<LoopContainer*>::iterator iterator;

	iterator begin() { return _loops.begin(); }
	iterator end() { return _loops.end(); }

	/**
	 * Inserts the specified loop if it is not already contained in this list.
	 */
	void insert(LoopContainer* loop);

	/**
	 * Removes the specified loop from the list if the list contains it.
	 */
	void erase(LoopContainer* loop);

	/**
	 * Sort the loops in this list according to their depth in the loop hierarchy.
	 * The first loop will have the highest depth.
	 */
	void sort();

	/**
	 * Searches this list for the specified loop and returns an iterator to the found element.
	 */
	iterator find(LoopContainer* loop) { return std::find(_loops.begin(), _loops.end(), loop); }
};

inline void LoopList::insert(LoopContainer* loop)
{
	iterator it = std::find(_loops.begin(), _loops.end(), loop);
	if (it == _loops.end())
		_loops.push_back(loop);
}

inline void LoopList::erase(LoopContainer* loop)
{
	iterator it = std::find(_loops.begin(), _loops.end(), loop);
	if (it != _loops.end())
		_loops.erase(it);
}

inline void LoopList::sort()
{
	std::sort(_loops.begin(), _loops.end(), depthGreaterThan);
}

#endif

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

