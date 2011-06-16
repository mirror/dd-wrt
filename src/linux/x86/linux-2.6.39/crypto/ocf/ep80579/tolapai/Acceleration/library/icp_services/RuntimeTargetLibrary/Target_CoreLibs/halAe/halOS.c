/**
 **************************************************************************
 * @file halOS.c
 *
 * @description
 *      This file provides implementation of AE Library(OS-dependent code)
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
#include "core_io.h"
#include "IxOsal.h"
#include "halAeApi.h"
#include "hal_global.h"
#include "halAeDrv.hxx"
#include "halMmap.hxx"
#include "halOS.h"
#include "halAeDrv.hxx"

extern SPINLOCK_T MEMINFO_LOCK;
extern SPINLOCK_T MEMTABLE_LOCK;
extern HalMemMap_T HalMmapTable;
extern aeDrv_SysMemInfo_T HalSysMemInfo;

void exportVirAddr(HalMemMap_T *mTab, aeDrv_SysMemInfo_T *sysMemInfo);

/**************************************************************************
 * Function: halAe_GetVirXaddr
 *
 * Description: Return the starting virtual address of the spcified physical
 *                region if it's mapped by the HAL, otherwise, return an
 *                invalid address (0xffffffffffffffff) is returned.
 *
 **************************************************************************/
uint64 
halAe_GetVirXaddr(uint64 phyXaddr, 
                  unsigned int phySize)
{
   uint64 addr;
   
   SPIN_LOCK(MEMTABLE_LOCK);
   addr = getVirXaddr(&HalMmapTable, phyXaddr, phySize);
   SPIN_UNLOCK(MEMTABLE_LOCK);    
    
   return (addr);
}

/**************************************************************************
 * Function: halAe_GetVirAddr
 *
 * Description: Return the starting virtual address of the spcified physical
 *                address if it's mapped by the HAL, otherwise, return an
 *                invalid address (0xffffffff) is returned.
 *
 **************************************************************************/
unsigned int 
halAe_GetVirAddr(unsigned int phyAddr)
{
    uint64 addr;
    
    addr = halAe_GetVirXaddr((uint64)phyAddr, 1);
    
    return ((unsigned int)addr);
}

/**************************************************************************
 * Function: halAe_GetPhyXaddr
 *
 * Description: Return the starting physical address of the spcified virtual
 *                region if it's mapped by the HAL, otherwise, return an
 *                invalid address (0xffffffffffffffff) is returned.
 *
 **************************************************************************/
uint64 
halAe_GetPhyXaddr(uint64 virtXaddr, 
                  unsigned int virtSize)
{
    uint64 addr;
    
    SPIN_LOCK(MEMTABLE_LOCK);
    addr = getPhyXaddr(&HalMmapTable, virtXaddr, virtSize);
    SPIN_UNLOCK(MEMTABLE_LOCK);
   
    return (addr);
}

/**************************************************************************
 * Function: halAe_GetSysMemInfo
 *
 * Description: Get the product ID and memory.
 *
 **************************************************************************/
int 
halAe_GetSysMemInfo(Hal_SysMemInfo_T *sysMemInfo)
{
    unsigned int i;

    if(!sysMemInfo) 
    {
        return (HALAE_BADARG);
    }
   
    SPIN_LOCK(MEMINFO_LOCK);

    sysMemInfo->prodId = HalSysMemInfo.prodId;
    sysMemInfo->aeClkMhz = HalSysMemInfo.aeClkMhz;
    sysMemInfo->strapOptions = HalSysMemInfo.strapOptions;

    sysMemInfo->numDramDesc = HalSysMemInfo.numDramChan;
    for(i=0; i < sysMemInfo->numDramDesc; i++)
    {
        sysMemInfo->dramDesc[i].dramBaseAddr = 
                                HalSysMemInfo.dramChan[i].dramBaseAddr;
        sysMemInfo->dramDesc[i].dramSize = 
                                HalSysMemInfo.dramChan[i].aeDramSize;

        sysMemInfo->dramDesc[i].aeDramSize = 
                                HalSysMemInfo.dramChan[i].aeDramSize;
        sysMemInfo->dramDesc[i].aeDramOffset = 
                                HalSysMemInfo.dramChan[i].aeDramBase;
    }

    sysMemInfo->numSramChan = HalSysMemInfo.numSramChan;
    for(i=0; i < sysMemInfo->numSramChan; i++)
    {
        sysMemInfo->sramChan[i].sramSize = HalSysMemInfo.sramChan[i].sramSize;
        sysMemInfo->sramChan[i].sramOffset = HalSysMemInfo.sramChan[i].sramBase;
    }
    SPIN_UNLOCK(MEMINFO_LOCK);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_getMMScheme
   Description: return NCDRAM/CDRAM map scheme
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
HalMemMapScheme 
halAe_getMMScheme(void)
{
   return (HalMmapTable.mm_scheme);
}
