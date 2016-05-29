;
; Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
; Robert Lougher <rob@jamvm.org.uk>.
;
; This file is part of JamVM.
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2,
; or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
;

.section __TEXT,__text,regular,pure_instructions
        .align 2
        .globl _callJNIMethod

;########################################################
; Function called with arguments
; r3 = JNIEnv
; r4 = Class if static or NULL 
; r5 = sig
; r6 = extra args
; r7 = stack
; r8 = func pntr
;
; Registers used as follows :
; r0 general scratch
; r3,r4 passed through to native func
; r5-r10 first 6 integer stack args passed to func
; fp1-fp13 first 13 float/double args passed to func
; r11 holds high word for long arg handling
; r12 points to stack area for args which overflow regs
; r14 signature pointer
; r15 operand stack pointer
; r16 saved stack pntr for return arg
; r17 jump address for next integer register
; r18 jump address for next float register
; r19 jump address for next long register pair
; r20 gpr register count

_callJNIMethod:
        mr r11,r1
        stdux r11, r1, r6

        mflr r0
        std r0,16(r11)
        std r14,-8(r11)
        std r15,-16(r11)
        std r16,-24(r11)
        std r17,-32(r11)
        std r18,-40(r11)
        std r19,-48(r11)
        std r20,-56(r11)

        ; setup signature and stack pntrs
        mr r14,r5
        addi r15,r7,-8

        ; save pntr to first stack arg for return value
        mr r16,r15

        ; if instance method set r4 to object pntr
        ; (first stack argument)
        cmpdi cr0,r4,0
        bne static

        ldu r4,8(r15)

static:
        bl get_pc

get_pc:
        mflr r20
        la r17,lo16(other0-get_pc)(r20)
        la r18,lo16(float0-get_pc)(r20)

        ; Set lr to function ptr, for calling later        
        mtlr r8

        ; Setup pointer to parameter area for args which
        ; do not fit into registers
        addi r12,r1,104

        ; Initialise gpr register count
        li r20,0

next:
        mtctr r17
        lbzu r0,1(r14)

        cmpi cr0,r0,41 ; ')'
        cmpi cr1,r0,68 ; 'D'
        cmpi cr5,r0,70 ; 'F'
        cmpi cr6,r0,74 ; 'J'

        beq cr0,finish
        beq cr1,do_double
        beq cr5,do_float
        beq cr6,do_long

skip_brackets:
        cmpi cr0,r0,91 ; '['
        bne out
        lbzu r0,1(r14)
        beq skip_brackets

out:
        cmpi cr0,r0,76 ; 'L'
        bne out2

skip_ref:
        lbzu r0,1(r14)
        cmpi cr0,r0,59 ; ';'
        bne skip_ref

out2:
        ldu r0,8(r15)
        cmpi cr0,r20,5
        bgt stack_push

skip_regs:
        addi r20,r20,1
        addi r17,r17,8
        bctr

do_long:
        ld r0,8(r15)
        addi r15,r15,16
        cmpi cr0,r20,5
        ble skip_regs

stack_push:
        stdu r0,8(r12)
        b next

do_double:
        lfd f0,8(r15)
        addi r15,r15,16
        b do_float_1

do_float:
        lfs f0,12(r15)
        addi r15,r15,8

do_float_1:
        mtctr r18
        addi r18,r18,8
        cmpi cr0,r20,5
        ble skip_regs

        addi r12,r12,8
        bctr

other0:
        mr r5,r0
        b next
        mr r6,r0
        b next
        mr r7,r0
        b next
        mr r8,r0
        b next
        mr r9,r0
        b next
        mr r10,r0
        b next

float0:
        fmr f1,f0
        b next
        fmr f2,f0
        b next
        fmr f3,f0
        b next
        fmr f4,f0
        b next
        fmr f5,f0
        b next
        fmr f6,f0
        b next
        fmr f7,f0
        b next
        fmr f8,f0
        b next
        fmr f9,f0
        b next
        fmr f10,f0
        b next
        fmr f11,f0
        b next
        fmr f12,f0
        b next
        fmr f13,f0
        b next

        addi r18,r18,-8
        stfd f0,0(r12)
        b next

finish:
        ; We've set up the args, so now call the function
        blrl

        lbz r0,1(r14)

        cmpi cr0,r0,86 ; 'V'
        cmpi cr1,r0,68 ; 'D'
        cmpi cr5,r0,70 ; 'F'
        cmpi cr6,r0,74 ; 'J'

        beq cr0, return

        beq cr1, ret_double
        beq cr5, ret_float

        stdu r3, 8(r16)
        bne cr6, return

        addi r16,r16,8
        b return

ret_double:
        stfdu f1,8(r16)
        addi r16,r16,8
        b return

ret_float:
        stfs f1,12(r16)
        addi r16,r16,8

return:
        addi r3,r16,8

        ld r11,0(r1)
        ld r0,16(r11)
        mtlr r0

        ld r14,-8(r11)
        ld r15,-16(r11)
        ld r16,-24(r11)
        ld r17,-32(r11)
        ld r18,-40(r11)
        ld r19,-48(r11)
        ld r20,-56(r11)
        
        mr r1,r11
        blr
