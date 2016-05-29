/*
 * Copyright (C) 2007, 2008, 2009, 2011, 2013
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Must be included first to get configure options */
#include "jam.h"

#ifdef INLINING
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hash.h"
#include "class.h"
#include "classlib.h"
#include "inlining.h"

/* To do inlining, we must know which handlers are relocatable.  This
   can be calculated either at runtime or at compile-time as part of
   the build.  Doing it at compile-time saves having a second copy of
   the interpreter and the runtime checks, reducing executable size by
   approx 20-30%.  However, it cannot be done at compile-time when
   cross-compiling (at least without extra effort).
*/
#ifdef RUNTIME_RELOC_CHECKS
static int handler_sizes[HANDLERS][LABELS_SIZE];
static int goto_len;
#else
#include "relocatability.inc"
#endif

#ifdef TRACEINLINING
#define TRACE(fmt, ...) jam_printf(fmt, ## __VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#define HASHTABSZE 1<<10
#define HASH(ptr) codeHash((unsigned char*)(ptr + 1), ptr->code_len)
#define COMPARE(ptr1, ptr2, hash1, hash2) ((ptr2 != DELETED) && \
         (hash1 == hash2) && codeComp((char *)(ptr1 + 1), ptr1->code_len, ptr2))
#define FOUND(ptr1, ptr2) foundExistingBlock(ptr1, ptr2)
#define PREPARE(ptr) newCodeBlock(ptr)
#define SCAVENGE(ptr) ptr == DELETED
#define DELETED ((void*)-1)

/* Global lock protecting handler rewriting */
static VMLock rewrite_lock;

#define CODE_INCREMENT 128*KB
#define ALIGN(size) ROUND(size, sizeof(void*))
#define ROUND(size, round) (size + round - 1) / round * round

typedef struct code_block_header {
    int len;
    int code_len;
    union {
        int ref_count;
        struct code_block_header *next;
    } u;
} CodeBlockHeader;

typedef struct test_code_block {
    int code_len;
    BasicBlock *patchers;
} TestCodeBlock;

static HashTable code_hash_table;

/* Init options */
static int branch_patching_dup;
static int branch_patching;
static int replication_threshold;
static int profile_threshold;
static int print_codestats;
static int profiling;

/* Total code memory allocated and
   the amount currently used */    
static int codemem = 0;
static int used_codemem = 0;

static int sys_page_size;
static int codemem_increment;
static unsigned int max_codemem;

/* Free list of code blocks that have been freed */
static CodeBlockHeader *code_free_list = NULL;

static char *min_entry_point = (char*)-1;
static char *max_entry_point  = NULL;

static int enabled;
int inlining_inited = FALSE;

static char *goto_start;
static char **handler_entry_points[HANDLERS];
static int branch_patch_offsets[HANDLERS][BRANCH_NUM];

char *reason(int reason) {
    switch(reason) {
        case MEMCMP_FAILED:
            return "memory compare failed";

        case END_REORDERED:
            return "end label re-ordered";

        case END_BEFORE_ENTRY:
            return "end label before entry label";
    }
    return "unknown reason";
}

void showRelocatability() {
    int i;

#ifdef RUNTIME_RELOC_CHECKS
    goto_len = calculateRelocatability(handler_sizes);
#endif

    if(goto_len >= 0)
        printf("Dispatch sequence is relocatable\n");
    else
        printf("Dispatch sequence is not relocatable (%s)\n", reason(goto_len));

    for(i = 0; i < HANDLERS; i++) {
        int j;

        printf("Opcodes at depth %d: \n", i);

        for(j = 0; j < LABELS_SIZE; j++) {
            int size = handler_sizes[i][j];

            if(size >= 0)
                printf("%d : is relocatable\n", j);
            else
                printf("%d : is not relocatable (%s)\n", j, reason(size));
        }
    }
}

int checkRelocatability() {
    char ***handlers = (char ***)executeJava();
    int i;

#ifdef RUNTIME_RELOC_CHECKS
    goto_len = calculateRelocatability(handler_sizes);
#endif

    /* Check relocatability of the indirect goto.  This is copied onto the
       end of each super-instruction.  If this is un-relocatable, inlining
       is disabled. */

    if(goto_len < 0)
        return FALSE;

    goto_start = handlers[ENTRY_LABELS][GOTO_START];

    /* Calculate handler code range within the program text.
       This is used to tell which handlers in a method have
       been rewritten when freeing the method data on class
       unloading */

    for(i = 0; i < HANDLERS; i++) {
        int j;

        for(j = 0; j < LABELS_SIZE; j++) {
            char *entry = handlers[ENTRY_LABELS+i][j];

            if(entry < min_entry_point)
                min_entry_point = entry;

            if(entry > max_entry_point)
                max_entry_point = entry;
        }

        handler_entry_points[i] = handlers[ENTRY_LABELS+i];
    }

    for(i = 0; i < HANDLERS; i++) {
        int j;

        for(j = 0; j < BRANCH_NUM; j++)
            branch_patch_offsets[i][j] = handlers[BRANCH_LABELS+i][j] -
                                  handler_entry_points[i][j + OPC_IFEQ];
    }

    return TRUE;
}

int initialiseInlining(InitArgs *args) {
    enabled = args->codemem > 0 ? checkRelocatability() : FALSE;

    if(enabled) {
        initVMLock(rewrite_lock);
        initHashTable(code_hash_table, HASHTABSZE, TRUE);

        sys_page_size = getpagesize();
        max_codemem = ROUND(args->codemem, sys_page_size);
        codemem_increment = ROUND(CODE_INCREMENT, sys_page_size);

        branch_patching_dup = args->branch_patching_dup;
        branch_patching = args->branch_patching;

        replication_threshold = args->replication_threshold;
        profile_threshold = args->profile_threshold;

        print_codestats = args->print_codestats;
        profiling = args->profiling;
    }

    inlining_inited = TRUE;
    return enabled;
}

void shutdownInlining() {
    if(print_codestats) {
        jam_printf("Allocated codemem: %d\n", codemem);
        jam_printf("Used codemem: %d\n", used_codemem);
    }
}

int codeHash(unsigned char *pntr, int len) {
    int hash = 0;

    for(; len > 0; len--)
        hash = hash * 37 + *pntr++;

    return hash;
}

int codeComp(char *code_pntr, int code_len, CodeBlockHeader *hashed_block) {
    if(code_len != hashed_block->code_len)
        return FALSE;

    return memcmp(code_pntr, hashed_block + 1, code_len) == 0;
}

int compareLabels(const void *pntr1, const void *pntr2) {
    char *v1 = *(char **)pntr1;
    char *v2 = *(char **)pntr2;

    return v1 - v2;
}

void addToFreeList(CodeBlockHeader **blocks, int len) {
    CodeBlockHeader *last = NULL;
    CodeBlockHeader **block_pntr = blocks;
    CodeBlockHeader *free_pntr = code_free_list;

    if(len > 1)
        qsort(blocks, len, sizeof(CodeBlockHeader*), compareLabels);

    for(; len--; block_pntr++) {
        for(; free_pntr && free_pntr < *block_pntr;
              last = free_pntr, free_pntr = free_pntr->u.next);

        if(last) {
            if((char*)last + last->len == (char*)*block_pntr) {
                last->len += (*block_pntr)->len;
                goto out;
            }
            last->u.next = *block_pntr;
        } else
            code_free_list = *block_pntr;

        (*block_pntr)->u.next = free_pntr;
        last = *block_pntr;

out:
        if((char*)last + last->len == (char*)free_pntr) {
            last->u.next = free_pntr->u.next;
            last->len += free_pntr->len;
            free_pntr = last;
        }
    }
}

void freeMethodInlinedInfo(MethodBlock *mb) {
    Instruction *instruction = mb->code;
    CodeBlockHeader **blocks = mb->code;
    CodeBlockHeader *block = NULL;
    QuickPrepareInfo *prepare_info;
    ProfileInfo *profile_info;
    int i;

    if(!enabled)
        return;

    /* Scan handlers within the method */

    for(i = mb->code_size; i--; instruction++) {
        char *handler = (char*)instruction->handler;

        if(handler >= min_entry_point && handler <= max_entry_point) {
            /* Handler is within the program text and so does
               not need freeing.  However, sequences which
               have not been rewritten yet will have associated
               preparation info. */
            if(handler == handler_entry_points[0][OPC_INLINE_REWRITER]) {
                PrepareInfo *info = instruction->operand.pntr;

                gcPendingFree(info->block->opcodes);
                gcPendingFree(info->block);
                gcPendingFree(info);
            }

            continue;
        }

        /* Check if the handler points within the current block.
           This will occur when inlining across basic block
           boundaries (the handler points to the join). */
        if(block != NULL && handler > (char*)block
                         && handler < (char*)block + block->len)
            continue;

        /* The handler is the start of an inlined block */
        block = ((CodeBlockHeader*)handler) - 1;

        if(block->u.ref_count <= 0) {
            /* Either a duplicate block, or a hashed block and this
               is the only reference to it.  Duplicates must be freed
               as this would be a leak.  Hashed blocks potentially
               will be re-used and so we could keep them around.
               However, we free them because it's better to free
               room for a potentially more useful sequence. */

            /* Add onto list to be freed */
            *blocks++ = block;

            if(block->u.ref_count == 0)
                deleteHashEntry(code_hash_table, block, FALSE);

            /* Update code stats */
            used_codemem -= block->len;
        } else
            block->u.ref_count--;
    }

    /* Free all the blocks in one go (more efficient than freeing
       one by one) */

    if(blocks > (CodeBlockHeader**)mb->code)
        addToFreeList(mb->code, blocks - (CodeBlockHeader**)mb->code);

    /* Free the prepare information for blocks ending with a
       quickened instruction that haven't yet been inlined */

    for(prepare_info = mb->quick_prepare_info; prepare_info != NULL;) {
        QuickPrepareInfo *next = prepare_info->next;

        gcPendingFree(prepare_info->block->opcodes);
        gcPendingFree(prepare_info->block);
        gcPendingFree(prepare_info);

        prepare_info = next;
    }

    /* Free the information for blocks which have been
       executed but haven't yet been inlined */

    for(profile_info = mb->profile_info; profile_info != NULL;) {
        ProfileInfo *next = profile_info->next;

        gcPendingFree(profile_info->block->opcodes);
        gcPendingFree(profile_info->block);
        gcPendingFree(profile_info);

        profile_info = next;
    }
}

CodeBlockHeader *expandCodeMemory(int size) {
    CodeBlockHeader *block;
    int remainder;

    int inc = size < codemem_increment ? codemem_increment
                                       : ROUND(size, sys_page_size);

    if(codemem + inc > max_codemem) {
        inc = max_codemem - codemem;
        if(inc < size)
            return NULL;
    }

    block = mmap(0, inc, PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_PRIVATE | MAP_ANON, -1, 0);

    if(block == MAP_FAILED)
        return NULL;

    block->len = size;
    if((remainder = inc - size) >= sizeof(CodeBlockHeader)) {
        CodeBlockHeader *rem = (CodeBlockHeader*)((char*)block + size);

        rem->len = remainder;
        addToFreeList(&rem, 1);
    }

    codemem += inc;
    return block;
}

CodeBlockHeader *allocCodeBlock(int code_size) {
    int size = ALIGN(code_size + sizeof(CodeBlockHeader));
    CodeBlockHeader *block, *last = NULL;

    /* Search free list for big enough block */
    for(block = code_free_list; block != NULL && block->len < size;
        last = block, block = block->u.next);

    if(block != NULL) {
        int remainder = block->len - size;

        /* Found one.  If not exact fit, need to split. */
        if(remainder >= sizeof(CodeBlockHeader)) {
            CodeBlockHeader *rem = (CodeBlockHeader*)((char*)block + size);

            rem->len = remainder;
            rem->u.next = block->u.next;

            block->len = size;
            block->u.next = rem;
        }

        if(last != NULL)
            last->u.next = block->u.next;
        else
            code_free_list = block->u.next;
    } else {
        /* No block big enough.  Need to allocate a new code chunk */
        if((block = expandCodeMemory(size)) == NULL)
            return NULL;
    }

    block->code_len = code_size;

    /* Update code stats */
    used_codemem += block->len;

    return block;
}
    
CodeBlockHeader *newCodeBlock(TestCodeBlock *block) {
    CodeBlockHeader *new_block = allocCodeBlock(block->code_len);

    if(new_block != NULL) {
        /* Initialise the block reference count */
        new_block->u.ref_count = 0;

        /* Copy the test block from malloc-ed memory into
           code memory */
        memcpy(new_block + 1, block + 1, block->code_len);

        /* Flush the instruction cache */
        FLUSH_CACHE(new_block + 1, block->code_len);
    }

    return new_block;
}

void patchExternalJumps(TestCodeBlock *test_block, CodeBlockHeader *new_block);

CodeBlockHeader *newDuplicateBlock(TestCodeBlock *test_block) {
    CodeBlockHeader *new_block = allocCodeBlock(test_block->code_len);

    if(new_block != NULL) {
        /* Mark the block as a duplicate */
        new_block->u.ref_count = -1;

        /* Copy the test block from malloc-ed memory into
           code memory */
        memcpy(new_block + 1, test_block + 1, test_block->code_len);

        /* As duplicate blocks are non-shared it can be
           specialised by patching jumps to external blocks
           (patching a shared block would change it for all
            other users). */
        patchExternalJumps(test_block, new_block);

        /* Flush the instruction cache */
        FLUSH_CACHE(new_block + 1, test_block->code_len);
    }

    return new_block;
}

CodeBlockHeader *foundExistingBlock(TestCodeBlock *test_block,
                                    CodeBlockHeader *existing_block) {

    /* If the number of usages of the block has reached the replication
       threshold duplicate the block */
    if(existing_block->u.ref_count >= replication_threshold) {
        CodeBlockHeader *dup_block = newDuplicateBlock(test_block);

        if(dup_block != NULL)
            return dup_block;
    }

    /* Either no code memory for duplicate or not reached
       replication threshold. Just increase usage count */
    existing_block->u.ref_count++;

    return existing_block;
}

CodeBlockHeader *findCodeBlock(TestCodeBlock *block) {
    CodeBlockHeader *ret_block;

    lockHashTable(code_hash_table);

    if(branch_patching_dup && block->patchers != NULL)
        ret_block = newDuplicateBlock(block);
    else {
        /* Search hash table.  Add if absent, scavenge and not locked */
        findHashEntry(code_hash_table, block, ret_block, TRUE, TRUE, FALSE);
    }

    unlockHashTable(code_hash_table);

    return ret_block;
}

int insSeqCodeLen(BasicBlock *block, int start, int len) {
    OpcodeInfo *opcodes = &block->opcodes[start];
    int i, code_len = 0;

    for(i = 0; i < len; i++)
        code_len += handler_sizes[opcodes[i].cache_depth]
                                 [opcodes[i].opcode];

    return code_len;
}

int blockSeqCodeLen(BasicBlock *start, int ins_start, BasicBlock *end,
                    int ins_end) {
    int code_len;

    if(start == end)
        code_len = insSeqCodeLen(start, ins_start, ins_end - ins_start + 1);
    else {
        code_len = insSeqCodeLen(start, ins_start, start->length - ins_start);

        for(start = start->next; start != end; start = start->next)
            code_len += insSeqCodeLen(start, 0, start->length);

        code_len += insSeqCodeLen(end, 0, ins_end + 1);
    }

    return code_len;
}

char *insSeqCodeCopy(char *code_pntr, Instruction *ins_start_pntr, char **map,
                     BasicBlock **patchers, BasicBlock *block, int start,
                     int len) {

    Instruction *instructions = &block->start[start];
    OpcodeInfo *opcodes = &block->opcodes[start];
    int opcode = OPC_NOP, size = 0, depth, i;

    map[instructions - ins_start_pntr] = code_pntr;

    for(i = 0; i < len; i++) {
        code_pntr += size;
        opcode = opcodes[i].opcode;
        depth = opcodes[i].cache_depth;
        size = handler_sizes[depth][opcode];

        memcpy(code_pntr, instructions[i].handler, size);
    }

    if(branch_patching && opcode >= OPC_IFEQ && opcode <= OPC_JSR) {
        block->u.patch.addr = code_pntr + branch_patch_offsets[depth]
                                                  [opcode - OPC_IFEQ];
        block->u.patch.next = *patchers;
        *patchers = block;
    }

    return code_pntr + size;
}

void *inlineProfiledBlock(Instruction *pc, MethodBlock *mb, int force_inlining);

char *blockSeqCodeCopy(MethodBlock *mb, TestCodeBlock *block, BasicBlock *start,
                       int ins_start, BasicBlock *end, int ins_end) {

    char *code_pntr = (char *)(block + 1);
    BasicBlock *patchers, *ext_patchers = NULL;
    Instruction *ins_start_pntr = &start->start[ins_start];
    char *map[end->start - start->start - ins_start + ins_end + 1];

    block->patchers = NULL;

    if(start == end)
        code_pntr = insSeqCodeCopy(code_pntr, ins_start_pntr, map,
                   &block->patchers, start, ins_start, ins_end - ins_start + 1);
    else {
        code_pntr = insSeqCodeCopy(code_pntr, ins_start_pntr, map,
                 &block->patchers, start, ins_start, start->length - ins_start);

        for(start = start->next; start != end; start = start->next)
            code_pntr = insSeqCodeCopy(code_pntr, ins_start_pntr, map,
                                    &block->patchers, start, 0, start->length);

        code_pntr = insSeqCodeCopy(code_pntr, ins_start_pntr, map,
                                   &block->patchers, end, 0, ins_end + 1);
    }

    for(patchers = block->patchers; patchers != NULL;) {
        Instruction *target = patchers->start[patchers->length - 1].operand.pntr;
        BasicBlock *next = patchers->u.patch.next;

        if(target >= ins_start_pntr && target <= end->start) {

            if(GEN_REL_JMP(map[target - ins_start_pntr],
                           patchers->u.patch.addr, goto_len)) {
                TRACE("Patched branch within block\n");
            }
        } else {
            inlineProfiledBlock(target, mb, TRUE);

            patchers->u.patch.next = ext_patchers;
            ext_patchers = patchers;
        }
        patchers = next;
    }

    block->patchers = ext_patchers;
    return code_pntr;
}

void patchExternalJumps(TestCodeBlock *test_block, CodeBlockHeader *new_block) {
    BasicBlock *patchers = test_block->patchers;
    char *test_addr = (char*)(test_block + 1);
    char *new_addr = (char*)(new_block + 1);

    for(; patchers; patchers = patchers->u.patch.next) {
        Instruction *target = patchers->start[patchers->length - 1].operand.pntr;
        char *handler = (char*)target->handler;

        if(handler < min_entry_point || handler > max_entry_point) {
            char *addr = patchers->u.patch.addr - test_addr + new_addr;

            if(GEN_REL_JMP(handler, addr, goto_len)) {
                TRACE("Patched branch to external block %p\n", handler);
            }
        }
    }
}

#define INUM(mb, block, off) &block->start[off] - (Instruction*)mb->code

void updateSeqStarts(MethodBlock *mb, char *code_pntr, BasicBlock *start,
                     int ins_start, BasicBlock *end, int ins_end) {

    TRACE("Updating start block (%d len %d) %p\n", INUM(mb, start, ins_start),
          start->length - ins_start, code_pntr);

    start->start[ins_start].handler = code_pntr;
    MBARRIER();

    if(start != end) {
        code_pntr += insSeqCodeLen(start, ins_start, start->length - ins_start);

        for(start = start->next; start != end; start = start->next) {
            TRACE("Updating block join (%d len %d) %p\n", INUM(mb, start, 0),
                  start->length, code_pntr);

            start->start->handler = code_pntr;
            MBARRIER();

            code_pntr += insSeqCodeLen(start, 0, start->length);
        }

        TRACE("Updating end block (%d len %d) %p\n", INUM(mb, end, 0),
              ins_end + 1, code_pntr);

        end->start->handler = code_pntr;
        MBARRIER();
    }
}

void inlineSequence(MethodBlock *mb, BasicBlock *start, int ins_start,
                    BasicBlock *end, int ins_end) {
    CodeBlockHeader *hashed_block;
    TestCodeBlock *block;
    int code_len;
    char *pntr;

    /* Calculate sequence length */
    code_len = goto_len + blockSeqCodeLen(start, ins_start, end, ins_end);

    /* The prospective sequence is generated in malloc-ed memory
       so that an existing sequence can be found in the block cache
       even when no code memory is available */
    block = sysMalloc(code_len + sizeof(TestCodeBlock));

    /* Store length at beginning of sequence */
    block->code_len = code_len;

    /* Concatenate the handler bodies together */
    pntr = blockSeqCodeCopy(mb, block, start, ins_start, end, ins_end);

    /* Add the dispatch onto the end of the super-instruction */
    memcpy(pntr, goto_start, goto_len);

    /* Look up new block in inlined block cache */
    hashed_block = findCodeBlock(block);
    sysFree(block);

    if(hashed_block != NULL) {
        TRACE("%s.%s Inlined sequence %d, %d\n",
              CLASS_CB(mb->class)->name, mb->name,
              INUM(mb, start, ins_start),
              INUM(mb, end, ins_end));

        /* Replace the start handler with new inlined block,
           and update block joins to point within the sequence */
        updateSeqStarts(mb, (char*)(hashed_block + 1), start, ins_start,
                        end, ins_end);
    }
}

void inlineBlocks(MethodBlock *mb, BasicBlock *start, BasicBlock *end) {
    BasicBlock *block, *terminator = end->next;
    int ins_start = 0;

    for(block = start; block != terminator; block = block->next) {
        int i;

        for(i = 0; i < block->length; i++) {
            int cache_depth = block->opcodes[i].cache_depth;
            int opcode = block->opcodes[i].opcode;
            int op1, op2, op3;

            /* The block opcodes contain the "un-quickened" opcode.
               This could have been quickened to one of several quick
               versions. */
            switch(opcode) {
                case OPC_LDC:
                    op1 = OPC_LDC_QUICK;
                    op2 = op3 = OPC_LDC_W_QUICK;
                    break;

                case OPC_GETSTATIC:
                    op1 = OPC_GETSTATIC_QUICK;
                    op2 = OPC_GETSTATIC2_QUICK;
                    op3 = OPC_GETSTATIC_QUICK_REF;
                    break;

                 case OPC_PUTSTATIC:
                    op1 = OPC_PUTSTATIC_QUICK;
                    op2 = OPC_PUTSTATIC2_QUICK;
                    op3 = OPC_PUTSTATIC_QUICK_REF;
                    break;

                case OPC_GETFIELD: {
                    op1 = OPC_GETFIELD_QUICK;
                    op2 = OPC_GETFIELD2_QUICK;
                    op3 = OPC_GETFIELD_QUICK_REF;
                    break;
                }
                case OPC_PUTFIELD:
                    op1 = OPC_PUTFIELD_QUICK;
                    op2 = OPC_PUTFIELD2_QUICK;
                    op3 = OPC_PUTFIELD_QUICK_REF;
                    break;

                case OPC_NEW: case OPC_ANEWARRAY: case OPC_CHECKCAST:
                case OPC_INVOKESTATIC: case OPC_INVOKEINTERFACE:
                case OPC_INVOKEVIRTUAL: case OPC_INVOKESPECIAL:
                case OPC_MULTIANEWARRAY: case OPC_INSTANCEOF:
                case OPC_INVOKEDYNAMIC:
                    op1 = op2 = op3 = GOTO_END;
                    break;

                default:
                    op1 = op2 = op3 = -1;
                    break;
            }

            if(op1 > 0) {
                /* Match which quickened opcode */
                char *match = handler_entry_points[cache_depth][op1];
                char *handler = (char*) block->start[i].handler;

                if(match == handler)
                    opcode = op1;
                else {
                    char *match = handler_entry_points[cache_depth][op2];

                    if(match == handler)
                        opcode = op2;
                    else
                        opcode = op3;
                }

                block->opcodes[i].opcode = opcode;
            }

            /* A non-relocatable opcode ends a sequence */
            if(handler_sizes[cache_depth][opcode] < 0) {
                if(start != block || i > ins_start) {
                    if(i != 0)
                        inlineSequence(mb, start, ins_start, block, i - 1);
                    else
                        inlineSequence(mb, start, ins_start, block->prev,
                                       block->prev->length - 1);
                }

                if((ins_start = i + 1) == block->length) {
                    ins_start = 0;
                    start = block->next;
                } else
                    start = block;
            }
        }
    }

    /* Inline the remaining sequence */
    if(start != terminator)
        inlineSequence(mb, start, ins_start, end, end->length - 1);
}

void rewriteLock(Thread *self) {
    /* Only disable/enable suspension (slow) if
       we have to block */
    if(!tryLockVMLock(rewrite_lock, self)) {
        disableSuspend(self);
        lockVMLock(rewrite_lock, self);
        enableSuspend(self);
    }
}

void rewriteUnlock(Thread *self) {
    unlockVMLock(rewrite_lock, self);
}

void removeFromProfile(MethodBlock *mb, BasicBlock *block) {
    ProfileInfo *profile_info = block->u.profile.profiled;

    /* If the profile info is null, this is a non-quickened
       block which can be inlined without executing first */
    if(profile_info == NULL) {
        int ins_idx = block->length - 1;
        Instruction *ins = &block->start[ins_idx];
        PrepareInfo *prepare_info = ins->operand.pntr;
        OpcodeInfo *opcode_info = &block->opcodes[ins_idx];

        TRACE("Unwrapping non-quickened block (start %p)...\n", block->start);

        /* Unwrap the original handler's operand */
        ins->operand = prepare_info->operand;
        MBARRIER();

        /* Unwrap the original handler */
        ins->handler = handler_entry_points[opcode_info->cache_depth]
                                           [opcode_info->opcode];

        sysFree(prepare_info);
        return;
    }

    TRACE("Removing block (start %p) from profile...\n", block->start);

    block->start->handler = profile_info->handler;

    if(profile_info->prev)
        profile_info->prev->next = profile_info->next;
    else
        mb->profile_info = profile_info->next;

    if(profile_info->next)
        profile_info->next->prev = profile_info->prev;

    sysFree(profile_info);
}

void inlineBlock(MethodBlock *mb, BasicBlock *block, Thread *self) {
    BasicBlock *start, *end;

    /* We scan backwards and forwards from block to find the range
       of inlineable blocks.  This improves performance as super-
       instructions can potentially be created across block
       boundaries, saving an indirect jump between blocks (for
       example, the fallthrough case in an if) */

    for(start = block; start->prev && (start->prev->u.profile.profiled ||
                                      !start->prev->u.profile.quickened);
                       start = start->prev)
        removeFromProfile(mb, start);

    removeFromProfile(mb, start);

    for(end = block; end->next && (end->next->u.profile.profiled ||
                                  !end->next->u.profile.quickened); )
        removeFromProfile(mb, end = end->next);

    if(start->prev)
        start->prev->next = NULL;
    if(end->next)
        end->next->prev = NULL;

    rewriteUnlock(self);

    TRACE("%s.%s InlineBlock trigger %d start %d end %d\n",
          CLASS_CB(mb->class)->name, mb->name, INUM(mb, block, 0),
          INUM(mb, start, 0), INUM(mb, end, 0));

    inlineBlocks(mb, start, end);

    /* Blocks have been inlined so we can now free the memory.  Note,
       inlineBlocks doesn't free the blocks as previously scanned
       blocks may be inlined later */

    for(end = end->next; start != end; ) {
        BasicBlock *next = start->next;

        sysFree(start->opcodes);
        sysFree(start);
        start = next;
    }
}

/* If profiling is enabled, basic blocks are not inlined immediately
   once they have been executed (and any quickening has been performed).
   Instead, the first opcode is replaced with a special profiler opcode
   and the block is added to the profile cache.  The block will then be
   inlined once the block reaches an execution threshold.  This enables
   surrounding blocks to be executed and quickened before inlining is
   attempted, potentially enabling inlining across block boundaries
   (without profiling, inlining is limited to the current block, as
   subsequent blocks which require quickening cannot be inlined until
   they have been executed).
*/
void addToProfile(MethodBlock *mb, BasicBlock *block, Thread *self) {
    ProfileInfo *info = sysMalloc(sizeof(ProfileInfo));

    TRACE("Adding block (start %p) to profile\n", block->start);
    info->profile_count = 0;
    info->block = block;

    block->u.profile.profiled = info;

    info->prev = NULL;
    if((info->next = mb->profile_info))
       info->next->prev = info;
    mb->profile_info = info;

    info->handler = block->start->handler;
    block->start->handler = handler_entry_points[0][OPC_PROFILE_REWRITER];

    rewriteUnlock(self);
}

void prepareBlock(MethodBlock *mb, BasicBlock *block, Thread *self) {
    if(profiling)
        addToProfile(mb, block, self);
    else {
        rewriteUnlock(self);

        inlineBlocks(mb, block, block);
        sysFree(block->opcodes);
        sysFree(block);
    }
}

void inlineBlockWrappedOpcode(MethodBlock *mb, Instruction *pc) {
    PrepareInfo *prepare_info = pc->operand.pntr;
    OpcodeInfo *info;
    int i;

    Thread *self = threadSelf();
    rewriteLock(self);

    for(i = 0; i < HANDLERS; i++)
        if(pc->handler == handler_entry_points[i][OPC_INLINE_REWRITER])
            break;

    if(i == HANDLERS) {
        rewriteUnlock(self);
        return;
    }

    /* Unwrap the original handler's operand */
    pc->operand = prepare_info->operand;

    /* Unwrap the original handler */
    info = &prepare_info->block->opcodes[prepare_info->block->length-1];
    pc->handler = handler_entry_points[info->cache_depth][info->opcode];

    prepareBlock(mb, prepare_info->block, self);
    sysFree(prepare_info);
}

/* A method's quick prepare info list holds prepare information for all
   blocks within the method that end with a quickened instruction.  If
   the quickened instruction being executed is in the list we must have
   reached the end of a block and we need to inline it */
void checkInliningQuickenedInstruction(Instruction *pc, MethodBlock *mb) {

    /* As there could be multiple threads executing this method,
       the list must be protected with a lock.  However, the 
       fast case of an empty list doesn't need locking. */
    if(mb->quick_prepare_info) {
        QuickPrepareInfo *info, *last = NULL;

        Thread *self = threadSelf();
        rewriteLock(self);

        /* Search list */
        for(info = mb->quick_prepare_info; info && info->quickened != pc;
            last = info, info = info->next);

        if(info == NULL) {
            rewriteUnlock(self);
            return;
        }

        /* Remove it from the list */
        if(last != NULL)
            last->next = info->next;
        else
            mb->quick_prepare_info = info->next;

        prepareBlock(mb, info->block, self);
        sysFree(info);
    }
}

/* Search the profile list for the block and inline if the execution
   threshold has been reached.  The profile list is per-method, and
   blocks are added to the head of the list.  Testing shows 70% of
   searches are found within the first 2 entries and 90% within the
   first 4.  This is more consistent than a hashtable where hit
   rate decreases with table occupancy.
*/
void *inlineProfiledBlock(Instruction *pc, MethodBlock *mb, int force_inlining) {
    Thread *self = threadSelf();
    ProfileInfo *info;
    void *ret;

    rewriteLock(self);

    /* Search profile cache for block */
    for(info = mb->profile_info; info != NULL && info->block->start != pc;
        info = info->next);

    if(info != NULL && (force_inlining ||
                        info->profile_count++ >= profile_threshold)) {

        inlineBlock(mb, info->block, self);
        return NULL;
    }

    ret = info == NULL ? NULL : (void*)info->handler;
    rewriteUnlock(self);

    return ret;
}
#endif

