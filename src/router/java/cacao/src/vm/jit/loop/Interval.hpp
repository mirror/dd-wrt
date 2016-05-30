/* src/vm/jit/loop/Interval.hpp

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

#ifndef _INTERVAL_HPP
#define _INTERVAL_HPP

#include <limits>
#include <iostream>

#include "vm/types.hpp"
#include "Scalar.hpp"

/**
 * An integer interval of the form
 * constant_0 + instruction_0 .. constant_1 + instruction_1
 * where both bounds are included.
 */
class Interval
{
	Scalar _lower;
	Scalar _upper;

public:

	static s4 min() { return Scalar::min(); }
	static s4 max() { return Scalar::max(); }

	/**
	 * Creates the interval MIN .. MAX
	 */
	Interval()
		: _lower(Scalar(min()))
		, _upper(Scalar(max()))
	{}

	explicit Interval(const Scalar& s)
		: _lower(s)
		, _upper(s)
	{}

	Interval(const Scalar& lower, const Scalar& upper)
		: _lower(lower)
		, _upper(upper)
	{}

	Scalar lower() const { return _lower; }
	void lower(const Scalar& s) { _lower = s; }

	Scalar upper() const { return _upper; }
	void upper(const Scalar& s) { _upper = s; }

	/**
	 * true if it can be proven that arrayVariable can be accessed at an index
	 * whose value lies within this interval.
	 */
	bool isWithinBounds(s4 arrayVariable) const;

	/**
	 * Computes a superset of the intersection of this interval with the specified interval.
	 * The result is stored in this object.
	 */
	void intersectWith(const Interval&);

	/**
	 * Computes a superset of the union of this interval with the specified interval.
	 * The result is stored in this object.
	 */
	void unionWith(const Interval&);

	/**
	 * Tries to remove the specified scalar from this interval.
	 * This interval is only changed if the result can be represented as an interval.
	 */
	void tryRemove(const Scalar&);
};

inline bool Interval::isWithinBounds(s4 arrayVariable) const
{
	return _lower.lower() >= 0												// index >= 0
		&& _upper.instruction().kind() == NumericInstruction::ARRAY_LENGTH	// index <= ? + ?.length
		&& _upper.instruction().variable() == arrayVariable					// index <= ? + arrayVariable.length
		&& _upper.constant() < 0;											// index <= -1 + arrayVariable.length
}

inline void Interval::intersectWith(const Interval& other)
{
	_lower.lowerBoundOfMaximumWith(other._lower);
	_upper.upperBoundOfMinimumWith(other._upper);
}

inline void Interval::unionWith(const Interval& other)
{
	_lower.lowerBoundOfMinimumWith(other._lower);
	_upper.upperBoundOfMaximumWith(other._upper);
}

inline void Interval::tryRemove(const Scalar& point)
{
	// Remove point from interval if it is an endpoint.
	if (point == _lower)
	{
		_lower.tryAdd(Scalar(1));
	}
	else if (point == _upper)
	{
		_upper.trySubtract(Scalar(1));
	}
}

// True if both endpoints are equal, otherwise false.
inline bool operator==(const Interval& a, const Interval& b)
{
	return a.lower() == b.lower() && a.upper() == b.upper();
}

// True if the intervals are not equal, otherwise false.
inline bool operator!=(const Interval& a, const Interval& b)
{
	return a.lower() != b.lower() || a.upper() != b.upper();
}

inline std::ostream& operator<<(std::ostream& out, const Interval& interval)
{
	return out << '[' << interval.lower() << ',' << interval.upper() << ']';
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

