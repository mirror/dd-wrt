/**
 * @file IxOsalOsOem.h
 *
 * @brief OS and platform specific definitions 
 *
 * Design Notes:
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

#ifndef IxOsalOsOem_H
#define IxOsalOsOem_H

/* This is used in IxOsalOs.h */
#include <asm/io.h>
 
/* This is used in IxOsalOs.h */
#include <linux/pci.h>
  
/* This is used in IxOsalOsTypes.h*/
#include <asm/atomic.h>

#include "IxOsalOsCdefs.h"

/*
 * Important Note: The current #defines in this file are just place-holders and should
 * be replaced and filled once the actual values are available.
 * This file has portions that has glue-code/logic/stubs and should cease to exist in 
 * the final code.
 */

/* physical addresses to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_EP805XX_DUMMY_PHYS_BASE         (0x0)

/* map sizes to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_EP805XX_DUMMY_MAP_SIZE          (0x0)	    /**< DUMMY map size */

/* virtual addresses to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_EP805XX_DUMMY_VIRT_BASE         (0x0) 

/*
 * Interrupt Levels
 */
#define IX_OSAL_EP805XX_DUMMY_IRQ_LVL     	   (0x0)


/*
 * IRQ for PMU
 */
/*#define IX_OSAL_OEM_IRQ_PMU		     	   	(0x0)*/
#define IX_OSAL_OEM_IRQ_PMU						IRQ_EP805XX_IA_PMU

/*
 * OS name retrieval
 */
#define IX_OSAL_OEM_OS_NAME_GET(name, limit) \
ixOsalOsEP805XXNameGet((INT8*)(name), (INT32) (limit))

/*
 * OS version retrieval
 */
#define IX_OSAL_OEM_OS_VERSION_GET(version, limit) \
ixOsalOsEP805XXVersionGet((INT8*)(version), (INT32) (limit))

/*
 * Function to retrieve the OS name
 */
PUBLIC IX_STATUS ixOsalOsEP805XXNameGet(INT8* osName, INT32 maxSize);

/*
 * Function to retrieve the OS version
 */
PUBLIC IX_STATUS ixOsalOsEP805XXVersionGet(INT8* osVersion, INT32 maxSize);

/* 
 * TimestampGet 
 */
PUBLIC UINT32 ixOsalOsEP805XXTimestampGet (void);

/*
 * Timestamp
 */
#define IX_OSAL_OEM_TIMESTAMP_GET ixOsalOsEP805XXTimestampGet


/*
 * Timestamp resolution
 */
PUBLIC UINT32 ixOsalOsEP805XXTimestampResolutionGet (void);

#define IX_OSAL_OEM_TIMESTAMP_RESOLUTION_GET ixOsalOsEP805XXTimestampResolutionGet

/* 
 * Retrieves the system clock rate 
 */
PUBLIC UINT32 ixOsalOsEP805XXSysClockRateGet (void);

#define IX_OSAL_OEM_SYS_CLOCK_RATE_GET ixOsalOsEP805XXSysClockRateGet

/*
 * required by FS but is not really platform-specific.
 */
#define IX_OSAL_OEM_TIME_GET(pTv) ixOsalTimeGet(pTv)

/*
 * Get PC
 */
#define IX_OSAL_OEM_GET_PC(regs) ixOsalOsEP805XXGetPc(regs)

#ifdef ENABLE_IOMEM

/* linux map/unmap functions */
PUBLIC void ixOsalLinuxMemMap (IxOsalMemoryMap * map);

PUBLIC void ixOsalLinuxMemUnmap (IxOsalMemoryMap * map);

#endif /* ENABLE_IOMEM */

#include "IxOsalOsOemSys.h"

						 
void ixOsalEP805XXSetInterruptedPc(struct pt_regs *regs);

#define IX_OSAL_OEM_SET_INTERRUPTED_PC(regs)    ixOsalEP805XXSetInterruptedPc(regs)
						  
/* ===================== End - Irq ====================== */


#define IX_OSAL_OEM_HOST_TO_NW_16(uData)  cpu_to_be16(uData)

#define IX_OSAL_OEM_HOST_TO_NW_32(uData)  cpu_to_be32(uData)  

#define IX_OSAL_OEM_HOST_TO_NW_64(uData)  cpu_to_be64(uData)

#define IX_OSAL_OEM_HOST_TO_NW_128(uDataSrc, uDataDest) \
  (uDataDest)->mUINT32[0] = IX_OSAL_OEM_HOST_TO_NW_32((uDataSrc)->mUINT32[0]);\
  (uDataDest)->mUINT32[1] = IX_OSAL_OEM_HOST_TO_NW_32((uDataSrc)->mUINT32[1]);\
  (uDataDest)->mUINT32[2] = IX_OSAL_OEM_HOST_TO_NW_32((uDataSrc)->mUINT32[2]);\
  (uDataDest)->mUINT32[3] = IX_OSAL_OEM_HOST_TO_NW_32((uDataSrc)->mUINT32[3]);


#define IX_OSAL_OEM_NW_TO_HOST_16(uData)  be16_to_cpu(uData)

#define IX_OSAL_OEM_NW_TO_HOST_32(uData)  be32_to_cpu(uData)

#define IX_OSAL_OEM_NW_TO_HOST_64(uData)  be64_to_cpu(uData)

#define IX_OSAL_OEM_NW_TO_HOST_128(uDataSrc, uDataDest) \
  (uDataDest)->mUINT32[0] = IX_OSAL_OEM_NW_TO_HOST_32((uDataSrc)->mUINT32[0]);\
  (uDataDest)->mUINT32[1] = IX_OSAL_OEM_NW_TO_HOST_32((uDataSrc)->mUINT32[1]);\
  (uDataDest)->mUINT32[2] = IX_OSAL_OEM_NW_TO_HOST_32((uDataSrc)->mUINT32[2]);\
  (uDataDest)->mUINT32[3] = IX_OSAL_OEM_NW_TO_HOST_32((uDataSrc)->mUINT32[3]);

#endif /* #define IxOsalOsOem_H */
