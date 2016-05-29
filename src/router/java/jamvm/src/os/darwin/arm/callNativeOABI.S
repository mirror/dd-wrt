/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
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
        .global _callJNIMethod

_callJNIMethod:
        mov     ip, sp
        stmfd   sp!, {r4, r5, ip, lr}
        mov     r4, ip
        mov     r5, r3                  @ save native_arg
        ldr     ip, [r4]                @ get ostack pntr
        ldr     r2, [r4, #8]            @ get args_count
        cmp     r1, #0                  @ is method static? 
        ldreq   r1, [ip], #4            @ no, load r0 with "this"
        addne   r2, r2, #1              @ yes, add 1 to cnt for "class" 
        subs    r2, r2, #3              @ no. of args fit in registers?
        ble     do_call                 @ yes, skip stack push
        add     lr, ip, #4

        @ loop pushing extra args from ostack
        @ onto native stack.
        @ lr = ostack pntr, r2 = loop counter
loop:
        ldr     r3, [lr, r2, lsl #2]
        str     r3, [sp, #-4]!
        subs    r2, r2, #1
        bne     loop

do_call:
        @ load last two registers in calling
        @ convention.  Doesn't matter if args < 4,
        @ faster than checking and safe to load rubbish

        ldr     r2, [ip]
        ldr     r3, [ip, #4]

        mov     lr, pc
        ldr     pc, [r4, #4]            @ call the function...

        @ reload ostack pntr for pushing return value
        ldr     r2, [r4]

        @ native_arg holds return type (set in nativeExtraArg)
        @ 0 = void, 1 = double, 2 = float, 3 = long
        @ 4 = everything else

        @ values arranged so that if it's >= 3, it's "int" or
        @ long, so push low word - r0 is now free so we can
        @ put opntr into it, for all types

#if defined(__VFP_FP__) || defined(__SOFTFP__)
        cmp     r5, #1
#else
        cmp     r5, #3
#endif
        strge   r0, [r2], #4
        mov     r0, r2

        @ optimise for most often case of "int" return - all others
        @ jump to 2 instruction "handler", but this falls through

        addle   pc, pc, r5, lsl #3

        @ room for one instruction here - enough to just return, all
        @ we need, as everything's been done above

        ldmdb   r4, {r4, r5, sp, pc}
        
        @ void return
        ldmdb   r4, {r4, r5, sp, pc}
        nop
        
#if !defined(__VFP_FP__) && !defined(__SOFTFP__)
        @ double return
        stfd    f0, [r0], #8
        ldmdb   r4, {r4, r5, sp, pc}

        @ float return
        stfs    f0, [r0], #4
        ldmdb   r4, {r4, r5, sp, pc}
#endif

        @long return
        str     r1, [r0], #4
        ldmdb   r4, {r4, r5, sp, pc}

