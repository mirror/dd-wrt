/*
 * Copyright (C) 2008, 2009 Robert Lougher <rob@jamvm.org.uk>.
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

.text
.align  2
.globl _callJNIMethod

/*
 * Arguments passed in:
 *
 * %rdi JNIEnv
 * %rsi class or NULL
 * %rdx sig
 * %rcx extra arg
 * %r8  ostack
 * %r9  function pntr
 */

_callJNIMethod:
    pushq   %rbp
    movq    %rsp, %rbp

    pushq   %r8
    pushq   %r9
    pushq   %rbx
    pushq   %r12
    pushq   %r14
    pushq   %r15

    testq   %rsi, %rsi
    jne     alloc_space

    movq    (%r8), %rsi
    addq    $8, %r8

alloc_space:
    subq    %rcx, %rsp

/* Setup argument registers:
 * rdx, rcx, r8, r9
 * xmm0 - xmm7
 *
 * local register usage:
 *  r10 = int reg count
 *  r11 = double handler
 *  r12 = float handler
 *  r14 = sig pntr
 *  r15 = stack pntr
 *  rbx = ostack pntr
 */

    leaq    double_regs-10(%rip), %r11
    leaq    float_regs-10(%rip), %r12
    leaq    1(%rdx), %r14
    movq    %rsp, %r15
    xorq    %r10, %r10
    movq    %r8, %rbx

next:
    movzbq  (%r14), %rax
    incq    %r14

    cmpq    $41, %rax                /* ')' */
    je      done
    cmpq    $68, %rax                /* 'D' */
    je      double
    cmpq    $70, %rax                /* 'F' */
    je      float
    cmpq    $74, %rax                /* 'J' */
    je      long

skip_brackets:
    cmpq    $91, %rax                /* '[' */
    jne     check_ref
    movzbq  (%r14), %rax
    incq    %r14
    jmp     skip_brackets

check_ref:
    cmpq    $76, %rax                /* 'L' */
    jne     int

skip_ref:
    movzbq  (%r14), %rax
    incq    %r14
    cmpq    $59, %rax                /* ';' */
    jne     skip_ref

int:
    movq    (%rbx), %rax
    addq    $8, %rbx

check_regs:
    incq    %r10
    cmpq    $2, %r10
    jg      check_4

    cmovneq %rax, %rdx
    cmoveq  %rax, %rcx
    jmp     next

check_4:
    cmpq    $4, %r10
    jg      stack_push

    cmovneq %rax, %r8
    cmoveq  %rax, %r9
    jmp     next

stack_push:
    movq    %rax, (%r15)
    addq    $8, %r15
    jmp     next

long:
    movq    (%rbx), %rax
    addq    $16, %rbx
    jmp     check_regs

double:
    addq    $10, %r11
    addq    $10, %r12
    addq    $16, %rbx
    jmp     *%r11

double_regs:
    movsd   -16(%rbx), %xmm0
    jmp     next
    movsd   -16(%rbx), %xmm1
    jmp     next
    movsd   -16(%rbx), %xmm2
    jmp     next
    movsd   -16(%rbx), %xmm3
    jmp     next
    movsd   -16(%rbx), %xmm4
    jmp     next
    movsd   -16(%rbx), %xmm5
    jmp     next
    movsd   -16(%rbx), %xmm6
    jmp     next
    movsd   -16(%rbx), %xmm7
    jmp     next
    movq    -16(%rbx), %rax
    subq    $10, %r11
    subq    $10, %r12
    jmp     stack_push

float:
    addq    $10, %r11
    addq    $10, %r12
    addq    $8, %rbx
    jmp     *%r12

float_regs:
    movss   -8(%rbx), %xmm0
    jmp     next
    movss   -8(%rbx), %xmm1
    jmp     next
    movss   -8(%rbx), %xmm2
    jmp     next
    movss   -8(%rbx), %xmm3
    jmp     next
    movss   -8(%rbx), %xmm4
    jmp     next
    movss   -8(%rbx), %xmm5
    jmp     next
    movss   -8(%rbx), %xmm6
    jmp     next
    movss   -8(%rbx), %xmm7
    jmp     next
    movl    -8(%rbx), %eax
    subq    $10, %r11
    subq    $10, %r12
    jmp     stack_push

done:
    /* Arguments all setup --
       call the native method */

    callq   *-16(%rbp)

    /* Get back ostack for return value */
    movq    -8(%rbp), %rcx

    /* Handle return type */

    movzbq  (%r14), %rbx
    cmpq    $86, %rbx                /* 'V' */
    je      return
    cmpq    $76, %rbx                /* 'L' */
    je      int_ret
    cmpq    $91, %rbx                /* '[' */
    je      int_ret
    cmpq    $73, %rbx                /* 'I' */
    je      int_ret
    cmpq    $68, %rbx                /* 'D' */
    je      double_ret
    cmpq    $70, %rbx                /* 'F' */
    je      float_ret
    cmpq    $74, %rbx                /* 'J' */
    je      long_ret
    cmpq    $67, %rbx                /* 'C' */
    je      char_ret
    cmpq    $83, %rbx                /* 'S' */
    je      short_ret

    /* remaining types Z and B */
    movsbq  %al, %rax

int_ret:
    movq    %rax, (%rcx)
    addq    $8, %rcx

return:
    movq    -24(%rbp), %rbx
    movq    -32(%rbp), %r12
    movq    -40(%rbp), %r14
    movq    -48(%rbp), %r15

    movq    %rbp, %rsp
    popq    %rbp

    movq    %rcx, %rax
    retq

char_ret:
    movzwq  %ax, %rax
    jmp     int_ret

short_ret:
    movswq  %ax, %rax
    jmp     int_ret

long_ret:
    movq    %rax, (%rcx)
    addq    $16, %rcx
    jmp     return

double_ret:
    movsd   %xmm0, (%rcx)
    addq    $16, %rcx
    jmp     return

float_ret:
    movss   %xmm0, (%rcx)
    addq    $8, %rcx
    jmp     return
