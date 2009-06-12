#ifndef __IDT_DMA_V_H__
#define __IDT_DMA_V_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * DMA register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/devs/eth/mips/acacia/current/src/dma_v.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: dma.h,v $
 *	    Revision 1.3  2002/06/06 18:34:03  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.2  2002/06/05 18:30:46  astichte
 *	    Removed IDTField
 *	
 *	    Revision 1.1  2002/05/29 17:33:21  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/
#include  "types.h"
#include  "dma.h"
#include  "eth.h"

#define DMA_CHAN_OFFSET  0x14
#define IS_DMA_USED(X) (((X) & (DMAD_f_m | DMAD_d_m | DMAD_t_m)) != 0)
#define DMA_COUNT(count)   \
  ((count) & DMAD_count_m)

#define DMA_HALT_TIMEOUT 500

static inline int rc32438_halt_dma(DMA_Chan_t ch)
{
	int timeout=1;
	U32 dmac;

	dmac = readl(&ch->dmac);
	if (dmac & DMAC_run_m) {
		writel(0, &ch->dmac); 
		for (timeout = DMA_HALT_TIMEOUT; timeout > 0; timeout--) {
			if (readl(&ch->dmas) & DMAS_h_m) {
				writel(0, &ch->dmas);  
				break;
			}
		}
	}

	return timeout ? 0 : 1;
}

static inline void rc32438_start_dma(DMA_Chan_t ch, U32 dma_addr)
{
	writel(0, &ch->dmandptr); 
	writel(dma_addr, &ch->dmadptr);
}

static inline void rc32438_chain_dma(DMA_Chan_t ch, U32 dma_addr)
{
	writel(dma_addr, &ch->dmandptr);
}

#endif	// __IDT_DMA_V_H__
