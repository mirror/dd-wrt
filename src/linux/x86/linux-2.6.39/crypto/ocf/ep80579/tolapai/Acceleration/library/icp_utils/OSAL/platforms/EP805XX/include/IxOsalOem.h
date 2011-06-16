/**
 * @file IxOsalOem.h
 *
 * @brief this file contains platform-specific defines.
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

#ifndef IxOsalOem_H
#define IxOsalOem_H

#include "IxOsalTypes.h"

/* OS-specific header for Platform package */

#include "IxOsalOsOem.h"


/*
 * Platform Name
 */
#define IX_OSAL_PLATFORM_NAME EP805XX

/*
 *  IRQ PMU for EP805XX - place holder
 */
#define IRQ_EP805XX_IA_PMU  	  0

/*
 * Cache line size
 */
#define IX_OSAL_CACHE_LINE_SIZE (64)


/* Various default module inclusions */

/* From IxOsalIoMem.h */
#define IX_OSAL_OEM_SWAP_LONG(wData)          ((wData >> 24) | (((wData >> 16) & 0xFF) << 8) | (((wData >> 8) & 0xFF) << 16) | ((wData & 0xFF) << 24))


/* PF specific definitions */
#define IX_OSAL_READ_LONG_LE(wAddr)          IX_OSAL_LE_BUSTOIAL(IX_OSAL_READ_LONG_IO((volatile UINT32 *) (wAddr) ))
#define IX_OSAL_READ_SHORT_LE(sAddr)         IX_OSAL_LE_BUSTOIAS(IX_OSAL_READ_SHORT_IO((volatile UINT16 *) (sAddr) ))
#define IX_OSAL_READ_BYTE_LE(bAddr)          IX_OSAL_LE_BUSTOIAB(IX_OSAL_READ_BYTE_IO((volatile UINT8 *) (bAddr) ))
#define IX_OSAL_WRITE_LONG_LE(wAddr, wData)  IX_OSAL_WRITE_LONG_IO((volatile UINT32 *) (wAddr), IX_OSAL_LE_IATOBUSL((UINT32) (wData) ))
#define IX_OSAL_WRITE_SHORT_LE(sAddr, sData) IX_OSAL_WRITE_SHORT_IO((volatile UINT16 *) (sAddr), IX_OSAL_LE_IATOBUSS((UINT16) (sData) ))
#define IX_OSAL_WRITE_BYTE_LE(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) (bAddr), IX_OSAL_LE_IATOBUSB((UINT8) (bData) ))


#define IX_OSAL_LE_IATOBUSL(wData)  (wData) 
#define IX_OSAL_LE_IATOBUSS(sData)  (sData) 
#define IX_OSAL_LE_IATOBUSB(bData)  (bData) 
#define IX_OSAL_LE_BUSTOIAL(wData)  (wData)
#define IX_OSAL_LE_BUSTOIAS(sData)  (sData)
#define IX_OSAL_LE_BUSTOIAB(bData)  (bData)


/* Platform-specific fastmutex implementation */
//PUBLIC IX_STATUS ixOsalOemFastMutexTryLock (IxOsalFastMutex * mutex);

/* Platform-specific init (MemMap) */
PUBLIC IX_STATUS
ixOsalOemInit (void);

/* Platform-specific unload (MemMap) */
PUBLIC void
ixOsalOemUnload (void);

#ifdef ENABLE_IOMEM

/**
 * Memory mapping table init. This function is public within OSAL only.
 * It is a dummy interface for EP805XX as dynamic memory map is done by HAL or 
   drivers instead of OSAL */
PUBLIC IX_STATUS
ixOsalMemMapInit (IxOsalMemoryMap *map, UINT32 numElement);

#endif /* ENABLE_IOMEM */

/* Default implementations */

PUBLIC UINT32
ixOsalEP805XXSharedTimestampGet (void);

UINT32
ixOsalEP805XXSharedTimestampRateGet (void);

UINT32
ixOsalEP805XXSharedSysClockRateGet (void);

void
ixOsalEP805XXSharedTimeGet (IxOsalTimeval * tv);


INT32
ixOsalEP805XXSharedLog (UINT32 level, UINT32 device, char *format, 
                       int arg1, int arg2, int arg3, int arg4, 
                       int arg5, int arg6);

/* A Null macro since IX_OSAL_LE is a valid endianness type for IA */
#define IX_OSAL_OEM_COMPONENT_COHERENCY_MODE_CHECK(fun_name, return_exp)

#endif /* IxOsalOem_H */
