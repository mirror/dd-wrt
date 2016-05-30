/* src/vm/jit/loop/analyze.cpp

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

#include "analyze.hpp"
#include "Interval.hpp"
#include "toolbox/logging.hpp"
#include "vm/jit/ir/icmd.hpp"

#include <sstream>

namespace
{
	void analyzeLoops(jitdata* jd);
	void analyzeBasicblockInLoop(jitdata* jd, basicblock* block, LoopContainer* loop);
	void analyzeFooter(jitdata* jd, LoopContainer* loop);
	void findLeaves(jitdata* jd);

	bool isRegularPredecessor(jitdata* jd, basicblock* node, basicblock* pred);
	LoopContainer* getLoopContainer(jitdata* jd, basicblock* header);
	IntervalMap analyze(jitdata* jd, basicblock* node, basicblock* target);

	bool isLocalIntVar(jitdata* jd, s4 varIndex);
	bool isLocalIntVar(jitdata* jd, s4 var0, s4 var1);
	bool isLocalVar(jitdata* jd, s4 varIndex);

	/**
	 * Analyzes a single basicblock that belongs to a loop and finds
	 *
	 *   -) all written variables,
	 *   -) candidates for the set of invariant integer variables,
	 *   -) candidates for the set of invariant array variables.
	 */
	void analyzeBasicblockInLoop(jitdata* jd, basicblock* block, LoopContainer* loop)
	{
		for (instruction* instr = block->iinstr; instr != block->iinstr + block->icount; instr++)
		{
			switch (instr->opc)
			{
				case ICMD_COPY:
				case ICMD_MOVE:
				case ICMD_ILOAD:
				case ICMD_LLOAD:
				case ICMD_FLOAD:
				case ICMD_DLOAD:
				case ICMD_ALOAD:
				case ICMD_ISTORE:
				case ICMD_LSTORE:
				case ICMD_FSTORE:
				case ICMD_DSTORE:
				case ICMD_ASTORE:
					
					// Save candidates for the set of invariant integer variables.
					if (isLocalIntVar(jd, instr->s1.varindex))
						loop->invariantVariables.insert(instr->s1.varindex);

					// The dst-variable is changed only if src != dst.
					if (instr->s1.varindex != instr->dst.varindex)
						loop->writtenVariables.insert(instr->dst.varindex);
					break;

				case ICMD_IALOAD:
				case ICMD_LALOAD:
				case ICMD_FALOAD:
				case ICMD_DALOAD:
				case ICMD_AALOAD:
				case ICMD_BALOAD:
				case ICMD_CALOAD:
				case ICMD_SALOAD:

				case ICMD_IASTORE:
				case ICMD_LASTORE:
				case ICMD_FASTORE:
				case ICMD_DASTORE:
				case ICMD_AASTORE:
				case ICMD_BASTORE:
				case ICMD_CASTORE:
				case ICMD_SASTORE:

				case ICMD_IASTORECONST:
				case ICMD_LASTORECONST:
				case ICMD_FASTORECONST:
				case ICMD_DASTORECONST:
				case ICMD_AASTORECONST:
				case ICMD_BASTORECONST:
				case ICMD_CASTORECONST:
				case ICMD_SASTORECONST:
					
					// Save candidates for the set of invariant array variables.
					if (isLocalVar(jd, instr->s1.varindex))
						loop->invariantArrays.insert(instr->s1.varindex);
					
					// fall through

				default:

					if (instruction_has_dst(instr))
					{
						// Store changed variable.
						loop->writtenVariables.insert(instr->dst.varindex);
					}
			}
		}
	}

	/**
	 * Analyzes the footer of the specified loop (Only the first footer is considered) and
	 * finds all counter variables.
	 *
	 * It is important that analyzeBasicblockInLoop has been called for every basicblock
	 * in this loop except the footer.
	 */
	void analyzeFooter(jitdata* jd, LoopContainer* loop)
	{
		assert(!loop->footers.empty());
		
		basicblock* footer = loop->footers.front();

		enum { SEEKING_JUMP, JUMP_FOUND, INC_FOUND } state = SEEKING_JUMP;

		for (s4 i = footer->icount - 1; i >= 0; i--)
		{
			instruction* instr = &footer->iinstr[i];
			
			switch (state)
			{
				case SEEKING_JUMP:

					// Skip NOPs at the end of the basicblock until the jump instruction is found.
					if (icmd_table[instr->opc].controlflow != CF_NORMAL)
						state = JUMP_FOUND;
					break;

				case JUMP_FOUND:

					if (instr->opc == ICMD_IINC &&				// For simplicity only IINC-instructions are considered.
						isLocalIntVar(jd, instr->dst.varindex) &&
						!loop->writtenVariables.contains(instr->dst.varindex))	// A counter variable is written only once in the whole loop.
					{
						loop->hasCounterVariable = true;
						loop->counterVariable = instr->dst.varindex;
						loop->counterIncrement = instr->sx.val.i;
						state = INC_FOUND;
					}
					else
					{
						return;
					}
					break;

				case INC_FOUND:

					switch (instr->opc)
					{
						case ICMD_COPY:
						case ICMD_MOVE:
						case ICMD_ILOAD:
						case ICMD_LLOAD:
						case ICMD_FLOAD:
						case ICMD_DLOAD:
						case ICMD_ALOAD:
						case ICMD_ISTORE:
						case ICMD_LSTORE:
						case ICMD_FSTORE:
						case ICMD_DSTORE:
						case ICMD_ASTORE:
							// If src == dst, the counter is definitely not changed.
							if (instr->s1.varindex == instr->dst.varindex)
								break;
							// fall through
						default:
							// The counter variable must not be written to in all other instructions.
							if (instruction_has_dst(instr) && instr->dst.varindex == loop->counterVariable)
							{
								loop->hasCounterVariable = false;
								return;
							}
					}
					break;
			}
		}
	}

	/**
	 * Returns true if there is no back-edge from pred to node, otherwise false.
	 */
	inline bool isRegularPredecessor(jitdata* jd, basicblock* node, basicblock* pred)
	{
		for (std::vector<Edge>::iterator it = jd->ld->loopBackEdges.begin(); it != jd->ld->loopBackEdges.end(); ++it)
		{
			if (it->from == pred && it->to == node)
				return false;
		}
		return true;
	}

	/**
	 * Removes all fully redundant array bound checks in node and its predecessors in the CFG.
	 * Returns the interval map for the edge from node to target.
	 */
	IntervalMap analyze(jitdata* jd, basicblock* node, basicblock* target)
	{
		if (node->ld->analyzed)
		{
			if (target == node->ld->jumpTarget)
				return node->ld->targetIntervals;
			else
				return node->ld->intervals;
		}

		// Prevent this node from being analyzed again.
		node->ld->analyzed = true;

		// Define shortcuts.
		IntervalMap& intervals = node->ld->intervals;
		IntervalMap& targetIntervals = node->ld->targetIntervals;

		// Initialize results. That is necessary because analyze can be called recursively.
		/*targetIntervals = IntervalMap(jd->varcount);
		intervals = IntervalMap(jd->varcount);*/

		// If node is a root, the variables (e.g. function arguments) can have any value.
		// Otherwise we can take the interval maps from the predecessors.
		if (node->predecessorcount > 0)
		{
			// Initialize intervals.
			if (isRegularPredecessor(jd, node, node->predecessors[0]))
				intervals = analyze(jd, node->predecessors[0], node);

			// Combine it with the other intervals.
			for (s4 i = 1; i < node->predecessorcount; i++)
			{
				if (isRegularPredecessor(jd, node, node->predecessors[i]))
					intervals.unionWith(analyze(jd, node->predecessors[i], node));
			}
		}

		// When a loop is left, we must reset the intervals of the invariant variables.
		for (s4 i = 0; i < node->predecessorcount; i++)
		{
			basicblock* pred = node->predecessors[i];
			for (LoopList::iterator predLoopIt = pred->ld->loops.begin(); predLoopIt != pred->ld->loops.end(); ++predLoopIt)
			{
				LoopContainer* predLoop = *predLoopIt;
				if (node->ld->loops.find(predLoop) == node->ld->loops.end())
				{
					// We left predLoop.
					// So we must reset the intervals of predLoop's invariant variables.
					for (VariableSet::iterator it = predLoop->invariantVariables.begin(); it != predLoop->invariantVariables.end(); ++it)
					{
						intervals[*it] = predLoop->invariantIntervals[*it];
					}

					// Set all intervals referencing an invariant variable to [MIN,MAX].
					for (size_t j = 0; j < intervals.size(); j++)
					{
						// Do not change the interval of an invariant variable.
						if (predLoop->invariantVariables.contains(j))
							continue;

						if (intervals[j].lower().instruction().kind() == NumericInstruction::VARIABLE &&
							predLoop->invariantVariables.contains(intervals[j].lower().instruction().variable()))
						{
							intervals[j] = Interval();
						}
						else if (intervals[j].upper().instruction().kind() == NumericInstruction::VARIABLE &&
							predLoop->invariantVariables.contains(intervals[j].upper().instruction().variable()))
						{
							intervals[j] = Interval();
						}
					}
				}
			}
		}

		if (node->ld->loop)   // node is a loop header
		{
			// Define the interval of the counter variable, not considering a possible overflow.
			// This interval will be overwritten after the last instruction in this node has been processed.
			if (node->ld->loop->hasCounterVariable)
			{
				if (node->ld->loop->counterIncrement >= 0)
					node->ld->loop->counterInterval = Interval(intervals[node->ld->loop->counterVariable].lower(), Scalar(Scalar::max()));
				else
					node->ld->loop->counterInterval = Interval(Scalar(Scalar::min()), intervals[node->ld->loop->counterVariable].upper());
			}

			// If node is the header of a loop L,
			// set every interval that belongs to a variable which is changed in L to [MIN,MAX].
			for (VariableSet::iterator it = node->ld->loop->writtenVariables.begin(); it != node->ld->loop->writtenVariables.end(); ++it)
			{
				intervals[*it] = Interval();
			}

			// Set the interval of every invariant variable x to [x,x] and save old interval.
			for (VariableSet::iterator it = node->ld->loop->invariantVariables.begin(); it != node->ld->loop->invariantVariables.end(); ++it)
			{
				node->ld->loop->invariantIntervals[*it] = intervals[*it];
				intervals[*it] = Interval(Scalar(NumericInstruction::newVariable(*it)));
			}
		}

		for (instruction* instr = node->iinstr; instr != node->iinstr + node->icount; instr++)
		{
			// arguments
			s4 dst_index = instr->dst.varindex;
			s4 s1_index = instr->s1.varindex;
			s4 s2_index = instr->sx.s23.s2.varindex;
			s4 value = instr->sx.val.i;

			// Update the variable intervals for this node.
			// For an unknown instruction, set the corresponding interval to [MIN,MAX].
			switch (instr->opc)
			{
				case ICMD_ICONST:
					if (isLocalIntVar(jd, dst_index))
					{
						intervals[dst_index] = Interval(Scalar(value));
					}
					break;

				case ICMD_COPY:
				case ICMD_MOVE:
				case ICMD_ILOAD:
				case ICMD_ISTORE:
					if (isLocalIntVar(jd, dst_index))
					{
						// The source variable does not have to be local because
						// the interval of a non-local variable is [MIN,MAX].

						if (s1_index != dst_index)
						{
							// Overwrite dst-interval by s1-interval.
							intervals[dst_index] = intervals[s1_index];
						}
					}
					break;

				case ICMD_ARRAYLENGTH:
					if (isLocalIntVar(jd, dst_index))
					{
						// Create interval [s1.length, s1.length]
						intervals[dst_index] = Interval(Scalar(NumericInstruction::newArrayLength(s1_index)));
					}
					break;

				case ICMD_IINC:
				case ICMD_IADDCONST:
					if (isLocalIntVar(jd, dst_index))
					{
						Scalar l = intervals[s1_index].lower();
						Scalar u = intervals[s1_index].upper();
						Scalar offset(value);

						if (l.tryAdd(offset) && u.tryAdd(offset))
						{
							intervals[dst_index] = Interval(l, u);
						}
						else   // overflow
						{
							intervals[dst_index] = Interval();
						}
					}
					break;

				case ICMD_ISUBCONST:
					if (isLocalIntVar(jd, dst_index))
					{
						Scalar l = intervals[s1_index].lower();
						Scalar u = intervals[s1_index].upper();
						Scalar offset(value);

						if (l.trySubtract(offset) && u.trySubtract(offset))
						{
							intervals[dst_index] = Interval(l, u);
						}
						else   // overflow
						{
							intervals[dst_index] = Interval();
						}
					}
					break;

				case ICMD_IALOAD:
				case ICMD_LALOAD:
				case ICMD_FALOAD:
				case ICMD_DALOAD:
				case ICMD_AALOAD:
				case ICMD_BALOAD:
				case ICMD_CALOAD:
				case ICMD_SALOAD:

				case ICMD_IASTORE:
				case ICMD_LASTORE:
				case ICMD_FASTORE:
				case ICMD_DASTORE:
				case ICMD_AASTORE:
				case ICMD_BASTORE:
				case ICMD_CASTORE:
				case ICMD_SASTORE:

				case ICMD_IASTORECONST:
				case ICMD_LASTORECONST:
				case ICMD_FASTORECONST:
				case ICMD_DASTORECONST:
				case ICMD_AASTORECONST:
				case ICMD_BASTORECONST:
				case ICMD_CASTORECONST:
				case ICMD_SASTORECONST:

					// Remove array bounds check.
					if (intervals[s2_index].isWithinBounds(s1_index))
						instr->flags.bits &= ~INS_FLAG_CHECK;
					break;

				case ICMD_IFEQ:   // if (S1 == INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					Scalar s(value);
					Interval i(s);

					// TRUE BRANCH
					targetIntervals = intervals;
					targetIntervals[s1_index].intersectWith(i);

					// FALSE BRANCH
					intervals[s1_index].tryRemove(s);
				
					break;
				}
				case ICMD_IFNE:   // if (S1 != INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					Scalar s(value);
					Interval i(s);

					// TRUE BRANCH
					targetIntervals = intervals;
					targetIntervals[s1_index].tryRemove(s);

					// FALSE BRANCH
					intervals[s1_index].intersectWith(i);
					
					break;
				}
				case ICMD_IFLT:   // if (S1 < INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						Scalar l(Scalar::min());
						Scalar u(value);

						// interval [MIN,value-1]
						Interval i(l, u);
						i.tryRemove(u);

						targetIntervals = intervals;
						targetIntervals[s1_index].intersectWith(i);
					}

					// FALSE BRANCH
					{
						Scalar l(value);
						Scalar u(Scalar::max());

						// interval [value,MAX].
						Interval i(l, u);

						intervals[s1_index].intersectWith(i);
					}
					
					break;
				}
				case ICMD_IFLE:   // if (S1 <= INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						Scalar l(Scalar::min());
						Scalar u(value);

						// interval [MIN,value]
						Interval i(l, u);

						targetIntervals = intervals;
						targetIntervals[s1_index].intersectWith(i);
					}

					// FALSE BRANCH
					{
						Scalar l(value);
						Scalar u(Scalar::max());

						// interval [value+1,MAX].
						Interval i(l, u);
						i.tryRemove(l);

						intervals[s1_index].intersectWith(i);
					}
					
					break;
				}
				case ICMD_IFGT:   // if (S1 > INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						Scalar l(value);
						Scalar u(Scalar::max());

						// interval [value+1,MAX]
						Interval i(l, u);
						i.tryRemove(l);

						targetIntervals = intervals;
						targetIntervals[s1_index].intersectWith(i);
					}

					// FALSE BRANCH
					{
						Scalar l(Scalar::min());
						Scalar u(value);

						// interval [MIN,value].
						Interval i(l, u);

						intervals[s1_index].intersectWith(i);
					}
					
					break;
				}
				case ICMD_IFGE:   // if (S1 >= INT_CONST) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						Scalar l(value);
						Scalar u(Scalar::max());

						// interval [value,MAX]
						Interval i(l, u);

						targetIntervals = intervals;
						targetIntervals[s1_index].intersectWith(i);
					}

					// FALSE BRANCH
					{
						Scalar l(Scalar::min());
						Scalar u(value);

						// interval [MIN,value-1].
						Interval i(l, u);
						i.tryRemove(u);

						intervals[s1_index].intersectWith(i);
					}
					
					break;
				}
				case ICMD_IF_ICMPEQ:   // if (S1 == S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH

					// S1, S2 lie in the intersection of their intervals.
					targetIntervals = intervals;
					targetIntervals[s1_index].intersectWith(intervals[s2_index]);
					targetIntervals[s2_index] = targetIntervals[s1_index];

					// FALSE BRANCH

					IntervalMap temp = intervals;

					// If the interval of S2 contains only one element A,
					// then A can be removed from the other interval.
					if (temp[s2_index].lower() == temp[s2_index].upper())
					{
						intervals[s1_index].tryRemove(temp[s2_index].lower());
					}

					// If the interval of S1 contains only one element A,
					// then A can be removed from the other interval.
					if (temp[s1_index].lower() == temp[s1_index].upper())
					{
						intervals[s2_index].tryRemove(temp[s1_index].lower());
					}

					break;
				}
				case ICMD_IF_ICMPNE:   // if (S1 != S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH

					targetIntervals = intervals;

					// If the interval of S2 contains only one element A,
					// then A can be removed from the other interval.
					if (intervals[s2_index].lower() == intervals[s2_index].upper())
					{
						targetIntervals[s1_index].tryRemove(intervals[s2_index].lower());
					}

					// If the interval of S1 contains only one element A,
					// then A can be removed from the other interval.
					if (intervals[s1_index].lower() == intervals[s1_index].upper())
					{
						targetIntervals[s2_index].tryRemove(intervals[s1_index].lower());
					}

					// FALSE BRANCH

					// S1, S2 lie in the intersection of their intervals.
					intervals[s1_index].intersectWith(intervals[s2_index]);
					intervals[s2_index] = intervals[s1_index];

					break;
				}
				case ICMD_IF_ICMPLT:   // if (S1 < S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						targetIntervals = intervals;

						// Create interval containing all elements less than the biggest instance of S2.
						Interval lessThan;
						lessThan.upper(intervals[s2_index].upper());
						lessThan.tryRemove(intervals[s2_index].upper());

						// Create interval containing all elements greater than the smallest instance of S1.
						Interval greaterThan;
						greaterThan.lower(intervals[s1_index].lower());
						greaterThan.tryRemove(intervals[s1_index].lower());

						// S1 is less than the biggest instance of S2.
						targetIntervals[s1_index].intersectWith(lessThan);

						// S2 is greater than the smallest instance of S1.
						targetIntervals[s2_index].intersectWith(greaterThan);
					}

					// FALSE BRANCH
					{
						// Create interval containing all elements greater than or equal the smallest instance of S2.
						Interval greaterThan;
						greaterThan.lower(intervals[s2_index].lower());

						// Create interval containing all elements less than or equal the biggest instance of S1.
						Interval lessThan;
						lessThan.upper(intervals[s1_index].upper());

						// S1 is greater than or equal the smallest instance of S2.
						intervals[s1_index].intersectWith(greaterThan);

						// S2 is less than or equal the biggest instance of S1.
						intervals[s2_index].intersectWith(lessThan);
					}

					break;
				}
				case ICMD_IF_ICMPLE:   // if (S1 <= S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						targetIntervals = intervals;

						// Create interval containing all elements less than or equal the biggest instance of S2.
						Interval lessThan;
						lessThan.upper(intervals[s2_index].upper());

						// Create interval containing all elements greater than or equal the smallest instance of S1.
						Interval greaterThan;
						greaterThan.lower(intervals[s1_index].lower());

						// S1 is less than or equal the biggest instance of S2.
						targetIntervals[s1_index].intersectWith(lessThan);

						// S2 is greater than or equal the smallest instance of S1.
						targetIntervals[s2_index].intersectWith(greaterThan);
					}

					// FALSE BRANCH
					{
						// Create interval containing all elements greater than the smallest instance of S2.
						Interval greaterThan;
						greaterThan.lower(intervals[s2_index].lower());
						greaterThan.tryRemove(intervals[s2_index].lower());

						// Create interval containing all elements less than the biggest instance of S1.
						Interval lessThan;
						lessThan.upper(intervals[s1_index].upper());
						lessThan.tryRemove(intervals[s1_index].upper());

						// S1 is greater than the smallest instance of S2.
						intervals[s1_index].intersectWith(greaterThan);

						// S2 is less than the biggest instance of S1.
						intervals[s2_index].intersectWith(lessThan);
					}

					break;
				}
				case ICMD_IF_ICMPGT:   // if (S1 > S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						targetIntervals = intervals;

						// Create interval containing all elements greater than the smallest instance of S2.
						Interval greaterThan;
						greaterThan.lower(intervals[s2_index].lower());
						greaterThan.tryRemove(intervals[s2_index].lower());

						// Create interval containing all elements less than the biggest instance of S1.
						Interval lessThan;
						lessThan.upper(intervals[s1_index].upper());
						lessThan.tryRemove(intervals[s1_index].upper());

						// S1 is greater than the smallest instance of S2.
						targetIntervals[s1_index].intersectWith(greaterThan);

						// S2 is less than the biggest instance of S1.
						targetIntervals[s2_index].intersectWith(lessThan);
					}

					// FALSE BRANCH
					{
						// Create interval containing all elements less than or equal the biggest instance of S2.
						Interval lessThan;
						lessThan.upper(intervals[s2_index].upper());

						// Create interval containing all elements greater than or equal the smallest instance of S1.
						Interval greaterThan;
						greaterThan.lower(intervals[s1_index].lower());

						// S1 is less than or equal the biggest instance of S2.
						intervals[s1_index].intersectWith(lessThan);

						// S2 is greater than or equal the smallest instance of S1.
						intervals[s2_index].intersectWith(greaterThan);
					}

					break;
				}
				case ICMD_IF_ICMPGE:   // if (S1 >= S2) goto DST
				{
					node->ld->jumpTarget = instr->dst.block;

					// TRUE BRANCH
					{
						targetIntervals = intervals;

						// Create interval containing all elements greater than or equal the smallest instance of S2.
						Interval greaterThan;
						greaterThan.lower(intervals[s2_index].lower());

						// Create interval containing all elements less than or equal the biggest instance of S1.
						Interval lessThan;
						lessThan.upper(intervals[s1_index].upper());

						// S1 is greater than or equal the smallest instance of S2.
						targetIntervals[s1_index].intersectWith(greaterThan);

						// S2 is less than or equal the biggest instance of S1.
						targetIntervals[s2_index].intersectWith(lessThan);
					}

					// FALSE BRANCH
					{
						// Create interval containing all elements less than the biggest instance of S2.
						Interval lessThan;
						lessThan.upper(intervals[s2_index].upper());
						lessThan.tryRemove(intervals[s2_index].upper());

						// Create interval containing all elements greater than the smallest instance of S1.
						Interval greaterThan;
						greaterThan.lower(intervals[s1_index].lower());
						greaterThan.tryRemove(intervals[s1_index].lower());

						// S1 is less than the biggest instance of S2.
						intervals[s1_index].intersectWith(lessThan);

						// S2 is greater than the smallest instance of S1.
						intervals[s2_index].intersectWith(greaterThan);
					}

					break;
				}

				case ICMD_ALOAD:
					// do nothing
					break;

				default:
					if (instruction_has_dst(instr))
					{
						if (isLocalIntVar(jd, dst_index))   // Integer produced by an unknown instruction.
						{
							intervals[dst_index] = Interval();   // [MIN,MAX]
						}
						else
						{
							// If dst_index is an array, reset all intervals referencing that array.
							for (size_t i = 0; i < intervals.size(); i++)
							{
								if (intervals[i].lower().instruction().kind() == NumericInstruction::ARRAY_LENGTH &&
									intervals[i].lower().instruction().variable() == dst_index)
								{
									intervals[i] = Interval();
								}
								else if (intervals[i].upper().instruction().kind() == NumericInstruction::ARRAY_LENGTH &&
									intervals[i].upper().instruction().variable() == dst_index)
								{
									intervals[i] = Interval();
								}
							}
						}
					}
			}
		}

		/*std::stringstream str;
		str << "# " << node->nr << " #  [F] " << intervals << "| [T] " << targetIntervals;
		log_text(str.str().c_str());*/

		// Compute interval of counter variable.
		if (node->ld->loop && node->ld->loop->hasCounterVariable)
		{
			if (node->ld->jumpTarget)
			{
				// Check if jump target is part of this loop.
				if (node->ld->jumpTarget->ld->loops.find(node->ld->loop) == node->ld->jumpTarget->ld->loops.end())
				{
					// Jump target is _not_ part of this loop.
					node->ld->loop->counterInterval.intersectWith(intervals[node->ld->loop->counterVariable]);
				}
				else
				{
					// Jump target is part of this loop.
					Interval temp = intervals[node->ld->loop->counterVariable];
					temp.unionWith(targetIntervals[node->ld->loop->counterVariable]);
					node->ld->loop->counterInterval.intersectWith(temp);
				}
			}
			else
			{
				node->ld->loop->counterInterval.intersectWith(intervals[node->ld->loop->counterVariable]);
			}
		}

		if (target == node->ld->jumpTarget)
			return targetIntervals;

		return intervals;
	}

	/**
	 * Checks whether the specified variable is local and of type integer.
	 */
	bool isLocalIntVar(jitdata* jd, s4 varIndex)
	{
		varinfo* info = VAR(varIndex);
		return IS_INT_TYPE(info->type) && (var_is_local(jd, varIndex) || var_is_temp(jd, varIndex));
	}
	
	/**
	 * Checks whether the specified variables are local and of type integer.
	 */
	bool isLocalIntVar(jitdata* jd, s4 var0, s4 var1)
	{
		varinfo* info0 = VAR(var0);
		varinfo* info1 = VAR(var1);
		return IS_INT_TYPE(info0->type) && (var_is_local(jd, var0) || var_is_temp(jd, var0))
			&& IS_INT_TYPE(info1->type) && (var_is_local(jd, var1) || var_is_temp(jd, var1));
	}

	/**
	 * Checks whether the specified variable is local.
	 */
	bool isLocalVar(jitdata* jd, s4 varIndex)
	{
		return var_is_local(jd, varIndex) || var_is_temp(jd, varIndex);
	}
}

/**
 * Analyzes all loops and for every loop it finds
 *
 *   -) all written variables,
 *   -) all counter variables.
 */
void analyzeLoops(jitdata* jd)
{
	for (std::vector<LoopContainer*>::iterator it = jd->ld->loops.begin(); it != jd->ld->loops.end(); ++it)
	{
		LoopContainer* loop = *it;

		assert(!loop->footers.empty());

		// For simplicity we consider only one back edge per loop
		basicblock* footer = loop->footers.front();

		// Analyze all blocks contained in this loop.
		analyzeBasicblockInLoop(jd, loop->header, loop);
		for (std::vector<basicblock*>::iterator blockIt = loop->nodes.begin(); blockIt != loop->nodes.end(); ++blockIt)
		{
			if (*blockIt != footer)
				analyzeBasicblockInLoop(jd, *blockIt, loop);
		}
		analyzeFooter(jd, loop);					// Find counter variables.
		analyzeBasicblockInLoop(jd, footer, loop);	// The footer must be the last node to be analyzed.

		// Compute the final set of invariant integer variables.
		// Compute the final set of invariant array variables.
		for (VariableSet::iterator it = loop->writtenVariables.begin(); it != loop->writtenVariables.end(); ++it)
		{
			loop->invariantVariables.remove(*it);
			loop->invariantArrays.remove(*it);
		}
	}
}

/**
 * Marks all basicblocks which are leaves.
 */
void findLeaves(jitdata* jd)
{
	for (std::vector<Edge>::const_iterator it = jd->ld->loopBackEdges.begin(); it != jd->ld->loopBackEdges.end(); ++it)
	{
		it->from->ld->outgoingBackEdgeCount++;
	}

	for (basicblock* b = jd->basicblocks; b != 0; b = b->next)
	{
		assert(b->successorcount >= b->ld->outgoingBackEdgeCount);
		b->ld->leaf = (b->successorcount == b->ld->outgoingBackEdgeCount);
	}
}

void removeFullyRedundantChecks(jitdata* jd)
{
	for (basicblock* block = jd->basicblocks; block != 0; block = block->next)
	{
		if (block->ld->leaf)
			analyze(jd, block, 0);
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

