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
        .text
        .align 2
        .globl callJNIMethod
	.ent callJNIMethod
	.type callJNIMethod, @function

/* On function entry parameters:
$4 = JNIEnv
$5 = Class if static or NULL
$6 = sig
$7 = extra args
16(sp) = operand stack
20(sp) = func pntr
*/

callJNIMethod:
	.frame $fp,8,$0
	.mask 0x00000000,0
	.fmask 0x00000000,0

	/* Fixed part of frame */
	subu $sp,8

	/* save registers */
	sw $31,4($sp)
	sw $fp,0($sp)

	/* set frame pointer to bottom of fixed frame */
	move $fp,$sp

	/* create variable frame area */
	subu $sp,$7

	/* setup operand and args stack pntrs */
	lw $8,24($fp)
	addu $9,$sp,8

        /* if instance method set second argument
           to object pntr  */

	bne $5,0,scan_sig
	lw $5,0($8)
	addu $8,4

scan_sig:
	lbu $7,1($6)
	addu $6,1

	beq $7,41,done    /* ')' */

	lw $10,0($8)
	addu $8,4

	beq $7,68,double  /* 'D' */
	beq $7,74,double  /* 'J' */

skip_brackets:
	bne $7,91,check_ref /* '[' */
	lbu $7,1($6)
	addu $6,1
	j skip_brackets

check_ref:
	bne $7,76,single /* 'L' */

skip_ref:
	lbu $7,1($6)
	addu $6,1
	bne $7,59,skip_ref /* ';' */
	
single:
	sw $10,0($9)
	addu $9,4
	j scan_sig

double:
	/* Ensure address is 8-byte aligned */
	addu $9,7
	and $9,~7

	sw $10,0($9)
	lw $7,0($8)
	addu $8,4
	sw $7,4($9)
	addu $9,8
	j scan_sig

done:
	/* store pntr to sig return value */
	sw $6,16($fp)

	/* arguments setup, load the argument
	   registers and call the function */

	lw $6, 8($sp)
	lw $7,12($sp)
	
	/* load function pntr */
	lw $25,28($fp)

	/* call the function */
	jalr $25

	/* restore return sig pntr */
	lw $6,16($fp)

	/* Reload operand stack pntr to store
	   return value */
	lw $8,24($fp)

	/* Get return type */
	lbu $9,1($6)

	beq $9,86,return     /* 'V' */
	beq $9,74,ret_long   /* 'J' */
	beq $9,68,ret_double /* 'D' */

#ifdef __mips_hard_float
	beq $9,70,ret_float  /* 'F' */
#endif

	/* int */
	sw $2,0($8)
	addu $8,4
	j return

#ifdef __mips_hard_float
ret_float:
	swc1 $f0,0($8)
	addu $8,4
	j return
#endif

ret_double:
#ifdef __mips_hard_float
	swc1 $f0,0($8)
	swc1 $f1,4($8)
	addu $8,8
	j return
#endif

ret_long:
	sw $2,0($8)
	sw $3,4($8)
	addu $8,8

return:
	/* restore saved registers */

	move $sp,$fp
	lw $31,4($sp)
	lw $fp,0($sp)

	/* pop frame */
	addu $sp,$sp,8

	/* return value is operand stack pntr */
	move $2,$8

	j $31
	.end callJNIMethod
#endif
