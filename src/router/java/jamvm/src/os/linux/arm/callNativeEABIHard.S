/*
 * Copyright (C) 2011, 2012 Robert Lougher <rob@jamvm.org.uk>.
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

        .text
        .syntax unified
        .arch armv7-a
        .thumb
        .align  2
        .global callJNIMethod
        .type   callJNIMethod,function

/*
 * Arguments passed in:
 *
 * r0 JNIEnv
 * r1 class or NULL
 * r2 sig
 * r3 extra arg
 * sp + 0 ostack
 * sp + 4 function pntr
 * sp + 8 args count
 */

/* Register usage :
 *
 * r11 function pntr
 * lr ostack pntr
 * ip args pntr
 * r8 sig pntr
 * r10 extra stack
 * r6 fp backfill 
 * r4 fp reg
 * r7 int reg
 * r5 scratch
 * r3, r2 outgoing int args
 * r1 outgoing class or this pntr
 * r0 outgoing JNIEnv (as passed in)
 *
 * s0 - s16 (d0 - d7) outgoing float args
 */

callJNIMethod:
        stmfd   sp!, {r4, r5, r6, r7, r8, r10, r11, lr}
        ldr     lr, [sp, #32]           /* get ostack pntr */
        ldr     r11, [sp, #36]          /* get function pntr */

        cmp     r1, #0                  /* is method non-static? */
        it      eq
        ldreq   r1, [lr], #4            /* yes, load r1 with "this" */

        mov     r10, r3
        subs    sp, sp, r3              /* allocate room for stacked */
        add     r8, r2, #1              /* init sig pntr -- skipping '(' */

        mov     ip, sp                  /* init loop pntr */

        mov     r7, #2
        mov     r4, #16
        mov     r6, #0

scan_sig:
        ldrb    r5, [r8], #1

        cmp     r5, #41                 /* ')' */
        beq     done

        cmp     r5, #70                 /* 'F' */
        beq     float

        cmp     r5, #68                 /* 'D' */
        beq     double

        cmp     r5, #74                 /* 'J' */
        beq     long

skip_brackets:
        cmp     r5, #91                 /* '[' */
        itt     eq
        ldrbeq  r5, [r8], #1
        beq     skip_brackets

        cmp     r5, #76                 /* 'L' */
        bne     int

skip_ref:
        ldrb    r5, [r8], #1
        cmp     r5, #59                 /* ';' */
        bne     skip_ref

int:
        cbz     r7, stack_int

        subs    r7, r7, #1
        ite     ne
        ldrne   r2, [lr], #4
        ldreq   r3, [lr], #4

        b       scan_sig

float:
        cbz     r6, no_backfill

        sub     r5, r6, #1
        mov     r6, #0
        b       load_float

no_backfill:
        cbz     r4, stack_int
        
        sub     r4, r4, #1
        mov     r5, r4

load_float:
        add     lr, lr, #4
        tbb     [pc, r5]

float_table:
        .byte (s15-float_table)/2
        .byte (s14-float_table)/2
        .byte (s13-float_table)/2
        .byte (s12-float_table)/2
        .byte (s11-float_table)/2
        .byte (s10-float_table)/2
        .byte (s9-float_table)/2
        .byte (s8-float_table)/2
        .byte (s7-float_table)/2
        .byte (s6-float_table)/2
        .byte (s5-float_table)/2
        .byte (s4-float_table)/2
        .byte (s3-float_table)/2
        .byte (s2-float_table)/2
        .byte (s1-float_table)/2
        .byte (s0-float_table)/2

stack_int:
        ldr     r5, [lr], #4
        str     r5, [ip], #4
        b       scan_sig

s0:
        vldr    s0, [lr, #-4]
        b       scan_sig
s1:
        vldr    s1, [lr, #-4]
        b       scan_sig
s2:
        vldr    s2, [lr, #-4]
        b       scan_sig
s3:
        vldr    s3, [lr, #-4]
        b       scan_sig
s4:
        vldr    s4, [lr, #-4]
        b       scan_sig
s5:
        vldr    s5, [lr, #-4]
        b       scan_sig
s6:
        vldr    s6, [lr, #-4]
        b       scan_sig
s7:
        vldr    s7, [lr, #-4]
        b       scan_sig
s8:
        vldr    s8, [lr, #-4]
        b       scan_sig
s9:
        vldr    s9, [lr, #-4]
        b       scan_sig
s10:
        vldr    s10, [lr, #-4]
        b       scan_sig
s11:
        vldr    s11, [lr, #-4]
        b       scan_sig
s12:
        vldr    s12, [lr, #-4]
        b       scan_sig
s13:
        vldr    s13, [lr, #-4]
        b       scan_sig
s14:
        vldr    s14, [lr, #-4]
        b       scan_sig
s15:
        vldr    s15, [lr, #-4]
        b       scan_sig

long:
        cmp     r7, #2
        mov     r7, #0
        bne     stack_long

        ldmia   lr!, {r2, r3}
        b       scan_sig

double:
        lsrs    r5, r4, #1
        it      cs
        movcs   r6, r4

        lsls    r4, r5, #1
        beq     stack_double

        sub     r4, r4, #2
        add     lr, lr, #8
        tbb     [pc, r5]

double_table:
        .byte 0
        .byte (d7-double_table)/2
        .byte (d6-double_table)/2
        .byte (d5-double_table)/2
        .byte (d4-double_table)/2
        .byte (d3-double_table)/2
        .byte (d2-double_table)/2
        .byte (d1-double_table)/2
        .byte (d0-double_table)/2
        .align 2
d0:
        vldr    d0, [lr, #-8]
        b       scan_sig
d1:
        vldr    d1, [lr, #-8]
        b       scan_sig
d2:
        vldr    d2, [lr, #-8]
        b       scan_sig
d3:
        vldr    d3, [lr, #-8]
        b       scan_sig
d4:
        vldr    d4, [lr, #-8]
        b       scan_sig
d5:
        vldr    d5, [lr, #-8]
        b       scan_sig
d6:
        vldr    d6, [lr, #-8]
        b       scan_sig
d7:
        vldr    d7, [lr, #-8]
        b       scan_sig

stack_double:
        mov     r6, #0

stack_long:
        /* Ensure address is 8 byte aligned */
        add     ip, ip, #7
        bic     ip, ip, #7
        
        ldr     r5, [lr], #4
        str     r5, [ip], #4
        ldr     r5, [lr], #4
        str     r5, [ip], #4
        b       scan_sig

done:
        /* Call the function */
        blx     r11

        add     sp, sp, r10             /* Pop argument area */

        ldr     r4, [sp, #32]           /* Reload ostack for */
                                        /* address of return value */

        ldrb    r5, [r8]                /* Return type */

        cmp     r5, #86                 /* 'V' */
        beq     return

        cmp     r5, #68                 /* 'D' */
        beq     return_double

        cmp     r5, #70                 /* 'F' */
        beq     return_float

        str     r0, [r4], #4            /* Low word */

        cmp     r5, #74                 /* 'J' */
        it      eq
        streq   r1, [r4], #4            /* High word */

return:
        mov     r0, r4                  /* return ostack */
        ldmfd   sp!, {r4, r5, r6, r7, r8, r10, r11, lr}
        bx      lr

return_float:
        vstr    s0, [r4]
        add     r4, r4, #4
        b       return

return_double:
        vstr    d0, [r4]
        add     r4, r4, #8
        b       return
