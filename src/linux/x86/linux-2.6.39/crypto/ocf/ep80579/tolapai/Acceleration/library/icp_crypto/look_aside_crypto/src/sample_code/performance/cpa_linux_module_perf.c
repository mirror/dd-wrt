/******************************************************************************
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
 *****************************************************************************/

/**
*****************************************************************************
 * @file cpa_linux_module_perf.c
 * 
 * This file contains the init and shutdown functions for the Look Aside
 * linux kernel Module. 
 * 
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include "cpa.h"
#include "cpa_cy_common.h"
#include "cpa_cy_sym.h"
#include "cpa_cy_rand.h"
#include "cpa_cy_im.h"
#include "cpa_perf_defines.h"


/*****************************************************************************/

static CpaStatus samplePerformanceInit(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    /* Start Cryptographic component */

    /* Start instance*/
    status = cpaCyStartInstance(CPA_INSTANCE_HANDLE_SINGLE);

    if (CPA_STATUS_SUCCESS != status)
    {
        PRINT_ERR("\ncpaCyStartInstance failed. Status = %d)\n", status);
            return status;
    }

    if (CPA_STATUS_SUCCESS != samplePerformance(&sampleCipherPerform) )
    {
        PRINT_ERR("sampleCipherPerform failed\n");
            return CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS != samplePerformance(&sampleHashPerform) )
    {
        PRINT_ERR("sampleHashPerform failed\n");
            return CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS != samplePerformance(&sampleAlgoChainPerform) )
    {
        PRINT_ERR("sampleAlgoChainPerform failed\n");
            return CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS != sampleRsaPerform() )
    {
        PRINT_ERR("RSA Decrypt 1024 bit failed\n");
            return CPA_STATUS_FAIL;
    }

    PRINT_INFO("Performance Code Complete- All Tests Pass\n");

    return status;
}

/*****************************************************************************/

static void samplePerformanceExit(void)
{
    
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Stop Cryptographic component */
    status = cpaCyStopInstance(CPA_INSTANCE_HANDLE_SINGLE);

    if (CPA_STATUS_SUCCESS != status)
    {
        PRINT_ERR("cpaCyStopInstance failed. Status = %d\n", status);
    }

    PRINT_INFO("Unloading Performance Sample code module\n");
}

module_init(samplePerformanceInit);
module_exit(samplePerformanceExit);

MODULE_AUTHOR("Intel Corporation");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Performance Sample Code");
