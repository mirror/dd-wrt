//-----------------------------------------------------------------
// Copyright (C) 2003, Gary Thomas <gary@mlbassoc.com>
//-----------------------------------------------------------------

#------------------------------------------------------------------------------
# Easier to read names for the registers

	.equ	r0, 0
	.equ	r1, 1
	.equ	r2, 2
	.equ	r3, 3
	.equ	r4, 4
	.equ	r5, 5
	.equ	r6, 6
	.equ	r7, 7
	.equ	r8, 8
	.equ	r9, 9
	.equ	r10, 10
	.equ	r11, 11
	.equ	r12, 12
	.equ	r13, 13
	.equ	r14, 14
	.equ	r15, 15
	.equ	r16, 16
	.equ	r17, 17
	.equ	r18, 18
	.equ	r19, 19
	.equ	r20, 20
	.equ	r21, 21
	.equ	r22, 22
	.equ	r23, 23
	.equ	r24, 24
	.equ	r25, 25
	.equ	r26, 26
	.equ	r27, 27
	.equ	r28, 28
	.equ	r29, 29
	.equ	r30, 30
	.equ	r31, 31

	.equ	sp, r1

#------------------------------------------------------------------------------
# Some useful coding macros

	# Load immediate word, has to be done with 2 instructions
	.macro	lwi	reg,val
	lis	\reg,\val@H
	ori	\reg,\reg,\val@L
	.endm
