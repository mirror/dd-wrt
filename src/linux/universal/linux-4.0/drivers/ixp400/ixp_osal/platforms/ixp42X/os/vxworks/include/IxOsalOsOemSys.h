/**
 * @file IxOsalOsOemSys.h
 *
 * @brief vxworks and IXP4XX specific defines
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
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

#ifndef IxOsalOsOemSys_H
#define IxOsalOsOemSys_H

#ifndef IxOsalOsOem_H
#error "Error: IxOsalOsOemSys.h cannot be included directly before IxOsalOsOem.h!"
#endif

/* Temp workaround. Once 425 BSP is updated to use ixdp400.h, the check below will be removed */
#ifdef __ixp42X
#include "ixdp425.h"
#define EXP_CS_MEMSIZE SZ_16M
#else
#include "ixdp400.h"
#endif

/* Base Address */
#define IX_OSAL_IXP400_ETH_MAC_A0_PHYS_BASE    (0x0C800C000)           /* MAC on NPE-A */
#define IX_OSAL_IXP400_TIMESYNC_PHYS_BASE      IXP400_TIMESYNC_BASE
#define IX_OSAL_IXP400_PARITYEN_PHYS_BASE      (0xCC00E51C)
#define IX_OSAL_IXP400_I2C_PHYS_BASE           IXP400_I2C_BASE
#define IX_OSAL_IXP400_SSP_PHYS_BASE           IXP400_SSP_BASE
#define IX_OSAL_IXP400_SDRAM_BASE              IXP400_SDRAM_BASE
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_PHYS_BASE			(0x70000000) 
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_PHYS_BASE  (0x70002100)

/* Expansion Bus */
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE          IXP400_EXPANSION_BUS_BASE2
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE     IXP400_EXPANSION_BUS_BASE1
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE 			IXP400_EXPANSION_BUS_CS0_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE 			IXP400_EXPANSION_BUS_CS1_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS2_PHYS_BASE 			IXP400_EXPANSION_BUS_CS2_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS3_PHYS_BASE 			IXP400_EXPANSION_BUS_CS3_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE 			IXP400_EXPANSION_BUS_CS4_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS5_PHYS_BASE 			IXP400_EXPANSION_BUS_CS5_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS6_PHYS_BASE 			IXP400_EXPANSION_BUS_CS6_BASE	
#define IX_OSAL_IXP400_EXP_BUS_CS7_PHYS_BASE 			IXP400_EXPANSION_BUS_CS7_BASE	

/* Map Size */
#define IX_OSAL_IXP400_ETH_MAC_A0_MAP_SIZE        (0x1000)	    /**< Eth MAC for NPEA map size */
#define IX_OSAL_IXP400_I2C_MAP_SIZE               (0x10)       /**< I2C map size */
#define IX_OSAL_IXP400_SSP_MAP_SIZE               (0x14)       /**< SSP map size */
#define IX_OSAL_IXP400_TIMESYNC_MAP_SIZE          (0xA0)       /**< Time Sync map size */
#define IX_OSAL_IXP400_PARITYEN_MAP_SIZE          (0x100000)   /**< Parity map size */
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_MAP_SIZE	(0x200)  /**< Crypto RNG and SHA map size */
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_MAP_SIZE		(0x2100)  /**< Crypto EAU map size */
/* Expansion Bus*/
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE           (IXP400_EXPANSION_BUS_TOTAL_SIZE) /**< Expansion bus map size */
#define IX_OSAL_IXP400_EXP_BUS_CS0_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS0 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS1_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS1 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS2_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS2 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS3_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS3 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS4_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS4 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS5_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS5 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS6_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS6 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS7_MAP_SIZE  			(EXP_CS_MEMSIZE) /**< CS7 map size */


/* Platform specific Macros */
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


