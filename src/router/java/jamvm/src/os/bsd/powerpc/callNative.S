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

#include  "config.h"

#ifndef USE_FFI
        .section ".text"
        .align 2
        .globl callJNIMethod

/********************************************************
 Function called with arguments
 r3 = JNIEnv
 r4 = Class if static or NULL 
 r5 = sig
 r6 = extra args
 r7 = stack
 r8 = func pntr

 Registers used as follows :
 r0 general scratch
 r3,r4 passed through to native func
 r5-r10 first 6 integer stack args passed to func
 fp1-fp8 first 8 float/double args passed to func
 r11 holds high word for long arg handling
 r12 points to stack area for args which overflow regs
 r14 signature pointer
 r15 operand stack pointer
 r16 saved stack pntr for return arg
 r17 jump address for next integer register
 r18 jump address for next float register
 r19 jump address for next long register pair
*/

callJNIMethod:
        stwu 1,-32(1)
        mflr 0
        stw 0,36(1)
        stw 14,28(1)
        stw 15,24(1)
        stw 16,20(1)
        stw 17,16(1)
        stw 18,12(1)
        stw 19,8(1)
        
        /* Set lr to function ptr, for calling later */
        mtlr 8

        /* setup signature and stack pntrs */
        mr 14,5
        addi 15,7,-4

        /* save pntr to first stack arg for return value */
        mr 16,15

        /* if instance method set r4 to object pntr
           (first stack argument) */
        cmpi 0,4,0
        bne static
        lwzu 4,4(15)

static:
        /* Calc bytes for extra args, round up to 16 bytes
           and adjust stack pointer */

        cmpi 0,6,0
        beq none

        addi 6,6,3
        lwz 0,0(1)
        rlwinm 6,6,2,0,27
        neg 6,6
        stwux 0,1,6
        addi 12,1,4

none:
        lis 17,other0@ha
        la 17,other0@l(17)
        lis 18,float0@ha
        la 18,float0@l(18)
        lis 19,long0@ha
        la 19,long0@l(19)

next:
        mtctr 17
        lbzu 0,1(14)

        cmpi 0,0,')'
        cmpi 1,0,'D'
        cmpi 5,0,'F'
        cmpi 6,0,'J'

        beq 0,finish
        beq 1,do_double
        beq 5,do_float
        beq 6,do_long

skip_brackets:
        cmpi 0,0,'['
        bne out
        lbzu 0,1(14)
        beq skip_brackets

out:
        cmpi 0,0,'L'
        bne out2

skip_ref:
        lbzu 0,1(14)
        cmpi 0,0,';'
        bne skip_ref

out2:
        addi 17,17,8
        addi 19,19,12
        lwzu 0,4(15)
        bctr

do_long:
        mtctr 19
        lwzu 0,4(15)
        addi 17,17,16
        addi 19,19,24
        lwzu 11,4(15)
        bctr

do_double:
        mtctr 18
        lfdu 0,4(15)
        addi 18,18,8
        addi 15,15,4
        bctr

do_float:
        mtctr 18
        lfsu 0,4(15)
        addi 18,18,8
        bctr

other0:
        mr 5,0
        b next
        mr 6,0
        b next
        mr 7,0
        b next
        mr 8,0
        b next
        mr 9,0
        b next
        mr 10,0
        b next

        addi 17,17,-8
        addi 19,19,-12
        stwu 0,4(12)
        b next

long0:
        mr 5,0
        mr 6,11
        b next
        addi 17,17,8
        addi 19,19,12
        nop
        mr 7,0
        mr 8,11
        b next
        addi 17,17,8
        addi 19,19,12
        nop
        mr 9,0
        mr 10,11
        b next
        addi 17,17,8
        addi 19,19,12
        nop

        addi 17,17,-16
        addi 19,19,-24
        ori 12,12,4
        stwu 0,4(12)
        stwu 11,4(12)
        b next

float0:
        fmr 1,0
        b next
        fmr 2,0
        b next
        fmr 3,0
        b next
        fmr 4,0
        b next
        fmr 5,0
        b next
        fmr 6,0
        b next
        fmr 7,0
        b next
        fmr 8,0
        b next

        addi 18,18,-8
        cmpi 0,0,'F'
        bne store_double
        stfsu 0,4(12)
        b next

store_double:
        ori 12,12,4
        stfdu 0,4(12)
        addi 12,12,4
        b next

finish:
        /* We've set up the args, so now call the function */

        blrl

        /* Handle return value */

        lbz 0,1(14)

        cmpi 0,0,'V'
        cmpi 1,0,'D'
        cmpi 5,0,'F'
        cmpi 6,0,'J'

        beq 0, return

        beq 1, ret_double
        beq 5, ret_float

        stwu 3, 4(16)
        bne 6, return

        stwu 4, 4(16)
        b return

ret_double:
        stfdu 1,4(16)
        addi 16,16,4
        b return

ret_float:
        stfsu 1,4(16)

return:
        addi 3,16,4

        lwz 11,0(1)
        lwz 0,4(11)
        mtlr 0

        lwz 14,-4(11)
        lwz 15,-8(11)
        lwz 16,-12(11)
        lwz 17,-16(11)
        lwz 18,-20(11)
        lwz 19,-24(11)
        
        mr 1,11
        blr
#endif
