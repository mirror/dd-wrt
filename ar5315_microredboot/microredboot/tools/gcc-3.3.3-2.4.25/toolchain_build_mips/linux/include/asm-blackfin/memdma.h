/*
 * include/asm-frionommu/memdma.h - Function declaration for memory to memory 
 * dma for Blackfin ADSP-21535
 *  derived from:
 *  include/asm-i386/dma.h
 *  Written by Hennus Bergman, 1992.
 *  Hannu Savolainen  and John Boyd, Nov. 1992.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * or (at your option) any later version as  published by the Free Software 
 * Foundation.
 *
 */
#ifndef _FRIONOMMU_MEMDMA_H
#define _FRIONOMMU_MEMDMA_H 1

void mem_dma_xmt_enable(void);
void mem_dma_xmt_disable(void);
void mem_dma_set_dma_mode(unsigned int channel);
int mem_dma_xmt_get_dma_residue(void);
void mem_dma_xmt_clear_dma_ff(void);
void mem_dma_rcv_clear_dma_ff(void);
int mem_dma_rcv_get_dma_residue(void);
void mem_dma_rcv_disable(void);
void mem_dma_rcv_enable(void);
void mem_dma_xmt_set_dma_addr(unsigned int addr);
void mem_dma_rcv_set_dma_addr(unsigned int addr);
void mem_dma_xmt_set_dma_count(unsigned int count);
void mem_dma_rcv_set_dma_count(unsigned int count);

#endif
