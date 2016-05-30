/* src/vm/jit/loop/loop.hpp

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

#ifndef _LOOP_HPP
#define _LOOP_HPP


typedef struct MethodLoopData MethodLoopData;
typedef struct BasicblockLoopData BasicblockLoopData;
typedef struct LoopContainer LoopContainer;
typedef struct Edge Edge;


#if defined(__cplusplus)


#include <vector>

#include "vm/jit/jit.hpp"
#include "LoopContainer.hpp"
#include "VariableSet.hpp"
#include "IntervalMap.hpp"
#include "LoopList.hpp"

/**
 * Per-method data used in jitdata.
 */
struct MethodLoopData
{
	std::vector<basicblock*>	vertex;
	s4 							n;
	basicblock*					root;

	// During the depth first traversal in dominator.cpp an edge is added to this vector
	// if it points from the current node to a node which has already been visited.
	// Not every edge in this vector is also a loopBackEdge!
	std::vector<Edge>			depthBackEdges;

	// Contains all edges (a,b) from depthBackEdges where b dominates a.
	std::vector<Edge>			loopBackEdges;

	// Contains pointers to all loops in this method.
	std::vector<LoopContainer*>	loops;

	// Every method has exactly one (pseudo) root loop that is executed exactly once.
	LoopContainer*				rootLoop;

	// An index to a free integer variable in the jd->var array.
	s4							freeVariable;

	MethodLoopData()
		: n(0)
		, root(0)
		, rootLoop(0)
		, freeVariable(0)
	{}
};

/**
 * Per-basicblock data used in basicblock.
 * Contains information about the dominator tree.
 */
struct BasicblockLoopData
{
	basicblock*					parent;
	std::vector<basicblock*>	pred;
	s4							semi;
	std::vector<basicblock*>	bucket;
	basicblock*					ancestor;
	basicblock*					label; 

	basicblock*					dom;			// after calculateDominators: the immediate dominator
	basicblock*					nextSibling;	// pointer to the next sibling in the dominator tree or 0.
	std::vector<basicblock*>	children;		// the children of a node in the dominator tree

	// Used to prevent this basicblock from being visited again during a traversal in loop.cpp.
	// This is NOT a pointer to the loop this basicblock belongs to because such a loop is not unique.
	LoopContainer*				visited;	

	LoopContainer*				loop;   // The loop which this basicblock is the header of. Can be 0.
	LoopList					loops;	// All loops this basicblock is a part of.

	// The number of loop back edges that leave this basicblock.
	s4							outgoingBackEdgeCount;

	bool						leaf;

	// true if analyze has been called for this node, false otherwise.
	bool						analyzed;

	basicblock*					jumpTarget;
	IntervalMap					targetIntervals;
	IntervalMap					intervals;

	LoopContainer*				belongingTo;	// used during loop duplication: a marker for blocks to be duplicated.
	basicblock*					copiedTo;		// used during loop duplication: points to the same block in the duplicated loop.

	// Used during grouping: This variable points to the first check after cloning this block.
	basicblock*					arrayIndexCheck;

	BasicblockLoopData()
		: parent(0)
		, semi(0)
		, ancestor(0)
		, label(0)
		, dom(0)
		, nextSibling(0)
		, visited(0)
		, loop(0)
		, outgoingBackEdgeCount(0)
		, leaf(false)
		, analyzed(false)
		, jumpTarget(0)
//		, targetIntervals(0)
//		, intervals(0)
		, belongingTo(0)
		, copiedTo(0)
		, arrayIndexCheck(0)
	{}
};

/**
 * An edge in the control flow graph.
 */
struct Edge
{
	basicblock*		from;
	basicblock*		to;

	Edge()
		: from(0)
		, to(0)
	{}

	Edge(basicblock* from, basicblock* to)
		: from(from)
		, to(to)
	{}
};


void removeArrayBoundChecks(jitdata*);


#endif // __cplusplus
#endif // _LOOP_HPP

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

