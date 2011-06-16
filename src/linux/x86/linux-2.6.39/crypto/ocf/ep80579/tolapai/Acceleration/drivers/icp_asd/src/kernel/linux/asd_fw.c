/**
 *****************************************************************************
 * @file  asd_fw.c
 *
 * @description
 *        This file contains the firmware loading code for the Acceleration
 *        System Driver.
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
 *****************************************************************************/

/*****************************************************************************
 *
 * Contents:
 *
 *        Definitions of following local functions:
 *            asd_load_firmware
 *            asd_release_firmware
 *
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/firmware.h>

#include "icp_asd_cfg.h"
#include "asd_cfg.h"
#include "asd_drv.h"
#include "icp_accel_handle.h"

#define FW_FAIL_REASON "Failure may have occured as the requested firmware "\
                       "image does not exist or a timeout may have expired "\
                       "during the firmware request. Ensure the requested "\
                       "firmware image exists and the "\
                       "/sys/class/firmware/timeout attribute is sufficient "\
                       "for the firmware image being loaded. "

/*****************************************************************************
 * Abstract:
 *    This function initiates the loading of the firmware into memory.
 *    The ASD shall use the Kernel Firmware API to load microcode into system 
 *    memory. A call to the request_firmware function requests that a user 
 *    space component locate and provide the required firmware image to the 
 *    kernel.
 *
 * Side Effects:
 *
 * Assumptions:
 *
 *****************************************************************************/
int asd_load_firmware(icp_accel_dev_t * accel_dev)
{
        int return_code=0;


        /*
         * Call request_firmware to request the user 
         * space to locate and provide a kernel image 
         */
        return_code = request_firmware(&accel_dev->pUofFirmwareLocation,
                                      "uof_firmware.bin",
                                      &accel_dev->ringCtrlr.pDev->dev);

        if (return_code != 0) {
                ASD_ERROR("Failed to load UOF Firmware\n");
                ASD_INFO("%s \n", FW_FAIL_REASON);
                return FAIL;
        }

        return_code = request_firmware(&accel_dev->pMmpFirmwareLocation,
                                      "mmp_firmware.bin",
                                      &accel_dev->ringCtrlr.pDev->dev);
        if (return_code !=0 ) {
                ASD_ERROR("Failed to load MMP Firmware\n");
                ASD_INFO("%s \n", FW_FAIL_REASON);
                return FAIL;
        }

        /* 
         * Set the ICP_ASD_CFG_PARAM_UOF/MMP_ADDRESS and          
         * ICP_ASD_CFG_PARAM_UOF/MMP_SIZE_BYTES parameters in the
         * configuration table                                
         */

        return_code = asd_cfg_param_set(ICP_ASD_CFG_PARAM_MMP_ADDRESS,
                                       (icp_asd_cfg_value_t)
                                       accel_dev->pMmpFirmwareLocation->data);

        if (return_code != 0) {
                ASD_ERROR("Failed to set ICP_ASD_CFG_PARAM_MMP_ADDRESS\n");
                return FAIL;
        }

        return_code = asd_cfg_param_set(ICP_ASD_CFG_PARAM_MMP_SIZE_BYTES,
                                       accel_dev->pMmpFirmwareLocation->size);

        if (return_code != 0) {
                ASD_ERROR("Failed to set ICP_ASD_CFG_PARAM_MMP_SIZE_BYTES\n");
                return FAIL;
        }


        return_code = asd_cfg_param_set(ICP_ASD_CFG_PARAM_UOF_ADDRESS,
                                       (icp_asd_cfg_value_t)
                                       accel_dev->pUofFirmwareLocation->data);

        if (return_code != 0) {
                ASD_ERROR("Failed to set ICP_ASD_CFG_PARAM_UOF_ADDRESS\n");
                return FAIL;
        }

        return_code = asd_cfg_param_set(ICP_ASD_CFG_PARAM_UOF_SIZE_BYTES,
                                       accel_dev->pUofFirmwareLocation->size);

        if (return_code != 0) {
                ASD_ERROR("Failed to set ICP_ASD_CFG_PARAM_UOF_SIZE_BYTES\n");
                return FAIL;
        }

        return SUCCESS;
}

/*****************************************************************************
 * Abstract:
 *    This function initiates the cleanup of the resources used during firmware
 *    loading.
 *
 * Side Effects:
 *
 * Assumptions:
 *
 *****************************************************************************/
int asd_release_firmware(icp_accel_dev_t * accel_dev)
{

        if (accel_dev == NULL) {
                ASD_ERROR("Invalid parameter supplied\n");
                return FAIL;
        }

        /* 
         * Release the in-kernel firmware structure
         */
        release_firmware(accel_dev->pUofFirmwareLocation);
        release_firmware(accel_dev->pMmpFirmwareLocation);

        return SUCCESS;
}
