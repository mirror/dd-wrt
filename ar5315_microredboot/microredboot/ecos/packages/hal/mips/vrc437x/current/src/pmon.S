//========================================================================
//
//      pmon.S
//
//      Low-level entry points into PMON monitor.
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg
// Date:          1999-04-21
// Purpose:       
// Description:   Low-level entry points into PMON monitor.
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================


#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

#if __mips < 3
  /* This machine does not support 64-bit operations.  */
  #define ADDU addu
  #define SUBU subu
#else
  /* This machine supports 64-bit operations.  */
  #define ADDU daddu
  #define SUBU dsubu
#endif


/* Standard MIPS register names: */
#define zero	$0
#define z0	$0
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26	/* kernel private register 0 */
#define k1	$27	/* kernel private register 1 */
#define gp	$28	/* global data pointer */
#define sp 	$29	/* stack-pointer */
#define fp	$30	/* frame-pointer */
#define ra	$31	/* return address */
#define pc	$pc	/* pc, used on mips16 */

#define fp0	$f0
#define fp1	$f1

/* Useful memory constants: */
#define K0BASE		0x80000000
#ifndef __mips64
#define K1BASE		0xA0000000
#else
#define K1BASE		0xFFFFFFFFA0000000LL
#endif

#define PHYS_TO_K1(a)   ((unsigned)(a) | K1BASE)

/* Standard Co-Processor 0 register numbers: */
#define C0_COUNT	$9		/* Count Register */
#define C0_SR		$12		/* Status Register */
#define C0_CAUSE	$13		/* last exception description */
#define C0_EPC		$14		/* Exception error address */
#define C0_CONFIG	$16		/* CPU configuration */

/* Standard Status Register bitmasks: */
#define SR_CU1		0x20000000	/* Mark CP1 as usable */
#define SR_FR		0x04000000	/* Enable MIPS III FP registers */
#define SR_BEV		0x00400000	/* Controls location of exception vectors */
#define SR_PE		0x00100000	/* Mark soft reset (clear parity error) */

#define SR_KX		0x00000080	/* Kernel extended addressing enabled */
#define SR_SX		0x00000040	/* Supervisor extended addressing enabled */
#define SR_UX		0x00000020	/* User extended addressing enabled */

/* Standard (R4000) cache operations. Taken from "MIPS R4000
   Microprocessor User's Manual" 2nd edition: */

#define CACHE_I		(0)	/* primary instruction */
#define CACHE_D		(1)	/* primary data */
#define CACHE_SI	(2)	/* secondary instruction */
#define CACHE_SD	(3)	/* secondary data (or combined instruction/data) */

#define INDEX_INVALIDATE		(0)	/* also encodes WRITEBACK if CACHE_D or CACHE_SD */
#define INDEX_LOAD_TAG			(1)
#define INDEX_STORE_TAG			(2)
#define CREATE_DIRTY_EXCLUSIVE		(3)	/* CACHE_D and CACHE_SD only */
#define HIT_INVALIDATE			(4)
#define CACHE_FILL			(5)	/* CACHE_I only */
#define HIT_WRITEBACK_INVALIDATE	(5)	/* CACHE_D and CACHE_SD only */
#define HIT_WRITEBACK			(6)	/* CACHE_I, CACHE_D and CACHE_SD only */
#define HIT_SET_VIRTUAL			(7)	/* CACHE_SI and CACHE_SD only */

#define BUILD_CACHE_OP(o,c)		(((o) << 2) | (c))

/* Individual cache operations: */
#define INDEX_INVALIDATE_I		BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_I)
#define INDEX_WRITEBACK_INVALIDATE_D	BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_D)
#define INDEX_INVALIDATE_SI             BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_SI)
#define INDEX_WRITEBACK_INVALIDATE_SD	BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_SD)

#define INDEX_LOAD_TAG_I		BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_I)
#define INDEX_LOAD_TAG_D                BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_D)
#define INDEX_LOAD_TAG_SI               BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_SI)
#define INDEX_LOAD_TAG_SD               BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_SD)

#define INDEX_STORE_TAG_I              	BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_I)
#define INDEX_STORE_TAG_D               BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_D)
#define INDEX_STORE_TAG_SI              BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_SI)
#define INDEX_STORE_TAG_SD              BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_SD)

#define CREATE_DIRTY_EXCLUSIVE_D        BUILD_CACHE_OP(CREATE_DIRTY_EXCLUSIVE,CACHE_D)
#define CREATE_DIRTY_EXCLUSIVE_SD       BUILD_CACHE_OP(CREATE_DIRTY_EXCLUSIVE,CACHE_SD)

#define HIT_INVALIDATE_I                BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_I)
#define HIT_INVALIDATE_D                BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_D)
#define HIT_INVALIDATE_SI               BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_SI)
#define HIT_INVALIDATE_SD               BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_SD)

#define CACHE_FILL_I                    BUILD_CACHE_OP(CACHE_FILL,CACHE_I)
#define HIT_WRITEBACK_INVALIDATE_D      BUILD_CACHE_OP(HIT_WRITEBACK_INVALIDATE,CACHE_D)
#define HIT_WRITEBACK_INVALIDATE_SD     BUILD_CACHE_OP(HIT_WRITEBACK_INVALIDATE,CACHE_SD)

#define HIT_WRITEBACK_I                 BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_I)
#define HIT_WRITEBACK_D                 BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_D)
#define HIT_WRITEBACK_SD                BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_SD)

#define HIT_SET_VIRTUAL_SI		BUILD_CACHE_OP(HIT_SET_VIRTUAL,CACHE_SI)
#define HIT_SET_VIRTUAL_SD              BUILD_CACHE_OP(HIT_SET_VIRTUAL,CACHE_SD)

	.text
	.align	2

#ifdef LSI
  #define PMON_VECTOR 0xffffffffbfc00200
#else
  #define PMON_VECTOR 0xffffffffbfc00500
#endif

#ifndef __mips_eabi
/* Provide named functions for entry into the monitor: */
#define INDIRECT(name,index)				\
	.globl	name;					\
	.ent	name;					\
	.set	noreorder;				\
name:	la	$2,+(PMON_VECTOR+((index)*4));		\
	lw	$2,0($2);				\
	j	$2;					\
	nop;						\
	.set	reorder;				\
	.end	name

#else
#define INDIRECT(name,index)				\
	.globl	name;					\
	.ent	name;					\
	.set	noreorder;				\
name:	la	$2,+(PMON_VECTOR+((index)*4));		\
	lw	$2,0($2);				\
	SUBU	sp,sp,0x40;				\
	sd	ra,0x38(sp);				\
	sd	fp,0x30(sp);				\
	jal	$2;					\
	move	fp,sp;					\
	ld	ra,0x38(sp);				\
	ld	fp,0x30(sp);				\
	j	ra;					\
	ADDU	sp,sp,0x40;				\
	.set	reorder;				\
	.end	name
#endif


/* The following magic numbers are for the slots into the PMON monitor */
/* The first are used as the lo-level library run-time: */
INDIRECT(pmon_read,0)
INDIRECT(pmon_write,1)
INDIRECT(pmon_open,2)
INDIRECT(pmon_close,3)
/* The following are useful monitor routines: */
INDIRECT(pmon_ioctl,4)
INDIRECT(pmon_printf,5)
INDIRECT(pmon_vsprintf,6)
INDIRECT(pmon_ttctl,7)
INDIRECT(pmon_cliexit,8)
INDIRECT(pmon_getenv,9)
INDIRECT(pmon_onintr,10)
INDIRECT(pmon_flush_cache,11)
INDIRECT(pmon_exception,12)

/* The following routine is required by the "print()" function: */
	.globl	pmon_outbyte
	.ent	pmon_outbyte
	.set	noreorder
pmon_outbyte:
	subu	sp,sp,0x20	/* allocate stack space for string */
	sd	ra,0x18(sp)	/* stack return address */
	sd	fp,0x10(sp)	/* stack frame-pointer */
	move	fp,sp		/* take a copy of the stack pointer */
	/* We leave so much space on the stack for the string (16
	   characters), since the call to mon_printf seems to corrupt
	   the 8bytes at offset 8 into the string/stack. */
	sb	a0,0x00(sp)	/* character to print */
	sb	z0,0x01(sp)	/* NUL terminator */
	jal	pmon_printf	/* and output the string */
	move	a0,sp		/* take a copy of the string pointer {DELAY SLOT} */

	move	sp,fp		/* recover stack pointer */
	ld	ra,0x18(sp)	/* recover return address */
	ld	fp,0x10(sp)	/* recover frame-pointer */
	j	ra		/* return to the caller */
	addu	sp,sp,0x20	/* dump the stack space {DELAY SLOT} */
	.set	reorder
	.end	pmon_outbyte
	
/* EOF pmon.S */
