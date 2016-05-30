/* src/vm/jit/loop/ValueMap.hpp

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

#ifndef _VALUE_MAP_HPP
#define _VALUE_MAP_HPP

#include <vector>

#include "Value.hpp"

/**
 * Contains a Value-object for every variable.
 * Initially every variable v is mapped to the value v + 0.
 */
class ValueMap
{
	std::vector<Value> _values;

public:

	//explicit ValueMap(size_t varCount);

	Value& operator[](size_t varIndex);
};

inline Value& ValueMap::operator[](size_t varIndex)
{
	for (size_t i = _values.size(); i <= varIndex; i++)
	{
		_values.push_back(Value::newAddition(i, 0));
	}
	return _values[varIndex];
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

