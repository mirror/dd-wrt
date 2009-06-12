/* vic - todo - add inline assembler to optimize ? */
/*
 * string.h: External definitions for optimized assembly string
 *           routines for the Linux Kernel.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __NIOS_STRING_H__
#define __NIOS_STRING_H__

#ifdef __KERNEL__ /* only set these up for kernel code */

#define __HAVE_ARCH_MEMMOVE
void * memmove(void * d, const void * s, size_t count);
#define __HAVE_ARCH_MEMCPY
extern void * memcpy(void *d, const void *s, size_t count);
#define __HAVE_ARCH_MEMSET
extern void * memset(void * s,int c,size_t count);

#if 0
#define __HAVE_ARCH_BCOPY
#define __HAVE_ARCH_STRLEN
#endif

#endif /* KERNEL */

#endif /* !(__NIOS_STRING_H__) */
