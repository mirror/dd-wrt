/**
 **************************************************************************
 * @file halMmap.c
 *
 * @description
 *      This file provides implementation of MEv2 HAL library
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
#include "halAeDrv.hxx"
#include "halAeApi.h"
#include "hal_global.h"

void exportVirAddr(HalMemMap_T *mTab, aeDrv_SysMemInfo_T *sysMemInfo);
void halAe_DrvMmap(HalMemMap_T *mTab);

SPINLOCK_T MEMINFO_LOCK;
SPINLOCK_T MEMTABLE_LOCK;
HalMemMap_T HalMmapTable;
aeDrv_SysMemInfo_T HalSysMemInfo;

volatile uint64 Hal_dram_ch0_virtAddr;
volatile uint64 Hal_dram_ch1_virtAddr;
volatile uint64 Hal_sram_ch0_virtAddr;
volatile uint64 Hal_sram_ch0_rd_wr_virtAddr;
volatile uint64 Hal_cap_global_ctl_csr_virtAddr;
volatile uint64 Hal_cap_ae_xfer_csr_virtAddr;
volatile uint64 Hal_cap_ae_local_csr_virtAddr;
volatile uint64 Hal_cap_pmu_csr_virtAddr;
volatile uint64 Hal_cap_hash_csr_virtAddr;
volatile uint64 Hal_scratch_rd_wr_swap_virtAddr;
volatile uint64 Hal_ae_fastaccess_csr_virtAddr;                       
volatile uint64 Hal_ssu_csr_virtAddr;                      
volatile uint64 Hal_eagletail_ring_csr_virtAddr;
volatile uint64 Hal_eagletail_ring_access_virtAddr;
volatile uint64 Hal_memory_target_csr_virtAddr;

/*-----------------------------------------------------------------------------
   Function:    exportVirAddr
   Description: Export virtual address of specified memory map table
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
exportVirAddr(HalMemMap_T *mTab, aeDrv_SysMemInfo_T *sysMemInfo)
{
    SPINLOCK_T myLock;

    SPIN_LOCK_INIT(myLock);
    SPIN_LOCK(myLock);

    /* Map the physical address to virtual address */
    halAe_DrvMmap(mTab);

    Hal_dram_ch0_virtAddr = mTab->dram_ch0.virt;
    Hal_dram_ch1_virtAddr = mTab->dram_ch1.virt;

    /* Hal_sram_chx_virtAddr addresses are from the beg of the sram channel
       through the end of the sram-read/write region */
    Hal_sram_ch0_virtAddr = mTab->sram_ch0.virt;

    /* sramBase should be relative to the begining of the SRAM channel */
    Hal_sram_ch0_rd_wr_virtAddr = SADD(Hal_sram_ch0_virtAddr, 
                                       sysMemInfo->sramChan[0].sramBase);

    Hal_cap_global_ctl_csr_virtAddr = mTab->cap_global_ctl_csr.virt;
    Hal_cap_ae_xfer_csr_virtAddr = mTab->cap_ae_xfer_csr.virt;    

    Hal_cap_hash_csr_virtAddr = SADD(mTab->cap_global_ctl_csr.virt, 0x0900);
    Hal_cap_pmu_csr_virtAddr = mTab->cap_pmu_csr.virt;
        
    Hal_scratch_rd_wr_swap_virtAddr = mTab->scratch_rd_wr_swap.virt;

    Hal_ae_fastaccess_csr_virtAddr = mTab->ae_fastaccess_csr.virt;                       
    Hal_ssu_csr_virtAddr = mTab->ssu_csr.virt;                      

    Hal_eagletail_ring_csr_virtAddr = mTab->eagletail_ring_csr.virt;
    Hal_eagletail_ring_access_virtAddr = mTab->eagletail_ring_access.virt;

    Hal_memory_target_csr_virtAddr = mTab->memory_target_csr.virt;
    
    SPIN_UNLOCK(myLock);
    SPIN_LOCK_FINI(myLock);

    return;
}

/**************************************************************************
 * Function: getMapIo
 *
 * Description: Return the mapIO structure associated with the specified
 *                physical region if it's mapped by the HAL, otherwise,
                  return NULL.
 *                
 *
 **************************************************************************/
HalMmapIo_T *
getMapIo(HalMemMap_T *mTab, uint64 phyAddr, unsigned int phySize)
{
    HalMmapIo_T *p_pvmap = NULL;
    for(p_pvmap = &mTab->dram_ch0; p_pvmap < &mTab->the_end; p_pvmap++)
    {
        if(p_pvmap->size == 0) 
        {
           continue;
        }   

        if(((phyAddr + phySize) >= p_pvmap->phys) &&
            ((phyAddr + phySize) <= (p_pvmap->phys + p_pvmap->size)))
        {
            return (p_pvmap);
        }
    }

    return (NULL);  
}

/**************************************************************************
 * Function: getVirXaddr
 *
 * Description: Return the starting virtual address of the spcified physical
 *                region if it's mapped by the HAL, otherwise, return an
 *                invalid address (0xffffffffffffffff) is returned.
 *
 **************************************************************************/
uint64 
getVirXaddr(HalMemMap_T *mTab, uint64 phyAddr, unsigned int phySize)
{
    HalMmapIo_T *p_pvmap = NULL;
    uint64 csrOffset = 0;

    for(p_pvmap = &mTab->dram_ch0; p_pvmap < &mTab->the_end; p_pvmap++)
    {
        if(p_pvmap->size == 0) 
        {
           continue;
        }   
        if((phyAddr >= p_pvmap->phys) &&
            ((phyAddr + phySize) <= (p_pvmap->phys + p_pvmap->size))) 
        {
            csrOffset = phyAddr - p_pvmap->phys;
            return (csrOffset + p_pvmap->virt);
        }
    }

    return (HALAE_INVALID_XADDR);  
}

/**************************************************************************
 * Function: getPhyXaddr
 *
 * Description: Return the starting physical address of the specified virtual
 *                region if it's mapped by the HAL, otherwise, return an
 *                invalid address (0xffffffffffffffff) is returned.
 *
 **************************************************************************/
uint64 
getPhyXaddr(HalMemMap_T *mTab, uint64 virtAddr, unsigned int virtSize)
{
    HalMmapIo_T *p_pvmap  = NULL;
    uint64 csrOffset = 0;

    for(p_pvmap = &mTab->dram_ch0; p_pvmap < &mTab->the_end; p_pvmap++)
    {
        if(p_pvmap->size == 0) 
        {
           continue;
        }   
        if((virtAddr >= p_pvmap->virt) &&
            ((virtAddr + virtSize) <= (p_pvmap->virt + p_pvmap->size))) 
        {
            csrOffset = virtAddr - p_pvmap->virt;
            return (csrOffset + p_pvmap->phys);
        }
    }
    return (HALAE_INVALID_XADDR);  
}
