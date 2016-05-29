/*
 * Copyright (C) 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define PREPARE_MB(mb)                        \
    if(mb->state < MB_PREPARED)               \
        prepare(mb, handlers)

/* Macros for handler/bytecode rewriting */

#ifdef USE_CACHE
#define OPCODE_CHANGED(opcode)                             \
(                                                          \
    pc->handler != L(opcode, 0, ENTRY) &&                  \
    pc->handler != L(opcode, 1, ENTRY) &&                  \
    pc->handler != L(opcode, 2, ENTRY)                     \
)

#else /* USE_CACHE */

#define OPCODE_CHANGED(opcode)                             \
(                                                          \
    pc->handler != L(opcode, 0, ENTRY)                     \
)
#endif

#define WITH_OPCODE_CHANGE_CP_DINDEX(opcode, index, cache) \
{                                                          \
    index = pc->operand.uui.u1;                            \
    cache = pc->operand.uui.i;                             \
    MBARRIER();                                            \
    if(OPCODE_CHANGED(opcode))                             \
        goto *pc->handler;                                 \
}

#define ARRAY_TYPE(pc)            pc->operand.i
#define SINGLE_INDEX(pc)          pc->operand.i
#define DOUBLE_INDEX(pc)          pc->operand.i
#define SINGLE_SIGNED(pc)         pc->operand.i
#define DOUBLE_SIGNED(pc)         pc->operand.i
#define IINC_LVAR_IDX(pc)         pc->operand.ii.i1
#define IINC_DELTA(pc)            pc->operand.ii.i2
#define INV_QUICK_ARGS(pc)        pc->operand.uu.u1
#define INV_QUICK_IDX(pc)         pc->operand.uu.u2
#define INV_INTF_IDX(pc)          pc->operand.uu.u1
#define INV_INTF_CACHE(pc)        pc->operand.uu.u2
#define MULTI_ARRAY_DIM(pc)       pc->operand.uui.u2
#define GETFIELD_THIS_OFFSET(pc)  pc->operand.i
#define RESOLVED_CONSTANT(pc)     pc->operand.u
#define RESOLVED_FIELD(pc)        ((FieldBlock*)pc->operand.pntr)
#define RESOLVED_METHOD(pc)       ((MethodBlock*)pc->operand.pntr)
#define RESOLVED_POLYMETHOD(pc)   ((PolyMethodBlock*)pc->operand.pntr)
#define RESOLVED_INVDYNMETHOD(pc) ((InvDynMethodBlock*)pc->operand.pntr)
#define RESOLVED_CLASS(pc)        (Class *)CP_INFO(cp, pc->operand.uui.u1)
#define INTRINSIC_ARGS(pc)        pc->operand.i
