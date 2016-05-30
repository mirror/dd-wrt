/* src/vm/breakpoint.hpp - breakpoint handling header

   Copyright (C) 2009-2013
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


#ifndef _BREAKPOINT_HPP
#define _BREAKPOINT_HPP

#include "config.h"

#include <cassert>
//#include <cstdint> <- requires C++0x
#include <stdint.h>
#include <map>

/**
 * This structure contains information about a breakpoint. Feel
 * free to extend it to hold all the information you need. It is
 * the responsibility of the user to set the fields accordingly.
 */
typedef struct Breakpoint {
	bool        is_oneshot;             ///< Indicates a "one-shot".

#if defined(ENABLE_JVMTI)
	int32_t     location;               ///< Used for JVMTI breakpoints.
	methodinfo* method;                 ///< Used for JVMTI breakpoints.
#endif
} Breakpoint;


/**
 * This class is used to record breakpoints in the methodinfo
 * structure. The term "location" is used to refer to the bytecode
 * index at which the breakpoint should be placed.
 */
class BreakpointTable {
private:
	std::map<int32_t, Breakpoint> _breakpoints;

public:
	// Querying operations.
	bool is_empty();
	bool contains(int32_t location);

	// Modification operations.
	Breakpoint* add_breakpoint   (int32_t location);
	Breakpoint* get_breakpoint   (int32_t location);
	void        remove_breakpoint(int32_t location);
};


inline bool BreakpointTable::is_empty()
{
	return _breakpoints.empty();
}

inline bool BreakpointTable::contains(int32_t location)
{
	return (_breakpoints.find(location) != _breakpoints.end());
}

inline Breakpoint* BreakpointTable::add_breakpoint(int32_t location)
{
	assert(!contains(location));
	_breakpoints.insert(std::make_pair(location, Breakpoint()));
	return &(_breakpoints.find(location)->second);
}

inline Breakpoint* BreakpointTable::get_breakpoint(int32_t location)
{
	assert(contains(location));
	return &(_breakpoints.find(location)->second);
}

inline void BreakpointTable::remove_breakpoint(int32_t location)
{
	assert(contains(location));
	_breakpoints.erase(location);
}

#endif /* _BREAKPOINT_HPP */


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
