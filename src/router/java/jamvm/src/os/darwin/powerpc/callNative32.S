;
; Copyright (C) 2003, 2004, 2005, 2006, 2007
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
        stwux r11, r1, r6

        mflr r0
        stw r0,8(r11)
        stw r14,-4(r11)
        stw r15,-8(r11)
        stw r16,-12(r11)
        stw r17,-16(r11)
        stw r18,-20(r11)
        stw r19,-24(r11)
        stw r20,-28(r11)

        ; setup signature and stack pntrs
        mr r14,r5
        addi r15,r7,-4

        ; save pntr to first stack arg for return value
        mr r16,r15

        ; if instance method set r4 to object pntr
        ; (first stack argument)
        cmpi cr0,r4,0
        bne static

        lwzu r4,4(r15)

static:
        bl get_pc

get_pc:
        mflr r20
        la r17,lo16(other0-get_pc)(r20)
        la r18,lo16(float0-get_pc)(r20)
        la r19,lo16(long0-get_pc)(r20)

        ; Set lr to function ptr, for calling later        
        mtlr r8

        ; Setup pointer to parameter area for args which
        ; do not fit into registers
        addi r12,r1,52

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
        lwzu r0,4(r15)
        cmpi cr0,r20,5
        bgt stack_push_1

skip_regs_1:
        addi r20,r20,1
        addi r17,r17,8
        addi r19,r19,12
        bctr

stack_push_1:
        stwu r0,4(r12)
        b next

do_long:
        mtctr r19
        lwzu r0,4(r15)
        lwzu r11,4(r15)
        cmpi cr0,r20,5
        bgt stack_push_2

skip_regs_2:
        addi r20,r20,2
        addi r17,r17,16
        addi r19,r19,24
        bctr

stack_push_2:
        stwu r0,4(r12)
        stwu r11,4(r12)
        b next

do_double:
        mtctr r18
        lfdu f0,4(r15)
        addi r18,r18,8
        addi r15,r15,4

        cmpi cr0,r20,5
        ble skip_regs_2

        addi r12,r12,8
        bctr

do_float:
        mtctr r18
        lfsu f0,4(r15)
        addi r18,r18,8

        cmpi cr0,r20,5
        ble skip_regs_1

        addi r12,r12,4
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

long0:
        mr r5,r0
        mr r6,r11
        b next
        mr r6,r0
        mr r7,r11
        b next
        mr r7,r0
        mr r8,r11
        b next
        mr r8,r0
        mr r9,r11
        b next
        mr r9,r0
        mr r10,r11
        b next
        mr r10,r0
        stwu r11,4(r12)
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
        cmpi cr0,r0,70 ; 'F'
        bne store_double
        stfs f0,0(r12)
        b next
store_double:
        stfd f0,-4(r12)
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

        stwu r3, 4(r16)
        bne cr6, return

        stwu r4, 4(r16)
        b return

ret_double:
        stfdu f1,4(r16)
        addi r16,r16,4
        b return

ret_float:
        stfsu f1,4(r16)

return:
        addi r3,r16,4

        lwz r11,0(r1)
        lwz r0,8(r11)
        mtlr r0

        lwz r14,-4(r11)
        lwz r15,-8(r11)
        lwz r16,-12(r11)
        lwz r17,-16(r11)
        lwz r18,-20(r11)
        lwz r19,-24(r11)
        lwz r20,-28(r11)
        
        mr r1,r11
        blr
