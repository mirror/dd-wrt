/**
 * @file IxOsalOsIxp425Sys.h
 *
 * @brief linux and IXP425 specific defines
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

/* Memory Base Address */

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP4XX_EXP_BUS_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE  	0x00000000 /* IXP4XX_EXP_BUS_BASE1_PHYS is removed from kernel 2.6 */
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   	IXP4XX_EXP_BUS_CS0_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   	IXP4XX_EXP_BUS_CS1_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS2_PHYS_BASE   	IXP4XX_EXP_BUS_CS2_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS3_PHYS_BASE   	IXP4XX_EXP_BUS_CS3_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   	IXP4XX_EXP_BUS_CS4_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS5_PHYS_BASE   	IXP4XX_EXP_BUS_CS5_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS6_PHYS_BASE   	IXP4XX_EXP_BUS_CS6_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS7_PHYS_BASE   	IXP4XX_EXP_BUS_CS7_BASE_PHYS
#else
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP425_EXP_BUS_BASE2_PHYS
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE  	IXP425_EXP_BUS_BASE1_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   	IXP425_EXP_BUS_CS0_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   	IXP425_EXP_BUS_CS1_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS2_PHYS_BASE   	IXP425_EXP_BUS_CS2_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS3_PHYS_BASE   	IXP425_EXP_BUS_CS3_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   	IXP425_EXP_BUS_CS4_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS5_PHYS_BASE   	IXP425_EXP_BUS_CS5_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS6_PHYS_BASE   	IXP425_EXP_BUS_CS6_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS7_PHYS_BASE   	IXP425_EXP_BUS_CS7_BASE_PHYS
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

/* Memory Mapping size */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE  (IXP4XX_PERIPHERAL_REGION_SIZE)	    /**< Peripheral space map size */
#else
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE  (IXP425_PERIPHERAL_REGION_SIZE)	    /**< Peripheral space map size */
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

/* Expansion Bus */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE		(IXP4XX_EXP_BUS_CSX_REGION_SIZE*8)
#else
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE       (0x08000000)  /**< Total Expansion bus map size */
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
#define IX_OSAL_IXP400_EXP_BUS_CS0_MAP_SIZE 	(0x01000000) /**< CS0 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS1_MAP_SIZE 	(0x01000000) /**< CS1 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS2_MAP_SIZE 	(0x01000000) /**< CS2 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS3_MAP_SIZE 	(0x01000000) /**< CS3 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS4_MAP_SIZE 	(0x01000000) /**< CS4 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS5_MAP_SIZE 	(0x01000000) /**< CS5 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS6_MAP_SIZE 	(0x01000000) /**< CS6 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS7_MAP_SIZE 	(0x01000000) /**< CS7 map size */

/* Time Stamp Resolution */
#define IX_OSAL_IXP400_TIME_STAMP_RESOLUTION    (66666666) /* 66.66MHz */


/*********************
 *	Memory map
 ********************/

/* Note: - dynamic maps will be mapped using ioremap() with the base addresses and sizes declared in this array (matched to include the requested zones, 
           but not using the actual IxOsalMemoryMap requests) 
         - static maps have to be also declared in arch/arm/mach-ixp425/mm.c using the exact same addresses and size
         - the user-friendly name will be made available in /proc for dynamically allocated maps 
         - the declared order of the maps is important if the maps overlap - when a common zone is requested only the
           first usable map will be always chosen */


/* Global memmap only visible to IO MEM module */


#ifdef IxOsalIoMem_C
IxOsalMemoryMap ixOsalGlobalMemoryMap[] = {
    /*
     * Queue Manager 
     */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    {
     IX_OSAL_DYNAMIC_MAP,		/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,	/* size            */
     0,					/* virtualAddress  */
     ixOsalLinuxMemMap,			/* mapFunction     */
     ixOsalLinuxMemUnmap,		/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,	/* endianType      */   
     "qMgr"				/* name            */
     },
#else
    {
     IX_OSAL_STATIC_MAP,		/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_QMGR_VIRT_BASE,	/* virtualAddress  */
     NULL,				/* mapFunction     */
     NULL,				/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,	/* endianType      */   
     "qMgr"				/* name            */
     },
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

    /*
     * APB Peripherals 
     */
    {
     IX_OSAL_STATIC_MAP,			/* type            */
     IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_PERIPHERAL_VIRT_BASE,	/* virtualAddress  */
     NULL,					/* mapFunction     */
     NULL,					/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,		/* endianType      */
     "peripherals"				/* name            */
     },

    /*
     * Expansion Bus Config Registers 
     */
    {
     IX_OSAL_STATIC_MAP,		/* type            */
     IX_OSAL_IXP400_EXP_CFG_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_EXP_REG_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_EXP_CFG_VIRT_BASE,	/* virtualAddress  */
     NULL,				/* mapFunction     */
     NULL,				/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "Exp Cfg"				/* name            */
     },

    /*
     * PCI config Registers 
     */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    {
     IX_OSAL_DYNAMIC_MAP,		/* type            */
     IX_OSAL_IXP400_PCI_CFG_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PCI_CFG_MAP_SIZE,	/* size            */
     0,					/* virtualAddress  */
     ixOsalLinuxMemMap,			/* mapFunction     */
     ixOsalLinuxMemUnmap,		/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "pciConfig"			/* name            */
     },
#else
    {
     IX_OSAL_STATIC_MAP,		/* type            */
     IX_OSAL_IXP400_PCI_CFG_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PCI_CFG_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_PCI_CFG_VIRT_BASE,	/* virtualAddress  */
     NULL,				/* mapFunction     */
     NULL,				/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "pciConfig"			/* name            */
     },
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

    /*
     * Expansion Bus 
     */
    {
     IX_OSAL_DYNAMIC_MAP,		/* type            */
     IX_OSAL_IXP400_EXP_BUS_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_EXP_BUS_MAP_SIZE,	/* size            */
     0,					/* virtualAddress  */
     ixOsalLinuxMemMap,			/* mapFunction     */
     ixOsalLinuxMemUnmap,		/* unmapFunction   */
     0,					/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "Exp Bus"				/* name            */
     }
};

#endif /* IxOsalIoMem_C */

#endif

