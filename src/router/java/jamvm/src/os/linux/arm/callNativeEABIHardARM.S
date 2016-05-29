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
 * lr ostack pntr
 * ip scratch
 * r11 function pntr
 * r10 fp backfill 
 * r8 fp reg
 * r7 int reg
 * r6 args pntr
 * r5 sig pntr
 * r4 extra stack
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
        ldreq   r1, [lr], #4            /* yes, load r1 with "this" */

        mov     r4, r3
        sub     sp, sp, r4              /* allocate room for stacked */
        add     r5, r2, #1              /* init sig pntr -- skipping '(' */

        mov     r6, sp                  /* init loop pntr */

        mov     r7, #2
        mov     r8, #16
        mov     r10, #0

scan_sig:
        ldrb    ip, [r5], #1

        cmp     ip, #41                 /* ')' */
        beq     done

        cmp     ip, #70                 /* 'F' */
        beq     float

        cmp     ip, #68                 /* 'D' */
        beq     double

        cmp     ip, #74                 /* 'J' */
        beq     long

skip_brackets:
        cmp     ip, #91                 /* '[' */
        ldreqb  ip, [r5], #1
        beq     skip_brackets

        cmp     ip, #76                 /* 'L' */
        bne     int

skip_ref:
        ldrb    ip, [r5], #1
        cmp     ip, #59                 /* ';' */
        bne     skip_ref

int:
        cmp     r7, #0
        beq     stack_int

        subs    r7, r7, #1

        ldrne   r2, [lr], #4
        ldreq   r3, [lr], #4

        b       scan_sig

stack_int:
        ldr     ip, [lr], #4
        str     ip, [r6], #4
        b       scan_sig

float:
        cmp     r10, #0
        beq     no_backfill

        sub     ip, r10, #1
        mov     r10, #0
        b       load_float

no_backfill:
        cmp     r8, #0
        beq     stack_int
        
        sub     r8, r8, #1
        mov     ip, r8

load_float:
        add     lr, lr, #4
        add     pc, pc, ip, lsl #3
        nop

        flds    s15, [lr, #-4]
        b       scan_sig
        flds    s14, [lr, #-4]
        b       scan_sig
        flds    s13, [lr, #-4]
        b       scan_sig
        flds    s12, [lr, #-4]
        b       scan_sig
        flds    s11, [lr, #-4]
        b       scan_sig
        flds    s10, [lr, #-4]
        b       scan_sig
        flds    s9, [lr, #-4]
        b       scan_sig
        flds    s8, [lr, #-4]
        b       scan_sig
        flds    s7, [lr, #-4]
        b       scan_sig
        flds    s6, [lr, #-4]
        b       scan_sig
        flds    s5, [lr, #-4]
        b       scan_sig
        flds    s4, [lr, #-4]
        b       scan_sig
        flds    s3, [lr, #-4]
        b       scan_sig
        flds    s2, [lr, #-4]
        b       scan_sig
        flds    s1, [lr, #-4]
        b       scan_sig
        flds    s0, [lr, #-4]
        b       scan_sig

long:
        cmp     r7, #2
        mov     r7, #0
        bne     stack_long

        ldr     r2, [lr], #4
        ldr     r3, [lr], #4
        b       scan_sig

double:
        lsrs    ip, r8, #1
        movcs   r10, r8

        lsls    r8, ip, #1
        beq     stack_double

        sub     r8, r8, #2
        add     lr, lr, #8
        add     pc, pc, ip, lsl #3
        nop
        nop
        nop

        fldd    d7, [lr, #-8]
        b       scan_sig
        fldd    d6, [lr, #-8]
        b       scan_sig
        fldd    d5, [lr, #-8]
        b       scan_sig
        fldd    d4, [lr, #-8]
        b       scan_sig
        fldd    d3, [lr, #-8]
        b       scan_sig
        fldd    d2, [lr, #-8]
        b       scan_sig
        fldd    d1, [lr, #-8]
        b       scan_sig
        fldd    d0, [lr, #-8]
        b       scan_sig

stack_double:
        mov     r10, #0

stack_long:
        /* Ensure address is 8 byte aligned */
        add     r6, r6, #7
        bic     r6, r6, #7
        
        ldr     ip, [lr], #4
        str     ip, [r6], #4
        ldr     ip, [lr], #4
        str     ip, [r6], #4
        b       scan_sig

done:
        /* Call the function */
#if defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__)
        mov     lr, pc
        bx      r11
#else
        blx     r11
#endif

        add     sp, sp, r4              /* Pop argument area */
        ldr     r4, [sp, #32]           /* Reload ostack for */
                                        /* address of return value */
        ldrb    ip, [r5]                /* Return type */

        cmp     ip, #86                 /* 'V' */
        beq     return

        cmp     ip, #68                 /* 'D' */
        beq     return_double

        cmp     ip, #70                 /* 'F' */
        beq     return_float

        str     r0, [r4], #4            /* Low word */

        cmp     ip, #74                 /* 'J' */
        streq   r1, [r4], #4            /* High word */

return:
        mov     r0, r4                  /* return ostack */
        ldmfd   sp!, {r4, r5, r6, r7, r8, r10, r11, lr}
        bx      lr

return_float:
        add     r4, r4, #4
        fsts    s0, [r4, #-4]
        b       return

return_double:
        add     r4, r4, #8
        fstd    d0, [r4, #-8]
        b       return
