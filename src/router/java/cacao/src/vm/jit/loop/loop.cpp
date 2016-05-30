/* src/vm/jit/loop/loop.cpp

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

#include <sstream>
#include <algorithm>
#include <queue>

#include "loop.hpp"
#include "dominator.hpp"
#include "analyze.hpp"
#include "duplicate.hpp"
#include "toolbox/logging.hpp"

#define INDENT 2

namespace
{
	void printBasicBlocks(jitdata* jd);
	void printDominatorTree(basicblock* block, int level);
	void printLoopTree(LoopContainer* loop, int level);

	void findLoopBackEdges(jitdata* jd);
	void findLoops(jitdata* jd);
	void reverseDepthFirstTraversal(basicblock* next, LoopContainer* loop);
	void mergeLoops(jitdata* jd);
	void buildLoopTree(jitdata* jd);
	void storeLoopsInHeaders(jitdata* jd);
	void computeLoopDepth(LoopContainer* loop, s4 depth);
	void sortLoopsInBasicblocks(jitdata* jd);


	bool compareLoopHeaders(const LoopContainer* a, const LoopContainer* b)
	{
		return a->header < b->header;
	}


	void printBasicBlocks(jitdata* jd)
	{
		/*
		std::stringstream s;
		s << "Variables: " << jd->varcount;
		log_text(s.str().c_str());
		*/

		// print basic block graph
		log_text("----- Basic Blocks -----");
		for (basicblock* bb = jd->basicblocks; bb != 0; bb = bb->next)
		{
			std::stringstream str;
			str << bb->nr;

			if (bb->type == basicblock::TYPE_EXH)
				str << "*";
			else
				str << " ";
			if (bb->ld->loop)
				str << "H";
			else
				str << " ";
			if (bb->ld->leaf)
				str << "L";
			else
				str << " ";

			str << " --> ";
			for (s4 i = 0; i < bb->successorcount; i++)
			{
				str << bb->successors[i]->nr << " ";
			}

			log_text(str.str().c_str());
		}

		// print immediate dominators
		log_text("------ Dominators ------");
		for (basicblock* bb = jd->basicblocks; bb != 0; bb = bb->next)
		{
			std::stringstream str;
			str << bb->nr << " --> ";
			if (bb->ld->dom)
				str << bb->ld->dom->nr;
			else
				str << "X";

			// print variable assignments
			//str << " " << bb->ld->writtenVariables;

			log_text(str.str().c_str());
		}

		// print dominator tree
		log_text("------ Dominator Tree ------");
		printDominatorTree(jd->ld->root, 0);

		// print loops
		log_text("------ Loops ------");
		for (size_t i = 0; i < jd->ld->loops.size(); i++)
		{
			LoopContainer* loop = jd->ld->loops[i];
			std::stringstream str;
			str << loop->header->nr;
			for (size_t j = 0; j < loop->nodes.size(); j++)
			{
				str << " " << loop->nodes[j]->nr;
			}

			// print variable assignments
			str << " W:" << loop->writtenVariables;
			
			// print invariant variables
			str << " I:" << loop->invariantVariables;

			// print counter
			if (loop->hasCounterVariable)
				str << " counter " << loop->counterVariable << ':' << loop->counterInterval << ',' << loop->counterIncrement;

			log_text(str.str().c_str());
		}

		// print loop tree
		log_text("------ Loop Tree ------");
		printLoopTree(jd->ld->rootLoop, 0);

		log_text("------------------------");
	}

	void printDominatorTree(basicblock* block, int level)
	{
		std::stringstream str;

		// print indentation
		for (int i = 0; i < level; i++)
		{
			str << "|";
			for (int j = 0; j < INDENT - 1; j++)
			{
				str << " ";
			}
		}

		// print root
		str << block->nr;
		log_text(str.str().c_str());

		// print children
		for (size_t i = 0; i < block->ld->children.size(); i++)
		{
			printDominatorTree(block->ld->children[i], level + 1);
		}
	}

	void printLoopTree(LoopContainer* loop, int level)
	{
		std::stringstream str;

		// print indentation
		for (int i = 0; i < level; i++)
		{
			str << "|";
			for (int j = 0; j < INDENT - 1; j++)
			{
				str << " ";
			}
		}

		// print root
		if (loop->header)
			str << loop->header->nr;
		else
			str << "R";   // pseudo root loop
		log_text(str.str().c_str());

		// print children
		for (size_t i = 0; i < loop->children.size(); i++)
		{
			printLoopTree(loop->children[i], level + 1);
		}
	}

	void findLoopBackEdges(jitdata* jd)
	{
		// Iterate over depthBackEdges and filter those out which are also loopBackEdges.
		for (std::vector<Edge>::const_iterator edge = jd->ld->depthBackEdges.begin(); edge != jd->ld->depthBackEdges.end(); ++edge)
		{
			// Check if edge->to dominates edge->from.
			// The case edge->to == edge->from is also considered.
			basicblock* ancestor = edge->from;
			while (ancestor)
			{
				if (ancestor == edge->to)
				{
					jd->ld->loopBackEdges.push_back(*edge);
					break;
				}

				// search the dominator tree bottom-up
				ancestor = ancestor->ld->dom;
			}
		}
	}

	/**
	 * For every loopBackEdge there is a loop.
	 * This function finds all basicblocks which belong to that loop.
	 */
	void findLoops(jitdata* jd)
	{
		for (std::vector<Edge>::const_iterator edge = jd->ld->loopBackEdges.begin(); edge != jd->ld->loopBackEdges.end(); ++edge)
		{
			basicblock* head = edge->to;
			basicblock* foot = edge->from;

			LoopContainer* loop = new LoopContainer;
			//loop->invariantIntervals = IntervalMap(jd->varcount);
			//loop->headerIntervals = IntervalMap(jd->varcount);
			jd->ld->loops.push_back(loop);
			
			loop->header = head;
			loop->footers.push_back(foot);

			// Save loop in basicblock.
			head->ld->loops.insert(loop);

			// find all basicblocks contained in this loop
			reverseDepthFirstTraversal(foot, loop);
		}
	}

	void reverseDepthFirstTraversal(basicblock* next, LoopContainer* loop)
	{
		if (next->ld->visited == loop)	// already visited
			return;

		if (next == loop->header)		// Stop the traversal at the header node.
			return;

		loop->nodes.push_back(next);

		// Save loop in basicblock.
		next->ld->loops.insert(loop);

		// Mark basicblock to prevent it from being visited again.
		next->ld->visited = loop;

		for (s4 i = 0; i < next->predecessorcount; i++)
		{
			reverseDepthFirstTraversal(next->predecessors[i], loop);
		}
	}

	/**
	 * There can be loop back edges having the same header node.
	 * This function merges those loops together.
	 */
	void mergeLoops(jitdata* jd)
	{
		// Sort the loops such that two loops having the same header are one after the other.
		std::sort(jd->ld->loops.begin(), jd->ld->loops.end(), &compareLoopHeaders);
		//std::sort(jd->ld->loops.begin(), jd->ld->loops.end(), LoopHeaderCompare());

		for (size_t i = 1; i < jd->ld->loops.size(); i++)
		{
			LoopContainer* first = jd->ld->loops[i - 1];
			LoopContainer* second = jd->ld->loops[i];

			if (first->header == second->header)
			{
				// Copy footer of second loop into first loop
				assert(second->footers.size() == 1);
				first->footers.push_back(second->footers.front());

				// merge loops
				for (std::vector<basicblock*>::const_iterator it2 = second->nodes.begin(); it2 != second->nodes.end(); ++it2)
				{
					basicblock* node = *it2;

					// Put node into the first loop if it does not already contain it.
					if (find(first->nodes.begin(), first->nodes.end(), node) == first->nodes.end())
					{
						first->nodes.push_back(node);
					}

					// Remove second loop from node and save first loop in it.
					node->ld->loops.erase(second);
					node->ld->loops.insert(first);
				}

				// Remove second loop from header too.
				first->header->ld->loops.erase(second);

				// delete second loop
				delete second;
				jd->ld->loops.erase(jd->ld->loops.begin() + i);
				i--;
			}
		}
	}

	/**
	 * Builds the loop hierarchy.
	 */
	void buildLoopTree(jitdata* jd)
	{
		std::queue<LoopContainer*> successors;

		// Create a pseudo loop that contains all other loops.
		jd->ld->rootLoop = new LoopContainer;
		for (std::vector<LoopContainer*>::const_iterator it = jd->ld->loops.begin(); it != jd->ld->loops.end(); ++it)
		{
			(*it)->parent = jd->ld->rootLoop;
			successors.push(*it);
		}

		// Do a breadth-first traversal through the loop nodes.
		while (!successors.empty())
		{
			LoopContainer* current = successors.front();
			successors.pop();

			// find all successors
			for (std::vector<LoopContainer*>::const_iterator it = jd->ld->loops.begin(); it != jd->ld->loops.end(); ++it)
			{
				LoopContainer* candidate = *it;

				// candidate is a sub loop of current iff its header is contained in current.
				for (std::vector<basicblock*>::const_iterator nodeIt = current->nodes.begin(); nodeIt != current->nodes.end(); ++nodeIt)
				{
					// The headers of two different loops are always different.
					// So it is not necessary to compare the headers.

					if (candidate->header == *nodeIt)
					{
						// Remember the parent of the sub loop.
						// We overwrite a previously written value.
						// Because we are doing a breadth-first traversal and we overwrite a previously written value,
						//   we will eventually get the longest paths from the root to the leaves.
						candidate->parent = current;

						successors.push(candidate);
						break;
					}
				}
			}
		}

		// Finally build the tree so that it can be traversed top-down.
		for (std::vector<LoopContainer*>::const_iterator it = jd->ld->loops.begin(); it != jd->ld->loops.end(); ++it)
		{
			LoopContainer* loop = *it;
			if (loop->parent)
				loop->parent->children.push_back(loop);
		}
	}

	void storeLoopsInHeaders(jitdata* jd)
	{
		for (std::vector<LoopContainer*>::const_iterator it = jd->ld->loops.begin(); it != jd->ld->loops.end(); ++it)
		{
			(*it)->header->ld->loop = (*it);
		}
	}

	void computeLoopDepth(LoopContainer* loop, s4 depth)
	{
		loop->depth = depth;
		for (std::vector<LoopContainer*>::iterator childIt = loop->children.begin(); childIt != loop->children.end(); ++childIt)
		{
			computeLoopDepth(*childIt, depth + 1);
		}
	}

	void sortLoopsInBasicblocks(jitdata* jd)
	{
		for (basicblock* block = jd->basicblocks; block != 0; block = block->next)
		{
			block->ld->loops.sort();
		}
	}
}


void removeArrayBoundChecks(jitdata* jd)
{
	//log_message_method("removeArrayBoundChecks: ", jd->m);

	createRoot(jd);
	calculateDominators(jd);
	buildDominatorTree(jd);
	
	findLoopBackEdges(jd);
	findLoops(jd);
	mergeLoops(jd);
	buildLoopTree(jd);
	storeLoopsInHeaders(jd);
	computeLoopDepth(jd->ld->rootLoop, 0);
	sortLoopsInBasicblocks(jd);

	analyzeLoops(jd);
	findLeaves(jd);
	
	removeFullyRedundantChecks(jd);

	//printBasicBlocks(jd);

	if (jd->exceptiontablelength == 0)   // Currently only methods without try blocks are allowed.
	{
		if (findFreeVariable(jd))
		{
			removePartiallyRedundantChecks(jd);
			groupArrayBoundsChecks(jd);
		}
	}
	else
	{
		//log_text("Exception handler found.");
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

