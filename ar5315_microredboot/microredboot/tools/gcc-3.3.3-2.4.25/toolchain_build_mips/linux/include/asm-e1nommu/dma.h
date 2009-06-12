#ifndef _HYPERSTONE_DMA_H
#define _HYPERSTONE_DMA_H

#define MAX_DMA_ADDRESS  	PAGE_OFFSET
#define MAX_DMA_CHANNELS 	0

extern int request_dma(unsigned int dmanr, const char * device_id);	/* reserve a DMA channel */
extern void free_dma(unsigned int dmanr);	/* release it again */

#endif /* _M68K_DMA_H */
