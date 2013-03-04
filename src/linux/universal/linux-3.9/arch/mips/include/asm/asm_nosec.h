/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995, 1996, 1997, 1999, 2001 by Ralf Baechle
 * Copyright (C) 1999 by Silicon Graphics, Inc.
 * Copyright (C) 2001 MIPS Technologies, Inc.
 * Copyright (C) 2002  Maciej W. Rozycki
 * Copyright (C) 2010  Wu Zhangjin <wuzhangjin@gmail.com>
 *
 * Derive from <asm/asm.h>
 *
 * Override the macros without -ffunction-sections and -fdata-sections support.
 * If several functions or data must be put in the same section, please include
 * this header file after the <asm/asm.h> to override the generic definition.
 */

#ifndef __ASM_ASM_NOSEC_H
#define __ASM_ASM_NOSEC_H

#undef LEAF
#undef NESTED
#undef EXPORT

/*
 * LEAF - declare leaf routine
 */
#define	LEAF(symbol)                                    \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol, @function;              \
		.ent	symbol, 0;                      \
symbol:		.frame	sp, 0, ra

/*
 * NESTED - declare nested routine entry point
 */
#define	NESTED(symbol, framesize, rpc)                  \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol, @function;              \
		.ent	symbol, 0;                       \
symbol:		.frame	sp, framesize, rpc

/*
 * EXPORT - export definition of symbol
 */
#define EXPORT(symbol)					\
		.globl	symbol;                         \
symbol:

#endif /* __ASM_ASM_NOSEC_H */
