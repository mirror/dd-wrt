/* src/vm/jit/loop/NumericInstruction.hpp

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

#ifndef _NUMERIC_INSTRUCTION_HPP
#define _NUMERIC_INSTRUCTION_HPP

#include <limits>
#include <iostream>
#include <cassert>

#include "vm/types.hpp"


/**
 * Represents a constant numeric instruction.
 */
class NumericInstruction
{
public:

	enum Kind   // sorted
	{
		ZERO = 0,
		ARRAY_LENGTH,
		VARIABLE
	};

private:

	Kind	_kind;
	s4		_var;

	static s4 lowerBounds[3];
	static s4 upperBounds[3];

	NumericInstruction(Kind kind, s4 variable)
		: _kind(kind)
		, _var(variable)
	{}

public:
	
	NumericInstruction(const NumericInstruction& other)
		: _kind(other._kind)
		, _var(other._var)
	{}
	
	static NumericInstruction newZero();
	static NumericInstruction newArrayLength(s4 variable);
	static NumericInstruction newVariable(s4 variable);

	Kind kind() const { return _kind; }
	s4 variable() const { return _var; }

	/**
	 * The smallest value this instruction can return.
	 */
	s4 lower() const;

	/**
	 * The largest value this instruction can return.
	 */
	s4 upper() const;
};


inline NumericInstruction NumericInstruction::newZero()
{
	return NumericInstruction(ZERO, 0);
}

inline NumericInstruction NumericInstruction::newArrayLength(s4 variable)
{
	return NumericInstruction(ARRAY_LENGTH, variable);
}

inline NumericInstruction NumericInstruction::newVariable(s4 variable)
{
	return NumericInstruction(VARIABLE, variable);
}

inline bool operator==(const NumericInstruction& a, const NumericInstruction& b)
{
	return a.kind() == b.kind() && a.variable() == b.variable();
}

inline bool operator!=(const NumericInstruction& a, const NumericInstruction& b)
{
	return a.kind() != b.kind() || a.variable() != b.variable();
}

inline s4 NumericInstruction::lower() const
{
	return lowerBounds[_kind];
}

inline s4 NumericInstruction::upper() const
{
	return upperBounds[_kind];
}

inline std::ostream& operator<<(std::ostream& out, const NumericInstruction& instruction)
{
	switch (instruction.kind())
	{
		case NumericInstruction::ZERO:			return out << '0';
		case NumericInstruction::ARRAY_LENGTH:	return out << '(' << instruction.variable() << ").length";
		case NumericInstruction::VARIABLE:		return out << '(' << instruction.variable() << ')';
		default:								assert(false);
	}
	return out;
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

