/*-
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
 **/

#if !defined(__LAC_MODULE_H__)
#define __LAC_MODULE_H__

#include "icp_qat_hw.h"

/**
*****************************************************************************
 * @ingroup LacModule
 *      Component state
 *
 * @description
 *      This struct type is used to hold the settings for LAC's statistics
 *      collection.
 *
 *****************************************************************************/

#define LAC_KPARAM_MAX_STRLEN (8)

typedef enum {
    ICP_CRYPTO_STATISTIC_ERROR  = -1,
    ICP_CRYPTO_STATISTIC_OFF    = 0,
    ICP_CRYPTO_STATISTIC_ON     = 1,
} icp_crypto_statistics_t;

typedef union {
    icp_crypto_statistics_t istat;
    char sstat[LAC_KPARAM_MAX_STRLEN];
} lac_kparam_u;


typedef struct lac_statistics_s {
    lac_kparam_u dsa;
    lac_kparam_u dh;
    lac_kparam_u ln;
    lac_kparam_u prime;
    lac_kparam_u rsa;
    lac_kparam_u key;
    lac_kparam_u cb;
    lac_kparam_u alg_chain;
    lac_kparam_u random;
    lac_kparam_u master;
    lac_kparam_u msgs;
} lac_statistics_t;

typedef struct lac_kparams_s {
   lac_statistics_t       statistics;
   icp_qat_hw_auth_mode_t qatHmacMode;
   int                    verbose;
} lac_kparams_t;

extern lac_kparams_t icp_crypto;

/* Lac module getter/setter for TUNABLE_INT in lac_module.c */
icp_qat_hw_auth_mode_t Lac_GetQatHmacMode(void);
void Lac_SetQatHmacMode(const icp_qat_hw_auth_mode_t );

#endif
