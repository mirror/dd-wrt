/**************************************************************************
 * ASD:
 *      Public Header file for layout of icp_accel_dev_t structure.
 *
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
 *
 **************************************************************************/
/**
 *****************************************************************************
 * @file icp_accel_handle.h
 * 
 * @defgroup icp_AsdAccelHandle Acceleration System Driver AccelHandle Interface
 * 
 * @ingroup icp_Asd
 *
 * @description
 *      This is the top level header file that contains the layout of the ASD
 *      icp_accel_dev_t structure and related macros/definitions.
 *      It can be used to dereference the CpaInstanceHandle passed into upper
 *      layers.
 *
 *****************************************************************************/

#ifndef ICP_ACCEL_HANDLE_H
#define ICP_ACCEL_HANDLE_H 1

#if (defined (__linux__) && defined (__KERNEL__))
#include <linux/types.h>
#include <asm/atomic.h>
#include <linux/pci.h>
#include <linux/firmware.h>
#endif

#if (defined (__freebsd) && defined (__KERNEL))
#include <sys/bus.h>
#endif

#ifndef WCSE
#define WCSE
#endif

/*
 * PCI Macros
 */

#define PCI_VENDOR_ID_INTEL 0x8086

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Device ID for the AE Cluster without ACP i.e. non-accelerated
 *    
 *****************************************************************************/
#define ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_NO_ACP 0x502C


/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Device ID for the AE Cluster with ACP on A0 silicon
 *    
 *****************************************************************************/
#define ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP 0x502C


/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Device ID for the AE Cluster with ACP
 *    
 *****************************************************************************/
#define ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP 0x502D


/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Device ID for the Ring Controller
 *    
 *****************************************************************************/
#define ICP_TLP_PCI_DEVICE_ID_RING_CTRLR 0x503F

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      BAR Registers for AE Cluster / Ring Controller
 *    
 *****************************************************************************/
#define ICP_MAX_PCI_BARS 6 


/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Number of MSI messages supported by the Ring Controller
 *    
 *****************************************************************************/

#define ICP_RING_CTRLR_MSGS 1

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *       4K CAP & Conv. Bridge register
 *    
 *****************************************************************************/
#define ICP_AE_CLUSTER_CAP_BAR         0 

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *       16K Scratch memory
 *    
 *****************************************************************************/
#define ICP_AE_CLUSTER_SP_BAR          1 

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      16K AE CSRs and transfer registers
 *    
 *****************************************************************************/
#define ICP_AE_CLUSTER_AE_BAR          2 

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      4K SSU CSRs
 *    
 *****************************************************************************/
#define ICP_AE_CLUSTER_SSU_BAR         3

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      AE Cluster Base Address Registers
 *    
 *****************************************************************************/
#define ICP_AE_CLUSTER_BARS            4

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      4K control register
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_CSR_BAR    0

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      4K Get/Put access
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_RING_BAR   1

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *       256K on-chip SRAM
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_SRAM_BAR   2

#ifdef WCSE
/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      CDRAM Base Address Register
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_CDRAM_BAR  3

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      NCDRAM Base Address Register
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_NCDRAM_BAR 4

#define ICP_WHOLECHIP_BARS             5


/* 
 * Workaround for Simulation environment issue to force the PCI interrupt working on level trigger mode
 */
#define ICP_SIM_PCI_CHECK(pdev)  { \
                unsigned char elcr[2];\
                unsigned int irq; \
                ASD_DEBUG("setting up WCS workaround for PCI level-triggered interrupts.\n");\
                elcr[1] = inb(0x04d1); \
                elcr[0] = inb(0x04d0); \
                irq = pdev->irq;                                        \
                elcr[irq>>3] |= (1 << (irq & 7)); \
                outb(elcr[1],0x04d1); \
                outb(elcr[0],0x04d0); \
        }
#endif
/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      The number of Ring Controller Base Address Registers
 *    
 *****************************************************************************/
#define ICP_RING_CONTROLLER_BARS       3


/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Accelerator capabilities
 *    
 *****************************************************************************/
typedef enum 
{
        ICP_ACCEL_CAPABILITIES_CRYPTO_SYMMETRIC=0,
        ICP_ACCEL_CAPABILITIES_CRYPTO_ASYMMETRIC,
        ICP_ACCEL_CAPABILITIES_RAID5,
        ICP_ACCEL_CAPABILITIES_RAID6
} icp_accel_capabilities_t;


typedef struct icp_accel_bar_s {
        Cpa64U               baseAddr;  /* read from PCI config */
        Cpa64U               virtAddr; /* mapped to kernel VA */
        Cpa32U               size; /* size of BAR */
} icp_accel_bar_t;

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Device States
 *    
 *****************************************************************************/
typedef enum 
{
        ICP_ASD_STATE_UNINITIALIZED=0,
        ICP_ASD_STATE_INITIALIZED,
        ICP_ASD_STATE_STARTED,               
        ICP_ASD_STATE_STOPPED,
        ICP_ASD_STATE_SHUTDOWN
} icp_asd_state_t;

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      PCI information - may be reference by any component
 *    
 *****************************************************************************/
typedef struct icp_accel_pci_info_s 
{
#if (defined (__linux__) && defined (__KERNEL__))
        struct pci_dev                *pDev;
#endif

#if (defined (__freebsd) && defined (__KERNEL))
        device_t pDev;
        struct resource *irqResource;
        void *intr_cookie;
        struct resource *resourceList[ICP_MAX_PCI_BARS];
        int memoryIdList[ICP_MAX_PCI_BARS];
#endif

        icp_asd_state_t       state;
        Cpa32U                deviceId;
        Cpa32U                irq;
        Cpa32U                numBars;
        icp_accel_bar_t       pciBars[ICP_MAX_PCI_BARS];

        Cpa32U                useMSI;
        Cpa64U                coreAffinityMask;
        Cpa8U                 revisionId;
} icp_accel_pci_info_t;

/**
 *****************************************************************************
 * @ingroup icp_AsdAccelHandle
 *
 * @description
 *      Accelerator Device Structure.
 *    
 *****************************************************************************/
typedef struct accel_dev_s 
{
        /* Some generic information may be used by all components */
        Cpa8U                                *pAccelName;                    /* Name given to accelerator */
        Cpa64U                        accelCapabilitiesMask;          /* Accelerator's capabilities mask */
        Cpa64U                        firmwareVersion;                /* Firmware version */

#if (defined (__linux__) && defined (__KERNEL__))
        const struct firmware                  *pUofFirmwareLocation;            
        const struct firmware                *pMmpFirmwareLocation;            

#endif

#if (defined (__freebsd) && defined (__KERNEL))
        struct firmware                  *pUofFirmwareLocation;            
        struct firmware                        *pMmpFirmwareLocation;            
#endif

#if (defined (__linux__) && defined (__KERNEL__))
        atomic_t                        outstandingRequests;            /* Number of outstanding requests to accelerator */
#endif
        icp_accel_pci_info_t                aeCluster;                    /* AE Cluster PCI Device */
        icp_accel_pci_info_t                ringCtrlr;                    /* Ring Controller PCI Device */

        /* Component specific fields - cast to relevent layer */
        void                                *pAsdHandle;                    /* For ASD */
        void                                *pHalHandle;                    /* For HAL layer */
        void                                *pRmHandle;                     /* For RM layer */
        void                                *pLacHandle;                    /* For LAC layer */
        void                                *pFastpathHandle;               /* For Fastpath layer */
        void                                *pInlineHandle;                 /* For Inline layer */
        void                                *pSalHandle;                        /* For Storage layer */
        void                                *pXmlHandle;                        /* For XML layer */

	/* Firmware loader handle */
	void                                *icp_firmware_loader_handle;

        /* Linked list of accel_handles - for easy traversing */
        struct accel_dev_s                *pNext;
        struct accel_dev_s                *pPrev;
} icp_accel_dev_t;

#endif /* ICP_ACCEL_HANDLE_H */
