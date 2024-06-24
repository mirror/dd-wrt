/*
 * Copyright (C) 2008, 2009, 2011, 2012 Robert Lougher <rob@jamvm.org.uk>.
 * Copyright (C) 2020 Simon South <simon@simonsouth.net>.
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

#include "config.h"

        .text
        .arch armv8-a
        .align 2
        .global callJNIMethod
        .type callJNIMethod,function

/*
 * Arguments passed in:
 *
 * x0 JNIEnv
 * x1 class or NULL
 * x2 sig
 * w3 extra arg
 * x4 ostack
 * x5 function pntr
 * w6 args count
 */

/* Register usage:
 *
 * x20 ostack
 * x19 sig pntr
 * x16 function pntr
 * x15 ostack pntr
 * x14 args pntr
 * x13 float/double handler
 * x12 int/long handler
 * w11 fp regs remaining
 * w10 int regs remaining
 * x9 scratch
 * x2-x7 outgoing int args
 * x1 outgoing class or this pntr
 * x0 outgoing JNIEnv (as passed in)
 *
 * d0 - d7 outgoing float args
 */

callJNIMethod:
        stp     x29, x30, [sp, #-32]!
        mov     x29, sp
        stp     x19, x20, [x29, #16]

        sub     sp, sp, w3              /* allocate room for stacked args */
        mov     x14, sp

        mov     x20, x4                 /* preserve ostack */
        add     x19, x2, #1             /* init sig pntr -- skipping '(' */

        mov     x16, x5                 /* save function pntr */
        mov     x15, x20                /* init ostack pntr */

        adr     x13, fp_reg_handlers-8
        adr     x12, int_reg_handlers-8

        mov     w11, #8                 /* fp regs remaining */
        mov     w10, #6                 /* int regs remaining */

        cbnz    x1, scan_sig            /* is method non-static? */
        ldr     x1, [x15], #8           /* yes, load x1 with "this" */

scan_sig:
        ldrb    w9, [x19], #1           /* get next sig char */

        cmp     w9, #41                 /* ')' */
        b.eq    done

        cmp     w9, #74                 /* 'J' */
        b.eq    long

        cmp     w9, #70                 /* 'F' */
        b.eq    float

        cmp     w9, #68                 /* 'D' */
        b.eq    double

skip_brackets:
        cmp     w9, #91                 /* '[' */
        b.ne    1f
        ldrb    w9, [x19], #1
        b       skip_brackets
1:
        cmp     w9, #76                 /* 'L' */
        b.ne    int

skip_ref:
        ldrb    w9, [x19], #1
        cmp     w9, #59                 /* ';' */
        b.ne    skip_ref

int:
        ldr     x9, [x15], #8
        cbz     w10, stack_push

load_int_reg:
        sub     w10, w10, #1
        add     x12, x12, #8
        br      x12

int_reg_handlers:
        mov     x2, x9
        b       scan_sig
        mov     x3, x9
        b       scan_sig
        mov     x4, x9
        b       scan_sig
        mov     x5, x9
        b       scan_sig
        mov     x6, x9
        b       scan_sig
        mov     x7, x9
        b       scan_sig

long:
        ldr     x9, [x15], #16
        cbz     w10, stack_push
        b       load_int_reg

float:
        ldr     w9, [x15], #8
        cbz     w11, stack_push
        b       load_fp_reg

double:
        ldr     x9, [x15], #16
        cbz     w11, stack_push

load_fp_reg:
        sub     w11, w11, #1
        add     x13, x13, #8
        br      x13

fp_reg_handlers:
        fmov    d0, x9
        b       scan_sig
        fmov    d1, x9
        b       scan_sig
        fmov    d2, x9
        b       scan_sig
        fmov    d3, x9
        b       scan_sig
        fmov    d4, x9
        b       scan_sig
        fmov    d5, x9
        b       scan_sig
        fmov    d6, x9
        b       scan_sig
        fmov    d7, x9
        b       scan_sig

stack_push:
        str     x9, [x14], #8
        b       scan_sig

done:
        /* Call the function */
        blr     x16

        mov     sp, x29                 /* Pop argument area */

        ldrb    w9, [x19]               /* Return type */

        cmp     w9, #86                 /* 'V' */
        b.eq    return

        cmp     w9, #68                 /* 'D' */
        b.ne    2f
        str     d0, [x20], #16
        b       return
2:
        cmp     w9, #70                 /* 'F' */
        b.ne    3f
        str     s0, [x20], #8
        b       return
3:
        cmp     w9, #74                 /* 'J' */
        b.ne    4f
        str     x0, [x20], #16
        b       return
4:
        str     x0, [x20], #8

return:
        mov     x0, x20                 /* return ostack */

        ldp     x19, x20, [x29, #16]
        ldp     x29, x30, [sp], #32
        ret
