/**
 * @file IxOsalOsOem.h
 *
 * @brief OS and platform specific definitions 
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

#ifndef IxOsalOsOem_H
#define IxOsalOsOem_H

#include <mach/hardware.h>
#include <mach/irqs.h>

#include "IxOsalOsTypes.h"


/* physical addresses to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_INTC_PHYS_BASE          IXP4XX_INTC_BASE_PHYS
#define IX_OSAL_IXP400_GPIO_PHYS_BASE          IXP4XX_GPIO_BASE_PHYS
#define IX_OSAL_IXP400_UART1_PHYS_BASE         IXP4XX_UART1_BASE_PHYS
#define IX_OSAL_IXP400_UART2_PHYS_BASE         IXP4XX_UART2_BASE_PHYS
#define IX_OSAL_IXP400_ETH_MAC_B0_PHYS_BASE    IXP4XX_EthB_BASE_PHYS   /* MAC on NPE-B */
#define IX_OSAL_IXP400_ETH_MAC_C0_PHYS_BASE    IXP4XX_EthC_BASE_PHYS   /* MAC on NPE-C */
#define IX_OSAL_IXP400_NPEA_PHYS_BASE          IXP4XX_NPEA_BASE_PHYS
#define IX_OSAL_IXP400_NPEB_PHYS_BASE          IXP4XX_NPEB_BASE_PHYS
#define IX_OSAL_IXP400_NPEC_PHYS_BASE          IXP4XX_NPEC_BASE_PHYS
#define IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE    IXP4XX_PERIPHERAL_BASE_PHYS
#define IX_OSAL_IXP400_QMGR_PHYS_BASE          IXP4XX_QMGR_BASE_PHYS
#define IX_OSAL_IXP400_OSTS_PHYS_BASE          IXP4XX_TIMER_BASE_PHYS
#define IX_OSAL_IXP400_USB_PHYS_BASE           IXP4XX_USB_BASE_PHYS
#define IX_OSAL_IXP400_EXP_CFG_PHYS_BASE       IXP4XX_EXP_CFG_BASE_PHYS
#define IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE  IXP4XX_EXP_CFG_BASE_PHYS
#define IX_OSAL_IXP400_PCI_CFG_PHYS_BASE       IXP4XX_PCI_CFG_BASE_PHYS
#define IX_OSAL_IXP400_SDRAM_CFG_PHYS_BASE     (0xCC000000)
#define IX_OSAL_IXP400_PMU_PHYS_BASE           IXP4XX_PMU_BASE_PHYS

/* map sizes to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_QMGR_MAP_SIZE        (0x4000)	    /**< Queue Manager map size */
#define IX_OSAL_IXP400_UART1_MAP_SIZE       (0x1000)	    /**< UART1 map size */
#define IX_OSAL_IXP400_UART2_MAP_SIZE       (0x1000)	    /**< UART2 map size */
#define IX_OSAL_IXP400_PMU_MAP_SIZE         (0x1000)	    /**< PMU map size */
#define IX_OSAL_IXP400_OSTS_MAP_SIZE        (0x1000)	    /**< OS Timers map size */
#define IX_OSAL_IXP400_NPEA_MAP_SIZE        (0x1000)	    /**< NPE A map size */
#define IX_OSAL_IXP400_NPEB_MAP_SIZE        (0x1000)	    /**< NPE B map size */
#define IX_OSAL_IXP400_NPEC_MAP_SIZE        (0x1000)	    /**< NPE C map size */
#define IX_OSAL_IXP400_ETH_MAC_B0_MAP_SIZE  (0x1000)	    /**< MAC Eth NPEB map size */
#define IX_OSAL_IXP400_ETH_MAC_C0_MAP_SIZE  (0x1000)	    /**< MAC Eth NPEC map size */
#define IX_OSAL_IXP400_USB_MAP_SIZE         (0x1000)	    /**< USB map size */
#define IX_OSAL_IXP400_GPIO_MAP_SIZE        (0x1000)	    /**< GPIO map size */
#define IX_OSAL_IXP400_EXP_REG_MAP_SIZE	    (0x1000)	    /**< Exp Bus Config Registers map size */
#define IX_OSAL_IXP400_PCI_CFG_MAP_SIZE     (0x1000)	    /**< PCI Bus Config Registers map size */

/* virtual addresses to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_GPIO_VIRT_BASE          IXP4XX_GPIO_BASE_VIRT
#define IX_OSAL_IXP400_UART1_VIRT_BASE         IXP4XX_UART1_BASE_VIRT
#define IX_OSAL_IXP400_UART2_VIRT_BASE         IXP4XX_UART2_BASE_VIRT
#define IX_OSAL_IXP400_ETHA_VIRT_BASE          IXP4XX_EthA_BASE_VIRT
#define IX_OSAL_IXP400_ETHB_VIRT_BASE          IXP4XX_EthB_BASE_VIRT
#define IX_OSAL_IXP400_NPEA_VIRT_BASE          IXP4XX_NPEA_BASE_VIRT
#define IX_OSAL_IXP400_NPEB_VIRT_BASE          IXP4XX_NPEB_BASE_VIRT
#define IX_OSAL_IXP400_NPEC_VIRT_BASE          IXP4XX_NPEC_BASE_VIRT
#define IX_OSAL_IXP400_PERIPHERAL_VIRT_BASE    IXP4XX_PERIPHERAL_BASE_VIRT
#define IX_OSAL_IXP400_QMGR_VIRT_BASE          IXP4XX_QMGR_BASE_VIRT
#define IX_OSAL_IXP400_OSTS_VIRT_BASE          IXP4XX_TIMER_BASE_VIRT
#define IX_OSAL_IXP400_USB_VIRT_BASE           IXP4XX_USB_BASE_VIRT
#define IX_OSAL_IXP400_EXP_CFG_VIRT_BASE       IXP4XX_EXP_CFG_BASE_VIRT
#define IX_OSAL_IXP400_PCI_CFG_VIRT_BASE       IXP4XX_PCI_CFG_BASE_VIRT

/*
 * Interrupt Levels
 */
#define IX_OSAL_IXP400_NPEA_IRQ_LVL		IRQ_IXP4XX_NPEA
#define IX_OSAL_IXP400_NPEB_IRQ_LVL		IRQ_IXP4XX_NPEB
#define IX_OSAL_IXP400_NPEC_IRQ_LVL		IRQ_IXP4XX_NPEC
#define IX_OSAL_IXP400_QM1_IRQ_LVL		IRQ_IXP4XX_QM1
#define IX_OSAL_IXP400_QM2_IRQ_LVL		IRQ_IXP4XX_QM2
#define IX_OSAL_IXP400_TIMER1_IRQ_LVL	IRQ_IXP4XX_TIMER1
#define IX_OSAL_IXP400_GPIO0_IRQ_LVL	IRQ_IXP4XX_GPIO0
#define IX_OSAL_IXP400_GPIO1_IRQ_LVL	IRQ_IXP4XX_GPIO1
#define IX_OSAL_IXP400_PCI_INT_IRQ_LVL	IRQ_IXP4XX_PCI_INT
#define IX_OSAL_IXP400_PCI_DMA1_IRQ_LVL	IRQ_IXP4XX_PCI_DMA1
#define IX_OSAL_IXP400_PCI_DMA2_IRQ_LVL	IRQ_IXP4XX_PCI_DMA2
#define IX_OSAL_IXP400_TIMER2_IRQ_LVL	IRQ_IXP4XX_TIMER2
#define IX_OSAL_IXP400_USB_IRQ_LVL		IRQ_IXP4XX_USB
#define IX_OSAL_IXP400_UART2_IRQ_LVL	IRQ_IXP4XX_UART2
#define IX_OSAL_IXP400_TIMESTAMP_IRQ_LVL IRQ_IXP4XX_TIMESTAMP
#define IX_OSAL_IXP400_UART1_IRQ_LVL	IRQ_IXP4XX_UART1
#define IX_OSAL_IXP400_WDOG_IRQ_LVL		IRQ_IXP4XX_WDOG
#define IX_OSAL_IXP400_AHB_PMU_IRQ_LVL	IRQ_IXP4XX_AHB_PMU
#define IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL IRQ_IXP4XX_XSCALE_PMU
#define IX_OSAL_IXP400_GPIO2_IRQ_LVL	IRQ_IXP4XX_GPIO2
#define IX_OSAL_IXP400_GPIO3_IRQ_LVL	IRQ_IXP4XX_GPIO3
#define IX_OSAL_IXP400_GPIO4_IRQ_LVL	IRQ_IXP4XX_GPIO4
#define IX_OSAL_IXP400_GPIO5_IRQ_LVL	IRQ_IXP4XX_GPIO5
#define IX_OSAL_IXP400_GPIO6_IRQ_LVL	IRQ_IXP4XX_GPIO6
#define IX_OSAL_IXP400_GPIO7_IRQ_LVL	IRQ_IXP4XX_GPIO7
#define IX_OSAL_IXP400_GPIO8_IRQ_LVL	IRQ_IXP4XX_GPIO8
#define IX_OSAL_IXP400_GPIO9_IRQ_LVL	IRQ_IXP4XX_GPIO9
#define IX_OSAL_IXP400_GPIO10_IRQ_LVL	IRQ_IXP4XX_GPIO10
#define IX_OSAL_IXP400_GPIO11_IRQ_LVL	IRQ_IXP4XX_GPIO11
#define IX_OSAL_IXP400_GPIO12_IRQ_LVL	IRQ_IXP4XX_GPIO12
#define IX_OSAL_IXP400_SW_INT1_IRQ_LVL	IRQ_IXP4XX_SW_INT1
#define IX_OSAL_IXP400_SW_INT2_IRQ_LVL	IRQ_IXP4XX_SW_INT2

#define IX_OSAL_IXP400_USB_HOST_IRQ_LVL  IRQ_IXP4XX_USB_HOST0
#define IX_OSAL_IXP400_I2C_IRQ_LVL  	 IRQ_IXP4XX_I2C
#define IX_OSAL_IXP400_USB_HOST1_IRQ_LVL IRQ_IXP4XX_USB_HOST1
#define IX_OSAL_IXP400_SSP_IRQ_LVL       IRQ_IXP4XX_SSP
#define IX_OSAL_IXP400_TSYNC_IRQ_LVL  	 IRQ_IXP4XX_TSYNC
#define IX_OSAL_IXP400_EAU_DONE_IRQ_LVL	 IRQ_IXP4XX_EAU_DONE
#define IX_OSAL_IXP400_SHA_HASHING_DONE_IRQ_LVL	IRQ_IXP4XX_SHA_DONE
#define IX_OSAL_IXP400_SWCP_IRQ_LVL		 IRQ_IXP4XX_SWCP_PE
#define IX_OSAL_IXP400_AQM_IRQ_LVL       IRQ_IXP4XX_QM_PE
#define IX_OSAL_IXP400_MCU_IRQ_LVL       IRQ_IXP4XX_MCU_ECC
#define IX_OSAL_IXP400_EBC_IRQ_LVL		 IRQ_IXP4XX_EXP_PE

/* USB interrupt level mask */
#define IX_OSAL_IXP400_INT_LVL_USB             IRQ_IXP4XX_USB

/* USB IRQ */
#define IX_OSAL_IXP400_USB_IRQ                 IRQ_IXP4XX_USB

/* */
#define IX_OSAL_IXP400_ICMR    				IXP4XX_ICMR
#define IX_OSAL_IXP400_ICLR     			IXP4XX_ICLR

/* PMU IRQ */
#define IX_OSAL_OEM_IRQ_PMU                 IRQ_IXP4XX_XSCALE_PMU  


/*
 ===========================================================
 				   Time Related functions
 ===========================================================
 */
/*
 * OS name retrieval
 */
#define IX_OSAL_OEM_OS_NAME_GET(name, limit) \
ixOsalOsIxp400NameGet((INT8*)(name), (INT32) (limit))

/*
 * OS version retrieval
 */
#define IX_OSAL_OEM_OS_VERSION_GET(version, limit) \
ixOsalOsIxp400VersionGet((INT8*)(version), (INT32) (limit))

/*
 * Function to retrieve the OS name
 */
PUBLIC IX_STATUS ixOsalOsIxp400NameGet(INT8* osName, INT32 maxSize);

/*
 * Function to retrieve the OS version
 */
PUBLIC IX_STATUS ixOsalOsIxp400VersionGet(INT8* osVersion, INT32 maxSize);

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
 * Retrieves the system clock rate 
 */
PUBLIC UINT32 ixOsalOsIxp400SysClockRateGet (void);

#define IX_OSAL_OEM_SYS_CLOCK_RATE_GET ixOsalOsIxp400SysClockRateGet

/*
 * required by FS but is not really platform-specific.
 */
#define IX_OSAL_OEM_TIME_GET(pTv) ixOsalTimeGet(pTv)

/* =============== Enf of Time functions ================ */


/*
 ===========================================================
 				Memory Related Functions 
 ===========================================================
 */

#ifdef ENABLE_IOMEM
/* linux map/unmap functions */
PUBLIC void ixOsalLinuxMemMap (IxOsalMemoryMap * map);

PUBLIC void ixOsalLinuxMemUnmap (IxOsalMemoryMap * map);

/* ==================== End - Memory ==================== */

#endif /* ENABLE_IOMEM */

/*
 ===========================================================
 				Thread Related Functions 
 ===========================================================
 */

									  
/* ===================== End - Thread =================== */


/* 
 =========================================================== 
                 Semaphore Related Functions 
 ===========================================================
 */

/* 
 * Fast mutex handle - fast mutex operations are implemented in
 * native assembler code using atomic test-and-set instructions 
 */
//typedef int IxOsalOsOemFastMutex;

IX_STATUS ixOsalixp4XXFastMutexInit(IxOsalFastMutex *mutex);

#define IX_OSAL_OEM_FAST_MUTEX_INIT(arg)        ixOsalixp4XXFastMutexInit(arg)

IX_STATUS ixOsalixp4XXFastMutexTryLock(IxOsalFastMutex *mutex);

#define IX_OSAL_OEM_FAST_MUTEX_TRYLOCK(arg)     ixOsalixp4XXFastMutexTryLock(arg)

IX_STATUS ixOsalixp4XXFastMutexUnlock(IxOsalFastMutex *mutex);

#define IX_OSAL_OEM_FAST_MUTEX_UNLOCK(arg)      ixOsalixp4XXFastMutexUnlock(arg)

IX_STATUS ixOsalixp4XXFastMutexDestroy(IxOsalFastMutex *mutex);

#define IX_OSAL_OEM_FAST_MUTEX_DESTROY(arg)     ixOsalixp4XXFastMutexDestroy(arg)
						 
/* =================== End - Semaphore ================== */


/*
 ===========================================================
                      Irq Related Functions 
 ===========================================================
 */
						 
void ixOsalixp4XXSetInterruptedPc(struct pt_regs *regs);

#define IX_OSAL_OEM_SET_INTERRUPTED_PC(regs)    ixOsalixp4XXSetInterruptedPc(regs)
						  
/* ===================== End - Irq ====================== */
  


#include "IxOsalOsOemSys.h"

#endif /* #define IxOsalOsOem_H */
