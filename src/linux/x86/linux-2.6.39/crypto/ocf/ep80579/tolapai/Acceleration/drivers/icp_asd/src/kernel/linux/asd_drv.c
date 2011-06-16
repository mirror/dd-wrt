/**
 **************************************************************************
 * @file asd_drv.c
 *
 * @description
 *      This file is the main source file for the Acceleration System Driver
 *      code. It contains the module init/release functions, performs PCI
 *      device driver registration, setup of system resource table and calls
 *      the main acceleration subsystem init and shutdown functions.
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
 *
 **************************************************************************/
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#include <linux/version.h>

#include <asm/io.h>

#include "icp_asd_cfg.h"
#include "icp_accel_handle.h"

#include "asd_drv.h"
#include "asd_cfg.h"
#include "asd_init.h"
#include "asd_fw.h"
#include "asd_dram.h"
#include "asd_isr.h"
/*
 * Module static variables
 */

int icp_asd_debug = 0;
int icp_asd_load_fw = 1;
int icp_asd_auto_init = 0;
int icp_asd_reg_isr = 1;
int icp_asd_ncdram_base = 0;
int icp_asd_ncdram_size = 0;
int icp_asd_cdram_base = 0;
int icp_asd_cdram_size = 0;
/* Module parameters */

module_param(icp_asd_debug, int, S_IRUGO);
module_param(icp_asd_load_fw, int, S_IRUGO);
module_param(icp_asd_auto_init, int, S_IRUGO);
module_param(icp_asd_reg_isr, int, S_IRUGO);
module_param(icp_asd_ncdram_base, int, S_IRUGO);
module_param(icp_asd_ncdram_size, int, S_IRUGO);
module_param(icp_asd_cdram_base, int, S_IRUGO);
module_param(icp_asd_cdram_size, int, S_IRUGO);

MODULE_DESCRIPTION("ICP Acceleration System Driver");
MODULE_AUTHOR("Intel Corporation");
MODULE_LICENSE(LICENSE_TYPE);

#define ACCELERATION_ENABLED( value )    ( value & 1 )

#define IS_AE_CLUSTER_DEVICE( device ) \
                ( (( device == ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP) ||  \
                   (device == ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP)) ?\
                    (TRUE) : (FALSE) )

/*
 * PCI Infrastructure prototypes/definitions
 */

/* Macros for setup of Signal Target IA Mask (SMIA) Register */
#define EP805XX_SMIA_REG 0xe8
#define EP805XX_SMIA_REG_MASK_SMIA6 0x40
#define EP805XX_SMIA_REG_MASK_SMIA5 0x20
#define EP805XX_SMIA_REG_MASK_SMIA4 0x10
#define EP805XX_SMIA_REG_MASK_SMIA3 0x8
#define EP805XX_SMIA_REG_MASK_SMIA2 0x4
#define EP805XX_SMIA_REG_MASK_SMIA1 0x2
#define EP805XX_SMIA_REG_MASK_SMIA0 0x1
#define EP805XX_SWSKU_CONFIG_OFFSET 0x44
#define EP805XX_RID_CONFIG_OFFSET   0x08

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
#define KBUILD_MODNAME "icp_asd"
#endif

#define ASD_SYSTEM_DEVICE(device_id) {\
        PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}

static struct pci_device_id asd_pci_tbl[] = {
        ASD_SYSTEM_DEVICE(ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP),
        ASD_SYSTEM_DEVICE(ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP),
        ASD_SYSTEM_DEVICE(ICP_TLP_PCI_DEVICE_ID_RING_CTRLR),
        {0,}
};

MODULE_DEVICE_TABLE(pci, asd_pci_tbl);

char asd_driver_name[] = "icp_asd";
static int asd_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit asd_remove(struct pci_dev *pdev);

static struct pci_driver asd_driver = {
        .name = asd_driver_name,
        .id_table = asd_pci_tbl,
        .probe = asd_probe,
        .remove = __devexit_p(asd_remove),
};

/*
 * Module init/exit prototypes
 */
static int asddrv_init(void);
static void asddrv_release(void);

/* 
 * ASD Accelerator table and functions
 * 
 */

static icp_accel_dev_t *accel_table = NULL;
static int num_accel = 0;

/*
 * asd_accel_table_check
 * Check the accel table for a structure that contains the PCI device
 * Returns a pointer to the accelerator structure or NULL if not found.
 */
static icp_accel_dev_t *asd_accel_table_check(struct pci_dev *pdev)
{
        icp_accel_dev_t *ptr = accel_table;
        while (ptr != NULL) {

                if ((ptr->aeCluster.pDev == pdev)
                    || (ptr->ringCtrlr.pDev == pdev)) {
                        return ptr;
                }

                ptr = ptr->pNext;
        }
        return ptr;
}

/*
 * asd_accel_table_get_device
 * Check the accel table for a structure suitable for the found PCI device.
 * Assume: can have multiple devices in table
 * Returns a pointer to the accelerator structure or NULL if not found or an 
 * error.
 */
static icp_accel_dev_t *asd_accel_table_get_device(struct pci_dev *pdev,
                                                   unsigned int deviceId)
{
        char my_bus = 0;
        icp_accel_dev_t *ptr = NULL;

        if ((pdev == NULL) || (deviceId <= 0)) {
                ASD_ERROR("Invalid parameters supplied\n");
                return NULL;
        }
        my_bus = pdev->bus->number;

        ptr = asd_accel_table_check(pdev);
        if (ptr == NULL) {
                ptr = accel_table;
                if (IS_AE_CLUSTER_DEVICE(deviceId)) {
                        /*
                         * Doesn't exist yet, search for partner ring
                         * controller device on same bus
                         */
                        while (ptr != NULL) {

                                if (ptr->ringCtrlr.pDev->bus->number == my_bus){
                                        return ptr;
                                }

                                ptr = ptr->pNext;
                        }

                        return ptr;

                }

                if (deviceId == ICP_TLP_PCI_DEVICE_ID_RING_CTRLR) {

                        /*
                         * Doesn't exist yet, search for partner AE Cluster
                         * device on same bus
                         */
                        while (ptr != NULL) {

                                if (ptr->aeCluster.pDev->bus->number == my_bus){
                                        return ptr;
                                }

                                ptr = ptr->pNext;
                        }
                        return ptr;
                }

        }

        return ptr;
}

/*
 * asd_accel_table_add
 * Add a new accelerator structure to the Accelerator Table
 */
static void asd_accel_table_add(icp_accel_dev_t * accel_dev)
{

        if (accel_table == NULL) {
                accel_table = accel_dev;
                accel_dev->pNext = NULL;
                accel_dev->pPrev = NULL;
        } else {
                accel_dev->pNext = accel_table;
                accel_table->pPrev = accel_dev;
                accel_dev->pPrev = NULL;
                accel_table = accel_dev;
        }
        return;
}

/*
 * asd_accel_table_remove
 * Remove an accelerator structure from the Accelerator Table
 * Returns SUCCESS on success, FAIL if a NULL pointer supplied
 */
static int asd_accel_table_remove(icp_accel_dev_t * accel_dev)
{
        icp_accel_dev_t *prev = NULL;
        icp_accel_dev_t *next = NULL;

        if (!accel_dev)
                return FAIL;

        prev = accel_dev->pPrev;
        next = accel_dev->pNext;

        if (prev) {
                prev->pNext = accel_dev->pNext;
                if (next)
                        next->pPrev = prev;
        } else {
                accel_table = accel_dev->pNext;
                if (next)
                        next->pPrev = accel_table;
        }
        return SUCCESS;
}

/*
 * asd_accel_device_ready
 * Returns whether an accelerator is in the required state.
 * Both PCI devices must be in same state for TRUE to be returned.
 * Returns TRUE on success, otherwise FALSE.
 */
static int asd_accel_device_ready(icp_accel_dev_t * accel_dev,
                                  icp_asd_state_t state)
{
        int ready = FALSE;

        ASD_DEBUG("aeCluster.deviceId = 0x%x state=%d\n",
                  accel_dev->aeCluster.deviceId, accel_dev->aeCluster.state);

        ASD_DEBUG("ringCtrlr.deviceId = 0x%x state=%d\n",
                  accel_dev->ringCtrlr.deviceId, accel_dev->ringCtrlr.state);

        /* Check that both PCI devices are in required state */
        if ((IS_AE_CLUSTER_DEVICE(accel_dev->aeCluster.deviceId))
            || (accel_dev->ringCtrlr.deviceId ==
                ICP_TLP_PCI_DEVICE_ID_RING_CTRLR)) {

                if ((accel_dev->aeCluster.state == state) &&
                    (accel_dev->ringCtrlr.state == state))
                        ready = TRUE;
        } else {
                if (accel_dev->ringCtrlr.state == state)
                        ready = TRUE;
        }
        return ready;
}

/*
 * asd_cleanup_accel
 * Cleans up any system resources used by an accelerator structure
 */
static int asd_cleanup_accel(icp_accel_dev_t * accel_dev)
{
        int status = 0;

        if (accel_dev == NULL) {
                ASD_ERROR("NULL accelerator structure\n");
                return -EFAULT;
        }

        if (accel_dev->aeCluster.pDev) {
                pci_release_regions(accel_dev->aeCluster.pDev);
                pci_disable_device(accel_dev->aeCluster.pDev);
        }

        if (accel_dev->ringCtrlr.pDev) {
                pci_release_regions(accel_dev->ringCtrlr.pDev);
                pci_disable_device(accel_dev->ringCtrlr.pDev);
        }

        if (icp_asd_load_fw) {
                status = asd_release_firmware(accel_dev);
                if (status != CPA_STATUS_SUCCESS)
                        ASD_ERROR("failed to release firmware\n");
        }

        /* Remove accelerator device from accelerator table */
        status = asd_accel_table_remove(accel_dev);
        if (status)
                ASD_ERROR("failed to remove device from accel table\n");

        kfree(accel_dev);

        return status;
}

/*
 * asd_get_swsku
 *
 * Function which retrieves the software sku information from the ring
 * controller device.
 */
static int asd_get_sw_sku(Cpa8U *sw_sku)
{
        struct pci_dev *pdev = NULL;
        int status = SUCCESS;

        pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
                              ICP_TLP_PCI_DEVICE_ID_RING_CTRLR, NULL);
        if (!pdev) {
                ASD_ERROR("Failed to acquire Ring Controller\n");
                return FAIL;
        }

        pci_read_config_byte(pdev, EP805XX_SWSKU_CONFIG_OFFSET, sw_sku);
        ASD_DEBUG("Ring Controller SWSKU register = 0x%x\n", *sw_sku);

        pci_dev_put(pdev);
        return status;
}

/*
 * is_accelerated_ae_cluster_device
 *
 * Function which determines if the AE Cluster Device specified is an 
 * accelerated device.
 */
int is_accelerated_ae_cluster_device(int device)
{
        Cpa8U sw_sku = 0;
        int status = SUCCESS;

        status = asd_get_sw_sku(&sw_sku);
        if (status == FAIL) {
                ASD_ERROR("Failed to read the SW SKU !!\n");
                return FALSE;
        }

        /* 
         * Unfused A0 parts (used on early CRBs)
         * Device ID 0x502C (with ACP)
         * SWSKU retrieved from Ring Controller config space is all-zeros 
         */
        if ((device == ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP) &&
            (sw_sku == 0)) {
                return TRUE;
        }

        /*
         * Fused A0 parts
         * Device ID 0x502C (without ACP)
         * SWSKU retrieved from Ring Controller config space is correctly 
         * reporting the correct SKU ID.
         */
        else if ((device == ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_NO_ACP) &&
                 (!(ACCELERATION_ENABLED(sw_sku)))) {
                return FALSE;
        }

        /* 
         * Fused A0 parts
         * Device ID 0x502D (with ACP)
         * SWSKU retrieved from Ring Controller config space is correctly
         * reporting the correct SKU ID.
         */

        else if ((device == ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP) &&
                 (ACCELERATION_ENABLED(sw_sku))) {
                return TRUE;
        } else {
                return FALSE;
        }
}

/*
 * asd_probe
 * ICP ASD Device Driver Probe function
 */
static int asd_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{

        int i = 0;
        icp_accel_dev_t *accel_dev = NULL;
        icp_accel_pci_info_t *pci_info = NULL;
        int new_device = 0;
        int status = 0;
        Cpa8U rev_id=0;

        /* Ensure have valid device */
        switch (ent->device) {
        case ICP_TLP_PCI_DEVICE_ID_RING_CTRLR:
                break;
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP:
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP:
                if (!(is_accelerated_ae_cluster_device(ent->device))) {
                        ASD_ERROR
                            ("Non-accelerated device 0x%x not supported !!\n",
                             ent->device);
                        return -ENODEV;
                }
                break;
        default:

                ASD_ERROR("invalid device 0x%x found..!!\n", ent->device);
                return -ENODEV;
        }

        /*
         *     Check Accel Table for an existing accelerator structure
         */
        accel_dev = asd_accel_table_get_device(pdev, ent->device);
        if (accel_dev == NULL) {
                accel_dev = kzalloc(sizeof(icp_accel_dev_t), GFP_KERNEL);
                if (accel_dev == NULL) {
                        ASD_ERROR("failed to allocate accel structure\n");
                        return -ENOMEM;
                }
                new_device = TRUE;
                num_accel++;
        } else {
                new_device = FALSE;
        }

        /* enable PCI device */
        if (pci_enable_device(pdev)) {
                asd_cleanup_accel(accel_dev);
                return -EIO;
        }

        /* set dma identifier */

        if (pci_set_dma_mask(pdev, DMA_64BIT_MASK)) {

                if ((pci_set_dma_mask(pdev, DMA_32BIT_MASK))) {

                        ASD_ERROR("No usable DMA configuration, aborting\n");
                        asd_cleanup_accel(accel_dev);
                        return -EIO;
                }
        }

        if ((status = pci_request_regions(pdev, asd_driver_name))) {
                asd_cleanup_accel(accel_dev);
                return status;
        }

        switch (ent->device) {
        case ICP_TLP_PCI_DEVICE_ID_RING_CTRLR:
                ASD_DEBUG("Ring Controller found..\n");
                pci_info = &accel_dev->ringCtrlr;
                break;
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP:
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP:
                ASD_DEBUG("AE Cluster found..\n");
                pci_info = &accel_dev->aeCluster;
                break;
        default:

                break;
        }

        /* 
         * Setup PCI Info structure for upper layers
         */
        pci_info->pDev = pdev;
        pci_info->deviceId = ent->device;
        pci_info->irq = pdev->irq;
        for (i = 0; i < ICP_MAX_PCI_BARS; i++) {
                /* Find all the device's BARS */
                pci_info->pciBars[i].baseAddr = pci_resource_start(pdev, i);
                if (!pci_info->pciBars[i].baseAddr)
                        break;
                pci_info->pciBars[i].size = pci_resource_len(pdev, i);
                pci_info->numBars++;
        }

        switch (ent->device) {
        case ICP_TLP_PCI_DEVICE_ID_RING_CTRLR:
                ASD_DEBUG("Ring Controller, numBars=%d\n", pci_info->numBars);
                status = asd_get_ncdram_cdram_info(accel_dev);
                if (status) {
                        ASD_ERROR("NCDRAM/CDRAM error\n");
                        asd_cleanup_accel(accel_dev);
                        return -ENOMEM;
                }

                /* 
                 * Enable interrupts to the IA by setting the appropriate bits
                 * in the SMIA register
                 */
                pci_write_config_byte(pdev, EP805XX_SMIA_REG,
                     EP805XX_SMIA_REG_MASK_SMIA4 | EP805XX_SMIA_REG_MASK_SMIA3);
                break;
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP:
        case ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP:
                pci_set_master(pdev);

                /* 
                 * Enable interrupts to the IA by setting the appropriate bits
                 * in the SMIA register
                 */
                pci_write_config_byte(pdev, EP805XX_SMIA_REG,
                                       (EP805XX_SMIA_REG_MASK_SMIA3 |
                                        EP805XX_SMIA_REG_MASK_SMIA2 |
                                        EP805XX_SMIA_REG_MASK_SMIA0));

                break;
        default:

                break;
        }

        /* Retrieve the revision id and add it to the PCI info structure */
        pci_read_config_byte(pdev, EP805XX_RID_CONFIG_OFFSET, &rev_id);
        pci_info->revisionId = rev_id;
        ASD_DEBUG("Revision ID ... 0x%x\n", pci_info->revisionId);
        pci_info->state = ICP_ASD_STATE_INITIALIZED;

        /* Add accel device to table */
        if (new_device) {
                asd_accel_table_add(accel_dev);
        }

        return status;
}

/*
 * asd_remove
 *
 * Don't shutdown subsystem until last PCI device is being removed.
 *
 */
static void asd_remove(struct pci_dev *pdev)
{

        int status = 0;
        icp_accel_dev_t *accel_dev = NULL;
        int remove_device = 0;

        /* Find the accel structure */
        accel_dev = asd_accel_table_check(pdev);
        if (accel_dev == NULL) {
                ASD_ERROR("failed to find accel_dev\n");
                return;
        }

        if (pdev == accel_dev->aeCluster.pDev)
                accel_dev->aeCluster.state = ICP_ASD_STATE_UNINITIALIZED;

        if (pdev == accel_dev->ringCtrlr.pDev)
                accel_dev->ringCtrlr.state = ICP_ASD_STATE_UNINITIALIZED;

        remove_device =
            asd_accel_device_ready(accel_dev, ICP_ASD_STATE_UNINITIALIZED);

       if (TRUE == remove_device) {
                ASD_DEBUG("calling asd_SubsystemShutdown ..\n");
                status = asd_SubsystemShutdown(accel_dev);
                if (status != CPA_STATUS_SUCCESS)
                        ASD_ERROR("Acceleration Subsystem shutdown failure\n");

                status = asd_cleanup_accel(accel_dev);
                if (status)
                        ASD_ERROR("asd_cleanup_accel failure\n");
        }

        return;
}

/*
 * asd_do_init
 *
 * Function to initialize the underlying acceleration subsystem for the
 * required accelerator.
 */
static int asd_do_init(icp_accel_dev_t * accel_dev)
{
        int status = 0;

        if (asd_accel_device_ready(accel_dev, ICP_ASD_STATE_INITIALIZED)) {

                ASD_DEBUG("probe successful.\n");
#ifdef WCSE
                /*
                 * Apply workaround on WCS
                 */
                if (accel_dev->ringCtrlr.numBars == ICP_WHOLECHIP_BARS) {
                        ICP_SIM_PCI_CHECK(accel_dev->ringCtrlr.pDev);
                        ICP_SIM_PCI_CHECK(accel_dev->aeCluster.pDev);
                }
#endif
                /* get firmware from FS */
                if (icp_asd_load_fw) {

                        ASD_DEBUG("loading firmware..\n");
                        status = asd_load_firmware(accel_dev);
                        if (status != CPA_STATUS_SUCCESS) {
                                ASD_ERROR("asd_load_firmware error\n");
                                asd_cleanup_accel(accel_dev);
                                return -EIO;
                        } else
                                ASD_DEBUG("asd_load_firmware success\n");
                }

                ASD_DEBUG("calling asd_SubsystemInit ..\n");
                status = asd_SubsystemInit(accel_dev);
                if (status != CPA_STATUS_SUCCESS) {
                        ASD_ERROR("asd_SubsystemInit error, shutting down\n");
                        asd_SubsystemShutdown(accel_dev);
                        asd_cleanup_accel(accel_dev);
                        return -EIO;
                }

        } else {
                ASD_ERROR("accelerator not ready for init ..\n");
                return -EIO;
        }

        return status;
}

/*
 * asd_init_devices
 *
 * Catchall function to initialize the Acceleration subsystem for all
 * accelerators found in the system
 */
int asd_init_devices(void)
{
        icp_accel_dev_t *accel_dev = accel_table;
        int status = 0;

        if (accel_dev == NULL) {
                ASD_ERROR("No accelerator found \n");
                return FAIL;
        }

        while (accel_dev) {

                status = asd_do_init(accel_dev);
                if (status) {

                        ASD_ERROR("failed to initialize accelerator\n");
                        return status;
                }

                accel_dev = accel_dev->pNext;
        }

         return SUCCESS;
}

/*
 * asddrv_init
 * ICP ASD Module Init function
 */
static int __init asddrv_init()
{
        int status = 0;

        /*
         * Check NCDRAM/CDRAM module params, must specify icp_asd_ncdram_base if 
         * icp_asd_cdram_base is specified
         */
        if ((icp_asd_cdram_base != 0) && (icp_asd_ncdram_base == 0)) {
                ASD_ERROR
                    ("Must specify icp_asd_ncdram_base module "
                     "parameter with icp_asd_cdram_base\n");
                return -EINVAL;
        }

        /* Allocate Resource Config table */
        status = asd_cfg_init();
        if (status) {
                ASD_ERROR("failed call to asd_cfg_init\n");
                return status;
        }

        status = pci_register_driver(&asd_driver);
        if (status) {
                ASD_ERROR("failed call to pci_register_driver\n");

                asd_cfg_free();
                return status;
        }

        /*
        * Check to see if the Ring Controller and AE Cluster PCI devices exist
        * Print a warning message to inform the user if they are not found
        */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
        if ((NULL == pci_find_device(PCI_VENDOR_ID_INTEL,
                        ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP, NULL)) &&
            (NULL == pci_find_device(PCI_VENDOR_ID_INTEL,
                        ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP, NULL))){
#else
        if ((NULL == pci_get_device(PCI_VENDOR_ID_INTEL,
                        ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_WITH_ACP, NULL)) &&
            (NULL == pci_get_device(PCI_VENDOR_ID_INTEL,
                        ICP_TLP_PCI_DEVICE_ID_AE_CLUSTER_A0_WITH_ACP, NULL))){
#endif
                ASD_WARN("Failed to find the AE Cluster PCI Device\n");
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
        if (NULL == pci_find_device(PCI_VENDOR_ID_INTEL,
                                    ICP_TLP_PCI_DEVICE_ID_RING_CTRLR, NULL)){
#else
        if (NULL == pci_get_device(PCI_VENDOR_ID_INTEL,
                                    ICP_TLP_PCI_DEVICE_ID_RING_CTRLR, NULL)){
#endif

                ASD_WARN("Failed to find the Ring Controller PCI Device\n");
        }

        if (icp_asd_auto_init) {
                status = asd_init_devices();
                if (status) {
                        ASD_ERROR("failed call to asd_init_devices\n");

                        pci_unregister_driver(&asd_driver);
                        asd_cfg_free();
                        return -EIO;
                }
        }
        /* Otherwise, going to get a kick from asd_ctl program */

        return 0;
}

/*
 * asddrv_release
 * ASD Module release function
 */
static void __exit asddrv_release()
{

        pci_unregister_driver(&asd_driver);

        asd_cfg_free();

        return;
}

module_init(asddrv_init);
module_exit(asddrv_release);
