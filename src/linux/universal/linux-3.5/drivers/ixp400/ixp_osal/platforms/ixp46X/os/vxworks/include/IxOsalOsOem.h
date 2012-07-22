/**
 * @file IxOsalOsOem.h 
 *
 * @brief vxworks and IXP400 specific defines
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


/* IXP400 -specific, vxWorks defines */
#ifndef IxOsalOsOem_H
#define IxOsalOsOem_H

/* This a temp workaround for ixp400IntrCtl.h, it'll be removed once device segregation is removed */
#define IX_DEVICE_IXP465 1

#include <ixp400.h>
#include <drv/intrCtl/ixp400IntrCtl.h>


/* Define Common IXP4XX Device physical base address here*/
/* Base Address */
#define IX_OSAL_IXP400_INTC_PHYS_BASE           IXP400_INTC_BASE
#define IX_OSAL_IXP400_GPIO_PHYS_BASE           IXP400_GPIO_BASE
#define IX_OSAL_IXP400_UART1_PHYS_BASE          IXP400_UART1_BASE
#define IX_OSAL_IXP400_UART2_PHYS_BASE          IXP400_UART2_BASE
#define IX_OSAL_IXP400_ETH_MAC_B0_PHYS_BASE     IXP400_EthA_BASE  /* MAC on NPE B */
#define IX_OSAL_IXP400_ETH_MAC_C0_PHYS_BASE     IXP400_EthB_BASE  /* MAC on NPE C */
#define IX_OSAL_IXP400_NPEA_PHYS_BASE           IXP400_NPEA_BASE  /* NPEA*/
#define IX_OSAL_IXP400_NPEB_PHYS_BASE           IXP400_NPEB_BASE  /* NPEB*/
#define IX_OSAL_IXP400_NPEC_PHYS_BASE           IXP400_NPEC_BASE  /* NPEC*/
#define IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE     IXP400_PERIPHERAL_BASE
#define IX_OSAL_IXP400_QMGR_PHYS_BASE           IXP400_QMGR_BASE
#define IX_OSAL_IXP400_OSTS_PHYS_BASE          	IXP400_OSTS
#define IX_OSAL_IXP400_USB_PHYS_BASE           	IXP400_USB_BASE
#define IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE   IXP400_EXP_CONFIG_BASE
#define IX_OSAL_IXP400_EXP_CFG_PHYS_BASE        IXP400_EXP_CONFIG_BASE
#define IX_OSAL_IXP400_PCI_CFG_PHYS_BASE        IXP400_PCI_CONFIG_BASE
#define IX_OSAL_IXP400_SDRAM_CFG_PHYS_BASE      IXP400_SDRAM_CONFIG_BASE
#define IX_OSAL_IXP400_PMU_PHYS_BASE            IXP400_PMU_BASE

/* Define Common Device Map size here*/
/* map sizes to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_QMGR_MAP_SIZE           (0x4000)	    /**< Queue Manager map size */
#define IX_OSAL_IXP400_EXP_REG_MAP_SIZE        (0x1000)	    /**< Exp Bus Registers map size */
#define IX_OSAL_IXP400_UART1_MAP_SIZE          (0x1000)	    /**< UART1 map size */
#define IX_OSAL_IXP400_UART2_MAP_SIZE          (0x1000)	    /**< UART2 map size */
#define IX_OSAL_IXP400_PMU_MAP_SIZE            (0x1000)	    /**< PMU map size */
#define IX_OSAL_IXP400_OSTS_MAP_SIZE           (0x1000)	    /**< OS Timers map size */
#define IX_OSAL_IXP400_NPEA_MAP_SIZE           (0x1000)	    /**< NPE A map size */
#define IX_OSAL_IXP400_NPEB_MAP_SIZE           (0x1000)	    /**< NPE B map size */
#define IX_OSAL_IXP400_NPEC_MAP_SIZE           (0x1000)	    /**< NPE C map size */
#define IX_OSAL_IXP400_ETH_MAC_B0_MAP_SIZE     (0x1000)	    /**< MAC for NPE B map size */
#define IX_OSAL_IXP400_ETH_MAC_C0_MAP_SIZE     (0x1000)	    /**< MAC for NPE C map size */
#define IX_OSAL_IXP400_USB_MAP_SIZE            (0x1000)	    /**< USB map size */
#define IX_OSAL_IXP400_GPIO_MAP_SIZE           (0x1000)	    /**< GPIO map size */
#define IX_OSAL_IXP400_EXP_REG_MAP_SIZE        (0x1000)	    /**< Exp Bus Registers map size */
#define IX_OSAL_IXP400_PCI_CFG_MAP_SIZE        (0x1000)	    /**< PCI Bus Config Registers map size */
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE     IXP400_PERIPHERAL_SIZE
#define IX_OSAL_IXP400_EXP_CONFIG_MAP_SIZE     IXP400_EXP_CONFIG_SIZE

/*
 * Interrupt Levels
 */
#define IX_OSAL_IXP400_NPEA_IRQ_LVL      IXP400_INT_LVL_NPEA
#define IX_OSAL_IXP400_NPEB_IRQ_LVL      IXP400_INT_LVL_NPEB
#define IX_OSAL_IXP400_NPEC_IRQ_LVL      IXP400_INT_LVL_NPEC
#define IX_OSAL_IXP400_QM1_IRQ_LVL       IXP400_INT_LVL_QM1
#define IX_OSAL_IXP400_QM2_IRQ_LVL       IXP400_INT_LVL_QM2
#define IX_OSAL_IXP400_TIMER1_IRQ_LVL    IXP400_INT_LVL_TIMER1
#define IX_OSAL_IXP400_GPIO0_IRQ_LVL     IXP400_INT_LVL_GPIO0
#define IX_OSAL_IXP400_GPIO1_IRQ_LVL     IXP400_INT_LVL_GPIO1
#define IX_OSAL_IXP400_PCI_INT_IRQ_LVL   IXP400_INT_LVL_PCI_INT
#define IX_OSAL_IXP400_PCI_DMA1_IRQ_LVL	 IXP400_INT_LVL_PCI_DMA1
#define IX_OSAL_IXP400_PCI_DMA2_IRQ_LVL	 IXP400_INT_LVL_PCI_DMA2
#define IX_OSAL_IXP400_TIMER2_IRQ_LVL    IXP400_INT_LVL_TIMER2	
#define IX_OSAL_IXP400_USB_IRQ_LVL       IXP400_INT_LVL_USB
#define IX_OSAL_IXP400_UART2_IRQ_LVL     IXP400_INT_LVL_UART2
#define IX_OSAL_IXP400_TIMESTAMP_IRQ_LVL IXP400_INT_LVL_TIMESTAMP
#define IX_OSAL_IXP400_UART1_IRQ_LVL     IXP400_INT_LVL_UART1
#define IX_OSAL_IXP400_WDOG_IRQ_LVL      IXP400_INT_LVL_WDOG
#define IX_OSAL_IXP400_AHB_PMU_IRQ_LVL   IXP400_INT_LVL_AHB_PMU
#define IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL IXP400_INT_LVL_XSCALE_PMU
#define IX_OSAL_IXP400_GPIO2_IRQ_LVL     IXP400_INT_LVL_GPIO2
#define IX_OSAL_IXP400_GPIO3_IRQ_LVL     IXP400_INT_LVL_GPIO3
#define IX_OSAL_IXP400_GPIO4_IRQ_LVL     IXP400_INT_LVL_GPIO4
#define IX_OSAL_IXP400_GPIO5_IRQ_LVL     IXP400_INT_LVL_GPIO5
#define IX_OSAL_IXP400_GPIO6_IRQ_LVL     IXP400_INT_LVL_GPIO6
#define IX_OSAL_IXP400_GPIO7_IRQ_LVL     IXP400_INT_LVL_GPIO7
#define IX_OSAL_IXP400_GPIO8_IRQ_LVL     IXP400_INT_LVL_GPIO8
#define IX_OSAL_IXP400_GPIO9_IRQ_LVL     IXP400_INT_LVL_GPIO9
#define IX_OSAL_IXP400_GPIO10_IRQ_LVL    IXP400_INT_LVL_GPIO10
#define IX_OSAL_IXP400_GPIO11_IRQ_LVL    IXP400_INT_LVL_GPIO11
#define IX_OSAL_IXP400_GPIO12_IRQ_LVL    IXP400_INT_LVL_GPIO12
#define IX_OSAL_IXP400_SW_INT1_IRQ_LVL   IXP400_INT_LVL_INT1
#define IX_OSAL_IXP400_SW_INT2_IRQ_LVL   IXP400_INT_LVL_INT2

#define IX_OSAL_IXP400_USB_HOST_IRQ_LVL  IXP400_INT_LVL_USB_HOST
#define IX_OSAL_IXP400_I2C_IRQ_LVL  	 IXP400_INT_LVL_I2C
#define IX_OSAL_IXP400_USB_HOST1_IRQ_LVL IXP400_INT_LVL_USB_HOST1
#define IX_OSAL_IXP400_SSP_IRQ_LVL       IXP400_INT_LVL_SSP
#define IX_OSAL_IXP400_TSYNC_IRQ_LVL  	 IXP400_INT_LVL_TSYNC
#define IX_OSAL_IXP400_EAU_DONE_IRQ_LVL	 IXP400_INT_LVL_EAU_DONE
#define IX_OSAL_IXP400_SHA_HASHING_DONE_IRQ_LVL	IXP400_INT_LVL_SHA_HASHING_DONE
#define IX_OSAL_IXP400_SWCP_IRQ_LVL		 IXP400_INT_LVL_SWCP
#define IX_OSAL_IXP400_AQM_IRQ_LVL       IXP400_INT_LVL_AQM
#define IX_OSAL_IXP400_MCU_IRQ_LVL       IXP400_INT_LVL_MCU
#define IX_OSAL_IXP400_EBC_IRQ_LVL		 IXP400_INT_LVL_EBC

/* USB interrupt level mask */
#define IX_OSAL_IXP400_INT_LVL_USB       IXP400_INT_LVL_USB

/* USB IRQ */
#define IX_OSAL_IXP400_USB_IRQ           IXP400_INT_LVL_USB

#define IX_OSAL_IXP400_QMGR_LE_DC_VIRT        IXP400_QMGR_LE_DC_VIRT
#define IX_OSAL_IXP400_EXP_CONFIG_LE_DC_VIRT	IXP400_EXP_CONFIG_LE_DC_VIRT
#define IX_OSAL_IXP400_PERIPHERAL_LE_DC_VIRT	IXP400_PERIPHERAL_LE_DC_VIRT

#define IX_OSAL_OEM_APB_CLOCK 	   		(66)  /* 66.66MHz*/

/*
 * Interrupt Controller Registers
 */
#define IX_OSAL_IXP400_ICMR    IXP400_ICMR
#define IX_OSAL_IXP400_ICLR    IXP400_ICLR

/* 
 * TimestampGet 
 */
PUBLIC UINT32 ixOsalOsIxp400TimestampGet (void);

/*
 * Timestamp
 */
#define IX_OSAL_OEM_TIMESTAMP_GET ixOsalOsIxp400TimestampGet

/*
 * Timestamp resolution
 */
PUBLIC UINT32 ixOsalOsIxp400TimestampResolutionGet (void);

#define IX_OSAL_OEM_TIMESTAMP_RESOLUTION_GET ixOsalOsIxp400TimestampResolutionGet


/*
 * required by FS but is not really platform-specific for vxworks.
 */
#define IX_OSAL_OEM_SYS_CLOCK_RATE_GET ixOsalSysClockRateGet


/*
 * required by FS but is not really platform-specific.
 */
#define IX_OSAL_OEM_TIME_GET(pTv) ixOsalTimeGet(pTv)

/*
 * OS name retrieval
 */
#define IX_OSAL_OEM_OS_NAME_GET(name, limit) \
ixOsalOsIxp400NameGet((char*)(name), (INT32) (limit))

/*
 * OS version retrieval
 */
#define IX_OSAL_OEM_OS_VERSION_GET(version, limit) \
ixOsalOsIxp400VersionGet((char*)(version), (INT32) (limit))

/*
 * Returns OS name string
 */
PUBLIC IX_STATUS
ixOsalOsIxp400NameGet(char* osName, INT32 maxSize);

/*
 * Returns OS version string
 */
PUBLIC IX_STATUS
ixOsalOsIxp400VersionGet(char* osVersion, INT32 maxSize);

/* 
NOTE: Include the apppropriate (IXP400 specific) Platform system Header file here 
			Platform - ixp425, ixp465, ixp435, and etc
*/
#include  "IxOsalOsOemSys.h"

#endif /*IxOsalOsOem_H */
