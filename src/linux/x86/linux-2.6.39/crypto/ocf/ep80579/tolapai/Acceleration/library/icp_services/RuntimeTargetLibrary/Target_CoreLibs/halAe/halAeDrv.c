/**
 **************************************************************************
 * @file halAeDrv.c
 *
 * @description
 *      This file provides Implementation of IOCTL interface and driver 
        framework
 *
 * @par 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 **************************************************************************/ 

#include "halAe_platform.h"
#include "IxOsal.h"
#include "core_io.h"
#include "halMmap.hxx"
#include "halAeApi.h"
#include "hal_global.h"
#include "halAeDrv.hxx"

extern SPINLOCK_T MEMINFO_LOCK;
extern SPINLOCK_T MEMTABLE_LOCK;
extern HalMemMap_T HalMmapTable;
extern aeDrv_SysMemInfo_T HalSysMemInfo;

unsigned int hal_ae_ncdram_size;
unsigned int hal_ae_cdram_size;
unsigned int hal_ae_ncdram_base;
unsigned int hal_ae_cdram_base;
unsigned int hal_ae_sram_base;

unsigned int rev_id;
unsigned int icp_dev_cfg;
unsigned char swsku;

unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

unsigned int MaxAe;
unsigned int MaxAeMask;

HalMemMap_T *halAe_InitMapTable(HalMemMap_T *mTab);
void exportVirAddr(HalMemMap_T *mTab, aeDrv_SysMemInfo_T *sysMemInfo);

unsigned int halAe_GetAeClkFreqMhz(void);
int halAe_GetMemSzInfo(aeDrv_SysMemInfo_T *sysMemInfo);
int halAe_GetChipSetting(void);

/*-----------------------------------------------------------------------------
   Function:    halAeDrvInit
   Description: Initialize HAL driver
   Returns:     0 if successful or -1 for failure
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAeDrvInit()
{
    HalMmapIo_T *pMemIo;
    /*
     * Initialize IA core global virtual address and size
     */
#ifdef _DBG_PRINT

    unsigned int totalMapSize=0;
    PRINTF("AeDrv Physical/Virtual mapping\n");

#endif /* _DBG_PRINT */

    SPIN_LOCK_INIT(MEMINFO_LOCK);
    SPIN_LOCK_INIT(MEMTABLE_LOCK);
    
    SPIN_LOCK(MEMTABLE_LOCK);
    halAe_InitMapTable(&HalMmapTable);
    SPIN_UNLOCK(MEMTABLE_LOCK);
    
    SPIN_LOCK(MEMINFO_LOCK);

    /* get chip-specific settings */ 
    if (halAe_GetChipSetting())
    {
	PRINTF("icp_hal error: halAeDrvInit: Failed to get chip-specific settings.\n");
        SPIN_UNLOCK(MEMINFO_LOCK);
        return (-1);
    }
    HalSysMemInfo.strapOptions = 0;  /* no strap CSR in AE */
    HalSysMemInfo.aeClkMhz = halAe_GetAeClkFreqMhz();

    /****************************************************************************
     * Get memory sizing information.
     * The CSR addresses must be mapped prior to calling this function.
     *
    ****************************************************************************/
    if(!HalSysMemInfo.valid && halAe_GetMemSzInfo(&HalSysMemInfo))
    {
	PRINTF("icp_hal error: halAeDrvInit: Failed to Get Memory Size Information.\n");
        SPIN_UNLOCK(MEMINFO_LOCK);
        return (-1);    
    }

    SPIN_UNLOCK(MEMINFO_LOCK);


#ifdef    _DBG_PRINT

    PRINTF("prodId=0x%x, osDramSize=0x%x\n \
             aeDram0Size=0x%x, aeDram0Base=0x%x, \
             aeDram1Size=0x%x, aeDram1Base=0x%x\n \
             sram0Size=0x%x, sram0Base=0x%x\n",
             HalSysMemInfo.prodId, HalSysMemInfo.osDramSize,
             HalSysMemInfo.dramChan[0].aeDramSize, 
             HalSysMemInfo.dramChan[0].aeDramBase,
             HalSysMemInfo.dramChan[1].aeDramSize, 
             HalSysMemInfo.dramChan[1].aeDramBase,
             HalSysMemInfo.sramChan[0].sramSize, 
             HalSysMemInfo.sramChan[0].sramBase);

#endif  /* _DBG_PRINT */

    SPIN_LOCK(MEMTABLE_LOCK);
    /* adjust table size for actual sizes */
    HalMmapTable.sram_ch0.size = HalSysMemInfo.sramChan[0].sramSize;

    /* map addresses that are used by driver and not mapped by the kernel */
    for(pMemIo = &HalMmapTable.dram_ch0; 
        pMemIo < &HalMmapTable.the_end; 
        pMemIo++)
    {
        if((pMemIo->size == 0) || (pMemIo->virt != 0)) 
        {
            continue;
        } 
        if(mapMemIo(pMemIo)) 
        {
	    PRINTF("icp_hal error: halAeDrvInit: Memory Mapping failed.\n");
            SPIN_UNLOCK(MEMTABLE_LOCK);
            return (-1);
        }   

#ifdef    _DBG_PRINT

        totalMapSize += pMemIo->size;
        PRINTF("AeDrv Physical Mapping...totalMapSize=0x%x, \
                phys=0x%"XFMT64",virt=0x%"XFMT64",len=0x%x\n",
                totalMapSize, pMemIo->phys, pMemIo->virt, pMemIo->size);

#endif   /* _DBG_PRINT */

    }
    SPIN_UNLOCK(MEMTABLE_LOCK);

    SPIN_LOCK(MEMINFO_LOCK);
    /* full init of all the exported values */
    exportVirAddr(&HalMmapTable, &HalSysMemInfo);        
    SPIN_UNLOCK(MEMINFO_LOCK);

#ifdef    _DBG_PRINT

    PRINTF("halAeDrvInit completed!\n");

#endif  /* _DBG_PRINT */

    return (0);
}
