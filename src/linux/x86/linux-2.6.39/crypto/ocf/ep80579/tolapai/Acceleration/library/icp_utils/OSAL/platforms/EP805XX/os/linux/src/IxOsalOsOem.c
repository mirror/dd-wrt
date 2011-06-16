/**
 * @file IxOsalOsOem.c  (linux)
 *
 * @brief this file contains implementation for platform-specific
 *        functionalities.
 * 
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
 */

#include "IxOsalOsTypes.h"
#include <linux/utsname.h>
#include "IxOsal.h"

PRIVATE BOOL IxOsalOemInitialized = FALSE;

PUBLIC UINT32 ixOsalLinuxInterruptedPc = 0;

extern struct new_utsname system_utsname;

PUBLIC UINT32
ixOsalOsEP805XXTimestampGet (void)
{
        UINT32 timestamp;
        /* Read the 32-bit LSB of the time stamp counter */
        rdtscl(timestamp);
        return (timestamp);
}

PUBLIC UINT32
ixOsalOsEP805XXTimestampResolutionGet (void)
{
   return (UINT32)IX_OSAL_EP805XX_TIME_STAMP_RESOLUTION ;
}

UINT32
ixOsalOsEP805XXSysClockRateGet (void)
{
    return HZ;
}

#ifdef ENABLE_IOMEM
/*
 * Dummy MemMapInit 
 */
PUBLIC IX_STATUS
ixOsalMemMapInit (IxOsalMemoryMap *map, UINT32 numElement)
{
    return IX_SUCCESS;
}

#endif /* ENABLE_IOMEM */

PUBLIC IX_STATUS
ixOsalOemInit (void)
{
    /*
     * Check flag 
     */
    if (IxOsalOemInitialized == TRUE)
    {
        return IX_SUCCESS;
    }

    IxOsalOemInitialized = TRUE;
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsEP805XXNameGet(INT8* osName, INT32 maxSize)
{
    if(maxSize < 0)
    {
        return IX_FAIL;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	strncpy((VOID *) osName, (utsname())->sysname, (UINT32) maxSize);
#else
	strncpy((VOID *) osName, system_utsname.sysname, (UINT32) maxSize);
#endif
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsEP805XXVersionGet(INT8* osVersion, INT32 maxSize)
{
    if(maxSize < 0)
    {
        return IX_FAIL;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	strncpy((VOID *) osVersion, (utsname())->release, (UINT32) maxSize);
#else
	strncpy((VOID *) osVersion, system_utsname.release, (UINT32) maxSize);
#endif
    return IX_SUCCESS;
}

PUBLIC void
ixOsalOemUnload (void)
{
    IxOsalOemInitialized = FALSE;
}

void
ixOsalEP805XXSetInterruptedPc(struct pt_regs *regs)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
	ixOsalLinuxInterruptedPc = regs->eip;
#else
	ixOsalLinuxInterruptedPc = regs->ip;
#endif
}

/* ===================== End - Irq ====================== */
