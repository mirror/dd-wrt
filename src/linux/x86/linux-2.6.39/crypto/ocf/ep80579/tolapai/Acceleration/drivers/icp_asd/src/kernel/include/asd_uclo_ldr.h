/**
 **************************************************************************
 * @file asd_uclo_ldr.h
 *
 * @description
 *      This is the header file for uclo loader services.
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

#ifndef _ASD_UCLO_LDR_H_
#define _ASD_UCLO_LDR_H_

#include "cpa.h"
#include "icp_accel_handle.h"

/*
 * asd_uclo_ldr_init
 * Initialise asd_uclo and submodules i.e. HAL, HAL_UcLo
 * Returns: CPA_STATUS_SUCCESS Success,
 *          other than CPA_STATUS_SUCCESS - Failure
 */
CpaStatus asd_uclo_ldr_init(icp_accel_dev_t *accel_dev);

/*
 * asd_rm_stop
 * Initialise asd_uclo and submodules i.e. HAL
 * Returns: CPA_STATUS_SUCCESS Success,
 *          other than CPA_STATUS_SUCCESS - Failure
 */
CpaStatus asd_uclo_ldr_stop(icp_accel_dev_t *accel_dev);

/*
 * asd_uclo_ae_map_ucode
 * Returns: CPA_STATUS_SUCCESS Success,
 *          other than CPA_STATUS_SUCCESS - Failure
 */
CpaStatus asd_uclo_ae_map_ucode(icp_accel_dev_t *accel_dev);

/*
 * asd_uclo_ae_load
 * Returns: CPA_STATUS_SUCCESS Success,
 *          other than CPA_STATUS_SUCCESS - Failure
 */
CpaStatus asd_uclo_ae_load(icp_accel_dev_t *accel_dev);


#endif	/* _ASD_UCLO_LDR_H_ */
