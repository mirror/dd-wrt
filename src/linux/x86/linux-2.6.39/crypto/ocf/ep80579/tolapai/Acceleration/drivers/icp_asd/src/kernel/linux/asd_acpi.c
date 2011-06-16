/**
 **************************************************************************
 * @file asd_acpi.c
 *
 * @description
 *      This file conatins the source to extract NCDRAM/CDRAM information
 *      from the ACPI tables setup by the BIOS.
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
#include <linux/kernel.h>
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
#include <acpi/acpi_drivers.h>
#include "asd_drv.h"
#include "asd_dram.h"

/*
 * Defines used to initialise the device class, hardware id and name in the 
 * acpi_driver structure
 */
#define ACPI_MEMORY_DEVICE_CLASS "memory"
#define ACPI_MEMORY_DEVICE_HID   "PNP0C02"
#define ACPI_MEMORY_DEVICE_NAME  "P2P1"

/*
 * Defines for the object names setup in ACPI table.
 */
#define NCDRAM_BASE_POINTER "PNMB"
#define NCDRAM_SIZE "SNMB"
#define CDRAM_BASE_POINTER "PCMB"
#define CDRAM_SIZE "SCMB"

ACPI_MODULE_NAME("asd_acpi_ncdram_cdram");

/*
 * Macro to extract the integer value from the acpi_object which has been
 * retrieved from the table.
 */
#define EXTRACT_NCDRAM_CDRAM_INFO( pointer, variable_to_set  )\
  ( {   union acpi_object *package = pointer;            \
        if ((package) == NULL)\
            return FAIL; \
        else{ \
            variable_to_set = (package)->integer.value;\
            variable_to_set <<= PAGE_SHIFT;\
        }\
    })


/* 
 * Declaration of asd_acpi_ncdram_cdram_scan; definition later in the file.
 */
static int asd_acpi_ncdram_cdram_scan(struct acpi_device *);
/*
 * Declaration of asd_init_acpi_driver; definition later in the file
 */
static int asd_init_acpi_driver(void);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
static struct acpi_device_id memory_device_ids[] = {
        {ACPI_MEMORY_DEVICE_HID},
        {"",0},
};
static int acpi_ncdram_cdram_add(struct acpi_device * device) {
        return 0;
}
static int acpi_ncdram_cdram_remove(struct acpi_device * device, int type) {
        return 0;
}
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)

/*
 * Setup the device name, class, hardware id and operations in the acpi_driver
 * structure
 */
static struct acpi_driver acpi_ncdram_cdram_driver = {
        .name = ACPI_MEMORY_DEVICE_NAME,
        .class = ACPI_MEMORY_DEVICE_CLASS,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
        .ids = ACPI_MEMORY_DEVICE_HID,
        .ops = {
                .scan = asd_acpi_ncdram_cdram_scan,
#else
        .ids = memory_device_ids,
        .ops = {
                .add = acpi_ncdram_cdram_add,
                .remove = acpi_ncdram_cdram_remove,
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
                },
};

static acpi_handle asd_ncdram_cdram_handle = NULL;

/*
 * asd_get_acpi_vars
 * Retrieve the NCDRAM / CDRAM details setup by the BIOS in the 
 * ACPI tables.
 */
int asd_get_acpi_vars(icp_accel_dev_t * accel_dev)
{
        int status = SUCCESS;
        struct acpi_device *asd_acpi_device = NULL;

        status = asd_init_acpi_driver();
        if (status)
                return FAIL;

        status = asd_acpi_ncdram_cdram_scan(asd_acpi_device);

        /*
         * Regardless of whether the scan returns a SUCCESS/FAIL the
         * acpi driver must be unregistered.
         */
        acpi_bus_unregister_driver(&acpi_ncdram_cdram_driver);

        /*
         * If the scan was unsuccessful return a FAIL, otherwise return SUCCESS
         */
        if (status)
                return FAIL;

        return SUCCESS;
}

/*
 * asd_acpi_ncdram_cdram_scan
 *
 * Function to retrieves each of the ACPI NCDRAM/CDRAM variables and updates the
 * global variables icp_asd_ncdram_base, icp_asd_ncdram_size, icp_asd_cdram_base, icp_asd_cdram_size
 *
 */
static int asd_acpi_ncdram_cdram_scan(struct acpi_device *asd_acpi_device)
{
        int result = 0;
        acpi_status status = 0;
        struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

        result = acpi_bus_get_device(asd_ncdram_cdram_handle, &asd_acpi_device);
        if (result) {
                ASD_ERROR("Cannot get ACPI bus device\n");
                return FAIL;
        }
        ASD_DEBUG("Device found by acpi_bus_get_device\n");

        status =
            acpi_evaluate_object(asd_acpi_device->handle, NCDRAM_BASE_POINTER,
                                 NULL, &buffer);
        if (ACPI_FAILURE(status)) {
                ACPI_EXCEPTION((AE_INFO, status,
                                "Failed to evaluate NCDRAM Base"));
                return FAIL;
        }

        /* Extract the NCDRAM base address from the acpi_buffer package */
        EXTRACT_NCDRAM_CDRAM_INFO( (union acpi_object *)buffer.pointer,
                                    icp_asd_ncdram_base);


        status =
            acpi_evaluate_object(asd_acpi_device->handle, NCDRAM_SIZE, NULL,
                                 &buffer);
        if (ACPI_FAILURE(status)) {
                ACPI_EXCEPTION((AE_INFO, status,
                                "Failed to evaluate NCDRAM Size"));
                return FAIL;
        }

        /* Extract the NCDRAM size from the acpi_buffer package */
        EXTRACT_NCDRAM_CDRAM_INFO((union acpi_object *)buffer.pointer,
                                   icp_asd_ncdram_size);

        status =
            acpi_evaluate_object(asd_acpi_device->handle, CDRAM_BASE_POINTER,
                                 NULL, &buffer);
        if (ACPI_FAILURE(status)) {
                ACPI_EXCEPTION((AE_INFO, status,
                                "Failed to evaluate CDRAM Base"));
                return FAIL;
        }

        /* Extract the CDRAM base address from the acpi_buffer package */
        EXTRACT_NCDRAM_CDRAM_INFO((union acpi_object *)buffer.pointer,
                                   icp_asd_cdram_base);

        status =
            acpi_evaluate_object(asd_acpi_device->handle, CDRAM_SIZE, NULL,
                                 &buffer);
        if (ACPI_FAILURE(status)) {
                ACPI_EXCEPTION((AE_INFO, status,
                                "Evaluating CDRAM Size"));
                return FAIL;
        }

        /* Extract the CDRAM size from the acpi_buffer package */
        EXTRACT_NCDRAM_CDRAM_INFO((union acpi_object *)buffer.pointer,
                                   icp_asd_cdram_size);

        return SUCCESS;
}

/*
 * asd_init_acpi_driver
 *
 * Function which registers the acpi bus driver and gets a handle to the
 * specified ACPI device.
 */
static int asd_init_acpi_driver(void)
{
        int result = 0;
        acpi_status status = 0;

        if (acpi_disabled) {
                ASD_WARN("ACPI is not enabled \n");
                return FAIL;
        }
        result = acpi_bus_register_driver(&acpi_ncdram_cdram_driver);

        if (result < 0) {
                ASD_ERROR("failed call to acpi_bus_register_driver \n");
                return FAIL;
        }

        ASD_DEBUG("acpi_ncdram_cdram_driver registered\n");

        /*
         * Get the handle into the ACPI table to allow access to the methods to
         * retrieve the NCDRAM/CDRAM info.
         * These methods are defined under the device scope of the ACP device
         * on the PCI bus where the device resides ex: Ring Cluster PCI device.
         * The routine will search for a caller specified name i.e. 
         * "\\SB.PCI0.P2P1.RGCL"  in the name space and return the acpi_handle.
         */
        status =
            acpi_get_handle(NULL, "\\_SB.PCI0.P2P1.RGCL",
                            &asd_ncdram_cdram_handle);

        if (ACPI_FAILURE(status)) {
                ASD_ERROR("ACPI device handle not found\n");

                acpi_bus_unregister_driver(&acpi_ncdram_cdram_driver);
                return FAIL;
        }

        ASD_DEBUG("ACPI device handle found\n");

        return SUCCESS;
}

