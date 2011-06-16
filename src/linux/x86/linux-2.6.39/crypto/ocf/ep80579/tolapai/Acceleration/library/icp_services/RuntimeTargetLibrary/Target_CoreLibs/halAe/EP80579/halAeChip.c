/**
 **************************************************************************
 * @file halAeChip.c
 *
 * @description
 *      This file provides Implementation of Ucode AE Library
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

#include "core_io.h"
#include "hal_global.h"
#include "halAeApi.h"
#include "halAeDrv.hxx"

extern unsigned int AeMask;
extern unsigned int AePerCluster;
extern unsigned int AeBadMask;
extern unsigned int AeMaxNum;
extern unsigned int MaxUstore;
extern unsigned int MaxLmemReg; 
extern unsigned int UpcMask;
extern unsigned int PrdMajType, PrdMinType, PrdMajRev, PrdMinRev;

int halAe_GetProdSetting(void);

/*-----------------------------------------------------------------------------
   Function:    halAe_GetProdInfo
   Description: Get the CPU type and product maj/min revisions, and return
                the CPU type, and the combined maj and min
                revision.
   Returns:     HALAE_SUCCESS, HALAE_FAIL, HALAE_BADARG, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetProdInfo(unsigned int *prodType, 
                  unsigned int *prodRev)
{
    HALAE_VERIFY_LIB();
    if(!prodType || !prodRev) 
    {
        return (HALAE_BADARG);
    }    

    switch(PrdMinType)
    {
    case HWID_ICP:
        if (PrdMajRev < 2)
        {
            /* EP80579 */
            *prodType = EP80579_CPU_TYPE; 
            *prodRev = (PrdMajRev << 4) | PrdMinRev;
        }
        else
        {
            return (HALAE_FAIL);
        }
        break;

    default: 
        return (HALAE_FAIL);
    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetProdSetting
   Description: Get chip specific settings
   Returns:     HALAE_SUCCESS, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int halAe_GetProdSetting(void)
{
    const unsigned int ustore8k=0x2000;
    switch(PrdMinType)
    {
    case HWID_ICP:
        if (PrdMajRev < 2)
        {
            /* EP80579 */
            AeMask = 0x3;  /* helps map AE number to number within the cluster */
            AePerCluster = 4;
            AeBadMask = 0xfffffff0;
            AeMaxNum = 3;
        }
        else
        {
            return (HALAE_FAIL);
        }
        UpcMask = 0x1ffff;
        MaxUstore = ustore8k;
        MaxLmemReg = MAX_ICP_LMEM_REG;
        break;

    default:
        return (HALAE_FAIL);
    }

    return (HALAE_SUCCESS);
}

