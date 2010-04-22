/**
 * @file IxOsalOsIxp425Sys.h
 *
 * @brief vxworks and IXP425 specific defines
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalOsIxp425Sys_H
#define IxOsalOsIxp425Sys_H

#ifndef IxOsalOsIxp400_H
#error "Error: IxOsalOsIxp425Sys.h cannot be included directly before IxOsalOsIxp400.h"
#endif
#include "ixdp425.h"

/* Base Address */
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP425_EXPANSION_BUS_BASE2
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE   IXP425_EXPANSION_BUS_BASE1
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   	IXP425_EXPANSION_BUS_CS0_BASE
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   	IXP425_EXPANSION_BUS_CS1_BASE
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   	IXP425_EXPANSION_BUS_CS4_BASE

/* Memory Map */
/* Expansion Bus Map size*/
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE        (0x08000000) /**< Total  Expansion bus map size */
#define IX_OSAL_IXP400_EXP_BUS_CS0_MAP_SIZE    (0x01000000) /**< CS0 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS1_MAP_SIZE    (0x01000000) /**< CS1 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS2_MAP_SIZE    (0x01000000) /**< CS2 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS3_MAP_SIZE    (0x01000000) /**< CS3 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS4_MAP_SIZE    (0x01000000) /**< CS4 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS5_MAP_SIZE    (0x01000000) /**< CS5 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS6_MAP_SIZE    (0x01000000) /**< CS6 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS7_MAP_SIZE    (0x01000000) /**< CS7 map size */

#define IX_OSAL_IXP400_TIME_STAMP_RESOLUTION    (66666666) /* 66.66MHz */

/* Global memmap only visible to IO MEM module */

#ifdef IxOsalIoMem_C

static IxOsalMemoryMap ixOsalGlobalMemoryMap[] = {
    /*
     * Global BE and LE_AC map 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x00000000,		/* physicalAddress */
     0xFFFFFFFF,		/* size            */
     0x00000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* coherency       */
     "global"			/* name            */
     },

    /*
     * QMgr LE_DC map 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_QMGR_LE_DC_VIRT,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_LE_DC,		/* coherency       */
     "qMgr LE_DC"		/* name            */
     },

    /*
     * Expansion Bus Config LE_DC map 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_EXP_CONFIG_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_EXP_CONFIG_LE_DC_VIRT,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_LE_DC,		/* coherency       */
     "Exp Cfg LE_DC"		/* name            */
     },

    /*
     * Peripherals LE_DC map 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_PERIPHERAL_LE_DC_VIRT,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_LE_DC,		/* coherency       */
     "peripherals LE_DC"	/* name            */
     }
};
#endif /* IxOsalIoMem_C visible */
#endif


