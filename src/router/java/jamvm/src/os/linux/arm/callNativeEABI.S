/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2011
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

callJNIMethod:
        stmfd   sp!, {r4, r5, lr}
        ldr     lr, [sp, #12]           /* get ostack pntr */
        ldr     ip, [sp, #16]           /* get function pntr */

        cmp     r1, #0                  /* is method non-static? */
        ldreq   r1, [lr], #4            /* yes, load r1 with "this" */

        orr     r4, r3, #4              /* allocate room for stacked */
        subs    sp, sp, r4              /* args -- keeping stack 8 */
                                        /* byte aligned */
        add     r5, r2, #1              /* init sig pntr -- skipping '(' */

        mov     r2, sp                  /* init loop pntr */

scan_sig:
        ldrb    r3, [r5], #1

        cmp     r3, #41                 /* ')' */
        beq     done

        cmp     r3, #68                 /* 'D' */
        cmpne   r3, #74                 /* 'J' */
        beq     double

skip_brackets:
        cmp     r3, #91                 /* '[' */
        ldreqb  r3, [r5], #1
        beq     skip_brackets

        cmp     r3, #76                 /* 'L' */
        bne     single

skip_ref:
        ldrb    r3, [r5], #1
        cmp     r3, #59                 /* ';' */
        bne     skip_ref

single:
        ldr     r3, [lr], #4
        str     r3, [r2], #4
        b       scan_sig

double:
        /* Ensure address is 8 byte aligned */
        add     r2, r2, #7
        bic     r2, r2, #7
        
        ldr     r3, [lr], #4
        str     r3, [r2], #4
        ldr     r3, [lr], #4
        str     r3, [r2], #4
        b       scan_sig

done:
        /* Load argument registers */
        ldmfd   sp!, {r2, r3}

        /* Call the function */
#if defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__)
        mov     lr, pc
        bx      ip
#else
        blx     ip
#endif

        subs    r4, r4, #8              /* Pop argument area */
                                        /* (minus 8 for r2/r3) */
        add     sp, sp, r4

        ldr     r4, [sp, #12]           /* Reload ostack for */
                                        /* address of return value */

        ldrb    r3, [r5]                /* Return type */

        cmp     r3, #86                 /* 'V' */
        beq     return

        str     r0, [r4], #4            /* Low word */

        cmp     r3, #74                 /* 'J' */
        cmpne   r3, #68                 /* 'D' */
        streq   r1, [r4], #4            /* High word */

return:
        mov     r0, r4                  /* return ostack */
        ldmfd   sp!, {r4, r5, lr}
        bx      lr

