/*
 * include/asm-arm/arch-sl2312/xor.h
 *
 * Copyright (C) 2005 Storlink Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARCH_XOR_H
#define _ASM_ARCH_XOR_H

/*
 * Function prototypes
 */
void xor_gemini_2(unsigned long bytes, unsigned long *p1, unsigned long *p2);

void xor_gemini_3(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		             unsigned long *p3);

void xor_gemini_4(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		             unsigned long *p3, unsigned long *p4);

void xor_gemini_5(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		             unsigned long *p3, unsigned long *p4, unsigned long *p5);

#endif /* _ASM_ARCH_XOR_H */

