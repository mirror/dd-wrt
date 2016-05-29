/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013
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

#ifdef DIRECT
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#include "thread.h"
#include "interp.h"
#include "symbol.h"
#include "inlining.h"
#include "shared.h"

#include "hash.h"
#include "class.h"
#include "classlib.h"

#ifdef TRACEDIRECT
#define TRACE(fmt, ...) jam_printf(fmt, ## __VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#define REWRITE_OPERAND(index) \
    quickened = TRUE;          \
    operand.uui.u1 = index;    \
    operand.uui.u2 = opcode;   \
    operand.uui.i = ins_cache;

/* Used to indicate that stack depth
   has not been calculated yet */
#define DEPTH_UNKNOWN -1

/* Global lock for method preparation */
static VMWaitLock prepare_lock;

#ifdef INLINING
int inlining_enabled;
static int join_blocks;
#endif

void initialiseDirect(InitArgs *args) {
#ifdef INLINING
    inlining_enabled = initialiseInlining(args);
    join_blocks      = args->join_blocks;
#endif
    initVMWaitLock(prepare_lock);
}

void prepare(MethodBlock *mb, const void ***handlers) {
    int code_len = mb->code_size;
#ifdef USE_CACHE
    signed char cache_depth[code_len + 1];
#endif
#ifdef INLINING
    int inlining = inlining_enabled && mb->name != SYMBOL(class_init);
    BasicBlock *last_block = NULL;
    OpcodeInfo opcodes[code_len];
    char info[code_len + 1];
#endif
    unsigned char *code = mb->code;
    Instruction *new_code = NULL;
    short map[code_len];
    int ins_count = 0;
    int pass;
    int i;

    /* The method's state field indicates whether the method
       has been prepared.  The check in the interpreter is
       unsynchronised, so grab the lock and recheck.  If
       another thread tries to prepare the method, they will
       wait on the lock.  This lock is global, but methods are
       only prepared once, and contention to prepare a method
       is unlikely. */

    Thread *self = threadSelf();

    disableSuspend(self);
    lockVMWaitLock(prepare_lock, self);

retry:
    switch(mb->state) {
        case MB_UNPREPARED:
            mb->state = MB_PREPARING;
            break;

        case MB_PREPARING:
            waitVMWaitLock(prepare_lock, self);
            goto retry;

        default:
            unlockVMWaitLock(prepare_lock, self);
            enableSuspend(self);
            return;
    }

    unlockVMWaitLock(prepare_lock, self);

    TRACE("Preparing %s.%s%s\n", CLASS_CB(mb->class)->name, mb->name, mb->type);

#ifdef USE_CACHE
    /* Initialise cache depth array, indicating that
       the depth of every bytecode is unknown */
    memset(cache_depth, DEPTH_UNKNOWN, code_len + 1);

    /* Exception handlers are entered at cache-depth zero.  Initialise
       the start of each handler to trap "fall-through" into the handler
       -- this should never happen in code produced by javac */
    for(i = 0; i < mb->exception_table_size; i++)
        cache_depth[mb->exception_table[i].handler_pc] = 0;
#endif
#ifdef INLINING
    /* Mark the start opcode of each exception handler as an
       exception */
    memset(info, FALSE, code_len + 1);
    for(i = 0; i < mb->exception_table_size; i++)
        info[mb->exception_table[i].handler_pc] = HANDLER;
#endif

    for(pass = 0; pass < 2; pass++) {
        int block_quickened = FALSE;
#ifdef INLINING
        int block_start = 0;
#endif
        int cache = 0;
        int pc;

        if(pass == 1)
            new_code = sysMalloc((ins_count + 1) * sizeof(Instruction));

        for(ins_count = 0, pc = 0; pc < code_len; ins_count++) {
            int quickened = FALSE;
            Operand operand;
            int ins_cache;
            int opcode;

            /* Load the opcode -- we change it if it's WIDE, or ALOAD_0
               and the method is an instance method */
            opcode = code[pc];

            /* On pass one we calculate the cache depth and the mapping
               between bytecode and instruction numbering */

            if(pass == 0) {
#ifdef USE_CACHE
                    TRACE("%d : pc %d opcode %d cache %d\n", ins_count, pc, opcode, cache);
                    cache_depth[pc] = cache;
#else
                    TRACE("%d : pc %d opcode %d\n", ins_count, pc, opcode);
#endif
                    map[pc] = ins_count;
            }
#ifdef DIRECT_DEBUG
            else {
                new_code[ins_count].opcode = opcode;
                new_code[ins_count].bytecode_pc = pc;
                new_code[ins_count].cache_depth = cache;
            }
#endif
            /* For instructions without an operand */
            operand.i = 0;
            ins_cache = cache;

            switch(opcode) {
                default:
                    jam_printf("Unrecognised bytecode %d found while preparing %s.%s%s\n",
                               opcode, CLASS_CB(mb->class)->name, mb->name, mb->type);
                    exitVM(1);

                case OPC_ALOAD_0:
                {
                    FieldBlock *fb;
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
#endif
                    /* If the next instruction is GETFIELD, this is an instance method
                       and the field is not 2-slots rewrite it to GETFIELD_THIS.  We
                       can safely resolve the field because as an instance method
                       the class must be initialised */

                    if((code[++pc] == OPC_GETFIELD) && !(mb->access_flags & ACC_STATIC)
                                    && (fb = resolveField(mb->class, READ_U2_OP(code + pc)))
                                    && !((*fb->type == 'J') || (*fb->type == 'D'))) {
                        if(*fb->type == 'L' || *fb->type == '[')
                            opcode = OPC_GETFIELD_THIS_REF;
                        else
                            opcode = OPC_GETFIELD_THIS;

                        operand.i = fb->u.offset;
                        pc += 3;
                    } else
                        opcode = OPC_ILOAD_0;
                    break;
                }
                case OPC_SIPUSH:
                    operand.i = READ_S2_OP(code + pc);
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
#endif
                    pc += 3;
                    break;
        
                case OPC_LDC_W:
                    REWRITE_OPERAND(READ_U2_OP(code + pc));
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
#endif
                    opcode = OPC_LDC;
                    pc += 3;
                    break;
    
                case OPC_LDC2_W:
                    operand.i = READ_U2_OP(code + pc);
#ifdef USE_CACHE
                    cache = 2;
#endif
                    pc += 3;
                    break;
        
                case OPC_LDC:
                    REWRITE_OPERAND(READ_U1_OP(code + pc));
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
#endif
                    pc += 2;
                    break;
    
                case OPC_BIPUSH:
                    operand.i = READ_S1_OP(code + pc);
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
#endif
                    pc += 2;
                    break;

                case OPC_ILOAD:
                case OPC_FLOAD:
                case OPC_ALOAD:
#ifdef USE_CACHE
                    operand.i = READ_U1_OP(code + pc);
                    if(cache < 2) 
                        cache++;
                    pc += 2;
                    break;
#endif
                case OPC_LLOAD:
                case OPC_DLOAD:
                    operand.i = READ_U1_OP(code + pc);
#ifdef USE_CACHE
                    cache = 2;
#endif
                    pc += 2;
                    break;
        
                case OPC_RETURN: case OPC_IRETURN:  case OPC_ARETURN:
                case OPC_FRETURN: case OPC_LRETURN: case OPC_DRETURN:
                case OPC_ATHROW:
#if defined(USE_CACHE) || defined(INLINING)
                    pc += 1;
#ifdef INLINING
                    info[pc] |= END;
#endif
#ifdef USE_CACHE
                    cache = 0;
#endif
                    break;
#endif
                case OPC_ACONST_NULL: case OPC_ICONST_M1: case OPC_ICONST_0:
                case OPC_FCONST_0: case OPC_ICONST_1: case OPC_ICONST_2:
                case OPC_ICONST_3: case OPC_ICONST_4: case OPC_ICONST_5:
                case OPC_FCONST_1: case OPC_FCONST_2: case OPC_ILOAD_0:
                case OPC_FLOAD_0: case OPC_ILOAD_1: case OPC_FLOAD_1:
                case OPC_ALOAD_1: case OPC_ILOAD_2: case OPC_FLOAD_2:
                case OPC_ALOAD_2: case OPC_ILOAD_3: case OPC_FLOAD_3:
                case OPC_ALOAD_3: case OPC_DUP:
#ifdef USE_CACHE
                    if(cache < 2) 
                        cache++;
                    pc += 1;
                    break;
#endif
                case OPC_LCONST_0: case OPC_DCONST_0: case OPC_DCONST_1:
                case OPC_LCONST_1: case OPC_LLOAD_0: case OPC_DLOAD_0:
                case OPC_LLOAD_1: case OPC_DLOAD_1: case OPC_LLOAD_2:
                case OPC_DLOAD_2: case OPC_LLOAD_3: case OPC_DLOAD_3:
                case OPC_LALOAD: case OPC_DALOAD: case OPC_DUP_X1:
                case OPC_DUP_X2: case OPC_DUP2: case OPC_DUP2_X1:
                case OPC_DUP2_X2: case OPC_SWAP: case OPC_LADD:
                case OPC_LSUB: case OPC_LMUL: case OPC_LDIV:
                case OPC_LREM: case OPC_LAND: case OPC_LOR:
                case OPC_LXOR: case OPC_LSHL: case OPC_LSHR:
                case OPC_LUSHR: case OPC_F2L: case OPC_D2L:
                case OPC_LNEG: case OPC_I2L:
#ifdef USE_CACHE
                    cache = 2;
                    pc += 1;
                    break;
#endif
                case OPC_NOP: case OPC_LSTORE_0: case OPC_DSTORE_0:
                case OPC_LSTORE_1: case OPC_DSTORE_1: case OPC_LSTORE_2:
                case OPC_DSTORE_2: case OPC_LSTORE_3: case OPC_DSTORE_3:
                case OPC_IASTORE: case OPC_FASTORE: case OPC_AASTORE:
                case OPC_LASTORE: case OPC_DASTORE: case OPC_BASTORE:
                case OPC_CASTORE: case OPC_SASTORE: case OPC_POP2:
                case OPC_FADD: case OPC_DADD: case OPC_FSUB:
                case OPC_DSUB: case OPC_FMUL: case OPC_DMUL:
                case OPC_FDIV: case OPC_DDIV: case OPC_I2F:
                case OPC_I2D: case OPC_L2F: case OPC_L2D:
                case OPC_F2D: case OPC_D2F: case OPC_FREM:
                case OPC_DREM: case OPC_FNEG: case OPC_DNEG:
                case OPC_MONITORENTER: case OPC_MONITOREXIT:
                case OPC_ABSTRACT_METHOD_ERROR:
                case OPC_MIRANDA_BRIDGE:
#ifdef USE_CACHE
                    cache = 0;
                    pc += 1;
                    break;
#endif
                case OPC_ISTORE_0: case OPC_ASTORE_0: case OPC_FSTORE_0:
                case OPC_ISTORE_1: case OPC_ASTORE_1: case OPC_FSTORE_1:
                case OPC_ISTORE_2: case OPC_ASTORE_2: case OPC_FSTORE_2:
                case OPC_ISTORE_3: case OPC_ASTORE_3: case OPC_FSTORE_3:
                case OPC_POP:
#ifdef USE_CACHE
                    if(cache > 0)
                        cache--;
                    pc += 1;
                    break;
#endif
                case OPC_INEG:
#ifdef USE_CACHE
                    cache = cache == 2 ? 2 : 1;
                    pc += 1;
                    break;
#endif
                case OPC_IALOAD: case OPC_AALOAD: case OPC_FALOAD:
                case OPC_BALOAD: case OPC_CALOAD: case OPC_SALOAD:
                case OPC_IADD: case OPC_IMUL: case OPC_ISUB:
                case OPC_IDIV: case OPC_IREM: case OPC_IAND:
                case OPC_IOR: case OPC_IXOR: case OPC_F2I:
                case OPC_D2I: case OPC_I2B: case OPC_I2C:
                case OPC_I2S: case OPC_ISHL: case OPC_ISHR:
                case OPC_IUSHR: case OPC_LCMP: case OPC_DCMPG:
                case OPC_DCMPL: case OPC_FCMPG: case OPC_FCMPL:
                case OPC_ARRAYLENGTH: case OPC_L2I:
#ifdef USE_CACHE
                    cache = 1;
#endif
                    pc += 1;
                    break;
    
                case OPC_LSTORE:
                case OPC_DSTORE:
#ifdef USE_CACHE
                    operand.i = READ_U1_OP(code + pc);
                    cache = 0;
                    pc += 2;
                    break;
#endif
                case OPC_ISTORE:
                case OPC_FSTORE:
                case OPC_ASTORE:
                    operand.i = READ_U1_OP(code + pc);
#ifdef USE_CACHE
                    if(cache > 0)
                        cache--;
#endif
                    pc += 2;
                    break;
        
                case OPC_IINC:
                    operand.ii.i1 = READ_U1_OP(code + pc); 
                    operand.ii.i2 = READ_S1_OP(code + pc + 1);
                    pc += 3;
                    break;
    
                case OPC_IFEQ: case OPC_IFNULL: case OPC_IFNE:
                case OPC_IFNONNULL: case OPC_IFLT: case OPC_IFGE:
                case OPC_IFGT: case OPC_IFLE: case OPC_IF_ACMPEQ:
                case OPC_IF_ICMPEQ: case OPC_IF_ACMPNE: case OPC_IF_ICMPNE:
                case OPC_IF_ICMPLT: case OPC_IF_ICMPGE: case OPC_IF_ICMPGT:
                case OPC_IF_ICMPLE:
                {
                    int dest = pc + READ_S2_OP(code + pc);
#ifdef USE_CACHE
                    /* Conflict can only occur on first pass, and dest must be backwards */
                    if(cache_depth[dest] > 0) {
                        TRACE("CONFLICT in IF target addr: %d\n", dest);

                        /* Reset depthes calculated from the (backwards) destination and
                           here.  By setting depth at dest to zero (see below) and starting
                           from dest again, we will immediately get a conflict which will be
                           resolved by adding a NOP. */
                        memset(&cache_depth[dest + 1], DEPTH_UNKNOWN, pc - dest);
                        cache = cache_depth[dest];
                        ins_count = map[dest] - 1;
                        pc = dest;
                    } else {
                        cache = 0;
#endif
                        pc += 3;
#ifdef USE_CACHE
                    }
#endif
    
                    if(pass == 1) {
                        TRACE("IF old dest %d new dest %d\n", dest, map[dest]);
                        operand.pntr = &new_code[map[dest]];
                    } else {
#ifdef USE_CACHE
                        /* Branches re-cache, so the cache depth at destination is zero */
                        cache_depth[dest] = 0;
#endif
#ifdef INLINING
                        info[pc]   |= FALLTHROUGH;
                        info[dest] |= TARGET;
#endif
                    }

                    break;
                }

                case OPC_INVOKEVIRTUAL: case OPC_INVOKESPECIAL:
                case OPC_INVOKESTATIC: case OPC_CHECKCAST:
                case OPC_INSTANCEOF: case OPC_PUTFIELD:
                    REWRITE_OPERAND(READ_U2_OP(code + pc));
#ifdef USE_CACHE
                    cache = 0;
#endif
                    pc += 3;
                    break;
        
                case OPC_GOTO_W:
                case OPC_GOTO:
                {
                    int delta, dest;
                    
                    if(opcode == OPC_GOTO)
                        delta = READ_S2_OP(code + pc);
                    else
                        delta = READ_S4_OP(code + pc);

                    dest = pc + delta;

#ifdef USE_CACHE
                    /* Conflict can only occur on first pass, and dest must be backwards */
                    if(cache_depth[dest] > 0) {
                        TRACE("CONFLICT in GOTO target addr: %d\n", dest);

                        /* Reset depthes calculated from the (backwards) destination and
                           here.  By setting depth at dest to zero (see below) and starting
                           from dest again, we will immediately get a conflict which will be
                           resolved by adding a NOP. */
                        memset(&cache_depth[dest + 1], DEPTH_UNKNOWN, pc - dest);
                        cache = cache_depth[dest];
                        ins_count = map[dest] - 1;
                        pc = dest;
                    } else {
                        cache = 0;
#endif
                        pc += opcode == OPC_GOTO ? 3 : 5;
#ifdef USE_CACHE
                    }
#endif

                    if(pass == 1) {
                        TRACE("GOTO old dest %d new dest %d\n", dest, map[dest]);
                        operand.pntr = &new_code[map[dest]];
                    } else {
#ifdef USE_CACHE
                        /* GOTO re-caches, so the cache depth at destination is zero */
                        cache_depth[dest] = 0;
#endif
#ifdef INLINING
                        info[pc]   |= END;
                        info[dest] |= TARGET;
#endif
                    }

                    opcode = OPC_GOTO;
                    break;
                }
        
                case OPC_TABLESWITCH:
                {
                    int *aligned_pc = (int*)(code + ((pc + 4) & ~0x3));
                    int deflt = ntohl(aligned_pc[0]);
                    int low   = ntohl(aligned_pc[1]);
                    int high  = ntohl(aligned_pc[2]);
                    int i;
    
                    if(pass == 0) {
#if defined(USE_CACHE) || defined(INLINING)
                        /* Destinations should always be forward WRT pc, i.e. not
                           visited yet.  Depths can only be -1 (or 0) so conflict
                           is impossible.  */
    
                        /* TABLESWITCH re-caches, so all possible destinations
                           are at cache depth zero */
#ifdef USE_CACHE
                        cache_depth[pc + deflt] = 0;
#endif
#ifdef INLINING
                        info[pc + deflt] |= TARGET;
#endif
                        for(i = 3; i < (high - low + 4); i++) {
                            int dest = pc + ntohl(aligned_pc[i]);
#ifdef USE_CACHE
                            cache_depth[dest] = 0;
#endif
#ifdef INLINING
                            info[dest] |= TARGET;
#endif
                        }
#else
                        i = high - low + 4;
#endif
                    } else {
                        SwitchTable *table = sysMalloc(sizeof(SwitchTable));

                        table->low = low;
                        table->high = high; 
                        table->deflt = &new_code[map[pc + deflt]];
                        table->entries = sysMalloc((high - low + 1) * sizeof(Instruction *));

                        for(i = 3; i < (high - low + 4); i++)
                            table->entries[i - 3] = &new_code[map[pc + ntohl(aligned_pc[i])]];

                        operand.pntr = table;
                    }

                    pc = (unsigned char*)&aligned_pc[i] - code;
#ifdef USE_CACHE
                    cache = 0;
#endif    
#ifdef INLINING
                    info[pc] |= END;
#endif    
                    break;
                }
        
                case OPC_LOOKUPSWITCH:
                {
                    int *aligned_pc = (int*)(code + ((pc + 4) & ~0x3));
                    int deflt  = ntohl(aligned_pc[0]);
                    int npairs = ntohl(aligned_pc[1]);
                    int i, j;
    
                    if(pass == 0) {
#if defined(USE_CACHE) || defined(INLINING)
                        /* Destinations should always be forward WRT pc, i.e. not
                           visited yet.  Depths can only be -1 (or 0) so conflict
                           is impossible.  */
    
                        /* LOOKUPSWITCH re-caches, so all possible destinations
                           are at cache depth zero */
#ifdef USE_CACHE
                        cache_depth[pc + deflt] = 0;
#endif
#ifdef INLINING
                        info[pc + deflt] |= TARGET;
#endif

                        for(i = 2; i < (npairs*2+2); i += 2) {
                            int dest = pc + ntohl(aligned_pc[i+1]);
#ifdef USE_CACHE
                            cache_depth[dest] = 0;
#endif
#ifdef INLINING
                            info[dest] |= TARGET;
#endif
                        }
#else
                        i = npairs*2+2;
#endif
                    } else {
                        LookupTable *table = sysMalloc(sizeof(LookupTable));
   
                        table->num_entries = npairs;
                        table->deflt = &new_code[map[pc + deflt]];
                        table->entries = sysMalloc(npairs * sizeof(LookupEntry));
                            
                        for(i = 2, j = 0; j < npairs; i += 2, j++) {
                            table->entries[j].key = ntohl(aligned_pc[i]);
                            table->entries[j].handler = &new_code[map[pc + ntohl(aligned_pc[i+1])]];
                        }
                        operand.pntr = table;
                    }

                    pc = (unsigned char*)&aligned_pc[i] - code;
#ifdef USE_CACHE
                    cache = 0;
#endif        
#ifdef INLINING
                    info[pc] |= END;
#endif        
                    break;
                }
    
                case OPC_GETSTATIC:
#ifdef USE_CACHE
                {
                    int idx = READ_U2_OP(code + pc);
                    REWRITE_OPERAND(idx);

                    cache = cache > 0 || peekIsFieldLong(mb->class, idx) ? 2 : 1;
                    pc += 3;
                    break;
                }
#endif        
                case OPC_PUTSTATIC: 
#ifdef USE_CACHE
                {
                    int idx = READ_U2_OP(code + pc);
                    REWRITE_OPERAND(idx);

                    cache = cache < 2 || peekIsFieldLong(mb->class, idx) ? 0 : 1;
                    pc += 3;
                    break;
                }
#endif        
                case OPC_GETFIELD:
                {
                    int idx = READ_U2_OP(code + pc);
                    REWRITE_OPERAND(idx);
#ifdef USE_CACHE
                    cache = cache == 2 || peekIsFieldLong(mb->class, idx) ? 2 : 1;
#endif        
                    pc += 3;
                    break;
                }
    
                case OPC_INVOKEINTERFACE:
#ifdef JSR292
                case OPC_INVOKEDYNAMIC:
#endif
                    REWRITE_OPERAND(READ_U2_OP(code + pc));
#ifdef USE_CACHE
                    cache = 0;
#endif        
                    pc += 5;
                    break;
    
                case OPC_JSR_W:
                case OPC_JSR:
                {
                    int delta, dest;
                    
                    if(opcode == OPC_JSR)
                        delta = READ_S2_OP(code + pc);
                    else
                        delta = READ_S4_OP(code + pc);

                    dest = pc + delta;
#ifdef USE_CACHE
                    /* Conflict can only occur on first pass, and dest must be backwards */
                    if(cache_depth[dest] > 0) {
                        TRACE("CONFLICT in JSR target addr: %d\n", dest);

                        /* Reset depthes calculated from the (backwards) destination and
                           here.  By setting depth at dest to zero (see below) and starting
                           from dest again, we will immediately get a conflict which will be
                           resolved by adding a NOP. */

                        memset(&cache_depth[dest + 1], DEPTH_UNKNOWN, pc - dest);
                        cache = cache_depth[dest];
                        ins_count = map[dest] - 1;
                        pc = dest;
                    } else {
                        cache = 0;
#endif
                        pc += opcode == OPC_JSR ? 3 : 5;
#ifdef USE_CACHE
                    }
#endif
                    if(pass == 1) {
                        TRACE("JSR old dest %d new dest %d\n", dest, map[dest]);
                        operand.pntr = &new_code[map[dest]];
                    } else {
#ifdef USE_CACHE
                        /* JSR re-caches, so the cache depth at destination is zero */
                        cache_depth[dest] = 0;
#endif
#ifdef INLINING
                        info[pc]   |= TARGET;
                        info[dest] |= TARGET;
#endif
                    }

                    opcode = OPC_JSR;
                    break;
                }
    
                case OPC_RET:
                    operand.i = READ_U1_OP(code + pc);
#ifdef USE_CACHE
                    cache = 0;
#endif
                    pc += 2;
#ifdef INLINING
                    info[pc] |= END;
#endif
                    break;

                case OPC_NEWARRAY:
                    operand.i = READ_U1_OP(code + pc);
#ifdef USE_CACHE
                    cache = 1;
#endif
                    pc += 2;
                    break;
        
                case OPC_NEW:
                case OPC_ANEWARRAY:
                    REWRITE_OPERAND(READ_U2_OP(code + pc));
#ifdef USE_CACHE
                    cache = 1;
#endif
                    pc += 3;
                    break;
    
                case OPC_MULTIANEWARRAY:
                    operand.uui.u1 = READ_U2_OP(code + pc);
                    operand.uui.u2 = READ_U1_OP(code + pc + 2);
                    operand.uui.i = cache;
                    quickened = TRUE;
#ifdef USE_CACHE
                    cache = 1;
#endif
                    pc += 4;
                    break;
    
                /* All instructions are "wide" in the direct interpreter --
                   rewrite OPC_WIDE to the actual widened instruction */
                case OPC_WIDE:
                {
                    opcode = code[pc + 1];
        
                    switch(opcode) {
                        case OPC_ILOAD:
                        case OPC_FLOAD:
                        case OPC_ALOAD:
#ifdef USE_CACHE
                            operand.i = READ_U2_OP(code + pc + 1);
                            if(cache < 2) 
                                cache++;
                            pc += 4;
                            break;
#endif
                        case OPC_LLOAD:
                        case OPC_DLOAD:
#ifdef USE_CACHE
                            operand.i = READ_U2_OP(code + pc + 1);
                            cache = 2;
                            pc += 4;
                            break;
#endif
                        case OPC_ISTORE:
                        case OPC_FSTORE:
                        case OPC_ASTORE:
#ifdef USE_CACHE
                            operand.i = READ_U2_OP(code + pc + 1);
                            if(cache > 0)
                                cache--;
                            pc += 4;
                            break;
#endif
                        case OPC_LSTORE:
                        case OPC_DSTORE:
                        case OPC_RET:
                            operand.i = READ_U2_OP(code + pc + 1);
#ifdef USE_CACHE
                            cache = 0;
#endif
                            pc += 4;
                            break;
    
                        case OPC_IINC:
                            operand.ii.i1 = READ_U2_OP(code + pc + 1); 
                            operand.ii.i2 = READ_S2_OP(code + pc + 3);
                            pc += 6;
                            break;
                    }
                }
            }

#ifdef INLINING
            if(opcode <= OPC_JSR_W)
                opcode = shared_opcodes[opcode];
#endif

#ifdef USE_CACHE
            /* If the next instruction is reached via a branch (or catching an
               exception) the depth will already be known.  If this is
               different to the depth "falling through" we have a conflict.
               Resolve it by inserting a NOP at the appropriate depth which
               re-caches.  This also handles dead-code.  Note, this generally
               only happens in the x ? y : z code sequence.

               The NOP is inserted by writing the current instruction and then
               replacing it with a NOP.  This is done so that the basic block
               (calculated below) includes the NOP */

            if((cache_depth[pc] != DEPTH_UNKNOWN) && (cache_depth[pc] != cache)) {
                TRACE("CONFLICT @ addr: %d depth1 %d depth2 %d\n", pc, cache_depth[pc], cache);

                if(pass == 1) {
                    new_code[ins_count].handler = handlers[ins_cache][opcode];
                    new_code[ins_count].operand = operand;
		}
#ifdef INLINING
                opcodes[ins_count].opcode = opcode;
                opcodes[ins_count].cache_depth = ins_cache;
#endif
		opcode = OPC_NOP;
		ins_cache = cache;
                cache = 0;
                ins_count++;
            }
#endif

            if(pass == 0) {
#ifdef INLINING
                opcodes[ins_count].opcode = opcode;
                opcodes[ins_count].cache_depth = ins_cache;
#endif
            } else {
#ifdef INLINING
                block_quickened = block_quickened || quickened;

                if(inlining && info[pc]) {
                    TRACE("Info %x @ pc %d opcode %d\n", info[pc], pc, opcode);

                    if(block_start != -1) {
                        int ins_start = map[block_start];
                        int block_len = ins_count - ins_start + 1;
                        BasicBlock *block = sysMalloc(sizeof(BasicBlock));

                        TRACE("Block start %d end %d length %d last opcode quickened %d\n",
                              ins_start, ins_count, block_len, quickened);

                        if(quickened) {
                            QuickPrepareInfo *prepare_info;

                            prepare_info = sysMalloc(sizeof(QuickPrepareInfo));
                            prepare_info->quickened = &new_code[ins_count];
                            prepare_info->next = mb->quick_prepare_info;
                            mb->quick_prepare_info = prepare_info;
                            prepare_info->block = block;
                        } else {
                            PrepareInfo *prepare_info;

                            prepare_info = sysMalloc(sizeof(PrepareInfo));
                            prepare_info->operand = operand;
                            operand.pntr = prepare_info;
                            opcode = OPC_INLINE_REWRITER;
                            prepare_info->block = block;
                        }

                        if(join_blocks) {
                            if((block->prev = last_block) != NULL)
                                last_block->next = block;
                            last_block = info[pc] & END ? NULL : block;
                        } else
                            block->prev = NULL;

                        block->next = NULL;
                        block->u.profile.profiled = NULL;

                        block->u.profile.quickened = block_quickened;
                        block_quickened = FALSE;

                        block->length = block_len;
                        block->start = &new_code[ins_start];
                        block->opcodes = sysMalloc(block_len * sizeof(OpcodeInfo));
                        memcpy(block->opcodes, &opcodes[ins_start], block_len * sizeof(OpcodeInfo));
                    }

                    block_start = info[pc] == END ? -1 : pc;
                }
#endif
                /* Store the new instruction */
                new_code[ins_count].handler = handlers[ins_cache][opcode];
                new_code[ins_count].operand = operand;
            }
        }
    }

    /* Update the method's line number and exception tables
      with the new instruction offsets */

    for(i = 0; i < mb->line_no_table_size; i++) {
        LineNoTableEntry *entry = &mb->line_no_table[i];
        entry->start_pc = map[entry->start_pc];
    }

    for(i = 0; i < mb->exception_table_size; i++) {
        ExceptionTableEntry *entry = &mb->exception_table[i];
        entry->start_pc = map[entry->start_pc];
        entry->end_pc = map[entry->end_pc];
        entry->handler_pc = map[entry->handler_pc];
    }

    /* Update the method with the new code, and
       mark the method as being prepared. */

    mb->code = new_code;
    mb->code_size = ins_count;

    lockVMWaitLock(prepare_lock, self);
    mb->state = MB_PREPARED;

    notifyAllVMWaitLock(prepare_lock, self);
    unlockVMWaitLock(prepare_lock, self);
    enableSuspend(self);

    /* We don't need the old bytecode stream anymore */
    if(!(mb->access_flags & (ACC_ABSTRACT | ACC_MIRANDA)))
        sysFree(code);
}
#endif
