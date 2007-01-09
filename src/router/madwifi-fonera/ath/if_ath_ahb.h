/*
 * Copyright (c) 2004 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: if_ath_ahb.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */

#ifndef _DEV_ATH_AHB_H_
#define _DEV_ATH_AHB_H_

#include <asm/io.h>
#include <asm/uaccess.h>

#define AR531X_WLAN0_NUM       0
#define AR531X_WLAN1_NUM       1

#define REG_WRITE(_reg,_val)		*((volatile u_int32_t *)(_reg)) = (_val);
#define REG_READ(_reg)			*((volatile u_int32_t *)(_reg))

/*
 * 5315 specific registers 
 */

/* 
 * PCI-MAC Configuration registers 
 */
#define AR5315_PCI		0xB0100000      /* PCI MMR */
#define AR5315_PCI_MAC_RC	(AR5315_PCI + 0x4000) 
#define AR5315_PCI_MAC_SCR	(AR5315_PCI + 0x4004)
#define AR5315_PCI_MAC_INTPEND	(AR5315_PCI + 0x4008)
#define AR5315_PCI_MAC_SFR	(AR5315_PCI + 0x400C)
#define AR5315_PCI_MAC_PCICFG	(AR5315_PCI + 0x4010)
#define AR5315_PCI_MAC_SREV	(AR5315_PCI + 0x4020)

#define AR5315_PCI_MAC_RC_MAC	0x00000001
#define AR5315_PCI_MAC_RC_BB	0x00000002

#define AR5315_PCI_MAC_SCR_SLMODE_M	0x00030000
#define AR5315_PCI_MAC_SCR_SLMODE_S	16        
#define AR5315_PCI_MAC_SCR_SLM_FWAKE	0         
#define AR5315_PCI_MAC_SCR_SLM_FSLEEP	1         
#define AR5315_PCI_MAC_SCR_SLM_NORMAL	2         

#define AR5315_PCI_MAC_SFR_SLEEP	0x00000001

#define AR5315_PCI_MAC_PCICFG_SPWR_DN	0x00010000

#define AR5315_IRQ_WLAN0_INTRS	3
#define AR5315_WLAN0		0xb0000000

#define AR5315_ENDIAN_CTL	0xb100000c
#define AR5315_CONFIG_WLAN	0x00000002      /* WLAN byteswap */
#define AR5315_AHB_ARB_CTL	0xb1000008
#define AR5315_ARB_WLAN		0x00000002

/*
 * Revision Register - Initial value is 0x3010 (WMAC 3.0, AR531X 1.0).
 */
#define AR5315_SREV		0xb1000014

#define AR5315_REV_MAJ		0x0080
#define AR5317_REV_MAJ		0x0090
#define AR5315_REV_MAJ_M	0x00f0
#define AR5315_REV_MAJ_S	4
#define AR5315_REV_MIN_M	0x000f
#define AR5315_REV_MIN_S	0
#define AR5315_REV_CHIP		(REV_MAJ|REV_MIN)

#define AR531X_IRQ_WLAN0_INTRS	2
#define AR531X_IRQ_WLAN1_INTRS	5
#define AR531X_WLAN0		0xb8000000
#define AR531X_WLAN1		0xb8500000
#define AR531X_WLANX_LEN	0x000ffffc

#define	AR531X_RESETCTL		0xbc003020
#define	AR531X_RESET_WLAN0			0x00000004	/* mac & bb */
#define	AR531X_RESET_WLAN1			0x00000200	/* mac & bb */
#define	AR531X_RESET_WARM_WLAN0_MAC		0x00002000
#define	AR531X_RESET_WARM_WLAN0_BB		0x00004000
#define	AR531X_RESET_WARM_WLAN1_MAC		0x00020000
#define	AR531X_RESET_WARM_WLAN1_BB		0x00040000

#define AR531X_ENABLE		0xbc003080
#define	AR531X_ENABLE_WLAN0			0x0001
#define	AR531X_ENABLE_WLAN1			0x0018	/* both DMA and PIO */

#define AR531X_RADIO_MASK_OFF	0xc8
#define AR531X_RADIO0_MASK	0x0003
#define AR531X_RADIO1_MASK	0x000c
#define AR531X_RADIO1_S		2

#define BUS_DMA_FROMDEVICE	0
#define BUS_DMA_TODEVICE	1

#define AR531X_APBBASE		0xbc000000
#define AR531X_RESETTMR		(AR531X_APBBASE  + 0x3000)
#define AR531X_REV		(AR531X_RESETTMR + 0x0090) /* revision */
#define AR531X_REV_MAJ		0x00f0
#define AR531X_REV_MAJ_S	4
#define AR531X_REV_MIN		0x000f
#define AR531X_REV_MIN_S	0

#define AR531X_BD_MAGIC 0x35333131   /* "5311", for all 531x platforms */

/* set bus cachesize in 4B word units */
static __inline void bus_dma_sync_single(void *hwdev, dma_addr_t dma_handle,
	size_t size, int direction)
{
    unsigned long addr;
    
    addr = (unsigned long) __va(dma_handle);
    dma_cache_wback_inv(addr, size);
}

static __inline dma_addr_t bus_map_single(void *hwdev, void *ptr,
	size_t size, int direction)
{
    unsigned long addr = (unsigned long) ptr;
    
    dma_cache_wback_inv(addr, size);
    
    return __pa(ptr);
}

static __inline void bus_unmap_single(void *hwdev, dma_addr_t dma_addr,
	size_t size, int direction)
{
    if (direction != BUS_DMA_TODEVICE) {
        unsigned long addr;
        
        addr = (unsigned long)__va(dma_addr);
        dma_cache_wback_inv(addr, size);
    }
}
void *bus_alloc_consistent(void *, size_t, dma_addr_t *);
void bus_free_consistent(void *, size_t, void *, dma_addr_t);

extern const char * get_system_type(void);
#define sysRegRead(phys)      (*(volatile u_int32_t *)phys)

#endif    /* _DEV_ATH_AHB_H_ */
