/* src/vm/jit/loop/duplicate.cpp

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

#include "toolbox/logging.hpp"
#include "vm/jit/ir/icmd.hpp"

#include "duplicate.hpp"
#include "Value.hpp"
#include "ValueMap.hpp"
#include "DynamicVector.hpp"

#include <cstring>

namespace
{
	basicblock* duplicateBasicblock(const basicblock* block);
	basicblock* createBasicblock(jitdata* jd, s4 numInstructions);
	bool checkLoop(jitdata* jd, LoopContainer* loop);
	void duplicateLoop(jitdata* jd, LoopContainer* loop);
	void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch, basicblock* loopTrampoline);
	void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch1, basicblock* loopSwitch2, basicblock* loopTrampoline);
	void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch1, basicblock* loopSwitch2, basicblock* loopSwitch3, basicblock* loopTrampoline);
	void removeChecks(LoopContainer* loop, s4 array, s4 index);
	basicblock* createTrampoline(basicblock* target);
	void redirectJumps(jitdata* jd, basicblock* loopSwitch);
	void optimizeLoop(jitdata* jd, LoopContainer* loop);
	bool isLocalIntVar(jitdata* jd, s4 varIndex);


	//                                                    //
	// Functions that generate intermediate instructions. //
	//                                                    //

	inline void opcode_ALOAD(instruction* instr, s4 src, s4 dst)
	{
		instr->opc = ICMD_ALOAD;
		instr->s1.varindex = src;
		instr->dst.varindex = dst;
	}

	inline void opcode_ILOAD(instruction* instr, s4 src, s4 dst)
	{
		instr->opc = ICMD_ILOAD;
		instr->s1.varindex = src;
		instr->dst.varindex = dst;
	}

	inline void opcode_ARRAYLENGTH(instruction* instr, s4 src, s4 dst)
	{
		instr->opc = ICMD_ARRAYLENGTH;
		instr->s1.varindex = src;
		instr->dst.varindex = dst;
	}

	inline void opcode_IADDCONST(instruction* instr, s4 src, s4 constant, s4 dst)
	{
		instr->opc = ICMD_IADDCONST;
		instr->s1.varindex = src;
		instr->sx.val.i = constant;
		instr->dst.varindex = dst;
	}

	inline void opcode_ISUBCONST(instruction* instr, s4 src, s4 constant, s4 dst)
	{
		instr->opc = ICMD_ISUBCONST;
		instr->s1.varindex = src;
		instr->sx.val.i = constant;
		instr->dst.varindex = dst;
	}

	inline void opcode_GOTO(instruction* instr, basicblock* dst)
	{
		instr->opc = ICMD_GOTO;
		instr->dst.block = dst;
	}

	inline void opcode_IFGT(instruction* instr, s4 src, s4 constant, basicblock* dst)
	{
		instr->opc = ICMD_IFGT;
		instr->s1.varindex = src;
		instr->sx.val.i = constant;
		instr->dst.block = dst;
	}

	inline void opcode_IFLT(instruction* instr, s4 src, s4 constant, basicblock* dst)
	{
		instr->opc = ICMD_IFLT;
		instr->s1.varindex = src;
		instr->sx.val.i = constant;
		instr->dst.block = dst;
	}

	inline void opcode_IFLE(instruction* instr, s4 src, s4 constant, basicblock* dst)
	{
		instr->opc = ICMD_IFLE;
		instr->s1.varindex = src;
		instr->sx.val.i = constant;
		instr->dst.block = dst;
	}

	inline void opcode_IFNULL(instruction* instr, s4 src, basicblock* dst)
	{
		instr->opc = ICMD_IFNULL;
		instr->s1.varindex = src;
		instr->dst.block = dst;
	}

	inline void opcode_IF_ICMPGE(instruction* instr, s4 src1, s4 src2, basicblock* dst)
	{
		instr->opc = ICMD_IF_ICMPGE;
		instr->s1.varindex = src1;
		instr->sx.s23.s2.varindex = src2;
		instr->dst.block = dst;
	}

	////////////////////////////////////////////////////////


	basicblock* duplicateBasicblock(const basicblock* block)
	{
		// flat clone
		basicblock* clone = new basicblock(*block);
		clone->ld = new BasicblockLoopData;

		// clone instructions
		clone->iinstr = new instruction[block->icount];
		memcpy(clone->iinstr, block->iinstr, sizeof(instruction) * block->icount);

		// clone table- and lookup-switches
		for (s4 i = 0; i < block->icount; i++)
		{
			instruction* instr = &block->iinstr[i];
			switch (instr->opc)
			{
				case ICMD_TABLESWITCH:
				{
					// count = (tablehigh - tablelow + 1) + 1 [default branch]
					s4 count = instr->sx.s23.s3.tablehigh - instr->sx.s23.s2.tablelow + 2;

					// clone switch table
					clone->iinstr[i].dst.table = new branch_target_t[count];
					memcpy(clone->iinstr[i].dst.table, instr->dst.table, sizeof(branch_target_t) * count);
					break;
				}
				case ICMD_LOOKUPSWITCH:
				{
					s4 count = instr->sx.s23.s2.lookupcount;

					// clone lookup table
					clone->iinstr[i].dst.lookup = new lookup_target_t[count];
					memcpy(clone->iinstr[i].dst.lookup, instr->dst.lookup, sizeof(lookup_target_t) * count);
					break;
				}
			}
		}

		return clone;
	}

	basicblock* createBasicblock(jitdata* jd, s4 numInstructions)
	{
		basicblock* block = new basicblock;
		memset(block, 0, sizeof(basicblock));
		block->ld     = new BasicblockLoopData;
		block->method = jd->m;
		block->state  = basicblock::FINISHED;
		block->mpc    = -1;
		block->icount = numInstructions;
		block->iinstr = new instruction[numInstructions];
		memset(block->iinstr, 0, sizeof(instruction) * numInstructions);
		return block;
	}

	/**
	 * Creates a basicblock with a single GOTO instruction.
	 */
	basicblock* createTrampoline(jitdata* jd, basicblock* target)
	{
		basicblock* trampoline = createBasicblock(jd, 1);
		opcode_GOTO(trampoline->iinstr, target);
		return trampoline;
	}

	void duplicateLoop(LoopContainer* loop)
	{
		// copy header
		loop->header->ld->copiedTo = duplicateBasicblock(loop->header);

		// copy other basicblocks
		for (std::vector<basicblock*>::iterator it = loop->nodes.begin(); it != loop->nodes.end(); ++it)
		{
			(*it)->ld->copiedTo = duplicateBasicblock(*it);
		}
	}

	/**
	 * Returns true (otherwise false) if the following points hold for the specified loop:
	 *
	 *   -) The loop does not contain a hole.
	 *   -) The first block is the header.
	 *   -) The stack depth before and after the loop is 0.
	 *
	 * The above points simplify the code generation.
	 *
	 * The arguments beforeLoopPtr and lastBlockInLoopPtr will be set
	 * to the basicblock before the loop and to the last basicblock in the loop respectively.
	 */
	bool checkLoop(jitdata* jd, LoopContainer* loop, basicblock** beforeLoopPtr, basicblock** lastBlockInLoopPtr)
	{
		*beforeLoopPtr = 0;
		*lastBlockInLoopPtr = 0;

		// Mark every basicblock which is part of the loop
		loop->header->ld->belongingTo = loop;
		for (std::vector<basicblock*>::iterator it = loop->nodes.begin(); it != loop->nodes.end(); ++it)
		{
			(*it)->ld->belongingTo = loop;
		}

		
		basicblock* lastBlock = 0;
		bool loopFound = false;

		// Analyze marked basicblocks.
		for (basicblock* block = jd->basicblocks; block; block = block->next)
		{
			if (block->ld->belongingTo == loop)
			{
				// Are we entering the loop?
				if (lastBlock == 0 || lastBlock->ld->belongingTo != loop)   // lastBlock is not part of the loop
				{
					// The loop contains a hole.
					if (loopFound)
					{
						//log_text("checkLoop: hole");
						return false;
					}

					// The first block is not the header.
					if (block->ld->loop == 0)
					{
						//log_text("checkLoop: header");
						return false;
					}

					loopFound = true;
					*beforeLoopPtr = lastBlock;
				}

				*lastBlockInLoopPtr = block;
			}
		
			lastBlock = block;
		}

		// Check stack depth.
		if (loop->header->indepth != 0 || (*lastBlockInLoopPtr)->outdepth != 0)
		{
			//log_text("checkLoop: stack depth");
			return false;
		}

		return true;
	}

	inline void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch, basicblock* loopTrampoline)
	{
		buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch, 0, 0, loopTrampoline);
	}

	inline void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch1, basicblock* loopSwitch2, basicblock* loopTrampoline)
	{
		buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch1, loopSwitch2, 0, loopTrampoline);
	}

	/**
	 * Inserts the copied loop, the loop switches and the trampoline into the global basicblock list.
	 * It also inserts these nodes into the predecessor loops.
	 *
	 * loop: The original loop that has been duplicated.
	 * loopSwitch2: Will be inserted after the first loop switch if it is != 0.
	 * loopSwitch3: Will be inserted after the first loop switch if it is != 0.
	 */
	void buildBasicblockList(jitdata* jd, LoopContainer* loop, basicblock* beforeLoop, basicblock* lastBlockInLoop, basicblock* loopSwitch1, basicblock* loopSwitch2, basicblock* loopSwitch3, basicblock* loopTrampoline)
	{
		assert(loopSwitch1);

		// insert first loop switch
		if (beforeLoop)
		{
			loopSwitch1->next = beforeLoop->next;
			beforeLoop->next = loopSwitch1;
		}
		else
		{
			loopSwitch1->next = jd->basicblocks;
			jd->basicblocks = loopSwitch1;
		}

		basicblock* lastLoopSwitch = loopSwitch1;

		// insert second loop switch
		if (loopSwitch2)
		{
			loopSwitch2->next = lastLoopSwitch->next;
			lastLoopSwitch->next = loopSwitch2;

			lastLoopSwitch = loopSwitch2;
		}

		// insert third loop switch
		if (loopSwitch3)
		{
			loopSwitch3->next = lastLoopSwitch->next;
			lastLoopSwitch->next = loopSwitch3;

			lastLoopSwitch = loopSwitch3;
		}

		// insert trampoline after loop
		lastBlockInLoop->next = loopTrampoline;

		// The basicblocks contained in the following container will be inserted into the predecessor loops.
		std::vector<basicblock*> duplicates;
		duplicates.reserve(loop->nodes.size() + 1);

		// insert copied loop after trampoline
		basicblock* end = loopTrampoline;
		for (basicblock* block = lastLoopSwitch->next; block != loopTrampoline; block = block->next)
		{
			end->next = block->ld->copiedTo;
			end = block->ld->copiedTo;

			// The copied basicblock must be inserted into the predecessor loops.
			duplicates.push_back(block->ld->copiedTo);

			// prepare for next loop duplication
			block->ld->copiedTo = 0;
		}

		// end->next already points to the basicblock after the second loop.

		// Insert nodes into predecessor loops except the root loop.
		for (LoopContainer* pred = loop->parent; pred->parent; pred = pred->parent)
		{
			for (std::vector<basicblock*>::iterator it = duplicates.begin(); it != duplicates.end(); ++it)
			{
				pred->nodes.push_back(*it);
			}
			pred->nodes.push_back(loopTrampoline);
			pred->nodes.push_back(loopSwitch1);
			if (loopSwitch2)
				pred->nodes.push_back(loopSwitch2);
			if (loopSwitch3)
				pred->nodes.push_back(loopSwitch3);
		}
	}

	void removeChecks(LoopContainer* loop, s4 array, s4 index)
	{
		for (std::vector<basicblock*>::iterator it = loop->nodes.begin(); it != loop->nodes.end(); ++it)
		{
			for (instruction* instr = (*it)->iinstr; instr != (*it)->iinstr + (*it)->icount; instr++)
			{
				switch (instr->opc)
				{
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

						if (array == instr->s1.varindex && index == instr->sx.s23.s2.varindex)
							instr->flags.bits &= ~INS_FLAG_CHECK;
						break;
				}
			}
		}
	}

	/**
	 * Redirects all jumps that go to the old loop:
	 * -) A jump from outside of the loop will be redirected to the loop switch.
	 * -) A jump from the duplicate will be redirected to the duplicate.
	 *
	 * This function must be called before the duplicate is inserted into the linked list.
	 */
	void redirectJumps(jitdata* jd, basicblock* loopSwitch)
	{
		// All jumps that go from the outside of the loop to the loop header must be redirected to the loop switch.
		for (basicblock* block = jd->basicblocks; block; block = block->next)
		{
			// Check if this block is not part of the original loop.
			if (!block->ld->copiedTo)
			{
				for (instruction* instr = block->iinstr; instr != block->iinstr + block->icount; instr++)
				{
					// If instr is a jump into the original loop, redirect it to the loop switch.
					switch (icmd_table[instr->opc].controlflow)
					{
						case CF_IF:
						case CF_GOTO:
						case CF_RET:
							if (instr->dst.block->ld->copiedTo)
								instr->dst.block = loopSwitch;
							break;
						case CF_JSR:
							if (instr->sx.s23.s3.jsrtarget.block->ld->copiedTo)
								instr->sx.s23.s3.jsrtarget.block = loopSwitch;
							break;
						case CF_TABLE:
						{
							// count = (tablehigh - tablelow + 1) + 1 [default branch]
							s4 count = instr->sx.s23.s3.tablehigh - instr->sx.s23.s2.tablelow + 2;

							branch_target_t* target = instr->dst.table;
							while (--count >= 0)
							{
								if (target->block->ld->copiedTo)
									target->block = loopSwitch;
								target++;
							}
							break;
						}
						case CF_LOOKUP:
						{
							// default target
							if (instr->sx.s23.s3.lookupdefault.block->ld->copiedTo)
								instr->sx.s23.s3.lookupdefault.block = loopSwitch;

							// other targets
							lookup_target_t* entry = instr->dst.lookup;
							s4 count = instr->sx.s23.s2.lookupcount;
							while (--count >= 0)
							{
								if (entry->target.block->ld->copiedTo)
									entry->target.block = loopSwitch;
								entry++;
							}
							break;
						}
						case CF_END:
						case CF_NORMAL:
							// nothing
							break;
					}
				}
			}
			else   // This block is part of the original loop.
			{
				// Redirect jumps that go from the duplicate to the original loop.
				basicblock* dupBlock = block->ld->copiedTo;
				for (instruction* instr = dupBlock->iinstr; instr != dupBlock->iinstr + dupBlock->icount; instr++)
				{
					// If instr is a jump into the original loop, redirect it to the duplicated block.
					switch (icmd_table[instr->opc].controlflow)
					{
						case CF_IF:
						case CF_GOTO:
						case CF_RET:
							if (instr->dst.block->ld->copiedTo)
								instr->dst.block = instr->dst.block->ld->copiedTo;
							break;
						case CF_JSR:
							if (instr->sx.s23.s3.jsrtarget.block->ld->copiedTo)
								instr->sx.s23.s3.jsrtarget.block = instr->sx.s23.s3.jsrtarget.block->ld->copiedTo;
							break;
						case CF_TABLE:
						{
							// count = (tablehigh - tablelow + 1) + 1 [default branch]
							s4 count = instr->sx.s23.s3.tablehigh - instr->sx.s23.s2.tablelow + 2;

							branch_target_t* target = instr->dst.table;
							while (--count >= 0)
							{
								if (target->block->ld->copiedTo)
									target->block = target->block->ld->copiedTo;
								target++;
							}
							break;
						}
						case CF_LOOKUP:
						{
							// default target
							if (instr->sx.s23.s3.lookupdefault.block->ld->copiedTo)
								instr->sx.s23.s3.lookupdefault.block = instr->sx.s23.s3.lookupdefault.block->ld->copiedTo;

							// other targets
							lookup_target_t* entry = instr->dst.lookup;
							s4 count = instr->sx.s23.s2.lookupcount;
							while (--count >= 0)
							{
								if (entry->target.block->ld->copiedTo)
									entry->target.block = entry->target.block->ld->copiedTo;
								entry++;
							}
							break;
						}
						case CF_END:
						case CF_NORMAL:
							// nothing
							break;
					}
				}
			}
		}
	}

	void optimizeLoop(jitdata* jd, LoopContainer* loop)
	{
		// Optimize inner loops.
		for (std::vector<LoopContainer*>::iterator it = loop->children.begin(); it != loop->children.end(); ++it)
		{
			optimizeLoop(jd, *it);
		}

		// Currently only loops with one footer are optimized.
		if (loop->footers.size() == 1 && loop->hasCounterVariable)
		{
			if (loop->counterIncrement >= 0 && loop->counterInterval.lower().lower() >= 0)
			{
				// counterInterval: [L, ?], L >= 0
				
				if (loop->counterInterval.upper().instruction().kind() == NumericInstruction::ARRAY_LENGTH &&
					loop->counterInterval.upper().constant() < 0)
				{
					// counterInterval: [L, array.length - c], L >= 0, c > 0

					//log_text("optimizeLoop: [non-negative, arraylength], inc");

					s4 array = loop->counterInterval.upper().instruction().variable();

					// The array variable must be invariant.
					if (!loop->invariantArrays.contains(array))
					{
						//log_println("optimizeLoop: %d not invariant", array);
						return;
					}

					basicblock *beforeLoop, *lastBlockInLoop;
					if (!checkLoop(jd, loop, &beforeLoop, &lastBlockInLoop))
						return;

					duplicateLoop(loop);

					// remove checks in original loop
					removeChecks(loop, array, loop->counterVariable);

					// create basicblocks that jump to the right loop
					basicblock* loopSwitch1 = createBasicblock(jd, 2);
					basicblock* loopSwitch2 = createBasicblock(jd, 4);
					instruction* instr;

					// Fill instruction array of first loop switch with:
					// if (array == null) goto unoptimized_loop

					instr = loopSwitch1->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_IFNULL(instr++, array, loop->header->ld->copiedTo);

					assert(instr - loopSwitch1->iinstr == loopSwitch1->icount);

					// Fill instruction array of second loop switch with:
					// if (array.length + upper_constant > MAX - increment) goto unoptimized_loop

					instr = loopSwitch2->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_ARRAYLENGTH(instr++, array, jd->ld->freeVariable);
					opcode_IADDCONST(instr++, jd->ld->freeVariable, loop->counterInterval.upper().constant(), jd->ld->freeVariable);
					opcode_IFGT(instr++, jd->ld->freeVariable, Scalar::max() - loop->counterIncrement, loop->header->ld->copiedTo);

					assert(instr - loopSwitch2->iinstr == loopSwitch2->icount);

					// create basicblock that jumps over the second loop
					basicblock* loopTrampoline = createTrampoline(jd, lastBlockInLoop->next);

					// Insert loop into basicblock list.
					redirectJumps(jd, loopSwitch1);
					buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch1, loopSwitch2, loopTrampoline);

					// Adjust statistical data.
					jd->basicblockcount += loop->nodes.size() + 4;
				}
				else if (loop->counterInterval.upper().instruction().kind() == NumericInstruction::VARIABLE &&
					loop->counterInterval.upper().constant() == 0)
				{
					// counterInterval: [L, x], L >= 0

					//log_text("optimizeLoop: [non-negative, invariant], inc");

					if (loop->invariantArrays.begin() == loop->invariantArrays.end())
					{
						//log_text("optimizeLoop: no invariant array");
						return;
					}
					
					// possible optimization: why take an arbitrary array variable?
					s4 array = *loop->invariantArrays.begin();

					basicblock *beforeLoop, *lastBlockInLoop;
					if (!checkLoop(jd, loop, &beforeLoop, &lastBlockInLoop))
						return;

					duplicateLoop(loop);

					// remove checks in original loop
					removeChecks(loop, array, loop->counterVariable);

					s4 invariantVariable = loop->counterInterval.upper().instruction().variable();

					// create loop switches that jump to the right loop
					basicblock* loopSwitch1 = createBasicblock(jd, 2);
					basicblock* loopSwitch2 = createBasicblock(jd, 2);
					basicblock* loopSwitch3 = createBasicblock(jd, 4);
					instruction* instr;

					// Fill instruction array of first loop switch with:
					// if (invariant > MAX - increment) goto unoptimized_loop

					instr = loopSwitch1->iinstr;

					opcode_ILOAD(instr++, invariantVariable, invariantVariable);
					opcode_IFGT(instr++, invariantVariable, Scalar::max() - loop->counterIncrement, loop->header->ld->copiedTo);

					assert(instr - loopSwitch1->iinstr == loopSwitch1->icount);

					// Fill instruction array of second loop switch with:
					// if (array == null) goto unoptimized_loop

					instr = loopSwitch2->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_IFNULL(instr++, array, loop->header->ld->copiedTo);

					assert(instr - loopSwitch2->iinstr == loopSwitch2->icount);

					// Fill instruction array of third loop switch with:
					// if (invariant >= array.length) goto unoptimized_loop

					instr = loopSwitch3->iinstr;

					opcode_ILOAD(instr++, invariantVariable, invariantVariable);
					opcode_ALOAD(instr++, array, array);
					opcode_ARRAYLENGTH(instr++, array, jd->ld->freeVariable);
					opcode_IF_ICMPGE(instr++, invariantVariable, jd->ld->freeVariable, loop->header->ld->copiedTo);

					assert(instr - loopSwitch3->iinstr == loopSwitch3->icount);

					// create basicblock that jumps over the second loop
					basicblock* loopTrampoline = createTrampoline(jd, lastBlockInLoop->next);

					// Insert loop into basicblock list.
					redirectJumps(jd, loopSwitch1);
					buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch1, loopSwitch2, loopSwitch3, loopTrampoline);

					// Adjust statistical data.
					jd->basicblockcount += loop->nodes.size() + 5;
				}
				else if (loop->counterInterval.upper().instruction().kind() == NumericInstruction::ZERO &&
					loop->counterInterval.upper().constant() <= Scalar::max() - loop->counterIncrement)
				{
					// counterInterval: [L, c], L >= 0, c <= MAX - increment

					//log_text("optimizeLoop: [non-negative, constant], inc");

					if (loop->invariantArrays.begin() == loop->invariantArrays.end())
					{
						//log_text("optimizeLoop: no invariant array");
						return;
					}
					
					// possible optimization: why take an arbitrary array variable?
					s4 array = *loop->invariantArrays.begin();

					basicblock *beforeLoop, *lastBlockInLoop;
					if (!checkLoop(jd, loop, &beforeLoop, &lastBlockInLoop))
						return;

					duplicateLoop(loop);

					// remove checks in original loop
					removeChecks(loop, array, loop->counterVariable);

					// create basicblock that jumps to the right loop
					basicblock* loopSwitch1 = createBasicblock(jd, 2);
					basicblock* loopSwitch2 = createBasicblock(jd, 3);
					instruction* instr;

					// Fill instruction array of first loop switch with:
					// if (array == null) goto unoptimized_loop

					instr = loopSwitch1->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_IFNULL(instr++, array, loop->header->ld->copiedTo);

					assert(instr - loopSwitch1->iinstr == loopSwitch1->icount);

					// Fill instruction array of second loop switch with:
					// if (array.length <= upper_constant) goto unoptimized_loop

					instr = loopSwitch2->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_ARRAYLENGTH(instr++, array, jd->ld->freeVariable);
					opcode_IFLE(instr++, jd->ld->freeVariable, loop->counterInterval.upper().constant(), loop->header->ld->copiedTo);

					assert(instr - loopSwitch2->iinstr == loopSwitch2->icount);

					// create basicblock that jumps over the second loop
					basicblock* loopTrampoline = createTrampoline(jd, lastBlockInLoop->next);

					// Insert loop into basicblock list.
					redirectJumps(jd, loopSwitch1);
					buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch1, loopSwitch2, loopTrampoline);

					// Adjust statistical data.
					jd->basicblockcount += loop->nodes.size() + 4;
				}
			}
			else if (loop->counterIncrement < 0 &&
				loop->counterInterval.upper().instruction().kind() == NumericInstruction::ARRAY_LENGTH &&
				loop->counterInterval.upper().constant() < 0)
			{
				// counterInterval: [?, U], U < array.length

				if (loop->counterInterval.lower().instruction().kind() == NumericInstruction::ZERO &&
					loop->counterInterval.lower().constant() >= 0)
				{
					// counterInterval: [c, U], c >= 0, U < array.length

					//log_text("optimizeLoop: [non-negative, arraylength], dec");

					s4 array = loop->counterInterval.upper().instruction().variable();

					// The array variable must be invariant.
					if (!loop->invariantArrays.contains(array))
					{
						//log_println("optimizeLoop: %d not invariant", array);
						return;
					}

					// No runtime check and so no loop duplication is necessary.

					// remove checks in loop
					removeChecks(loop, array, loop->counterVariable);
				}
				else if (loop->counterInterval.lower().instruction().kind() == NumericInstruction::VARIABLE &&
					loop->counterInterval.lower().constant() == 0)
				{
					// counterInterval: [x, U], U < array.length

					//log_text("optimizeLoop: [invariant, arraylength], dec");

					s4 array = loop->counterInterval.upper().instruction().variable();

					// The array variable must be invariant.
					if (!loop->invariantArrays.contains(array))
					{
						//log_println("optimizeLoop: %d not invariant", array);
						return;
					}

					basicblock *beforeLoop, *lastBlockInLoop;
					if (!checkLoop(jd, loop, &beforeLoop, &lastBlockInLoop))
						return;

					duplicateLoop(loop);

					// remove checks in original loop
					removeChecks(loop, array, loop->counterVariable);

					s4 invariantVariable = loop->counterInterval.lower().instruction().variable();

					// create basicblock that jumps to the right loop
					basicblock* loopSwitch = createBasicblock(jd, 2);

					// Fill instruction array of loop switch with:
					// if (invariant < 0) goto unoptimized_loop

					instruction* instr = loopSwitch->iinstr;

					opcode_ILOAD(instr++, invariantVariable, invariantVariable);
					opcode_IFLT(instr++, invariantVariable, 0, loop->header->ld->copiedTo);

					assert(instr - loopSwitch->iinstr == loopSwitch->icount);

					// create basicblock that jumps over the second loop
					basicblock* loopTrampoline = createTrampoline(jd, lastBlockInLoop->next);

					// Insert loop into basicblock list.
					redirectJumps(jd, loopSwitch);
					buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch, loopTrampoline);

					// Adjust statistical data.
					jd->basicblockcount += loop->nodes.size() + 3;
				}
				else if (loop->counterInterval.lower().instruction().kind() == NumericInstruction::ARRAY_LENGTH &&
					loop->counterInterval.lower().constant() < 0 &&
					loop->counterInterval.lower().constant() > Scalar::min())   // prevent overflow during negation
				{
					// counterInterval: [array.length + c, U], MIN < c < 0, U < array.length

					//log_text("optimizeLoop: [arraylength, arraylength], dec");

					s4 array = loop->counterInterval.upper().instruction().variable();
					
					// Both array variables must be the same.
					if (array != loop->counterInterval.lower().instruction().variable())
					{
						//log_text("optimizeLoop: arrays are different");
						return;
					}

					// The array variable must be invariant.
					if (!loop->invariantArrays.contains(array))
					{
						//log_println("optimizeLoop: %d not invariant", array);
						return;
					}

					basicblock *beforeLoop, *lastBlockInLoop;
					if (!checkLoop(jd, loop, &beforeLoop, &lastBlockInLoop))
						return;

					duplicateLoop(loop);

					// remove checks in original loop
					removeChecks(loop, array, loop->counterVariable);

					// create basicblock that jumps to the right loop
					basicblock* loopSwitch = createBasicblock(jd, 3);

					// Fill instruction array of loop switch with:
					// if (array.length < -1 * lower_constant) goto unoptimized_loop

					instruction* instr = loopSwitch->iinstr;

					opcode_ALOAD(instr++, array, array);
					opcode_ARRAYLENGTH(instr++, array, jd->ld->freeVariable);
					opcode_IFLT(instr++, jd->ld->freeVariable, -loop->counterInterval.lower().constant(), loop->header->ld->copiedTo);

					assert(instr - loopSwitch->iinstr == loopSwitch->icount);

					// create basicblock that jumps over the second loop
					basicblock* loopTrampoline = createTrampoline(jd, lastBlockInLoop->next);

					// Insert loop into basicblock list.
					redirectJumps(jd, loopSwitch);
					buildBasicblockList(jd, loop, beforeLoop, lastBlockInLoop, loopSwitch, loopTrampoline);

					// Adjust statistical data.
					jd->basicblockcount += loop->nodes.size() + 3;
				}
			}
		}
	}

	/**
	 * Checks whether the specified variable is local and of type integer.
	 */
	bool isLocalIntVar(jitdata* jd, s4 varIndex)
	{
		varinfo* info = VAR(varIndex);
		return IS_INT_TYPE(info->type) && (var_is_local(jd, varIndex) || var_is_temp(jd, varIndex));
	}
}

bool findFreeVariable(jitdata* jd)
{
	// Reserve temporary variable.
	if (jd->vartop < jd->varcount)
	{
		jd->ld->freeVariable = jd->vartop++;

		varinfo* v = VAR(jd->ld->freeVariable);
		v->type = TYPE_INT;

		return true;
	}

	return false;
}

void removePartiallyRedundantChecks(jitdata* jd)
{
	for (std::vector<LoopContainer*>::iterator it = jd->ld->rootLoop->children.begin(); it != jd->ld->rootLoop->children.end(); ++it)
	{
		optimizeLoop(jd, *it);
	}
}

void groupArrayBoundsChecks(jitdata* jd)
{
	bool optimizationDone = false;
	basicblock* lastBlock = 0;

	// Optimize every basicblock separately.
	for (basicblock* block = jd->basicblocks; block; lastBlock = block, block = block->next)
	{
		// makes block duplication simpler
		if (block->indepth != 0 || block->outdepth != 0)
			continue;
		
		// A variable v is mapped to the value w+c, where w represents the value the variable w has at the beginning of the block.
		// At the beginning of a basicblock every variable v has the value v+0.
		ValueMap values;

		// first index: array variable
		// second index: index variable
		DynamicVector<DynamicVector<s4> > smallestConstants;
		DynamicVector<DynamicVector<s4> > biggestConstants;
		DynamicVector<DynamicVector<s4> > accessCounts;

		std::vector<s4> instructionArrayMap;   // Maps each instruction index to the used array variable.
		std::vector<s4> instructionIndexMap;   // Maps each instruction index to the used index variable.

		instructionArrayMap.resize(block->icount);
		instructionIndexMap.resize(block->icount);

		for (s4 i = 0; i < block->icount; i++)
		{
			instruction* instr = &block->iinstr[i];

			switch (instr->opc)
			{
				case ICMD_COPY:
				case ICMD_MOVE:
				case ICMD_ILOAD:
				case ICMD_ISTORE:
					if (isLocalIntVar(jd, instr->dst.varindex))
						values[instr->dst.varindex] = values[instr->s1.varindex];
					else
						values[instr->dst.varindex] = Value::newUnknown();   // dst can be an array
					break;

				case ICMD_IINC:
				case ICMD_IADDCONST:
					if (isLocalIntVar(jd, instr->dst.varindex))
					{
						values[instr->dst.varindex] = values[instr->s1.varindex];
						values[instr->dst.varindex].addConstant(instr->sx.val.i);
					}
					break;

				case ICMD_ISUBCONST:
					if (isLocalIntVar(jd, instr->dst.varindex))
					{
						values[instr->dst.varindex] = values[instr->s1.varindex];
						values[instr->dst.varindex].subtractConstant(instr->sx.val.i);
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
				{
					// the value of the index variable
					const Value& value = values[instr->sx.s23.s2.varindex];
					s4 array = instr->s1.varindex;

					if (!value.isUnknown() &&
						(instr->flags.bits & INS_FLAG_CHECK))   // ABC has not been removed yet.
					{
						if (accessCounts[array][value.variable()] == 0)
						{
							smallestConstants[array][value.variable()] = value.constant();
							biggestConstants[array][value.variable()] = value.constant();
						}
						else
						{
							// update smallest constant
							if (value.constant() < smallestConstants[array][value.variable()])
								smallestConstants[array][value.variable()] = value.constant();

							// update biggest constant
							if (value.constant() > biggestConstants[array][value.variable()])
								biggestConstants[array][value.variable()] = value.constant();
						}

						++accessCounts[array][value.variable()];

						instructionArrayMap[i] = array;
						instructionIndexMap[i] = value.variable();
					}
					break;
				}
				default:
					if (instruction_has_dst(instr))
					{
						values[instr->dst.varindex] = Value::newUnknown();
					}
			}
		}

		// find most used array/index combination
		s4 bestCount = 0;
		s4 bestArray = 0;
		s4 bestIndex = 0;
		for (size_t array = 0; array < accessCounts.size(); array++)
		{
			if (!values[array].isUnknown())   // Is array invariant?
			{
				DynamicVector<s4> row = accessCounts[array];
				for (size_t index = 0; index < row.size(); index++)
				{
					s4 count = row[index];
					if (count > bestCount)
					{
						bestCount = count;
						bestArray = array;
						bestIndex = index;
					}
				}
			}
		}

		s4 smallestConstant = smallestConstants[bestArray][bestIndex];
		s4 biggestConstant = biggestConstants[bestArray][bestIndex];

		if (bestCount > 3 &&
			smallestConstant > std::numeric_limits<s4>::min() &&	// prevent overflow
			biggestConstant >= 0)									// prevent overflow
		{
			basicblock* safeBlock = duplicateBasicblock(block);

			// Remove checks in block.
			for (s4 i = 0; i < block->icount; i++)
			{
				if (instructionArrayMap[i] == bestArray && instructionIndexMap[i] == bestIndex)
				{
					block->iinstr[i].flags.bits &= ~INS_FLAG_CHECK;
				}
			}

			instruction* instr;

			// Create first check: if (bestIndex < -1 * smallestConstant) goto safeBlock
			basicblock* lowerBoundsCheck = createBasicblock(jd, 2);
			instr = lowerBoundsCheck->iinstr;
			opcode_ILOAD(instr++, bestIndex, bestIndex);
			opcode_IFLT(instr++, bestIndex, -smallestConstant, safeBlock);
			assert(instr - lowerBoundsCheck->iinstr == lowerBoundsCheck->icount);

			// Create second check: if (bestArray == null) goto safeBlock
			basicblock* nullCheck = createBasicblock(jd, 2);
			instr = nullCheck->iinstr;
			opcode_ALOAD(instr++, bestArray, bestArray);
			opcode_IFNULL(instr++, bestArray, safeBlock);
			assert(instr - nullCheck->iinstr == nullCheck->icount);

			// Create third check: if (bestIndex >= bestArray.length - biggestConstant) goto safeBlock
			basicblock* upperBoundsCheck = createBasicblock(jd, 5);
			instr = upperBoundsCheck->iinstr;
			opcode_ILOAD(instr++, bestIndex, bestIndex);
			opcode_ALOAD(instr++, bestArray, bestArray);
			opcode_ARRAYLENGTH(instr++, bestArray, jd->ld->freeVariable);
			opcode_ISUBCONST(instr++, jd->ld->freeVariable, biggestConstant, jd->ld->freeVariable);
			opcode_IF_ICMPGE(instr++, bestIndex, jd->ld->freeVariable, safeBlock);
			assert(instr - upperBoundsCheck->iinstr == upperBoundsCheck->icount);

			// Create GOTO-Block between optimized and unoptimized basicblock.
			basicblock* trampoline = createTrampoline(jd, block->next);

			// Insert all blocks into the linked list.
			safeBlock->next = block->next;
			trampoline->next = safeBlock;
			block->next = trampoline;
			upperBoundsCheck->next = block;
			nullCheck->next = upperBoundsCheck;
			lowerBoundsCheck->next = nullCheck;
			if (lastBlock)
				lastBlock->next = lowerBoundsCheck;
			else
				jd->basicblocks = lowerBoundsCheck;

			// Store a pointer to the first check. Used for jump redirection.
			block->ld->arrayIndexCheck = lowerBoundsCheck;
			optimizationDone = true;

			// Adjust statistical data.
			jd->basicblockcount += 5;

			// Set iteration variable to the correct value.
			block = safeBlock;
		}
	}

	if (optimizationDone)
	{
		// Redirect jumps that go to a duplicated basicblock to lowerBoundsCheck.
		for (basicblock* block = jd->basicblocks; block; block = block->next)
		{
			for (instruction* instr = block->iinstr; instr != block->iinstr + block->icount; instr++)
			{
				switch (icmd_table[instr->opc].controlflow)
				{
					case CF_IF:
					case CF_GOTO:
					case CF_RET:
						if (instr->dst.block->ld->arrayIndexCheck)
							instr->dst.block = instr->dst.block->ld->arrayIndexCheck;
						break;
					case CF_JSR:
						if (instr->sx.s23.s3.jsrtarget.block->ld->arrayIndexCheck)
							instr->sx.s23.s3.jsrtarget.block = instr->sx.s23.s3.jsrtarget.block->ld->arrayIndexCheck;
						break;
					case CF_TABLE:
					{
						// count = (tablehigh - tablelow + 1) + 1 [default branch]
						s4 count = instr->sx.s23.s3.tablehigh - instr->sx.s23.s2.tablelow + 2;

						branch_target_t* target = instr->dst.table;
						while (--count >= 0)
						{
							if (target->block->ld->arrayIndexCheck)
								target->block = target->block->ld->arrayIndexCheck;
							target++;
						}
						break;
					}
					case CF_LOOKUP:
					{
						// default target
						if (instr->sx.s23.s3.lookupdefault.block->ld->arrayIndexCheck)
							instr->sx.s23.s3.lookupdefault.block = instr->sx.s23.s3.lookupdefault.block->ld->arrayIndexCheck;

						// other targets
						lookup_target_t* entry = instr->dst.lookup;
						s4 count = instr->sx.s23.s2.lookupcount;
						while (--count >= 0)
						{
							if (entry->target.block->ld->arrayIndexCheck)
								entry->target.block = entry->target.block->ld->arrayIndexCheck;
							entry++;
						}
						break;
					}
					case CF_END:
					case CF_NORMAL:
						// nothing
						break;
				}
			}
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

