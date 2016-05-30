/* src/vm/jit/loop/dominator.cpp

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

#include <cassert>

#include "dominator.hpp"
#include "toolbox/logging.hpp"
#include "vm/method.hpp"


void depthFirstTraversal(jitdata* jd, basicblock* block);
void printBasicBlocks(jitdata*);
void link(basicblock* v, basicblock* w);
basicblock* eval(basicblock*);
void compress(basicblock*);


void createRoot(jitdata* jd)
{
	assert(jd->basicblocks);

	// Create a new basicblock and initialize all fields that will be used.
	jd->ld->root       = new basicblock;
	jd->ld->root->nr   = ROOT_NUMBER;
	jd->ld->root->type = basicblock::TYPE_STD;
	jd->ld->root->successorcount = 1;   // at least 1 arrayslot for jd->basicblocks

	// Find all exception handling basicblocks.
	for (basicblock* block = jd->basicblocks->next; block != 0; block = block->next)
	{
		if (block->type == basicblock::TYPE_EXH)
			jd->ld->root->successorcount++;
	}

	// Allocate and fill successor array with all found basicblocks.
	jd->ld->root->successors = new basicblock*[jd->ld->root->successorcount];
	jd->ld->root->successors[0] = jd->basicblocks;
	s4 index = 1;
	for (basicblock* block = jd->basicblocks->next; block != 0; block = block->next)
	{
		if (block->type == basicblock::TYPE_EXH)
		{
			jd->ld->root->successors[index] = block;
			index++;
		}
	}
}

/**
 * Calculates the immediate dominators of all basicblocks.
 */
void calculateDominators(jitdata* jd)
{
	assert(jd->ld->root);

	/******************
	 *     Step 1     *
	 ******************/

	// Insert an arbitrary element so that the first real item has index 1. This simplifies the implementation.
	jd->ld->vertex.push_back(0);

	assert(jd->ld->vertex.size() == 1);

	for (basicblock* block = jd->basicblocks; block != 0; block = block->next)
	{
		block->ld = new BasicblockLoopData;
	}

	// jd->ld->root is not contained in the linked list jd->basicblocks.
	jd->ld->root->ld = new BasicblockLoopData;
	
	depthFirstTraversal(jd, jd->ld->root);

	assert(static_cast<s4>(jd->ld->vertex.size()) == jd->ld->n + 1);
	for (s4 i = jd->ld->n; i >= 2; i--)
	{
		basicblock* w = jd->ld->vertex[i];

		/******************
		 *     Step 2     *
		 ******************/

		typedef std::vector<basicblock*>::iterator Iterator;

		for (Iterator it = w->ld->pred.begin(); it != w->ld->pred.end(); ++it)
		{
			basicblock* v = *it;

			basicblock* u = eval(v);
			if (u->ld->semi < w->ld->semi)
				w->ld->semi = u->ld->semi;
		}

		jd->ld->vertex[w->ld->semi]->ld->bucket.push_back(w);
		link(w->ld->parent, w);

		/******************
		 *     Step 3     *
		 ******************/

		std::vector<basicblock*>& bucket = w->ld->parent->ld->bucket;
		for (Iterator it = bucket.begin(); it != bucket.end(); ++it)
		{
			basicblock* v = *it;

			basicblock* u = eval(v);
			if (u->ld->semi < v->ld->semi)
				v->ld->dom = u;
			else
				v->ld->dom = w->ld->parent;
		}
		bucket.clear();
	}

	/******************
	 *     Step 4     *
	 ******************/

	for (s4 i = 2; i <= jd->ld->n; i++)
	{
		basicblock* w = jd->ld->vertex[i];

		if (w->ld->dom != jd->ld->vertex[w->ld->semi])
			w->ld->dom = w->ld->dom->ld->dom;
	}
	jd->ld->root->ld->dom = 0;
}

void depthFirstTraversal(jitdata* jd, basicblock* block)
{
	block->ld->semi = ++jd->ld->n;
	jd->ld->vertex.push_back(block);
	block->ld->label = block;

	// Check if jd->ld->vertex[jd->ld->n] == block
	assert(static_cast<s4>(jd->ld->vertex.size()) == jd->ld->n + 1);
	
	for (s4 i = 0; i < block->successorcount; i++)
	{
		basicblock* successor = block->successors[i];

		if (successor->ld->semi == 0)   // visited the first time?
		{
			successor->ld->parent = block;
			depthFirstTraversal(jd, successor);
		}
		else   // back edge found
		{
			jd->ld->depthBackEdges.push_back(Edge(block, successor));
		}

		successor->ld->pred.push_back(block);
	}
}

void link(basicblock* v, basicblock* w)
{
	assert(v);
	assert(w);

	w->ld->ancestor = v;
}

basicblock* eval(basicblock* block)
{
	assert(block);

	if (block->ld->ancestor == 0)
	{
		return block;
	}
	else
	{
		compress(block);
		return block->ld->label;
	}
}

void compress(basicblock* block)
{
	basicblock* ancestor = block->ld->ancestor;

	assert(ancestor != 0);

	if (ancestor->ld->ancestor != 0)
	{
		compress(ancestor);
		if (ancestor->ld->label->ld->semi < block->ld->label->ld->semi)
		{
			block->ld->label = ancestor->ld->label;
		}
		block->ld->ancestor = ancestor->ld->ancestor;
	}
}

/**
 * Uses the immediate dominators to build a dominator tree which can be traversed from the root to the leaves.
 */
void buildDominatorTree(jitdata* jd)
{
	for (basicblock* block = jd->basicblocks; block != 0; block = block->next)
	{
		if (block->ld->dom)
		{
			std::vector<basicblock*>& children = block->ld->dom->ld->children;
			
			// Every basicblock has a pointer to the next sibling in the dominator tree.
			if (!children.empty())
				children.back()->ld->nextSibling = block;

			children.push_back(block);
		}
	}
}

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

