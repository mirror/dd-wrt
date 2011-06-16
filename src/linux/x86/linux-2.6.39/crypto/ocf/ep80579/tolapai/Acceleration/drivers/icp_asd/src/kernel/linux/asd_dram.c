/**
 **************************************************************************
 * @file asd_dram.c
 *
 * @description
 *      This file contains ASD code to retrieve NCDRAM / CDRAM information
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
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/efi.h>
#include "asd_cfg.h"
#include "asd_dram.h"
#include "asd_drv.h"

#define BLOCK_SIZE_4K  (1 << 12)
#define MEG_1          (0x100000)
#define MEG_32    (MEG_1 << 5)

/**
 * Function to get the efi variables and set the appropriate parameters
 * in the configuration table with theses variables.
 */
static int asd_get_efi_vars(icp_accel_dev_t * accel_dev)
{
        /* Extracting NCDRAM/CDRAM information from EFI is unsupported */

        return FAIL;
}

/**
 * asd_get_ncdram_cdram_info
 * Function to set the NCDRAM and CDRAM parameters in the configuration table.
 */
int asd_get_ncdram_cdram_info(icp_accel_dev_t * accel_dev)
{
        icp_accel_pci_info_t *pci_info = NULL;
        int status = SUCCESS;

        if (accel_dev == NULL)
                return FAIL;

        pci_info = &accel_dev->ringCtrlr;

        if (pci_info ==NULL)
                return FAIL;


#ifdef WCSE
/*
 * On the Wholechip Simulation Environment the NCDRAM/CDRAM information is setup
 * in additional PCI BARS. Set the values in the config table to the values
 * read from PCI bars.
 */
        if (pci_info->numBars == ICP_WHOLECHIP_BARS) {
                status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_CDRAM_ADDRESS,
                          pci_info->
                          pciBars[ICP_RING_CONTROLLER_CDRAM_BAR].
                          baseAddr);
                if (status)
                        return status;

                status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_CDRAM_SIZE_BYTES,
                          pci_info->
                          pciBars[ICP_RING_CONTROLLER_CDRAM_BAR].size);
                if (status)
                        return status;

                status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_NCDRAM_ADDRESS,
                          pci_info->
                          pciBars[ICP_RING_CONTROLLER_NCDRAM_BAR].
                          baseAddr);
                if (status)
                        return status;

                status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_NCDRAM_SIZE_BYTES,
                          pci_info->
                          pciBars[ICP_RING_CONTROLLER_NCDRAM_BAR].size);
                return status;
        }
#endif

        /* 
         * If module parameters have been specifed in the NCDRAM/CDRAM global
	 * variables by the user the config table is updated with these values.
         */
        if (icp_asd_ncdram_base != 0) {
                if (icp_asd_ncdram_size == 0)
                        icp_asd_ncdram_size = MEG_32;
                else
                        icp_asd_ncdram_size *= BLOCK_SIZE_4K;

                if (icp_asd_cdram_size == 0)
                        icp_asd_cdram_size = icp_asd_ncdram_size;
                else
                        icp_asd_cdram_size *= BLOCK_SIZE_4K;

                if (icp_asd_cdram_base == 0)
                        icp_asd_cdram_base = icp_asd_ncdram_base
                                             + icp_asd_ncdram_size;
        }else{
                /*
                 * Attempt to retrieve the NCRAM/CDRAM Information 
                 * from the efi variables
                 */
                status = asd_get_efi_vars(accel_dev);
                if (status) {
                        /*
                         * Attempt to read the NCRAM/CDRAM information from the
                         * ACPI tables.
                         */
                         status = asd_get_acpi_vars(accel_dev);

#ifdef ASD_DEV_BUILD
                         if (status) {
                                ASD_WARN("Failed to read the NCRAM/CDRAM "
                                         "information from the ACPI table. "
                                         "Attempting to read BIOS "
                                         "Registers \n");
                                /*
                                 * Attempt to check registers setup by BIOS
                                 * to interpret NCDRAM/CDRAM regions
                                 */
                                status = asd_read_bios_registers(accel_dev);
                                if (status != SUCCESS) {
                                        ASD_ERROR("Failed to read BIOS "
                                                 "Registers \n");
                                        return FAIL;
                                }
                        }
#endif
#ifndef ASD_DEV_BUILD
                        if (status) {
                                ASD_ERROR("Failed to read the NCDRAM/CDRAM "
                                          "information from the ACPI "
                                          "table.\n");
                                return FAIL;
                        }
#endif
                }
        }

        ASD_INFO("icp_asd_ncdram_base = 0x%x icp_asd_ncdram_size = 0x%x\n",
                 icp_asd_ncdram_base, icp_asd_ncdram_size);
        ASD_INFO("icp_asd_cdram_base  = 0x%x icp_asd_cdram_size  = 0x%x\n",
                 icp_asd_cdram_base, icp_asd_cdram_size);

        status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_CDRAM_ADDRESS,
                                (icp_asd_cfg_value_t)icp_asd_cdram_base);
        if (status){
                ASD_ERROR("Failed to set "
                          "ICP_ASD_CFG_PARAM_CDRAM_ADDRESS\n");
                return status;
        }

        status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_CDRAM_SIZE_BYTES,
                                (icp_asd_cfg_value_t)icp_asd_cdram_size);
        if (status) {
                ASD_ERROR("Failed to set "
                          "ICP_ASD_CFG_PARAM_CDRAM_SIZE_BYTES\n");
                return status;
        }

        status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_NCDRAM_ADDRESS,
                                (icp_asd_cfg_value_t)icp_asd_ncdram_base);
        if (status){
                ASD_ERROR("Failed to set "
                          "ICP_ASD_CFG_PARAM_NCDRAM_ADDRESS\n");
                return status;
        }

        status = asd_cfg_param_set(ICP_ASD_CFG_PARAM_NCDRAM_SIZE_BYTES,
                                (icp_asd_cfg_value_t)icp_asd_ncdram_size);
        if (status){
                ASD_ERROR("Failed to set "
                          "ICP_ASD_CFG_PARAM_NCDRAM_SIZE_BYTES\n");
                return status;
        }

        return SUCCESS;
}
