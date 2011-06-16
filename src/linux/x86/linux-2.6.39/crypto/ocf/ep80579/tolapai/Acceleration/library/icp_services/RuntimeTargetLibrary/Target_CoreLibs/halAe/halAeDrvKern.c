/**
 **************************************************************************
 * @file halAeDrvKern.c
 *
 * @description
 *      This file provides Implementation of functions for kernel mode
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

#include "IxOsal.h"
#include "halAeDrv.hxx"

extern unsigned int hal_ae_ncdram_base;
extern unsigned int hal_ae_ncdram_size;
extern unsigned int hal_ae_cdram_base;
extern unsigned int hal_ae_cdram_size;
extern unsigned int hal_ae_sram_base;

extern unsigned int icp_dev_cfg;
extern unsigned char swsku;

unsigned int halAe_GetAeClkFreqMhz(void);
int halAe_GetMemSzInfo(aeDrv_SysMemInfo_T *sysMemInfo);

/* 
 * ICP_DEVICE_CONFIG register (offset=0x40) in 
 * AE Cluster Configuration Space 
 *   AESPD bit-31: 
 *     AE Speed Clock Override with ASYNC reset. 0=533MHz or 400MHz,
 *     1=800MHz. If AESPDFUSE = 0, writing a 1 to this register
 *     has no effect 
 *   AESPDFUSE bit-30:
 *     AE Speed Clock Override FUSE setting. 0=533MHz or 400MHz,
 *     1=800MHz
 *     
 *   SKU ID:
 *     1,       ae_speed = 400 MHz
 *     3/5/7,   if ((AESPDFUSE && AESPD) == 0) ae_speed = 533 MHz
 *              if ((AESPDFUSE && AESPD) == 1) ae_speed = 800 MHz
 *     2/4/6/8, ae_speed = 0
 *     others,  ae_speed = 400 MHz
 *
 *   EP80579 A0, SWSKU = 0, ae_speed = 400 MHz
 *   EP80579 B0, SWSKU = 5, ae_speed = 533 MHz
 */
/* SKU ID vs AE frequency:                              0,   1, 2,   3, 4,   5, 6,   7, 8 */
static unsigned int AeSpdClkLo[] = {DEFAULT_ICP_FREQUENCY, 400, 0, 533, 0, 533, 0, 533, 0};
static unsigned int AeSpdClkHi[] = {DEFAULT_ICP_FREQUENCY, 400, 0, 800, 0, 800, 0, 800, 0};

/*-----------------------------------------------------------------------------
   Function:    halAe_GetAeClkFreqMhz
   Description: Get the accelEngine clock frequency
   Returns:     Clock frequency
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
halAe_GetAeClkFreqMhz(void)
{
    unsigned int ae_speed = DEFAULT_ICP_FREQUENCY;

    if(swsku < (sizeof(AeSpdClkLo)/sizeof(unsigned int)))
    {
        /* low speed AE */
        if(((icp_dev_cfg >> AESPDFUSE_BITPOS) && (icp_dev_cfg >> AESPD_BITPOS)) == 0)
        {
            ae_speed = AeSpdClkLo[swsku];
        }
        /* high speed AE */
        if(((icp_dev_cfg >> AESPDFUSE_BITPOS) && (icp_dev_cfg >> AESPD_BITPOS)) == 1)
        {
            ae_speed = AeSpdClkHi[swsku];
        }
    } 

    return (ae_speed);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetMemSzInfo
   Description: Get memory sizing information of EP80579 product.
   Returns:     0 if successful or -1 for failure
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetMemSzInfo(aeDrv_SysMemInfo_T *sysMemInfo)
{
    ixOsalMemSet(&sysMemInfo->dramChan, 0, 
           sizeof(aeDrv_drChanDesc_T) * MAX_DRAM_CHAN);
    ixOsalMemSet(&sysMemInfo->sramChan, 0, 
           sizeof(aeDrv_srChanDesc_T) * MAX_SRAM_CHAN);

    /* get dram size info */
    sysMemInfo->numDramChan = MAX_DRAM_CHAN;
    sysMemInfo->dramChan[0].aeDramSize = hal_ae_ncdram_size;
    sysMemInfo->dramChan[0].aeDramBase = 0;
    sysMemInfo->dramChan[0].dramBaseAddr = hal_ae_ncdram_base;
    
    sysMemInfo->dramChan[1].aeDramSize = hal_ae_cdram_size;
    sysMemInfo->dramChan[1].aeDramBase = 0;
    sysMemInfo->dramChan[1].dramBaseAddr = hal_ae_cdram_base;

    sysMemInfo->totDramSize = sysMemInfo->dramChan[0].aeDramSize + 
                                sysMemInfo->dramChan[1].aeDramSize;

    /* get sram size info */
    sysMemInfo->numSramChan = MAX_SRAM_CHAN;        
    sysMemInfo->sramChan[0].sramBase = hal_ae_sram_base;
    /* force value to 256KB */
    sysMemInfo->sramChan[0].sramSize = KILO_256;  

    sysMemInfo->valid = 1;
    
    return (0);
}
