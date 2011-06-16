/**
 **************************************************************************
 * @file asd_uclo_ldr.c
 *
 * @description
 *      This file contains ASD microcode loader code
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
#include "asd_drv.h"
#include "asd_uclo_ldr.h"
#include "uclo.h"

#define HAL_INIT_MASK 0xF

/* Hal prototypes */
extern int  halAe_Init(unsigned int aeMask);
extern void halAe_DelLib(void);
#define HALAE_SUCCESS 0

/*****************************************************************************
 * asd_uclo_ldr_init
 *
 * This function initialise hal uclo loader
 ******************************************************************************/

CpaStatus asd_uclo_ldr_init(icp_accel_dev_t *accel_dev)
{
        CpaStatus status = CPA_STATUS_SUCCESS;
        int hal_status = 0;

        if ( NULL == accel_dev ) {
                ASD_ERROR("Invalid Handle\n");
                return CPA_STATUS_INVALID_PARAM;
        }

        /* initialize the hal library */
        hal_status = halAe_Init( HAL_INIT_MASK );
        if ( HALAE_SUCCESS != hal_status ) {
                ASD_ERROR("Error initialising HAL library\n");
                return CPA_STATUS_FAIL;
        }
        /*
         * Init Ucode Object File Loader facilities
         */
        UcLo_InitLib();
        return status;
}


/*****************************************************************************
 * asd_uclo_ae_map_ucode
 *
 * This function maps objects using hal
 ******************************************************************************/

CpaStatus asd_uclo_ae_map_ucode(icp_accel_dev_t *accel_dev)
{
        CpaStatus status = CPA_STATUS_SUCCESS;
        int hal_status = 0;
        if ( NULL == accel_dev ) {
                ASD_ERROR("Invalid parameter accel_dev\n");
                return CPA_STATUS_INVALID_PARAM;
        }

        if ( NULL != accel_dev->icp_firmware_loader_handle ) {
                ASD_ERROR("Object File mapped already \n");
                return CPA_STATUS_FAIL;
        }

        /*
         * Map Ucode Object File
         */
        hal_status = UcLo_MapObjAddr(
                             &accel_dev->icp_firmware_loader_handle,
                             (void *) accel_dev->pUofFirmwareLocation->data,
                             accel_dev->pUofFirmwareLocation->size,
                             0 );
        if ( HALAE_SUCCESS != hal_status ) {
                ASD_ERROR("Error mapping object file. Error code %d\n", hal_status);
                status = CPA_STATUS_FAIL;
        }

        return status;
}

/*****************************************************************************
 * asd_uclo_ae_load
 *
 * This function loads the mcode to ae using hal
 ******************************************************************************/

CpaStatus asd_uclo_ae_load(icp_accel_dev_t *accel_dev)
{
        CpaStatus status = CPA_STATUS_SUCCESS;
        int hal_status = 0;

        if ( NULL == accel_dev ) {
                ASD_ERROR("Invalid parameter accel_dev\n");
                return CPA_STATUS_INVALID_PARAM;
        }
        if ( NULL == accel_dev->icp_firmware_loader_handle ) {
                ASD_ERROR("Object File not mapped \n");
                return CPA_STATUS_INVALID_PARAM;
        }

        hal_status = UcLo_WriteUimageAll(
                             accel_dev->icp_firmware_loader_handle );

        if ( HALAE_SUCCESS != hal_status ) {
                ASD_ERROR("Error loading object file to AE %d\n", hal_status);
                status = CPA_STATUS_FAIL;
        }

        return status;
}


/*****************************************************************************
 * asd_uclo_ldr_stop
 *
 * This function deletes the mcode
 ******************************************************************************/

CpaStatus asd_uclo_ldr_stop(icp_accel_dev_t *accel_dev)
{
        CpaStatus status = CPA_STATUS_SUCCESS;
        int hal_status = 0;

        if ( NULL == accel_dev ) {
                ASD_ERROR("Invalid parameter accel_dev\n");
                return CPA_STATUS_INVALID_PARAM;
        }
        if ( NULL == accel_dev->icp_firmware_loader_handle ) {
                ASD_ERROR("Object File not mapped \n");
                return CPA_STATUS_INVALID_PARAM;
        }

        hal_status = UcLo_DeleObj(accel_dev->icp_firmware_loader_handle );
        if ( HALAE_SUCCESS != hal_status ) {
                ASD_ERROR("Error removing object file handle %d\n", hal_status);
                status = CPA_STATUS_FAIL;
        }

        halAe_DelLib();

        accel_dev->icp_firmware_loader_handle = NULL;

        return status;
}
