/**
 * @file IxOsalOsIxp465Sys.h
 *
 * @brief linux and IXP465 specific defines
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

#ifndef IxOsalOsIxp465Sys_H
#define IxOsalOsIxp465Sys_H

#ifndef IxOsalOsIxp400_H
#error "Error: IxOsalOsIxp465Sys.h cannot be included directly before IxOsalOsIxp400.h"
#endif

/* Memory Base Address */
#define IX_OSAL_IXP400_ETH_MAC_A0_PHYS_BASE    IXP4XX_EthA_BASE_PHYS           /* MAC on NPE-A */
#define IX_OSAL_IXP400_TIMESYNC_PHYS_BASE      IXP4XX_TIMESYNC_BASE_PHYS
#define IX_OSAL_IXP400_PARITYEN_PHYS_BASE      (0xCC00E51C)
#define IX_OSAL_IXP400_I2C_PHYS_BASE           IXP4XX_I2C_BASE_PHYS
#define IX_OSAL_IXP400_SSP_PHYS_BASE           IXP4XX_SSP_BASE_PHYS
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_PHYS_BASE			(0x70000000) 
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_PHYS_BASE		(0x70002100)
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP4XX_EXP_BUS_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE	0x00000000	/* This definition being removed from Linux 2.6 */
#else
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       	IXP425_EXP_BUS_BASE2_PHYS
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE  	IXP425_EXP_BUS_BASE1_PHYS
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   	IXP4XX_EXP_BUS_CS0_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   	IXP4XX_EXP_BUS_CS1_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS2_PHYS_BASE   	IXP4XX_EXP_BUS_CS2_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS3_PHYS_BASE   	IXP4XX_EXP_BUS_CS3_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   	IXP4XX_EXP_BUS_CS4_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS5_PHYS_BASE   	IXP4XX_EXP_BUS_CS5_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS6_PHYS_BASE   	IXP4XX_EXP_BUS_CS6_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_CS7_PHYS_BASE   	IXP4XX_EXP_BUS_CS7_BASE_PHYS

/* Memory Mapping */
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE  IXP4XX_PERIPHERAL_REGION_SIZE    /**< Peripheral space map size */
#define IX_OSAL_IXP400_ETH_MAC_A0_MAP_SIZE    (0x1000)	    /**< Eth B map size */
#define IX_OSAL_IXP400_I2C_MAP_SIZE         	(0x10)          /**< I2C map size */
#define IX_OSAL_IXP400_SSP_MAP_SIZE         	(0x14)          /**< SSP map size */
#define IX_OSAL_IXP400_TIMESYNC_MAP_SIZE    	(0x100)         /**< Time Sync map size */
#define IX_OSAL_IXP400_PARITYEN_MAP_SIZE    	(0x100000)      /**< Parity map size */
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_MAP_SIZE	(0x200)  /**< Crypto RNG and SHA map size */
#define IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_MAP_SIZE		(0x2100)  /**< Crypto EAU map size */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE		(IXP4XX_EXP_BUS_CSX_REGION_SIZE*8)
#else
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE     	(IXP4XX_EXP_BUS_REGION_SIZE) /**< Total Expansion bus map size */
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
#define IX_OSAL_IXP400_EXP_BUS_CS0_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS0 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS1_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS1 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS2_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS2 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS3_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS3 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS4_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS4 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS5_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS5 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS6_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS6 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS7_MAP_SIZE 	(IXP4XX_EXP_BUS_CSX_REGION_SIZE) /**< CS7 map size */

/* Interrupt IRQ Level */
#define IX_OSAL_IXP400_USB_HOST_IRQ_LVL			(32)
#define IX_OSAL_IXP400_I2C_IRQ_LVL			(33)
#define IX_OSAL_IXP400_SSP_IRQ_LVL 			(34)
#define IX_OSAL_IXP400_TSYNC_IRQ_LVL			(35)
#define IX_OSAL_IXP400_EAU_DONE_IRQ_LVL			(36)
#define IX_OSAL_IXP400_SHA_HASHING_DONE_IRQ_LVL		(37)
#define IX_OSAL_IXP400_RSVD_38_IRQ_LVL			(38)
#define IX_OSAL_IXP400_RSVD_39_IRQ_LVL			(39)
#define IX_OSAL_IXP400_RSVD_40_IRQ_LVL			(40)
#define IX_OSAL_IXP400_RSVD_41_IRQ_LVL			(41)
#define IX_OSAL_IXP400_RSVD_42_IRQ_LVL			(42)
#define IX_OSAL_IXP400_RSVD_43_IRQ_LVL			(43)
#define IX_OSAL_IXP400_RSVD_44_IRQ_LVL			(44)
#define IX_OSAL_IXP400_RSVD_45_IRQ_LVL			(45)
#define IX_OSAL_IXP400_RSVD_46_IRQ_LVL			(46)
#define IX_OSAL_IXP400_RSVD_47_IRQ_LVL			(47)
#define IX_OSAL_IXP400_RSVD_48_IRQ_LVL			(48)
#define IX_OSAL_IXP400_RSVD_49_IRQ_LVL			(49)
#define IX_OSAL_IXP400_RSVD_50_IRQ_LVL			(50)
#define IX_OSAL_IXP400_RSVD_51_IRQ_LVL			(51)
#define IX_OSAL_IXP400_RSVD_52_IRQ_LVL			(52)
#define IX_OSAL_IXP400_RSVD_53_IRQ_LVL			(53)
#define IX_OSAL_IXP400_RSVD_54_IRQ_LVL			(54)
#define IX_OSAL_IXP400_RSVD_55_IRQ_LVL			(55)
#define IX_OSAL_IXP400_RSVD_56_IRQ_LVL			(56)
#define IX_OSAL_IXP400_RSVD_57_IRQ_LVL			(57)
#define IX_OSAL_IXP400_SWCP_IRQ_LVL			(58)
#define IX_OSAL_IXP400_RSVD_59_IRQ_LVL			(59)
#define IX_OSAL_IXP400_AQM_IRQ_LVL	  		(60)
#define IX_OSAL_IXP400_MCU_IRQ_LVL			(61)
#define IX_OSAL_IXP400_EBC_IRQ_LVL			(62)
#define IX_OSAL_IXP400_RSVD_63_IRQ_LVL			(63)	

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
     IX_OSAL_DYNAMIC_MAP,			/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,		/* size            */
     0,						/* virtualAddress  */
     ixOsalLinuxMemMap,				/* mapFunction     */
     ixOsalLinuxMemUnmap,			/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,		/* endianType      */   
     "qMgr"					/* name            */
     },
#else
    {
     IX_OSAL_STATIC_MAP,			/* type            */
     IX_OSAL_IXP400_QMGR_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_QMGR_MAP_SIZE,		/* size            */
     IX_OSAL_IXP400_QMGR_VIRT_BASE,		/* virtualAddress  */
     NULL,					/* mapFunction     */
     NULL,					/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,		/* endianType      */   
     "qMgr"					/* name            */
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
     IX_OSAL_STATIC_MAP,			/* type            */
     IX_OSAL_IXP400_EXP_CFG_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_EXP_REG_MAP_SIZE,		/* size            */
     IX_OSAL_IXP400_EXP_CFG_VIRT_BASE,		/* virtualAddress  */
     NULL,					/* mapFunction     */
     NULL,					/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,		/* endianType      */
     "Exp Cfg"					/* name            */
     },

    /*
     * PCI config Registers 
     */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    {
     IX_OSAL_DYNAMIC_MAP,			/* type            */
     IX_OSAL_IXP400_PCI_CFG_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_PCI_CFG_MAP_SIZE,		/* size            */
     0,						/* virtualAddress  */
     ixOsalLinuxMemMap,				/* mapFunction     */
     ixOsalLinuxMemUnmap,			/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,		/* endianType      */
     "pciConfig"				/* name            */
     },
#else
    {
     IX_OSAL_STATIC_MAP,			/* type            */
     IX_OSAL_IXP400_PCI_CFG_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_PCI_CFG_MAP_SIZE,		/* size            */
     IX_OSAL_IXP400_PCI_CFG_VIRT_BASE,		/* virtualAddress  */
     NULL,					/* mapFunction     */
     NULL,					/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,		/* endianType      */
     "pciConfig"				/* name            */
     },
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

    /*
     * Expansion Bus 
     */
    {
     IX_OSAL_DYNAMIC_MAP,			/* type            */
     IX_OSAL_IXP400_EXP_BUS_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_EXP_BUS_MAP_SIZE,		/* size            */
     0,						/* virtualAddress  */
     ixOsalLinuxMemMap,				/* mapFunction     */
     ixOsalLinuxMemUnmap,			/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,		/* endianType      */
     "Exp Bus"					/* name            */
     },

     /*
     * Parity Error Notifier config Registers 
     */
    {
     IX_OSAL_DYNAMIC_MAP,	            	/* type            */
     IX_OSAL_IXP400_PARITYEN_PHYS_BASE,		/* physicalAddress */
     IX_OSAL_IXP400_PARITYEN_MAP_SIZE,		/* size            */
     0,						/* virtualAddress  */
     ixOsalLinuxMemMap,				/* mapFunction     */
     ixOsalLinuxMemUnmap,	            	/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	    	/* endianType      */
     "Parity Error Notifier - MCU"	    	/* name            */
    },

    /*
     * Crypto Registers and RAM
     */
    {
     IX_OSAL_DYNAMIC_MAP,	            /* type            */
     IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_PHYS_BASE,    /* physicalAddress */
     IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_EAU_MAP_SIZE,	/* size            */
     0,				                    		    /* virtualAddress  */
     ixOsalLinuxMemMap,		            /* mapFunction     */
     ixOsalLinuxMemUnmap,	            /* unmapFunction   */
     0,				                    /* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,	    /* endianType      */
     "PKE crypto EAU"	    		    /* name            */
     },

    /*
     * Crypto Registers and RAM
     */
    {
     IX_OSAL_DYNAMIC_MAP,	            /* type            */
     IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_PHYS_BASE,    /* physicalAddress */
     IX_OSAL_IXP400_PKE_CRYPTO_ENGINE_RNG_SHA_MAP_SIZE,	/* size            */
     0,				                    		    /* virtualAddress  */
     ixOsalLinuxMemMap,		            /* mapFunction     */
     ixOsalLinuxMemUnmap,	            /* unmapFunction   */
     0,				                    /* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_DC,	    /* endianType      */
     "PKE crypto SHA RNG"	    		    /* name            */
     }


};

#endif /* IxOsalIoMem_C */

#endif

