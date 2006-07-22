/**
 * @file IxOsalOsIxp425Sys.h
 *
 * @brief linux and IXP425 specific defines
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifndef IxOsalOsIxp425Sys_H
#define IxOsalOsIxp425Sys_H

#ifndef IxOsalOsIxp400_H
#error "Error: IxOsalOsIxp425Sys.h cannot be included directly before IxOsalOsIxp400.h"
#endif

/* Memory Base Address */
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP4XX_EXP_BUS_BASE2_PHYS
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE  	IXP4XX_EXP_BUS_BASE1_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   	IXP4XX_EXP_BUS_CS0_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   	IXP4XX_EXP_BUS_CS1_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   	IXP4XX_EXP_BUS_CS4_BASE_PHYS

/* Memory Mapping size */
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE  	IXP4XX_PERIPHERAL_REGION_SIZE    /**< Peripheral space map size */
/* Expansion Bus */
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE       (0x08000000)  /**< Total Expansion bus map size */
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
    {
     IX_OSAL_DYNAMIC_MAP,	/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,	/* size            */
     0,				/* virtualAddress  : IX_OSAL_IXP400_QMGR_VIRT_BASE */
     ixOsalLinuxMemMap,		/* mapFunction     */
     ixOsalLinuxMemUnmap,	/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,	/* endianType      */   
     "qMgr"			/* name            */
     },

    /*
     * APB Peripherals 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_PERIPHERAL_VIRT_BASE,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "peripherals"		/* name            */
     },

    /*
     * Expansion Bus Config Registers 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_EXP_CFG_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_EXP_REG_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_EXP_CFG_VIRT_BASE,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "Exp Cfg"			/* name            */
     },

    /*
     * PCI config Registers 
     */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     IX_OSAL_IXP400_PCI_CFG_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_PCI_CFG_MAP_SIZE,	/* size            */
     IX_OSAL_IXP400_PCI_CFG_VIRT_BASE,	/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "pciConfig"		/* name            */
     },

    /*
     * Expansion Bus 
     */
    {
     IX_OSAL_DYNAMIC_MAP,	/* type            */
     IX_OSAL_IXP400_EXP_BUS_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_IXP400_EXP_BUS_MAP_SIZE,	/* size            */
     0,				/* virtualAddress  */
     ixOsalLinuxMemMap,		/* mapFunction     */
     ixOsalLinuxMemUnmap,	/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	/* endianType      */
     "Exp Bus"			/* name            */
     }
};

#endif /* IxOsalIoMem_C */

#endif

