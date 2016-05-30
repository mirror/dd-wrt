/* src/vm/jit/loop/IntervalMap.hpp

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

#ifndef _INTERVAL_MAP_HPP
#define _INTERVAL_MAP_HPP

#include <iostream>
#include <cassert>
#include <algorithm>

#include "Interval.hpp"
#include "DynamicVector.hpp"

/**
 * Maps variable names to intervals.
 */
class IntervalMap
{
	DynamicVector<Interval> _intervals;

public:

	size_t size() const { return _intervals.size(); }

	/**
	 * Returns the interval of the specified variable.
	 */
	Interval& operator[](size_t varIndex);

	/**
	 * Computes the union set of this map with the specified map.
	 * This object will hold the result.
	 */
	void unionWith(const IntervalMap&);


	friend std::ostream& operator<<(std::ostream&, IntervalMap&);
};


inline Interval& IntervalMap::operator[](size_t varIndex)
{
	return _intervals[varIndex];
}

inline void IntervalMap::unionWith(const IntervalMap& other)
{
	_intervals.resize(std::min(_intervals.size(), other._intervals.size()));

	for (size_t i = 0; i < _intervals.size(); i++)
	{
		_intervals[i].unionWith(other._intervals[i]);
	}
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

