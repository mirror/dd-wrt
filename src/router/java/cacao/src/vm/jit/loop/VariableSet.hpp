/* src/vm/jit/loop/VariableSet.hpp

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

#ifndef _VARIABLE_SET
#define _VARIABLE_SET

#include <set>
#include <iostream>

#include "vm/types.hpp"

/**
 * A container for variables names.
 * Every variable is contained no more than once.
 */
class VariableSet
{
	std::set<s4> _variables;

public:

	typedef std::set<s4>::iterator iterator;
	
	void insert(s4 variableIndex) { _variables.insert(variableIndex); }
	void remove(s4 variableIndex) { _variables.erase(variableIndex); }
	bool contains(s4 variableIndex) { return _variables.find(variableIndex) != _variables.end(); }

	std::set<s4>::iterator begin() { return _variables.begin(); }
	std::set<s4>::iterator end() { return _variables.end(); }

	friend std::ostream& operator<<(std::ostream&, const VariableSet&);
};

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

