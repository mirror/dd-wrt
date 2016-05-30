/* src/vm/jit/loop/LoopContainer.hpp

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

#ifndef _LOOP_CONTAINER_HPP
#define _LOOP_CONTAINER_HPP

#include <vector>

#include "vm/jit/jit.hpp"
#include "VariableSet.hpp"
#include "IntervalMap.hpp"

/**
 * Represents a single loop.
 */
struct LoopContainer
{
	LoopContainer*					parent;		// the parent loop or 0 if this is the root loop
	std::vector<LoopContainer*>		children;	// all loops contained in this loop
	s4								depth;		// The depth of this loop in the loop hierarchy. The root has depth 0.

	basicblock*						header;		// the unique entry point of this loop
	std::vector<basicblock*>		nodes;		// all nodes contained in this loop except the header
	std::vector<basicblock*>		footers;	// all nodes from which there is a back edge to the header

	VariableSet						writtenVariables;		// Contains all variables that are possibly assigned/changed in this loop.
	VariableSet						invariantVariables;		// Contains (not necessarily all) integer variables that are not changed in this loop.
	VariableSet						invariantArrays;		// Contains (not necessarily all) array variables that are not changed in this loop.

	IntervalMap						invariantIntervals;		// The intervals the invariants have before entering the loop.
	//IntervalMap						headerIntervals;		// The intervals which are valid right before the header.

	bool							hasCounterVariable;
	s4								counterVariable;	// The first counter variable of this loop.
	s4								counterIncrement;	// The value which is added to the counterVariable in every iteration.
	Interval						counterInterval;	// This interval is valid for the counter variable in every loop node except the header.

	LoopContainer()
		: parent(0)
		, depth(0)
		, header(0)
//		, invariantIntervals(0)
//		, headerIntervals(0)
		, hasCounterVariable(false)
		, counterVariable(0)
		, counterIncrement(0)
	{}
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

