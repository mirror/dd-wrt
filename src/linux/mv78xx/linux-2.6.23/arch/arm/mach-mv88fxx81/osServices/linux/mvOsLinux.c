/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/

#include "mvOsLinux.h"
#include <linux/random.h>
#include <asm/delay.h>
 
MV_U32 mvOsIoVirtToPhy( void* pDev, void* pVirtAddr )
{
    /* size = 0 */
    return pci_map_single( pDev, pVirtAddr, 0, PCI_DMA_BIDIRECTIONAL );
}
 
void* mvOsIoCachedMalloc( void* pDev, MV_U32 size, MV_ULONG* pPhyAddr )
{
    void *p = kmalloc( size, GFP_KERNEL );
    *pPhyAddr = pci_map_single( pDev, p, 0, PCI_DMA_BIDIRECTIONAL );
    return p;
}
void* mvOsIoUncachedMalloc( void* pDev, MV_U32 size, MV_U32* pPhyAddr )
{
    return pci_alloc_consistent( pDev, size, (dma_addr_t *)pPhyAddr );
}
 
void mvOsIoUncachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    return pci_free_consistent( pDev, size, pVirtAddr, (dma_addr_t)phyAddr );
} 
                                                                                                                                               
void mvOsIoCachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    return kfree( pVirtAddr );
}
 
MV_U32 mvOsCacheFlush( void* pDev, void* p, int size )
{
    return pci_map_single( pDev, p, size, PCI_DMA_TODEVICE );
}
 
MV_U32 mvOsCacheInvalidate( void* pDev, void* p, int size )
{
    return pci_map_single( pDev, p, size, PCI_DMA_FROMDEVICE );
}
 
int mvOsRand(void)
{
    int rand;
    get_random_bytes(&rand, sizeof(rand) );
    return rand;
}


