/*
 * include/asm-sparc/dma.h
 *
 * Copyright 1995 (C) David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef _ASM_SPARC_DMA_H
#define _ASM_SPARC_DMA_H

#include <linux/kernel.h>

#define MAX_DMA_CHANNELS 2
#define MAX_DMA_ADDRESS  (~0UL)
#define DMA_MODE_READ    1
#define DMA_MODE_WRITE   2

extern int get_dma_list(char *);
extern int request_dma(unsigned int, const char *);
extern void free_dma(unsigned int);

#endif /* !(_ASM_SPARC_DMA_H) */
