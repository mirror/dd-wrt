/*
 * include/asm-frionommu/dma.h
 * Data structures and register support for DMA on Blackfin ADSP-21535.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * or (at your option) any later version as  published by the Free Software 
 * Foundation.
 *
 */

#ifndef _FRIONOMMU_DMA_H
#define _FRIONOMMU_DMA_H 1

#include <linux/config.h>
#include <linux/mm.h>  // Included for memory management
#include <asm/blackfin.h>
#include <asm/memdma.h>
/* it's useless on the m68k, but unfortunately needed by the new
   bootmem allocator (but this should do it for this) */
#define MAX_DMA_ADDRESS PAGE_OFFSET

/*#define MAX_DMA_CHANNELS 8 */
/* Code may need to identify peripheral caller*/
#define BYTE_REF(addr)		 (*((volatile unsigned char*)addr))
#define HALFWORD_REF(addr) 	 (*((volatile unsigned short*)addr))
#define WORD_REF(addr)		 (*((volatile unsigned long*)addr))
#define MAX_DMA_CHANNELS        13
/* The configuration word below will enable any DMA channel but will not be ready yet to transfer any data  DMA_CONFIGURATION_WORD 0x8005                           */
/* System I/O mapping values */

#define DMA_DBP_ADDR            0xffc04880 /* DMA Descriptor Base pointer register */
#define DMA_DBP			HALFWORD_REF(DMA_DBP_ADDR)

#define DMA_CONFIGURATION_WORD_ENABLE  0x0005  /*  Ownership still with processor Default granularity 8-bit check for options*/
#define DMA_CONFIGURATION_WORD_DISABLE  0x0004  /* Ownership still with processor Must check for other options*/
#define DMA_END_SEQUENCE	0x0000 /* The 15th bit must be 0*/
#define DMA_START_SEQUENCE	0x8005 /* 15th bit 1 and def gran = 8 bit with channel enabled */
#define DMA2_MASK_REG
#define DESCRIPTOR_BLOCK_BASE_ADDRESS
/* DMA Channel mappings */
#define SPORT0_DMA_RCV		0
#define SPORT1_DMA_RCV		1
#define SPORT0_DMA_XMT		2
#define SPORT1_DMA_XMT		3
#define USB_DMA_CONTROL		4
#define SPI0_DMA_CONTROL	5
#define SPI1_DMA_CONTROL	6
#define UART0_DMA_RCV		7
#define UART1_DMA_RCV		8
#define UART0_DMA_XMT		9
#define UART1_DMA_XMT		10
#define MEM_DMA_XMT		11
#define MEM_DMA_RCV		12

/* Descriptor LInked list structure
*/
typedef struct _dma_descriptor_block
{
 unsigned short configuration_word; /* BASE + 0 */
 unsigned short transfer_count;     /* BASE + 2 */
 unsigned short start_addr_lsb;     /* BASE + 4 */
 unsigned short start_addr_msb;     /* BASE + 6 */
 unsigned short next_descriptor;     /* BASE + 8 */
}dma_descriptor_block;

typedef struct _dma_channel_struct
{
 const char* device_id;
 unsigned int dma_intr;
 unsigned int lock;
 dma_descriptor_block descriptor;    //Defined for standalone sequence only   
}dma_channel;


/* Bits in the configuration word */
#define DMA_ENABLE	0x0001
#define DMA_DIRECTION	
#define DMA_INTR_COMPLETE	0x0004
#define DMA_BUFFER_CLEAR	(0x0080 ^ 0x0080) /* must be zero in configuration word */
#define DMA_CONPLETION_ERROR	0x4000
#define DMA_DESCRIPTOR_OWNER	0x8000
#define DMA_MODE_WRITE		0x0002
#define DMA_MODE_READ		0x0000
#define DATA_SIZE_8_BIT		0x1008
#define DATA_SIZE_16_BIT	
#define DATA_SIZE_32_BIT	
extern spinlock_t  dma_spin_lock;


#endif /* _FRIONOMMU_DMA_H */

